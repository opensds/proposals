#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

ORAUSER_FILE="${AGENT_ROOT_PATH}/conf/orauser.conf"

# check oracle install, 0-installed  1-uninstalled
CheckOracleIsInstall()
{
    INVENTORY_LOC=/etc/oraInst.loc
    if [ "${sysName}" = "HP-UX" ] || [ "${sysName}" = "SunOS" ] 
    then
        INVENTORY_LOC=/var/opt/oracle/oraInst.loc
    fi
    
    # check file exists
    if [ ! -f "${INVENTORY_LOC}" ]
    then
        Log "oraInst.loc is not exists."
        return ${ERROR_ORACLE_NOT_INSTALLED}
    fi
    
    INVENTORY_PATH=`cat "${INVENTORY_LOC}" | grep "inventory_loc" | ${MYAWK} -F "=" '{print $2}'`
    if [ -z "${INVENTORY_PATH}" ] 
    then
        Log "Can not get inventory_loc path."
        return ${ERROR_ORACLE_NOT_INSTALLED}
    fi
    
    if [ -d "${INVENTORY_PATH}" ]
    then
        Log "${INVENTORY_PATH} diretory exists, oracle is installed."
        return 0
    else
        Log "${INVENTORY_PATH} diretory not exists, oracle is not installed."
        return ${ERROR_ORACLE_NOT_INSTALLED}
    fi
}

CheckOracleUserExists()
{
    # ----------su - oracle---------
    Log "Begin su - ${DBUser}!"
    su - ${DBUser} -c "date"
    if [ "$?" != "0" ]
    then
        Log "su - ${DBUser} error!"
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    Log "End su - ${DBUser}!"
    return 0
}

GetOracleCluster()
{
    ORACLE_LOC=/etc/oracle/ocr.loc
    if [ "${sysName}" = "HP-UX" ] || [ "${sysName}" = "SunOS" ]
    then
        ORACLE_LOC=/var/opt/oracle/ocr.loc
    fi

    if [ -f "${ORACLE_LOC}" ]
    then
        CLUSTER_FLAG=`cat "${ORACLE_LOC}" | grep "local_only" | ${MYAWK} -F "=" '{print $2}'`
        CLUSTER_FLAG=`echo ${CLUSTER_FLAG} | tr [a-z] [A-Z]`
        if [ "${CLUSTER_FLAG}" = "FALSE" ]
        then
            DBISCLUSTER=1
        fi
    else
        Log "Can not found file ocr.loc."
    fi
}

GetOracleVersion()
{
    # ----------get oracle version---------
    Log "Begin get oralce vesion!"
    ORA_VERSION="--"
    echo "exit" > "${SQLEXIT}"

    DeleteFile "${ORCLVESION}"
    touch "${ORCLVESION}"
    chmod 666 "${ORCLVESION}"
    su - ${DBUser} ${ORACLE_SHELLTYPE} -c "sqlplus /nolog @\"$SQLEXIT\" >> \"${ORCLVESION}\"" >> "${LOG_FILE_NAME}" 2>&1
    
    Log "get version from oracle sqlplus."
    ORA_VERSION=`cat "${ORCLVESION}" | grep "SQL\*Plus: Release" | $MYAWK -F " " '{print $3}'`
    DeleteFile "${ORCLVESION}"
    DeleteFile "${SQLEXIT}"

    if [ "${ORA_VERSION}" = "${SPACE}" ]
    then
        ORA_VERSION="--"
    fi
    
    if [ "${ORA_VERSION}" != "--" ]
    then
        PREVERSION=`RDsubstr $ORA_VERSION 1 4`
    fi
    
    Log "End get oracle version($ORA_VERSION), preVersion($PREVERSION)."
}

SQLPlusTimeOut()
{
    "$MONITOR_FILENAME" $$ sqlplus "$1" "${AGENT_ROOT_PATH}" >>"${LOG_FILE_NAME}" 2>&1 &
}

RmanTimeOut()
{
    "$MONITOR_FILENAME" $$ rman "$1" "${AGENT_ROOT_PATH}" >>"${LOG_FILE_NAME}" 2>&1 &
}

GetOracle_homeOfSG()
{
    local CUR_DIR parent_dir workdir
    workdir=$1
    INSTNAME=$2
    cd ${workdir}
    CUR_DIR=`pwd`
    for x in `ls $CUR_DIR`
    do
        if [ -f "$CUR_DIR/$x" ]
        then
            if [ "${x}" = "haoracle.conf" ]
            then
                Log "INFO:The haoracle.conf file is $x=====$INSTNAME"
                SG_SID_NAME=`cat $CUR_DIR/$x|grep -v '#'|grep '^SID_NAME'|$MYAWK -F "=" '{print $2}'`
                if [ "$SG_SID_NAME" = "$INSTNAME" ]
                then
                    Log "==$SG_SID_NAME==$INSTNAME"
                    IN_ORACLE_HOME=`cat $CUR_DIR/$x|grep -v '#'|grep '^ORACLE_HOME'|$MYAWK -F "=" '{print $2}'`
                    return
                fi
            fi
        elif [ -d "$x" ]
        then
            cd "$x";
            GetOracle_homeOfSG $CUR_DIR/$x $INSTNAME
            if [ "$IN_ORACLE_HOME" != "" ]
            then
                return
            fi
            cd ..
        fi
    done
}

# create New Method to Execute Sql
OracleExeSql()
{
    DeleteFile $3
    touch "$3"
    chmod 666 "$3"
    chmod 666 "$2"
    
    ORA_INSTANCENAME=$4
    TIMEOUT_SECOND=$5

    SQLPlusTimeOut ${TIMEOUT_SECOND}
    SHELLTYPE=`cat /etc/passwd | grep "^${DBUser}:" | $MYAWK -F "/" '{print $NF}'`
    if [ "$IN_ORACLE_HOME" = "" ]
    then
        if [ "$SHELLTYPE" = "csh" ]
        then
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "setenv ORACLE_SID $ORA_INSTANCENAME; sqlplus -L '$1' @\"$2\" >> \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        else
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "ORACLE_SID=$ORA_INSTANCENAME; export ORACLE_SID; sqlplus -L '$1' @\"$2\" >> \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        fi
    else
        if [ "$SHELLTYPE" = "csh" ]
        then
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "setenv ORACLE_SID $ORA_INSTANCENAME; setenv ORACLE_HOME $IN_ORACLE_HOME; sqlplus -L '$1' @\"$2\" >> \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        else
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "ORACLE_SID=$ORA_INSTANCENAME; export ORACLE_SID; ORACLE_HOME=$IN_ORACLE_HOME; export ORACLE_HOME; sqlplus -L '$1' @\"$2\" >> \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        fi
    fi

    # not check result code, because when user password error, the result code is 1,but need to check result
    # now check result file exist
    if [ ! -f "$3" ]
    then
        KillProcMonitor $$
        Log "Execute sql script failed."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    KillProcMonitor $$
    
    #ORA-01034: ORACLE not available
    HAVEERROR=`cat "$3" | grep "ORA-01034" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: ORACLE not available."
        return ${ERROR_INSTANCE_NOSTART}
    fi
    
    #ORA-00020: maximum number of processes (150) exceeded
    HAVEERROR=`cat "$3" | grep "ORA-00020" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "Database($ORA_INSTANCENAME) can not connect, maximum number of processes exceeded."
        return ${ERROR_ORACLE_APPLICATION_OVER_MAX_LINK}
    fi
    
    #ORA-01017: invalid username/password;logon denied
    HAVEERROR=`cat "$3" | grep "ORA-01017" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: invalid username/password; logon denied."
        return ${ERROR_DB_USERPWD_WRONG}
    fi
    
    #ORA-01031: insufficient privileges
    HAVEERROR=`cat "$3" | grep "ORA-01031" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: insufficient privileges."
        return ${ERROR_INSUFFICIENT_WRONG}
    fi

    #ORA-01123: cannot start online backup; media recovery not enabled 
    HAVEERROR=`cat "$3" | grep "ORA-01123" | wc -l`
    if [ ${HAVEERROR} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: cannot start online backup; media recovery not enabled."
        return ${ERROR_ORACLE_NOARCHIVE_MODE}
    fi
    
    #ORA-01146: cannot start online backup - file 1 is already in backup
    ERROR_NUM=`cat "$3" | grep "ORA-01146" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: cannot start online backup - file 1 is already in backup."
        return ${ERROR_ORACLE_DB_ALREADY_INBACKUP}
    fi
    
    #ORA-01142: cannot end online backup - none of the files are in backup
    ERROR_NUM=`cat "$3" | grep "ORA-01142" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: cannot end online backup - none of the files are in backup."
        return ${ERROR_ORACLE_DB_INHOT_BACKUP}
    fi
    
    #ORA-01081:cannot start already-running
    ERROR_NUM=`cat "$3" | grep "ORA-01081" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: cannot start already-running."
        return ${ERROR_ORACLE_DB_ALREADYRUNNING}
    fi
    
    #ora-01100 database already mounted
    ERROR_NUM=`cat "$3" | grep "ORA-01100" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: database already mounted."
        return ${ERROR_ORACLE_DB_ALREADYMOUNT}
    fi
    
    #ORA-01531: a database already open by the instance
    ERROR_NUM=`cat "$3" | grep "ORA-01531" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: a database already open by the instance."
        return ${ERROR_ORACLE_DB_ALREADYOPEN}
    fi
    
    #ORA-01507: database not mounted
    ERROR_NUM=`cat "$3" | grep "ORA-01507" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: database not mounted."
        return ${ERROR_ORACLE_NOT_MOUNTED}
    fi
    
    #ORA-01109: database not open
    ERROR_NUM=`cat "$3" | grep "ORA-01109" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: database not open."
        return ${ERROR_ORACLE_NOT_OPEN}
    fi
    
    #ORA-10997: another startup/shutdown operation of this instance inprogress
    ERROR_NUM=`cat "$3" | grep "ORA-10997" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: another startup/shutdown operation of this instance inprogress."
        return ${ERROR_ORACLE_ANOTHER_STARTING}
    fi
    
    #ORA-01154: database busy. Open, close, mount, and dismount not allowed now
    ERROR_NUM=`cat "$3" | grep "ORA-01154" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "Database($ORA_INSTANCENAME) execsql failed: database busy. Open, close, mount, and dismount not allowed now."
        return ${ERROR_ORACLE_DB_BUSY}
    fi
    
    #ORA-28002: the password will expire within 7 days
    #ORA-32004: obsolete and/or deprecated parameter(s) specified
    HAVEERROR=`cat "$3" | grep -v "ORA-28002" | grep -v "ORA-32004" | grep "ORA-" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "=====================Database($DBNAME) exec script failed.==========================="
        cat "$3" >> "${LOG_FILE_NAME}"
        Log "=====================Database($DBNAME) exec script failed.==========================="
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    return 0
}

# **************************************** Exec RMAN Script *************************
RmanExeScript()
{
    DeleteFile $3
    touch "$3"
    chmod 666 "$3"
    chmod 666 "$2"
    
    ORA_INSTANCENAME=$4
    TIMEOUT_SECOND=$5

    RmanTimeOut ${TIMEOUT_SECOND}
    SHELLTYPE=`cat /etc/passwd | grep "^${DBUser}:" | $MYAWK -F "/" '{print $NF}'`
    if [ "$IN_ORACLE_HOME" = "" ]
    then
        if [ "$SHELLTYPE" = "csh" ]
        then
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "setenv ORACLE_SID $ORA_INSTANCENAME; rman target '$1' cmdfile \"$2\" log \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        else
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "ORACLE_SID=$ORA_INSTANCENAME; export ORACLE_SID; rman target '$1' cmdfile \"$2\" log \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        fi
    else
        if [ "$SHELLTYPE" = "csh" ]
        then
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "setenv ORACLE_SID $ORA_INSTANCENAME; setenv ORACLE_HOME $IN_ORACLE_HOME; rman target '$1' cmdfile \"$2\" log \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        else
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "ORACLE_SID=$ORA_INSTANCENAME; export ORACLE_SID; ORACLE_HOME=$IN_ORACLE_HOME; export ORACLE_HOME; rman target '$1' cmdfile \"$2\" log \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        fi
    fi

    RMAN_RET=$?
    KillProcMonitor $$
    
    #ORA-01034: TNS Protocol Adapter
    HAVEERROR=`cat "$3" | grep "ORA-12560" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: TNS protocol adapter error."
        return ${ERROR_ORACLE_TNS_PROTOCOL_ADAPTER}
    fi
    
    #ORA-01034: ORACLE not available
    HAVEERROR=`cat "$3" | grep "ORA-01034" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: ORACLE not available."
        return ${ERROR_INSTANCE_NOSTART}
    fi
    
    #ORA-00020: maximum number of processes (150) exceeded
    HAVEERROR=`cat "$3" | grep "ORA-00020" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: can not connect, maximum number of processes exceeded."
        return ${ERROR_ORACLE_APPLICATION_OVER_MAX_LINK}
    fi
    
    #ORA-01017: invalid username/password;logon denied
    HAVEERROR=`cat "$3" | grep "ORA-01017" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: invalid username/password; logon denied."
        return ${ERROR_DB_USERPWD_WRONG}
    fi
    
    #ORA-01031: insufficient privileges
    HAVEERROR=`cat "$3" | grep "ORA-01031" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: insufficient privileges."
        return ${ERROR_INSUFFICIENT_WRONG}
    fi

    #ORA-01123: cannot start online backup; media recovery not enabled 
    HAVEERROR=`cat "$3" | grep "ORA-01123" | wc -l`
    if [ ${HAVEERROR} -ne 0 ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: cannot start online backup; media recovery not enabled."
        return ${ERROR_ORACLE_NOARCHIVE_MODE}
    fi
    
    #ORA-01146: cannot start online backup - file 1 is already in backup
    ERROR_NUM=`cat "$3" | grep "ORA-01146" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: cannot start online backup - file 1 is already in backup."
        return ${ERROR_ORACLE_DB_ALREADY_INBACKUP}
    fi
    
    #ORA-01142: cannot end online backup - none of the files are in backup
    ERROR_NUM=`cat "$3" | grep "ORA-01142" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: cannot end online backup - none of the files are in backup."
        return ${ERROR_ORACLE_DB_INHOT_BACKUP}
    fi
    
    #ORA-01081:cannot start already-running
    ERROR_NUM=`cat "$3" | grep "ORA-01081" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: cannot start already-running."
        return ${ERROR_ORACLE_DB_ALREADYRUNNING}
    fi
    
    #ora-01100 database already mounted
    ERROR_NUM=`cat "$3" | grep "ORA-01100" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: database already mounted."
        return ${ERROR_ORACLE_DB_ALREADYMOUNT}
    fi
    
    #ORA-01531: a database already open by the instance
    ERROR_NUM=`cat "$3" | grep "ORA-01531" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: a database already open by the instance."
        return ${ERROR_ORACLE_DB_ALREADYOPEN}
    fi
    
    #ORA-01507: database not mounted
    ERROR_NUM=`cat "$3" | grep "ORA-01507" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: database not mounted."
        return ${ERROR_ORACLE_NOT_MOUNTED}
    fi
    
    #ORA-01109: database not open
    ERROR_NUM=`cat "$3" | grep "ORA-01109" | wc -l`
    if [ ${ERROR_NUM} -ne 0 ]
    then
        Log "RMAN($ORA_INSTANCENAME) execute script failed: database not open."
        return ${ERROR_ORACLE_NOT_OPEN}
    fi
    
    #ORA-28002: the password will expire within 7 days
    #ORA-32004: obsolete and/or deprecated parameter(s) specified
    HAVEERROR=`cat "$3" | grep -v "ORA-28002" | grep -v "ORA-32004" | grep "ORA-" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "=====================RMAN($ORA_INSTANCENAME) execute script failed.==========================="
        cat "$3" >> "${LOG_FILE_NAME}"
        Log "=====================RMAN($ORA_INSTANCENAME) execute script failed.==========================="
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    #RMAN-00569: RMAN Error
    HAVEERROR=`cat "$3" | grep "RMAN-00569" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "=====================RMAN($ORA_INSTANCENAME) execute script failed.==========================="
        cat "$3" >> "${LOG_FILE_NAME}"
        Log "=====================RMAN($ORA_INSTANCENAME) execute script failed.==========================="
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi
    
    if [ "$RMAN_RET" != "0" ]
    then
        Log "Exec RMAN script failed."
        cat "$3" >> "${LOG_FILE_NAME}"
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    return 0
}

# **************************************** Exec ASM Instance SQL *************************
ASMExeSql()
{
    DeleteFile $3
    touch "$3"
    chmod 666 "$3"
    chmod 666 "$2"
    
    ASMS_INSTNAME=$4
    TIMEOUT_SECOND=$5

    SQLPlusTimeOut ${TIMEOUT_SECOND}
    if [ "$VERSION" -ge "112" ]
    then
        SHELLTYPE=`cat /etc/passwd | grep "^${GridUser}:" | $MYAWK -F "/" '{print $NF}'`
        if [ "$SHELLTYPE" = "csh" ]
        then
            su - ${GridUser} ${GRID_SHELLTYPE} -c "setenv ORACLE_SID ${ASMS_INSTNAME}; sqlplus -L '$1' @\"$2\" >> \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        else
            su - ${GridUser} ${GRID_SHELLTYPE} -c "ORACLE_SID=${ASMS_INSTNAME}; export ORACLE_SID; sqlplus -L '$1' @\"$2\" >> \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        fi
    else
        SHELLTYPE=`cat /etc/passwd | grep "^${DBUser}:" | $MYAWK -F "/" '{print $NF}'`
        if [ "$SHELLTYPE" = "csh" ]
        then
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "setenv ORACLE_SID ${ASMS_INSTNAME}; sqlplus -L '$1' @\"$2\" >> \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        else
            su - ${DBUser} ${ORACLE_SHELLTYPE} -c "ORACLE_SID=${ASMS_INSTNAME}; export ORACLE_SID; sqlplus -L '$1' @\"$2\" >> \"$3\"" >> "${LOG_FILE_NAME}" 2>&1
        fi
    fi

    # not check result code, because when user password error, the result code is 1,but need to check result
    # now check result file exist
    if [ ! -f "$3" ]
    then
        KillProcMonitor $$
        Log "ASMExeSql Execute sql script failed."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    KillProcMonitor $$

    #ORA-01034: ORACLE not available
    HAVEERROR=`cat "$3" | grep "ORA-01034" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "ASM($ASMS_INSTNAME) execsql failed: ORACLE not available."
        return ${ERROR_ORACLE_ASM_INSTANCE_NOSTART}
    fi
    
    #ORA-00020: maximum number of processes (150) exceeded
    HAVEERROR=`cat "$3" | grep "ORA-00020" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "ASM($ASMS_INSTNAME) can not connect, maximum number of processes exceeded."
        return ${ERROR_ORACLE_APPLICATION_OVER_MAX_LINK}
    fi
    
    #ORA-01017: invalid username/password;logon denied
    HAVEERROR=`cat "$3" | grep "ORA-01017" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "ASM($ASMS_INSTNAME) execsql failed: invalid username/password; logon denied."
        return ${ERROR_ORACLE_ASM_DBUSERPWD_WRONG}
    fi
    
    #ORA-01031: insufficient privileges
    HAVEERROR=`cat "$3" | grep "ORA-01031" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "ASM(${ASMS_INSTNAME}) execsql failed: insufficient privileges."
        return ${ERROR_ORACLE_ASM_INSUFFICIENT_WRONG}
    fi
    
    #already mount
    HAVEERROR=`cat "$3" | grep "ORA-15013" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "ASM(${ASMS_INSTNAME}) execsql failed: already mount."
        return ${ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT}
    fi
    
    #ORA-15001: diskgroup "PLAG001" does not exist or is not mounted
    HAVEERROR=`cat "$3" | grep "ORA-15001" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "ASM(${ASMS_INSTNAME}) execsql failed: diskgroup not mount or not exist."
        return ${ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT}
    fi

    HAVEERROR=`cat "$3" | grep "ORA-" | wc -l`
    if [ "$HAVEERROR" -ne "0" ]
    then
        Log "=====================ASM(${ASMS_INSTNAME}) exec script failed.==========================="
        cat $3 >> "${LOG_FILE_NAME}"
        Log "=====================ASM(${ASMS_INSTNAME}) exec script failed.==========================="
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    return 0
}

# ************************** check instance status ***********************************
GetOracleInstanceStatus()
{
    ORA_INSTANCENAME=$1
    if [ "$DBUSER" = "sys" ]
    then 
        DBROLE="as sysdba"
    fi
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"

    echo "select status from v\$instance;" > "${CHECK_INSTANCE_STATUS}"
    echo "exit" >> "${CHECK_INSTANCE_STATUS}"
    
    Log "Exec SQL to get status of instance."
    OracleExeSql "${LOGIN_AUTH}" "${CHECK_INSTANCE_STATUS}" "${CHECK_INSTANCE_STATUSRST}" "${ORA_INSTANCENAME}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" != "0" ]
    then
        DeleteFile "${CHECK_INSTANCE_STATUS}"
        DeleteFile "${CHECK_INSTANCE_STATUSRST}"
        return ${RET_CODE}
    else        
        #STARTED - After STARTUP NOMOUNT
        #MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        #OPEN - After STARTUP or ALTER DATABASE OPEN
        #OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
        #get result like below, and get the content between ----- and black
        #   Connected to:
        #   Oracle Database 11g Enterprise Edition Release 11.2.0.3.0 - 64bit Production
        #   With the Partitioning, OLAP, Data Mining and Real Application Testing options
        #
        #
        #   STATUS
        #   ------------
        #   OPEN
        #
        #   Disconnected from Oracle Database 11g Enterprise Edition Release 11.2.0.3.0 - 64bit Production
        INSTANCESTATUS=`sed -n '/----------/,/^ *$/p' "${CHECK_INSTANCE_STATUSRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1}'`
        Log "INSTANCESTATUS=${INSTANCESTATUS}."
        DeleteFile "${CHECK_INSTANCE_STATUS}"
        DeleteFile "${CHECK_INSTANCE_STATUSRST}"
    fi
    
    return 0
}

#Create temporary sql script function, eg: CrtTmpSql ResultFile SqlScrpitFile Sql Section
CrtTmpSql()
{
    if [ $# -ne 4 ]
    then
        Log "Create tempory sql script failed, number of parameter is not correctly."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi

    echo "set serveroutput on" > "$2"
    echo "set echo off" >> "$2"
    echo "set feedback off" >> "$2"
    echo "set heading off" >> "$2"
    echo "set verify off" >> "$2"
    echo "set trimspool off" >> "$2"
    echo "set trims on" >> "$2"
    echo "spool $1" >> "$2"
    echo "declare cursor cur_tblspace is $3" >> "$2"
    echo "begin" >> "$2"
    echo "    for ct in cur_tblspace loop" >> "$2"
    echo "  dbms_output.put_line('' || ct.$4 || '');" >> "$2"
    echo "  end loop;" >> "$2"
    echo "end;" >> "$2"
    echo "/" >> "$2"
    echo "spool off;" >> "$2"
    echo "exit;" >> "$2"
}

# ************************** get database backup tablespace count ***********************************
GetBackupModeTBCount()
{
    CrtTmpSql "${BKFILENAME}" "${TMPBKSCRIPT}" "${QRY_BACKUP_TB_SQL}" "status"
    if [ $? -ne 0 ]
    then
        Log "Create tmp sql error."
        DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}"
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    DeleteFile "${BKFILENAME}"
    touch "${BKFILENAME}"
    chmod 666 "${BKFILENAME}"
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec SQL to get backup tablespace."
    OracleExeSql "${LOGIN_AUTH}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}" "${DBINSTANCE}" 60 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}"
        Log "Get active tablespace count failed."
        return $RET_CODE
    fi
    ACTIVE_TB_COUNT=`cat "${BKFILENAME}" | wc -l`
    DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}"
    return 0
}

# ************************** get database not backup tablespace count ***********************************
GetNotBackupModeTBCount()
{
    GetOracleCDBType ${DBINSTANCE}
    RET_CODE=$?
    if [[ $RET_CODE -ne 0 ]] && [[ $RET_CODE -ne 1 ]]
    then
        return $RET_CODE
    elif [ $RET_CODE -eq 0 ]
    then
        QRY_NOT_BACKUP_TB_SQL="select * from v\$backup where status='NOT ACTIVE' and con_id not in (select con_id from v\$pdbs where name='PDB\$SEED' or open_mode!='READ WRITE');"
    fi
    
    CrtTmpSql "${BKFILENAME}" "${TMPBKSCRIPT}" "${QRY_NOT_BACKUP_TB_SQL}" "status"
    if [ $? -ne 0 ]
    then
        Log "Create tmp sql error."
        DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}"
        return $ERROR_SCRIPT_EXEC_FAILED
    fi

    DeleteFile ${BKFILENAME}
    touch "${BKFILENAME}"
    chmod 666 "${BKFILENAME}"
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Exec SQL to get no backup tablespace."
    OracleExeSql "${LOGIN_AUTH}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}" "${DBINSTANCE}" 60 >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}"
        Log "Get not active tablespace count failed."
        return $ERROR_SCRIPT_EXEC_FAILED
    fi
    NOT_ACTIVE_TB_COUNT=`cat "${BKFILENAME}" | wc -l`
    DeleteFile "${BKFILENAME}" "${TMPBKSCRIPT}" "${TMPBKSCRIPTRST}"
    return 0
}

StartASMInstance()
{
    Log "Begin to start css services."
    if [ "$VERSION" -ge "112" ]
    then
        # startup has
        ${ORA_CRS_HOME}/bin/crsctl start has 1>>"${LOG_FILE_NAME}" 2>&1
        # startup cssd
        ${ORA_CRS_HOME}/bin/crsctl start resource ora.cssd 1>>"${LOG_FILE_NAME}" 2>&1
    else
        ORA_HOME_PATH=${ORACLE_HOME}
        if [ "${ISCLUSTER}" = "1" ]
        then
            ORA_HOME_PATH=${ORA_CRS_HOME}
        fi
        CHECK_CSS_INFO=`${ORA_HOME_PATH}/bin/crsctl check css`
        # 11.1 Cluster Synchronization Services appears healthy
        # 10.2 CSS appears healthy
        if [ "${CHECK_CSS_INFO}" != "Cluster Synchronization Services appears healthy" ] && [ "${CHECK_CSS_INFO}" != "CSS appears healthy" ]
        then
            # startup crs
            ${ORA_HOME_PATH}/bin/crsctl start crs 1>>"${LOG_FILE_NAME}" 2>&1
            # start crs
            ${ORA_HOME_PATH}/bin/localconfig reset 1>>"${LOG_FILE_NAME}" 2>&1
        else
            echo "${CHECK_CSS_INFO}" >> "${LOG_FILE_NAME}"
        fi
    fi

    # check ASM instance auth mode user/pwd or /
    checkASMAuthMode
    
    Log "Check ASM status"
    RET_CODE=1
    ASMPROCESSNUM=`ps -ef | grep "asm_...._$ASMSIDNAME" | grep -v "grep" | wc -l`
    if [ "${ASMPROCESSNUM}" -eq 0 ]
    then
        # cluster start ASM fail  exit
        if [ "${ISCLUSTER}" = "1" ]
        then
            Log "Cluster start ASM instance failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        
        Log "Begin start ASM[${ORA_VERSION}]."
        CreateASMStartSql ${ASMSTARTSQL}
        ASM_LOG_AUTH="`CreateASMLoginCmd ${VERSION} ${AUTHMODE} \"${ASMAuthMode}\"`"

        Log "Exec ASM SQL to startup ASM instance."
        ASMExeSql "${ASM_LOG_AUTH}" ${ASMSTARTSQL} ${ASMSTARTRST} ${ASMSIDNAME} 120
        RET_CODE=$?
        DeleteFile "${ASMSTARTSQL}" "${ASMSTARTRST}"
        # check process to sure if ok
    fi
    
    ASMPROCESSNUM=`ps -ef | grep "asm_...._$ASMSIDNAME" | grep -v "grep" | wc -l`
    if [ ${ASMPROCESSNUM} -eq 0 ]
    then
        Log "End start ASM[${ORA_VERSION}] failed."
        exit ${RET_CODE}
    else
        Log "End start ASM[${ORA_VERSION}] success."
    fi
    
    # Cluster, get ASM instance name
    if [ "${ISCLUSTER}" = "1" ]
    then
        ASMSIDNAME=`ps -ef | grep "asm_...._$ASMSIDNAME" | grep -v "grep" | $MYAWK -F '+' '{print $NF}'`
        ASMSIDNAME=`echo ${ASMSIDNAME} | $MYAWK -F " " '{print $1}'`
        ASMSIDNAME="+"${ASMSIDNAME}
        Log "ASM instance: ${ASMSIDNAME}"
    fi
    
    # mount diskgroup
    DiskGroupManage 1
}

# **************************mount diskgroup***********************************
DiskGroupManage()
{
    #$1 mount(1) or umount(0) disk group
    MMode=$1
    MModeStr=""
    if [ $MMode -eq 1 ]
    then
        MModeStr=mount
    else
        MModeStr=dismount
    fi
    
    # check asm diskgroup whether mount, mount if not mount
    Log "ASMGROUPNAMES=${ASMGROUPNAMES}"
    ASMGROUPNAMES=`echo ${ASMGROUPNAMES} | sed -e 's/+/ /g'`
    for dgName in ${ASMGROUPNAMES}
    do
        #=========================start to mount diskgroup=========================
        echo "alter diskgroup $dgName $MModeStr;" > "${GETDISKGROUPSQL}"
        echo "exit" >> "${GETDISKGROUPSQL}"
        ASM_LOG_AUTH="`CreateASMLoginCmd ${VERSION} ${AUTHMODE} \"${ASMAuthMode}\"`"

        Log "Exec ASM SQL to mount diskgroup($dgName)."
        ASMExeSql "${ASM_LOG_AUTH}" ${GETDISKGROUPSQL} ${GETDISKGROUPSQLRST} ${ASMSIDNAME} 60
        RET_MOUNT=$?
        DeleteFile "${GETDISKGROUPSQL}" "${GETDISKGROUPSQLRST}"

        #=========================get diskgroup status=========================
        echo "set linesize 300" > "${GETDISKGROUPSQL}"
        echo "col state for a50" >> "${GETDISKGROUPSQL}"
        echo "select state from v\$asm_diskgroup where NAME='"${dgName}"';" >> "${GETDISKGROUPSQL}"
        echo "exit" >> "${GETDISKGROUPSQL}"
        ASMExeSql "${ASM_LOG_AUTH}" ${GETDISKGROUPSQL} ${GETDISKGROUPSQLRST} ${ASMSIDNAME} 60
        RET_CODE=$?
        DGSTATUS=
        if [ "${RET_CODE}" != "0" ]
        then
            DeleteFile "${GETDISKGROUPSQL}"
            DeleteFile "${GETDISKGROUPSQLRST}"
            Log "Get diskgroup status failed after ${MMode} diskgroup."
            exit ${RET_CODE}
        else
            DGSTATUS=`sed -n '/----------/,/^ *$/p' "${GETDISKGROUPSQLRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1}'`
            Log "DGSTATUS=${DGSTATUS}."
            DeleteFile "${GETDISKGROUPSQL}"
            DeleteFile "${GETDISKGROUPSQLRST}"
        fi
    
        if [ $MMode -eq 1 ]
        then
            if [ "${DGSTATUS}" = "MOUNTED" ]
            then
                Log "$MModeStr diskgroup ${dgName} success."
                continue
            else
                Log "$MModeStr diskgroup ${dgName} failed."
                exit ${ERROR_DEVICE_FILESYS_MOUNT_FAILED}
            fi
        elif [ $MMode -eq 0 ]
        then
            if [ "${DGSTATUS}" = "DISMOUNTED" ]
            then
                Log "$MModeStr diskgroup ${dgName} success."
                continue
            else
                Log "$MModeStr diskgroup ${dgName} failed."
                exit ${ERROR_DEVICE_FILESYS_UNMOUNT_FAILED}
            fi
        fi
    done
    
    return 0
}

# check ASM instance authentication mode
# try login ASM instance with username and password, 
# if exists ORA-01031 error, then try log ASM instance without username and password
checkASMAuthMode()
{
    echo "exit" > "${GETDISKGROUPSQL}"
    ASM_LOGIN=${ASMUSER}/\"${ASMUSERPWD}\"
    ASM_LOG_AUTH="`CreateASMLoginCmd ${VERSION} ${AUTHMODE} \"${ASM_LOGIN}\"`"

    Log "Exec ASM SQL to check auth mode of ASM instance."
    ASMExeSql "${ASM_LOG_AUTH}" ${GETDISKGROUPSQL} ${GETDISKGROUPSQLRST} ${ASMSIDNAME} 60
    RET_CODE=$?
    DeleteFile "${GETDISKGROUPSQL}" "${GETDISKGROUPSQLRST}"
    #ORA-01031: insufficient privileges
    if [ "${RET_CODE}" -eq "${ERROR_ORACLE_ASM_INSUFFICIENT_WRONG}" ]
    then
        ASMAuthMode=/
        Log "ASMAuthMode uses without username,password."
    else
        ASMAuthMode=$ASMUSER/\"$ASMUSERPWD\"
        Log "ASMAuthMode uses username/password."
    fi
}

# **************************************** Create ASM start SQL ***************************
CreateASMStartSql()
{
    echo "startup;" > "$1"
    echo "exit" >> "$1"
}

GetOracleBasePath()
{
    Log "Begin get oralce base path!"
    DeleteFile "${ORATMPINFO}"
    touch "${ORATMPINFO}"
    chmod 666 "${ORATMPINFO}"
    su - ${DBUser} ${ORACLE_SHELLTYPE} -c "echo \$ORACLE_BASE >> \"${ORATMPINFO}\""
    IN_ORACLE_BASE=`cat "${ORATMPINFO}"`
    DeleteFile "${ORATMPINFO}"
    Log "ORACLE_BASE=$IN_ORACLE_BASE"
}

GetOracleHomePath()
{
    Log "Begin get oralce HOME path!"
    DeleteFile "${ORATMPINFO}"
    touch "${ORATMPINFO}"
    chmod 666 "${ORATMPINFO}"
    su - ${DBUser} ${ORACLE_SHELLTYPE} -c "echo \$ORACLE_HOME >> \"${ORATMPINFO}\""
    IN_ORACLE_HOME=`cat "${ORATMPINFO}"`
    DeleteFile "${ORATMPINFO}"
    Log "ORACLE_HOME=$IN_ORACLE_HOME"
}

GetORA_CRS_HOME()
{
    ORACLE_VERSION=$1
    Log "Begin get oralce crs home path!"
    if [ "$ORACLE_VERSION" -ge "112" ]
    then
        touch "${ORATMPINFO}"
        chmod 666 "${ORATMPINFO}"
        su - ${GridUser} ${GRID_SHELLTYPE} -c "echo \$ORACLE_HOME >> \"${ORATMPINFO}\""
        ORA_CRS_HOME=`cat "${ORATMPINFO}"`
        DeleteFile "${ORATMPINFO}"
    else
        ORA_CRS_HOME=${ORA_CRS_HOME}
    fi
    Log "ORA_CRS_HOME=${ORA_CRS_HOME}"
}

GetArchiveLogMode()
{
    DB_SID=$1
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "Begin get archive log mode."
    echo "select LOG_MODE from v\$database;" > "${GET_ARCHIVE_LOG_MODE_SQL}"
    echo "exit" >> "${GET_ARCHIVE_LOG_MODE_SQL}"
    
    Log "Exec sql script to get archive log mode."
    OracleExeSql "${LOGIN_AUTH}" "${GET_ARCHIVE_LOG_MODE_SQL}" "${ARCHIVE_LOG_MODE_RST}" ${DB_SID} 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "$RET_CODE" -ne "0" ]
    then
        Log "Get archive log list failed."
        DeleteFile "${GET_ARCHIVE_LOG_MODE_SQL}" "${ARCHIVE_LOG_MODE_RST}"
        return ${RET_CODE}
    fi
    
    # non archive log mode
    ISARCHIVE=`cat "${ARCHIVE_LOG_MODE_RST}" | grep "NOARCHIVELOG"`
    if [ "$?" -eq "0" ]
    then
        Log "Non archive log mode."
        DeleteFile "${GET_ARCHIVE_LOG_MODE_SQL}" "${ARCHIVE_LOG_MODE_RST}"
        return 0
    fi
    
    DeleteFile "${GET_ARCHIVE_LOG_MODE_SQL}" "${ARCHIVE_LOG_MODE_RST}"
    Log "Get archive log mode succ."
    return 1
}

CreateLoginCmd()
{
    OS_AUTH_FLG=$1
    ORA_USER=$2
    ORA_PWD="$3"
    ORA_ROLE="$4"
    if [ "${OS_AUTH_FLG}" = "1" ]
    then
        echo "/ as sysdba"
    elif [ "$OS_AUTH_FLG" = "0" ]
    then
        echo "${ORA_USER}/\"${ORA_PWD}\" ${ORA_ROLE}"
    fi
}

CreateASMLoginCmd()
{
    ORACLE_VERSION=$1
    OS_AUTH_FLG=$2
    ASM_AUTH="$3"

    if [ "$ORACLE_VERSION" -ge "112" ]
    then
        if [ "$OS_AUTH_FLG" = "1" ]
        then
            echo "/ as sysasm"
        elif [ "$OS_AUTH_FLG" = "0" ]
        then
            echo "${ASM_AUTH} as sysasm"
        fi
    else
        if [ "$OS_AUTH_FLG" = "1" ]
        then
            echo "/ as sysdba"
        elif [ "$OS_AUTH_FLG" = "0" ]
        then
            echo "${ASM_AUTH} as sysdba"
        fi
    fi
}

CreateRmanLoginCmd()
{
    OS_AUTH_FLG=$1
    ORA_USER=$2
    ORA_PWD="$3"

    if [ "$OS_AUTH_FLG" = "1" ]
    then
        echo "/"
    elif [ "$OS_AUTH_FLG" = "0" ]
    then
        echo "${ORA_USER}/\"${ORA_PWD}\""
    fi
}

#
# need ${LOGIN_AUTH} ${QUERYCDBSCRIPT} ${QUERYCDBSCRIPTRST}
#           ${QUERYCDBSCRIPT} ${QUERYCDBSCRIPTRST}
# $1 $ORA_INSTANCENAME
# return 
# 1- not CDB
# 0- CDB
# else error code
#
GetOracleCDBType()
{    
    ORA_INSTANCENAME=$1
    
    #get Oracle version and check if it's 12
    GetOracleVersion
    VERSION=`echo $PREVERSION | $MYAWK -F "." '{print $1}'`
    Log "VERSION=${VERSION}"
    
    if [[ "${VERSION}" = "10" ]] || [[ "${VERSION}" = "11" ]];
    then
        Log "Oracle version($ORA_VERSION) not supports CDB."
        return 1
    fi
    if [ ! "${VERSION}" = "12" ]
    then
        Log "Oracle version($ORA_VERSION) is not supported."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi

    #get CDB type
    echo "select cdb from v\$database;" > "${QUERYCDBSCRIPT}"
    echo "exit" >> "${QUERYCDBSCRIPT}"
    
    Log "Exec SQL to get CDB type of instance."
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    OracleExeSql "${LOGIN_AUTH}" "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}" "${ORA_INSTANCENAME}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" != "0" ]
    then
        DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
        exit ${RET_CODE}
    else
        ORACLECDBTYPE=`sed -n '/---/,/^ *$/p' "${QUERYCDBSCRIPTRST}" | sed -e '/---/d' -e '/^ *$/d'`
        Log "ORACLECDBTYPE=${ORACLECDBTYPE}."
        DeleteFile "${QUERYCDBSCRIPT}" "${QUERYCDBSCRIPTRST}"
    fi
    if [ "${ORACLECDBTYPE}" = "YES" ]
    then
        return 0
    else
        return 1
    fi
}

GetPDBStatus()
{
    local ORA_INSTANCENAME=$1
    local ORA_PDB_NAME=$2
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    
    PDB_STATUS_FILE="${TMP_PATH}/PDBStatus${PID}.sql"
    PDB_STATUS_FILERST="${TMP_PATH}/PDBStatusRST${PID}.txt"
    
    touch ${PDB_STATUS_FILE}
    touch ${PDB_STATUS_FILERST}
    
    echo "select open_mode from v\$pdbs where NAME='${ORA_PDB_NAME}';" > "${PDB_STATUS_FILE}"
    echo "exit" >> "${PDB_STATUS_FILE}"
    
    Log "Exec SQL to get PDB List."
    OracleExeSql "${LOGIN_AUTH}" "${PDB_STATUS_FILE}" "${PDB_STATUS_FILERST}" "${ORA_INSTANCENAME}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        DeleteFile "${PDB_STATUS_FILE}" "${PDB_STATUS_FILERST}"
        exit ${RET_CODE}
    else        
        #   OPEN_MODE
        #   ----------
        #   MOUNTED
        ORA_PDB_STATUS=`sed -n '/----------/,/^ *$/p' "${PDB_STATUS_FILERST}" | sed -e '/----------/d' -e '/^ *$/d'`
        DeleteFile "${PDB_STATUS_FILE}" "${PDB_STATUS_FILERST}"
        return 0
    fi
    DeleteFile "${PDB_STATUS_FILE}" "${PDB_STATUS_FILERST}"
    return 1
}


# get oracle user info, include database user and grid user
# default, the db and grid user are oracle and grid.
# SAP + oracle, db user is consistent of ora + instance name,
# so, all function are call this function to get oracle and grid user,
# if no file exists, SAP exists, get SAP db user and grid
# SAP not exists, write oracle and grid
getOracleUserInfo()
{
    if [ ! -f "${ORAUSER_FILE}" ]
    then
        GridUser=grid
        # check SAP oracle user
        DBUser=`cat /etc/passwd | grep "SAP Database Administrator" | ${MYAWK} -F ":" '{print $1}'`
        if [ -z "${DBUser}" ]
        then
            DBUser=oracle
        fi
        
        if [ -z "${DBUser}" ]
        then
            Log "Can not get DB User."
            return 1
        fi
        
        echo "DBUser=${DBUser}" > "${ORAUSER_FILE}"
        echo "GridUser=${GridUser}" >> "${ORAUSER_FILE}"
        chmod 600 "${ORAUSER_FILE}"
    fi
    
    # get user info from conf file
    DBUser=`cat "${ORAUSER_FILE}" | grep DBUser | ${MYAWK} -F "=" '{print $2}'`
    GridUser=`cat "${ORAUSER_FILE}" | grep GridUser | ${MYAWK} -F "=" '{print $2}'`
    
    if [ -z "${DBUser}" ] || [ -z "${GridUser}" ]
    then
        Log "Can not get DB User or Grid User."
        Log "`cat \"${ORAUSER_FILE}\"`"
        return 1
    fi
    
    # check os user
    cat /etc/passwd | grep "^${DBUser}:" >> /dev/null
    if [ $? -ne 0 ]
    then
        Log "${DBUser} is not exists in OS."
        return 1
    fi
    
    Log "Get Oracle DB user info: ${DBUser},${GridUser}"
    return 0
} 
