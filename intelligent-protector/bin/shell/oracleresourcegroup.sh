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

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/oracleresourcegroup.log"
DBUser=""
GridUser=""
#load the agent function library script
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/oraclefunc.sh"

ArgFile=$TMP_PATH"/ARG"$PID
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${PID}"

#get oracle user name
getOracleUserInfo
if [ $? -ne 0 ]
then
    Log "Get Oracle user info failed."
    exit 1
fi

INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"
ACTIVE_NODE=""
##################start to get database path########################
Log "INFO:Begin to get param info."

GetValue "$INPUTINFO" RESGRPNAME
RESGRP_NAME=$ArgValue

GetValue "$INPUTINFO" DEVGRPNAME
DEVGRP_NAME=$ArgValue

GetValue "$INPUTINFO" CLUSTERTYPE
CLUSTER_TYPE=$ArgValue

GetValue "$INPUTINFO" OPERTYPE
OPER_TYPE=$ArgValue

Log "RESGRP_NAME=$RESGRP_NAME;CLUSTER_TYPE=$CLUSTER_TYPE;OPER_TYPE=$OPER_TYPE"
# *****************************set envionment variable ************************************




if [ 4 -eq ${CLUSTER_TYPE} ]  #HACMP 
then
    Log "INFO:The cluster is powerHA cluster."
    Log "INFO:Begin to get active node."
    ACTIVE_NODE=`/usr/es/sbin/cluster/utilities/clfindres ${RESGRP_NAME} | grep -n "ONLINE" | awk '{print $3}'`
    Log "INFO:Get active node($ACTIVE_NODE) succ."
    
    if [ "${OPER_TYPE}" = "0" ] 
    then
        Log "INFO:Begin to offline resgrp($RESGRP_NAME)."
        if [ "" = "${ACTIVE_NODE}" ]
        then
            Log "INFO:resgrp($RESGRP_NAME) has been offline."
            exit 0
        else
            tmp_result=`/usr/es/sbin/cluster/utilities/clRGmove -s 'false'  -d -i -g ${RESGRP_NAME} -n ${ACTIVE_NODE} 2>> "$LOG_FILE_NAME"`
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
            LOCAL_HOST=`hostname`
            Log "The node ${LOCAL_HOST} will be Online."
            tmp_result=`/usr/es/sbin/cluster/utilities/clRGmove -s 'false'  -u -i -g ${RESGRP_NAME} -n ${LOCAL_HOST} 2>> "$LOG_FILE_NAME"`
            if [ $? != 0 ]
            then
                Log "ERROR:exec cmd of online resgrp($RESGRP_NAME) failed."
                exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED
            else
                Log "INFO:exec cmd of online resgrp($RESGRP_NAME) succ."
                ACTIVE_NODE=`/usr/es/sbin/cluster/utilities/clfindres ${RESGRP_NAME} | grep -n "ONLINE" | awk '{print $NF}'`
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
    ACTIVE_NODE=`cmviewcl -p ${RESGRP_NAME} | grep "${RESGRP_NAME}" | awk '{print $NF}'`
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
                STATE_CHECK=`cmviewcl -p ${RESGRP_NAME} | grep "${RESGRP_NAME}" | awk '{print $NF}'`
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
                ACTIVE_NODE=`cmviewcl -p ${RESGRP_NAME} | grep "${RESGRP_NAME}" | awk '{print $NF}'`
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
        Log "INFO:Begin to online resgrp($RESGRP_NAME)." 
        tmp_result=`/usr/sbin/clusvcadm -d ${RESGRP_NAME} > /dev/null`
        if [ $? != 0 ]
        then
            Log "ERROR:exec cmd of clusvcadm -d resgrp($RESGRP_NAME) failed."
        else
            Log "INFO:exec cmd of clusvcadm -d resgrp($RESGRP_NAME) success." 
        fi
        tmp_result=`/usr/sbin/clusvcadm -s ${RESGRP_NAME} > /dev/null`
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
        tmp_result=`/usr/sbin/clusvcadm -d ${RESGRP_NAME} > /dev/null`
        if [ $? != 0 ]
        then
            Log "ERROR:exec cmd of clusvcadm -d resgrp($RESGRP_NAME) failed."
        else
            Log "INFO:exec cmd of clusvcadm -d resgrp($RESGRP_NAME) success." 
        fi
        tmp_result=`/usr/sbin/clusvcadm -e ${RESGRP_NAME} > /dev/null`
        if [ $? != 0 ]
        then
            Log "ERROR:exec cmd of clusvcadm -e resgrp($RESGRP_NAME) failed."
            exit $ERROR_CLUSTER_RESOURCE_ONLINE_FAILED 
        else
            Log "INFO:exec cmd online resgrp($RESGRP_NAME) success."
            exit 0
        fi
    fi
elif [ 7 -eq ${CLUSTER_TYPE} ]  #SunCluster
then
    DG_LIST=`echo ${DEVGRP_NAME} | $MYAWK -F "+" '{for(x=1;x<=NF;x++) print $x}'`
    Log "INFO:The cluster is SunCluster."
    if [ "${OPER_TYPE}" = "0" ] #offline resource group
    then
        Log "INFO:Begin to offline resgrp($RESGRP_NAME)." 
        tmp_result=`clrg offline ${RESGRP_NAME} 2>>"$LOG_FILE_NAME"`
        if [ $? != 0 ]
        then
            Log "ERROR:exec cmd of offline resgrp($RESGRP_NAME) failed."
        else
            Log "INFO:exec cmd of offline resgrp($RESGRP_NAME) successful." 
        fi
        for DG in $DG_LIST
        do
            tmp_result=`cldg offline ${DG} 2>>"$LOG_FILE_NAME"`
            if [ $? != 0 ]
            then
                Log "ERROR:exec offline dg($DG) failed."
                exit $ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED 
            else
                Log "INFO:exec offline dg($DG) successful."
                exit 0
            fi
        done

    elif [ "${OPER_TYPE}" = "1" ] #online resource group
    then 
        Log "INFO:Begin to online resgrp($RESGRP_NAME)." 
        RG_STATUS=`clrg status $RESGRP_NAME 2>> "$LOG_FILE_NAME" | nawk -v rg="${RESGRP_NAME}" '{if($1==rg) print $NF}'`
        tmp_result=`scswitch -Z -g ${RESGRP_NAME} 2>>"$LOG_FILE_NAME"`
        if [ $? != 0 ]
        then
            Log "ERROR:exec cmd of online resgrp($RESGRP_NAME) failed."
        else
            Log "INFO:exec cmd of online resgrp($RESGRP_NAME) successful." 
        fi    
    fi
else
    Log "ERROR:The cluster type($CLUSTER_TYPE) is not be surported."
    exit $ERROR_PARAM_WRONG
fi
exit 0