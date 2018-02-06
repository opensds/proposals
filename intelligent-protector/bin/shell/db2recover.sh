#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x
##############################################################
# This script can stop/start the db2 databases.
#
## @Company: HUAWEI Tech. Co., Ltd.
#
## @Filename db2_auto_recover.sh
## @Usage db2_auto_recover.sh [thread id] ["DBInstance=xxx;DBName=xxx,xxx;UserName=xxx;Password=xx;CheckType=0/1"
## @Description 
#
## @Options option description in details
## @History
## @Author  
## @Version V100R005
## @Created 04.10.2012 12:36:50
##############################################################
AGENT_ROOT_PATH=$1
ID=$2
NeedLogFlg=1

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/db2recover.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

ArgFile="$TMP_PATH/ARG$ID"
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${ID}"
ERRORINFOFILE="$TMP_PATH/ERRORINFO$ID.txt"
DBINSTNAME=""
DBNAME=""
DB_USERNAME=""
DB_USERPWD=""
EXEFLG=""

INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"

AIX_CMD=

AnalyseErrorInfo_DB2()
{
    ERROR_INFO_FILE="$1"
    ERROR_INFO=`cat "$ERROR_INFO_FILE"`
    echo $ERROR_INFO |grep "SQL30082N"|grep "reason \"24\""|grep "SQLSTATE=08001" > /dev/null 2>&1
    if [ $? = 0 ]
    then
        Log "ERROR:The username or password of db is not correct."
        ERROR_CODE=${ERROR_DB_USERPWD_WRONG}
        return
    fi
 
    echo $ERROR_INFO |grep "SQL1032N"|grep "SQLSTATE=57019" > /dev/null 2>&1
    if [ $? = 0 ]
    then
        Log "ERROR:The instance is not started."
        ERROR_CODE=${ERROR_INSTANCE_NOSTART}
        return
    fi
    ERROR_CODE=$ERROR_SCRIPT_EXEC_FAILED
    return
}

# *****************************Get dbname username and password**************************
GetValue "$INPUTINFO" INSTNAME
INST_NAME=$ArgValue

GetValue "$INPUTINFO" DBNAME
DB_NAME=$ArgValue

GetValue "$INPUTINFO" DBUSERNAME
DB_USERNAME=$ArgValue

GetValue "$INPUTINFO" DBPASSWORD
DB_USERPWD=$ArgValue

GetValue "$INPUTINFO" OPERTYPE
OPERTYPE=${ArgValue}

DB_USERPWD1=\'${DB_USERPWD}\' 
  
#******************************

Log "INST_NAME=$INST_NAME;DBNAME=$DB_NAME;DBUSERNAME=$DB_USERNAME;OPERTYPE=$OPERTYPE"

# *****************************set envionment variable ************************************
if [ "$sysName" = "AIX" ]
then
    SHELLTYPE=`cat /etc/passwd | grep "^${DB_USERNAME}:" | ${MYAWK} -F "/" '{print $NF}'`
    if [ "$SHELLTYPE" = "bash" ]
    then
        AIX_CMD=-l
    fi
fi

PATH=`su - $DB_USERNAME ${AIX_CMD} -c 'echo $PATH'`:$PATH
SHELLTYPE=`cat /etc/passwd | grep "^${LOGNAME}:" | ${MYAWK} -F "/" '{print $NF}'`
if [ "$SHELLTYPE" = "csh" ]
then
    setenv DB2INSTANCE $INST_NAME
else
    DB2INSTANCE=$INST_NAME
    export DB2INSTANCE
fi
#******************************

# ******************************CheckType=0,ShutDown DataBase**********************************
if [ "$OPERTYPE" = "0" ]
then
    # *****************************stop the databases**************************
    su - $DB_USERNAME ${AIX_CMD} -c "db2start" > /dev/null
    
    tmp_result="$(su - $DB_USERNAME ${AIX_CMD} -c "db2stop force")" 
    if [ $? -eq 0 ]
    then
        Log "Successful to stop db2."
        exit 0 
    else
        Log "Failed to stop db2."
        Log "$(echo ${tmp_result})"
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
else
    # *****************************restart the database**************************
    #Restart instance
    su - $DB_USERNAME ${AIX_CMD} -c "db2start" > /dev/null
 
    #Resume Databases  
    db2 restart db $DB_NAME user $DB_USERNAME using $DB_USERPWD1 write resume 2>>"$ERRORINFOFILE" 1>>"$ERRORINFOFILE"
    if [ $? -eq 0 ]
    then
        cat "$ERRORINFOFILE" >> "$LOG_FILE_NAME"
        DeleteFile "$ERRORINFOFILE"
        Log "Successful to resume db $DB_NAME."
        exit 0
    else
        cat "$ERRORINFOFILE" >> "$LOG_FILE_NAME"
        AnalyseErrorInfo_DB2  $ERRORINFOFILE
        DeleteFile "$ERRORINFOFILE"
        
        Log "Failed to resume db $DB_NAME."
        exit $ERROR_CODE
    fi
fi
