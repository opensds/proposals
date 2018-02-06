#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################


find ${AGENT_ROOT} -name "*.gcno" | xargs rm -f
find ${AGENT_ROOT} -name "*.gcda" | xargs rm -f
find ${AGENT_ROOT} -name "*.o" | xargs rm -f
find ${AGENT_ROOT} -name "*.d" | xargs rm -f
find ${AGENT_ROOT} -name "*.txt" | xargs rm -f

rm -rf ${AGENT_ROOT}/open_src/fcgi
rm -rf ${AGENT_ROOT}/open_src/openssl
rm -rf ${AGENT_ROOT}/open_src/jsoncpp
rm -rf ${AGENT_ROOT}/open_src/snmp++
rm -rf ${AGENT_ROOT}/open_src/nginx
rm -rf ${AGENT_ROOT}/open_src/nginx_tmp
rm -rf ${AGENT_ROOT}/open_src/sqlite
rm -rf ${AGENT_ROOT}/open_src/tinyxml
rm -rf ${AGENT_ROOT}/open_src/objs_ngx