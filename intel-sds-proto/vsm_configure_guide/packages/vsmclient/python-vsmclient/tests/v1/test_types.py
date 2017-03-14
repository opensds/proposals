from vsmclient import exceptions
from vsmclient.v1 import vsm_types
from tests import utils
from tests.v1 import fakes

cs = fakes.FakeClient()


class TypesTest(utils.TestCase):
    def test_list_types(self):
        tl = cs.vsm_types.list()
        cs.assert_called('GET', '/types')
        for t in tl:
            self.assertTrue(isinstance(t, vsm_types.VolumeType))

    def test_create(self):
        t = cs.vsm_types.create('test-type-3')
        cs.assert_called('POST', '/types')
        self.assertTrue(isinstance(t, vsm_types.VolumeType))

    def test_set_key(self):
        t = cs.vsm_types.get(1)
        t.set_keys({'k': 'v'})
        cs.assert_called('POST',
                         '/types/1/extra_specs',
                         {'extra_specs': {'k': 'v'}})

    def test_unsset_keys(self):
        t = cs.vsm_types.get(1)
        t.unset_keys(['k'])
        cs.assert_called('DELETE', '/types/1/extra_specs/k')

    def test_delete(self):
        cs.vsm_types.delete(1)
        cs.assert_called('DELETE', '/types/1')
