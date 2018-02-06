#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

set +x

sysName=`uname -s`

ERROR_DB_FREEZE_YES=0
ERROR_DB_FREEZE_NO=1
ERROR_DB_FREEZE_UNKNOWN=2
#common 5-19
ERROR_SCRIPT_EXEC_FAILED=5
ERROR_RESULT_FILE_NOT_EXIST=6
ERROR_TMP_FILE_IS_NOT_EXIST=7
ERROR_PATH_WRONG=8
ERROR_PARAM_WRONG=9
ERROR_DB_USERPWD_WRONG=10
ERROR_INSTANCE_NOSTART=11

ERROR_INSUFFICIENT_WRONG=15
ERROR_NOSUPPORT_DBFILE_ON_BLOCKDEVICE=16
ERROR_DEVICE_FILESYS_MOUNT_FAILED=17
ERROR_DEVICE_FILESYS_UNMOUNT_FAILED=18
#ORACLE error code 20-69

ERROR_ORACLE_ASM_DBUSERPWD_WRONG=21
ERROR_ORACLE_ASM_INSUFFICIENT_WRONG=22
ERROR_ORACLE_ASM_INSTANCE_NOSTART=23
ERROR_ORACLE_NOARCHIVE_MODE=24
ERROR_ORACLE_OVER_ARCHIVE_USING=25
ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT=26
ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT=27
ERROR_ORACLE_APPLICATION_OVER_MAX_LINK=28

ERROR_ORACLE_DB_ALREADY_INBACKUP=29
ERROR_ORACLE_DB_INHOT_BACKUP=30
ERROR_ORACLE_DB_ALREADYRUNNING=31
ERROR_ORACLE_DB_ALREADYMOUNT=32
ERROR_ORACLE_DB_ALREADYOPEN=33
ERROR_ORACLE_DB_ARCHIVEERROR=34
ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED=35
ERROR_ORACLE_END_HOT_BACKUP_FAILED=36
ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT=37
ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP=38
ERROR_ASM_NO_STARTUP_TNS=39
ERROR_ORACLE_NOT_MOUNTED=40
ERROR_ORACLE_NOT_OPEN=41
ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED=42
ERROR_ORACLE_TNS_PROTOCOL_ADAPTER=43
#oracle not install
ERROR_ORACLE_NOT_INSTALLED=44
ERROR_ORACLE_ANOTHER_STARTING=45
ERROR_ORACLE_DB_BUSY=46

ERROR_SCRIPT_ORACLE_INST_NOT_CDB=47
ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT=48
ERROR_SCRIPT_START_PDB_FAILED=49
ERROR_DB_FILE_NOT_EXIST=50

#DB2 error code 70-99
ERROR_DB2_SUSPEND_IO_FAILED=70
ERROR_DB2_RESUME_IO_FAILED=71
ERROR_DB2_SUSPEND_IO_TIMEOUT=72
#CLUSTER error code 160-189
ERROR_CLUSTER_SERVICE_NOSTART=160
ERROR_CLUSTER_DB_NOT_INCLUSTER=161
ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL=162

ERROR_CLUSTER_RESOURCE_ONLINE_FAILED=163
ERROR_CLUSTER_RESOURCE_OFFLINE_FAILED=164
ERROR_NOT_ACTIVE_NODE=165

# inner error code, not return to RD server


RESULT_TMP_FILE_PREFIX=result_tmp

MAXLOGSIZE=3145728
LOGFILE_SUFFIX="gz"
BACKLOGCOUNT=`${AGENT_ROOT_PATH}/bin/xmlcfg read System log_count`

#global path
BIN_PATH="${AGENT_ROOT_PATH}/bin"
TMP_PATH="${AGENT_ROOT_PATH}/tmp"
LOG_PATH="${AGENT_ROOT_PATH}/log"
CONF_PATH="${AGENT_ROOT_PATH}/conf"
RESULT_FILE="${TMP_PATH}/${RESULT_TMP_FILE_PREFIX}${PID}"
# global var, for kill monitor
MONITOR_FILENAME="${BIN_PATH}/procmonitor.sh"
SPACE=

#for 
QRY_NOT_BACKUP_TB_SQL="select status from v\$backup where status = 'NOT ACTIVE';"
QRY_BACKUP_TB_SQL="select status from v\$backup where status = 'ACTIVE';"

if [ "$sysName" = "SunOS" ]
then
    MYAWK=nawk
else
    MYAWK=awk
fi

# Get the specified value from input argument
##############################################################
#
## @Usage Get the specified value from input argument
#
## @Return If existed sepcified value return 0, else return 1
#
## @Description This function gets specified value from input argument
##############################################################
GetValue()
{
   if [ $# -ne 2 ]
   then
       exit $ERROR_SCRIPT_EXEC_FAILED
   fi

   if [ -f "$ArgFile" ]
   then
      rm -fr "$ArgFile"
   fi

   echo $1 | $MYAWK -F ":" '{for(x=1;x<=NF;x++) print $x}' > "$ArgFile"
   ArgValue=`$MYAWK -F "=" '$1=="'$2'" {print $2}' "$ArgFile"`

   if [ -f "$ArgFile" ]
   then
       rm -fr "$ArgFile"
   fi
}

DeleteFile()
{
    if [ $# -lt 1 ]
    then
        return 0
    fi

    for i in "$@"
    do
        if [ -f "${i}" ]
        then
            rm -fr "${i}"
        fi
    done

    return 0
}

Log()
{
    if [ ! -f "${LOG_FILE_NAME}" ]
    then
        touch "${LOG_FILE_NAME}"
        chmod 600 "${LOG_FILE_NAME}" 
    fi
    
    DATE=`date +%y-%m-%d--%H:%M:%S`
    if [ "SunOS" = "$sysName" ]
    then
        USER_NAME=`/usr/ucb/whoami`
    else
        USER_NAME=`whoami`
    fi
    
    echo "${DATE}:[$$][${USER_NAME}] $1" >> "${LOG_FILE_NAME}"
    
    BACKLOGNAME="${LOG_FILE_NAME}.${BACKLOGCOUNT}.${LOGFILE_SUFFIX}"
    NUMBER=`expr ${BACKLOGCOUNT} - 1` 
    LOGFILESIZE=`ls -l "${LOG_FILE_NAME}" | $MYAWK -F " " '{print $5}'`
    if [ ${LOGFILESIZE} -gt ${MAXLOGSIZE} ]
    then
        if [ -f "${BACKLOGNAME}" ]
        then
            rm -f "${BACKLOGNAME}"
        fi
        
        while [ $NUMBER -ge 0 ]
        do
            if [ $NUMBER -eq 0 ]
            then
                gzip -f -q -9 "${LOG_FILE_NAME}"
                BACKLOGNAME="${LOG_FILE_NAME}.${LOGFILE_SUFFIX}"
            else
                BACKLOGNAME="${LOG_FILE_NAME}.${NUMBER}.${LOGFILE_SUFFIX}"                 
            fi
            
            if [  -f "${BACKLOGNAME}" ]
            then
                DestNum=`expr $NUMBER + 1`
                mv -f "${BACKLOGNAME}" "${LOG_FILE_NAME}.${DestNum}.${LOGFILE_SUFFIX}" 
                chmod 440 "${LOG_FILE_NAME}.${DestNum}.${LOGFILE_SUFFIX}" 
            fi
            
            NUMBER=`expr $NUMBER - 1`
        done
    fi

    if [ ! -f "${LOG_FILE_NAME}" ]
    then
        touch "${LOG_FILE_NAME}" 
        chmod 600 "${LOG_FILE_NAME}"
    fi
}

ChangeDirPrivilege()
{
    if [ "${AGENT_ROOT_TMP}" != "/" ]
    then
        chmod o+rx "${AGENT_ROOT_TMP}"
        AGENT_ROOT_TMP=`dirname $AGENT_ROOT_TMP`
        ChangeDirPrivilege
    fi
}

ChangePrivilege()
{    
    chown -R ${AGENT_USER}:${AGENT_GROUP} "${AGENT_ROOT_PATH}/"*
    
    AGENT_ROOT_TMP=${AGENT_ROOT_PATH}
    ChangeDirPrivilege
    
    chmod -R 500 "${AGENT_ROOT_PATH}/bin"
    chmod 400 "${AGENT_ROOT_PATH}"/bin/thirdparty/sample/*.sh
    chmod 700 "${AGENT_ROOT_PATH}/conf"
    chmod 700 "${AGENT_ROOT_PATH}/log"
    chmod 707 "${AGENT_ROOT_PATH}/tmp"
    chmod 700 "${AGENT_ROOT_PATH}/db"

    chmod -R 700 "${AGENT_ROOT_PATH}/bin/nginx"
    chmod 500 "${AGENT_ROOT_PATH}/bin/nginx/rdnginx"
    chmod 600 "${AGENT_ROOT_PATH}/bin/nginx/conf/"*
    
    NGINX_LOGS=`ls "${AGENT_ROOT_PATH}/bin/nginx/logs/"`
    if [ "${NGINX_LOGS}" != "" ]
    then
        chmod 600 "${AGENT_ROOT_PATH}/bin/nginx/logs/"*
    fi
    
    chmod 600 "${AGENT_ROOT_PATH}/db/AgentDB.db"
    chown root:${AGENT_GROUP} "${AGENT_ROOT_PATH}/bin/rootexec"
    chmod 550 "${AGENT_ROOT_PATH}/bin/rootexec"
    chmod +s "${AGENT_ROOT_PATH}/bin/rootexec"
    
    if [ -f "${AGENT_ROOT_PATH}/conf/HostSN" ]
    then
        chmod 600 "${AGENT_ROOT_PATH}/conf/HostSN"
    fi

    chmod 600 "${AGENT_ROOT_PATH}/conf/pluginmgr.xml"
    chmod 600 "${AGENT_ROOT_PATH}/conf/agent_cfg.xml"
    chmod 400 "${AGENT_ROOT_PATH}/conf/script.sig"
    
    [ -f "${AGENT_ROOT_PATH}/conf/version" ] && chmod 400 "${AGENT_ROOT_PATH}/conf/version"
    [ -f "${AGENT_ROOT_PATH}/conf/svn" ] && chmod 400 "${AGENT_ROOT_PATH}/conf/svn"
    [ -f "${AGENT_ROOT_PATH}/conf/kmc_config.txt" ] && chmod 600 "${AGENT_ROOT_PATH}/conf/kmc_config.txt"
    [ -f "${AGENT_ROOT_PATH}/conf/kmc_store.txt" ] && chmod 600 "${AGENT_ROOT_PATH}/conf/kmc_store.txt"

    [ -f "${AGENT_ROOT_PATH}/bin/nginx/conf/bcmagentca.crt" ] && chmod 400 "${AGENT_ROOT_PATH}/bin/nginx/conf/bcmagentca.crt"
    [ -f "${AGENT_ROOT_PATH}/bin/nginx/conf/server.crt" ] && chmod 400 "${AGENT_ROOT_PATH}/bin/nginx/conf/server.crt"
    [ -f "${AGENT_ROOT_PATH}/bin/nginx/conf/server.key" ] && chmod 400 "${AGENT_ROOT_PATH}/bin/nginx/conf/server.key"
    [ -f "${AGENT_ROOT_PATH}/bin/nginx/conf/kmc_config_bak.txt" ] && chmod 600 "${AGENT_ROOT_PATH}/bin/nginx/conf/kmc_config_bak.txt"
    [ -f "${AGENT_ROOT_PATH}/bin/nginx/conf/kmc_store_bak.txt" ] && chmod 600 "${AGENT_ROOT_PATH}/bin/nginx/conf/kmc_store_bak.txt"
    
    # change root privilege
    CHG_ROOTSCRIPT_LIST="db2clusterinfo.sh db2info.sh db2luninfo.sh db2recover.sh db2resourcegroup.sh db2sample.sh initiator.sh oraasmaction.sh oraclecheckarchive.sh oracleclusterinfo.sh oracleconsistent.sh oracleinfo.sh oraclefunc.sh oracleluninfo.sh oracleresourcegroup.sh oracletest.sh oradbaction.sh agent_install.sh agent_uninstall.sh agent_upgrade.sh datamigration packlog.sh getinput scandisk.sh cacheclusterinfo.sh cachefunc.sh cacheinfo.sh cacheluninfo.sh cachesample.sh oraclepdbinfo.sh oraclepdbstart.sh oraclecheckcdb.sh"
    for SCRIPTFILE in ${CHG_ROOTSCRIPT_LIST}
    do
        [ -f "${AGENT_ROOT_PATH}/bin/${SCRIPTFILE}" ] && chown root "${AGENT_ROOT_PATH}/bin/${SCRIPTFILE}"
    done
}

SetAutoStart()
{
    if [ "Linux" = "$sysName" ]
    then
        if [ -f /etc/SuSE-release ]
        then
            if [ ! -f /etc/rc.d/boot.local ]
            then
                touch /etc/rc.d/boot.local
            fi

            chmod 755 /etc/rc.d/boot.local
                
            RM_LINENO=`cat /etc/rc.d/boot.local | grep -n -w "su - ${AGENT_USER} ${UNIX_CMD} -c" | grep -w "${AGENT_ROOT_PATH}/bin/monitor"`
            if [ "${RM_LINENO}" = "" ]
            then
                echo "su - ${AGENT_USER} ${UNIX_CMD} -c \"${AGENT_ROOT_PATH}/bin/monitor &\"  > /dev/null"  >> /etc/rc.d/boot.local
            fi
        elif [ -f /etc/redhat-release ]
        then
            if [ ! -f /etc/rc.d/rc.local ]
            then
                touch /etc/rc.d/rc.local
            fi
            
            chmod 755 /etc/rc.d/rc.local

            RM_LINENO=`cat /etc/rc.d/rc.local | grep -n -w "su - ${AGENT_USER} ${UNIX_CMD} -c" | grep -w "${AGENT_ROOT_PATH}/bin/monitor"`
            if [ "${RM_LINENO}" = "" ]
            then
                echo "su - ${AGENT_USER} ${UNIX_CMD} -c \"${AGENT_ROOT_PATH}/bin/monitor &\"  > /dev/null"  >> /etc/rc.d/rc.local
            
            fi            
        elif [ "${SysRockyFlag}" != "" ]
        then
            if [ ! -f /etc/rc.local ]
            then
                touch /etc/rc.local
            fi
            
            chmod 755 /etc/rc.local
            
            RM_LINNO=`cat /etc/rc.local | grep -n -w 'exit 0' | $MYAWK -F ':' 'END {print $1}'`
            if [ "${RM_LINNO}" != "" ]
            then
                sed -i "${RM_LINNO}d" /etc/rc.local
            fi

            RM_LINENO=""
            RM_LINENO=`cat /etc/rc.local | grep -n -w "su - ${AGENT_USER} ${UNIX_CMD} -c" | grep -w "${AGENT_ROOT_PATH}/bin/monitor"`
            if [ "${RM_LINENO}" = "" ]
            then
                echo "su - ${AGENT_USER} ${UNIX_CMD} -c \"${AGENT_ROOT_PATH}/bin/monitor &\"  > /dev/null"  >> /etc/rc.local
            fi
        else
            Log "Unsupport OS."
        fi
    elif [ "AIX" = "$sysName" ]
    then
        if [ -f /etc/rc_rdagent.local ]
        then
            rm /etc/rc_rdagent.local
        fi
        
        touch /etc/rc_rdagent.local
        chmod 755 /etc/rc_rdagent.local
        
        echo "su - ${AGENT_USER} ${UNIX_CMD} -c \"${AGENT_ROOT_PATH}/bin/monitor &\" > /dev/null">>/etc/rc_rdagent.local
        
        if [ -f /etc/inittab ]
        then
            lsitab -a | grep /etc/rc_rdagent.local >/dev/null 2>&1
            if [ $? -ne 0 ]
            then
                mkitab  rdagent:2:once:/etc/rc_rdagent.local
                init q
            fi
        fi
    elif [ "HP-UX" = "$sysName" ]
    then
        START_SCRIPT=/sbin/init.d/AgentStart
        AGENT_CONF=/etc/rc.config.d/Agentconf
        SCRIPT=/sbin/rc3.d/S999AgentStart
        
        rm -rf ${SCRIPT}
        rm -rf ${START_SCRIPT}
        rm -rf ${AGENT_CONF}
        
        echo "#!/sbin/sh" >       $START_SCRIPT
        echo "AGENT_ROOT=${AGENT_ROOT_PATH}"  >> $START_SCRIPT
        echo "AGENT_USER=rdadmin" >> $START_SCRIPT
        echo "AGENT_CONF=/etc/rc.config.d/Agentconf"    >> $START_SCRIPT
        echo "PATH=/bin:$PATH" >> $START_SCRIPT
        echo "export PATH"     >> $START_SCRIPT
        echo "if [ -f \$AGENT_CONF ];then"   >> $START_SCRIPT
        echo "    . \$AGENT_CONF"            >> $START_SCRIPT
        echo "else"            >> $START_SCRIPT
        echo "    echo ERROT: \$AGENT_CONF defaults file missing." >> $START_SCRIPT
        echo "fi"              >> $START_SCRIPT
        echo "if [ \$APPPROXY -eq 1 ];then" >> $START_SCRIPT
        echo "    MONITOR_FLAG=\$(ps -u \${AGENT_USER} | grep -v grep | grep monitor)" >> $START_SCRIPT
        echo "    if [ \"\${MONITOR_FLAG}\" = \"\" ];then" >> $START_SCRIPT
        echo "        nohup su - \${AGENT_USER} ${UNIX_CMD} -c \"\${AGENT_ROOT}/bin/monitor &\" > /dev/null" >> $START_SCRIPT
        echo "    fi" >> $START_SCRIPT
        echo "fi"     >> $START_SCRIPT
                
        touch $AGENT_CONF
        echo "APPPROXY=1" >> $AGENT_CONF

        chmod 755 $START_SCRIPT
        chmod 755 $AGENT_CONF
        ln -s $START_SCRIPT $SCRIPT
    elif [ "SunOS" = "$sysName" ]
    then
        START_SCRIPT=/etc/init.d/agentstart
        SCRIPT=/etc/rc2.d/S99agentstart
        
        rm -rf ${SCRIPT}
        rm -rf ${START_SCRIPT}
        
        echo "#!/sbin/sh" >       $START_SCRIPT
        echo "AGENT_ROOT=${AGENT_ROOT_PATH}"  >> $START_SCRIPT
        echo "AGENT_USER=rdadmin" >> $START_SCRIPT
        echo "PATH=/bin:$PATH" >> $START_SCRIPT
        echo "export PATH"     >> $START_SCRIPT
        echo "MONITOR_FLAG=\`ps -u \${AGENT_USER} | grep -v grep | grep monitor\`" >> $START_SCRIPT
        echo "if [ \"\${MONITOR_FLAG}\" = \"\" ];then" >> $START_SCRIPT
        echo "    nohup su - \${AGENT_USER} ${UNIX_CMD} -c \"\${AGENT_ROOT}/bin/monitor &\" > /dev/null" >> $START_SCRIPT
        echo "fi" >> $START_SCRIPT
        
        chmod 755 $START_SCRIPT
        
        ln -s $START_SCRIPT $SCRIPT 
    else
        Log "Unsupport OS."
    fi
}

RDsubstr()
{
    from=$2
    if [ $# -eq 2 ]
    then
        echo $1 | $MYAWK -v bn="$from" '{print substr($1,bn)}'    
    elif [ $# -eq 3 ]
    then
        len=$3
        echo $1 | $MYAWK -v bn="$from" -v ln="$len" '{print substr($1,bn,ln)}'
    fi
}

KillProcMonitor()
{
    MYPPID=$1
    MONITORNAME=procmonitor.sh
    Log "Begin kill all $MONITORNAME."
    MONITORPIDs=`ps -ef | grep ${MONITORNAME} | grep -v "grep" | $MYAWK -v ppid="${MYPPID}" '{if ($3==ppid) print $2}'`
    
    for MONITORPID in ${MONITORPIDs}
    do
        Log "get procMonitor id=${MONITORPID}."
        kill -9 ${MONITORPID}
    done
    
    Log "End kill all $MONITORNAME."
    return 0
}

GetUserShellType()
{
    if [ "${sysName}" = "AIX" ] || [ "$sysName" = "HP-UX" ]
    then
        SHELLTYPE=`cat /etc/passwd | grep "^${DBUser}:" | $MYAWK -F "/" '{print $NF}'`
        if [ "$SHELLTYPE" = "bash" ]
        then
            ORACLE_SHELLTYPE=-l
        fi
        SHELLTYPE=`cat /etc/passwd | grep "^${GridUser}:" | $MYAWK -F "/" '{print $NF}'`
        if [ "$SHELLTYPE" = "bash" ]
        then
            GRID_SHELLTYPE=-l
        fi
        SHELLTYPE=`cat /etc/passwd | grep "^rdadmin:" | $MYAWK -F "/" '{print $NF}'`
        if [ "$SHELLTYPE" = "bash" ]
        then
            RDADMIN_SHELLTYPE=-l
        fi    
    fi
}

PrintPrompt()
{
    if [ "${sysName}" = "Linux" ]; then
        echo -n ">> "
    else
        echo ">> \c"
    fi
}

#VgName:abc,a-b
#LvName:def,c-d
#Vg_Lv_Name:abc-def,a--b-c--d
#return VgName
AnalyseVgName_Linux_LVM()
{
    Vg_Lv_Name=$1
    LEN=${#Vg_Lv_Name}
    NUM1=1
    while [ $NUM1 -le $LEN ] 
    do
        STR1=`echo $Vg_Lv_Name | cut -b $NUM1`
        if [ "$STR1" = "-" ]
        then
            NUM2=`expr $NUM1 + 1`
            STR2=`echo $Vg_Lv_Name | cut -b $NUM2`
            if [ "$STR2" = "-" ] 
            then
                NUM1=`expr $NUM1 + 2`
                continue
            fi
            break
        fi
        NUM1=`expr $NUM1 + 1`
    done
    NUM1=`expr $NUM1 - 1`
    NUM2=`expr $NUM1 + 2`
    Vg_Name=`echo $Vg_Lv_Name | cut -b 1-$NUM1`
    LEN=${#Vg_Name}
    NUM1=1
    while [ $NUM1 -le $LEN ]
    do
        STR1=`echo $Vg_Name | cut -b $NUM1`
        if [ "$STR1" = "-" ]
        then
            NUM2=`expr $NUM1 + 1`
            STR2=`echo $Vg_Name | cut -b $NUM2`
            if [ "$STR2" = "-" ] 
            then
                NUM2=`expr $NUM2 + 1`
                Vg_Name=`echo $Vg_Name | cut -b 1-$NUM1,$NUM2-`
                LEN=${#Vg_Name}
            fi
        fi
        NUM1=`expr $NUM1 + 1`
    done
    echo "$Vg_Name"
}

#function:check if the linux is redhat7
#param: no
#return: if is redhat7 then return 1, else return 0;
IsRedhat7()
{
    #is not redhat
    if [ ! -f /etc/redhat-release ]
    then
      return 0;
    fi
    
    #redhat
    LINE=`cat /etc/redhat-release`
    REDHAT_VERSION=`echo $LINE|awk -F '.' '{print $1}'|awk '{print $NF}'`
    if [ "$REDHAT_VERSION" = "7" ]
    then
        return 1
    fi
    return 0
}