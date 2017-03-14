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
#    under the License

from django.core.urlresolvers import reverse_lazy
import json

from horizon import forms
from horizon import tables

from openstack_dashboard import api
from openstack_dashboard.dashboards.admin.discovercapability \
    import tables as project_tables
from openstack_dashboard.dashboards.admin.discovercapability \
    import forms as project_forms

import logging
LOG = logging.getLogger(__name__)


class Discover(forms.ModalFormView):

    form_class = project_forms.Discover
    template_name = 'admin/discovercapability/discover.html'
    success_url = reverse_lazy("horizon:admin:discovercapability:index")


class StorageDetails(forms.ModalFormView):

    form_class = project_forms.StorageDetails
    template_name = 'admin/discovercapability/storagedetails.html'
    success_url = reverse_lazy("horizon:admin:discovercapability:index")

    def get_context_data(self, **kwargs):
        context = super(StorageDetails, self).get_context_data(**kwargs)
        id = self.kwargs["id"]
        data = api.sds.storage_tiers_get(self.request, id)
        context['content'] = \
            json.dumps(data._info, sort_keys=True, indent=4, separators=(',', ': '))

        return context


class IndexView(tables.DataTableView):
    table_class = project_tables.DiscoverCapabilityTable
    template_name = 'admin/discovercapability/index.html'

    def get_data(self):
        return api.sds.storage_discovered_data(self.request)
