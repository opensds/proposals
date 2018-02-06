@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


rem @dest:   application agent for oracle10g
rem @date:   2009-05-19
rem @authr:  
rem @modify: 2010-02-23

setlocal EnableDelayedExpansion
set /a ERROR_SCRIPT_EXEC_FAILED=5
set /a ERROR_DBUSERPWD_WRONG=10
set /a ERROR_RECOVER_INSTANCE_NOSTART=11
set /a ERROR_INSUFFICIENT_WRONG=15

set /a ERROR_ASM_DBUSERPWD_WRONG=21
set /a ERROR_ASM_INSUFFICIENT_WRONG=22
set /a ERROR_ASM_RECOVER_INSTANCE_NOSTART=23
set /a ERROR_ORACLE_NOARCHIVE_MODE=24
set /a ERROR_ORACLE_OVER_ARCHIVE_USING=25
set /a ERROR_ASM_DISKGROUP_ALREADYMOUNT=26
set /a ERROR_ASM_DISKGROUP_NOTMOUNT=27
set /a ERROR_APPLICATION_OVER_MAX_LINK=28
set /a ERROR_DB_ALREADY_INBACKUP=29
set /a ERROR_DB_INHOT_BACKUP=30
set /a ERROR_DB_ALREADYRUNNING=31
set /a ERROR_DB_ALREADYMOUNT=32
set /a ERROR_DB_ALREADYOPEN=33
set /a ERROR_DB_ARCHIVEERROR=34

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"

set CMD_GETVERSION=getversion
set CMD_EXECSQL=execsql
set CMD_GETVALUE=getvalue
set CMD_EXECASMSQL=execasmsql

set TESTQRYDBOPENMODEFILE="%AGENT_TMP_PATH%TestDBOpenMode%PID%.sql"
set TESTQRYDBOPENMODERSTFILE="%AGENT_TMP_PATH%TestDBOpenModeRST%PID%.txt"
set TESTQRYDBOPENMODE_READ=read
set LEN_READ=4
set DB_LOGIN=
set ASM_LOGIN=

set LOGFILE=oracletest.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

set QUREYSERVICE="%AGENT_TMP_PATH%QueryService%PID%.txt"

rem check username password
set CHECKUSERPWD="%AGENT_TMP_PATH%CheckUserPwd%PID%.sql"
set CHECKUSERPWDRST="%AGENT_TMP_PATH%CheckUserPwdRST%PID%.txt"

rem check instance status
set CHECKINSTANCESTATUS="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set CHECKINSTANCESTATUSRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"

rem #############################################################
set INPUTINFO=
for /f "delims=" %%a in ('type %PARAM_FILE%') do (
    if not "%%a" == "" (
        set INPUTINFO=%%a
    )
)
call :DeleteFile %PARAM_FILE%

if "!INPUTINFO!" == "" (
    call :Log "INPUTINFO is null."
    exit %ERROR_SCRIPT_EXEC_FAILED%
)

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
call :UperTOLow !DBUSERL! DBUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)

call :SetDBAuth
call :SetASMAuth

rem get current process id
wmic process where name="cmd.exe" get processid > %CURRENTPIDRST%
set /a NUM=0
for /f %%a in ('type %CURRENTPIDRST%') do (
    if !NUM! NEQ 0 (
        set processID=%%a
        wmic process where processid="!processID!" get Commandline > %CURRENTCMDLineRST%
        set AGENT_ROOT_TMP=%AGENT_ROOT: =%
        set BLACKFLAG=1
        if "!AGENT_ROOT_TMP!" == "!AGENT_ROOT!" (
            set BLACKFLAG=0
        )
        if "!BLACKFLAG!" == "1" (
            more %CURRENTCMDLineRST% | findstr /c:"oracletest.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oracletest.bat %AGENT_ROOT% %PID%"
        )
        if !errorlevel! EQU 0 (
            set /a CURRENTPID=%%a
        )
    )
    set /a NUM=!NUM!+1
)

call :DeleteFile %CURRENTPIDRST%
call :DeleteFile %CURRENTCMDLineRST%
if !CURRENTPID! EQU 0 (
    call :Log "CURRENTPID equal 0, the timeout is will failed."
)
set monitorPIDs="%AGENT_TMP_PATH%ProcMonitorLists!CURRENTPID!.txt"

echo DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%
call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;CURRENTPID=!CURRENTPID!;"

set ENVFILERORACLETMP="%AGENT_TMP_PATH%RORACLETMP%PID%.txt"

set DBROLE=
if "%DBUSER%"=="sys" (
    set DBROLE=as sysdba
)

::set environment enable
if not exist %ENVFILERORACLETMP% (set > %ENVFILERORACLETMP%)
for /f "tokens=1,2 delims==" %%i in ('type %ENVFILERORACLETMP%') do (
    if %%i==PATH (
        set PATH=%%j
    )
)
call :DeleteFile %ENVFILERORACLETMP%

rem ************************get the version information************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION

set FSTCHAR=%DBINSTANCE:~0,1%
if "!FSTCHAR!" == "+" (
    call :TestASMInstanceConnect
) else (
    call :TestDBConnect
)

exit 0

rem Print log function, controled by "NEEDLOGFLG".
:Log
    echo %date:~0,10% %time:~0,8% [%username%] [!CURRENTPID!] %~1 >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
    goto :EOF

rem Delete file function, it can delete many files.
:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
    goto :EOF

rem create the sql that get the open mode of the DB
:CreatCheckDBReadSQL
    echo select open_mode from v$database; > "%~1"
    echo exit >> "%~1"
    goto :EOF

rem Convert str to low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    
    set %~2=!CONVERTSTR!
    
goto :EOF

:TestDBConnect
    rem check asm intance status
    call :Log "Start to check oracle instance status."
    call :GetInstanceStatus INSTStatus RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "Get instance status failed."
        exit !RetCode!
    )

    rem STARTED - After STARTUP NOMOUNT
    rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    rem OPEN - After STARTUP or ALTER DATABASE OPEN
    rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if not "!INSTStatus:~0,4!" == "OPEN" (
        call :Log "Instance status !INSTStatus! no open"
        exit %ERROR_RECOVER_INSTANCE_NOSTART%
    )
    call :Log "End to check oracle instance status."

    call :CreatCheckDBReadSQL %TESTQRYDBOPENMODEFILE%
    call :Log "Exec sql to get status of database."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %TESTQRYDBOPENMODEFILE% %TESTQRYDBOPENMODERSTFILE% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Execute Script to query DB open Mode failed."
        call :Log "Test %DBNAME% failed."
        call :DeleteFile %TESTQRYDBOPENMODEFILE%
        call :DeleteFile %TESTQRYDBOPENMODERSTFILE%
        exit !RetCode!
    ) else (
        set /a OPENMODE_ISREAD=0
        set /a FLG_FIND=0
        for /f "skip=12 delims=" %%i in ('type %TESTQRYDBOPENMODERSTFILE%') do (            
            if "!FLG_FIND!" EQU "1" (
                set LINE=%%i
                set LINE=!LINE:~0,4!
                call :UperTOLow !LINE! NEWMODE

                rem check type
                if "!NEWMODE!"=="!TESTQRYDBOPENMODE_READ!" (
                    set /a OPENMODE_ISREAD=1
                )
                
                call :Log "readStatus=%%i"
            )
            
            echo %%i | findstr \-\-\-\-\-\-
            if "!errorlevel!" EQU "0" (
                set /a FLG_FIND=1
            )
        )

        call :DeleteFile %TESTQRYDBOPENMODEFILE%
        call :DeleteFile %TESTQRYDBOPENMODERSTFILE%
        if !OPENMODE_ISREAD! NEQ 1 (
            call :Log "Check DB open mode failed."
            call :Log "Test %DBNAME% failed."
            exit %ERROR_SCRIPT_EXEC_FAILED%
        ) else (
            echo Test %DBNAME% successful.
            call :Log "Test %DBNAME% successful."
            exit 0
        )
    )
    goto :eof

:TestASMInstanceConnect
    rem try to get ASM instance name
    sc query state= all | findstr /i "OracleASMService%DBINSTANCE%" > %QUREYSERVICE%
    set ASMINSTANCENAME=+ASM
    for /f "tokens=1,2 delims=:" %%a in ('type %QUREYSERVICE%') do (
        if "SERVICE_NAME"=="%%a" (
            for /f "tokens=1,2 delims=+" %%i in ("%%b") do (
                set ASMINSTANCENAME=+%%j
            )
        )
    )
    call :DeleteFile %QUREYSERVICE%
    call :Log "check ASM Instance=!ASMINSTANCENAME!"

    echo select status from v$instance; > %CHECKUSERPWD%
    echo exit >> %CHECKUSERPWD%

    call :Log "Exec sql to get status of asm instance."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %CHECKUSERPWD% %CHECKUSERPWDRST% %ASMINSTANCENAME% "!ASM_LOGIN!" 30 RetCode
    call :DeleteFile %CHECKUSERPWD%
    call :DeleteFile %CHECKUSERPWDRST%
    if "!RetCode!" NEQ "0" (
        call :Log "Test ASM instance failed."
        exit !RetCode!
    ) else (
        call :Log "Test ASM instance successful."
    )
goto :eof

rem ************************** check instance status ***********************************
:GetInstanceStatus
    echo select status from v$instance; > %CHECKINSTANCESTATUS%
    echo exit >> %CHECKINSTANCESTATUS%

    call :Log "Exec sql to get status of instance."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %CHECKINSTANCESTATUS% %CHECKINSTANCESTATUSRST% %DBINSTANCE% "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
        set /a %~2 = !RetCode!
        call :DeleteFile %CHECKINSTANCESTATUS%
        call :DeleteFile %CHECKINSTANCESTATUSRST%
    ) else (
        rem STARTED - After STARTUP NOMOUNT
        rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        rem OPEN - After STARTUP or ALTER DATABASE OPEN
        rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
        set /a FLG_FIND=0
        for /f "skip=12" %%i in ('type %CHECKINSTANCESTATUSRST%') do (
            if "!FLG_FIND!" EQU "1" (
                set %~1=%%i
                set /a %~2 = 0
                call :DeleteFile %CHECKINSTANCESTATUS%
                call :DeleteFile %CHECKINSTANCESTATUSRST%
                call :Log "InstanceStatus=%%i"
                goto :eof
            )
            
            echo %%i | findstr \-\-\-\-\-\-
            if "!errorlevel!" EQU "0" (
                set /a FLG_FIND=1
            )
        )
    )

goto :eof

:SetDBAuth
    if !AUTHMODE!==1 (
        set DB_LOGIN=/ as sysdba
    )
    if !AUTHMODE!==0 (
        if "%DBUSER%"=="sys" (
            set DB_LOGIN=%DBUSER%/"%DBUSERPWD%" as sysdba
        ) else (
            set DB_LOGIN=%DBUSER%/"%DBUSERPWD%"
        )
    )
goto :eof

:SetASMAuth
    if !AUTHMODE!==1 (
        set ASM_LOGIN=/ as sysasm
    )
    if !AUTHMODE!==0 (
        set ASM_LOGIN=%DBUSER%/"%DBUSERPWD%" as sysasm
    )
goto :eof

endlocal
