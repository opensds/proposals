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

QUERYFILE=$TMPPATH"/querystate${PID}.sql"
QUERYFILERST=$TMPPATH"/querystateRST${PID}.txt"

# The related business script code need to be here.
########Begin########
echo "Begin to do query freeze database." >> ${LOGFILE}
echo "select count(*) from v\$backup where status='ACTIVE';" > ${QUERYFILE}
echo "exit;" >> ${QUERYFILE}

touch ${QUERYFILERST}
chmod 666 ${QUERYFILERST}
su - oracle -c "export ORACLE_SID=dbora; sqlplus / as sysdba @${QUERYFILE} >> ${QUERYFILERST}" 

# if the operation is successed, need to write blank string into the result file ${RSTFILE}. 
# Otherwise please write material error infomation into the result file ${$RSTFILE}.
# For example,
# business code result
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    echo "Query freeze state failed." >> ${LOGFILE}
    [ -f ${QUERYFILE} ] && rm -f ${QUERYFILE}
    [ -f ${QUERYFILERST} ] && rm -f ${QUERYFILERST}
    exit 1
fi

cat ${QUERYFILERST} | grep "ERROR" >> /dev/null
RSTCODE=$?
if [ ${RSTCODE} -ne 0 ]
then
    echo "Query freeze state failed:" >> ${LOGFILE}
    cat ${QUERYFILERST} >> ${LOGFILE}
    [ -f ${QUERYFILE} ] && rm -f ${QUERYFILE}
    [ -f ${QUERYFILERST} ] && rm -f ${QUERYFILERST}
    exit 1
fi

ACTIVECOUNT=`sed -n '/-----/,/^ *$/p' "${QUERYFILERST}" | sed -e '/-----/d' -e '/^ *$/d' | awk '{print $1}'`
if [ "${ACTIVECOUNT}" -eq "0" ]
then
    echo "There are no backup tablespace." >> ${LOGFILE}
    echo 1 > "${RSTFILE}"
else
    echo "Database is in hot backup mode:${ACTIVECOUNT}." >> ${LOGFILE}
    echo 0 > "${RSTFILE}"
fi
        
[ -f ${QUERYFILE} ] && rm -f ${QUERYFILE}
[ -f ${QUERYFILERST} ] && rm -f ${QUERYFILERST}

echo "Finish doing query freeze database." >> ${LOGFILE}
exit 0
########End#######
