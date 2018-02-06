@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


set AGENT_ROOT_PATH=%~1
set ID=%~2

set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\log\
set AGENT_TMP_PATH=%AGENT_ROOT_PATH%\tmp\
set NGINX_LOG_PATH=%AGENT_BIN_PATH%\nginx\logs\
set ZIP_TOOL=%AGENT_BIN_PATH%\7z.exe
set INPUT_TMP_FILE=%AGENT_TMP_PATH%input_tmp%ID%
set LOG_FILE_NAME="%AGENT_LOG_PATH%packlog.log"

call :Log "Begin to package log."
if not exist "%INPUT_TMP_FILE%" (
    call :Log "input_tmp%ID% is not exists."
    exit /b 1
)

for /f "delims=" %%i in ('type "%INPUT_TMP_FILE%"') do (
    if not "%%i" == "" (
        set LOG_FOLDER=%%i
    )
)

SET PACKAGE_LOG=%AGENT_TMP_PATH%%LOG_FOLDER%

del /f /q "%INPUT_TMP_FILE%"

mkdir "%PACKAGE_LOG%"
mkdir "%PACKAGE_LOG%\nginx_log"
mkdir "%PACKAGE_LOG%\agent_log"

if exist "%NGINX_LOG_PATH%"    copy /y  "%NGINX_LOG_PATH%"*.log*   "%PACKAGE_LOG%\nginx_log\"  >Nul
if exist "%AGENT_LOG_PATH%"    copy /y  "%AGENT_LOG_PATH%"*.log*   "%PACKAGE_LOG%\agent_log\"  >Nul

call :Log "Compress agent log."
"%ZIP_TOOL%"  a -tzip "%PACKAGE_LOG%.zip"  "%PACKAGE_LOG%" -mx=9 >Nul

rmdir  /s/q   "%PACKAGE_LOG%"
call :Log "Finish packaging log."
exit /b 0

rem ************************************************************************
rem function name: Log
rem aim:           Print log function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOG_FILE_NAME%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOG_FILE_NAME%
    goto :EOF