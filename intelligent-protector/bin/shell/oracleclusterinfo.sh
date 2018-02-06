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
PID=$2
NeedLogFlg=1

IN_ORACLE_HOME=""
ArgFile="$TMP_PATH/ARG$PID"
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/oracleclusterinfo.log"
DBUser=""
GridUser=""
#load the agent function library script
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/oraclefunc.sh"

QUERYNAMECRIPT="$TMP_PATH/QueryFileName$PID.sql"
QUERYNAMECRIPTRST="$TMP_PATH/QueryFileNameRST$PID.sql"
DBTMPFILE="$TMP_PATH/db$PID.txt"
DGFILE="$TMP_PATH/dg$PID.txt"

PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${PID}"
INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"
AUTHMODE=0
DBROLE=""
PKGNAME=""
RESGROUP=""
VGACTIVEMODE=""
DG_NAME=""
ISCLUSTER=0
STORAGETYPE=0

GetPackageName_SG()
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
                VG_LIST=`cat $CUR_DIR/$x|grep -v '^#'|grep '^VG\['|awk -F "=" '{print $2}'|tr -d '"'`
                for VGNAME in $VG_LIST
                do
                    if [ x"`RDsubstr $VGNAME 1 5`" = x"/dev/" ]
                    then
                        VGNAME=`RDsubstr $VGNAME 6`
                    fi
                    
                    if [ "$VGNAME" = "$VG_NAME" ]
                    then
                        LV_LIST=`cat $CUR_DIR/$x|grep -v '^#'|grep '^LV\['|awk -F "=" '{print $2}' | awk -F ";" '{print $1}' |tr -d '"'`
                        if [ "$STORAGETYPE" = "0" ]
                        then
                            for LVNAME in $LV_LIST
                            do
                                if [ "$LVNAME" = "$LV_NAME" ]
                                then
                                    ISCLUSTER=1
                                    CONFFILE="$CUR_DIR/${x%%.*}.conf"
                                    Log "INFO:The conf file of db in serviceguard is `basename $CONFFILE`."
                                    Log "INFO:The cntl file of db in serviceguard is $x."
                                    PKGNAME=`cat $CONFFILE|grep -v '^#'|grep '^PACKAGE_NAME'|awk '{print $2}'`
                                    VGACTIVEMODE=`cat $CUR_DIR/$x | grep -v '^#'|grep '^VGCHANGE'|awk -F "\"" '{print $2}'`
                                    VGACTIVEMODE=`echo "$VGACTIVEMODE" | awk -F 'vgchange' '{print $2}'`
                                    echo "$CLUSTERNAME:$PKGNAME:$DG_NAME:$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
                                    return
                                fi
                            done
                        elif [ "$STORAGETYPE" = "1" ]
                        then
                            ISCLUSTER=1
                            CONFFILE="$CUR_DIR/${x%%.*}.conf"
                            Log "INFO:The conf file of db in serviceguard is `basename $CONFFILE`."
                            Log "INFO:The cntl file of db in serviceguard is $x."
                            PKGNAME=`cat $CONFFILE|grep -v '^#'|grep '^PACKAGE_NAME'|awk '{print $2}'`
                            VGACTIVEMODE=`cat $CUR_DIR/$x | grep -v '^#'|grep '^VGCHANGE'|awk -F "\"" '{print $2}'`
                            VGACTIVEMODE=`echo "$VGACTIVEMODE" | awk -F 'vgchange' '{print $2}'`
                            echo "$CLUSTERNAME:$PKGNAME:$DG_NAME:$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
                            return
                        fi
                    fi
                done
            fi
        elif [ -d "$x" ]
        then
            cd "$x";
            GetPackageName_SG $CUR_DIR/$x
            if [ "$ISCLUSTER" != "0" ]
            then
                return
            fi
            cd ..
        fi
    done
}

####################################################################################
# function name: QueryTableSpacePath()
# aim:           query db file localtion function
# input:         na
# output:        
####################################################################################
QueryTableSpacePath()
{
    DBTABLESPACENAME=$1
    #************************temporary sql script for quering file_name*****************
    echo "set linesize 500;" > "$QUERYNAMECRIPT"

    echo "select file_name from dba_data_files;" >> "$QUERYNAMECRIPT"
    echo 'select MEMBER from v$logfile;' >> "$QUERYNAMECRIPT"
    echo 'select name from v$controlfile;' >> "$QUERYNAMECRIPT"

    echo 'select file_name from dba_temp_files;' >> "$QUERYNAMECRIPT"
    echo "select VALUE from v\$parameter where name='spfile';" >> "$QUERYNAMECRIPT"

    echo "exit" >> "$QUERYNAMECRIPT"
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"

    Log "INFO:Exec SQL to get files of database."
    OracleExeSql "${LOGIN_AUTH}" "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" "${DBINSTANCE}" 30 >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    if [ "${RET_CODE}" -ne "0" ]
    then
        Log "ERROR:Search database(${DBINSTANCE}) file failed."
        DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}"
        exit ${RET_CODE}
    fi
    
    #****************************get db location********************************
    if [ `cat "$QUERYNAMECRIPTRST" | wc -l` -eq 0 ]
    then
        DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" 
        Log "ERROR:QueryTableSpacePath:Connect to database $DBNAME failed, result is null."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
}

####################################################################################
# function name: QueryRHCSDBPath()
# aim:           query db file localtion function
# input:         na
# output:        
####################################################################################
QueryRHCSDBPath()
{
    Log "Enter QueryRHCSDBPath"
    QueryRHCSDBScript="$TMP_PATH/QueryRHCSDBName$PID.sql"
    QueryRHCSDBResult="$TMP_PATH/QueryRHCSDBResult$PID.sql"
    #************************temporary sql script for quering file_name*****************
    echo "select file_name from dba_data_files;" > "$QueryRHCSDBScript"
    LOGIN_AUTH="`CreateLoginCmd ${AUTHMODE} ${DBUSER} \"${DBUSERPWD}\" \"${DBROLE}\"`"
    Log "INFO:Exec SQL to get files of database in QueryRHCSDBPath."
    OracleExeSql "${LOGIN_AUTH}" ${QueryRHCSDBScript} ${QueryRHCSDBResult} ${DBINSTANCE} 30 >> "${LOG_FILE_NAME}" 2>&1
    RET=$?
    if [ $RET != 0 ]
    then
        Log "ERROR:Search database(${DBINSTANCE}) file failed."
        DeleteFile "${QueryRHCSDBScript}" "${QueryRHCSDBResult}"
        exit ${RET}
    fi
    Log "QueryRHCSDBScript=$QueryRHCSDBScript, QueryRHCSDBResult=$QueryRHCSDBResult, DBINSTANCE=$DBINSTANCE"
    #****************************check********************************
    if [ `cat "$QueryRHCSDBResult" | wc -l` -eq 0 ]
    then
        DeleteFile "${QueryRHCSDBScript}" "${QueryRHCSDBResult}"
        Log "ERROR:QueryTableSpacePath:Connect to database $DBNAME failed, result is null."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    Log "INFO: Result of sql query"
    cat "$QueryRHCSDBResult">>"${LOG_FILE_NAME}"
    First_dbf=`sed -n '/----------/,/^ *$/p' "${QueryRHCSDBResult}" | sed -e '/----------/d' -e '/^ *$/d' | sed -n "1p"`
    Log "First_dbf=$First_dbf"
    echo $First_dbf
    DeleteFile "${QueryRHCSDBScript}" "${QueryRHCSDBResult}"
}

#get oracle user name
getOracleUserInfo
if [ $? -ne 0 ]
then
    Log "Get Oracle user info failed."
    exit 1
fi

#################Entry of script to query the information of oracle portfolio###########
GetValue "$INPUTINFO" INSTNAME
DBINSTANCE=$ArgValue

GetValue "$INPUTINFO" DBNAME
DBNAME=$ArgValue

GetValue "$INPUTINFO" DBUSERNAME
DBUSERL=$ArgValue
DBUSER=`echo "$DBUSERL" | tr '[A-Z]' '[a-z]'`

GetValue "$INPUTINFO" DBPASSWORD
DBUSERPWD=$ArgValue

GetValue "$INPUTINFO" CLUSTERTYPE
CLUSTERTYPE=$ArgValue

Log "INSTNAME=$DBINSTANCE;DBNAME=$DBNAME;DBUSERNAME=$DBUSER;CLUSTERTYPE=$CLUSTERTYPE"

if [ "$sysName" = "HP-UX" ]
then
    SG_CONF_DIR="/etc/cmcluster"
    if [ -d "$SG_CONF_DIR" ]
    then
        GetOracle_homeOfSG $SG_CONF_DIR $DBINSTANCE
    fi
fi

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

if [ "$CLUSTERTYPE" = "4" -o "$CLUSTERTYPE" = "6" -o "$CLUSTERTYPE" = "5" -o "$CLUSTERTYPE" = "7" ]
then
    ISCLUSTER=0
    GetUserShellType
    Log "INFO:Begin to Stat oracle database file info."
    QueryTableSpacePath
    FILE_TYPE_CHECK=`cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "+"`
    if [ $? = 0 ]
    then
        Log "ERROR:The storage type(asm) in cluster is not be suported."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    cat "${QUERYNAMECRIPTRST}" | sed '/^$/d' | grep "/" > "${DBTMPFILE}"
    DeleteFile "${QUERYNAMECRIPT}" "${QUERYNAMECRIPTRST}" 
    cat "${DBTMPFILE}" >> "$LOG_FILE_NAME"
    Log "INFO:Stat oracle database file info succ."
fi


if [ "$CLUSTERTYPE" = "4" ]
then
    Log "INFO:The cluster is powerha cluster."
    
    CLUSTERNAME=`/usr/es/sbin/cluster/utilities/cltopinfo|grep "Cluster Name"|awk '{print $NF}'`
    RESGROUP_LIST=`/usr/es/sbin/cluster/utilities/clshowres|grep "Resource Group Name"|awk '{print $NF}'`
    if [ "$RESGROUP_LIST" = "" ]
    then
        Log "ERROR:There is not resource groups in the cluster."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    
    Log "INFO:Begin to analyse cluster info of db $DBNAME."
    while read line
    do
        FILE_TYPE=`ls -l ${line} |awk '{print $1}'`
        FILE_TYPE=`RDsubstr $FILE_TYPE 1 1`
        if [ "$FILE_TYPE" = "c" ]
        then
            Log "INFO:The db file is on raw device,device is ($line)."
            tmpDevName=`RDsubstr $line 7`
        elif [ "$FILE_TYPE" = "-" ]
        then
            Log "INFO:The db file is on fs,fs is ($line)."
            FILESYSTEM=`df -i ${line} 2>>"$LOG_FILE_NAME"| sed '1d' | awk '{print $1}'|sed -n '1p'`
            if [ $? != 0 ]
            then
                Log "ERROR:exec df -i ${line} failed."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
            tmpDevName=`RDsubstr $FILESYSTEM 6`  
        fi
        VGNAME=`lslv -L $tmpDevName 2>>"$LOG_FILE_NAME"| grep 'VOLUME GROUP:' | awk '{print $6}'`
        if [ $? != 0 ]
        then
            Log "ERROR:exec lslv -L $tmpDevName failed."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi
        if [ "$FILE_TYPE" = "-" ]
        then
            MOUNT_POINT=`mount |grep -w "$FILE_SYSTEM" |awk '{print $2}'`
            if [ $? != 0 ]
            then
                Log "ERROR:exec mount | grep -w $FILE_SYSTEM failed."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        fi
        
        Log "INFO:Storage info of db file($line):$VGNAME;$tmpDevName."
        for RESGROUP in $RESGROUP_LIST
        do
            VG_LIST=`/usr/es/sbin/cluster/utilities/clshowres -g $RESGROUP|grep "Volume Groups"|sed -n '1p'|awk '{for(x=3;x<=NF;x++) print $x}'`
            CHECK=`echo ${VG_LIST}|grep "$VGNAME"`
            if [ $? = 0 ]
            then
                ISCLUSTER=1
                echo "$CLUSTERNAME:$RESGROUP:$DG_NAME:$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
            fi
        done
        Log "INFO:Analyse cluster info of db file($line) succ."
    done < "$DBTMPFILE"

    Log "INFO:Get powerha cluster info of db $DBNAME succ."
elif [ "$CLUSTERTYPE" = "5" ]
then
    Log "INFO:The cluster is serviceguar cluster."
    SG_CONF_DIR="/etc/cmcluster"
    CLUSTER_CONFFILE="/etc/cmcluster/cmclconf.ascii"
    CLUSTERNAME=`cat "$CLUSTER_CONFFILE"|grep -v '#'|grep '^CLUSTER_NAME'|awk '{print $2}'`
    
    Log "INFO:Begin to analyse cluster info of inst $DBINSTANCE."
    while read line
    do
        FILE_TYPE=`ls -l ${line} |awk '{print $1}'`
        FILE_TYPE=`RDsubstr $FILE_TYPE 1 1`
        if [ "$FILE_TYPE" = "c" ]
        then
            Log "INFO:The db file is on raw device,device is ($line)."
            STORAGETYPE=1
            LV_NAME=$line
            VG_NAME=`echo $line | awk -F "/" '{print $3}'`
        elif [ "$FILE_TYPE" = "-" ]
        then
            Log "INFO:The db file is on fs,fs is ($line)."
            STORAGETYPE=0
            LV_NAME=`df -i $line 2>>"$LOG_FILE_NAME"| sed -n '1p'| awk -F "(" '{print $2}' | ${MYAWK} -F ")" '{print $1}' | tr -d " "`
            if [ "$LV_NAME" = "" ]
            then
                Log "Error: df -i $line failed."
                exit $ERROR_TMP_FILE_IS_NOT_EXIST
            fi
            
            VG_NAME=`lvdisplay $LV_NAME | grep "VG Name" | awk -F "/" '{print $NF}'`
            if [ "$VG_NAME" = "" ]
            then
                Log "Error: lvdisplay $LV_NAME failed."
                exit $ERROR_TMP_FILE_IS_NOT_EXIST
            fi
        
            FS=`mount |grep -w "$LV_NAME" |awk '{print $3}'`
            if [ "$FS" = "" ]
            then
                Log "Error: FS is null."
                exit $ERROR_SCRIPT_EXEC_FAILED
            fi
        fi
        Log "INFO:vgname($VG_NAME),lvname($LV_NAME)."
        Log "INFO:Begin to analyse cluster info of db file($line)."
        GetPackageName_SG $SG_CONF_DIR
        Log "INFO:End to analyse cluster info of db file($line)."
        RESGROUP=$PKGNAME
    done < "$DBTMPFILE"
    
    Log "INFO:Get serviceguard cluster info of inst $DBINSTANCE succ."
fi

if [ "$CLUSTERTYPE" = "6" ]
then
    ClusterConfigFile=/etc/cluster/cluster.conf
    Log "INFO:Begin to get rhcs cluster info of db $DBNAME."
    FirstDbf=`QueryRHCSDBPath`
    if [ "$FirstDbf" = "" ]
    then
        Log "ERROR: query rhcs db path failed"
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    DevName=`df -i $FirstDbf 2>>"$LOG_FILE_NAME"| sed '1d' | awk '{print $1}'|sed -n '1p'`
    if [ "$DevName" = "" ]
    then
        Log "ERROR: df $FirstDbf failed"
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    MountPoint=`mount |grep -w "$DevName" |awk '{print $3}'`
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
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    DeleteFile "${RHCSMPFile}" "${RHCSMPFileTmp}"
    Log "FirstDbf=$FirstDbf, DevName=$DevName, MountPoint=$MountPoint, FsName=$FsName"
    CLUSTERNAME="rhcscluster"
    BEGIN_NUMS=`cat "$ClusterConfigFile" | grep -n '<service ' | awk -F ":" '{print $1}'`
    if [ "$BEGIN_NUMS" = "" ]
    then
        Log "ERROR:BEGIN_NUMS is empty."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    END_NUMS=`cat "$ClusterConfigFile" | grep -n '</service>' | awk -F ":" '{print $1}'`
    if [ "$END_NUMS" = "" ]
    then
        Log "ERROR:END_NUMS is empty."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    DB_NUM=`cat "$ClusterConfigFile" | grep -n 'fs ref' | grep "$FsName" | awk -F ":" '{print $1}'`
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
        sed -n "$beginNum,$endNum p" "$ClusterConfigFile" | grep '<service' >"$RHCSMPFile"
        resGroupName=`sed -n 's/ /\n/gp' "$RHCSMPFile" | sed -n '/name/p' | sed -n 's/name=//gp'`	
        Log "resGroupName=$resGroupName"
        if [ "$resGroupName" = "" ]
        then
            Log "ERROR:resGroupName is empty."
            exit $ERROR_SCRIPT_EXEC_FAILED
        fi	
        DeleteFile "${RHCSMPFile}"
        strLen=`echo $resGroupName | awk '{print length}'`
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
    ISCLUSTER=1
fi   

if [ "$CLUSTERTYPE" = "7" ]
then
    Log "Begin to get suncluster info of db $DBNAME."
    RS_LIST=`clrs list 2>>"$LOG_FILE_NAME"`
    RS_NAME=
    for RS in $RS_LIST
    do
        RS_ORACLE_SID=`clrs show -p ORACLE_SID $RS 2>>"$LOG_FILE_NAME" | $MYAWK '{if($1=="ORACLE_SID:") print $2}'`
        if [ "${RS_ORACLE_SID}" = "$DBINSTANCE" ]
        then
            RS_NAME=$RS
            break
        fi
    done
    
    Log "INFO:The db resource is $RS_NAME."
    if [ "$RS_NAME" = "" ]
    then
        ISCLUSTER=0
    else
        ISCLUSTER=1 
        RG_NAME=`clrs show -p Group $RS_NAME 2>>"$LOG_FILE_NAME" | $MYAWK '{if($1=="Group:") print $2}'`
        
        CLUSTER_NAME=""
        CLUSTER_LIST=`cluster list 2>>"$LOG_FILE_NAME"`
        for CLUSTER in $CLUSTER_LIST
        do
            RG_LIST=`cluster show $CLUSTER 2>>"$LOG_FILE_NAME" | grep "Resource Group:" | $MYAWK '{print $3}'`
            for RG in $RG_LIST
            do
                if [ "$RG" = "$RG_NAME" ]
                then
                    CLUSTER_NAME=$CLUSTER
                    break
                fi
            done
            if [ "$CLUSTER_NAME" != "" ]
            then
                break
            fi
        done
        
        Log "INFO:The resource group is $RG_NAME."

        while read line
        do
            FILESYSTEM_DEVICE=`df -k "${line}" | sed -n '2p' | $MYAWK '{print $1}'`    
            IS_VXVM=`echo $FILESYSTEM_DEVICE | grep "/dev/vx/dsk/"`
            if [ $? = 0 ]
            then
                MOUNT_POINT=`mount |${MYAWK} -v fs="$FILESYSTEM_DEVICE" '{if($3==fs) print $1}'`
                VG_LV_NAME=`RDsubstr $FILESYSTEM_DEVICE 13`
                VG_NAME=`echo $VG_LV_NAME|${MYAWK} -F "/" '{print $1}'`
                DG_NAME=$VG_NAME
                echo "$DG_NAME" >> "$DGFILE.tmp"
            else
                Log "INFO:The volume of db file($line) is not vxvm."
            fi
        done < "$DBTMPFILE"

        cat "$DGFILE.tmp"|sort|uniq > "$DGFILE"
        DG_NAME=""
        exec 4<&0
        exec <"$DGFILE"
        while read line
        do
            if [ "$DG_NAME" = "" ]
            then
                DG_NAME=$line
            else
                DG_NAME="$DG_NAME+$line"
            fi
        done 
        exec 0<&4 4<&-
        
        echo "$CLUSTER_NAME:$RG_NAME:$DG_NAME:$VGACTIVEMODE" >> "$RESULT_FILE.tmp"
        Log "$CLUSTER_NAME:$RG_NAME:$DG_NAME"
    fi
fi
if [ "$ISCLUSTER" = "0" ]
then
    Log "ERROR:The db is not in cluster."
    exit $ERROR_CLUSTER_DB_NOT_INCLUSTER
fi

cat "$RESULT_FILE.tmp"|sort|uniq > "$RESULT_FILE"
Log "INFO:Cluster INFO of $DBNAME: $CLUSTERNAME:$RESGROUP"
DeleteFile "$ArgFile" "$DBTMPFILE" "$RESULT_FILE.tmp"

exit 0
