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

import re

from django.utils.translation import ugettext_lazy as _

from horizon import exceptions
from horizon import forms
from horizon import messages

from openstack_dashboard import api
from openstack_dashboard.api import base


NEW_LINES = re.compile(r"\r|\n")

NETWORK_RANGE_REGEX = re.compile(r"^[\d\.]+$", re.UNICODE)
NETWORK_ERROR_MESSAGES = {'invalid': _('Network range may '
                                   'only contain numbers, '
                                   'and hyphens.')}


class Discover(forms.SelfHandlingForm):

    storage_system = forms.ChoiceField(label=_("Storage System"), required=True)
    ip = forms.RegexField(max_length=255, label=_("Network Range"),
                          regex=NETWORK_RANGE_REGEX,
                          error_messages=NETWORK_ERROR_MESSAGES, required=False)
    user = forms.CharField(max_length=255, label=_("User Name"), required=False)
    fsid = forms.CharField(max_length=255, label=_("Cluster fsid"),
                           required=False)
    key = forms.CharField(max_length=255, label=_("Password"), required=False)

    def __init__(self, request, *args, **kwargs):
        super(Discover, self).__init__(request, *args, **kwargs)
        storage_choices = [("", _("Select storage system")),
                           ("ceph", _("Ceph(None authentication)")),
                           ("cephx", _("Ceph(Cephx authentication)")),
                           ("swift", _("Swift(User/Password authentication)")),
                           ("swift_key", _("Swift(Keystone authentication)"))]

        self.fields['storage_system'].choices = storage_choices

    def handle(self, request, data):
        try:
            ip_cidr = data.pop("ip")
            storage_type = data.pop("storage_system")

            metadata = {}
            for k, v in data.iteritems():
                if v:
                    metadata[k] = v
            if storage_type == 'swift_key':
                metadata = {'preauthtoken':request.user.token.id,
                            'preauthurl':base.url_for(request, "object-store")}

            #NOTE(fengqian): Cephx also means ceph, hard code here.
            if storage_type in ('cephx', 'ceph'):
                storage_type = 'ceph'
            elif storage_type in ('swift', 'swift_key'):
                storage_type = 'swift'

            discover_data = api.sds.discover_storage(request, ip_cidr,
                                                    storage_type, metadata)
            messages.success(request,
                             _('Successfully discover storage system: %s') \
                             % ip_cidr)

            return discover_data
        except Exception:
            exceptions.handle(request,
                              _('Unable to discover storage system.'))
            return False


class StorageDetails(forms.SelfHandlingForm):
    """ Empty class for StorageDetails form"""
    def handle(self, request, data):
        pass
