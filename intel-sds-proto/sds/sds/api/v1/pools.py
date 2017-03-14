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
from sds.compose import api as compose_api
from sds.api import xmlutil
from sds.api.v1.views import pools as views_pools

CONF = cfg.CONF
LOG = logging.getLogger(__name__)

class StoragePoolTemplate(xmlutil.TemplateBuilder):
    def construct(self):
        root = xmlutil.make_flat_dict('storage_pool', selector='storage_pool')
        return xmlutil.MasterTemplate(root, 1)

class StoragePoolController(wsgi.Controller):
    """The storage system pooly API controller for the OpenStack API."""

    _view_builder_class = views_pools.ViewBuilder

    def __init__(self, *args, **kwargs):
        super(StoragePoolController, self).__init__(*args, **kwargs)
        self.pool_api = compose_api.API()

    def _notify_storage_pool_error(self, context, method, payload):
        rpc.get_notifier('storagePool').error(context, method, payload)

    """
        Create a pool for a given service type(s), setup associated configuration file (e.g. for cinder
        cinder.conf file) with backend info.
        pool : <pool>
        backend_name : name to be used to group storage system + tiers associated with this pool. Normally this
                translates to 'volume_backend_name' in cinder.conf for cinder.
        services: ['volume','file','backup','object']
        backends: [{'name': <name or id>, 'tiers' : [{'name': <name or id>, ..}]}, ..]
    """
    @wsgi.action("create")
    @wsgi.serializers(xml=StoragePoolTemplate)
    def _pool(self, req, body):
        context = req.environ['sds.context']

        if not self.is_valid_body(body, 'storage_pool'):
            raise webob.exc.HTTPBadRequest()

        # get info
        _info = body['storage_pool']
        pool = _info.get('pool')
        backend_name = _info.get('backend_name')
        if not backend_name: # use pool as backend_name name too which is nothing but volume_backend_name for cinder
            backend_name = pool
        services = _info.get('services')
        if not services: # use default service 'volume'
            services = ['volume']
        hosts = _info.get('hosts')
        if not isinstance(services, list):
            services = services.split(',')
        backends = _info.get('backends')

        # check few things before proceeding
        if not pool or pool == "" or not backends or not isinstance(backends, list) or len(backends) < 1:
            LOG.warn("Pool: %s, Backend: %s values are invalid" % (pool, backends))
            raise webob.exc.HTTPBadRequest()

        pool_info = dict(pool=pool,backend_name=backend_name,backends=backends,services=services,hosts=hosts)
        try:
            LOG.debug("calling create_pool with pool_info: %s" % (pool_info))
            self.pool_api.create_pool(context, pool, backend_name, backends, services, hosts)
            rpc.get_notifier('storagePool').info(context, 'storage_pool.create', pool_info)
        except Exception as err:
            notifier_err = dict(notifier_info=pool_info, error_message=err)
            self._notify_storage_pool_error(context, 'storage_pool.create', notifier_err)
            raise webob.exc.HTTPNotFound()

        return dict(storage_pool=pool_info)

    def index(self, req):
        """Returns the list of storage backends."""
        context = req.environ['sds.context']
        params = req.params.copy()
        services = ['volume', 'backup', 'file', 'object']
        if params.get('services'):
            services = params.get('services')
                
        try:
            pools_list = self.pool_api.get_pool_list(context, services)
            LOG.debug("pools_list: %s" % (pools_list))
            return self._view_builder.summary_list(req, pools_list)
        except Exception as err:
            notifier_err = dict(notifier_info=services, error_message=err)
            self._notify_storage_pool_error(context, 'storage_pool.index', notifier_err)
            raise webob.exc.HTTPNotFound()

    def show(self, req, id):
        LOG.debug("StoragePoolController show method got called with %s" % req)
        raise exc.HTTPNotFound()

    @wsgi.action("delete")
    def delete(self, req, id):
        """Deletes pool for a given pool, group, backends."""
        context = req.environ['sds.context']
        try:
            LOG.debug("deleting id: %s" % (id))
            self.pool_api.delete_pool_by_id(context, id)
            rpc.get_notifier('storagePool').info(context, 'storage_pool.delete', dict(id=id))
            return webob.Response(status_int=202)
        except Exception as err:
            LOG.warn("Exception: %s" % unicode(err))
            notifier_err = dict(notifier_info=id, error_message=err)
            self._notify_storage_pool_error(context, 'storage_pool.delete', notifier_err)
            raise webob.exc.HTTPNotFound(explanation=err.message)

def create_resource():
    return wsgi.Resource(StoragePoolController())
