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

UNFREEZEFILE=$TMPPATH"/unfreezeoracle${PID}.sql"
UNFREEZEFILERST=$TMPPATH"/unfreezeoracleRST${PID}.txt"

# The related business script code need to be here.
########Begin########
echo "Begin to do unfreeze db." >> ${LOGFILE}
echo "alter database end backup;" > ${UNFREEZEFILE}
echo "exit;" >> ${UNFREEZEFILE}

touch ${UNFREEZEFILERST}
chmod 666 ${UNFREEZEFILERST}
su - oracle -c "export ORACLE_SID=dbora; sqlplus / as sysdba @${UNFREEZEFILE} >> ${UNFREEZEFILERST}"

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    echo "Close hot back up failed." >> ${LOGFILE}
    [ -f ${UNFREEZEFILERST} ] && rm -f ${UNFREEZEFILERST}
    [ -f ${UNFREEZEFILE} ] && rm -f ${UNFREEZEFILE}
    exit 1
fi

cat ${UNFREEZEFILERST} | grep "ERROR" >> /dev/null
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    echo "Close hot back up failed:" >> ${LOGFILE}
    cat ${UNFREEZEFILERST} >> ${LOGFILE}
    [ -f ${UNFREEZEFILERST} ] && rm -f ${UNFREEZEFILERST}
    [ -f ${UNFREEZEFILE} ] && rm -f ${UNFREEZEFILE}
    exit 1
fi

[ -f ${UNFREEZEFILERST} ] && rm -f ${UNFREEZEFILERST}
[ -f ${UNFREEZEFILE} ] && rm -f ${UNFREEZEFILE}

echo "Finish doing unfreeze db." >> ${LOGFILE}
exit 0
########End#######
