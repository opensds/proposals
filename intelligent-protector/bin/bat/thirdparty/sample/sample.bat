@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

rem The parameters %1 is passed by Agent. So Please does not change following script.
set CURR_PATH=%~dp0
set RSTFILE="%CURR_PATH%"\..\..\tmp\RST%1.txt
set LOGFILE="%CURR_PATH%"\..\..\log\sample.log

rem The related business script code need to be here.
rem ########Begin########
echo "Begin to do something." >> %LOGFILE%

echo "Finish doing something." >> %LOGFILE%
rem ########End#######

rem if the operation is successed, need to write blank string into the result file %RSTFILE%. 
rem Otherwise please write material error infomation into the result file %$RSTFILE%.
rem For example,
set /a RSTCODE=%errorlevel%
if %RSTCODE% EQU 0 (
    echo "Here:record success." >> %LOGFILE%
    echo "" > %RSTFILE%
    exit 0
) else (
    echo "Here:record some error." >> %LOGFILE%
    echo %RSTCODE% > %RSTFILE%
    exit 1
)
