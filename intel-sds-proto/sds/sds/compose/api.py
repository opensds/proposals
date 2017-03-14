# Copyright (c) 2014 Intel Corporation
# Copyright (c) 2014 OpenStack Foundation
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

"""
Handles all requests related to storage discovery operations
"""

import datetime
import hashlib
import hmac
import random
import os
import re
import tempfile
import subprocess
import shlex

from oslo.config import cfg

from sds.db import base
from sds.compose import cinder_client
from sds.compose import iniwriter
from sds.common import exception
from sds.openstack.common import excutils
from sds.openstack.common import importutils
from sds.openstack.common import uuidutils
from sds.openstack.common import log as logging
from sds.discover import storage_backends
from sds.discover import storage_tiers
from sds.compose import storage_pools
from sds import discover

compose_opts = [
    cfg.StrOpt('copy_from_remote_cmd',
               default='scp -B %user%@%host%:%file% %temp%',
               help='command to copy .conf file from remote host'),
    cfg.StrOpt('copy_to_remote_cmd',
               default='scp -B %temp% %user%@%host%:%file%',
               help='command to copy .conf file from remote host')
]

CONF = cfg.CONF
CONF.register_opts(compose_opts)

LOG = logging.getLogger(__name__)

class API():
    """API for interacting with composing pools and configuring *.conf files for cinder, manila, nova etc."""

    driver_mod_prefix = "sds.discover.drivers"
    compose_class = "compose"

    def __init__(self):
        LOG.debug("Compose API Init")

    def copy_file(self, cmd, user, host, conf, source):
        tokens = {'%user%' : user, '%host%' : host, '%file%' : conf, '%temp%' : source}
        for token in tokens:
            cmd = re.sub(token, tokens[token], cmd)
        subprocess.check_call(shlex.split(cmd)) 

    """
        update or delete config on specified hosts. If backend matches with existing service then it 
        will only update or delete config on hosts that have this backend.
        mode is either 'update' or 'delete'
        returns upd_sections that include host and actual ini sections used
    """
    def update_conf(self, hosts, conf_file, search_sections, new_sections, user, mode = 'update'):
        # TODO(arc): Any failure aborts processing rest of the hosts. Need this to be fixed later.
        upd_sections = {}
        for host in hosts:
            try:
                (temp, target) = (None, None)
                (fd, temp) = tempfile.mkstemp()
                self.copy_file(CONF.copy_from_remote_cmd, user, host, conf_file, temp)
                (tfd, target) = tempfile.mkstemp()
                upd_sections[host] = iniwriter.IniConfigWriter.change_backends(temp, target, search_sections, 
                                                                               new_sections, mode)
                # backup the current file
                self.copy_file(CONF.copy_to_remote_cmd, user, host, "%s.orig.%s" % (conf_file, uuidutils.generate_uuid()), temp)
                self.copy_file(CONF.copy_to_remote_cmd, user, host, conf_file, target)
                os.remove(temp)
                os.remove(target)
            except Exception as e:
                # in case of exceptions, lets leave temp files in-tact for debugging
                LOG.warn("Unable to update %s file on host %s. Exception: %s" % (conf_file, host, e))
                raise e
        return upd_sections


    """
        This method provides the ini sections and search sections. It depends on driver to provide
        right entries in ini file. Driver gets called with backend info and expects ini configuration
        as part of driver call return response. 
    """
    def get_backend_ini_config(self, context, pool, backend_name, service, backends):
        ini_cfg = {}
        search_opts = {}
        db_upd_info = []
        for backend in backends:
            # get backend information from database
            name = backend['name']
            if (uuidutils.is_uuid_like(name)):
                db_info = storage_backends.get_backend_by_id(context, backend['name'], True)
            else:
                db_info = storage_backends.get_backend_by_name(context, backend['name'], True)
            tiers = []
            # TODO(arc) - assumes db_info returns only one backend and not list. Need to change this to list.
            if backend.get('tiers', None) and db_info.get('tiers', None):
                for tier in backend['tiers']: # only one field is used at this time
                    tier_info = None
                    for backend_tier in db_info['tiers']:
                        if tier['name'] == backend_tier['name'] or tier['name'] == backend_tier['id']:
                            tier_info = backend_tier
                            break
                    if not tier_info:
                        raise exception.StorageTierNotFound(key = str(tier['name']))
                    else:
                        tiers.append(tier_info)
                    
            # use filtered list of tiers
            db_info['tiers'] = tiers

            # call the driver to get the processed ini config
            # TODO(arc) - need to store backend type so driver mapping can be done
            if not db_info.get('driver'):
               raise exception.DriverMappingError(key = str(name)) 

            driver = discover.API().get_driver_instance(db_info.get('driver'))
            opts, cfg = driver.get_ini_config(service, pool, backend_name, db_info)
            ini_cfg.update(cfg)
            search_opts.update(opts)

            # add database entries that need to be created or updated
            db_rec = dict(pool=pool,backend_name=backend_name,services=service,storage_backend_id=db_info['id'])
            if db_info.get('tiers'):
                for tier in db_info.get('tiers'):
                    db_upd_info.append(dict(dict(storage_tier_id=tier['id']), **db_rec))
            else:
                db_upd_info.append(db_rec)

        return search_opts, ini_cfg, db_upd_info
        

    """
        Create a pool for a given service type(s), setup associated configuration file (e.g. for cinder
        cinder.conf file) with backend info. 
        pool : <pool>
        backend_name : name to be used to group storage system + tiers associated with this pool. Normally this
                translates to 'volume_backend_name' in cinder.conf for cinder.
        services: ['volume','file','backup','object']
        backends: [{'name': <name or id>, 'tiers' : [{'name': <name or id>, ..}]}, ..]
        hosts: list of hosts where config need to be updated [<host>,<host>,..]
    """
    def create_pool(self, context, pool, backend_name, backends, services = ['volume'], hosts = None):
        LOG.debug("create_pool info pool: %s, backend_name: %s, backends: %s, services: %s" % 
                  (pool, backend_name, backends, services))
        
        for service in services:
            if service == 'volume':
                self.create_volume_pool(context, pool, backend_name, backends, hosts)


    # see create_pool params
    # pool is volume_type, backend_name is volume_backend_name
    def create_volume_pool(self, context, pool, backend_name, backends, hosts):
        LOG.debug("create_volume_pool got called with pool: %s, backend_name: %s, backends: %s" % 
                  (pool, backend_name, backends))

        # get cinder client session
        client = cinder_client.get_cinder_client()

        # create pool in the specified backend - we will use same name
        # for type and backend
        cinder_client.create_backend(client, pool, backend_name)

        # get list of hosts for cinder-volume service if no hosts is specified
        if not hosts:
            hosts = cinder_client.get_hosts(client)

        # get ini configuration entries
        search_sections, new_ini_sections, db_info = self.get_backend_ini_config(context, pool, backend_name, 
                                                                                 'volume', backends)

        # update config file
        # NOTE: new_ini_sections will include what is being used for the ini file updates
        upd_sections = self.update_conf(hosts, CONF.cinder.conf_file, search_sections, new_ini_sections, 
                                        CONF.cinder.os_user, 'update')

        # insert pool info into database
        storage_pools.create_pool(context, db_info)


    """
        Get pool info for a given service type(s) from wide variety of sources (cinder.conf, scheduler API etc.)
        services: <optional> ['volume','file','backup','object']
        Returns:
            {pool: <>, {backend_name:<>, {<key>:<value>}
    """
    def get_pool_list(self, context, services = ['volume']):
        pool_info = []
        for service in services:
            if service == 'volume':
                info = self.get_volume_pool_info(context)
                if info:
                    for i in info:
                        pool_info.append(i)

        #TODO(arc): pool_info needs to be merged from multiple services at some point
        return pool_info

    def merge_entries(self, cur, new):
        res = set()
        if cur:
            res.update(set(re.split(r'[,\s]\s*', cur.strip())))
        if new:
            res.update(set(re.split(r'[,\s]\s*', new.strip())))
        return ','.join(res)

    def get_volume_pool_info(self, context):
        # [{pool:<>,backend_name:<>,service:<>,storage_backend_id=<>,storage_tier_id=<>}, ...]
        db_list = storage_pools.get_pool(context, None)

        client = cinder_client.get_cinder_client()
        # sample cinder_info: {u'bronze': {'section': u'rbd-1', 'host': u'ihcontroller', 'backend_name': u'rbd-backend'}, 
        #                      u'silver': {'section': u'lvm-1', 'host': u'ihcontroller', 'backend_name': u'iscsi_backend'}}
        cinder_info = cinder_client.get_pool_info(client)

        # merge cinder info into database pool info 
        if cinder_info:
            for p in db_list:
                pool = p['pool']
                if pool in cinder_info and p.get('backend_name') == cinder_info[pool].get('backend_name'):
                    p['section'] = self.merge_entries(p.get('section'), cinder_info[pool].get('section'))
                    p['host'] = self.merge_entries(p.get('host'), cinder_info[pool].get('host'))
                    cinder_info.pop(pool)
        
        # add all items that are not created by controller
        # TODO (arc): This can be confusing, for now - not adding this to the list
        """
        for p in cinder_info:
            db_list.append(dict(dict(pool=p), **cinder_info[p]))
        """

        return db_list

    """
        Delete a pool for a given service type(s), remove associated configuration file entries (e.g. for cinder
        cinder.conf file) with backend info.
        pool : <pool>
        backend_name : name to be used to group storage system + tiers associated with this pool. Normally this
                translates to 'volume_backend_name' in cinder.conf for cinder.
        services: ['volume','file','backup','object']
        backends: [{'name': <name or id>, 'tiers' : [{'name': <name or id>, ..}]}, ..]
        hosts: list of hosts where config need to be updated [<host>,<host>,..]
    """
    def delete_pool(self, context, pool, backend_name, backends = None, services = ['volume'], hosts = None):
        LOG.debug("delete info pool: %s, backend_name: %s, backends: %s, services: %s" %
                  (pool, backend_name, backends, services))

        # check if pool exists in database - this will raise StoragePoolNotFound exception
        # this means only pools created from SDS controller will get deleted
        db_info = storage_pools.get_pool(context, dict(pool=pool, backend_name=backend_name))

        # if backends are not specified, delete all backends
        if not backends:
            backends = []
            for pool_info in db_info:
                if pool_info.get('storage_tier_id'):
                    tiers = [dict(name=pool_info['storage_tier_id'])]
                    backends.append(dict(name=pool_info['storage_backend_id'], tiers=tiers))
                else:
                    backends.append(dict(name=pool_info['storage_backend_id']))

        for service in services:
            if service == 'volume':
                self.delete_volume_pool(context, pool, backend_name, backends, hosts)

    def delete_pool_by_id(self, context, id):
        db_info = storage_pools.get_pool(context, dict(id=id))
        for pool_info in db_info:
            backends = [] 
            if pool_info.get('storage_tier_id'):
                tiers = [dict(name=pool_info['storage_tier_id'])]
                backends.append(dict(name=pool_info['storage_backend_id'], tiers=tiers))
            else:
                backends.append(dict(name=pool_info['storage_backend_id'])) 

            if pool_info.get('services'):
                services = re.split(r'[,\s]\s*', pool_info.get('services').strip())
            else:
                services = ['volume']

            for service in services:
                if service == 'volume':
                    self.delete_volume_pool(context, pool_info['pool'], pool_info['backend_name'], backends)

    # see delete_pool params
    # pool is volume_type, backend_name is volume_backend_name
    def delete_volume_pool(self, context, pool, backend_name, backends, hosts = None):
        LOG.debug("delete_volume_pool got called with pool: %s, backend_name: %s, backends: %s" %
                  (pool, backend_name, backends))

        # get cinder client session
        client = cinder_client.get_cinder_client()
        
        # check if volume type and associated backend exists
        type_info = cinder_client.get_volume_type(client, pool)
        if not type_info or not type_info.extra_specs or not type_info.extra_specs.get('volume_backend_name'):
            name = None if not type_info else type_info.name
            raise Exception(exception.CinderTypeOrBackendNameNotFound(type=name))
            
        # get list of hosts for cinder-volume service if no hosts is specified
        if not hosts:
            hosts = cinder_client.get_hosts(client)

        # get ini configuration entries
        search_sections, del_ini_sections, db_info = self.get_backend_ini_config(context, pool, backend_name,
                                                                                 'volume', backends)
        # update config file
        # NOTE: new_ini_sections will include what is being used for the ini file updates
        upd_sections = self.update_conf(hosts, CONF.cinder.conf_file, search_sections, del_ini_sections, 
                                        CONF.cinder.os_user, 'delete')

        # delete cinder entries if there are no entries for a given backend.
        del_type = False
        try:
            db_res = storage_pools.get_pool(context, dict(backend_name=backend_name))
            if db_res and db_info and len(db_res) == len(db_info):
                del_type = True
        except (exception.StoragePoolNotFound):
            del_type = True

        if del_type:
            cinder_client.delete_volume_type(client, type_info.id) 

        # delete pool info from database
        storage_pools.destroy_pool(context, db_info)
