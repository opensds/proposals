#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#--------------------------------------------

LOG_USER=${LOGNAME}
if [ "root" != "${LOG_USER}" ]
then
    echo "Please execute this script with user root."
    exit 1
fi

TRY_NUM=0
UNIX_CMD=
ENV_FILE=".bashrc"
ENV_EFFECT="export"
ENV_FLAG="="
ROCKY4_FLAG=0
AGENT_USER=rdadmin
AWK=awk

sysName=`uname`
if [ "SunOS" = "${sysName}" ]
then
    AWK=nawk
fi

if [ "${sysName}" = "Linux" ]
then
    SysRockyFlag=`cat /etc/issue | grep 'Linx'`
fi

if [ "${sysName}" = "Linux" ]
then
    rocky4=`cat /etc/issue | grep 'Rocky'`
    if [ -n "${rocky4}" ]
    then
        ROCKY4_FLAG=1
    fi
fi

RDADMIN_HOME_PATH=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F ':' '{print $6}'`

if [ "${sysName}" = "AIX" ] || [ "${sysName}" = "HP-UX" ] || [ 1 -eq ${ROCKY4_FLAG} ]
then
    SHELL_TYPE=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F "/" '{print $NF}'`
    if [ "${SHELL_TYPE}" = "bash" ]
    then
        UNIX_CMD=-l
    fi
fi

AGENT_ROOT_PATH=${RDADMIN_HOME_PATH}/Agent

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/agent_uninstall.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

RDADMIN_HOME_PATH_LEN=`echo $RDADMIN_HOME_PATH | $AWK '{print length()}'`
SLASH_FLAG=`RDsubstr $RDADMIN_HOME_PATH $RDADMIN_HOME_PATH_LEN 1`

if [ "$SLASH_FLAG" = "/" ]
then
    AGENT_ROOT_PATH="${RDADMIN_HOME_PATH}Agent"
fi


DeleteAutoStart()
{
    RM_LINENO=""
    if [ -f $1 ]
    then
        RM_LINENO=`cat $1 | grep -n -w "su - ${AGENT_USER} ${UNIX_CMD} -c" | grep -w "${AGENT_ROOT_PATH}/bin/monitor" | head -n 1 | $AWK -F ':' '{print $1}'`
        if [ "${RM_LINENO}" != "" ]
        then
            sed -i "${RM_LINENO}d" "$1"
            DeleteAutoStart $1
        fi
    fi
}

GetInput()
{
    set IFLAG=1
    
    echo "Are you sure you want to uninstall OceanStor BCManager Agent? (y/n):"
    PrintPrompt; read UNINSTALL
    
    if [ "${UNINSTALL}" != "y" ] && [ "${UNINSTALL}" != "n" ]
    then
        TRY_NUM=`expr ${TRY_NUM} + 1`
        if [ ${TRY_NUM} -lt 3 ]
        then
            echo Please enter y or n.
            GetInput
        else
            echo "Input invalid value over 3 times."
            echo "The uninstallation of OceanStor BCManager Agent will be stopped."
            sleep 1
            exit 1
        fi
    fi
    
    if [ "${UNINSTALL}" != "y" ]
    then
        echo "The uninstallation of OceanStor BCManager Agent will be stopped."
        sleep 1
        exit 1
    fi
}

DeteleENV()
{
    SHELL_TYPE=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F "/" '{print $NF}'`
    if [ "${SHELL_TYPE}" = "bash" ]
    then
        if [ "${SysRockyFlag}" != "" ] || [ "${sysName}" = "AIX" ] || [ "${sysName}" = "HP-UX" ] || [ "${sysName}" = "SunOS" ]
        then
            ENV_FILE=".profile"
        fi
    elif [ "${SHELL_TYPE}" = "ksh" ]
    then
        ENV_FILE=".profile"
    elif [ "${SHELL_TYPE}" = "csh" ]
    then
        ENV_FILE=".cshrc"
        ENV_EFFECT="setenv"
        ENV_FLAG=" "
    fi
    
    RDADMIN_ENV_FILE="${RDADMIN_HOME_PATH}/${ENV_FILE}"
    AGENT_ROOT_LINENO=`cat "${RDADMIN_ENV_FILE}" | grep -n "${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}" | $AWK -F ':' '{print $1}'`

    if [ "${AGENT_ROOT_LINENO}" != "" ]
    then
        sed  "${AGENT_ROOT_LINENO}d" "${RDADMIN_ENV_FILE}" > "${RDADMIN_ENV_FILE}".bak
        mv "${RDADMIN_ENV_FILE}".bak "${RDADMIN_ENV_FILE}"
        
    fi
    
    if [ "AIX" = "${sysName}" ]
    then
        LIBPATH_LINENO=`cat "${RDADMIN_ENV_FILE}" | grep -n "${ENV_EFFECT} LIBPATH${ENV_FLAG}" | $AWK -F ':' '{print $1}'`
        if [ "${LIBPATH_LINENO}" != "" ]
        then
            sed  "${LIBPATH_LINENO}d" "${RDADMIN_ENV_FILE}" > "${RDADMIN_ENV_FILE}".bak
            mv "${RDADMIN_ENV_FILE}".bak "${RDADMIN_ENV_FILE}"
        fi
    else
        LD_LIBRARY_PATH_LINENO=`cat "${RDADMIN_ENV_FILE}" | grep -n "${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}" | $AWK -F ':' '{print $1}'`
        if [ "${LD_LIBRARY_PATH_LINENO}" != "" ]
        then
            sed  "${LD_LIBRARY_PATH_LINENO}d" "${RDADMIN_ENV_FILE}" > "${RDADMIN_ENV_FILE}".bak
            mv "${RDADMIN_ENV_FILE}".bak "${RDADMIN_ENV_FILE}"
        fi
    fi
    
    Log "Environment variables of ceanStor BCManager Agent was deleted successfully."
}

if [ "$1" != "-q" ] && [ "$1" != "-r" ]
then
    echo "You are about to uninstall the OceanStor BCManager Agent. This operation stops the OceanStor BCManager Agent service and deletes the OceanStor BCManager Agent and customized configuration data which cannot be recovered. Therefore, applications on the host are no longer protected."
    echo ""
    echo "Suggestion: Confirm whether the customized configuration data, such as customized script, has been backed up."
    echo ""
    GetInput
fi

echo "Stop OceanStor BCManager Agent..."
su - ${AGENT_USER} 1>/dev/null ${UNIX_CMD} -c "\"${AGENT_ROOT_PATH}/bin/agent_stop.sh\""
if [ $? -ne 0 ]
then
    Log "OceanStor BCManager Agent was uninstalled failed."
    sleep 1
    exit 1
fi
    
sleep 1

echo "Delete auto start configure file of OceanStor BCManager Agent..."
if [ "${sysName}" = "Linux" ]
then
    SysRockyFlag=`cat /etc/issue | grep 'Linx'`
    
    if [ -f /etc/SuSE-release ]
    then
        DeleteAutoStart "/etc/rc.d/boot.local"
    elif [ -f /etc/redhat-release ]
    then
        DeleteAutoStart "/etc/rc.d/rc.local"
    elif [ "${SysRockyFlag}" != "" ]
    then
        DeleteAutoStart "/etc/rc.local"
    else
        Log "Unsupport OS."
    fi
elif [ "AIX" = "$sysName" ]
then
    if [ -f /etc/rc_rdagent.local ]
    then
        rm /etc/rc_rdagent.local
    fi
       
    if [ -f /etc/inittab ]
    then
        lsitab -a | grep /etc/rc_rdagent.local >/dev/null 2>&1
        if [ $? -eq 0 ]
        then
            rmitab  rdagent:2:once:/etc/rc_rdagent.local
            init q
        fi
    fi
elif [ "HP-UX" = "$sysName" ]
then
    START_SCRIPT=/sbin/init.d/AgentStart
    AGENT_CONF=/etc/rc.config.d/Agentconf
    SCRIPT=/sbin/rc3.d/S99AgentStart
    
    rm -rf ${SCRIPT}
    rm -rf ${START_SCRIPT}
    rm -rf ${AGENT_CONF}
elif [ "SunOS" = "$sysName" ]
then
    START_SCRIPT=/etc/init.d/agentstart
    SCRIPT=/etc/rc2.d/S99agentstart
    
    rm -rf ${SCRIPT}
    rm -rf ${START_SCRIPT}
else
    Log "Unsupport OS."
fi

echo "Delete environment variables of OceanStor BCManager Agent..."
DeteleENV

echo "OceanStor BCManager Agent was uninstalled successfully."
echo "Please remove the installation folders of OceanStor BCManager Agent."
Log "OceanStor BCManager Agent was uninstalled successfully."
sleep 1
exit 0
