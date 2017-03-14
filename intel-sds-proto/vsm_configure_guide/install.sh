#!/bin/bash

set -e
set -o xtrace

TOPDIR=`pwd`
OS=Fedora
if [[ -x "`which lsb_release 2>/dev/null`" ]]; then
    OS=$(lsb_release -si)
fi

if [ ! -f $TOPDIR/deploy.ini ]; then
    echo "deploy.ini missing, exit!"
    exit 
fi

command -v pip > /dev/null 2>&1 || { echo >&2 "I require pip but it's not installed.  Aborting."; exit 1; }

if [ -f $TOPDIR/keystone/deploy.ini ]; then
    rm $TOPDIR/keystone/deploy.ini
fi

cp $TOPDIR/deploy.ini $TOPDIR/keystone

if [ -d $TOPDIR/vsm_controller_node ]; then
    rm -rf $TOPDIR/vsm_controller_node
fi

mkdir $TOPDIR/vsm_controller_node
cp -r $TOPDIR/packages/vsm_keystone_update/* $TOPDIR/vsm_controller_node
cp $TOPDIR/deploy.ini $TOPDIR/vsm_controller_node
cp $TOPDIR/vsm_controller_config.sh  $TOPDIR/vsm_controller_node
pip freeze |grep ^python-keystoneclient= > $TOPDIR/vsm_controller_node/pip.inst
pip freeze |grep ^keystonemiddleware= >> $TOPDIR/vsm_controller_node/pip.inst
pip freeze |grep ^six= >> $TOPDIR/vsm_controller_node/pip.inst
pip freeze |grep ^kombu= >> $TOPDIR/vsm_controller_node/pip.inst

##############Configurate the keystone of VSM######################

####Install vsmclient###########################
cd $TOPDIR
cd packages/vsmclient/python-vsmclient
sudo python setup.py install

####Install Ceph###############################
if [[ "$OS" == 'Ubuntu' ]]; then
    sudo apt-get update
    sudo apt-get -y install ceph ceph-common ceph-mds libcephfs1 librados-dev python-ceph
else
    sudo yum -y install ceph ceph-common libcephfs1 python-ceph
fi


####Run keystone_vsm script############
cd $TOPDIR/keystone
bash keystone_vsm.sh

