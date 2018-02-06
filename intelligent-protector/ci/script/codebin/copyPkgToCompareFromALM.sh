#!/bin/sh
MAIN_PATH=`pwd`

umount /home/dirALM
mount -t cifs -o username=Administrator,password=Huawei@123 //10.183.52.105/codebin/alm /home/dirALM
if [ $? -ne 0 ]
then
    echo "ERROR:mount //10.183.52.105/codebin/win_dest to /mnt failed!"
    exit 1
fi

cd /home/compareCode
dest_file=`find ./ | grep Agent.zip` 
cp -r "$dest_file" /home/dirALM

umount /home/dirALM
