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

    def trim(self, storage_tier):
        trimmed = dict(id=storage_tier.get('id'),
                       name=storage_tier.get('name'),
                       storage_backend_id=storage_tier.get('storage_backend_id'),
                       capability_specs_id=storage_tier.get('capability_specs_id'))
        return trimmed

    def show(self, request, storage_tier, brief=False):
        """Trim away extraneous storage backend attributes."""
        return self.trim(storage_tier) if brief else dict(storage_tier=storage_tier)

    def list(self, request, storage_tier, brief=False):
        """Trim away extraneous storage backend attributes."""
        return self.trim(storage_tier) if brief else storage_tier

    def summary_list(self, request, storage_tiers):
        """Index over trimmed backend tiers."""
        storage_tiers_list = [self.list(request, storage_tier, True)
                             for storage_tier in storage_tiers]
        return dict(storage_tiers=storage_tiers_list)

    def detail_list(self, request, storage_tiers):
        """Index over trimmed backend tiers."""
        storage_tiers_list = [self.list(request, storage_tier, False)
                             for storage_tier in storage_tiers]
        return dict(storage_tiers=storage_tiers_list)
