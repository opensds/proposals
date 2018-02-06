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

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/db2resourcegroup.log"
#load the agent function library script
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

ArgFile="$TMP_PATH/ARG$ID"
RESULT_FILE="${AGENT_ROOT_PATH}/tmp/${RESULT_TMP_FILE_PREFIX}${ID}"
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${ID}"
RESFILE="${TMP_PATH}/resource$ID.txt"

INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"
ACTIVE_NODE=""
INST_MOUNTRES=""
FAIL_NUM=0
##################start to get database path########################
Log "INFO:Begin to get param info."

GetValue "$INPUTINFO" RESGRPNAME
RESGRP_NAME=$ArgValue

GetValue "$INPUTINFO" CLUSTERTYPE
CLUSTER_TYPE=$ArgValue

GetValue "$INPUTINFO" OPERTYPE
OPER_TYPE=$ArgValue

Log "RESGRP_NAME=$RESGRP_NAME;CLUSTER_TYPE=$CLUSTER_TYPE;OPER_TYPE=$OPER_TYPE"
# *****************************set envionment variable ************************************


#******************************
GetResList()
{
    RES="$1"
    echo "$RES">>"$RESFILE"
    while [ "$RES" != "" ]
    do
        COUNT=0
        ALL_CHILD_RES=""
        for SUB_RES in $RES
        do
            RES_NUM=`echo $RES|$MYAWK '{print NF}'`
            COUNT=`expr $COUNT + 1`

            FAULTED_NODE_LIST=`hares -state $SUB_RES|grep "FAULTED"|$MYAWK '{if ($2=="State") print $3}'`
            if [ "${FAULTED_NODE_LIST}" != "" ]
            then
                for FAULTED_NODE in $FAULTED_NODE_LIST
                do
                    Log "ERROR:$SUB_RES on $FAULTED_NODE is faulted."
                    hares -clear $SUB_RES -sys $FAULTED_NODE
                    if [ $? != 0 ]
                    then
                        Log "ERROR:Clear $SUB_RES on $FAULTED_NODE failed."
                    fi
                    Log "INFO:Clear $SUB_RES on $FAULTED_NODE succ."
                done
            fi
			
            CHILD_RES=`hares -dep $SUB_RES|$MYAWK -v sub_res="$SUB_RES" '{if ($2==sub_res) print $3}'`
            if [ "$CHILD_RES" != "" ]
            then
                INST_MOUNTRES_CHECK=`echo $CHILD_RES|grep $INST_MOUNTRES`
                if [ $? = 0 ]
                then
                    Log "INFO:mount point of instance dir is a mount res($INST_MOUNTRES)."
                    CHILD_RES=`echo $CHILD_RES|sed "s/$INST_MOUNTRES//"`
                fi

                if [ "$ALL_CHILD_RES" = "" ]
                then
                    ALL_CHILD_RES="$CHILD_RES"
                else
                    ALL_CHILD_RES="$ALL_CHILD_RES $CHILD_RES"
                fi
            fi
            if [ $COUNT -eq $RES_NUM ]
            then
                if [ "$ALL_CHILD_RES" != "" ]
                then
                    RES=`echo $ALL_CHILD_RES|sed 's/ /\n/g'|sort|uniq`
                    for SUB_CHILD_RES in $RES
                    do
                        RES_TYPE=`hares -value $SUB_CHILD_RES Type`
                        if [ "$RES_TYPE" != "NIC" ]
                        then
                            echo -e "$SUB_CHILD_RES \c">>"$RESFILE"
                        fi
                    done
                    echo "">>"$RESFILE"
                else
                    RES=""
                    break
                fi
            fi
        done
    done
}

GetActiveNode()
{
    while read line
    do
        RES=`echo $line | $MYAWK '{for(x=1;x<=NF;x++) print $x}'`
        for SUB_RES in $RES
        do
            ACTIVE_NODE=`hares -state $SUB_RES|$MYAWK '{if ($4=="ONLINE") print $3}'`
            if [ "$ACTIVE_NODE" != "" ]
            then
                Log "INFO:Active Node is $ACTIVE_NODE."
                break
            fi
        done
        if [ "$ACTIVE_NODE" != "" ]
        then
            break
        fi
    done < "$RESFILE"
}

FaultedCheck()
{
    NODE=$1
    while read line
    do
        RES=`echo $line|$MYAWK '{for(x=1;x<=NF;x++) print $x}'`
        for SUB_VCS_RES in $RES
        do
            RES_STATE=`hares -state $SUB_VCS_RES -sys $NODE`
            if [ "$RES_STATE" = "FAULTED" ]
            then
                Log "ERROR:$SUB_VCS_RES is faulted on $NODE"
                exit $ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL
            fi
        done
    done < "$RESFILE"
}

OnlineResGrp()
{
    while read line
    do
        RES=`echo $line|$MYAWK '{for(x=1;x<=NF;x++) print $x}'`
        for SUB_RES in $RES
        do
            IS_ONLINE_CHECK=`hares -state $SUB_RES|$MYAWK '{if ($4=="ONLINE") print $3}'`
            if [ "${IS_ONLINE_CHECK}" = "" ]
            then
                if [ "$ACTIVE_NODE" = "" ]
                then
                    for SUB_NODE in $NODE_LIST
                    do
                        hares -online $SUB_RES -sys $SUB_NODE
                        if [ $? != 0 ]
                        then
                            FAIL_NUM=`expr $FAIL_NUM + 1`
                            Log "ERROR:$SUB_RES online failed on $SUB_NODE"
                            continue
                        fi 
                        TSTART=$SECONDS
                        NUM_TEMP=0
                        while [ $NUM_TEMP -lt 1800 ]
                        do
                            IS_ONLINE_CHECK=`hares -state $SUB_RES|$MYAWK '{if ($4=="ONLINE") print $3}'`
                            if [ "${IS_ONLINE_CHECK}" != "" ]
                            then
                                Log "INFO:$SUB_RES is online on $SUB_NODE."
                                ACTIVE_NODE="$SUB_NODE"
                                break
                            fi
                            FaultedCheck $SUB_NODE
                            NUM_TEMP=`expr $SECONDS - $TSTART`
                        done
                        if [ $NUM_TEMP -ge 1800 ]
                        then
                            Log "ERROR:$SUB_RES online over time or failed on $SUB_NODE"
                            exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED 
                        fi        
                        if [ "$ACTIVE_NODE" != "" ]
                        then
                            break
                        fi                   
                    done
                    if [ $FAIL_NUM = 2 ]
                    then
                        Log "ERROR:$SUB_RES online failed."
                        FAIL_NUM=0
                        exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED
                    fi
                else
                    hares -online $SUB_RES -sys $ACTIVE_NODE
                    if [ $? != 0 ]
                    then
                        Log "ERROR:$SUB_RES online failed on $ACTIVE_NODE."
                        exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED 
                    fi
                    TSTART=$SECONDS
                    NUM_TEMP=0
                    while [ $NUM_TEMP -lt 1800 ]                
                    do
                        IS_ONLINE_CHECK=`hares -state $SUB_RES|$MYAWK '{if ($4=="ONLINE") print $3}'`
                        if [ "${IS_ONLINE_CHECK}" != "" ]
                        then
                            Log "INFO:$SUB_RES is online on $ACTIVE_NODE."
                            break
                        fi
                        FaultedCheck $ACTIVE_NODE
                        NUM_TEMP=`expr $SECONDS - $TSTART`
                    done 
                    if [ $NUM_TEMP -ge 1800 ]
                    then
                        Log "ERROR:$SUB_RES online over time or failed on $ACTIVE_NODE."
                        exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED
                    fi
                fi       
            else
                Log "INFO:$SUB_RES is online on $IS_ONLINE_CHECK."
            fi
        done 
    done <"$RESFILE"
}


OfflineResGrp()
{
    while read line
    do
        RES=`echo $line|$MYAWK '{for(x=1;x<=NF;x++) print $x}'`
        for SUB_RES in $RES
        do
            RES_STATE_CHECK=`hares -state $SUB_RES -sys $ACTIVE_NODE|grep 'ONLINE'`
            if [ "$RES_STATE_CHECK" != "" ]
            then
                RES_TYPE=`hares -value $SUB_RES Type`
                if [ "$RES_TYPE" = "IP" -o "$RES_TYPE" = "NIC" ]
                then
                    continue
                fi
                hares -offline $SUB_RES -sys $ACTIVE_NODE
                if [ $? != 0 ]
                then
                    Log "ERROR:$SUB_RES offline failed on $ACTIVE_NODE"
                    exit $ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED 
                fi 
                TSTART=$SECONDS
                NUM_TEMP=0
                while [ $NUM_TEMP -lt 1800 ]
                do
                    RES_STATE=`hares -state $SUB_RES -sys $ACTIVE_NODE`
                    if [ "$RES_STATE" = "FAULTED" ]
                    then
                        Log "$SUB_RES is faulted on $ACTIVE_NODE."
                        exit $ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL 
                    fi 
                    if [ "$RES_STATE" = "OFFLINE" ]
                    then
                        Log "INFO:$SUB_RES is offline."
                        break
                    fi
                    NUM_TEMP=`expr $SECONDS - $TSTART`
                done
                if [ $NUM_TEMP -ge 1800 ]
                then
                    Log "INFO:$SUB_RES offline over time or failed."
                    exit $ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED 
                fi                
            fi
        done
    done < "$RESFILE"   
}

if [ 2 -eq ${CLUSTER_TYPE} ]  
then
    Log "INFO:The cluster is vcs cluster"
    DB_RES=$RESGRP_NAME
	
    Log "INFO:Begin to get node list of resgrp($RESGRP_NAME)."
    RESGRP_NAME=`hares -value $DB_RES Group 2>>"$LOG_FILE_NAME"`
    if [ $? != 0 ]
    then
        Log "ERROR:Get resgrp name of dbres($DB_RES) failed."
        exit $ERROR_SCRIPT_EXEC_FAILED
    fi
    NODE_LIST=`hagrp -list|grep "$RESGRP_NAME"|$MYAWK '{print $NF}'`
    Log "INFO:$NODE_LIST"
    Log "INFO:Get node list of resgrp($RESGRP_NAME) succ."
	
    Log "INFO:Begin to get resource tree of db res($DB_RES)."
    GetResList $DB_RES
    cat "$RESFILE">>"$LOG_FILE_NAME"
    Log "INFO:Get resource tree succ of db res($DB_RES) succ."
	
    Log "INFO:Begin to get active node."
    GetActiveNode
    Log "INFO:Get active node($ACTIVE_NODE) succ."

    if [ "$OPER_TYPE" = "0" ]
    then
        Log "INFO:Begin to get mount res of instance."
        DB2INST_HOME=`hares -value $DBRES DB2InstHome`
        MOUNTRES_LIST_TEMP=`hatype -resources Mount`
        for SUB_MOUNTRES in $MOUNTRES_LIST_TEMP
        do
            RESOURCE_GROUP=`hares -value $SUB_MOUNTRES Group`
            if [ "$RESOURCE_GROUP" = "$RESGRP_NAME" ]
            then
                echo "$SUB_MOUNTRES">>"$MOUNTRESFILE" 
            fi
        done 
	
        NUM=0
        while read SUB_MOUNTRES 
        do
            SUB_MOUNTRES_DIR=`hares -value $SUB_MOUNTRES MountPoint`
            INSTDIR_CHECK=`echo $DB2INST_HOME|grep $SUB_MOUNTRES_DIR`
            if [ $? = 0 ]
            then
                LENGTH=`expr ${#DB2INST_HOME} - ${#SUB_MOUNTRES_DIR}`
                if [ $NUM = 0 ]
                then
                    LENGTH_TMP=$LENGTH
                    INST_MOUNTRES="$SUB_MOUNTRES"
                fi

                if [ $LENGTH_TMP -gt $LENGTH ]
                then
                    INST_MOUNTRES="$SUB_MOUNTRES"
                    LENGTH_TMP=$LENGTH
                fi
                NUM=`expr $NUM + 1`
            fi
        done < "$MOUNTRESFILE"
        rm -fr "$MOUNTRESFILE" 
        Log "INFO:Get mount res($INST_MOUNTRES) of instance succ."
		
        Log "INFO :Begin to offline db res($DB_RES)."
        OfflineResGrp
        rm -fr "$RESFILE"
        Log "INFO:Successful to offline db res($DB_RES)." 
        exit 0
    else
        Log "INFO:Begin to online db res($DB_RES)."
        OnlineResGrp
        Log "INFO:Successful to online db res($DB_RES)." 
        rm -fr "$RESFILE" 
    fi
elif [ 4 -eq ${CLUSTER_TYPE} ] 
then
    Log "INFO:The cluster is powerHA cluster."
    Log "INFO:Begin to get active node."
    ACTIVE_NODE=`/usr/es/sbin/cluster/utilities/clfindres ${RESGRP_NAME} | grep -n "ONLINE" | $MYAWK '{print $3}'`
    Log "INFO:Get active node($ACTIVE_NODE) succ."
	
    if [ "${OPER_TYPE}" = "0" ] 
    then
        Log "INFO:Begin to offline resgrp($RESGRP_NAME)."
        if [ "" = "${ACTIVE_NODE}" ]
        then
            Log "INFO:resgrp($RESGRP_NAME) has been offline."
            exit 0
        else
            tmp_result=`/usr/es/sbin/cluster/utilities/clRGmove -s 'false'  -d -i -g ${RESGRP_NAME} -n ${ACTIVE_NODE} 2>>"$LOG_FILE_NAME"`
            if [ $? -eq 0 ]
            then
                Log "INFO:exec cmd of offline resgrp($RESGRP_NAME) succ."
                STATE_CHECK=`/usr/es/sbin/cluster/utilities/clfindres ${RESGRP_NAME} |sed -n '4,5p' |grep -v 'OFFLINE'`
                if [ "$STATE_CHECK" = "" ]
                then
                    Log "INFO:Successful to offline resgrp($RESGRP_NAME)."
                    exit 0
                else
                    Log "ERROR:Failed to offline resgrp($RESGRP_NAME)."
                    exit $ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED
                fi
            else
                Log "ERROR:exec cmd of offline resgrp($RESGRP_NAME) failed."
                Log "$tmp_result"
                exit $ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED
            fi    
        fi	
    elif [ "${OPER_TYPE}" = "1" ] 
    then
        Log "INFO:Begin to online resgrp($RESGRP_NAME)."
        if [ "" = "${ACTIVE_NODE}" ]
        then
            PRIORITY_NODE=`/usr/es/sbin/cluster/utilities/clshowres -g ${RESGRP_NAME} | grep "Node Name" | sed -n '2p' | $MYAWK '{print $3}'`
            Log "The node ${PRIORITY_NODE} will be Online."
            tmp_result=`/usr/es/sbin/cluster/utilities/clRGmove -s 'false'  -u -i -g ${RESGRP_NAME} -n ${PRIORITY_NODE} 2>>"$LOG_FILE_NAME"`
            if [ $? != 0 ]
            then
                Log "ERROR:exec cmd of online resgrp($RESGRP_NAME) failed."
                exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED
            else
                Log "INFO:exec cmd of online resgrp($RESGRP_NAME) succ."
                ACTIVE_NODE=`/usr/es/sbin/cluster/utilities/clfindres ${RESGRP_NAME} | grep -n "ONLINE" | $MYAWK '{print $NF}'`
                if [ "" = "${ACTIVE_NODE}" ]
                then
                    Log "ERROR:Failed to online resgrp($RESGRP_NAME)."
                    exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED
                else
                    Log "INFO:Successful to online resgrp($RESGRP_NAME)."
                    exit 0
                fi  
            fi
        else
            Log "INFO:The resgrp($RESGRP_NAME) has been online."
            exit 0
        fi 	 
    fi    
elif [ 5 -eq ${CLUSTER_TYPE} ] 
then
    Log "INFO:The cluster is serviceguard cluster."
    Log "INFO:Begin to get active node."
    ACTIVE_NODE=`cmviewcl -p ${RESGRP_NAME} | grep "${RESGRP_NAME}" | $MYAWK '{print $NF}'`
    Log "INFO:Get active node($ACTIVE_NODE) succ."
    if [ "${OPER_TYPE}" = "0" ] 
    then
        Log "INFO:Begin to offline package($RESGRP_NAME)."
        if [ "unowned" = "${ACTIVE_NODE}" ]
        then
            Log "INFO:resgrp($RESGRP_NAME) has been offline."
            exit 0
        else
            tmp_result=`cmhaltpkg $RESGRP_NAME 2>>"$LOG_FILE_NAME"`
            if [ $? -eq 0 ]
            then
                Log "INFO:exec cmd of offline package($RESGRP_NAME) succ."
                STATE_CHECK=`cmviewcl -p ${RESGRP_NAME} | grep "${RESGRP_NAME}" | $MYAWK '{print $NF}'`
                if [ "$STATE_CHECK" = "unowned" ]
                then
                    Log "INFO:Successful to offline package($RESGRP_NAME)."
                    exit 0
                else
                    Log "ERROR:Failed to offline package($RESGRP_NAME)."
                    exit $ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED
                fi
            else
                Log "ERROR:exec cmd of offline package($RESGRP_NAME) failed."
                Log "$tmp_result"
                exit $ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED
            fi    
        fi	
    elif [ "${OPER_TYPE}" = "1" ] 
    then
        Log "INFO:Begin to online package($RESGRP_NAME)."
        if [ "unowned" = "${ACTIVE_NODE}" ]
        then
            LOCAL_HOST=`hostname`
            Log "The node ${LOCAL_HOST} will be Online."
            tmp_result=`cmrunpkg -n $LOCAL_HOST $RESGRP_NAME 2>>"$LOG_FILE_NAME"`
            if [ $? != 0 ]
            then
                Log "ERROR:exec cmd of online package($RESGRP_NAME) failed."
                exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED
            else
                Log "INFO:exec cmd of online package($RESGRP_NAME) succ."
                ACTIVE_NODE=`cmviewcl -p ${RESGRP_NAME} | grep "${RESGRP_NAME}" | $MYAWK '{print $NF}'`
                if [ "unowned" = "${ACTIVE_NODE}" ]
                then
                    Log "ERROR:Failed to online package($RESGRP_NAME)."
                    exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED
                else
                    Log "INFO:Successful to online package($RESGRP_NAME)."
                    exit 0
                fi  
            fi
        else
            Log "INFO:The package($RESGRP_NAME) has been online."
            exit 0
        fi 	 
    fi
elif [ 6 -eq ${CLUSTER_TYPE} ]  #RHCS cluster
then
    Log "INFO:The cluster is rhcs cluster."
    if [ "${OPER_TYPE}" = "0" ] #offline resource group
    then
        Log "INFO:Begin to offline resgrp($RESGRP_NAME)."
        tmp_result=`/usr/sbin/clusvcadm -d ${RESGRP_NAME} 1>>"$LOG_FILE_NAME" 2>>"$LOG_FILE_NAME"`
        if [ $? != 0 ]
        then
            Log "exec cmd of clusvcadm -d resgrp($RESGRP_NAME) failed."
        else
            Log "INFO:exec cmd of clusvcadm -d resgrp($RESGRP_NAME) success." 
        fi
        tmp_result=`/usr/sbin/clusvcadm -s ${RESGRP_NAME} 1>>"$LOG_FILE_NAME" 2>>"$LOG_FILE_NAME"`
        if [ $? != 0 ]
        then
            Log "ERROR:exec cmd of clusvcadm -s resgrp($RESGRP_NAME) failed."
            exit $ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED 
        else
            Log "INFO:exec cmd offline resgrp($RESGRP_NAME) success."
            exit 0
        fi
    elif [ "${OPER_TYPE}" = "1" ] #online resource group
    then    
        Log "INFO:Begin to online resgrp($RESGRP_NAME)." 
        tmp_result=`/usr/sbin/clusvcadm -d ${RESGRP_NAME} 1>>"$LOG_FILE_NAME" 2>>"$LOG_FILE_NAME"`
        if [ $? != 0 ]
        then
            Log "ERROR:exec cmd of clusvcadm -d resgrp($RESGRP_NAME) failed."
        else
            Log "INFO:exec cmd of clusvcadm -d resgrp($RESGRP_NAME) success." 
        fi
        tmp_result=`/usr/sbin/clusvcadm -e ${RESGRP_NAME} 1>>"$LOG_FILE_NAME" 2>>"$LOG_FILE_NAME"`
        if [ $? != 0 ]
        then
            Log "ERROR:exec cmd of clusvcadm -e resgrp($RESGRP_NAME) failed."
            exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED 
        else
            Log "INFO:exec cmd online resgrp($RESGRP_NAME) success."
            exit 0
        fi
    fi
else
    Log "ERROR:The cluster type($CLUSTER_TYPE) is not be surported."
    exit $ERROR_PARAM_WRONG
fi
exit 0