#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

AGENT_ROOT_PATH=$1
ID=$2

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/initiator.log"
#load the agent function library script
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
SYS=`uname -s`
RESULT_FILE="${AGENT_ROOT_PATH}/tmp/${RESULT_TMP_FILE_PREFIX}${ID}"

INPUT_TMP_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${ID}"

INITIATOR_DEV="${AGENT_ROOT_PATH}/tmp/initiator_dev${ID}"
INITIATOR_INFO="${AGENT_ROOT_PATH}/tmp/initiator_info${ID}"

QUERY_RST=1
VERSION=NEW

# fc or iscsi
if [ ! -f "$INPUT_TMP_FILE" ]
then
    Log "ERROR: Input parameter temp file is not exist."
    exit 1
fi

QUERY_PARAM=`cat "$INPUT_TMP_FILE"`
rm -rf "${INPUT_TMP_FILE}"

GetLinuxVersion()
{
    if [ -f /etc/redhat-release ]
    then
        VERSION_TMP=`cat /etc/redhat-release`
        VERSION_TMP=`echo ${VERSION_TMP##*release}`
        VERSION_TMP=`echo ${VERSION_TMP%%(*}`
        VERSION_TMP=`echo ${VERSION_TMP%%.*}`
  
        if [ $VERSION_TMP -gt 4 ]
        then
            VERSION=NEW
        else
            VERSION=REDHAT4
        fi
  
    elif [ -f /etc/SuSE-release ]
    then
        VERSION_TMP=`cat /etc/SuSE-release`
        FIND=`echo $VERSION_TMP|grep "PATCHLEVEL"`
    
        if [ -z "$FIND" ]
        then
            #Suse 11
            VERSION_TMP=`echo ${VERSION_TMP##*VERSION =}`
            VERSION_TMP=`echo ${VERSION_TMP%%(*}`
            VERSION_TMP=`echo ${VERSION_TMP%%.*}`
        else
            #Suse 10
            VERSION_TMP=`echo ${VERSION_TMP##*VERSION}`
            VERSION_TMP=`echo ${VERSION_TMP%%PATCHLEVEL*}`
            VERSION_TMP=`echo ${VERSION_TMP##*=}`
            VERSION_TMP=`echo ${VERSION_TMP%%(*}`
            VERSION_TMP=`echo ${VERSION_TMP%%.*}`
        fi

        if [ $VERSION_TMP -gt 9 ]
        then
            VERSION=NEW
        else
            VERSION=SUSE9
        fi
    else        
        Log "ERROR: Don't support the OS edition:$VERSION_TMP."
        return 1
    fi
    
    return 0
}

GetFCInfoLinux()
{
    FC_NUMBER=`lspci | grep Fibre|wc -l`
    if [ "$?" != "0" ]
    then
        Log "ERROR: Get number of HBA failed."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi   
    
    if [ $FC_NUMBER -gt 0 ]
    then
        if [ "$VERSION" = "NEW" ]
        then           
            lspci | grep Fibre | while read line
            do
                FC_TMPNAME=`echo $line | awk -F ' ' '{print $1}'`
                
                FC_TMP=`systool -av -c scsi_host | grep "Device path" | grep $FC_TMPNAME`
                if [ "$?" != "0" ]
                then
                    Log "ERROR: Get FC $FC_TMPNAME information failed."
                    continue
                fi
        
                FC_TMP=`echo ${FC_TMP##*/}`
                FC_TMP=`echo ${FC_TMP%%\"*}`

                FC_WWNNUMBER=--
                FC_WWNTMP=`cat /sys/class/fc_host/"$FC_TMP"/port_name`
                if [ "$?" = "0" ]
                then
                    FC_TMPWWN=$FC_WWNTMP
                    FC_WWNTMP=`echo $FC_WWNTMP | awk '{print toupper($0)}'`
                    FC_WWNTMP=`expr substr $FC_WWNTMP 1 2`

                    if [ "$FC_WWNTMP" = "0X" ]
                    then
                        FC_WWNTMP=`echo $FC_TMPWWN|cut -b3-`
                    else
                        FC_WWNTMP=$FC_TMPWWN
                    fi
            
                    FC_WWNNUMBER=$FC_WWNTMP
                    
                    echo "$FC_WWNNUMBER" >> "${RESULT_FILE}"
                    Log "INFO: Get FC $FC_TMPNAME WWN $FC_WWNNUMBER successful."
                else
                    Log "ERROR: Get FC $FC_TMPNAME WWN failed."
                    continue
                fi                                 
            done
            Log "INFO: Get all FC WWN successful."  
            return 0
        else
            Log "ERROR: Unsupported OS: $VERSION."
            return 1
        fi                
    else
        Log "ERROR: No FC in your system."
        return 1
    fi
}

GetISCSIInfoLinux()
{
    ISCSI_FILE=
    ISCSI_NAME=--
    if [ -f /etc/initiatorname.iscsi ]
    then
        ISCSI_FILE=/etc/initiatorname.iscsi
    elif [ -f /etc/iscsi/initiatorname.iscsi ]
    then
        ISCSI_FILE=/etc/iscsi/initiatorname.iscsi
    else
        Log "ERROR: Iscsi initiator name file dose not exist."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi

    #test read permission, only root can read this file in suse
    if [ ! -r ${ISCSI_FILE} ]
    then
        Log "ERROR: Can't read iscsi file ${ISCSI_FILE}."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi
    
    while read line
    do
        TMP_NAME=`echo ${line} | awk -F "=" '{print $1}'`
        if [ "${TMP_NAME}" = "InitiatorName" ]
        then
            ISCSI_NAME=`echo ${line} | awk -F "=" '{print $2}'`
            echo ${ISCSI_NAME} >> "${RESULT_FILE}"
            Log "INFO: Get iSCSI initiator name ${ISCSI_NAME} successful."
        fi
    done < ${ISCSI_FILE}

    if [ "${ISCSI_NAME}" = "--" ]
    then
        Log "ERROR: Read iscsi name failed."
        return 1
    fi
    
    return 0
}


GetFCInfoAIX()
{
    FC_WWNNUMBER=--
    lsdev -Cc adapter -S a | grep fcs > "$INITIATOR_DEV"
    if [ "$?" != "0" ]
    then
    echo No HBA in your system
        Log "ERROR: No HBA in your system."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi

    while read line
    do
        FC_TMPNAME=`echo $line | awk -F " " '{print $1}'`
        FC_TMPNAME=`echo $FC_TMPNAME | sed 's/ //g'`
        
        lscfg -vpl $FC_TMPNAME > "$INITIATOR_INFO"
        if [ "$?" != "0" ]
        then
            Log "ERROR: Get HBA $FC_TMPNAME WWN failed."
            continue
        fi
    
        while read line
        do
            FC_TMPFLAG=`echo $line | sed 's/\./ /g'`
            FC_TMPFLAG=`echo $FC_TMPFLAG | awk -F " " '{print $1}'`
            FC_TMPFLAG=`echo $FC_TMPFLAG | sed 's/ //g'`         
            
            if [ "$FC_TMPFLAG" = "Network" ]
            then
                FC_TMP=`echo $line | sed 's/\./ /g'`
                FC_TMP=`echo $FC_TMP | awk -F " " '{print $2}'`
                FC_TMP=`echo $FC_TMP | sed 's/ //g'`

                if [ "$FC_TMP" = "Address" ]
                then
                    FC_TMP=`echo $line | sed 's/\./ /g'`
                    FC_TMP=`echo $FC_TMP | awk -F " " '{print $3}'`
                    FC_WWNTMP=`echo $FC_TMP | sed 's/ //g'` 
                    FC_TMPWWN=$FC_WWNTMP
                    FC_WWNTMP=`echo $FC_WWNTMP | awk '{print toupper($0)}'`
                    FC_WWNTMP=`expr $FC_WWNTMP : '^\(..\).*$'`
                
                    if [ "$FC_WWNTMP" = "0X" ]
                    then
                        FC_WWNTMP=`echo $FC_TMPWWN|cut -b3-`
                    else
                        FC_WWNTMP=$FC_TMPWWN
                    fi
            
                    FC_WWNNUMBER=$FC_WWNTMP                      
                    Log "INFO: Get HBA $FC_TMPNAME WWN $FC_WWNNUMBER successful."

                    echo "$FC_WWNNUMBER" >> "${RESULT_FILE}"
                fi
            fi     
        done < "${INITIATOR_INFO}"
    done < "${INITIATOR_DEV}"
    
    if [ "$FC_WWNNUMBER" = "--" ]
    then
        Log "ERROR: Get HBA $FC_TMPNAME WWN failed."
        return 1
    fi
    
    return 0
}

GetISCSIInfoAIX()
{
    ISCSI_NAME=--
    lsdev | grep iscsi > "${INITIATOR_DEV}"
    if [ "$?" != "0" ]
    then
        Log "ERROR: Iscsi initiator name file dose not exist."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi
    
    while read line
    do
        ISCSI_TMP=`echo $line | awk -F " " '{print $3}'`
        if [ "$ISCSI_TMP" == "iSCSI" ]
        then
            ISCSI_TMP=`echo $line | awk -F " " '{print $2}'`
            if [ "${ISCSI_TMP}" == "Available" ]
            then
                ISCSI_DEVNAME=`echo $line | awk -F " " '{print $1}'`
            fi
        fi
        
        lsattr -El ${ISCSI_DEVNAME} > "${INITIATOR_INFO}"
        if [ "$?" != "0" ]
        then
            Log "ERROR: Get iscsi initiator name failed."
            continue
        fi
        
        while read line
        do
            ISCSI_TMP=`echo $line | awk -F " " '{print $1}'`
            if [ "${ISCSI_TMP}" == "initiator_name" ]
            then
                ISCSI_NAME=`echo $line | awk -F " " '{print $2}'`
                
                Log "INFO: Get iSCSI initiator name ${ISCSI_NAME} successful."
                echo ${ISCSI_NAME} >> "${RESULT_FILE}"
            fi            
        done < "${INITIATOR_INFO}"
    done < "${INITIATOR_DEV}"
    
    if [ "$ISCSI_NAME" = "--" ]
    then
        Log "ERROR: Get iSCSI initiator name failed."
        return 1
    fi   
    
    return 0
}


GetFCInfoHPUX()
{
    FC_WWNNUMBER=--
    ioscan -funC fc > "${INITIATOR_DEV}"
    if [[ "$?" != "0" ]]
    then
        Log "ERROR: No FC in your system."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi
     
    while read line
    do
        FC_DEVNAME=`echo $line | awk -F " " '{print $1}'`
        FC_DEVNAME=`echo $FC_DEVNAME | sed 's/ //g'`
        FC_TMPNAME=`expr substr "$FC_DEVNAME" 1 4`
        if [[ "$FC_TMPNAME" = "/dev" ]]
        then
            fcmsutil $FC_DEVNAME > "$INITIATOR_INFO"
            if [[ "$?" != "0" ]]
            then
                Log "ERROR: Get FC $FC_DEVNAME information failed."
                continue
            fi
            while read line
            do
                FC_TMP=`echo $line | awk -F "=" '{print $1}'`
                FC_TMP=`echo $FC_TMP | sed 's/ //g'`
                if [[ "$FC_TMP" = "N_PortPortWorldWideName" ]]
                then
                    FC_TMP=`echo $line | sed 's/ //g'`
                    FC_WWNTMP=`echo $FC_TMP | awk -F "=" '{print $2}'`
                    FC_TMPWWN=$FC_WWNTMP
                    FC_WWNTMP=`echo $FC_WWNTMP | awk '{print toupper($0)}'`
                    FC_WWNTMP=`expr substr $FC_WWNTMP 1 2`
                    if [ "$FC_WWNTMP" = "0X" ]
                    then
                        FC_WWNTMP=`echo $FC_TMPWWN | cut -b3-`
                    else
                        FC_WWNTMP=$FC_TMPWWN
                    fi
                    FC_WWNNUMBER=$FC_WWNTMP 
                    Log "INFO: Get FC $FC_DEVNAME WWN $FC_WWNNUMBER successful."
                        
                    echo ${FC_WWNNUMBER} >> "${RESULT_FILE}"
                fi                
            done < "${INITIATOR_INFO}"
        fi       
    done < "${INITIATOR_DEV}"
    
    if [ "${FC_WWNNUMBER}" = "--" ]
    then
        Log "ERROR: Get FC initiator name failed."
        return 1
    fi
    
    return 0
}

GetISCSIInfoHPUX()
{
    ISCSI_NAME=--
    swlist | grep iSCSI-00 >/dev/null 2>&1
    if [ "$?" != "0" ]
    then
        Log "ERROR: No iscsi initiator in your system."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi 
    
    /opt/iscsi/bin/iscsiutil -l > ${INITIATOR_INFO}
    if [ "$?" != "0" ]
    then
        Log "ERROR: Get iscsi initiator name failed."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi 
    
    while read line
    do
        ISCSI_TMP=`echo $line | awk -F ":" '{print $1}'`
        ISCSI_TMP=`echo ${ISCSI_TMP} | sed 's/ //g'`
        if [[ "${ISCSI_TMP}" = "InitiatorName" ]]
        then
            ISCSI_NAME=`echo $line | awk -F " " '{print $4}'`            
            Log "INFO: Get iSCSI initiator name ${ISCSI_NAME} successful."
            
            echo ${ISCSI_NAME} >> "${RESULT_FILE}"
        fi               
    done < "${INITIATOR_INFO}"
    
    rm -rf "${INITIATOR_INFO}"
    
    if [ "${ISCSI_NAME}" = "--" ]
    then
        Log "ERROR: Get iSCSI initiator name failed."
        return 1
    fi    
    
    return 0
}


GetFCInfoSolaris()
{
    TMP_NAME=""
    TMP_NAME=`luxadm -e port`
    if [ "$TMP_NAME" = "" ]
    then
        Log "ERROR: No HBA in your system."
        return 1
    fi
    
    #T12Z-1971 modify begin
    prtconf -vp | grep -i port_wwn > $INITIATOR_INFO
    if [ "$?" != "0" ]
    then
        prtconf -vp | grep -i port-wwn > $INITIATOR_INFO
        if [ "$?" != "0" ]
        then
            echo Get HBA WWN failed
            Log "ERROR: Get HBA WWN failed."
            return ${ERROR_SCRIPT_EXEC_FAILED}
        fi
    fi
    
    #T12Z-1971 modify end
    while read line
    do
        WWN_NUMBER=--
        
        TMP=`echo $line | nawk -F " " '{print $2}'`
        WWN_TMP=`echo $TMP | sed 's/\.//'`
        TMP_WWN=$WWN_TMP
        WWN_TMP=`echo $WWN_TMP | nawk '{print toupper($0)}'`
        WWN_TMP=`expr $WWN_TMP : '^\(..\).*$'`
        
        if [ "$WWN_TMP" = "0X" ]
        then
            WWN_TMP=`echo $TMP_WWN|cut -b3-`
        else
            WWN_TMP=$TMP_WWN
        fi
        
        WWN_NUMBER=$WWN_TMP    
        Log "INFO: Get HBA WWN:$WWN_NUMBER successful."
        echo "${WWN_NUMBER}" >> "${RESULT_FILE}"
    done < "${INITIATOR_INFO}"
     
    return 0
}

GetISCSIInfoSolaris()
{
    pkginfo SUNWiscsiu SUNWiscsir >/dev/null 2>&1
    if [ "$?" != "0" ]
    then        
        Log "INFO: No iscsi initiator in your system."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi
       
    iscsiadm list initiator-node > "${INITIATOR_INFO}"
    if [ "$?" != "0" ]
    then
        Log "ERROR: Get iSCSI initiator name failed."
        return ${ERROR_SCRIPT_EXEC_FAILED}
    fi
    
    while read line
    do
        ISCSI_NAME=--
        TMP_NAME=`echo $line | nawk -F ":" '{print $1}'`
        if [ "$TMP_NAME" = "Initiator node name" ]
        then
            TMP_NAME=`echo $line | sed 's/Initiator node name:/Initiatornodename:/'`
            ISCSI_NAME=`echo $TMP_NAME | nawk -F " " '{print $2}'`
            Log "INFO: Get iSCSI initiator name:$ISCSI_NAME successful."
            echo "${ISCSI_NAME}" >> "${RESULT_FILE}"
        fi
    done < "${INITIATOR_INFO}" 
    
    if [ "$ISCSI_NAME" = "--" ]
    then
        Log "ERROR: Get iSCSI initiator name failed."
        return 1
    fi  
    
    return 0
}


if [ -f "${RESULT_FILE}" ]
then
    rm -rf "${RESULT_FILE}"
fi

if [ -f "${INITIATOR_DEV}" ]
then
    rm -rf "${INITIATOR_DEV}"
fi

if [ -f "${INITIATOR_INFO}" ]
then
    rm -rf "${INITIATOR_INFO}"
fi

if [ "$QUERY_PARAM" = "fcs" ]
then
    if [ ${SYS} = "AIX" ]
    then
        GetFCInfoAIX
        QUERY_RST=$?
    elif [ ${SYS} = "Linux" ]
    then
        GetLinuxVersion
        GetFCInfoLinux
        QUERY_RST=$?
    elif [ ${SYS} = "HP-UX" ]
    then
        GetFCInfoHPUX
        QUERY_RST=$?
    elif [ ${SYS} = "SunOS" ]
    then
        GetFCInfoSolaris
        QUERY_RST=$?
    else
        Log "ERROR: Unsupported OS."
        QUERY_RST=1
    fi
    
    if [ "$QUERY_RST" = "1" ]
    then
        Log "ERROR: Query FC WWN failed in system $SYS"
    fi
    
elif [ "$QUERY_PARAM" = "iscsis" ]
then
    if [ ${SYS} = "AIX" ]
    then
        GetISCSIInfoAIX
        QUERY_RST=$?
    elif [ ${SYS} = "Linux" ]
    then
        GetISCSIInfoLinux
        QUERY_RST=$?
    elif [ ${SYS} = "HP-UX" ]
    then
        GetISCSIInfoHPUX
        QUERY_RST=$?
    elif [ ${SYS} = "SunOS" ]
    then
        GetISCSIInfoSolaris
        QUERY_RST=$?
    else
        Log "ERROR: Unsupported OS."
        QUERY_RST=1
    fi
    
    if [ "$QUERY_RST" = "1" ]
    then
        Log "ERROR: Query ISCSI IQN failed in system $SYS"
    fi
fi

if [ -f "${INITIATOR_DEV}" ]
then
    rm -rf "${INITIATOR_DEV}"
fi

if [ -f "${INITIATOR_INFO}" ]
then
    rm -rf "${INITIATOR_INFO}"
fi

Log "INFO: Exit $QUERY_RST"
exit $QUERY_RST

