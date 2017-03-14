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

import logging

from django.conf import settings

from horizon import exceptions
from horizon.utils.memoized import memoized  # noqa
from sdsclient.v1 import client as sds_client
from sdsclient.v1.contrib import list_extensions as sds_list_extensions

from openstack_dashboard.api import base

LOG = logging.getLogger(__name__)


#NOTE(fengqian): SDS only have API v1
VERSIONS = base.APIVersionManager("sds", preferred_version=1)
VERSIONS.load_supported_version(1, {"client": sds_client, "version": 1})


@memoized
def sdsclient(request):
    api_version = VERSIONS.get_active_version()    
    insecure = getattr(settings, 'OPENSTACK_SSL_NO_VERIFY', False)
    cacert = getattr(settings, 'OPENSTACK_SSL_CACERT', None)

    try:    
        sds_url = base.url_for(request, "storage_controller_v1")
    except exceptions.ServiceCatalogException:
        LOG.error("No sds service")
        raise

    LOG.debug('sdsclient create connection use token "%s", url "%s"' % \
              (request.user.token.id, sds_url))

    ret = api_version['client'].Client(request.user.username,
                                       request.user.token.id,
                                       project_id=request.user.tenant_id,
                                       auth_url=sds_url,
                                       insecure=insecure,
                                       cacert=cacert,
                                       http_log_debug=settings.DEBUG)

    ret.client.auth_token = request.user.token.id
    ret.client.management_url = sds_url
    return ret


def discover_storage(request, ip_cidr, storage_type, metadata):
    discover_data = sdsclient(request).storage_discover.discover(ip_cidr,
                                                                 storage_type,
                                                                 metadata)
    return discover_data


#FIXME(fengqian): We have to grather all need data here. It is a little urgly and
#need to be fixed in the future.
def storage_discovered_data(request):
    data = []
    raw_data = storage_backends_list(request, True)
    for _raw_data in raw_data:
        for _tier in _raw_data.tiers:
            storage_specs = _raw_data.capability_specs.copy()
            storage_specs['storagesystem'] = _raw_data.name
            storage_specs['tier'] = _tier.get('name')
            storage_specs['id'] = _tier.get('id')
            storage_specs['total'] = " %s  Kb" % \
                                    storage_specs.pop('capacity_total_kb', 0)

            capability_specs = _tier.get('capability_specs', None)
            if capability_specs is not None:
                storage_specs['used'] = "%s  Kb" % \
                                capability_specs.get('capacity_used_kb', 0)

                if capability_specs.get('data_protection', None) == 'replication':
                    storage_specs['protection'] = "Replication(min_size: %s, " % \
                            capability_specs.get('replication_min_size') + \
                            "size: %s)" % capability_specs.get('replication_size')

                if capability_specs.get('data_protection', None) == \
                                                                'erasure_code':
                    storage_specs['protection'] ="Erasure(data: %s, " % \
                        capability_specs.get('k') + "parity: %s " % \
                        capability_specs.get('m') + "algo: %s)" % \
                        capability_specs.get('technique')

            data.append(storage_specs)

    return [base.APIDictWrapper(_data) for _data in data]


def storage_backends_list(request, detailed=False):
    backends_data = sdsclient(request).storage_backends.list(detailed=detailed)
    return backends_data


def storage_backends_get(request, backend_id):
    backends_data = sdsclient(request).storage_backends.get(backend_id)
    return backends_data


def storage_tiers_list(request, detailed=False):
    tiers_data = sdsclient(request).storage_tiers.list()
    return tiers_data


def storage_tiers_get(request, tier_id):
    tier_data = sdsclient(request).storage_tiers.get(tier_id)
    return tier_data


def storage_pools_list(request):
    return sdsclient(request).storage_pools.list()
 
 
def storage_pools_create(request, pool, backend_name, backends,
                         services=['volume']):
    return sdsclient(request).storage_pools.create(pool=pool,
                                                   backend_name=backend_name,
                                                   backends=backends,
                                                   services=services)


def storage_pools_delete(request, id):
    return sdsclient(request).storage_pools.delete(id=id)


@memoized
def list_extensions(request):
    return sds_list_extensions.ListExtManager(sdsclient(request))\
        .show_all()


@memoized
def extension_supported(request, extension_name):
    """This method will determine if SDS supports a given extension name.
    """
    extensions = list_extensions(request)
    for extension in extensions:
        if extension.name == extension_name:
            return True
    return False
