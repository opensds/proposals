@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal EnableDelayedExpansion
set NEED_LOG_FLAG=1
set DISK_NUM=%~1
set CURRENT_PATH=%~dp0
set AGENT_ROOT=%CURRENT_PATH%..\\
set AGENT_BIN_PATH=%AGENT_ROOT%bin\\
set AGENT_LOG_PATH=%AGENT_ROOT%log\\
set AGENT_TMP_PATH=%AGENT_ROOT%tmp\\
set LOG_FILE_PATH="%AGENT_LOG_PATH%online.log"

call :Log "###########################################"
call :Log "Begin to online disk %DISK_NUM%."
rem check current os version
ver | findstr "6\.[0-3]\.[0-9][0-9][0-9][0-9]"
if errorlevel 1 goto BeforeWin2008
if errorlevel 0 goto AfterWin2008

:BeforeWin2008
    call :Log "Do nothing in the old operating system before windows 2008."
    exit 0

:AfterWin2008
    rem get current time, store in NOW veriable
    FOR /f "tokens=1-4 delims=:. " %%h IN ("%TIME%") DO (SET TIMENOW=%%h%%i%%j%%k)
    set ONLINE_CMD_FILE=%AGENT_TMP_PATH%online_%RANDOM%%TIMENOW%
    set OFFLINE_CMD_FILE=%AGENT_TMP_PATH%offline_%RANDOM%%TIMENOW%

    rem For the occasional debug of Disk Management on windows(the Online Lun is unavailable).
    rem It must offline the Lun and Online the Lun. 

    rem offline the Lun 
    echo select disk %DISK_NUM% >> "%OFFLINE_CMD_FILE%"
    echo att disk clear readonly noerr>> "%OFFLINE_CMD_FILE%"
    echo offline disk>> "%OFFLINE_CMD_FILE%"

    diskpart /s "%OFFLINE_CMD_FILE%" >> %LOG_FILE_PATH%
    if exist "%OFFLINE_CMD_FILE%" (del /f /q "%OFFLINE_CMD_FILE%")

    rem online the Lun 
    echo select disk %DISK_NUM% >> "%ONLINE_CMD_FILE%"
    echo att disk clear readonly noerr>> "%ONLINE_CMD_FILE%"
    echo online disk>> "%ONLINE_CMD_FILE%"
    diskpart /s "%ONLINE_CMD_FILE%" >> %LOG_FILE_PATH%
    if !errorlevel! EQU 0 (
        if exist "%ONLINE_CMD_FILE%" (del /f /q "%ONLINE_CMD_FILE%")
        call :Log "Online disk %DISK_NUM% succ."
        exit 0
    ) else (
        if exist "%ONLINE_CMD_FILE%" (del /f /q "%ONLINE_CMD_FILE%")
        call :Log "Online disk %DISK_NUM% failed, error !errorlevel!."
        exit 1
    )

:Log
    if %NEED_LOG_FLAG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOG_FILE_PATH%
    )
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOG_FILE_PATH%
    goto :EOF

endlocal

