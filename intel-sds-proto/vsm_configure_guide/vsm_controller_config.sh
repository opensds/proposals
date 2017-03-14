#!/bin/bash

set -e
set -o xtrace
TOPDIR=`pwd`

if [ $# -ne 4 ]; then
    echo "Input error! Please following the format 'OPENSTACK_KEYSTONE_IP ADMIN_PASSWORD AGENT_PASSWORD KEYSTONE_VSM_SERVICE_PASSWORD'"
    exit
fi

openstack_ip=$1
admin_password=$2
agent_password=$3
vsm_service_password=$4
echo "http_proxy: $http_proxy"
echo "https_proxy: $https_proxy"

command -v pip >/dev/null 2>&1 || { yum -y install gcc python-setuptools; easy_install pip; }

########## Clear keystoneclient packages #########################
if [ -d /usr/lib/python2.6/site-packages/keystoneclient ]; then
    rm -rf /usr/lib/python2.6/site-packages/keystoneclient*
    rm -rf /usr/lib/python2.6/python_keystoneclient*
fi

cp -fr /usr/lib/python2.6/site-packages/python_keystoneclient* .

########## Clear oslo packages##################################
if [-d /usr/lib/python2.6/site-packages/oslo ]; then
    rm -rf /usr/lib/python2.6/site-packages/oslo*
fi

########### Install requirements packages ######################
pip install -r pip.inst

cp -fr python_keystoneclient* /usr/lib/python2.6/site-packages/

######### Change VSM configuration file #########################
sed -i 's/keystoneclient.middleware/keystonemiddleware/' /etc/vsm/api-paste.ini
sed  -i "s/^auth_host.*/auth_host = $openstack_ip/g" /etc/vsm/api-paste.ini
sed  -i "s/^admin_password.*/admin_password = $admin_password/g" /etc/vsm/api-paste.ini

#########Change vsmdeploy configuration file###################
sed -i "s/^KEYSTONE_HOST.*/KEYSTONE_HOST=$openstack_ip/g" /etc/vsmdeploy/deployrc
sed -i "s/^ADMIN_PASSWORD.*/ADMIN_PASSWORD=$admin_password/g" /etc/vsmdeploy/deployrc
sed -i "s/^ADMIN_TOKEN.*/ADMIN_TOKEN=$admin_password/g" /etc/vsmdeploy/deployrc
sed -i "s/^AGENT_PASSWORD.*/AGENT_PASSWORD=$agent_password/g" /etc/vsmdeploy/deployrc
sed -i "s/^KEYSTONE_VSM_SERVICE_PASSWORD.*/KEYSTONE_VSM_SERVICE_PASSWORD=$vsm_service_password/g" /etc/vsmdeploy/deployrc

######### Change VSM dashboard configuration files ##############

sed -i "s/^OPENSTACK_HOST.*/OPENSTACK_HOST = \"$openstack_ip\"/g" /etc/vsm-dashboard/local_settings
sed -i "s/^KEYSTONE_VSM_SERVICE_PASSWORD.*/KEYSTONE_VSM_SERVICE_PASSWORD = \"$vsm_service_password\"/g" /etc/vsm-dashboard/local_settings

sed -i "s/^OPENSTACK_HOST.*/OPENSTACK_HOST = \"$openstack_ip\"/g" /usr/share/vsm-dashboard/vsm_dashboard/local/local_settings.py
sed -i "s/^KEYSTONE_VSM_SERVICE_PASSWORD.*/KEYSTONE_VSM_SERVICE_PASSWORD = \"$vsm_service_password\"/g" /usr/share/vsm-dashboard/vsm_dashboard/local/local_settings.py

service httpd restart

#########Restart VSM services#################################
service vsm-api restart
service vsm-conductor restart
service vsm-scheduler restart

