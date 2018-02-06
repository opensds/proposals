#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

#@dest:  application agent for cache
#@date:  2016-02-19
#@authr: 

AGENT_ROOT_PATH=$1
PID=$2
NeedLogFlg=1

LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/cachesample.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"
. "${AGENT_ROOT_PATH}/bin/cachefunc.sh"

INST_NAME=""

INST_NAMES=""
INSFLG=1

ArgFile="$TMP_PATH/ARG$PID"
TMP_FILE="$TMP_PATH/TMP$PID"
PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${PID}"
INPUTINFO=`cat "$PARAM_FILE"`
DeleteFile "$PARAM_FILE"

#############################################################
GetValue "$INPUTINFO" INSTNAME
INST_NAME=$ArgValue

Log "INFO:INSTNAME=$INST_NAME"

IsInstanceCorrect ${INST_NAME}

INST_STATUS=`ccontrol view $INST_NAME | sed -n '6p' | ${MYAWK} -F ":    " '{print $2}' | ${MYAWK} -F "," '{print $1}'`
if [ $? != 0 ]
then
    Log "ERROR:get instance($INST_STATUS) status failed."
    exit $ERROR_SCRIPT_EXEC_FAILED
fi
if [ x"$INST_STATUS" != x"running" ]
then
    Log "ERROR:Instance $INST_NAME is not start."
    exit $ERROR_INSTANCE_NOSTART
fi

Log "INFO:Test instance $INST_NAME success."
Log "INFO:End to test instance."
exit 0
