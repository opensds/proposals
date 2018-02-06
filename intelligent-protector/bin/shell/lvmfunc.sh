#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

# **************************************** Get VG name on linux LVM***************************


GetStorInfo_HP_LVM()
{
    STORAGETYPE=$1
    LANG=C
    PV_LIST=`vgdisplay -v $VG_NAME 2>>"$LOG_FILE_NAME" | grep "PV Name" | $MYAWK -F " " '{print $3}'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec vgdisplay -v $VG_NAME failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    for PV_PATH in $PV_LIST
    do
        echo "$LV_PATH:$VG_NAME:$DEVICE_PATH:$PV_PATH:$VOLTYPE_HPLVM:$STORAGETYPE" >> "${RESULT_FILE}.tmp"
    done
}

GetStorInfo_AIX_LVM()
{
    STORAGETYPE=$1
    VG_NAME=`lslv -L ${LV_NAME} 2>>"$LOG_FILE_NAME" | grep 'VOLUME GROUP:' | $MYAWK '{print $6}'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec lslv -L ${LV_NAME} failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    PV_LIST=`lsvg -p ${VG_NAME} 2>>"$LOG_FILE_NAME"|sed '1,2d'|$MYAWK -F " " '{print $1}'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec lsvg -p ${VG_NAME} failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    for PV_NAME in $PV_LIST
    do
        PV_PATH="/dev/$PV_NAME"
        echo "$LV_PATH:$VG_NAME:$DEVICE_PATH:$PV_PATH:$VOLTYPE_AIXLVM:$STORAGETYPE" >> "${RESULT_FILE}.tmp"
    done
}

GetStorInfo_Linux_LVM()
{
    STORAGETYPE=$1
    PV_LIST=`pvs 2>>"$LOG_FILE_NAME"| grep -w "$VG_NAME"| $MYAWK '{print $1}'`
    if [ $? != 0 ]
    then
        Log "ERROR:exec pvs failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    for PV_NAME in $PV_LIST
    do
        LEN=${#PV_NAME}
        COUNT=1
        while [ $COUNT -le $LEN ]
        do
            STRING=`expr substr $PV_NAME $COUNT 1`
            NUMBER_CHECK=`echo $STRING|grep [0-9]`
            if [ "$NUMBER_CHECK" != "" ] 
            then
                break
            fi  
            COUNT=`expr $COUNT + 1` 
        done
        COUNT=`expr $COUNT - 1`
        PV_PATH=`expr substr $PV_NAME 1 $COUNT` 
        echo "$LV_PATH:$VG_NAME:$DEVICE_PATH:$PV_PATH:$VOLTYPE_LINUXLVM:$STORAGETYPE">>"${RESULT_FILE}.tmp"
    done 
    return
}

GetStorInfo_VXVM()
{
    STORAGETYPE=$1
    VG_LV_NAME=$(RDsubstr $LV_PATH 13)
    VG_NAME=`echo $VG_LV_NAME|$MYAWK -F "/" '{print $1}'`
    LV_NAME=`echo $VG_LV_NAME|$MYAWK -F "/" '{print $2}'`
    PV_LIST=`vxdisk -e list 2>>"$LOG_FILE_NAME" | $MYAWK -v vg_name="$VG_NAME" '{if($4==vg_name) print $7}'`
    if [ -z "${PV_LIST}" ]
    then
        Log "ERROR:exec vxdisk -e list failed."
        exit $ERROR_SCRIPT_EXEC_FAILED    
    fi
    for PV_NAME in $PV_LIST
    do
        PV_PATH="/dev/rdsk/$PV_NAME"
        echo "$LV_PATH:$VG_NAME:$DEVICE_PATH:$PV_PATH:$VOLTYPE_VXVM:$STORAGETYPE">>"${RESULT_FILE}.tmp"
    done
    return
}

GetStorInfo_Linux_BlockDevice()
{
   STORAGETYPE=$1
   LEN=`echo "$DEVICE_NAME"|$MYAWK '{print length()}'`
   COUNT=1
   while [ $COUNT -le $LEN ]
   do
       STRING=`expr substr $DEVICE_NAME $COUNT 1`
       NUMBER_CHECK=`echo $STRING|grep [0-9]`
       if [ "$NUMBER_CHECK" != "" ]
       then
           break
       fi
       COUNT=`expr $COUNT + 1`
   done
   COUNT=`expr $COUNT - 1`
   PV_PATH=`expr substr $DEVICE_NAME 1 $COUNT` 
   echo "$DEVICE_NAME::$DEVICE_PATH:$PV_PATH:$VOLTYPE_NOVOLUME:$STORAGETYPE">>"${RESULT_FILE}.tmp"
   return
}

