#!/bin/sh
OSTYPE=`uname`
if [ "${OSTYPE}" = "SunOS" ]
then
    export ICP_ROOT=/export/home/icp/Agent
else
    export ICP_ROOT=/home/icp/Agent
fi

cd ../../../../
CUR_PATH=`pwd`
export PKG_PATH=${CUR_PATH}/Package

[ ! -d "${PKG_PATH}" ] && mkdir "${PKG_PATH}"

sudo umount /mnt
sudo mount -t cifs -o username=Administrator,password=Huawei@123 //10.183.243.153/RDV2R1C10_AgentPackage_App /mnt
if [ $? -ne 0 ]
then
    echo "ERROR:mount //10.183.243.153/RDV2R1C10_AgentPackage_App to /mnt failed!"
    exit 1
fi

PKG_VERSION=`cat /home/icp/Agent/V2R1C10VersionNo.txt`

rm ${PKG_PATH}/*
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-AIX53-ppc_64.tar.gz"  ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-AIX-ppc_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-HP-UX_11.23-ia_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-HP-UX_11.31-ia_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-iSoft3-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-OL5-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-OL6-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-RedHat5-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-RedHat6-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-RedHat7-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-Rocky4-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-Rocky6-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-Solaris-sparc_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-SuSE10-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-SuSE11-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-SuSE12-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-WIN64.zip" ${PKG_PATH}/

cd ${PKG_PATH}
rm "${CUR_PATH}/OceanStor BCManager ${PKG_VERSION}_eReplication_Agent.zip"
zip "OceanStor BCManager ${PKG_VERSION}_eReplication_Agent.zip" *.tar.gz *.zip
# copy to release pkg
cp "OceanStor BCManager ${PKG_VERSION}_eReplication_Agent.zip" /mnt/
# copy to from upload vmp
mv "OceanStor BCManager ${PKG_VERSION}_eReplication_Agent.zip" ../

sudo umount /mnt
