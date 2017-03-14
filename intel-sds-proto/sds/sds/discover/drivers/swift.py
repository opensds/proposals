#    Copyright 2013 OpenStack Foundation
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
"""Swift Discovery Driver"""

from __future__ import absolute_import
import io
import json
import os
import tempfile
import urllib
import subprocess
import json
import socket
import swiftclient
import requests
import re

from netaddr import *

from oslo.config import cfg

from sds.common import exception
from sds.openstack.common import fileutils
from sds.openstack.common import log as logging
from sds.openstack.common import strutils
from sds.discover import driver

from swiftclient import client as swift_client

LOG = logging.getLogger(__name__)

swift_opts = [
    cfg.IntOpt('timeout',
               default=2,
               help='swift client connect timeout'),
    cfg.IntOpt('ping_count',
               default=2,
               help='number of ICMP ping requests to check if host is alive'),
    cfg.IntOpt('port',
               default=8888,
               help='Swift default port'),
    cfg.StrOpt('ca_certificates_file',
               help='Location of ca certificates file to use for swift '
                    'client requests.'),
    cfg.BoolOpt('api_insecure',
                default=False,
                help='Allow to perform insecure SSL (https) requests to '
                     'swift'),
    cfg.BoolOpt('api_ssl_compression',
                default=False,
                help='Enables or disables negotiation of SSL layer '
                     'compression. In some cases disabling compression '
                     'can improve data throughput.'),
    cfg.StrOpt('user',
               help='swift user'),
    cfg.StrOpt('key',
                secret = True,
                help='swift user password'),
    cfg.StrOpt('region_name',
               help='Region name of this node'),
    cfg.StrOpt('tenant_name',
               help='Swift tenant name'),
    cfg.StrOpt('auth_version',
               default='2.0',
               help='authentication version to use for Swift'),
    cfg.StrOpt('auth_uri',
               help='URL for v1.0 authentication - normally meant for swift tempauth'),
]

CONF = cfg.CONF
SWIFT_OPT_GROUP = 'swift'
CONF.register_opts(swift_opts, group=SWIFT_OPT_GROUP)
SWIFT_DRIVER = 'swift'

class SwiftDriver(driver.DiscoverDriver):
    """Implements automated discovery for Swift storage system."""

    VERSION = '1.1.0'

    def __init__(self, *args, **kwargs):
        LOG.debug("SWIFT Driver __init__") 
        super(SwiftDriver, self).__init__(*args, **kwargs)


    def _get_swift_info(self, conf):
        # Get Auth Key for Swift using 'user' for user and 'passwd' for password for now
        client = swift_client.Connection(**conf)
        """
        capabilities format:
            {'keystoneauth': {}, 
             'swift': {
                    'max_file_size': 5368709122, 
                    'account_listing_limit': 10000, 
                    'account_autocreate': True, 
                    'max_meta_count': 90, 
                    'max_meta_value_length': 256, 
                    'container_listing_limit': 10000, 
                    'max_container_name_length': 256, 
                    'max_meta_overall_size': 4096, 
                    'version': '2.2.1.14.g682f660', 
                    'max_meta_name_length': 128, 
                    'max_header_size': 8192, 
                    'policies': [{'default': True, 'name': 'gold'}, {'name': 'silver'}], 
                    'max_object_name_length': 1024, 
                    'max_account_name_length': 256, 
                    'strict_cors_mode': True, 
                    'allow_account_management': True
               }
              }
        """
        if conf.get('preauthurl'):
            capabilities = client.get_capabilities(conf.get('preauthurl'))
        else:
            capabilities = client.get_capabilities()
            

        # capabilities return very limited information - onlt version, policies is used
        # any keys that are not swift will be tagged as vendor services
        backend_capability_specs = dict(capacity_total_kb = '',
                                        capacity_avail_kb = '',
                                        capacity_used_kb = '',
                                        data_type = 'object',
                                        data_efficiency = '',
                                        data_services = 'backup',
                                        vendor_services = '',
                                        performance_IOPS = '')

        for key in capabilities:
            if key != 'swift':
                if backend_capability_specs['vendor_services'] != '':
                    backend_capability_specs['vendor_services'] = "%s,%s" % (backend_capability_specs['vendor_services'], key)
                else:
                    backend_capability_specs['vendor_services'] = key

        config_specs = dict(conf)
        config_specs.pop('password', None)
        version = capabilities['swift']['version']
        config_specs.update(dict(version=version))

        policy_list = list()
        if capabilities.get('swift') and capabilities.get('swift').get('policies'):
            for policy in capabilities['swift']['policies']:
                tier_capability_specs = dict()
                name = None
                for key in policy:
                    if key != 'name':
                       tier_capability_specs[key] = policy[key]
                    else:
                        name = policy['name']
                if name:
                    policy_list.append(dict(name=name, capability_specs = tier_capability_specs))

        """
        head account format: 
        {'x-account-storage-policy-gold-bytes-used': '16', 
         'x-account-storage-policy-gold-container-count': '1', 
         'x-account-storage-policy-gold-object-count': '1', 
         'x-account-bytes-used': '16', 
         'x-account-container-count': '1', 
         'x-account-object-count': '1', 
         'x-timestamp': '1419893784.67007', 
         'x-trans-id': 'tx7c1e5d0e232f4a82894c6-0054abadce', 
         'date': 'Tue, 06 Jan 2015 09:41:35 GMT', 
         'content-length': '0', 
         'content-type': 'text/plain; charset=utf-8', 
         'accept-ranges': 'bytes'}
        """
        """
        info = client.head_account()

        # Get Number of Containers on Account
        for key, value in info.iteritems():
            # Find storage policies and their usage in Backend
            matchObj = re.match(r'^x-account-storage-policy-.*-bytes-used', key)
            if matchObj:
                policyKBytes = str(int(value)/1024)
                name = re.sub(r'^x-account-storage-policy-', "", matchObj.group())
                name = re.sub(r'-bytes-used$', "", name)
                policy_list.append(dict(name=name,capability_specs=dict(data_protection='', capacity_used_kb=policyKBytes)))

        accountContainerCount = info['x-account-container-count']
        accountBytesInUse = info['x-account-bytes-used']
        backend_capability_specs['capacity_used_kb'] = str(int(accountBytesInUse)/1024)
        accountObjectCount = info['x-account-object-count']
        """

        return dict(name = 'swift',
                        driver = SWIFT_DRIVER,
                        config_specs = config_specs,
                        capability_specs = backend_capability_specs,
                        tiers = policy_list)

        
    def get_auth_url(self, ip, port, conf):
        authurl = conf['authurl']
            
        if conf.get('cacert'):
            secure = 's'
        else:
            secure = ''

        if port:
            str_port = ":%s" % port
        else:
            str_port = ""

        tokens = {'%secure%' : secure, '%host%' : ip, '%port%' : str_port}
        for token in tokens:
            if tokens[token] != None:
                authurl = re.sub(token, str(tokens[token]), authurl)

        return authurl

    """
        metadata includes 1) username 2) passwd 3) [optional] port
        metadata can include (take a look at swiftclient documentation)
                 authurl=None, user=None, key=None, retries=5,
                 preauthurl=None, preauthtoken=None, snet=False,
                 starting_backoff=1, max_backoff=64, tenant_name=None,
                 os_options=None, auth_version="1", cacert=None,
                 insecure=False, ssl_compression=True,
                 retry_on_ratelimit=False
    """
    def discover(self, ip_cidr = None, metadata = None):
        conf = dict(metadata) if metadata else dict()

        # get rid of non swift params
        timeout = conf.pop('timeout', CONF.swift.timeout)
        port = conf.pop('port', CONF.swift.port)
    
        # get defaults if they are no specified
        conf['cacert'] = conf.pop('cacert', CONF.swift.ca_certificates_file)
        conf['insecure'] = conf.pop('insecure', CONF.swift.api_insecure)
        conf['ssl_compression'] = conf.pop('ssl_compression', CONF.swift.api_ssl_compression)
        
        # setup defaults
        if not conf.get('preauthtoken'):
            conf['user'] = conf.pop('user', CONF.swift.user)
            conf['key'] = conf.pop('key', CONF.swift.key)
            conf['tenant_name'] = conf.pop('tenant_name', CONF.swift.tenant_name)
            conf['authurl'] = conf.pop('authurl', CONF.swift.auth_uri)
            
            #if you don't have configuration in swift section, use keystone urls
            if not conf['user']:
                conf['user'] = CONF.keystone_authtoken.admin_user
            if not conf['key']:
                conf['key'] = CONF.keystone_authtoken.admin_password
            if not conf['tenant_name']:
                conf['tenant_name'] = CONF.keystone_authtoken.admin_tenant_name
            if not conf['authurl']:
                conf['authurl'] = CONF.keystone_authtoken.auth_uri
        
        print("^^ conf: %s" % (conf))

        # if input contains keystone or tempauth then follow the direct discovery model
        if conf.get('preauthurl') or conf.get('preauthtoken') or conf.get('authurl'):
            # change auth_version if not provided to one given in CONF
            conf['auth_version'] = conf.get('auth_version', CONF.swift.auth_version)
            try:
                return self._get_swift_info(conf)
            except Exception as e:
                LOG.info("exception %s while connecting to Swift with %s" % (e, conf))
                # try other options before giving up

        # this makes sense only for tempauth. For keystone, it needs to use keystone authentication
        # to get swift service end point which is taken care by above method
        if ip_cidr and conf.get('authurl'):
            try:
                ip_addresses = IPNetwork(ip_cidr)

                for ip in ip_addresses:
                    LOG.info("connect to ip: %s, port: %s" % (ip, port))
                    if (ip_addresses.size > 1 and (ip == ip_addresses.network or ip == ip_addresses.broadcast)):
                        continue
                    str_ip = str(ip)
                    try:
                        #Using socket option to not only check if host is reachable but also if swift end point exists.
                        sock = socket.socket()
                        sock.settimeout(timeout)
                        sock.connect((str_ip, port))
                    except Exception as e:
                        continue

                    try:
                        auth_conf = dict(conf)
                        auth_conf.pop('preauthurl', None)
                        auth_conf.pop('preauthtoken', None)
                        auth_conf['authurl'] = self.get_auth_url(str_ip, port, auth_conf)
                        return self._get_swift_info(auth_conf)
                    except Exception as e:
                        print("exception %s while connecting to Swift with %s" % (e, auth_conf))
                        LOG.info("exception %s while connecting to Swift with %s" % (e, auth_conf))

            except Exception as e:
                LOG.info("exception %s while connecting to Swift with %s" % (e, conf))
                raise
        
        # return empty list
        return list()

def get_driver_class():
    return "SwiftDriver"
