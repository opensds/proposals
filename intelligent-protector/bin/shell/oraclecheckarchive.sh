#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#@dest:  to check oracle archive dest using
#@date:  2015-01-24
#@authr: 
USAGE="Usage: ./oraclecheckarchive.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oraclecheckarchive.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
#for GetValue
ArgFile=$$
#for GetArchiveLogMode
GET_ARCHIVE_LOG_MODE_SQL="${TMP_PATH}/get_archive_log_mode${PID}.sql"
ARCHIVE_LOG_MODE_RST="${TMP_PATH}/archive_log_mode_rst${PID}.txt"
#********************************define these for the functions of agent_func.sh********************************

#********************************define these for local script********************************
DBNAME=""
DBUSER=""
DBUSERPWD=""
DBINSTANCE=""
#Threshold of the database archive using, unit(M) 
DBARCHIVE_LIMEN=0

# global var, for kill monitor
ASMGETCAPACITY="${TMP_PATH}/ASMCapacity${PID}.txt"
# Archive Log
ARCHIVEDESTSQL="${TMP_PATH}/ArchiveDestSQL${PID}.sql"
ARCHIVEDESTRST="${TMP_PATH}/ArchiveDestRST${PID}.txt"
ARCHIVE_DEST_LIST="${TMP_PATH}/ArchiveDestList${PID}.txt"
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"
#********************************define these for local script********************************

checkArchiveModeAndArchiveUsing()
{
    # create check archive mode sql
    Log "Begin check archive dest directory."
    #query archive log mode
    GetArchiveLogMode ${DBINSTANCE}
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ] && [ ${RET_CODE} -ne 1 ]
    then
        Log "Get archive log mode failed."
        exit ${RET_CODE}
    fi

    if [ ${RET_CODE} -eq 0 ]
    then
        Log "Archive Mode=No Archive Mode, check archive mode failed."
        exit ${ERROR_ORACLE_NOARCHIVE_MODE}
    fi

    # create check archive mode sql
    Log "Begin check archive dest directory."
    echo "set linesize 300;" > "${ARCHIVEDESTSQL}"
    echo "col DESTINATION for a255;" >> "${ARCHIVEDESTSQL}"
    #status of archive dest
    # VALID - Initialized and available
    # INACTIVE - No destination information
    # DEFERRED - Manually disabled by the user
    # ERROR - Error during open or copy
    # DISABLED - Disabled after error
    # BAD PARAM - Parameter has errors
    # ALTERNATE - Destination is in an alternate state
    #             alter system LOG_ARCHIVE_DEST_STATE_n={enable|defer|alternate}
    #             enable - Specifies that a valid log archive destination can be used for a subsequent archiving operation (automatic or manual). This is the default.
    #             defer - Specifies that valid destination information and attributes are preserved, but the destination is excluded from archiving operations until re-enabled.
    #             alternate - Specifies that a log archive destination is not enabled but will become enabled if communications to another destination fail.
    # FULL - Exceeded quota size for the destination
    echo "select DESTINATION from v\$archive_dest where STATUS='VALID';" >> "${ARCHIVEDESTSQL}"
    echo "exit" >> "${ARCHIVEDESTSQL}"
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"

    Log "Exec SQL to get destination of archive dest(INACTIVE)."
    OracleExeSql "${LOGIN_AUTH}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" 30 >> "${LOG_FILE_NAME}" 2>&1
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
    
    exec 4<&0
    exec <"${ARCHIVE_DEST_LIST}"
    while read line
    do
        STRARCHIVEDEST=$line
        if [ -z "${STRARCHIVEDEST}" ]
        then
            continue
        fi
        IS_RECOVERY_FILE_DEST=0
        FAST_USEDCAPACITY=0
        FAST_ALLCAPACITY=0
        
        Log "STRARCHIVEDEST=${STRARCHIVEDEST}"
        # default dest, need to search direcory
        if [ "${STRARCHIVEDEST}" = "USE_DB_RECOVERY_FILE_DEST" ]
        then
            IS_RECOVERY_FILE_DEST=1
            # get archive dest
            echo "set linesize 300;" > "${ARCHIVEDESTSQL}"
            echo "col NAME for a255;" >> "${ARCHIVEDESTSQL}"
            echo "select NAME from V\$RECOVERY_FILE_DEST;" >> "${ARCHIVEDESTSQL}"
            echo "exit" >> "${ARCHIVEDESTSQL}"
            LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"

            Log "Exec SQL to get name of archive dest."
            OracleExeSql "${LOGIN_AUTH}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" 30 >> "${LOG_FILE_NAME}" 2>&1
            RET_CODE=$?
            if [ "$RET_CODE" -ne "0" ]
            then
                DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
                DeleteFile "${ARCHIVE_DEST_LIST}"
                Log "Get Archive dest failed."
                exit ${RET_CODE}
            fi
    
            STRARCHIVEDEST=`sed -n '/----------/,/^ *$/p' "${ARCHIVEDESTRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1}'`
            Log "STRARCHIVEDEST=${STRARCHIVEDEST}."
            DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
            
            # get archive using
            echo "set linesize 100;" > "${ARCHIVEDESTSQL}"
            echo "select SPACE_LIMIT/1024/1024,SPACE_USED/1024/1024 from V\$RECOVERY_FILE_DEST; " >> "${ARCHIVEDESTSQL}"
            echo "exit" >> "${ARCHIVEDESTSQL}"
            LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"

            Log "Exec SQL to get using of archive dest."
            OracleExeSql "${LOGIN_AUTH}" "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${DBINSTANCE}" 30 >> "${LOG_FILE_NAME}" 2>&1
            RET_CODE=$?
            if [ "$RET_CODE" -ne "0" ]
            then
                DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
                DeleteFile "${ARCHIVE_DEST_LIST}"
                Log "Get Archive dest using failed."
                exit ${RET_CODE}
            fi
            
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
            TMPLINE=`sed -n '/----------/,/^ *$/p' "${ARCHIVEDESTRST}" | sed -e '/----------/d' -e '/^ *$/d' | ${MYAWK} '{print $1";"$2}'`
            for i in ${TMPLINE}
            do
                FAST_ALLCAPACITY=`echo $i | ${MYAWK} -F ";" '{print $1}'`
                FAST_USEDCAPACITY=`echo $i | ${MYAWK} -F ";" '{print $2}'`
                # deal number pot
                FAST_ALLCAPACITY=`echo $FAST_ALLCAPACITY | ${MYAWK} -F "." '{print $1}'`
                FAST_USEDCAPACITY=`echo $FAST_USEDCAPACITY | ${MYAWK} -F "." '{print $1}'`
                # first value
                break
            done
            
            DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
            Log "FAST_ALLCAPACITY=$FAST_ALLCAPACITY;FAST_USEDCAPACITY=$FAST_USEDCAPACITY"
        else
            # check whether the archive dest is exists, if not, the suffix maybe is the perfix of the archive files.
            #.eg 
            # Archive destination            /lvoracle/app/oracle/11.2.0/db_1/dbs/arch/abc
            # the last abc mabe is the perfix of the archive files.
            LASTCHAR=`echo ${STRARCHIVEDEST} | ${MYAWK} '{print substr($NF,length($NF),1)}'`
            if [ "`RDsubstr $STRARCHIVEDEST 1 1`" = "/" ] && [ "${LASTCHAR}" != "/" ] && [ ! -d "${STRARCHIVEDEST}" ]
            then
                FILEPREFIX=`echo ${STRARCHIVEDEST} | ${MYAWK} -F "/" '{print $NF}'`
                LENPREFIX=`echo $FILEPREFIX | ${MYAWK} '{print length($1)}'`
                LENALLDEST=`echo $STRARCHIVEDEST | ${MYAWK} '{print length($1)}'`
                
                LENDEST=`expr $LENALLDEST - $LENPREFIX - 1`
                STRARCHIVEDEST=`RDsubstr $STRARCHIVEDEST 1 $LENDEST`
            fi
            
            if [ ! -d "${STRARCHIVEDEST}" ]
            then
                Log "Archive Dest ${STRARCHIVEDEST} is not exists"
            fi
        fi
        
        Log "STRARCHIVEDEST=${STRARCHIVEDEST}"
        if [ "`RDsubstr $STRARCHIVEDEST 1 12`" = "/dev/raw/raw" ]
        then
            Log "Archive dest ${STRARCHIVEDEST} is the raw, not support check archive dest size."
            continue
        fi
        
        FREECAPACITY=0
        ALLCAPACITY=0
        #file system
        if [ "`RDsubstr $STRARCHIVEDEST 1 1`" = "/" ]
        then
            if [ "${sysName}" = "HP-UX" ]
            then
                FREECAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $1}' | sed -n '1p'`
                FREECAPACITY=`expr $FREECAPACITY / 1024`
                ALLCAPACITY=`df -k $STRARCHIVEDEST | sed '2,4d' | $MYAWK -F ":" '{print $2}' | $MYAWK '{print $1}'`
                ALLCAPACITY=`expr $ALLCAPACITY / 1024`
            elif [ "${sysName}" = "AIX" ]
            then
                FREECAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $3}'`
                FREECAPACITY=`expr $FREECAPACITY / 1024`
                ALLCAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $2}'`
                ALLCAPACITY=`expr $ALLCAPACITY / 1024`
            elif [ "${sysName}" = "SunOS" ]
            then
                FREECAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $4}'`
                FREECAPACITY=`expr $FREECAPACITY / 1024`
                ALLCAPACITY=`df -k $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $2}'`
                ALLCAPACITY=`expr $ALLCAPACITY / 1024`
            else
                LINENUM=`df -m $STRARCHIVEDEST | wc -l`
                if [ $LINENUM = "2" ]
                then
                    FREECAPACITY=`df -m $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $4}'`
                    ALLCAPACITY=`df -m $STRARCHIVEDEST | sed '1d' | $MYAWK '{print $2}'`
                else
                    FREECAPACITY=`df -m $STRARCHIVEDEST | sed '1,2d' | $MYAWK '{print $3}'`
                    ALLCAPACITY=`df -m $STRARCHIVEDEST | sed '1,2d' | $MYAWK '{print $1}'`
                fi
             fi
            Log "file system capacity:ALLCAPACITY=$ALLCAPACITY,FREECAPACITY=$FREECAPACITY."
        fi

        #archive dest
        if [ "`RDsubstr $STRARCHIVEDEST 1 1`" = "+" ]
        then
            ASMSIDNAME=`ps -ef | grep "asm_...._${ASMSIDNAME}" | grep -v "grep" | ${MYAWK} -F '+' '{print $NF}'`
            ASMSIDNAME=`echo ${ASMSIDNAME} | ${MYAWK} -F " " '{print $1}'`
            ASMSIDNAME="+"${ASMSIDNAME}
            Log "Check ASM instance name $ASMSIDNAME."

            #check ASM instance status
            ASMINSTNUM=`ps -ef | grep "asm_...._${ASMSIDNAME}" | grep -v "grep" | wc -l`
            if [ "${ASMINSTNUM}" -eq "0" ]
            then
                Log "The ASM instance is not open."
                DeleteFile "${ARCHIVE_DEST_LIST}"
                exit ${ERROR_ORACLE_ASM_INSTANCE_NOSTART}
            fi

            ASM_DEST=`RDsubstr $STRARCHIVEDEST 2`
            ASM_DISKGROUPNAME=`echo $ASM_DEST | $MYAWK -F "/" '{print $1}'`
            
            touch "${ASMGETCAPACITY}"
            chmod 666 "${ASMGETCAPACITY}"
            #################Execute temporary sql script to get asm diskgoup member###############
            if [ "$VERSION" -ge "112" ]
            then
                SHELLTYPE=`cat /etc/passwd | grep "^${GridUser}:" | ${MYAWK} -F "/" '{print $NF}'`
                CMD_SETENV=
                if [ "$SHELLTYPE" = "csh" ]
                then
                    CMD_SETENV="setenv ORACLE_SID ${ASMSIDNAME}"
                else
                    CMD_SETENV="ORACLE_SID=${ASMSIDNAME};export ORACLE_SID"
                fi
                su - ${GridUser} ${GRID_SHELLTYPE} -c "${CMD_SETENV}; echo 'lsdg ${ASM_DISKGROUPNAME}' | asmcmd >> \"${ASMGETCAPACITY}\"" >> "${LOG_FILE_NAME}" 2>&1
                

                    ALLCAPACITY=`cat "${ASMGETCAPACITY}" | sed -n '2p' | $MYAWK '{print $7}'`
                    FREECAPACITY=`cat "${ASMGETCAPACITY}" | sed -n '2p' | $MYAWK '{print $8}'`
                
                DeleteFile "${ASMGETCAPACITY}"
            else
                SHELLTYPE=`cat /etc/passwd | grep "^oracle:" | ${MYAWK} -F "/" '{print $NF}'`
                if [ "$SHELLTYPE" = "csh" ]
                then
                    CMD_SETENV="setenv ORACLE_SID ${ASMSIDNAME}"
                else
                    CMD_SETENV="ORACLE_SID=${ASMSIDNAME};export ORACLE_SID"
                fi
                
                su - ${DBUser} ${ORACLE_SHELLTYPE} -c "${CMD_SETENV}; echo 'lsdg ${ASM_DISKGROUPNAME}' | asmcmd >> \"${ASMGETCAPACITY}\"" >> "${LOG_FILE_NAME}" 2>&1

                if [ "$VERSION" = "111" ]
                then
                    ALLCAPACITY=`cat "${ASMGETCAPACITY}" | sed -n '2p' | $MYAWK '{print $7}'`
                    FREECAPACITY=`cat "${ASMGETCAPACITY}" | sed -n '2p' | $MYAWK '{print $8}'`
                else
                    ALLCAPACITY=`cat "${ASMGETCAPACITY}" | sed -n '2p' | $MYAWK '{print $8}'`
                    FREECAPACITY=`cat "${ASMGETCAPACITY}" | sed -n '2p' | $MYAWK '{print $9}'`
                fi
                
                DeleteFile "${ASMGETCAPACITY}"
            fi
            Log "ASM capacity:ALLCAPACITY=$ALLCAPACITY,FREECAPACITY=$FREECAPACITY."
        fi
        
        Log "ALLCAPACITY=${ALLCAPACITY},IS_RECOVERY_FILE_DEST=${IS_RECOVERY_FILE_DEST}"
        if [ "${ALLCAPACITY}" -le "0" ] || [ "${ALLCAPACITY}" = "" ] || [ "${FREECAPACITY}" -lt "0" ] || [ "${FREECAPACITY}" = "" ]
        then
            Log "ALLCAPACITY=${ALLCAPACITY} and FREECAPACITY=${FREECAPACITY} is invalid, not to check."
            continue
        fi
        
        Log "FREECAPACITY=${FREECAPACITY};DBARCHIVE_LIMEN=${DBARCHIVE_LIMEN}."
        Log "FAST_ALLCAPACITY=${FAST_ALLCAPACITY};FAST_USEDCAPACITY=${FAST_USEDCAPACITY}."
        if [ "${FAST_ALLCAPACITY}" -gt "0" ] && [ "${FAST_ALLCAPACITY}" != "" ] && [ "${FAST_USEDCAPACITY}" -gt "0" ] && [ "${FAST_USEDCAPACITY}" != "" ]
        then
            FAST_FREECAPACITY=`expr $FAST_ALLCAPACITY - $FAST_USEDCAPACITY`
            Log "FAST_FREECAPACITY=${FAST_FREECAPACITY};FREECAPACITY=${FREECAPACITY}."
            if [ "${FAST_FREECAPACITY}" -lt "${FREECAPACITY}" ]
            then
                FREECAPACITY=${FAST_FREECAPACITY}
                Log "Using flash recover area using."
            fi
        fi

        if [ "${FREECAPACITY}" -lt "${DBARCHIVE_LIMEN}" ]
        then
            Log "Free capacity ${FREECAPACITY}MB is less than ${DBARCHIVE_LIMEN}MB."
            DeleteFile "${ARCHIVE_DEST_LIST}"
            exit ${ERROR_ORACLE_OVER_ARCHIVE_USING}
        else
            Log "Free capacity ${FREECAPACITY}MB is bigger than ${DBARCHIVE_LIMEN}MB."
        fi
    done
    exec 0<&4 4<&-
    
    DeleteFile "${ARCHIVE_DEST_LIST}"
    Log "End check archive dest directory using."
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
#############################################################
GetValue "$INPUTINFO" InstanceName
DBINSTANCE=${ArgValue}

# Reuse the parameter of FrushMode
GetValue "$INPUTINFO" ArchiveThreshold
if [ "$ArgValue" = "" ]
then
    DBARCHIVE_LIMEN=0
else
    DBARCHIVE_LIMEN=$ArgValue
fi

GetValue "$INPUTINFO" UserName
DBUSER=${ArgValue}

GetValue "$INPUTINFO" Password
DBUSERPWD=${ArgValue}

GetValue "$INPUTINFO" OracleHome
IN_ORACLE_HOME=${ArgValue}

GetValue "$INPUTINFO" ASMInstanceName
ASMSIDNAME=$ArgValue
if [ -z "${ASMSIDNAME}" ]
then
    ASMSIDNAME="+ASM"
fi

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

Log "DBINSTANCE=${DBINSTANCE};DBUSER=${DBUSER};DBARCHIVE_LIMEN=${DBARCHIVE_LIMEN};AUTHMODE=$AUTHMODE;IN_ORACLE_HOME=$IN_ORACLE_HOME"

if [ "$DBARCHIVE_LIMEN" -gt "1048576" ]
then
    Log "[warning] DBARCHIVE_LIMEN is greater than 1048576."
fi

#get user shell type
GetUserShellType

#get Oracle version
GetOracleVersion
VERSION=`echo $PREVERSION | tr -d '.'`
#check archive mode & archive using
if [ "${DBARCHIVE_LIMEN}" -ne "0" ]
then
    checkArchiveModeAndArchiveUsing
    if [ "$?" -ne "0" ]
    then
        Log "checkArchiveModeAndArchiveUsing exec failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
else
    Log "DBARCHIVE_LIMEN is zero, not to check archive using."
fi

exit 0
