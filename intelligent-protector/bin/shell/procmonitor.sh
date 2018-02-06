#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#@dest: monitor child process for a program
#@date: 2009-05-20
#@authr: 

USAGE="./ProcMonitor.sh PPID MONITORNAME TIMEOUT AGENT_ROOT"

MYPPID=$1
MYPID=$$
MONITORPID=
MONITORNAME=$2
TIMEOUT=$3
AGENT_ROOT_PATH="$4"
BINPATH="${AGENT_ROOT_PATH}/bin"
TMPPATH="${AGENT_ROOT_PATH}/tmp"

NeedLogFlg=1
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/procmonitor.log";

OSNAME=`uname`
#Get monitor process ID whose parent process ID is same.
GetMONITORPID()
{
    MONITORPID=""
    if [ "$OSNAME" = "AIX" ]
    then
        #-----ps -fT-----
        ProcNameStr=`ps -fT ${MYPPID} | grep $MONITORNAME | grep -v grep`
        if [ "$ProcNameStr" = "" ]
        then
            return 0
        fi
        
        #demo root  6881348  3735560   0   Dec 05      -  0:00    |\--/usr/sbin/rsct/bin/IBM.AuditRMd
        MONITORPID=`echo $ProcNameStr | awk '{print $2}'`
    elif [ "$OSNAME" = "HP-UX" ]
    then
        #-----ptree-------
        ProcNameStr=`ptree -s ${MYPPID} | grep $MONITORNAME | grep -v grep`
        if [ "$ProcNameStr" = "" ]
        then
            return 0
        fi
        
        #demo 1543 /usr/sbin/inetd
        MONITORPID=`echo $ProcNameStr | awk '{print $1}'`
    elif [ "$OSNAME" = "Linux" ]
    then
        #-----pstree-----
        ProcNameStr=`pstree -p ${MYPPID} | grep $MONITORNAME | grep -v grep`
        if [ "$ProcNameStr" = "" ]
        then
            return 0
        fi

        #demo: `-su(14790)---bash(14791)---sqlplus(14815)---oracle_14816_db(14816)
        MONITORPID=`echo $ProcNameStr | sed -r "s/.*$MONITORNAME\((\w*)\).*/\1/"`
    else
        Log "[$MYPPID] not support OS type {$OSNAME}."
        exit 1
    fi
}

#Check parent process whether exist
IsPPIDExist()
{
    ALLPPID=`ps -aef | grep -v grep | grep $MYPPID | awk '{print $2}'`
    if [ "$ALLPPID" = "" ]
    then
        return 0
    fi

    for TMPPPID in $ALLPPID
    do
        if [ "$TMPPPID" = "$MYPPID" ]
        then
            return 1
        fi
    done

    return 0
}

if [ "$MYPPID" = "" ]
then
    exit 1
fi

i=0
while [ $i -lt $TIMEOUT ]
do
    sleep 1

    GetMONITORPID
    if [ "$MONITORPID" = "" ]
    then
        IsPPIDExist
        if [ $? -eq 0 ]
        then
            exit 0
        fi
    fi

    i=`expr $i + 1`
done

if [ "$MONITORPID" != "" ]
then
    Log "kill process id=${MONITORPID}."
    kill -9 $MONITORPID
fi

exit 0
