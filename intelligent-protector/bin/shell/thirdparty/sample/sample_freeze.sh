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

FREEZEFILE=$TMPPATH"/freezeoracle${PID}.sql"
FREEZEFILERST=$TMPPATH"/freezeoracleRST${PID}.txt"

# The related business script code need to be here.
########Begin########
echo "Begin to do freeze db." >> ${LOGFILE}
echo "alter database begin backup;" > ${FREEZEFILE}
echo "exit;" >> ${FREEZEFILE}

touch ${FREEZEFILERST}
chmod 666 ${FREEZEFILERST}
su - oracle -c "export ORACLE_SID=dbora; sqlplus / as sysdba @${FREEZEFILE} >> ${FREEZEFILERST}"

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    echo "Open hot back up failed:" >> ${LOGFILE}
    [ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
    [ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}
    exit 1
fi

cat ${FREEZEFILERST} | grep "ERROR" >> /dev/null
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    echo "Open hot back up failed." >> ${LOGFILE}
    cat ${FREEZEFILERST} >> ${LOGFILE}
    [ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
    [ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}
    exit 1
fi

[ -f ${FREEZEFILERST} ] && rm -f ${FREEZEFILERST}
[ -f ${FREEZEFILE} ] && rm -f ${FREEZEFILE}

echo "Finish doing freeze db." >> ${LOGFILE}
exit 0
########End#######
