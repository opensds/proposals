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

set QUERY_FILE="%CURR_PATH%\..\..\tmp\querystate%1.sql"
set QUERYRST_FILE="%CURR_PATH%\..\..\tmp\querystateRST%1.txt"

rem The related business script code need to be here.
rem ########Begin########
echo "Begin to do query freeze state." >> %LOGFILE%
if exist %PARAM_FILE% (del /f /q %PARAM_FILE%)

rem if the operation is successed, need to write blank string into the result file %RSTFILE%. 
rem Otherwise please write material error infomation into the result file %$RSTFILE%.
rem For example,
set ORACLE_SID=dbFS
echo select count(*) from v$backup where status='ACTIVE'; > %QUERY_FILE%
echo exit; >> %QUERY_FILE%
sqlplus / as sysdba @%QUERY_FILE% > %QUERYRST_FILE%
if not exist %QUERYRST_FILE% (
    echo "Exec SQL failed." >> %LOGFILE%
    if exist %QUERY_FILE% (del /f /q %QUERY_FILE%)
    if exist %QUERYRST_FILE% (del /f /q %QUERYRST_FILE%)
    endlocal
    exit 1
) else (
    type %QUERYRST_FILE% | findstr "ERROR" >nul
    if !errorlevel! EQU 0 (
        echo "=====================Database execsql failed=====================" >> %LOGFILE%
        type %QUERYRST_FILE% >> %LOGFILE%
        echo "=====================Database execsql failed=====================" >> %LOGFILE%
        if exist %QUERY_FILE% (del /f /q %QUERY_FILE%)
        if exist %QUERYRST_FILE% (del /f /q %QUERYRST_FILE%)
        endlocal
        exit 1
    )
    
    set /a FLG_FIND=0
    set /a ACTIVECOUNT=0
    for /f "skip=9" %%i in ('type %QUERYRST_FILE%') do (
        if "!FLG_FIND!" EQU "1" (
            set /a ACTIVECOUNT=%%i
            if exist %QUERY_FILE% (del /f /q %QUERY_FILE%)
            if exist %QUERYRST_FILE% (del /f /q %QUERYRST_FILE%)
            echo "ACTIVECOUNT=%%i" >> %LOGFILE%
            goto :ENDFIND
        )
        
        echo %%i | findstr \-\-\-\-\-\-
        if "!errorlevel!" EQU "0" (
            set /a FLG_FIND=1
        )
    )
    
    if exist %QUERY_FILE% (del /f /q %QUERY_FILE%)
    if exist %QUERYRST_FILE% (del /f /q %QUERYRST_FILE%)
:ENDFIND
    if "!ACTIVECOUNT!" EQU "0" (
        echo "There are no backup tablespace." >> %LOGFILE%
        echo 1 > %RSTFILE%
    ) else (
        echo "Database is in hot backup mode-!ACTIVECOUNT!." >> %LOGFILE%
        echo 0 > %RSTFILE%
    )
)

echo "Finish doing query freeze state." >> %LOGFILE%
endlocal
exit 0
rem ########End#######
