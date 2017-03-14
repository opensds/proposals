from vsmclient.v1 import client
from vsmclient.v1 import pool_usages


class ExtensionManager:
    def __init__(self, name, manager_class):
        self.name = name
        self.manager_class = manager_class

vsmclient = client.Client(
                 'vsm',
                 'keystone_vsm_password',
                 'service',
                 auth_url='http://127.0.0.1:5000/v2.0/',
                 extensions=[ExtensionManager('PoolUsageManager',
                                                pool_usages.PoolUsageManager)])
#
pool_id = ['1', '2', '3']
post = vsmclient.PoolUsageManager.create(pool_id)

print post

get = vsmclient.PoolUsageManager.list()
print get

for i in get:
    i.update(attach_status='success')

j = 0
for i in get:
    if j % 2 == 0:
        i.delete()
    j += 1

get = vsmclient.PoolUsageManager.list()
print get


