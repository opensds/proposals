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


import six
import webob

from oslo.config import cfg

from sds.common import wsgi
from sds.common import exception
from sds.common import rpc
from sds.openstack.common import log as logging
from sds import discover
from sds.api import xmlutil

CONF = cfg.CONF
LOG = logging.getLogger(__name__)

class StorageDiscoverTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        root = xmlutil.make_flat_dict('storage_discover', selector='storage_discover')
        return xmlutil.MasterTemplate(root, 1)


class StorageDiscoverController(wsgi.Controller):
    """The storage system discovery API controller for the OpenStack API."""

    def __init__(self, *args, **kwargs):
        super(StorageDiscoverController, self).__init__(*args, **kwargs)
        self.discover_api = discover.API()

    def _notify_storage_discover_error(self, context, method, payload):
        rpc.get_notifier('storageDiscover').error(context, method, payload)

    @wsgi.action("create")
    @wsgi.serializers(xml=StorageDiscoverTemplate)
    def _discover(self, req, body):
        """Discover a new storage backend."""
        context = req.environ['sds.context']

        if not self.is_valid_body(body, 'storage_discover'):
            raise webob.exc.HTTPBadRequest()

        _info = body['storage_discover']
        ip_cidr = _info.get('ip_cidr', None)
        storage_type = _info.get('storage_type', None)
        metadata = _info.get('metadata', None)

        if ip_cidr is None or ip_cidr == "" or storage_type is None or storage_type == "":
            raise webob.exc.HTTPBadRequest()

        try:
            storage_info = self.discover_api.discover(context, ip_cidr, storage_type, metadata)
            notifier_info = dict(storage_info=storage_info)
            rpc.get_notifier('storageDiscover').info(context, 'storage_discover.create',
                                                notifier_info)

        except Exception as err:
            notifier_err = dict(storage_info=_info, error_message=err)
            self._notify_storage_discover_error(context,
                                           'storage_discover.create',
                                           notifier_err)
            raise webob.exc.HTTPNotFound()

        return dict(storage_discover=storage_info)

    def index(self, req):
        LOG.debug("StorageDiscoverController index method got called with %s" % req)
        raise exc.HTTPNotFound()

    def show(self, req, id):
        LOG.debug("StorageDiscoverController show method got called with %s" % req)
        raise exc.HTTPNotFound()

def create_resource():
    return wsgi.Resource(StorageDiscoverController())
