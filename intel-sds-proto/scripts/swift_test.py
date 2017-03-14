import logging
import swiftclient

LOG = logging.getLogger(__name__)


def required_swift_connection(fn):
    """Decorators to check the connection status"""
    def _wrapper(self, **kwargs):
        if self.con is None:
            LOG.debug("Connection status error, reconnecting")
            self._connect_swift()
        else:
            LOG.debug("Connection status is OK")

        return fn(self)

    return _wrapper
            

class SwiftDriver(object):
    """Class to get Swift information using swiftclient"""

    def __init__(self, host_ip, username, password):
        self.host_ip = host_ip
        self.username = username
        self.password = password
        self.container_list = []
        self.con = None
        self._connect_swift()

    def __del__(self):
        if self.con:
            self.con.close()

    def _connect_swift(self):
        LOG.debug("Connecting to Swift")

        self.authURL = "http://%s:8080/auth/v1.0" % self.host_ip
        self.con = swiftclient.client.Connection(self.authURL, self.username,
                                                 self.password)

    @required_swift_connection
    def get_container_list(self):
        containers = self.con.get_account()[1]
        for _container in containers:
            self.container_list.append(dict(name=_container.get('name')))
        
        return self.container_list

    @required_swift_connection
    def get_capabilities(self):
        return self.con.get_capabilities().get('swift')

    @required_swift_connection
    def get_container_info(self):
        for _container in self.container_list:
            print "Getting Container info for: %s" % _container['name']
            print self.con.head_container(_container['name']).get('x-storage-policy')

if __name__ == '__main__':
    test = SwiftDriver('10.0.0.51', 'test:tester', 'testing')
    print test.get_container_list()
    print test.get_capabilities()
    test.get_container_info()

