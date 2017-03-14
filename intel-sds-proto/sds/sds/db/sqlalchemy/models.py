# Copyright (c) 2011 X.commerce, a business unit of eBay Inc.
# Copyright 2010 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# Copyright 2011 Piston Cloud Computing, Inc.
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
"""
SQLAlchemy models for sds data.
"""

from oslo.config import cfg
from oslo.db.sqlalchemy import models
from sqlalchemy import Column, Integer, String, Text, schema
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import ForeignKey, DateTime, Boolean
from sqlalchemy.orm import relationship, backref

from sds.openstack.common import timeutils


CONF = cfg.CONF
BASE = declarative_base()


class SdsBase(models.TimestampMixin,
                 models.ModelBase):
    """Base class for Sds Models."""

    __table_args__ = {'mysql_engine': 'InnoDB'}

    # TODO(rpodolyaka): reuse models.SoftDeleteMixin in the next stage
    #                   of implementing of BP db-cleanup
    deleted_at = Column(DateTime)
    deleted = Column(Boolean, default=False)
    metadata = None

    def delete(self, session):
        """Delete this object."""
        self.deleted = True
        self.deleted_at = timeutils.utcnow()
        self.save(session=session)


class Service(BASE, SdsBase):
    """Represents a running service on a host."""

    __tablename__ = 'services'
    id = Column(Integer, primary_key=True)
    host = Column(String(255))  # , ForeignKey('hosts.id'))
    binary = Column(String(255))
    topic = Column(String(255))
    report_count = Column(Integer, nullable=False, default=0)
    disabled = Column(Boolean, default=False)
    availability_zone = Column(String(255), default='sds')
    disabled_reason = Column(String(255))


################### 
class StorageBackends(BASE, SdsBase):
    """Represent possible storage_backends """
    __tablename__ = "storage_backends"
    id = Column(String(36), primary_key=True)
    driver = Column(String(255))
    name = Column(String(255))
    capability_specs_id = Column(String(36))
    config_specs_id = Column(String(36))

class StorageExtraSpecs(BASE, SdsBase):
    """Represents additional specs as key/value pairs for backends, tiers."""
    __tablename__ = 'storage_extra_specs'
    id = Column(Integer, primary_key=True)
    skey = Column(String(255))
    svalue = Column(Text)
    storage_id = Column(String(36), nullable=False)

class StorageBackendTiers(BASE, SdsBase):
    """Represents additional specs as key/value pairs for backends, tiers."""
    __tablename__ = 'storage_backend_tiers'
    id = Column(String(36), primary_key=True)
    name = Column(String(255))
    storage_backend_id = Column(String(36), ForeignKey('storage_backends.id'))
    capability_specs_id = Column(String(36))
    storage_backend = relationship(
        StorageBackends,
        backref="storage_backend_tiers",
        foreign_keys=storage_backend_id,
        primaryjoin='and_('
        'StorageBackendTiers.storage_backend_id == StorageBackends.id,'
        'StorageBackendTiers.deleted == False)'
    )

class StoragePools(BASE, SdsBase):
    """Represent possible storage_backends """
    __tablename__ = "storage_pools"
    id = Column(String(36), primary_key=True)
    pool = Column(String(255))
    backend_name = Column(String(255))
    services = Column(Text)
    storage_backend_id = Column(String(36), ForeignKey('storage_backends.id'))
    storage_tier_id = Column(String(36), ForeignKey('storage_backend_tiers.id'))
    storage_backend = relationship(
        StorageBackends,
        backref="storage_pools",
        foreign_keys=storage_backend_id,
        primaryjoin='and_('
        'StoragePools.storage_backend_id == StorageBackends.id,'
        'StoragePools.deleted == False)'
    )
    storage_tier = relationship(
        StorageBackendTiers,
        backref="storage_pools",
        foreign_keys=storage_tier_id,
        primaryjoin='and_('
        'StoragePools.storage_tier_id == StorageBackendTiers.id,'
        'StoragePools.deleted == False)'
    )
    


################### 

def register_models():
    """Register Models and create metadata.

    Called from sds.db.sqlalchemy.__init__ as part of loading the driver,
    it will never need to be called explicitly elsewhere unless the
    connection is lost and needs to be reestablished.
    """
    from sqlalchemy import create_engine
    models = (
              Service,
              StorageBackends,
              StorageExtraSpecs,
              StoragePools
              )
    engine = create_engine(CONF.database.connection, echo=False)
    for model in models:
        model.metadata.create_all(engine)
