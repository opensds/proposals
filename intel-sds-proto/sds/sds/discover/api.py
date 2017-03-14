# Copyright (c) 2014 Intel Corporation
# Copyright (c) 2014 OpenStack Foundation
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
Handles all requests related to storage discovery operations
"""

import datetime
import hashlib
import hmac
import random

from oslo.config import cfg

from sds.db import base
from sds.common import exception
from sds.openstack.common import excutils
from sds.openstack.common import importutils
from sds.openstack.common import log as logging
import storage_backends
import storage_tiers

discover_opts = [
    cfg.IntOpt('ping_count',
               default=2,
               help='number of ICMP ping requests to check if host is alive')
]

CONF = cfg.CONF
CONF.register_opts(discover_opts)

LOG = logging.getLogger(__name__)

class API(base.Base):
    """API for interacting with discovery."""

    mod_prefix = 'sds.discover.drivers'
    discover_class = "discover"

    def get_driver_instance(self, storage_type):
        # load driver
        try:
            _module_name = self.mod_prefix + "." + storage_type
            _module = importutils.import_module(_module_name)
        except Exception as e:
            LOG.warn("Failed to load module: %s. Error: %s" % (_module_name, e))
            raise e
        try:
            _class = _module.get_driver_class()
            driver = importutils.import_object(
                            _module_name + '.' + _class, 
                            db=self.db)
        except Exception as e:
            LOG.warn("Unable to load %s. Error: %s" % ("%s.%s" % (_module_name, _class), e))
            raise e
        return driver

    def __init__(self, db_driver=None):
        LOG.info("Discover API Init")
        super(API, self).__init__(db_driver)

    def store_in_db(self, context, _info):
        if not _info.get('name'):
            raise StorageBackendMissingKey(key='name')

        # check if data exists already
        _data = None
        try:
            _data = storage_backends.get_backend_by_name(context, _info.get('name'))
        except exception.StorageBackendNotFound:
            pass
        backend_update = True if _data else False
        storage_backends.create_backend(context, _info.get('driver'), _info.get('name'), 
                                        _info.get('capability_specs'), _info.get('config_specs'), 
                                        backend_update)

        LOG.info("Checking if there are Tiers in the Info dict")
        tiers = _info.get('tiers')
        if tiers:
            # NOTE: These are discrete transactions and it is possible to store only partial no. of of tiers
            for tier in tiers:
                LOG.info("Tier is: %s" % tier)
                if not tier.get('name'):
                    LOG.info("No Tier Defined!  Error!")
                    raise StorageTierMissingKey(key='name')

                _data = None
                try:
                    _data = storage_tiers.get_tier_by_name(context, tier.get('name'))
                except exception.StorageTierNotFound:
                    pass

                tier_update = True if _data else False
                storage_tiers.create_tier(context, tier['name'], _info['name'], tier.get('capability_specs'), tier_update)
                
    
    def discover(self, context, ip_cdr, storage_type, metadata, persist = True):
        LOG.info("Discover with ip_cdr: %s, storage_type: %s, metadata: %s" % (ip_cdr, storage_type, metadata))

        # load driver
        driver = self.get_driver_instance(storage_type)

        # discover storage system
        _info = driver.discover(ip_cdr, metadata)
        LOG.info("discover results for ip_cdr: %s is %s" % (ip_cdr, _info))
        
        # optionally store data in database
        if persist and _info:
            try:
                self.store_in_db(context, _info)
            except Exception as e:
                LOG.warn("Unable to store discovery info into database. Exception: %s" % (e))
                raise e

        return _info
