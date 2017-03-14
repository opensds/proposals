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
Storage Backend interface.
"""

import six
try:
    from urllib import urlencode
except ImportError:
    from urllib.parse import urlencode

from sdsclient import base


class StorageBackend(base.Resource):
    def __repr__(self):
        return "<StorageBackend: %s>" % self.name

    def create_storage_tiers(self, metadata):
        body = {'storage_tier': metadata}
        return self.manager._create(
            "/backends/%s/tiers" % base.getid(self),
            body,
            "storage_tier",
            return_raw=True)

    def get_capability_keys(self):
        _resp, body = self.manager.api.client.get(
            "/backends/%s/capability_specs" %
            base.getid(self))
        return body["capability_specs"]

    def get_config_keys(self):
        _resp, body = self.manager.api.client.get(
            "/backends/%s/config_specs" %
            base.getid(self))
        return body["config_specs"]

    def set_capability_keys(self, metadata):
        body = {'capability_specs': metadata}
        return self.manager._create(
            "/backends/%s/capability_specs" % base.getid(self),
            body,
            "capability_specs",
            return_raw=True)

    def set_config_keys(self, metadata):
        body = {'config_specs': metadata}
        return self.manager._create(
            "/backends/%s/config_specs" % base.getid(self),
            body,
            "config_specs",
            return_raw=True)

    def delete_config_keys(self, keys):
        resp = None
        for k in keys:
            resp = self.manager._delete(
                "/backends/%s/config_specs/%s" % (
                base.getid(self), k))
            if resp is not None:
                return resp

    def delete_capability_keys(self, keys):
        resp = None
        for k in keys:
            resp = self.manager._delete(
                "/backends/%s/capability_specs/%s" % (
                base.getid(self), k))
            if resp is not None:
                return resp

class StorageBackendManager(base.ManagerWithFind):
    """
    Manage :class:`StorageBackend` resources.
    """
    resource_class = StorageBackend

    def list(self, detailed=False, search_opts=None):
        """
        Get a list of all storage backends.

        :rtype: list of :class:`StorageBackend`.
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
        return self._list("/backends%s%s" % (detail, query_string), "storage_backends")

    def get(self, storage_backend):
        """
        Get a specific storage backend.

        :param storage_backend: The ID of the :class:`StorageBackend` to get.
        :rtype: :class:`StorageBackend`
        """
        return self._get("/backends/%s" % base.getid(storage_backend), "storage_backend")

    def delete(self, storage_backend):
        """
        Delete a specific storage_backend.

        :param storage_backend: The ID of the :class:`StorageBackend` to get.
        """
        self._delete("/backends/%s" % base.getid(storage_backend))

    def create(self, name, specs):
        """
        Create a storage backend.

        :param name: Descriptive name of the storage backend
        :rtype: :class:`StorageBackend`
        """

        body = {
            "storage_backend": {
                "name": name,
                "capability_specs": specs,
            }
        }
        return self._create("/backends", body, "storage_backend")
