#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

AGENT_ROOT_PATH=$1
ID=$2

AGENT_LOG_PATH="${AGENT_ROOT_PATH}/log"
AGENT_BIN_PATH="${AGENT_ROOT_PATH}/bin"
NGINX_LOG_PATH="${AGENT_BIN_PATH}/nginx/logs"
AGENT_TMP_PATH="${AGENT_ROOT_PATH}/tmp"
INPUT_TMP_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${ID}"
LOG_FILE_NAME="${AGENT_LOG_PATH}/packlog.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

Log "Begin to package log."
if [ ! -f "$INPUT_TMP_FILE" ]
then
    Log "INPUT_TMP_FILE is not exists."
    exit 1
fi

LOG_FOLDER=`cat $INPUT_TMP_FILE`
PACKAGE_LOG=${AGENT_TMP_PATH}/${LOG_FOLDER}
rm -rf "${INPUT_TMP_FILE}"

mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}"
mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/nginx_log"
mkdir "${AGENT_TMP_PATH}/${LOG_FOLDER}/agent_log"

if [ -d "${NGINX_LOG_PATH}" ]
then
    cp -r "${NGINX_LOG_PATH}"/*.log*  "${AGENT_TMP_PATH}/${LOG_FOLDER}/nginx_log"
    cp -r "${NGINX_LOG_PATH}"/*.pid   "${AGENT_TMP_PATH}/${LOG_FOLDER}/nginx_log"
fi

if [ -d "${AGENT_LOG_PATH}" ]
then
    cp -r "${AGENT_LOG_PATH}"/*.log*  "${AGENT_TMP_PATH}/${LOG_FOLDER}/agent_log"
    cp -r "${AGENT_LOG_PATH}"/*.pid  "${AGENT_TMP_PATH}/${LOG_FOLDER}/agent_log"
fi

Log "Compress agent log."
cd "${AGENT_TMP_PATH}"
tar cvf "${PACKAGE_LOG}.tar"  "${LOG_FOLDER}" > /dev/null
gzip "${PACKAGE_LOG}.tar" > /dev/null

rm -rf "${AGENT_TMP_PATH}/${LOG_FOLDER}"
Log "Finish packaging log."
exit 0
