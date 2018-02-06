#!/bin/sh
MAIN_PATH=`pwd`

umount /home/dirCom
mount -t cifs -o username=Administrator,password=Huawei@123 //10.183.52.105/codebin/compile /home/dirCom
if [ $? -ne 0 ]
then
    echo "ERROR:mount //10.183.191.190/codebin/win_src to /home/dirCom failed!"
    exit 1
fi

umount /home/dirCompile
mount -t cifs -o username=Administrator,password=Huawei@123 //10.183.192.206/RDV2R1C00_AgentPackage_Codebin /home/dirCompile
if [ $? -ne 0 ]
then
    echo "ERROR:mount //10.183.192.206/RDV2R1C00_Package to /home/dirCompile failed!"
    exit 1
fi

cd /home/dirCompile
dest_file=`find ./ | grep Agent.zip` 
cp -r "$dest_file" /home/dirCom
cd ..

umount /home/dirCom
umount /home/dirCompile
