#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

AGENT_ROOT_PATH=$1
NGINX_LOG_PATH=$AGENT_ROOT_PATH/bin/nginx/logs
NGINX_EXE_PATH=$AGENT_ROOT_PATH/bin/nginx/
LOG_FILE_NAME="${AGENT_ROOT_PATH}/log/rotatenginxlog.log"
. "${AGENT_ROOT_PATH}/bin/agent_func.sh"

Log "Begin to rotate nginx log."
if [ -f "${NGINX_LOG_PATH}/error.log.1.gz" ]
then
    rm -f "${NGINX_LOG_PATH}/error.log.1.gz"
fi

mv -f "${NGINX_LOG_PATH}/error.log" "${AGENT_ROOT_PATH}/tmp/error.log"
cd $NGINX_EXE_PATH
./rdnginx -s reopen >>"$AGENT_ROOT_PATH/log/monitor.log"

Log "Compress nginx log."
gzip -f -q -9 "${AGENT_ROOT_PATH}/tmp/error.log"
mv -f "${AGENT_ROOT_PATH}/tmp/error.log.gz"  "${NGINX_LOG_PATH}/error.log.1.gz"

chmod 440 "${NGINX_LOG_PATH}/error.log.1.gz"

rm -f "${AGENT_ROOT_PATH}/tmp/error.log"
Log "Finish rotating nginx log."
exit 0
