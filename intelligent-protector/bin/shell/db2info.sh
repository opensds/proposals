#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

AGENT_ROOT_PATH=$1
ID=$2
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/db2info.log"

#load the agent function library script
NeedLogFlg=1
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

DB_COUNT=0
DB_TEMPCOUNT=0
DB_SETUPPATH=""
DB_VERSION=""
DB_LOCALINSTANCE=""
DB_TEMP=""


#############set tmpfile name#############
RESULT_FILE="${AGENT_ROOT_PATH}/tmp/${RESULT_TMP_FILE_PREFIX}${ID}"
DB_SETUP_PATH="$TMP_PATH/SETUP${ID}"
DB_VERSION_PATH="$TMP_PATH/LEVEL${ID}"
DB_DI_PATH="$TMP_PATH/DI${ID}"
DB_DI_PATH_TEMP="$TMP_PATH/DI_TMP${ID}"
DB_INSTANCE_STATE="$TMP_PATH/IS${ID}"
DB_TEMP_FILE="$TMP_PATH/TPF${ID}"

TmpFile="$TMP_PATH/TmpFile${ID}.txt"

OLDPATH=`pwd`
############################################################################################
# function name: GetDBMSG()
# aim:           write instance,databasename and state in RESULT_FILE file
# input:         instance name;instance state;file path
# output:        file $RESULT_FILE
############################################################################################
GetDBMSG()
{
    DB_LOCALINSTANCE=""
    DB_LOCALINSTANCE=$1
    INSTANCESTATE=""
    INSTANCESTATE=$2
    DB_DI_PATH="";
    DB_DI_PATH=$3

    Count=0
    Flag=0
    DB_NAME="--"
    REALNAME="--"
    while read line
    do
        Count=`expr $Count + 1`
        DB_TEMP=`echo $line | $MYAWK -F ">" '{print $1}'`
        
        if [ "$DB_TEMP" = "[DB" ]
        then
            Flag=`expr $Count + 2`
        fi

        if [ $Flag -eq $Count ]
        then
            DB_TEMP=`echo $line | $MYAWK -F "=" '{print $1}'`
            if [ "DBName" = "$DB_TEMP" ]
            then
                REALNAME=`echo $line | $MYAWK -F "=" '{print $2}'|tr -d ""` 
                echo $DB_LOCALINSTANCE":"$REALNAME":"$DB_VERSION":"$INSTANCESTATE>>"$RESULT_FILE"
                echo $DB_LOCALINSTANCE":"$REALNAME":"$DB_VERSION":"$INSTANCESTATE>>"$LOG_FILE_NAME"
            fi    
        fi        
    done < "$DB_DI_PATH"

    return 0
}

#############get setup path#############
Log "INFO:Begin to get db2 setup path."
cd "/usr/local/bin"
if [ $? != 0 ]
then
    Log "INFO:cd /usr/local/bin erro."
    cd "$OLDPATH"
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

./db2ls > "$DB_SETUP_PATH"
if [ "$?" = "0" ]
then
    DB_SETUPPATH=`cat "$DB_SETUP_PATH" | sed -n '4,$p' | $MYAWK '{print $1}'`
else
    Log "INFO:exec db2ls failed."
    DB_SETUPPATH=`db2level 2>>"$LOG_FILE_NAME"| grep "Product is installed at" | $MYAWK -F "\"" '{print $2}'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec db2level failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi	
fi
rm -rf "$DB_SETUP_PATH"

if [ "$DB_SETUPPATH" = "" ]
then
    Log "ERROR:get db2 setup path error."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

Log "INFO:End to get db2 setup path($DB_SETUPPATH)."
DB_ALL_SETUPPATH="$DB_SETUPPATH"
for DB_SETUPPATH in $DB_ALL_SETUPPATH
do
    Log "INFO:The db2 product is on $DB_SETUPPATH."
    #############go into instance path#############
    cd "$DB_SETUPPATH/bin"
    if [ $? != 0 ]
    then
        Log "INFO:cd $DB_SETUPPATH/bin error."
        cd "$OLDPATH"
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi

    #############get LEVEL#############
    Log "INFO:Begin to get db2 level."
    LANG=en_US.utf8   
    DB_VERSION=--

    ./db2level>"$DB_VERSION_PATH"

    if [ $? = 0 ]
    then 
        while read line;
        do
            DB_VERSION_INFO=`echo $line|grep "Informational tokens are"`
            if [ $? = 0 ]
            then
                DB_VERSION=`echo $DB_VERSION_INFO | $MYAWK -F "\"" '{print $2}'`
                break
            fi
        done < "$DB_VERSION_PATH"

        if [ "$DB_VERSION" = "" ]
        then
            DB_VERSION=--
        fi
    fi

    rm -rf "$DB_VERSION_PATH"

    Log "INFO:End to get db2 level."

    ########################main ####################################
    Log "INFO:Begin to get db2 instance list."
    cd "$DB_SETUPPATH/bin"
    DB_INSTANCELIST=`./db2ilist 2>>"$LOG_FILE_NAME"`  
    if [ "$DB_INSTANCELIST" = "" ]
    then
        Log "INFO:The instance list is empty on $DB_SETUPPATH."
        cd "$OLDPATH"
        continue
    fi
    Log "INFO:End to get db2 instance list."

    Log "INFO:Begin to get db info of db2 instance."
    for instlist in $DB_INSTANCELIST
    do

        AIX_CMD=
        if [ "$sysName" = "AIX" ]
        then
            SHELLTYPE=`cat /etc/passwd | grep "^${instlist}:" | ${MYAWK} -F "/" '{print $NF}'`
            if [ "$SHELLTYPE" = "bash" ]
            then
                AIX_CMD=-l
            fi
        fi

        su - $instlist -c "" 2>>"$LOG_FILE_NAME"
        if [ $? != 0 ]
        then
            Log "INFO:su - $instlist error."
            continue
        fi

        #############change user#############
        Log "INFO:Begin to get instance($instlist) status."
        if [ ! -f "$DB_DI_PATH_TEMP" ]
        then
            touch "$DB_DI_PATH_TEMP"
            chmod 666 "$DB_DI_PATH_TEMP"
            if [ ! -f "$DB_DI_PATH_TEMP" ]
            then
                Log "ERROR:$DB_DI_PATH_TEMP is not exist."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        fi

        su - $instlist ${AIX_CMD} -c "db2gcf -s -i $instlist">"$DB_TEMP_FILE"

        su - $instlist ${AIX_CMD} -c "db2cfexp \"$DB_DI_PATH_TEMP\"  maintain"

        INSTANCESTATE=""
        INSTANCESTATE_TEMP=`cat "$DB_TEMP_FILE" | grep 'DB2 State'| $MYAWK -F " " '{print $4}'`
        if [ "$INSTANCESTATE_TEMP" = "Available" ]
        then
            INSTANCESTATE="0"
        else
            INSTANCESTATE="1"
        fi
        rm -rf "$DB_TEMP_FILE"

        Log "INFO:End to get instance($instlist) status($INSTANCESTATE)."
        cat "$DB_DI_PATH_TEMP" | grep -v "Drive">"$DB_DI_PATH"
        cat "$DB_DI_PATH">>"$LOG_FILE_NAME"
        Log "INFO:Begin to get db info of instance($instlist)."
        
        GetDBMSG $instlist $INSTANCESTATE "$DB_DI_PATH"
        
        Log "INFO:End to get db info of instance($instlist)."
        
        rm -rf "$DB_DI_PATH_TEMP"
        rm -rf "$DB_DI_PATH"
    done
done
rm -rf "$TmpFile"

Log "INFO:End to get db info of db2 instance."

exit 0
