#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#--------------------------------------------
# $1: -r for push upgrade
#--------------------------------------------

LOG_USER=${LOGNAME}
if [ "root" != "${LOG_USER}" ]
then
    echo "Please execute this script with user root."
    exit 1
fi

#1 for V1R3, 2 for V1R5 and new
UPGRADE_TYPE=0
CUR_VERSION_VR=
UNIX_CMD=
FREE_SPACE_MIN=512000
AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
TRY_NUM=0
AWK=awk
UPGRADE_FLAG=-r
INPUT_PARAM=$1

# -------------error code----------------------
ERR_INPUT_PARAM_ERR=10
ERR_DISK_FREE_ISLESS_500MB=13
ERR_INSTALLATION_PATH_EXIST=15
ERR_CHECK_VSERSION_FAILED=20
ERR_CHECK_WORKING_USER_FAILED=21
ERR_AGENT_FILE_MIGRATE_FALIED=22
ERR_AGENT_STOP_FAILED=23
EER_AGENT_UNINSTALL_FAILED=24
ERR_REGISTER_SERVICE_FALIED=25
ERR_SET_CONF_FILE_FAILED=26
ERR_AGENT_START_FAILED=27
# -------------error code----------------------

if [ "${INPUT_PARAM}" != "" ]
then
    echo "Input param is error."
    exit $ERR_INPUT_PARAM_ERR
fi

sysName=`uname`
if [ "SunOS" = "${sysName}" ]
then
    AWK=nawk
fi

RDADMIN_HOME_PATH=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F ':' '{print $6}'`
RDADMIN_HOME_PATH_LEN=`echo $RDADMIN_HOME_PATH | $AWK '{print length()}'`

AGENT_BACKUP_PATH=${RDADMIN_HOME_PATH}/Agent_bak
AGENT_RM_PATH=${RDADMIN_HOME_PATH}/Agent_rm
AGENT_TMP_PATH=${RDADMIN_HOME_PATH}/install

AGENT_ROOT_OLD="/opt/Huawei/ReplicationDirector/Agent"

COM_AGENT_FILES="                    \
    bin/agent_func.sh                \
    bin/agent_install.sh             \
    bin/agent_start.sh               \
    bin/agent_stop.sh                \
    bin/agent_uninstall.sh           \
    bin/db2clusterinfo.sh            \
    bin/db2info.sh                   \
    bin/db2luninfo.sh                \
    bin/db2recover.sh                \
    bin/db2resourcegroup.sh          \
    bin/db2sample.sh                 \
    bin/initiator.sh                 \
    bin/oraasmaction.sh              \
    bin/oraclecheckarchive.sh        \
    bin/oracleclusterinfo.sh         \
    bin/oracleconsistent.sh          \
    bin/oracleinfo.sh                \
    bin/oracleluninfo.sh             \
    bin/oracleresourcegroup.sh       \
    bin/oracletest.sh                \
    bin/oradbaction.sh               \
    bin/procmonitor.sh               \
    bin/agentcli                     \
    bin/datamigration                \
    bin/getinput                     \
    bin/libcommon.so                 \
    bin/monitor                      \
    bin/rdagent                      \
    bin/rootexec                     \
    bin/xmlcfg                       \
    bin/plugins/libcluster-*.so      \
    bin/plugins/libdb2-*.so          \
    bin/plugins/libdevice-*.so       \
    bin/plugins/libhost-*.so         \
    bin/plugins/liboracle-*.so       \
    bin/nginx/conf/fastcgi_params    \
    bin/nginx/conf/nginx.conf        \
    bin/nginx/html/50x.html          \
    bin/nginx/html/index.html        \
    conf/agent_cfg.xml               \
    conf/pluginmgr.xml               \
    conf/script.sig                  \
    db/AgentDB.db                    \
    "
    
PRE_CONF_FILES="                     \
    conf/rdagent.ini                 \
    conf/rdagentSign.ini             \
    conf/rdagent.db                  \
    conf/server.pem                  \
    db/AgentDB.db                    \
    bin/agent_func.sh                \
    bin/rdagentService.sh            \
    bin/agent_func.sh                \
    bin/rdagentService.sh            \
    bin/rdmonitor                    \
    bin/ProcMonitor.sh               \
    bin/rdagent                      \
    bin/rdcrypto                     \
    bin/rdexec                       \
    "
 
SHELL_TYPE=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F "/" '{print $NF}'`
if [ "${SHELL_TYPE}" != "bash" ] && [ "${SHELL_TYPE}" != "ksh" ] && [ "${SHELL_TYPE}" != "csh" ]
then
    echo "The shell type bash, ksh and csh only be supported in user ${AGENT_USER} by OceanStor BCManager Agent."
    echo "The upgrade OceanStor BCManager Agent will be stopped."
    sleep 1
    exit 1
fi
 
 
if [ "${sysName}" = "AIX" ] || [ "${sysName}" = "HP-UX" ]
then
    if [ "${SHELL_TYPE}" = "bash" ]
    then
        UNIX_CMD=-l
    fi
fi

if [ -f /etc/linx-release ] #rocky4
then
     RockyVersion=rocky`cat /etc/linx-release|awk '{print $2}'|awk -F '.' '{print $1}'`
    if [ "$RockyVersion" = "rocky4" ] && [ "${SHELL_TYPE}" = "bash" ]
    then
        UNIX_CMD=-l
    fi
fi

GetInstallPath()
{
    CURRENT_PATH=`pwd`
    PARAM_PATH=`dirname "$1"`
    cd "${PARAM_PATH}"
    AGENT_INSTALL_PATH=`pwd`
    cd "${CURRENT_PATH}"
    
    return 0
}

GetInstallPath $0

AGENT_ROOT_PATH=`dirname "${AGENT_INSTALL_PATH}"`
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/agent_upgrade.log"
. "${AGENT_INSTALL_PATH}/agent_func.sh"

SLASH_FLAG=`RDsubstr ${RDADMIN_HOME_PATH} ${RDADMIN_HOME_PATH_LEN} 1`
if [ "${SLASH_FLAG}" = "/" ]
then
    AGENT_TMP_PATH=${RDADMIN_HOME_PATH}install
fi

PATH_CHECK=`echo "${AGENT_INSTALL_PATH}" | grep "${AGENT_TMP_PATH}"`
if [ "${PATH_CHECK}" = "" ]
then
    echo "Please put the upgrade installation package into path ${AGENT_TMP_PATH}."
    exit $ERR_INSTALLATION_PATH_EXIST
fi

AGENT_ROOT_PATH=${RDADMIN_HOME_PATH}/Agent

MakeSureAgentNoWorks()
{
    set IFLAG=1
    
    echo "Please make sure OceanStor BCManager Agent is not working now. (y/n):"
    PrintPrompt; read CONT_UP
    
    if [ "${CONT_UP}" != "y" ] && [ "${CONT_UP}" != "n" ]
    then
        TRY_NUM=`expr ${TRY_NUM} + 1`
        if [ ${TRY_NUM} -lt 3 ]
        then
            echo "Please enter y or n."
            MakeSureAgentNoWorks
        else
            echo "Input invalid value over 3 times."
            echo "The upgrade OceanStor BCManager Agent will be stopped."
            sleep 1
            exit 1
        fi
    fi
    
    if [ "${CONT_UP}" != "y" ]
    then
        echo "The upgrade OceanStor BCManager Agent will be stopped."
        sleep 1
        exit 1
    fi
}

#check disk space
CheckFreeDiskSpace()
{
    Log "Check free space of installation path."
    if [ "${sysName}" = "Linux" ]
    then
        FREE_SPACE=`df -k ${RDADMIN_HOME_PATH} | grep -n '' | $AWK 'END{print $4}'`
    elif [ "${sysName}" = "AIX" ]
    then
        FREE_SPACE=`df -k ${RDADMIN_HOME_PATH} | grep -n '' | $AWK '{print $3}' | sed -n '2p'`
    elif [ "${sysName}" = "HP-UX" ]
    then
        FREE_SPACE=`df -k ${RDADMIN_HOME_PATH} | grep -w 'free' | $AWK '{print $1}'`
    elif [ "${sysName}" = "SunOS" ]
    then
        FREE_SPACE=`df -k ${RDADMIN_HOME_PATH} | $AWK '{print $4}' | sed -n '2p'`
    fi
    
    if [ ${FREE_SPACE_MIN} -gt ${FREE_SPACE} ]
    then
        echo "The installation path free space ${FREE_SPACE} is less than the minimum space requirements ${FREE_SPACE_MIN}."
        Log "The installation path free space ${FREE_SPACE} is less than the minimum space requirements ${FREE_SPACE_MIN}, then exit 1."
        echo "The upgrade OceanStor BCManager Agent will be stopped."
        exit $ERR_DISK_FREE_ISLESS_500MB
    fi
}

#check version and agent port
CheckVersion()
{
    #Check Agent, which is ready to install 
    Log "Check version of previous OceanStor BCManager Agent."
    if [ ! -f ${AGENT_INSTALL_PATH}/agentcli ]
    then
        echo "File agentcli was missed in new OceanStor BCManager Agent."
        Log "agentcli was missed in new OceanStor BCManager Agent."
        echo "The upgrade OceanStor BCManager Agent will be stopped."
        sleep 1
        exit $ERR_CHECK_VSERSION_FAILED
    fi
    
    touch "${AGENT_INSTALL_PATH}/../log/rdagent.log"
    chown -R rdadmin:rdadmin "${AGENT_INSTALL_PATH}/../log/rdagent.log"
    NEW_VERSION=`su - rdadmin -c "${AGENT_INSTALL_PATH}/agentcli show 2>/dev/null" | grep Version | $AWK -F ': ' '{print $2}'` 
    if [ -z "$NEW_VERSION" ]
    then
        echo "Check Version of new OceanStor BCManager Agent failed."
        Log "Check Version of new OceanStor BCManager Agent failed."
        echo "The upgrade OceanStor BCManager Agent will be stopped."
        sleep 1
        exit $ERR_CHECK_VSERSION_FAILED
    fi
    
    Log "Get the version ${NEW_VERSION} of new OceanStor BCManager Agent."
    NEW_VER_VR=`echo ${NEW_VERSION} | $AWK -F '[VRCBSPT]' '{print $2$3}'`
    Log "New install version: ${NEW_VERSION}"
    echo "New install version: ${NEW_VERSION}"
    
    # Get Agent version,which was already installed
    OLD_VERSION=
    OLD_VER_VRC=
    OLD_VER_VRCSPC=
    ## Check V1R5 and later
    if [ -f ${AGENT_ROOT_PATH}/bin/agentcli ]
    then
        OLD_VERSION=`${AGENT_ROOT_PATH}/bin/agentcli show | grep Version | $AWK -F ': ' '{print $2}'`
        if [ "$OLD_VERSION" = "" ]; then
            OLD_VERSION=`su - rdadmin -c "${AGENT_ROOT_PATH}/bin/agentcli show 2>/dev/null" | grep Version | $AWK -F ': ' '{print $2}'`
        fi
        if [ "$OLD_VERSION" = "" ]
        then
            echo "Check Version of previous OceanStor BCManager Agent failed."
            Log "Check Version of previous OceanStor BCManager Agent failed."
            echo "The upgrade OceanStor BCManager Agent will be stopped."
            sleep 1
            exit $ERR_CHECK_VSERSION_FAILED
        fi
        UPGRADE_TYPE=2
        OLD_VER_VR=`echo ${OLD_VERSION} | $AWK -F '[VRCBSPT]' '{print $2$3}'`
        CUR_VERSION_VR=${OLD_VER_VR}
        Log "Get the version ${OLD_VERSION} of previous OceanStor BCManager Agent."
    else
        ## V1R5 is not exist.Check V1R3
        if [ "${sysName}" = "Linux" ]
        then
            V1R3_CHECK=`rpm -qa | grep ReplicationDirector-Agent`
        elif [ "${sysName}" = "AIX" ]
        then
            V1R3_CHECK=`lslpp -l | grep ReplicationDirector-Agent.rte`
        elif [ "${sysName}" = "HP-UX" ]
        then
            V1R3_CHECK=`swlist | grep ReplicationDirector-Agent`
        else
            echo "Unsupport OS."
            sleep 1
            exit $ERR_CHECK_VSERSION_FAILED
        fi
        
        if [ "${V1R3_CHECK}" != "" ]
        then
            if [ -f ${AGENT_ROOT_OLD}/bin/rdagent ]
            then
                OLD_VERSION_NUM=`${AGENT_ROOT_OLD}/bin/rdagent -v | grep 'Version:' | $AWK '{print $2}' | $AWK -F '[VRCBSP]' '{print $2$3$4}'`
                if [ $? -ne 0 ]
                then
                    echo "Check Version of previous OceanStor BCManager Agent failed."
                    Log "Check Version of previous OceanStor BCManager Agent failed."
                    echo "The upgrade OceanStor BCManager Agent will be stopped."
                    sleep 1
                    exit $ERR_CHECK_VSERSION_FAILED
                fi
                
                OLD_VERSION=`${AGENT_ROOT_OLD}/bin/rdagent -v | grep 'Version:' | $AWK '{print $2}'`
                if [ "${OLD_VERSION_NUM}" = "10000310" ]
                then
                    UPGRADE_TYPE=1
                    Log "Get the version ${OLD_VERSION} of previous OceanStor BCManager Agent."
                    
                    
                    #check agent port
                    AGENT_PORT=`"${AGENT_INSTALL_PATH}/xmlcfg" read System port`
                    Log "Get the agent port ${AGENT_PORT} of new OceanStor BCManager Agent."
                    
                    if [ ${AGENT_PORT} -gt 65535 -o ${AGENT_PORT} -le 1024 ]
                    then
                        echo "The agent port number should be more than 1024 and less than or equal to 65535."
                        echo "The upgrade OceanStor BCManager Agent will be stopped."
                        sleep 1
                        exit $ERR_CHECK_VSERSION_FAILED
                    fi
                    
                    Log "Check the agent port number ${AGENT_PORT} be used in this system."
                    
                    if [ "Linux" = "${sysName}" ]
                    then
                        PORTSTATUS=`lsof -i:${AGENT_PORT}`
                    else
                        PORTSTATUS=""
                        
                        if [ "HP-UX" = "${sysName}" ]
                        then
                            PORTS=`netstat -an | grep ${AGENT_PORT} | $AWK -F " " '{print $4}' | $AWK -F "." '{print $NF}'`
                        elif [ "AIX" = "${sysName}" ]
                        then
                            PORTS`netstat -an | grep ${AGENT_PORT} | $AWK -F '.' '{print $2}' | $AWK -F ' ' '{print $1}'`
                        fi
                        
                        for PORTEACH in ${PORTS}
                        do
                            if [ "${PORTEACH}" = "${AGENT_PORT}" ]
                            then
                                PORTSTATUS=${PORTEACH}
                                break
                            fi
                        done
                    fi
                    
                    if [ "${PORTSTATUS}" != "" ]
                    then 
                        echo "The port number ${AGENT_PORT} in agent_cfg.xml is used by other process, Please modify it."
                        Log "The port ${AGENT_PORT} in agent_cfg.xml is used by other process."
                        echo "The upgrade OceanStor BCManager Agent will be stopped."
                        sleep 1
                        exit $ERR_CHECK_VSERSION_FAILED
                    fi
                else
                    echo "Version ${OLD_VERSION} of OceanStor BCManager Agent was unsupported."
                    Log "Version ${OLD_VERSION} of OceanStor BCManager Agent was unsupported."
                    echo "The upgrade OceanStor BCManager Agent will be stopped."
                    sleep 1
                    exit $ERR_CHECK_VSERSION_FAILED
                fi
            else
                echo "File rdagent was missed in previous OceanStor BCManager Agent."
                Log "File rdagent was missed in previous OceanStor BCManager Agent."
                echo "The upgrade OceanStor BCManager Agent will be stopped."
                sleep 1
                exit $ERR_CHECK_VSERSION_FAILED
            fi
        else
            echo "NO supported OceanStor BCManager Agent to upgrade in this System."
            Log "NO supported OceanStor BCManager Agent to upgrade in this System."
            echo "The upgrade OceanStor BCManager Agent will be stopped."
            sleep 1
            exit $ERR_CHECK_VSERSION_FAILED
        fi
    fi
    
    Log "Current version: ${OLD_VERSION}"
    echo "Current version: ${OLD_VERSION}"
    OLD_VER_V=`echo ${OLD_VERSION} | $AWK -F '[VRCBSPT]' '{print $2}'`
    OLD_VER_VRC=`echo ${OLD_VERSION} | $AWK -F '[VRCBSPT]' '{print "V"$2"R"$3"C"$4}'`
    OLD_VER_VRCSPC=`echo ${OLD_VERSION} | $AWK -F 'SPC' '{print $2}'`
    # Adjust version in xml file
    if [ -n "${OLD_VER_VRCSPC}" ]
    then
        OLD_VER_VRCSPC="SPC${OLD_VER_VRCSPC}"
    fi
    echo OLD_VER_VRCSPC=${OLD_VER_VRCSPC}
    Log "OLD_VER_VRCSPC=${OLD_VER_VRCSPC}"
    
    SUPORT_VER=`"${AGENT_INSTALL_PATH}/xmlcfg" read Upgrade ${OLD_VER_VRC} version`
    #Can't get result of ${OLD_VER_VRC}
    if [ $? -ne 0  ]
    then
        echo "Can not get available Version(${OLD_VER_VRC}) to upgrade from list file(${SUPORT_VER})."
        Log "Can not get available Version(${OLD_VER_VRC}) to upgrade from list file(${SUPORT_VER})."
        echo "The upgrade OceanStor BCManager Agent will be stopped."
        sleep 1
        exit $ERR_CHECK_VSERSION_FAILED
    else #Get result of ${OLD_VER_VRC} success.
        if [ "${SUPORT_VER}" = "all" ]
        then
            echo "Current version is ${OLD_VERSION}, all versions in ${OLD_VERSION} are allowed to upgrade."
            Log "Current version is ${OLD_VERSION}, all versions in ${OLD_VERSION} are allowed to upgrade."
            return 0
        else
            CHECK_SPC_RET=1
            if [ -z "${OLD_VER_VRCSPC}" ]
            then
                echo "${SUPORT_VER}" | ${AWK} '{split($0,mon,"|");for(i in mon){if(mon[i] == ""){exit 0;}}exit 1}'
                CHECK_SPC_RET=$?
            else
                echo "${SUPORT_VER} ${OLD_VER_VRCSPC}" | ${AWK} '{split($1,mon,"|");for(i in mon){if(mon[i] == $2){exit 0;}}exit 1}'
                CHECK_SPC_RET=$?
            fi
            
            #SPC of version is exist
            if [ ${CHECK_SPC_RET} -eq 0 ]
            then 
                Log "Get available Version(${OLD_VER_VRCSPC}) to upgrade from list file(${SUPORT_VER})."
                return 0
            else
                Log "Can not get available Version((${OLD_VER_VRCSPC})) to upgrade from list file(${SUPORT_VER})."
                echo "The upgrade OceanStor BCManager Agent will be stopped."
                sleep 1
                exit $ERR_CHECK_VSERSION_FAILED
            fi
        fi
    fi

    echo "NO supported OceanStor BCManager Agent to upgrade in this System."
    Log "NO supported OceanStor BCManager Agent to upgrade in this System."
}

PrintFileMissed()
{
    local FILE_NAME=$1
    echo "Check file ${FILE_NAME} failed, which is missed in OceanStor BCManager Agent."
    Log "Check file ${FILE_NAME} failed, which is missed in OceanStor BCManager Agent."
    echo "The upgrade OceanStor BCManager Agent will be stopped."
    sleep 1
    exit $ERR_AGENT_FILE_MIGRATE_FALIED  
}

CheckPerFile()
{
    CHECK_FILE_NAME=$1
    SO_FLAG=`echo ${CHECK_FILE_NAME} | grep '*'`
    if [ "${SO_FLAG}" != "" ]
    then
        FULL_NAME=`basename ${CHECK_FILE_PATH}/${CHECK_FILE_NAME}`
        CHECK_FILE_NAME=bin/plugins/${FULL_NAME}
    fi  
    if [ ! -f "${CHECK_FILE_PATH}/${CHECK_FILE_NAME}" ]
    then
        if [ "$CHECK_FILE_NAME" = "bin/nginx/nginx" ]
        then 
            CHECK_FILE_NAME_RDNGINX="bin/nginx/rdnginx" 
            if [ ! -f "${CHECK_FILE_PATH}/${CHECK_FILE_NAME_RDNGINX}" ]
            then
                PrintFileMissed "$CHECK_FILE_NAME_RDNGINX"
            fi
            Log "Check file ${CHECK_FILE_NAME_RDNGINX} ....ok"
            return
        fi       
        PrintFileMissed "$CHECK_FILE_NAME"       
    else
        Log "Check file ${CHECK_FILE_NAME} ....ok"
    fi
}

CheckNewAgentFiles()
{
    CHECK_FILE_PATH=`dirname "${AGENT_INSTALL_PATH}"`
    for file in ${COM_AGENT_FILES}
    do
        CheckPerFile ${file}
    done
    
    # check file not in COM_AGENT_FILES, but in new version 
    CheckPerFile "bin/oraclecheckcdb.sh"
    CheckPerFile "bin/oraclepdbinfo.sh"
    CheckPerFile "bin/oraclepdbstart.sh"
    CheckPerFile "bin/nginx/rdnginx"
    
    # if upgrage from v2r1, check the server.crt and server.key at 'bin/nginx/conf'
    if [ "${NEW_VER_VR}" -eq "200001" ]
    then
        echo "Check point: NEW_VER_VR=${NEW_VER_VR}, check new Agent certification file."
        Log "Check point: NEW_VER_VR=${NEW_VER_VR}, check new Agent certification file."
        CheckPerFile "bin/nginx/conf/server.crt"
        CheckPerFile "bin/nginx/conf/server.key"
        CheckPerFile "bin/nginx/conf/bcmagentca.crt"
    fi
    
    Log "CHeck new OceanStor BCManager Agent files successfully."
}

CheckPreAgentFiles()
{
    if [ ${UPGRADE_TYPE} -eq 2 ]
    then
        CHECK_FILE_PATH=${AGENT_ROOT_PATH}
        for file in ${COM_AGENT_FILES}
        do
            CheckPerFile ${file}
        done
        
        # check file not in COM_AGENT_FILES, but in old version 
        CheckPerFile "bin/nginx/nginx"
        
        # if upgrage from v1r5, check the server.pem at 'bin/nginx/conf'
        if [ "${CUR_VERSION_VR}" -eq "100005" ]
        then
            echo "Check point: CUR_VERSION_VR=${CUR_VERSION_VR}, check Agent file server.pem."
            Log "Check point: CUR_VERSION_VR=${CUR_VERSION_VR}, check Agent file server.pem."
            CheckPerFile "bin/nginx/conf/server.pem"
        fi
        return 0
    fi
    
    CHECK_FILE_PATH=${AGENT_ROOT_OLD}
    for file in ${PRE_CONF_FILES}
    do
        CheckPerFile ${file}
    done
    
    Log "CHeck previous OceanStor BCManager Agent files successfully."
}

#stop agent
StopProcess()
{ 
    Log "Stop previous OceanStor BCManager Agent."
    if [ ${UPGRADE_TYPE} -eq 1 ]
    then        
        su - ${AGENT_USER} 1>/dev/null ${UNIX_CMD} -c "${AGENT_ROOT_OLD}/bin/rdagentService.sh stop"
        if [ $? -ne 0 ]
        then
            echo "The previous OceanStor BCManager Agent was stopped failed."
            Log "The previous OceanStor BCManager Agent was stopped failed."
            echo "The upgrade OceanStor BCManager Agent will be stopped."
            sleep 1
            exit $ERR_AGENT_STOP_FAILED
        fi
        
        return 0
    fi
    
    su - ${AGENT_USER} ${UNIX_CMD} -c "${AGENT_ROOT_PATH}/bin/agent_stop.sh" 2>&1
    if [ $? -ne 0 ]
    then
        echo "The previous OceanStor BCManager Agent was stopped failed."
        Log "The previous OceanStor BCManager Agent was stopped failed."
        echo "The upgrade OceanStor BCManager Agent will be stopped."
        sleep 1
        exit $ERR_AGENT_STOP_FAILED
    fi
    
    Log "The previous OceanStor BCManager Agent was stopped successfully."
    return 0
}

#back up agent
MoveAgentDir()
{
    rm -rf ${AGENT_RM_PATH}
    
    if [ -d ${AGENT_BACKUP_PATH} ]
    then
        mv ${AGENT_BACKUP_PATH} ${AGENT_RM_PATH}
    fi
    
    if [ ${UPGRADE_TYPE} -eq 2 ]
    then
        mv ${AGENT_ROOT_PATH} ${AGENT_BACKUP_PATH}
    fi
    
    if [ ${UPGRADE_TYPE} -eq 1 ]
    then
        mkdir ${AGENT_BACKUP_PATH}
        chown -R ${AGENT_USER} ${AGENT_BACKUP_PATH}
        chmod 775 ${AGENT_BACKUP_PATH}
        
        cp -r ${AGENT_ROOT_OLD}/* ${AGENT_BACKUP_PATH}
        
    fi
    
    if [ ! -d ${AGENT_ROOT_PATH} ]
    then
        mkdir ${AGENT_ROOT_PATH}
    fi
    
    chown -R ${AGENT_USER} ${AGENT_ROOT_PATH}
    chmod 775 ${AGENT_ROOT_PATH}
    
    cp -r ${AGENT_INSTALL_PATH}/../* ${AGENT_ROOT_PATH}
    
    rm -rf ${AGENT_ROOT_PATH}/"OceanStor BCManager "V*_Agent-*
    
    Log "Move new installation package to installation path ${AGENT_ROOT_PATH}."
}

MVV1R3Rdagentini()
{
    VIP_ADDR=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep IP | $AWK -F '= ' '{print $2}'`
    VNGINX_PORT=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep PORT | $AWK -F '= ' '{print $2}'`
    VKEY_PWD=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep KEY_PWD | $AWK -F '= ' '{print $2}'`
    
    sed "/listen/s/.*/        listen       ${VIP_ADDR}:${VNGINX_PORT} ssl;/g"  ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak
    rm -rf ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
    mv ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
    
    
    sed "/fastcgi_pass/s/.*/            fastcgi_pass   127.0.0.1:${AGENT_PORT};/g"  ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak
    rm -rf ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
    mv ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
    
    VLOG_LEVEL=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep LOG_LEVEL | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write System log_level ${VLOG_LEVEL}
    
    VLOG_COUNT=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep LOG_COUNT | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write System log_count ${VLOG_COUNT}
    
    VTHREADCNT=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep THREADCNT | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor rdagent thread_count ${VTHREADCNT}
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor nginx thread_count ${VTHREADCNT}
    
    VHANDLECNT=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep HANDLECNT | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor rdagent handle_count ${VHANDLECNT}
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor nginx handle_count ${VHANDLECNT}
    
    VPYHMEMSIZE=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep PYHMEMSIZE | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor rdagent pm_size ${VPYHMEMSIZE}
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor nginx pm_size ${VPYHMEMSIZE}
    
    VVIRMEMSIZE=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep VIRMEMSIZE | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor rdagent vm_size ${VVIRMEMSIZE}
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor nginx vm_size ${VVIRMEMSIZE}
    
    VCPUUSAGE=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep CPUUSAGE | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor rdagent cpu_usage ${VCPUUSAGE}
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor nginx cpu_usage ${VCPUUSAGE}
    
    VTMPFILETOTALSIZE=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep TMPFILETOTALSIZE | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor rdagent tmpfile_size ${VTMPFILETOTALSIZE}
    
    VBASERETRYTIMES=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep BASERETRYTIMES | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor retry_time ${VBASERETRYTIMES}
    
    VBASECYCTIME=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep BASECYCTIME | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write Monitor monitor_interval ${VBASECYCTIME}
    
    VSECURITYLEVEL=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep SECURITYLEVEL | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP security_level ${VSECURITYLEVEL}
    
    VCONTEXTNAME=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep CONTEXTNAME | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP context_name ${VCONTEXTNAME}
    
    VCONTEXTENGINEID=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep CONTEXTENGINEID | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP engine_id ${VCONTEXTENGINEID}
    
    VPRIVATEPWD=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep PRIVATEPWD | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP private_password ${VPRIVATEPWD}
    
    VAUTHPWD=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep AUTHPWD | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP auth_password ${VAUTHPWD}
    
    VAUTHPROTOCOL=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep AUTHPROTOCOL | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP auth_protocol ${VAUTHPROTOCOL}
    
    VPRIVATEPROTOCOL=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep PRIVATEPROTOCOL | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP private_protocol ${VPRIVATEPROTOCOL}
    
    VSECURITYNAME=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep SECURITYNAME | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP security_name ${VSECURITYNAME}
    
    VSECURITYMODEL=`cat ${AGENT_ROOT_OLD}/conf/rdagent.ini | grep SECURITYMODEL | $AWK -F '= ' '{print $2}'`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write SNMP security_model ${VSECURITYMODEL}
    
    Log "Move third scripts to new OceanStor BCManager Agent." 
}

SetV1R5Conf()
{
    INPUT_NAME=$1
    PARENT_TAG=$2
    CHILD_TAG=$3
    ARG_VALUE=`"${AGENT_BACKUP_PATH}/bin/xmlcfg" read ${PARENT_TAG} ${CHILD_TAG} ${INPUT_NAME}`
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write ${PARENT_TAG} ${CHILD_TAG} ${INPUT_NAME} ${ARG_VALUE}
    Log "Reconfig ${CHILD_TAG}-${PARENT_TAG}-${INPUT_NAME} in agent_cfg.xml ....ok"
}

GEV1R5Config()
{
    SetV1R5Conf port System
    SetV1R5Conf log_level System
    SetV1R5Conf log_count System
    SetV1R5Conf name System
    SetV1R5Conf sl System
    SetV1R5Conf hash System
    
    SetV1R5Conf retry_time Monitor
    SetV1R5Conf monitor_interval Monitor
    SetV1R5Conf rdagent Monitor
    SetV1R5Conf thread_count Monitor rdagent
    SetV1R5Conf handle_count Monitor rdagent
    SetV1R5Conf pm_size Monitor rdagent
    SetV1R5Conf vm_size Monitor rdagent
    SetV1R5Conf cpu_usage Monitor rdagent
    SetV1R5Conf tmpfile_size Monitor rdagent
    
    SetV1R5Conf nginx Monitor
    SetV1R5Conf thread_count Monitor nginx
    SetV1R5Conf handle_count Monitor nginx
    SetV1R5Conf pm_size Monitor nginx
    SetV1R5Conf vm_size Monitor nginx
    SetV1R5Conf cpu_usage Monitor nginx
    
    SetV1R5Conf context_name SNMP
    SetV1R5Conf engine_id SNMP
    SetV1R5Conf private_password SNMP
    SetV1R5Conf private_protocol SNMP
    SetV1R5Conf auth_password SNMP
    SetV1R5Conf auth_protocol SNMP
    SetV1R5Conf security_level SNMP
    SetV1R5Conf security_name SNMP
    SetV1R5Conf security_model SNMP
    
    if [ ${OLD_VER_V} -ge 200 ]
    then
        echo "Check point: OLD_VER_V=${OLD_VER_V}, set log_cache_threshold and nginx_log_size."
        Log "Check point: OLD_VER_V=${OLD_VER_V}, set log_cache_threshold and nginx_log_size."
        SetV1R5Conf log_cache_threshold System
        SetV1R5Conf nginx_log_size Monitor nginx
    fi
    
    Log "Set conf file agent_cfg.xml to the new OceanStor BCManager Agent successfully."
    
    if [ -f ${AGENT_BACKUP_PATH}/conf/HostSN ]
    then
        cp -r ${AGENT_BACKUP_PATH}/conf/HostSN              ${AGENT_ROOT_PATH}/conf
    fi
    
    rm -rf ${AGENT_ROOT_PATH}/db/*
    cp -r ${AGENT_BACKUP_PATH}/db/*                ${AGENT_ROOT_PATH}/db
    
    Log "Move db AgentDB.db to the new OceanStor BCManager Agent successfully."
     
    if [ -d ${AGENT_BACKUP_PATH}/bin/thirdparty ]
    then
        rm -rf ${RDADMIN_HOME_PATH}/Agent/bin/thirdparty/*
        cp -r ${AGENT_BACKUP_PATH}/bin/thirdparty/*    ${AGENT_ROOT_PATH}/bin/thirdparty
    fi
    
    Log "Move third script files to the new OceanStor BCManager Agent successfully."
    
    STR_LISTEN=`cat "${AGENT_BACKUP_PATH}/bin/nginx/conf/nginx.conf" | grep -w 'listen'`
    STR_FASTCGI_PASS=`cat "${AGENT_BACKUP_PATH}/bin/nginx/conf/nginx.conf" | grep -w 'fastcgi_pass'`
    STR_CRT_FILE=`cat "${AGENT_BACKUP_PATH}/bin/nginx/conf/nginx.conf" | grep -w 'ssl_certificate'`
    STR_CRT_KEY_FILE=`cat "${AGENT_BACKUP_PATH}/bin/nginx/conf/nginx.conf" | grep -w 'ssl_certificate_key'`
    
    sed "/listen/s/.*/${STR_LISTEN}/g"  ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak
    rm -rf ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
    mv ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
    Log "Reconfig listen in nginx.conf ....ok"
    # current version is v2r1, need to replace certificate, certificate key and certificate key password with current version information
    # current version is v1r5, do not to replace information, but using the new install version information
    if [ "${CUR_VERSION_VR}" -eq "200001" ]
    then
        echo "Check point: CUR_VERSION_VR=${CUR_VERSION_VR}, do Agent certification file."
        Log "Check point: CUR_VERSION_VR=${CUR_VERSION_VR}, do Agent certification file."
        # cp certificate file
        CRT_FILE=`echo ${STR_CRT_FILE} | $AWK '{print $NF}' | $AWK -F ";" '{print $1}'`
        if [ ! -f "${AGENT_BACKUP_PATH}/bin/nginx/conf/${CRT_FILE}" ]
        then
            Log "${AGENT_BACKUP_PATH}/bin/nginx/conf/${CRT_FILE} is not exists."
            echo "${AGENT_BACKUP_PATH}/bin/nginx/conf/${CRT_FILE} is not exists."
            CurV2R1UpgradeFailedRollback
            exit ${ERR_AGENT_FILE_MIGRATE_FALIED}
        fi
        
        # cp certificate file key
        CRT_KEY_FILE=`echo ${STR_CRT_KEY_FILE} | $AWK '{print $NF}' | $AWK -F ";" '{print $1}'`
        if [ ! -f "${AGENT_BACKUP_PATH}/bin/nginx/conf/${CRT_KEY_FILE}" ]
        then
            Log "${AGENT_BACKUP_PATH}/bin/nginx/conf/${CRT_KEY_FILE} is not exists."
            echo "${AGENT_BACKUP_PATH}/bin/nginx/conf/${CRT_KEY_FILE} is not exists."
            CurV2R1UpgradeFailedRollback
            exit ${ERR_AGENT_FILE_MIGRATE_FALIED}
        fi
        
        cp -r ${AGENT_BACKUP_PATH}/bin/nginx/conf/${CRT_FILE}  ${AGENT_ROOT_PATH}/bin/nginx/conf
        cp -r ${AGENT_BACKUP_PATH}/bin/nginx/conf/${CRT_KEY_FILE}  ${AGENT_ROOT_PATH}/bin/nginx/conf
        
        sed "/ssl_certificate /s/.*/${STR_CRT_FILE}/g"  ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak
        rm -rf ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
        mv ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
        Log "Reconfig ssl_certificate in nginx.conf ....ok"
        
        sed "/ssl_certificate_key /s/.*/${STR_CRT_KEY_FILE}/g"  ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak
        rm -rf ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
        mv ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
        Log "Reconfig ssl_certificate_key in nginx.conf ....ok"
    fi
    
    sed "/fastcgi_pass/s/.*/${STR_FASTCGI_PASS}/g"  ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf > ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak
    rm -rf ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
    mv ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak ${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf
    Log "Reconfig fastcgi_pass in nginx.conf ....ok"
    
    if [ ${OLD_VER_V} -lt 200 ]
    then
        echo "Check point: OLD_VER_V=${OLD_VER_V}, do datamigration."
        Log "Check point: OLD_VER_V=${OLD_VER_V}, do datamigration."
        "${AGENT_ROOT_PATH}/bin/datamigration" record
        ERR_RESULT=$?
    
        "${AGENT_ROOT_PATH}/bin/datamigration" pass
        ERR_TMP=$?
        if [ ${ERR_RESULT} -eq 0 ]
        then
            ERR_RESULT=$ERR_TMP
        fi
        
        if [ ${ERR_RESULT} -ne 0 ]
        then
            echo "Datas migrate to new OceanStor BCManager Agent was failed."
            Log "Datas migrate from ${OLD_VERSION} to ${NEW_VERSION} was failed."
            
            echo "The previous OceanStor BCManager Agent is starting..."
            rm -rf "${AGENT_ROOT_PATH}"
            mv "${AGENT_BACKUP_PATH}" "${AGENT_ROOT_PATH}"
            
            if [ -d "${AGENT_RM_PATH}" ]
            then
                mv "${AGENT_RM_PATH}" "${AGENT_BACKUP_PATH}"
            fi
            
            su - ${AGENT_USER} 1>/dev/null ${UNIX_CMD} -c "\"${AGENT_ROOT_PATH}/bin/agent_start.sh\""
            if [ $? -ne 0 ]
            then
                echo "The previous OceanStor BCManager Agent start failed, rollback failed."
                Log "The previous OceanStor BCManager Agent start failed, rollback failed."
            fi
            
            echo "The upgrade OceanStor BCManager Agent will be stopped."
            exit ${ERR_AGENT_FILE_MIGRATE_FALIED}
        fi
    fi
    
    Log "Set conf file nginx.conf to the new OceanStor BCManager Agent successfully."
}

CurV2R1UpgradeFailedRollback()
{
    echo "The previous OceanStor BCManager Agent is starting..."
    rm -rf "${AGENT_ROOT_PATH}"
    mv "${AGENT_BACKUP_PATH}" "${AGENT_ROOT_PATH}"
    
    if [ -d "${AGENT_RM_PATH}" ]
    then
        mv "${AGENT_RM_PATH}" "${AGENT_BACKUP_PATH}"
    fi
    
    su - ${AGENT_USER} 1>/dev/null ${UNIX_CMD} -c "\"${AGENT_ROOT_PATH}/bin/agent_start.sh\""
    if [ $? -ne 0 ]
    then
        echo "The previous OceanStor BCManager Agent start failed, rollback failed."
        Log "The previous OceanStor BCManager Agent start failed, rollback failed."
    fi
    
    echo "The upgrade OceanStor BCManager Agent will be stopped."
    exit ${ERR_AGENT_FILE_MIGRATE_FALIED}
}

V1R3Config()
{
    MVV1R3Rdagentini
    
    AGENT_SL=`cat "${AGENT_ROOT_OLD}/conf/rdagentSign.ini" | grep SALT | $AWK -F '= ' '{print $2}'`

    AGENT_NAME=`cat ${AGENT_ROOT_OLD}/conf/rdagent.db | sed -n '1p'`
    if [ "${sysName}" = "HP-UX" ]
    then
        AGENT_HASH=`cat ${AGENT_ROOT_OLD}/conf/rdagent.db | $AWK 'END{print $1}'`
    else
        AGENT_HASH=`cat ${AGENT_ROOT_OLD}/conf/rdagent.db | sed -n '2p'`
    fi

    "${AGENT_ROOT_PATH}/bin/xmlcfg" write System sl ${AGENT_SL}
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write System name ${AGENT_NAME}
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write System hash ${AGENT_HASH}
    
    if [ -f ${AGENT_ROOT_OLD}/conf/HostSN ]
    then
        cp -r ${AGENT_ROOT_OLD}/conf/HostSN              ${AGENT_ROOT_PATH}/conf
        echo "">>${AGENT_ROOT_PATH}/conf/HostSN
    fi
    
    Log "Set conf files to the new OceanStor BCManager Agent successfully."
}

#move data
AgentUpgrade()
{
    if [ ${UPGRADE_TYPE} -eq 2 ]
    then
        GEV1R5Config
    else
        V1R3Config

        
        rm -rf ${AGENT_ROOT_PATH}/db/AgentDB.db
        cp -r ${AGENT_ROOT_OLD}/db/AgentDB.db  ${AGENT_ROOT_PATH}/db
        
        "${AGENT_ROOT_PATH}/bin/datamigration" table
        ERR_RESULT=$?
        
        "${AGENT_ROOT_PATH}/bin/datamigration" record
        ERR_TMP=$?
        if [ ${ERR_RESULT} -eq 0 ]
        then
            ERR_RESULT=$ERR_TMP
        fi
        
        "${AGENT_ROOT_PATH}/bin/datamigration" pass
        ERR_TMP=$?
        if [ ${ERR_RESULT} -eq 0 ]
        then
            ERR_RESULT=$ERR_TMP
        fi
        
        "${AGENT_ROOT_PATH}/bin/datamigration" salt
        ERR_TMP=$?
        if [ ${ERR_RESULT} -eq 0 ]
        then
            ERR_RESULT=$ERR_TMP
        fi
        
        if [ ${ERR_RESULT} -ne 0 ]
        then
            echo "Datas migrate to new OceanStor BCManager Agent was failed."
            Log "Datas migrate from ${OLD_VERSION} to ${NEW_VERSION} was failed."
            
            echo "The previous OceanStor BCManager Agent is starting..."
            rm -rf "${AGENT_BACKUP_PATH}"
            
            if [ -d "${AGENT_RM_PATH}" ]
            then
                mv "${AGENT_RM_PATH}" "${AGENT_BACKUP_PATH}"
            fi
            
            su - ${AGENT_USER} 1>/dev/null ${UNIX_CMD} -c "${AGENT_ROOT_OLD}/bin/rdagentService.sh start"
            if [ $? -ne 0 ]
            then
                echo "The previous OceanStor BCManager Agent start failed, rollback failed."
                Log "The previous OceanStor BCManager Agent start failed, rollback failed."
            fi
            
            echo "The upgrade OceanStor BCManager Agent will be stopped."
            exit ${ERR_AGENT_FILE_MIGRATE_FALIED}
        fi
        
        if [ -d ${AGENT_ROOT_OLD}/bin/thirdparty ]
        then
            rm -rf ${AGENT_ROOT_PATH}/bin/thirdparty/*
            cp -r ${AGENT_ROOT_OLD}/bin/thirdparty/*    ${AGENT_ROOT_PATH}/bin/thirdparty
        fi
    fi

    Log "Config data set successfully."
}

RollBackAgent()
{
    su - ${AGENT_USER} ${UNIX_CMD} -c "\"${AGENT_ROOT_PATH}/bin/agent_stop.sh\"" 2>&1
    sleep 3
    
    rm -rf "${AGENT_ROOT_PATH}"
    if [ ${UPGRADE_TYPE} -eq 2 ]
    then
        mv "${AGENT_BACKUP_PATH}" "${AGENT_ROOT_PATH}"
    else
        rm -rf "${AGENT_BACKUP_PATH}"
    fi
    
    if [ -d "${AGENT_RM_PATH}" ]
    then
        mv "${AGENT_RM_PATH}" "${AGENT_BACKUP_PATH}"
    fi
    
    if [ ${UPGRADE_TYPE} -eq 2 ]
    then
        su - ${AGENT_USER} ${UNIX_CMD} -c "\"${AGENT_ROOT_PATH}/bin/agent_start.sh\"" 2>&1
    else
        su - ${AGENT_USER} ${UNIX_CMD} -c "${AGENT_ROOT_OLD}/bin/rdagentService.sh start" 2>&1
    fi
    
    if [ 0 != $? ]
    then
        echo "Previous OceanStor BCManager Agent was rolled back failed."
        Log "Previous OceanStor BCManager Agent was rolled back failed."
    else
        echo "Previous OceanStor BCManager Agent was rolled back successfully."
        Log "Previous OceanStor BCManager Agent was rolled back successfully."
    fi
}

#start agent
StartAgent()
{
    #waite for free the port
    sleep 3
    
    "${AGENT_ROOT_PATH}/bin/agent_install.sh" -up
    
    su - ${AGENT_USER} ${UNIX_CMD} -c "\"${AGENT_ROOT_PATH}/bin/agent_start.sh\"" 2>&1
    if [ 0 != $? ]
    then
        echo "New OceanStor BCManager Agent was started failed, begin rollback."
        Log "New OceanStor BCManager Agent was started failed."
        RollBackAgent
            
        echo "OceanStor BCManager Agent was upgraded failed."
        Log "OceanStor BCManager Agent was upgraded failed."
        sleep 1
        exit $ERR_AGENT_START_FAILED
    fi
    
    return 0
}

UninstallV1R3Agent()
{
    if [ ${UPGRADE_TYPE} -eq 1 ]
    then
        if [ "${sysName}" = "Linux" ]
        then
            rpm -e ReplicationDirector-Agent
        elif [ "${sysName}" = "AIX" ]
        then
            installp -u ReplicationDirector-Agent
        elif [ "${sysName}" = "HP-UX" ]
        then
            swremove ReplicationDirector-Agent_HP-UX
        fi
    fi
    
    rm -rf ${AGENT_RM_PATH}
    
    #set auto start
    AGENT_ROOT_PATH=/home/rdadmin/Agent
    SetAutoStart
}


if [ "${INPUT_PARAM}" != "${UPGRADE_FLAG}" ]
then
    echo "You are about to upgrade the OceanStor BCManager Agent. This operation suspends the OceanStor BCManager Agent, causing the applications on the host to be out of protection during the upgrade."
    echo ""
    echo "Suggestion: Upgrade the OceanStor BCManager Agent after confirming that no application is during the implementation of a protection task."
    echo ""
    MakeSureAgentNoWorks
fi

echo "Begin to upgrade OceanStor BCManager Agent..."
Log "Begin to upgrade OceanStor BCManager Agent."

echo "Check free space of installation path..."
CheckFreeDiskSpace

echo "Check version of previous OceanStor BCManager Agent..."
CheckVersion
Log "The version of previous OceanStor BCManager Agent is ${OLD_VERSION}"

echo "Check files in new OceanStor BCManager Agent..."
CheckNewAgentFiles

echo "Check files in previous OceanStor BCManager Agent..."
CheckPreAgentFiles

echo "Stop previous OceanStor BCManager Agent..."
StopProcess

echo "Move new installation package to installation path..."
MoveAgentDir

echo "Move the configure data from previous to new OceanStor BCManager Agent..."
AgentUpgrade

ChangePrivilege

echo "Start the new OceanStor BCManager Agent..."
StartAgent

echo "Uninstall the previous OceanStor BCManager Agent..."
UninstallV1R3Agent

echo "OceanStor BCManager Agent was upgraded successfully."
Log "OceanStor BCManager Agent was upgraded successfully."
sleep 1

rm -rf "${RDADMIN_HOME_PATH}/Agent/bin/agent_upgrade.sh"
exit 0
