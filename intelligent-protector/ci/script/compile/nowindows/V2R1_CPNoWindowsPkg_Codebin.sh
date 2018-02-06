#!/bin/sh
OSTYPE=`uname`
if [ "${OSTYPE}" = "SunOS" ]
then
    ICP_ROOT=/export/home/icp/Agent
else
    ICP_ROOT=/home/icp/Agent
fi
PKG_PATH=${ICP_ROOT}/RD_V200R001C00_Agent_Codebin/code/current/AGENT_PACK_TEMP
export PKG_PATH ICP_ROOT

PKG_VERSION=`cat V2R1C00VersionNo.txt`

if [ "${OSTYPE}" = "Linux" ]
then
    REDHAT7=0
    ISREDHAT=`lsb_release -a | grep "Distributor ID" | grep "RedHat" | wc -l`
    if [ "${ISREDHAT}" = "1" ]
    then
        VERSION=`lsb_release -a | grep "Release" | grep "7.0" | wc -l`
        if [ "${VERSION}" = "1" ] 
        then
            REDHAT7=1
        fi
    fi
    
    if [ "${REDHAT7}" -eq "1" ]
    then
ftp -n<<!
open 10.183.192.206 50021
user icp huawei#123
binary
hash
cd pkg
lcd ${PKG_PATH}
prompt
mput *x86_64.tar.gz
close
bye
!
    else
        MOUNT_PATH=/mnt
        umount ${MOUNT_PATH}
        mount -t cifs -o username=Administrator,password=Huawei@123 //10.183.192.206/RDV2R1C00_AgentPackage_Codebin ${MOUNT_PATH}
        if [ $? -ne 0 ]
        then
            echo "ERROR:mount //10.183.192.206/RDV2R1C00_AgentPackage_Codebin to ${MOUNT_PATH} failed!"
            exit 1
        fi

        PKG_NAME=`ls ${PKG_PATH} | grep "x86_64.tar.gz"`
        cp "${PKG_PATH}/${PKG_NAME}" ${MOUNT_PATH}/pkg/
        umount ${MOUNT_PATH}
    fi
else
ftp -n<<!
open 10.183.192.206 50021
user icp huawei#123
binary
hash
cd pkg
lcd ${PKG_PATH}
prompt
mput *64.tar.gz
close
bye
!
fi