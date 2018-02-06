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

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/agent_stop.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

WILD_PROC_LIST_KILL="monitor rdagent nginx"

GetPidsByName()
{
    PROC_NAME="$1"
    if [ "${sysName}" = "AIX" ]
    then
        PROC_IDS=`ps -u ${LOGNAME} | grep -v grep | grep $PROC_NAME | $MYAWK '{print $2}'`
    else
        PROC_IDS=`ps -u ${LOGNAME} | grep -v grep | grep $PROC_NAME | $MYAWK '{print $1}'`
    fi

    RESULT=""
    for PROC_ID in ${PROC_IDS}
    do
        #check specified PID
        TEST_RET=`ps -p  ${PROC_ID} | grep ${PROC_ID}`
        if [ "*${TEST_RET}" != "*" ]; then
            RESULT="${RESULT} ${PROC_ID}"
        fi
    done
    
    echo ${RESULT}
}

#kill -9 all processed now
KillProcessByNameForce()
{
    PROC_LIST="$1"
    
    for PROC_NAME in $PROC_LIST
    do
        if [ "nginx" = "${PROC_NAME}" ]
        then
            if [ -f "${AGENT_ROOT_PATH}/bin/nginx/logs/nginx.pid" ]
            then
                NGINX_PPID=`cat ${AGENT_ROOT_PATH}/bin/nginx/logs/nginx.pid`
            fi
            #check nginx exist or not
            if [ "*" != "*${NGINX_PPID}" ]
            then
                NGINX_EXIST=`ps -u ${LOGNAME} -ef | grep ${NGINX_PPID} | grep -v grep`
                if [ "*" != "*${NGINX_EXIST}" ]
                then
                    #kill The parent process of nginx
                    kill -9 ${NGINX_PPID}
                    echo "Send SIGKILL to $PROC_NAME, call: kill -9 ${NGINX_PPID}"
                    Log "INFO: Send SIGKILL to parent $PROC_NAME, call: kill -9 ${NGINX_PPID}"
                fi
            fi
        fi
        
        PROC_ID=`GetPidsByName $PROC_NAME`
        
        if [ "*$PROC_ID" = "*" ]; then
            continue
        fi
        
        echo "Send SIGKILL to $PROC_NAME, call: kill -9 $PROC_ID"
        Log "INFO: Send SIGKILL to $PROC_NAME, call: kill -9 $PROC_ID"
        kill -9 $PROC_ID
    done
}

if [ $# = 1 ]
then
    KillProcessByNameForce "$1"
else
    KillProcessByNameForce "${WILD_PROC_LIST_KILL}"
fi
echo "OceanStor BCManager Agent was stopped successfully."
Log "OceanStor BCManager Agent was stopped successfully."
