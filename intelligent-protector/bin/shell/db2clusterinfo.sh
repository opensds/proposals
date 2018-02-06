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

##############parameter###############

AGENT_ROOT_PATH=$1
ID=$2
NeedLogFlg=1

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/db2clusterinfo.log"
#load the agent function library script
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

ArgFile="$TMP_PATH/ARG$ID"
RESULT_FILE="${AGENT_ROOT_PATH}/tmp/${RESULT_TMP_FILE_PREFIX}${ID}"
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${ID}"


DBPATH="$TMP_PATH/DBPATH$ID.txt"
TABLESAPCEINFO="$TMP_PATH/TSINFO$ID.txt"
INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"

PKGNAME=""
CLUSTERNAME=""
RESGROUP=""
VGACTIVEMODE=""
VG_NAME=""
LV_NAME=""
ISCLUSTER=0
STORAGETYPE=0

GetErrorCode()
{
    echo ${tmp_result} |grep "SQL30082N"|grep "reason \"24\""|grep "SQLSTATE=08001" > /dev/null 2>&1
    if [ $? = 0 ]
    then
        Log "ERROR:The username or password of db is not correct."
        exit ${ERROR_DB_USERPWD_WRONG}
    fi
 
    echo ${tmp_result} |grep "SQL1032N"|grep "SQLSTATE=57019" > /dev/null 2>&1
    if [ $? = 0 ]
    then
        Log "ERROR:The instance is not started."
        exit ${ERROR_INSTANCE_NOSTART}
    fi
    exit $ERROR_SCRIPT_EXEC_FAILED	
}

GetPackageName()
{
    local CUR_DIR parent_dir workdir
    workdir=$1
    cd ${workdir}
    CUR_DIR=`pwd`
    for x in `ls $CUR_DIR`
    do
        if [ -f "$CUR_DIR/$x" ]
        then
            if [ "${x##*.}" = "cntl" ]
            then
                Log "INFO:find cntl file($CUR_DIR/$x)."
                VG_LIST=`cat $CUR_DIR/$x|grep -v '^#'|grep '^VG\['|$MYAWK -F "=" '{print $2}'|tr -d '"'`
                for VGNAME in $VG_LIST
                do
                    if [ x"$(RDsubstr $VGNAME 1 5)" = x"/dev/" ]
                    then
                        VGNAME=$(RDsubstr $VGNAME 6)
                    fi
                    
                    if [ "$VGNAME" = "$VG_NAME" ]
                    then
                        LV_LIST=`cat $CUR_DIR/$x|grep -v '^#'|grep '^LV\['|$MYAWK -F "=" '{print $2}' | $MYAWK -F ";" '{print $1}' |tr -d '"'`
                        if [ "$STORAGETYPE" = "0" ]
                        then
                            for LVNAME in $LV_LIST
                            do
                                if [ "$LVNAME" = "$LV_NAME" ]
                                then
                                    ISCLUSTER=1
                                    CONFFILE="$CUR_DIR/${x%%.*}.conf"
                                    Log "INFO:The conf file of db in serviceguard is $CONFFILE."
                                    Log "INFO:The cntl file of db in serviceguard is $CUR_DIR/$x."
                                    PKGNAME=`cat $CONFFILE|grep -v '^#'|grep '^PACKAGE_NAME'|$MYAWK '{print $2}'`
                                    VGACTIVEMODE=`cat $CUR_DIR/$x | grep -v '^#'|grep '^VGCHANGE'|$MYAWK -F "\"" '{print $2}'`
                                    VGACTIVEMODE=`echo "$VGACTIVEMODE" | $MYAWK -F 'vgchange' '{print $2}'`
                                    echo "$CLUSTERNAME:$PKGNAME:$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
                                    return
                                fi
                            done
                        elif [ "$STORAGETYPE" = "1" ]
                        then
                            ISCLUSTER=1
                            CONFFILE="$CUR_DIR/${x%%.*}.conf"
                            Log "INFO:The conf file of db in serviceguard is $CONFFILE."
                            Log "INFO:The cntl file of db in serviceguard is $CUR_DIR/$x."
                            PKGNAME=`cat $CONFFILE|grep -v '^#'|grep '^PACKAGE_NAME'|$MYAWK '{print $2}'`
                            VGACTIVEMODE=`cat $CUR_DIR/$x | grep -v '^#'|grep '^VGCHANGE'|$MYAWK -F "\"" '{print $2}'`
                            VGACTIVEMODE=`echo "$VGACTIVEMODE" | $MYAWK -F 'vgchange' '{print $2}'`
                            echo "$CLUSTERNAME:$PKGNAME::$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
                            return
                        fi
                    fi
                done
            fi
        elif [ -d "$x" ]
        then
            cd "$x";
            GetPackageName $CUR_DIR/$x
            if [ "$ISCLUSTER" != "0" ]
            then
                return
            fi
            cd ..
        fi
    done
}

GetValue "$INPUTINFO" INSTNAME
INST_NAME=$ArgValue

GetValue "$INPUTINFO" DBNAME
DB_NAME=$ArgValue

GetValue "$INPUTINFO" DBUSERNAME
DB_USERNAME=$ArgValue

GetValue "$INPUTINFO" DBPASSWORD
DB_USERPWD=$ArgValue

GetValue "$INPUTINFO" CLUSTERTYPE
CLUSTERTYPE=$ArgValue

DB_USERPWD2=\'\"${DB_USERPWD}\"\'
DB_USERPWD1=\'${DB_USERPWD}\'
Log "INSTNAME=$INST_NAME;DBNAME=$DB_NAME;DBUSERNAME=$DB_USERNAME;CLUSTERTYPE=$CLUSTERTYPE"

AIX_CMD=
if [ "$sysName" = "AIX" ]
then
    SHELLTYPE=`cat /etc/passwd | grep "^${INST_NAME}:" | ${MYAWK} -F "/" '{print $NF}'`
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

tmp_result="$(db2 connect to ${DB_NAME} user ${DB_USERNAME} using ${DB_USERPWD1})"
if [ $? != 0 ]
then
    Log "ERROR:$tmp_result"
    GetErrorCode
fi

echo "$tmp_result">>"$LOG_FILE_NAME"
Log "INFO:connect to db $DB_NAME succ."

if [ -f "${RESULT_FILE}.tmp" ]
then
    rm "${RESULT_FILE}.tmp"
fi

touch "${RESULT_FILE}.tmp"

if [ "$CLUSTERTYPE" = "2" ]
then
    Log "INFO:The cluster is vcs cluster."
    DBRES_LIST=`hatype -resources Db2udb`
    if [ $? != 0 ]
    then
        CLUSTERSERVICE_NOSTART_CHECK=`echo  "$DBRES_LIST"|grep "VCS ERROR V-16-1-10600 Cannot connect to VCS engine"`
        if [ $? = 0 ]
        then
            Log "$DBRES_LIST"
            Log "ERROR:The vcs cluster is not started"
            exit $ERROR_CLUSTER_SERVICE_NOSTART
        fi
        Log "$DBRES_LIST"
        Log "ERROR:The vcs cluster is error."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    Log "INFO:Vcs cluster service has been started."
    Log "INFO:Begin to analyse cluster info of db $DB_NAME."
    for DBRES in $DBRES_LIST
    do
        DBNAME=`hares -value $DBRES DatabaseName | tr '[a-z]' '[A-Z]'`
        INSTNAME=`hares -value $DBRES DB2InstOwner`
        if [ "$DBNAME" = "$DB_NAME" -a "$INSTNAME" = "$INST_NAME" ]
        then
            ISCLUSTER=1
            RESGRP_NAME=`hares -value $DBRES Group 2>>"$LOG_FILE_NAME"`
            CLUSTER_NAME=`hagrp -value $RESGRP_NAME ClusterList|$MYAWK -F " " '{print $1}'`
            RESGROUP=$DBRES
            echo "$CLUSTER_NAME:$DBRES::$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
            break
        fi
    done
    Log "INFO:Get vcs cluster info of db $DBNAME succ."
elif [ "$CLUSTERTYPE" = "4" ]
then
    Log "INFO:The cluster is powerha cluster."
    touch "$TABLESAPCEINFO"
    chmod 666 "$TABLESAPCEINFO"
    touch "$DBPATH"
    chmod 666 "$DBPATH" 
    if [ ! -f "$DBPATH" ]
    then
        Log "ERROR:$DBPATH is not exist."
        exit $ERROR_TMP_FILE_IS_NOT_EXIST
    fi
	
    Log "INFO:Begin to get db file info."
    
    su - ${INST_NAME} ${AIX_CMD} << EOF
    db2 connect to ${DB_NAME} user ${DB_USERNAME} using ${DB_USERPWD2}
    db2pd -db $DB_NAME -dbcfg  | grep "Path to log files (memory)"|$MYAWK '{print \$NF}'>> "$DBPATH"
    db2 list tablespaces show detail |sed -n '4,\${p;n;n;n;n;n;n;n;n;n;n;n;n;n;n;n;n;}'|$MYAWK '{print \$NF}'>>"$TABLESAPCEINFO" 
    while read line
    do
       db2 list tablespace containers for \$line | grep "/" |$MYAWK '{print \$NF}' >>"$DBPATH"
    done < "$TABLESAPCEINFO"
EOF

    cat "$DBPATH">>"$LOG_FILE_NAME"
    Log "INFO:End to get db file info."
    CLUSTER_SERVICE_STATE=`lssrc -ls clstrmgrES | sed -n '1p' | $MYAWK '{print $3}'`
    if [ "$CLUSTER_SERVICE_STATE" != "ST_STABLE" ]
    then
        Log "ERROR:The powerha cluster is not started,state($CLUSTER_SERVICE_STATE)."
        exit $ERROR_CLUSTER_SERVICE_NOSTART
    fi
    
    Log "INFO:Powerha cluster service has been started."
    CLUSTERNAME=`/usr/es/sbin/cluster/utilities/cltopinfo|grep "Cluster Name"|$MYAWK '{print $NF}'`
    RESGROUP_LIST=`/usr/es/sbin/cluster/utilities/clshowres|grep "Resource Group Name"|$MYAWK '{print $NF}'`
    if [ "$RESGROUP_LIST" = "" ]
    then
        Log "ERROR:There is not resource groups in the cluster."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
	
    Log "INFO:Begin to analyse cluster info of db $DB_NAME."
    while read line
    do
        FILE_TYPE=`ls -l ${line} |$MYAWK '{print $1}'`
        FILE_TYPE=$(RDsubstr $FILE_TYPE 1 1)
        if [ "$FILE_TYPE" = "c" ]
        then
            Log "INFO:The db file is on raw device,device is ($line)."
            tmpDevName=$(RDsubstr $line 7)
        elif [ "$FILE_TYPE" = "-" -o  -d "$line" ]
        then
            Log "INFO:The db file is on fs,fs is ($line)."
            FILESYSTEM=`df -i ${line} 2>>"$LOG_FILE_NAME"| sed '1d' | $MYAWK '{print $1}'|sed -n '1p'`
            if [ $? != 0 ]
            then
                Log "ERROR:exec df -i ${line} failed."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
            tmpDevName=$(RDsubstr $FILESYSTEM 6)  
        fi
        VGNAME=`lslv -L $tmpDevName 2>>"$LOG_FILE_NAME"| grep 'VOLUME GROUP:' | $MYAWK '{print $6}'`
        if [ $? != 0 ]
        then
            Log "ERROR:exec lslv -L $tmpDevName failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        if [ "$FILE_TYPE" = "-" ]
        then
            MOUNT_POINT=`mount |grep -w "$FILE_SYSTEM" |$MYAWK '{print $2}'`
            if [ $? != 0 ]
            then
                Log "ERROR:exec mount | grep -w $FILE_SYSTEM failed."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        fi
        
        Log "INFO:Storage info of db file($line):$VGNAME;$tmpDevName."
        for RESGROUP in $RESGROUP_LIST
        do
            VG_LIST=`/usr/es/sbin/cluster/utilities/clshowres -g $RESGROUP|grep "Volume Groups"|sed -n '1p'|$MYAWK '{for(x=3;x<=NF;x++) print $x}'`
            CHECK=`echo ${VG_LIST}|grep "$VGNAME"`
            if [ $? = 0 ]
            then
                ISCLUSTER=1
                echo "$CLUSTERNAME:$RESGROUP::$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
            fi
        done
        Log "INFO:Analyse cluster info of db file($line) succ."
    done < "$DBPATH"

    Log "INFO:Get powerha cluster info of db $DB_NAME succ."
elif [ "$CLUSTERTYPE" = "5" ]
then
    Log "INFO:The cluster is serviceguard cluster."
    CONF_DIR="/etc/cmcluster"
    CLUSTER_CONFFILE="/etc/cmcluster/cmclconf.ascii"
    CLUSTERNAME=`cat $CLUSTER_CONFFILE|grep -v '#'|grep '^CLUSTER_NAME'|$MYAWK '{print $2}'`
    
    touch "$TABLESAPCEINFO"
    chmod 666 "$TABLESAPCEINFO"
    touch "$DBPATH"
    chmod 666 "$DBPATH" 
    if [ ! -f "$DBPATH" ]
    then
        Log "ERROR:$DBPATH is not exist."
        exit $ERROR_TMP_FILE_IS_NOT_EXIST
    fi
	
    Log "INFO:Begin to get db file info."
    
    su - ${INST_NAME} ${AIX_CMD} << EOF
    db2 connect to ${DB_NAME} user ${DB_USERNAME} using ${DB_USERPWD2}
    db2pd -db $DB_NAME -dbcfg  | grep "Path to log files (memory)"|$MYAWK '{print \$NF}'>> "$DBPATH"
    db2 list tablespaces show detail |sed -n '4,\${p;n;n;n;n;n;n;n;n;n;n;n;n;n;n;n;n;}'|$MYAWK '{print \$NF}'>>"$TABLESAPCEINFO" 
    while read line
    do
       db2 list tablespace containers for \$line | grep "/" |$MYAWK '{print \$NF}' >>"$DBPATH"
    done < "$TABLESAPCEINFO"
EOF

    cat "$DBPATH" >> "$LOG_FILE_NAME"
    Log "INFO:End to get db file info."
    Log "INFO:Begin to analyse cluster info of db $DB_NAME."
    while read line
    do
        FILE_TYPE=`ls -l ${line} |$MYAWK '{print $1}'`
        FILE_TYPE=$(RDsubstr $FILE_TYPE 1 1)
        if [ "$FILE_TYPE" = "c" ]
        then
            Log "INFO:The db file is on raw device,device is ($line)."
            STORAGETYPE=1
            LV_NAME=$line
            VG_NAME=`echo $line | $MYAWK -F "/" '{print $3}'`
        elif [ "$FILE_TYPE" = "-"  -o  -d "$line" ]
        then
            STORAGETYPE=0
            LANG=C
            Log "INFO:The db file is on fs,fs is ($line)."
            LV_NAME=`df -i $line 2>>"$LOG_FILE_NAME"| sed -n '1p'| $MYAWK -F "(" '{print $2}' | ${MYAWK} -F ")" '{print $1}' | tr -d " "`
            if [ "$LV_NAME" = "" ]
            then
                Log "Error: df -i $line failed."
                exit $ERROR_TMP_FILE_IS_NOT_EXIST
            fi
            
            VG_NAME=`lvdisplay $LV_NAME | grep "VG Name" | $MYAWK -F "/" '{print $NF}'`
            if [ "$VG_NAME" = "" ]
            then
                Log "Error: lvdisplay $LV_NAME failed."
                exit $ERROR_TMP_FILE_IS_NOT_EXIST
            fi
            
            FS=`mount |grep -w "$LV_NAME" |$MYAWK '{print $3}'`
            if [ "$FS" = "" ]
            then
                Log "Error: FS is null."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        fi
        Log "INFO:vgname($VG_NAME),lvname($LV_NAME)."
        
        Log "INFO:Begin to analyse cluster info of db $DB_NAME."
        GetPackageName $CONF_DIR
        Log "INFO:End to analyse cluster info of db $DB_NAME."
        RESGROUP=$PKGNAME
    done < "$DBPATH"
    
    Log "INFO:Get serviceguard cluster info of inst $INST_NAME succ."
    
elif [ "$CLUSTERTYPE" = "6" ]
then
    Log "INFO:Begin to get rhcs cluster info of inst $INST_NAME."
    ClusterConfigFile=/etc/cluster/cluster.conf
    CLUSTERNAME="rhcscluster"
	touch "$DBPATH"
    chmod 666 "$DBPATH" 
    if [ ! -f "$DBPATH" ]
    then
        Log "ERROR:$DBPATH is not exist."
        exit $ERROR_TMP_FILE_IS_NOT_EXIST
    fi
	
    su - ${INST_NAME} ${AIX_CMD} << EOF
    db2 connect to ${DB_NAME} user ${DB_USERNAME} using ${DB_USERPWD2}
    db2pd -db $DB_NAME -dbcfg  | grep "Path to log files (memory)"|$MYAWK '{print \$NF}'>> "$DBPATH"
EOF
    cat "$DBPATH">>"$LOG_FILE_NAME"
    PDName=`sed -n '1p' "$DBPATH"`
    if [ "$PDName" = "" ]
    then
        Log "ERROR: db2pd $DB_NAME failed"
	    exit
    fi
	DeleteFile "${DBPATH}"
	DevName=`df -i $PDName 2>>"$LOG_FILE_NAME"| sed '1d' | $MYAWK '{print $1}'|sed -n '1p'`
	if [ "$DevName" = "" ]
	then
	    Log "Error: df -i $PDName failed"
	fi
	MountPoint=`mount |grep -w "$DevName" |$MYAWK '{print $3}'`
	if [ "$MountPoint" = "" ]
	then
	    Log "Error: MountPoint is null"
		exit $ERROR_SCRIPT_EXEC_FAILED
	fi
	RHCSMPFileTmp="$TMP_PATH/RHCSMPFileTmp"
	RHCSMPFile="$TMP_PATH/RHCSMPFile"
	MPTag=mountpoint=\"$MountPoint\"
	cat "${ClusterConfigFile}" | grep ${MPTag} >"$RHCSMPFileTmp"
	if [ $? != 0 ]
	then
	    Log "ERROR:cat cluster.conf grep $MPTag failed"
		DeleteFile "${RHCSMPFileTmp}"
		exit $ERROR_SCRIPT_EXEC_FAILED
	fi
	sed 's/\/>//g' "$RHCSMPFileTmp" > "$RHCSMPFile"
	if [ $? != 0 ]
	then
	    Log "sed RHCSMPFile failed"
		exit $ERROR_SCRIPT_EXEC_FAILED
	fi
	FsName=`sed -n 's/ /\n/gp' "$RHCSMPFile" | sed -n '/name/p' | sed -n 's/name=//gp'`
	if [ $? != 0 ]
	then
	    Log "ERROR:sed $RHCSMPFile failed"
		DeleteFile "${RHCSMPFile}" "${RHCSMPFileTmp}"
		exit $ERROR_SCRIPT_EXEC_FAILED
	fi
	DeleteFile "${RHCSMPFile}" "${RHCSMPFileTmp}"
	Log "PDName=$PDName, DevName=$DevName, MountPoint=$MountPoint, FsName=$FsName"
	BEGIN_NUMS=`cat "$ClusterConfigFile" | grep -n '<service ' | $MYAWK -F ":" '{print $1}'`
	if [ "$BEGIN_NUMS" = "" ]
	then
	    Log "ERROR:BEGIN_NUMS is empty."
        exit $ERROR_SCRIPT_EXEC_FAILED
	fi
	END_NUMS=`cat "$ClusterConfigFile" | grep -n '</service>' | $MYAWK -F ":" '{print $1}'`
	if [ "$END_NUMS" = "" ]
	then
	    Log "ERROR:END_NUMS is empty."
        exit $ERROR_SCRIPT_EXEC_FAILED
	fi
	DB_NUM=`cat "$ClusterConfigFile" | grep -n 'fs ref' | grep "$FsName" | $MYAWK -F ":" '{print $1}'`
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
		    if [ $DB_NUM -lt $lineNo ]
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
		sed -n "$beginNum,$endNum p" "$ClusterConfigFile" | grep '<service' > "$RHCSMPFile"
        resGroupName=`sed -n 's/ /\n/gp' "$RHCSMPFile" | sed -n '/name/p' | sed -n 's/name=//gp'`	
		Log "resGroupName=$resGroupName"
        if [ "$resGroupName" = "" ]
        then
		    Log "ERROR:resGroupName is empty."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi	
		DeleteFile "${RHCSMPFile}"
	    strLen=`echo $resGroupName | $MYAWK '{print length}'`
		cutLen=`expr $strLen - 2`
		Log "INFO resGroupName=$resGroupName, len=$cutLen"
	    RESGROUP=$(RDsubstr $resGroupName 2 $cutLen)
		Log "RESGROUP=$RESGROUP"
	    if [ "$RESGROUP" = "" ]
	    then
	        Log "ERROR:RESGROUP is empty."
            exit $ERROR_SCRIPT_EXEC_FAILED
	    fi
	    echo "$CLUSTERNAME:$RESGROUP::$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
	done
	ISCLUSTER=1
else
    Log "ERROR:The cluster type($CLUSTERTYPE) is not be surported."
    exit $ERROR_PARAM_WRONG
fi

if [ "$ISCLUSTER" = "0" ]
then
    Log "ERROR:The db is not in cluster."
    exit $ERROR_CLUSTER_DB_NOT_INCLUSTER
fi

rm -fr "$ArgFile"
rm -fr "$DBPATH" 
rm -fr "$TABLESAPCEINFO"
cat "$RESULT_FILE.tmp"|sort|uniq > "$RESULT_FILE"
Log "INFO:Cluster INFO of $DB_NAME:$CLUSTERNAME;$RESGROUP;$VGACTIVEMODE."
rm -fr "$RESULT_FILE.tmp"
exit 0
