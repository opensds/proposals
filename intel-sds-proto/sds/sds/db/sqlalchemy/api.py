# Copyright (c) 2011 X.commerce, a business unit of eBay Inc.
# Copyright 2010 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# Copyright 2014 IBM Corp.
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

"""Implementation of SQLAlchemy backend."""


import functools
import sys
import threading
import time
import uuid
import re
import warnings

from oslo.config import cfg
from oslo.db import exception as db_exc
from oslo.db import options
from oslo.db.sqlalchemy import session as db_session
import osprofiler.sqlalchemy
import sqlalchemy
from sqlalchemy import or_
from sqlalchemy.orm import joinedload, joinedload_all
from sqlalchemy.orm import RelationshipProperty
from sqlalchemy.sql.expression import literal_column
from sqlalchemy.sql import func

from sds.common import sqlalchemyutils
from sds.db.sqlalchemy import models
from sds.common import exception
from sds.i18n import _
from sds.openstack.common import log as logging
from sds.openstack.common import timeutils
from sds.openstack.common import uuidutils

CONF = cfg.CONF
CONF.import_group("profiler", "sds.service")
LOG = logging.getLogger(__name__)

options.set_defaults(CONF, connection='sqlite:///$state_path/sds.sqlite')

_LOCK = threading.Lock()
_FACADE = None


def _create_facade_lazily():
    global _LOCK
    with _LOCK:
        global _FACADE
        if _FACADE is None:
            _FACADE = db_session.EngineFacade(
                CONF.database.connection,
                **dict(CONF.database.iteritems())
            )

            if CONF.profiler.profiler_enabled:
                if CONF.profiler.trace_sqlalchemy:
                    osprofiler.sqlalchemy.add_tracing(sqlalchemy,
                                                      _FACADE.get_engine(),
                                                      "db")

        return _FACADE


def get_engine():
    facade = _create_facade_lazily()
    return facade.get_engine()


def get_session(**kwargs):
    facade = _create_facade_lazily()
    return facade.get_session(**kwargs)

_DEFAULT_QUOTA_NAME = 'default'


def get_backend():
    """The backend is this module itself."""

    return sys.modules[__name__]


def is_admin_context(context):
    """Indicates if the request context is an administrator."""
    if not context:
        warnings.warn(_('Use of empty request context is deprecated'),
                      DeprecationWarning)
        raise Exception('die')
    return context.is_admin


def is_user_context(context):
    """Indicates if the request context is a normal user."""
    if not context:
        return False
    if context.is_admin:
        return False
    if not context.user_id or not context.project_id:
        return False
    return True


def authorize_project_context(context, project_id):
    """Ensures a request has permission to access the given project."""
    if is_user_context(context):
        if not context.project_id:
            raise exception.NotAuthorized()
        elif context.project_id != project_id:
            raise exception.NotAuthorized()


def authorize_user_context(context, user_id):
    """Ensures a request has permission to access the given user."""
    if is_user_context(context):
        if not context.user_id:
            raise exception.NotAuthorized()
        elif context.user_id != user_id:
            raise exception.NotAuthorized()


def require_admin_context(f):
    """Decorator to require admin request context.

    The first argument to the wrapped function must be the context.

    """

    def wrapper(*args, **kwargs):
        if not is_admin_context(args[0]):
            raise exception.AdminRequired()
        return f(*args, **kwargs)
    return wrapper


def require_context(f):
    """Decorator to require *any* user or admin context.

    This does no authorization for user or project access matching, see
    :py:func:`authorize_project_context` and
    :py:func:`authorize_user_context`.

    The first argument to the wrapped function must be the context.

    """

    def wrapper(*args, **kwargs):
        if not is_admin_context(args[0]) and not is_user_context(args[0]):
            raise exception.NotAuthorized()
        return f(*args, **kwargs)
    return wrapper


def _retry_on_deadlock(f):
    """Decorator to retry a DB API call if Deadlock was received."""
    @functools.wraps(f)
    def wrapped(*args, **kwargs):
        while True:
            try:
                return f(*args, **kwargs)
            except db_exc.DBDeadlock:
                LOG.warn(_("Deadlock detected when running "
                           "'%(func_name)s': Retrying..."),
                         dict(func_name=f.__name__))
                # Retry!
                time.sleep(0.5)
                continue
    functools.update_wrapper(wrapped, f)
    return wrapped


def model_query(context, *args, **kwargs):
    """Query helper that accounts for context's `read_deleted` field.

    :param context: context to query under
    :param session: if present, the session to use
    :param read_deleted: if present, overrides context's read_deleted field.
    :param project_only: if present and context is user-type, then restrict
            query to match the context's project_id.
    """
    session = kwargs.get('session') or get_session()
    read_deleted = kwargs.get('read_deleted') or context.read_deleted
    project_only = kwargs.get('project_only')

    query = session.query(*args)

    if read_deleted == 'no':
        query = query.filter_by(deleted=False)
    elif read_deleted == 'yes':
        pass  # omit the filter to include deleted and active
    elif read_deleted == 'only':
        query = query.filter_by(deleted=True)
    else:
        raise Exception(
            _("Unrecognized read_deleted value '%s'") % read_deleted)

    if project_only and is_user_context(context):
        query = query.filter_by(project_id=context.project_id)

    return query


###################


@require_admin_context
def service_destroy(context, service_id):
    session = get_session()
    with session.begin():
        service_ref = _service_get(context, service_id, session=session)
        service_ref.delete(session=session)


@require_admin_context
def _service_get(context, service_id, session=None):
    result = model_query(
        context,
        models.Service,
        session=session).\
        filter_by(id=service_id).\
        first()
    if not result:
        raise exception.ServiceNotFound(service_id=service_id)

    return result


@require_admin_context
def service_get(context, service_id):
    return _service_get(context, service_id)


@require_admin_context
def service_get_all(context, disabled=None):
    query = model_query(context, models.Service)

    if disabled is not None:
        query = query.filter_by(disabled=disabled)

    return query.all()


@require_admin_context
def service_get_all_by_topic(context, topic, disabled=None):
    query = model_query(
        context, models.Service, read_deleted="no").\
        filter_by(topic=topic)

    if disabled is not None:
        query = query.filter_by(disabled=disabled)

    return query.all()


@require_admin_context
def service_get_by_host_and_topic(context, host, topic):
    result = model_query(
        context, models.Service, read_deleted="no").\
        filter_by(disabled=False).\
        filter_by(host=host).\
        filter_by(topic=topic).\
        first()
    if not result:
        raise exception.ServiceNotFound(service_id=None)
    return result


@require_admin_context
def service_get_all_by_host(context, host):
    return model_query(
        context, models.Service, read_deleted="no").\
        filter_by(host=host).\
        all()


@require_admin_context
def _service_get_all_topic_subquery(context, session, topic, subq, label):
    sort_value = getattr(subq.c, label)
    return model_query(context, models.Service,
                       func.coalesce(sort_value, 0),
                       session=session, read_deleted="no").\
        filter_by(topic=topic).\
        filter_by(disabled=False).\
        outerjoin((subq, models.Service.host == subq.c.host)).\
        order_by(sort_value).\
        all()


@require_admin_context
def service_get_by_args(context, host, binary):
    result = model_query(context, models.Service).\
        filter_by(host=host).\
        filter_by(binary=binary).\
        first()

    if not result:
        raise exception.HostBinaryNotFound(host=host, binary=binary)

    return result


@require_admin_context
def service_create(context, values):
    service_ref = models.Service()
    service_ref.update(values)
    if not CONF.enable_new_services:
        service_ref.disabled = True

    session = get_session()
    with session.begin():
        service_ref.save(session)
        return service_ref


@require_admin_context
def service_update(context, service_id, values):
    session = get_session()
    with session.begin():
        service_ref = _service_get(context, service_id, session=session)
        service_ref.update(values)
        return service_ref


###################


#### Helper Functions ####

@require_context
def _storage_get_info(context, model, filters=None, inactive=False, all=False, session=None):
    read_deleted = "yes" if inactive else "no"

    if not filters:
        rows = model_query(context, model, session=session, read_deleted=read_deleted).\
                   order_by("name").\
                   all()
    else:
        rows = model_query(context, model, session=session, read_deleted=read_deleted).\
                   filter_by(**filters).\
                   all()

    # returns empty list if there are no rows
    result = list()
    if rows:
        result = [dict(row) for row in rows]

    # return just the dict if there is only one element otherwise return list
    if all:
        return result
    else:
        if len(result) > 0:
            return result[0]
        else:
            return None

@require_context
def _find_storage_backend(context, values, update, session, inactive=False):
    if values.get('name'):
        filter=dict(name=values.get('name'))
    elif values.get('id'):
        filter=dict(id=values.get('id'))
    else:
        raise exception.StorageBackendMissingKey(key = 'name or id')

    _info = _storage_get_info(context, models.StorageBackends, filters=filter, session = session, inactive = inactive)
    if update:
        if not _info:
            raise exception.StorageBackendNotFound(key = str(filter))
    else:
        if _info:
            raise exception.StorageBackendExists(key = str(filter))
    return _info


@require_context
def _find_storage_tier(context, values, update, session, inactive=False):
    if values.get('name'):
        filter=dict(name=values.get('name'))
    elif values.get('id'):
        filter=dict(id=values.get('id'))
    elif values.get('storage_backend_id'):
        filter=dict(storage_backend_id=values.get('storage_backend_id'))
    else:
        raise exception.StorageTierMissingKey(key = 'name or id or storage_backend_id')

    _info = _storage_get_info(context, models.StorageBackendTiers, filters=filter, session = session, inactive = inactive)
    if update:
        if not _info:
            raise exception.StorageTierNotFound(key = str(filter))
    else:
        if _info:
            raise exception.StorageTierExists(key = str(filter))
    return _info


@require_context
def _storage_extra_specs_get_item(context, storage_id, key, session=None):
    result = _storage_extra_specs_query(
        context, storage_id, session=session).\
        filter_by(skey=key).\
        first()

    if not result:
        raise exception.StorageExtraSpecsNotFound(
            extra_specs_key=key,
            storage_id=storage_id)

    return result

@require_context
def _storage_extra_specs_query(context, storage_id, session=None):
    return model_query(context, models.StorageExtraSpecs, session=session,
                       read_deleted="no").\
        filter_by(storage_id=storage_id)

@require_context
def _storage_specs_update_or_create(context, storage_id, specs, session):
    spec_ref = None
    for k, v in specs.iteritems():
        try:
            spec_ref = _storage_extra_specs_get_item(context, storage_id, k, session)
            model_query(context, models.StorageExtraSpecs, session=session,
                        read_deleted="no").\
                        filter_by(id=spec_ref['id']).\
                        update({"skey": k, "svalue": str(v), 'updated_at': timeutils.utcnow()})
        except exception.StorageExtraSpecsNotFound:
            spec_ref = models.StorageExtraSpecs()
            spec_ref.update({"skey": k, 
                             "svalue": str(v),
                             "storage_id": storage_id,
                             "deleted": False})
            spec_ref.save(session=session)
    return specs


@require_context
def _storage_backend_update(context, values, session):
        model_query(context, models.StorageBackends, session=session).\
            filter_by(id=values['id']).\
            update({'driver': values['driver'],
                    'updated_at': timeutils.utcnow()})

@require_context
def _storage_backend_capability_specs_update_or_create(context, values, session):
    _storage_backend = _find_storage_backend(context, values, True, session)

    if not _storage_backend.get('capability_specs_id'):
        _storage_backend['capability_specs_id'] = str(uuid.uuid4())
        model_query(context, models.StorageBackends, session=session).\
            filter_by(id=_storage_backend['id']).\
            update({'capability_specs_id': _storage_backend['capability_specs_id'],
                    'updated_at': timeutils.utcnow()})

    return _storage_specs_update_or_create(context, _storage_backend['capability_specs_id'], 
                values['capability_specs'], session)

@require_context
def _storage_backend_config_specs_update_or_create(context, values, session):
    _storage_backend = _find_storage_backend(context, values, True, session)

    if not _storage_backend.get('config_specs_id'):
        _storage_backend['config_specs_id'] = str(uuid.uuid4())
        model_query(context, models.StorageBackends, session=session).\
            filter_by(id=_storage_backend['id']).\
            update({'config_specs_id': _storage_backend['config_specs_id'],
                    'updated_at': timeutils.utcnow()})

    return _storage_specs_update_or_create(context, _storage_backend['config_specs_id'], 
                     values['config_specs'], session)

@require_context
def _storage_tier_capability_specs_update_or_create(context, values, session):
    _storage_tier = _find_storage_tier(context, values, True, session)

    if not _storage_tier.get('capability_specs_id'):
        _storage_tier['capability_specs_id'] = str(uuid.uuid4())
        model_query(context, models.StorageBackendTiers).\
            filter_by(id=_storage_tier['id']).\
            update({'capability_specs_id': _storage_tier['capability_specs_id'],
                    'updated_at': timeutils.utcnow()})

    return _storage_specs_update_or_create(context, _storage_tier['capability_specs_id'],
                values['capability_specs'], session)

@require_admin_context
def _storage_backend_destroy(context, filters):
    session = get_session()
    with session.begin():
        _backend = _find_storage_backend(context, filters, True, session)
        try: 
            _storage_tier_destroy(context, dict(storage_backend_id=_backend['id']), session)
        except exception.StorageTierNotFound:
            pass
        storage_backend_capability_specs_destroy(context, filters, session)
        storage_backend_config_specs_destroy(context, filters, session)
        model_query(context, models.StorageBackends, session=session).\
            filter_by(**filters).\
            update({'deleted': True,
                    'deleted_at': timeutils.utcnow(),
                    'updated_at': literal_column('updated_at')})

@require_admin_context
def _storage_tier_destroy(context, filters, session = None):
    if not session:
        session = get_session()
        with session.begin():
            _storage_tier_destroy_in_session(context, filters, session)
    else:
        _storage_tier_destroy_in_session(context, filters, session)

@require_admin_context
def _storage_tier_destroy_in_session(context, filters, session):
    storage_tier_capability_specs_destroy(context, filters, session)
    model_query(context, models.StorageBackendTiers, session=session).\
        filter_by(**filters).\
        update({'deleted': True,
                'deleted_at': timeutils.utcnow(),
                'updated_at': literal_column('updated_at')})


#### Functions called from external modules (API, Discovery etc.)

"""Create a new storage system or update an existing storage system
    "values" is a dictionary with following attributes
        'id': storage system UUID (optional)
        'driver': driver that is responsible for managing this storage system
        'name': name of the storage system (mandatory)
        'capability_specs': dictionary of storage system capabilities specified using key/value pairs (optional)
        'config_specs': dictionary of storage system configuration specified using key/value pairs (optional)
    "update"
        'True' - update existing record
        'False' - insert a new record
    returns dictionary of backend name/value pairs
"""
@require_admin_context
def storage_backend_create(context, values, update = False):
    session = get_session()
    with session.begin():
        try:
            # check if storage system exists
            _info = _find_storage_backend(context, values, update, session)

            # if record already exists
            if _info:
                _info['capability_specs']= values.get('capability_specs')
                if not _info.get('capability_specs_id') and values.get('capability_specs'):
                    _info['capability_specs_id']= str(uuid.uuid4())
                _info['config_specs'] = values.get('config_specs')
                if not _info.get('config_specs_id') and values.get('config_specs'):
                    _info['config_specs_id'] = str(uuid.uuid4())
                if not _info.get('driver') and values.get('driver'):
                    _info['driver'] = values.get('driver')
                    self._storage_backend_update(context, _info, session)
            else:
                _info = values
                # generate uuid if it doesn't exist
                if not _info.get('id'):
                    _info['id'] = str(uuid.uuid4())
                if _info.get('capability_specs'):
                    _info['capability_specs_id']= str(uuid.uuid4())
                if values.get('config_specs'):
                    _info['config_specs_id'] = str(uuid.uuid4())

                # create storage system
                storage_backend_ref = models.StorageBackends()
                storage_backend_ref.update(_info)
                storage_backend_ref.save(session=session)

            # insert all specification entries for QoS specs
            if values.get('capability_specs'):
                _storage_backend_capability_specs_update_or_create(context, _info, session)

            # insert all specification entries for QoS specs
            if values.get('config_specs'):
                _storage_backend_config_specs_update_or_create(context, _info, session)

        except (exception.StorageBackendExists, exception.StorageBackendNotFound, exception.StorageBackendMissingKey):
            raise
        except Exception as e:
            raise db_exc.DBError(e)
    return _info


"""Create a new storage backend tier or update an existing storage tier
    "values" is a dictionary with following attributes
        'backend_id': storage system UUID (either backend_id or backend_name must be specified)
        'backend_name': name of the storage system (either backend_id or backend_name must be specified)
        'tier_name': name of the storage tier (mandatory)
        'tier_id': uuid of storage tier (optional)
        'config_specs': dictionary of storage system configuration specified using key/value pairs (optional)
    "update"
        'True' - update existing record
        'False' - insert a new record
    returns dictionary of storage tier name/value pairs
"""
@require_admin_context
def storage_tier_create(context, values, update = False):
    session = get_session()
    with session.begin():
        try:
            _storage_backend = _find_storage_backend(context, 
                                    dict(name=values.get('backend_name'),id=values.get('backend_id')), 
                                    True, 
                                    session)


            _tier_ref = {'storage_backend_id': _storage_backend['id'], 'name' : values.get('tier_name')}

            # check if tier exists - exception is raised if it insert and if it already exists
            _info = _find_storage_tier(context, _tier_ref, update, session)

            if _info:
                _tier_ref = _info
                if values.get('capability_specs'):
                    if not _tier_ref['capability_specs_id']:
                        _tier_ref['capability_specs_id']= str(uuid.uuid4())
            else:
                if not values.get('tier_id'):
                    _tier_ref['id'] = str(uuid.uuid4())
                else:
                    _tier_ref['id'] =  values.get('tier_id')
                if values.get('capability_specs'):
                    _tier_ref['capability_specs_id']= str(uuid.uuid4())

                storage_tier_ref = models.StorageBackendTiers()
                storage_tier_ref.update(_tier_ref)
                storage_tier_ref.save(session=session)

            # insert all specification entries for QoS specs
            if values.get('capability_specs'):
                _tier_ref['capability_specs'] = values.get('capability_specs')
                _storage_tier_capability_specs_update_or_create(context, _tier_ref, session)


        except (exception.StorageBackendExists, exception.StorageBackendNotFound, exception.StorageBackendMissingKey):
            raise
        except (exception.StorageTierExists, exception.StorageTierNotFound, exception.StorageTierMissingKey):
            raise
        except Exception as e:
            raise db_exc.DBError(e)
    return _tier_ref

"""Create or update configuration for storage backend
    "values" is a dictionary with following attributes
        'id': storage system UUID (either name or id must be specified)
        'name': name of the storage system (either name or id must be specified)
        'config_specs': dictionary of storage system configuration specified using key/value pairs (optional)
            'config_specs' : {'k1': 'v1', 'k2': 'v2', ...}
    NOTE: empty config_specs won't generate any exception
"""
@require_admin_context
def storage_backend_config_specs_create(context, values):
    session = get_session()
    with session.begin():
        return _storage_backend_config_specs_update_or_create(context, values, session)

"""Create or update capabilities for storage backend
    "values" is a dictionary with following attributes
        'id': storage system UUID (either name or id must be specified)
        'name': name of the storage system (either name or id must be specified)
        'capability_specs': dictionary of storage system capabilities specified using key/value pairs (optional)
            'capability_specs' : {'k1': 'v1', 'k2': 'v2', ...}
    NOTE: empty capability_specs won't generate any exception
"""
@require_admin_context
def storage_backend_capability_specs_create(context, values):
    session = get_session()
    with session.begin():
        return _storage_backend_capability_specs_update_or_create(context, values, session)

"""Create or update capabilities for storage tier
    "values" is a dictionary with following attributes
        'id': storage tier UUID (either name or id or storage_backend_id must be specified)
        'name': name of the storage tier (either name or id or storage_backend_id must be specified)
        'storage_backend_id': storage system UUID (either name or id or storage_backend_id must be specified)
        'capability_specs': dictionary of storage tier capabilities specified using key/value pairs (optional)
            'capability_specs' : {'k1': 'v1', 'k2': 'v2', ...}
    NOTE: empty capability_specs won't generate any exception
"""
@require_admin_context
def storage_tier_capability_specs_create(context, values):
    session = get_session()
    with session.begin():
        return _storage_tier_capability_specs_update_or_create(context, values, session)

"""Get backend info for a given backend name
    returns a dictionary with {'name', 'config_specs_id', 'capability_specs_id', 'id'} key/value entries
"""
@require_context
def storage_backend_get_by_name(context, name, inactive=False):
    """Return a dict describing specific storage backend."""
    return _find_storage_backend(context, dict(name = name), True, None, inactive=inactive)


"""Get backend info for a given backend id
    returns a dictionary with {'name', 'config_specs_id', 'capability_specs_id', 'id'} key/value entries
"""
@require_context
def storage_backend_get_by_id(context, id, inactive=False):
    """Return a dict describing specific storage backend."""
    return _find_storage_backend(context, dict(id = id), True, None, inactive=inactive)

"""Get backend info for all backends
    returns a list with each backend in a dictionary with {'name', 'config_specs_id', 'capability_specs_id', 'id'} 
        key/value entries
"""
@require_context
def storage_backend_get_all(context, inactive=False, filters=None):
    """Returns a dict describing all storage_backends with name as key."""
    _info = _storage_get_info(context, models.StorageBackends, filters=filters, inactive=inactive, all=True)
    if not _info or len(_info) < 1:
        raise exception.StorageBackendNotFound(key = str('all entries'))
    return _info


"""Get capability specs for a given backend (specified by either id or name)
    "values" is a dictionary with following attributes
        'id': storage tier UUID (either name or id must be specified)
        'name': name of the storage tier (either name or id must be specified)

    returns a dictionary with
        'id': storage backend UUID 
        'name': storage backend name
        'capability_specs': dictionary of key/valye pairs 
"""
@require_context
def storage_backend_capability_specs_get(context, values, session=None, inactive=False):
    read_deleted = "yes" if inactive else "no"

    _storage_backend = _find_storage_backend(context, values, True, session, inactive) 

    rows = model_query(context,
                         models.StorageExtraSpecs,
                         session=session,
                         read_deleted=read_deleted).\
             filter_by(storage_id=_storage_backend['capability_specs_id']).\
             all()

    result = {'id' : _storage_backend['id'], 'name': _storage_backend['name']}
    if rows:
        _specs = dict([(x['skey'], x['svalue']) for x in rows])
        result['capability_specs'] = _specs
    else:
        result['capability_specs'] = None

    return dict(result)

"""Get config specs for a given backend (specified by either id or name)
    "values" is a dictionary with following attributes
        'id': storage tier UUID (either name or id must be specified)
        'name': name of the storage tier (either name or id must be specified)

    returns a dictionary with
        'id': storage backend UUID
        'name': storage backend name
        'config_specs': dictionary of key/valye pairs
"""
@require_context
def storage_backend_config_specs_get(context, values, session=None, inactive=False):
    read_deleted = "yes" if inactive else "no"

    _storage_backend = _find_storage_backend(context, values, True, session, inactive=inactive)

    rows = model_query(context,
                         models.StorageExtraSpecs,
                         session=session,
                         read_deleted=read_deleted).\
             filter_by(storage_id=_storage_backend['config_specs_id']).\
             all()

    result = {'id' : _storage_backend['id'], 'name': _storage_backend['name']}
    if rows:
        _specs = dict([(x['skey'], x['svalue']) for x in rows])
        result['config_specs'] = _specs
    else:
        result['config_specs'] = None

    return dict(result)

"""Get tier info for a given tier id
    returns a dictionary with {'name', 'capability_specs_id', 'id', 'storage_backend_id'} key/value entries
"""
@require_context
def storage_tier_get_by_id(context, id, inactive=False):
    return _find_storage_tier(context, dict(id = id), True, None, inactive=inactive)


"""Get tier info for a given tier name
    returns a dictionary with {'name', 'capability_specs_id', 'id', 'storage_backend_id'} key/value entries
"""
@require_context
def storage_tier_get_by_name(context, name, inactive=False):
    return _find_storage_tier(context, dict(name = name), True, None, inactive=inactive)

"""Get tier info for a given backend id
    returns a dictionary with {'name', 'capability_specs_id', 'id', 'storage_backend_id'} key/value entries
"""
@require_context
def storage_tier_get_by_backend_id(context, id, inactive=False):
    return _find_storage_tier(context, dict(storage_backend_id = id), True, None, inactive=inactive)


"""Get tier info for all
    returns a list of tiers with each entry specified using dictionary with {'name', 'capability_specs_id', 'id', 
        'storage_backend_id'} key/value entries
"""
@require_context
def storage_tier_get_all(context, inactive=False, filters=None):
    _info = _storage_get_info(context, models.StorageBackendTiers, filters=filters, inactive=inactive, all=True)
    if not _info:
        raise exception.StorageTierNotFound(key = str('all entries'))
    return _info


"""Get capability specs for a given tier (specified by either id or name or storage backend id)
    "values" is a dictionary with following attributes
        'id': storage tier UUID (either name or id or storage_backend_id must be specified)
        'name': name of the storage tier (either name or id or storage_backend_id must be specified)
        'storage_backend_id': storage system UUID (either name or id or storage_backend_id must be specified)

    returns a dictionary with
        'id': storage backend UUID
        'name': storage backend name
        'storage_backend_id': storage system UUID 
        'capability_specs': dictionary of key/valye pairs
"""
@require_context
def storage_tier_capability_specs_get(context, values, inactive=False):
    read_deleted = "yes" if inactive else "no"
    _storage_tier = _find_storage_tier(context, values, True, None, inactive)

    rows = model_query(context,
                         models.StorageExtraSpecs,
                         read_deleted=read_deleted).\
             filter_by(storage_id=_storage_tier['capability_specs_id']).\
             all()

    result = {'id' : _storage_tier['id'], 
              'storage_backend_id': _storage_tier['storage_backend_id'], 
              'name': _storage_tier['name']}
    if rows:
        _specs = dict([(x['skey'], x['svalue']) for x in rows])
        result['capability_specs'] = _specs
    else:
        result['capability_specs'] = None

    return dict(result)


"""Delete backend for a given id
   NOTE: Exception is not generated if there are no matching rows to be deleted
"""
@require_admin_context
def backend_destroy_by_id(context, id):
    return _storage_backend_destroy(context,dict(id=id))

"""Delete backend for a given name
   NOTE: Exception is not generated if there are no matching rows to be deleted
"""
@require_admin_context
def backend_destroy_by_name(context, name):
    return _storage_backend_destroy(context,dict(name=name))

"""Delete configuration specs for a given (backend id or name) and specific spec
   values is a dictionary that contains
        'id': storage backend UUID (either name or id must be specified)
        'name': name of the storage tier (either name or id must be specified)
        'spec_id': uuid of a given spec (optional)
        'skey': spec key (optional)
        'svalue': spec value (optional)
   NOTE: Exception is not generated if there are no matching rows to be deleted
"""
@require_context
def storage_backend_config_specs_destroy(context, values, session=None, inactive=False):
    read_deleted = "yes" if inactive else "no"

    _storage_backend = _find_storage_backend(context, values, True, None, inactive=inactive)
    if not _storage_backend.get('config_specs_id'):
        return

    filter_dict = dict(storage_id=_storage_backend['config_specs_id'])
    if values.get('spec_id'):
        filter_dict['id'] = values.get('spec_id')
    if values.get('skey'):
        filter_dict['skey'] = values.get('skey')
    if values.get('svalue'):
        filter_dict['svalue'] = values.get('svalue')
        
    model_query(context, models.StorageExtraSpecs, session=session).\
             filter_by(**filter_dict).\
             update({'deleted': True,
                     'deleted_at': timeutils.utcnow(),
                     'updated_at': literal_column('updated_at')})

"""Delete capability specs for a given (backend id or name) and specific spec
   values is a dictionary that contains
        'id': storage backend UUID (either name or id must be specified)
        'name': name of the storage tier (either name or id must be specified)
        'spec_id': uuid of a given spec (optional)
        'skey': spec key (optional)
        'svalue': spec value (optional)
   NOTE: Exception is not generated if there are no matching rows to be deleted
"""
@require_context
def storage_backend_capability_specs_destroy(context, values, session=None, inactive=False):
    read_deleted = "yes" if inactive else "no"

    _storage_backend = _find_storage_backend(context, values, True, session, inactive=inactive)
    if not _storage_backend.get('capability_specs_id'):
        return

    filter_dict = dict(storage_id=_storage_backend['capability_specs_id'])
    if values.get('spec_id'):
        filter_dict['id'] = values.get('spec_id')
    if values.get('skey'):
        filter_dict['skey'] = values.get('skey')
    if values.get('svalue'):
        filter_dict['svalue'] = values.get('svalue')

    model_query(context, models.StorageExtraSpecs, session=session).\
             filter_by(**filter_dict).\
             update({'deleted': True,
                     'deleted_at': timeutils.utcnow(),
                     'updated_at': literal_column('updated_at')})

"""Delete tier for a given id
   NOTE: Exception is not generated if there are no matching rows to be deleted
"""
@require_admin_context
def storage_tier_destroy_by_id(context, id):
    return _storage_tier_destroy(context,dict(id=id))

"""Delete tier for a given name
   NOTE: Exception is not generated if there are no matching rows to be deleted
"""
@require_admin_context
def storage_tier_destroy_by_name(context, name):
    return _storage_tier_destroy(context,dict(name=name))

"""Delete tier for a given backend id
   NOTE: Exception is not generated if there are no matching rows to be deleted
"""
@require_admin_context
def storage_tier_destroy_by_backend_id(context, id):
    return _storage_tier_destroy(context,dict(storage_backend_id=id))

"""Delete capability specs for a given (backend id or tier name or tier) and optionally specific spec
   values is a dictionary that contains
        'id': storage backend UUID (either name or id or storage_backend_id must be specified)
        'name': name of the storage tier (either name or id or storage_backend_id must be specified)
        'storage_backend_id': storage system UUID (either name or id or storage_backend_id must be specified)
        'spec_id': uuid of a given spec (optional)
        'skey': spec key (optional)
        'svalue': spec value (optional)
   NOTE: Exception is not generated if there are no matching rows to be deleted
"""
@require_context
def storage_tier_capability_specs_destroy(context, values, session=None, inactive=False):
    read_deleted = "yes" if inactive else "no"

    _storage_tier = _find_storage_tier(context, values, True, session, inactive)
    if not _storage_tier.get('capability_specs_id'):
        return

    filter_dict = dict(storage_id=_storage_tier['capability_specs_id'])
    if values.get('spec_id'):
        filter_dict['id'] = values.get('spec_id')
    if values.get('skey'):
        filter_dict['skey'] = values.get('skey')
    if values.get('svalue'):
        filter_dict['svalue'] = values.get('svalue')

    model_query(context, models.StorageExtraSpecs, session=session).\
             filter_by(**filter_dict).\
             update({'deleted': True,
                     'deleted_at': timeutils.utcnow(),
                     'updated_at': literal_column('updated_at')})


#### pool operations ####

@require_context
def storage_pool_get(context, values, session=None, inactive=False):
    read_deleted = "yes" if inactive else "no"
    if not session:
        session = get_session()
        session.begin() 

    filters = {}
    if values:
        for attr in ['id', 'pool', 'backend_name', 'storage_backend_id', 'storage_tier_id']:
            if values.get(attr):
                filters[attr] = values.get(attr)

    rows = model_query(context, models.StoragePools, session=session, read_deleted=read_deleted).\
                filter_by(**filters).\
                all()

    # raise exception if no rows
    if not rows or len(rows) < 1:
        if values:
            raise exception.StoragePoolNotFound(key="%s" % str(filters))
        else:
            return []

    # get backend and tier-name for each row
    result = [dict(row) for row in rows]
    for row in result:
        backend = storage_backend_get_by_id(context, row['storage_backend_id'])
        row['storage_system_name'] = backend['name']
        if row.get('storage_tier_id') and row.get('storage_tier_id') != '':
            tier = storage_tier_get_by_id(context, row['storage_tier_id'])
            row['storage_tier_name'] = tier['name']
    return result

def merge_services(cur, new):
    res = set()
    if cur:
        res.update(set(re.split(r'[,\s]\s*', cur.strip())))
    if new:
        res.update(set(re.split(r'[,\s]\s*', new.strip())))
    return ','.join(res)

def delete_services(cur, drop):
    res = set()
    if cur:
        res.update(set(re.split(r'[,\s]\s*', cur.strip())))
        if drop:
            for e in set(re.split(r'[,\s]\s*', drop.strip())):
                res.discard(e) # no exception is raised
        
    return ','.join(res)

"""
[{pool:<>,backend_name:<>,services:<>,storage_backend_id=<>,storage_tier_id=<>}, ...]
"""
@require_admin_context
def storage_pool_create(context, values, update = False):
    session = get_session()

    pool_results = []
    with session.begin():
        try:
            for pool in values:
                try:
                    pool_list = storage_pool_get(context, pool, session)
                    for pool_info in pool_list:
                        pool_info['services'] = merge_services(pool_info.get('services'), pool.get('services'))
                        upd_rec = dict(pool_info)
                        upd_rec.pop('id') # no need to update id
                        upd_rec['updated_at'] = timeutils.utcnow()
                        model_query(context, models.StoragePools, session=session, read_deleted="no").\
                            filter_by(id=pool_info['id']).\
                            update(upd_rec)
                        pool_results.append(pool_info)
                except (exception.StoragePoolNotFound):
                    pool_info = dict(pool)
                    pool_info['id'] = str(uuid.uuid4())
                    pool_ref = models.StoragePools()
                    pool_ref.update(dict(dict(deleted=False), **pool_info))
                    pool_ref.save(session=session)
                    pool_results.append(pool_info)
        except Exception as e:
            raise db_exc.DBError(e)
    return pool_results

"""
[{pool:<>,backend_name:<>,services:<>,storage_backend_id=<>,storage_tier_id=<>}, ...]
"""
@require_admin_context
def storage_pool_delete(context, values):
    session = get_session()

    with session.begin():
        try:
            for pool in values:
                pool_list = storage_pool_get(context, pool, session)
                for pool_info in pool_list:
                    if pool.get('services'):
                        pool_info['services'] = delete_services(pool_info.get('services'), pool.get('services'))
                    else:
                        pool_info['services'] = None

                filters = {}
                for attr in ['id', 'pool', 'backend_name', 'storage_backend_id', 'storage_tier_id']:
                    if pool.get(attr):
                        filters[attr] = pool.get(attr)

                # delete record if it doesn't have any entries in services, otherwise just change services
                if pool_info.get('services') and pool_info['services'] != "":
                    model_query(context, models.StoragePools, session=session).\
                        filter_by(**filters). \
                        update({'services': pool_info['services']})
                else:
                    model_query(context, models.StoragePools, session=session).\
                        filter_by(**filters). \
                        update({'deleted': True,
                                'deleted_at': timeutils.utcnow(),
                                'updated_at': literal_column('updated_at')})
        except Exception as e:
            raise db_exc.DBError(e)

