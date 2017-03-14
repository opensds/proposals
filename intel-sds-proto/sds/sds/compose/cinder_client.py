import copy
import sys
import inspect
import os
import random
import re
import six.moves.urllib.parse as urlparse

from sds import version

from oslo.config import cfg
from oslo.config import iniparser
from oslo.config import types
from oslo.utils import strutils

from keystoneclient import exceptions as keystone_exception
from keystoneclient import session
from keystoneclient.middleware import auth_token
from keystoneclient.auth.identity import v2 as v2_auth
from keystoneclient.auth.identity import v3 as v3_auth

from cinderclient import client as cinder_client
from cinderclient import exceptions as cinder_exception
from cinderclient.v1 import client as v1_client

from sds.openstack.common import excutils
from sds.openstack.common import importutils
from sds.openstack.common import log as logging

# cinder configuration
cinder_opts = [
    cfg.StrOpt('region_name',
               help='Region name of this node'),
    cfg.IntOpt('http_retries',
               default=3,
               help='Number of cinderclient retries on failed http calls'),
    cfg.IntOpt('api_version',
               default=2,
               help='Cinder API version'),
    cfg.StrOpt('admin_user',
               help='Keystone account username'),
    cfg.StrOpt('admin_password',
               secret=True,
               help='Keystone account password'),
    cfg.StrOpt('admin_tenant_name',
           default='admin',
           help='Keystone service account tenant name to validate'
           ' user tokens'),
    cfg.StrOpt('auth_uri',
               default=None,
               help='Complete public Identity API endpoint'),
    cfg.StrOpt('conf_file',
               default='/etc/cinder/cinder.conf',
               help='Cinder config file that need to be processed for updating backend info'),
    cfg.StrOpt('os_user',
               default='cinder',
               help='Cinder user that owns cinder configuration files')
]


CONF = cfg.CONF
CINDER_OPT_GROUP = 'cinder'
CONF.register_opts(cinder_opts, group=CINDER_OPT_GROUP)

LOG = logging.getLogger(__name__)

def get_keystone_session():
    ks_session = session.Session(False)
    ks_session.auth = v2_auth.Password(
        auth_url=CONF.cinder.auth_uri,
        username=CONF.cinder.admin_user,
        password=CONF.cinder.admin_password,
        tenant_name=CONF.cinder.admin_tenant_name)
    return ks_session

def create_cinder_client(session):
    return cinder_client.Client(str(CONF.cinder.api_version),
                                session=session,
                                username=CONF.cinder.admin_user,
                                api_key=CONF.cinder.admin_password,
                                tenant_id=CONF.cinder.admin_tenant_name,
                                auth_url=CONF.cinder.auth_uri,
                                #region_name=CONF.cinder.region_name,
                                connect_retries=CONF.cinder.http_retries)

# Helper function
def get_cinder_client():
    session = get_keystone_session()
    client = create_cinder_client(session)
    return client

"""
    get volume type for a given type_name (this can be either name or id)
"""
def get_volume_type(client, type_name):
    vol_types = client.volume_types.list()
    for t in vol_types:
        if type_name == t.name or type_name == t.id:
            return t

    return None

"""
    create volume type and set the backend name in extra specs
"""
def create_volume_type(client, type_name, backend_name):
    request = {
        "volume_type": {
            "name": type_name,
            "extra_specs": {
                "volume_backend_name": backend_name
            }
         }
    }

    resp, body = client.client.post('/types', body=request)

"""
    delete volume type 
"""
def delete_volume_type(client, type_id):
    client.volume_types.delete(type_id)


"""
    create cinder backend type if it doesn't exist and set the volume_backend_name 
"""
def create_backend(client, type_name, backend_name):
    # check if volume type exists
    vol_type = get_volume_type(client, type_name)
    if not vol_type:
        create_volume_type(client, type_name, backend_name)
        vol_type = get_volume_type(client, type_name)


""" 
    get list of hosts that need configuration update
    if backend name is provided, then it returns all cinder volume hosts servicing this backend
    if backend name is empty, then it will provide all cinder volume hosts
"""
def get_hosts(client, backend_name = None):
    hosts = set()
    services = client.services.list()
    for service in services:
        if service.binary == 'cinder-volume':
            elms = service.host.split('@')
            host = elms[0]
            if backend_name: # add only hosts that match backend name
                if len(elms) > 1 and elms[1] == backend_name:
                    hosts.add(host)
            else: # add every cinder-volume host
                hosts.add(host)

    return list(hosts)
    

"""
    get pool information from cinder scheduler
"""
def get_pool_info(client):
    hosts = set()

    # get volume types
    vol_types = client.volume_types.list()

    # TODO(arc) - does this raise exception or do we need to parse for status codes
    # e.g., resp.status_code in (200, 204)
    resp, body = client.client.get("/scheduler-stats/get_pools")
    pools = body.get('pools')
    pool_info = {}
    # sample: {u'pools': [{u'name': u'ihcontroller@lvm-1#iscsi_backend'}, {u'name': u'ihcontroller@rbd-1#rbd-backend'}]}
    for pool in pools:
        name = pool['name']
        entries = re.split(r'[@#]', name)
        if (len(entries) == 3): # TODO(arc): for now silently ignore parse errors
            host = entries[0]
            section = entries[1]
            backend_name = entries[2]
            for t in vol_types:
                if t.extra_specs and t.extra_specs['volume_backend_name'] == backend_name:
                    pool_info[t.name] = dict(host=host, section=section, backend_name=backend_name)

    return pool_info

