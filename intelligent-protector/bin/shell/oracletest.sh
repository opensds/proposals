#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#@dest:  application agent for oracle
#@date:  2009-04-07
#@authr: 
USAGE="Usage: ./oracletest.sh AgentRoot PID"

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
#for Log
LOG_FILE_NAME="${LOG_PATH}/oracletest.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${TMP_PATH}"/ORCV"${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleInstanceStatus
CHECK_INSTANCE_STATUS="${TMP_PATH}/CheckInstanceStatus${PID}.sql"
CHECK_INSTANCE_STATUSRST="${TMP_PATH}/CheckInstanceStatusRST${PID}.txt"
#STARTED - After STARTUP NOMOUNT
#MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
#OPEN - After STARTUP or ALTER DATABASE OPEN
#OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
INSTANCESTATUS=""
AUTHMODE=0
DBUSER=""
DBUSERPWD=""
#for GetValue
ArgFile=$$
#********************************define these for the functions of agent_func.sh********************************

#********************************define these for local script********************************
DBNAME=""
DBINSTANCE=""

TESTQRYDBOPENMODEFILE="${TMP_PATH}/TestDBOpenMode${PID}.sql"
TESTQRYDBOPENMODERSTFILE="${TMP_PATH}/TestDBOpenModeRST${PID}.txt"
TESTQRYDBOPENMODE_READ="READ"
LEN_READ=4

QUERYNAMECRIPT="$TMP_PATH/CheckASM$PID.sql"
QUERYNAMECRIPTRST="$TMP_PATH/CheckASMRST$PID.txt"
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"
#********************************define these for local script********************************

#create the sql that get the open mode of the DB
CreatCheckDBReadSQL()
{
    echo "select open_mode from v\$database;" > "$1" 
    echo "exit" >> "$1"
}

#check database can be read
CheckDBReadability()
{
    TMPFILE=$1
    CHECKVAL=$2
    NAMEEQ=0
    TMPLINE=`cat "${TMPFILE}" | ${MYAWK} '{print $1}'`
    for i in ${TMPLINE} 
    do 
        LINE=$i
        NOWDBNAME=`echo $LINE | tr '[a-z]' '[A-Z]'`
        NOWDBNAME=`RDsubstr $NOWDBNAME 1 $LEN_READ`
        if [ "${NOWDBNAME}" = "${CHECKVAL}" ]
        then
            NAMEEQ=1
            break
        fi
    done
    if [ $NAMEEQ -eq 0 ]
    then
        return 0
    else
        return 1
    fi
}

TestDBConnect()
{
    Log "Start to check oracle instance status."
    GetOracleInstanceStatus "${DBINSTANCE}"
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Get instance status failed."
        exit ${RET_CODE}
    fi

    #STARTED - After STARTUP NOMOUNT
    #MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    #OPEN - After STARTUP or ALTER DATABASE OPEN
    #OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if [ ! "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
    then
        Log "Instance status($INSTANCESTATUS) no open."
        exit ${ERROR_INSTANCE_NOSTART}
    fi
    Log "end to check oracle instance status."

    # check v$database£¬ open_mode if have read
    CreatCheckDBReadSQL ${TESTQRYDBOPENMODEFILE}
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec SQL to check database status of read write."
    OracleExeSql "${LOGIN_AUTH}" "${TESTQRYDBOPENMODEFILE}" "${TESTQRYDBOPENMODERSTFILE}" "${DBINSTANCE}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Connect to database(${DBINSTANCE}) failed on test."
        DeleteFile "${TESTQRYDBOPENMODEFILE}"
        DeleteFile "${TESTQRYDBOPENMODERSTFILE}"
        exit ${RET_CODE}
    else
        CheckDBReadability "${TESTQRYDBOPENMODERSTFILE}" "${TESTQRYDBOPENMODE_READ}"
        if [ "$?" != "1" ]
        then
            Log "Check DB(${DBINSTANCE}) readability failed."
            DeleteFile "${TESTQRYDBOPENMODEFILE}"
            DeleteFile "${TESTQRYDBOPENMODERSTFILE}"
            exit $ERROR_SCRIPT_EXEC_FAILED
        else 
            DeleteFile "${TESTQRYDBOPENMODEFILE}"
            DeleteFile "${TESTQRYDBOPENMODERSTFILE}"
            Log "Connect to database(${DBINSTANCE}) successful on test."
            exit 0
        fi
    fi
}

TestASMInstanceConnect()
{
    #check ASM instance status
    ASMINSTNUM=`ps -ef | grep "asm_...._$DBINSTANCE" | grep -v "grep" | wc -l`
    if [ ${ASMINSTNUM} -eq 0 ]
    then
        Log "${DBINSTANCE} is not open."
        exit ${ERROR_ORACLE_ASM_INSTANCE_NOSTART}
    fi

    ASMSIDNAME=`ps -ef | grep "asm_...._$DBINSTANCE" | grep -v "grep" | ${MYAWK} -F '+' '{print $NF}'`
    ASMSIDNAME=`echo ${ASMSIDNAME} | ${MYAWK} -F " " '{print $1}'`
    ASMSIDNAME="+"${ASMSIDNAME}
    Log "Check ASM instance name $ASMSIDNAME."
    
    # get asm disk group memeber
    echo "select status from v\$instance;" > "${QUERYNAMECRIPT}"
    echo "exit" >> "${QUERYNAMECRIPT}"  
    ASM_LOGIN=${DBUSER}/\"${DBUSERPWD}\"
    ASM_LOG_AUTH="`CreateASMLoginCmd ${VERSION} ${AUTHMODE} \"${ASM_LOGIN}\"`"

    Log "Exec ASM SQL to check status of ASM instance."
    ASMExeSql "${ASM_LOG_AUTH}" "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${ASMSIDNAME}" 30
    RET_CODE=$?
    DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}"
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Test ASM instance failed."
        exit ${RET_CODE}
    else
        Log "Test ASM instance successful."
        exit 0
    fi
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
#############################################################
GetValue "$INPUTINFO" InstanceName
DBINSTANCE=$ArgValue

GetValue "$INPUTINFO" AppName
DBNAME=$ArgValue

GetValue "$INPUTINFO" UserName
DBUSERL=$ArgValue
DBUSER=`echo "$DBUSERL" | tr '[A-Z]' '[a-z]'`

GetValue "$INPUTINFO" Password
DBUSERPWD=$ArgValue

GetValue "$INPUTINFO" OracleHome
IN_ORACLE_HOME=$ArgValue

if [ "$DBUSERPWD" = "" ]
then
    AUTHMODE=1
fi

Log "DBNAME=$DBNAME;DBUSER=$DBUSER;DBINSTANCE=$DBINSTANCE;PID=${PID};AUTHMODE=$AUTHMODE;IN_ORACLE_HOME=$IN_ORACLE_HOME"

DBROLE=
if [ "${DBUSER}" = "sys" ]
then
    DBROLE="as sysdba"
fi

#get user shell type
GetUserShellType

#get Oracle version
GetOracleVersion
VERSION=`echo $PREVERSION | tr -d '.'`
PRE_INSTANCE=`RDsubstr $DBINSTANCE 1 1`
if [ "${PRE_INSTANCE}" = "+" ]
then
    TestASMInstanceConnect
else
    TestDBConnect
fi

exit 0
