#! /bin/sh

AGENT_ROOT=${HOME}/Agent
AGENT_TEST_BIN=${AGENT_ROOT}/test/bin
export LD_LIBRARY_PATH=${AGENT_ROOT}/bin:${LD_LIBRARY_PATH}

ls -l ${AGENT_TEST_BIN} > temp
while read line 
do 
	#第一个字符为-，表示为普通文件第四个字符为x表示可执行
	if [[ ${line:0:1} = "-" ]] && [[ ${line:3:1} = "x" ]];
	then 
		script=`echo $line | awk '{print $NF}'`
		if [[ ${script} = "run.sh" ]] || [[ ${script} = "libtest.so" ]];
			then continue;
		fi;
		${AGENT_TEST_BIN}/${script}
	fi;
done < temp;
rm temp;

