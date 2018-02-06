#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

############################################################################################
#program name:          Db2AdaptiveInfo.sh     
#function:              db2 database message collect
#author:
#time:                  2012-02-21
#function and description:  
# function              description
# rework:               First Programming
# author:
# time:
# explain:
############################################################################################
AGENT_ROOT_PATH=$1
ID=$2
NeedLogFlg=1

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/db2luninfo.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/lvmfunc.sh"

###############set filename##############
ArgFile="$TMP_PATH/ARG$ID"
RESULT_FILE="${AGENT_ROOT_PATH}/tmp/${RESULT_TMP_FILE_PREFIX}${ID}"
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${ID}"


DBFILE="$TMP_PATH/DBFILE$ID.txt"
TABLESAPCEINFO="$TMP_PATH/TSINFO$ID.txt"
ERRORINFOFILE="$TMP_PATH/ERRORINFO$ID.txt"

ERROR_CODE=

LV_PATH=
VG_NAME=
DEVICE_NAME=
DEVICE_PATH=

VOLTYPE_NOVOLUME=0
VOLTYPE_LINUXLVM=1
VOLTYPE_VXVM=2
VOLTYPE_AIXLVM=3
VOLTYPE_HPLVM=4

STORAGETYPE_RAW=1
STORAGETYPE_FS=0

INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"

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


GetValue "$INPUTINFO" INSTNAME
INST_NAME=$ArgValue

GetValue "$INPUTINFO" DBNAME
DB_NAME=$ArgValue

GetValue "$INPUTINFO" DBUSERNAME
DB_USERNAME=$ArgValue

GetValue "$INPUTINFO" DBPASSWORD
DB_USERPWD=$ArgValue

DB_USERPWD2=\'\"${DB_USERPWD}\"\'
Log "INFO:INSTNAME=$INST_NAME;DBNAME=$DB_NAME;DBUSERNAME=$DB_USERNAME"



AIX_CMD=
if [ "$OSTYPE" = "AIX" ]
then
    SHELLTYPE=`cat /etc/passwd | grep "^${INST_NAME}:" | ${MYAWK} -F "/" '{print $NF}'`
    if [ "$SHELLTYPE" = "bash" ]
    then
        AIX_CMD=-l
    fi
fi

##################connect to databse#########################
su - ${INST_NAME} ${AIX_CMD} << EOF
db2 connect to ${DB_NAME} user ${DB_USERNAME} using ${DB_USERPWD2} 1>>"$ERRORINFOFILE" 2>>"$ERRORINFOFILE"
EOF

if [ $? != 0 ]
then
    cat "$ERRORINFOFILE" >> "$LOG_FILE_NAME"
    AnalyseErrorInfo_DB2  $ERRORINFOFILE
    DeleteFile "$ERRORINFOFILE"
    exit $ERROR_CODE
fi

cat "$ERRORINFOFILE">>"$LOG_FILE_NAME"
DeleteFile "$ERRORINFOFILE"
Log "INFO:Begin to get tablespace info."

touch "$DBFILE"
chmod 666 "$DBFILE" 

if [ ! -f "$DBFILE" ]
then
    Log "ERROR:$DBFILE is not exist."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi

touch "$TABLESAPCEINFO"
chmod 666 "$TABLESAPCEINFO"
	
su - ${INST_NAME} ${AIX_CMD} << EOF
db2 connect to ${DB_NAME} user ${DB_USERNAME} using ${DB_USERPWD2}
db2pd -db ${DB_NAME} -dbcfg  | grep "Path to log files (memory)"|$MYAWK '{print \$NF}'>> "$DBFILE"
db2 list tablespaces show detail |sed -n '4,\${p;n;n;n;n;n;n;n;n;n;n;n;n;n;n;n;n;}'|$MYAWK '{print \$NF}'>>"$TABLESAPCEINFO" 
while read line
do
   db2 list tablespace containers for \$line | grep "/" |$MYAWK '{print \$NF}' >>"$DBFILE"
done < "$TABLESAPCEINFO"
EOF

cat "${DBFILE}">>"$LOG_FILE_NAME"
Log "INFO:End to get tablespace info."

if [ -f "${RESULT_FILE}.tmp" ]
then
    rm "${RESULT_FILE}.tmp"
fi

touch "${RESULT_FILE}.tmp"

Log "INFO:Begin get storage info of db files."

while read FILE_PATH
do
    FILE_TYPE=`ls -l ${FILE_PATH} |$MYAWK '{print $1}'`
    if [ x"$(RDsubstr $FILE_TYPE 1 1)" = x"c" ]
    then
        DEVICE_PATH=$FILE_PATH
        IS_VXVM=`echo "$FILE_PATH" | grep "/dev/vx/rdsk"`
        if [ $? = 0 ]
        then
            Log "INFO:The StorageType is rlv on VXVM,rawdevice path is [$DEVICE_PATH]."
            GetStorInfo_VXVM $STORAGETYPE_RAW
        fi
        
        if [ "${sysName}" = "HP-UX" ]
        then
            RAWDEVICE_NAME=`echo $DEVICE_PATH | $MYAWK -F "/" '{print $NF}'`
            LV_NAME=`echo $RAWDEVICE_NAME | cut -b 2-`
            VG_NAME=`echo $DEVICE_PATH | $MYAWK -F "/" '{print $3}'`
            LV_PATH="/dev/$VG_NAME/$LV_NAME"
            IS_HP_LVM=`lvdisplay $LV_PATH`
            if [ $? = 0 ]
            then
                Log "INFO:The StorageType is rlv on HP LVM,rawdevice path is [$DEVICE_PATH]."
                GetStorInfo_HP_LVM $STORAGETYPE_RAW
            else
                Log "ERROR:NO support other raw device on HP-UX."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        elif [ "${sysName}" = "Linux" ]
        then
            Log "INFO:The StorageType is raw on Linux."
            MAJORNUM=`raw -q ${DEVICE_PATH} | ${MYAWK} '{print $5}'`
            MINORNUM=`raw -q ${DEVICE_PATH} | ${MYAWK} '{print $7}'`
            if [ -z "${MAJORNUM}" ]
            then
                Log "ERROR:excute: raw -q $DEVICE_PATH failed."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
            DEVICE_NAME=`ls -l /dev/sd* | ${MYAWK} -v major="${MAJORNUM}" -v minor="${MINORNUM}" '{if ($5==major && $6==minor) print $NF}'`
            if [ -z "${DEVICE_NAME}" ]
            then
                DEVICE_NAME=`ls -l /dev/mapper/* | ${MYAWK} -v major="${MAJORNUM}" -v minor="${MINORNUM}" '{if ($5==major && $6==minor) print $NF}'`
                if [ -z "${DEVICE_NAME}" ]
                then
                    Log "ERROR:NO support other raw device on Linux."
                    exit $ERROR_SCRIPT_EXEC_FAILED
                else
                    Log "INFO:The StorageType is raw on linux LVM,rawdevice path is [$DEVICE_PATH]."
                    VG_LV_NAME=$(RDsubstr ${DEVICE_NAME} 13)
                    VG_NAME=$(AnalyseVgName_Linux_LVM $VG_LV_NAME)
                    LV_PATH=$DEVICE_NAME
                    GetStorInfo_Linux_LVM $STORAGETYPE_RAW
                fi
            else
                Log "INFO:The StorageType is raw on Linux blockdevice,rawdevice path is [$DEVICE_PATH]."
                GetStorInfo_Linux_BlockDevice $STORAGETYPE_RAW
            fi
        elif [ "${sysName}" = "AIX" ]
        then
            LV_NAME=$(RDsubstr $DEVICE_PATH 7)
            LV_PATH="/dev/${LV_NAME}"
            IS_AIX_LVM=`lslv -L ${LV_NAME} 2>>"$LOG_FILE_NAME"`
            if [ $? = 0 ]
            then
                Log "INFO:The StorageType is rlv on AIX LVM,rawdevice path is [$DEVICE_PATH]."
                GetStorInfo_AIX_LVM $STORAGETYPE_RAW
            else 
                Log "ERROR:NO support other raw device on AIX."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        elif [ "${sysName}" = "SunOS" ]
        then
            Log "ERROR:The StorageType type of SunOS raw device is not suported."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
    else
        if [ "${sysName}" = "HP-UX" ]
        then
            FILESYSTEM_DEVICE=`df -i ${FILE_PATH} 2>>"$LOG_FILE_NAME" | sed -n '1p'| $MYAWK -F "(" '{print $2}' | ${MYAWK} -F ")" '{print $1}' | tr -d " "`
            DEVICE_PATH=`mount |grep -w "$FILESYSTEM_DEVICE" |$MYAWK '{print $1}'`
        elif [ "${sysName}" = "SunOS" ]
        then
            FILESYSTEM_DEVICE=`df -k ${FILE_PATH} 2>>"$LOG_FILE_NAME"| sed -n '2p' | $MYAWK '{print $1}'`
            DEVICE_PATH=`mount |${MYAWK} -v fs="$FILESYSTEM_DEVICE" '{if($3==fs) print $1}'`
        else
            FILESYSTEM_DEVICE=`df -i ${FILE_PATH} 2>>"$LOG_FILE_NAME"| sed -n '2p' | $MYAWK '{print $1}'`
            if [ "${sysName}" = "Linux" ]
            then
                DEVICE_PATH=`mount |grep -w "$FILESYSTEM_DEVICE" |$MYAWK '{print $3}'`
            elif [ "${sysName}" = "AIX" ]
			then
                DEVICE_PATH=`mount |grep -w "$FILESYSTEM_DEVICE" |$MYAWK '{print $2}'`
            fi
        fi
        
        if [ "$FILESYSTEM_DEVICE" = "" ]
        then
            Log "ERROR:exec df -i $FILE_PATH failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        
        IS_VXVM=`echo "$FILESYSTEM_DEVICE" | grep "/dev/vx/dsk"`
        if [ $? = 0 ]
        then
            Log "INFO:The StorageType is fs on VXVM,filesystem is [$DEVICE_PATH]."
            GetStorInfo_VXVM $STORAGETYPE_FS
        fi
        
        if [ "${sysName}" = "HP-UX" ]
        then
            LV_PATH=$FILESYSTEM_DEVICE
            VG_NAME=`echo $LV_PATH | $MYAWK -F "/" '{print $3}'`
            IS_HP_LVM=`lvdisplay $LV_PATH 2>>"$LOG_FILE_NAME"`
            if [ $? = 0 ]
            then
                Log "INFO:The StorageType is fs on HP LVM,filesystem is [$FILESYSTEM_DEVICE]."
                GetStorInfo_HP_LVM $STORAGETYPE_FS
            else
                Log "ERROR:NO support other raw device on HP-UX."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        elif [ "${sysName}" = "Linux" ]
        then
            LV_PATH=$FILESYSTEM_DEVICE
            IS_LVM_LINUX=`echo $LV_PATH|grep "/dev/mapper/"`
            if [ $? = 0 ]
            then
                VG_LV_NAME=$(RDsubstr ${LV_PATH} 13)
                VG_NAME=$(AnalyseVgName_Linux_LVM $VG_LV_NAME)
                Log "INFO:The StorageType is fs on linux LVM,filesystem is [$FILESYSTEM_DEVICE]."
                GetStorInfo_Linux_LVM $STORAGETYPE_FS
            else
                DEVICE_NAME=$FILESYSTEM_DEVICE
                DISK_NAME=$(RDsubstr $DEVICE_NAME 6)
                if [ x"`ls /dev/sd*|grep $DISK_NAME  2>/dev/null`" != x"" ]
                then
                    Log "INFO:The StorageType is fs on block device,filesystem is [$FILESYSTEM_DEVICE]."
                    GetStorInfo_Linux_BlockDevice $STORAGETYPE_FS
                fi
            fi
        elif [ "${sysName}" = "AIX" ]
        then
            LV_PATH=$FILESYSTEM_DEVICE
            LV_NAME=$(RDsubstr $LV_PATH 6)  ###Remove the heading "/dev/"  for file  
            IS_AIX_LVM=`lslv -L ${LV_NAME} 2>>"$LOG_FILE_NAME"`
            if [ $? = 0 ]
            then
                Log "INFO:StorageType is fs on AIX LVM,filesystem is [$FILESYSTEM_DEVICE]."
                GetStorInfo_AIX_LVM $STORAGETYPE_FS
            else 
                Log "ERROR:NO support other fs device on AIX."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        elif [ "${sysName}" = "SunOS" ]
        then
            Log "ERROR:The StorageType type of SunOS raw device is not suported."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
    fi
done < "${DBFILE}"

cat "${RESULT_FILE}.tmp" |sort|uniq > "${RESULT_FILE}"
cat "${RESULT_FILE}" >> "$LOG_FILE_NAME"
Log "INFO:End get storage info of db files."

rm "${RESULT_FILE}.tmp"
rm "$DBFILE"
rm "$TABLESAPCEINFO"
exit 0
