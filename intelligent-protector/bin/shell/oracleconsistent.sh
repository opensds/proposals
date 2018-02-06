#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#@dest:  application agent for oracle10g
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
#for log
LOG_FILE_NAME="${LOG_PATH}/oracleconsistent.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
#for GetValue
ArgFile=$$
#for CrtTmpSql
DBUSER=""
DBUSERPWD=""
DBROLE=
AUTHMODE=0
#for GetBackupModeTBCount
TMPBKSCRIPT="${TMP_PATH}/TmpBkScrpit${PID}.sql"
TMPBKSCRIPTRST="${TMP_PATH}/TmpBkScrpitRST${PID}.txt"
BKFILENAME="${TMP_PATH}/BackupFile${PID}.txt"
#for GetArchiveLogMode
GET_ARCHIVE_LOG_MODE_SQL="${TMP_PATH}/get_archive_log_mode${PID}.sql"
ARCHIVE_LOG_MODE_RST="${TMP_PATH}/archive_log_mode_rst${PID}.txt"
QUERYCDBSCRIPT="$TMP_PATH/QueryCDB$PID.sql"
QUERYCDBSCRIPTRST="$TMP_PATH/QueryCDBRST$PID.txt"
DBINSTANCE=
ACTIVE_TB_COUNT=0
NOT_ACTIVE_TB_COUNT=0
#********************************define these for the functions of agent_func.sh********************************

#********************************define these for local script********************************
DBNAME=""

FREEZEFILEPRE="FreezeTblSpc"
FREEZESCRIPT="${TMP_PATH}/${FREEZEFILEPRE}${PID}.sql"
FREEZESCRIPTRST="${TMP_PATH}/${FREEZEFILEPRE}RST${PID}.txt"

THAWFILEPRE="ThawTbleSpc"
THAWSCRIPT="${TMP_PATH}/${THAWFILEPRE}${PID}.sql"
THAWSCRIPTRST="${TMP_PATH}/${THAWFILEPRE}RST${PID}.txt"
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

TRUNCATE_LOG_TIME=""
TRUNCFILEPRE="TruncArchLog"
TRUNCSCRIPT="${TMP_PATH}/${TRUNCFILEPRE}${PID}.rcv"
TRUNCSCRIPTRST="${TMP_PATH}/${TRUNCFILEPRE}RST${PID}.txt"
#********************************define these for local script********************************

CrtFreeDBSql()
{
    echo "alter database begin backup;" > "$1"
    echo "exit" >> "$1"
}

CrtThawDBSql()
{
    echo "alter database end backup;" > "$1"
    echo "exit" >> "$1"
}

CrtArchiveDBSql()
{
    echo "ALTER SYSTEM ARCHIVE LOG CURRENT;" > "$1"
    echo "exit" >> "$1"
}

CrtTruncateArchiveLogScript()
{
    echo "delete noprompt force archivelog until time \"to_date('${TRUNCATE_LOG_TIME}','yyyy-mm-dd-hh24-mi-ss')\";" > "$1"
    echo "exit" >> "$1"
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

GetValue "$INPUTINFO" TruncateLogTime
TRUNCATE_LOG_TIME=$ArgValue

GetValue "$INPUTINFO" OracleHome
IN_ORACLE_HOME=$ArgValue

# 0 freeze database
# 1 thaw database
# 2 search database backup mode
GetValue "$INPUTINFO" FrushType
FRUSH_TYPE=$ArgValue

if [ "$DBUSERPWD" = "" ]
then
    AUTHMODE=1
fi

if [ "${DBUSER}" = "sys" ]
then
    DBROLE="as sysdba"
fi

Log "DBNAME=$DBNAME;DBUSER=$DBUSER;DBINSTANCE=$DBINSTANCE;PID=${PID};AUTHMODE=$AUTHMODE;IN_ORACLE_HOME=$IN_ORACLE_HOME;TRUNCATE_LOG_TIME=$TRUNCATE_LOG_TIME"

#get user shell type
GetUserShellType

#query archive log mode
CheckArchiveLogMode()
{
    GetArchiveLogMode ${DBINSTANCE}
    RET_CODE=$?
    if [[ ${RET_CODE} -ne 0 ]] && [[ ${RET_CODE} -ne 1 ]]
    then
        Log "Get archive log mode failed."
        exit ${RET_CODE}
    fi
    
    if [ ${RET_CODE} -eq 0 ]
    then
        Log "Get archive log mode failed."
        exit ${ERROR_ORACLE_NOARCHIVE_MODE}
    fi
}

beginTime=`date +%s`
if [ "${FRUSH_TYPE}" = "0" ]
then
    #check archive log mode
    CheckArchiveLogMode
    
    #Freeze all database, if failed, then rollback.
    CrtFreeDBSql "${FREEZESCRIPT}"
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec SQL to freeze database."
    OracleExeSql "${LOGIN_AUTH}" "${FREEZESCRIPT}" "${FREEZESCRIPTRST}" "${DBINSTANCE}" 300 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile "${FREEZESCRIPT}" "${FREEZESCRIPTRST}"
    if [ ${RET_CODE} -eq 0 ]
    then
        #get the count of backup mode tablespace
        GetNotBackupModeTBCount
        if [ $? -eq 0 ]
        then
            if [ "${NOT_ACTIVE_TB_COUNT}" -eq "0" ]
            then
                Log "Freeze $DBNAME successful."
            else
                Log "Freeze $DBNAME failed, so start to rollback."

                #begin new timeout
                CrtThawDBSql ${THAWSCRIPT}
                LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
                OracleExeSql "${LOGIN_AUTH}" "${THAWSCRIPT}" "${THAWSCRIPTRST}" "${DBINSTANCE}" 600 >> "${LOG_FILE_NAME}" 2>&1
                RET_CODE=$?
                DeleteFile "${THAWSCRIPT}" "${THAWSCRIPTRST}"
                if [ ${RET_CODE} -eq 0 ]
                then
                    Log "Freeze $DBNAME failed, and rollback succ."
                else
                    Log "Freeze $DBNAME failed, but rollback failed."
                fi
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        else
            Log "Get not backup mode tablespace failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
    else
        endTime=`date +%s`
        execTime=`expr $endTime - $beginTime`
        
        #if not in backup before, begin to end backup mode
        if [ "${ERROR_ORACLE_DB_ALREADY_INBACKUP}" -ne "${RET_CODE}" ]
        then
            Log "Freeze $DBNAME failed, so start to rollback."
            #begin new timeout
            CrtThawDBSql ${THAWSCRIPT}
            LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
            OracleExeSql "${LOGIN_AUTH}" "${THAWSCRIPT}" "${THAWSCRIPTRST}" "${DBINSTANCE}" 600 >> "${LOG_FILE_NAME}" 2>&1
            THAW_RET_CODE=$?
            DeleteFile "${THAWSCRIPT}" "${THAWSCRIPTRST}"
            if [ ${THAW_RET_CODE} -eq 0 ]
            then
                Log "Freeze $DBNAME failed, and thaw succ."
            else
                Log "Freeze $DBNAME failed, but thaw failed."
            fi
            
            # timeout
            if [ "${execTime}" -gt "300" ]
            then
                exit ${ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT}
            else
                exit ${RET_CODE}
            fi
        else
            exit ${ERROR_ORACLE_DB_ALREADY_INBACKUP}
        fi
    fi
elif [ "${FRUSH_TYPE}" = "1" ]
then
    #check archive log mode
    CheckArchiveLogMode
    
    #Thaw all database.
    CrtThawDBSql "${THAWSCRIPT}"
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec SQL to thaw database."
    OracleExeSql "${LOGIN_AUTH}" "${THAWSCRIPT}" "${THAWSCRIPTRST}" "${DBINSTANCE}" 600 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile "${THAWSCRIPT}" "${THAWSCRIPTRST}"
    if [ ${RET_CODE} -eq 0 ]
    then
        #get the count of backup mode tablespace
        GetBackupModeTBCount
        if [ $? -eq 0 ]
        then
            if [ "${ACTIVE_TB_COUNT}" -eq "0" ]
            then
                Log "Thaw $DBNAME successful."
            else
                Log "Exec thaw $DBNAME succ, but these are tablespace these alread in backup mode."
                exit ${ERROR_ORACLE_END_HOT_BACKUP_FAILED}
            fi
        else
            Log "Exec thaw $DBNAME succ, but get count of backupup mode tablespace failed."
            exit ${ERROR_ORACLE_END_HOT_BACKUP_FAILED}
        fi
    else
        endTime=`date +%s`
        execTime=`expr $endTime - $beginTime`
        Log "Exec thaw $DBNAME failed."
        exit ${RET_CODE}
    fi
elif [ "${FRUSH_TYPE}" = "2" ]
then
    if [ "$sysName" = "HP-UX" ]
    then
        SG_CONF_DIR="/etc/cmcluster"
        if [ -d "$SG_CONF_DIR" ]
        then
            GetOracle_homeOfSG $SG_CONF_DIR $DBINSTANCE
        fi
    fi
    # hot backup        : 0  
    # not hot backup    : 1  
    # return failed     : 2
    # check if there are tablespaces that is in backup mode
    GetBackupModeTBCount
    if [ $? -eq 0 ]
    then
        if [ "${ACTIVE_TB_COUNT}" -eq "0" ]
        then
            Log "There are no backup tablespace."
            echo ${ERROR_DB_FREEZE_NO} >> "${RESULT_FILE}"
            exit 0
        else
            Log "Database is in hot backup mode."
            echo ${ERROR_DB_FREEZE_YES} >> "${RESULT_FILE}"
            exit 0
        fi
    else
        Log "Get the count of backup mode tablespace failed."
        echo ${ERROR_DB_FREEZE_UNKNOWN} >> "${RESULT_FILE}"
        exit 0
    fi
elif [ "${FRUSH_TYPE}" = "3" ]
then
    #archive database
    CrtArchiveDBSql ${THAWSCRIPT}
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec SQL to archive database."
    OracleExeSql "${LOGIN_AUTH}" ${THAWSCRIPT} ${THAWSCRIPTRST} ${DBINSTANCE} 600 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile ${THAWSCRIPT} ${THAWSCRIPTRST}
    if [ ${RET_CODE} -ne 0 ]
    then
        Log "Archive database-${DBINSTANCE} failed, error=${RET_CODE}."
        if [ "${ERROR_SCRIPT_EXEC_FAILED}" = "${RET_CODE}" ] 
        then
            exit ${ERROR_ORACLE_DB_ARCHIVEERROR}
        else
            exit ${RET_CODE}
        fi
    else
        Log "Archive database-${DBINSTANCE} succ."
    fi
elif [ "${FRUSH_TYPE}" = "4" ]
then
    #delete archivelog of database
    CrtTruncateArchiveLogScript ${TRUNCSCRIPT}
    RMAN_AUTH="`CreateRmanLoginCmd ${AUTHMODE} ${DBUSER} ${DBUSERPWD}`"
    Log "Exec RMAN to delete archivelog of database."
    RmanExeScript "${RMAN_AUTH}" ${TRUNCSCRIPT} ${TRUNCSCRIPTRST} ${DBINSTANCE} 600 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile ${TRUNCSCRIPT} ${TRUNCSCRIPTRST}
    if [ ${RET_CODE} -ne 0 ]
    then
        Log "Delete archivelog of database-${DBINSTANCE} failed, error=${RET_CODE}."
        if [ "${ERROR_SCRIPT_EXEC_FAILED}" = "${RET_CODE}" ] 
        then
            exit ${ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED}
        else
            exit ${RET_CODE}
        fi
    else
        Log "Delete archivelog of database-${DBINSTANCE} succ."
    fi
elif [ "${FRUSH_TYPE}" = "5" ]
then
    #query archive log mode
    GetArchiveLogMode ${DBINSTANCE}
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ] && [ ${RET_CODE} -ne 1 ]
    then
        Log "Get archive log mode failed."
        exit ${RET_CODE}
    fi

    echo ${RET_CODE} >> "${RESULT_FILE}"
elif [ "${FRUSH_TYPE}" = "6" ]
then
    #check oracle is installed
    CheckOracleIsInstall
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        exit ${RET_CODE}
    fi
    Log "Check oracle install succ."
else
    Log "FrushType($FRUSH_TYPE) is wrong."
    exit $ERROR_PARAM_WRONG
fi

exit 0

