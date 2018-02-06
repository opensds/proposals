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

#gcc secure opt
NX_OPT="-Wl,-z,noexecstack"
SP_OPT="-fstack-protector --param ssp-buffer-size=4 -Wstack-protector"
RELRO_OPT="-Wl,-z,relro,-z,now"
RPATH_OPT="-Wl,--disable-new-dtags,--rpath=/lib64:/usr/lib64" 

AWK=awk

AGENT_VERSION=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "define AGENT_VERSION" | $AWK -F '"' '{print $2}'`
AGENT_BUILD_NUM=`cat ${AGENT_ROOT}/src/inc/common/AppVersion.h | grep "define AGENT_BUILD_NUM" | $AWK -F '"' '{print $2}'`

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
    fi
    #sqlite
    if [ ! -d ${OPEN_SRC_SQLITE_DIR} ]
    then
        unzip ${OPEN_SRC_SQLITE}
        UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SQLITE} .zip`
        mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_SQLITE_DIR}
        mv ${AGENT_ROOT}/src/patch/sqlite/os_unix.c ${AGENT_ROOT}/open_src/sqlite/src/os_unix.c
        mv ${AGENT_ROOT}/src/patch/sqlite/rtree.c ${AGENT_ROOT}/open_src/sqlite/ext/rtree/rtree.c
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
    mkdir -p ${AGENT_ROOT}/obj/apps/oracle
    mkdir -p ${AGENT_ROOT}/obj/apps/cache
    mkdir -p ${AGENT_ROOT}/obj/apps/sybase
    mkdir -p ${AGENT_ROOT}/obj/apps/hana
    mkdir -p ${AGENT_ROOT}/obj/apps/app
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
    mkdir -p ${AGENT_ROOT}/obj/plugins/cache
    mkdir -p ${AGENT_ROOT}/obj/plugins/sybase
    mkdir -p ${AGENT_ROOT}/obj/plugins/hana
    mkdir -p ${AGENT_ROOT}/obj/plugins/app
    mkdir -p ${AGENT_ROOT}/obj/rest
    mkdir -p ${AGENT_ROOT}/obj/tools
    mkdir -p ${AGENT_ROOT}/obj/securec
    mkdir -p ${AGENT_ROOT}/obj/kmc
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
        if [ "${FILE_NAME}" = "KMC_Crt.conf" ]
        then
            continue
        fi
        
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

    export CXXFLAGS="-pipe -m64 -fpic"
    
    export ssl_CFLAGS="-I${AGENT_ROOT}/open_src/openssl/include"
    export ssl_LIBS="-lssl -lcrypto"
    export LDFLAGS="-L${AGENT_ROOT}/open_src/openssl"
    ./configure --with-ssl --disable-logging --disable-namespace --enable-shared=no

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
    
    ./configure --prefix=${AGENT_ROOT}/open_src/fcgi/.fcgi --disable-shared "$FCGI_CP_OPT"

    rm -rf ${AGENT_ROOT}/open_src/fcgi/.fcgi
    
    cp -rf ${AGENT_ROOT}/src/patch/fcgi/fcgio.h ${AGENT_ROOT}/open_src/fcgi/include/

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

    echo "#########################################################"
    echo "   Start to make Nginx."
    echo "#########################################################"
    NGINX_CFG_OPT="`which gcc`"
    cd ${AGENT_ROOT}/open_src/nginx_tmp/
    if [  ${NGINX_CPU_OPT_FLAG} -eq 1 ]
    then
        export CFLAGS="${CFLAGS} -Werror"
    fi

    auto/configure --prefix=../nginx --sbin-path=../nginx --without-http_rewrite_module --without-http_gzip_module --with-http_ssl_module --with-openssl=../openssl --with-openssl-opt=-fPIC --with-cc=$NGINX_CFG_OPT

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
    #rename nginx to rdnginx
    mv ${AGENT_ROOT}/bin/nginx/nginx ${AGENT_ROOT}/bin/nginx/rdnginx
    
    export CFLAGS=${OLD_CFLAGS}
    echo "#########################################################"
    echo "   Make nginx succ."
    echo "#########################################################"
}

make_init()
{
    if [ $# != 0 ]
    then
        if [ "$1" = "clean" ]
        then
            if [ $# = 2 ]
            then
                if [ "$2" = "all" ]
                then
                    CLEAN_ALL=1
                else
                    echo "Invalid make option, 'make clean [all]'."
                    exit 2
                fi
            else
                CLEAN=1
            fi

        elif [ "$1" = "fortify" ]
        then
            FORTIFY=1
        elif [ "$1" = "mst_coverage" ] #module system test
        then 
            MST_COVERAGE=1
        else
            echo "Invalid make option, support clean or fortify only."
            exit 2
        fi
    fi

    if [ $sys = "Linux" ]
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
        
        # add define STDCXX_98_HEADERS, support for snmp++ 3.3.7
        if [ ${FREEZE_SUPPORT} -eq 1 ]
        then
            CFLAGS="-g -pipe -m64 -fpic -DLINUX -DLIN_FRE_SUPP -D${OS_NAME} -DSTDCXX_98_HEADERS"
        else
            CFLAGS="-g -pipe -m64 -fpic -DLINUX -D${OS_NAME} -DSTDCXX_98_HEADERS"
        fi
        
        cFLAGS=${CFLAGS}
        OFLAGS="-g -m64 -luuid $NX_OPT $SP_OPT $RELRO_OP $RPATH_OPT"
        oFLAGS="-g -m64 $NX_OPT $SP_OPT $RELRO_OP $RPATH_OPT"
        DFLAGS="-g -m64 -shared -luuid"
        dFLAGS="-g -m64 -shared"
    else
        echo "Unsupported OS"
        exit 0
    fi
}

make_openssl()
{
    OPENSSL_INSTALL_PATH=${AGENT_ROOT}/open_src/openssl/.openssl
    #the openssl 1.0.2 version need compile with dynamic file for solaris, so modify option "no-shared" to "shared"
    OPENSSL_OPT="--prefix=$OPENSSL_INSTALL_PATH no-shared no-threads"
	
    if [ "$1" = "clean" ]
    then
        echo "clean openssl"
    elif [ ! -f "$OPENSSL_INSTALL_PATH/lib/libssl.a" ] || [ ! -f "$OPENSSL_INSTALL_PATH/lib/libcrypto.a" ]
    then
        cd ${AGENT_ROOT}/open_src/openssl
        chmod +x ./config
        chmod +x ./Configure
        
        ./config -fPIC $OPENSSL_OPT
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
        make $MAKE_JOB -f ${AGENT_ROOT}/build/makefile ${MAKE_OPTION}
    else
        if [ ${CLEAN_ALL} -eq 1 ]
        then
            MAKE_OPTION="clean"
            rm_shell
        else
            copy_shell
        fi

        make_openssl ${MAKE_OPTION}
            
        make_nginx ${MAKE_OPTION}
        
        make_snmp ${MAKE_OPTION}

        make_fcgi ${MAKE_OPTION}
        
        COMPILE_TIME=`date`
        sed "9s/compile/$COMPILE_TIME/1" ${AGENT_ROOT}/src/inc/common/AppVersion.h >  ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak
        rm -rf ${AGENT_ROOT}/src/inc/common/AppVersion.h
        mv ${AGENT_ROOT}/src/inc/common/AppVersion.h.bak ${AGENT_ROOT}/src/inc/common/AppVersion.h
        
        export SYSTEM=$sys
        make $MAKE_JOB -f ${AGENT_ROOT}/build/makefile ${MAKE_OPTION}
    fi
}

compile_gcov_out()
{
    if [ "$MST_COVERAGE" = "1" ]
    then 
        ${CC} -shared -fPIC ${AGENT_ROOT}/test/auto/src/gcov_out.c -o ${AGENT_ROOT}/obj/gcov_out.so
    fi
}

echo "#########################################################"
echo "   Copyright (C), 2013-2014, Huawei Tech. Co., Ltd."
echo "   Start to compile Agent "
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

make_init $*

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

compile_gcov_out

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

