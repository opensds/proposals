#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

AGENT_ROOT_PATH=$1
PID=$2

. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/hanafunc.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/hanatest.log"

# hana temp file
HANA_IN_TMP_PATH="${TMP_PATH}/HANA_IN_TMP_PATH${PID}.txt"
HANA_OUT_TMP_PATH="${TMP_PATH}/HANA_OUT_TMP_PATH${PID}.txt"

#for GetValue
ArgFile=$$

#for param file
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

function ConnectionDB()
{
    DeleteFile "${HANA_IN_TMP_PATH}"
    echo "\s" >> ${HANA_IN_TMP_PATH}
    Log "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -d ${DB_SID} -u ${DB_USER}"
    
    su - "${DB_SID}adm" -c "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -I ${HANA_IN_TMP_PATH} -o ${HANA_OUT_TMP_PATH} -d ${DB_SID} -u ${DB_USER} -p ${DB_USERPWD}"
    if [ $? -ne 0 ]
    then
        cat ${HANA_OUT_TMP_PATH} | grep "SQLSTATE: 28000"
        if [ $? -eq 0 ]
        then
            Log "ERROR:The username or password of db is not correct."
            DeleteFile "${HANA_IN_TMP_PATH}"
            DeleteFile "${HANA_OUT_TMP_PATH}"
            exit $ERROR_DB_USERPWD_WRONG
        fi
        DeleteFile "${HANA_IN_TMP_PATH}"
        DeleteFile "${HANA_OUT_TMP_PATH}"
        Log "Connect ${DB_SID} by user ${DB_USER} failed on test."
        return 1
    fi    

    DeleteFile "${HANA_IN_TMP_PATH}"
    DeleteFile "${HANA_OUT_TMP_PATH}"
    Log "Connect ${DB_SID} by user ${DB_USER} successful on test."
    return 0
}

function TestDBConnect()
{
    Log "Start to test sybase database connect."

    CheckInstanceStart
    if [ $? -eq 1 ]
    then
        Log "Instance:${DB_SID} is not start."
        exit ${ERROR_INSTANCE_NOSTART}
    fi
    
    ConnectionDB
    if [ $? -eq 1 ]
    then
        exit 1
    fi
    
    Log "End test sybase database connect."
    exit 0
}

INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"

GetValue "${INPUTINFO}" INSTNUM
INSTANCE_NUMBER=$ArgValue

GetValue "${INPUTINFO}" DBNAME
DB_SID=`echo ${ArgValue} | tr '[:upper:]' '[:lower:]'`

GetValue "${INPUTINFO}" DBUSERNAME
DB_USER=$ArgValue

GetValue "${INPUTINFO}" DBPASSWORD
DB_USERPWD=$ArgValue

GetValue "${INPUTINFO}" OPERTYPE
EXEC_TYPE=$ArgValue

Log "Get input param, instance number:${INSTANCE_NUMBER}, db SID:${DB_SID}, user name:${DB_USER}, check type:${EXEC_TYPE}."
TestDBConnect
