#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#input:$1=conf file
GetAllDbNames()
{
    if [ $# -ne 1 ]
    then
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    CONF_FILE=$1
    Count=0
    Flag=0
    DB_NAMES=""
    while read line
    do
        Flag=`expr $Flag + 1`
        if [ x"$line" = x"[Datasets]" ]
        then
            break
        fi
    done < "$CONF_FILE"
    while read line
    do
        Count=`expr $Count + 1`

        if [ $Flag -ge $Count ]
        then
            continue
        fi
        if [ x"$line" = x"[Namespaces]" ]
        then
           break
        fi
        DATASET=`echo $line | cut -c 1-8`
        if [ x"$DATASET" = x"DataSet_" ]
        then
            DBNAME=`echo $line | ${MYAWK} -F "=" '{print $2}' | ${MYAWK} -F "," '{print $1}'`
            if [ $? != 0 ]
            then
                Log "ERROR:get DBNAME from conf file `basename $CONF_FILE` failed."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
            DB_NAMES="$DB_NAMES"" $DBNAME"
        fi
        if [ x"$line" = x"[Namespaces]" ]
        then
           break
        fi
    done < "$CONF_FILE"
}

#input:$1=conf file,$2=db name
GetCacheDbDir()
{
    if [ $# -ne 2 ]
    then
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    CONF_FILE=$1
    DB_NAME=$2
    Count=0
    Flag=0
    DB_DIR=""
    DB_DIR_TMP=""
    INST_DIR=`dirname "$CONF_FILE"`
    while read line
    do
        Flag=`expr $Flag + 1`
        if [ x"$line" = x"[Datasets]" ]
        then
            break
        fi
    done < "$CONF_FILE"
    while read line
    do
        Count=`expr $Count + 1`

        if [ $Flag -ge $Count ]
        then
            continue
        fi
        if [ x"$line" = x"[Namespaces]" ]
        then
           break
        fi
        DB_NAME_INNER=`echo $line | ${MYAWK} -F "=" '{print $2}' | ${MYAWK} -F "," '{print $1}'`
        if [ $? != 0 ]
        then
            Log "ERROR:analysis db name from conf file `basename $CONF_FILE` failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        
        if [ x"$DB_NAME" = x"$DB_NAME_INNER" ]
        then
            Log "INFO:get db($DB_NAME) config"
            INDEX_PATH_TMP=`echo $line | ${MYAWK} -F "=" '{print $2}' | ${MYAWK} -F "," '{for(x=1;x<=NF;x++) print $x}' | sed -n '2,$p' | wc -l`
            if [ $? != 0 ]
            then
                Log "ERROR:get db($DB_NAME) tmp path index failed."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
            INDEX_PATH=`expr $INDEX_PATH_TMP - 5`
            if [ -f "$TMP_FILE" ]
            then
                rm -rf "$TMP_FILE"
            fi
            
            echo $line | ${MYAWK} -F "=" '{print $2}' | ${MYAWK} -F "," '{for(x=1;x<=NF;x++) print $x}' | sed -n '2,$p' | sed -n '1,'"$INDEX_PATH"'p' > "$TMP_FILE"
            if [ $? != 0 ]
            then
                Log "ERROR:get db($DB_NAME) tmp path failed."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
            while read line
            do
                DB_DIR_TMP="$DB_DIR_TMP","$line"
            done < "$TMP_FILE"
            
            if [ -f "$TMP_FILE" ]
            then
                rm -rf "$TMP_FILE"
            fi
            
            DB_DIR_TMP=`echo $DB_DIR_TMP | sed 's/^,//'`
            if [ x"$DB_DIR_TMP" = x"" ]
            then
                DB_DIR="$INST_DIR"/mgr
                Log "INFO:DB_DIR for db($DB_NAME):$DB_DIR"
                break
            fi
            
            DB_DIR_PRE=`echo $DB_DIR_TMP | cut -c 1`
            if [ $DB_DIR_PRE = "/" ]
            then
                DB_DIR="$DB_DIR_TMP"
            else
                DB_DIR="$INST_DIR"/mgr/$DB_DIR_TMP
            fi
            Log "INFO:DB_DIR for db($DB_NAME):$DB_DIR"
            break
        fi
    done < "$CONF_FILE"
}

#input:$1=conf file
GetWIJDir()
{
    Log "INFO:Begin get WIJ dir info."
    Count=0
    Flag=0
    CONF_FILE=$1
    INST_DIR=`dirname "$CONF_FILE"`
    while read line
    do
        Flag=`expr $Flag + 1`
        if [ x"$line" = x"[config]" ]
        then
            break
        fi
    done < "$CONF_FILE"
    while read line
    do
        Count=`expr $Count + 1`

        if [ $Flag -ge $Count ]
        then
            continue
        fi
        if [ x"$line" = x"[Startup]" ]
        then
           break
        fi
        CONF_NAME=`echo $line | ${MYAWK} -F "=" '{print $1}'`
        if [ x"$CONF_NAME" = x"wijdir" ]
        then
            WIJ_DIR=`echo $line | ${MYAWK} -F "=" '{print $2}'`
            if [ x"$WIJ_DIR" = x"" ]
            then
                WIJ_DIR="$INST_DIR/mgr"
            fi
            Log "INFO:WIJ dir:$WIJ_DIR"
            echo "$WIJ_DIR" >> "$DBPATH"
            break
        fi
    done < "$CONF_FILE"
    Log "INFO:End get WIJ dir info."
}

#input:$1=conf file
GetJRNDir()
{
    Log "INFO:Begin get JRN dir info."
    Count=0
    Flag=0
    CONF_FILE=$1
    INST_DIR=`dirname "$CONF_FILE"`
    while read line
    do
        Flag=`expr $Flag + 1`
        if [ x"$line" = x"[Journal]" ]
        then
            break
        fi
    done < "$CONF_FILE"
    while read line
    do
        Count=`expr $Count + 1`

        if [ $Flag -ge $Count ]
        then
            continue
        fi
        if [ x"$line" = x"[Shadowing]" ]
        then
           break
        fi
        CONF_NAME=`echo $line | ${MYAWK} -F "=" '{print $1}'`
        if [ x"$CONF_NAME" = x"CurrentDirectory" ]
        then
            JRN_DIR=`echo $line | ${MYAWK} -F "=" '{print $2}'`
            if [ x"$JRN_DIR" = x"" ]
            then
                JRN_DIR="$INST_DIR/mgr/journal"
            fi
            Log "INFO:JRN dir:$JRN_DIR."
            echo "$JRN_DIR" >> "$DBPATH"
        fi
        if [ x"$CONF_NAME" = x"AlternateDirectory" ]
        then
            ALT_JRN_DIR=`echo $line | ${MYAWK} -F "=" '{print $2}'`
            if [ x"$ALT_JRN_DIR" = x"" ]
            then
                ALT_JRN_DIR="$INST_DIR/mgr/journal"
            fi
            Log "INFO:Alter JRN dir:$ALT_JRN_DIR."
            echo "$ALT_JRN_DIR" >> "$DBPATH"
        fi
    done < "$INST_DIR/$INST_FILE"
    Log "INFO:End get JRN dir info."
}

IsInstanceCorrect()
{
    INST_NAME=$1
    INST_NAMES=`ccontrol view | grep "Instance" | ${MYAWK} -F "'" '{print $2}'`
    if [ $? != 0 ]
    then
        Log "ERROR:get INST_NAMES failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi

    INST_NAME_TMP=`echo $INST_NAME | tr 'a-z' 'A-Z'`

    for i in $INST_NAMES
    do
        if [ x"$INST_NAME_TMP" = x"$i" ]
        then
            INSFLG=0
            break
        fi
    done
    
    if [ "$INSFLG" = "1" ]
    then
        Log "ERROR:Instance $INST_NAME is not correct."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
}