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

from __future__ import absolute_import

from django.conf import settings

import logging
LOG = logging.getLogger(__name__)


#NOTE(fengqian): If vsmclient is not installed, use a fake one.
try:
    from vsmclient.v1 import client as vsm_client
    from vsmclient.v1.pool_usages import PoolUsageManager
    from vsmclient.v1.appnodes import AppNodeManager


    class ExtensionManager:
        def __init__(self, name, manager_class):
            self.name = name
            self.manager_class = manager_class
    
    
    def vsmclient(request):
        key_vsm_pass = getattr(settings,'KEYSTONE_VSM_SERVICE_PASSWORD')
        key_url = getattr(settings, 'OPENSTACK_KEYSTONE_URL')
        c = vsm_client.Client('vsm',
                              key_vsm_pass,
                              'service',
                              key_url,
                              extensions=[ExtensionManager('PoolUsageManager',
                                                    PoolUsageManager),
                                          ExtensionManager('AppNodeManager',
                                                    AppNodeManager)])
        return c

except ImportError:
    def vsmclient(request):

        class Server(object):
            @property
            def servers(self):   

                class _server(object):
                    def __init__(self):
                        setattr(self, 'list', lambda: [])
                        setattr(self, 'add', lambda x: x)
                        setattr(self, 'remove', lambda x: x)
                        setattr(self, 'get', lambda x: x)
                        setattr(self, 'start', lambda x: x)
                        setattr(self, 'stop', lambda x: x)

                return _server()
        
        return Server()


def add_servers(request, servers=[]):
    return vsmclient(request).servers.add(servers)


def remove_servers(request, servers=[]):
    return vsmclient(request).servers.remove(servers)


def get_server_list(request):
    data = vsmclient(request).servers.list()
    for _data in data:
        _data.storagesystem = 'Ceph'
    return data


def get_server(request, id):
    data = vsmclient(request).servers.get(id)
    if data:
        data.storagesystem = 'Ceph'
    return data


def start_server(request, servers=None):
    """Start servers.
       servers = [{'id': 1}, {'id': 2}]
    """
    return vsmclient(request).servers.start(servers)


def stop_server(request, servers=None):
    """Stop servers.
       servers = [{'id': 1}, {'id': 2}]
    """
    return vsmclient(request).servers.stop(servers)
