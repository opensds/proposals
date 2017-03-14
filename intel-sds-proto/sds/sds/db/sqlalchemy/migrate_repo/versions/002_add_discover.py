# Copyright (C) 2012 - 2014 EMC Corporation.
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

from sqlalchemy import Boolean, Column, DateTime, Text
from sqlalchemy import Integer, MetaData, String, Table, ForeignKey

from sds.i18n import _
from sds.openstack.common import log as logging

LOG = logging.getLogger(__name__)

def upgrade(migrate_engine):
    meta = MetaData()
    meta.bind = migrate_engine

    # New table
    storage_backends = Table(
        'storage_backends', meta,
        Column('created_at', DateTime(timezone=False)),
        Column('updated_at', DateTime(timezone=False)),
        Column('deleted_at', DateTime(timezone=False)),
        Column('deleted', Boolean(create_constraint=True, name=None)),
        Column('id', String(36), primary_key=True, nullable=False),
        Column('driver', String(length=255)),
        Column('name', String(length=255)),
        Column('capability_specs_id', String(length=36), index=True),
        Column('config_specs_id', String(length=36), index=True),
        mysql_engine='InnoDB',
        mysql_charset='utf8',
    )

    try:
        storage_backends.create()
    except Exception:
        LOG.error(_("Table |%s| not created!"), repr(storage_backends))
        raise

    # New table
    storage_backend_tiers = Table(
        'storage_backend_tiers', meta,
        Column('created_at', DateTime(timezone=False)),
        Column('updated_at', DateTime(timezone=False)),
        Column('deleted_at', DateTime(timezone=False)),
        Column('deleted', Boolean(create_constraint=True, name=None)),
        Column('id', String(36), primary_key=True, nullable=False),
        Column('name', String(length=255)),
        Column('storage_backend_id', String(length=36), ForeignKey('storage_backends.id'), nullable=False),
        Column('capability_specs_id', String(length=36), index=True),
        mysql_engine='InnoDB',
        mysql_charset='utf8',
    )

    try:
        storage_backend_tiers.create()
    except Exception:
        LOG.error(_("Table |%s| not created!"), repr(storage_backend_tiers))
        raise


    # New table
    storage_extra_specs = Table(
        'storage_extra_specs', meta,
        Column('created_at', DateTime(timezone=False)),
        Column('updated_at', DateTime(timezone=False)),
        Column('deleted_at', DateTime(timezone=False)),
        Column('deleted', Boolean(create_constraint=True, name=None)),
        Column('id', Integer, primary_key=True, nullable=False),
        Column('storage_id', String(length=36), index=True),
        Column('skey', String(length=255)),
        Column('svalue', Text),
        mysql_engine='InnoDB',
        mysql_charset='utf8',
    )

    try:
        storage_extra_specs.create()
    except Exception:
        LOG.error(_("Table |%s| not created!"), repr(storage_extra_specs))
        raise

    # New table
    storage_pools = Table(
        'storage_pools', meta,
        Column('created_at', DateTime(timezone=False)),
        Column('updated_at', DateTime(timezone=False)),
        Column('deleted_at', DateTime(timezone=False)),
        Column('deleted', Boolean(create_constraint=True, name=None)),
        Column('id', String(36), primary_key=True, nullable=False),
        Column('pool', String(length=255)),
        Column('backend_name', String(length=255)),
        Column('services', String(length=255)),
        Column('storage_backend_id', String(length=36), ForeignKey('storage_backends.id'), nullable=False),
        Column('storage_tier_id', String(length=36), ForeignKey('storage_backend_tiers.id'), nullable=False),
        mysql_engine='InnoDB',
        mysql_charset='utf8',
    )

    try:
        storage_pools.create()
    except Exception:
        LOG.error(_("Table |%s| not created!"), repr(storage_pools))
        raise

def downgrade(migrate_engine):
    meta = MetaData()
    meta.bind = migrate_engine

    # Drop table
    storage_extra_specs = Table('storage_extra_specs', meta, autoload=True)
    try:
        storage_extra_specs.drop()
    except Exception:
        LOG.error(_("storage_extra_specs table not dropped"))
        raise

    # Drop table
    storage_pools = Table('storage_pools', meta, autoload=True)
    try:
        storage_pools.drop()
    except Exception:
        LOG.error(_("storage_pools table not dropped"))
        raise

    # Drop table
    storage_backend_tiers = Table('storage_backend_tiers', meta, autoload=True)
    try:
        storage_backend_tiers.drop()
    except Exception:
        LOG.error(_("storage_backend_tiers table not dropped"))
        raise

    # Drop table
    storage_backends = Table('storage_backends', meta, autoload=True)
    try:
        storage_backends.drop()
    except Exception:
        LOG.error(_("storage_backends table not dropped"))
        raise
