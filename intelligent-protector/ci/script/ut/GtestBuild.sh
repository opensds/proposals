#!/bin/sh
set +x

############################################################################################
#program name: auto package UTReport      
#author:
#time:  2016/10/14                        
############################################################################################

#ICP AGENT PATH
CURRENT_DIR=../../../../
#translate to full path name
cd ${CURRENT_DIR}
CURRENT_DIR=`pwd ${CURRENT_DIR}`
cd - >/dev/null
#root switch ${DEFAULT_USER} with a param
PARAM=$1
echo "PARAM=$PARAM"
if [ "$PARAM" != "" ];
then
    CURRENT_DIR=$PARAM
fi
echo "CURRENT_DIR=${CURRENT_DIR}"

#Can't execute this script with root, must set DEFAULT_USER and DEFAULT_DIR.
DEFAULT_USER="zb"
DEFAULT_DIR=/home/zb
HOME_AGENT=${CURRENT_DIR}/Agent
OUTPUT_DIR=${CURRENT_DIR}/UTReport

HOME_TEST_DIR=${HOME_AGENT}/test
HOME_BUILD_DIR=${HOME_TEST_DIR}/build
HOME_BIN_DIR=${HOME_TEST_DIR}/bin

MAKEFILE_SCRIPT="test_make.sh"
MAKE_PARAMS="coverage"
RESULT_LCOV="html.tar.gz"
MAKEFILE_CLEAN_SCRIPT="test_clean.sh"
#LCOV_MAKE_PARAMS="-fprofile-arcs -ftest-coverage"

OUTPUT_GTEST_HOME=${OUTPUT_DIR}/gtest
OUTPUT_LCOV_HOME=${OUTPUT_DIR}/lcov
OUTPUT_ERR_FILE=${OUTPUT_DIR}/log.err
OUTPUT_LOG_FILE=${OUTPUT_DIR}/log
OUTPUT_RUN_LOG=${OUTPUT_DIR}/run.sh.log
OUTPUT_RUN_ERR=${OUTPUT_DIR}/run.sh.err
OUTPUT_GTEST_LOG=${OUTPUT_DIR}/gtest.log
OUTPUT_GTEST_ERR=${OUTPUT_DIR}/gtest.err
OUTPUT_TMP_FILE=${OUTPUT_DIR}/temps

createDir()
{
    if [ ! -d "${OUTPUT_DIR}" ];
    then
        mkdir -p ${OUTPUT_DIR}
    fi
    
    if [[ "${OUTPUT_DIR}" != "" ]] && [[ "${OUTPUT_DIR}" != "/" ]]
    then
        rm -rf "${OUTPUT_DIR}/*"
    fi
    
    if [ ! -d "${OUTPUT_GTEST_HOME}" ];
    then
        mkdir -p ${OUTPUT_GTEST_HOME}
    fi
    
    if [ ! -d "${OUTPUT_LCOV_HOME}" ];
    then
        mkdir -p ${OUTPUT_LCOV_HOME}
    fi
}

toDo()
{
    cd $HOME_BUILD_DIR
    echo "Open ${HOME_BUILD_DIR}."
    
    #backup ${HOME}
    #set AGENT_ROOT in env.sh as HOME_AGENT
    HOME_BAK=${HOME}
    HOME=`dirname ${HOME_AGENT}`
    
    source ${HOME_BUILD_DIR}/env.sh
    
    #recover ${HOME}
    HOME=${HOME_BAK}
    
    sh $HOME_BUILD_DIR/$MAKEFILE_CLEAN_SCRIPT
    
    sh $HOME_BUILD_DIR/$MAKEFILE_SCRIPT $MAKE_PARAMS >$OUTPUT_LOG_FILE 2>$OUTPUT_ERR_FILE
    
    echo "Leave ${HOME_BUILD_DIR}."
    cd - >/dev/null
}


createLcovReport()
{
    echo "Begin create Lcov report."
    if [[ ${OUTPUT_LCOV_HOME} = "" ]] || [[ ${HOME_AGENT} = "" ]]
    then 
        echo "Check path:${OUTPUT_LCOV_HOME}, ${HOME_AGENT} is illegal"
        return 1;
    fi
    rm -rf ${OUTPUT_LCOV_HOME}/html
    rm -rf ${HOME_AGENT}/test/html
    
    #set AGENT_ROOT in run.sh as HOME_AGENT
    HOME_BAK=${HOME}
    HOME=`dirname ${HOME_AGENT}`
    #if you just wanna run your own binary file
    #you shuld change run.sh to your own binary file name
    sh ${HOME_AGENT}/test/bin/run.sh >${OUTPUT_RUN_LOG} 2>${OUTPUT_RUN_ERR}
    #recover ${HOME}
    HOME=${HOME_BAK}
    
    mkdir -p ${HOME_AGENT}/test/temp
    lcov -d ${HOME_AGENT} -o ${HOME_AGENT}/test/temp/total.info -b ${HOME_AGENT} -c
    genhtml -o ${HOME_AGENT}/test/html ${HOME_AGENT}/test/temp/total.info
    gensummary ${HOME_AGENT}/test/temp/total.info ${HOME_AGENT}/test/summary.xml
    rm -rf ${HOME_AGENT}/test/temp/
    find ${HOME_AGENT}/test/obj -name "*.gcno" | xargs rm -f
    find ${HOME_AGENT}/test/obj -name "*.gcda" | xargs rm -f
    cd - >/dev/null
    
    cd $HOME_TEST_DIR
    tar -zcvf ${OUTPUT_LCOV_HOME}/html.tar.gz html
    mv summary.xml ${OUTPUT_LCOV_HOME}
    echo "End create Lcov report."
    cd - >/dev/null
}

createGtestReport()
{
    cd $HOME_BIN_DIR
    echo "Open ${HOME_BIN_DIR}."
    
    export LD_LIBRARY_PATH=${HOME_BIN_DIR}:${LD_LIBRARY_PATH}
    chmod a+x *

    ls -l ${HOME_BIN_DIR} > ${OUTPUT_TMP_FILE};
    pwd
    
    echo "" >${OUTPUT_GTEST_LOG} 2>${OUTPUT_GTEST_ERR}
    while read line
    do 
        if [[ ${line:0:1} = "-" ]] && [[ ${line:3:1} = "x" ]];
        then 
            script=`echo $line | awk '{print $NF}'`
            if [[ ${script} = "run.sh" ]] || [[ ${script} = "libtest.so" ]];
                then continue;
            fi
            echo "*******************************************************************"
            echo "Output: ${OUTPUT_GTEST_HOME}/report_${script}.xml"
            echo "*******************************************************************"
            ${HOME_BIN_DIR}/${script} --gtest_output=xml:${OUTPUT_GTEST_HOME}/report_${script}.xml >>${OUTPUT_GTEST_LOG} 2>>${OUTPUT_GTEST_ERR}
            
            #avoid error in AgentArrayTest
            if [ "${script}" = "AgentArrayTest" ]
            then
                sed 's/classname="CArrayTest"r/classname="CArrayTest"/g' ${OUTPUT_GTEST_HOME}/report_${script}.xml > ${OUTPUT_GTEST_HOME}/report_${script}.tmp

                sed 's/classname="CDiskTest"r/classname="CDiskTest"/g' ${OUTPUT_GTEST_HOME}/report_${script}.tmp > ${OUTPUT_GTEST_HOME}/report_${script}.xml

                rm ${OUTPUT_GTEST_HOME}/report_${script}.tmp
            fi
        fi;
    done < ${OUTPUT_TMP_FILE};
    rm ${OUTPUT_TMP_FILE};
    
    echo "Leave ${HOME_BIN_DIR}."
    cd - >/dev/null
}

if [ $UID -eq 0 ];
then
    echo "Can not run this script with root."
    echo "Switch switch to ${DEFAULT_USER}, now"    
    SCRIPT_NAME="script.sh"
    cp $0 "${DEFAULT_DIR}/${SCRIPT_NAME}"
    
    if [[ "${DEFAULT_DIR}" = "" ]] || [[ "${DEFAULT_DIR}" = "/"  ]] || [[ "${SCRIPT_NAME}" = "" ]] || [[ "${SCRIPT_NAME}" = "/"  ]]
    then
        echo "Check path:${DEFAULT_DIR}/${SCRIPT_NAME} is illegal"
        exit 1
    fi
    chown -R ${DEFAULT_USER}  "${DEFAULT_DIR}/${SCRIPT_NAME}"
    
    if [[ "${HOME_AGENT}" = "" ]] || [[ "${HOME_AGENT}" = "/"  ]]
    then
        echo "Check path:${HOME_AGENT} is illegal"
        exit 1
    fi
    chown -R ${DEFAULT_USER} ${HOME_AGENT}
    
    PARENT_DIR_NAME=`dirname ${OUTPUT_DIR}`
    if [[ "${PARENT_DIR_NAME}" = "" ]] || [[ "${PARENT_DIR_NAME}" = "/"  ]]
    then
        echo "Check path:${PARENT_DIR_NAME} is illegal"
        exit 1
    fi
    chown -R ${DEFAULT_USER} ${PARENT_DIR_NAME}
    
    su - ${DEFAULT_USER} -c "sh ~/${SCRIPT_NAME} ${CURRENT_DIR}"
    echo "Logout ${DEFAULT_USER}"
    exit 1
fi

createDir

toDo

createGtestReport

createLcovReport
