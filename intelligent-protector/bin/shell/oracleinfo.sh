#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

############################################################################################
#program name:          oracleinfo.sh     
#function:              Oracle database message collect
#author:
#time:                  
#function and description:  
# function              description
# rework:               First Programming
# author:
# time:
# explain:
############################################################################################
AGENT_ROOT_PATH=$1
PID=$2

DBUser=""
GridUser=""

. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/oraclefunc.sh"

#===define these for the functions of agent_func.sh and oraclefunc.sh===
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/oracleinfo.log"
#for GetOracleVersion
ORA_VERSION=
ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleBasePath,GetOracleHomePath
IN_ORACLE_BASE=
IN_ORACLE_HOME=
# oracle temp file
ORATMPINFO="${TMP_PATH}/ORATMPINFO${PID}.txt"
SIDTMPINFO="${TMP_PATH}/SIDTMPINFO${PID}.txt"
#===define these for the functions of agent_func.sh===

#===define these for local script===
PARAMFILENAME=
ORAPWDFILENAME=
#===define these for local script===

#get oracle user name
getOracleUserInfo
if [ $? -ne 0 ]
then
    Log "Get Oracle user info failed."
    exit 1
fi

#check oracle user exists
CheckOracleUserExists
if [ $? -ne 0 ]
then
    Log "oracle user not exists."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

#get user shell type
GetUserShellType

#get oracle base path
GetOracleBasePath

#get oracle home path
GetOracleHomePath

#get Oracle version
GetOracleVersion

CheckParameterFile()
{
    PARAM_FILE_NAME=$1
    #first copy init file to tmp directory
    if [ ! -f "${PARAM_FILE_NAME}" ]
    then
        Log "CheckParameterFile:`basename ${PARAM_FILE_NAME}` not exists."
        return 0
    fi

    TMP_PARAM_FILE="$TMP_PATH/paramFile${STRSID}$PID.ora"
    cp -f "${PARAM_FILE_NAME}" "${TMP_PARAM_FILE}"

    ISFIND=0
    for line in `cat "${TMP_PARAM_FILE}"`
    do
        TMP=`echo ${line} | tr [a-z] [A-Z]`
        echo $TMP | grep "SPFILE" > /dev/null
        if [ "$?" != "0" ]
        then
            echo $TMP | grep "CONTROL_FILES=" > /dev/null
            if [ "$?" != "0" ]
            then
                continue
            fi
        fi

        TMP=`echo $TMP | $MYAWK -F "=" '{print $2}'`
        TMP=`echo $TMP | sed 's/ //g'`
        TMP=`RDsubstr $TMP 2 1`

        if [ "$TMP" = "+" ]
        then
            ISFIND=1
            break
        fi
    done

    DeleteFile "${TMP_PARAM_FILE}"

    if [ "$ISFIND" = "1" ]
    then
        return 1
    fi
    
    return 0
}

# -------------------------------------------------------
# function£ºCheckIsASM()
# description: check Oracle is ASM type
# para: instants 
# return£ºisASM
# -------------------------------------------------------
CheckIsASM()
{
    STRSID=$1
    #first copy init file to tmp directory
    CheckParameterFile "${IN_ORACLE_HOME}/dbs/init${STRSID}.ora"
    if [ "$?" -eq "1" ]
    then
       return 1
    fi

    CheckParameterFile "${IN_ORACLE_HOME}/dbs/spfile${STRSID}.ora"
    if [ "$?" -eq "1" ]
    then
       return 1
    fi

   return 0
}

# ----------Get oracle database information---------
Log "Begin to check oracle version."
MAIN_VERSION=`RDsubstr $ORA_VERSION 1 2`
if [ "${MAIN_VERSION}" != "10" ] && [ "${MAIN_VERSION}" != "11" ] && [ "${MAIN_VERSION}" != "12" ]
then
    Log "Oracle version($ORA_VERSION) is not supported."
    exit 1
fi

Log "Begin to read oracle mapping info of database and instance info."
DeleteFile "${ORATMPINFO}"
for DB_NAME in `ls ${IN_ORACLE_BASE}/admin`
do
    if [ "${MAIN_VERSION}" = "10" ]
    then
        for SID_NAME in `ls -l ${IN_ORACLE_BASE}/admin/${DB_NAME}/bdump/alert_*.log | $MYAWK '{print $NF}'`
        do
            SID_NAME=`echo ${SID_NAME} | sed 's/.*alert_\(.*\)\.log/\1/'`
            if [ -z "${SID_NAME}" ]
            then
                Log "DBNAME=${DB_NAME} could not found alert file."
                continue
            fi
            
            echo "${SID_NAME} ${DB_NAME}" >> "${ORATMPINFO}"
        done
    else
        DB_NAME_LOWER=`echo ${DB_NAME} | tr '[A-Z]' '[a-z]'`
        
        for SID_NAME in `ls -l ${IN_ORACLE_BASE}/diag/rdbms/${DB_NAME_LOWER} | grep "^d" | $MYAWK '{print $NF}'`
        do
            echo "${SID_NAME} ${DB_NAME}" >> "${ORATMPINFO}" 
        done
    fi
done

Log "Begin to read oracle instance list."
DeleteFile "${SIDTMPINFO}"
for SID_NAME_IN in `ls -l ${IN_ORACLE_HOME}/dbs/init*.ora | $MYAWK '{print $NF}'`
do
    SID_NAME=`echo ${SID_NAME_IN} | sed 's/.*init\(.*\)\.ora/\1/'`
    if [ -z "${SID_NAME}" ]
    then
        Log "Intance=${SID_NAME_IN} could not get instance name."
        continue
    fi
    echo ${SID_NAME} >> ${SIDTMPINFO}
done

for SID_NAME_IN in `ls -l ${IN_ORACLE_HOME}/dbs/spfile*.ora | $MYAWK '{print $NF}'`
do
    SID_NAME=`echo ${SID_NAME_IN} | sed 's/.*spfile\(.*\)\.ora/\1/'`
    if [ -z "${SID_NAME}" ]
    then
        Log "Intance=${SID_NAME_IN} could not get instance name."
        continue
    fi
    
    cat "${SIDTMPINFO}" | grep "^${SID_NAME}$" > /dev/null
    if [ $? -eq 0 ]
    then
        continue
    fi
    echo ${SID_NAME} >> ${SIDTMPINFO}
done

Log "Begin to read oracle all information."
DeleteFile "${RESULT_FILE}"
touch "${RESULT_FILE}"
while read SID_NAME
do
    if [ -z "${SID_NAME}" ]
    then
        continue
    fi

    #check ASM instance
    if [ "`RDsubstr ${SID_NAME} 1 1`" = "+" ]
    then
        continue
    fi
    
    # database asm status
    CheckIsASM ${SID_NAME}
    ISASM=$?
    # ----------database status---------
    STATUS=1
    ps -aef | sed 's/ *$//g' | grep -i "ora_...._${SID_NAME}$" | grep -v "grep" > /dev/null
    if [ "$?" = "0" ]
    then
        STATUS=0
    else
        STATUS=1
    fi
    
    FLG_GETDBNAME=0
    #if a sid have more than one database, ignore
    DB_NUM=`cat "${ORATMPINFO}" | grep "^${SID_NAME} " | wc -l`
    if [ ${DB_NUM} -gt 1 ]
    then
        Log "SID_NAME=${SID_NAME} map more than one oracle database."
        Log "${ORA_VERSION};${SID_NAME};;${STATUS};${ISASM};${IN_ORACLE_HOME}"
        echo ${ORA_VERSION}";"${SID_NAME}";;"${STATUS}";"${ISASM}";"${IN_ORACLE_HOME} >> "${RESULT_FILE}"
        continue
    else
        #find db name
        for DB_NAME in `cat "${ORATMPINFO}" | grep "^${SID_NAME} " | $MYAWK '{print $2}'`
        do
            FLG_GETDBNAME=1
            DEFULTLEN=8
            DB_NAME_STR=${DB_NAME}
            NOWLEN=`echo "${DB_NAME}" | $MYAWK '{print length()}'`
            if [ "$NOWLEN" -gt "$DEFULTLEN" ]
            then
                DB_NAME_STR=`RDsubstr $DB_NAME 1 8`
            fi
            Log "${ORA_VERSION};${SID_NAME};${DB_NAME_STR};${STATUS};${ISASM};${IN_ORACLE_HOME}"
            echo ${ORA_VERSION}";"${SID_NAME}";"${DB_NAME_STR}";"${STATUS}";"${ISASM}";"${IN_ORACLE_HOME} >> "${RESULT_FILE}"
        done
        
        if [ "${FLG_GETDBNAME}" -eq "0" ]
        then
            Log "instance ${SID_NAME} can not found database name."
            Log "${ORA_VERSION};${SID_NAME};;${STATUS};${ISASM};${IN_ORACLE_HOME}"
            echo ${ORA_VERSION}";"${SID_NAME}";;"${STATUS}";"${ISASM}";"${IN_ORACLE_HOME} >> "${RESULT_FILE}"
        fi
    fi
done < "${SIDTMPINFO}"

DeleteFile "${ORATMPINFO}"
DeleteFile "${SIDTMPINFO}"
exit 0
