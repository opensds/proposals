#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#--------------------------------------------
# $1: -up for upgrade and push upgrade
# $1: -r for push installation
#--------------------------------------------

AGENT_USER=rdadmin
AGENT_GROUP=rdadmin
UNIX_CMD=
ENV_FILE=".bashrc"
ENV_EFFECT="export"
ENV_FLAG="="
SHELL_PATH=""
SHELL_TYPE=""
AGENT_ROOT=""
LD_LIBRARY_PATH=""
LIBPATH=""
AGENT_PORT=""
IP=0
ECHO_E=""
ROCKY4_FLAG=0
FREE_SPACE_MIN=512000
KEY_USERNAME=username
KEY_USERPWD=userpwd
KEY_IPADDRESS=ipaddress
KEY_PORT=port
UPGRADE_FLAG=-up
INSTALL_FLAG=-r
AWK=awk

# -------------error code----------------------
ERR_INPUT_PARAM_ERR=10
ERR_DISK_FREE_ISLESS_500MB=13
ERR_WORKINGUSER_ADD_FAILED=14
ERR_CHECK_INSTALLATION_PATH_FAILED=15
ERR_USERNAME_SET_FAILED=16
ERR_PASSWORD_SET_FAILED=17
ERR_IPADDR_SET_FAILED=18
ERR_PORT_SET_FAILED=19
ERR_CHECK_WORKING_USER_FAILED=21
ERR_AGENT_START_FAILED=27
ERR_ENV_SET_FAILED=28
ERR_PROCESS_IS_EXIST=29
# -------------error code----------------------

INPUT_PARAM=$1
AGENT_INFO=$2

if [ "${INPUT_PARAM}" != "" ] && [ "${INPUT_PARAM}" != "${UPGRADE_FLAG}" ]
then
    echo "Input param is error."
    exit $ERR_INPUT_PARAM_ERR
fi

DEFAULT_HOME_DIR=/home/${AGENT_USER}

#Check login user
sysName=`uname -s`
if [ "${sysName}" = "SunOS" ]
then
    AWK=nawk
    DEFAULT_HOME_DIR=/export/home/${AGENT_USER}
fi

LOG_USER=$LOGNAME
if [ "root" != $LOG_USER ]
then
    echo "Log user must be root, but ${LOG_USER} indeed."
    echo "The installation of OceanStor BCManager Agent will be stopped."
    exit 1
fi

if [ "${INSTALL_FLAG}" != "${INPUT_PARAM}" ]
then
    RDADMIN_HOME_PATH=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F ':' '{print $6}'`
    if [ "" = "${RDADMIN_HOME_PATH}" ]
    then
        echo "Agent working user ${AGENT_USER} is not exist."
        echo "The installation of OceanStor BCManager Agent will be stopped."
        exit $ERR_CHECK_WORKING_USER_FAILED
    fi
else
    if [ "" = "${AGENT_INFO}" ] || [ ! -f "${AGENT_INFO}" ]
    then
        echo "User info conf file ${AGENT_INFO} is not exist."
        echo "The installation of OceanStor BCManager Agent will be stopped."
        exit $ERR_INPUT_PARAM_ERR
    fi
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

#Get environment variables
GetEnvPath()
{
    CURRENT_PATH=`pwd`
    PARAM_PATH=`dirname "$1"`
    cd "${PARAM_PATH}"
    LD_LIBRARY_PATH=`pwd`
    cd "${CURRENT_PATH}"
 
    LIBPATH="${LD_LIBRARY_PATH}"
    CURRENT_ROOT=`dirname "${LD_LIBRARY_PATH}"`

    return 0
}

GetEnvPath $0

AGENT_ROOT_PATH=${CURRENT_ROOT}

LOG_FILE_NAME=${AGENT_ROOT_PATH}/log/agent_install.log
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

# only bash, ksh and csh
ChooseShell()
{
    PATH_SHELL=`which ksh 2>/dev/null | $AWK '{print $1}'`
    if [ "" != "${PATH_SHELL}" ]
    then
        PATH_CHECK=`RDsubstr ${PATH_SHELL} 1 1`
        if [ "/" = "${PATH_CHECK}" ]
        then
            USER_SHELL_TYPE=${PATH_SHELL}
            return 0
        fi
    fi
    
    PATH_SHELL=`which csh 2>/dev/null | $AWK '{print $1}'`
    if [ "" != "${PATH_SHELL}" ]
    then
        PATH_CHECK=`RDsubstr ${PATH_SHELL} 1 1`
        if [ "/" = "${PATH_CHECK}" ]
        then
            USER_SHELL_TYPE=${PATH_SHELL}
            return 0
        fi
    fi
    
    PATH_SHELL=`which bash 2>/dev/null | $AWK '{print $1}'`
    if [ "" != "${PATH_SHELL}" ]
    then
        PATH_CHECK=`RDsubstr ${PATH_SHELL} 1 1`
        if [ "/" = "${PATH_CHECK}" ]
        then
            USER_SHELL_TYPE=${PATH_SHELL}
            return 0
        fi
    fi
    
    echo "There is not supported shell in this system."
    Log "There is not supported shell in this system."
    exit $ERR_WORKINGUSER_ADD_FAILED
}

AddUserAndGroup()
{
    #check Agent group
    GROUP_CHECK=`cat /etc/group | grep "^${AGENT_GROUP}:"`
    if [ "" = "${GROUP_CHECK}" ]
    then
        if [ "${sysName}" = "AIX" ]
        then
            mkgroup ${AGENT_GROUP}
        else
            groupadd ${AGENT_GROUP}
        fi
        
        if [ 0 -ne $? ]
        then
            echo "Agent working group ${AGENT_GROUP} was added failed."
            echo "The installation of OceanStor BCManager Agent will be stopped."
            Log "Agent working group ${AGENT_GROUP} was added failed."
            exit $ERR_WORKINGUSER_ADD_FAILED
        fi
    fi
    
    #check Agent user
    USER_CHECK=`cat /etc/passwd | grep "^${AGENT_USER}:"`
    if [ "" = "${USER_CHECK}" ]
    then
        if [ -d "${DEFAULT_HOME_DIR}" ]
        then
            echo "The home directory ${DEFAULT_HOME_DIR} of user ${AGENT_USER} is exist."
            echo "The installation of OceanStor BCManager Agent will be stopped."
            Log "The home directory ${DEFAULT_HOME_DIR} of user ${AGENT_USER} is exist."
            exit $ERR_WORKINGUSER_ADD_FAILED
        fi
        
        ChooseShell
        Log "Agent working user choose shell type ${USER_SHELL_TYPE}."
        
        if [ "${sysName}" = "SunOS" ]
        then
            useradd -m -s ${USER_SHELL_TYPE} -d ${DEFAULT_HOME_DIR} -g ${AGENT_GROUP} ${AGENT_USER}
        else
            useradd -m -s ${USER_SHELL_TYPE} -g ${AGENT_GROUP} ${AGENT_USER}
        fi
        
        if [ 0 -ne $? ]
        then
            echo "Agent working user ${AGENT_USER} was added failed."
            echo "The installation of OceanStor BCManager Agent will be stopped."
            Log "Agent working user ${AGENT_USER} was added failed."
            exit $ERR_WORKINGUSER_ADD_FAILED
        fi
    fi
    
    if [ "" = "${GROUP_CHECK}" ]
    then
        if [ "${sysName}" = "HP-UX" ]
        then
            HP_OS_VERSION=`uname -a | awk '{print $3}' | awk -F "." '{print $2"."$3}'`
            if [ "${HP_OS_VERSION}" = "11.31" ]
            then
                usermod -F -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
            else
                usermod -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
            fi
        else
            usermod -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
        fi
        
        if [ 0 -ne $? ]
        then
            echo "Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
            echo "The installation of OceanStor BCManager Agent will be stopped."
            Log "Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
            exit $ERR_WORKINGUSER_ADD_FAILED
        fi
    else
        if [ "${sysName}" = "SunOS" ]
        then
            TMP_GROUP=`/usr/xpg4/bin/id -g -n ${AGENT_GROUP}`
        else
            TMP_GROUP=`id -g -n ${AGENT_GROUP}`
        fi
        if [ "${TMP_GROUP}" != "${AGENT_GROUP}" ]
        then 
            if [ "${sysName}" = "HP-UX" ]
            then
                HP_OS_VERSION=`uname -a | awk '{print $3}' | awk -F "." '{print $2"."$3}'`
                if [ "${HP_OS_VERSION}" = "11.31" ]
                then
                    usermod -F -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
                else
                    usermod -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
                fi
            else
                usermod -g ${AGENT_GROUP} ${AGENT_USER} 2>/dev/null
            fi
        
            if [ 0 -ne $? ]
            then
                echo "Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
                echo "The installation of OceanStor BCManager Agent will be stopped."
                Log "Add Agent working user ${AGENT_USER} to group ${AGENT_GROUP} failed."
                exit $ERR_WORKINGUSER_ADD_FAILED
            fi
        fi
    fi 
}


AddUserAndGroup

RDADMIN_HOME_PATH=`cat /etc/passwd | grep "^${AGENT_USER}:" | $AWK -F ':' '{print $6}'`

#Check installation path
if [ "$1" != "${UPGRADE_FLAG}" ] && [ "$1" != "${INSTALL_FLAG}" ]
then
    RDADMIN_HOME_PATH_LEN=`echo $RDADMIN_HOME_PATH | $AWK '{print length()}'`
    SLASH_FLAG=`RDsubstr $RDADMIN_HOME_PATH $RDADMIN_HOME_PATH_LEN 1`
    AGENT_ROOT_CHECK="${RDADMIN_HOME_PATH}/Agent"

    if [ "$SLASH_FLAG" = "/" ]
    then
        AGENT_ROOT_CHECK="${RDADMIN_HOME_PATH}Agent"
    fi

    if [ "${CURRENT_ROOT}" != "${AGENT_ROOT_CHECK}" ]
    then
        echo "The installation path of OceanStor BCManager Agent can only be ${AGENT_ROOT_CHECK}."
        echo "The installation of OceanStor BCManager Agent will be stopped."
        Log "The installation path of OceanStor BCManager Agent can only be ${AGENT_ROOT_CHECK}."
        exit $ERR_CHECK_INSTALLATION_PATH_FAILED
    fi
    
    chown -R ${AGENT_USER}:${AGENT_GROUP} ${RDADMIN_HOME_PATH}
    chmod 775 ${AGENT_ROOT_CHECK}
fi

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
else
    echo "The shell type bash, ksh and csh only be supported in user ${AGENT_USER} by OceanStor BCManager Agent."
    echo "The installation of OceanStor BCManager Agent will be stopped."
    Log "The shell type bash, ksh and csh only be supported in user ${AGENT_USER} by OceanStor BCManager Agent."
    exit $ERR_ENV_SET_FAILED
fi

if [ "${SysRockyFlag}" = "" ]
then
    ECHO_E=-e
fi

if [ "${sysName}" = "AIX" ] || [ "${sysName}" = "HP-UX" ] || [ "${sysName}" = "SunOS" ] || [ 1 -eq ${ROCKY4_FLAG} ]
then
    ECHO_E=""
    if [ "${SHELL_TYPE}" = "bash" ]
    then
        UNIX_CMD=-l
    fi
fi

#check disk space
CheckFreeDiskSpace()
{    
	AGENT_INSTALL_PATH=${RDADMIN_HOME_PATH}/Agent
	
    if [ "${sysName}" = "Linux" ]
    then
        FREE_SPACE=`df -k ${AGENT_INSTALL_PATH} | grep -n '' | $AWK 'END{print $4}'`
    elif [ "${sysName}" = "AIX" ]
    then
        FREE_SPACE=`df -k ${AGENT_INSTALL_PATH} | grep -n '' | $AWK '{print $3}' | sed -n '2p'`
    elif [ "${sysName}" = "HP-UX" ]
    then
        FREE_SPACE=`df -k ${AGENT_INSTALL_PATH} | grep -w 'free' | $AWK '{print $1}'`
    elif [ "${sysName}" = "SunOS" ]
    then
        FREE_SPACE=`df -k ${AGENT_INSTALL_PATH} | $AWK '{print $4}' | sed -n '2p'`
    fi
    
    if [ ${FREE_SPACE_MIN} -gt ${FREE_SPACE} ]
    then
        echo "The installation path free space ${FREE_SPACE} is less than the minimum space requirements ${FREE_SPACE_MIN}."
        Log "The installation path free space ${FREE_SPACE} is less than the minimum space requirements ${FREE_SPACE_MIN}, then exit."
        echo "The installation of OceanStor BCManager Agent will be stopped."
        exit $ERR_DISK_FREE_ISLESS_500MB
    fi
}

CheckAgentexist()
{
    ps -lu ${AGENT_USER} | grep -v grep | grep $1 1>/dev/null 2>&1
    if [ $? -ne 0 ]
    then
        Log "Process $1 is not exist, then install OceanStor BCManager Agent."
        return 0
    fi

    echo "Process $1 is already exist, then exit installation."
    Log "Process $1 is already exist, then exit installation."
    exit $ERR_PROCESS_IS_EXIST
}

DealAfterInstallFailed()
{
    "${AGENT_ROOT_PATH}/bin/agent_uninstall.sh" -r 1>/dev/null 2>&1
    
    if [ "${INPUT_PARAM}" = "${INSTALL_FLAG}" ]
    then
        rm -rf "${AGENT_ROOT_PATH}"
    fi
}

InstallPrepare()
{
    if [ "${INSTALL_FLAG}" = "${INPUT_PARAM}" ]
    then
        if [ ! -d "${AGENT_ROOT_PATH}" ]
        then
            mkdir "${AGENT_ROOT_PATH}"
        fi
        
        DIR_ISEMPTY=`ls "${AGENT_ROOT_PATH}"`
        if [ "" != "${DIR_ISEMPTY}" ]
        then
            Log "The ${AGENT_ROOT_PATH} is not empty."
            echo "The ${AGENT_ROOT_PATH} is not empty."
            exit $ERR_CHECK_INSTALLATION_PATH_FAILED
        fi
        
        
        
        cp -r "${CURRENT_ROOT}/bin"   "${AGENT_ROOT_PATH}"
        cp -r "${CURRENT_ROOT}/conf"  "${AGENT_ROOT_PATH}"
        cp -r "${CURRENT_ROOT}/db"    "${AGENT_ROOT_PATH}"
        cp -r "${CURRENT_ROOT}/log"   "${AGENT_ROOT_PATH}"
        cp -r "${CURRENT_ROOT}/tmp"   "${AGENT_ROOT_PATH}"
        cp -r "${CURRENT_ROOT}/Open Source Software Notice.docx"  "${AGENT_ROOT_PATH}"
        
        rm -f "${AGENT_ROOT_PATH}/log"/*
        rm -f "${AGENT_ROOT_PATH}/tmp"/*
        
        chown -R ${AGENT_USER}:${AGENT_GROUP} "${AGENT_ROOT_PATH}"
        chmod -R 775 "${AGENT_ROOT_PATH}"
        
    fi
}

# $1: /bin:/sbin:/usr/sbin:/opt/VRTS/bin:/opt/VRTSvcs/bin
SetPathEnv()
{
    ENV_VAR_CHECK=`su - ${AGENT_USER} ${UNIX_CMD} -c "env" | grep "^PATH=" | grep -w "$1"`
    if [ "" = "${ENV_VAR_CHECK}" ]
    then
        if [ -d "$1" ]
        then
            ENV_PATH=${ENV_PATH}:$1
        fi
    fi
}

CheckRdadminUserEnv()
{
    RDADMIN_ENV_FILE="${RDADMIN_HOME_PATH}/${ENV_FILE}"
    
    if [ ! -f $RDADMIN_ENV_FILE ]
    then
        touch "$RDADMIN_ENV_FILE"
        chown ${AGENT_USER}:${AGENT_GROUP} "${RDADMIN_ENV_FILE}"
        chmod 700 "${RDADMIN_ENV_FILE}"
        Log "User env file is not exist, touch ${RDADMIN_ENV_FILE}."
    fi
    
    AGENT_ROOT_OLD=`cat "${RDADMIN_ENV_FILE}" | grep "${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}"`
    AGENT_ROOT_OLD_LINENO=`cat "${RDADMIN_ENV_FILE}" | grep -n "${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}" | $AWK -F ':' '{print $1}'`
    
    if [ "" != "$AGENT_ROOT_OLD" ]
    then
        Log "Environment variable AGENT_ROOT ${AGENT_ROOT_OLD} is exist, then covery it with ${AGENT_ROOT_PATH}."
        sed  "${AGENT_ROOT_OLD_LINENO}d" "${RDADMIN_ENV_FILE}" > "${RDADMIN_ENV_FILE}".bak
        mv "${RDADMIN_ENV_FILE}".bak "${RDADMIN_ENV_FILE}"
    else
        Log "Environment variable AGENT_ROOT is not exist, then set AGENT_ROOT=${AGENT_ROOT_PATH}."
    fi
    
    echo "${ENV_EFFECT} AGENT_ROOT${ENV_FLAG}\"${AGENT_ROOT_PATH}\"" >> "${RDADMIN_ENV_FILE}"
    
    if [ "AIX" = "${sysName}" ]
    then
        LIBPATH_OLD=`cat "${RDADMIN_ENV_FILE}" | grep "${ENV_EFFECT} LIBPATH${ENV_FLAG}"`
        LIBPATH_OLD_LINENO=`cat "${RDADMIN_ENV_FILE}" | grep -n "${ENV_EFFECT} LIBPATH${ENV_FLAG}" | $AWK -F ':' '{print $1}'`
    
        if [ "" != "$LIBPATH_OLD" ]
        then
            Log "Environment variable LIBPATH ${LIBPATH_OLD} is exist, then covery it with ${LIBPATH}."
            sed  "${LIBPATH_OLD_LINENO}d" "${RDADMIN_ENV_FILE}" > "${RDADMIN_ENV_FILE}".bak
            mv "${RDADMIN_ENV_FILE}".bak "${RDADMIN_ENV_FILE}"
        else
            Log "Environment variable LIBPATH is not exist, then set LIBPATH=${LIBPATH}."
        fi
    
        echo "${ENV_EFFECT} LIBPATH${ENV_FLAG}\"${LIBPATH}\"" >> "${RDADMIN_ENV_FILE}"
        
    else
        LD_LIBRARY_PATH_OLD=`cat "${RDADMIN_ENV_FILE}" | grep "${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}"`
        LD_LIBRARY_PATH_OLD_LINENO=`cat "${RDADMIN_ENV_FILE}" | grep -n "${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}" | $AWK -F ':' '{print $1}'`
    
        if [ "" != "$LD_LIBRARY_PATH_OLD" ]
        then
            Log "Environment variable LD_LIBRARY_PATH ${LD_LIBRARY_PATH_OLD} is exist, then covery it with ${LD_LIBRARY_PATH}."
            sed  "${LD_LIBRARY_PATH_OLD_LINENO}d" "${RDADMIN_ENV_FILE}" > "${RDADMIN_ENV_FILE}".bak
            mv "${RDADMIN_ENV_FILE}".bak "${RDADMIN_ENV_FILE}"
        else
            Log "Environment variable LD_LIBRARY_PATH is not exist, then set LD_LIBRARY_PATH=${LD_LIBRARY_PATH}."
        fi
    
        echo "${ENV_EFFECT} LD_LIBRARY_PATH${ENV_FLAG}\"${LD_LIBRARY_PATH}\"" >> "${RDADMIN_ENV_FILE}"
        
    fi
    
    # set PATH: /bin:/sbin:/usr/sbin:/opt/VRTS/bin:/opt/VRTSvcs/bin
    if [ "${UPGRADE_FLAG}" != "${INPUT_PARAM}" ]
    then
        ENV_PATH='.:${PATH}'
        
        SetPathEnv "/bin"       
        SetPathEnv "/sbin"      
        SetPathEnv "/usr/sbin"      
        SetPathEnv "/opt/VRTS/bin"
        SetPathEnv "/opt/VRTSvcs/bin"
        
        if [ ${ENV_PATH} != '.:${PATH}' ]
        then
            echo "${ENV_EFFECT} PATH${ENV_FLAG}${ENV_PATH}" >> "${RDADMIN_ENV_FILE}"
        fi
    fi
    
    chown ${AGENT_USER}:${AGENT_GROUP} "${RDADMIN_ENV_FILE}"
    chmod 700 "${RDADMIN_ENV_FILE}"
}

SetUserNameAndPasswd()
{
    NAMENUM=0
    COMPLEX=0
    USERNAME=
    PASSWORD=
    PASSWORDTEMP=
    USERREX="^[a-zA-Z]\\w\\{3,15\\}$"
    
    if [ "AIX" = "$sysName" ]
    then
        USERREX="^[a-zA-Z][a-zA-Z0-9_]\{3,15\}$"
    elif [ "HP-UX" = "$sysName" ] || [ "SunOS" = "$sysName" ]
    then
        USERREX="^[a-zA-Z]"
    fi 

    while [ ${NAMENUM} -lt 3 ]
    do
        if [ "${INSTALL_FLAG}" = "${INPUT_PARAM}" ]
        then
            NAMENUM=2
            USERNAME=`cat "${AGENT_INFO}" | grep "${KEY_USERNAME}" | $AWK -F '=' '{print $2}'`

            USERNAME=`echo "${USERNAME}" | $AWK 'gsub(/^ *| *$/,"")'`
        else
            echo "Please input your name:"
            PrintPrompt; read USERNAME
        fi
        
        USERCHECK=`echo "${USERNAME}" | grep -n ${USERREX}`
        if [ "${USERCHECK}" = "" ]
        then
            echo "The name should contain 4 to 16 characters,  including case-sensitive letters, digits or underscores (_),and must start with a letter."
            NAMENUM=`expr ${NAMENUM} + 1`
            sleep 1
            continue
        fi
        
        if [ "HP-UX" = "$sysName" ] || [ "SunOS" = "$sysName" ]
        then
            LENGTH=`echo $USERNAME | $AWK '{print length()}'`
            
            if [ ${LENGTH} -gt 16 ] || [ ${LENGTH} -lt 4 ]
            then
            echo "The name sholud contain 4 to 16 characters.." 
                sleep 1
                NAMENUM=`expr ${NAMENUM} + 1`
                continue
            fi
            
            TMP=1
            SWITCH=0
            while [ $TMP -le $LENGTH ]
            do
                CHECK=`echo $USERNAME | $AWK -v i="$USERNAME" -v j="$TMP" '{print substr(i,j,1)}' | grep -n [a-zA-Z0-9_]`
                if [ "$CHECK" = "" ]
                then                
                    SWITCH=1
                fi
                TMP=`expr $TMP + 1`
            done
            
            if [ $SWITCH -eq 1 ]
            then
                echo "The name should contains only letters(a-zA-Z) digits(0-9) and underscores(_)." 
                sleep 1
                NAMENUM=`expr ${NAMENUM} + 1`
                continue
            fi
            
        fi
        
        break
    done

    if [ ${NAMENUM} = 3 ]
    then
        echo ${ECHO_E} "\nThe installation of OceanStor BCManager Agent will be stopped."
        Log "The name that you entered($USERNAME) is error! so installation was stopped!"
        DealAfterInstallFailed
        exit $ERR_USERNAME_SET_FAILED
    fi
    
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write System name ${USERNAME}

    #set password
    if [ "${INSTALL_FLAG}" = "${INPUT_PARAM}" ]
    then
        "${AGENT_ROOT_PATH}/bin/getinput" "${AGENT_INFO}"
    else
        "${AGENT_ROOT_PATH}/bin/getinput"
    fi
    
    if [ $? -ne 0 ]
    then
        echo ${ECHO_E} "\nThe installation of OceanStor BCManager Agent will be stopped."
        Log "The installation of OceanStor BCManager Agent will be stopped."
        if [ -f "${AGENT_ROOT_PATH}/tmp/en_tmp" ]
        then
            rm -rf "${AGENT_ROOT_PATH}/tmp/en_tmp"
        fi
        
        DealAfterInstallFailed
        exit $ERR_PASSWORD_SET_FAILED
    fi
    
    if [ ! -f "${AGENT_ROOT_PATH}/tmp/en_tmp" ]
    then
        echo ${ECHO_E} "\nThe temporary file en_tmp does not exist, installation of OceanStor BCManager Agent will be stopped."
        Log "The temporary file en_tmp does not exist."
        DealAfterInstallFailed
        exit $ERR_PASSWORD_SET_FAILED
    fi
    
    NEW_PASSWD=`cat "${AGENT_ROOT_PATH}/tmp/en_tmp"`
    rm -rf "${AGENT_ROOT_PATH}/tmp/en_tmp"
    
    "${AGENT_ROOT_PATH}/bin/xmlcfg" write System hash ${NEW_PASSWD}
    if [ $? -ne 0 ]
    then
        echo ${ECHO_E} "\nSet user ${USERNAME} password failed, The installation of OceanStor BCManager Agent will be stopped."
        Log "Set user ${USERNAME} password failed."
        DealAfterInstallFailed
        exit $ERR_PASSWORD_SET_FAILED
    fi
    
    Log "Set password and user name($USERNAME) successfully!"
}

#function:check if the linux is redhat7
#param: no
#return: if is redhat7 then return 1, else return 0;
IsRedhat7()
{
    #is not redhat
    if [ ! -f /etc/redhat-release ]
    then
      return 0
    fi
    
    #redhat
    LINE=`cat /etc/redhat-release`
    REDHAT_VERSION=`echo $LINE|awk -F '.' '{print $1}'|awk '{print $NF}'`
    if [ "$REDHAT_VERSION" = "7" ]
    then
        return 1
    fi
    return 0
}

#get all the ip on the machine;
getLocalIpAddr()
{
    LOCALIPARRAY=""
    if [ "AIX" = "${sysName}" ] || [ "SunOS" = "${sysName}" ]
    then
        LOCALIPARRAY=`ifconfig -a|grep "inet "|$AWK -F " " '{print $2}'`
    elif [ "HP-UX" = "${sysName}" ]
    then
        LOCALIPARRAY=`netstat -ni|grep -v 'Address'|awk '{print $4}'`
    else
        IsRedhat7
        isRedHat7=$?
        if [ 1 = $isRedHat7 ]
        then
            LOCALIPARRAY=`ifconfig -a|grep "inet "|$AWK -F " " '{print $2}'`
        else
            LOCALIPARRAY=`ifconfig -a|grep "inet "|$AWK -F " " '{print $2}' |$AWK -F ":" '{print $2}'`
        fi
    fi
    #filter lo ip
    for ip in $LOCALIPARRAY
    do
        if [ "$ip" != "127.0.0.1" ]
        then
            echo $ip
        fi
    done
}

#$1-- user chooice, $2 LOCALIPARRAY count;
#return :0 -- error, 1 -- right
isIpIndexRight()
{
    input=$1
    limit=$2
    check=`echo $input | grep "^[0-9]\{1,\}$"`
    if [ -z "$check" ] || [ $input -gt $limit ]
    then 
        return 0
    fi
    return 1
}

SetIp()
{ 
    LOCALIPARRAY=`getLocalIpAddr`
    if [ "${LOCALIPARRAY}" = "" ]
    then
        echo "Failed to obtain the IP address, installation aborted."
        Log "Failed to obtain the IP address, installation aborted."
        exit $ERR_IPADDR_SET_FAILED
    fi

    NUMBER=0
    while [ $NUMBER -lt 3 ]
    do
        echo "Please choose IP Address binded by nginx."
        echo "Please select the serial number is greater than 0 Numbers!"
        index=1
        for ip in ${LOCALIPARRAY}
        do
            echo "   $index  $ip"
            index=`expr $index + 1`
        done;
        PrintPrompt
        
        read ipIndex
        if [ -z "$ipIndex" ] || [ "0" = "$ipIndex" ]
        then 
            echo "Your choice is not valid."
            continue
        fi
        localIpArraySize=`echo $LOCALIPARRAY | $AWK '{print NF}'`
        isIpIndexRight $ipIndex $localIpArraySize
        right=$?
        if [ 0 = $right ]
        then 
            echo "Your choice is not valid."
            NUMBER=`expr $NUMBER + 1`
            sleep 1
            continue
        fi
        IP=`echo $LOCALIPARRAY | $AWK -v Index="$ipIndex" '{print $Index}'`
        break
    done

    if [ $NUMBER = 3 ]
    then
        echo ${ECHO_E} "The installation of OceanStor BCManager Agent will be stopped."
        Log "The IP you entered($IP) is error, so installation was stopped!"
        exit $ERR_IPADDR_SET_FAILED
    fi
  
    Log "Set IP($IP) successfully!"
}

SetIpForPush()
{
    FLAG=0
    IP_START=""
    IP_NEXT=""
    IPREX="^[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}$"
    
    IP=`cat "${AGENT_INFO}" | grep "${KEY_IPADDRESS}" | $AWK -F '=' '{print $2}'`
    IP=`echo "${IP}" | $AWK 'gsub(/^ *| *$/,"")'`
    
    IPCHECK=`echo "${IP}" | grep -n "${IPREX}"`
    if [ "$IPCHECK" = "" ]
    then
        echo "IP address format is error."
        Log "IP address format is error."
        DealAfterInstallFailed
        exit $ERR_IPADDR_SET_FAILED
    fi
    
    if [ "$IP" = "0.0.0.0" ]
    then 
        Log "The IP Address was set 0.0.0.0."
        return 0
    fi
    
    IP_NUM=`echo "$IP" | $AWK -F "." '{for(x=1;x<=NF;x++) print $x}'`
    for loop in ${IP_NUM}
    do
        if [ "HP-UX" = "${sysName}" ]
        then
            if [ ${#loop} -gt 3 ]
            then
                Log "IP Address number: $loop is error"
                FLAG=1
                break
            fi
                    
            if [ ${#loop} -eq 3 ]
            then
                if [ $loop -lt 100 ]
                then
                    Log "IP Address number: $loop is error"
                    FLAG=1
                    break
                fi
            elif [ ${#loop} -eq 2 ]
            then
                if [ $loop -lt 10 ]
                then
                    Log "IP Address number: $loop is error"
                    FLAG=1
                    break
                fi
            fi
        else
            IP_START=`RDsubstr $loop 1 1`
            IP_NEXT=`RDsubstr $loop 2 1`
                
            if [ "AIX" = "${sysName}" ] || [ "SunOS" = "${sysName}" ]
            then
                LOCALIP_LIST=`ifconfig -a | grep "inet " | $AWK -F " " '{print $2}'`
                    
            #for linux
            else
                #check is redhat7
                IsRedhat7
                ISREDHAT7=$?
                if [ "$ISREDHAT7" = "0" ]
                then
                    LOCALIP_LIST=`ifconfig -a | grep "inet " | $AWK -F " " '{print $2}' | $AWK -F ":" '{print $2}'`
                else
                    LOCALIP_LIST=`ifconfig -a | grep "inet " | $AWK -F " " '{print $2}'`
                fi
            fi
            
            if [ $IP_START -eq 0 ]
            then    
                if [ "$IP_NEXT" != "" ]
                then
                    Log "IP Address number[$loop] can not start with 0."
                    FLAG=1
                    break
                fi 
            fi
        fi
        
        if [ $loop -lt 0 ]||[ $loop -gt 255 ]
        then
            Log "The IP Address number should be more than 0 and less than 255."
            FLAG=1
            break
        fi
    done
    
    if [ $FLAG -eq 1 ]
    then
        echo "IP Address was set failed."
        Log "IP Address was set failed."
        DealAfterInstallFailed
        exit $ERR_IPADDR_SET_FAILED
    fi
    
    if [ $loop -eq 0 ]
    then
        echo "The last number can not be 0."
        Log "The last number can not be 0"
        DealAfterInstallFailed
        exit $ERR_IPADDR_SET_FAILED
    fi
    
    if [ "${IP}" = "127.0.0.1" ]
    then 
        echo "The IP Address can not be 127.0.0.1."
        Log "The IP Address can not be 127.0.0.1."
        DealAfterInstallFailed
        exit $ERR_IPADDR_SET_FAILED
    fi
    
    FLAG=0
    if [ "HP-UX" = "${sysName}" ]
    then
        UP_LAN=`lanscan 2>/dev/null | grep "UP" | $AWK '{print $5}'`
        if [ "$UP_LAN" = "" ]
        then
            echo "No UP IP was found a in HP-UX."
            Log "No UP IP was found a in HP-UX."
            DealAfterInstallFailed
            exit $ERR_IPADDR_SET_FAILED
        fi
            
        for tmp in $UP_LAN
        do
            UP_IP=`ifconfig $tmp | grep "inet " | $AWK '{print $2}'`          
            if [ "$IP" = "$UP_IP" ]
            then
                FLAG=1
                break
            fi
        done
    else
        for LOCALIP in $LOCALIP_LIST
        do
            if [ "$LOCALIP" = "$IP" ]
            then
                FLAG=1
                break
            fi
        done
    fi
    
    if  [ $FLAG -eq 0 ]
    then
        echo "The IP Address input is not a local IP Address."
        Log "The IP Address input is not a local IP Address."
        DealAfterInstallFailed
        exit $ERR_IPADDR_SET_FAILED
    fi
}

# $1 agent and nginx port number
SetPort()
{
    PORTREX="^[1-9][0-9]\\{0,4\\}$"
    PORT=0
    PORT_NUMBER=0
    while [ ${PORT_NUMBER} -lt 3 ]
    do       
        if [ "${INSTALL_FLAG}" = "${INPUT_PARAM}" ]
        then
            PORT_NUMBER=2
            if [ "rdagent" = "$1" ]
            then
                PORT=8091
            else
                PORT=`cat "${AGENT_INFO}" | grep "${KEY_PORT}" | $AWK -F '=' '{print $2}'`
                PORT=`echo "${PORT}" | $AWK 'gsub(/^ *| *$/,"")'`
            fi
        else
            if [ "rdagent" = "$1" ]
            then
                echo "Please input $1 listening port number 1024-65535, default port 8091:"
            else
                echo "Please input $1 listening port number 1024-65535, default port 59526:"
            fi
            PrintPrompt; read PORT
        fi
    
        if [ "${PORT}" = "" ]
        then
            if [ "rdagent" = "$1" ]
            then
                echo "You choose $1 default port number[8091]."
                PORT=8091
            else
                echo "You choose $1 default port number[59526]."
                PORT=59526
            fi    
        fi
    
        PORTCHECK=`echo "${PORT}" | grep -n "${PORTREX}"`
        if [ "${PORTCHECK}" = "" ]
        then
            echo "The port number should be contain 1 to 5 digits and start with 1-9."
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        elif [ ${PORT} -gt 65535 -o ${PORT} -le 1024 ]
        then
            echo "The port number should be more than 1024 and less than or equal to 65535."
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        fi
    
        if [ "Linux" = "${sysName}" ]
        then
            PORTS=`netstat -an | grep ${PORT} | $AWK '{print $4}' | $AWK -F ':' '{print $NF}'`
        elif [ "HP-UX" = "${sysName}" ]
        then
            PORTS=`netstat -an | grep ${PORT} | $AWK -F " " '{print $4}' | $AWK -F "." '{print $NF}'`
        elif [ "AIX" = "${sysName}" ]
        then
            PORTS=`netstat -an | grep ${PORT} | $AWK -F '.' '{print $2}' | $AWK -F ' ' '{print $1}'`
        elif [ "SunOS" = "${sysName}" ]
        then
            PORTS=`netstat -an | grep ${PORT} | $AWK '{print $1}' | $AWK -F '.' '{print $NF}'`
        fi

        PORTSTATUS=""
        for PORTEACH in ${PORTS}
        do
            if [ "${PORTEACH}" = "${PORT}" ]
            then
                PORTSTATUS=${PORTEACH}
                break
            fi
        done
    
        
        if [ "${PORTSTATUS}" != "" ]
        then 
            echo "The port number is used by other process!"
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        fi
        
        if [ "${AGENT_PORT}" = "${PORT}" ]
        then
            Log "nginx port number is the same with agent port number."
            echo "OceanStor BCManager Agent nginx port number ${PORT} is the same with agent port number ${AGENT_PORT}"
            PORT_NUMBER=`expr ${PORT_NUMBER} + 1`
            sleep 1
            continue
        fi
        
        AGENT_PORT=${PORT}
        break
    done

    if [ ${PORT_NUMBER} = 3 ]
    then
        echo "The installation of OceanStor BCManager Agent will be stopped."
        Log "The port you entered($PORT) is error, so installation was stopped!"
        DealAfterInstallFailed
        exit $ERR_PORT_SET_FAILED
    fi
    
    if [ "rdagent" = "$1" ]
    then
        "${AGENT_ROOT_PATH}/bin/xmlcfg" write System port ${PORT}
        sed  "/fastcgi_pass/s/.*/            fastcgi_pass   127.0.0.1:${PORT};/g" "${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf" > "${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak"
    else
        sed  "/listen/s/.*/        listen       ${IP}:${PORT} ssl;/g" "${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf" > "${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak"
    fi
    
    mv "${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf.bak" "${AGENT_ROOT_PATH}/bin/nginx/conf/nginx.conf"
    Log "Set port($PORT) successfully!"
}

StartAgent()
{
    su - ${AGENT_USER} 1>/dev/null ${UNIX_CMD} -c "\"${AGENT_ROOT_PATH}/bin/agent_start.sh\""
    if [ 0 != $? ]
    then
        echo "OceanStor BCManager Agent was installed failed."
        Log "Start OceanStor BCManager Agent failed."
        Log "OceanStor BCManager Agent was installed failed."
        DealAfterInstallFailed
        exit $ERR_AGENT_START_FAILED
    else
        Log "Start OceanStor BCManager Agent successfully."
    fi
}


AGENT_ROOT=${RDADMIN_HOME_PATH}/Agent
AGENT_ROOT_PATH=${RDADMIN_HOME_PATH}/Agent
LD_LIBRARY_PATH=${RDADMIN_HOME_PATH}/Agent/bin
LIBPATH=${RDADMIN_HOME_PATH}/Agent/bin

if [ "${INPUT_PARAM}" = "${UPGRADE_FLAG}" ]
then
    CheckRdadminUserEnv
else
    Log "######################agent_install########################"
    Log "Begin to install OceanStor BCManager Agent."
    
    Log "Check free space of  OceanStor BCManager Agent installation."
    CheckFreeDiskSpace

    Log "Check proccess of OceanStor BCManager Agent."
    CheckAgentexist rdagent
    CheckAgentexist rdnginx
    CheckAgentexist monitor
    
    InstallPrepare

    Log "Check env of OceanStor BCManager Agent."
    CheckRdadminUserEnv

    Log "Set env of OceanStor BCManager Agent."
    SetUserNameAndPasswd
    
    Log "Set IP of OceanStor BCManager Agent."
    if [ "${INPUT_PARAM}" = "${INSTALL_FLAG}" ]
    then
        SetIpForPush
    else
        SetIp
    fi
    
    Log "Set port of OceanStor BCManager Agent."
    SetPort rdagent
    SetPort nginx

    Log "Change privilege of OceanStor BCManager Agent."
    ChangePrivilege

    Log "Set Auto start of OceanStor BCManager Agent."
    SetAutoStart

    Log "Start OceanStor BCManager Agent."
    StartAgent
    
    echo "OceanStor BCManager Agent was installed successfully."
    Log "OceanStor BCManager Agent was installed successfully."
    exit 0 
fi
