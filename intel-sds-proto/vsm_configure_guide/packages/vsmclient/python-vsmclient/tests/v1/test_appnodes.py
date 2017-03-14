from vsmclient.v1 import client
from vsmclient.v1 import appnodes


class ExtensionManager:
    def __init__(self, name, manager_class):
        self.name = name
        self.manager_class = manager_class

vsmclient = client.Client(
                 'vsm',
                 'keystone_vsm_password',
                 'service',
                 auth_url='http://127.0.0.1:5000/v2.0/',
                 extensions=[ExtensionManager('AppNodeManager',
                                                appnodes.AppNodeManager)])

#ip_list = ["10.239.131.170", "10.239.131.255"]
##ip_list = '10.239.131.255'
#post = vsmclient.AppNodeManager.create(ip_list)
#
#print post

get = vsmclient.AppNodeManager.list()
print get

#for i in get:
#    i.update(ssh_status='running', log_info='test')
#
j = 0
for i in get:
    if j % 2 == 0:
        i.delete()
    j += 1

#get = vsmclient.AppNodeManager.list()
#print get


