#!/bin/sh

# path of all
export AGENT_ROOT=/home/icp/Agent/RD_V200R001C10_Agent_StaticCheck/code/current/Agent
export LOG_DIR=/opt/codeDex/log
export TMP_DIR=/opt/codeDex/tmp
# export FORTIFY_DIR=/opt/codeDex/fortify_dir
# export PATH=/home/icp/plugins/CodeDEX/tool:$PATH


if [ -d $LOG_DIR ] ;
then
	rm -rf  $LOG_DIR/* 2>nul
else
	mkdir -p  $LOG_DIR 2>nul
fi


if [ -d $TMP_DIR ] ;
then
	rm -rf  $TMP_DIR/* 2>nul
else
	mkdir -p  $TMP_DIR 2>nul
fi