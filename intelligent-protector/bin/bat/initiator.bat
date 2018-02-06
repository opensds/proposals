@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

rem ******Enable Delayed Expansion******
setlocal EnableDelayedExpansion
rem  ******************************************************************************************
rem  *program name:          queryinitiator.bat   
rem  *function:              query FC WWM or iscsi initator name for windows
rem  *author:                
rem  *time:                  2011-08-17
rem  *function and description:
rem  *rework:               First Programming
rem  *author:
rem  *time:
rem  *explain:
rem  ******************************************************************************************
rem fcinfo.exe download ulr:http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=17530

rem ********************** var **********************
set AGENT_ROOT_PATH=%1
set RANDOM_ID=%2

set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\log\
set AGENT_TMP_PATH=%AGENT_ROOT_PATH%\tmp\
set FCINFO_BIN_PATH=C:\Windows\System32\

set NEED_LOG_FLG=1
set ERROR_SCRIPT_EXEC_FAILED=5
set EXIT_CODE_NUM=0

rem *********************set file name*********************
set SYS_TEM_PATH=%windir%\system32
set LOG_FILE_PATH=%AGENT_LOG_PATH%initiator.log
set RESULT_FILE=%AGENT_TMP_PATH%result_tmp%RANDOM_ID%
set OSINFO_32=32_fcinfo
set OSINFO_64=64_fcinfo
set FC_INFO_PATH=""
set INPUTINFO=""
set ALL_INFO=%AGENT_TMP_PATH%AI%RANDOM_ID%
set SYS_TEM_INFO=%windir%\SysWOW64
set ISCSI_TMP_FILE=%AGENT_TMP_PATH%iscsitmpfile%RANDOM_ID%
set INPUT_TMP_FILE=%AGENT_TMP_PATH%input_tmp%RANDOM_ID%

if exist %RESULT_FILE% (del /f /q %RESULT_FILE%)

for /f "delims=" %%i in ('type %INPUT_TMP_FILE%') do (
    if not "%%i" == "" (
        set INPUTINFO="%%i"
    )
)
if exist %INPUT_TMP_FILE% (del /f /q %INPUT_TMP_FILE%)

echo INFO: Input info:!INPUTINFO!
call :Log "INFO: Input info:%INPUTINFO%"
if !INPUTINFO!=="iscsis" (
    echo "INFO: Input info:%INPUTINFO%"
    goto ISCSIFLAG
)

if !INPUTINFO!=="fcs" (
    echo "INFO: Input info:%INPUTINFO%"
    goto FCFLAG
)

set EXIT_CODE_NUM=1
goto EXITFLAG
rem **********************get fc info**********************
:FCFLAG
echo "INFO: Begin query fc info."
call :log "INFO: Begin query fc info."

if "%PROCESSOR_ARCHITECTURE%" == "x86" (
    set FC_INFO_PATH=%OSINFO_32%
    echo x86 system
) else (
    set FC_INFO_PATH=%OSINFO_64%
    echo x64 system
)

rem change path, fcinfo locate bin path
set FC_INFO_PATH=""

if not exist "%FCINFO_BIN_PATH%fcinfo.exe" (
    echo "ERROR: %FCINFO_BIN_PATH%fcinfo.exe not exits"
    call :Log "ERROR: %FCINFO_BIN_PATH%fcinfo.exe not exits"
    set EXIT_CODE_NUM=%ERROR_SCRIPT_EXEC_FAILED%
    goto EXITFLAG
)


rem **********************exec command**********************
"%FCINFO_BIN_PATH%fcinfo.exe" /details > %ALL_INFO%
if !errorlevel! NEQ 0 (
    echo "ERROR: No HBA in your system"
    call :Log "ERROR: No HBA in your system"
    set EXIT_CODE_NUM=1
    goto EXITFLAG
)

rem **********************get hba info**********************
set ADAPTER_NAME=--
set TMP_WWN_NUMBER=--
set INITIATOR_NUM=--

for /f "tokens=1 delims=" %%a in ('type %ALL_INFO%') do (
    set LINE=%%a
    set FLAG=--
    set VALUES=--
    
    for /f "tokens=1,2 delims=:" %%i in ("!LINE!") do (
        set FLAG=%%i
        set FLAG=!FLAG: =!
        set VALUES=%%j
    )
     
    if !FLAG! == adapter (
        set ADAPTER_NAME=!VALUES!
        set ADAPTER_NAME=!ADAPTER_NAME: =!
        echo ADAPTER_NAME:!ADAPTER_NAME!
        call :Log "INFO: adapter:!ADAPTER_NAME!"
    )
    
    if !FLAG! == port_wwn (
        set TMPWWN=!LINE!
        set TMPWWN=!TMPWWN: =!
        set VALUES=!TMPWWN:~9!
        set TMP_WWN_NUMBER=!VALUES!
        set INITIATOR_NUM=!TMP_WWN_NUMBER::=!

        echo INITIATOR_NUM=:!INITIATOR_NUM!
        call :Log "INFO: port_wwn:!INITIATOR_NUM!"
        
        echo !INITIATOR_NUM! >> %RESULT_FILE% 
    )
)

goto EXITFLAG

rem **********************get iscsi info**********************
:ISCSIFLAG

echo "INFO: Begin query iscsi initiator."
call :Log "INFO: Begin query iscsi initiator."

if exist %ALL_INFO% (del /f /q %ALL_INFO%)

if exist %ISCSI_TMP_FILE% (del /f /q %ISCSI_TMP_FILE%)

set INITIATOR_NUM=--
if not exist %SYS_TEM_PATH%\iscsicli.exe (
    echo No iscsi initiator in your system
    call :Log "ERROR: No iscsi initiator in your system"
    set EXIT_CODE_NUM=%ERROR_SCRIPT_EXEC_FAILED%
    goto EXITFLAG
)

echo ^^c  > %ISCSI_TMP_FILE%
iscsicli.exe > %ALL_INFO% < %ISCSI_TMP_FILE%
if !errorlevel! NEQ 0 (
    call :Log "ERROR: The Microsoft iSCSI initiator service has not been started."
    set EXIT_CODE_NUM=1
    goto EXITFLAG
)

set index=0

for /f "tokens=1 delims=]" %%i in ('type %ALL_INFO%') do (
    set INITIATOR_NUM=%%i
    set /a index=index+1
    set INITIATOR_NUM=!INITIATOR_NUM:[=!
    set TMPNAME=!INITIATOR_NUM!
    set TMPNAME=!TMPNAME:~0,3!
    if !index!==2 (
    echo iSCSI initiator name:!INITIATOR_NUM!
    echo !INITIATOR_NUM! >> %RESULT_FILE%
    call :Log "INFO: iSCSI initiator name:!INITIATOR_NUM!"
    goto EXITFLAG
     ) 
)
    
:EXITFLAG
if exist %ALL_INFO% (del /f /q %ALL_INFO%)
if exist %ISCSI_TMP_FILE% (del /f /q %ISCSI_TMP_FILE%)

if "%EXIT_CODE_NUM%"=="0" (
    echo "INFO: Query initiator %INPUTINFO%: %INITIATOR_NUM% succ."
    call :Log "INFO: Query initiator %INPUTINFO%: %INITIATOR_NUM% succ."
) else (
    echo "ERROR: Query initiator %INPUTINFO% failed."
    call :Log "ERROR: Query initiator %INPUTINFO% failed."
)

exit /b %EXIT_CODE_NUM%

:Log
    if %NEED_LOG_FLG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> %LOG_FILE_PATH%
    )
    call %AGENT_BIN_PATH%agent_func.bat %LOG_FILE_PATH%
goto :EOF
