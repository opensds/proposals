@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal EnableDelayedExpansion

set NEED_LOG=1
set CURBAT_PATH=%~dp0
set SCRIPTNAME=%~nx0
set AGENT_BIN_PATH=%CURBAT_PATH%
set LOGFILENAME="%CURBAT_PATH%..\log\agent_stop.log"

set USAGE= Usage: "agent_stop" or "agent_stop rdagent" or "agent_stop monitor" or "agent_stop nginx" or "agent_stop provider"

if not "%2" == "" goto other
if "%1" == "rdagent" goto stopagent
if "%1" == "monitor" goto stopmonitor
if "%1" == "nginx" goto stopnginx

if "%1" == "" (
    call :stopmonitor
    call :stopnginx
    call :stopagent
)


exit /b 0

:stopagent
    set AGENT_CHECK=
    for /f "delims=" %%i in ('call tasklist ^| findstr rdagent.exe') do (set AGENT_CHECK=%%i)
    
    if "!AGENT_CHECK!" == "" (
        call :Log "Process rdagent of OceanStor BCManager Agent is not exist."
        echo Process rdagent of OceanStor BCManager Agent is not exist.
    ) else (
        call taskkill /f /im rdagent.exe
    )
    
    set AGENT_CHECK=
    timeout /T 1 /NOBREAK >nul
    for /f "delims=" %%i in ('call tasklist ^| findstr rdagent.exe') do (set AGENT_CHECK=%%i)
    if "!AGENT_CHECK!" == "" (
        call :Log "Process rdagent of OceanStor BCManager Agent stop successful."
        echo Process rdagent of OceanStor BCManager Agent stop successful.
    ) else (
        call :Log "Process rdagent of OceanStor BCManager Agent stop failed."
        echo Process rdagent of OceanStor BCManager Agent failed.
        exit 1
    )
    goto :EOF
    
:stopmonitor
    set MONITOR_CHECK=
    for /f "delims=" %%i in ('call tasklist ^| findstr monitor.exe') do (set MONITOR_CHECK=%%i)
    
    if "!MONITOR_CHECK!" == "" (
        call :Log "Process monitor of OceanStor BCManager Agent is not exist."
        echo Process monitor of OceanStor BCManager Agent is not exist.
    ) else (
        call taskkill /f /im monitor.exe
    )
    
    set MONITOR_CHECK=
    timeout /T 1 /NOBREAK >nul
    for /f "delims=" %%i in ('call tasklist ^| findstr monitor.exe') do (set MONITOR_CHECK=%%i)
    if "!MONITOR_CHECK!" == "" (
        call :Log "Process monitor of OceanStor BCManager Agent stop successful."
        echo Process monitor of OceanStor BCManager Agent stop successful.
    ) else (
        call :Log "Process monitor of OceanStor BCManager Agent stop failed."
        echo Process monitor of OceanStor BCManager Agent stop failed.
        exit 1
    )
    goto :EOF
    
:stopnginx
    set NGINX_CHECK=
    for /f "delims=" %%i in ('call tasklist ^| findstr rdnginx.exe') do (set NGINX_CHECK=%%i)
    
    if "!NGINX_CHECK!" == "" (
        call :Log "Process nginx of OceanStor BCManager Agent is not exist."
        echo Process nginx of OceanStor BCManager Agent is not exist.
    ) else (
        call taskkill /f /im rdnginx.exe
    )
    
    rem waite 3 sec after query nginx
    timeout /T 3 /NOBREAK >nul
    set NGINX_CHECK=
    for /f "delims=" %%i in ('call tasklist ^| findstr rdnginx.exe') do (set NGINX_CHECK=%%i)
    if "!NGINX_CHECK!" == "" (
        call :Log "Process nginx of OceanStor BCManager Agent stop successful."
        echo Process nginx of OceanStor BCManager Agent stop successful.
    ) else (
        call :Log "Process nginx of OceanStor BCManager Agent stop failed."
        echo Process nginx of OceanStor BCManager Agent stop failed.
        exit 1
    )
    goto :EOF
    
:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> %LOGFILENAME%
    )
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILENAME%
    goto :EOF


:other
    echo %USAGE%
    
:end
    endlocal