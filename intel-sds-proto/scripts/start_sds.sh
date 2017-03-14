#! /bin/bash

HOSTNAME='ENTER_HOSTNAME_HERE'
ADMIN_PASSWORD='ENTER_PASSWORD_HERE'


check_fail()
{
	if [ $? -ne 0 ]
	then
		echo "FAILURE!!"
		exit
	fi
}
# Setup Credentials
source /home/stack/devstack/accrc/admin/admin
check_fail

# Create SDS Endpoint/service if it's not registered yet
test=`keystone service-list | awk '{if ($4 ~ /^sds/) print $4}'`
if [ -z "$test" ]
then 
	keystone user-create --name=sds --pass='$ADMIN_PASSWORD' --email=email@example.com
	keystone user-role-add --user=sds --tenant=service --role=admin
	keystone service-create --name=sds --type=sds --description="SDS Service"
	keystone endpoint-create \
	  --service-id=$(keystone service-list | awk '/ sds / {print $2}') \
	  --publicurl=http://$HOSTNAME:9776/v1/%\(tenant_id\)s \
	  --internalurl=http://$HOSTNAME:9776/v1/%\(tenant_id\)s \
	  --adminurl=http://$HOSTNAME:9776/v1/%\(tenant_id\)s
fi 
check_fail

echo "Keystone Service and Endpoint Created!"
keystone service-list
