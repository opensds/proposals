from tests import utils
from tests.v1 import fakes


cs = fakes.FakeClient()


class VolumesTest(utils.TestCase):

    def test_delete_vsm(self):
        v = cs.vsms.list()[0]
        v.delete()
        cs.assert_called('DELETE', '/vsms/1234')
        cs.vsms.delete('1234')
        cs.assert_called('DELETE', '/vsms/1234')
        cs.vsms.delete(v)
        cs.assert_called('DELETE', '/vsms/1234')

    def test_create_vsm(self):
        cs.vsms.create(1)
        cs.assert_called('POST', '/vsms')

    def test_attach(self):
        v = cs.vsms.get('1234')
        cs.vsms.attach(v, 1, '/dev/vdc')
        cs.assert_called('POST', '/vsms/1234/action')

    def test_detach(self):
        v = cs.vsms.get('1234')
        cs.vsms.detach(v)
        cs.assert_called('POST', '/vsms/1234/action')

    def test_reserve(self):
        v = cs.vsms.get('1234')
        cs.vsms.reserve(v)
        cs.assert_called('POST', '/vsms/1234/action')

    def test_unreserve(self):
        v = cs.vsms.get('1234')
        cs.vsms.unreserve(v)
        cs.assert_called('POST', '/vsms/1234/action')

    def test_begin_detaching(self):
        v = cs.vsms.get('1234')
        cs.vsms.begin_detaching(v)
        cs.assert_called('POST', '/vsms/1234/action')

    def test_roll_detaching(self):
        v = cs.vsms.get('1234')
        cs.vsms.roll_detaching(v)
        cs.assert_called('POST', '/vsms/1234/action')

    def test_initialize_connection(self):
        v = cs.vsms.get('1234')
        cs.vsms.initialize_connection(v, {})
        cs.assert_called('POST', '/vsms/1234/action')

    def test_terminate_connection(self):
        v = cs.vsms.get('1234')
        cs.vsms.terminate_connection(v, {})
        cs.assert_called('POST', '/vsms/1234/action')

    def test_set_metadata(self):
        cs.vsms.set_metadata(1234, {'k1': 'v1'})
        cs.assert_called('POST', '/vsms/1234/metadata',
                         {'metadata': {'k1': 'v1'}})

    def test_delete_metadata(self):
        keys = ['key1']
        cs.vsms.delete_metadata(1234, keys)
        cs.assert_called('DELETE', '/vsms/1234/metadata/key1')
