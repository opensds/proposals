#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

# The parameters $1 and $2 is passed by Agent. So Please does not change following script.
PID=$1
AGENTROOT=$2
BINPATH=${AGENTROOT}"/bin"
TMPPATH=${AGENTROOT}"/tmp"
LOGPATH=${AGENTROOT}"/log"
RSTFILE=$TMPPATH"/RST"${PID}".txt"
LOGFILE=${LOGPATH}"/sample.log"

# The related business script code need to be here.
########Begin########
echo "Begin to do something." >> ${LOGFILE}

echo "Finish doing something." >> ${LOGFILE}
########End#######

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    echo "Here:record some error." >> ${LOGFILE}
    echo ${RSTCODE} > ${RSTFILE}
    exit 1
else
    echo "Here:record success." >> ${LOGFILE}
    echo "" > ${RSTFILE}
    exit 0
fi
