# Copyright 2012 OpenStack Foundation
# Copyright 2012 Nebula, Inc.
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

from django.template.defaultfilters import title  # noqa
from django.utils.translation import pgettext_lazy
from django.utils.translation import ugettext_lazy as _
from django.utils.translation import ungettext_lazy

from horizon import tables

from openstack_dashboard import api

import logging

LOG = logging.getLogger(__name__)


STATUS_DISPLAY_CHOICES = (
    ("active", pgettext_lazy("Current status of storage system", u"Active")),
    ("available", pgettext_lazy("Current status of storage system", u"Available")),
    ("error", pgettext_lazy("Current status of storage system", u"Error")),)


STATUS_CHOICES = (("active", True), ("available", True), ("error", False),
    )


class VirtualPoolFilterAction(tables.FilterAction):
    # Change default name of 'filter' to distinguish this one from the
    # project instances table filter, since this is used as part of the
    # session property used for persisting the filter.
    name = "filter_virtual_pool"
    filter_type = "server"
    filter_choices = (('name', _("Name="), True),
                      ('storagesystem', _("Storage System="), True),
                      ('tier', _("Tier="), False),
                      ('datatype', _("Data Type ="), True),
                      ('capacity', _("Capacity >="), True),
                      ('used', _("Capacity Used >="), True))

    def filter(self, table, data, filter_string):
        """Server side search.
        When filtering is supported in the api, then we will handle in view
        """
        filter_field = table.get_filter_field()
        if filter_field == 'Name' and filter_string:
            return [_data for _data in data
                    if data.storagesystem == filter_string]
        return data


class CreateVirtulPool(tables.LinkAction):
    name = "createvirtualpool"
    verbose_name = _("Create Virtual Pool")
    url = "horizon:admin:composepool:create"
    classes = ("ajax-modal", "btn-create")
    icon = "plus"


class RemoveVirtualPool(tables.DeleteAction):
    @staticmethod
    def action_present(count):
        return ungettext_lazy(
            u"Remove Virtual Pool",
            u"Remove Virtual Pools",
            count
        )

    @staticmethod
    def action_past(count):
        return ungettext_lazy(
            u"Remove Virtual Pool",
            u"Remove Virtual Pools",
            count
        )

    def allowed(self, request, pool=None):
        print("^^allowed got called for request: %s, pool: %s" % (request, pool))
        LOG.debug("^^^ allowed got called with pool: %s" % (pool))
        return True

    def delete(self, request, id):
        print("^^^ delete got called for request: %s, id: %s" % (request, id))
        LOG.debug("^^ request to delete id: %s" % (id))
        api.sds.storage_pools_delete(request, id)



class ComposePoolTable(tables.DataTable):
    
    #id  = tables.Column("id", verbose_name=_("ID"))
    name  = tables.Column("backend_name", verbose_name=_("Backend Name"))
    pool = tables.Column('pool', verbose_name=_("Pool"))
    services = tables.Column("services",
                         verbose_name=_("Services"))
    datatype = tables.Column("datatype",
                       verbose_name=_("Data Type"))

    class Meta:
        name = "composepool"
        verbose_name = _("Compose Virtual Pool")
        table_actions = (CreateVirtulPool, RemoveVirtualPool,)
        #row_actions = (StorageSystemDetail,)
