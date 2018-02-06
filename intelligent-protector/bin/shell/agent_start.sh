#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

LOG_USER=$LOGNAME
if [ "root" = "${LOG_USER}" ]
then
    echo "You can not execute this script with user root."
    exit 1
fi

#use AGENT_ROOT env param
AGENT_ROOT_PATH="${AGENT_ROOT}"

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/agent_start.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

CheckProcess()
{
    ps -lu ${LOGNAME} | grep -v grep | grep $1 1>/dev/null 2>&1
    if [ $? -ne 0 ]
    then
        Log "INFO: Process $1 of OceanStor BCManager Agent is not exist."
        return 1
    fi

    Log "INFO: Process $1 of OceanStor BCManager Agent is is already exist."
    return 0
}

StartNginx()
{
    CheckProcess rdnginx
    if [ $? -ne 0 ]
    then
        cd "${AGENT_ROOT_PATH}/bin/nginx"
        OLD_UMASK=`umask`
        umask 077
        ./rdnginx
        umask ${OLD_UMASK}
        if [ $? -eq 0 ]
        then
            Log "INFO: Process nginx of OceanStor BCManager Agent is start successfully."
            return 0
        else
            Log "ERROR: Process nginx of OceanStor BCManager Agent start failed."
            return 1
        fi
    fi
    
    Log "INFO: Process nginx of OceanStor BCManager Agent has already started."
    return 0
}

StartAgent()
{
    CheckProcess rdagent
    if [ $? -ne 0 ]
    then
        nohup "${AGENT_ROOT_PATH}/bin/rdagent" 1>/dev/null 2>&1 &
        if [ $? -eq 0 ]
        then
            Log "INFO: Process rdagent of OceanStor BCManager Agent start successfully."
            return 0
        else
            Log "ERROR: Process rdagent of OceanStor BCManager Agent start failed."
            return 1
        fi
    fi
    
    Log "INFO: Process rdagent of OceanStor BCManager Agent has already started."
    return 0
}

StartMonitor()
{
    CheckProcess monitor
    if [ $? -ne 0 ]
    then
        nohup "${AGENT_ROOT_PATH}/bin/monitor" 1>/dev/null 2>&1 &
        if [ $? -eq 0 ]
        then
            Log "INFO: Process monitor of OceanStor BCManager Agent start successfully."
            return 0
        else
            Log "ERROR: Process monitor of OceanStor BCManager Agent start failed."
            return 1
        fi
    fi
    
    Log "INFO: Process monitor of OceanStor BCManager Agent has already started."
    return 0
}

GetProcessPid()
{
    if [ ${sysName} = "AIX" ]
    then
        RDAGENT_PID=`ps -u ${LOGNAME} | grep -v grep | grep $1 | $MYAWK '{print $2}'`
    else
        RDAGENT_PID=`ps -u ${LOGNAME} | grep -v grep | grep $1 | $MYAWK '{print $1}'`
    fi
    if [ $? -ne 0 ]
    then
        echo "Query $1 pid in user ${LOGNAME} failed."
        exit $?
    fi
    
    echo ${RDAGENT_PID}>"${AGENT_ROOT_PATH}/log/$1.pid"
    chmod 600 "${AGENT_ROOT_PATH}/log/$1.pid"
}

if [ $# = 1 ]
then
    if [ "$1" = "rdagent" ]
    then
        StartAgent  
        if [ $? -ne 0 ]
        then
            echo "Process agent of OceanStor BCManager Agent was started failed."
        else
            GetProcessPid rdagent
        fi
        exit $?
    elif [ "$1" = "nginx" ]
    then
        StartNginx  
        if [ $? -ne 0 ]
        then
            echo "Process nginx of OceanStor BCManager Agent was started failed."
        fi
        exit $?
    elif [ "$1" = "monitor" ]
    then
        StartMonitor  
        if [ $? -ne 0 ]
        then
            echo "Process monitor of OceanStor BCManager Agent was started failed."
        else
            GetProcessPid monitor
        fi
        exit $?
    else
        echo "Invalid parameter."
        exit 1
    fi    
fi        

${AGENT_ROOT_PATH}/bin/agentcli startnginx 1>/dev/null 2>&1
if [ $? -ne 0 ]
then
    echo "Process nginx of OceanStor BCManager Agent was started failed."
    exit 1
fi

StartAgent
if [ $? -ne 0 ]
then
    echo "Process agent of OceanStor BCManager Agent was started failed."
    exit 1    
fi

GetProcessPid rdagent

StartMonitor
if [ $? -ne 0 ]
then
    echo "Process monitor of OceanStor BCManager Agent was started failed."
    exit 1   
fi

GetProcessPid monitor
echo "OceanStor BCManager Agent was started successfully."
Log "OceanStor BCManager Agent start successfully."
exit 0

