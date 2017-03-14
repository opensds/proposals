# Copyright 2010 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
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

"""Sds base exception handling.

Includes decorator for re-raising Sds-type exceptions.

SHOULD include dedicated exception logging.

"""

import sys

from oslo.config import cfg
import six
import webob.exc

from sds.i18n import _
from sds.openstack.common import log as logging


LOG = logging.getLogger(__name__)

exc_log_opts = [
    cfg.BoolOpt('fatal_exception_format_errors',
                default=False,
                help='Make exception message format errors fatal.'),
]

CONF = cfg.CONF
CONF.register_opts(exc_log_opts)


class ConvertedException(webob.exc.WSGIHTTPException):
    def __init__(self, code=0, title="", explanation=""):
        self.code = code
        self.title = title
        self.explanation = explanation
        super(ConvertedException, self).__init__()


class Error(Exception):
    pass


class SdsException(Exception):
    """Base Sds Exception

    To correctly use this class, inherit from it and define
    a 'message' property. That message will get printf'd
    with the keyword arguments provided to the constructor.

    """
    message = _("An unknown exception occurred.")
    code = 500
    headers = {}
    safe = False

    def __init__(self, message=None, **kwargs):
        self.kwargs = kwargs

        if 'code' not in self.kwargs:
            try:
                self.kwargs['code'] = self.code
            except AttributeError:
                pass

        for k, v in self.kwargs.iteritems():
            if isinstance(v, Exception):
                self.kwargs[k] = six.text_type(v)

        if not message:
            try:
                message = self.message % kwargs

            except Exception:
                LOG.debug("^^^^ Got Exception for message: %s..." % (message))
                exc_info = sys.exc_info()
                # kwargs doesn't match a variable in the message
                # log the issue and the kwargs
                LOG.exception(_('Exception in string format operation'))
                for name, value in kwargs.iteritems():
                    LOG.error("%s: %s" % (name, value))
                if CONF.fatal_exception_format_errors:
                    raise exc_info[0], exc_info[1], exc_info[2]
                # at least get the core message out if something happened
                message = self.message
        elif isinstance(message, Exception):
            message = six.text_type(message)

        # NOTE(luisg): We put the actual message in 'msg' so that we can access
        # it, because if we try to access the message via 'message' it will be
        # overshadowed by the class' message attribute
        self.msg = message
        super(SdsException, self).__init__(message)

    def __unicode__(self):
        return unicode(self.msg)


class NotAuthorized(SdsException):
    message = _("Not authorized.")
    code = 403


class AdminRequired(NotAuthorized):
    message = _("User does not have admin privileges")


class PolicyNotAuthorized(NotAuthorized):
    message = _("Policy doesn't allow %(action)s to be performed.")

class Invalid(SdsException):
    message = _("Unacceptable parameters.")
    code = 400

class SfJsonEncodeFailure(SdsException):
    message = _("Failed to load data into json format")


class InvalidResults(Invalid):
    message = _("The results are invalid.")


class InvalidInput(Invalid):
    message = _("Invalid input received: %(reason)s")


class InvalidContentType(Invalid):
    message = _("Invalid content type %(content_type)s.")


class InvalidHost(Invalid):
    message = _("Invalid host: %(reason)s")


# Cannot be templated as the error syntax varies.
# msg needs to be constructed when raised.
class InvalidParameterValue(Invalid):
    message = _("%(err)s")


class InvalidAuthKey(Invalid):
    message = _("Invalid auth key: %(reason)s")


class InvalidConfigurationValue(Invalid):
    message = _('Value "%(value)s" is not valid for '
                'configuration option "%(option)s"')


class ServiceUnavailable(Invalid):
    message = _("Service is unavailable at this time.")


class DeviceUnavailable(Invalid):
    message = _("The device in the path %(path)s is unavailable: %(reason)s")


class InvalidUUID(Invalid):
    message = _("Expected a uuid but received %(uuid)s.")


class NotFound(SdsException):
    message = _("Resource could not be found.")
    code = 404
    safe = True


class ServiceNotFound(NotFound):
    message = _("Service %(service_id)s could not be found.")


class HostNotFound(NotFound):
    message = _("Host %(host)s could not be found.")


class HostBinaryNotFound(NotFound):
    message = _("Could not find binary %(binary)s on host %(host)s.")


class FileNotFound(NotFound):
    message = _("File %(file_path)s could not be found.")


class MalformedRequestBody(SdsException):
    message = _("Malformed message body: %(reason)s")


class ConfigNotFound(NotFound):
    message = _("Could not find config at %(path)s")


class ParameterNotFound(NotFound):
    message = _("Could not find parameter %(param)s")


class PasteAppNotFound(NotFound):
    message = _("Could not load paste app '%(name)s' from %(path)s")


class NoValidHost(SdsException):
    message = _("No valid host was found. %(reason)s")


class NoMoreTargets(SdsException):
    """No more available targets."""
    pass


class SSHInjectionThreat(SdsException):
    message = _("SSH command injection detected: %(command)s")


class KeyManagerError(SdsException):
    msg_fmt = _("key manager error: %(reason)s")


########### 
class StorageBackendNotFound(NotFound):
    message = _("Storage System not found for %(key)s.")

class StorageBackendExists(SdsException):
    message = _("Storage System %(key)s already exists.")

class StorageBackendMissingKey(Invalid):
    message = _("Storage system needs $(key)s for query")

class InvalidStorageBackend(Invalid):
    message = _("Invalid storage backend: %(reason)s")

class StorageExtraSpecsNotFound(NotFound):
    message = _("Storage %(storage_id)s has no extra specs with "
                "key %(extra_specs_key)s.")

class StorageTierNotFound(NotFound):
    message = _("Storage tier not found for %(key)s.")

class StorageTierExists(SdsException):
    message = _("Storage tier %(key)s already exists.")

class StorageTierMissingKey(Invalid):
    message = _("Storage tier needs $(key)s for query")

class DriverMappingError(SdsException):
    message = _("Unable to find driver for backend %(key)s.")

class StoragePoolNotFound(SdsException):
    message = _("Storage pool could not be found for %(key)s.")

class CinderTypeOrBackendNameNotFound(SdsException):
    message = _("Cinder volume_type or volume_backend_name could not be found for volume_type: %(type)s.")


########### 
