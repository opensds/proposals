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

from django.core.urlresolvers import reverse
from django.core.urlresolvers import reverse_lazy
from django.utils.datastructures import SortedDict
from django.utils.translation import ugettext_lazy as _

from horizon import exceptions
from horizon import forms
from horizon import tables
from horizon.utils import memoized

from openstack_dashboard import api
from openstack_dashboard.dashboards.admin.composepool \
    import tables as project_tables
from openstack_dashboard.dashboards.admin.composepool \
    import forms as project_forms


LOG = logging.getLogger(__name__)

class IndexView(tables.DataTableView):
    table_class = project_tables.ComposePoolTable
    template_name = 'admin/composepool/index.html'

    def get_data(self):
        return api.sds.storage_pools_list(self.request)


class CreateVirtualPool(forms.ModalFormView):
    form_class = project_forms.CreateVirtualPoolForm
    template_name = 'admin/composepool/create.html'
    success_url = reverse_lazy("horizon:admin:composepool:index")

