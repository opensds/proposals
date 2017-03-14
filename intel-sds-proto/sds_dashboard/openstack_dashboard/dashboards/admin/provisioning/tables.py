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
from django import forms

from horizon import tables
from horizon.utils import filters

from openstack_dashboard import api
import logging
LOG = logging.getLogger(__name__)


STATUS_DISPLAY_CHOICES = (
    ("active", pgettext_lazy("Current status of storage system", u"Active")),
    ("available", pgettext_lazy("Current status of storage system", u"Available")),
    ("error", pgettext_lazy("Current status of storage system", u"Error")),)


STATUS_CHOICES = (("active", True), ("available", True), ("error", False),)

def empty_value_maker(type, name, value, attrs=None):
    def _empty_value_caller(datum):
        if type == "text":
            widget = forms.TextInput()
        elif type == "choice":
            widget = forms.ChoiceField().widget
        elif type == "checkbox":
            widget = forms.CheckboxInput()
        data = dict(name=name, value=value)

        if attrs:
            data.update(dict(attrs=attrs))
        data = widget.render(**data)
        return data
    return _empty_value_caller


class UpdateRow(tables.Row):
    ajax = True

    def get_data(self, request, id):
        res = api.vsm.get_server(request, id)
        return res


class ProvisioningFilterAction(tables.FilterAction):

    name = "filter_provisioning"
    filter_type = "server"
    filter_choices = (('name', _("Name"), True),
                      ('id', _("ID"), False),
                      ('ip', _("IP Address ="), True),
                      ('datadrive', _("Data Drives ="), True),
                      ('status', _("Status ="), True))

    def filter(self, table, data, filter_string):
        """Server side search.
        When filtering is supported in the api, then we will handle in view
        """
        filter_field = table.get_filter_field()
        if filter_field == 'Name' and filter_string:
            return [_data for _data in data
                    if data.name == filter_string]
        return data


class AddServersAction(tables.LinkAction):
    name = "add servers"
    verbose_name = _("Add Servers")
    url = "horizon:admin:provisioning:addserversview"
    classes = ("ajax-modal", "btn-create")
    icon = "plus"


class RemoveServersAction(tables.LinkAction):
    name = "remove servers"
    verbose_name = _("Remove Servers")
    url = "horizon:admin:provisioning:removeserversview"
    classes = ("ajax-modal", "btn-create")


class ProvisioningTable(tables.DataTable):

    storagesystem = tables.Column("storagesystem",
                                  verbose_name=_("Storage System"))
    name = tables.Column("host",
                         verbose_name=_("Name"))
    ip = tables.Column("primary_public_ip",
                       verbose_name=_("IP Address"),
                       attrs={'data-type': "ip"})
    
    datadrive = tables.Column("osds", verbose_name=_("Data Drives"))

    status = tables.Column(
        "status",
        filters=(title, filters.replace_underscores),
        verbose_name=_("Status"),
        status=True,
        status_choices=STATUS_CHOICES,
        display_choices=STATUS_DISPLAY_CHOICES)

    class Meta:
        name = "provisioning"
        verbose_name = _("Provisioning")
        status_columns = ["status"]
        table_actions = (AddServersAction, RemoveServersAction,)
        row_class = UpdateRow


class Ad(tables.LinkAction):
    name = "add"
    verbose_name = _(" ")
    url = "horizon:admin:provisioning:addserversview"


class AddServerTable(tables.DataTable):

    id = tables.Column("id",
                         verbose_name=_("ID"), classes=("server_id",))

    storagesystem = tables.Column("storagesystem",
                                  verbose_name=_("Storage System"))
    name = tables.Column("host",
                         verbose_name=_("Name"), classes=("name",))
    ip = tables.Column("primary_public_ip",
                       verbose_name=_("IP Address"),
                       attrs={'data-type': "ip"}, classes=("ip",))

    is_monitor = tables.Column("is_monitor", verbose_name=_("Monitor"),
                  classes=('monitor',),
                   empty_value=empty_value_maker("checkbox","is_monitor",""))

    is_storage = tables.Column("is_storage", verbose_name=_("Storage"),
                               classes=('storage',), \
            empty_value=empty_value_maker("checkbox","is_storage",1), hidden=True)

    zone = tables.Column("zone_id", verbose_name=_("Zone"), classes=('zone',))
 
    datadrive = tables.Column("osds", verbose_name=_("Data Drives(OSDs)"),
                               classes=("datadrive",))

    status = tables.Column(
        "status",
        filters=(title, filters.replace_underscores),
        verbose_name=_("Status"),
        status=True,
        status_choices=STATUS_CHOICES,
        display_choices=STATUS_DISPLAY_CHOICES,
        classes=("status",))

    class Meta:
        name = "serversaction"
        verbose_name = _("Servers")
        multi_select = True
        status_columns = ["status"]
        table_actions = (Ad,)


class RemoveServerTable(tables.DataTable):

    server_id = tables.Column("id", verbose_name=_("ID"), classes=("server_id",))
    storagesystem = tables.Column("storagesystem",
                                  verbose_name=_("Storage System"))

    name = tables.Column("host", classes=("name",), verbose_name="Name")

    ip = tables.Column("primary_public_ip",
                       verbose_name=_("IP Address"),
                       attrs={'data-type': "ip"}, classes=("ip",))

    zone = tables.Column("zone_id", verbose_name=_("Zone"), classes=('zone',))
    
    osds = tables.Column("osds", verbose_name=_("Data Drives(OSDs)"),
                         classes=("datadrive",))

    role = tables.Column("type", classes=("role",),
                               verbose_name=_("Role"))

    remove_storage = tables.Column("remove_storage", verbose_name=_("Storage"),
                                   classes=('remove_storage',), empty_value=\
                                   empty_value_maker("checkbox",
                                                     "remove_storage",True),
                                   hidden=True)

    status = tables.Column(
        "status",
        filters=(title, filters.replace_underscores),
        verbose_name=_("Status"),
        status=True,
        status_choices=STATUS_CHOICES,
        display_choices=STATUS_DISPLAY_CHOICES,
        classes=("status",))

    class Meta:
        name = "serversaction"
        verbose_name = _("Servers")
        multi_select = True
        status_columns = ["status"]
        table_actions = (Ad,)

