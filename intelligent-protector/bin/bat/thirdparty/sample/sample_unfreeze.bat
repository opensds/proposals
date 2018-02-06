@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal EnableDelayedExpansion
rem The parameters %1 is passed by Agent. So Please does not change following script.
set CURR_PATH=%~dp0
set RSTFILE="%CURR_PATH%"\..\..\tmp\RST%1.txt
set LOGFILE="%CURR_PATH%"\..\..\log\sample.log
set PARAM_FILE="%CURR_PATH%"\..\..\tmp\input_tmp%1

set UNFREEZE_FILE="%CURR_PATH%\..\..\tmp\unfreezedb%1.sql"
set UNFREEZERST_FILE="%CURR_PATH%\..\..\tmp\unfreezedbRST%1.txt"

rem The related business script code need to be here.
rem ########Begin########
echo "Begin to do unfreeze db." >> %LOGFILE%
if exist %PARAM_FILE% (del /f /q %PARAM_FILE%)

rem if the operation is successed, need to write blank string into the result file %RSTFILE%. 
rem Otherwise please write material error infomation into the result file %$RSTFILE%.
rem For example,
set ORACLE_SID=dbFS
echo alter database end backup; > %UNFREEZE_FILE%
echo exit; >> %UNFREEZE_FILE%
sqlplus / as sysdba @%UNFREEZE_FILE% > %UNFREEZERST_FILE%
if not exist %UNFREEZERST_FILE% (
    echo "Exec SQL failed." >> %LOGFILE%
    if exist %UNFREEZE_FILE% (del /f /q %UNFREEZE_FILE%)
    if exist %UNFREEZERST_FILE% (del /f /q %UNFREEZERST_FILE%)
    endlocal
    exit 1
) else (
    type %UNFREEZERST_FILE% | findstr "ERROR" >nul
    if !errorlevel! EQU 0 (
        echo "=====================Database execsql failed=====================" >> %LOGFILE%
        type %UNFREEZERST_FILE% >> %LOGFILE%
        echo "=====================Database execsql failed=====================" >> %LOGFILE%
        if exist %UNFREEZE_FILE% (del /f /q %UNFREEZE_FILE%)
        if exist %UNFREEZERST_FILE% (del /f /q %UNFREEZERST_FILE%)
        endlocal
        exit 1
    )
    if exist %UNFREEZE_FILE% (del /f /q %UNFREEZE_FILE%)
    if exist %UNFREEZERST_FILE% (del /f /q %UNFREEZERST_FILE%)
)

echo "Finish doing unfreeze db." >> %LOGFILE%
endlocal
exit 0
rem ########End#######
