# Copyright 2012 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
#
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

from django.utils.translation import ugettext_lazy as _

from horizon import exceptions
from horizon import forms
from horizon import messages

from openstack_dashboard import api

LOG = logging.getLogger(__name__)


class CreateVirtualPoolForm(forms.SelfHandlingForm):

    name = forms.CharField(max_length=255, label=_("Name"))
    system_type = forms.ChoiceField(label = _('Storage System'), required = True,)
    ceph = forms.ChoiceField(label = _('Attached Tiers'), required = False,)
    swift = forms.ChoiceField(label = _('Attached Tiers'), required = False,)

    def __init__(self, request, *args, **kwargs):
        super(CreateVirtualPoolForm, self).__init__(request, *args, **kwargs)
        storage_choices = [("", _("Select storage system")),
                           ("ceph", _("Ceph")), 
                           ("swift", _("Swift"))]
        
        backends = api.sds.storage_backends_list(request, True)
        tiers_choices = dict(ceph=[], swift=[])
        for _backend in backends:
            tiers_choices[_backend.name] = [(tier['id'], tier['name']) for \
                                            tier in _backend.tiers]
        for _tiers_choices in tiers_choices.values():
            if _tiers_choices:
                _tiers_choices.insert(0, ("", _("Select tiers")))
            else:
                _tiers_choices.insert(0, ("", _("No avaliable tiers")))
        self.fields['system_type'].choices = storage_choices
        self.fields['ceph'].choices = tiers_choices['ceph']
        self.fields['swift'].choices = tiers_choices['swift']

    def handle(self, request, data):
        pool = data.get('name')
        #NOTE(fengqian): Default backend_name = pool_name + 'backend'
        backend_name = "%s_backend" % pool
        backends_name = data.get('system_type')
        backends_tiers = data.get(backends_name)
        backends = [dict(name=backends_name, tiers=[dict(name=backends_tiers)])]

        try:
            ret = api.sds.storage_pools_create(request, pool, backend_name, backends)
            messages.success(request,
                             _('Successfully compose virtual pool: %s') \
                             % pool)
            return ret
        
        except Exception:
            exceptions.handle(request, _('Unable to compose virtual pool.'))
            return False
