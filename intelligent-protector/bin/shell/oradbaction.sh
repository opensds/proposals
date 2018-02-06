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
LOG_FILE_NAME="${LOG_PATH}/oradbaction.log"
#for GetValue
ArgFile=$$
#for GetClusterType
DBISCLUSTER=0
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
IS_INCLUDE_ARCH=
#for GetBackupModeTBCount
TMPBKSCRIPT="${TMP_PATH}/TmpBkScrpit${PID}.sql"
TMPBKSCRIPTRST="${TMP_PATH}/TmpBkScrpitRST${PID}.txt"
BKFILENAME="${TMP_PATH}/BackupFile${PID}.txt"
DBINSTANCE=
ACTIVE_TB_COUNT=0
NOT_ACTIVE_TB_COUNT=0
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
ARCHIVE_DEST_LIST="${TMP_PATH}/ArchiveDestList${PID}.txt"

RECOVERSCRIPT="${TMP_PATH}/RecoverSql$PID.sql"
SHUTDOWNSCRIPT="${TMP_PATH}/ShutdownSql$PID.sql"
STARTUPSCRIPT="${TMP_PATH}/StartupSql$PID.sql"
OPENDBSCRIPT="${TMP_PATH}/OpenDbSql$PID.sql"
# ************************** 2012-05-14 modify problem: ********************
STARTRECOVERSCRIPT="${TMP_PATH}/ExecRecover$PID.sql"

STARTUPRST="${TMP_PATH}/StartupTmp$PID.txt"
RECOVERRST="${TMP_PATH}/RecoverTmp$PID.txt"
SHUTDOWNRST="${TMP_PATH}/ShutdownTmp$PID.txt"
OPENDBRST="${TMP_PATH}/OpenDbTmp$PID.txt"
# ************************** 2012-05-14 modify problem: ********************
STARTRECOVERRST="${TMP_PATH}/RecoverDbTmp$PID.txt"

# ****ASM SQL
ASMSTARTSQL="${TMP_PATH}/StartupASMSQL${PID}.sql"
ASMSHUTDWONSQL="${TMP_PATH}/ShutdownASMSQL${PID}.sql"
ASMSTARTRST="${TMP_PATH}/StartupASMRST${PID}.txt"
ASMSHUTDOWNRST="${TMP_PATH}/ShutdownASMRST${PID}.txt"

# Archive Log
ARCHIVEDESTSQL="${TMP_PATH}/ArchiveDestSQL${PID}.sql"
ARCHIVEDESTRST="${TMP_PATH}/ArchiveDestRST${PID}.txt"

# mount diskgroup
GETDISKGROUPSQL="${TMP_PATH}/GetDiskgroup${PID}.sql"
GETDISKGROUPSQLRST="${TMP_PATH}/GetDiskgroupRST${PID}.txt"

# check database flashback status
FLASHBACKSTATUS="${TMPPATH}/flashbackStatus${PID}.sql"
FLASHBACKSTATUSRST="${TMPPATH}/flashbackStatusRST${PID}.txt"
FLASHBACKTATUS=""

# check cluster status
CHECKCLUSTERSTATUS="${TMP_PATH}/CheckClustereStatus${PID}.txt"
CLUSTER_START_NUM=600
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

# RAC whether start db
FLAG_RACSTARTDB=0
#********************************define these for local script********************************

# **************************************** Create ShutDown SQL ************************
CreateShutdownSql()
{
    echo "shutdown immediate;" > "$1"
    echo "exit" >> "$1"
}
# **************************************** Create Recover SQL *************************
CreateRecoverSql()
{
    echo "startup nomount;" > "$1"
    echo "exit" >> "$1"
}

CreateEndBackupSql()
{
    echo "alter database end backup;" > "$1"
    echo "exit" >> "$1"
}

# **************************************** Create Startup SQL *************************
CreateStartupSql()
{
    echo "alter database mount;" > "$1"
    echo "exit" >> "$1"
}
# **************************************** Create Mount SQL ***************************
CreateOpenSql()
{
    echo "alter database open;" > "$1"
    echo "exit" >> "$1"
}

#rem ************************************************************************
#rem function name: CreateArchiveDirectory
#rem aim:           create archive directory
#rem input:         
#rem output:        
#rem ************************************************************************
CreateArchiveDirectory()
{
    # create check archive mode sql
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Begin check archive dest directory."
    echo "set linesize 300;" > "${ARCHIVEDESTSQL}"
    echo "col DESTINATION for a255;" >> "${ARCHIVEDESTSQL}"
    echo "select DESTINATION from v\$archive_dest where STATUS <> 'INACTIVE';" >> "${ARCHIVEDESTSQL}"
    echo "exit" >> "${ARCHIVEDESTSQL}"

    Log "Exec SQL to get archive dest."
    OracleExeSql "${LOGIN_AUTH}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" 60 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
        Log "Get Archive log dest list failed."
        exit ${RET_CODE}
    fi
        
    DeleteFile "${ARCHIVE_DEST_LIST}"
    touch "${ARCHIVE_DEST_LIST}"
    cat "${ARCHIVEDESTRST}" | grep "^/" >> "${ARCHIVE_DEST_LIST}"
    cat "${ARCHIVEDESTRST}" | grep "^+" >> "${ARCHIVE_DEST_LIST}"
    cat "${ARCHIVEDESTRST}" | grep "USE_DB_RECOVERY_FILE_DEST" >> "${ARCHIVE_DEST_LIST}"
    DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
    
    while read line
    do
        STRARCHIVEDEST=$line
        if [ -z "${STRARCHIVEDEST}" ]
        then
            continue
        fi
        
        if [ "`RDsubstr $STRARCHIVEDEST 1 1`" = "/" ]
        then
            # check last char, if it is /, create directory, or not /, remove last / char to end
            # eg. /home/abc/ => create directory /home/abc/
            # eg. /home/abc => create /home
            
            LENSTR=`echo $STRARCHIVEDEST | ${MYAWK} '{print length($1)}'`
            LASTCHAR=`RDsubstr $STRARCHIVEDEST $LENSTR 1`
            if [ "${LASTCHAR}" != "/" ]
            then
                STRARCHIVEDEST=`dirname ${STRARCHIVEDEST}`
            fi
            
            # -----mkdir archive directory------
            if [ ! -d "${STRARCHIVEDEST}" ]
            then
                Log "Begin create directory [${STRARCHIVEDEST}]."
                mkdir "${STRARCHIVEDEST}"
                groupid=`id -g ${DBUser}`
                chown ${DBUser}:$groupid "${STRARCHIVEDEST}"
                Log "Create directory [${STRARCHIVEDEST}] success."
            else
                Log "Directory [${STRARCHIVEDEST}] exists."
            fi
        else
            Log "Archive dest ${STRARCHIVEDEST} is not filesystem."
            DeleteFile "${ARCHIVE_DEST_LIST}"
            return 0
        fi
    done < "${ARCHIVE_DEST_LIST}"
    
    DeleteFile "${ARCHIVE_DEST_LIST}"
    Log "End create archive dest directory."
    return 0
}

# ************************** Stop single db instance ***********************************
StopSingleInstance()
{    
    # **************************Create ShutDown DataBase Sql****************************************
    CreateShutdownSql $SHUTDOWNSCRIPT
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    # **************************Check Instance is Exist******************************************
    HAVEPROCESS=`ps -aef | grep -v grep | sed 's/ *$//g' | grep "ora_...._$DBINSTANCE$" |wc -l`
    if [ "$HAVEPROCESS" -ne "0" ]
    then
        Log "Exec SQL to shutdown database."
        OracleExeSql "${LOGIN_AUTH}" "${SHUTDOWNSCRIPT}" "${SHUTDOWNRST}" "${DBINSTANCE}" 300 >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        DeleteFile "${SHUTDOWNSCRIPT}" "${SHUTDOWNRST}"
        if [ "${RET_CODE}" -ne "0" ] && [ "${RET_CODE}" -ne "${ERROR_ORACLE_NOT_MOUNTED}" ] && [ "${RET_CODE}" -ne "${ERROR_ORACLE_NOT_OPEN}" ]
        then
            Log "Shutdown database(${DBINSTANCE}) file failed."
            exit ${RET_CODE}
        fi

        Log "shutdown database successful."
    else
        Log "The Instance isn't Started."
    fi
    return 0
}
# ************************** Stop single db instance ***********************************

# ************************** Start single db instance ***********************************
StartSingleInstance()
{
    # check database whether is started.
    GetOracleInstanceStatus ${DBINSTANCE}
    RET_CODE=$?
    if [ "${RET_CODE}" -eq "0" ]
    then
        #STARTED - After STARTUP NOMOUNT
        #MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        #OPEN - After STARTUP or ALTER DATABASE OPEN
        #OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
        if [ "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
        then
            Log "The status of database is open."
            exit 0
        fi
    fi

    # *****************************start the ASM instance when asm instance *************
    if [ "${ISASM}" = "1" ]
    then
        StartASMInstance
    fi

    SINGLE_NUM=0
    #After mount asm diskgroup, oracle will start instance automatic in the sometime
    #Prevent instance starting conflict, start script will monitor the intance process in 10 seconds.
    while [ $SINGLE_NUM -lt 10 ]
    do
        sleep 1
        SINGLE_NUM=`expr $SINGLE_NUM + 1`
        HAVEPROCESS=`ps -aef | grep -v grep | sed 's/ *$//g' | grep "ora_...._$DBINSTANCE$" |wc -l`
        if [ "${HAVEPROCESS}" -ne "0" ]
        then
            GetOracleInstanceStatus ${DBINSTANCE}
            IRET=$?
            if [ $IRET -ne 0 ]; then
                # instance status is not ready or instance is closed
                #Clear resource after cluster start failed, if starting the single instance, it will be stopped by cluster.
                Log "After mount diskgroup, start instance failed, waiting to start instance manual."
                continue
            else
                if [ "`RDsubstr $INSTANCESTATUS 1 4`" != "OPEN" ]; then
                    Log "The status of database is not open."
                    continue
                fi
                # instace is opened
                Log "After mount diskgroup, monitor instance, now database is open."
                return 0
            fi
        fi
    done
        
    # startup nomount
    StartUpNoMountDatabase
    Res=$?
    RetryNum=0
    while [ ${Res} -ne 0 ] 
    do
        if [ ${RetryNum} -ge 5 ]
        then
            Log "start nomount database have retry 5 times, start nomount database failed."
            exit $Res
        fi
        
        sleep 10
        Log "try to startup nomount[$RetryNum]"
        StartUpNoMountDatabase
        Res=$?
        RetryNum=`expr $RetryNum + 1`
    done    
    
    # mount database
    MountDatabase
    Res=$?
    RetryNum=0
    while [ ${Res} -ne 0 ] 
    do
        if [ ${RetryNum} -ge 5 ]
        then
            Log "alter database mount have retry 5 times, alter database mount failed."
            exit $Res
        fi
        
        sleep 10
        Log "try to alter database mount[$RetryNum]"
        MountDatabase
        Res=$?
        RetryNum=`expr $RetryNum + 1`
    done    
 
    
    # check flashback and turn off
    TurnDatabaseFlashBackOff
    Res=$?
    RetryNum=0
    while [ ${Res} -ne 0 ] 
    do
        if [ ${RetryNum} -ge 5 ]
        then
            Log "alter database flashback off have retry 5 times, alter database flashback off failed."
            exit $Res
        fi
        
        sleep 10
        Log "try to turn database flashback off[$RetryNum]"
        TurnDatabaseFlashBackOff
        Res=$?
        RetryNum=`expr $RetryNum + 1`
    done
    
    # Recover database
    RecoverDatabase ${IS_INCLUDE_ARCH}
    Res=$?
    RetryNum=0
    while [ ${Res} -ne 0 ] 
    do
        if [ ${RetryNum} -ge 5 ]
        then
            Log "recover database have retry 5 times, recover database failed."
            exit $Res
        fi
        
        sleep 10
        Log "try to recover database[$RetryNum]"
        RecoverDatabase
        Res=$?
        RetryNum=`expr $RetryNum + 1`
    done     

    # open database
    OpenDatabase
    Res=$?
    RetryNum=0
    while [ ${Res} -ne 0 ] 
    do
        if [ ${RetryNum} -ge 5 ]
        then
            Log "alter database open have retry 5 times, alter database open failed."
            exit $Res
        fi
        
        sleep 10
        Log "try to open database[$RetryNum]"
        OpenDatabase
        Res=$?
        RetryNum=`expr $RetryNum + 1`
    done     
}

# ************************** Start single db instance ***********************************
StartUpNoMountDatabase()
{
    CreateRecoverSql $RECOVERSCRIPT
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    # **********************************check database have start************************
    HAVEPROCESS=`ps -aef | grep -v grep | sed 's/ *$//g' | grep "ora_...._$DBINSTANCE$" |wc -l`   
    if [ "${HAVEPROCESS}" -ne "0" ]
    then
        Log "The Database ($DBNAME) has started."    
        DeleteFile "$RECOVERSCRIPT"
    else
        Log "Exec SQL to startup nomount database."
        OracleExeSql "${LOGIN_AUTH}" "${RECOVERSCRIPT}" "${RECOVERRST}" "${DBINSTANCE}" 120 >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        DeleteFile "${RECOVERSCRIPT}" "${RECOVERRST}"
        if [ "${RET_CODE}" -eq "${ERROR_ORACLE_DB_ALREADYRUNNING}" ]
        then
            Log "The Database ($DBNAME) already started."
        elif [ "${RET_CODE}" -eq "0" ]
        then
            Log "Start DataBase ($DBNAME) successful."
        elif [ "${RET_CODE}" -eq "${ERROR_ORACLE_ANOTHER_STARTING}" ]
        then
            Log "Start DataBase ($DBNAME) failed, another process is starting."
            return 1
        elif [ "${RET_CODE}" -eq "${ERROR_ORACLE_DB_BUSY}" ]
        then
            Log "Start DataBase ($DBNAME) failed, database is busy."
            return 1
        else
            Log "Startup nomount database(${DBNAME}) file failed."
            exit ${RET_CODE}
        fi
    fi
    return 0
}

MountDatabase()
{
    # check instance status, it is OK when the status can be get
    GetOracleInstanceStatus ${DBINSTANCE}
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Get instance status failed."
        return $RET_CODE
    fi
    
    CreateStartupSql ${STARTUPSCRIPT}
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec SQL to mount database."
    OracleExeSql "${LOGIN_AUTH}" "${STARTUPSCRIPT}" "${STARTUPRST}" "${DBINSTANCE}" 120 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile "${STARTUPSCRIPT}" "${STARTUPRST}"
    if [ "${RET_CODE}" -eq "${ERROR_ORACLE_DB_ALREADYMOUNT}" ]
    then
        Log "The Database ($DBNAME) already mounted."
    elif [ "${RET_CODE}" -eq "0" ]
    then
        Log "Mount DataBase ($DBNAME) successful."
    elif [ "${RET_CODE}" -eq "${ERROR_ORACLE_ANOTHER_STARTING}" ]
    then
        Log "Start DataBase ($DBNAME) failed, another process is starting."
        return 1
    elif [ "${RET_CODE}" -eq "${ERROR_ORACLE_DB_BUSY}" ]
    then
        Log "Start DataBase ($DBNAME) failed, database is busy."
        return 1
    else
        Log "Mount database(${DBNAME}) file failed."
        exit ${RET_CODE}
    fi
    return 0
}

TurnDatabaseFlashBackOff()
{
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    echo "select flashback_on from v\$database;" > ${FLASHBACKSTATUS}
    echo "exit" >> ${FLASHBACKSTATUS}
    
    Log "Exec SQL to get database flashback status."
    OracleExeSql "${LOGIN_AUTH}" "${FLASHBACKSTATUS}" "${FLASHBACKSTATUSRST}" "${DBINSTANCE}" 120 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" != "0" ]
    then         
        DeleteFile "${FLASHBACKSTATUS}" "${FLASHBACKSTATUSRST}"
        return $ERROR_SCRIPT_EXEC_FAILED
    else
        # modify temporary.
        FLASHBACKTATUS=`sed -n '/----------/,/^ *$/p' "${FLASHBACKSTATUSRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1}'`
        DeleteFile "${FLASHBACKSTATUS}" "${FLASHBACKSTATUSRST}"
    fi
    
    # database flashback is not on
    Log "FLASHBACKTATUS=${FLASHBACKTATUS}"
    if [ "${FLASHBACKTATUS}" = "NO" ]
    then
        Log "Database flashback is off."
        return 0
    fi

    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    echo "alter database flashback off;" > ${FLASHBACKSTATUS}
    echo "exit" >> ${FLASHBACKSTATUS}
    
    Log "Exec SQL to turn flashback off."
    OracleExeSql "${LOGIN_AUTH}" "${FLASHBACKSTATUS}" "${FLASHBACKSTATUSRST}" "${DBINSTANCE}" 120 >> "${LOG_FILE_NAME}" 2>&1
    DeleteFile "${FLASHBACKSTATUS}" "${FLASHBACKSTATUSRST}"
    RET_CODE=$?
    if [ "$RET_CODE" != "0" ]
    then
        Log "turn off database flashback failed."
        return $ERROR_SCRIPT_EXEC_FAILED
    else
        Log "turn off database flashback succ."
        return 0
    fi
}

RecoverDatabase()
{
    Log "Begin recover database ${DBINSTANCE}."
    # check if there are tablespaces that is in backup mode
    GetBackupModeTBCount
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Get count of backup mode tablespace failed."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi

    if [ "${ACTIVE_TB_COUNT}" -eq "0" ]
    then
        Log "There are no backup tablespace, recover database successful."
        return 0
    fi

    Log "Create end backup sql script file."
    CreateEndBackupSql ${STARTRECOVERSCRIPT}
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec sql to recover database."
    OracleExeSql "${LOGIN_AUTH}" "${STARTRECOVERSCRIPT}" "${STARTRECOVERRST}" "${DBINSTANCE}" 120 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile "${STARTRECOVERSCRIPT}" "${STARTRECOVERRST}" 
    if  [ "${RET_CODE}" -eq "0" ]
    then
        Log "Recover database ($DBNAME) successful."
    else
        Log "Recover database(${DBNAME}) failed."
        exit ${RET_CODE}
    fi

    return 0
}

OpenDatabase()
{    
    #***********************************Open Database **********************
    CreateOpenSql $OPENDBSCRIPT

    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec SQL to open database."
    OracleExeSql "${LOGIN_AUTH}" "${OPENDBSCRIPT}" "${OPENDBRST}" "${DBINSTANCE}" 120 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile "${OPENDBSCRIPT}" "${OPENDBRST}"
    if [ "${RET_CODE}" -eq "${ERROR_ORACLE_DB_ALREADYOPEN}" ]
    then
        Log "The Database ($DBNAME) already open."
    elif [ "${RET_CODE}" -eq "0" ]
    then
        Log "Open DataBase ($DBNAME) successful."
    elif [ "${RET_CODE}" -eq "${ERROR_ORACLE_ANOTHER_STARTING}" ]
    then
        Log "Start DataBase ($DBNAME) failed, another process is starting."
        return 1
    elif [ "${RET_CODE}" -eq "${ERROR_ORACLE_DB_BUSY}" ]
    then
        Log "Start DataBase ($DBNAME) failed, database is busy."
        return 1
    else
        Log "Open database(${DBNAME}) file failed."
        exit ${RET_CODE}
    fi

    return 0
}

# ************************** Stop RAC db instance ***********************************
StopRACInstance()
{
    if [ -z "${ORA_CRS_HOME}" ] 
    then
        Log "Oracle grid home[\${ORA_CRS_HOME}] path is null."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi

    # shutdown database
    # **************************Check Instance is Exist******************************************
    HAVEPROCESS=`ps -aef | grep -v grep | sed 's/ *$//g' | grep "ora_...._$DBINSTANCE$" |wc -l`
    if [ "$HAVEPROCESS" -ne "0" ]
    then
        # **************************Create ShutDown DataBase Sql****************************************
        LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
        CreateShutdownSql $SHUTDOWNSCRIPT

        Log "Exec SQL to shutdown database."
        OracleExeSql "${LOGIN_AUTH}" "${SHUTDOWNSCRIPT}" "${SHUTDOWNRST}" "${DBINSTANCE}" 300 >> "${LOG_FILE_NAME}" 2>&1
        RET_CODE=$?
        DeleteFile "${SHUTDOWNSCRIPT}" "${SHUTDOWNRST}"
        if [ "${RET_CODE}" -ne "0" ]
        then
            Log "Shutdown database(${DBINSTANCE}) file failed, errorcode=${RET_CODE}."
        else
            Log "shutdown database successful."
        fi
    else
        Log "The Instance isn't Started."
    fi
    
    # bug list
    # Bug 12387330 : SOL-SP64-11203 'CRSCTL STOP CRS' FAIL DUE TO FAILE TO STOP DG RESOURCE 
    # Bug 13827808 : ORA-15032/ORA-15027 OCCUR ON ASM DURING SHUTDOWN CRS STACK 
    # start database against the status of database is not consistent, database is online but crs_stat is offline.
    ${ORA_CRS_HOME}/bin/srvctl stop database -d ${DBNAME} 1>>"$LOG_FILE_NAME" 2>&1
    
    if [ "$VERSION" -ge "112" ]
    then
        Log "Begin stop RAC[${ORA_VERSION}]"
        ${ORA_CRS_HOME}/bin/crsctl stop cluster -all 1>>"$LOG_FILE_NAME" 2>&1
        Log "End stop RAC[${ORA_VERSION}]"
    elif [ "`RDsubstr $ORA_VERSION 1 2`" = "10" ] || [ "`RDsubstr $ORA_VERSION 1 2`" = "11" ]
    then
        Log "Begin stop RAC[${ORA_VERSION}]"
        ${ORA_CRS_HOME}/bin/crs_stop -all 1>>"$LOG_FILE_NAME" 2>&1
        Log "End stop RAC[${ORA_VERSION}]"
    else
        Log "Unsupport database version[${ORA_VERSION}]."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
}
# ************************** Stop RAC db instance ***********************************

# ************************** Start RAC db instance ***********************************
StartRACInstance()
{
    if [ -z "${ORA_CRS_HOME}" ] 
    then
        Log "Oracle grid home[\${ORA_CRS_HOME}] path is null."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi

    # check cluster status
    if [ "$VERSION" -ge "112" ]
    then
        ${ORA_CRS_HOME}/bin/crsctl stat res -t 1>>"$CHECKCLUSTERSTATUS" 2>&1
    else
        ${ORA_CRS_HOME}/bin/crs_stat -t 1>>"$CHECKCLUSTERSTATUS" 2>&1
    fi
    
    #CRS-4535: Cannot communicate with Cluster Ready Services
    #CRS-0184: Cannot communicate with the CRS daemon.
    ERROR_NO_START1=`cat "$CHECKCLUSTERSTATUS" | grep "CRS-4535" | wc -l`
    ERROR_NO_START2=`cat "$CHECKCLUSTERSTATUS" | grep "CRS-0184" | wc -l`
    ERROR_NO_START=`expr $ERROR_NO_START1 + $ERROR_NO_START2`
    if [ "$ERROR_NO_START" -ne "0" ]
    then
        # cluster not started
        CLUSTER_START_NUM=600
    else
        # cluster had started
        CLUSTER_START_NUM=120
        if [ "`RDsubstr $ORA_VERSION 1 2`" = "10" ] || [ "`RDsubstr $ORA_VERSION 1 4`" = "11.1" ]
        then
            #if target status of all resource is OFFLINE, tell the cluster is offline, like this
            #    # ./crs_stat -t | grep OFFLINE
            #    ora....SM1.asm application    OFFLINE   OFFLINE
            #    ora....C1.lsnr application    OFFLINE   OFFLINE
            #    ora.rac1.gsd   application    OFFLINE   OFFLINE
            #    ora.rac1.ons   application    OFFLINE   OFFLINE
            #    ora.rac1.vip   application    OFFLINE   OFFLINE
            #    ora....SM2.asm application    OFFLINE   OFFLINE  
            TARGET_ONLINE=`cat "$CHECKCLUSTERSTATUS" | sed '1,2d' | ${MYAWK} '{print $3}'| grep "ONLINE" | wc -l`
            if [ "${TARGET_ONLINE}" = "0" ]
            then
                CLUSTER_START_NUM=600
            fi
        fi
    fi
    DeleteFile "${CHECKCLUSTERSTATUS}"
    
    if [ "$VERSION" -ge "112" ]
    then
        Log "Begin start RAC[${ORA_VERSION}]"
        ${ORA_CRS_HOME}/bin/crsctl start has 1>>"$LOG_FILE_NAME" 2>&1
        ${ORA_CRS_HOME}/bin/crsctl start cluster -all 1>>"$LOG_FILE_NAME" 2>&1
        Log "End start RAC[${ORA_VERSION}] success"
    elif [ "`RDsubstr $ORA_VERSION 1 2`" = "10" ] || [ "`RDsubstr $ORA_VERSION 1 2`" = "11" ]
    then
        Log "Begin start RAC[${ORA_VERSION}]"
        #Startup will be queued to init within 90 seconds.
        /etc/init.d/init.crs start 1>>"$LOG_FILE_NAME" 2>&1
        sleep 90
        ${ORA_CRS_HOME}/bin/crs_start -all 1>>"$LOG_FILE_NAME" 2>&1
        Log "End start RAC[${ORA_VERSION}] success"
    else
        Log "Unsupport database version[${ORA_VERSION}]."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    # after start RAC, wait for resource ready, need to check process against rac to start instance
    FLAG_RACSTARTDB=0
    NUM=0
    while [ $NUM -lt 60 ]
    do
        sleep 1
        NUM=`expr $NUM + 1`
        HAVEPROCESS=`ps -aef | grep -v grep | sed 's/ *$//g' | grep "ora_...._$DBINSTANCE$" |wc -l`
        if [ "${HAVEPROCESS}" -ne "0" ]
        then
            FLAG_RACSTARTDB=1
        fi
    done
    Log "Check ${DBINSTANCE} start status, FLAG_RACSTARTDB=${FLAG_RACSTARTDB}."
    CLUSTER_START_NUM=`expr $CLUSTER_START_NUM - 60`

    TARGET_STATUS=
    if [ "$VERSION" -ge "112" ]
    then
        Log "check resource start mode"
        # get start mode by "crsctl status resource res_name -P | grep "AUTO_START=" | awk -F "=" '{print $2}'"
        #- always: Restarts the resource when the server restarts regardless of the state of the resource when the server stopped.
        #- restore: Restores the resource to the same state that it was in when the server stopped. Oracle Clusterware attempts to restart the resource if the value of TARGET was ONLINE before the server stopped.
        #- never: Oracle Clusterware never restarts the resource regardless of the state of the resource when the server stopped.
        #1. get all node name
        NODELIST=`${ORA_CRS_HOME}/bin/crsctl status server | grep "NAME=" | ${MYAWK} -F "=" '{print $2}'`
        for nodename in ${NODELIST}
        do
            #2. get reliationship between instance name and database name
            ${ORA_CRS_HOME}/bin/crsctl status resource ora.${DBNAME_CASE}.db -f -n ${nodename} | grep "USR_ORA_INST_NAME=${DBINSTANCE}$" > /dev/null
            if [ $? -eq 0 ]
            then
                TARGET_STATUS=`${ORA_CRS_HOME}/bin/crsctl status resource ora.${DBNAME_CASE}.db -v -n ${nodename} | grep "TARGET=" | ${MYAWK} -F "=" '{print $2}'`
                break
            fi
        done
    else
        TARGET_STATUS=`${ORA_CRS_HOME}/bin/crs_stat ora.${DBNAME_CASE}.${DBINSTANCE_CASE}.inst | grep "TARGET=" | ${MYAWK} -F "=" '{print $2}'`
    fi
    
    Log "Get ${DBNAME_CASE} target status:${TARGET_STATUS}."
    # target status is the target which cluster will start database
    # if get empty string, will wait for 10 minite
    # if have ONLINE state, will wait for 10 minite, because agent can't get the node name
    if [ "${TARGET_STATUS}" != "" ]
    then
        echo ${TARGET_STATUS} | grep "ONLINE" >> /dev/null
        if [ $? -ne 0 ]
        then
            CLUSTER_START_NUM=10
        fi
    fi
}
# ************************** Start RAC db instance ***********************************

# check instance status
# return 0, instance status is opened
# return 1, instance is closed
# return 2, instance resource is not ready
CheckInstanceStatus()
{
    GetOracleInstanceStatus ${DBINSTANCE}
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        return 2
    fi

    # retry 600 second
    STATUS_NUM=0
    STATUS_MAXNUM=600
    while [ $STATUS_NUM -lt $STATUS_MAXNUM ]
    do
        GetOracleInstanceStatus ${DBINSTANCE}
        RET_CODE=$?
        if [ "${RET_CODE}" -ne "0" ]
        then
            return 1
        fi
        
        #STARTED - After STARTUP NOMOUNT
        #MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        #OPEN - After STARTUP or ALTER DATABASE OPEN
        #OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
        if [ "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
        then
            return 0
        fi
        
        STATUS_NUM=`expr $STATUS_NUM + 1`
        sleep 1
    done
    
    return 1
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
DBINSTANCE=${ArgValue}
DBINSTANCE_CASE=`echo ${DBINSTANCE} | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'`

GetValue "$INPUTINFO" AppName
DBNAME=${ArgValue}
DBNAME_CASE=`echo ${DBNAME} | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'`

GetValue "$INPUTINFO" UserName
DBUSER=${ArgValue}

GetValue "$INPUTINFO" Password
DBUSERPWD=${ArgValue}

GetValue "$INPUTINFO" Action
CHECKTYPE=${ArgValue}

GetValue "$INPUTINFO" IsASM
ISASM=${ArgValue}

GetValue "$INPUTINFO" ASMDiskGroups
ASMGROUPNAMES=${ArgValue}

GetValue "$INPUTINFO" ASMUserName
ASMUSER=${ArgValue}

GetValue "$INPUTINFO" ASMPassword
ASMUSERPWD=${ArgValue}

GetValue "$INPUTINFO" ASMInstanceName
ASMSIDNAME=${ArgValue}

GetValue "$INPUTINFO" OracleHome
IN_ORACLE_HOME=${ArgValue}

GetValue "$INPUTINFO" IsIncludeArchLog
IS_INCLUDE_ARCH=${ArgValue}

AUTHMODE=0
if [ "$DBUSERPWD" = "" ]
then
    AUTHMODE=1
fi
if [ -z "${ASMSIDNAME}" ]
then
    ASMSIDNAME="+ASM"
fi

DBROLE=
if [ "${DBUSER}" = "sys" ]
then
    DBROLE="as sysdba"
fi
#******************************
Log "DBINSTANCE=$DBINSTANCE;DBNAME=$DBNAME;DBUSER=$DBUSER;CHECKTYPE=$CHECKTYPE;ISASM=$ISASM;ASMUSER=$ASMUSER;ASMGROUPNAMES=$ASMGROUPNAMES;ASMInstanceName=$ASMSIDNAME;AUTHMODE=$AUTHMODE;IN_ORACLE_HOME=$IN_ORACLE_HOME"

#get user shell type
GetUserShellType

#get Oracle version
GetOracleVersion
VERSION=`echo $PREVERSION | tr -d '.'`
# get cluster flat
GetOracleCluster

ISCLUSTER=${DBISCLUSTER}
Log "Check cluster [ISCLUSTER=${ISCLUSTER}]."

# *****************************login in oracle ************************************
# ----------Can logon in Oracle---------
Log "Begin su - ${DBUser}!"
su - ${DBUser} -c "date"
if [ "$?" != "0" ]
then
    Log "Su - ${DBUser} error!"
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

Log "End su - ${DBUser}!"

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
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        Log "End su - ${GridUser}!"
        
        # ASM and cluster need to check ORA_CRS_HOME
        if [ -z "${ORA_CRS_HOME}" ]
        then
            Log "Orace 11gR2 \${ORA_CRS_HOME} is null."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
    fi
fi

# check ORA_CRS_HOME
if [ "${ISCLUSTER}" = "1" ]
then
    if [ -z "${ORA_CRS_HOME}" ]
    then
        Log "Oracle RAC \${ORA_CRS_HOME} is null."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
fi

ASMAuthMode=/
# ******************************CheckType=0,ShutDown DataBase**********************************
if [ "$CHECKTYPE" = "1" ]
then
    if [ "${ISCLUSTER}" = "1" ]
    then
        StopRACInstance
    else
        StopSingleInstance
    fi
else
    if [ "${ISCLUSTER}" = "1" ]
    then
        StartRACInstance
        Log "cluster try to start num[$CLUSTER_START_NUM]."
        
        # sleep 10 minute, and check db status
        NUM=0
        FLAG=0
        while [ $NUM -lt $CLUSTER_START_NUM ]
        do
            sleep 1
            NUM=`expr $NUM + 1`
            HAVEPROCESS=`ps -aef | grep -v grep | sed 's/ *$//g' | grep "ora_...._$DBINSTANCE$" |wc -l`
            if [ "${HAVEPROCESS}" -ne "0" ]
            then
                CheckInstanceStatus
                IRET=$?
                # instance status is not ready
                if [ $IRET -eq 2 ]
                then
                    continue
                # instance is closed
                elif [ $IRET -eq 1 ]
                then
                    #Clear resource after cluster start failed, if starting the single instance, it will be stopped by cluster.
                    sleep 120
                    StartSingleInstance
                    FLAG=1
                    break
                # instace is opened
                elif [ $IRET -eq 0 ]
                then
                    Log "Database is open."
                    FLAG=1
                    break
                fi
            elif [ ${FLAG_RACSTARTDB} -eq 1 ] # rac start db before, now instance is down, begin to start instance immediately
            then
                #Clear resource after cluster start failed, if starting the single instance, it will be stopped by cluster.
                Log "Cluster have close instance, now begin to start it."
                sleep 60
                StartSingleInstance
                FLAG=1
                break
            fi
        done
        
        if [ ${FLAG} -eq 0 ]
        then
            StartSingleInstance
        fi
        
        # bug list
        # Bug 12387330 : SOL-SP64-11203 'CRSCTL STOP CRS' FAIL DUE TO FAILE TO STOP DG RESOURCE 
        # Bug 13827808 : ORA-15032/ORA-15027 OCCUR ON ASM DURING SHUTDOWN CRS STACK 
        # start database against the status of database is not consistent, database is online but crs_stat is offline.
        ${ORA_CRS_HOME}/bin/srvctl start database -d ${DBNAME} 1>>"$LOG_FILE_NAME" 2>&1
    else
        StartSingleInstance
    fi
fi

exit 0

