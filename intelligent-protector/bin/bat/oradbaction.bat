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

set /a ERROR_ORACLE_NOT_MOUNTED=40
set /a ERROR_ORACLE_NOT_OPEN=41
set /a ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED=42
set /a ERROR_ORACLE_TNS_PROTOCOL_ADAPTER=43
set /a ERROR_ORACLE_NOT_INSTALLED=44
set /a ERROR_ORACLE_ANOTHER_STARTING=45
set /a ERROR_ORACLE_DB_BUSY=46

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
rem **************************Set file name and path***************************************
rem the interval of querying is 2 second
set QUERYINTERVAL=200
rem the circling time is 60 tim
set /a CIRCLETIME=120
set /a COMSTATE=1
set TNSNAME=TNSLSNR

set LOGININPRE=LoginInSql
set LOGININSCRIPT="%AGENT_TMP_PATH%%LOGININPRE%%PID%.sql"

set SHUTDOWNSQLPRE=ShutdownSql
set SHUTDOWNSCRIPT="%AGENT_TMP_PATH%%SHUTDOWNSQLPRE%%PID%.sql"

set STARTUPPRE=StartupSql
set STARTUPSCRIPT="%AGENT_TMP_PATH%%STARTUPPRE%%PID%.sql"

rem ****************************2012-05-14 modify problem:***********************
set STARTRECOVERPRE=ExecRecoverSql
set STARTUPECOVERSCRIPT="%AGENT_TMP_PATH%%STARTRECOVERPRE%%PID%.sql"

set MOUNTDBPRE=MountDBSql
set MOUNTDBSCRIPT="%AGENT_TMP_PATH%%MOUNTDBPRE%%PID%.sql"

set RECOVERPRE=StartupNomountSql
set RECOVERSCRIPT="%AGENT_TMP_PATH%%RECOVERPRE%%PID%.sql"
set RECOVERSCRIPTRST="%AGENT_TMP_PATH%%RECOVERPRE%%PID%.txt"

set SERVICESTATUSPRE=ServiceStatus
set SERVICESTATUS="%AGENT_TMP_PATH%%SERVICESTATUSPRE%%PID%.txt"

set LOGININRSPRE=LoginInTmp
set LOGININRST="%AGENT_TMP_PATH%%LOGININRSPRE%%PID%.txt"

set STARTUPRSLPRE=StartupTmp
set STARTUPRST="%AGENT_TMP_PATH%%STARTUPRSLPRE%%PID%.txt"

set SHUTDOWNPRE=ShutdownTmp
set SHUTDOWNRST="%AGENT_TMP_PATH%%SHUTDOWNPRE%%PID%.txt"

rem ****************************2012-05-14 modify problem:***********************
set STARTRECOVERLOGPRE=RecoverDBTmp
set STARTRECOVERRST="%AGENT_TMP_PATH%%STARTRECOVERLOGPRE%%PID%.txt"

set MOUNTDBLOGPRE=MountDBTmp
set MOUNTDBRST="%AGENT_TMP_PATH%%MOUNTDBLOGPRE%%PID%.txt"

set RECOVERRSLPRE=RecoverTmp
set RECOVERRST="%AGENT_TMP_PATH%%RECOVERRSLPRE%%PID%.txt"

rem ***************************2012-05-25 modify problem:**************************
set TNSLSNRPRE=TNSLSNRTmp
set TNSLSNRRST="%AGENT_TMP_PATH%%TNSLSNRPRE%%PID%.txt"

set LOGFILE=oradbaction.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

rem create archive directory
set GETARCHIVEINFOFILE="%AGENT_TMP_PATH%GetArchive%PID%.sql"
set GETARCHIVEINFORST="%AGENT_TMP_PATH%GetArchiveRST%PID%.txt"
set GETFLASHINFOFILE="%AGENT_TMP_PATH%GetFlash%PID%.sql"
set GETFLASHINFORST="%AGENT_TMP_PATH%GetFlashRST%PID%.txt"

rem ASM
set STARTASMSCRIPT="%AGENT_TMP_PATH%StartASMInstance%PID%.sql"
set STARTASMSCRIPTRST="%AGENT_TMP_PATH%StartASMInstanceRST%PID%.txt"
set MOUNTDISKGROUPSQL="%AGENT_TMP_PATH%MountDiskgroup%PID%.sql"
set MOUNTDISKGROUPRST="%AGENT_TMP_PATH%MountDiskgroupRST%PID%.txt"
set GETBACKUPTBCOUNT="%AGENT_TMP_PATH%GetBackupTableSpace%PID%.sql"
set GETBACKUPTBCOUNTRST="%AGENT_TMP_PATH%GetBackupTableSpaceRST%PID%.txt"

set QUREYSERVICE="%AGENT_TMP_PATH%QueryServiceRST%PID%.txt"

rem check instance status
set CHECKINSTANCESTATUS="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set CHECKINSTANCESTATUSRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"

rem check database flashback status
set FLASHBACKSTATUS="%AGENT_TMP_PATH%flashbackStatus%PID%.sql"
set FLASHBACKSTATUSRST="%AGENT_TMP_PATH%flashbackStatusRST%PID%.txt"
set FLASHBACKTATUS=

rem check cluster status
set CHECKCLUSTERSTATUS="%AGENT_TMP_PATH%CheckClustereStatus%PID%.txt"
set /a CLUSTER_START_NUM=300
set DBISCLUSTER=0

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"

set TARGETSTATUS_FILE="%AGENT_TMP_PATH%TargetStatus%PID%.txt"
set CLUSTERSTATUS_FILE="%AGENT_TMP_PATH%ClusterStatus%PID%.txt"
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

rem ASM instance auth mode. user/pwd or /
set ASMAuthMode=/
set /a FLAG_RACSTARTDB=0

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "InstanceName" DBINSTANCE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "AppName" DBNAME
call :UperTOLow !DBNAME! DBNAME_CASE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
call :UperTOLow !DBUSERL! DBUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Action" CHECKTYPE
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "IsASM" ISASM
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMDiskGroups" ASMGROUPNAMES
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMSIDNAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "IsIncludeArchLog" IS_INCLUDE_ARCH

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)

if "%ASMSIDNAME%" == "" (
    set ASMSIDNAME=+ASM
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
            more %CURRENTCMDLineRST% | findstr /c:"oradbaction.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oradbaction.bat %AGENT_ROOT% %PID%"
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

call :Log "DBInstance=%DBINSTANCE%;DBNAME=%DBNAME%;DBUSER=%DBUSER%;CHECKTYPE=%CHECKTYPE%;ISCLUSTER=%ISCLUSTER%;ISASM=%ISASM%;ASMUSER=%ASMUSER%;ASMGROUPNAMES=%ASMGROUPNAMES%;ASMInstanceName=%ASMSIDNAME%;AUTHMODE=!AUTHMODE!;CURRENTPID=!CURRENTPID!;IS_INCLUDE_ARCH=!IS_INCLUDE_ARCH!;"

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
if "%ISASM%" EQU "1" (
    sc query state= all | findstr /i "OracleASMService%ASMSIDNAME%" > %QUREYSERVICE%
    for /f "tokens=1,2 delims=:" %%a in ('type %QUREYSERVICE%') do (
        if "SERVICE_NAME"=="%%a" (
            for /f "tokens=1,2 delims=+" %%i in ("%%b") do (
                set ASMSIDNAME=+%%j
            )
        )
    )
    call :DeleteFile %QUREYSERVICE%
    call :Log "check ASM Instance=!ASMSIDNAME!"
)

rem *****************************Check type ******************************************
set ORACLEDBSERVICE=OracleService%DBINSTANCE%

if "%CHECKTYPE%" EQU "1" (
    if "%ISCLUSTER%" EQU "1" (
        call :StopRACInstance
    ) else (
        call :StopSingleInstance
    )
) else (
    if "%ISCLUSTER%" EQU "0" (
        call :StartSingleInstance
    ) else (
        call :StartRACInstance
        call :Log "cluster try to start num[!CLUSTER_START_NUM!]."

        rem sleep 10 minute, and check db status
        set /a RACNUM=0
        set /a FLAG=0

:StartINSTAfterRAC
        if !RACNUM! LSS !CLUSTER_START_NUM! (
            rem sleep 2 second
            call :WinSleep 2
            rem sleep end
            
            set /a RACNUM=!RACNUM! + 1
            call :Log "RACNUM=!RACNUM!"
            
            call :CheckInstanceStatus RetCode
            rem instance status is not ready
            if !RetCode! EQU 2 (
                if !FLAG_RACSTARTDB! EQU 1 (
                    rem Clear resource after cluster start failed, if starting the single instance, it will be stopped by cluster.
                    call :Log "Cluster have close instance, now begin to start it."
                    call :WinSleep 60
                    call :Log "Sleep complete."
                    rem sleep end
                    call :StartSingleInstance
                    set /a FLAG=1
                    goto :StartINSTAfterRAC_END
                ) else (
                    goto :StartINSTAfterRAC
                )
            )
            
            rem instance is closed
            if !RetCode! EQU 1 (
                rem after cluster start cluster failed, cluster will clean resource, if now start instance will closed by cluster,
                rem wait 120 for cluster clean resource, and start instance
                call :Log "CheckInstanceStatus retcode=1, begin to start single database."
                call :WinSleep 120
                call :Log "Sleep complete."
                rem sleep end
                call :StartSingleInstance
                set /a FLAG=1
                goto :StartINSTAfterRAC_END
            )

            rem instace is opened
            if !RetCode! EQU 0 (
                call :Log "Database is open."
                set /a FLAG=1
                goto :StartINSTAfterRAC_END
            )
            
            goto :StartINSTAfterRAC
        ) else (
            call :Log "!RACNUM! is bigger !CLUSTER_START_NUM!, begin to start single database."
            call :StartSingleInstance
            goto :StartINSTAfterRAC_END
        )
    )
)

:StartINSTAfterRAC_END
call :DeleteFile %SERVICESTATUS%
exit 0

:StopRACInstance
    call :CreatShutdownSql %SHUTDOWNSCRIPT%
    call :CreatLoginSql %LOGININSCRIPT%
    rem Check Service start 
    for /f "tokens=2 delims=:" %%i in ('sc query %ORACLEDBSERVICE% ^| findstr /i "STATE"') do (
        set INSTANCESTATUS=%%i
        for /f "tokens=1 delims= " %%j in ("!INSTANCESTATUS!") do (
            rem Service has started
            if %%j EQU 4 (
                call :Log "Exec sql to check login database."
                call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %LOGININSCRIPT% %LOGININRST% %DBINSTANCE% "!DB_LOGIN!" 60 RetCode
                call :DeleteFile %LOGININSCRIPT%
                call :DeleteFile %LOGININRST%
                if !RetCode! NEQ 0 (
                    call :Log "Excute sql script for login in database failed."
                )

                call :Log "Exec sql to shutdown database."
                call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %SHUTDOWNSCRIPT% %SHUTDOWNRST% %DBINSTANCE% "!DB_LOGIN!" 300 RetCode
                call :DeleteFile %SHUTDOWNSCRIPT%
                call :DeleteFile %SHUTDOWNRST%
                if !RetCode! NEQ 0 (
                    call :Log "Excute sql script for shutdown database failed."
                )
                
                call :Log "Exec sc to stop service."
                sc stop %ORACLEDBSERVICE% >nul
                if !errorlevel! NEQ 0 (
                    call :Log "Shutdown database %DBNAME% failed."
                ) else (
                    set /a DelayCount=0
                    call :WaitService %QUERYINTERVAL% %CIRCLETIME% %ORACLEDBSERVICE% 1
                    call :Log "Shutdown the database %ORACLEDBSERVICE% service Successful."
                )
            ) else (
                if %%j NEQ 1 (
                    call :Log "The %ORACLEDBSERVICE% Service Status isn't Right."
                ) else (
                    call :Log "The %ORACLEDBSERVICE% Service isn't Started."   
                )
            )
        )
    )

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

rem ************************** check instance status ***********************************
:GetInstanceStatus
    echo select status from v$instance; > %CHECKINSTANCESTATUS%
    echo exit >> %CHECKINSTANCESTATUS%

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

rem check instance status
rem return 0, instance status is opened
rem return 1, instance is closed
rem return 2, instance resource is not ready
:CheckInstanceStatus
    call :GetInstanceStatus INSTStatus RetCode
    rem instance resource is not ready
    if !RetCode! NEQ 0 (
        set /a %~1 = 2
        goto :eof
    )
    
    rem retry 600 second
    set /a STATUS_NUM=0
    set /a STATUS_MAXNUM=300
:CheckINSTSatatus_INNER
    if !STATUS_NUM! geq !STATUS_MAXNUM! (
        set /a %~1 = 1
        goto :eof
    )
    
    call :GetInstanceStatus INSTStatus RetCode
    if !RetCode! NEQ 0 (
        set /a %~1 = 1
        goto :eof
    )
    
    rem STARTED - After STARTUP NOMOUNT
    rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    rem OPEN - After STARTUP or ALTER DATABASE OPEN
    rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if "!INSTStatus:~0,4!" == "OPEN" (
        set /a %~1 = 0
        goto :eof
    )
    
    set /a STATUS_NUM=!STATUS_NUM! + 1
    rem sleep 2 second
    call :WinSleep 2
    rem sleep end
    goto :CheckINSTSatatus_INNER
goto :eof

:StartRACInstance
    set ORANEW=0
    if "!PREDBVERSION!" == "11.2" (
        set ORANEW=1
    )
    
    if "!PREDBVERSION:~0,2!" GEQ "12" (
        set ORANEW=1
    )

    rem check cluster status
    if "!ORANEW!" == "1" (
        "%GRIDHOMEPATH%\bin\crsctl" stat res -t 1>>%CHECKCLUSTERSTATUS% 2>&1
    ) else (
        "%GRIDHOMEPATH%\bin\crs_stat" -t 1>>%CHECKCLUSTERSTATUS% 2>&1
    )
    
    rem CRS-4535: Cannot communicate with Cluster Ready Services
    rem CRS-0184: Cannot communicate with the CRS daemon.
    more %CHECKCLUSTERSTATUS% | findstr "CRS-4535" >nul
    if !errorlevel! EQU 0 (
        rem cluster not started
        set /a CLUSTER_START_NUM=300
    ) else (
        rem cluster had started
        set /a CLUSTER_START_NUM=60
    )
    call :DeleteFile %CHECKCLUSTERSTATUS%
    
    call :Log "Begin start RAC[%DBVERSION%]"
    if "!ORANEW!" == "1" (
        "%GRIDHOMEPATH%\bin\crsctl" start has >> %LOGFILEPATH%
        "%GRIDHOMEPATH%\bin\crsctl" start cluster -all >> %LOGFILEPATH%
    ) else (
        "%GRIDHOMEPATH%\bin\crs_start" -all >> %LOGFILEPATH%
    )

    call :Log "End start RAC[%DBVERSION%]"
    
    set /a RACNUM=0
    set /a FLAG_RACSTARTDB=0
:CHECKCLUSTER_INNER
    if !RACNUM! LSS 30 (
        rem sleep 1 second
        call :WinSleep 2
        rem sleep end

        set /a RACNUM=!RACNUM! + 1
        call :Log "After start RAC, RACNUM=!RACNUM!"
        
        call :GetInstanceStatus INSTStatus RetCode
        rem instance status is not ready
        if !RetCode! EQU 0 (
            set /a FLAG_RACSTARTDB=1
        )
        goto :CHECKCLUSTER_INNER
    )
    call :Log "Check !DBINSTANCE! start status, FLAG_RACSTARTDB=!FLAG_RACSTARTDB!."
    set /a CLUSTER_START_NUM=!CLUSTER_START_NUM! - 30
    
    rem get node list
    call :DeleteFile !CLUSTERSTATUS_FILE!
    call :DeleteFile !TARGETSTATUS_FILE!
    "%GRIDHOMEPATH%\bin\crsctl" status server | findstr "NAME=" > !CLUSTERSTATUS_FILE!
    for /f "tokens=1,2 delims==" %%a in ('type !CLUSTERSTATUS_FILE!') do (
        set nodename=%%b
        call :Log "nodename=!nodename!"
        rem 2. get reliationship between instance name and database name
        "%GRIDHOMEPATH%\bin\crsctl" status resource ora.!DBNAME_CASE!.db -f -n !nodename! | findstr "USR_ORA_INST_NAME=!DBINSTANCE!$"
        if !errorlevel! EQU 0 (
            "%GRIDHOMEPATH%\bin\crsctl" status resource ora.!DBNAME_CASE!.db -v -n !nodename! | findstr "TARGET=" > !TARGETSTATUS_FILE!
        )
    )
    call :DeleteFile !CLUSTERSTATUS_FILE!
    
    rem target status is the target which cluster will start database
    rem if get empty string, will wait for 10 minite
    rem if have ONLINE state, will wait for 10 minite, because agent can't get the node name
    call :Log "Get !DBNAME_CASE! target status:"
    more !TARGETSTATUS_FILE! >> %LOGFILEPATH%
    more !TARGETSTATUS_FILE! | findstr /i "ONLINE" >nul
    if !errorlevel! NEQ 0 (
        set /a CLUSTER_START_NUM=5
    )
    call :DeleteFile !TARGETSTATUS_FILE!
    
    goto :eof

:StartupNoMountDatabase
    call :Log "Begin startup no mount database."
    call :CreateStartupNoMountSql %RECOVERSCRIPT%

    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %RECOVERSCRIPT% %RECOVERSCRIPTRST% %DBINSTANCE% "!DB_LOGIN!" 120 RetCode
    call :DeleteFile %RECOVERSCRIPT%
    call :DeleteFile %RECOVERSCRIPTRST%
    
    if "!RetCode!" == "!ERROR_DB_ALREADYRUNNING!" (
        call :Log "The database !DBINSTANCE! already started."
        set /a %~1 = 0
        goto :eof
    )
    
    if "!RetCode!" == "0" (
        call :Log "Start database !DBINSTANCE! successful."
        set /a %~1 = 0
        goto :eof
    )
    
    if "!RetCode!" == "!ERROR_ORACLE_ANOTHER_STARTING!" (
        call :Log "Start database !DBINSTANCE! failed: another startup/shutdown operation of this instance inprogress."
        set /a %~1 = 1
        goto :eof
    )
    
    if "!RetCode!" == "!ERROR_ORACLE_DB_BUSY!" (
        call :Log "Start database !DBINSTANCE! failed: database busy."
        set /a %~1 = 1
        goto :eof
    )
    
    call :Log "Startup nomount database !DBINSTANCE! file failed."
    exit !RetCode!
goto :eof
    
rem ************************************************************************
rem function name: StartupMountDatabase
rem aim:           StartupMountDatabase
rem input:         
rem output:        
rem ************************************************************************
:MountDatabase
    call :Log "Begin mount database."
    call :CreateMountSql %MOUNTDBSCRIPT%
    
    rem Execute Mount database
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %MOUNTDBSCRIPT% %MOUNTDBRST% %DBINSTANCE% "!DB_LOGIN!" 120 RetCode
    call :DeleteFile %MOUNTDBSCRIPT%
    call :DeleteFile %MOUNTDBRST%
    
    if "!RetCode!" == "!ERROR_DB_ALREADYMOUNT!" (
        call :Log "The database !DBINSTANCE! already mounted."
        set /a %~1 = 0
        goto :eof
    )
    
    if "!RetCode!" == "0" (
        call :Log "Mount database !DBINSTANCE! successful."
        set /a %~1 = 0
        goto :eof
    )
    
    if "!RetCode!" == "!ERROR_ORACLE_ANOTHER_STARTING!" (
        call :Log "Start database !DBINSTANCE! failed: another startup/shutdown operation of this instance inprogress."
        set /a %~1 = 1
        goto :eof
    )
    
    if "!RetCode!" == "!ERROR_ORACLE_DB_BUSY!" (
        call :Log "Start database !DBINSTANCE! failed: database busy."
        set /a %~1 = 1
        goto :eof
    )
    
    call :Log "Mount database !DBINSTANCE! failed."
    exit !RetCode!
goto :eof

rem ************************************************************************
rem function name: CreateArchiveDirectory
rem aim:           CreateArchiveDirectory
rem input:         
rem output:        
rem ************************************************************************
:CreateArchiveDirectory
    rem *************************begin 2014-01-16 modify create archive directory******************
    call :Log "Begin create archive dest directory."
    set TITLE_LOGMODE=Database log mode
    set TITLE_ARCHIVE_DEST=Archive destination
    set STRARCHIVEDEST=

    echo ALTER SESSION SET NLS_LANGUAGE='AMERICAN'; > %GETARCHIVEINFOFILE%
    echo archive log list; >> %GETARCHIVEINFOFILE%
    echo exit >> %GETARCHIVEINFOFILE%
    
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GETARCHIVEINFOFILE% %GETARCHIVEINFORST% %DBINSTANCE% "!DB_LOGIN!" 60 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for get archive info failed."
        call :DeleteFile %GETARCHIVEINFOFILE%
        call :DeleteFile %GETARCHIVEINFORST%
        goto :endcreate
    ) else (
        rem get dest info
        for /f "skip=10 delims=" %%i in ('type %GETARCHIVEINFORST%') do (
            set strline=%%i
            if "!strline:~0,19!" == "!TITLE_ARCHIVE_DEST!" (
                for /f "tokens=3 delims= " %%a in ("!strline!") do (
                    set STRARCHIVEDEST=%%a
                    call :Log "STRARCHIVEDEST=%%i"
                )
            )
        )
        
        call :DeleteFile %GETARCHIVEINFOFILE%
        call :DeleteFile %GETARCHIVEINFORST%
        call :Log "Dest=!STRARCHIVEDEST!"
        if "!STRARCHIVEDEST!" == "USE_DB_RECOVERY_FILE_DEST" (
            echo set linesize 300; > %GETFLASHINFOFILE%
            echo col VALUE for a255; >> %GETFLASHINFOFILE%
            echo select VALUE from v$spparameter where name = 'db_recovery_file_dest'; >> %GETFLASHINFOFILE%
            echo exit >> %GETFLASHINFOFILE%
            
            call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GETFLASHINFOFILE% %GETFLASHINFORST% %DBINSTANCE% "!DB_LOGIN!" 60 RetCode
            if !RetCode! NEQ 0 (
                call :Log "Excute sql script for get archive dest failed."
                call :DeleteFile %GETFLASHINFOFILE%
                call :DeleteFile %GETFLASHINFORST%
                goto :endcreate
            ) else (               
                set /a FLG_FIND=0
                for /f "skip=12" %%i in ('type %GETFLASHINFORST%') do (
                    if "!FLG_FIND!" EQU "1" (
                        set STRARCHIVEDEST=%%i
                        goto :end
                    )
                    
                    echo %%i | findstr \-\-\-\-\-\-
                    if "!errorlevel!" EQU "0" (
                        set /a FLG_FIND=1
                    )
                )
            )
        )
:end
        call :Log "Archive Dest=!STRARCHIVEDEST!"
        call :DeleteFile %GETFLASHINFOFILE%
        call :DeleteFile %GETFLASHINFORST%
        
        set FSTCHAR=!STRARCHIVEDEST:~1,1!
        if "!FSTCHAR!" == ":" (
            if not exist "!STRARCHIVEDEST!" (
                md "!STRARCHIVEDEST!"
                
                if not exist "!STRARCHIVEDEST!" (
                    call :Log "create !STRARCHIVEDEST! failed."
                ) else (
                    call :Log "create !STRARCHIVEDEST! successful."
                )
            ) else (
                call :Log "!STRARCHIVEDEST! exists."
            )
        ) else (
            call :Log "!STRARCHIVEDEST! is not filesystem, do not create."
        )
    )
:endcreate
    call :Log "End create archive dest directory."
    rem *************************end 2014-01-16 modify create archive directory******************
    goto :eof

rem get database count
:GetDatabaseTBCount
    call :Log "Begin get database tb count."
    set /a %~1 = 0
    call :CreateGetBackupTbCountSql %GETBACKUPTBCOUNT%
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GETBACKUPTBCOUNT% %GETBACKUPTBCOUNTRST% %DBINSTANCE% "!DB_LOGIN!" 60 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for get backup count failed."
        call :DeleteFile %GETBACKUPTBCOUNT%
        call :DeleteFile %GETBACKUPTBCOUNTRST%
        set /a %~2 = !RetCode!
    ) else (
        set /a BACKUPTBCOUNT=0
        set /a FLG_FIND=0
        for /f "skip=12 delims=" %%j in ('type %GETBACKUPTBCOUNTRST%') do (
            if "!FLG_FIND!" EQU "1" (
                set /a BACKUPTBCOUNT=%%j
                call :Log "BACKUPTBCOUNT=!BACKUPTBCOUNT!."
                goto :EndCount
            )
            
            echo %%j | findstr \-\-\-\-\-\-
            if "!errorlevel!" EQU "0" (
                set /a FLG_FIND=1
            )
        )
:EndCount
        set /a %~1 = !BACKUPTBCOUNT!
        set /a %~2 = 0
        call :DeleteFile %GETBACKUPTBCOUNT%
        call :DeleteFile %GETBACKUPTBCOUNTRST%
    )
    call :Log "End get database tb count."
    goto :eof

rem ************************************************************************
rem function name: TurnDatabaseFlashBackOff
rem aim:           TurnDatabaseFlashBackOff
rem input:         
rem output:        
rem **********************************************************************
:TurnDatabaseFlashBackOff
    echo select flashback_on from v$database; > %FLASHBACKSTATUS%
    echo exit >> %FLASHBACKSTATUS%
    
    rem *************************startup database********************
    call :Log "Exec sql to get flashback off status of database."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %FLASHBACKSTATUS% %FLASHBACKSTATUSRST% %DBINSTANCE% "!DB_LOGIN!" 120 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for get database flashback status failed."
        call :DeleteFile %FLASHBACKSTATUS%
        call :DeleteFile %FLASHBACKSTATUSRST%
        set /a %~1 = 1
    ) else (
        set /a FLG_FIND=0
        for /f "skip=12" %%i in ('type %FLASHBACKSTATUSRST%') do (
            if "!FLG_FIND!" EQU "1" (
                set FLASHBACKTATUS=%%i
                call :Log "FLASHBACKTATUS=%%i"
                goto :JUMPOUT_TURNOFF
            )
            
            echo %%i | findstr \-\-\-\-\-\-
            if "!errorlevel!" EQU "0" (
                set /a FLG_FIND=1
            )
        )
:JUMPOUT_TURNOFF
        call :DeleteFile %FLASHBACKSTATUS%
        call :DeleteFile %FLASHBACKSTATUSRST%
    )
    
    call :Log "FLASHBACKTATUS=!FLASHBACKTATUS!"
    if "!FLASHBACKTATUS!" == "NO" (
        call :Log "Database flashback is off."
        set /a %~1 = 0
        goto :eof
    )
    
    echo alter database flashback off; > %FLASHBACKSTATUS%
    echo exit >> %FLASHBACKSTATUS%
    
    rem *************************alter database flashback off********************
    call :Log "Exec sql to turn database flackback off."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %FLASHBACKSTATUS% %FLASHBACKSTATUSRST% %DBINSTANCE% "!DB_LOGIN!" 120 RetCode
    call :DeleteFile %FLASHBACKSTATUS%
    call :DeleteFile %FLASHBACKSTATUSRST%
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for turn off database flashback failed."
        set /a %~1 = 1
    ) else (
        call :Log "turn off database flashback succ."
        set /a %~1 = 0
    )
goto :eof

rem ************************************************************************
rem function name: RecoverDatabase
rem aim:           RecoverDatabase
rem input:         
rem output:        
rem **********************************************************************
:RecoverDatabase
    set isIncludeArch=%~1
    call :Log "Begin recover database, isIncludeArch !isIncludeArch!."
    call :GetDatabaseTBCount backupCount RetCode
    if !RetCode! NEQ 0 (
        call :Log "Get database tb count failed."
        exit !RetCode!
    )
    
    if !backupCount! EQU 0 (
        call :Log "There are no backup tablespace, recover database successful."
        set /a %~2 = 0
        goto :eof
    )

    call :Log "Create end backup sql script file."
    call :CreatEndBackupSql %STARTUPECOVERSCRIPT%
    
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %STARTUPECOVERSCRIPT% %STARTRECOVERRST% %DBINSTANCE% "!DB_LOGIN!" 120 RetCode
    call :DeleteFile %STARTUPECOVERSCRIPT%
    call :DeleteFile %STARTRECOVERRST%
    if "!RetCode!" == "0" (
        call :Log "Recover DataBase !DBINSTANCE! successful."
        set /a %~2 = 0
        goto :eof
    )
    
    call :Log "Recover database !DBINSTANCE! failed."
    exit !RetCode!
goto :eof

rem ************************************************************************
rem function name: OpenDatabase
rem aim:           OpenDatabase
rem input:         
rem output:        
rem **********************************************************************
:OpenDatabase
    call :Log "Begin open database."
    call :CreatStartupSql %STARTUPSCRIPT%
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %STARTUPSCRIPT% %STARTUPRST% %DBINSTANCE% "!DB_LOGIN!" 120 RetCode
    call :DeleteFile %STARTUPSCRIPT%
    call :DeleteFile %STARTUPRST%
    if "!RetCode!" == "!ERROR_DB_ALREADYOPEN!" (
        call :Log "The Database !DBINSTANCE! already open."
        set /a %~1 = 0
        goto :eof
    )
    
    if "!RetCode!" == "0" (
        call :Log "Open DataBase !DBINSTANCE! successful."
        set /a %~1 = 0
        goto :eof
    )
    
    if "!RetCode!" == "!ERROR_ORACLE_ANOTHER_STARTING!" (
        call :Log "Start database !DBINSTANCE! failed: another startup/shutdown operation of this instance inprogress."
        set /a %~1 = 1
        goto :eof
    )

    if "!RetCode!" == "!ERROR_ORACLE_DB_BUSY!" (
        call :Log "Start database !DBINSTANCE! failed: database busy."
        set /a %~1 = 1
        goto :eof
    )
    
    call :Log "Open database !DBINSTANCE! failed."
    exit !RetCode!
goto :eof
   
:StartWindowsServices
    set SERVICENAME=%~1
    set /a %~2 = 1
    for /f "tokens=2 delims=:" %%i in ('sc query %SERVICENAME% ^| findstr /i "STATE"') do (
        set INSTANCESTATUS=%%i
        for /f "tokens=1 delims= " %%j in ("!INSTANCESTATUS!") do (
            rem Check Service is Start
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

    set DGLIST=%ASMGROUPNAMES%
    set /a %~2 = 0
    rem check asm diskgroup whether mount, mount if not mount
:BEGSPLIT
    for /f "tokens=1,* delims=+ " %%a in ("%DGLIST%") do (
        set dgName=%%a
        set DGLIST=%%b

        call :SetASMAuth
        echo alter diskgroup !dgName! !MModeStr!; > %MOUNTDISKGROUPSQL%
        echo exit; >> %MOUNTDISKGROUPSQL%

        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %MOUNTDISKGROUPSQL% %MOUNTDISKGROUPRST% %ASMSIDNAME% "!ASM_LOGIN!" 60 RetMount
        call :DeleteFile %MOUNTDISKGROUPSQL%
        call :DeleteFile %MOUNTDISKGROUPRST%
        
        rem get status of diskgroup
        call :SetASMAuth
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
    
rem ************************************************************************
rem function name: StartSingleInstance
rem aim:           StartSingleInstance
rem input:         
rem output:        
rem ************************************************************************
:StartSingleInstance
    set /a COMSTATE=4

    rem check status of database
    call :GetInstanceStatus INSTStatus RetCode    
    rem STARTED - After STARTUP NOMOUNT
    rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
    rem OPEN - After STARTUP or ALTER DATABASE OPEN
    rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
    if "!INSTStatus:~0,4!" == "OPEN" (
        set /a %~1 = 0
        call :Log "The status of database is open."
        goto :eof
    )
    
    if "%ISASM%" EQU "1" (
        rem start has--crsctl start has
        "%GRIDHOMEPATH%\bin\crsctl" start has >> %LOGFILEPATH%
        
        rem start css--crsctl start resource ora.cssd
        "%GRIDHOMEPATH%\bin\crsctl" start resource ora.cssd >> %LOGFILEPATH%
        
        rem start ASM instance-- start service,startup
        call :StartWindowsServices "OracleASMService!ASMSIDNAME!" RetCode
        if "!RetCode!" NEQ "0" (
            call :Log "Start OracleASMService!ASMSIDNAME! failed."
            exit %ERROR_SCRIPT_EXEC_FAILED%
        )
        
        call :checkASMAuthMode %ASMUSER% %ASMUSERPWD%
        rem start ASM instance
        call :CreatASMStartupSql %STARTASMSCRIPT%
        call :SetASMAuth
        call :Log "Execute sql to startup asm instance."
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %STARTASMSCRIPT% %STARTASMSCRIPTRST% %ASMSIDNAME% "!ASM_LOGIN!" 120 RetCode
        call :DeleteFile %STARTASMSCRIPT%
        call :DeleteFile %STARTASMSCRIPTRST%
        
        rem mount diskgoup
        call :DiskGroupManage 1 RetCode
        if "!RetCode!" EQU "1" (
            call :Log "Mount diskgroup failed."
        )
    )

    rem Check the database service started and Start Service
    for /f "tokens=2 delims=:" %%i in ('sc query %ORACLEDBSERVICE% ^| findstr /i "STATE"') do (
        set INSTANCESTATUS=%%i
        for /f "tokens=1 delims= " %%j in ("!INSTANCESTATUS!") do (
            rem Check Service is Start
            if %%j EQU 1 (
                sc start %ORACLEDBSERVICE% > nul
                if !errorlevel! NEQ 0 (
                    call :Log "Execute sc start %ORACLEDBSERVICE% command failed."
                    call :DeleteFile %SERVICESTATUS%
                    exit %ERROR_SCRIPT_EXEC_FAILED%
                ) else (
                    set /a DelayCount=0
                    call :WaitService %QUERYINTERVAL% %CIRCLETIME% %ORACLEDBSERVICE%
                    call :Log "Start the database %ORACLEDBSERVICE% service successful."
                ) 
            ) else (
                if %%j NEQ 4 (
                    call :Log "The %ORACLEDBSERVICE% Service Status isn't right."
                    exit %ERROR_SCRIPT_EXEC_FAILED%
                ) else (
                    call :Log "The %ORACLEDBSERVICE% Service has Started."   
                )
            )
        )
    )

    rem startup database
    call :StartupNoMountDatabase RetCode
    set /a RetryNum = 0
:startupDB
    if "!RetCode!" NEQ "0" (
        if !RetryNum! geq 5 (
            call :Log "Start nomount database failed, retried 5 times."
            exit %ERROR_SCRIPT_EXEC_FAILED%
        )
        
        rem sleep 10 second
        call :WinSleep 10
        rem sleep end
        
        call :Log "Try to startup nomount database[!RetryNum!]"
        call :StartupNoMountDatabase RetCode
        set /a RetryNum=!RetryNum! + 1
        goto :startupDB
    )
    
    rem startup database
    call :MountDatabase RetCode
    set /a RetryNum = 0
:mountDB
    if "!RetCode!" NEQ "0" (
        if !RetryNum! geq 5 (
            call :Log "Mount database failed, retried 5 times."
            exit %ERROR_SCRIPT_EXEC_FAILED%
        )
        
        rem sleep 10 second
        call :WinSleep 10
        rem sleep end
        
        call :Log "Try to mount database[!RetryNum!]"
        call :MountDatabase RetCode
        set /a RetryNum=!RetryNum! + 1
        goto :mountDB
    )

    call :CreateArchiveDirectory

    rem turn off database flashback off
    call :TurnDatabaseFlashBackOff RetCode
    set /a RetryNum = 0
:turnOffFlashBack
    if "!RetCode!" NEQ "0" (
        if !RetryNum! geq 5 (
            call :Log "turn off database flashback off have retry 5 times, turn off database flashback off failed."
            exit %ERROR_SCRIPT_EXEC_FAILED%
        )
        
        rem sleep 10 second
        call :WinSleep 10
        rem sleep end
        
        call :Log "turn off database flashback off[!RetryNum!]"
        call :TurnDatabaseFlashBackOff RetCode
        set /a RetryNum=!RetryNum! + 1
        goto :turnOffFlashBack
    )

    rem recover database
    call :RecoverDatabase %IS_INCLUDE_ARCH% RetCode
    set /a RetryNum = 0
:recoverDB
    if "!RetCode!" NEQ "0" (
        if !RetryNum! geq 5 (
            call :Log "Recover database have retry 5 times, recover database failed."
            exit %ERROR_SCRIPT_EXEC_FAILED%
        )
        
        rem sleep 10 second
        call :WinSleep 10
        rem sleep end
        
        call :Log "Try to recover database[!RetryNum!]"
        call :RecoverDatabase %IS_INCLUDE_ARCH% RetCode
        set /a RetryNum=!RetryNum! + 1
        goto :recoverDB
    )
    
    rem open database
    call :OpenDatabase RetCode
    set /a RetryNum = 0
:openDB
    if "!RetCode!" NEQ "0" (
        if !RetryNum! geq 5 (
            call :Log "alter database open have retry 5 times, alter database open failed."
            exit %ERROR_SCRIPT_EXEC_FAILED%
        )
        
        rem sleep 10 second
        call :WinSleep 10
        rem sleep end
        
        call :Log "try to open database[!RetryNum!]"
        call :OpenDatabase RetCode
        set /a RetryNum=!RetryNum! + 1
        goto :openDB
    )
    goto :EOF

rem ************************************************************************
rem function name: StopSingleInstance
rem aim:           StopSingleInstance
rem input:         
rem output:        
rem ************************************************************************
:StopSingleInstance
    set /a COMSTATE=1
    
    call :CreatShutdownSql %SHUTDOWNSCRIPT%
    call :CreatLoginSql %LOGININSCRIPT%
    rem Check Service start 
    for /f "tokens=2 delims=:" %%i in ('sc query %ORACLEDBSERVICE% ^| findstr /i "STATE"') do (
        set INSTANCESTATUS=%%i
        for /f "tokens=1 delims= " %%j in ("!INSTANCESTATUS!") do (
            rem Service has started
            if %%j EQU 4 (
                call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %LOGININSCRIPT% %LOGININRST% %DBINSTANCE% "!DB_LOGIN!" 60 RetCode
                call :DeleteFile %LOGININSCRIPT%
                call :DeleteFile %LOGININRST%
                if !RetCode! NEQ 0 (
                    call :Log "Excute sql script for login in database failed."
                )

                call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %SHUTDOWNSCRIPT% %SHUTDOWNRST% %DBINSTANCE% "!DB_LOGIN!" 300 RetCode
                call :DeleteFile %SHUTDOWNSCRIPT%
                call :DeleteFile %SHUTDOWNRST%
                if !RetCode! NEQ 0 (
                    if !RetCode! NEQ !ERROR_RECOVER_INSTANCE_NOSTART! (
                        if !RetCode! NEQ !ERROR_ORACLE_NOT_MOUNTED! (
                            if !RetCode! NEQ !ERROR_ORACLE_NOT_OPEN! (
                                call :Log "Excute sql script for shutdown database failed, RetCode:!RetCode!."
                                exit !RetCode!
                            )
                        )
                    )
                )

                sc stop %ORACLEDBSERVICE% >nul
                if !errorlevel! NEQ 0 (
                    call :Log "Shutdown database %DBNAME% failed."
                    exit %ERROR_SCRIPT_EXEC_FAILED%
                ) else (
                    set /a DelayCount=0
                    call :WaitService %QUERYINTERVAL% %CIRCLETIME% %ORACLEDBSERVICE% 1
                    call :Log "Shutdown the database %ORACLEDBSERVICE% service Successful."
                )
            ) else (
                if %%j NEQ 1 (
                    call :Log "The %ORACLEDBSERVICE% Service Status isn't Right."
                    exit %ERROR_SCRIPT_EXEC_FAILED%
                ) else (
                    call :Log "The %ORACLEDBSERVICE% Service isn't Started."   
                )
            )
        )
    )
  
    goto :EOF

:WinSleep
    timeout %1 > nul
    goto :eof
    
rem ************************************************************************
rem function name: CreatLoginSql
rem aim:           Create temporary sql script for recover database
rem input:         the sql script name
rem output:        RECOVERSCRIPT
rem ************************************************************************
:CreatLoginSql
    echo exit > "%~1"
    goto :EOF

:CreatEndBackupSql
    echo alter database end backup; > "%~1"
    echo exit >> "%~1"
    goto :EOF

rem ************************************************************************
rem function name: CreateGetBackupTbCountSql
rem aim:           Create get backup instance SQL 
rem input:         the sql script name
rem output:        EXERECOVERSCRIPT 2012-05-14
rem ************************************************************************
:CreateGetBackupTbCountSql
    echo select count(*) from v$backup where STATUS='ACTIVE'; > "%~1"
    echo exit >> "%~1"
    goto :eof

rem ************************************************************************
rem function name: CreateStartupNoMountSql
rem aim:           Create temporary sql script for startup database
rem input:         the sql script name
rem output:        MOUNTDBSCRIPT
rem ************************************************************************
:CreateStartupNoMountSql
    call :Log "Create start nomount database script."
    echo startup nomount; > "%~1"
    echo exit >> "%~1"
    goto :EOF

rem ************************************************************************
rem function name: CreateMountSql
rem aim:           Create temporary sql script for startup database
rem input:         the sql script name
rem output:        MOUNTDBSCRIPT
rem ************************************************************************
:CreateMountSql
    call :Log "Create mount database script."
    echo alter database mount; > "%~1"
    echo exit >> "%~1"
    goto :EOF

rem ************************************************************************
rem function name: CreatStartupSql
rem aim:           Create temporary sql script for startup database
rem input:         the sql script name
rem output:        STARTUPSCRIPT
rem ************************************************************************
:CreatStartupSql
    call :Log "Create alter database open script file."
    echo alter database open; > "%~1"
    echo exit >> "%~1"
    goto :EOF

rem ************************************************************************
rem function name: CreatShutdownSql
rem aim:           Logon database and execute sql script function
rem input:         the sql script name to execute
rem output:        RESULTFILE
rem ************************************************************************
:CreatShutdownSql
    echo shutdown immediate; > "%~1"
    echo exit >> "%~1"
    goto :EOF    

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
rem function name: ShutdownService
rem aim:           ShutdownService until the status of database is stop
rem input:         DBInstanceName
rem output:        
rem ************************************************************************
:ShutdownService
rem **********Shutdown Service**********
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

rem Convert str to Low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j d l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    
    set %~2=!CONVERTSTR!
goto :EOF

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

    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %SHUTDOWNSCRIPT% %SHUTDOWNRST% %ASMSIDNAME% "!ASM_LOGIN!" 30 RetCode
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
        set ASM_LOGIN=!ASMAuthMode! as sysasm
    )
goto :eof

endlocal

