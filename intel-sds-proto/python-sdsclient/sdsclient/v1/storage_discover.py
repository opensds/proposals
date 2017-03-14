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
Volume Type interface.
"""

from sdsclient import base

class StorageDiscover(base.Resource):
    pass

class StorageDiscoverManager(base.Manager):
    """
    Manage :class:`StorageDiscover` resources.
    """

    resource_class =  StorageDiscover

    def _action(self, url, action, info=None, **kwargs):
        body = {action: info}
        self.run_hooks('modify_body_for_action', body, **kwargs)
        return self.api.client.post(url, body=body)

    def discover(self, ip_cidr, storage_type, metadata):
        """
        Discover a storage backend.
        """

        body = {
                "ip_cidr": ip_cidr,
                "storage_type": storage_type,
                "metadata": metadata,
        }
        (res, body) = self._action("/discover", "storage_discover", body)
        return body['storage_discover']
