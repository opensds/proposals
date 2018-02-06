#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

export AGENT_ROOT=${HOME}/Agent
export PATH=.:${PATH} #:${AGENT_ROOT}/bin
if [ -z ${LD_LIBRARY_PATH} ]
then
    export LD_LIBRARY_PATH=${AGENT_ROOT}/bin
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGENT_ROOT}/bin
fi

export TERM=vt100

PS1="\h [\u]:\w # "
alias h="history"
alias ll="ls -l"
alias lsa="ls -al"
alias dir="ls -lF"

alias mk="sh ${AGENT_ROOT}/test/build/test_make.sh $*"

chmod 0755 ${AGENT_ROOT}/test/build/*.sh

