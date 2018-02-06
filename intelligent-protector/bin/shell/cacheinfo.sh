#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

AGENT_ROOT_PATH=$1
PID=$2
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/cacheinfo.log"

#load the agent function library script
NeedLogFlg=1
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

DB_LOCALINSTANCE=""
DB_VERSION=""
DB_STATUS=""
DB_VERSION_TMP1=""
DB_VERSION_TMP2=""

DB_STATUS_TMP=""


#############set tmpfile name#############
DB_TEMP_FILE="$TMP_PATH/TPF${PID}"

Log "INFO:Begin to get cache instance info."

ccontrol qlist > "$DB_TEMP_FILE"
if [ "$?" != "0" ]
then
    Log "ERROR:exec ccontrol failed."
    exit $ERROR_SCRIPT_EXEC_FAILED   
fi

while read line
do
    DB_STATUS_TMP=`echo $line | ${MYAWK} -F"\^" '{print $4}' | ${MYAWK} -F"," '{print $1}'`
    if [ x"$DB_STATUS_TMP" = x"running" ]
    then
        DB_STATUS=0
    else
        DB_STATUS=1
    fi
	
    DB_VERSION_TMP1=`echo $line | ${MYAWK} -F"\^" '{print $3}'`
    DB_VERSION_TMP2=`echo $line | ${MYAWK} -F"\^" '{print $2}'`
	if [ $DB_VERSION_TMP1 != $DB_VERSION_TMP2 ]
    then
        DB_VERSION=`echo $line | ${MYAWK} -F"\^" '{print $3}'`
	else
		DB_VERSION="---"
    fi

    DB_LOCALINSTANCE_TMP=`echo $line | ${MYAWK} -F"\^" '{print $1}'`
    DB_LOCALINSTANCE_FLAG=`RDsubstr $DB_LOCALINSTANCE_TMP 1 1`
    if [ x"$DB_LOCALINSTANCE_FLAG" = x">" ]
    then
        DB_LOCALINSTANCE=`RDsubstr $DB_LOCALINSTANCE_TMP 2`
    else
        DB_LOCALINSTANCE=$DB_LOCALINSTANCE_TMP
    fi
    Log "INFO:$DB_LOCALINSTANCE's status is $DB_STATUS"
    echo $DB_LOCALINSTANCE":"$DB_VERSION":"$DB_STATUS >> "$RESULT_FILE"
done < "$DB_TEMP_FILE"

rm -rf "$DB_TEMP_FILE"

Log "INFO:End to get cache instance info."

exit 0
