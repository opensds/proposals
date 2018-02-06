#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#@dest:  application agent for db2
#@date:  2009-05-09
#@authr: 

AGENT_ROOT_PATH=$1
ID=$2
NeedLogFlg=1

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/db2sample.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

INST_NAME=""
DB_NAME=""
DB_USERNAME=""
DB_USERPWD=""
EXEFLG=""
STATUS_SUSPEND="0x00010000"
TMP="TMP"
INSFLG=1
tmp_result=""

ArgFile="$TMP_PATH/ARG$ID"
TMPFILE="${TMP_PATH}/${TMP}$ID.txt"
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${ID}"
ERRORINFOFILE="$TMP_PATH/ERRORINFO$ID.txt"
INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"
RESULT_FILE="${AGENT_ROOT_PATH}/tmp/${RESULT_TMP_FILE_PREFIX}${ID}"
# global var, for kill monitor
MONITORFILENAME="${BIN_PATH}/procmonitor.sh"

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

DB2TimeOut()
{
    "$MONITORFILENAME" $$ db2 $1 "${AGENT_ROOT_PATH}" >>"${LOG_FILE_NAME}" 2>&1 &
}

#############################################################
GetValue "$INPUTINFO" INSTNAME
INST_NAME=$ArgValue

GetValue "$INPUTINFO" DBNAME
DB_NAME=$ArgValue

GetValue "$INPUTINFO" DBUSERNAME
DB_USERNAME=$ArgValue

GetValue "$INPUTINFO" DBPASSWORD
DB_USERPWD=$ArgValue

GetValue "$INPUTINFO" OPERTYPE
EXEFLG=$ArgValue

DB_USERPWD2=\'\"${DB_USERPWD}\"\'
DB_USERPWD1=\'${DB_USERPWD}\'
Log "INFO:INSTNAME=$INST_NAME;DBNAME=$DB_NAME;DBUSERNAME=$DB_USERNAME;EXEFLG=$EXEFLG"

if [ -f "$TMPFILE" ]
then
    rm -rf "$TMPFILE"
fi

AIX_CMD=
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

DB2TimeOut 300

db2 connect to $DB_NAME user $DB_USERNAME using $DB_USERPWD1 2>>"$ERRORINFOFILE" 1>>"$ERRORINFOFILE"
if [ $? -eq 0 ]
then
    db2ilist > "$TMPFILE"
    if [ $? -eq 0 ] 
    then
        while read line
        do
            TMP=`echo $line`
            if [ "$INST_NAME" = "$TMP" ]
            then
                DB2INSTANCE="$INST_NAME"
                INSFLG=0
                db2 disconnect $DB_NAME
                Log "INFO:Connect $DB_NAME by user $DB_USERNAME successful."
                break
            fi
        done < "$TMPFILE"   
    fi
    cat "$ERRORINFOFILE" >> "$LOG_FILE_NAME"
    DeleteFile "$ERRORINFOFILE"
    KillProcMonitor $$
else
    cat "$ERRORINFOFILE" >> "$LOG_FILE_NAME"
    AnalyseErrorInfo_DB2  $ERRORINFOFILE
    DeleteFile "$ERRORINFOFILE"
    
    Log "ERROR:Faild to connect $DB_NAME by user $DB_USERNAME."
    KillProcMonitor $$
    exit $ERROR_CODE
fi

if [ -f "$TMPFILE" ]
then
    rm -rf "$TMPFILE"
fi

if [ "$INSFLG" = "1" ]
then
    Log "ERROR:Instance $INST_NAME is not correct."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

#############################################################
#Start to suspend or resume write for database
DB2TimeOut 300
#Freeze DB
if [ ${EXEFLG} = "0" ]
then
    Log "INFO:Begin to freeze db."
    db2 connect to $DB_NAME user $DB_USERNAME using $DB_USERPWD1 2>>"$LOG_FILE_NAME" 1>>"$LOG_FILE_NAME"
    if [ $? -eq 0 ]
    then
        db2 set write suspend for database 2>>"$LOG_FILE_NAME" 1>>"$LOG_FILE_NAME"
        if [ $? -eq 0 ] 
        then
            db2 commit
            Log "INFO:Freeze $DB_NAME by user $DB_USERNAME successful."
            db2 disconnect $DB_NAME
            KillProcMonitor $$
            exit 0
        else
            Log "ERROR:Freeze $DB_NAME by user $DB_USERNAME failed."
            db2 disconnect $DB_NAME
            KillProcMonitor $$
            exit $ERROR_DB2_SUSPEND_IO_FAILED
        fi
    else
        Log "ERROR:Faild to connect $DB_NAME by user $DB_USERNAME when freeze db."
        KillProcMonitor $$
        exit $ERROR_DB2_SUSPEND_IO_FAILED
    fi

fi

#Thaw DB
if [ ${EXEFLG} = "1" ]
then
    Log "INFO:Begin to thaw db."

    db2 connect to $DB_NAME user $DB_USERNAME using $DB_USERPWD1 2>>"$LOG_FILE_NAME" 1>>"$LOG_FILE_NAME"
    if [ $? -eq 0 ]
    then
        db2 set write resume for database 2>>"$LOG_FILE_NAME" 1>>"$LOG_FILE_NAME"
        if [ $? -eq 0 ]
        then
            Log "INFO:Thaw $DB_NAME by user $DB_USERNAME successful."
            db2 commit
            db2 disconnect $DB_NAME
            KillProcMonitor $$
            exit 0
        else
            Log "ERROR:Thaw $DB_NAME by user $DB_USERNAME failed."
            db2 disconnect $DB_NAME
            KillProcMonitor $$
            exit $ERROR_DB2_RESUME_IO_FAILED
        fi
    else
        Log "ERROR:Faild to connect to $DB_NAME by user $DB_USERNAME when thaw db."
        KillProcMonitor $$
        exit $ERROR_DB2_RESUME_IO_FAILED
    fi
    
fi

#Test DB
if [ ${EXEFLG} = "2" ]
then
    Log "INFO:Begin to test db."
    su - $DB_USERNAME -c ""
    if [ $? -ne 0 ]
    then
        Log "ERROR:The user name $DB_USERNAME is incorrect."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    db2 connect to $DB_NAME user $DB_USERNAME using $DB_USERPWD1 2>>"$LOG_FILE_NAME" 1>>"$LOG_FILE_NAME"
    if [ $? -eq 0 ]
    then
        Log "INFO:Connect $DB_NAME by user $DB_USERNAME successful on test."
        db2 disconnect $DB_NAME
        KillProcMonitor $$
        exit 0
    else
        Log "ERROR:Connect $DB_NAME by user $DB_USERNAME failed on test."
        Log "DB2INSTANCE=$DB2INSTANCE;PATH=$PATH"
        db2 disconnect $DB_NAME
        KillProcMonitor $$
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
fi

#Check DB status

if [ -f "$TMPFILE" ]
then
    rm -rf "$TMPFILE"
fi

if [ ${EXEFLG} = "3" ]
then
    Log "INFO:Begin to query db freeze state."

    touch "$TMPFILE"
    chmod 666 "$TMPFILE"
    su - ${INST_NAME} ${AIX_CMD} -c "db2pd -db ${DB_NAME} -tablespaces | $MYAWK -F \" \" '{print \$10}' | grep \"0x\" >\"$TMPFILE\""
    if [ $? -eq 0 ] 
    then
        Log "INFO:Query db freeze state successful."           
        cat "$TMPFILE" >> "$LOG_FILE_NAME"       
        while read line
        do
            TMP=`echo $line`
            if [ "$STATUS_SUSPEND" = "$TMP" ]
            then
                Log "INFO:The db is in suspend state."
                KillProcMonitor $$ 
                echo "$ERROR_DB_FREEZE_YES">"$RESULT_FILE"
                rm -fr "$TMPFILE"
                exit 0                 
            fi
        done < "$TMPFILE"
        Log "INFO:The db is not in suspend state."
        KillProcMonitor $$ 
        echo "$ERROR_DB_FREEZE_NO">"$RESULT_FILE"
        rm -fr "$TMPFILE"
        exit 0
    else
        Log "ERROR:Faild to select $DB_NAME by user $DB_USERNAME when check DB status."
        Log "INFO:DB2INSTANCE=$DB2INSTANCE;PATH=$PATH"
        KillProcMonitor $$
        rm -fr "$TMPFILE"
        echo "$ERROR_DB_FREEZE_NO">"$RESULT_FILE"
        exit 0
    fi
fi

Log "ERROR:The frush flag:${EXEFLG} is invalid."
KillProcMonitor $$
exit $ERROR_SCRIPT_EXEC_FAILED
