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
LOG_FILE_NAME="${LOG_PATH}/sybaserecover.log"

# sybase temp file
SYB_IN_TMP_PATH="${TMP_PATH}/SYB_IN_TMP_PATH${PID}.txt"
SYB_OUT_TMP_PATH="${TMP_PATH}/SYB_OUT_TMP_PATH${PID}.txt"

#for param file
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

#for GetValue
ArgFile=$$

#for sybase env file
ENV_SCRIPT_PATH=""

#for sybase start
START_FILE_NAME=""
START_FILE_PATH=""

GetSybaseStartPath()
{
    local START_FILE_PATH_TMP=""
    local CURREN_PATH=`pwd`
    cd "$SYBASE/$SYBASE_ASE/"
    START_FILE_PATH_TMP=`find -name startserver`
    START_FILE_PATH=$(cd "$(dirname "${START_FILE_PATH_TMP}")" && pwd)
    cd "${CURREN_PATH}"
}

GetStartFile()
{
    local START_FILE_NAME_TMP=""
    local CURREN_PATH=`pwd`
    cd "$SYBASE/$SYBASE_ASE/"
    START_FILE_NAME_TMP=`find -name showserver`
    START_FILE_NAME_TMP=$(cd "$(dirname "${START_FILE_NAME_TMP}")" && pwd)
    START_FILE_NAME="${START_FILE_NAME_TMP}/RUN_${INSTANCE_NAME}"
    cd "${CURREN_PATH}"
}

StartDatabase()
{
    Log "Begin start database."
    local CURREN_PATH=`pwd`
    local START_TIMEOUT=0
    cd "${START_FILE_PATH}"
    if [ $? -ne 0 ]
    then
        Log "Go to start path error!"
        cd "${CURREN_PATH}"
        echo ${ERROR_SCRIPT_EXEC_FAILED} >> "${RESULT_FILE}"
        exit 0
    fi
    
    ./startserver -f ${START_FILE_NAME} >> "${LOG_FILE_NAME}"
    if [ $? -ne 0 ]
    then
        Log "Execute start failed."
        cd "${CURREN_PATH}"
        exit 1
    fi
    cd "${CURREN_PATH}"
    
    while [ 1 ]
    do
        sleep 10
        CheckInstanceStart
        if [ $? -eq 0 ]
        then
            Log "Start instance:${INSTANCE_NAME} successful,end start database."
            exit 0
        fi
        if [ $START_TIMEOUT -eq 10 ]
        then
            Log "Start instance:${INSTANCE_NAME} failed,end start database."
            exit 1
        fi
        START_TIMEOUT=`expr $START_TIMEOUT + 5`
    done
}

StopDatabase()
{
    Log "Begin stop database"
    DeleteFile "${SYB_IN_TMP_PATH}"
    echo shutdown >> ${SYB_IN_TMP_PATH}
    echo go >> ${SYB_IN_TMP_PATH}
    Log "$EXEC_COMMAND -U$DB_USER -S$INSTANCE_NAME  -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}" 
    $EXEC_COMMAND -U$DB_USER -P$DB_USERPWD -S$INSTANCE_NAME -X -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}>> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Log "Connect $INSTANCE_NAME by user $DB_USER failed on test."
        DeleteFile "${SYB_IN_TMP_PATH}"
        DeleteFile "${SYB_OUT_TMP_PATH}"
        exit 1
    else
        cat "${SYB_OUT_TMP_PATH}" | grep "Server SHUTDOWN by request" >>/dev/null 2>&1
        if [ $? -ne 0 ]
        then
            DeleteFile "${SYB_IN_TMP_PATH}"
            DeleteFile "${SYB_OUT_TMP_PATH}"
            Log "shutdown instance:${INSTANCE_NAME} failed"
            exit 1
        fi
    fi
    DeleteFile "${SYB_IN_TMP_PATH}"
    DeleteFile "${SYB_OUT_TMP_PATH}"
    Log "shutdown instance:${INSTANCE_NAME} successful,end stop database"
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

GetSybaseStartPath

GetStartFile

CheckInstanceExits
if [ $? -eq 1 ]
then
    exit 1 
fi

#*******************check service if start 0:start 1:stop ***************
if [ ${EXEC_TYPE} -eq 0 ]
then
    CheckInstanceStart
    if [ $? -eq 0 ]
    then
        Log "Instance:${INSTANCE_NAME} is already started."
        exit 0
    else
        StartDatabase
    fi
elif [ ${EXEC_TYPE} -eq 1 ]
then
    CheckInstanceStart
    if [ $? -eq 1 ]
    then
        Log "Instance:${INSTANCE_NAME} is already stopped."
        exit 0
    else
        StopDatabase
    fi
else
    Log "EXEC_TYPE is wrong"
    exit 1
fi

