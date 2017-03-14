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

"""The storage backend & storage backends extra specs extension."""

import six
import webob
from webob import exc
from oslo.config import cfg
from sds.common import wsgi
from sds.api import xmlutil
from sds.common import exception
from sds.common import rpc
from sds.openstack.common import log as logging
from sds.discover import storage_backends
from sds.discover import storage_tiers
from sds.api.v1.views import tiers as views_tiers

CONF = cfg.CONF
LOG = logging.getLogger(__name__)

def make_backendtype(elem):
    elem.set('id')
    elem.set('name')
    #capability_specs = xmlutil.make_flat_dict('capability_specs', selector='capability_specs')
    #elem.append(capability_specs)


class StorageBackendTierTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        root = xmlutil.TemplateElement('storage_tier', selector='storage_tier')
        make_backendtype(root)
        return xmlutil.MasterTemplate(root, 1)


class StorageBackendTiersTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        root = xmlutil.TemplateElement('storage_tiers')
        elem = xmlutil.SubTemplateElement(root, 'storage_tier',
                                          selector='storage_tiers')
        make_backendtype(elem)
        return xmlutil.MasterTemplate(root, 1)


class StorageBackendTiersController(wsgi.Controller):
    """The storage backends API controller for the OpenStack API."""

    _view_builder_class = views_tiers.ViewBuilder

    def _notify_storage_tier_error(self, context, method, payload):
        rpc.get_notifier('storageTier').error(context, method, payload)

    def _get_tiers(self, req, is_detail):
        """Returns the list of storage backends."""
        context = req.environ['sds.context']
        params = req.params.copy()

        search_opts = dict()
        if params:
            if params.get('id'):
                search_opts['id'] = params.get('id')
            if params.get('name'):
                search_opts['name'] = params.get('name')
            if params.get('storage_backend_id'):
                search_opts['storage_backend_id'] = params.get('storage_backend_id')
            if params.get('capability_specs_id'):
                search_opts['capability_specs_id'] = params.get('capability_specs_id')

        try:
            return storage_tiers.get_all_tiers(context, search_opts=search_opts, is_detail=is_detail)
        except exception.StorageTierNotFound:
            if len(search_opts) > 0: # raise exception only when specific tier info is requested
                raise

        return list()


    def _check_key_names(self, keys):
        if not common.validate_key_names(keys):
            expl = _('Key names can only contain alphanumeric characters, '
                     'underscores, periods, colons and hyphens.')

            raise webob.exc.HTTPBadRequest(explanation=expl)

    @wsgi.serializers(xml=StorageBackendTiersTemplate)
    def index(self, req):
        """Returns the list of storage backends."""
        _storage_tiers = self._get_tiers(req, is_detail = False)
        return self._view_builder.summary_list(req, _storage_tiers)

    @wsgi.serializers(xml=StorageBackendTiersTemplate)
    def detail(self, req):
        """Returns the list of storage backends."""
        _storage_tiers = self._get_tiers(req, is_detail = True)
        return self._view_builder.detail_list(req, _storage_tiers)

    @wsgi.serializers(xml=StorageBackendTierTemplate)
    def show(self, req, id):
        """Return a single storage backend item."""
        context = req.environ['sds.context']

        try:
            storage_tier = storage_tiers.get_tier_by_id(context, id, is_detail = True)
        except exception.NotFound:
            LOG.warn("Exception %s" % exception)
            raise exc.HTTPNotFound()

        return self._view_builder.show(req, storage_tier)

    @wsgi.action("create")
    @wsgi.serializers(xml=StorageBackendTiersTemplate)
    def create(self, req, body=None):
        context = req.environ['sds.context']

        if not self.is_valid_body(body, 'storage_tier'):
            raise webob.exc.HTTPBadRequest()

        tier_ref = body['storage_tier']
        tier_name = tier_ref.get('tier_name', None)
        backend_name = tier_ref.get('backend_name', None)
 
        if tier_name is None or tier_name == "":
            raise webob.exc.HTTPBadRequest()

        if backend_name is None or backend_name == "":
            raise webob.exc.HTTPBadRequest()

        try:
            result = storage_tiers.create_tier(context, tier_name, backend_name, tier_ref.get('capability_specs'))
            _backend_tier_info = storage_tiers.get_tier_by_id(context, result['id'])

            notifier_info = dict(tier_info=_backend_tier_info)
            notifier = rpc.get_notifier('storageBackendTiers')
            notifier.info(context, 'storage_backend_tiers.create',
                          notifier_info)

        except exception.StorageTierExists as err:
            notifier_err = dict(tier_info=tier_ref, error_message=err)
            self._notify_storage_tier_error(context,
                                           'storage_tier.create',
                                           notifier_err)

            raise webob.exc.HTTPConflict(explanation=six.text_type(err))
        except Exception as err:
            LOG.warn("Exception: %s" % err)
            notifier_err = dict(tier_info=tier_ref, error_message=err)
            self._notify_storage_tier_error(context,
                                           'storage_tier.create',
                                           notifier_err)
            raise webob.exc.HTTPNotFound()

        return self._view_builder.show(req, _backend_tier_info)

    @wsgi.action("delete")
    def delete(self, req, id):
        """Deletes an existing extra spec."""
        context = req.environ['sds.context']

        try:
            storage_tiers.destroy_tier_by_id(context, id)
        except Exception as error:
            LOG.warn("Exception: %s" % error)
            raise webob.exc.HTTPNotFound(explanation=error.message)

        notifier_info = dict(id=id)
        notifier = rpc.get_notifier('storageBackendTiers')
        notifier.info(context,
                      'storage_backend_tiers.delete',
                      notifier_info)
        return webob.Response(status_int=202)


def create_resource():
    return wsgi.Resource(StorageBackendTiersController())
