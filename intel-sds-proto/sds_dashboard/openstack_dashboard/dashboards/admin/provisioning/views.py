# Copyright 2012 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
#
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

import logging
import os
import json
import time

from django.http import HttpResponse

from horizon import tables

from openstack_dashboard import api
from openstack_dashboard.dashboards.admin.provisioning \
    import tables as project_tables

LOG = logging.getLogger(__name__)

class ModalEditTableMixin(object):

    def get_template_names(self):
        if self.request.is_ajax():
            if not hasattr(self, "ajax_template_name"):
                # Transform standard template name to ajax name (leading "_")
                bits = list(os.path.split(self.template_name))
                bits[1] = "".join(("_", bits[1]))
                self.ajax_template_name = os.path.join(*bits)
            template = self.ajax_template_name
        else:
            template = self.template_name
        return template

    def get_context_data(self, **kwargs):
        context = super(ModalEditTableMixin, self).get_context_data(**kwargs)
        context['verbose_name'] = getattr(self, "verbose_name", "")
        context['submit_btn'] = getattr(self, "submit_btn", {})
        if self.request.is_ajax():
            context['hide'] = True
        return context


class AddServersView(ModalEditTableMixin, tables.DataTableView):
    table_class = project_tables.AddServerTable
    template_name = 'admin/provisioning/serversaction.html'
    #submit_btn = {"verbose_name": "Add Servers", "class": "add-server-commit"}

    def get_data(self):
        ret = []
        servers = api.vsm.get_server_list(self.request)
        for _server in servers:
            if _server.status in ['available', 'Available']:
                ret.append(_server)
        return ret


class RemoveServersView(ModalEditTableMixin, tables.DataTableView):
    table_class = project_tables.RemoveServerTable
    template_name = 'admin/provisioning/serverremove.html'
    #submit_btn = {"verbose_name": "Remove Servers", "class": "remove-server-commit"}

    def get_data(self):
        ret = []
        servers = api.vsm.get_server_list(self.request)
        for _server in servers:
            if _server.status in ['active', 'Active']:
                ret.append(_server)
        return ret


def ServersAction(request, action):
    
    post_data = request.body

    if not len(post_data):
        status = "error"
        msg = "No server selected"
    else:
        if action == "add":
            api.vsm.add_servers(request, json.loads(post_data))
            status = "info"
            msg = "Began to add servers"
        elif action == "remove":
            api.vsm.remove_servers(request, json.loads(post_data))
            status = "info"
            msg = "Began to remove servers"

    resp = dict(message=msg, status=status, data="")
    resp = json.dumps(resp)
    return HttpResponse(resp)


class IndexView(tables.DataTableView):
    table_class = project_tables.ProvisioningTable
    template_name = 'admin/provisioning/index.html'

    def get_data(self):
        ### TODO(arc): IndexView gets called after ServersAction and if it is fast, you may get stale 
        ### status resulting in not showing spinning progress bar
        time.sleep(1)
        ret = api.vsm.get_server_list(self.request)
        return ret

    def get_filters(self, filters):
        filter_field = self.table.get_filter_field()
        filter_action = self.table._meta._filter_action
        if filter_action.is_api_filter(filter_field):
            filter_string = self.table.get_filter_string()
            if filter_field and filter_string:
                filters[filter_field] = filter_string
        return filters
