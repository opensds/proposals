#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

############################################################################################
#program name:          OracleAdptiveCmd.sh     
#function:              Stat. oracle portfolio information,including
#                       the path of database, total size and free size
#                       and tablespace name,total size and free size
#author:
#time:                  2010-12-13
#function and description:  
# function              description
# GetValue()            Get the specified value from input parameter
#                            parameter:input parameter; result:ArgValue
# Log()                 Print log function, controled by "NeedLogFlg"
#                            parameter:record log;
# DeleteFile()          Delete file function, it can delete many files
#                            parameter:deleted file;
# rework:               First Programming
# author:
# time:
# explain:             USAGE="Usage: ./OracleAdaptive.sh PID AppInfo AgentRoot"
#                      AppInfo="AppName=xxx;SubAppName=xxx;UserName=xxx;Password=xxx;ASMUserName=xxx;ASMPassword=xxx"
# 0 - 文件系统 1 - 裸设备 2 - ASM磁盘 3-ASM裸设备 4-ASM链接文件
############################################################################################

USAGE="Usage: ./oracleluninfo.sh AgentRoot PID"

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
LOG_FILE_NAME="${LOG_PATH}/oracleluninfo.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
ORCLVESION="${TMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${TMP_PATH}/EXITSQL${PID}.sql"
#for GetOracleInstanceStatus
CHECK_INSTANCE_STATUS="${TMP_PATH}/CheckInstanceStatus${PID}.sql"
CHECK_INSTANCE_STATUSRST="${TMP_PATH}/CheckInstanceStatusRST${PID}.txt"
ARCHIVEDESTSQL="${TMP_PATH}/ArchiveDestSQL${PID}.sql"
ARCHIVESTATUSLOGSQL="${TMP_PATH}/ArchiveStatusSQL${PID}.sql"
ARCHIVESTATUSRST="${TMP_PATH}/ArchiveStatusRST${PID}.txt"
ARCHIVEDESTRST="${TMP_PATH}/ArchiveDestRST${PID}.txt"
#STARTED - After STARTUP NOMOUNT
#MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
#OPEN - After STARTUP or ALTER DATABASE OPEN
#OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
INSTANCESTATUS=""
AUTHMODE=0
DBUSER=""
DBUSERPWD=""
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
ASMUSER=""
ASMUSERPWD=""
ASMINSTANCENAME=""
DBTABLESPACENAME=""
DBINSTANCE=""


RESULT_TMP_FILE="${RESULT_FILE}.tmp"

STORAGE_FS="0"
STORAGE_RAW="1"
STORAGE_ASMDISK="2"
STORAGE_ASMRAW="3"
STORAGE_ASMLINK="4"
STORAGE_ASMUDEV="5"

VOLTYPE_NOVOLUME="0"
VOLTYPE_LINUXLVM="1"
VOLTYPE_VXVM="2"
VOLTYPE_AIXLVM="3"
VOLTYPE_HPLVM="4"

UDEV_TYPE=""
SOURCE_DEVICENAME=""
UDEV_RESULT=""
UDEV_DEVICENAME=""
SRCDEVICENAME=""
S_DEVICENAME=""

#######################################set file name##################################
QUERYNAMECRIPT="$TMP_PATH/QueryFileName$PID.sql"
QUERYNAMECRIPTRST="$TMP_PATH/QueryFileNameRST$PID.sql"
ASMTMPFILE="$TMP_PATH/ASMTmp$PID.txt"
ASMMEMBERFILE="$TMP_PATH/ASMMember$PID.txt"
DISKLISTTMPFILE="$TMP_PATH/DISKLISTTMP$PID.txt"
DISKLISTFILE="$TMP_PATH/DISKLIST$PID.txt"

# global variable
ASMDISKGROUPLIST=""
STRASMDISKPATH="/dev/oracleasm/disks"
UDEV_ROOT=""
RST_SEPARATOR=";"

SYSTEM_DEVICENAME=""
PARAM_FILE="${TMP_PATH}/input_tmp${PID}"
#********************************define these for local script********************************

# Get system diskname, from /dev/sdb1 to /dev/sdb
GetSystemDeviceName()
{
    OLDDeviceName=$1
    SYSTEM_DEVICENAME=`echo ${OLDDeviceName} | ${MYAWK} -F "[0-9]" '{print $1}'` 
}
UDEV_CHECK()
{
    if [ "$UDEV_CMD" = "UDEVINFO" ]
    then
        UDEV_INFO=`udevinfo -q 'all' -n $1`
    elif [ "$UDEV_CMD" = "UDEVADM" ]
    then
        UDEV_INFO=`udevadm info  --query=all --name=$1` 
    fi
    if [ "$UDEV_INFO" != "" ]
    then
        if [ "$UDEV_TYPE" = "SYMLINK" ]
        then
            UDEV_DEVICENAME=`echo "$UDEV_INFO" | grep 'S:'|grep -v "disk/by" |${MYAWK} -F ":" '{print $2}'|tr -d " "|tail -1`
        else
            UDEV_DEVICENAME=`echo "$UDEV_INFO" | grep 'N:'|${MYAWK} -F ":" '{print $2}'|tr -d " "`
        fi
        UDEV_RESULT=`echo "$UDEV_INFO"|grep 'E: ID_SERIAL='|${MYAWK} -F "=" '{print $2}'|tr -d " "`
        if [ "$UDEV_RESULT" = "" ]
        then
            UDEV_DEVICENAME=""
        fi
        SOURCE_DEVICENAME=`echo "$UDEV_INFO"|grep 'P:'|${MYAWK} -F "/" '{print $NF}'|tr -d " "`
    else
        UDEV_DEVICENAME=""
    fi
}

LINK_CHECK()
{
    local LINK_FILE_NAME=$1
    if [ "$UDEV_CMD" = "UDEVINFO" ]
    then
        UDEV_INFO=`udevinfo -q 'all' -n $1`
    elif [ "$UDEV_CMD" = "UDEVADM" ]
    then
        UDEV_INFO=`udevadm info  --query=all --name=$1` 
    fi
    
    if [ "$UDEV_INFO" != "" ]
    then
        UDEV_DEVICPATH=`echo "$UDEV_INFO" | grep 'E: DEVLINKS'| grep "$LINK_FILE_NAME" | ${MYAWK} -F " " '{print $NF}'`
    else
        return 1
    fi

    if [ "$UDEV_DEVICPATH" = "" ]
    then
        return 1
    fi
    
    return 0 
}


GetUDEVDevice()
{
    DEVICE_NAME=$1
    
    UDEV_DEVICENAME=""
    SOURCE_DEVICENAME=""
    UDEV_RESULT=""
    SRCDEVICENAME=""
    UDEV_TYPE="RENAME"
    UDEV_CHECK ${DEVICE_NAME} $UDEV_TYPE

    if [ "${UDEV_DEVICENAME}" != "" ]
    then
        UDEV_FULLNAME="${UDEV_ROOT}${UDEV_DEVICENAME}"
        SRCDEVICENAME="/dev/$SOURCE_DEVICENAME" 
        if [ "$UDEV_FULLNAME" = "$SRCDEVICENAME" ]
        then
            SRCDEVICENAME=""
            UDEV_TYPE="SYMLINK"
            UDEV_CHECK ${DEVICE_NAME} $UDEV_TYPE
        fi

        # Get device name in linux, like /dev/sdb1 => /dev/sdb
        if [ "$sysName" = "Linux" ] && [ ! -z "${SRCDEVICENAME}"]
        then
            GetSystemDeviceName ${SRCDEVICENAME}
            SRCDEVICENAME=${SYSTEM_DEVICENAME}
        fi
    fi
}

find_udev()
{
    local CUR_DIR parent_dir workdir
    workdir=$3
    MAJORNUM=$1
    MINORNUM=$2
    cd ${workdir}
    CUR_DIR=`pwd`

    UDEV_DEVICENAME=`ls -l "$CUR_DIR/"|${MYAWK} -v major="${MAJORNUM}" -v minor="${MINORNUM}" '{if ($5==major && $6==minor) print $NF}'`
    if [ "$UDEV_DEVICENAME" = "$S_DEVICENAME" ]
    then
        UDEV_DEVICENAME=""
    fi 
    if [ "$UDEV_DEVICENAME" != "" ]
    then
        return
    fi

    for x in `ls -l "$CUR_DIR" | grep "^d" | awk '{print $NF}'`
    do
        cd "$x";
        find_udev $MAJORNUM $MINORNUM $CUR_DIR/$x
        if [ "$UDEV_DEVICENAME" != "" ]
        then
            return
        fi
        cd ..
    done
}

####################################################################################
# function name: GENERATERST()
# aim:           output content to file
# input:         na
# output:      
# desc: output format like this
# storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
#STORAGE_FS="0"
#STORAGE_RAW="1"
#STORAGE_ASMDISK="2"
#STORAGE_ASMRAW="3"
#STORAGE_ASMLINK="4"
#STORAGE_ASMUDEV="5"
#VOLTYPE_NOVOLUME="0"
#VOLTYPE_LINUXLVM="1"
#VOLTYPE_VXVM="2"
#VOLTYPE_AIXLVM="3"
# eg.
# FS
#   LINUX LVM
#       STORAGE_FS;VOLTYPE_LINUXLVM;/dev/sdb;/dev/mapper/oracle_fs-lvmoracle;/test;oracle_fs;;;
#   LINUX NO VOL
#       STORAGE_FS;VOLTYPE_NOVOLUME;/dev/sdb;/dev/sdb2;/test;;;;
#   LINUX UDEV
#       STORAGE_FS;VOLTYPE_NOVOLUME;/dev/sdb;/dev/udev/disk/disk2;/test;;;36200bc71001f375401b6b34b0000009b;/dev/udevdisk
#   AIX LVM
#       STORAGE_FS;VOLTYPE_AIXLVM;/dev/hdisk12;/dev/lv_db2alone;/test;;;;
# RAW
#   LINUX 
#       STORAGE_RAW;VOLTYPE_NOVOLUME;/dev/sdb;/dev/sdb2;/dev/raw/raw1;;;;
# ASMLIB
#   STORAGE_ASMDISK;VOLTYPE_NOVOLUME;/dev/sdb;/dev/sdb2;/dev/oracleasm/disks/asmdisk1;;DGTEST;;
# ASMRAW
#   LINUX
#       STORAGE_ASMRAW;VOLTYPE_NOVOLUME;/dev/sdb;/dev/sdb2;/dev/raw/raw1;;DGTEST;;
#   AIX
#       STORAGE_ASMRAW;VOLTYPE_NOVOLUME;/dev/hdisk1;/dev/hdisk1;/dev/rhdisk1;;DBTEST;;
#   HP-UX
#       STORAGE_ASMRAW;VOLTYPE_NOVOLUME;/dev/disk/disk1;/dev/disk/disk1;/dev/rdisk/disk1;;DBTEST;;
# ASMLINK
#   STORAGE_ASMLINK;VOLTYPE_NOVOLUME;/dev/sdb;/dev/sdb2;/dev/disk/linkdisk1;;DGTEST;;
# ASMUDEV
#   STORAGE_ASMUDEV;VOLTYPE_NOVOLUME;dev/sdb;/dev/udev/disks/disk1;;;DGTEST;36200bc71001f375401b6b34b0000009b;/dev/udevdisk
# 
####################################################################################
GENERATERST()
{
    #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult;UDEVNAME;
    storMainType=$1
    storSubType=$2
    systemDeviceName=$3
    deviceNameInterface=$4
    devicePathInterface=$5
    vgNameInterface=$6
    if [ "${vgNameInterface}" = "--" ]
    then
        vgNameInterface=
    fi
    ASMDGName=$7
    if [ "${ASMDGName}" = "--" ]
    then
        ASMDGName=
    fi
    UDEVResult=$8
    UDEV_DEVICENAME=$9

    echo "${storMainType}${RST_SEPARATOR}${storSubType}${RST_SEPARATOR}${systemDeviceName}${RST_SEPARATOR}${deviceNameInterface}${RST_SEPARATOR}${devicePathInterface}${RST_SEPARATOR}${vgNameInterface}${RST_SEPARATOR}${ASMDGName}${RST_SEPARATOR}${UDEVResult}${RST_SEPARATOR}${UDEV_DEVICENAME}" >> "${RESULT_TMP_FILE}"
    Log "${storMainType}${RST_SEPARATOR}${storSubType}${RST_SEPARATOR}${systemDeviceName}${RST_SEPARATOR}${deviceNameInterface}${RST_SEPARATOR}${devicePathInterface}${RST_SEPARATOR}${vgNameInterface}${RST_SEPARATOR}${ASMDGName}${RST_SEPARATOR}${UDEVResult}${RST_SEPARATOR}${UDEV_DEVICENAME}"
}

####################################################################################
# function name: GetDeviceNameByLinkFile()
# aim:           get the DeviceName when link file
# input:         na
# output:        
####################################################################################
GetDeviceNameByLinkFile()
{
    LINKFILENAME=$1
    STORAE_TYPE=$2
    ASMDISKNAME=$3
    if [ ${ASMDISKNAME} = "" ]
    then
        ASMDISKNAME=--
    fi

    LINK_CHECK "$LINKFILENAME"
    if [ $? = 0 ]
    then
        UDEV_TYPE="SYMLINK"
        UDEV_CHECK "$1" $UDEV_TYPE
        if [ "${UDEV_DEVICENAME}" != "" ]
        then
            GetSystemDeviceName "/dev/${SOURCE_DEVICENAME}"
            #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
            GENERATERST ${STORAGE_ASMUDEV} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} "/dev/${SOURCE_DEVICENAME}" ${LINKFILENAME} "--" ${ASMDISKNAME} ${UDEV_RESULT} ${UDEV_DEVICENAME}
            UDEV_DEVICENAME=""
            SOURCE_DEVICENAME=""
            UDEV_RESULT=""
            return
        fi
    fi

    TARGENTDEVICE=`ls -l "${LINKFILENAME}" | ${MYAWK} '{print $NF}'`
    if [ -z "${TARGENTDEVICE}" ]
    then
        Log "find link device ${LINKFILENAME} failed."
        DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}" "${ASMMEMBERFILE}"
        exit 1
    fi
    
    NOT_FULL_PATH_LINK=`echo "$TARGENTDEVICE" | grep "^../"`
    if [ $? = 0 ]
    then
        DIR_TARGENTDEVICE=`dirname $LINKFILENAME`
        TARGENTDEVICE="$DIR_TARGENTDEVICE/$TARGENTDEVICE"
    fi
    TARGENTDEVICETYPE=`ls -l "${TARGENTDEVICE}" | ${MYAWK} '{print $1}'`
    TARGENTDEVICETYPE=`RDsubstr $TARGENTDEVICETYPE 1 1`

    # if the target device is link, call GetDeviceNameByMajor to get target device
    if [ ${TARGENTDEVICETYPE} = "l" ]
    then
        if [ "$sysName" != "SunOS" ]
        then
            UDEV_CHECK ${TARGENTDEVICE} $UDEV_TYPE
            if [ "${UDEV_DEVICENAME}" != "" ]
            then
                GetSystemDeviceName "/dev/${SOURCE_DEVICENAME}"
                #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} "/dev/${SOURCE_DEVICENAME}" ${LINKFILENAME} "--" ${ASMDISKNAME} ${UDEV_RESULT} ${UDEV_DEVICENAME}
                UDEV_DEVICENAME=""
                SOURCE_DEVICENAME=""
                UDEV_RESULT=""
            else
                Log "GetDeviceNameByLinkFile: Link not support multiple link, unknow device type device=${TARGENTDEVICE} deviceType=${TARGENTDEVICETYPE}."
            fi
        else
            DISK_LIST=`iostat -En | $MYAWK '{if(NR%5==1) {print $1}}'`
            DISK_PARTITION_NAME=`echo $TARGENTDEVICE | $MYAWK -F "/" '{print $NF}'`
            NUMBER_BEGIN=`echo $DISK_PARTITION_NAME | $MYAWK '{print length($0)-2}'`
            NUMBER_END=`echo $DISK_PARTITION_NAME | $MYAWK '{print length($0)-1}'`
            DISK_NAME=`RDsubstr $DISK_PARTITION_NAME 1 $NUMBER_BEGIN`
            PARTITION_NAME=`RDsubstr $DISK_PARTITION_NAME $NUMBER_END 2`
            IS_SOLARIS_DISK=`echo $DISK_LIST | grep $DISK_NAME`
            IS_SOLARIS_PARTITON=`echo $PARTITION_NAME | grep "^s[0-7]"`
            if [ "$IS_SOLARIS_DISK" != "" -a "$IS_SOLARIS_PARTITON" != "" ]
            then
                GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${TARGENTDEVICE} ${TARGENTDEVICE} ${LINKFILENAME} "--" ${ASMDISKNAME} "" ""
            else
                Log "GetDeviceNameByLinkFile: unknow device type device=${TARGENTDEVICE} deviceType=${TARGENTDEVICETYPE}."
            fi
        fi
    elif [ ${TARGENTDEVICETYPE} = "b" ]
    then
        UDEV_TYPE="RENAME"
        UDEV_CHECK ${TARGENTDEVICE} $UDEV_TYPE
        if [ "${UDEV_DEVICENAME}" != "" ]
        then
            GetSystemDeviceName "/dev/${SOURCE_DEVICENAME}"
            #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
            GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} "/dev/${SOURCE_DEVICENAME}" ${LINKFILENAME} "--" ${ASMDISKNAME} ${UDEV_RESULT} ${UDEV_DEVICENAME} 
            UDEV_DEVICENAME=""
            SOURCE_DEVICENAME=""
            UDEV_RESULT=""
        else
            GetSystemDeviceName ${TARGENTDEVICE}
            #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
            GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} ${TARGENTDEVICE} ${LINKFILENAME} "--" ${ASMDISKNAME} "" ""
        fi
    elif [ "${TARGENTDEVICETYPE}" = "c" ]
    then
        if [ "$sysName" = "SunOS" ]
        then
            DISK_LIST=`iostat -En | $MYAWK '{if(NR%5==1) {print $1}}'`
            DISK_PARTITION_NAME=`echo $LINKFILENAME | $MYAWK -F "/" '{print $NF}'`
            NUMBER_BEGIN=`echo $DISK_PARTITION_NAME | $MYAWK '{print length($0)-2}'`
            NUMBER_END=`echo $DISK_PARTITION_NAME | $MYAWK '{print length($0)-1}'`
            DISK_NAME=`RDsubstr $DISK_PARTITION_NAME 1 $NUMBER_BEGIN`
            PARTITION_NAME=`RDsubstr $DISK_PARTITION_NAME $NUMBER_END 2`
            IS_SOLARIS_DISK=`echo $DISK_LIST | grep $DISK_NAME`
            IS_SOLARIS_PARTITON=`echo $PARTITION_NAME | grep "^s[0-7]"`
            if [ "$IS_SOLARIS_DISK" != "" -a "$IS_SOLARIS_PARTITON" != "" ]
            then
                STORAE_TYPE=$STORAGE_ASMRAW
                DEVICE_NAME="/dev/dsk/$DISK_PARTITION_NAME"
                GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${LINKFILENAME} ${DEVICE_NAME} ${LINKFILENAME} "--" ${ASMDISKNAME} "" ""
            else
                Log "GetDeviceNameByLinkFile: unknow device type device=${TARGENTDEVICE} deviceType=${TARGENTDEVICETYPE}."
            fi
        else
            Log "GetDeviceNameByLinkFile: unknow device type device=${TARGENTDEVICE} deviceType=${TARGENTDEVICETYPE}."
        fi
    fi
}

####################################################################################
# function name: GetDeviceNameByRAW()
# aim:           get the DeviceName from the db file list when this is raw disk
# input:         like this: /dev/raw/rawxx or /dev/rlvxx or /dev/rhdiskxx or /dev/vgXX/rlvxx
# output:        
####################################################################################
GetDeviceNameByRAW()
{
    FILENAME=$1
    STORAE_TYPE=$2
    ASMDISKNAME=$3
    if [ ${ASMDISKNAME} = "" ]
    then
        ASMDISKNAME=--
    fi
    MAJORNUM=""
    MINORNUM=""
    
    # check device type
    RAW_DEVICENAME=`echo $FILENAME|${MYAWK} -F "/" '{print $NF}'` 
    DEVICETYPE=`ls -l "${FILENAME}" | ${MYAWK} '{print $1}'`
    DEVICETYPE=`RDsubstr $DEVICETYPE 1 1`
    if [ ${DEVICETYPE} = "c" ]       # raw device
    then
        if [ "$sysName" = "AIX" ]
        then
            LV_NAME=`RDsubstr $FILENAME 7` # from "/dev/r" 
            IS_AIX_LVM=`lslv -L ${LV_NAME} 2>>"$LOG_FILE_NAME"`
            if [ $? = 0 ]
            then
                Log "INFO:StorageType is raw device on AIX LV,rawdevice path is [$FILENAME]."
                VG_NAME=`lslv -L ${LV_NAME} 2>>"$LOG_FILE_NAME" | grep 'VOLUME GROUP:' | awk '{print $6}'`
                if [ $? != 0 ]
                then
                    Log "ERROR:exec lslv -L ${LV_NAME} failed."
                    DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}" "${ASMMEMBERFILE}"
                    exit $ERROR_SCRIPT_EXEC_FAILED
                fi
                PV_LIST=`lsvg -p ${VG_NAME} 2>>"$LOG_FILE_NAME"|sed '1,2d'|awk -F " " '{print $1}'`
                if [ $? != 0 ]
                then
                    Log "ERROR:exec lsvg -p ${VG_NAME} failed."
                    DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}" "${ASMMEMBERFILE}"
                    exit $ERROR_SCRIPT_EXEC_FAILED
                fi
                
                for PV_NAME in $PV_LIST
                do
                    PV_PATH="/dev/$PV_NAME"
                    GENERATERST ${STORAE_TYPE} ${VOLTYPE_AIXLVM} "$PV_PATH" "/dev/${LV_NAME}" ${FILENAME} "$VG_NAME" ${ASMDISKNAME} "" ""
                done
            else
                TEMP_DISK_NAME="$LV_NAME"
                DISK_LIST=`lsdev -Cc disk|awk '$2=="Available" {print $1}'`
                IS_AIX_RAWPV=`echo "$DISK_LIST" | grep -w "$TEMP_DISK_NAME"`
                if [ $? = 0 ]
                then
                    PV_NAME=`RDsubstr $FILENAME 7`
                    Log "INFO:StorageType is raw device on AIX PV,rawdevice path is [$FILENAME]."        
                    #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                    GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} "/dev/${PV_NAME}" "/dev/${PV_NAME}" ${FILENAME} "--" ${ASMDISKNAME} "" ""
                else
                    MAJORNUM=`ls -l "${FILENAME}" | ${MYAWK} '{print $5}'`
                    MINORNUM=`ls -l "${FILENAME}" | ${MYAWK} '{print $6}'`
                    Log "INFO:StorageType is raw device(mknod) on AIX PV,rawdevice path is [$FILENAME]."  
                    for SUB_DISK_NAME in $DISK_LIST
                    do
                        DEVICENAME=`ls -l /dev/r$SUB_DISK_NAME | ${MYAWK} -v major="${MAJORNUM}" -v minor="${MINORNUM}" '{if ($5==major && $6==minor) print $NF}'`
                        if [ ! -z "${DEVICENAME}" ]
                        then
                            #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                            GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} "/dev/${SUB_DISK_NAME}" "/dev/${SUB_DISK_NAME}" ${FILENAME} "--" ${ASMDISKNAME} "" ""
                            break
                        fi
                    done

                fi
            fi    
        elif [ "$sysName" = "HP-UX" ]
        then
            RAWDEVICE_PATH=$FILENAME
            RAWDEVICE_NAME=`echo $RAWDEVICE_PATH | awk -F "/" '{print $NF}'`
            LV_NAME=`echo $RAWDEVICE_NAME | cut -b 2-`
            VG_NAME=`echo $RAWDEVICE_PATH | awk -F "/" '{print $3}'`
            LV_PATH="/dev/$VG_NAME/$LV_NAME"
            IS_HP_LVM=`lvdisplay $LV_PATH`
            if [ $? -eq 0 ]
            then
                Log "INFO:StorageType is rlv on HP LVM,rawdevice path is [$FILENAME]."
                PV_LIST=`vgdisplay -v $VG_NAME 2>>"$LOG_FILE_NAME" | grep "PV Name" | awk -F " " '{print $3}'`
                if [ $? != 0 ]
                then
                    Log "ERROR:exec vgdisplay -v $VG_NAME failed."
                    DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}" "${ASMMEMBERFILE}"
                    exit $ERROR_SCRIPT_EXEC_FAILED
                fi
                for PV_NAME in $PV_LIST
                do
                    #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                    GENERATERST ${STORAE_TYPE} ${VOLTYPE_HPLVM} ${PV_NAME} ${LV_PATH} ${FILENAME} ${VG_NAME} ${ASMDISKNAME} "" ""
                done
            else
                MAJORNUM=`ls -l "${FILENAME}" | ${MYAWK} '{print $5}'`
                MINORNUM=`ls -l "${FILENAME}" | ${MYAWK} '{print $6}'`
                
                DEVICENAME=`ls -l /dev/rdsk | ${MYAWK} -v major="${MAJORNUM}" -v minor="${MINORNUM}" '{if ($5==major && $6==minor) print $NF}'|${MYAWK} -F "/" '{print $NF}' | grep '^c[0-9]\{1,\}t[0-9]\{1,\}d[0-9]\{1,\}'`
                if [ ! -z "${DEVICENAME}" ]
                then
                    #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                    GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} "/dev/dsk/${DEVICENAME}" "/dev/dsk/${DEVICENAME}" ${FILENAME} "--" ${ASMDISKNAME} "" ""
                fi
                
                DEVICENAME=`ls -l /dev/rdisk | ${MYAWK} -v major="${MAJORNUM}" -v minor="${MINORNUM}" '{if ($5==major && $6==minor) print $NF}'|${MYAWK} -F "/" '{print $NF}' | grep '^disk[0-9]\{1,\}'`
                if [ ! -z "${DEVICENAME}" ]
                then
                    #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                    GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} "/dev/disk/${DEVICENAME}" "/dev/disk/${DEVICENAME}" ${FILENAME} "--" ${ASMDISKNAME} "" ""
                fi
            fi
        else
            IsRedhat7
            is_redhat7=$?
            if [ "$is_redhat7" = "1" ]
            then
                RAWINFO="`raw -qa|grep ${FILENAME}`"
                MAJORNUM=`echo ${RAWINFO} | ${MYAWK} '{print $5}'`
                MINORNUM=`echo ${RAWINFO} | ${MYAWK} '{print $7}'`
            else
                MAJORNUM=`raw -q ${FILENAME} | ${MYAWK} '{print $5}'`
                MINORNUM=`raw -q ${FILENAME} | ${MYAWK} '{print $7}'`
            fi

            # get data failed
            if [ -z "${MAJORNUM}" ]
            then
                Log "excute: raw -q $FILENAME failed."
                DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}" "${ASMMEMBERFILE}"
                exit 1
            fi
            
            DEVICENAME=`cat "$DISKLISTFILE" | ${MYAWK} -v major="${MAJORNUM}" -v minor="${MINORNUM}" '{if ($5==major && $6==minor) print $NF}'|${MYAWK} -F "/" '{print $NF}'`
            if [ -z "${DEVICENAME}" ]
            then
                S_DEVICENAME="$FILENAME"
                find_udev $MAJORNUM $MINORNUM $UDEV_ROOT
                S_DEVICENAME=""
                UDEV_TYPE="RENAME"
                UDEV_CHECK ${UDEV_DEVICENAME} $UDEV_TYPE
                if [ "${UDEV_DEVICENAME}" != "" ]
                then
                    GetSystemDeviceName "/dev/${SOURCE_DEVICENAME}"
                    #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                    GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} "/dev/${SOURCE_DEVICENAME}" ${FILENAME} "--" ${ASMDISKNAME} ${UDEV_RESULT} ${UDEV_DEVICENAME}
                    UDEV_DEVICENAME=""
                    SOURCE_DEVICENAME=""
                    UDEV_RESULT="" 
                else
                    Log "RAW excute: ls -l /dev/ | ${MYAWK} \"{if (\$5==$MAJORNUM && \$6==$MINORNUM) print \$NF}\" failed."
                fi
            else
                UDEV_TYPE="SYMLINK"
                UDEV_CHECK ${DEVICENAME} ${UDEV_TYPE}
                if [ "${UDEV_DEVICENAME}" != "" ]
                then
                    GetSystemDeviceName "/dev/${SOURCE_DEVICENAME}"
                    #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                    GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} "/dev/${SOURCE_DEVICENAME}" ${FILENAME} "--" ${ASMDISKNAME} ${UDEV_RESULT} ${UDEV_DEVICENAME}
                    UDEV_DEVICENAME=""
                    SOURCE_DEVICENAME=""
                    UDEV_RESULT=""
                else
                    GetSystemDeviceName "/dev/${DEVICENAME}"
                    #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                    GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} "/dev/${DEVICENAME}" ${FILENAME} "--" ${ASMDISKNAME} "" ""
                fi
            fi
        fi
    else
        Log "GetDeviceNameByRAW: unknown device type device=${FILENAME} devicetype=${DEVICETYPE}."
    fi
}

####################################################################################
# function name: GetDeviceNameByASMDISK()
# aim:           get the DeviceName from the db file list when device is ASM disk function
# input:         /dev/oracleasm/disks/xxxx
# output:        
####################################################################################
GetDeviceNameByASMDISK()
{
    FILENAME=$1
    STORAE_TYPE=$2
    ASMGROUPNAME=$3
    
    if [ "$sysName" = "Linux" ]
    then
        MAJORNUM=`ls -l "${FILENAME}" | ${MYAWK} '{print $5}'`
        MINORNUM=`ls -l "${FILENAME}" | ${MYAWK} '{print $6}'`
        
         # get data failed
        if [ -z "${MAJORNUM}" ]
        then
            Log "ls -l "${FILENAME}" | ${MYAWK} '{print $5} failed."
            DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}" "${ASMMEMBERFILE}"
            exit 1
        fi
        if [[ $FILENAME = *${STRASMDISKPATH}* ]] 
        then
            STORAE_TYPE=$STORAGE_ASMDISK
        else
            STORAE_TYPE=$STORAGE_ASMUDEV
        fi 
        DEVICENAME=`cat "$DISKLISTFILE"| ${MYAWK} -v major="${MAJORNUM}" -v minor="${MINORNUM}" '{ if ($5==major && $6==minor) print $NF }'|${MYAWK} -F "/" '{print $NF}'`
        if [ -z "${DEVICENAME}" ]
        then
            UDEV_DEVICENAME="$FILENAME"
            if [ "$STORAE_TYPE" = "$STORAGE_ASMDISK" ]
            then
                UDEV_TYPE="RENAME"
                UDEV_CHECK ${UDEV_DEVICENAME} $UDEV_TYPE
                if [ "${UDEV_DEVICENAME}" != "" ]
                then
                    STORAE_TYPE=$STORAGE_ASMUDEV
                else
                    S_DEVICENAME="$FILENAME"
                    find_udev $MAJORNUM $MINORNUM $UDEV_ROOT
                    S_DEVICENAME=""
                fi
            fi 
            UDEV_TYPE="RENAME"
            UDEV_CHECK ${UDEV_DEVICENAME} $UDEV_TYPE
            if [ "${UDEV_DEVICENAME}" != "" ]
            then
                GetSystemDeviceName "/dev/${SOURCE_DEVICENAME}"
                #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} "/dev/${SOURCE_DEVICENAME}" ${FILENAME} "--" ${ASMGROUPNAME} ${UDEV_RESULT} ${UDEV_DEVICENAME}
                UDEV_DEVICENAME=""
                SOURCE_DEVICENAME=""
                UDEV_RESULT=""
            else
                Log "ASM excute: ls -l /dev/ | ${MYAWK} \"{if (\$5==$MAJORNUM && \$6==$MINORNUM) print \$NF}\" failed."
                DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}" "${ASMMEMBERFILE}"
                exit 1
            fi
        else
            UDEV_TYPE="SYMLINK"
            UDEV_CHECK ${DEVICENAME} $UDEV_TYPE
            if [ "${UDEV_DEVICENAME}" != "" ]
            then
                Log "$SOURCE_DEVICENAME"
                GetSystemDeviceName "/dev/"${SOURCE_DEVICENAME}
                #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} ${SYSTEM_DEVICENAME} "/dev/${SOURCE_DEVICENAME}" ${FILENAME} "--" ${ASMGROUPNAME} ${UDEV_RESULT} ${UDEV_DEVICENAME}
                UDEV_DEVICENAME=""
                SOURCE_DEVICENAME=""
                UDEV_RESULT=""
            else
                UDEV_DEVICENAME=""
                SOURCE_DEVICENAME=""
                UDEV_RESULT=""
                #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                GetSystemDeviceName ${DEVICENAME}
                GENERATERST ${STORAE_TYPE} ${VOLTYPE_NOVOLUME} "/dev/${SYSTEM_DEVICENAME}" "/dev/${DEVICENAME}" ${FILENAME} "--" ${ASMGROUPNAME} ${UDEV_RESULT} ${UDEV_DEVICENAME}
            fi 
        fi
    else
        Log "why come here? os=${sysName} FILENAME=${FILENAME}."
    fi
}

####################################################################################
# function name: GetDeviceNameOnFS()
# aim:           get the DeviceName from the db file list--FS
# input:         database file full path
# output:        
####################################################################################
GetDeviceNameOnFS()
{
    DBFILE=$1
    Log "Begin get device name on fs, device name ${DBFILE}."
    # archive dir
    # 1.check archive whether is exists
    # 2.no exists, clear buf
    if [ "$DBTABLESPACENAME" = "archive" ]
    then
        if [ ! -d "$DBFILE" ]
        then
            LAST_CHAR=${DBFILE: -1}
            if [ "${LAST_CHAR}" = "/" ]
            then
                Log "archice dest(${LAST_CHAR}) is not exists."
            else
                DBFILE=`echo ${DBFILE} | sed 's/[^\/]*$//'`
                Log "archice dest($1)->($DBFILE)"
            fi
        fi
    fi
    
    if [ "${sysName}" = "HP-UX" ]
    then
        FILESYSTEM_DEVICE=`df -i "${DBFILE}" | sed -n '1p' | ${MYAWK} -F "(" '{print $2}' | ${MYAWK} -F ")" '{print $1}' | tr -d " "`
    elif [ "${sysName}" = "SunOS" ]
    then
        FILESYSTEM_DEVICE=`df -k "${DBFILE}" | sed -n '2p' | ${MYAWK} '{print $1}'`
    else
        FILESYSTEM_DEVICE=`df -i "${DBFILE}" | sed -n '2p' | ${MYAWK} '{print $1}'`
    fi
    
    IS_VXVM=`echo $FILESYSTEM_DEVICE | grep "/dev/vx/dsk/"`
    if [ $? = 0 ]
    then
        MOUNT_POINT=`mount |${MYAWK} -v fs="$FILESYSTEM_DEVICE" '{if($3==fs) print $1}'`
        VG_LV_NAME=`RDsubstr $FILESYSTEM_DEVICE 13`
        VG_NAME=`echo $VG_LV_NAME|${MYAWK} -F "/" '{print $1}'`
        LV_NAME=`echo $VG_LV_NAME|${MYAWK} -F "/" '{print $2}'`
        PV_LIST=`vxdisk -e list 2>>"$LOG_FILE_NAME" | $MYAWK -v vg_name="$VG_NAME" '{if($4==vg_name) print $6}'`
        for PV_NAME in $PV_LIST
        do
            PV_PATH="/dev/rdsk/$PV_NAME"
            #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
            GENERATERST ${STORAGE_FS} ${VOLTYPE_VXVM} ${PV_PATH} ${FILESYSTEM_DEVICE} ${MOUNT_POINT} ${VG_NAME} "--" ${UDEV_RESULT} ${UDEV_DEVICENAME}
        done
        return
    fi
        
    if [ "${sysName}" = "Linux" ]
    then
        #LVM and VxVM do not analyse UDEV
        MOUNT_POINT=`mount | grep -w "${FILESYSTEM_DEVICE}" |${MYAWK} '{print $3}'`
        IS_LVM_LINUX=`echo ${FILESYSTEM_DEVICE} | grep "/dev/mapper/"`
        if [ $? = 0 ]
        then
            VG_LV_NAME=`RDsubstr ${FILESYSTEM_DEVICE} 13`
            VG_NAME=`AnalyseVgName_Linux_LVM $VG_LV_NAME`
            
            Log "Get vg name ${VG_NAME}, lv name ${LV_NAME}."
            PV_LIST=`pvs 2>/dev/null| grep -w "$VG_NAME"| ${MYAWK} '{print $1}'`
            for PV_NAME in $PV_LIST
            do
                Log "Get pv name ${PV_NAME}."
                GetSystemDeviceName ${PV_NAME}
                DISK_NAME=${SYSTEM_DEVICENAME}
                #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
                GENERATERST ${STORAGE_FS} ${VOLTYPE_LINUXLVM} ${DISK_NAME} ${FILESYSTEM_DEVICE} ${MOUNT_POINT} ${VG_NAME} "--" ${UDEV_RESULT} ${UDEV_DEVICENAME}
            done
                
            Log "Get deivce name on linux lvm succ."
            return
        fi
        
        if [ x"`ls -l $FILESYSTEM_DEVICE  2>/dev/null`" != x"" ]
        then
            GetUDEVDevice ${FILESYSTEM_DEVICE}
            GetSystemDeviceName ${FILESYSTEM_DEVICE}
            DISK_NAME=${SYSTEM_DEVICENAME}
            #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
            GENERATERST ${STORAGE_FS} ${VOLTYPE_NOVOLUME} ${DISK_NAME} ${FILESYSTEM_DEVICE} ${MOUNT_POINT} "--" "--" ${UDEV_RESULT} ${UDEV_DEVICENAME}
            return
        fi
        
        Log "Find the storage of filesystem $FILESYSTEM_DEVICE failed."
        exit $ERROR_DB_FILE_NOT_EXIST
    elif [ "${sysName}" = "AIX" ]
    then
        MOUNT_POINT=`df -i "${DBFILE}" | grep -v Filesystem | ${MYAWK} '{print $NF}'| sed -n '1p'`
        LVNAME=`echo $FILESYSTEM_DEVICE|${MYAWK} -F "/" '{print $NF}'`
        VGNAME=`lslv "$LVNAME" | sed -n '1p' | ${MYAWK} '{print $NF}'`
        
        if [ "$VGNAME" = "" ] || [ "$LVNAME" = "" ] 
        then
            Log "Get VGNAME or LVNAME is null in AIX, database file is ${DBFILE}."
            DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}"
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        PV_LIST=`lsvg -p ${VGNAME} 2>>"$LOG_FILE_NAME"|sed '1,2d'|awk -F " " '{print $1}'`
        for PV_NAME in $PV_LIST
        do
            PV_PATH="/dev/$PV_NAME"
            #storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
            GENERATERST ${STORAGE_FS} ${VOLTYPE_AIXLVM} ${PV_PATH} ${FILESYSTEM_DEVICE} ${MOUNT_POINT} "$VGNAME" "" "" ""
        done
    elif [ "${sysName}" = "HP-UX" ]
    then
        MOUNT_POINT=`df -i "${DBFILE}" | sed -n '1p' | ${MYAWK} -F " " '{print $1}'`
        IS_LVM_HP=`lvdisplay $FILESYSTEM_DEVICE`
        if [ $? = 0 ]
        then
            Log "INFO:StorageType is fs on HP LVM,filesystem is [$FILESYSTEM_DEVICE]."
            VGNAME=`echo $FILESYSTEM_DEVICE | awk -F "/" '{print $3}'`
            PV_LIST=`vgdisplay -v $VGNAME 2>>"$LOG_FILE_NAME" | grep "PV Name" | awk -F " " '{print $3}'`
            if [ $? != 0 ]
            then
                Log "ERROR:exec vgdisplay -v $VG_NAME failed."
                DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "${ASMTMPFILE}"
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
            for PV_NAME in $PV_LIST
            do
                GENERATERST ${STORAGE_FS} ${VOLTYPE_HPLVM} ${PV_NAME} ${FILESYSTEM_DEVICE} ${MOUNT_POINT} "$VGNAME" "" "" ""
            done
        fi
    fi
}

####################################################################################
# function name: GetDeviceNameOnASM()
# aim:           get the DeviceName from the db file list when ASM disk group function
# input:         na
# output:        
####################################################################################
GetDeviceNameOnASM()
{
    ASMFile=$1
    ASMDISKGROUPNAME=`echo "${ASMFile}" | ${MYAWK} -F '/' '{print $1}'`
    ASMDISKGROUPNAME=`RDsubstr $ASMDISKGROUPNAME 2`
    # judge the disk group whether read
    echo ${ASMDISKGROUPLIST} | grep ",${ASMDISKGROUPNAME}," > /dev/null
    if [ $? -eq 0 ]    # the current disk group had parse, return
    then
        return
    fi

    #get asm instance name
    ASMSIDNAME=`ps -ef | grep "asm_...._$ASMINSTANCENAME" | grep -v "grep" | ${MYAWK} -F '+' '{print $NF}'`
    ASMSIDNAME=`echo ${ASMSIDNAME} | ${MYAWK} -F " " '{print $1}'`
    ASMSIDNAME="+"${ASMSIDNAME}
    
    # get asm disk group memeber
    echo "select A.PATH from v\$asm_disk A, v\$asm_diskgroup B where A.GROUP_NUMBER = B.GROUP_NUMBER and B.Name='${ASMDISKGROUPNAME}';" > "$QUERYNAMECRIPT"
    echo "exit" >> "$QUERYNAMECRIPT"
    ASM_LOGIN=${ASMUSER}/\"${ASMUSERPWD}\"
    ASM_LOG_AUTH="`CreateASMLoginCmd ${VERSION} ${AUTHMODE} \"${ASM_LOGIN}\"`"

    Log "Exec ASM SQL to get asm disk path of ASM diskgroup."
    ASMExeSql "${ASM_LOG_AUTH}" "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${ASMSIDNAME}" 30
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${RESULT_TMP_FILE}" "${ASMTMPFILE}" "${DISKLISTFILE}"
        Log "Get ASM disks of diskgroup failed."
        exit ${RET_CODE}
    fi

    #****************************asm disk group member********************************
    if [ `cat "$QUERYNAMECRIPTRST" | wc -l` -eq 0 ]
    then
        DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${RESULT_TMP_FILE}" "${ASMTMPFILE}" "${DISKLISTFILE}"
        Log "Get ASM disks of diskgroup failed, the result is null."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi

    #****************************parse Disk List********************************
    cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "/" > "${ASMMEMBERFILE}"
    cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "ORCL" >> "${ASMMEMBERFILE}"
    DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}"
    
    while read line
    do
        if [ "`RDsubstr $line 1 4`" = "ORCL" ]     # ORCL:XXX
        then
            ASMDISKNAME=`RDsubstr $line 6`         # eraser
            GetDeviceNameByASMDISK "/dev/oracleasm/disks/${ASMDISKNAME}" ${STORAGE_ASMDISK} ${ASMDISKGROUPNAME}
        else                                        # linux /dev/raw/rawN /dev/oracleasm/disks /dev/diskN(ln) AIX /dev/rxxxx  
            DEVICETYPE=`ls -l ${line} | ${MYAWK} '{print $1}'`
            DEVICETYPE=`RDsubstr $DEVICETYPE 1 1`
            
            if [ ${DEVICETYPE} = "b" ]
            then
                GetDeviceNameByASMDISK ${line} ${STORAGE_ASMUDEV} ${ASMDISKGROUPNAME}
            elif [ ${DEVICETYPE} = "c" ]
            then
                GetDeviceNameByRAW ${line} ${STORAGE_ASMRAW} ${ASMDISKGROUPNAME}
            elif [ ${DEVICETYPE} = "l" ]
            then
                GetDeviceNameByLinkFile ${line} ${STORAGE_ASMLINK} ${ASMDISKGROUPNAME}
            else
                Log "GetDeviceNameOnASM: unknown device type device=${line} devicetype=${DEVICETYPE}."
            fi
        fi
    done < "${ASMMEMBERFILE}"
    
    # add the current disk gourp name into the list
    if [ -z "${ASMDISKGROUPLIST}" ]
    then
        ASMDISKGROUPLIST=","${ASMDISKGROUPNAME}","
    else
        ASMDISKGROUPLIST=${ASMDISKGROUPLIST}${ASMDISKGROUPNAME}","
    fi
    
    DeleteFile "${ASMMEMBERFILE}"
}

####################################################################################
# function name: GetDeviceNameByDbFile()
# aim:           get the DeviceName from the db file list function
# input:         na
# output:        
####################################################################################
GetDeviceNameByDbFile()
{
    # get file list,include asm and raw
    cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "/" > "${ASMTMPFILE}"
    cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "+" >> "${ASMTMPFILE}"
    DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}"
    
    while read dbFile
    do        
        if [ "`RDsubstr $dbFile 1 1`" = "+" ]                # what is beginning with + is asm disk group
        then
            GetDeviceNameOnASM "${dbFile}"
        else
            DEVICETYPE=`ls -l "${dbFile}" | ${MYAWK} '{print $1}'`
            DEVICETYPE=`RDsubstr $DEVICETYPE 1 1`
            
            if [ ${DEVICETYPE} = "c" ]                          # what is beginning with /dev/raw/raw is raw
            then 
                GetDeviceNameByRAW ${dbFile} ${STORAGE_RAW} "--"
            elif [ ${DEVICETYPE} = "-" ]                        # another, it is file system
            then
                GetDeviceNameOnFS "${dbFile}"
            elif [ ${DEVICETYPE} = "b" ] 
            then
                Log "ERROR:No suport db file on block device."
                DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "$ASMTMPFILE"
                exit $ERROR_NOSUPPORT_DBFILE_ON_BLOCKDEVICE
            elif [ ${DEVICETYPE} = "l" ] 
            then
                Log "ERROR:No suport db file on block device."
                DeleteFile "${RESULT_TMP_FILE}" "${DISKLISTFILE}" "$ASMTMPFILE"
                exit $ERROR_NOSUPPORT_DBFILE_ON_BLOCKDEVICE
            else
                Log "GetDeviceNameByDbFile: unknow device type device=${dbFile}, deviceType=${DEVICETYPE}."
                exit $ERROR_DB_FILE_NOT_EXIST
            fi
        fi
    done < "$ASMTMPFILE"
    
    DeleteFile "$ASMTMPFILE"
}

GetDeviceNameByArchDir()
{
    # get file list,include asm and raw
    cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "/" > "${ASMTMPFILE}"
    cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "+" >> "${ASMTMPFILE}"
    # grep "USE_DB_RECOVERY_FILE_DEST" only for archive logs
    cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "USE_DB_RECOVERY_FILE_DEST" >> "${ASMTMPFILE}"
    DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}"
    
    Log "Begin get device name by arch dir."
    while read archDir
    do
        if [ ${archDir} = "USE_DB_RECOVERY_FILE_DEST" ]
        then
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
                DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}" "${ASMTMPFILE}" "${RESULT_TMP_FILE}" "${DISKLISTFILE}"
                Log "Get Archive dest failed."
                exit ${RET_CODE}
            fi
            archDir=`sed -n '/----------/,/^ *$/p' "${ARCHIVEDESTRST}" | sed -e '/----------/d' -e '/^ *$/d'`
            Log "archDir=${archDir}."
            DeleteFile "${ARCHIVEDESTSQL}" "${ARCHIVEDESTRST}"
        fi
        
        if [ "`RDsubstr ${archDir} 1 1`" = "+" ]
        then
            GetDeviceNameOnASM "${archDir}"
        else
            GetDeviceNameOnFS "${archDir}"
        fi
    done < "$ASMTMPFILE"
    
    DeleteFile "$ASMTMPFILE"
    Log "End get device name by arch dir."
}

####################################################################################
# function name: QueryTableSpacePath()
# aim:           query db file localtion function
# input:         na
# output:        
####################################################################################
QueryTableSpacePath()
{
    ORACLE_TMP_VERSION=`RDsubstr $ORA_VERSION 1 2`
    #************************temporary sql script for quering file_name*****************
    echo "set linesize 500;" > "$QUERYNAMECRIPT"
    if [ "$DBTABLESPACENAME" = "must" ]
    then
        if [ ${ORACLE_TMP_VERSION} -lt 12 ]   
        then
            echo "select file_name from dba_data_files;" >> "$QUERYNAMECRIPT"     
        else 
            echo "select name from v\$datafile;" >> "$QUERYNAMECRIPT"   		
        fi
        echo 'select MEMBER from v$logfile;' >> "$QUERYNAMECRIPT"			  
        echo 'select name from v$controlfile;' >> "$QUERYNAMECRIPT"
    elif [ "$DBTABLESPACENAME" = "option" ]
    then
        if [ ${ORACLE_TMP_VERSION} -lt 12 ]  
        then
            echo 'select file_name from dba_temp_files;' >> "$QUERYNAMECRIPT"     
        else
            echo 'select name from v$tempfile;' >> "$QUERYNAMECRIPT"      
        fi
    else
        echo "col DESTINATION for a255;" >> "$QUERYNAMECRIPT"
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
        echo "select DESTINATION from v\$archive_dest where STATUS='VALID';" >> "$QUERYNAMECRIPT"
    fi
    echo "exit" >> "$QUERYNAMECRIPT"
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"

    Log "Exec SQL to get files of database."
    OracleExeSql "${LOGIN_AUTH}" "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${DBINSTANCE}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "Search database(${DBINSTANCE}) file failed."
        DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${RESULT_TMP_FILE}" "${DISKLISTFILE}"
        exit ${RET_CODE}
    fi
    
    #****************************get db location********************************
    if [ `cat "$QUERYNAMECRIPTRST" | wc -l` -eq 0 ]
    then
        DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${RESULT_TMP_FILE}" "${DISKLISTFILE}"
        Log "QueryTableSpacePath:Connect to database $DBNAME failed, result is null."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi

    # analyse database file list
    if [ "$DBTABLESPACENAME" = "archive" ]
    then
        CheckArchiveMode ${DBINSTANCE}
        GetDeviceNameByArchDir
    else
        GetDeviceNameByDbFile
    fi
    
    DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}"
}

CheckArchiveMode()
{
    Log "Begin check archive mode."
    #query archive log mode
    GetArchiveLogMode ${DBINSTANCE}
    RET_CODE=$?
    if [ ${RET_CODE} -ne 0 ] && [ ${RET_CODE} -ne 1 ]
    then
        DeleteFile "$QUERYNAMECRIPTRST" "$QUERYNAMECRIPT" "${RESULT_TMP_FILE}" "${DISKLISTFILE}"
        Log "Get archive log mode failed."
        exit ${RET_CODE}
    fi

    if [ ${RET_CODE} -eq 0 ]
    then
        Log "Archive Mode=No Archive Mode, check archive mode failed."
        DeleteFile "$QUERYNAMECRIPTRST" "$QUERYNAMECRIPT" "${RESULT_TMP_FILE}" "${DISKLISTFILE}"
        exit ${ERROR_ORACLE_NOARCHIVE_MODE}
    fi
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
#################Entry of script to query the information of oracle portfolio###########
GetValue "$INPUTINFO" InstanceName
DBINSTANCE=$ArgValue

GetValue "$INPUTINFO" AppName
DBNAME=$ArgValue

GetValue "$INPUTINFO" UserName
DBUSERL=$ArgValue
DBUSER=`echo "$DBUSERL" | tr '[A-Z]' '[a-z]'`

GetValue "$INPUTINFO" Password
DBUSERPWD=$ArgValue

GetValue "$INPUTINFO" TableSpaceName
DBTABLESPACENAME=$ArgValue

GetValue "$INPUTINFO" ASMUserName
ASMUSER=$ArgValue

GetValue "$INPUTINFO" ASMPassword
ASMUSERPWD=$ArgValue

GetValue "$INPUTINFO" ASMInstanceName
ASMINSTANCENAME=$ArgValue
if [ -z "${ASMINSTANCENAME}" ]
then
    ASMINSTANCENAME="+ASM"
fi

GetValue "$INPUTINFO" OracleHome
IN_ORACLE_HOME=$ArgValue

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

Log "SubAppName=$DBINSTANCE;AppName=$DBNAME;UserName=$DBUSER;TableSpaceName=$DBTABLESPACENAME;ASMUserName=$ASMUSER;ASMInstanceName=$ASMINSTANCENAME;AUTHMODE=$AUTHMODE;IN_ORACLE_HOME=$IN_ORACLE_HOME"

# begin to get UDEV configuration
cat /proc/partitions|sed '1,2d' > "${DISKLISTTMPFILE}"
while read SUB_DISKINFO
do
    DISK=`echo $SUB_DISKINFO|${MYAWK} '{print $NF}'`
    if [ -b "/dev/$DISK" ]
    then
        ls -l /dev/$DISK>>"${DISKLISTFILE}"
    fi
done < "${DISKLISTTMPFILE}"
DeleteFile "${DISKLISTTMPFILE}"

UDEV_ROOT=`cat /etc/udev/udev.conf |grep -v '#'|grep "udev_root"|${MYAWK} -F "\"" '{print $2}'`
if [ "$UDEV_ROOT" = "" ]
then
    UDEV_ROOT="/dev/"
fi

UDEV_RULES_DIR=`cat /etc/udev/udev.conf |grep -v '#'|grep "udev_rules"|${MYAWK} -F "\"" '{print $2}'`
if [ "${UDEV_RULES_DIR}" = "" ]
then
    UDEV_RULES_DIR="/etc/udev/rules.d/"
fi

if  [ -f /etc/SuSE-release ]
then
    OS_VERSION=`cat /etc/SuSE-release | grep VERSION | ${MYAWK} -F "=" '{print $2}' | sed 's/^[ \t]*//g'`
    if [ "$OS_VERSION" = "10" ]
    then
        UDEV_CMD="UDEVINFO"
    else
        UDEV_CMD="UDEVADM"
    fi
elif [ -f /etc/redhat-release ]
then
    OS_VERSION=`cat /etc/redhat-release | ${MYAWK} -F '.' '{print $1}' | ${MYAWK} '{print $NF}'`
    if [ "$OS_VERSION" = "5" ]
    then
        UDEV_CMD="UDEVINFO"
    else
        UDEV_CMD="UDEVADM"
    fi
else
    UDEV_CMD="UDEVINFO"
fi
#end to get udev configuration

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
    DeleteFile "${DISKLISTFILE}"
    exit ${RET_CODE}
fi

#STARTED - After STARTUP NOMOUNT
#MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
#OPEN - After STARTUP or ALTER DATABASE OPEN
#OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
if [ ! "`RDsubstr $INSTANCESTATUS 1 4`" = "OPEN" ]
then
    Log "Instance status($INSTANCESTATUS) no open."
    DeleteFile "${DISKLISTFILE}"
    exit ${ERROR_INSTANCE_NOSTART}
fi
Log "end to check oracle instance status."


Log "Start to Stat. oracle database file information."
touch "${RESULT_TMP_FILE}"

##query oracle database file location
QueryTableSpacePath

cat "${RESULT_TMP_FILE}" | uniq > "${RESULT_FILE}"
##delete temporary file
DeleteFile "$ArgFile" "$QUERYNAMECRIPTRST" "$QUERYNAMECRIPT" "${RESULT_TMP_FILE}" "${DISKLISTFILE}"

Log "Stat. oracle database file information successful."

exit 0
