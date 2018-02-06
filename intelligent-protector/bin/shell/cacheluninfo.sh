#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

############################################################################################
#program name:          cacheluninfo.sh     
#function:              cache database message collect
#author:
#time:                  2016-02-19
#function and description:  
# function              description
# rework:               First Programming
# author:
# time:
# explain:
############################################################################################

##############parameter###############
 
AGENT_ROOT_PATH=$1
PID=$2
NeedLogFlg=1

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/cacheluninfo.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/cachefunc.sh"
#load the agent function library script

INSFLG=1

ArgFile="$TMP_PATH/ARG$PID"
TMP_FILE="$TMP_PATH/TMP$PID"
DBPATH="$TMP_PATH/DBPATH$PID.txt"
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${PID}"
INPUTINFO=`cat "$PARAM_FILE"`

###############set filename##############

VOLTYPE_NOVOLUME=0
VOLTYPE_LINUXLVM=1
VOLTYPE_VXVM=2
VOLTYPE_AIXLVM=3
VOLTYPE_HPLVM=4

STORAGETYPE_FS=0
sysName=$(uname)

GetStorInfoOfAIX()
{
    CACHE_DB_DIR=$1
    FILE_SYSTEM=`df -i "${CACHE_DB_DIR}" 2>>"$LOG_FILE_NAME"| sed '1d' | ${MYAWK} '{print $1}'|sed -n '1p'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec df -i $CACHE_DB_DIR failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    Log "INFO:StorageType is fs on AIX LVM,filesystem is [$FILE_SYSTEM]."
    DEVICE_NAME=`RDsubstr $FILE_SYSTEM 6`  ###Remove the heading "/dev/"  for file  
    LV_NAME="$DEVICE_NAME"
    MOUNT_POINT=`mount |grep -w "$FILE_SYSTEM" |${MYAWK} '{print $2}'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec mount | grep -w $FILE_SYSTEM failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    VG_NAME=`lslv -L ${LV_NAME} 2>>"$LOG_FILE_NAME" | grep 'VOLUME GROUP:' | ${MYAWK} '{print $6}'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec lslv -L ${LV_NAME} failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    PV_LIST=`lsvg -p ${VG_NAME} 2>>"$LOG_FILE_NAME"|sed '1,2d'|${MYAWK} -F " " '{print $1}'`
    
    if [ $? != 0 ] || [ -z "${PV_LIST}" ]
    then
        Log "ERROR:exec lsvg -p ${VG_NAME} failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    for PV_NAME in $PV_LIST
    do
        DISK_NAME="/dev/$PV_NAME" 
        echo "${FILE_SYSTEM}:$VG_NAME:$MOUNT_POINT:$DISK_NAME:$VOLTYPE_AIXLVM:$STORAGETYPE_FS" >> "${RESULT_FILE}.tmp"
    done 
}

GetRedhatDbInfo()
{
    if [ $# -ne 1 ]
    then
        Log "ERROR:param count error."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    CONF_FILE=$1
    Count=0
    Flag=0
    DB_NAME=""
    DB_DIR=""
    while read line
    do
        Flag=`expr $Flag + 1`
        if [ x"$line" = x"[Databases]" ]
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
		
        if [ x"$line" = x"[Namespaces]" ] || [ x"$line" = x"" ]
        then
           break
        fi

        DB_NAME=`echo $line | ${MYAWK} -F "=" '{print $1}'`
        if [ $? != 0 ]
        then
            Log "ERROR:get DB_NAME from conf file `basename $CONF_FILE` failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
		
        DB_DIR=`echo $line | ${MYAWK} -F "=" '{print $2}'`
        if [ $? != 0 ]
        then
            Log "ERROR:get DB_DIR from conf file `basename $CONF_FILE` failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        Log "INFO:DB_DIR for db($DB_NAME):$DB_DIR"
        echo "$DB_DIR" >> "$DBPATH"
        if [ x"$line" = x"[Namespaces]" ]
        then
           break
        fi
    done < "$CONF_FILE"
}

GetStorInfoOfRedhat()
{
    CACHE_DB_DIR=$1
    FILE_SYSTEM=`df -i "${CACHE_DB_DIR}" 2>>"$LOG_FILE_NAME"| sed '1d' | ${MYAWK} '{print $1}'|sed -n '1p'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec df -i $CACHE_DB_DIR failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    Log "INFO:StorageType is fs on Redhat LVM,filesystem is [$FILE_SYSTEM]."
    DEVICE_NAME=`RDsubstr $FILE_SYSTEM 6`  ###Remove the heading "/dev/"  for file  
    LV_NAME="$DEVICE_NAME"
    MOUNT_POINT=`mount |grep -w "$FILE_SYSTEM" |${MYAWK} '{print $3}'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec mount | grep -w $FILE_SYSTEM failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    DEVICE_NAME_TMP=`RDsubstr $DEVICE_NAME 1 2`
    DEVICE_NAME=`RDsubstr $DEVICE_NAME 1 3`
    check="/dev/s" 
    counts=`echo $FILE_SYSTEM | tr -cd "/" | wc -c`
    echo $FILE_SYSTEM | grep ^$check &>/dev/null
    if [ $? = 0 ] && [ "$counts" = "2" ];then
        echo "${FILE_SYSTEM}:$VG_NAME:$MOUNT_POINT:/dev/$DEVICE_NAME:$VOLTYPE_AIXLVM:$STORAGETYPE_FS" >> "${RESULT_FILE}.tmp"      
    else
        VG_NAME=`lvdisplay ${FILE_SYSTEM} 2>>"$LOG_FILE_NAME" | grep 'VG Name' |${MYAWK} '{print $3}'`
        if [ $? != 0 ] || [ -z "${VG_NAME}" ]
        then
            Log "ERROR:exec lvdisplay ${FILE_SYSTEM} failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        PV_LIST=`vgdisplay -v ${VG_NAME} 2>>"$LOG_FILE_NAME"|grep 'PV Name'|${MYAWK} -F " " '{print $3}'`
        if [ $? != 0 ] || [ -z "${PV_LIST}" ]
        then
            Log "ERROR:exec vgdisplay -v ${VG_NAME} failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        for PV_NAME in $PV_LIST
        do
            Log "start: $PV_NAME"
            DISK_NAME="$PV_NAME"
            len=`echo ${#DISK_NAME}` 
            if [ "$len" = "9" ];then
                DISK_NAME=`echo ${DISK_NAME:0:8}`
            fi
            echo "${FILE_SYSTEM}:$VG_NAME:$MOUNT_POINT:$DISK_NAME:$VOLTYPE_AIXLVM:$STORAGETYPE_FS" >> "${RESULT_FILE}.tmp"
        done 
    fi
}

GetWIJDir()
{
    Log "INFO:Begin get WIJ dir info."
    Count=0
    Flag=0
    while read line
    do
        Flag=`expr $Flag + 1`
        if [ x"$line" = x"[config]" ]
        then
            break
        fi
    done < "$INST_DIR/$INST_FILE"
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
    done < "$INST_DIR/$INST_FILE"
    Log "INFO:End get WIJ dir info."
}

GetJRNDir()
{
    Log "INFO:Begin get JRN dir info."
    Count=0
    Flag=0
    while read line
    do
        Flag=`expr $Flag + 1`
        if [ x"$line" = x"[Journal]" ]
        then
            break
        fi
    done < "$INST_DIR/$INST_FILE"
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

GetValue "$INPUTINFO" INSTNAME
INST_NAME=$ArgValue

Log "INFO:INSTNAME=$INST_NAME"

INST_STAT=`ccontrol $INST_NAME | grep status | ${MYAWK} -F":    " '{print $2}' | ${MYAWK} -F"," '{print $1}'`
if [ x"$INST_STAT" = x"down" ]
then
    Log "ERROR:get instance($INST_NAME) status is $INST_STAT."
    exit $ERROR_INSTANCE_NOSTART
fi

INST_NAMES=`ccontrol view | grep "Instance" | ${MYAWK} -F "'" '{print $2}'`
if [ $? != 0 ]
then
    Log "ERROR:get INST_NAMES failed."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi
INST_NAME_TMP=`echo $INST_NAME | tr 'a-z' 'A-Z'`
for i in $INST_NAMES
do
    if [ $INST_NAME_TMP = $i ]
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

INST_DIR=`ccontrol view $INST_NAME | sed -n '3p' | ${MYAWK} -F ": " '{print $2}'`
if [ $? != 0 ]
then
    Log "ERROR:get instance($INST_STATUS) dir failed."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi
INST_FILE=`ccontrol view $INST_NAME | sed -n '5p' | ${MYAWK} -F ":" '{print $2}' | ${MYAWK} '{print $1}'`
if [ $? != 0 ]
then
    Log "ERROR:get instance($INST_STATUS) conf file failed."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

touch "$DBPATH"
chmod 666 "$DBPATH" 

if [ ! -f "$DBPATH" ]
then
    Log "ERROR:$DBPATH is not exist."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

if [ "$sysName" = "AIX" ]
then
    GetAllDbNames "$INST_DIR/$INST_FILE"

    ALLDBNAME=$DB_NAMES

    Log "INFO:Begin to get instance info."

	#get db dir
    for SUBDBNAME in ${ALLDBNAME}
    do
	    Log "INFO:Begin to get db($SUBDBNAME) info."
	    GetCacheDbDir "$INST_DIR/$INST_FILE" $SUBDBNAME
	    CACHE_DIR="$DB_DIR"
	    if [ -f "$CACHE_DIR/CACHE.DAT" ]
	    then
		    echo "$CACHE_DIR" >> "$DBPATH"
	    else
		    Log "WARN:CACHEINSTANCE=$INST_NAME,DBNAME=$SUBDBNAME，CACHE.DAT not found."
	    fi
	    Log "INFO:End to get db($SUBDBNAME) info."
    done
fi
if [ "$sysName" = "Linux" ]
then
    GetRedhatDbInfo "$INST_DIR/$INST_FILE"
fi

#get WIJ dir
GetWIJDir
#get jrn dir
GetJRNDir
#get conf file dir
Log "INFO:Begin get conf file dir info."
echo "$INST_DIR" >> "$DBPATH"
Log "INFO:conf file dir:$INST_DIR."
Log "INFO:End get conf file dir info."

cat "${DBPATH}">>"$LOG_FILE_NAME"
Log "INFO:End to get instance info."

if [ -f "${RESULT_FILE}.tmp" ]
then
    rm "${RESULT_FILE}.tmp"
fi

touch "${RESULT_FILE}.tmp"

Log "INFO:Begin get storage info of instance."
while read line
do
    if [ "$sysName" = "AIX" ]
    then
        GetStorInfoOfAIX ${line}
    elif [ "$sysName" = "Linux" ]
    then
        GetStorInfoOfRedhat ${line}
    else
        exit -1
    fi
done < "${DBPATH}"

cat "${RESULT_FILE}.tmp" |sort|uniq > "${RESULT_FILE}"
cat "${RESULT_FILE}" >> "$LOG_FILE_NAME"
Log "INFO:End get storage info of instance."

rm "${RESULT_FILE}.tmp"
rm "$DBPATH"
exit 0
