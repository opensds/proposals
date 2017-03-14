# Copyright (c) 2011 X.commerce, a business unit of eBay Inc.
# Copyright 2010 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
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

"""Defines interface for DB access.

The underlying driver is loaded as a :class:`LazyPluggable`.

Functions in this module are imported into the sds.db namespace. Call these
functions from sds.db namespace, not the sds.db.api namespace.

All functions in this module return objects that implement a dictionary-like
interface. Currently, many of these objects are sqlalchemy objects that
implement a dictionary interface. However, a future goal is to have all of
these objects be simple dictionaries.


**Related Flags**

:backend:  string to lookup in the list of LazyPluggable backends.
           `sqlalchemy` is the only supported backend right now.

:connection:  string specifying the sqlalchemy connection to use, like:
              `sqlite:///var/lib/sds/sds.sqlite`.

:enable_new_services:  when adding a new service to the database, is it in the
                       pool of available hardware (Default: True)

"""

from oslo.config import cfg
from oslo.db import concurrency as db_concurrency
from oslo.db import options as db_options


db_opts = [
    cfg.StrOpt('db_backend',
               default='sqlalchemy',
               help='The backend to use for db'),
    cfg.BoolOpt('enable_new_services',
                default=True,
                help='Services to be added to the available pool on create'), ]


CONF = cfg.CONF
CONF.register_opts(db_opts)
db_options.set_defaults(CONF)
CONF.set_default('sqlite_db', 'sds.sqlite', group='database')

_BACKEND_MAPPING = {'sqlalchemy': 'sds.db.sqlalchemy.api'}

IMPL = db_concurrency.TpoolDbapiWrapper(CONF, _BACKEND_MAPPING)


###################


def service_destroy(context, service_id):
    """Destroy the service or raise if it does not exist."""
    return IMPL.service_destroy(context, service_id)


def service_get(context, service_id):
    """Get a service or raise if it does not exist."""
    return IMPL.service_get(context, service_id)


def service_get_by_host_and_topic(context, host, topic):
    """Get a service by host it's on and topic it listens to."""
    return IMPL.service_get_by_host_and_topic(context, host, topic)


def service_get_all(context, disabled=None):
    """Get all services."""
    return IMPL.service_get_all(context, disabled)


def service_get_all_by_topic(context, topic, disabled=None):
    """Get all services for a given topic."""
    return IMPL.service_get_all_by_topic(context, topic, disabled=disabled)


def service_get_all_by_host(context, host):
    """Get all services for a given host."""
    return IMPL.service_get_all_by_host(context, host)


def service_get_by_args(context, host, binary):
    """Get the state of an service by node name and binary."""
    return IMPL.service_get_by_args(context, host, binary)


def service_create(context, values):
    """Create a service from the values dictionary."""
    return IMPL.service_create(context, values)


def service_update(context, service_id, values):
    """Set the given properties on an service and update it.

    Raises NotFound if service does not exist.

    """
    return IMPL.service_update(context, service_id, values)


###################

def storage_backend_create(context, values, update = False):
    """Create a new storage backend ."""
    return IMPL.storage_backend_create(context, values, update)

def storage_backend_get_all(context, inactive=False, filters=None):
    """Get all storage backends"""
    return IMPL.storage_backend_get_all(context, inactive, filters)

def storage_backend_get_by_id(context, id, inactive=False):
    """Get volume type by id."""
    return IMPL.storage_backend_get_by_id(context, id, inactive)

def storage_backend_get_by_name(context, name):
    """Get volume type by name."""
    return IMPL.storage_backend_get_by_name(context, name)

def storage_backend_destroy(context, backend_id):
    """Destroy the backend or raise if it does not exist."""
    return IMPL.backend_destroy_by_id(context, backend_id)

def storage_backend_capability_specs_create(context, values):
    return IMPL.storage_backend_capability_specs_create(context, values)

def storage_backend_config_specs_create(context, values):
    return IMPL.storage_backend_config_specs_create(context, values)

def storage_backend_capability_specs_get(context, values):
    return IMPL.storage_backend_capability_specs_get(context, values)

def storage_backend_config_specs_get(context, values):
    return IMPL.storage_backend_config_specs_get(context, values)

def storage_backend_config_specs_destroy(context, values):
    return IMPL.storage_backend_config_specs_destroy(context, values)

def storage_backend_capability_specs_destroy(context, values):
    return IMPL.storage_backend_capability_specs_destroy(context, values)

def storage_tier_create(context, values, update = False):
    return IMPL.storage_tier_create(context, values, update)

def storage_tier_get_by_id(context, id, inactive=False):
    return IMPL.storage_tier_get_by_id(context, id, inactive)

def storage_tier_get_by_name(context, name, inactive=False):
    return IMPL.storage_tier_get_by_name(context, name, inactive)

def storage_tier_get_by_backend_id(context, id, inactive=False):
    return IMPL.storage_tier_get_by_backend_id(context, id, inactive)

def storage_tier_get_all(context, inactive=False, filters=None):
    return IMPL.storage_tier_get_all(context, inactive, filters)

def storage_tier_destroy_by_id(context, id):
    return IMPL.storage_tier_destroy_by_id(context, id)

def storage_tier_destroy_by_name(context, name):
    return IMPL.storage_tier_destroy_by_name(context, name)

def storage_tier_destroy_by_backend_id(context, id):
    return IMPL.storage_tier_destroy_by_backend_id(context, id)

def storage_tier_capability_specs_create(context, values):
    return IMPL.storage_tier_capability_specs_create(context, values)

def storage_tier_capability_specs_get(context, values):
    return IMPL.storage_tier_capability_specs_get(context, values)

def storage_tier_capability_specs_destroy(context, values):
    return IMPL.storage_tier_capability_specs_destroy(context, values)

def storage_pool_create(context, values, update = False):
    return IMPL.storage_pool_create(context, values, update)

def storage_pool_delete(context, values):
    return IMPL.storage_pool_delete(context, values)

def storage_pool_get(context, values, inactive = False):
    return IMPL.storage_pool_get(context, values, inactive=inactive)
############# 
