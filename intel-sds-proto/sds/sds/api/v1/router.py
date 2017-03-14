# Copyright 2011 OpenStack Foundation
# Copyright 2011 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
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
WSGI middleware for OpenStack SDS API.
"""

#from sds.api import versions
from sds.common import wsgi
from sds.api.v1 import backends
from sds.api.v1 import tiers
from sds.api.v1 import discover
from sds.api.v1 import services
from sds.api.v1 import backend_capability_specs
from sds.api.v1 import backend_config_specs
from sds.api.v1 import tier_capability_specs
from sds.api.v1 import pools
from sds.openstack.common import log as logging

LOG = logging.getLogger(__name__)

class APIRouter(wsgi.Router):
    """Routes requests on the API to the appropriate controller and method."""

    def __init__(self, mapper):
        #mapper.connect("versions", "/",
        #               controller=versions.create_resource(),
        #               action='show')
        #mapper.redirect("", "/")

        mapper.resource("backend", "backends",
                        controller=backends.create_resource(),
                        collection={'detail': 'GET'})

        mapper.resource("config_specs", "config_specs", 
                        controller=backend_config_specs.create_resource(),
                        parent_resource=dict(member_name='backend', collection_name='backends'))

        mapper.resource("capability_specs", "capability_specs", 
                        controller=backend_capability_specs.create_resource(),
                        parent_resource=dict(member_name='backend', collection_name='backends'))

        mapper.resource("tier", "tiers",
                        controller=tiers.create_resource(),
                        collection={'detail': 'GET'})

        mapper.resource("capability_specs", "capability_specs", 
                        controller=tier_capability_specs.create_resource(),
                        parent_resource=dict(member_name='tier', collection_name='tiers'))

        mapper.resource("discover", "discover",
                        controller=discover.create_resource())

        mapper.resource("pool", "pools",
                        controller=pools.create_resource())

        mapper.resource("os-services", "os-services",
                        controller=services.create_resource())

        super(APIRouter, self).__init__(mapper)
