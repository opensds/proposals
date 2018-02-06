#!/bin/sh
#################################################################################
# Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
#################################################################################


CLEAN=0
CLEAN_ALL=0
FORTIFY=0
CPPC=
CC=
cc=
CFLAGS=
cFLAGS=
OFLAGS=
DFLAGS=
OS_NAME=
NGINX_CPU_OPT=
NGINX_CPU_OPT_FLAG=0
#Linux version 2.6.29 support the file system freeze and thaw
FREEZE_VERSION=2629
#0 not support the freeze, 1 support
FREEZE_SUPPORT=0

#for module system test's coverage
MST_COVERAGE=0
COV_CFLAGS=
COV_OFLAGS=

sys=`uname -s`

#open source packages
OPEN_SRC_FCGI=fcgi*.tar.gz
OPEN_SRC_JSONCPP=jsoncpp*.zip
OPEN_SRC_NGINX=nginx*.tar.gz
OPEN_SRC_OPENSSL=openssl*.tar.gz
OPEN_SRC_SNMP=snmp++*.tar.gz
OPEN_SRC_SQLITE=sqlite*.zip
OPEN_SRC_TINYXML=tinyxml*.tar.gz

OPEN_SRC_FCGI_DIR=fcgi
OPEN_SRC_JSONCPP_DIR=jsoncpp
OPEN_SRC_NGINX_DIR=nginx_tmp
OPEN_SRC_OPENSSL_DIR=openssl
OPEN_SRC_SQLITE_DIR=sqlite
OPEN_SRC_SNMP_DIR=snmp++
OPEN_SRC_TINYXML_DIR=tinyxml
AWK=
MAKE_JOB=""
KMC_OPT="-DWSEC_COMPILE_CAC_OPENSSL -DWSEC_OPENSSL101 -DWSEC_LINUX -DKMC_MULTIPROCESS -DWSEC_ERR_CODE_BASE=0"

GTESTLIB_DIR= 
GTEST_DIR=${AGENT_ROOT}/test/src/gtest/
if [ $sys = "SunOS" ]
then
    AWK=nawk
else
    AWK=awk
fi

AGENT_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "define AGENT_VERSION" | $AWK -F '"' '{print $2}'`
AGENT_BUILD_NUM=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "define AGENT_BUILD_NUM" | $AWK -F '"' '{print $2}'`
sysLinuxName=$(cat /etc/issue | grep 'Linx')


CompareSysVersion()
{
    SYS_VER=`uname -r | $AWK -F '-' '{print $1}' | $AWK -F '.' '{print $1$2$3}'`
    
    if [ ${SYS_VER} -lt ${FREEZE_VERSION} ]
    then
        FREEZE_SUPPORT=0
    else
        FREEZE_SUPPORT=1
    fi
}

unzip_open_src_packages()
{
    cd ${AGENT_ROOT}/open_src
    #fcgi
    if [ ! -d ${OPEN_SRC_FCGI_DIR} ]
    then
        gzip -cd ${OPEN_SRC_FCGI} | tar -xvf -
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_FCGI} .tar.gz`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_FCGI_DIR}
    fi
    #jsoncpp
    if [ ! -d ${OPEN_SRC_JSONCPP_DIR} ]
    then
        unzip ${OPEN_SRC_JSONCPP}
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_JSONCPP} .zip`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_JSONCPP_DIR}
    fi
    #nginx
    if [ ! -d ${OPEN_SRC_NGINX_DIR} ]
    then
        gzip -cd ${OPEN_SRC_NGINX} | tar -xvf -
        #UNZIPED_DIR_NAME=`basename ${OPEN_SRC_NGINX} .tar.gz`
        UNZIPED_DIR_NAME=nginx-79a20c61e23a
        mv ${UNZIPED_DIR_NAME}/ ${OPEN_SRC_NGINX_DIR}
    fi
    #openssl
    if [ ! -d ${OPEN_SRC_OPENSSL_DIR} ]
    then
        gzip -cd ${OPEN_SRC_OPENSSL} | tar -xvf -
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_OPENSSL} .tar.gz`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_OPENSSL_DIR}
    fi
    #snmp
    if [ ! -d ${OPEN_SRC_SNMP_DIR} ]
    then
        gzip -cd ${OPEN_SRC_SNMP} | tar -xvf -
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SNMP} .tar.gz`
        mv ${UNZIPED_DIR_NAME}/ ${OPEN_SRC_SNMP_DIR}
	cp ${OPEN_SRC_SNMP_DIR}/libsnmp.h  ${OPEN_SRC_SNMP_DIR}/include/libsnmp.h
    fi
    #sqlite
    if [ ! -d ${OPEN_SRC_SQLITE_DIR} ]
    then
        unzip ${OPEN_SRC_SQLITE}
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SQLITE} .zip`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_SQLITE_DIR}
    fi
    #tinyxml
    if [ ! -d ${OPEN_SRC_TINYXML_DIR} ]
    then
        gzip -cd ${OPEN_SRC_TINYXML} | tar -xvf -
    fi
}

delete_open_src_unzipped_files()
{
    cd ${AGENT_ROOT}/open_src

    #fcgi
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_FCGI} .tar.gz`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_FCGI_DIR}
    #jsoncpp
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_JSONCPP} .zip`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_JSONCPP_DIR}
    #nginx
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_NGINX} .tar.gz`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_NGINX_DIR}
    #openssl
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_OPENSSL} .tar.gz`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_OPENSSL_DIR}
    #snmp
    rm -rf ${OPEN_SRC_SNMP_DIR}
    #sqlite
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SQLITE} .zip`
    rm -rf ${UNZIPED_DIR_NAME}
    rm -rf ${OPEN_SRC_SQLITE_DIR}
    #tinyxml
    rm -rf ${OPEN_SRC_TINYXML_DIR}
}

create_dir()
{
    #create plugins dir
    mkdir -p ${AGENT_ROOT}/bin/plugins
    #create objs dir
    mkdir -p ${AGENT_ROOT}/obj
    mkdir -p ${AGENT_ROOT}/obj/agent
    mkdir -p ${AGENT_ROOT}/obj/rootexec
    mkdir -p ${AGENT_ROOT}/obj/crypto
    mkdir -p ${AGENT_ROOT}/obj/scriptsign
    mkdir -p ${AGENT_ROOT}/obj/datamigration
    mkdir -p ${AGENT_ROOT}/obj/monitor
    mkdir -p ${AGENT_ROOT}/obj/xmlcfg
    mkdir -p ${AGENT_ROOT}/obj/agentcli
    mkdir -p ${AGENT_ROOT}/obj/getinput
    mkdir -p ${AGENT_ROOT}/obj/apps/db2
    mkdir -p ${AGENT_ROOT}/obj/apps/app
    mkdir -p ${AGENT_ROOT}/obj/apps/exchange
    mkdir -p ${AGENT_ROOT}/obj/apps/oracle
    mkdir -p ${AGENT_ROOT}/obj/apps/sybase
    mkdir -p ${AGENT_ROOT}/obj/apps/hana
    mkdir -p ${AGENT_ROOT}/obj/cluster
    mkdir -p ${AGENT_ROOT}/obj/array
    mkdir -p ${AGENT_ROOT}/obj/common
    mkdir -p ${AGENT_ROOT}/obj/device
    mkdir -p ${AGENT_ROOT}/obj/host
    mkdir -p ${AGENT_ROOT}/obj/pluginfx
    mkdir -p ${AGENT_ROOT}/obj/plugins
    mkdir -p ${AGENT_ROOT}/obj/plugins/device
    mkdir -p ${AGENT_ROOT}/obj/plugins/cluster
    mkdir -p ${AGENT_ROOT}/obj/plugins/host
    mkdir -p ${AGENT_ROOT}/obj/plugins/db2
    mkdir -p ${AGENT_ROOT}/obj/plugins/oracle
    mkdir -p ${AGENT_ROOT}/obj/plugins/app
    mkdir -p ${AGENT_ROOT}/obj/plugins/cache
    mkdir -p ${AGENT_ROOT}/obj/plugins/sybase
    mkdir -p ${AGENT_ROOT}/obj/plugins/hana
    mkdir -p ${AGENT_ROOT}/obj/rest
    mkdir -p ${AGENT_ROOT}/obj/tools
    mkdir -p ${AGENT_ROOT}/obj/securec
    mkdir -p ${AGENT_ROOT}/obj/sqlite
    mkdir -p ${AGENT_ROOT}/obj/json
    mkdir -p ${AGENT_ROOT}/obj/tinyxml
    mkdir -p ${AGENT_ROOT}/obj/alarm
}

rm_shell()
{
    SHELL_FILES=`ls ${AGENT_ROOT}/bin/*.sh 2>/dev/null`
    if [ "${SHELL_FILES}" != "" ]
    then
        rm ${AGENT_ROOT}/bin/*.sh
    fi

    if [ -d ${AGENT_ROOT}/bin/thirdparty ]
    then
        rm -rf ${AGENT_ROOT}/bin/thirdparty
    fi
}

copy_shell()
{
    rm_shell

    cp ${AGENT_ROOT}/bin/shell/*.sh ${AGENT_ROOT}/bin
    chmod 0755 ${AGENT_ROOT}/bin/*.sh

    #create thirdpatry directory
    if [ ! -d ${AGENT_ROOT}/bin/thirdparty/sample ]
    then
        mkdir -p ${AGENT_ROOT}/bin/thirdparty/sample
    fi

    #copy thirdparty files
    FILE_COUNT=`ls ${AGENT_ROOT}/bin/shell/thirdparty | wc -l`
    if [ ${FILE_COUNT} != 0 ]
    then
        cp -rf ${AGENT_ROOT}/bin/shell/thirdparty/* ${AGENT_ROOT}/bin/thirdparty
        chmod -R 0755 ${AGENT_ROOT}/bin/thirdparty/*
    fi
}

dos2unix_conf_files()
{
    for FILE_NAME in `ls ${AGENT_ROOT}/conf`
    do
        CURR_FILE=${AGENT_ROOT}/conf/${FILE_NAME}
        BAK_CURR_FILE=${CURR_FILE}.bak
        cat ${CURR_FILE} | tr -d '\r' > ${BAK_CURR_FILE}
        
        cp ${BAK_CURR_FILE} ${CURR_FILE}
        rm ${BAK_CURR_FILE}
    done
}

make_snmp()
{
    echo "Begin to compile snmp."
    if [ "$1" = "clean" ]
    then
        rm -rf ${AGENT_ROOT}/open_src/snmp/snmp++/src/.libs
        return
    fi
    
    if [ -f "${AGENT_ROOT}/open_src/snmp++/src/.libs/libsnmp++.a" ]
    then 
        return
    fi

    OLD_CMD_CC=${CC}
    cd ${AGENT_ROOT}/open_src/snmp++
    if [ $sys = "AIX" ]
    then
        export CXXFLAGS="-g -q64 -G"
        export OBJECT_MODE=64
    elif [ $sys = "HP-UX" ]
    then
        export CXXFLAGS="-g +DD64 -w -AA -D_REENTRANT -DHP_UX_IA -DHP_UX -mt -b +z"
    elif [ $sys = "SunOS" ]
    then
        export CXXFLAGS="-g -m64 -mt -xcode=pic32 -G -PIC"
        export CXX="CC"
        export CC="cc"
    else
        export CXXFLAGS="-pipe -m64 -fpic"
    fi
    
    export ssl_CFLAGS="-I${AGENT_ROOT}/open_src/openssl/include"
    export ssl_LIBS="-lssl -lcrypto"
    export LDFLAGS="-L${AGENT_ROOT}/open_src/openssl"
    ./configure --with-ssl --disable-logging --disable-namespace --enable-shared=no
    
    if [ $sys = "AIX" ]
    then
        # In AIX, use PRIu64 need to define __64BIT__ or _LONG_LONG,
        # now do not know define what, so undefine HAVE_INTTYPES_H to go to old logical branch.
        # check it in the file '/usr/include/inttypes.h' in the AIX
        echo "#undef HAVE_INTTYPES_H" >> config.h
        
        # In AIX, compile failed in auth_priv.cpp, need to replace public Hasher to public AuthSHABase::Hasher
        TMP_FILE=${AGENT_ROOT}/open_src/snmp++/auth_test.cpp
        sed -g 's/public\ Hasher/public\ AuthSHABase::Hasher/' ${AGENT_ROOT}/open_src/snmp++/src/auth_priv.cpp > ${TMP_FILE}
        mv ${TMP_FILE} ${AGENT_ROOT}/open_src/snmp++/src/auth_priv.cpp
        
        # UdpAddress not define override function "operator=(const char *address)"
        # In AIX, compile failed, but IpAddress define contructor function "IpAddress::IpAddress(const char *inaddr)"
        # use contructor to resovle this, but do not know why can be compile failed.
        TMP_FILE=${AGENT_ROOT}/open_src/snmp++/uxsnmp_test.cpp
        sed -g 's/fromaddress\ =\ tmp_buffer;/IpAddress\ ipaddr\ =\ tmp_buffer;fromaddress = ipaddr;/' src/uxsnmp.cpp > ${TMP_FILE}
        mv ${TMP_FILE} ${AGENT_ROOT}/open_src/snmp++/src/uxsnmp.cpp
        
        sed -g 's/fromaddress\ =\ inet_ntoa(((sockaddr_in\&)from_addr)\.sin_addr);/IpAddress\ ipaddr\ =\ inet_ntoa(((sockaddr_in\&)from_addr)\.sin_addr);fromaddress\ =\ ipaddr;/' src/uxsnmp.cpp > ${TMP_FILE}
        mv ${TMP_FILE} ${AGENT_ROOT}/open_src/snmp++/src/uxsnmp.cpp
    elif [ $sys = "HP-UX" ]
    then
        # In HP-UX, the function clock_gettime not support parameter CLOCK_MONOTONIC, 
        # so undefine HAVE_CLOCK_GETTIME to go to old logical branch.
        # check it by `man clock_gettime`
        echo "#undef HAVE_CLOCK_GETTIME" >> config.h
    fi
    
    # start to make
    make
    
    main_result=$?
    export CC="${OLD_CMD_CC}"
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Compile snmp failed."
        echo "#########################################################"
        
        exit ${main_result}
    fi
    
    if [ ! -f "${AGENT_ROOT}/open_src/snmp++/src/.libs/libsnmp++.a" ]
    then
        echo "#########################################################"
        echo "   Compile snmp failed, libsnmp++.a is not exists."
        echo "#########################################################"
        
        exit 1
    fi
    
    echo "#########################################################"
    echo "   Compile snmp succ."
    echo "#########################################################"
}

make_fcgi()
{
    if [ "$1" = "clean" ]
    then
        return
    fi

    cd ${AGENT_ROOT}/open_src/fcgi
    if [ -f ".fcgi/lib/libfcgi.a" ]
    then 
        return
    fi
    
    cd ${AGENT_ROOT}/open_src/fcgi
        sed '200s/cgi-fcgi examples//1' ${AGENT_ROOT}/open_src/fcgi/Makefile.in > ${AGENT_ROOT}/open_src/fcgi/Makefile.in.bak
        rm -rf ${AGENT_ROOT}/open_src/fcgi/Makefile.in
        mv ${AGENT_ROOT}/open_src/fcgi/Makefile.in.bak ${AGENT_ROOT}/open_src/fcgi/Makefile.in  
    if [ $sys = "SunOS" ]
    then
        FCGI_CP_OPT="CFLAGS=-xcode=pic32 -m64"
    fi
    ./configure --prefix=${AGENT_ROOT}/open_src/fcgi/.fcgi --disable-shared "$FCGI_CP_OPT"
    if [ $sys = "Linux" ]
    then
        sed '34i #include <cstdio>' -i ${AGENT_ROOT}/open_src/fcgi/include/fcgio.h
    fi
    rm -rf ${AGENT_ROOT}/open_src/fcgi/.fcgi
    
    make $MAKE_JOB $1
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make fcgi failed."
        echo "#########################################################"

        exit ${main_result}
    fi
    
    mkdir ${AGENT_ROOT}/open_src/fcgi/.fcgi
    make install
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make install fcgi failed."
        echo "#########################################################"

        exit ${main_result}
    fi

    cp ${AGENT_ROOT}/open_src/fcgi/fcgi_config.h ${AGENT_ROOT}/open_src/fcgi/include/fcgi_config.h
    rm -rf ${AGENT_ROOT}/open_src/fcgi/.fcgi/lib/*.so*
    echo "#########################################################"
    echo "   Make fcgi succ."
    echo "#########################################################"
}

#Generate sqlite3.c file
make_sqlite3_file()
{
    if [ "$1" = "clean" ]
    then
        return
    fi
    
    cd ${AGENT_ROOT}/open_src/sqlite
    if [ -f "sqlite3.c" ] && [ -f "sqlite3.h" ]
    then
        return
    fi

    chmod +x ./configure
    ./configure
    make $MAKE_JOB sqlite3.c $1
    
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make sqlite3.c failed."
        echo "#########################################################"
        
        exit ${main_result}
    fi

    echo "#########################################################"
    echo "   Make sqlite3.c succ."
    echo "#########################################################"
}

prepare_nginx()
{
    if [ ! -f ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c ]
    then
        return
    fi
    if [ ! -f ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c ]
    then
        return
    fi
    if [ ! -f ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h ]
    then
        return
    fi
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h1
    
    n=0
    while IFS= read -r line
    do
        n=`expr $n + 1`
        if [ "$n" = "37" ]
        then
            echo '    ngx_str_t                       certificate_key_password;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h1
        fi
        echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h1
    done < ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h
    
    #rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h
    mv ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h0
    mv ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h1 ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.h
    
    n=0
    while IFS= read -r line
    do
        n=`expr $n + 1`
        if [ "$n" = "96" ]
        then
            echo '    { ngx_string("ssl_certificate_key_password"),'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
            echo '      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1,'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
            echo '      ngx_conf_set_str_slot,'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
            echo '      NGX_HTTP_SRV_CONF_OFFSET,'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
            echo '      offsetof(ngx_http_ssl_srv_conf_t, certificate_key_password),'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
            echo '      NULL },'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
        elif [ "$n" = "578" ]
        then
            echo '    ngx_conf_merge_str_value(conf->certificate_key_password, prev->certificate_key_password, "");'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
        fi
        echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1
    done < ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c
    
    #rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c
    mv ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c0
    mv ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c1 ${AGENT_ROOT}/open_src/nginx_tmp/src/http/modules/ngx_http_ssl_module.c
    
   
    
    # To solve problem in HP-UX,  #error ngx_atomic_cmp_set() is not defined!
    # ngx_rwlock.c is a new file in nginx 1.10.1, It hasn't influence on now version
    n=0
    while IFS= read -r line
    do
        n=`expr $n + 1`
        if [ "$n" = "11" ]
        then
            echo '#ifdef HP_UX_IA'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1
            echo '#define NGX_HAVE_ATOMIC_OPS 1'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1
            echo '#endif'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1
        fi
        echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1
    done < ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c
    mv ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c1 ${AGENT_ROOT}/open_src/nginx_tmp/src/core/ngx_rwlock.c

    n=0
    while IFS= read -r line
    do
        n=`expr $n + 1`
        if [ "$n" = "12" ]
        then
            echo '#include <ngx_http_ssl_module.h>'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#include <stdio.h>'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#include <stdlib.h>'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#include <string.h>'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#include "wsec_type.h"'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#include "sdp_itf.h"'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#include "kmc_itf.h"'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#include "securec.h"'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            
            echo '#define KMC_STORE_FILE "../../conf/kmc_store.txt"'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#define KMC_STORE_FILE_BAK "conf/kmc_store_bak.txt"'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#define KMC_CONFIG_FILE "../../conf/kmc_config.txt"'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '#define KMC_CONFIG_FILE_BAK "conf/kmc_config_bak.txt"'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            
            echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
        elif [ "$n" = "14" ]
        then
            echo 'WSEC_VOID AppWriteLog_KMC(WSEC_INT32 nLevel, const WSEC_CHAR* pszModuleName, const WSEC_CHAR* pszOccurFileName, WSEC_INT32 nOccurLine, const WSEC_CHAR* pszLog)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '{'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    return;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '}'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1

            echo 'WSEC_VOID APPRecvNotify_KMC(WSEC_NTF_CODE_ENUM eNtfCode, const WSEC_VOID* pData, WSEC_SIZE_T nDataSize)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '{'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    return;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '}'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1

            echo 'WSEC_VOID APPDoEvents_KMC()'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '{'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    return;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '}'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1

            echo 'int Initialize_KMC()'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '{'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    KMC_FILE_NAME_STRU filename ={0};'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    WSEC_FP_CALLBACK_STRU stMandatoryFun = {0};'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    WSEC_ERR_T ret = 0;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    filename.pszKeyStoreFile[0] = KMC_STORE_FILE;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    filename.pszKeyStoreFile[1] = KMC_STORE_FILE_BAK;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    filename.pszKmcCfgFile[0] = KMC_CONFIG_FILE;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    filename.pszKmcCfgFile[1] = KMC_CONFIG_FILE_BAK;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    stMandatoryFun.stRelyApp.pfWriLog = AppWriteLog_KMC;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    stMandatoryFun.stRelyApp.pfNotify = APPRecvNotify_KMC;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    stMandatoryFun.stRelyApp.pfDoEvents = APPDoEvents_KMC;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    ret = WSEC_Initialize(&filename, &stMandatoryFun, 0, 0);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if(ret != WSEC_SUCCESS)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return -1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    return 0;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '}'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1

            echo 'int Finalize_KMC()'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '{'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    WSEC_ERR_T ret = WSEC_Finalize();'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if(ret != WSEC_SUCCESS)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return -1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    return 0;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '}'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1

            echo 'int hexToStr(const unsigned char* hexIn, unsigned char* strOut, unsigned int inLen, unsigned int outLen)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '{'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    unsigned int i = 0;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    char byteStr[5];'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    int ret = 0;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (inLen == 0 || inLen % 2 == 1)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return -1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (hexIn == NULL || strOut == NULL)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return -1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    inLen /= 2;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (outLen < inLen)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return -1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    ret = memset_s(strOut, outLen, 0, outLen);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (ret != 0)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return -1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    for (i = 0; i < inLen; i++)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        ret = snprintf_s(byteStr, sizeof(byteStr), sizeof(byteStr), "0x%c%c", hexIn[2 * i], hexIn[2 * i + 1]);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        if (ret == -1)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '            return -1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        strOut[i] = (unsigned char)strtol((const char *)byteStr, NULL, 16);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    return 0;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '}'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1

            echo 'char* Decrypt_Str_KMC(const char* inStr, const unsigned int inStrLen)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '{'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    WSEC_BYTE *inBuf;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    WSEC_BYTE *outBuf;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    int ret;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    WSEC_ERR_T ret1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    unsigned int inBufLen;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    unsigned int outLen;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    unsigned int outBufLen;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    ret = Initialize_KMC();'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (ret != 0)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (inStrLen == 0 || inStrLen % 2 == 1)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    inBufLen = inStrLen / 2;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    inBuf = (WSEC_BYTE *)malloc(inBufLen);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (inBuf == NULL)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    ret = hexToStr((const unsigned char *)inStr, inBuf, inStrLen, inBufLen);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (ret != 0)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        free(inBuf);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    outLen = inBufLen;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    outBufLen = inBufLen+1;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    outBuf = (WSEC_BYTE *)malloc(outBufLen);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (outBuf == NULL)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        free(inBuf);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    ret = memset_s(outBuf, outBufLen, 0, outBufLen);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if (ret != 0)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        free(inBuf);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        free(outBuf);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    ret1 = SDP_Decrypt(0, inBuf, inBufLen, outBuf, &outLen);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    if(ret1 != WSEC_SUCCESS)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        free(inBuf);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        free(outBuf);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        return NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    free(inBuf);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    Finalize_KMC();'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    return (char*)outBuf;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '}'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            
            echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
        elif [ "$n" = "301" ]
        then
            echo '    if (data != NULL) {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        ngx_http_ssl_srv_conf_t *conf = (ngx_http_ssl_srv_conf_t *)data;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        if (conf->certificate_key_password.len != 0) {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '            char *encryptStr = (char*)(conf->certificate_key_password.data);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '            char *passStr = NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '            if (encryptStr != NULL)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '            {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '                passStr = Decrypt_Str_KMC(encryptStr, strlen(encryptStr));'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '                if (passStr != NULL)'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '                {'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '                    strncpy((char*)(conf->certificate_key_password.data), passStr, strlen(passStr) + 1);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '                    memset(passStr, 0, strlen(passStr));'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '                    free(passStr);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '                }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '            }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '            SSL_CTX_set_default_passwd_cb_userdata(ssl->ctx, conf->certificate_key_password.data);'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '        }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    }'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            
            echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
        elif [ "$n" = "315" ]
        then
            echo '    u_char *pw=NULL;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            
            echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
        elif [ "$n" = "502" ]
        then
            echo '    pw = (*((*ssl).ctx)).default_passwd_callback_userdata;'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            echo '    ngx_memzero(pw, ngx_strlen(pw));'>> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            
            echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
        elif [ "$n" = "1150" ]
        then
            if [ $sys = "Linux" ]
            then
                echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            else
                echo "            *d = '\\\\0';">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            fi
        elif [ "$n" = "3301" ]
        then
            if [ $sys = "Linux" ] 
            then
                echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            else
                echo "            *p++ = '\\\\t';">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
            fi
        else
            echo "$line">> ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1
        fi
    done < ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c
    
    #rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c
    mv ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c0
    mv ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c1 ${AGENT_ROOT}/open_src/nginx_tmp/src/event/ngx_event_openssl.c
}

make_nginx()
{
    if [ "$1" = "clean" ]
    then
        rm -rf ${AGENT_ROOT}/open_src/nginx
        rm -rf ${AGENT_ROOT}/bin/nginx
        return
    fi

    if [ -f "${AGENT_ROOT}/open_src/nginx/nginx" ]
    then
        return
    fi

    OLD_CFLAGS=${CFLAGS}
    if [ $sys = "AIX" ]
    then
        export OBJECT_MODE=64
    elif [ $sys = "HP-UX" ]
    then
        NGINX_CFG_OPT="--with-cc-opt=-Agcc"
        #to resolve download log failed when the size of log is over 32K
        sed '42s/^.*$/            fastcgi_keep_conn on;/' ${AGENT_ROOT}/conf/nginx.conf >${AGENT_ROOT}/conf/nginx.conf.1
        sed '43s/^.*$/            fastcgi_buffers 6 8k;/' ${AGENT_ROOT}/conf/nginx.conf.1 >${AGENT_ROOT}/conf/nginx.conf.2
        rm -rf ${AGENT_ROOT}/conf/nginx.conf
        rm -rf ${AGENT_ROOT}/conf/nginx.conf.1
        mv ${AGENT_ROOT}/conf/nginx.conf.2 ${AGENT_ROOT}/conf/nginx.conf
	# HP-UX 11v2 nginx will close connection in ngx_event_accept.c(line 295, c->addr_text.len is zero), because the sa_family is zero after accept(line 82) 
        # do not known cause, in function ngx_event_accept change "socklen_t  socklen" to "int socklen"
        OS_VERSION=`uname -a | awk '{print $3}' | awk -F "." '{print $2"."$3}'`
        #HP-UX 11v2
        if [ "${OS_VERSION}" = "11.23" ]
        then
            export CFLAGS="${CFLAGS} -DHP_UX_11V2"
        fi
    elif [ $sys = "SunOS" ]
    then
        NGINX_CFG_OPT=""    
        CFLAGS=""
    fi
    
    echo "#########################################################"
    echo "   Start to make Nginx."
    echo "#########################################################"

    cd ${AGENT_ROOT}/open_src/nginx_tmp/
    if [  ${NGINX_CPU_OPT_FLAG} -eq 1 ]
    then
        export CFLAGS="${CFLAGS} -Werror"
    fi
    if [ $sys = "SunOS" ]
    then
        auto/configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module --with-http_ssl_module --with-openssl=../openssl --with-cpu-opt=sparc64 --with-cc-opt="-xmemalign -DWSEC_ERR_CODE_BASE=0"
	else
        auto/configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module --with-http_ssl_module --with-openssl=../openssl $NGINX_CFG_OPT
    fi
    
    if [ $sys = "HP-UX" ]
    then
        export LPATH=/usr/lib/hpux64/
    fi
    
    OPENSSL_DEL_BEGIN_LINE=`cat ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile | grep -n '../openssl/.openssl/include/openssl/ssl.h:'| $AWK -F ":" '{print $1}'`
    OPENSSL_DEL_END_LINE=`expr $OPENSSL_DEL_BEGIN_LINE + 5`
    sed $OPENSSL_DEL_BEGIN_LINE,${OPENSSL_DEL_END_LINE}d ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile >${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    sed -n '/-I ..\/openssl\/.openssl\/include/=' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    ROW_NUM_RET=`sed -n '2p' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row`
    if [ $sys = "Linux" -o $sys = "SunOS" ]
    then
        SED_CMD=$ROW_NUM_RET's/\\/-I ..\/..\/platform\/kmc\/include -I ..\/..\/platform\/kmc\/src\/sdp -I ..\/..\/platform\/securec\/include \\/1'
    else
        SED_CMD=$ROW_NUM_RET's/\\\\/-I ..\/..\/platform\/kmc\/include -I ..\/..\/platform\/kmc\/src\/sdp -I ..\/..\/platform\/securec\/include \\\\/1'
    fi
    echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    sed -n '/..\/openssl\/.openssl\/include\/openssl\/ssl.h/=' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    ROW_NUM_RET=`sed -n '1p' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row`
    if [ $sys = "Linux" -o $sys = "SunOS" ]
    then
        SED_CMD=$ROW_NUM_RET's/\\/..\/..\/platform\/kmc\/include\/wsec_type.h ..\/..\/platform\/kmc\/include\/kmc_itf.h ..\/..\/platform\/kmc\/src\/sdp\/sdp_itf.h ..\/..\/platform\/securec\/include\/securec.h \\/1'
    else
        SED_CMD=$ROW_NUM_RET's/\\\\/..\/..\/platform\/kmc\/include\/wsec_type.h ..\/..\/platform\/kmc\/include\/kmc_itf.h ..\/..\/platform\/kmc\/src\/sdp\/sdp_itf.h ..\/..\/platform\/securec\/include\/securec.h \\\\/1'
    fi
    echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    sed -n '/libcrypto.a/=' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    ROW_NUM_RET=`sed -n '1p' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row`
    if [ $sys = "SunOS" ] 
    then 
        SED_CMD=$ROW_NUM_RET's/libcrypto.a/libcrypto.a -lm ..\/..\/platform\/kmc\/lib\/libKMC.a/1'
    else
        SED_CMD=$ROW_NUM_RET's/libcrypto.a/libcrypto.a ..\/..\/platform\/kmc\/lib\/libKMC.a/1'
    fi
    echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    if [ $sys = "SunOS" ] 
    then
        ROW_NUM_RET=`expr $ROW_NUM_RET + 1`
        SED_CMD=$ROW_NUM_RET's/-m64/-m64 -xmemalign/1'
        echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
        sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
        rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
        rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
        mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    fi
    
    sed -n '/src\/event\/ngx_event_openssl.c/=' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    ROW_NUM_RET=`sed -n '1p' ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row`
    ROW_NUM_RET=`expr $ROW_NUM_RET + 1`
    SED_CMD=$ROW_NUM_RET's/$(CORE_INCS)/$(CORE_INCS) $(HTTP_INCS)/1'
    echo $SED_CMD > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    sed -f ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile > ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Row
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Cmd
    rm -rf ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    mv ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile.bak ${AGENT_ROOT}/open_src/nginx_tmp/objs/Makefile
    
    make $1
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make nginx failed."
        echo "#########################################################"

        exit ${main_result}
    fi

    make install
    main_result=$?
    if [ ${main_result} != 0 ]
    then
        echo "#########################################################"
        echo "   Make install nginx failed."
        echo "#########################################################"

        exit ${main_result}
    fi

    cp -rf ${AGENT_ROOT}/open_src/nginx ${AGENT_ROOT}/bin
    rm -rf ${AGENT_ROOT}/bin/nginx/conf/*
    cp ${AGENT_ROOT}/conf/nginx.conf ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/fastcgi_params ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/server.crt ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/server.key ${AGENT_ROOT}/bin/nginx/conf
    cp ${AGENT_ROOT}/conf/bcmagentca.crt ${AGENT_ROOT}/bin/nginx/conf
    
    #AIX5.3,nginx cannot running if "ssl_session_cache    shared:SSL:1m;" not comment,
    #now, do not known why, after get the really reason modify this
    if [ "$sys" = "AIX" ]
    then
        OS_VERSION=`oslevel | awk -F "." '{print $1$2}'`
        if [ "${OS_VERSION}" = "53" ]
        then
            sed "s/ssl_session_cache/#ssl_session_cache/" ${AGENT_ROOT}/bin/nginx/conf/nginx.conf > ${AGENT_ROOT}/bin/nginx/conf/nginx.conf.bk
            mv ${AGENT_ROOT}/bin/nginx/conf/nginx.conf.bk ${AGENT_ROOT}/bin/nginx/conf/nginx.conf
        fi
    fi
    
    export CFLAGS=${OLD_CFLAGS}
    echo "#########################################################"
    echo "   Make nginx succ."
    echo "#########################################################"
}
make_gtest()
{
    cd ${AGENT_ROOT}/test/obj/
	${CPPC} -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	mkdir gtestlib
	mv libgtest.a gtestlib/
	cp -r ${GTEST_DIR}/include gtestlib/
	rm gtest-all.o
}
make_init()
{
    if [ $# != 0 ]
    then
        if [ "$1" = "coverage" ]
        then
            GCOV_COMPILE_FLAG="-fprofile-arcs -ftest-coverage"
            GCOV_LINK_FLAG="-lgcov -luuid"
            export GCOV_COMPILE_FLAG GCOV_LINK_FLAG
        else
            echo "Please use 'sh test_make.sh [coverage]'"
			exit 2
        fi
	else
        GCOV_COMPILE_FLAG=
        GCOV_LINK_FLAG=
        export GCOV_COMPILE_FLAG GCOV_LINK_FLAG
    fi

    if [ $sys = "AIX" ]
    then
        CPPC=xlC_r
        CC=xlC_r
        cc=xlC_r
        OS_VERSION=`oslevel | awk -F "." '{print $1$2}'`
        if [ "${OS_VERSION}" = "53" ]
        then
            CFLAGS="-g -q64 -DAIX -DAIX53 $KMC_OPT"
        else
            CFLAGS="-g -q64 -DAIX $KMC_OPT"
        fi
        cFLAGS=${CFLAGS}
        OFLAGS="-g -q64 -brtl -bexpall"
        oFLAGS=${OFLAGS}
        DFLAGS="-g -q64 -qmkshrobj -G"
        dFLAGS=${DFLAGS}
    elif [ $sys = "HP-UX" ]
    then
        CPPC=aCC
        CC=aCC
        #for sqlite and securec
        cc=cc
        CFLAGS="-g -w -AA +DD64 -D_REENTRANT -DHP_UX_IA -DHP_UX -mt $KMC_OPT"
        cFLAGS="-g -w +DD64 -D_REENTRANT -DHP_UX_IA -DHP_UX -mt"
        oFLAGS="-g +DD64 -ldl"
        OFLAGS="-g +DD64 -ldl -ldcekt"
        dFLAGS="-g +DD64 -b"
        DFLAGS="-g +DD64 -b -ldcekt"
    elif [ $sys = "Linux" ]
    then
        sysLinuxName=`cat /etc/issue | grep 'Linx'`
        if [ -f /etc/SuSE-release ]
        then
            OS_NAME=SUSE
        elif [ -f /etc/isoft-release ]
        then
            OS_NAME=ISOFT
        elif [ -f /etc/redhat-release ]
        then
            OS_NAME=REDHAT
        elif [ "${sysLinuxName}" != "" ]
        then
            OS_NAME=ROCKY
        elif [ -z "${sysLinuxName}" ]
        then
            sysLinuxName_tmp=`cat /etc/issue | grep 'Rocky'`
            if [ "${sysLinuxName_tmp}" != "" ]
            then
                OS_NAME=ROCKY
                NGINX_CPU_OPT_FLAG=1
            fi
        fi

        if [ ${FORTIFY} -eq 1 ]
        then
            CPPC="sourceanalyzer -b rdagent g++"
            CC="sourceanalyzer -b rdagent gcc"
            cc="sourceanalyzer -b rdagent gcc"
        else
            CPPC="g++"
            CC="gcc"
            cc="gcc"
            if [ ${MST_COVERAGE} -eq 1 ]
            then
                COV_CFLAGS="-fprofile-arcs -ftest-coverage"
                COV_OFLAGS="-ftest-coverage -fprofile-arcs -lgcov"
            fi
        fi
        
        CompareSysVersion
        
        if [ ${FREEZE_SUPPORT} -eq 1 ]
        then
            CFLAGS="-g -pipe -m64 -fpic -DLINUX -DLIN_FRE_SUPP -D${OS_NAME} $KMC_OPT"
        else
            CFLAGS="-g -pipe -m64 -fpic -DLINUX -D${OS_NAME} $KMC_OPT"
        fi
        
        cFLAGS=${CFLAGS}
        OFLAGS="-g -m64 -luuid"
        oFLAGS="-g -m64"
        DFLAGS="-g -m64 -shared -luuid"
        dFLAGS="-g -m64 -shared"
	elif [ $sys = "SunOS" ]
	then
        CPPC=CC
        CC=cc
        #for sqlite and securec
        cc=cc
        CFLAGS="-xmemalign -m64 -D_REENTRANT -mt -g -xcode=pic32 -DSOLARIS $KMC_OPT"
        cFLAGS=$CFLAGS
        OFLAGS="-g -xmemalign -m64 -lsocket -lnsl -luuid"
        oFLAGS=${OFLAGS}
        DFLAGS="-g -xmemalign -m64 -shared -lsocket -lnsl -luuid"
        dFLAGS=${DFLAGS}
    else
        echo "Unsupported OS"
        exit 0
    fi
	
}

make_openssl()
{
	OPENSSL_INSTALL_PATH=${AGENT_ROOT}/open_src/openssl/.openssl
    OPENSSL_OPT="--prefix=$OPENSSL_INSTALL_PATH no-shared no-threads"
	
    if [ "$1" = "clean" ]
    then
        echo "clean openssl"
	elif [ ! -f "$OPENSSL_INSTALL_PATH/lib/libssl.a" ] || [ ! -f "$OPENSSL_INSTALL_PATH/lib/libcrypto.a" ]
	then
	    cd ${AGENT_ROOT}/open_src/openssl
        chmod +x ./config
        chmod +x ./Configure
        if [ $sys = "AIX" ]
        then
            ./Configure aix64-cc $OPENSSL_OPT
        elif [ $sys = "HP-UX" ]
        then
            ./Configure hpux64-ia64-cc $OPENSSL_OPT
		elif [ $sys = "SunOS" ]
	    then
		    sed '248s/-xarch=v9/-m64/g' ${AGENT_ROOT}/open_src/openssl/Configure > ${AGENT_ROOT}/open_src/openssl/Configure.bak
			rm -rf ${AGENT_ROOT}/open_src/openssl/Configure
			mv ${AGENT_ROOT}/open_src/openssl/Configure.bak ${AGENT_ROOT}/open_src/openssl/Configure
			chmod 777 ${AGENT_ROOT}/open_src/openssl/Configure
		    echo >>${AGENT_ROOT}/open_src/openssl/Makefile
		    ./Configure solaris64-sparcv9-cc -xcode=pic32 $OPENSSL_OPT
        else
            ./config -fPIC $OPENSSL_OPT
        fi
	    make $MAKE_JOB $1
		main_result=$?
		if [ ${main_result} != 0 ]
        then
            echo "#########################################################"
            echo "   Compile openssl failed."
            echo "#########################################################"
            exit ${main_result}
		fi
	    make install
		main_result=$?
		if [ ${main_result} != 0 ]
        then
            echo "#########################################################"
            echo "   Compile openssl failed."
            echo "#########################################################"
            exit ${main_result}
		fi
    fi

    echo "#########################################################"
    echo "   Compile openssl succ."
    echo "#########################################################"
}

main_enter()
{
    if [ ${AGENT_ROOT:-0} = 0 ]
    then
        echo "Please source env.csh first"
        exit 2
    else
        create_dir
    fi

    dos2unix_conf_files

    MAKE_OPTION=
    #don't clean third part objs
    if [ ${CLEAN} -eq 1 ]
    then
        MAKE_OPTION="clean"
        make $MAKE_JOB -f ${AGENT_ROOT}/test/build/makefile ${MAKE_OPTION}
    else
        if [ ${CLEAN_ALL} -eq 1 ]
        then
            MAKE_OPTION="clean"
            rm_shell
        else
            copy_shell
        fi
		
        make_openssl ${MAKE_OPTION}
		
		[ ! -f "${AGENT_ROOT}/platform/kmc/kmc_make.sh" ] && cp "${AGENT_ROOT}/build/kmc/kmc_make.sh" "${AGENT_ROOT}/platform/kmc/kmc_make.sh"
        [ ! -f "${AGENT_ROOT}/platform/kmc/makefile_imp" ] && cp "${AGENT_ROOT}/build/kmc/makefile_imp" "${AGENT_ROOT}/platform/kmc/makefile_imp"
		sh ${AGENT_ROOT}/platform/kmc/kmc_make.sh ${MAKE_OPTION}
		main_result=$?
		if [ ${main_result} != 0 ]
        then
            echo "#########################################################"
            echo "   Compile Agent failed."
            echo "#########################################################"
            exit ${main_result}
		fi
		
        prepare_nginx
        #compile nginx first
        make_nginx ${MAKE_OPTION}
        
        make_snmp ${MAKE_OPTION}

        make_fcgi ${MAKE_OPTION}
        
        make_gtest
        
        COMPILE_TIME=`date`
        sed "9s/compile/$COMPILE_TIME/1" ${AGENT_ROOT}/src/inc/common/AppVersion.h >  ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak
        rm -rf ${AGENT_ROOT}/src/inc/common/AppVersion.h
        mv ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak ${AGENT_ROOT}/src/inc/common/AppVersion.h
        
        make -f ${AGENT_ROOT}/test/build/makefile ${MAKE_OPTION}
    fi
}

echo "#########################################################"
echo "   Copyright (C), 2013-2014, Huawei Tech. Co., Ltd."
echo "   Start to compile Agent "
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

make_init $*

if [ $sys = "SunOS" ]
then
    sed 's/-lsnmp++/-lsnmp++ -lresolv/g' makefile > makefile.bak
    mv makefile.bak makefile
fi

if [ ${CLEAN_ALL} -eq 1 ]
then
    delete_open_src_unzipped_files
else
    #unzip all open source project package
    unzip_open_src_packages
    #compile sqlite3 before export env params for hp
    make_sqlite3_file
fi

export CPPC CC cc CFLAGS cFLAGS OFLAGS oFLAGS DFLAGS dFLAGS AGENT_BUILD_NUM
export COV_CFLAGS COV_OFLAGS

main_enter

main_result=$?

if [ ${main_result} != 0 ]
then
    echo "#########################################################"
    echo "   Compile Agent failed."
    echo "#########################################################"

    exit ${main_result}
fi

EndTime=`date '+%Y-%m-%d %H:%M:%S'`
echo "#########################################################"
echo "   Compile Agent completed."
echo "   begin at ${StartTime}"
echo "   end   at ${EndTime}"
echo "#########################################################"

exit $main_result

