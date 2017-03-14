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

def create_pool(ctxt, values, update=False):
    """Creates storage pools."""
    return db.storage_pool_create(ctxt, values, update)

def destroy_pool(ctxt, values):
    return db.storage_pool_delete(ctxt, values)

def get_pool(ctxt, values, inactive = False):
    return db.storage_pool_get(ctxt, values, inactive)
