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


CONF = cfg.CONF
LOG = logging.getLogger(__name__)

def create_tier(ctxt, name, backend_name, capability_specs={}, update = False):
    """Creates storage tiers."""
    return db.storage_tier_create(ctxt, dict(tier_name=name, backend_name=backend_name, capability_specs=capability_specs), update)

def create_tier_capability_specs(ctxt, id, specs):
    return db.storage_tier_capability_specs_create(ctxt, dict(id = id, capability_specs=specs))

def destroy_tier_by_id(ctxt, id):
    """Marks storage backends as deleted."""
    return db.storage_tier_destroy_by_id(ctxt, id)

def destroy_tier_capability_specs(ctxt, id, skey):
    return db.storage_tier_capability_specs_destroy(ctxt,  dict(id=id, skey=skey))

def set_tier_extra_info(ctxt, tier, inactive=0):
    if tier.get('capability_specs_id'):
        tier['capability_specs'] = get_tier_capability_specs(ctxt, tier.get('id')).get('capability_specs')
    else:
        tier['capability_specs'] = None

def get_all_tiers(ctxt, inactive=0, search_opts={}, is_detail = False):
    _tier_info = db.storage_tier_get_all(ctxt, inactive, search_opts)

    if is_detail:
        for _tier in _tier_info:
            set_tier_extra_info(ctxt, _tier, inactive)

    return _tier_info

def get_tier_capability_specs(ctxt, id):
    return db.storage_tier_capability_specs_get(ctxt, dict(id = id))

def get_tier_by_id(ctxt, id, is_detail = False):
    _tier = db.storage_tier_get_by_id(ctxt, id)
    if is_detail:
        set_tier_extra_info(ctxt, _tier)
    return _tier

def get_tier_by_name(ctxt, name, is_detail = False):
    _tier = db.storage_tier_get_by_name(ctxt, name)
    if is_detail:
        set_tier_extra_info(ctxt, _tier)
    return _tier

def get_tier_by_backend_id(ctxt, id, is_detail = False):
    _tier = db.storage_tier_get_by_backend_id(ctxt, id)
    if is_detail:
        set_tier_extra_info(ctxt, _tier)
    return _tier
