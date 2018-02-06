#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################

sys=`uname -s`
if [ $sys = "SunOS" ]
then
    AWK=nawk
else
    AWK=awk
fi

getLinuxOsName()
{
    if [ -f /etc/oracle-release ] #oracle linux 
    then
        OS_NAME=OL`cat /etc/oracle-release| $AWK -F '.' '{print $1}' | $AWK '{print $NF}'`
    elif [ -f /etc/SuSE-release ] #SuSe10/11/12
    then
        OS_NAME=SuSE`cat /etc/SuSE-release | $AWK '$1 == "VERSION" {print $3}'`
    elif [ -f /etc/isoft-release ] #isoft
    then
        OS_NAME=iSoft$(cat /etc/isoft-release | awk -F '.' '{print $1}' | awk '{print $NF}')
    elif [ -f /etc/redhat-release ] #Redhat5/6/7
    then
        OS_NAME=RedHat`cat /etc/redhat-release | $AWK -F '.' '{print $1}' | $AWK '{print $NF}'`
    elif [ -f /etc/debian_version ] #rocky6
    then
        OS_NAME=Rocky`cat /etc/debian_version|$AWK -F '.' '{print $1}'`
    elif [ -f /etc/linx-release ] #rocky4
    then
        OS_NAME=Rocky`cat /etc/linx-release|awk '{print $2}'|awk -F '.' '{print $1}'`
    fi
    
    echo "$OS_NAME";
}

AGENT_PACK=${HOME}/AGENT_PACK_TEMP
AGENT_VERSION=
AGENT_PACKAGE_VERSION=
BASE_NAME=
PACK_BASE_NAME=
BIN_NAME_BASE=
SRC_NAME_BASE=
DATE_STR=
OS_NAME=
FLAG_BIT="x86_64"

pack_files_src="           \
    bin                    \
    build                  \
    ci                     \
    conf                   \
    open_src               \
    platform               \
    selfdevelop            \
    src                    \
    test                   \
    third_party_groupware  \
    vsprj
  "

pack_files_bin="                     \
    bin/*.sh                         \
    bin/thirdparty                   \
    bin/nginx                        \
    bin/rdagent                      \
    bin/rootexec                     \
    bin/datamigration                \
    bin/monitor                      \
    bin/xmlcfg                       \
    bin/agentcli                     \
    bin/getinput                     \
    bin/lib*                         \
    bin/plugins/*                    \
    conf/agent_cfg.xml               \
    conf/pluginmgr.xml               \
    conf/KMC_Crt.conf               \
    conf/script.sig                  \
    conf/version                     \
    log                              \
    tmp                              \
    *.docx                           \
    db/AgentDB.db
    "

if [ -f ${AGENT_ROOT}/conf/svn ]
then 
    pack_files_bin="$pack_files_bin conf/svn"
fi

if [ ${AGENT_ROOT:-0} = 0 ]
then
    echo "please source env.csh first"
    exit 2
fi

sys=`uname -s`
if [ $sys = "AIX" ]
then
    OS_NAME="AIX"
    OS_VERSION=`oslevel | awk -F "." '{print $1$2}'`
    if [ "${OS_VERSION}" = "53" ]
    then
        OS_NAME="AIX53"
    fi
    FLAG_BIT="ppc_64"
elif [ $sys = "Linux" ]
then
    OS_NAME=$(getLinuxOsName)
elif [ $sys = "HP-UX" ]
then
    OS_VERSION=`uname -a | awk '{print $3}' | awk -F "." '{print $2"."$3}'`
    OS_NAME="HP-UX_$OS_VERSION"
    FLAG_BIT="ia_64"
elif [ $sys = "SunOS" ]
then
    OS_NAME="Solaris"
    FLAG_BIT="sparc_64"
else
    echo "Unsupported OS"
    exit 1
fi

AGENT_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "define AGENT_VERSION" | $AWK -F '"' '{print $2}'`
AGENT_BUILD_NUM=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "define AGENT_BUILD_NUM" | $AWK -F '"' '{print $2}'`
AGENT_PACKAGE_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "define AGENT_PACKAGE_VERSION" | $AWK -F '"' '{print $2}'`

BASE_NAME="OceanStor BCManager ${AGENT_VERSION}_Agent"
PACK_BASE_NAME="OceanStor BCManager ${AGENT_PACKAGE_VERSION}_Agent"
BIN_NAME_BASE="${BASE_NAME}-${OS_NAME}-${FLAG_BIT}"
SRC_NAME_BASE="${BASE_NAME}-SRC-"
DATE_STR=`date +%y-%m-%d`

#clean obj files before pack src files
if [ $sys = "SunOS" ]
then
    bash ${AGENT_ROOT}/build/agent_make.sh clean all
else
${AGENT_ROOT}/build/agent_make.sh clean all
fi

#del svn files
find ${AGENT_ROOT} -name ".svn" |xargs rm -rf 2>/dev/null
find ${AGENT_ROOT} -name ".gitignore" |xargs rm -rf 2>/dev/null

#pack src files
echo "#########################################################"
echo "   Copyright (C), 2013-2017, Huawei Tech. Co., Ltd."
echo "   Start to pack src files"
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

if [ ! -d ${AGENT_PACK} ]
then
    mkdir ${AGENT_PACK}
fi

#prepare log and tmp dir
if [ ! -d ${AGENT_ROOT}/log ]
then
    mkdir ${AGENT_ROOT}/log
fi
rm -rf ${AGENT_ROOT}/log/*

if [ ! -d ${AGENT_ROOT}/tmp ]
then
    mkdir ${AGENT_ROOT}/tmp
fi
rm -rf ${AGENT_ROOT}/tmp/*

#build src package
cd ${AGENT_PACK}
rm -f "${SRC_NAME_BASE}${DATE_STR}.tar"  "${SRC_NAME_BASE}${DATE_STR}.tar.gz"
cd ${AGENT_ROOT}
tar cvf "${AGENT_PACK}/${SRC_NAME_BASE}${DATE_STR}.tar" ${pack_files_src}
gzip "${AGENT_PACK}/${SRC_NAME_BASE}${DATE_STR}.tar"

EndTime=`date '+%Y-%m-%d %H:%M:%S'`
echo "#########################################################"
echo "   Pack src files completed."
echo "   begin at ${StartTime}"
echo "   end   at ${EndTime}"
echo "#########################################################"

#make agent
if [ $sys = "SunOS" ]
then
    bash ${AGENT_ROOT}/build/agent_make.sh $*
else
${AGENT_ROOT}/build/agent_make.sh $*
fi
if [ $? != 0 ]
then
    exit 1
fi
echo ""

#mk version file
if [ -f ${AGENT_ROOT}/conf/version ]
then
    rm -rf ${AGENT_ROOT}/conf/version
fi

echo ${AGENT_VERSION}>${AGENT_ROOT}/conf/version
echo ${AGENT_BUILD_NUM}>>${AGENT_ROOT}/conf/version

#modify plugin.xml
OLD_PLUG_VER=$(cat ${AGENT_ROOT}/conf/pluginmgr.xml | grep '" service="' | $AWK -F '" service="' '{print $1}' |  $AWK -F 'version="' '{print $2}' | sed -n 1p)
mv ${AGENT_ROOT}/conf/pluginmgr.xml ${AGENT_ROOT}/conf/pluginmgr.xml.bak
sed "s/${OLD_PLUG_VER}/${AGENT_BUILD_NUM}/g"  ${AGENT_ROOT}/conf/pluginmgr.xml.bak > ${AGENT_ROOT}/conf/pluginmgr.xml

${AGENT_ROOT}/bin/scriptsign
rm -f ${AGENT_ROOT}/log/scriptsign.log

#pack bin files
echo "#########################################################"
echo "   Copyright (C), 2013-2017, Huawei Tech. Co., Ltd."
echo "   Start to pack bin files"
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

if [ ! -d ${AGENT_ROOT}/db ]
then
    mkdir ${AGENT_ROOT}/db
    chmod 755 ${AGENT_ROOT}/db
    cp ${AGENT_ROOT}/selfdevelop/AgentDB.db ${AGENT_ROOT}/db
fi

cp "${AGENT_ROOT}/build/copyRight/Open Source Software Notice.docx" ${AGENT_ROOT}/

#initialize snmp config
INPUT_PATH=${AGENT_ROOT}/build/input
rm -rf $INPUT_PATH
echo BCM@DataProtect8 >$INPUT_PATH
SNMP_CONFIG=`${AGENT_ROOT}/bin/crypto -a 0 -i $INPUT_PATH`
${AGENT_ROOT}/bin/xmlcfg write SNMP private_password "$SNMP_CONFIG"

rm -rf $INPUT_PATH
echo BCM@DataProtect6 >$INPUT_PATH
SNMP_CONFIG=`${AGENT_ROOT}/bin/crypto -a 0 -i $INPUT_PATH`
${AGENT_ROOT}/bin/xmlcfg write SNMP auth_password "$SNMP_CONFIG"

#initialize nginx config
rm -rf $INPUT_PATH
#echo '$1$huawei$v912RCOmdxCRq2Vpetk2k/' >$INPUT_PATH
echo 'BCM@DataProtect123' >$INPUT_PATH
NGINX_CONFIG=`${AGENT_ROOT}/bin/crypto -a 0 -i $INPUT_PATH`
${AGENT_ROOT}/bin/xmlcfg write Monitor nginx ssl_key_password "$NGINX_CONFIG"

rm -f ${AGENT_ROOT}/log/*

#build bin package
cd ${AGENT_PACK}
rm -f "${BIN_NAME_BASE}.tar" "${BIN_NAME_BASE}.tar.gz"
cd ${AGENT_ROOT}
tar cvf "${AGENT_PACK}/${BIN_NAME_BASE}.tar" ${pack_files_bin}
gzip "${AGENT_PACK}/${BIN_NAME_BASE}.tar"

rm -rf ${AGENT_ROOT}/db
EndTime=`date '+%Y-%m-%d %H:%M:%S'`
echo "#########################################################"
echo "   Pack bin files completed."
echo "   begin at ${StartTime}"
echo "   end   at ${EndTime}"
echo "#########################################################"

