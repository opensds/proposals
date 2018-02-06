@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

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
set /a ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED=35
set /a ERROR_ORACLE_END_HOT_BACKUP_FAILED=36
set /a ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT=37

set /a ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP=38
set /a ERROR_ASM_NO_STARTUP_TNS=39

set /a ERROR_ORACLE_NOT_MOUNTED=40
set /a ERROR_ORACLE_NOT_OPEN=41
set /a ERROR_ORACLE_TRUNCATE_ARCHIVELOG_FAILED=42
set /a ERROR_ORACLE_TNS_PROTOCOL_ADAPTER=43
set /a ERROR_ORACLE_NOT_INSTALLED=44
set /a ERROR_ORACLE_ANOTHER_STARTING=45
set /a ERROR_ORACLE_DB_BUSY=46

rem no support EnableDelayedExpansion, can't get value of variable if open
rem setlocal EnableDelayedExpansion
set AGENT_ROOT=%~1
set CMDNAME=%~2
set PID=%~3
set LOGFILE=%~4
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\

set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set PROCMONITOR_EXIT="%AGENT_TMP_PATH%pmexit!CURRENTPID!.flg"

rem command name list
set CMD_GETVERSION=getversion
set CMD_EXECSQL=execsql
set CMD_EXECRMANSQL=execrmansql
set CMD_EXECASMSQL=execasmsql
set CMD_GETVALUE=getvalue
set CMD_GETORAPATH=getoraclepath
set CMD_GETGRIDPATH=getgridpath
set CMD_CHECKORACLE=checkoracleinstall

set nls_lang=AMERICAN_AMERICA.ZHS16GBK

rem get oracle version
if "!CMDNAME!" == "%CMD_GETVERSION%" (
    set SQLPLUSEXITFILE="%AGENT_TMP_PATH%SqlPlusExit%PID%.sql"
    set SQLPLUSRST="%AGENT_TMP_PATH%SqlPlusRsl%PID%.txt"
    set SQLPLUSTMP="%AGENT_TMP_PATH%SqlPlusRslTmp%PID%.txt"

    echo exit > !SQLPLUSEXITFILE!
    call :ExeSql !SQLPLUSEXITFILE! !SQLPLUSRST! RetCode
    if !RetCode! NEQ 0 (
        set %~5=--
        call :DeleteFile !SQLPLUSRST!
        call :DeleteFile !SQLPLUSEXITFILE!
        call :Log "Connect Oracle database failed."
    ) else (
        call :Log "Connect Oracle database successful."
        type !SQLPLUSRST! | find /v "(" > !SQLPLUSTMP!
        if !errorlevel! NEQ 0 (
            set %~5=--
            type !SQLPLUSRST! >> !LOGFILEPATH!
            call :Log "Get Oracle database version failed."  
        ) else (
            for /f "skip=1 tokens=1 delims=-" %%i in ('type !SQLPLUSTMP!') do (
                for /f "tokens=2 delims=:" %%j in ("%%i") do (
                    for /f "tokens=2" %%k in ("%%j") do (
                        set %~5=%%k
                        call :Log "Get Oracle database version-%%k successful."
                    )
                )
            ) 
        )
        call :DeleteFile !SQLPLUSRST!
        call :DeleteFile !SQLPLUSTMP!
        call :DeleteFile !SQLPLUSEXITFILE!
    )
)

rem get oracle base path and home path
if "!CMDNAME!" == "%CMD_GETORAPATH%" (
    set ORACLEBASEPATH=
    set ORACLEHOMEPATH=
    for /f "tokens=1,2,* " %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\ORACLE" /f "KEY_ORADB*" /k') do (
        for /f "tokens=1,2,* " %%i in ('reg query "%%a" /s /v "ORACLE_BASE"') do (
            if "%%i" == "ORACLE_BASE" (
                set "ORACLEBASEPATH=%%k"
            )
        )
        
        for /f "tokens=1,2,* " %%i in ('reg query "%%a" /s /v "ORACLE_HOME"') do (
            if "%%i" == "ORACLE_HOME" (
                set "ORACLEHOMEPATH=%%k"
            )
        )
    )

    if "!ORACLEBASEPATH!" == "" (
        for /f "tokens=1,2,* " %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\ORACLE" /f "KEY_ORAHOME*" /k') do (
            for /f "tokens=1,2,* " %%i in ('reg query "%%a" /s /v "ORACLE_BASE"') do (
                if "%%i" == "ORACLE_BASE" (
                    set "ORACLEBASEPATH=%%k"
                )
            )
            
            for /f "tokens=1,2,* " %%i in ('reg query "%%a" /s /v "ORACLE_HOME"') do (
                if "%%i" == "ORACLE_HOME" (
                    set "ORACLEHOMEPATH=%%k"
                )
            )
        )
    )
    set %~5=!ORACLEBASEPATH!
    set %~6=!ORACLEHOMEPATH!
)

rem get grid home path
if "!CMDNAME!" == "%CMD_GETGRIDPATH%" (
    set DBVERSION=%~5
    set GRIDHOMEPATH=
    if "!DBVERSION:~0,4!" == "11.2" (
        for /f "tokens=1,2,* " %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\ORACLE" /f "KEY_ORACRS*" /k') do (
            for /f "tokens=1,2,* " %%i in ('reg query "%%a" /s /v "ORACLE_HOME"') do (
                if "%%i" == "ORACLE_HOME" (
                    set "GRIDHOMEPATH=%%k"
                )
            )
        )
    ) 

    if "!DBVERSION:~0,2!" GEQ "12" (
        for /f "tokens=1,2,* " %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\ORACLE" /f "KEY_ORAGI*" /k') do (
            for /f "tokens=1,2,* " %%i in ('reg query "%%a" /s /v "ORACLE_HOME"') do (
                if "%%i" == "ORACLE_HOME" (
                    set "GRIDHOMEPATH=%%k"
                )
            )
        )
    )
    set %~6=!GRIDHOMEPATH!
)

if "!CMDNAME!" == "%CMD_GETVALUE%" (
    set ARGFILENAME="%AGENT_TMP_PATH%ArgFile%PID%"
    set ArgValue=
    set INPUTINFO=%~5
    set KEYNAME=%~6
    call :GetValue "!INPUTINFO!" !KEYNAME!
    set %~7=!ArgValue!
)

rem !!!!!!!!!!!!!!!========important========!!!!!!!!!!!!!!!
rem shift parameter when the parameter bigger than 10
rem  CMD_EXECSQL=execsql
rem  CMD_EXECRMANSQL=execrmansql
rem  CMD_EXECASMSQL=execasmsql
rem over 3 function must in the last, or modify paramter order
rem and the "shift /0" can't put into the "if" scope, if do, "shift /0" have no any action
rem !!!!!!!!!!!!!!!========important========!!!!!!!!!!!!!!!
shift /0

if "!CMDNAME!" == "%CMD_EXECSQL%" (
    set SQLFILE="%~4"
    set SQLRST="%~5"
    set INSTANCENAME=%~6
    set DB_AUTH=%~7
    set TIMEOUT=%~8

    call :SQLPlusTimeOut !TIMEOUT!
    set ORACLE_SID=!INSTANCENAME!
    sqlplus -L !DB_AUTH! > !SQLRST! @!SQLFILE!
    rem not check errorlevel, because when user password error, the errorlevel is 1,but need to check result
    rem now check result file exist
    if not exist !SQLRST! (
        call :KillProcMonitor 
        set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        call :Log "Exec SQL failed."
    ) else (
        call :KillProcMonitor 
        set /a %~9 = 0
        
        rem check result file
        rem ORA-12560: TNS protocol adapter error
        more !SQLRST! | findstr /i "ORA-12560" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: TNS protocol adapter error."
            set /a %~9=%ERROR_ORACLE_TNS_PROTOCOL_ADAPTER%
            goto :EOF
        )
        
        rem ORA-01034: ORACLE not available
        more !SQLRST! | findstr /i "ORA-01034" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: ORACLE not available."
            set /a %~9=%ERROR_RECOVER_INSTANCE_NOSTART%
            goto :EOF
        )
        
        rem ORA-00020: maximum number of processes (150) exceeded
        more !SQLRST! | findstr /i "ORA-00020" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! can not connect, maximum number of processes exceeded."
            set /a %~9=%ERROR_APPLICATION_OVER_MAX_LINK%
            goto :EOF
        )

        rem ORA-01017: invalid username/password;logon denied
        more !SQLRST! | findstr /i "ORA-01017" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: invalid username/password; logon denied."
            set /a %~9=%ERROR_DBUSERPWD_WRONG%
            goto :EOF
        )
        
        rem ORA-01031: insufficient privileges
        more !SQLRST! | findstr /i "ORA-01031" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: insufficient privileges."
            set /a %~9=%ERROR_INSUFFICIENT_WRONG%
            goto :EOF
        )
        
        rem ORA-01123: cannot start online backup; media recovery not enabled 
        more !SQLRST! | findstr /i "ORA-01123" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: cannot start online backup; media recovery not enabled."
            set /a %~9=%ERROR_ORACLE_NOARCHIVE_MODE%
            goto :EOF
        )
        
        rem ORA-01146: cannot start online backup - file 1 is already in backup
        more !SQLRST! | findstr /i "ORA-01146" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: cannot start online backup - file 1 is already in backup."
            set /a %~9=%ERROR_DB_ALREADY_INBACKUP%
            goto :EOF
        )
        
        rem ORA-01142: cannot end online backup - none of the files are in backup
        more !SQLRST! | findstr /i "ORA-01142" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: cannot end online backup - none of the files are in backup."
            set /a %~9=%ERROR_DB_INHOT_BACKUP%
            goto :EOF
        )
        
        rem ORA-01081:cannot start already-running
        more !SQLRST! | findstr /i "ORA-01081" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: cannot start already-running."
            set /a %~9=%ERROR_DB_ALREADYRUNNING%
            goto :EOF
        )
        
        rem ora-01100 database already mounted
        more !SQLRST! | findstr /i "ORA-01100" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: database already mounted."
            set /a %~9=%ERROR_DB_ALREADYMOUNT%
            goto :EOF
        )
        
        rem ORA-01531: a database already open by the instance
        more !SQLRST! | findstr /i "ORA-01531" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: a database already open by the instance."
            set /a %~9=%ERROR_DB_ALREADYOPEN%
            goto :EOF
        )
        
        rem ORA-01260:warning: END BACKUP succeeded but some files found not to be in backup mode
        more !SQLRST! | findstr /i "ORA-01260" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: warning: END BACKUP succeeded but some files found not to be in backup mode."
            set /a %~9=%ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP%
            goto :EOF
        )
        
        rem ORA-01507: database not mounted
        more !SQLRST! | findstr /i "ORA-01507" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: database not mounted."
            set /a %~9=%ERROR_ORACLE_NOT_MOUNTED%
            goto :EOF
        )
        
        rem ORA-01109: database not open
        more !SQLRST! | findstr /i "ORA-01109" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: database not open."
            set /a %~9=%ERROR_ORACLE_NOT_OPEN%
            goto :EOF
        )
        
        rem ORA-10997: another startup/shutdown operation of this instance inprogress
        more !SQLRST! | findstr /i "ORA-10997" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: another startup/shutdown operation of this instance inprogress."
            set /a %~9=%ERROR_ORACLE_ANOTHER_STARTING%
            goto :EOF
        )
        
        rem ORA-01154: database busy. Open, close, mount, and dismount not allowed now
        more !SQLRST! | findstr /i "ORA-01154" >nul
        if !errorlevel! EQU 0 (
            call :Log "Database-!INSTANCENAME! execsql failed: database busy. Open, close, mount, and dismount not allowed now."
            set /a %~9=%ERROR_ORACLE_DB_BUSY%
            goto :EOF
        )
        
        more !SQLRST! | findstr "ERROR" >nul
        if !errorlevel! EQU 0 (
            call :Log "=====================Database-!INSTANCENAME! execsql failed====================="
            type !SQLRST! >> !LOGFILEPATH!
            call :Log "=====================Database-!INSTANCENAME! execsql failed====================="
            set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        )
        
        more !SQLRST! | findstr "ORA-" >nul
        if !errorlevel! EQU 0 (
            call :Log "=====================Database-!INSTANCENAME! execsql failed====================="
            type !SQLRST! >> !LOGFILEPATH!
            call :Log "=====================Database-!INSTANCENAME! execsql failed====================="
            set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        )
    )

    goto :EOF
)

if "!CMDNAME!" == "%CMD_EXECRMANSQL%" (
    set RMANFILE="%~4"
    set RMANRST="%~5"
    set INSTANCENAME=%~6
    set RMAN_AUTH=%~7
    set TIMEOUT=%~8
    
    call :RmanTimeOut !TIMEOUT!
    cd /d "!AGENT_TMP_PATH!"
    set ORACLE_SID=!INSTANCENAME!
    rman target !RMAN_AUTH! cmdfile !RMANFILE! log !RMANRST!
    set /a RMAN_RET=!errorlevel!

    call :KillProcMonitor 
    
    rem check result file
    rem ORA-12560: TNS protocol adapter error
    more !RMANRST! | findstr /i "ORA-12560" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script failed: TNS protocol adapter error."
        set /a %~9=%ERROR_ORACLE_TNS_PROTOCOL_ADAPTER%
        goto :EOF
    )
    
    rem ORA-01034: ORACLE not available
    more !RMANRST! | findstr /i "ORA-01034" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script failed: ORACLE not available."
        set /a %~9=%ERROR_RECOVER_INSTANCE_NOSTART%
        goto :EOF
    )
    
    rem ORA-00020: maximum number of processes (150) exceeded
    more !RMANRST! | findstr /i "ORA-00020" >nul
    if !errorlevel! EQU 0 (
        call :Log "Database-!INSTANCENAME! can not connect, maximum number of processes exceeded."
        set /a %~9=%ERROR_APPLICATION_OVER_MAX_LINK%
        goto :EOF
    )

    rem ORA-01017: invalid username/password;logon denied
    more !RMANRST! | findstr /i "ORA-01017" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: invalid username/password; logon denied."
        set /a %~9=%ERROR_DBUSERPWD_WRONG%
        goto :EOF
    )
    
    rem ORA-01031: insufficient privileges
    more !RMANRST! | findstr /i "ORA-01031" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: insufficient privileges."
        set /a %~9=%ERROR_INSUFFICIENT_WRONG%
        goto :EOF
    )
    
    rem ORA-01123: cannot start online backup; media recovery not enabled 
    more !RMANRST! | findstr /i "ORA-01123" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: cannot start online backup; media recovery not enabled."
        set /a %~9=%ERROR_ORACLE_NOARCHIVE_MODE%
        goto :EOF
    )
    
    rem ORA-01146: cannot start online backup - file 1 is already in backup
    more !RMANRST! | findstr /i "ORA-01146" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: cannot start online backup - file 1 is already in backup."
        set /a %~9=%ERROR_DB_ALREADY_INBACKUP%
        goto :EOF
    )
    
    rem ORA-01142: cannot end online backup - none of the files are in backup
    more !RMANRST! | findstr /i "ORA-01142" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: cannot end online backup - none of the files are in backup."
        set /a %~9=%ERROR_DB_INHOT_BACKUP%
        goto :EOF
    )
    
    rem ORA-01081:cannot start already-running
    more !RMANRST! | findstr /i "ORA-01081" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: cannot start already-running."
        set /a %~9=%ERROR_DB_ALREADYRUNNING%
        goto :EOF
    )
    
    rem ora-01100 database already mounted
    more !RMANRST! | findstr /i "ORA-01100" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: database already mounted."
        set /a %~9=%ERROR_DB_ALREADYMOUNT%
        goto :EOF
    )
    
    rem ORA-01531: a database already open by the instance
    more !RMANRST! | findstr /i "ORA-01531" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: a database already open by the instance."
        set /a %~9=%ERROR_DB_ALREADYOPEN%
        goto :EOF
    )
    
    rem ORA-01260:warning: END BACKUP succeeded but some files found not to be in backup mode
    more !RMANRST! | findstr /i "ORA-01260" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: warning: END BACKUP succeeded but some files found not to be in backup mode."
        set /a %~9=%ERROR_DB_ENDSUCC_SOMETBNOT_INBACKUP%
        goto :EOF
    )
    
    rem ORA-01507: database not mounted
    more !RMANRST! | findstr /i "ORA-01507" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: database not mounted."
        set /a %~9=%ERROR_ORACLE_NOT_MOUNTED%
        goto :EOF
    )
    
    rem ORA-01109: database not open
    more !RMANRST! | findstr /i "ORA-01109" >nul
    if !errorlevel! EQU 0 (
        call :Log "RMAN(!INSTANCENAME!) execute script: database not open."
        set /a %~9=%ERROR_ORACLE_NOT_OPEN%
        goto :EOF
    )
    
    more !RMANRST! | findstr "ERROR" >nul
    if !errorlevel! EQU 0 (
        call :Log "=====================RMAN(!INSTANCENAME!) execute script failed====================="
        type !RMANRST! >> !LOGFILEPATH!
        call :Log "=====================RMAN(!INSTANCENAME!) execute script failed====================="
        set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        goto :EOF
    )
    
    more !RMANRST! | findstr "ORA-" >nul
    if !errorlevel! EQU 0 (
        call :Log "=====================RMAN(!INSTANCENAME!) execute script failed====================="
        type !RMANRST! >> !LOGFILEPATH!
        call :Log "=====================RMAN(!INSTANCENAME!) execute script failed====================="
        set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        goto :EOF
    )
    
    more !RMANRST! | findstr "RMAN-00569" >nul
    if !errorlevel! EQU 0 (
        call :Log "=====================RMAN(!INSTANCENAME!) execute script failed====================="
        type !RMANRST! >> !LOGFILEPATH!
        call :Log "=====================RMAN(!INSTANCENAME!) execute script failed====================="
        set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        goto :EOF
    )
    
    if !RMAN_RET! NEQ 0 (       
        set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        call :Log "Exec RMAN script failed."
        type !RMANRST! >> !LOGFILEPATH!
        goto :EOF
    ) 

    set /a %~9=0
    goto :EOF
)

if "!CMDNAME!" == "%CMD_EXECASMSQL%" (
    set SQLFILE="%~4"
    set SQLRST="%~5"
    set INSTANCENAME=%~6
    set ASM_AUTH=%~7
    set TIMEOUT=%~8
    
    call :SQLPlusTimeOut !TIMEOUT!
    set ORACLE_SID=!INSTANCENAME!
    sqlplus -L !ASM_AUTH! > !SQLRST! @!SQLFILE!
    
    rem not check errorlevel, because when user password error, the errorlevel is 1,but need to check result
    rem now check result file exist
    if not exist !SQLRST! (
        call :KillProcMonitor 
        set /a %~9 = %ERROR_SCRIPT_EXEC_FAILED%
        call :Log "Exec ASM SQL failed."
    ) else (
        call :KillProcMonitor 
        set /a %~9 = 0
        
        rem check result file
        rem ORA-01034: ORACLE not available
        more !SQLRST! | findstr /i "ORA-01034" >nul
        if !errorlevel! EQU 0 (
            call :Log "ASM-!INSTANCENAME! execsql failed: ORACLE not available."
            set /a %~9=%ERROR_RECOVER_INSTANCE_NOSTART%
            goto :EOF
        )
        
        rem ORA-00020: maximum number of processes (150) exceeded
        more !SQLRST! | findstr /i "ORA-00020" >nul
        if !errorlevel! EQU 0 (
            call :Log "ASM-!INSTANCENAME! can not connect, maximum number of processes exceeded."
            set /a %~9=%ERROR_APPLICATION_OVER_MAX_LINK%
            goto :EOF
        )
        
        rem ORA-01017: invalid username/password;logon denied
        more !SQLRST! | findstr /i "ORA-01017" >nul
        if !errorlevel! EQU 0 (
            call :Log "ASM-!INSTANCENAME! execsql failed: invalid username/password; logon denied."
            set /a %~9=%ERROR_ASM_DBUSERPWD_WRONG%
            goto :EOF
        )
        
        rem ORA-01031: insufficient privileges
        more !SQLRST! | findstr /i "ORA-01031" >nul
        if !errorlevel! EQU 0 (
            call :Log "ASM-!INSTANCENAME! execsql failed: insufficient privileges."
            set /a %~9=%ERROR_ASM_INSUFFICIENT_WRONG%
            goto :EOF
        )
        
        rem already mount
        more !SQLRST! | findstr /i "ORA-15013" >nul
        if !errorlevel! EQU 0 (
            call :Log "ASM-!INSTANCENAME! execsql failed: already mount."
            set /a %~9=%ERROR_ASM_DISKGROUP_ALREADYMOUNT%
            goto :EOF
        )
        
        rem ORA-15001: diskgroup "PLAG001" does not exist or is not mounted
        more !SQLRST! | findstr /i "ORA-15001" >nul
        if !errorlevel! EQU 0 (
            call :Log "ASM-!INSTANCENAME! execsql failed: diskgroup not mount or not exist."
            set /a %~9=%ERROR_ASM_DISKGROUP_NOTMOUNT%
            goto :EOF
        )
        
        rem ORA-12560: TNS:protocol adapter error
        more !SQLRST! | findstr /i "ORA-12560" >nul
        if !errorlevel! EQU 0 (
            call :Log "ASM-!INSTANCENAME! execsql failed: TNS:protocol adapter error."
            set /a %~9=%ERROR_ASM_NO_STARTUP_TNS%
            goto :EOF
        )

        rem ORA-28002: the password will expire within 7 days
        more !SQLRST! | findstr "ERROR" >nul
        if !errorlevel! EQU 0 (
            call :Log "=====================ASM-!INSTANCENAME! execsql failed====================="
            type !SQLRST! >> !LOGFILEPATH!
            call :Log "=====================ASM-!INSTANCENAME! execsql failed====================="
            set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        )
        
        more !SQLRST! | findstr "ORA-" >nul
        if !errorlevel! EQU 0 (
            call :Log "=====================ASM-!INSTANCENAME! execsql failed====================="
            type !SQLRST! >> !LOGFILEPATH!
            call :Log "=====================ASM-!INSTANCENAME! execsql failed====================="
            set /a %~9=%ERROR_SCRIPT_EXEC_FAILED%
        )
    )
)

goto :EOF

rem ************************************************************************
rem function name: DeleteFile()
rem aim:           Delete file function
rem input:         the deleted file
rem output:        
rem ************************************************************************
:DeleteFile
    set FileName="%~1"
    if exist %FileName% (del /f /q %FileName%)
    
    goto :EOF

rem ************************************************************************
rem function name: Log
rem aim:           Print log function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%agent_func.bat" %LOGFILEPATH%
goto :EOF

rem Get the specified value from input argument
:GetValue
call :start "%~1" "%~2"
goto :eof

:start
if exist %ARGFILENAME% del %ARGFILENAME%
set ArgInput=%~1
set ArgName=%~2
for /l %%a in (1 1 16) do call :cut %%a

for /f "tokens=1,2 delims==" %%i in ('type %ARGFILENAME%') do (
    if %%i==!ArgName! (
        set ArgValue=%%j
    )
)
if exist %ARGFILENAME% del %ARGFILENAME%
goto :eof

:cut
for /f "tokens=%1 delims=:" %%i in ("%ArgInput%") do (
    echo %%i>> %ARGFILENAME%
)
goto :eof

rem ************************************************************************
rem function name: GetValue
rem aim:           Get the specified value from input parameter
rem input:         $2 and ArgName
rem output:        ArgValue
rem ************************************************************************
:ExeSql
    set SQLFILE="%~1"
    set SQLRST="%~2"
    sqlplus /nolog > !SQLRST! @!SQLFILE!
    if !errorlevel! NEQ 0 (
        set /a %~3 = 1
    ) else (    
        set /a %~3 = 0
    )

goto :EOF

:SQLPlusTimeOut
    call :DeleteFile !PROCMONITOR_EXIT!
    cd /d "%AGENT_BIN_PATH%"
    start /min ProcMonitor.bat !CURRENTPID! sqlplus.exe %1 "%AGENT_ROOT%"
goto :EOF

:RmanTimeOut
    call :DeleteFile !PROCMONITOR_EXIT!
    cd /d "%AGENT_BIN_PATH%"
    start /min ProcMonitor.bat !CURRENTPID! rman.exe %1 "%AGENT_ROOT%"
goto :EOF

:KillProcMonitor
    set MONITORPID=
    echo exit > !PROCMONITOR_EXIT!
    call :WinSleep 2
    call :DeleteFile !PROCMONITOR_EXIT!

    rem get current process id
    call :Log "start to kill proc monitor."
    wmic process where name="cmd.exe" get processid > %CURRENTPIDRST%
    more %CURRENTPIDRST% >> %LOGFILEPATH%
    set /a NUM=0
    for /f %%a in ('type %CURRENTPIDRST%') do (
        if !NUM! NEQ 0 (
            set processID=%%a
            wmic process where processid="!processID!" get CommandLine > %CURRENTCMDLineRST%
            more %CURRENTCMDLineRST% >> %LOGFILEPATH%
            more %CURRENTCMDLineRST% | findstr "ProcMonitor.bat"
            if !errorlevel! EQU 0 (
                wmic process where processid="!processID!" get ParentProcessId > %CURRENTCMDLineRST%
                more %CURRENTCMDLineRST% >> %LOGFILEPATH%
                more %CURRENTCMDLineRST% | findstr /i ^"^!CURRENTPID! ^"
                if !errorlevel! EQU 0 (
                    set MONITORPID=%%a
                    if not "!MONITORPID!" == "" (
                        rem check sub process after procmonitor.bat exit
                        call :CheckProcMonitorSubProc !MONITORPID!
                        call :Log "Exists procMonitor.bat id=!MONITORPID!, begin to kill it."
                        taskkill /F /PID !MONITORPID!
                        call :DeleteFile !monitorPIDs!
                    ) else (
                        call :Log "MONITORPID is null."
                    )
                )
            )
        )
        set /a NUM=!NUM!+1
    )
    
    if !NUM! EQU 0 (
        call :Log "canot found ProcMonitor.bat."
        more %CURRENTPIDRST% >> %LOGFILEPATH%
        call :Log "Info end."
    )
    
    call :DeleteFile %CURRENTPIDRST%
    call :DeleteFile %CURRENTCMDLineRST%
    call :Log "finish to kill proc monitor."

goto :EOF

:CheckProcMonitorSubProc
    set /a PMProcessid=%~1
    wmic process where parentprocessid="!PMProcessid!" get name,processid,parentprocessid > %CURRENTCMDLineRST%
    more %CURRENTCMDLineRST% >> %LOGFILEPATH%
    call :DeleteFile %CURRENTCMDLineRST%
goto :EOF

:WinSleep
    timeout %1 > nul
goto :eof

rem endlocal
