# Copyright (c) 2014 Intel Corporation
# Copyright (c) 2014 OpenStack Foundation
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
"""Ceph Discovery Driver"""

from __future__ import absolute_import
import io
import json
import os
import tempfile
import urllib
import subprocess
import json
import socket

from netaddr import *

from oslo.config import cfg

from sds.common import exception
from sds.openstack.common import fileutils
from sds.openstack.common import log as logging
from sds.openstack.common import strutils
from sds.discover import driver

try:
    import rados
except ImportError:
    rados = None

LOG = logging.getLogger(__name__)

ceph_opts = [
    cfg.IntOpt('timeout',
               default=2,
               help='rados client connect timeout'),
    cfg.IntOpt('ping_count',
               default=2,
               help='number of ICMP ping requests to check if host is alive'),
    cfg.IntOpt('mon_port',
               default=6789,
               help='Ceph default monitoring port'),
    cfg.StrOpt('cluster_name',
               default='ceph',
               help='Ceph default cluster name'),
    cfg.StrOpt('conf_file',
               default='/etc/ceph/ceph.conf',
               help='Ceph default config file name')
]

CONF = cfg.CONF
CEPH_OPT_GROUP = 'ceph'
CONF.register_opts(ceph_opts, group=CEPH_OPT_GROUP)

cinder_ceph_opts = [
    cfg.StrOpt('volume_driver',
               default='cinder.volume.drivers.rbd.RBDDriver',
               help='Ceph volume driver class name.'),
    cfg.StrOpt('rbd_ceph_conf',
               default='',  # default determined by librados
               help='Path to the ceph configuration file'),
    cfg.BoolOpt('rbd_flatten_volume_from_snapshot',
                default=False,
                help='Flatten volumes created from snapshots to remove '
                     'dependency from volume to snapshot'),
    cfg.StrOpt('rbd_secret_uuid',
               default=None,
               help='The libvirt uuid of the secret for the rbd_user '
                    'volumes'),
    cfg.IntOpt('rbd_max_clone_depth',
               default=5,
               help='Maximum number of nested volume clones that are '
                    'taken before a flatten occurs. Set to 0 to disable '
                    'cloning.'),
    cfg.StrOpt('rbd_user',
               default=None,
               help='The RADOS client name for accessing rbd volumes '
                    '- only set when using cephx authentication'),
]
CEPH_CINDER_OPT_GROUP = 'cinder_ceph'
CONF.register_opts(cinder_ceph_opts, group=CEPH_CINDER_OPT_GROUP)

CEPH_DRIVER = 'ceph'

class CephDriver(driver.DiscoverDriver):
    """Implements automated discovery for Ceph storage system."""

    VERSION = '1.1.0'

    def __init__(self, *args, **kwargs):
        super(CephDriver, self).__init__(*args, **kwargs)

    # service: one of "volume, backup, share, object"
    # pool is volume_type <not used in this>
    # backend_name is used for 'volume_backend_name'
    # info: <same as discover output - see below>
    def get_ini_config(self, service, pool, backend_name, info):
        # pre-req checks

        # will include <section>:{<key>:<value>}
        ini_cfg = {}
        search_opts = {}
        for tier in info.get('tiers', []):
            # for now we will use <fsid_pool> to create unique group since cluster can have name collision
            #section = "%s_%s" % (info['config_specs']['fsid'], tier['name'])
            section = "%s_%s" % (backend_name, tier['name'])
            # get rid of , char since it conflicts with , separator in enabled_backends=<>
            section.replace(',', '')

            cfg = {}

            # add search params - this is used to find out existing section
            cfg['rbd_pool'] = tier['name']
            cfg['volume_backend_name'] = backend_name

            search_opts[section] = dict(cfg)

            # add remaining config params
            cfg['volume_driver'] = CONF.cinder_ceph.volume_driver
            cfg['rbd_flatten_volume_from_snapshot'] = CONF.cinder_ceph.rbd_flatten_volume_from_snapshot
            cfg['rbd_max_clone_depth'] = CONF.cinder_ceph.rbd_max_clone_depth
            if CONF.cinder_ceph.rbd_user:
                cfg['rbd_user'] = CONF.cinder_ceph.rbd_user
            elif info['config_specs']['user']:
                cfg['rbd_user'] = info['config_specs']['user']
            else:
                raise Exception("rbd_user param is empty. Specify 'rbd_user' in sds.conf under %s section" %
                            (CEPH_CINDER_OPT_GROUP))
            if not CONF.cinder_ceph.rbd_secret_uuid:
                raise Exception("rbd_secret_uuid param is null. Configure 'rbd_secret_uuid' in sds.conf under %s section" % 
                                (CEPH_CINDER_OPT_GROUP))
            else:
                cfg['rbd_secret_uuid'] = CONF.cinder_ceph.rbd_secret_uuid
            if CONF.cinder_ceph.rbd_ceph_conf != '':
                cfg['rbd_ceph_conf'] = CONF.cinder_ceph.rbd_ceph_conf

            # add to the section 
            ini_cfg[section] = dict(cfg)

        return search_opts, ini_cfg
             
            
    def _get_pool_capabilities(self, client):
        try:
            cmd = ('{"prefix": "osd dump", "format": "json"}')
            ret, buf, errs = client.mon_command(cmd, '', 0)
            if (ret < 0):
                raise Exception(errs)

            osd_dump_formatted = json.loads(buf)
            pools = osd_dump_formatted['pools']
            ec_profiles = osd_dump_formatted['erasure_code_profiles']

            # list of <dict(pool attributes)>
            pool_capabilities = list()
            for pool in pools:
                name =  str(pool.get('pool_name'))
                pcap_specs = dict()
                if (pool.get('type') == 1):
                    pcap_specs['data_protection'] = 'replication'
                    pcap_specs['replication_min_size'] = pool.get('min_size')
                    pcap_specs['replication_size'] = pool.get('size')
                elif (pool.get('type') == 3):
                    pcap_specs['data_protection'] = 'erasure_code'
                    pcap_specs['erasure_code_stripe_width'] = pool.get('stripe_width')
                    pcap_specs['erasure_code_profile'] = pool.get('erasure_code_profile')
                    pcap_specs[pool.get('erasure_code_profile')] = ec_profiles[pool.get('erasure_code_profile')]
                else:
                    pass
                
                # get pool stats (we need only pool siz - bare minimum)
                ioctx = client.open_ioctx(name)
                pool_stats = ioctx.get_stats()
                pcap_specs['capacity_used_kb'] = pool_stats.get('num_kb', 0)
                ioctx.close()
                
                pool_capabilities.append(dict(name=name, capability_specs = pcap_specs))

            return pool_capabilities
        except:
            raise

    def _get_mon_list(self, client):
        cmd = ('{"prefix": "mon_status", "format": "json"}')
        ret, buf, errs = client.mon_command(cmd, '', 0)
        if (ret < 0):
            raise Exception(errs)
        mon_list = json.loads(buf)
        return mon_list

    def _get_ceph_info(self, timeout, user=None, name=None, clustername=None,
                 conf_defaults=None, conffile=None, conf=None, flags=0):
        try:
            LOG.info("connecting to Ceph instance using %s" % (dict(rados_id=user, name=name, clustername=clustername,
                      conf_defaults=conf_defaults, conffile=conffile, conf=conf, flags=flags)))
            client = rados.Rados(rados_id=user, name=name, clustername=clustername, 
                                 conf_defaults=conf_defaults, conffile=conffile, conf=conf, 
                                 flags=flags)
            client.connect(timeout = timeout)
        except Exception as e:
            LOG.info("exception %s while connecting to Ceph with %s" % (e, dict(rados_id=user, name=name, clustername=clustername,
                      conf_defaults=conf_defaults, conffile=conffile, conf=conf, flags=flags)))
            raise e

        LOG.info("connected to Ceph instance using %s" % (dict(rados_id=user, name=name, clustername=clustername,
                  conf_defaults=conf_defaults, conffile=conffile, conf=conf, flags=flags)))
        try:
            mon_list = self._get_mon_list(client)
            fsid = client.get_fsid()
            version = str(client.version())
            pool_list = self._get_pool_capabilities(client)
            cluster_stats = client.get_cluster_stats()
     
            config_specs = dict()
            if conf_defaults:
                config_specs.update(config_specs)
            if conf:
                config_specs.update(conf)
            config_specs.update(dict(fsid = fsid,
                                     version = version,
                                     user = user,
                                     name = name,
                                     conffile = conffile,
                                     monitors = mon_list['monmap']['mons']))

            capability_specs = dict(capacity_total_kb = cluster_stats['kb'],
                                    capacity_avail_kb = cluster_stats['kb_avail'],
                                    capacity_used_kb = cluster_stats['kb_used'],
                                    data_type = 'unified (object, block, file)',
                                    data_efficiency = 'thin provision',
                                    data_services = 'striping,in-memory caching,copy-on-write cloning,snapshots,incremental backups',
                                    vendor_services = 'caching tier',
                                    performance_IOPS = '')
     
            return dict(name = clustername, 
                        driver = CEPH_DRIVER,
                        config_specs = config_specs,
                        capability_specs = capability_specs, 
                        tiers = pool_list)
        except Exception as e:
            LOG.info("exception %s while connecting to Ceph with %s" % (e, dict(rados_id=user, name=name, clustername=clustername,
                      conf_defaults=conf_defaults, conffile=conffile, conf=conf, flags=flags)))
            raise e

        return list()


    """
        metadata includes 1) username 2) fsid 3) monitor port 4) timeout 5) cluster name
        TODO: A way to pass Cephx authentication key
    """
    def discover(self, ip_cidr = None, metadata = None):
        conf = dict(metadata) if metadata else dict()

        timeout = conf.pop('timeout', CONF.ceph.timeout)
        port = conf.pop('mon_port', CONF.ceph.mon_port)
        if conf.get('user'):
            user = strutils.safe_encode(conf.pop('user'))
        else:
            user = None 
        name = conf.pop('name', None)
        clustername = conf.pop('clustername', CONF.ceph.cluster_name)
        conffile = conf.pop('conffile', None)

        if conf.get('fsid'):
            conf['fsid'] = strutils.safe_encode(conf.pop('fsid'))

        if ip_cidr:
            try:
                ip_addresses = IPNetwork(ip_cidr)

                for ip in ip_addresses:
                    LOG.debug("processing ip = %s" % ip)
                    if (ip_addresses.size > 1 and (ip == ip_addresses.network or ip == ip_addresses.broadcast)):
                        continue
                    str_ip = str(ip)
                    try:
                        #socket.gethostbyaddr(str_ip)
                        #subprocess.check_call(['ping','-c'+str(CONF.ceph.ping_count),str_ip], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                        """
                        Using socket option to not only check if host is reachable but also if monitor exists.
                        Initiating connection on Rados where Ceph monitor does not existing takes 5 minutes to timeout -
                        thread.join() call takes lot longer
                        """
                        sock = socket.socket()
                        sock.settimeout(timeout)
                        sock.connect((str_ip, port))
                    except Exception as e:
                        continue

                    try:
                        if port:
                            conf['mon_host'] = "%s:%s" % (str(ip), port)
                        else:
                            conf['mon_host'] = str(ip)
                        return self._get_ceph_info(timeout=timeout, user=user, name=name, clustername=clustername, 
                                                   conf=conf, conffile=conffile)
                    except Exception as err:
                        continue
            except:
                raise
        else:
            # try with default conf file if it is not specified
            if not conffile:
                conffile = CONF.ceph.conf_file
            return self._get_ceph_info(timeout=timeout, user=user, name=name, clustername=clustername, 
                                       conf=conf, conffile=conffile)
            
        # return empty list
        return list()


def get_driver_class():
    return "CephDriver"
