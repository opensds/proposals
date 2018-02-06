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
LOG_FILE_NAME="${LOG_PATH}/oraclepdbinfo.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleInstanceStatus
CHECK_INSTANCE_STATUS="${TMP_PATH}/CheckInstanceStatus${PID}.sql"
CHECK_INSTANCE_STATUSRST="${TMP_PATH}/CheckInstanceStatusRST${PID}.txt"

QUERYCDBSCRIPT="$TMP_PATH/QueryCDB${PID}.sql"
QUERYCDBSCRIPTRST="$TMP_PATH/QueryCDBRST${PID}.sql"

ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"

INSTANCESTATUS=""
AUTHMODE=0
#for GetValue
ArgFile=$$

#********************************define these for local script********************************
DBINSTANCE=""
DBUSER=""
DBUSERPWD=""
PDBNAME=""

RESULT_TMP_FILE="${RESULT_FILE}.tmp"
#######################################set file name##################################
QUERYNAMECRIPT="$TMP_PATH/QueryFileName$PID.sql"
QUERYNAMECRIPTRST="$TMP_PATH/QueryFileNameRST$PID.sql"

# global variable
RST_SEPARATOR=";"

PARAM_FILE="${TMP_PATH}/input_tmp${PID}"

# ************************** Find PDB List ***********************************
GetOraclePDBList()
{
    ORA_INSTANCENAME=$1
    local ORACLE_PDB_NAME=$2
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    
    CHECK_PDB_LIST="${TMP_PATH}/CheckPDBList${PID}.sql"
    CHECK_PDB_LISTRST="${TMP_PATH}/CheckPDBListRST${PID}.txt"
    touch ${CHECK_PDB_LIST}
    touch ${CHECK_PDB_LISTRST}
    
    echo "set pagesize 300;" > "${CHECK_PDB_LIST}"  #max pdb is 252
    echo "select con_id,name,open_mode from v\$pdbs;" >> "${CHECK_PDB_LIST}"
    echo "exit" >> "${CHECK_PDB_LIST}"
    
    Log "Exec SQL to get PDB List."
    OracleExeSql "${LOGIN_AUTH}" "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${ORA_INSTANCENAME}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ]
    then
        DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}"
        exit ${RET_CODE}
    else 
        #   >select con_id,name from v$pdbs;
        #   CON_ID	    NAME	
        #   ---------- ---------- 
        #   2 4165023779 PDB$SEED			     READ ONLY
        #   3 2358941778 PDB12C1			     MOUNTED
        #   4 2054691292 PDB12C2			     MOUNTED
        #   5 2104474211 PDB12C3			     MOUNTED
        #   Disconnected from Oracle Database 11g Enterprise Edition Release 11.2.0.3.0 - 64bit Production
        PDBTAB="${LOG_PATH}/pdbtabs${PID}.txt"
        ONEPDBINFO="${LOG_PATH}/onepdbinfo${PID}.txt"
        touch ${PDBTAB}
        touch ${ONEPDBINFO}
        sed -n '/----------/,/^ *$/p' "${CHECK_PDB_LISTRST}" > "${PDBTAB}" 
        cat -n ${PDBTAB} >> "${LOG_FILE_NAME}"
        
        LINE_CNT=`cat ${PDBTAB} | wc -l`
        Log "LINE_CNT: ${LINE_CNT}"
        
        for((x=2;x<${LINE_CNT};x++));  #line first just include "-----------" the last line include nothing
        do	
            sed -n "${x}p" "${PDBTAB}" > "${ONEPDBINFO}"
        
            tmpCONID=`sed -n '1p' "${ONEPDBINFO}" | $MYAWK '{print $1}'`
            tmpPDBNAME=`sed -n '1p' "${ONEPDBINFO}" | $MYAWK '{print $2}'`
            
            tmpSTATUS_1=`sed -n '1p' "${ONEPDBINFO}" | $MYAWK '{print $3}'` #READ
            tmpSTATUS_2=""
            if [ "${tmpSTATUS_1}" = "READ" ]
            then
                tmpSTATUS_2=`sed -n '1p' "${ONEPDBINFO}" | $MYAWK '{print $4}'` #ONLY
                tmpSTATUS="${tmpSTATUS_1} ${tmpSTATUS_2}"
            elif [ "${tmpSTATUS_1}" = "" ]
            then
                Log "Error: get this pdb($tmpPDBNAME) status failed, status: $tmpSTATUS_1."
                exit $ERROR_SCRIPT_EXEC_FAILED
            else
                tmpSTATUS="${tmpSTATUS_1}"
            fi
            
            if [ "${ORACLE_PDB_NAME}" = "" ]
            then
                Log "CONID: ${tmpCONID}, PDBNAME: ${tmpPDBNAME}, STATUS: ${tmpSTATUS}"
                echo "${tmpCONID}${RST_SEPARATOR}${tmpPDBNAME}${RST_SEPARATOR}${tmpSTATUS}" >> "${RESULT_TMP_FILE}"
            else
                Log "PDBNAME: ${ORACLE_PDB_NAME}, tmpPDBNAME: ${tmpPDBNAME}."
                if [ "${tmpPDBNAME}" = "${ORACLE_PDB_NAME}" ] #if server put the pdbname is not NULL, just get the info of this pdb
                then
                    Log "PDBNAME: ${ORACLE_PDB_NAME}, tmpPDBNAME: ${tmpPDBNAME}, get it."
                    Log "CONID: ${tmpCONID}, PDBNAME: ${tmpPDBNAME}, STATUS: ${tmpSTATUS}"
                    echo "${tmpCONID}${RST_SEPARATOR}${tmpPDBNAME}${RST_SEPARATOR}${tmpSTATUS}" >> "${RESULT_TMP_FILE}"
                    DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${PDBTAB}" "${ONEPDBINFO}"
                    return 0
                else
                   continue 
                fi
            fi
        done
        DeleteFile "${CHECK_PDB_LIST}" "${CHECK_PDB_LISTRST}" "${PDBTAB}" "${ONEPDBINFO}"
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
touch ${LOG_FILE_NAME}
#################Entry of script to query the information of oracle portfolio###########
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
    exit ${RET_CODE}
fi

if [ ! "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
then
    Log "Instance status($INSTANCESTATUS) no open."
    exit ${ERROR_INSTANCE_NOSTART}
fi
Log "end to check oracle instance status, status is: OPEN, normal."

touch "${RESULT_TMP_FILE}"

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

Log "Start to get oracle PDB list."
GetOraclePDBList ${DBINSTANCE} ${PDBNAME}
RET_CODE=$?
if [ ${RET_CODE} -ne 0 ]
then
    Log "Get PDB list failed."
    exit ${RET_CODE}
fi
Log "end to get oracle PDB list."

cat "${RESULT_TMP_FILE}" | uniq > "${RESULT_FILE}"
##delete temporary file
DeleteFile "$ArgFile" "$QUERYNAMECRIPTRST" "$QUERYNAMECRIPT" "${RESULT_TMP_FILE}"

Log "Stat. oracle PDB information successful."

exit 0
