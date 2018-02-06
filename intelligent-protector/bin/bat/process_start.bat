@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal EnableDelayedExpansion

set CURBAT_PATH=%~dp0
set SCRIPTNAME=%~nx0
set NEED_LOG=1
set AGENT_BIN_PATH=%CURBAT_PATH%
set LOGFILENAME="%CURBAT_PATH%\..\log\agent_start.log"

set USAGE= Usage: "agent_start" or "agent_start rdagent" or "agent_start monitor" or "agent_start nginx" or "agent_start provider"

if not "%2" == "" goto other
if "%1" == "nginx" goto startnginx
if "%1" == "rdagent" goto startagent
if "%1" == "monitor" goto startmonitor

if "%1" == "" (
    call :startnginx
    call :startagent
    call :startmonitor
)

exit /b 0
    

:startagent
    for /f "delims=" %%i in ('call tasklist ^| findstr rdagent.exe') do (set AGENT_CHECK=%%i)
    
    if "!AGENT_CHECK!" == "" (
        start /b rdagent.exe
    ) else (
        call :Log "Process rdagent of OceanStor BCManager Agent is exist."
        echo Process rdagent of OceanStor BCManager Agent is exist.
    )
    
    set AGENT_CHECK=
    timeout /T 1 /NOBREAK >nul
    for /f "delims=" %%i in ('call tasklist ^| findstr rdagent.exe') do (set AGENT_CHECK=%%i)
    if "!AGENT_CHECK!" == "" (
        call :Log "Process rdagent of OceanStor BCManager Agent start failed."
        echo Process rdagent of OceanStor BCManager Agent start failed.
        exit 1
    ) else (
        call :Log "Process rdagent of OceanStor BCManager Agent successful."
        echo Process rdagent of OceanStor BCManager Agent start successful.
    )
    goto :EOF
    
:startmonitor
    for /f "delims=" %%i in ('call tasklist ^| findstr monitor.exe') do (set MONITOR_CHECK=%%i)
    
    if "!MONITOR_CHECK!" == "" (
        start /b monitor.exe
    ) else (
        call :Log "Process monitor of OceanStor BCManager Agent is exist."
        echo Process monitor of OceanStor BCManager Agent is exist.
    )
    
    set MONITOR_CHECK=
    timeout /T 1 /NOBREAK >nul
    for /f "delims=" %%i in ('call tasklist ^| findstr monitor.exe') do (set MONITOR_CHECK=%%i)
    if "!MONITOR_CHECK!" == "" (
        call :Log "Process monitor of OceanStor BCManager Agent start failed."
        echo Process monitor of OceanStor BCManager Agent start failed.
        exit 1
    ) else (
        call :Log "Process monitor of OceanStor BCManager Agent start successful."
        echo Process monitor of OceanStor BCManager Agent start successful.
    )
    goto :EOF
    
:startnginx
    for /f "delims=" %%i in ('call tasklist ^| findstr rdnginx.exe') do (set NGINX_CHECK=%%i)
    
    if not "!NGINX_CHECK!" == "" (
        call :Log "Process nginx of OceanStor BCManager Agent is exist."
        echo Process nginx of OceanStor BCManager Agent is exist.
        goto :EOF
    )
    
    cd /d "%CURBAT_PATH%nginx"
    start /b rdnginx.exe
    timeout /T 3 /NOBREAK >nul
    
    set NGINX_CHECK=
    for /f "delims=" %%i in ('call tasklist ^| findstr rdnginx.exe') do (set NGINX_CHECK=%%i)
    if "!NGINX_CHECK!" == "" (
        call :Log "Process nginx of OceanStor BCManager Agent start failed."
        echo Process nginx of OceanStor BCManager Agent start failed.
        exit 1
    )
    
    
    call :Log "Process nginx of OceanStor BCManager Agent start successful."
    echo Process nginx of OceanStor BCManager Agent start successful.
    
    goto :EOF

:other
    echo %USAGE%

:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> %LOGFILENAME%
    )
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILENAME%
    goto :EOF

:end
    endlocal