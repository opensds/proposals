@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


rem @dest:   application agent for oracle10g
rem @date:   2009-05-19
rem @authr:  
rem @modify: 2010-02-23

setlocal EnableDelayedExpansion

set /a ERROR_DB_FREEZE_YES=0
set /a ERROR_DB_FREEZE_NO=1
set /a ERROR_DB_FREEZE_UNKNOWN=2
set /a ERROR_SCRIPT_EXEC_FAILED=5

set /a ERROR_RECOVER_INSTANCE_NOSTART=11
set /a ERROR_DBUSERPWD_WRONG=10
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
set /a ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED=35
set /a ERROR_ORACLE_END_HOT_BACKUP_FAILED=36
set /a ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT=37
set /a ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP=38
set /a ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED=42
set /a ERROR_ORACLE_TNS_PROTOCOL_ADAPTER=43
set /a ERROR_ORACLE_NOT_INSTALLED=44

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
set CMD_EXECRMANSQL=execrmansql
set CMD_CHECKORACLE=checkoracleinstall

set SQLTBLNAME=tablespace_name
set SQLSTATUS=status

set DB_LOGIN=
set ASM_LOGIN=

set QRYNOBACKUPSQL=select status from v$backup where status = 'NOT ACTIVE';
set QRYBACKUPSQL=select status from v$backup where status = 'ACTIVE';

set FREEZEFILEPRE=FreezeTblSpc
set FREEZESCRIPT="%AGENT_TMP_PATH%%FREEZEFILEPRE%%PID%.sql"
set FREEZESCRIPTRST="%AGENT_TMP_PATH%%FREEZEFILEPRE%RST%PID%.txt"

set THAWFILEPRE=ThawTbleSpc
set THAWSCRIPT="%AGENT_TMP_PATH%%THAWFILEPRE%%PID%.sql"
set THAWSCRIPTRST="%AGENT_TMP_PATH%%THAWFILEPRE%RST%PID%.txt"

set BKFILEPRE=BackupFile
set BKFILENAME="%AGENT_TMP_PATH%%BKFILEPRE%%PID%.txt"

set LOGFILE=oracleconsistent.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

set GETBACKUPTBCOUNT="%AGENT_TMP_PATH%GetBackupTableSpace%PID%.sql"
set GETBACKUPTBCOUNTRST="%AGENT_TMP_PATH%GetBackupTableSpaceRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0
set GETSECONDS="%AGENT_TMP_PATH%GetSeconds%PID%.txt"

set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"

set TRUNCATE_LOG_PRE=TruncateLog
set TRUNCATE_LOG_SCRIPT=%TRUNCATE_LOG_PRE%%PID%.rcv
set TRUNCATE_LOG_RST=%TRUNCATE_LOG_PRE%RST%PID%.txt
set TRUNCATE_LOG_SCRIPT_FULL="%AGENT_TMP_PATH%%TRUNCATE_LOG_SCRIPT%"
set TRUNCATE_LOG_RST_FULL="%AGENT_TMP_PATH%%TRUNCATE_LOG_RST%"

rem check archivelog mdoe
set GET_ARCHIVE_LOG_MODE_SQL="%AGENT_TMP_PATH%get_archive_log_mode%PID%.sql"
set GET_ARCHIVE_LOG_MODE_RST="%AGENT_TMP_PATH%get_archive_log_rst%PID%.txt"

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
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "TruncateLogTime" TRUNCATE_LOG_TIME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "FrushType" EXEFLG

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)

call :SetDBAuth
call :SetRMANAuth

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
            more %CURRENTCMDLineRST% | findstr /c:"oracleconsistent.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oracleconsistent.bat %AGENT_ROOT% %PID%"
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

echo DBNAME=%DBNAME%;DBUSER=%DBUSER%;EXEFLG=%EXEFLG%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%
call :Log "DBNAME=%DBNAME%;DBUSER=%DBUSER%;EXEFLG=%EXEFLG%;DBINSTANCE=%DBINSTANCE%;CURRENTPID=%CURRENTPID%;AUTHMODE=!AUTHMODE!;TRUNCATE_LOG_TIME=%TRUNCATE_LOG_TIME%;"

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

if exist %ENVFILERORACLETMP% (del /f /q %ENVFILERORACLETMP%) 

if not defined EXEFLG (
    echo The input parameter is not right!
    call :Log "The input parameter is not right!"
    echo %USAGE%
    exit %ERROR_SCRIPT_EXEC_FAILED%
)

if %EXEFLG%==0 (
    call :FreezeDB RetCode
    if !RetCode! EQU 0 (
        echo Freeze %DBNAME% tablespace successful.
        call :Log "Freeze %DBNAME% tablespace successful."
    ) else (
        call :DeleteFile %FREEZESCRIPT%
        call :DeleteFile %FREEZESCRIPTRST%
        call :DeleteFile %THAWSCRIPT%
        call :DeleteFile %THAWSCRIPTRST%
        call :DeleteFile %TMPBKSCRIPT%
        call :DeleteFile %BKFILENAME%
        exit %ERROR_SCRIPT_EXEC_FAILED%
    )
)

if %EXEFLG%==1 (
    call :ThawDB RetCode
    if !RetCode! EQU 0 (
        echo Thaw %DBNAME% tablespace successful.
        call :Log "Thaw %DBNAME% tablespace successful."
    ) else (
        call :DeleteFile %FREEZESCRIPT%
        call :DeleteFile %FREEZESCRIPTRST%
        call :DeleteFile %THAWSCRIPT%
        call :DeleteFile %THAWSCRIPTRST%
        call :DeleteFile %TMPBKSCRIPT%
        call :DeleteFile %BKFILENAME%
        exit !RetCode!
    )
)

if %EXEFLG%==2 (
    call :GetBackupModeTBCount BACKUPCOUNT RetCode
    call :Log "BACKUPCOUNT=!BACKUPCOUNT!"
    if !RetCode! EQU 0 (
        if "!BACKUPCOUNT!" EQU "0" (
            call :Log "There are no backup tablespace."
            echo %ERROR_DB_FREEZE_NO% >> %RSTFILE%
        ) else (
            call :Log "Database is in hot backup mode."
            echo %ERROR_DB_FREEZE_YES% >> %RSTFILE%
        )
    ) else (
        echo %ERROR_DB_FREEZE_UNKNOWN% >> %RSTFILE%
    )
)

if %EXEFLG%==3 (
    call :ArchiveDB RetCode
)

if %EXEFLG%==4 (
    call :TruncateArchiveLog RetCode
    exit !RetCode!
)

if %EXEFLG%==5 (
    call :GetArchiveLogMode ArchiveLogMode RetCode
    if !RetCode! EQU 0 (
        call :Log "Get archive log mode !ArchiveLogMode!."
        echo !ArchiveLogMode! >> %RSTFILE%
    ) else (
        call :Log "Get archive log mode failed, exit !RetCode!."
        exit !RetCode!
    )
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

rem Create temporary sql script function, eg: CrtTmpSql ResultFile SqlScrpitFile Sql Section
:CrtTmpSql
    echo set serveroutput on > "%~2"
    echo set echo off >> "%~2"
    echo set feedback off >> "%~2"
    echo set heading off >> "%~2"
    echo set verify off >> "%~2"
    echo set trimspool off >> "%~2"
    echo set trims on >> "%~2"
    echo spool '%~1' >> "%~2"
    echo declare cursor cur_tblspace is %~3 >> "%~2"
    echo begin >> "%~2"
    echo    for ct in cur_tblspace loop >> "%~2"
    echo    dbms_output.put_line('' ^|^| ct.%~4 ^|^| ''); >> "%~2"
    echo    end loop; >> "%~2"
    echo end; >> "%~2"
    echo / >> "%~2"
    echo spool off; >> "%~2"
    echo exit >> "%~2"
    goto :EOF

rem Freeze all database tablespace, if failed, then roolback.
:FreezeDB
    call :Log "Begin to freeze db..."
    set /a beginTime=0
    "%AGENT_BIN_PATH%agentcli" genseconds > %GETSECONDS%
    for /f "delims=" %%a in ('type %GETSECONDS%') do (
        if not "%%a" == "" (
            set /a beginTime=%%a
        )
    )
    call :DeleteFile %GETSECONDS% 
    echo alter database begin backup; > %FREEZESCRIPT%
    echo exit >> %FREEZESCRIPT%

    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %FREEZESCRIPT% %FREEZESCRIPTRST% %DBINSTANCE% "!DB_LOGIN!" 300 RetCode
    call :DeleteFile %FREEZESCRIPT%
    call :DeleteFile %FREEZESCRIPTRST%
    if !RetCode! EQU 0 (
        call :GetNotBackupModeTBCount NOBACKUPCOUNT RetCode
        call :Log "NOBACKUPCOUNT=!NOBACKUPCOUNT!"
        if !RetCode! EQU 0 (
            if "!NOBACKUPCOUNT!" EQU "0" (
                call :Log "Freeze %DBINSTANCE% successful."
            ) else (
                call :Log "Freeze %DBINSTANCE% failed, so start to rollback."
                
                call :ThawDB RetCode
                if !RetCode! EQU 0 (
                    echo "Freeze %DBINSTANCE% failed, and rollback failed."
                    call :Log "Freeze %DBINSTANCE% failed, and rollback succ."
                ) else (
                    echo "Freeze %DBINSTANCE% failed, and rollback failed."
                    call :Log "Freeze %DBINSTANCE% failed, and rollback failed."
                )
                exit %ERROR_SCRIPT_EXEC_FAILED%
            )
        ) else (
            call :Log "Get %DBINSTANCE% not backup mode tablespace failed."
            exit %ERROR_SCRIPT_EXEC_FAILED%
        )
    ) else (
        set /a endTime=0
        "%AGENT_BIN_PATH%agentcli" genseconds > %GETSECONDS%
        for /f "delims=" %%a in ('type %GETSECONDS%') do (
            if not "%%a" == "" (
                set /a endTime=%%a
            )
        )
        call :DeleteFile %GETSECONDS%

        if "%ERROR_DB_ALREADY_INBACKUP%" == "!RetCode!" (
            exit %ERROR_DB_ALREADY_INBACKUP%
        ) else (
            set /a FREEZECODE=!RetCode!
            echo Freeze %DBINSTANCE% tablespace failed at exec script failed, so start to rollback.
            call :Log "Freeze %DBINSTANCE% tablespace failed at exec script failed, so start to rollback."
            call :ThawDB RetCode
            if !RetCode! EQU 0 (
                echo Thaw %DBINSTANCE% tablespace successful.
                call :Log "Thaw %DBINSTANCE% tablespace successful."
            ) else (
                echo Thaw %DBINSTANCE%tablespace failed.
                call :Log "Thaw %DBINSTANCE%% tablespace failed."
            )

            set /a execTime=!endTime! - !beginTime!
            if !execTime! GEQ 300 (
                call :Log "Exec freeze DB timetout."
                exit %ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT%
            ) else (
                exit !FREEZECODE!
            )
            set %~1=1
         )
    )
    call :Log "End to freeze db..."
    goto :EOF


:ThawDB
    call :Log "Begin to thaw db..."
    echo alter database end backup; > %THAWSCRIPT%
    echo exit >> %THAWSCRIPT%

    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %THAWSCRIPT% %THAWSCRIPTRST% %DBINSTANCE% "!DB_LOGIN!" 600 RetCode
    call :DeleteFile %THAWSCRIPT%
    call :DeleteFile %THAWSCRIPTRST%
    rem end backup succ, but go on to check backup mode tablespace number.
    if "!RetCode!" EQU "%ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP%" (
        set /a RetCode=0
    )
    if "!RetCode!" EQU "0" (
        call :GetBackupModeTBCount BACKUPCOUNT RetCode
        call :Log "BACKUPCOUNT=!BACKUPCOUNT!"
        if "!RetCode!" EQU "0" (
            if "!BACKUPCOUNT!" EQU "0" (
                call :Log "Thaw %DBINSTANCE% successful."
                set %~1=0
            ) else (
                call :Log "Thaw %DBINSTANCE% succ, but these are tablespace these alread in backup mode."
                exit %ERROR_ORACLE_END_HOT_BACKUP_FAILED%
            )
        ) else (
            call :Log "Get %DBINSTANCE% backup mode tablespace failed."
            exit %ERROR_ORACLE_END_HOT_BACKUP_FAILED%
        )
    ) else (
        set %~1=!RetCode!
    )
    call :Log "End to thaw db..."
goto :EOF

:ArchiveDB
    call :Log "Begin to archive db..."
    echo ALTER SYSTEM ARCHIVE LOG CURRENT; > %THAWSCRIPT%
    echo exit >> %THAWSCRIPT%

    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %THAWSCRIPT% %THAWSCRIPTRST% %DBINSTANCE% "!DB_LOGIN!" 600 RetCode
    call :DeleteFile %THAWSCRIPT%
    call :DeleteFile %THAWSCRIPTRST%
    if "!RetCode!" EQU "0" (
        call :Log "Archive %DBINSTANCE% successful."
        set %~1=0
    ) else (
        call :Log "Archive %DBINSTANCE% failed."
        if "%ERROR_SCRIPT_EXEC_FAILED%" EQU "!RetCode!" (
            exit %ERROR_DB_ARCHIVEERROR%
        ) else (
            exit !RetCode!
        )
    )
    call :Log "End to arhive db..."
goto :EOF

:TruncateArchiveLog
    call :Log "Begin to truncate archivelog of database %DBINSTANCE%."
    echo delete noprompt force archivelog until time "to_date('%TRUNCATE_LOG_TIME%','yyyy-mm-dd-hh24-mi-ss')"; > %TRUNCATE_LOG_SCRIPT_FULL%
    echo exit >> %TRUNCATE_LOG_SCRIPT_FULL%

    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECRMANSQL% %PID% %LOGFILE% %TRUNCATE_LOG_SCRIPT% %TRUNCATE_LOG_RST% %DBINSTANCE% "!RMAN_LOGIN!" 600 Ret_Code
    call :DeleteFile %TRUNCATE_LOG_SCRIPT_FULL%
    call :DeleteFile %TRUNCATE_LOG_RST_FULL%
    
    set /a %~1=!Ret_Code!
    if "!Ret_Code!" EQU "0" (
        call :Log "Truncate archive log successfully."
    ) else (
        call :Log "Truncate archive log failed, ret !Ret_Code!."
        if "%ERROR_SCRIPT_EXEC_FAILED%" EQU "!Ret_Code!" (
            set /a %~1=%ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED%
        )
    )
    call :Log "End truncate archivelog of database %DBINSTANCE%."
goto :EOF

:GetArchiveLogMode
    set /a IS_RECOVERY_FILE_DEST=0
    
    call :Log "Begin get archive log mode."
    set TITLE_LOGMODE=Database log mode
    set TITLE_ARCHIVE_DEST=Archive destination
    set STRARCHIVEDEST=

    echo ALTER SESSION SET NLS_LANGUAGE='AMERICAN'; > %GET_ARCHIVE_LOG_MODE_SQL%
    echo select LOG_MODE from v$database; >> %GET_ARCHIVE_LOG_MODE_SQL%
    echo exit >> %GET_ARCHIVE_LOG_MODE_SQL%
    
    call :Log "Exec sql to get archive mode."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GET_ARCHIVE_LOG_MODE_SQL% %GET_ARCHIVE_LOG_MODE_RST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Get archive log list failed, ret code !RetCode!."
        call :DeleteFile %GET_ARCHIVE_LOG_MODE_SQL%
        call :DeleteFile %GET_ARCHIVE_LOG_MODE_RST%
        set %~2=!RetCode!
    ) else (
        rem LOGMODE:NOARCHIVELOG|ARCHIVELOG
        more %GET_ARCHIVE_LOG_MODE_RST% | find /I "NOARCHIVELOG" >nul
        if !errorlevel! EQU 0 (
            call :Log "Non archive log mode."
            more %GET_ARCHIVE_LOG_MODE_RST% >> %LOGFILEPATH%
            call :Log "info end."
            call :DeleteFile %GET_ARCHIVE_LOG_MODE_SQL%
            call :DeleteFile %GET_ARCHIVE_LOG_MODE_RST%
            set %~1=0
            set %~2=0
        ) else (
            call :DeleteFile %GET_ARCHIVE_LOG_MODE_SQL%
            call :DeleteFile %GET_ARCHIVE_LOG_MODE_RST%
            call :Log "Get archive log mode succ."
            set %~1=1
            set %~2=0
        )
    )
goto :eof

rem Convert str to low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    
    set %~2=!CONVERTSTR!
    
goto :EOF

rem  ************************** get database count ***********************************
:GetBackupModeTBCount
    set /a %~2 = 0
    call :CrtTmpSql %BKFILENAME% %GETBACKUPTBCOUNT% "%QRYBACKUPSQL%" "%SQLSTATUS%"
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GETBACKUPTBCOUNT% %GETBACKUPTBCOUNTRST% %DBINSTANCE% "!DB_LOGIN!" 60 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for get backup count failed."
        call :DeleteFile %GETBACKUPTBCOUNT%
        call :DeleteFile %GETBACKUPTBCOUNTRST%
        call :DeleteFile %BKFILENAME%
        set /a %~2 = 1
    ) else (
        more %GETBACKUPTBCOUNTRST% | find /I "ORA-" > nul
        if !errorlevel! EQU 0 (
            call :Log "=====================GetNotBackupModeTBCount failed.==========================="
            more %GETBACKUPTBCOUNTRST% >> %LOGFILEPATH%
            call :Log "=====================GetNotBackupModeTBCount failed.==========================="
            set /a %~2 = 1
        ) else (
            set /a BACKUPTBCOUNT=0
            for /f %%a in ('type %BKFILENAME%') do (
                set /a BACKUPTBCOUNT=!BACKUPTBCOUNT! + 1
            )
            set /a %~1 = !BACKUPTBCOUNT!
            set /a %~2 = 0
        )
        call :DeleteFile %GETBACKUPTBCOUNT%
        call :DeleteFile %GETBACKUPTBCOUNTRST%
        call :DeleteFile %BKFILENAME%
    )
goto :eof
    
rem  ************************** get database count ***********************************
:GetNotBackupModeTBCount
    set /a %~2 = 0
    call :CrtTmpSql %BKFILENAME% %GETBACKUPTBCOUNT% "%QRYNOBACKUPSQL%" "%SQLSTATUS%"
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %GETBACKUPTBCOUNT% %GETBACKUPTBCOUNTRST% %DBINSTANCE% "!DB_LOGIN!" 60 RetCode
    if !RetCode! NEQ 0 (
        call :Log "Excute sql script for get backup count failed."
        call :DeleteFile %GETBACKUPTBCOUNT%
        call :DeleteFile %GETBACKUPTBCOUNTRST%
        call :DeleteFile %BKFILENAME%
        set /a %~2 = 1
    ) else (
        more %GETBACKUPTBCOUNTRST% | find /I "ORA-" > nul
        if !errorlevel! EQU 0 (
            call :Log "=====================GetNotBackupModeTBCount failed.==========================="
            more %GETBACKUPTBCOUNTRST% >> %LOGFILEPATH%
            call :Log "=====================GetNotBackupModeTBCount failed.==========================="
            set /a %~2 = 1
        ) else (
            set /a BACKUPTBCOUNT=0
            for /f %%a in ('type %BKFILENAME%') do (
                set /a BACKUPTBCOUNT=!BACKUPTBCOUNT! + 1
            )
            set /a %~1 = !BACKUPTBCOUNT!
            set /a %~2 = 0
        )
        call :DeleteFile %GETBACKUPTBCOUNT%
        call :DeleteFile %GETBACKUPTBCOUNTRST%
        call :DeleteFile %BKFILENAME%
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

:SetRMANAuth
    if !AUTHMODE!==1 (
        set RMAN_LOGIN=/
    )
    if !AUTHMODE!==0 (
        set RMAN_LOGIN=%DBUSER%/"%DBUSERPWD%"
    )
goto :eof

endlocal
