#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#for log
LOG_FILE_NAME="${LOG_PATH}/hanafunc.log"

#for GetSybaseVersion
HANA_VERSION=
HANAVESION="${TMP_PATH}/SYBV${PID}.txt"

INSTANCE_NUMBER=""
DB_SID=""
DB_USER=""
DB_USERPWD=""
EXEC_TYPE=""
RET_NUM=""

CheckInstanceExist()
{
    cat /etc/passwd | grep -w "${DB_SID}adm" >> /dev/null 2>&1
    if [ $? -ne 0 ]
    then
        Log "User ${DB_SID}adm is not exist"
        return 1
    fi
    
    return 0 
}

CheckInstanceStart()
{
    CheckInstanceExist
    if [ $? -ne 0 ]
    then
        return 1
    fi 

    Log " su - ${DB_SID}adm -c sapcontrol -nr ${INSTANCE_NUMBER} -function GetProcessList"
    su - "${DB_SID}adm" -c "sapcontrol -nr ${INSTANCE_NUMBER} -function GetProcessList " >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 3 ]
    then
        Log "Instance:${DB_SID} is not start."
        return 1
    fi
    Log "Instance:${DB_SID} is started."
    return 0
}

