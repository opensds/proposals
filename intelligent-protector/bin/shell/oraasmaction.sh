#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

USAGE="Usage: ./oraasmaction.sh AgentRoot PID"

AGENT_ROOT_PATH=$1
PID=$2
IN_ORACLE_HOME=""
DBUser=""
GridUser=""

. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/oraclefunc.sh"

#********************************define these for the functions of agent_func.sh********************************
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/oraasmaction.log"
#for GetValue
ArgFile=$$
#for GetClusterType
DBISCLUSTER=0
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
# oracle temp file
ORATMPINFO="${TMP_PATH}/ORATMPINFO${PID}.txt"
#********************************define these for the functions of agent_func.sh********************************

#********************************define these for local script********************************

# ****ASM SQL
ASMSTARTSQL="${TMP_PATH}/StartupASMSQL${PID}.sql"
ASMSHUTDWONSQL="${TMP_PATH}/ShutdownASMSQL${PID}.sql"
ASMSTARTRST="${TMP_PATH}/StartupASMRST${PID}.txt"
ASMSHUTDOWNRST="${TMP_PATH}/ShutdownASMRST${PID}.txt"

# mount diskgroup
GETDISKGROUPSQL="${TMP_PATH}/GetDiskgroup${PID}.sql"
GETDISKGROUPSQLRST="${TMP_PATH}/GetDiskgroupRST${PID}.txt"
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"
#********************************define these for local script********************************

# **************************************** Create ASM stop SQL ***************************
CreateASMShutdownSql()
{
    echo "shutdown immediate;" > "$1"
    echo "exit" >> "$1"
}

# ************************** Stop +ASM instance ***********************************
ShutdownASMInstance()
{
    Log "Begin shutdown ASM[${ORA_VERSION}]"
    CreateASMShutdownSql ${ASMSHUTDWONSQL}
    ASM_LOG_AUTH="`CreateASMLoginCmd ${VERSION} ${AUTHMODE} \"${ASMAuthMode}\"`"

    Log "Exec ASM SQL to shutdown ASM instance."
    ASMExeSql "${ASM_LOG_AUTH}" ${ASMSHUTDWONSQL} ${ASMSHUTDOWNRST} ${ASMSIDNAME} 120
    RET_CODE=$?
    DeleteFile "${ASMSHUTDWONSQL}" "${ASMSHUTDOWNRST}"

    # check process
    ASMPROCESSNUM=`ps -ef | grep "asm_...._$ASMSIDNAME" | grep -v "grep" | wc -l`
    if [ ${ASMPROCESSNUM} -eq 0 ]
    then
        Log "End shutdown ASM[${ORA_VERSION}] success."
        return 0
    else
        Log "End shutdown ASM[${ORA_VERSION}] failed."
        return ${RET_CODE}
    fi
}

# ************************** Stop +ASM instance ***********************************
StopASMInstance()
{
    RET_CODE=0
    ASMPROCESSNUM=`ps -ef | grep "asm_...._$ASMSIDNAME" | grep -v "grep" | wc -l`
    
    # if no +ASM process, startup
    if [ ${ASMPROCESSNUM} -ne 0 ]
    then
        if [ "${ISCLUSTER}" = "1" ]
        then
            if [ "$VERSION" -ge "112" ]
            then
                Log "Begin stop RAC[${ORA_VERSION}]"
                "${ORA_CRS_HOME}/bin/crsctl" stop cluster -all 1>>"${LOG_FILE_NAME}" 2>&1
                Log "End stop RAC[${ORA_VERSION}]"
            elif [ "`RDsubstr $ORA_VERSION 1 2`" = "10" ] || [ "`RDsubstr $ORA_VERSION 1 2`" = "11" ]
            then
                Log "Begin stop RAC[${ORA_VERSION}]"
                "${ORA_CRS_HOME}/bin/crs_stop" -all 1>>"${LOG_FILE_NAME}" 2>&1
                Log "End stop RAC[${ORA_VERSION}]"
            else
                Log "Unsupport database version[${ORA_VERSION}]."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        else
            # check ASM instance auth mode user/pwd or /
            checkASMAuthMode
            
            ShutdownASMInstance
            Res=$?
            RetryNum=0
            while [ ${Res} -ne 0 ] 
            do
                if [ "${Res}" -eq "${ERROR_ORACLE_APPLICATION_OVER_MAX_LINK}" ] || [ "${Res}" -eq "${ERROR_ORACLE_ASM_DBUSERPWD_WRONG}" ] || [ "${Res}" -eq "${ERROR_ORACLE_ASM_INSUFFICIENT_WRONG}" ]
                then
                    Log "Shutdown ASM instance failed, error code:${Res}."
                    exit ${Res}
                fi
                
                if [ ${RetryNum} -ge 60 ]
                then
                    Log "shutdown ASM Instance have retry 60 times, shutdown ASM Instance failed."
                    exit $ERROR_SCRIPT_EXEC_FAILED
                fi
                
                sleep 5
                Log "try to shutdown ASM Instance[$RetryNum]"
                ShutdownASMInstance
                Res=$?
                RetryNum=`expr $RetryNum + 1`
            done
        fi
    else
        Log "+ASM has not started."
    fi
    
    return 0
}
#get oracle user name
getOracleUserInfo
if [ $? -ne 0 ]
then
    Log "Get Oracle user info failed."
    exit 1
fi

INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"
# *****************************Get dbname username and password**************************
GetValue "$INPUTINFO" InstanceName
ASMSIDNAME=${ArgValue}

GetValue "$INPUTINFO" ASMUserName
ASMUSER=${ArgValue}

GetValue "$INPUTINFO" ASMPassword
ASMUSERPWD=${ArgValue}

GetValue "$INPUTINFO" ASMDiskGroups
ASMGROUPNAMES=${ArgValue}

GetValue "$INPUTINFO" Action
CHECKTYPE=${ArgValue}

GetValue "$INPUTINFO" OracleHome 
IN_ORACLE_HOME=${ArgValue}

if [ "$ASMUSER" = "" ]
then
    AUTHMODE=1
    ASMAuthMode=/
else
    AUTHMODE=0
    ASMAuthMode=${ASMUSER}/\"${ASMUSERPWD}\"
fi
if [ -z "${ASMSIDNAME}" ]
then
    ASMSIDNAME="+ASM"
fi

#******************************
Log "ASMUSER=$ASMUSER;ASMGROUPNAMES=$ASMGROUPNAMES;ASMInstanceName=$ASMSIDNAME;AUTHMODE=$AUTHMODE;CHECKTYPE=${CHECKTYPE};IN_ORACLE_HOME=$IN_ORACLE_HOME"

#get user shell type
GetUserShellType

#get Oracle version
GetOracleVersion
VERSION=`echo $PREVERSION | tr -d '.'`
# get cluster flat
GetOracleCluster

ISCLUSTER=${DBISCLUSTER}
Log "Check cluster [ISCLUSTER=${ISCLUSTER}]."

# get ORA_CRS_HOME
GetORA_CRS_HOME ${VERSION}

# *****************************login in grid ************************************
if [ "$VERSION" -ge "112" ]
then
    if [ "${ISASM}" = "1" ] || [ "${ISCLUSTER}" = "1" ] 
    then
        Log "Begin su - ${GridUser}!"
        su - ${GridUser} -c "date"
        if [ "$?" != "0" ]
        then
            Log "Su - ${GridUser} error!"
            echo 1 > "$RESULTFILE"
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        Log "End su - ${GridUser}!"
        
        # ASM and cluster need to check ORA_CRS_HOME
        if [ -z "${ORA_CRS_HOME}" ]
        then
            Log "Orace 11gR2 \${ORA_CRS_HOME} is null."
            echo 1 > "$RESULTFILE"
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
    fi
fi

# check ASM instance
PREINSTANCE=`RDsubstr $ASMSIDNAME 1 1`
if [ "${PREINSTANCE}" != "+" ]
then
    Log "ASMINSTANCE=$ASMSIDNAME is not correct."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

if [ "$CHECKTYPE" = "0" ]
then
    # check cluster type
    if [ "${ISCLUSTER}" = "1" ]
    then
        Log "cluster is not support to start ASM instance"
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi

    StartASMInstance
    if [ "$?" -ne "0" ] 
    then
        Log "Start ASM instance failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
else
    StopASMInstance
    if [ "$?" -ne "0" ] 
    then
        Log "Stop ASM instance failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
fi

exit 0 
