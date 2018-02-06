#!/bin/bash
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

############################################################################################
#program name:          scandisk.sh
#function:              scan disk module
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
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/scandisk.log"
# global var, for kill monitor
MONITOR_FILENAME="${BIN_PATH}/procmonitor.sh"
DD_TMP_FILE="${TMP_PATH}/ddtmp${PID}"

PROC_NAME_FILTER=",UltraPathScsi,mpp,vsc,huasy_vbd,"
g_SANVendor=",HUASY,HUAWEI,HS,SYMANTEC,"
readonly g_ScsiSdDir="/sys/bus/scsi/drivers/sd"
    
#REPORT and WWN tools
CMD_TOOL="${AGENT_ROOT_PATH}/bin/agentcli"

# glbal var
thisProcName="unknown"
g_DevsName=","   #sdb,sdc
g_reportLunResult=""
g_LUNIDS=""
g_wwnudev=""
g_wwn83pge=""


# get udev info
if  [ -f /etc/SuSE-release ]
then
    OS_VERSION=`cat /etc/SuSE-release | grep VERSION | ${MYAWK} -F "=" '{print $2}' | sed 's/^[ \t]*//g'`
    if [ "$OS_VERSION" = "10" ]
    then
        UDEV_CMD="UDEVINFO"
    else
        UDEV_CMD="UDEVADM"
    fi
elif [ -f /etc/isoft-release ]   # isoft
then
    UDEV_CMD="UDEVADM"
elif [ -f /etc/linx-release ]    # rocky
then
    UDEV_CMD="UDEVADM"
elif [ -f /etc/oracle-release ]  # oracle linux
then
    OS_VERSION=`cat /etc/oracle-release| ${MYAWK} -F '.' '{print $1}' | ${MYAWK} '{print $NF}'`
    if [ "$OS_VERSION" = "5" ]
    then
        UDEV_CMD="UDEVINFO"
    else
        UDEV_CMD="UDEVADM"
    fi
elif [ -f /etc/redhat-release ]  # redhat
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
# get udev info

# check multipath type
# return 0 -- ultrapath
# return 1 -- device mapper multipath
function CheckMultiPathType()
{
    # check ultrapath
    UP=`rpm -qa | grep UltraPath`
    if [ $? -eq 0 ]
    then
        Log "UltraPath:${UP} is installed."
        return 0
    fi
    
    if [ -f /etc/linx-release ]    # no rocky check rpm, rocky check process
    then
        return 1
    else
        # check device mapper multipath
        DMMP=`rpm -qa | grep device-mapper`
        if [ $? -eq 0 ]
        then
            Log "DMMP:${DMMP} is installed."
            return 1
        fi
    fi
    
    Log "No multipath sw is installed."
    return 2
}

function CheckDMMPStatus()
{
    # found multipath excute command
    DMMP_NUM=`which multipath | wc -l`
    if [ "${DMMP_NUM}" -eq "0" ]
    then
        Log "Cant not found multipath."
        return 1
    fi

    DMMP_NUM=`ps -ef | grep "multipathd" | grep -v grep | wc -l`
    if [ "${DMMP_NUM}" -eq "0" ]
    then
        Log "Cant not found multipath process."
        return 2
    fi

    return 0
}

function getHostName()
{
    local hostdir=""
    
    thisProcName="unknown"
    if [ `echo "$1"|grep -E -c "^host[0-9]+"` -eq 0 ]
    then
        Log "getHostName param($1) error."
        exit
    fi
    
    hostdir="/sys/class/scsi_host/${1}"
    if [ -f "${hostdir}/isp_name" ]
    then
        thisProcName="qla2xxx"
    elif [ -f "${hostdir}/lpfc_drvr_version" ]
    then
        thisProcName="lpfc"
    elif [ -d "${hostdir}" ]
    then
        thisProcName=`cat "${hostdir}/proc_name" 2>/dev/null|head -n 1`
    else
        thisProcName="linuxIscsi"
    fi
    
    thisProcName=$(echo ${thisProcName})
    Log "hostname:$1==${thisProcName}"
    if [ `echo "${thisProcName}"|grep -c "^[[:blank:]]*$"` -ne 0 ]
    then
        Log "the host proc_name is NULL,used \"unknown\" now.[$1]"
        thisProcName="unknown"
    fi
    return 0
}

# get device major and minor by x:x:x:x
function getSgDevByTgt()
{
    if [ `echo "$1"|grep -E -c "^[0-9]+$"` -eq 0 ] || [ `echo "$2"|grep -E -c "^[0-9]+$"` -eq 0 ] || [ `echo "$3"|grep -E -c "^[0-9]+$"` -eq 0 ]
    then
        Log "getSgDevByTgt param error."
        exit 1
    fi
    local hostNum="${1}"
    local chanNum="${2}"
    local tgtNum="${3}"
    local tmpLine=""
    local tmpSG=""
    
    g_DevsName=","
    for tmpLine in `ls -l /sys/block/*/device 2>/dev/null | grep -E "/${hostNum}:${chanNum}:${tgtNum}:[0-9]+$" | sed "s/^.*\/sys\/block\/\(.*\)\/device[[:blank:]].*$/\1/"`
    do
        tmpSG=`echo ${tmpLine}`
        g_DevsName="${g_DevsName}${tmpSG},"
    done
    Log "host=${hostNum} chan=${chanNum} tgt=${tgtNum} g_DevsName=${g_DevsName}"
    return 0
}

# return
# 0 get reportLUN result
# 1 get zero reportLUN
# 2 get reportLUN failed
function reportLunByDeviceName()
{
    local sgResult=""
    local tmpLun=""
    
    g_reportLunResult=","
    if [ `echo "$1" | grep -E -c "/dev"` -eq 0 ]
    then
        Log "reportLunByDeviceName param error."
        exit 1
    fi

    sgResult="`${CMD_TOOL} reportlun ${1} 2>&1`"
    Log "report lun by $1,sgResult=${sgResult}"
    if [ `echo "${sgResult}"|grep -E -c "agentcli is locked"` -ne 0 ] || [ `echo "${sgResult}"|grep -E -c "Init .* failed."` -ne 0 ]
    then
        Log "agentcli excute failed, do nothing."
        exit 1
    fi
    
    if [ `echo "${sgResult}"|grep -E -c "Lun list length = [0-9]+ which imples [0-9]+ lun entr"` -ne 0 ]
    then
        for tmpLun in `echo "${sgResult}"|sed -n 's/.*lun=\(.*\)/\1/p'`
        do
            g_reportLunResult="${g_reportLunResult}${tmpLun},"
        done
    elif [ `echo "${sgResult}" | grep -E -c "report lun failed."` -ne 0 ]
    then
        return 2
    else
        return 1
    fi
    return 0
}

function reportLuns()
{
    reportLunResult=""
    local sgDevs=""
    local tmpDev=""
    local ret=""
    local fullDev=""
    
    sgDevs="${g_DevsName}"
    if [ "${sgDevs}" = "" ] || [ "${sgDevs}" = "," ]
    then
        Log "not found devices for the host,report lun failed.[${sgDevs}]"
        return 2
    fi

    local oldIFS="${IFS}"
    IFS=","
    sgDevs=(${sgDevs})
    IFS="${oldIFS}"
    for tmpDev in ${sgDevs[@]}
    do
        if [ "${tmpDev}" = "" ]
        then
            continue
        fi

        fullDev="/dev/${tmpDev}"
        reportLunByDeviceName "${fullDev}"
        ret=$?
        if [ ${ret} -eq 0 ]
        then
            ret=${g_reportLunResult}
            Log "report lun:[${tmpDev}--${ret}]"
            return 0
        else
            Log "report lun from ${tmpDev} failed."
            continue
        fi
    done
    return ${ret}
}

function checkExist()
{
    if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]
    then
        Log "checkExist param error[$1,$2,$3]"
        exit 1
    fi
    
    if [ "$1" "$2" ]
    then
        if [ "$3" = "1" ]
        then
            Log "file or directory is exist[$2]"
            exit 1
        fi
    else
        if [ "$3" = "2" ]
        then
            Log "file or directory is not exist[$2]"
            exit 1
        fi
    fi
    return 0
}

function lun0IsNotConfigured()
{
    if [ `echo "$1"|grep -E -c "^[0-9]+$"` -eq 0 ] ||
        [ `echo "$2"|grep -E -c "^[0-9]+$"` -eq 0 ] ||
        [ `echo "$3"|grep -E -c "^[0-9]+$"` -eq 0 ]
    then
        Log "lun0IsNotConfigured param error."
        exit 1
    fi
    
    checkExist -d "${g_ScsiSdDir}" 2
    if [ `ls ${g_ScsiSdDir}/${1}:${2}:${3}:0 2>/dev/null|wc -l` -ne 0 ]
    then
        return 0
    else
        Log "the lun is unconfigured.[${1}:${2}:${3}:0]"
        return 1
    fi
    return 0
}

function testExist()
{
    local host=""
    local channel=""
    local id=""
    local lun=""
    local grepstr=""
      
    checkExist -d "/sys/class/scsi_device/" 2
    if [ `ls /sys/class/scsi_device/ |grep -c "^[[:blank:]]*${1}[[:blank:]]*$"` -ne 0 ]
    then
        return 1
    else
        return 0
    fi
    return 0
}

function removeLUN()
{
    local devnr=""
    local ret=0
    local LUNID=${1}

    testExist "${LUNID}"
    ret=$?
    if [ ${ret} -ne 0 ]
    then        
        if [ -f /sys/class/scsi_device/${LUNID}/device/delete ]
        then
            echo 1 > /sys/class/scsi_device/${LUNID}/device/delete
        else
            Log "not found /sys/class/scsi_device/${LUNID}/device/delete"
        fi
        testExist "${LUNID}"
        ret=$?
    fi
    
    if [ ${ret} -ne 0 ]
    then
        Log "delete lun failed.[$1]"
        return 1
    else
        Log "delete lun successfully.[$1]"
        return 0
    fi
    return 0
}

function delLunsNotExistInReport()
{
    if [ `echo "$1"|grep -E -c "^[0-9]+$"` -eq 0 ] || [ "$2" = "" ] || [ "$3" = "" ]
    then
        Log "delLunsNotExistInReport param error."
        exit 1
    fi
    
    if [ "${g_reportLunResult}" = "" ] 
    then
        return 0
    fi

    local hostNum="${1}"
    local chanNum="${2}" 
    local tgtNum="${3}"
    local thisVendor
      
    if [ `ls /sys/class/scsi_device/${hostNum}:${chanNum}:${tgtNum}:*/device/delete 2>/dev/null|wc -l` -ne 0 ]
    then
        for tmpDev in `ls /sys/class/scsi_device/${hostNum}:${chanNum}:${tgtNum}:*/device/delete`
        do
            lunID=${tmpDev#*/sys/class/scsi_device/}
            lunID=${lunID%/device/delete*}
            lunNum=`echo "${lunID}"|${MYAWK} -F ":" '{print $4}'`
            
            thisVendor=`cat /sys/class/scsi_device/${hostNum}:${chanNum}:${tgtNum}:${lunNum}/device/vendor 2>/dev/null`
            thisVendor=$(echo ${thisVendor})
            Log "${hostNum}:${chanNum}:${tgtNum}:${lunNum},thisVendor=${thisVendor}"
            if [ `echo "${g_SANVendor}" | grep -c ",${thisVendor},"` -eq 0 ]
            then
                continue
            fi
            
            # 1.g_reportLunResult is ",", can not get reportlun, delete 3:0:0:0
            # 2.target number = 0(the host lun ID=0), if lun is mapped(/sys/bus/scsi/drivers/sd/xx:xx:xx:0 is exists), do not delete x:x:x:0.
            # 3.if lun is not mapped(/sys/bus/scsi/drivers/sd/xx:xx:xx:0 is not exists), delete x:x:x:0.
            if [ "${g_reportLunResult}" = "," ] && [ ${lunNum} -eq 0 ]
            then
                removeLUN "${hostNum}:${chanNum}:${tgtNum}:${lunNum}"
            elif [ `echo "${g_reportLunResult}" | grep -c ",${lunNum},"` -eq 0 ]
            then
                if [ ${lunNum} -eq 0 ]
                then
                    lun0IsNotConfigured "${hostNum}" "${chanNum}" "${tgtNum}"
                    ret=$?
                    if [ ${ret} -ne 0 ]
                    then
                        continue
                    fi
                fi
                removeLUN "${hostNum}:${chanNum}:${tgtNum}:${lunNum}"
            else
                if [ ${lunNum} -eq 0 ]
                then
                    lun0IsNotConfigured "${hostNum}" "${chanNum}" "${tgtNum}"
                    ret=$?
                    if [ ${ret} -ne 0 ]
                    then
                        removeLUN "${hostNum}:${chanNum}:${tgtNum}:${lunNum}"
                    fi
                fi
            fi
        done
    fi
    return 0
}

function delNotExistLunByTgt() 
{
    local hostNum=""
    local channel=""
    local tgt=""
    local ret
   
    if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ]
    then
        Log "delNotExistLunByTgt param error."
        exit 1
    fi
    hostNum="$1"
    channel="$2"
    tgt="$3"
   
    Log "delete LUNs not exist in report.[${hostNum},${channel},${tgt}]"  
    getSgDevByTgt "${hostNum}" "${channel}" "${tgt}"
    reportLuns
    ret=$?
    if [ ${ret} -eq 0 ]
    then
        delLunsNotExistInReport "${hostNum}" "${channel}" "${tgt}"
    elif [ ${ret} -eq 2 ]
    then
        Log "the connect is faild,delete all luns.[H:${hostNum}C:${channel}T:${tgt}]" 
        g_reportLunResult=","
        delLunsNotExistInReport "${hostNum}" "${channel}" "${tgt}"
    else
        Log "report lun failed.[H:${hostNum}C${channel}:T${tgt},proc_name:${2}]"
    fi 
    return 0
}

# the host lun id mapped lun have removed.
function delAllNotExistLun()
{
    local tgt
    local hostNum
    local channel
    local tmpLine
    local thisTgt=""
    local thisVendor=""
    
    Log "begin deleting non-existent luns."
    for tmpLine in `ls /sys/class/scsi_device/ | grep -E "[0-9]:+" | sort`
    do
        # do only huawei
        thisVendor=`cat /sys/class/scsi_device/${tmpLine}/device/vendor 2>/dev/null`
        thisVendor=$(echo ${thisVendor})
        Log "${tmpLine},thisVendor=${thisVendor}"
        if [ `echo "${g_SANVendor}" | grep -c ",${thisVendor},"` -eq 0 ]
        then
            continue
        fi
        
        if [ "${thisTgt}" = "${tmpLine%:*}" ]
        then
            continue
        else
            thisTgt="${tmpLine%:*}"
        fi
        Log "analyse ${tmpLine}"
        hostNum=${tmpLine%%:*}
        channel=${tmpLine#*:}
        channel=${channel%%:*}
        tgt=${tmpLine%:*}
        tgt=${tgt##*:}

        getHostName "host${hostNum}"
        if [ `echo "${PROC_NAME_FILTER}"|grep -c ",${thisProcName},"` -ne 0 ]
        then
            Log "host${hostNum} is in filter.[procName:${thisProcName}]"
            continue
        fi
        delNotExistLunByTgt "${hostNum}" "${channel}" "${tgt}"
    done
    return 0
}

function getWWNBy83pge()
{
    local deviceName="${1}"
    g_wwn83pge=""
    sgResult="`${CMD_TOOL} getwwn ${1} 2>&1`"
    Log "sgResult=${sgResult}"
    if [ `echo "${sgResult}"|grep -E -c "agentcli is locked"` -ne 0 ] || [ `echo "${sgResult}"|grep -E -c "Init .* failed."` -ne 0 ]
    then
        Log "agentcli excute failed, do nothing."
        exit 1
    fi
    
    if [ `echo "${sgResult}" | grep -E -c "Designation descriptor number"` -eq 0 ]
    then
       Log "inquiry WNN failed! [$1, dev/${sgDev}]"
       return 2
    fi
    g_wwn83pge=`echo "${sgResult}" | sed -n "s/^.*\[0[x|X]\(.*\)\].*$/\1/p"`
    g_wwn83pge="`echo ${g_wwn83pge} | tr [a-z] [A-Z]`"
}

function getWWNByUdev()
{
    g_wwnudev=""
    if [ "$UDEV_CMD" = "UDEVINFO" ]
    then
        g_wwnudev=`udevinfo -q 'all' -n $1 | grep -E "ID_SERIAL=" | ${MYAWK} -F "=" '{print $NF}'`
    elif [ "$UDEV_CMD" = "UDEVADM" ]
    then
        g_wwnudev=`udevadm info --query=all --name=$1 | grep -E "ID_SERIAL=" | ${MYAWK} -F "=" '{print $NF}'` 
    fi
    g_wwnudev=`RDsubstr ${g_wwnudev} 2`
    g_wwnudev="`echo ${g_wwnudev} | tr [a-z] [A-Z]`"
}

# remove LUN when host lun id mapped lun have changed
function delLunMapChanged()
{
    local IDNum=""
    local deviceNameMajor=""
    local thisVendor=""
    local tmpDev=""
    local fullDev=""
    
    Log "begin deleting Changed luns."
    for tmpLine in `ls /sys/class/scsi_device/ | grep -E "[0-9]:+" | sort`
    do
        # do only huawei
        thisVendor=`cat /sys/class/scsi_device/${tmpLine}/device/vendor 2>/dev/null`
        thisVendor=$(echo ${thisVendor})
        Log "${tmpLine},thisVendor=${thisVendor}"
        if [ `echo "${g_SANVendor}" | grep -c ",${thisVendor},"` -eq 0 ]
        then
            continue
        fi

        IDNum=${tmpLine}
        # get device name
        tmpDev=`ls -l /sys/block/*/device 2>/dev/null | grep -E "/${IDNum}$" | sed "s/^.*\/sys\/block\/\(.*\)\/device[[:blank:]].*$/\1/"`
        if [ "${tmpDev}" = "" ]
        then
            Log "Found(${IDNum}) device name failed."
            continue
        fi
        
        fullDev="/dev/${tmpDev}"
        Log "fullDev=${fullDev},IDNum=${IDNum}."
        
        getWWNBy83pge "${fullDev}"
        if [ "${g_wwn83pge}" = "" ]
        then
            Log "Get ${fullDev} 83pge wwn failed."
            continue
        fi
        
        getWWNByUdev "${fullDev}"
        if [ "${g_wwnudev}" = "" ]
        then
            Log "Get ${fullDev} Udev wwn failed."
            continue
        fi
        
        Log "fullDev=${fullDev} get udev WWN=${g_wwnudev}, get 83pge WWN=${g_wwn83pge}."
        [ "${g_wwn83pge}" = "${g_wwnudev}" ] && continue
        removeLUN "${tmpLine}"
    done
}

function ScanDiskByDMMP()
{
    # delete no exist LUN, it will delete 3:0:0:0 device when there is no device mapped to host
    delAllNotExistLun
    
    # delete map changed LUN
    delLunMapChanged

    # multipath scandisk
    multipath -F >> ${LOG_FILE_NAME}
    
    # scan disk twice against 2 scene:
    # A
    #    1.host lun id is 0, and is not mapped a lun, the device x:0:0:0 is deleted,
    #    2.now mappped 3 luns to host
    #    3.scan first, the host can not get 3 luns
    #    4.scan twice, the host can get 3 luns
    #    5.if in the step1, the device x:0:0:0 is not deleted, scan first, the host can get 3 luns
    # B
    #    1.host lun id is 0, and is mapped a lun, the device x:0:0:0 is deleted,
    #    2.now mappped 3 luns to host
    #    3.scan first, the host can get 3 luns.
    # reason: ultrapath get host lun id list by scsi generic, and can get the mapped host lun id list, 
    #         and known whether to delete the x:0:0:0 device, but use device mapper multipath, BCM Agent
    #         do not known the mapped host lun id list, using twice scan against above scene.
    # scan disk
    for host in $(ls -1d /sys/class/scsi_host/*); 
    do
        if [ -f ${host}/scan ]
        then
            Log "scan ${host}."
            echo "- - -" > ${host}/scan
        else
            Log "not found ${host}/scan"
        fi
    done

    # scan disk
    for host in $(ls -1d /sys/class/scsi_host/*); 
    do
        if [ -f ${host}/scan ]
        then
            Log "scan ${host}."
            echo "- - -" > ${host}/scan
        else
            Log "not found ${host}/scan"
        fi
    done
    multipath -v2 >> ${LOG_FILE_NAME}
    
    # udev become effective by partprobe, ask suse Engineer, partprobe is read disk partition, there is no affect to OS.
    partprobe >> ${LOG_FILE_NAME}
}

CheckMultiPathType
ret=$?
if [ ${ret} -eq 2 ]
then
    exit 1
fi

Log "Begin to scan disk."
if [ ${ret} -eq 0 ]
then
    # scan disk by ultrapath
    /usr/sbin/hot_add >> ${LOG_FILE_NAME}
elif [ ${ret} -eq 1 ]
then
    # check DMMP status
    CheckDMMPStatus
    ret=$?
    if [ ${ret} -ne 0 ]
    then
        exit 1
    fi
    
    # scan disk by DMMP
    ScanDiskByDMMP
fi
Log "Finish scaning disk."

exit 0
