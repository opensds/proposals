
import urllib
from vsmclient import base


class License(base.Resource):
    """"""
    def __repr__(self):
        return "<License: %s>" % self.id

    def delete(self):
        """Delete this vsm."""
        self.manager.delete(self)

    def update(self, **kwargs):
        """Update the display_name or display_description for this vsm."""
        self.manager.update(self, **kwargs)

    def force_delete(self):
        """Delete the specified vsm ignoring its current state.

        :param vsm: The UUID of the vsm to force-delete.
        """
        self.manager.force_delete(self)


class LicenseManager(base.ManagerWithFind):
    """"""
    resource_class = License


    def license_get(self):
        url = '/licenses/license_status_get'
        return self.api.client.get(url)


    def license_create(self, value):
        body = {'value': value}
        url = '/licenses/license_status_create'
        return self.api.client.post(url, body=body)

    def license_update(self, value):
        body = {'value': value}
        url = '/licenses/license_status_update'
        return self.api.client.post(url, body=body)
        
