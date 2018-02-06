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
LOG_FILE_NAME="${LOG_PATH}/hanaconsistent.log"

# hana temp file
HANA_IN_TMP_PATH="${TMP_PATH}/HANA_IN_TMP_PATH${PID}.txt"
HANA_OUT_TMP_PATH="${TMP_PATH}/HANA_OUT_TMP_PATH${PID}.txt"

#for param file
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

#for GetValue
ArgFile=$$

#for freeze status.0:already freeze 1:already thaw
FREEZESTAT=""

#for freeze and thaw database
BACKUP_ID=""

function GetDataFile()
{
    if [ -f "${RESULT_FILE}.tmp" ]
    then
        rm "${RESULT_FILE}.tmp"
    fi
    
    touch "${RESULT_FILE}.tmp"
    local DB_SID_TMP=`echo ${DB_SID} | tr '[:lower:]' '[:upper:]'`
    local DbDataFilePath=`cat /usr/sap/$DB_SID_TMP/SYS/global/hdb/custom/config/global.ini 2>/dev/null | grep "basepath_datavolumes" | sed 's/\(.*\)[ ]*\(=\)[ ]*\(.*\)/\1\2\3/g' | ${MYAWK} -F "=" '{print $2}'`
    if [ ! -d "$DbDataFilePath" ]; then
        Log "The datavolume path of $DB_SID_TMP is not correct, start database failed."
        exit 1
    fi  
    
    find $DbDataFilePath -name "datavolume_*.dat" >> "${RESULT_FILE}.tmp" 2>/dev/null
    if [ $? -ne 0 ]
    then
        Log "ERROR:find data file failed."
        exit 1
    fi
    
    find $DbDataFilePath -name "snapshot_databackup_*" >> "${RESULT_FILE}.tmp" 2>/dev/null
    if [ $? -ne 0 ]
    then
        Log "ERROR:find snapshot file failed."
        exit 1
    fi
    
    cat "${RESULT_FILE}.tmp" |sort|uniq > "${RESULT_FILE}"
    cat "${RESULT_FILE}" >> "$LOG_FILE_NAME"
    rm "${RESULT_FILE}.tmp"
    Log "Get data file successful"
}

function QueryFreezeStat()
{
    Log "Begin query database freeze status"
    DeleteFile "${HANA_IN_TMP_PATH}"
    echo "select BACKUP_ID from M_BACKUP_CATALOG WHERE ENTRY_TYPE_NAME = 'data snapshot' and STATE_NAME = 'prepared'">> ${HANA_IN_TMP_PATH}
    Log "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -d ${DB_SID} -u ${DB_USER}" 

    su - "${DB_SID}adm" -c "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -I ${HANA_IN_TMP_PATH} -o ${HANA_OUT_TMP_PATH} -d ${DB_SID} -u ${DB_USER} -p ${DB_USERPWD}" >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Log "Excute hdbsql failed"
        return 1
    fi
    
    BACKUP_ID=`sed -n "2p" ${HANA_OUT_TMP_PATH}`
    if [ "${BACKUP_ID}" = "" ]
    then
        DeleteFile "${HANA_IN_TMP_PATH}"
        DeleteFile "${HANA_OUT_TMP_PATH}"
        Log "End query database freeze status,datbase:$DB_SID is not freeze."
        FREEZESTAT=1
    else
        DeleteFile "${HANA_IN_TMP_PATH}"
        DeleteFile "${HANA_OUT_TMP_PATH}"
        Log "End query database freeze status,datbase:$DB_SID is freeze."
        FREEZESTAT=0
    fi
    return 0
}

FreezeDatabase()
{
    Log "Begin freeze database"
    DeleteFile "${HANA_IN_TMP_PATH}"
    echo "BACKUP DATA CREATE SNAPSHOT">> ${HANA_IN_TMP_PATH}
    Log "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -d ${DB_SID} -u ${DB_USER}" 

    su - "${DB_SID}adm" -c "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -I ${HANA_IN_TMP_PATH} -d ${DB_SID} -u ${DB_USER} -p ${DB_USERPWD}" >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Log "Excute hdbsql failed"
        return 1
    fi
    
    GetDataFile
    
    QueryFreezeStat
    if [ ${FREEZESTAT} -eq 1 ]
    then
        DeleteFile "${HANA_IN_TMP_PATH}"
        Log "Database ${DB_SID} freeze failed"
        return 1
    fi
    
    DeleteFile "${HANA_IN_TMP_PATH}"
    Log "Freeze Database ${DB_SID} successful,end freeze database"
    return 0
}

TrawDatabase()
{
    local THAW_TIME=0
    Log "Begin thaw database"
    DeleteFile "${HANA_IN_TMP_PATH}"
    echo "BACKUP DATA CLOSE SNAPSHOT BACKUP_ID ${BACKUP_ID} SUCCESSFUL 'USE THIS'">> ${HANA_IN_TMP_PATH}
    Log "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -d ${DB_SID} -u ${DB_USER}" 

    su - "${DB_SID}adm" -c "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -I ${HANA_IN_TMP_PATH} -d ${DB_SID} -u ${DB_USER} -p ${DB_USERPWD}" >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Log "Excute hdbsql failed"
        return 1
    fi
    
    QueryFreezeStat
    if [ ${FREEZESTAT} -eq 0 ]
    then
        while [ 1 ]
        do
            su - "${DB_SID}adm" -c "hdbsql -i ${INSTANCE_NUMBER} -n localhost:3${INSTANCE_NUMBER}15 -I ${HANA_IN_TMP_PATH} -d ${DB_SID} -u ${DB_USER} -p ${DB_USERPWD}" >> "${LOG_FILE_NAME}" 2>&1
            if [ $? -ne 0 ]
            then
                Log "Excute hdbsql failed"
            fi
            
            QueryFreezeStat
            if [ ${FREEZESTAT} -eq 1 ]
            then
                DeleteFile "${HANA_IN_TMP_PATH}"
                Log "Thaw Database ${DB_SID} successful,end thaw database"
                return 0
            fi
            
            if [ ${THAW_TIME} -eq 3 ]
            then
                DeleteFile "${HANA_IN_TMP_PATH}"
                Log "Database ${DB_SID} unfreeze failed"
                return 1
            fi
            THAW_TIME=`expr ${THAW_TIME} + 1`
            sleep 2
        done
    fi
    
    DeleteFile "${HANA_IN_TMP_PATH}"
    Log "Thaw Database ${DB_SID} successful,end thaw database"
    return 0
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

CheckInstanceStart
if [ $? -eq 1 ]
then
    Log "DATABASE_NAME:${DB_SID} is not started."
    exit ${ERROR_INSTANCE_NOSTART}
fi

#*******************check service if freeze 0:freeze 1:thaw 2:query***************
if [ ${EXEC_TYPE} -eq 2 ]
then
    QueryFreezeStat
    if [ $? -ne 0 ]
    then 
        Log "Query database freeze status failed"
        exit 1
    else
        echo ${FREEZESTAT} >> "${RESULT_FILE}"
        exit 0
    fi
elif [ ${EXEC_TYPE} -eq 0 ]
then
    QueryFreezeStat
    if [ ${FREEZESTAT} -eq 0 ]
    then
        Log "Database ${DB_SID} is already freeze"
        exit 1
    fi
    
    FreezeDatabase
    RET_NUM=$?
    if [ ${RET_NUM} -ne 0 ]
    then
        exit ${RET_NUM}
    fi
elif [ ${EXEC_TYPE} -eq 1 ]
then
        
    QueryFreezeStat 
    if [ ${FREEZESTAT} -eq 1 ]
    then
        Log "Database ${DB_SID} is already thaw"
        exit 1
    fi
    
    TrawDatabase
    RET_NUM=$?
    if [ ${RET_NUM} -ne 0 ]
    then
        exit ${RET_NUM}
    fi
else
    Log "EXEC_TYPE is wrong"
    exit 1
fi


