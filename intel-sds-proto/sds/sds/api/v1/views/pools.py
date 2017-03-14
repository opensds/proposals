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

from sds.api import common
from sds.openstack.common import log as logging
LOG = logging.getLogger(__name__)


class ViewBuilder(common.ViewBuilder):

    def trim(self, pool_info):
        trimmed = dict()
        for key in ['id', 'pool', 'backend_name', 'services', 'storage_backend_id', 'storage_system_name', 'storage_tier_id', 'storage_tier_name', 'section', 'host']:
           trimmed[key] = pool_info.get(key) 
        return trimmed

    def summary_list(self, request, pools_list):
        _list = [self.trim(pool_info) for pool_info in pools_list]
        return dict(storage_pools=_list)
