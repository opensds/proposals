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
LOG_FILE_NAME="${LOG_PATH}/hanarecover.log"

#hana temp file
HANA_IN_TMP_PATH="${TMP_PATH}/HANA_IN_TMP_PATH${PID}.txt"
HANA_OUT_TMP_PATH="${TMP_PATH}/HANA_OUT_TMP_PATH${PID}.txt"

#for GetValue
ArgFile=$$

#for param file
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

#for GetValue
ArgFile=$$

StartDatabase()
{
    Log "Begin start database."
    
    #Restore the database using a database snapshot, the log file will be purged after recovery, and the database reverts to the last point in time of the snapshot
    su - "${DB_SID}adm" -c "HDBSettings.sh recoverSys.py --command=\"recover data using snapshot CLEAR LOG\"" >> "${HANA_OUT_TMP_PATH}" 2>&1
    cat ${HANA_OUT_TMP_PATH} >> "${LOG_FILE_NAME}" 2>&1
    cat ${HANA_OUT_TMP_PATH} | grep -w "recoverSys failed"
    if [ $? -eq 0 ]
    then
        Log "Restore System by snapshot failed,try to start database."
    fi
    DeleteFile ${HANA_OUT_TMP_PATH}
    
    su - "${DB_SID}adm" -c "HDB start" >> "${LOG_FILE_NAME}" 2>&1
    
    CheckInstanceStart
    if [ $? -eq 0 ]
    then
        Log "Start instance:${DB_SID} successful,end start database."
        exit 0
    fi
    
    Log "Start instance:${DB_SID} failed,end start database."
    exit 1
}

StopDatabase()
{
    Log "Begin stop database"
    
    su - "${DB_SID}adm" -c "HDB stop" >> "${LOG_FILE_NAME}" 2>&1
    
    CheckInstanceStart
    if [ $? -eq 1 ]
    then
        Log "shutdown instance:${DB_SID} successful,end stop database."
        exit 0
    fi
    Log "shutdown instance:${DB_SID} failed,end stop database"
    exit 1
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

