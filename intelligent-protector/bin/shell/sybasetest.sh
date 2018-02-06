#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

export LANG=C

AGENT_ROOT_PATH=$1
PID=$2

. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/sybasefunc.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/sybasetest.log"

#for param file
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

#for GetValue
ArgFile=$$

#for sybase env file
ENV_SCRIPT_PATH=""

# sybase temp file
SYB_IN_TMP_PATH="${TMP_PATH}/SYB_IN_TMP_PATH${PID}.txt"
SYB_OUT_TMP_PATH="${TMP_PATH}/SYB_OUT_TMP_PATH${PID}.txt"

ConnectionDB()
{
    DeleteFile "${SYB_IN_TMP_PATH}"
    echo exit >> ${SYB_IN_TMP_PATH}
    Log "$EXEC_COMMAND -U$DB_USER -S$INSTANCE_NAME  -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}"
    $EXEC_COMMAND -U$DB_USER -P$DB_USERPWD -S$INSTANCE_NAME -X -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}>> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        cat ${SYB_OUT_TMP_PATH} | grep "Msg 4002"
        if [ $? -eq 0 ]
        then
            Log "ERROR:The username or password of db is not correct."
            DeleteFile "${SYB_IN_TMP_PATH}"
            DeleteFile "${SYB_OUT_TMP_PATH}"
            exit $ERROR_DB_USERPWD_WRONG
        fi
        Log "Connect $INSTANCE_NAME by user $DB_USER failed on test."
        DeleteFile "${SYB_IN_TMP_PATH}"
        DeleteFile "${SYB_OUT_TMP_PATH}"
        return 1
    fi
    DeleteFile "${SYB_IN_TMP_PATH}"
    DeleteFile "${SYB_OUT_TMP_PATH}"
    return 0
}

TestDBConnect()
{
    Log "Start to test sybase database connect."
    CheckInstanceExits
    if [ $? -eq 1 ]
    then
        exit 1
    fi

    CheckInstanceStart
    if [ $? -eq 1 ]
    then
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

GetValue "${INPUTINFO}" INSTNAME
INSTANCE_NAME=$ArgValue

GetValue "${INPUTINFO}" DBNAME
DB_NAME=$ArgValue

GetValue "${INPUTINFO}" DBUSERNAME
DB_USER=$ArgValue

GetValue "${INPUTINFO}" DBPASSWORD
DB_USERPWD=$ArgValue

GetValue "${INPUTINFO}" OPERTYPE
EXEC_TYPE=$ArgValue

Log "Get input param, instance name:${INSTANCE_NAME}, dbname:${DB_NAME}, user name:${DB_USER}, check type:${EXEC_TYPE}."

GetSybaseEnvScriptPath
if [ $? -eq 1 ]
then
    exit 1
fi
source "${ENV_SCRIPT_PATH}/SYBASE.sh"

SetSybaseExecCommand

TestDBConnect
    



