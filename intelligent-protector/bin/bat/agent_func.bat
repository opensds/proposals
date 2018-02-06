@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal EnableDelayedExpansion

set CURRENTPATH=%~dp0
set AGENT_ROOT=%CURRENTPATH%..\
set AGENT_CONF_PATH=%AGENT_ROOT%conf\
set MAXLOGSIZE=3145728
set LOGFILE_SUFFIX=zip

set TEMPLOGNAME=%~nx1

set LOGFILENAME=%AGENT_ROOT%log\%TEMPLOGNAME%

set BACKLOGNAME="%LOGFILENAME%.%BACKLOGCOUNT%"

for /f "delims=" %%i in ("%LOGFILENAME%") do (
    set filesize=%%~zi
) 

for /f tokens^=1-3^ delims^=^" %%i in ('findstr "log_count" "%AGENT_CONF_PATH%agent_cfg.xml"') do (
    set  BACKLOGCOUNT=%%j
)

set BACKLOGNAME="%LOGFILENAME%.%BACKLOGCOUNT%.%LOGFILE_SUFFIX%"
if !filesize! gtr %MAXLOGSIZE% (
    if exist !BACKLOGNAME! (del /f /q !BACKLOGNAME!)
    set /a NUMBER=%BACKLOGCOUNT%-1
    :backlog
    if !NUMBER! GEQ 0 (
        if !NUMBER! EQU 0 (
            set BACKLOGNAME="%LOGFILENAME%.%LOGFILE_SUFFIX%"
            call "%CURRENTPATH%7z.exe" a -y -tzip %BACKLOGNAME% "%LOGFILENAME%" -mx=9 > nul
            del /f /q  "%LOGFILENAME%"
        ) else (
            set BACKLOGNAME="%LOGFILENAME%.!NUMBER!.%LOGFILE_SUFFIX%"
        )
        if exist !BACKLOGNAME! (
            set /a NUMBER_TEMP=!NUMBER!+1
            set DESTLOGNAME="%TEMPLOGNAME%.!NUMBER_TEMP!.%LOGFILE_SUFFIX%"
            REN !BACKLOGNAME! !DESTLOGNAME!
        )
        set /a NUMBER=!NUMBER!-1
        goto :backlog
    )
    cd ./>"%LOGFILENAME%"
)

rem set file access
echo Y | Cacls "%LOGFILENAME%" /E /R Users > nul

endlocal
