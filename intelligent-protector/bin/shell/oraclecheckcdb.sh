#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

############################################################################################
#program name:          
#function:              
#author:
#time:                  
#function and description:  
# function             
# rework:               
# author:
# time:
# explain:             
############################################################################################

USAGE="Usage: ./oracleluninfo.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclecheckcdb.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleInstanceStatus
CHECK_INSTANCE_STATUS="${TMP_PATH}/CheckInstanceStatus${PID}.sql"
CHECK_INSTANCE_STATUSRST="${TMP_PATH}/CheckInstanceStatusRST${PID}.txt"
ARCHIVEDESTSQL="${TMP_PATH}/ArchiveDestSQL${PID}.sql"
ARCHIVESTATUSLOGSQL="${TMP_PATH}/ArchiveStatusSQL${PID}.sql"
ARCHIVESTATUSRST="${TMP_PATH}/ArchiveStatusRST${PID}.txt"
ARCHIVEDESTRST="${TMP_PATH}/ArchiveDestRST${PID}.txt"
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
#for GetArchiveLogMode
GET_ARCHIVE_LOG_MODE_SQL="${TMP_PATH}/get_archive_log_mode${PID}.sql"
ARCHIVE_LOG_MODE_RST="${TMP_PATH}/archive_log_mode_rst${PID}.txt"
#********************************define these for the functions of agent_func.sh********************************

#********************************define these for local script********************************
RET_RESULT_CDB=1
RET_RESULT_NOT_CDB=0
EXIT_CODE_NORMAL=0

#######################################set file name##################################
QUERYCDBSCRIPT="$TMP_PATH/QueryCDB$PID.sql"
QUERYCDBSCRIPTRST="$TMP_PATH/QueryCDBRST$PID.sql"
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

#get oracle user name
getOracleUserInfo
if [ $? -ne 0 ]
then
    Log "Get Oracle user info failed."
    exit 1
fi

INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"
#################Entry of script to query the information of oracle portfolio###########
GetValue "$INPUTINFO" InstanceName
DBINSTANCE=$ArgValue

GetValue "$INPUTINFO" AppName
DBNAME=$ArgValue

GetValue "$INPUTINFO" UserName
DBUSERL=$ArgValue
DBUSER=`echo "$DBUSERL" | tr '[A-Z]' '[a-z]'`

GetValue "$INPUTINFO" Password
DBUSERPWD=$ArgValue

GetValue "$INPUTINFO" TableSpaceName
DBTABLESPACENAME=$ArgValue

GetValue "$INPUTINFO" ASMUserName
ASMUSER=$ArgValue

GetValue "$INPUTINFO" ASMPassword
ASMUSERPWD=$ArgValue

GetValue "$INPUTINFO" ASMInstanceName
ASMINSTANCENAME=$ArgValue
if [ -z "${ASMINSTANCENAME}" ]
then
    ASMINSTANCENAME="+ASM"
fi

GetValue "$INPUTINFO" OracleHome
IN_ORACLE_HOME=$ArgValue

AUTHMODE=0
if [ "$DBUSERPWD" = "" ]
then
    AUTHMODE=1
fi

DBROLE=
if [ "${DBUSER}" = "sys" ]
then
    DBROLE="as sysdba"
fi

Log "SubAppName=$DBINSTANCE;AppName=$DBNAME;UserName=$DBUSER;TableSpaceName=$DBTABLESPACENAME;ASMUserName=$ASMUSER;ASMInstanceName=$ASMINSTANCENAME;AUTHMODE=$AUTHMODE;IN_ORACLE_HOME=$IN_ORACLE_HOME"

CheckCDB()
{
    Log "Start to check CDB."

    #************************temporary sql script for quering file_name*****************
    echo 'select name,cdb from v$database;' > "$QUERYCDBSCRIPT"
    echo "exit" >> "$QUERYCDBSCRIPT"

    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"

    Log "Exec SQL to get files of database."
    OracleExeSql "${LOGIN_AUTH}" "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}" "${DBINSTANCE}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Search database(${DBINSTANCE}) file failed."
        DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
        exit ${RET_CODE}
    fi
    
    #****************************get db location********************************
    if [ `cat "$QUERYCDBSCRIPTRST" | wc -l` -eq 0 ]
    then
        DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
        Log "QueryTableSpacePath:Connect to database $DBNAME failed, result is null."
        exit ${ERROR_SCRIPT_EXEC_FAILED}
    fi

    # analyse database file list
    # TODO
    IS_ILLEGAL_SQL=0
    while read line
    do 
        line_arg1=`echo $line | awk '{print $1}'`
        line_arg2=`echo $line | awk '{print $2}'`
        
        echo "line_arg1=${line_arg1} line_arg2=${line_arg2}" >> "${LOG_FILE_NAME}" 2>&1
        if [[ ${line_arg1} = "NAME" ]] && [[ ${line_arg2} = "CDB" ]];
        then
            IS_ILLEGAL_SQL=1
            continue;
        fi
        
        if [[ ${line_arg2} = "NO" ]] && [[ ${IS_ILLEGAL_SQL} -eq 1 ]];
        then
            echo $RET_RESULT_NOT_CDB > ${RESULT_FILE}
            DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
            Log "end to check CDB."
            exit ${EXIT_CODE_NORMAL}
        fi
        
        if [[ ${line_arg2} = "YES" ]] && [[ ${IS_ILLEGAL_SQL} -eq 1 ]];
        then
            echo $RET_RESULT_CDB > ${RESULT_FILE}
            DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
            Log "end to check CDB."
            exit ${EXIT_CODE_NORMAL}
        fi
    done < $QUERYCDBSCRIPTRST;

    DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
    
    Log "Failed to check CDB."
    exit ${ERROR_SCRIPT_EXEC_FAILED}
}


CheckInstanceStatus()
{
    Log "Start to check oracle instance status."
    GetOracleInstanceStatus ${DBINSTANCE}
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
}


CheckOracleVersion()
{
    Log "Start to check oracle version."
    #get Oracle version and check if it's 12
    GetOracleVersion
    VERSION=`echo $PREVERSION | $MYAWK -F "." '{print $1}'`
    
    if [[ "${VERSION}" = "10" ]] || [[ "${VERSION}" = "11" ]];
    then
        Log "Oracle version($ORA_VERSION) not supports CDB."
        echo $RET_RESULT_NOT_CDB > ${RESULT_FILE}
        exit ${EXIT_CODE_NORMAL}
    fi

    if [ ! "${VERSION}" = "12" ]
    then
        Log "Oracle version($ORA_VERSION) is not supported."
        exit ${ERROR_SCRIPT_EXEC_FAILED}
    fi
    Log "end to check oracle version."
}

GetUserShellType

CheckInstanceStatus

CheckOracleVersion

CheckCDB