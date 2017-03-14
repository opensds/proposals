# Copyright (c) 2013-2014 OpenStack Foundation
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from __future__ import print_function

import argparse
import copy
import os
import sys
import time
import json

import six

from sdsclient import exceptions
from sdsclient import utils
from sdsclient.openstack.common import strutils


def _poll_for_status(poll_fn, obj_id, action, final_ok_states,
                     poll_period=5, show_progress=True):
    """Blocks while an action occurs. Periodically shows progress."""
    def print_progress(progress):
        if show_progress:
            msg = ('\rInstance %(action)s... %(progress)s%% complete'
                   % dict(action=action, progress=progress))
        else:
            msg = '\rInstance %(action)s...' % dict(action=action)

        sys.stdout.write(msg)
        sys.stdout.flush()

    print()
    while True:
        obj = poll_fn(obj_id)
        status = obj.status.lower()
        progress = getattr(obj, 'progress', None) or 0
        if status in final_ok_states:
            print_progress(100)
            print("\nFinished")
            break
        elif status == "error":
            print("\nError %(action)s instance" % {'action': action})
            break
        else:
            print_progress(progress)
            time.sleep(poll_period)


def _translate_keys(collection, convert):
    for item in collection:
        keys = item.__dict__
        for from_key, to_key in convert:
            if from_key in keys and to_key not in keys:
                setattr(item, to_key, item._info[from_key])


def _extract_metadata(args):
    metadata = {}
    for metadatum in args.metadata:
        # unset doesn't require a val, so we have the if/else
        if '=' in metadatum:
            (key, value) = metadatum.split('=', 1)
        else:
            key = metadatum
            value = None

        metadata[key] = value
    return metadata


def do_endpoints(cs, args):
    """Discovers endpoints registered by authentication service."""
    catalog = cs.client.service_catalog.catalog
    for e in catalog['serviceCatalog']:
        utils.print_dict(e['endpoints'][0], e['name'])


def do_credentials(cs, args):
    """Shows user credentials returned from auth."""
    catalog = cs.client.service_catalog.catalog
    utils.print_dict(catalog['user'], "User Credentials")
    utils.print_dict(catalog['token'], "Token")


@utils.arg('--host', metavar='<hostname>', default=None,
           help='Host name. Default=None.')
@utils.arg('--binary', metavar='<binary>', default=None,
           help='Service binary. Default=None.')
def do_service_list(cs, args):
    """Lists all services. Filter by host and service binary."""
    result = cs.services.list(host=args.host, binary=args.binary)
    columns = ["Binary", "Host", "Zone", "Status", "State", "Updated_at"]
    # NOTE(jay-lau-513): we check if the response has disabled_reason
    # so as not to add the column when the extended ext is not enabled.
    if result and hasattr(result[0], 'disabled_reason'):
        columns.append("Disabled Reason")
    utils.print_list(result, columns)


@utils.arg('host', metavar='<hostname>', help='Host name.')
@utils.arg('binary', metavar='<binary>', help='Service binary.')
def do_service_enable(cs, args):
    """Enables the service."""
    result = cs.services.enable(args.host, args.binary)
    columns = ["Host", "Binary", "Status"]
    utils.print_list([result], columns)


@utils.arg('host', metavar='<hostname>', help='Host name.')
@utils.arg('binary', metavar='<binary>', help='Service binary.')
@utils.arg('--reason', metavar='<reason>',
           help='Reason for disabling service.')
def do_service_disable(cs, args):
    """Disables the service."""
    columns = ["Host", "Binary", "Status"]
    if args.reason:
        columns.append('Disabled Reason')
        result = cs.services.disable_log_reason(args.host, args.binary,
                                                args.reason)
    else:
        result = cs.services.disable(args.host, args.binary)
    utils.print_list([result], columns)


def _extract_search_opts(args):
    search_opts = {}
    for opt in args.search_opts:
        # unset doesn't require a val, so we have the if/else
        if '=' in opt:
            (key, value) = opt.split('=', 1)
        else:
            key = opt
            value = None

        search_opts[key] = value
    return search_opts

def _print_storage_backend_list(backends, detailed = False):
    if detailed:
        for backend in backends:
            print("============== storage system info ================")
            utils.print_list([backend], ['id', 'name', 'config_specs_id', 'config_specs', 'capability_specs_id', 'capability_specs'])
            tiers = backend.tiers
            if tiers:
                print("============== tier info ================")
                for row in tiers:
                    utils.print_dict(row)
    else:
        utils.print_list(backends, ['id', 'Name', 'config_specs_id', 'capability_specs_id'])


def _print_backend_and_extra_specs_list(backends):
    formatters = {'extra_specs': _print_type_extra_specs}
    utils.print_list(backends, ['id', 'Name', 'extra_specs'], formatters)

def _find_backend(cs, btype):
    """Get a backend type by name or ID."""
    return utils.find_resource(cs.storage_backends, btype)

def _find_tier(cs, btype):
    """Get a backend type by name or id."""
    return utils.find_resource(cs.storage_tiers, btype)

@utils.arg('--detail',
           dest='detail',
           metavar='<0|1>',
           nargs='?',
           type=int,
           const=1,
           default=0,
           help='Shows detailed backend info')
@utils.arg('search_opts',
           metavar='<key=value>',
           nargs='*',
           default=[],
           help='Search options with series of key value pairs for getting backend details')
def do_backend_list(cs, args):
    """Print a list of storage backends."""
    detailed = True if args.detail else False
    backends = cs.storage_backends.list(detailed = detailed, search_opts = _extract_search_opts(args))
    _print_storage_backend_list(backends, detailed)

@utils.arg('name',
           metavar='<name>',
           help="Name of the new storage backend")
@utils.arg('metadata',
           metavar='<key=value>',
           nargs='*',
           default=[],
           help='Specifications for Storage Backend Capabilities')
def do_backend_create(cs, args):
    """Create a new storage backend."""
    keypair = None
    if args.metadata is not None:
        keypair = _extract_metadata(args)
    backend = cs.storage_backends.create(args.name, keypair)
    _print_storage_backend_list([backend])

@utils.arg('id',
           metavar='<id>',
           help="Unique id of the storage backend to delete")
def do_backend_delete(cs, args):
    """Delete a storage backend."""
    cs.storage_backends.delete(args.id)

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend")
def do_backend_show(cs, args):
    """Get a storage backend."""
    vtype = _find_backend(cs, args.vtype)
    backend = cs.storage_backends.get(vtype.id)
    _print_storage_backend_list([backend], True)

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend")
@utils.arg('metadata',
           metavar='<key=value>',
           nargs='+',
           default=[],
           help='Specifications for Storage Backend Capabilities')
def do_backend_capability_set(cs, args):
    """Set storage capability."""
    vtype = _find_backend(cs, args.vtype)

    if args.metadata is not None:
        keypair = _extract_metadata(args)
        vtype.set_capability_keys(keypair)

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend")
@utils.arg('metadata',
           metavar='<key=value>',
           nargs='+',
           default=[],
           help='Specifications for Storage Backend Capabilities')
def do_backend_config_set(cs, args):
    """Set storage config."""
    vtype = _find_backend(cs, args.vtype)

    if args.metadata is not None:
        keypair = _extract_metadata(args)
        vtype.set_config_keys(keypair)

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend")
def do_backend_capability_show(cs, args):
    """Get storage capabilities."""
    vtype = _find_backend(cs, args.vtype)
    _specs = vtype.get_capability_keys()
    utils.print_list([_specs], ['id', 'name', 'capability_specs'])

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend")
def do_backend_config_show(cs, args):
    """Get storage config."""
    vtype = _find_backend(cs, args.vtype)
    _specs = vtype.get_config_keys()
    utils.print_list([_specs], ['id', 'name', 'config_specs'])

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend")
@utils.arg('metadata', metavar='key=value',
           nargs='+',
           default=[],
           help='config keys to delete')
def do_backend_config_keys_delete(cs, args):
    """Delete storage config for given keys."""
    vtype = _find_backend(cs, args.vtype)
    keypair = _extract_metadata(args)
    vtype.delete_config_keys(keypair)


@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend")
@utils.arg('metadata', metavar='key=value',
           nargs='+',
           default=[],
           help='capability keys to delete')
def do_backend_capability_keys_delete(cs, args):
    """Delete storage capability for given keys."""
    vtype = _find_backend(cs, args.vtype)
    keypair = _extract_metadata(args)
    vtype.delete_capability_keys(keypair)

@utils.arg('backend',
           metavar='<backend>',
           help="Name of the backend")
@utils.arg('tier_name',
           metavar='<tier_name>',
           help="Name of the new storage tier")
@utils.arg('metadata',
           metavar='<key=value>',
           nargs='*',
           default=[],
           help='Specifications for Storage Tier Capabilities')
def do_backend_tier_create(cs, args):
    """Create a new storage tier."""
    keypair = None
    if args.metadata is not None:
        keypair = _extract_metadata(args)
    backend_tier = cs.storage_tiers.create(args.backend, args.tier_name, keypair)
    utils.print_list([backend_tier], ['id', 'name', 'storage_backend_id'])

@utils.arg('id',
           metavar='<id>',
           help="Unique id of the storage backend tier to delete")
def do_backend_tier_delete(cs, args):
    """Delete a storage tier."""
    cs.storage_tiers.delete(args.id)

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend tier")
def do_backend_tier_show(cs, args):
    """Get a storage backend."""
    vtype = _find_tier(cs, args.vtype)
    tier = cs.storage_tiers.get(vtype.id)
    utils.print_list([tier], ['id', 'name', 'storage_backend_id', 'capability_specs_id', 'capability_specs'])

@utils.arg('--detail',
           dest='detail',
           metavar='<0|1>',
           nargs='?',
           type=int,
           const=1,
           default=0,
           help='Shows detailed tier info')
@utils.arg('search_opts',
           metavar='<key=value>',
           nargs='*',
           default=[],
           help='Search options with series of key value pairs for getting tier details')
def do_backend_tier_list(cs, args):
    """List storage tiers."""
    detailed = True if args.detail else False
    tiers = cs.storage_tiers.list(detailed = detailed, search_opts = _extract_search_opts(args))
    utils.print_list(tiers, ['id', 'name', 'storage_backend_id', 'capability_specs_id', 'capability_specs'])

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend tier")
@utils.arg('metadata',
           metavar='<key=value>',
           nargs='+',
           default=[],
           help='Specifications for Storage Backend Capabilities')
def do_backend_tier_capability_set(cs, args):
    """Set storage tier capabilities."""
    vtype = _find_tier(cs, args.vtype)

    if args.metadata is not None:
        keypair = _extract_metadata(args)
        vtype.set_capability_keys(keypair)

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend tier")
def do_backend_tier_capability_show(cs, args):
    """Get storage tier capabilities."""
    vtype = _find_tier(cs, args.vtype)
    _specs = vtype.get_capability_keys()
    utils.print_list([_specs], ['id', 'name', 'storage_backend_id', 'capability_specs_id', 'capability_specs'])

@utils.arg('vtype',
           metavar='<vtype>',
           help="Name or id of the backend tier")
@utils.arg('metadata', metavar='key=value',
           nargs='+',
           default=[],
           help='capability keys to delete')
def do_backend_tier_capability_keys_delete(cs, args):
    """Delete storage tier capabilities for given keys."""
    vtype = _find_tier(cs, args.vtype)
    keypair = _extract_metadata(args)
    vtype.delete_capability_keys(keypair)


@utils.arg('storage_type',
           metavar='<storage_type>',
           help="Type of storage system to search for (e.g., swift, ceph)")
@utils.arg('ip_cidr',
           metavar='<ip_cidr>',
           help="Search IP or IP Range with CIDR for storage systems")
@utils.arg('metadata', metavar='key=value',
           nargs='*',
           default=[],
           help='configuration keys to use for searching (e.g., user, cluster name)')
def do_discover(cs, args):
    """Discover storage systems."""
    metadata = _extract_metadata(args)
    ret = cs.storage_discover.discover(args.ip_cidr, args.storage_type, metadata)
    if ret:
        print("============== storage system info ================")
        print("name = %s" % ret.get('name'))
        print("storage config = %s" % ret.get('config_specs'))
        print("storage capabilities = %s" % ret.get('capability_specs'))
        tiers = ret.get('tiers')
        if tiers:
            print("============== tier info ================")
            for row in tiers:
                print("name = %s" % row.get('name'))
                print("capability specs = %s" % row.get('capability_specs'))


@utils.arg('pool',
           metavar='<pool>',
           help="Pool name to be created")
@utils.arg('backend_name',
           metavar='<backend_name>',
           help="Pool backend name to be used for config file setup")
@utils.arg('services',
           metavar='<services>',
           help="Services to be updated [volume,file,backup,object]")
@utils.arg('backends',
           metavar='<backends>',
           help='storage backends in json format e.g. [{"name": "ceph", "tiers": [{"name": "os-images"}]}]')
def do_pool_create(cs, args):
    """Create a new storage backend."""
    backends = json.loads(args.backends)
    pool = cs.storage_pools.create(args.pool, args.backend_name, args.services, backends)
    utils.print_list([pool], ['pool', 'backend_name'])


@utils.arg('--detail',
           dest='detail',
           metavar='<0|1>',
           nargs='?',
           type=int,
           const=1,
           default=0,
           help='Shows detailed backend info')
@utils.arg('search_opts',
           metavar='<key=value>',
           nargs='*',
           default=[],
           help='Search options with series of key value pairs for getting backend details')
def do_pool_list(cs, args):
    """Print a list of storage pools."""
    detailed = True if args.detail else False
    pools = cs.storage_pools.list(detailed = detailed, search_opts = _extract_search_opts(args))
    utils.print_list(pools, ['id', 'pool', 'backend_name', 'services', 'storage_backend_id', 'storage_system_name', 'storage_tier_id', 'storage_tier_name', 'section', 'host'])

@utils.arg('id',
           metavar='<id>',
           help="id for a given pool, backend_name, services, storage backends combination")
def do_pool_delete(cs, args):
    cs.storage_pools.delete(args.id)
