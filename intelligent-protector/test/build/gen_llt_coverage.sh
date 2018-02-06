#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################


rm -rf ${AGENT_ROOT}/test/html
#if you just wanna run your own binary file
#you shuld change run.sh to your own binary file name
sh ${AGENT_ROOT}/test/bin/run.sh 
mkdir -p ${AGENT_ROOT}/test/temp
lcov -d ${AGENT_ROOT} -o ${AGENT_ROOT}/test/temp/total.info -b ${AGENT_ROOT} -c
genhtml -o ${AGENT_ROOT}/test/html ${AGENT_ROOT}/test/temp/total.info
rm -rf ${AGENT_ROOT}/test/temp
find ${AGENT_ROOT}/test/obj -name "*.gcno" | xargs rm -f
find ${AGENT_ROOT}/test/obj -name "*.gcda" | xargs rm -f
