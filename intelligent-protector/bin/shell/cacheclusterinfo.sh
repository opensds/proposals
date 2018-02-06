#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

AGENT_ROOT_PATH=$1
PID=$2
NeedLogFlg=1

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/cacheclusterinfo.log"
#load the agent function library script
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/cachefunc.sh"

ArgFile="$TMP_PATH/ARG$PID"
TMP_FILE="$TMP_PATH/TMP$PID"
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${PID}"


INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"

CLUSTERNAME=""
RESGROUP=""
VGACTIVEMODE=""
VG_NAME=""
INSFLG=1
DBPATH="$TMP_PATH/DBPATH$PID.txt"

GetValue "$INPUTINFO" INSTNAME
INST_NAME=$ArgValue

GetValue "$INPUTINFO" CLUSTERTYPE
CLUSTERTYPE=$ArgValue

Log "INSTNAME=$INST_NAME;CLUSTERTYPE=$CLUSTERTYPE"

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


INST_STAT=`ccontrol view $INST_NAME | grep status | ${MYAWK} -F":    " '{print $2}' | ${MYAWK} -F"," '{print $1}'`
if [ x"$INST_STAT" = x"down" ]
then
    Log "ERROR:get instance($INST_NAME) status is $INST_STAT."
    exit $ERROR_NOT_ACTIVE_NODE
fi

DeleteFile "${RESULT_FILE}.tmp"

touch "${RESULT_FILE}.tmp"

IsInstanceCorrect ${INST_NAME}

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
			Log "WARN:CACHEINSTANCE=$INST_NAME,DBNAME=$SUBDBNAME',$CACHE_DIR/CACHE.DAT not found."
		fi
		Log "INFO:End to get db($SUBDBNAME) info."
	done
fi

if [ "$sysName" = "Linux" ]
then
    GetRedhatDbInfo "$INST_DIR/$INST_FILE"
fi

#get WIJ dir
GetWIJDir "$INST_DIR/$INST_FILE"
#get jrn dir
GetJRNDir "$INST_DIR/$INST_FILE"
Log "INFO:Begin get conf file dir info."
echo "$INST_DIR" >> "$DBPATH"
Log "INFO:conf file dir:$INST_DIR."
Log "INFO:End get conf file dir info."
Log "INFO:End to get instance info."

if [ "$CLUSTERTYPE" = "4" ]
then
    Log "INFO:The cluster is powerha cluster."
    CLUSTER_SERVICE_STATE=`lssrc -ls clstrmgrES | sed -n '1p' | ${MYAWK} '{print $3}'`  
    if [ "$CLUSTER_SERVICE_STATE" != "ST_STABLE" ]
    then
        Log "ERROR:The powerha cluster is not started,state($CLUSTER_SERVICE_STATE)."
        exit $ERROR_CLUSTER_SERVICE_NOSTART
    fi
    
    Log "INFO:Powerha cluster service has been started."
    CLUSTERNAME=`/usr/es/sbin/cluster/utilities/cltopinfo|grep "Cluster Name"|${MYAWK} '{print $NF}'`  
    RESGROUP_LIST=`/usr/es/sbin/cluster/utilities/clshowres|grep "Resource Group Name"|${MYAWK} '{print $NF}'`  
    if [ "$RESGROUP_LIST" = "" ]
    then
        Log "ERROR:There is not resource groups in the cluster."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi

    Log "INFO:Begin to analyse cluster info of instance $INST_NAME."
    while read line
    do
        Log "INFO:The db file is on fs,fs is ($line)."
        FILESYSTEM=`df -i "${line}" 2>>"$LOG_FILE_NAME"| sed '1d' | ${MYAWK} '{print $1}'|sed -n '1p'`
        if [ $? != 0 ]
        then
            Log "ERROR:exec df -i ${line} failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        tmpDevName=`RDsubstr $FILESYSTEM 6`
        VGNAME=`lslv -L $tmpDevName 2>>"$LOG_FILE_NAME"| grep 'VOLUME GROUP:' | ${MYAWK} '{print $6}'`  
        if [ $? != 0 ]
        then
            Log "ERROR:exec lslv -L $tmpDevName failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi

        MOUNT_POINT=`mount |grep -w "$FILE_SYSTEM" |${MYAWK} '{print $2}'`
        if [ $? != 0 ]
        then
            Log "ERROR:exec mount | grep -w $FILE_SYSTEM failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        
        Log "INFO:Storage info of db file($line):$VGNAME;$tmpDevName."
        RESULT="false"
        for RESGROUP in $RESGROUP_LIST
        do
            VG_LIST=`/usr/es/sbin/cluster/utilities/clshowres -g $RESGROUP|grep "Volume Groups"|sed -n '1p'|${MYAWK} '{for(x=3;x<=NF;x++) print $x}'` 
            CHECK=`echo ${VG_LIST}|grep "$VGNAME"`
            if [ $? = 0 ]
            then
                echo "$CLUSTERNAME:$RESGROUP::$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
                RESULT="true"
            fi
        done
        if [ "$RESULT" = "false" ]
        then
            Log "ERROR:cat cluster resource error!"  
            exit $ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL
        fi
        Log "INFO:Analyse cluster info of db file($line) succ."
    done < "$DBPATH"

    Log "INFO:Get powerha cluster info of instance $INST_NAME succ."
fi

if [ "$CLUSTERTYPE" = "6" ]
then
    ClusterConfigFile=/etc/cluster/cluster.conf
    Log "INFO:Begin to get rhcs cluster info of db $DBNAME."
    while read line
    do
        DevName=`df -i "${line}" 2>>"$LOG_FILE_NAME"| sed '1d' | ${MYAWK} '{print $1}'|sed -n '1p'`
        if [ "$DevName" = "" ]
        then
            Log "ERROR: get DevName failed"
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        MountPoint=`mount |grep -w "$DevName" |${MYAWK} '{print $3}'`
        if [ "$MountPoint" = "" ]
        then
            Log "ERROR: MountPoint is null"
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        RHCSMPFileTmp="$TMP_PATH/RHCSMPFileTmp"
        RHCSMPFile="$TMPPATH/RHCSMPFile"
        MPTag=mountpoint=\"$MountPoint\"
        cat "$ClusterConfigFile" | grep $MPTag >"$RHCSMPFileTmp"
        if [ $? != 0 ]
        then
            Log "ERROR:cat cluster.conf grep $MPTag failed"
            exit $ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL
        fi
        sed 's/\/>//g' "$RHCSMPFileTmp" > "$RHCSMPFile"
        if [ $? != 0 ]
        then
            Log "ERROR:replace exec failed"
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        FsName=`sed -n 's/ /\n/gp' "$RHCSMPFile" | sed -n '/name/p' | sed -n 's/name=//gp'`
        if [ $? != 0 ]
        then
            Log "ERROR:get FsName failed"
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        DeleteFile "${RHCSMPFile}" "${RHCSMPFileTmp}"
        Log "FirstDbf=$FirstDbf, DevName=$DevName, MountPoint=$MountPoint, FsName=$FsName"
        CLUSTERNAME="rhcscluster"
        BEGIN_NUMS=`cat "$ClusterConfigFile" | grep -n '<service ' | ${MYAWK} -F ":" '{print $1}'`
        if [ "$BEGIN_NUMS" = "" ]
        then
            Log "ERROR:BEGIN_NUMS is empty."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        END_NUMS=`cat "$ClusterConfigFile" | grep -n '</service>' | ${MYAWK} -F ":" '{print $1}'`
        if [ "$END_NUMS" = "" ]
        then
            Log "ERROR:END_NUMS is empty."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        DB_NUM=`cat "$ClusterConfigFile" | grep -n 'fs ref' | grep "$FsName" | ${MYAWK} -F ":" '{print $1}'`
        if [ "$DB_NUM" = "" ]
        then
            Log "ERROR:DB_NUM is empty." 
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        for dbNum in $DB_NUM
        do
            beginIndex=0
            endIndex=0	
            beginNum=0 
            endNum=0
            for lineNo in $END_NUMS
            do
                endIndex=`expr $endIndex + 1`
                if [ $dbNum -lt $lineNo ]
                then 
                    endNum=$lineNo
                    break
                fi
            done
            for lineNo in $BEGIN_NUMS
            do
                beginIndex=`expr $beginIndex + 1`
                if [ $beginIndex -eq $endIndex ]
                then
                    beginNum=$lineNo
                    break
                fi
            done
            Log "beginNum=$beginNum, endNum=$endNum"
            sed -n "$beginNum,$endNum p" "$ClusterConfigFile" | grep '<service' >"$RHCSMPFile"
            resGroupName=`sed -n 's/ /\n/gp' "$RHCSMPFile" | sed -n '/name/p' | sed -n 's/name=//gp'`	
            Log "resGroupName=$resGroupName"
            if [ "$resGroupName" = "" ]
            then
                Log "ERROR:resGroupName is empty."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi	
            DeleteFile "${RHCSMPFile}"
            strLen=`echo $resGroupName | ${MYAWK} '{print length}'`
            cutLen=`expr $strLen - 2`
            Log "INFO resGroupName=$resGroupName, len=$cutLen"
            RESGROUP=`RDsubstr $resGroupName 2 $cutLen`
            Log "RESGROUP=$RESGROUP"
            if [ "$RESGROUP" = "" ]
            then
                Log "ERROR:RESGROUP is empty."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
            echo "$CLUSTERNAME:$RESGROUP:$DG_NAME:$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
        done
    done < "$DBPATH"
fi

rm -fr "$ArgFile"
rm -fr "$DBPATH" 
cat "$RESULT_FILE.tmp"|sort|uniq > "$RESULT_FILE"
Log "INFO:Cluster INFO of $CLUSTERNAME;$RESGROUP;$VGACTIVEMODE."
rm -fr "$RESULT_FILE.tmp"
exit 0
