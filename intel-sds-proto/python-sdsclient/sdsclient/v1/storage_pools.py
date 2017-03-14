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


from sdsclient import base
import six
try:
    from urllib import urlencode
except ImportError:
    from urllib.parse import urlencode


class StoragePool(base.Resource):
    pass

class StoragePoolManager(base.Manager):
    """
    Manage :class:`StoragePool` resources.
    """

    resource_class =  StoragePool

    def _action(self, url, action, info=None, **kwargs):
        body = {action: info}
        self.run_hooks('modify_body_for_action', body, **kwargs)
        return self.api.client.post(url, body=body)

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
        return self._list("/pools%s%s" % (detail, query_string), "storage_pools")

    def delete(self, id):
        """
        Delete storage pool for a given id
        """
        self._delete("/pools/%s" % base.getid(id))

    def create(self, pool, backend_name, services, backends):
        """
        create pool
        """

        body = dict(storage_pool=dict(pool=pool,backend_name=backend_name,backends=backends,services=services))
        return self._create("/pools", body, "storage_pool")
