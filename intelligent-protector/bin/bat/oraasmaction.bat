@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


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

set /a ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP=38
set /a ERROR_ASM_NO_STARTUP_TNS=39

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

set DB_LOGIN=
set ASM_LOGIN=

set SHUTDOWNSQLPRE=ShutdownSql
set SHUTDOWNSCRIPT="%AGENT_TMP_PATH%%SHUTDOWNSQLPRE%%PID%.sql"

set SERVICESTATUSPRE=ServiceStatus
set SERVICESTATUS="%AGENT_TMP_PATH%%SERVICESTATUSPRE%%PID%.txt"

set SHUTDOWNPRE=ShutdownTmp
set SHUTDOWNRST="%AGENT_TMP_PATH%%SHUTDOWNPRE%%PID%.txt"

rem ***************************2012-05-25 modify problem:**************************
rem the interval of querying is 2 second
set QUERYINTERVAL=200
rem the circling time is 60 tim
set /a CIRCLETIME=120
set /a COMSTATE=1
set TNSNAME=TNSLSNR

set TNSLSNRPRE=TNSLSNRTmp
set TNSLSNRRST="%AGENT_TMP_PATH%%TNSLSNRPRE%%PID%.txt"

set LOGFILE=oraasmaction.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

rem ASM
set STARTASMSCRIPT="%AGENT_TMP_PATH%StartASMInstance%PID%.sql"
set STARTASMSCRIPTRST="%AGENT_TMP_PATH%StartASMInstanceRST%PID%.txt"
set MOUNTDISKGROUPSQL="%AGENT_TMP_PATH%MountDiskgroup%PID%.sql"
set MOUNTDISKGROUPRST="%AGENT_TMP_PATH%MountDiskgroupRST%PID%.txt"
set GETBACKUPTBCOUNT="%AGENT_TMP_PATH%GetBackupTableSpace%PID%.sql"
set GETBACKUPTBCOUNTRST="%AGENT_TMP_PATH%GetBackupTableSpaceRST%PID%.txt"

set QUREYSERVICE="%AGENT_TMP_PATH%QueryServiceRST%PID%.txt"
set DBISCLUSTER=0

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"

rem ASM instance auth mode. user/pwd or /
set ASMAuthMode=/

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

rem ASM instance auth mode. user/pwd or /
set ASMAuthMode=/
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" ASMSIDNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Action" CHECKTYPE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMDiskGroups" ASMGROUPNAMES
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD

set AUTHMODE=0
if "%ASMUSERPWD%" == "" (
    set AUTHMODE=1
)

if "%ASMSIDNAME%" == "" (
    set ASMSIDNAME=+ASM
)

call :SetASMAuth

rem get current process id
wmic process where name="cmd.exe" get processid > %CURRENTPIDRST%
set /a NUM=0
for /f %%a in ('type %CURRENTPIDRST%') do (
    if !NUM! NEQ 0 (
        set processID=%%a
        wmic process where processid="!processID!" get Commandline > %CURRENTCMDLineRST%
        rem more %CURRENTCMDLineRST% | findstr /c:"oraasmaction.bat\" \"%AGENT_ROOT%\""
        set AGENT_ROOT_TMP=%AGENT_ROOT: =%
        set BLACKFLAG=1
        if "!AGENT_ROOT_TMP!" == "!AGENT_ROOT!" (
            set BLACKFLAG=0
        )
        if "!BLACKFLAG!" == "1" (
            more %CURRENTCMDLineRST% | findstr /c:"oraasmaction.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraasmaction.bat %AGENT_ROOT% %PID%"
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

call :Log "CHECKTYPE=%CHECKTYPE%;ASMUSER=%ASMUSER%;ASMGROUPNAMES=%ASMGROUPNAMES%;ASMInstanceName=%ASMSIDNAME%;AUTHMODE=!AUTHMODE!;CURRENTPID=!CURRENTPID!;"

rem ************************get the version information************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION
set PREDBVERSION=%ORA_VERSION:~0,4%

rem ************************get oracle path ******************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETORAPATH% %PID% %LOGFILE% ORACLEBASEPATH ORACLEHOMEPATH

if "!ORACLEBASEPATH!" == "" (
    call :Log "Get Oracle base path failed." 
    exit %ERROR_SCRIPT_EXEC_FAILED%
)

if "!ORACLEHOMEPATH!" == "" (
    call :Log "Get Oracle home path failed." 
    exit %ERROR_SCRIPT_EXEC_FAILED%
)

rem ************************get grid path ******************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETGRIDPATH% %PID% %LOGFILE% !ORA_VERSION! GRIDHOMEPATH
if "!GRIDHOMEPATH!" == "" (
    call :Log "Get grid home path failed." 
)


rem check if iscluster
if not "!GRIDHOMEPATH!" == "" (
    if exist "!GRIDHOMEPATH!\bin\crsctl.exe" (
        "!GRIDHOMEPATH!\bin\crsctl.exe" check help | findstr /c:"crsctl check crs">nul
        if !errorlevel! EQU 0 (
            set DBISCLUSTER=1
        )
    )
)

set ISCLUSTER=!DBISCLUSTER!
call :Log "Check cluster [ISCLUSTER=!ISCLUSTER!]."

if "%ISCLUSTER%" EQU "1" (
    if "%GRIDHOMEPATH%" == "" (
        call :Log "Get grid home path failed." 
        exit %ERROR_SCRIPT_EXEC_FAILED%
    )
)

if "%ISASM%" EQU "1" (
    if "%GRIDHOMEPATH%" == "" (
        call :Log "Get grid home path failed." 
        exit %ERROR_SCRIPT_EXEC_FAILED%
    )
)
rem ************************get oracle path end***************************

rem check ASM instance
if "!CHECKTYPE!" EQU "1" (
    set /a COMSTATE=1
    set /a RetryNum=0
:STOPASMINSTNACE_BEG
    call :StopASMInstance RetCode
    if "!RetCode!" EQU "0" (
        exit 0
    ) else (
        if "!RetCode!" EQU "!ERROR_APPLICATION_OVER_MAX_LINK!" (
            call :Log "Shutdown ASM instance failed, error code:!RetCode!."
            exit !RetCode!
        )
        if "!RetCode!" EQU "!ERROR_ASM_DBUSERPWD_WRONG!" (
            call :Log "Shutdown ASM instance failed, error code:!RetCode!."
            exit !RetCode!
        )
        if "!RetCode!" EQU "!ERROR_ASM_INSUFFICIENT_WRONG!" (
            call :Log "Shutdown ASM instance failed, error code:!RetCode!."
            exit !RetCode!
        )
        
        if !RetryNum! LSS 60 (
            set /a RetryNum=!RetryNum! + 1
            call :WinSleep 5
            call :Log "try to shutdown ASM isntance [!RetryNum!]."
            goto :STOPASMINSTNACE_BEG
        ) else (
            call :Log "shutdown ASM Instance have retry 60 times, shutdown ASM Instance failed."
            exit !RetCode!
        )
    )
) else (
    set /a COMSTATE=4
    if "%ISCLUSTER%" EQU "1" (
        call :Log "cluster is not support to start ASM instance"
        exit %ERROR_SCRIPT_EXEC_FAILED%
    )
    
    rem start has--crsctl start has
    "%GRIDHOMEPATH%\bin\crsctl" start has >> %LOGFILEPATH%
    
    rem start css--crsctl start resource ora.cssd
    "%GRIDHOMEPATH%\bin\crsctl" start resource ora.cssd >> %LOGFILEPATH%
    
    rem start ASM instance-- start service,startup
    call :StartWindowsServices "OracleASMService!ASMSIDNAME!" RetCode
    if "!RetCode!" NEQ "0" (
        call :Log "start OracleASMService!ASMSIDNAME! failed."
        exit %ERROR_SCRIPT_EXEC_FAILED%
    )
    
    call :checkASMAuthMode %ASMUSER% %ASMUSERPWD%
    rem start ASM instance
    call :SetASMAuth
    call :CreatASMStartupSql %STARTASMSCRIPT%
    call :Log "Execute sql to startup asm instance."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %STARTASMSCRIPT% %STARTASMSCRIPTRST% !ASMSIDNAME! "!ASM_LOGIN!" 120 RetCode
    call :DeleteFile %STARTASMSCRIPT%
    call :DeleteFile %STARTASMSCRIPTRST%
    
    rem mount diskgoup
    call :DiskGroupManage 1 RetCode
    if "!RetCode!" EQU "1" (
        call :Log "mount diskgroup failed."
    )
)

exit 0

:StopRACInstance
    rem >=11.2
    set ORANEW=0
    if "!PREDBVERSION!" == "11.2" (
        set ORANEW=1
    )
    
    if "!PREDBVERSION:~0,2!" GEQ "12" (
        set ORANEW=1
    )
    
    call :Log "Begin stop RAC[%DBVERSION%]"
    if "!ORANEW!" == "1" (
        "%GRIDHOMEPATH%\bin\crsctl" stop cluster -all >> %LOGFILEPATH%
    ) else (
        "%GRIDHOMEPATH%\bin\crs_stop" -all >> %LOGFILEPATH%
    )
    
    call :Log "End stop RAC[%DBVERSION%]"
    goto :eof

:StartWindowsServices
    set SERVICENAME=%~1
    set /a %~2 = 1
    for /f "tokens=2 delims=:" %%i in ('sc query %SERVICENAME% ^| findstr /i "STATE"') do (
        set INSTANCESTATUS=%%i
        for /f "tokens=1 delims= " %%j in ("!INSTANCESTATUS!") do (
            rem *********************Check Service is Start*************************************
            if %%j EQU 1 (
                sc start %SERVICENAME% > nul
                if !errorlevel! NEQ 0 (
                    call :Log "Execute sc start %SERVICENAME% command failed."
                    call :DeleteFile %SERVICESTATUS%
                    set /a %~2 = 1
                ) else (
                    set /a DelayCount=0
                    call :WaitService %QUERYINTERVAL% %CIRCLETIME% %SERVICENAME%
                    call :Log "Start the database %SERVICENAME% service successful."
                    set /a %~2 = 0
                ) 
            ) else (
                if %%j NEQ 4 (
                    call :Log "The %SERVICENAME% Service Status isn't right."
                    call :DeleteFile %SERVICESTATUS%
                    set /a %~2 = 1
                ) else (
                    call :Log "The %SERVICENAME% Service has Started." 
                    set /a %~2 = 0
                )
            )
        )
    )
    goto :eof

rem **************************mount diskgroup***********************************
:DiskGroupManage
    rem mount(1) umount(0)
    set MMode=%~1
    set MModeStr=
    if "!MMode!" EQU "1" (
        set MModeStr=mount
    ) else (
        set MModeStr=dismount
    )

    call :checkASMAuthMode %ASMUSER% %ASMUSERPWD%
    call :SetASMAuth
    
    set DGLIST=%ASMGROUPNAMES%
    set /a %~2 = 0
    rem check asm diskgroup whether mount, mount if not mount
:BEGSPLIT
    for /f "tokens=1,* delims=+ " %%a in ("%DGLIST%") do (
        set dgName=%%a
        set DGLIST=%%b

        echo alter diskgroup !dgName! !MModeStr!; > %MOUNTDISKGROUPSQL%
        echo exit; >> %MOUNTDISKGROUPSQL%
        
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %MOUNTDISKGROUPSQL% %MOUNTDISKGROUPRST% !ASMINSTANCENAME! "!ASM_LOGIN!" 30 RetMount
        call :DeleteFile %MOUNTDISKGROUPSQL%
        call :DeleteFile %MOUNTDISKGROUPRST%
        
        rem get status of diskgroup
        echo set linesize 300 > %MOUNTDISKGROUPSQL%
        echo col state for a50 >> %MOUNTDISKGROUPSQL%
        echo select state from v$asm_diskgroup where NAME='!dgName!'; >> %MOUNTDISKGROUPSQL%
        echo exit; >> %MOUNTDISKGROUPSQL%
        
        set DGSTATUS=
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %MOUNTDISKGROUPSQL% %MOUNTDISKGROUPRST% %ASMSIDNAME% "!ASM_LOGIN!" 60 RetCode
        if "!RetCode!" NEQ "0" (
            call :DeleteFile %MOUNTDISKGROUPSQL%
            call :DeleteFile %MOUNTDISKGROUPRST%
            call :Log "Get diskgroup status failed after !MModeStr! diskgroup."
            exit !RetCode!
        ) else (
            set /a flag=0
            set /a FLG_FIND=0
            for /f "skip=12" %%i in ('type %MOUNTDISKGROUPRST%') do (
                if "!FLG_FIND!" EQU "1" (
                    if "!flag!" EQU "0" (
                        set DGSTATUS=%%i
                        call :Log "DGSTATUS=%%i"
                    )
                    set /a flag=1
                )
                
                echo %%i | findstr \-\-\-\-\-\-
                if "!errorlevel!" EQU "0" (
                    set /a FLG_FIND=1
                )
            )
            call :DeleteFile %MOUNTDISKGROUPSQL%
            call :DeleteFile %MOUNTDISKGROUPRST%
        )

        if "!MMode!" EQU "1" (
            if "!DGSTATUS!" == "MOUNTED" (
                call :Log "!MModeStr! diskgroup !dgName! success"
            ) else (
                call :Log "!MModeStr! diskgroup !dgName! failed"
                exit !RetMount!
            )
        ) else (
            if "!DGSTATUS!" == "DISMOUNTED" (
                call :Log "!MModeStr! diskgroup !dgName! success"
            ) else (
                call :Log "!MModeStr! diskgroup !dgName! failed"
                exit !RetMount!
            )
        )


        if "!DGLIST!" == "" (
            set /a %~2 = 0
            goto :ENDSPLIT
        ) else (
            goto :BEGSPLIT
        )
    )

:ENDSPLIT
    goto :eof

:WinSleep
    timeout %1 > nul
    goto :eof

rem ************************************************************************
rem function name: CreatASMStartupSql
rem aim:           Create temporary sql script for startup ASM instance
rem input:         the sql script name
rem output:        STARTUPSCRIPT
rem ************************************************************************
:CreatASMStartupSql
    echo startup; > "%~1"
    echo exit >> "%~1"
    goto :EOF 
  
rem ************************************************************************
rem function name: WaitService
rem aim:           Waitting until the status of database is running
rem input:         QUERYINTERVAL, CIRCLETIME, SERVICENAME
rem output:        
rem ************************************************************************
   
:WaitService
rem **********OnceDelayTime units:millisecond**********
    call :DeleteFile %SERVICESTATUS%
    set TMPSTATUS=!COMSTATE!
    call :Log "Want to Check OracleService state is !TMPSTATUS!"    
    if !DelayCount! GEQ %~2 (
        call :DeleteFile %SERVICESTATUS%
        if !TMPSTATUS! EQU 4 (
            call :Log "Start the %~3 service failed."
        ) else (
            call :Log "Stop the %~3 service failed."
        )
        exit %ERROR_SCRIPT_EXEC_FAILED%
    )

    sc query %~3 | findstr /i "STATE" > %SERVICESTATUS%
    set /p SERVICESTATUSTMP= < %SERVICESTATUS%
    if "!SERVICESTATUSTMP!"=="" (
        call :Log "Service %~3 is not existent."
        call :DeleteFile %SERVICESTATUS%
        exit %ERROR_SCRIPT_EXEC_FAILED%
    )
    
    for /f "tokens=2 delims=:" %%i in ('sc query %~3 ^| findstr /i "STATE"') do (
        echo %%i > %SERVICESTATUS%
        for /f "tokens=1 delims= " %%j in ('type %SERVICESTATUS%') do (
            set TMP=%%j
            call :Log "Now OracleService state is !TMP!"
            if !TMP! NEQ !TMPSTATUS! (
                set /a DelayCount+=1
                call :DeleteFile %SERVICESTATUS%
                call :ProcDelay %~1
                goto :WaitService %~1 %~2 %~3
            )        
        )
    )
    call :DeleteFile %SERVICESTATUS%
    goto :EOF

:ProcDelay DelayMSec
    for /f "tokens=1-4 delims=:. " %%h in ("%TIME%") do set start=%%h%%i%%j%%k
:ProcwaitLoop
    for /f "tokens=1-4 delims=:. " %%h in ("%TIME%") do set now=%%h%%i%%j%%k
    set /a diff=%now%-%start%
    if %diff% LSS %1 goto ProcwaitLoop
    ENDLOCAL & goto :EOF
:EOF

rem ************************************************************************
rem function name: WaitTNSService
rem aim:           Waitting until the status of database is running
rem input:         QUERYINTERVAL, CIRCLETIME, SERVICENAME
rem output:        
rem ************************************************************************
   
:WaitTNSService
rem **********OnceDelayTime units:millisecond**********
    
    if !DelayCount! GEQ %~2 (
        call :DeleteFile %TNSLSNRRST%
        set /a %~3=1
        goto :EOF
    )
    
    tasklist |findstr %TNSNAME% > %TNSLSNRRST%
    if !errorlevel! NEQ 0 (
        call :Log "Execute command(search linster) failed."
        set /a %~3 = 1
        goto :EOF
    ) else (
        call :CheckFileIsEmpty %TNSLSNRRST% RetCode
        if !RetCode! EQU 0 (
            set /a DelayCount+=1
            call :DeleteFile %TNSLSNRRST%
            call :ProcTNSDelay %~1
            goto :WaitTNSService %~1 %~2 %~3
        )
    )
    set /a %~3=0
    goto :EOF

:ProcTNSDelay DelayMSec
    for /f "tokens=1-4 delims=:. " %%h in ("%TIME%") do set TNSstart=%%h%%i%%j%%k
:ProcTNSwaitLoop
    for /f "tokens=1-4 delims=:. " %%h in ("%TIME%") do set TNSnow=%%h%%i%%j%%k
    set /a diff=%TNSnow%-%TNSstart%
    if %diff% LSS %1 goto ProcTNSwaitLoop
    ENDLOCAL & goto :EOF
:EOF
        
rem ************************************************************************
rem function name: Log
rem aim:           Print log function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [!CURRENTPID!] [%username%] %~1 >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
    goto :EOF
    
rem ************************************************************************
rem function name: DeleteFile
rem aim:           Delete file function
rem input:         the deleted file
rem output:        
rem ************************************************************************
:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
    
    goto :EOF

rem ************************************************************************
rem function name: CheckFileIsEmpty
rem aim:           Check file function
rem input:         the check file
rem output:        
rem ************************************************************************
:CheckFileIsEmpty
    set FileName="%~1"
    for %%i in (%FileName%) do (
        if %%~zi EQU 0 (
            set /a %~2 = 0
            goto :EOF
        )
    )
    
    set /a %~2 = 1
    goto :EOF
    
:StopASMInstance
    rem try to get ASM instance name
    sc query state= all | findstr /i "OracleASMService%ASMSIDNAME%" > %QUREYSERVICE%
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

    if "%ISCLUSTER%" EQU "1" (
        call :StopRACInstance
    ) else (
        call :checkASMAuthMode %ASMUSER% %ASMUSERPWD%

        rem exec shutdown ASM instance SQL
        echo shutdown immediate; > %SHUTDOWNSCRIPT%
        echo exit >> %SHUTDOWNSCRIPT%
        call :SetASMAuth
        
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %SHUTDOWNSCRIPT% %SHUTDOWNRST% !ASMINSTANCENAME! "!ASM_LOGIN!" 120 RetCode
        call :DeleteFile %SHUTDOWNSCRIPT%
        call :DeleteFile %SHUTDOWNRST%
    )
    
    call :checkASMAuthMode %ASMUSER% %ASMUSERPWD%
    rem check ASM instance status
    echo select status from v$instance; > %SHUTDOWNSCRIPT%
    echo exit >> %SHUTDOWNSCRIPT%
    call :SetASMAuth
    
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %SHUTDOWNSCRIPT% %SHUTDOWNRST% !ASMINSTANCENAME! "!ASM_LOGIN!" 60 RetCode
    call :DeleteFile %SHUTDOWNSCRIPT%
    call :DeleteFile %SHUTDOWNRST%
    
    if "!RetCode!" EQU "!ERROR_ASM_NO_STARTUP_TNS!" (
        call :Log "ASM instance is already closed."
        set /a %~1 = 0
        goto :eof
    )
    
    if "!RetCode!" EQU "!ERROR_RECOVER_INSTANCE_NOSTART!" (
        call :Log "ASM instance is already closed."
        set /a %~1 = 0
        goto :eof
    )

    if "!RetCode!" EQU "0" (
        call :Log "ASM instance is already open."
        set /a %~1 =%ERROR_SCRIPT_EXEC_FAILED%
        goto :eof
    )
    
    call :Log "Execute get ASM status sql script failed."
    set /a %~1 = !RetCode!
    goto :eof

:checkASMAuthMode
    set UserName=%~1
    set Password=%~2
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

    rem exec shutdown ASM instance SQL
    if !AUTHMODE!==1 (
        set ASM_LOGIN=/ as sysasm
    )
    if !AUTHMODE!==0 (
        set ASM_LOGIN=!UserName!/"!Password!" as sysasm
    )
    echo select status from v$instance; > %SHUTDOWNSCRIPT%
    echo exit >> %SHUTDOWNSCRIPT%
    
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %SHUTDOWNSCRIPT% %SHUTDOWNRST% !ASMINSTANCENAME! "!ASM_LOGIN!" 30 RetCode
    call :DeleteFile %SHUTDOWNSCRIPT%
    call :DeleteFile %SHUTDOWNRST%
    if "!RetCode!" EQU "!ERROR_ASM_INSUFFICIENT_WRONG!" (
        set ASMAuthMode=/
        call :Log "ASMAuthMode uses without username,password."
    ) else (
        set ASMAuthMode=!UserName!/"!Password!"
        call :Log "ASMAuthMode uses username/password."
    )
goto :eof

:SetASMAuth
    if !AUTHMODE!==1 (
        set ASM_LOGIN=/ as sysasm
    )
    if !AUTHMODE!==0 (
        set ASM_LOGIN=!ASMAuthMode! as sysasm
    )
goto :eof

endlocal
