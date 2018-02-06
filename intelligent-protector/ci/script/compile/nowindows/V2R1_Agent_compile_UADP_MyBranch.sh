#! /bin/sh
OSTYPE=`uname`
if [ "${OSTYPE}" = "SunOS" ]
then
    ICP_ROOT=/export/home/icp/Agent
else
    ICP_ROOT=/home/icp/Agent
fi
cd ../../../../
AGENT_ROOT=`pwd`
#AGENT_ROOT=${ICP_ROOT}/UADP_RD_Agent_MyBranch/code/current/Agent

export AGENT_ROOT ICP_ROOT

if [ "${OSTYPE}" = "SunOS" ]
then
    sed 's/^AGENT_PACK=${HOME}\/AGENT_PACK_TEMP/AGENT_PACK=\/export\/home\/icp\/Agent\/UADP_RD_Agent_MyBranch\/code\/current\/AGENT_PACK_TEMP/g' ${AGENT_ROOT}/build/agent_pack.sh > Agent_pack_tmp
    rm -rf "${AGENT_ROOT}/build/agent_pack.sh"
    mv Agent_pack_tmp "${AGENT_ROOT}/build/agent_pack.sh"
elif [ "${OSTYPE}" = "AIX" ]
then
    sed 's/^AGENT_PACK=${HOME}\/AGENT_PACK_TEMP/AGENT_PACK=\/home\/icp\/Agent\/UADP_RD_Agent_MyBranch\/code\/current\/AGENT_PACK_TEMP/g' ${AGENT_ROOT}/build/agent_pack.sh > Agent_pack_tmp
    mv Agent_pack_tmp "${AGENT_ROOT}/build/agent_pack.sh"
elif [ "${OSTYPE}" = "HP-UX" ]
then
    sed 's/^AGENT_PACK=${HOME}\/AGENT_PACK_TEMP/AGENT_PACK=\/home\/icp\/Agent\/UADP_RD_Agent_MyBranch\/code\/current\/AGENT_PACK_TEMP/g' ${AGENT_ROOT}/build/agent_pack.sh > Agent_pack_tmp
    mv Agent_pack_tmp "${AGENT_ROOT}/build/agent_pack.sh"
else
    sed -i 's/^AGENT_PACK=${HOME}\/AGENT_PACK_TEMP/AGENT_PACK=\/home\/icp\/Agent\/UADP_RD_Agent_MyBranch\/code\/current\/AGENT_PACK_TEMP/g' ${AGENT_ROOT}/build/agent_pack.sh
fi

chmod +x "${AGENT_ROOT}/build/agent_pack.sh"
chmod +x "${AGENT_ROOT}/build/agent_make.sh"

if [ "${OSTYPE}" = "SunOS" ]
then
    cd ${AGENT_ROOT}/build
    bash ${AGENT_ROOT}/build/agent_pack.sh > ${ICP_ROOT}/compile.log
else
    ${AGENT_ROOT}/build/agent_pack.sh > ${ICP_ROOT}/compile.log
fi

