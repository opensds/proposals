@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


rem @dest:   to check oracle archive dest using
rem @date:   2005-01-24
rem @authr:  
rem @modify: 2005-01-24

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

set LOGFILE=oraclecheckarchive.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

set ARGFILEPRE=ArgFile
set ARGFILENAME="%AGENT_TMP_PATH%%ARGFILEPRE%%PID%"

rem create archive directory
set GETARCHIVEINFOFILE="%AGENT_TMP_PATH%GetArchive%PID%.sql"
set GETARCHIVEINFORST="%AGENT_TMP_PATH%GetArchiveRST%PID%.txt"
set GETFLASHINFOFILE="%AGENT_TMP_PATH%GetFlash%PID%.sql"
set GETFLASHINFORST="%AGENT_TMP_PATH%GetFlashRST%PID%.txt"
set GETDISKUSEDING="%AGENT_TMP_PATH%DiskUsing%PID%.txt"

set ARCHIVEDESTSQL="%AGENT_TMP_PATH%ArchiveDestSQL%PID%.sql"
set ARCHIVEDESTRST="%AGENT_TMP_PATH%ArchiveDestRST%PID%.txt"
set ARCHIVE_DEST_LIST="%AGENT_TMP_PATH%ArchiveDestList%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0

set /a DBARCHIVE_LIMEN=0
set DB_LOGIN=
set ASM_LOGIN=

set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set QUREYSERVICE="%AGENT_TMP_PATH%QueryService%PID%.txt"
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
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "UserName" DBUSERL
call :UperTOLow !DBUSERL! DBUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "Password" DBUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMSIDNAME
if "!ASMSIDNAME!" == "" (
    set ASMSIDNAME=+ASM
)
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ArchiveThreshold" LIMEN
if !LIMEN! == "" (
    set /a DBARCHIVE_LIMEN=0
) else (
    set /a DBARCHIVE_LIMEN=!LIMEN!
)

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)

call :SetDBAuth

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
            more %CURRENTCMDLineRST% | findstr /c:"oraclecheckarchive.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oraclecheckarchive.bat %AGENT_ROOT% %PID%"
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

echo "DBINSTANCE=!DBINSTANCE!;DBUSER=!DBUSER!;ASMSIDNAME=!ASMSIDNAME!;DBARCHIVE_LIMEN=!DBARCHIVE_LIMEN!;CURRENTPID=!CURRENTPID!;"
call :Log "DBINSTANCE=!DBINSTANCE!;DBUSER=!DBUSER!;ASMSIDNAME=!ASMSIDNAME!;DBARCHIVE_LIMEN=!DBARCHIVE_LIMEN!;CURRENTPID=!CURRENTPID!;AUTHMODE=!AUTHMODE!;"

set nls_lang=AMERICAN_AMERICA.ZHS16GBK
rem ************************get the version information************************
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION

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

if !DBARCHIVE_LIMEN! NEQ 0 (
    call :checkArchiveModeAndArchiveUsing RetCode
    if !RetCode! NEQ 0 (
        call :Log "Exec fuction failed."
        exit %ERROR_SCRIPT_EXEC_FAILED%
    )
) else (
    call :Log "DBARCHIVE_LIMEN is zero, not to check archive using."
)

exit 0

rem Print log function, controled by "NEEDLOGFLG".
:Log
    echo %date:~0,10% %time:~0,8% [!CURRENTPID!] [%username%] %~1 >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
    goto :EOF

rem Delete file function, it can delete many files.
:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
    goto :EOF

rem Convert str to low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    
    set %~2=!CONVERTSTR!
    
goto :EOF

rem ************************************************************************
rem function name: checkArchiveModeAndArchiveUsing
rem aim:           checkArchiveModeAndArchiveUsing
rem input:         
rem output:        
rem ************************************************************************
:checkArchiveModeAndArchiveUsing
    rem *************************begin 2014-12-15 check archive directory******************
    set /a USEDCAPACITY=0
    set /a ALLCAPACITY=0
    set /a FREECAPACTIY=0
    set /a FAST_USEDCAPACITY=0
    set /a FAST_ALLCAPACITY=0
    set /a IS_RECOVERY_FILE_DEST=0
    
    call :Log "Begin check archive dest directory."
    set TITLE_LOGMODE=Database log mode
    set TITLE_ARCHIVE_DEST=Archive destination
    set STRARCHIVEDEST=
    
    echo ALTER SESSION SET NLS_LANGUAGE='AMERICAN'; > %GETARCHIVEINFOFILE%
    echo select LOG_MODE from v$database; >> %GETARCHIVEINFOFILE%
    echo exit >> %GETARCHIVEINFOFILE%
    
    call :Log "Execute sql to get database log mode."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GETARCHIVEINFOFILE% %GETARCHIVEINFORST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for get archive info failed."
        call :DeleteFile %GETARCHIVEINFOFILE%
        call :DeleteFile %GETARCHIVEINFORST%
        exit !RetCode!
    )
    
    rem LOGMODE:NOARCHIVELOG|ARCHIVELOG
    more %GETARCHIVEINFORST% | find /I "NOARCHIVELOG" >nul
    if !errorlevel! EQU 0 (
        call :Log "Database is no in archive mode."
        more %GETARCHIVEINFORST% >> %LOGFILEPATH%
        call :Log "info end."
        call :DeleteFile %GETARCHIVEINFOFILE%
        call :DeleteFile %GETARCHIVEINFORST%
        exit %ERROR_ORACLE_NOARCHIVE_MODE%
    )
        
    call :DeleteFile %GETARCHIVEINFOFILE%
    call :DeleteFile %GETARCHIVEINFORST%
    
    rem status of archive dest
    rem VALID - Initialized and available
    rem INACTIVE - No destination information
    rem DEFERRED - Manually disabled by the user
    rem ERROR - Error during open or copy
    rem DISABLED - Disabled after error
    rem BAD PARAM - Parameter has errors
    rem ALTERNATE - Destination is in an alternate state
    rem             alter system LOG_ARCHIVE_DEST_STATE_n={enable|defer|alternate}
    rem             enable - Specifies that a valid log archive destination can be used for a subsequent archiving operation (automatic or manual). This is the default.
    rem             defer - Specifies that valid destination information and attributes are preserved, but the destination is excluded from archiving operations until re-enabled.
    rem             alternate - Specifies that a log archive destination is not enabled but will become enabled if communications to another destination fail.
    rem FULL - Exceeded quota size for the destination
    echo ALTER SESSION SET NLS_LANGUAGE='AMERICAN'; > %ARCHIVEDESTSQL%
    echo set linesize 300; >> %ARCHIVEDESTSQL%
    echo col DESTINATION for a255; >> %ARCHIVEDESTSQL%
    echo select DESTINATION from v$archive_dest where STATUS='VALID'; >> %ARCHIVEDESTSQL%
    echo exit >> %ARCHIVEDESTSQL%
    
    call :Log "Execute sql to get destination of database."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %ARCHIVEDESTSQL% %ARCHIVEDESTRST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for get archive dest list failed."
        call :DeleteFile %ARCHIVEDESTSQL%
        call :DeleteFile %ARCHIVEDESTRST%
        exit !RetCode!
    )
    call :DeleteFile %ARCHIVEDESTSQL%
    type %ARCHIVEDESTRST% | findstr /i ":\\" > %ARCHIVE_DEST_LIST%
    type %ARCHIVEDESTRST% | findstr /i "^[a-zA-Z]:$" >> %ARCHIVE_DEST_LIST%
    type %ARCHIVEDESTRST% | findstr /i "^+" >> %ARCHIVE_DEST_LIST%
    type %ARCHIVEDESTRST% | findstr /i "USE_DB_RECOVERY_FILE_DEST" >> %ARCHIVE_DEST_LIST%
    call :DeleteFile %ARCHIVEDESTRST%
    
    for /f %%i in ('type %ARCHIVE_DEST_LIST%') do (
        set STRARCHIVEDEST=%%i
        call :Log "Dest=!STRARCHIVEDEST!"
        if "!STRARCHIVEDEST!" == "USE_DB_RECOVERY_FILE_DEST" (
            set /a IS_RECOVERY_FILE_DEST=1
            echo set linesize 300; > %GETFLASHINFOFILE%
            echo col NAME for a255; >> %GETFLASHINFOFILE%
            echo select NAME from V$RECOVERY_FILE_DEST; >> %GETFLASHINFOFILE%
            echo exit >> %GETFLASHINFOFILE%
            
            call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GETFLASHINFOFILE% %GETFLASHINFORST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
            if !RetCode! NEQ 0 (
                call :Log "Excute sql script for get archive dest failed."
                call :DeleteFile %GETFLASHINFOFILE%
                call :DeleteFile %GETFLASHINFORST%
                call :DeleteFile %ARCHIVE_DEST_LIST%
                exit !RetCode!
            )
            set /a NUM=0
            set /a FLG_FIND=0
            for /f "skip=12" %%i in ('type %GETFLASHINFORST%') do (
                if "!FLG_FIND!" EQU "1" (
                    if !NUM! equ 0 (
                       set STRARCHIVEDEST=%%i
                       call :Log "STRARCHIVEDEST=%%i"
                    )
                    set /a NUM=!NUM! + 1
                )
                
                echo %%i | findstr \-\-\-\-\-\-
                if "!errorlevel!" EQU "0" (
                    set /a FLG_FIND=1
                )
            )

            echo set linesize 100; > %GETFLASHINFOFILE%
            echo select SPACE_LIMIT/1024/1024,SPACE_USED/1024/1024 from V$RECOVERY_FILE_DEST; >> %GETFLASHINFOFILE%
            echo exit >> %GETFLASHINFOFILE%
            
            call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GETFLASHINFOFILE% %GETFLASHINFORST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
            if !RetCode! NEQ 0 (
                call :Log "excute sql script for get archive dest limit and used failed."
                call :DeleteFile %GETFLASHINFOFILE%
                call :DeleteFile %GETFLASHINFORST%
                call :DeleteFile %ARCHIVE_DEST_LIST%
                exit !RetCode!
            )
            
            set /a NUM=0
            set /a FLG_FIND=0
            for /f "skip=12 tokens=1-3 delims=	 " %%i in ('type %GETFLASHINFORST%') do (                
                if "!FLG_FIND!" EQU "1" (
                    if !NUM! equ 0 (
                        set /a FAST_ALLCAPACITY=%%i
                        set /a FAST_USEDCAPACITY=%%j
                        call :Log "Archive FAST_USEDCAPACITY=!STRARCHIVEDEST!;FAST_ALLCAPACITY=!FAST_ALLCAPACITY!"
                        call :Log "UsedInfo=%%i %%j"
                    )
                    set /a NUM=!NUM! + 1
                )
                
                echo %%i | findstr \-\-\-\-\-\-
                if "!errorlevel!" EQU "0" (
                    set /a FLG_FIND=1
                )
            )
        )

        call :Log "Archive Dest=!STRARCHIVEDEST!"
        call :DeleteFile %GETFLASHINFOFILE%
        call :DeleteFile %GETFLASHINFORST%
        
        rem ASM diskgroup is flash recover area
        if "!STRARCHIVEDEST:~0,1!" == "+" (
            set AGNAME=!STRARCHIVEDEST:~1!
            for /f "delims=/ tokens=1" %%a in ("!AGNAME!") do (
                set AGNAME=%%a
            )
            call :Log "diskgroup name is !AGNAME!."
            
            rem try to get ASM instance name
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
            call :Log "GRIDHOMEPATH=!GRIDHOMEPATH!"
            
            set ORACLE_SID=!ASMSIDNAME!
            set ORACLE_HOME=!GRIDHOMEPATH!
            call asmcmd "lsdg" > %GETDISKUSEDING%
            for /f "skip=1 tokens=7-13" %%a in ('type %GETDISKUSEDING%') do (
                if "!AGNAME!/" == "%%g" (
                    set /a ALLCAPACITY=%%a
                    set /a FREECAPACTIY=%%b
                )
            )
        ) else (  rem filesystem is flash recover area
            set STRARCHIVEDEST=!STRARCHIVEDEST:~0,2!
            call fsutil volume diskfree !STRARCHIVEDEST! > %GETDISKUSEDING%
            set /a NUM=0
            for /f "tokens=2 delims=:" %%i in ('type %GETDISKUSEDING%') do (
                if !NUM! equ 0 (
                    set FREECAPACTIY=%%i
                    rem not support bigger than 32bit int number
                    set FREECAPACTIY=!FREECAPACTIY:~0,-6!
                    if !FREECAPACTIY! == "" (
                        set /a FREECAPACTIY=0
                    ) else (
                        set /a FREECAPACTIY=!FREECAPACTIY!
                    )
                )
                if !NUM! equ 1 (
                    set ALLCAPACITY=%%i
                    set ALLCAPACITY=!ALLCAPACITY:~0,-6!
                    set /a ALLCAPACITY=!ALLCAPACITY!
                )
                set /a NUM=!NUM! + 1
            )
        )

        call :DeleteFile %GETDISKUSEDING%
        call :Log "ALLCAPACITY=!ALLCAPACITY!;FREECAPACTIY=!FREECAPACTIY!"
        
        if !FREECAPACTIY! LSS 0 (
            call :Log "FREECAPACTIY=!FREECAPACTIY! is not invalid, not check."
            set /a %~1=0
            call :DeleteFile %ARCHIVE_DEST_LIST%
            goto :eof
        )
        
        if !IS_RECOVERY_FILE_DEST! equ 1 (
            call :Log "FAST_ALLCAPACITY=!FAST_ALLCAPACITY!;FAST_USEDCAPACITY=!FAST_USEDCAPACITY!"
            set /a FAST_FREECAPACTIY=!FAST_ALLCAPACITY!-!FAST_USEDCAPACITY!
            
            if !FAST_FREECAPACTIY! GEQ 0 (
                if !FAST_FREECAPACTIY! LSS !FREECAPACTIY! (
                    set /a FREECAPACTIY=!FAST_FREECAPACTIY!
                    call :Log "flash recover area is letter than disk using, use flash recover area-!FREECAPACTIY!."
                )
            )
        )
        if !FREECAPACTIY! LSS !DBARCHIVE_LIMEN! (
            call :Log "Free capacity !FREECAPACTIY!MB is less than limen !DBARCHIVE_LIMEN!MB"
            call :DeleteFile %ARCHIVE_DEST_LIST%
            exit %ERROR_ORACLE_OVER_ARCHIVE_USING%
        ) else (
            call :Log "Free capacity !FREECAPACTIY!MB is bigger than limen !DBARCHIVE_LIMEN!MB"
        )
    )
    call :Log "End check archive dest using directory."
    set /a %~1=0
    call :DeleteFile %ARCHIVE_DEST_LIST%
    rem *************************end 2014-01-16 modify create archive directory******************
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

endlocal
