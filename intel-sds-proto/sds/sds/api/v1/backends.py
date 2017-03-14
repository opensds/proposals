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

import webob
import six
from webob import exc

from sds.common import exception
from sds.common import rpc
from sds.api import xmlutil
from sds.common import wsgi
from sds.discover import storage_backends
from sds.openstack.common import log as logging
from sds.api.v1.views import backends as views_backends

LOG = logging.getLogger(__name__)

def make_backendtype(elem):
    elem.set('id')
    elem.set('name')
    #extra_specs = xmlutil.make_flat_dict('extra_specs', selector='extra_specs')
    #elem.append(extra_specs)


class StorageBackendTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        root = xmlutil.TemplateElement('storage_backend', selector='storage_backend')
        make_backendtype(root)
        return xmlutil.MasterTemplate(root, 1)


class StorageBackendsTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        root = xmlutil.TemplateElement('storage_backends')
        elem = xmlutil.SubTemplateElement(root, 'storage_backend',
                                          selector='storage_backends')
        make_backendtype(elem)
        return xmlutil.MasterTemplate(root, 1)


class StorageBackendsController(wsgi.Controller):
    """The storage backends API controller for the OpenStack API."""

    _view_builder_class = views_backends.ViewBuilder

    def _notify_storage_backend_error(self, context, method, payload):
        rpc.get_notifier('storageBackend').error(context, method, payload)

    def _get_backends(self, req, is_detail):
        """Returns the list of storage backends."""
        context = req.environ['sds.context']
        params = req.params.copy()

        search_opts = dict()
        if params:
            if params.get('id'):
                search_opts['id'] = params.get('id')
            if params.get('name'):
                search_opts['name'] = params.get('name')
            if params.get('config_specs_id'):
                search_opts['config_specs_id'] = params.get('config_specs_id')
            if params.get('capability_specs_id'):
                search_opts['capability_specs_id'] = params.get('capability_specs_id')

        try:
            return storage_backends.get_all_backends(context, search_opts=search_opts, is_detail=is_detail)
        except exception.StorageBackendNotFound:
            if len(search_opts) > 0: # raise exception only when specific backend info is requested
                raise

        return list()

    @wsgi.serializers(xml=StorageBackendsTemplate)
    def index(self, req):
        """Returns the list of storage backends."""
        _storage_backends = self._get_backends(req, is_detail = False)
        return self._view_builder.summary_list(req, _storage_backends)

    @wsgi.serializers(xml=StorageBackendsTemplate)
    def detail(self, req):
        """Returns the list of storage backends."""
        _storage_backends = self._get_backends(req, is_detail = True)
        return self._view_builder.detail_list(req, _storage_backends)

    @wsgi.serializers(xml=StorageBackendTemplate)
    def show(self, req, id):
        """Return a single storage backend item."""
        context = req.environ['sds.context']
        #params = req.params.copy()
        #detail = params.pop('detail', None)

        try:
            storage_backend = storage_backends.get_backend_by_id(context, id, is_detail = True)
        except exception.NotFound as err:
            LOG.warn("Exception %s" % unicode(err))
            raise exc.HTTPNotFound()

        storage_backend['id'] = str(storage_backend['id'])
        return self._view_builder.show(req, storage_backend)

    @wsgi.action("create")
    @wsgi.serializers(xml=StorageBackendTemplate)
    def _create(self, req, body):
        """Creates a new backend."""
        context = req.environ['sds.context']

        if not self.is_valid_body(body, 'storage_backend'):
            raise webob.exc.HTTPBadRequest()

        storage_backend = body['storage_backend']
        name = storage_backend.get('name', None)
        driver = storage_backend.get('driver', None)
        capability_specs = storage_backend.get('capability_specs', {})
        config_specs = storage_backend.get('config_specs', {})

        if name is None or name == "":
            raise webob.exc.HTTPBadRequest()

        try:
            storage_backends.create_backend(context, driver, name, capability_specs, config_specs)
            storage_backend = storage_backends.get_backend_by_name(context, name)
            notifier_info = dict(storage_backends=storage_backend)
            rpc.get_notifier('storageBackend').info(context, 'storage_backend.create',
                                                    notifier_info)

        except exception.StorageBackendExists as err:
            notifier_err = dict(backend_info=storage_backend, error_message=err)
            self._notify_storage_backend_error(context,
                                               'storage_backend.create',
                                               notifier_err)

            raise webob.exc.HTTPConflict(explanation=six.text_type(err))
        except Exception as err:
            LOG.warn("Exception: %s" % err)
            notifier_err = dict(backend_info=storage_backend, error_message=err)
            self._notify_storage_backend_error(context,
                                               'storage_backend.create',
                                               notifier_err)
            raise webob.exc.HTTPNotFound()

        return self._view_builder.show(req, storage_backend)

    @wsgi.action("delete")
    def delete(self, req, id):
        """Deletes an existing extra spec."""
        context = req.environ['sds.context']

        try:
            storage_backends.destroy_backend(context, id)
        except Exception as error:
            LOG.warn("Exception: %s" % error)
            raise webob.exc.HTTPNotFound(explanation=error.message)

        notifier_info = dict(id=id)
        notifier = rpc.get_notifier('storageBackend')
        notifier.info(context,
                      'storage_backend.delete',
                      notifier_info)
        return webob.Response(status_int=202)

def create_resource():
    return wsgi.Resource(StorageBackendsController())
