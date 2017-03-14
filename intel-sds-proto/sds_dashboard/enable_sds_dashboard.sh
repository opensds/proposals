#!/bin/bash

set -e
set -o xtrace

HORIZON_PATH='/opt/stack/horizon'
STATIC_PATH='/opt/stack/horizon'
USER_ID=stack

if [ ! -d $HORIZON_PATH ]; then
    echo "The script assume the horizon path is /opt/stack/horizon. But the horizon couldn't be found now \
          Please check. Or else, please input the horizon folder path:"
    read -p "HORIZON_PATH:" HORIZON_PATH
    until [[ -d $HORIZON_PATH ]];
    do
        echo "Inputed horizon path not found, please input again!"
        read -p "HORIZON_PATH:" HORIZON_PATH
        echo $HORIZON_PATH
    done
    echo "Inputed horizon path found!"

else
    echo "Horizon path found on $HORIZON_PATH"
fi

#######Back up and copy horizon code##################################
mv $HORIZON_PATH/horizon/templates/horizon/_scripts.html $HORIZON_PATH/horizon/templates/horizon/_scripts.html-bak
cp horizon/templates/horizon/_scripts.html   $HORIZON_PATH/horizon/templates/horizon/_scripts.html
cp horizon/js/horizon.sds.js   $HORIZON_PATH/horizon/static/horizon/js/

######Copy sds.py/vsm.py#######################
mv $HORIZON_PATH/openstack_dashboard/api/__init__.py $HORIZON_PATH/openstack_dashboard/api/__init__.py-bak
cp openstack_dashboard/api/*.py   $HORIZON_PATH/openstack_dashboard/api

######Back up and copy dashboard.py#######################
mv $HORIZON_PATH/openstack_dashboard/dashboards/admin/dashboard.py $HORIZON_PATH/openstack_dashboard/dashboards/admin/dashboard.py-bak
cp openstack_dashboard/dashboards/admin/dashboard.py $HORIZON_PATH/openstack_dashboard/dashboards/admin/dashboard.py

######Copy sds related folder for dashboard##############
cp -r openstack_dashboard/dashboards/admin/composepool  $HORIZON_PATH/openstack_dashboard/dashboards/admin
cp -r openstack_dashboard/dashboards/admin/discovercapability  $HORIZON_PATH/openstack_dashboard/dashboards/admin
cp -r openstack_dashboard/dashboards/admin/provisioning  $HORIZON_PATH/openstack_dashboard/dashboards/admin

###### Copy static folder for dashboard ######
if [ -e $STATIC_PATH/static/dashboard/js/json2.js ]
then
    mv $STATIC_PATH/static/dashboard/js/json2.js $STATIC_PATH/static/dashboard/js/json2.js-bak
fi
cp openstack_dashboard/static/dashboard/js/json2.js $STATIC_PATH/static/dashboard/js/
if [ -e $STATIC_PATH/static/dashboard/js/servermgmt.js ]
then
    mv $STATIC_PATH/static/dashboard/js/servermgmt.js $STATIC_PATH/static/dashboard/js/servermgmt.js-bak
fi
cp openstack_dashboard/static/dashboard/js/servermgmt.js $STATIC_PATH/static/dashboard/js/

chown -R $USER_ID:$USER_ID $HORIZON_PATH/openstack_dashboard/dashboards/admin
chown -R $USER_ID:$USER_ID $HORIZON_PATH/horizon/static/horizon/js/
chown -R $USER_ID:$USER_ID $HORIZON_PATH/horizon/templates/horizon
chown -R $USER_ID:$USER_ID $STATIC_PATH/static/dashboard/js/

#####Restart apache service############
sudo service apache2 restart

