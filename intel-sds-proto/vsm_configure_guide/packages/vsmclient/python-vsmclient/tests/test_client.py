
import vsmclient.client
import vsmclient.v1.client
import vsmclient.v2.client
from tests import utils


class ClientTest(utils.TestCase):

    def test_get_client_class_v1(self):
        output = vsmclient.client.get_client_class('1')
        self.assertEqual(output, vsmclient.v1.client.Client)

    def test_get_client_class_v2(self):
        output = vsmclient.client.get_client_class('2')
        self.assertEqual(output, vsmclient.v2.client.Client)

    def test_get_client_class_unknown(self):
        self.assertRaises(vsmclient.exceptions.UnsupportedVersion,
                          vsmclient.client.get_client_class, '0')
