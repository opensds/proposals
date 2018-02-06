#!/bin/sh

export AGENT_ROOT=/home/icp/Agent/RD_V200R001C00_Agent/code/current/Agent
FORTIFY_DIR=/home/icp/Agent/RD_V200R001C00_Agent/code/current/AGENT_PACK_TEMP

if [ ! -d ${FORTIFY_DIR} ]
then
   mkdir -p ${FORTIFY_DIR}
else
   rm -rf ${FORTIFY_DIR}/*
fi
cd ${FORTIFY_DIR}
sh ${AGENT_ROOT}/build/agent_make.sh clean all
sh ${AGENT_ROOT}/build/agent_make.sh fortify 1> /home/icp/Agent/RD_V200R001C00_Agent/fortify.log 2> /home/icp/Agent/RD_V200R001C00_Agent/fortify.log

cd ${FORTIFY_DIR}/Fortify/

7za a -tzip fortify.zip * -r
mv fortify.zip ${FORTIFY_DIR}/
cd ${FORTIFY_DIR}/
rm -rf Fortify
