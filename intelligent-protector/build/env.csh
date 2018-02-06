#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

setenv AGENT_ROOT ${HOME}/Agent
setenv PATH .:${PATH}:${AGENT_ROOT}/bin
if ($?LD_LIBRARY_PATH) then
    setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${AGENT_ROOT}/bin
else
    setenv LD_LIBRARY_PATH ${AGENT_ROOT}/bin
endif

stty erase ^H
stty intr "^C"
setenv TERM vt100
set prompt="<\! `hostname` [$LOGNAME] :$cwd>"
set filec
alias cd 'cd \!*; set prompt="<\! `hostname` [$LOGNAME] :$cwd>"'
alias h history
alias ll "ls -l"
alias lsa "ls -al"
alias dir ls -lF

alias mk "sh ${AGENT_ROOT}/build/agent_make.sh $*"
alias pk "sh ${AGENT_ROOT}/build/agent_pack.sh $*"
alias ppp "ps -ef | grep -v grep | grep rdagent | grep ${LOGNAME}; ps -ef | grep -v grep | grep nginx | grep ${LOGNAME}"
alias pp "ps -ef | grep rdagent; ps -ef | grep nginx"
alias bin "cd ${AGENT_ROOT}/bin"
alias conf "cd ${AGENT_ROOT}/conf"
alias obj "cd ${AGENT_ROOT}/obj"
alias src "cd ${AGENT_ROOT}/src/src"
alias inc "cd ${AGENT_ROOT}/src/inc"
alias 3src "cd ${AGENT_ROOT}/open_src"
alias test "cd ${AGENT_ROOT}/test"
alias build "cd ${AGENT_ROOT}/build"
alias log "cd ${AGENT_ROOT}/log"
alias tmp "cd ${AGENT_ROOT}/tmp"
alias stop "${AGENT_ROOT}/bin/agent_stop.sh"
alias start "${AGENT_ROOT}/bin/agent_start.sh"

chmod 0755 ${AGENT_ROOT}/build/*.sh

