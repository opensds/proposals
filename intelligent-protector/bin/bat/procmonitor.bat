@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


rem @dest:   application agent for check pid
rem @date:   2009-05-19
rem @authr:  
rem @modify: 2010-02-23

setlocal EnableDelayedExpansion

set USAGE="procmonitor.bat PPID MONITORNAME TIMEOUT AGENT_ROOT"

set MYPPID=%~1
set MONITORNAME=%~2
set /a TIMEOUT=%~3
set AGENT_ROOT=%~4
set MONITORPID=

set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\

set LOGFILEPATH="%AGENT_LOG_PATH%ProcMonitor.log"
set monitorPIDs="%AGENT_TMP_PATH%ProcMonitorLists%MYPPID%.txt"
echo monitorPIDs=%monitorPIDs%
set PROCMONITOR_EXIT="%AGENT_TMP_PATH%pmexit%MYPPID%.flg"

if "%MYPPID%" == "" (
    call :Log "MYPPID is NULL."
    exit 1
)

set /a NUM=0
:CheckNum
    if !NUM! LSS !TIMEOUT! (
        if exist !PROCMONITOR_EXIT! (
            call :Log "flag file exist, PPID=%MYPPID%,exit self now."
            exit 0
        )
        rem sleep 1 second
        call :WinSleep 1
        rem sleep end
        call :GetMONITORPID 
        if "!MONITORPID!" == "" (
            wmic process where processid="%MYPPID%" get processid > %monitorPIDs%
            more %monitorPIDs% | findstr "ProcessId"
            rem parent process is not exists
            if "!errorlevel!" EQU "1" (
                call :Log "parentid=%MYPPID% is not exists."
                call :DeleteFile %monitorPIDs%
                exit 0
            ) else (
                call :Log "parentid=%MYPPID% is exists."
            )
            call :DeleteFile %monitorPIDs%
        )
        set /a NUM=!NUM! + 1
        goto :CheckNum
    )

call :Log "Have check finish, NUM=!NUM!."
if not "!MONITORPID!" == "" (
    call :Log "MONITORPID=!MONITORPID! is killed."
    taskkill /F /PID !MONITORPID!
)

endlocal
exit 0

rem Get monitor process ID whose parent process ID is same.
:GetMONITORPID
    wmic process where name="%MONITORNAME%" get processid,parentprocessid > %monitorPIDs%
    set MONITORPID=
    for /f "tokens=1,2 delims= " %%a in ('type %monitorPIDs%') do (
        if "%%a"=="%MYPPID%" (
            set MONITORPID=%%b
            echo "MONITORPID=!MONITORPID!"
        )
    )
    call :DeleteFile %monitorPIDs%
goto :EOF

rem Print log function, controled by "NEEDLOGFLG".
:Log
    echo %date:~0,10% %time:~0,8% [%MYPPID%] [%username%] %~1 >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
goto :EOF

rem Delete file function, it can delete many files.
:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
goto :EOF

:WinSleep
    timeout %1 > nul
goto :eof
