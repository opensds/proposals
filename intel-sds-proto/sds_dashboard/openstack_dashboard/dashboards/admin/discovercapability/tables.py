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

from django.utils.translation import ugettext_lazy as _

from horizon import tables


class StorageSystemDetails(tables.LinkAction):
    name = "storagedetails"
    verbose_name = _("Details")
    url = "horizon:admin:discovercapability:storagedetails"
    classes = ("ajax-modal",)
    icon = "pencil"


class DiscoverFilterAction(tables.FilterAction):
    name = "filter_discover"
    filter_type = "server"
    filter_choices = (('storagesystem', _("Storage System="), True),
                      ('tier', _("Tier="), False),
                      ('datatype', _("Data Type ="), True),
                      ('capacity', _("Capacity >="), True),
                      ('used', _("Capacity Used >="), True))

    def filter(self, table, data, filter_string):
        """Server side search.
        When filtering is supported in the api, then we will handle in view
        """
        filter_field = table.get_filter_field()
        if filter_field == 'Storage System' and filter_string:
            return [_data for _data in data
                    if data.storagesystem == filter_string]
        return data


class Discover(tables.LinkAction):
    name = "discover"
    verbose_name = _("Discover")
    classes = ("ajax-modal",)
    url = "horizon:admin:discovercapability:discover"
    icon = "plus"


class DiscoverCapabilityTable(tables.DataTable):
    
    storagesystem = tables.Column('storagesystem',
                                  verbose_name=_("Storage System"))
    tier = tables.Column("tier",
                         verbose_name=_("Tier"))
    datatype = tables.Column("data_type",
                       verbose_name=_("Data Type"))
    
    capacity = tables.Column("total", verbose_name=_("Capacity Total"))
    used = tables.Column("used", verbose_name=_("Capacity Used"))
    
    efficiency = tables.Column("data_efficiency",
                               verbose_name=_("Data Efficiency"))
    protection = tables.Column("protection", verbose_name=_("Data Protection"))
    dataservices = tables.Column("data_services", verbose_name=_("Data Services"))
    vendorservices = tables.Column("vendor_services",
                                   verbose_name=_("Vendor Services"))
    performance = tables.Column("performance_IOPS", verbose_name=_("Performance"))

    class Meta:
        name = "discover"
        verbose_name = _("Discover Capability")
        table_actions = (Discover,)
        row_actions = (StorageSystemDetails,)
