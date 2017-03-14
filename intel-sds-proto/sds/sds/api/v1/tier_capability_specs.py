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
from sds.discover import storage_tiers
from sds.openstack.common import log as logging

CONF = cfg.CONF
LOG = logging.getLogger(__name__)


class StorageBackendTierCapabilitySpecsTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        root = xmlutil.make_flat_dict('capability_specs', selector='capability_specs')
        return xmlutil.MasterTemplate(root, 1)


class StorageBackendTierCapabilitySpecTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        tagname = xmlutil.Selector('key')

        def extraspec_sel(obj, do_raise=False):
            # Have to extract the key and value for later use...
            key, value = obj.items()[0]
            return dict(key=key, value=value)

        root = xmlutil.TemplateElement(tagname, selector=extraspec_sel)
        root.text = 'value'
        return xmlutil.MasterTemplate(root, 1)


class StorageBackendTierCapabilitySpecsController(wsgi.Controller):
    """The storage backend tier capability specs API controller for the OpenStack API."""

    def _get_capability_specs(self, context, tier_id):
        capability_specs = storage_tiers.get_tier_capability_specs(context, tier_id)
         
        specs_dict = {}
        for key, value in capability_specs.iteritems():
            specs_dict[key] = value
        return dict(capability_specs=specs_dict)

    def _check_backend_tier(self, context, tier_id):
        try:
            storage_tiers.get_tier_by_id(context, tier_id)
        except exception.NotFound as ex:
            raise webob.exc.HTTPNotFound(explanation=ex.msg)

    @wsgi.serializers(xml=StorageBackendTierCapabilitySpecsTemplate)
    def index(self, req, tier_id):
        """Returns the list of extra specs for a given volume type."""
        context = req.environ['sds.context']
        self._check_backend_tier(context, tier_id)
        return self._get_capability_specs(context, tier_id)

    @wsgi.serializers(xml=StorageBackendTierCapabilitySpecsTemplate)
    def create(self, req, tier_id, body=None):
        context = req.environ['sds.context']

        if not self.is_valid_body(body, 'capability_specs'):
            raise webob.exc.HTTPBadRequest()

        self._check_backend_tier(context, tier_id)
        specs = body['capability_specs']
        self._check_key_names(specs.keys())
        storage_tiers.create_tier_capability_specs(context, tier_id, specs)
        notifier_info = dict(tier_id=tier_id, specs=specs)
        notifier = rpc.get_notifier('storageBackendTierExtraSpecs')
        notifier.info(context, 'storage_tier_capability_specs.create',
                      notifier_info)
        return body

    def delete(self, req, tier_id, id):
        """Deletes an existing extra spec."""
        context = req.environ['sds.context']
        self._check_backend_tier(context, tier_id)

        try:
            storage_tiers.destroy_tier_capability_specs(context, tier_id, id)
        except Exception as error:
            LOG.warn("Exception: %s" % error)
            raise webob.exc.HTTPNotFound(explanation=error.message)

        notifier_info = dict(tier_id=tier_id, skey=id)
        notifier = rpc.get_notifier('storageBackendTierExtraSpecs')
        notifier.info(context,
                      'storage_tier_capability_specs.delete',
                      notifier_info)
        return webob.Response(status_int=202)

    def _check_key_names(self, keys):
        if not common.validate_key_names(keys):
            expl = _('Key names can only contain alphanumeric characters, '
                     'underscores, periods, colons and hyphens.')

            raise webob.exc.HTTPBadRequest(explanation=expl)

def create_resource():
    return wsgi.Resource(StorageBackendTierCapabilitySpecsController())
