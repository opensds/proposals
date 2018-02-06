#!/bin/sh
OSTYPE=`uname`
MYAWK=
if [ "${OSTYPE}" = "SunOS" ]
then
    MYAWK=nawk
    ICP_ROOT=/export/home/icp/Agent
else
    MYAWK=awk
    ICP_ROOT=/home/icp/Agent
fi
AGENT_ROOT=${ICP_ROOT}/RD_V200R001C00_Agent_Codebin/code/current/Agent
export ICP_ROOT AGENT_ROOT


SVN_NUM=`cat ${ICP_ROOT}/RD_V200R001C00_Agent_Codebin/code/current/scmInfo.xml | grep svnVersion | ${MYAWK} '{print $3}' | ${MYAWK} -F "\"" '{print $2}'`
echo ${SVN_NUM} > "${AGENT_ROOT}/conf/svn"
