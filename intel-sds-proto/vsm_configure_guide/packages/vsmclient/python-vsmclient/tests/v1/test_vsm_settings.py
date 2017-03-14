from vsmclient.v1 import client
from vsmclient import exceptions

vsmclient = client.Client(
                 'vsm',
                 'keystone_vsm_password',
                 'service',
                 auth_url='http://127.0.0.1:5000/v2.0/')

ret = vsmclient.vsm_settings.list(detailed=True)
print ret

err1 = {
        'name': '',
        'value': 'csefwe'

}

err2 = {
        'name': '123',
        'value': 'dfweqw'

    }


#try:
#    ret = vsmclient.vsm_settings.create(err1)
#except exceptions.BadRequest as e:
#    print e.message
#
#try:
#    ret = vsmclient.vsm_settings.create(err2)
#except exceptions.BadRequest as e:
#    print e.message
try:
    ret = vsmclient.vsm_settings.get(name='fhweiofhweo')
    print ret
except exceptions.NotFound as e:
    print e.message