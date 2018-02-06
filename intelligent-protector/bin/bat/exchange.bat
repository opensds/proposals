@echo off    
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal EnableDelayedExpansion

set AGENT_ROOT_PATH=%1
set RANDOM_ID=%2
cd /d %~dp0

set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\\bin\\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%\\log\\
set AGENT_TMP_PATH=%AGENT_ROOT_PATH%\\tmp\\
set NEED_LOG=1
set LOGFILE_PATH=%AGENT_LOG_PATH%exchange.log
set INPUT_TMP_FILE=%AGENT_TMP_PATH%input_tmp%RANDOM_ID%
set RESULT_FILE=%AGENT_TMP_PATH%result_tmp%RANDOM_ID%

set ERR_CODE=1
set ERR_SCRIPT_EXEC_FAILED = 5
set ERR_SCRIPT_NOT_EXIST=255
set ERR_RESULT_FILE_NOT_EXIST=6
set ERR_INPUT_TEMP_NOT_EXSIT=7
set ERR_FILE_IS_EMPTY=8
set ERR_RECOVERY_DB_FAILED=132
set ERR_REMOVE_FAILED=130

set ARGFILEPRE=ArgFile
set ARGFILENAME=%AGENT_TMP_PATH%%ARGFILEPRE%%RANDOM_ID%.txt

rem Parse the InputInfo
call :log "Begin to parse the InputInfo."

if not exist %INPUT_TMP_FILE% (
    set ERR_CODE=%ERR_INPUT_TEMP_NOT_EXSIT%
    call :log "input_tmp%RANDOM_ID% file is not exist, exit code is %ERR_CODE%."
    goto error
)

for /f "delims=" %%i in ('type %INPUT_TMP_FILE%') do (
    if not "%%i" == "" (
        set INPUTINFO="%%i"
    )
)
if exist %INPUT_TMP_FILE% (del /f /q %INPUT_TMP_FILE%)

call :log "INPUTINFO=!INPUTINFO!"

rem OPERTYPE, 2 -- get exchange info, 0 -- mount exchange db,  1 -- remove exchange db

call:GetValue %INPUTINFO% OperType
set OPERTYPE=%ArgValue%

call :log "Operate type %OPERTYPE%."

call:GetValue %INPUTINFO% Version
set VERSION=%ArgValue%

call:GetValue %INPUTINFO% NewStorageGroupName
set NEWSTORAGEGROUPNAME=%ArgValue%

call:GetValue %INPUTINFO% NewMailBoxDBName
set NEWMAILBOXDBNAME=%ArgValue%

call:GetValue %INPUTINFO% HostName
set HOSTNAME=%ArgValue%

call :log "Data base version %VERSION%, new storage group name %NEWSTORAGEGROUPNAME%, new mailbox db name %NEWMAILBOXDBNAME%, hostname %HOSTNAME%."
    
call:GetValue %INPUTINFO% EdbFilePath
set EDBFILEPATH=%ArgValue%
        
call:GetValue %INPUTINFO% LogFilePath
set LOGFILEPATH=%ArgValue%
        
call:GetValue %INPUTINFO% SysFilePath
set SYSFILEPATH=%ArgValue%
        
call:GetValue %INPUTINFO% OldStorageGroupName
set OLDSTORAGEGROUPNAME=%ArgValue%

call:GetValue %INPUTINFO% OldMailBoxDBName
set OLDMAILBOXDBNAME=%ArgValue%
        
call:GetValue %INPUTINFO% RecoveryType
set RECOVERYTYPE=%ArgValue%
        
call :log "Log dir path %LOGFILEPATH%, edb file path %EDBFILEPATH%, sys dir path %SYSFILEPATH%, old storage group name %OLDSTORAGEGROUPNAME%, old mailbox db name %OLDMAILBOXDBNAME%, recovery type %RECOVERYTYPE%."

rem Get server name
call :log "Begin to get server name."
for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\ComputerName\ComputerName" /v "ComputerName"') do set "SERVER_NAME=%%k"
if "!SERVER_NAME!"=="" (
    call :log "Get server name failed."
    set ERR_CODE=%ERR_FILE_IS_EMPTY%
    goto error
)
call :log "Get server name succ, server name !SERVER_NAME!."

rem Get powershell path
call :log "Begin to get powershell path."
for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\PowerShell\1\PowerShellEngine" /v "ApplicationBase"') do set "POWERSHELL_PATH=%%k" 
if "!POWERSHELL_PATH!"=="" (
    call :log "Get powershell path faild."
    set ERR_CODE=%ERR_FILE_IS_EMPTY%
    goto error
)
call :log "Get powershell path succ, powershell path %POWERSHELL_PATH%."

rem Get exchange install path for 2007 and 2010
call :log "Begin to get exchange path."
for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Exchange\MSFTESQLInstMap\51" /v "Path"') do set "EXCHANGE_PATH=%%k"
if "!EXCHANGE_PATH!"=="" (
    call :log "Exchange install path for 2007 or 2010 is empty."

    rem Get exchange install path for 2013
    for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\ExchangeServer\v15\setup" /v "MsiInstallPath"') do set "EXCHANGE_PATH=%%k"
    if "!EXCHANGE_PATH!"=="" (
        call :log "Get exchange install path failed."
        set ERR_CODE=%ERR_FILE_IS_EMPTY%
        goto error
    ) else (
        set EXCHANGE_PATH=!EXCHANGE_PATH!bin\
    )
)
call :log "Get exchange path succ, exchange path %EXCHANGE_PATH%."

if "%OPERTYPE%"=="2" (
    rem Excute powershell script to get exchange info
    if not exist %AGENT_BIN_PATH%queryexchangeinfo.ps1 (
        set ERR_CODE=%ERR_SCRIPT_NOT_EXIST%
        call :log "Script queryexchangeinfo.ps1 is not exist, exit code %ERR_CODE%"
        goto error
    )
    
    call :log "Begin to execute query powershell, server name %SERVER_NAME%, result file result_tmp%RANDOM_ID%."
    "%POWERSHELL_PATH%\powershell.exe" -PSConsoleFile "%EXCHANGE_PATH%exshell.psc1" -command ".\queryexchangeinfo.ps1 '%SERVER_NAME%' '%RESULT_FILE%'"; exit $LASTEXITCODE
    
    if errorlevel 1 (
        call :log "Execute query powershell[queryexchangeinfo.ps1] failed, errorlevel is not 0."
        set ERR_CODE=%ERR_SCRIPT_EXEC_FAILED%
        goto error
    ) else (      
        if not exist %RESULT_FILE% (
            set ERR_CODE=%ERR_RESULT_FILE_NOT_EXIST%
            call :log "Execute query powershell[queryexchangeinfo.ps1] failed, result file is not exist, exit code !ERR_CODE!"
            goto error
        )
        call :log "Execute query powershell[queryexchangeinfo.ps1] succ."
        goto end
    )
) else if "%OPERTYPE%"=="0" (
    rem Excute powershell script to recover exchange db
    if not exist %AGENT_BIN_PATH%operexchangedb.ps1 (
        set ERR_CODE=%ERR_SCRIPT_NOT_EXIST%
        call :log "Script operexchangedb.ps1 is not exist, exit code !ERR_CODE!"
        goto error
    )
    
    call :log "Begin to execute recover powershell, server name %SERVER_NAME%."
    "%POWERSHELL_PATH%\powershell.exe" -PSConsoleFile "%EXCHANGE_PATH%exshell.psc1" -command ".\operexchangedb.ps1 %OPERTYPE% %VERSION% '%OLDSTORAGEGROUPNAME%' '%OLDMAILBOXDBNAME%' '%NEWSTORAGEGROUPNAME%' '%NEWMAILBOXDBNAME%' '%HOSTNAME%' '%EDBFILEPATH%' '%LOGFILEPATH%' '%SYSFILEPATH%' '%RECOVERYTYPE%' '%RESULT_FILE%'"; exit $LASTEXITCODE
    
    if errorlevel 1 (
        if exist %RESULT_FILE% (
            
            for /f "delims=" %%i in ('type %RESULT_FILE%') do (
                if not "%%i" == "" (
                    set ERR_CODE=%%i
                )
            )
            del /f /q %RESULT_FILE%
            call :log "Execute recover powershell[operexchangedb.ps1] failed, exit code !ERR_CODE!."
            goto error
        )
        
        set ERR_CODE=%ERR_RECOVERY_DB_FAILED%
        call :log "Execute recover powershell[operexchangedb.ps1] failed, mount db failed, exit code !ERR_CODE!."
        goto error    
    ) else (
        call :log "Execute recover powershell[operexchangedb.ps1] succ."
        goto end
    )
) else (
    if not exist %AGENT_BIN_PATH%operexchangedb.ps1 (
        set ERR_CODE=%ERR_SCRIPT_NOT_EXIST%
        call :log "Script operexchangedb.ps1 is not exist, exit code !ERR_CODE!"
        goto error
    )
    
    rem Excute powershell script to remove exchange db
    call :log "Begin to execute remove powershell, server name %SERVER_NAME%."
    "%POWERSHELL_PATH%\powershell.exe" -PSConsoleFile "%EXCHANGE_PATH%exshell.psc1" -command ".\operexchangedb.ps1 %OPERTYPE% %VERSION% '%NEWSTORAGEGROUPNAME%' '%NEWMAILBOXDBNAME%' '%HOSTNAME%' '%RESULT_FILE%'"; exit $LASTEXITCODE

    if errorlevel 1 (
        if exist %RESULT_FILE% (
            
            for /f "delims=" %%i in ('type %RESULT_FILE%') do (
                if not "%%i" == "" (
                    set ERR_CODE=%%i
                )
            )
            del /f /q "%RESULT_FILE%" 
            call :log "Execute remove powershell[operexchangedb.ps1] failed, exit code !ERR_CODE!."
            goto error
        )
        
        set ERR_CODE=%ERR_REMOVE_FAILED%
        call :log "Execute remove powershell[operexchangedb.ps1] failed, remove db failed, exit code !ERR_CODE!."
        goto error    
    ) else (
        call :log "Execute remove powershell[operexchangedb.ps1] succ."
        goto end
    )
)

:GetValue
    call :DeleteFile %ARGFILENAME%
    set ArgInput=%~1
    set ArgName=%~2
    set "ArgValue="

    for /l %%a in (1 1 20) do call :Separate %%a

    for /f "tokens=1,2 delims==" %%i in ('type %ARGFILENAME%') do (
        if %%i==%ArgName% (
            set ArgValue=%%j
        )
    )
    call :DeleteFile %ARGFILENAME%
    goto :EOF
    
:Separate
    for /f "tokens=%1 delims=;" %%i in ("%ArgInput%") do (
        echo %%i>> %ARGFILENAME%
    )
    goto :EOF
    
:DeleteFile
    set FileName=%1
    if exist %FileName% (del /f /q %FileName%)
    
    goto :EOF
    
:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILE_PATH%
    )
    call %AGENT_BIN_PATH%\agent_func.bat %LOGFILE_PATH%
    goto :EOF

:error
    call :log "Exchange MailBox db Operater failed."
    exit /b %ERR_CODE%
    
:end
    endlocal
    
