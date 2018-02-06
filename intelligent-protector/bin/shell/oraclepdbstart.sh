#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

############################################################################################
#program name:          oraclepdbinfo.sh     
#function:              Stat. oracle portfolio information,including
#                       the path of database, total size and free size
#                       and tablespace name,total size and free size
#author:
############################################################################################

USAGE="Usage: ./oraclepdbinfo.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclepdbstart.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleInstanceStatus
CHECK_INSTANCE_STATUS="${TMP_PATH}/CheckInstanceStatus${PID}.sql"
CHECK_INSTANCE_STATUSRST="${TMP_PATH}/CheckInstanceStatusRST${PID}.txt"

CHECK_PDB_LIST="${TMP_PATH}/CheckPDBList${PID}.sql"
CHECK_PDB_LISTRST="${TMP_PATH}/CheckPDBListRST${PID}.txt"

ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"

QUERYCDBSCRIPT="$TMP_PATH/QueryCDB${PID}.sql"
QUERYCDBSCRIPTRST="$TMP_PATH/QueryCDBRST${PID}.sql"

START_PDB="${TMP_PATH}/StartPDB${PID}.sql"
START_PDBRST="${TMP_PATH}/StartPDBRST${PID}.txt"

INSTANCESTATUS=""
AUTHMODE=0
#for GetValue
ArgFile=$$

FIND_PDB_CONID=
FIND_PDB_STATUS=
FIND_PDB_NAME=

#********************************define these for local script********************************
DBINSTANCE=""
DBUSER=""
DBUSERPWD=""
PDBNAME=""

# global variable
RST_SEPARATOR=";"

PARAM_FILE="${TMP_PATH}/input_tmp${PID}"
RESULT_TMP_FILE="${RESULT_FILE}.tmp"
touch "${RESULT_TMP_FILE}"
touch "${RESULT_FILE}"

# ************************** Find PDB ***********************************
#GetThisOraclePDB()
CheckPdbNameExist()
{
    local ORA_INSTANCENAME=$1
    local ORA_PDB_NAME=$2
    if [ "${ORA_PDB_NAME}" = "" ]
    then
        Log "PDB name is NUll"
        return 1
    fi
    
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    
    CHECK_PDB_LIST="${TMP_PATH}/CheckPDBList${PID}.sql"
    CHECK_PDB_LISTRST="${TMP_PATH}/CheckPDBListRST${PID}.txt"
    touch ${CHECK_PDB_LIST}
    touch ${CHECK_PDB_LISTRST}
    
    echo "select name from v\$pdbs where NAME='${ORA_PDB_NAME}';" > "${CHECK_PDB_LIST}"
    echo "exit" >> "${CHECK_PDB_LIST}"
    
    Log "Exec SQL to check PDB exist."
    OracleExeSql "${LOGIN_AUTH}" "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${ORA_INSTANCENAME}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}"
        exit ${RET_CODE}
    else  
        tmpPDBNAME=`sed -n '/----------/,/^ *$/p' "${CHECK_PDB_LISTRST}" | sed -e '/----------/d' -e '/^ *$/d'`
        if [ "${tmpPDBNAME}" = "${ORA_PDB_NAME}" ] #if server put the pdbname is not NULL, just get the info of this pdb
        then
            Log "ORA_PDB_NAME: ${ORA_PDB_NAME}, tmpPDBNAME: ${tmpPDBNAME}, get it."
            DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}"
            return 0
        fi
        DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}"
    fi

    return 1
}

# ************************** Start PDB ***********************************
StartThisOraclePDB()
{
    local ORA_INSTANCENAME=$1
    local ORA_PDB_NAME=$2
    if [ "${ORA_PDB_NAME}" = "" ]
    then
        DeleteFile "${START_PDB}" "${START_PDBRST}" "${RESULT_TMP_FILE}"
        return 1
    fi
    if [ "${ORA_PDB_NAME}" = "PDB\$SEED" ]
    then
        Log "Cannot start SEEDPDB(${ORA_PDB_NAME}) to READ WRITE."
        DeleteFile "${START_PDB}" "${START_PDBRST}" "${RESULT_TMP_FILE}"
        return 1 
    fi
    
    GetPDBStatus ${DBINSTANCE} ${ORA_PDB_NAME}
    RET_CODE=$?
    if [ ${RET_CODE} -eq 0 ] && [ "${ORA_PDB_STATUS}" = "READ WRITE" ]
    then
        DeleteFile  "${START_PDB}" "${START_PDBRST}" "${RESULT_TMP_FILE}"
        return 0
    fi

    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"

    echo "alter pluggable database ${ORA_PDB_NAME} open;" > "${START_PDB}"    
    echo "exit" >> "${START_PDB}"
    
    Log "Exec SQL to start this PDB."
    OracleExeSql "${LOGIN_AUTH}" "${START_PDB}" "${START_PDBRST}" "${ORA_INSTANCENAME}" 600 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        DeleteFile "${START_PDB}" "${START_PDBRST}"
        Log "Start PDB failed, PDB name: ${ORA_PDB_NAME}."
        exit ${RET_CODE}
    fi
    
    GetPDBStatus ${DBINSTANCE} ${ORA_PDB_NAME}
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ] || [ "${ORA_PDB_STATUS}" != "READ WRITE" ]
    then
        DeleteFile "${START_PDB}" "${START_PDBRST}"
        return 1
    fi
    
    Log "Start PDB success, PDB name: ${ORA_PDB_NAME}."
    DeleteFile "${START_PDB}" "${START_PDBRST}"
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
touch ${LOG_FILE_NAME}

GetValue "$INPUTINFO" InstanceName
DBINSTANCE=$ArgValue

GetValue "$INPUTINFO" OracleHome
IN_ORACLE_HOME=$ArgValue

GetValue "$INPUTINFO" UserName
DBUSERL=$ArgValue
DBUSER=`echo "$DBUSERL" | tr '[A-Z]' '[a-z]'`

GetValue "$INPUTINFO" Password
DBUSERPWD=$ArgValue

GetValue "$INPUTINFO" PDBName
PDBNAME=$ArgValue

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

Log "SubAppName=$DBINSTANCE;AppName=$DBNAME;UserName=$DBUSER;AUTHMODE=$AUTHMODE;IN_ORACLE_HOME=$IN_ORACLE_HOME;PDBNAME=${PDBNAME};"

#get user shell type
GetUserShellType

#get Oracle version
GetOracleVersion
VERSION=`echo $PREVERSION | tr -d '.'`

Log "Start to check oracle instance status."
GetOracleInstanceStatus ${DBINSTANCE}
RET_CODE=$?
if [ "${RET_CODE}" -ne "0" ]
then
    Log "Get instance status failed."
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${RET_CODE}
fi

if [ ! "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
then
    Log "Instance status($INSTANCESTATUS), error."
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${ERROR_INSTANCE_NOSTART}
fi
Log "end to check oracle instance status, status is: OPEN, normal."

Log "Start to check oracle database type."
GetOracleCDBType ${DBINSTANCE}
ORACLE_IS_CDB=$?
if [ ${ORACLE_IS_CDB} -ne 0 ]
then
    Log "ORACLE_TMP_VERSION: $ORACLE_TMP_VERSION, this is not a CDB, start PDB failed." 
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${ERROR_SCRIPT_ORACLE_INST_NOT_CDB}   
fi
Log "end to check oracle database type, normal."

Log "Start to check oracle PDB exist."
CheckPdbNameExist ${DBINSTANCE} ${PDBNAME}
RET_CODE=$?
if [ ${RET_CODE} -ne 0 ]
then
    Log "Get PDB failed, pdb not exist, return code: ${ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT}."
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT}
fi
Log "end to get oracle PDB, ${PDBNAME} exist."

Log "Start to start this oracle PDB(${PDBNAME})."
StartThisOraclePDB ${DBINSTANCE} ${PDBNAME}
RET_CODE=$?
if [ ${RET_CODE} -ne 0 ]
then
    Log "Start this oracle PDB failed."
    DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
    exit ${ERROR_SCRIPT_START_PDB_FAILED}   
fi
Log "end to start this oracle PDB."

cat "${RESULT_TMP_FILE}" | uniq > "${RESULT_FILE}"
DeleteFile "$ArgFile" "${RESULT_TMP_FILE}"
Log "start PDB successful."

exit 0
