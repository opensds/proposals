# Copyright (c) 2014 Intel Corporation
# Copyright (c) 2014 OpenStack Foundation
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

"""Built-in volume type properties."""


from oslo.config import cfg
from oslo.db import exception as db_exc

from sds import context
from sds import db
from sds.common import exception
from sds.openstack.common import log as logging
import storage_tiers


CONF = cfg.CONF
LOG = logging.getLogger(__name__)


def create_backend(ctxt, driver, name, capability_specs={}, config_specs={}, update=False):
    """Creates storage backends."""
    return db.storage_backend_create(ctxt, dict(driver=driver, name=name, capability_specs=capability_specs, config_specs=config_specs), update)

def create_backend_capability_specs(ctxt, id, specs):
    return db.storage_backend_capability_specs_create(ctxt, dict(id = id, capability_specs=specs))

def create_backend_config_specs(ctxt, id, specs):
    return db.storage_backend_config_specs_create(ctxt, dict(id = id, config_specs=specs))

def destroy_backend(ctxt, id):
    """Marks storage backends as deleted."""
    return db.storage_backend_destroy(ctxt, id)

def destroy_backend_capability_specs(ctxt, id, skey):
    return db.storage_backend_capability_specs_destroy(ctxt,  dict(id=id, skey=skey))

def destroy_backend_config_specs(ctxt, id, skey):
    return db.storage_backend_config_specs_destroy(ctxt,  dict(id=id, skey=skey))

def set_backend_extra_info(ctxt, backend, inactive=0):
    if backend.get('config_specs_id'):
        backend['config_specs'] = get_backend_config_specs(ctxt, backend.get('id')).get('config_specs')
    else:
        backend['config_specs'] = None
    if backend.get('capability_specs_id'):
        backend['capability_specs'] = get_backend_capability_specs(ctxt, backend.get('id')).get('capability_specs')
    else:
        backend['capability_specs'] = None
    backend['tiers'] = storage_tiers.get_all_tiers(ctxt, 
                                                   search_opts=dict(storage_backend_id=backend.get('id')),
                                                   is_detail = True)

def get_all_backends(ctxt, inactive=0, search_opts={}, is_detail = False):
    _backend_info = db.storage_backend_get_all(ctxt, inactive, search_opts)
    if is_detail:
        for _backend in _backend_info:
            set_backend_extra_info(ctxt, _backend, inactive)
    return _backend_info

def get_backend_by_id(ctxt, id, is_detail = False):
    """Retrieves single storage backend by id."""
    _backend = db.storage_backend_get_by_id(ctxt, id)
    if _backend and is_detail:
        set_backend_extra_info(ctxt, _backend)
    return _backend

def get_backend_by_name(ctxt, name, is_detail = False):
    """Retrieves single backend by name."""
    _backend = db.storage_backend_get_by_name(ctxt, name)
    if is_detail:
        set_backend_extra_info(ctxt, _backend)
    return _backend

def get_backend_capability_specs(ctxt, id):
    return db.storage_backend_capability_specs_get(ctxt, dict(id = id))

def get_backend_config_specs(ctxt, id):
    return db.storage_backend_config_specs_get(ctxt, dict(id = id))
