# Copyright (c) 2014 Intel Corporation
# Copyright (c) 2014 OpenStack Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.


"""
Storage Tier interface.
"""

import six
try:
    from urllib import urlencode
except ImportError:
    from urllib.parse import urlencode

from sdsclient import base


class StorageTier(base.Resource):
    def __repr__(self):
        return "<StorageTier: %s>" % self.name

    def get_capability_keys(self):
        _resp, body = self.manager.api.client.get(
            "/tiers/%s/capability_specs" %
            base.getid(self))
        return body["capability_specs"]

    def set_capability_keys(self, metadata):
        body = {'capability_specs': metadata}
        return self.manager._create(
            "/tiers/%s/capability_specs" % base.getid(self),
            body,
            "capability_specs",
            return_raw=True)

    def delete_capability_keys(self, keys):
        resp = None
        for k in keys:
            resp = self.manager._delete(
                "/tiers/%s/capability_specs/%s" % (
                base.getid(self), k))
            if resp is not None:
                return resp

class StorageTierManager(base.ManagerWithFind):
    """
    Manage :class:`StorageTier` resources.
    """
    resource_class = StorageTier

    def list(self, detailed=False, search_opts=None):
        """
        Get a list of all storage tiers.

        :rtype: list of :class:`StorageTier`.
        """
        if search_opts is None:
            search_opts = {}

        qparams = {}

        for opt, val in six.iteritems(search_opts):
            if val:
                qparams[opt] = val

        # Transform the dict to a sequence of two-element tuples in fixed
        # order, then the encoded string will be consistent in Python 2&3.
        if qparams:
            new_qparams = sorted(qparams.items(), key=lambda x: x[0])
            query_string = "?%s" % urlencode(new_qparams)
        else:
            query_string = ""

        detail = ""
        if detailed:
            detail = "/detail"
        return self._list("/tiers%s%s" % (detail, query_string), "storage_tiers")

    def get(self, storage_tier):
        """
        Get a specific storage tier.

        :param storage_tier: The ID of the :class:`StorageTier` to get.
        :rtype: :class:`StorageTier`
        """
        return self._get("/tiers/%s" % base.getid(storage_tier), "storage_tier")

    def delete(self, storage_tier):
        """
        Delete a specific storage tier.

        :param storage_tier: The ID of the :class:`StorageTier` to get.
        """
        self._delete("/tiers/%s" % base.getid(storage_tier))

    def create(self, backend_name, tier_name, specs):
        """
        Create a storage tier.

        :param name: Descriptive name of the storage tier
        :rtype: :class:`StorageTier`
        """

        body = {
            "storage_tier": {
                "backend_name": backend_name,
                "tier_name": tier_name,
                "capability_specs": specs,
            }
        }
        return self._create("/tiers", body, "storage_tier")
