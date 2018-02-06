#!/bin/sh

#path of all
export AGENT_ROOT=/home/icp/Agent/RD_V200R001C10_Agent_StaticCheck/code/current/Agent
export LOG_DIR=/opt/codeDex/log
export CODEDEX_DIR=/opt/codeDex/tmp
export FORTIFY_PRO_DIR=/opt/codeDex/fortify
export PATH=/home/icp/plugins/CodeDEX/tool:/home/icp/plugins/CodeDEX/tool/fortify/bin:$PATH


if [ ! -d ${CODEDEX_DIR} ] ;
then
   mkdir -p ${CODEDEX_DIR}
#else
#   rm -rf ${CODEDEX_DIR}/*
fi

if [ ! -d ${FORTIFY_PRO_DIR} ] ;
then
   mkdir -p ${FORTIFY_PRO_DIR}
else
   rm -rf ${FORTIFY_PRO_DIR}/*
fi


cd ${FORTIFY_PRO_DIR}
sh ${AGENT_ROOT}/build/agent_make.sh clean all
sh ${AGENT_ROOT}/build/agent_make.sh fortify 1>${LOG_DIR}/fortify.log 2> ${LOG_DIR}/fortify.err

#cd ${CODEDEX_DIR}

7za a -tzip fortify.zip * -r
mv fortify.zip ${CODEDEX_DIR}/
cd ${CODEDEX_DIR}/
#rm -rf Fortify
#mv Fortify /opt/codeDex/
