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
LOG_FILE_NAME="${LOG_PATH}/sybaseconsistent.log"

# sybase temp file
SYB_IN_TMP_PATH="${TMP_PATH}/SYB_IN_TMP_PATH${PID}.txt"
SYB_OUT_TMP_PATH="${TMP_PATH}/SYB_OUT_TMP_PATH${PID}.txt"

#for param file
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

#for GetValue
ArgFile=$$

#for sybase env file
ENV_SCRIPT_PATH=""

#for freeze status.0:already freeze 1:already thaw
FREEZESTAT=""

#for query database freeze status
SINGLE_DB_NAME=""
LOOP_NUM=""

FreezeDatabase()
{
    Log "Begin freeze database"
    DeleteFile "${SYB_IN_TMP_PATH}"
    echo quiesce database $INSTANCE_NAME hold $DB_NAME>> ${SYB_IN_TMP_PATH}
    echo go >> ${SYB_IN_TMP_PATH}
    echo exit >> ${SYB_IN_TMP_PATH}
    Log "$EXEC_COMMAND -U$DB_USER -S$INSTANCE_NAME  -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}" 
    $EXEC_COMMAND -U$DB_USER -P$DB_USERPWD -S$INSTANCE_NAME -X -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}>> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        DeleteFile "${SYB_IN_TMP_PATH}"
        DeleteFile "${SYB_OUT_TMP_PATH}"
        Log "Connect $DB_NAME by user $DB_USER failed on freeze."
        return 1
    else
        AnalyseErrorCode ${SYB_OUT_TMP_PATH}
        RET_NUM=$?
        if [ $RET_NUM -ne 0 ]
        then
            DeleteFile "${SYB_IN_TMP_PATH}"
            DeleteFile "${SYB_OUT_TMP_PATH}"
            return $RET_NUM
        fi
        
        if [ `cat ${SYB_OUT_TMP_PATH} | wc -l` -eq 0 ]
        then
            DeleteFile "${SYB_OUT_TMP_PATH}"
            Log "Freeze $DB_NAME by user $DB_USER successful."
        else
            DeleteFile "${SYB_IN_TMP_PATH}"
            DeleteFile "${SYB_OUT_TMP_PATH}"
            Log "Quiesce database $DB_NAME hold failed."
            return 1
        fi
    fi

    DeleteFile "${SYB_IN_TMP_PATH}"
    
    Log "End freeze database"
    return 0
}

TrawDatabase()
{
    Log "Begin thaw database"
    DeleteFile "${SYB_IN_TMP_PATH}"
    echo quiesce database $INSTANCE_NAME release>> ${SYB_IN_TMP_PATH}
    echo go >> ${SYB_IN_TMP_PATH}
    echo exit >> ${SYB_IN_TMP_PATH}
    Log "$EXEC_COMMAND -U$DB_USER -S$INSTANCE_NAME -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}" 
    $EXEC_COMMAND -U$DB_USER -P$DB_USERPWD -S$INSTANCE_NAME -X -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}>> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        DeleteFile "${SYB_IN_TMP_PATH}"
        DeleteFile "${SYB_OUT_TMP_PATH}"
        Log "Connect $DB_NAME by user $DB_USER failed on thaw."
        return 1
    else
        AnalyseErrorCode ${SYB_OUT_TMP_PATH}
        RET_NUM=$?
        if [ $RET_NUM -ne 0 ]
        then
            DeleteFile "${SYB_IN_TMP_PATH}"
            DeleteFile "${SYB_OUT_TMP_PATH}"
            return $RET_NUM
        fi
        
        if [ `cat ${SYB_OUT_TMP_PATH} | wc -l` -eq 0 ]
        then
            DeleteFile "${SYB_OUT_TMP_PATH}"
            Log "Thaw $DB_NAME by user $DB_USER successful."
        else
            DeleteFile "${SYB_IN_TMP_PATH}"
            DeleteFile "${SYB_OUT_TMP_PATH}"
            Log "Quiesce database $DB_NAME release failed."
            return 1
        fi
    fi
    
    DeleteFile "${SYB_IN_TMP_PATH}"
    
    Log "End thaw database"
    return 0
}

QueryFreezeStat()
{
    local DB_NAME_TMP=""
    local INPUT_DB_NAME=""
    Log "Begin query database freeze status"
    DeleteFile "${SYB_IN_TMP_PATH}"
    DB_NAME_TMP=`echo $1 | $MYAWK -F "," '{for(x=1;x<=NF;x++) print $x}'`
    for INPUT_DB_NAME in $DB_NAME_TMP
    do
        echo sp_helpdb $INPUT_DB_NAME>> ${SYB_IN_TMP_PATH}
        echo go >> ${SYB_IN_TMP_PATH}
        echo exit >> ${SYB_IN_TMP_PATH}
        Log "$EXEC_COMMAND -U$DB_USER -S$INSTANCE_NAME -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}" 
        $EXEC_COMMAND -U$DB_USER -P$DB_USERPWD -S$INSTANCE_NAME -X -w200 -i${SYB_IN_TMP_PATH} -o${SYB_OUT_TMP_PATH}>> "${LOG_FILE_NAME}" 2>&1
        if [ $? -ne 0 ]
        then
            DeleteFile "${SYB_IN_TMP_PATH}"
            DeleteFile "${SYB_OUT_TMP_PATH}"
            Log "Connect $1 by user $DB_USER failed on query."
            exit 1
        else
            AnalyseErrorCode ${SYB_OUT_TMP_PATH}
            RET_NUM=$?
            if [ $RET_NUM -ne 0 ]
            then
                DeleteFile "${SYB_IN_TMP_PATH}"
                DeleteFile "${SYB_OUT_TMP_PATH}"
                return $RET_NUM
            fi
    
            if [ `cat ${SYB_OUT_TMP_PATH} | wc -l` -eq 0 ]
            then
                DeleteFile "${SYB_IN_TMP_PATH}"
                DeleteFile "${SYB_OUT_TMP_PATH}"
                Log "sp_helpdb $INPUT_DB_NAME by user $DB_USER failed on query."
                exit 1
            else
                #若下发查询多个数据，则遍历查询，由于sybase的解冻操作是基于实例而言的，一旦冻结后就无法再次下发冻结命令
                #所以若多个数据库中存在一个数据库为冻结状态，则直接返回状态为冻结。
                cat ${SYB_OUT_TMP_PATH} | grep "quiesce database">>/dev/null 2>&1
                if [ $? -eq 0 ]
                then
                    FREEZESTAT=0
                    DeleteFile "${SYB_IN_TMP_PATH}"
                    DeleteFile "${SYB_OUT_TMP_PATH}"
                    Log "End query database freeze status,datbase:$INPUT_DB_NAME is freeze."
                    return 0
                else
                    DeleteFile "${SYB_IN_TMP_PATH}"
                    DeleteFile "${SYB_OUT_TMP_PATH}"
                    FREEZESTAT=1
                fi
            fi
        fi
    done

    Log "End query database freeze status"
    return 0
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

#*******************check service if freeze 0:freeze 1:thaw 2:query***************
if [ ${EXEC_TYPE} -eq 2 ]
then
    QueryFreezeStat $DB_NAME
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
    LOOP_NUM=1
    while [ 1 ]
    do
        SINGLE_DB_NAME=`echo $DB_NAME | $MYAWK -F "," -v NUM=$LOOP_NUM '{print $NUM}'`
        if [ "$SINGLE_DB_NAME" = "" ]
        then
            break
        fi
        
        QueryFreezeStat $SINGLE_DB_NAME
        if [ ${FREEZESTAT} -eq 0 ]
        then
            Log "Database ${DB_NAME} is already quiesce"
            exit 1
        fi
        LOOP_NUM=`expr $LOOP_NUM + 1`
    done
    FreezeDatabase
    RET_NUM=$?
    if [ ${RET_NUM} -ne 0 ]
    then
        exit ${RET_NUM}
    fi
elif [ ${EXEC_TYPE} -eq 1 ]
then
    LOOP_NUM=1
    while [ 1 ]
    do
        SINGLE_DB_NAME=`echo $DB_NAME | $MYAWK -F "," -v NUM=$LOOP_NUM '{print $NUM}'`
        if [ "$SINGLE_DB_NAME" = "" ]
        then
            Log "Database ${DB_NAME} is already release"
            exit 1
        fi
        
        QueryFreezeStat $SINGLE_DB_NAME
        if [ ${FREEZESTAT} -eq 0 ]
        then
            break
        fi
        LOOP_NUM=`expr $LOOP_NUM + 1`
    done
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


