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

import webob
import six

from oslo.config import cfg
from sds.api import common
from sds.common import wsgi
from sds.api import xmlutil
from sds.common import exception
from sds.common import rpc
from sds.discover import storage_backends
from sds.openstack.common import log as logging

CONF = cfg.CONF
LOG = logging.getLogger(__name__)


class StorageBackendCapabilitySpecsTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        root = xmlutil.make_flat_dict('capability_specs', selector='capability_specs')
        return xmlutil.MasterTemplate(root, 1)


class StorageBackendCapabilitySpecTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        tagname = xmlutil.Selector('key')

        def extraspec_sel(obj, do_raise=False):
            # Have to extract the key and value for later use...
            key, value = obj.items()[0]
            return dict(key=key, value=value)

        root = xmlutil.TemplateElement(tagname, selector=extraspec_sel)
        root.text = 'value'
        return xmlutil.MasterTemplate(root, 1)


class StorageBackendCapabilitySpecsController(wsgi.Controller):
    """The storage backend capability specs API controller for the OpenStack API."""

    def _get_capability_specs(self, context, backend_id):
        capability_specs = storage_backends.get_backend_capability_specs(context, backend_id)
        specs_dict = {}
        for key, value in capability_specs.iteritems():
            specs_dict[key] = value
        return dict(capability_specs=specs_dict)

    def _check_backend(self, context, backend_id):
        try:
            storage_backends.get_backend_by_id(context, backend_id)
        except exception.NotFound as ex:
            raise webob.exc.HTTPNotFound(explanation=ex.msg)

    @wsgi.serializers(xml=StorageBackendCapabilitySpecsTemplate)
    def index(self, req, backend_id):
        """Returns the list of extra specs for a given backend id."""
        context = req.environ['sds.context']
        self._check_backend(context, backend_id)
        return self._get_capability_specs(context, backend_id)

    @wsgi.serializers(xml=StorageBackendCapabilitySpecsTemplate)
    def create(self, req, backend_id, body=None):
        context = req.environ['sds.context']

        if not self.is_valid_body(body, 'capability_specs'):
            raise webob.exc.HTTPBadRequest()

        self._check_backend(context, backend_id)
        specs = body['capability_specs']
        self._check_key_names(specs.keys())
        storage_backends.create_backend_capability_specs(context, backend_id, specs)
        notifier_info = dict(backend_id=backend_id, specs=specs)
        notifier = rpc.get_notifier('storageBackendExtraSpecs')
        notifier.info(context, 'storage_backend_capability_specs.create',
                      notifier_info)
        return body

    def delete(self, req, backend_id, id):
        """Deletes an existing extra spec."""
        context = req.environ['sds.context']
        self._check_backend(context, backend_id)

        try:
            storage_backends.destroy_backend_capability_specs(context, backend_id, id)
        except Exception as error:
            LOG.warn("Exception: %s" % error)
            raise webob.exc.HTTPNotFound(explanation=error.message)

        notifier_info = dict(backend_id=backend_id, skey=id)
        notifier = rpc.get_notifier('storageBackendExtraSpecs')
        notifier.info(context,
                      'storage_backend_capability_specs.delete',
                      notifier_info)
        return webob.Response(status_int=202)

    def _check_key_names(self, keys):
        if not common.validate_key_names(keys):
            expl = _('Key names can only contain alphanumeric characters, '
                     'underscores, periods, colons and hyphens.')

            raise webob.exc.HTTPBadRequest(explanation=expl)

def create_resource():
    return wsgi.Resource(StorageBackendCapabilitySpecsController())

