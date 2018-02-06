#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#for log
LOG_FILE_NAME="${LOG_PATH}/sybasefunc.log"

#for sybase home path
SYB_HOME_PATH="${AGENT_ROOT_PATH}/conf/SYBHOME"

#for GetSybaseVersion
SYB_VERSION=
SYBVESION="${TMP_PATH}/SYBV${PID}.txt"

#for tmp file
SID_TMP_PATH="${TMP_PATH}/SID_TMP_PATH${PID}.txt"

INSTANCE_NAME=""
DB_NAME=""
DB_USER=""
DB_USERPWD=""
EXEC_TYPE=""
RET_NUM=""
EXEC_COMMAND="isql"

CheckInstanceStart()
{
    local CURR_PATH=`pwd`
    local SHOW_FILE_PATH=""
    local SHOW_FILE_PATH_TMP=""
    DeleteFile "${SID_TMP_PATH}"
    cd "$SYBASE/$SYBASE_ASE/"
    SHOW_FILE_PATH_TMP=`find -name showserver`
    SHOW_FILE_PATH=$(cd "$(dirname "${SHOW_FILE_PATH_TMP}")" && pwd)
    cd "${SHOW_FILE_PATH}"
    ./showserver >>"${SID_TMP_PATH}"
    RET_NUM=`cat ${SID_TMP_PATH} | grep -w "\-s${INSTANCE_NAME}" | grep "dataserver" | wc -l`
    if [ ${RET_NUM} -eq 0 ]
    then
        Log "Instance:${INSTANCE_NAME} is not started."
        return 1
    fi
    DeleteFile "${SID_TMP_PATH}"
    cd "${CURR_PATH}"
    return 0
}

CheckInstanceExits()
{
    INTERFACES_PATH="$SYBASE/interfaces"
    RET_NUM=`cat "$INTERFACES_PATH" | grep -w "$INSTANCE_NAME" | wc -l`
    if [ ${RET_NUM} -eq 0 ]
    then
        Log "Instance:$INSTANCE_NAME is not exit."
        return 1
    fi 
    
    return 0
}

AnalyseErrorCode()
{
    local ERROR_NUM
    
    ERROR_NUM=`cat $1 | grep "Msg 4002"| wc -l 2>> "${LOG_FILE_NAME}"` 
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "The password or user name wrong"
        return $ERROR_DB_USERPWD_WRONG
    fi
    
    ERROR_NUM=`cat $1 | grep "Msg 2232"| wc -l 2>> "${LOG_FILE_NAME}"`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "The param input is wrong"
        return $ERROR_SYBASE_PARAM_WRONG
    fi
    
    return 0
}

GetSybaseVersion()
{
    Log "Begin get sybase vesion!"
    SYB_VERSION="--"

    DeleteFile "${SYBVESION}"
    touch "${SYBVESION}"
    chmod 666 "${SYBVESION}"
    $SYBASE/$SYBASE_ASE/bin/dataserver -v >> "${SYBVESION}"
    
    Log "get version from sybase dataserver."
    SYB_VERSION=`cat "${SYBVESION}" | grep "Adaptive Server Enterprise" | $MYAWK -F "/" '{print $2}'`
    DeleteFile "${SYBVESION}"

    if [ "${SYB_VERSION}" = "" ]
    then
        SYB_VERSION="--"
    fi
    
    Log "End get sybase version($SYB_VERSION)."
}

GetSybaseEnvScriptPath()
{
    local ENV_SCRIPT_PATH_TMP
    local CURRENT_PATH=`pwd`
    if [ -f ${SYB_HOME_PATH} ]
    then
        ENV_SCRIPT_PATH=`cat ${SYB_HOME_PATH} | $MYAWK -F "=" '{print $2}'`
        return 0
    fi
    cd "/opt/"
    ENV_SCRIPT_PATH_TMP=`find -name SYBASE.sh | sed -n 1p`
    if [ "${ENV_SCRIPT_PATH_TMP}" = "" ]
    then
        cd "/home/"
        ENV_SCRIPT_PATH_TMP=`find -name SYBASE.sh | sed -n 1p`
        if [ "${ENV_SCRIPT_PATH_TMP}" = "" ]
        then
            cd "/"
            ENV_SCRIPT_PATH_TMP=`find -name SYBASE.sh | sed -n 1p`
        fi
    fi
    
    if [ "${ENV_SCRIPT_PATH_TMP}" = "" ]
    then
        Log "Get SYBASE.sh path failed."
        cd "${CURRENT_PATH}"
        return 1
    fi
    
    ENV_SCRIPT_PATH=$(cd "$(dirname "${ENV_SCRIPT_PATH_TMP}")" && pwd)
    touch ${SYB_HOME_PATH}
    chmod 600 ${SYB_HOME_PATH}
    chown root:root ${SYB_HOME_PATH}
    echo "SYB_HOME=${ENV_SCRIPT_PATH}" >${SYB_HOME_PATH}
    cd "${CURRENT_PATH}"
    return 0
}
SetSybaseExecCommand()
{
    isql --help
    if [ $? -ne 0 ]
    then
        EXEC_COMMAND=isql64
    fi
}
