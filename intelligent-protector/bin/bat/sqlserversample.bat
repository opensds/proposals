@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

cd %~dp0

set AGENT_ROOT=%~1
set RANDOM_ID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set NEED_LOG=1
set LOG_FILE="%AGENT_LOG_PATH%sqlserversample.log"
rem 2014(version=12); 2012(version=11); 2008(version=10); 2005(version=9);
rem 2016(version=13);
set /a MIN_SQL_SERVER_VER_NUMBER=9
set /a MAX_SQL_SERVER_VER_NUMBER=13
set /a DB_STATUS_ONLINE=0
set /a DB_STATUS_OFFLINE=1
set /a EXEC_TYPE_TESTDB=2
set /a EXEC_TYPE_TRUNCATE=3
set TRIM_STRING=
set INPUT_PARAM=
set /a ERR_CODE=0
set CLUSTER_NAME_LABEL=ClusterName
set DEFAULT_INSTANCE=MSSQLSERVER
set INPUT_TMP_FILE="%AGENT_TMP_PATH%input_tmp%RANDOM_ID%"
set RESULT_FILE="%AGENT_TMP_PATH%result_tmp%RANDOM_ID%"
set TMP_FILE="%AGENT_TMP_PATH%sql_sample_tmp%RANDOM_ID%"
set ARG_FILE="%AGENT_TMP_PATH%sql_sample_args%RANDOM_ID%"
set OSQL_EXIT_SQL_FILE="%AGENT_TMP_PATH%sql_sample_osql_exit%RANDOM_ID%"
set SQL_STATUS_FILE="%AGENT_TMP_PATH%sql_sample_status%RANDOM_ID%"
set INSTANCE_LIST_FILE="%AGENT_TMP_PATH%sql_sample_instance_list%RANDOM_ID%"

rem error code
set /a ERROR_SCRIPT_COMMON_PATH_WRONG=8
set /a ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG=10
set /a ERROR_SCRIPT_COMMON_INSTANCE_NOSTART=11
set /a ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR=130
set /a ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED=131
set /a ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED=134
set /a ERROR_SCRIPT_SQLSERVER_DB_STATUS_OFFLINE=135
set /a ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST=136

call :Log "####################Begin sqlserver operations.####################"
rem get the input params from the input tmp file
for /f "delims=" %%i in ('type %INPUT_TMP_FILE%') do (
    if not "%%i" == "" (
        set INPUT_PARAM=%%i
    )
)
call :DeleteFile %INPUT_TMP_FILE%

call:GetValue "%INPUT_PARAM%" INSTNAME
set INSTANCE_NAME=%ArgValue%
call:GetValue "%INPUT_PARAM%" DBNAME
set DB_NAME=%ArgValue%
call:GetValue "%INPUT_PARAM%" DBUSERNAME
set USER_NAME=%ArgValue%
call:GetValue "%INPUT_PARAM%" DBPASSWORD
rem set the env param for the special charactors in password
set OSQLPASSWORD=%ArgValue%
call:GetValue "%INPUT_PARAM%" CHECKTYPE
set EXEC_TYPE=%ArgValue%

rem we must enable delayed expasion here, or we may get the wrong password.
setlocal EnableDelayedExpansion
call :Log "Get input param, instance name !INSTANCE_NAME!, db name !DB_NAME!, user name !USER_NAME!, check type !EXEC_TYPE!."

call :GetOsqlCmdPath RET_CODE OSQL_PATH
if !RET_CODE! NEQ 0 (
    set /a ERR_CODE=!ERROR_SCRIPT_COMMON_PATH_WRONG!
    call :Log "Get osql cmd path failed, exit code !ERROR_SCRIPT_COMMON_PATH_WRONG!."
    goto :error
)

call :IsInstanceExist RET_CODE !INSTANCE_NAME!
if !RET_CODE! NEQ 0 (
    set /a ERR_CODE=%ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST%
    call :Log "Instance !INSTANCE_NAME! not exist, exit code !ERR_CODE!."
    goto :error
)

call :IsInstanceStart !INSTANCE_NAME!
if !errorlevel! NEQ 0 (
    set /a ERR_CODE=%ERROR_SCRIPT_COMMON_INSTANCE_NOSTART%
    call :Log "Instance !INSTANCE_NAME! is not start."
    goto :error
)

call :GetClusterInstanceInfo !INSTANCE_NAME! IS_CLUSTER CLUSTER_NAME

call :TestConnection RET_CODE !INSTANCE_NAME! !USER_NAME! !IS_CLUSTER! !CLUSTER_NAME!
if !RET_CODE! NEQ 0 (
    set /a ERR_CODE=!RET_CODE!
    call :Log "Test connection failed, exit code !ERR_CODE!."
    goto :error
)

if %EXEC_TYPE%==%EXEC_TYPE_TESTDB% (
    call :TestDB RET_CODE !INSTANCE_NAME! !DB_NAME! !USER_NAME! !IS_CLUSTER! !CLUSTER_NAME!
    if !RET_CODE! NEQ 0 (
        set /a ERR_CODE=!RET_CODE!
        call :Log "Test db failed, exit code !ERR_CODE!."
        goto :error
    )
    
    call :DeleteFile %TMP_FILE%
    
) else if %EXEC_TYPE%==%EXEC_TYPE_TRUNCATE% (
    call :TruncateLog RET_CODE !INSTANCE_NAME! !DB_NAME! !USER_NAME! !IS_CLUSTER! !CLUSTER_NAME!
    if !RET_CODE! NEQ 0 (
        set /a ERR_CODE=!RET_CODE!
        call :Log "Truncate log failed, exit code !ERR_CODE!."
        goto :error
    )
) else (
    set /a ERR_CODE=%ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR%
    call :Log "Invalid execute type."
    goto :error
)

goto :end

rem input param: ret code; instance name;
:IsInstanceExist
    call :GetInstanceList RET_CODE
    if !RET_CODE! NEQ 0 (
        set /a %1=1
        call :Log "Get instance list failed."
        goto :eof
    )
    
    for /f "tokens=*" %%a in ('type %INSTANCE_LIST_FILE%') do (
        set TRIM_STRING=%%a
        call :TrimSpace
        call :Log "Compare instance %2 to !TRIM_STRING![%%a]."
        if %2 EQU !TRIM_STRING! (
            set /a %1=0
            call :DeleteFile %INSTANCE_LIST_FILE%
            call :Log "Instance %2 exist."
            goto :eof
        )
    )
    
    set /a %1=1
    call :DeleteFile %INSTANCE_LIST_FILE%
    call :Log "Instance %2 not exist."
    goto :eof
    
rem input param: instance name
:IsInstanceStart
    set SERVICE_CHECK=
    if "%~1" == "%DEFAULT_INSTANCE%" (
        set SERVICE_INSTANCE=%~1
    ) else (
        set SERVICE_INSTANCE=MSSQL$%~1
    )
    
    call :Log "SC Query the status about Service [!SERVICE_INSTANCE!]."
    for /f "delims=" %%i in ('sc query !SERVICE_INSTANCE! ^| findstr " STATE" ^| findstr "RUNNING"') do (set SERVICE_CHECK=%%i)
    if "!SERVICE_CHECK!" == "" (
        call :Log "The service status !SERVICE_INSTANCE! is not RUNNING."
        exit /b 1
    )
    
    exit /b 0

:TrimSpace
    :Trimleft
    if "!TRIM_STRING:~0,1!"==" " (
        set TRIM_STRING=!TRIM_STRING:~1!
        goto Trimleft
    )
        
    :TrimRight
    if "!TRIM_STRING:~-1!"==" " (
        set TRIM_STRING=!TRIM_STRING:~0,-1!
        goto TrimRight
    )
    goto :eof
    
rem input param: ret code;
:GetInstanceList
    set /a %1=1
    call :Log "Begin get instance list."
    rem get default instance
    sc query state= all | findstr -c:"SERVICE_NAME: MSSQLSERVER"
    if !errorlevel! EQU 0 (
        call :Log "Get default instance MSSQLSERVER."
        echo %DEFAULT_INSTANCE% >> %INSTANCE_LIST_FILE%
        set /a %1=0
    )

    rem get other instances
    sc query state= all | findstr -c:"SERVICE_NAME: MSSQL$" >> %TMP_FILE%
    if !errorlevel! EQU 0 (
        for /f "tokens=1,2 delims=$" %%i in ('type %TMP_FILE%') do (
            call :Log "Get sql instance %%j."
            echo %%j >> %INSTANCE_LIST_FILE%
            set /a %1=0
        )
    )
    
    call :DeleteFile %TMP_FILE%
    call :Log "End get instance list."
    goto :eof

rem input param: ret code; instance name; user name; is cluster flag; cluster name;
rem Connection for instance, not for database
:TestConnection
    call :Log "Begin test connection %2 %3 %4 %5."
    call :ExecSql RET_CODE %2 "" "" %3 %4 %5
    if !RET_CODE! NEQ 0 (
        set /a %1=%ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG%
        call :DeleteFile %TMP_FILE%
        call :Log "Test connection failed."
        goto :eof
    )

    call :Log "Test connection succ."
    goto :eof
     
rem input param: ret code; instance name; db name; user name; is cluster flag; cluster name;
:TestDB
    call :Log "Begin test db %2 %3 %4 %5 %6."
    call :GetDBOnlineStatus RET_CODE %2 %3 %4 DB_ONLINE_STATUS %5 %6
    if !RET_CODE! NEQ 0 (
        set /a %1=%ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED%
        call :Log "Get db [%3] online status failed or not exist."
        goto :eof
    )

    if !DB_ONLINE_STATUS! NEQ %DB_STATUS_ONLINE% (
        set /a %1=%ERROR_SCRIPT_SQLSERVER_DB_STATUS_OFFLINE%
        call :Log "Database is offline."
        goto :eof
    )

    call :ExecSql RET_CODE %2 %3 "" %4 %5 %6
    if !RET_CODE! NEQ 0 (
        set /a %1=!RET_CODE!
        call :DeleteFile %TMP_FILE%
        call :Log "Connect db failed."
        goto :eof
    )

    set /a %1=0
    call :Log "Test db succ, ret code !%1!."
    goto :eof

rem input param: ret code; instance name; db name; user name; is cluster flag; cluster name;
:TruncateLog
    call :Log "Begin truncate log %2 %3 %4 %5 %6."
    call :ExecSql RET_CODE %2 %3 "BACKUP LOG [%3] TO DISK='NUL:'" %4 %5 %6
    if !RET_CODE! NEQ 0 (
        set /a %1=!RET_CODE!
        call :DeleteFile %TMP_FILE%
        call :Log "Truncate log failed."
        goto :eof
    )

    set /a %1=0
    call :DeleteFile %TMP_FILE%
    call :Log "End truncate log."
    goto :eof

rem input param: ret code; instance name; db name; user name; online status(0 -- online, 1 -- offline); is cluster flag; cluster name;
:GetDBOnlineStatus
    call :Log "Begin get db online status, instance name %2, db name %3."
    call :ExecSql RET_CODE %2 "" "sp_helpdb [%3]" %4 %6 %7
    if !RET_CODE! NEQ 0 (
        rem db not exist or query status failed.
        set /a %1=1
        call :DeleteFile %TMP_FILE%
        call :Log "Db [%3] is not exist or get status failed."
        goto :eof
    )

    rem if the database if offline, we there is no "Status" string exist in SQL_STATUS_FILE file
    type %TMP_FILE% | findstr /i "Status=" > %SQL_STATUS_FILE%
    if !errorlevel! NEQ 0 (
        rem db offline, the  SQL_STATUS_FILE not exist Status=...
        set /a %5=%DB_STATUS_OFFLINE%
        call :DeleteFile %TMP_FILE%
        call :DeleteFile %SQL_STATUS_FILE%
        call :Log "Status string dose not exist, set the online status to offline."
        goto :eof
    )
    
    for /f "delims=," %%a in ('type %SQL_STATUS_FILE%') do (
        call :Log "Find status string %%a."
        for /f "tokens=2 delims==" %%b in ("%%a") do (
            if %%b==ONLINE (
                set /a %5=%DB_STATUS_ONLINE%
            ) else (
                set /a %5=%DB_STATUS_OFFLINE%
            )
        )
    )

    set /a %1=0
    call :DeleteFile %TMP_FILE%
    call :DeleteFile %SQL_STATUS_FILE%
    call :Log "Get db online status succ, status !%4!."
    goto :eof

rem input param list: ret code; instance name; db name; sql; user name; is cluster flag; cluster name;
rem the caller need to delete the "TMP_FILE" outside of this function
:ExecSql
    call :Log "Begin execute sql, instance name %2, db name %3, sql %~4, user name %5, cluster flag %6, cluster name %7."
    echo exit > %OSQL_EXIT_SQL_FILE%
    rem non cluster instance
    if %6==0 (
        if %~2 EQU %DEFAULT_INSTANCE% (
            call :Log "Execute sql on default non cluster instance."
            "!OSQL_PATH!" -b -S localhost -d%3 -q %4 -U%5 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        ) else (
            call :Log "Execute sql on non default non cluster instance."
            "!OSQL_PATH!" -b -S localhost\%~2 -d%3 -q %4 -U%5 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        )
    rem cluster instance
    ) else (
        if %~2 EQU %DEFAULT_INSTANCE% (
            call :Log "Execute sql on default cluster instance."
            "!OSQL_PATH!" -b -S %7 -d%3 -q %4 -U%5 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        ) else (
            call :Log "Execute sql on non default cluster instance."
            "!OSQL_PATH!" -b -S %7\%~2 -d%3 -q %4 -U%5 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        )
    )

    rem set ret code
    set /a %1=!errorlevel!
    call :DeleteFile %OSQL_EXIT_SQL_FILE%
    call :Log "End execute sql, ret code !%1!."
    goto :eof

 rem input param: instance name; is cluster instance(0 -- no, 1 -- yes); cluster name;
 :GetClusterInstanceInfo
     call :Log "Begin get cluster info, instance %1."
     set /a GET_EXPECTED_LINE=0
     for /f "tokens=* delims=" %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server" /f "MSSQL*.%1" /k') do (
         if !GET_EXPECTED_LINE! EQU 1 (
             call :Log "Skip this line, %%i."
             goto :eof
         )
         
         set CLUSTER_INSTANCE_REG_INFO="%%i\Cluster"
         call :Log "Query reg info %%i."
         call :GetClusterName !CLUSTER_INSTANCE_REG_INFO! TMP_CLUSTER_NAME RET_CODE
         if !RET_CODE! EQU 0 (
             set /a %2=1
             set %3=!TMP_CLUSTER_NAME!
             call :Log "Current instance is belonged to cluster !TMP_CLUSTER_NAME!."
         ) else (
             set /a %2=0
             call :Log "Current instance is not a cluster instance."
         )
         
         set /a GET_EXPECTED_LINE=1
     )
     
     call :Log "End get cluster info."
     goto :eof
 
:GetClusterName
    set %3=1
    call :Log "Begin get cluster name."
    for /f "tokens=1,2,* " %%i in ('reg query %1 /v "%CLUSTER_NAME_LABEL%"') do (
        set "%2=%%k"
        set %3=0
    )
    call :Log "End get cluster."
    goto :eof

rem input param: ret code; osql path;
:GetOsqlCmdPath
    set /a %1=1
    for /l %%a in (%MIN_SQL_SERVER_VER_NUMBER% 1 %MAX_SQL_SERVER_VER_NUMBER%) do (
        for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server\%%a0\Tools\ClientSetup" /v "Path"') do (
            set "OSQL_PATH_TMP=%%k"
            set %2=!OSQL_PATH_TMP!\osql
            set /a %1=0
        )
    )
    goto :eof

:GetValue
    call :DeleteFile %ARG_FILE%
    set ArgInput=%~1
    set ArgName=%~2
    set "ArgValue="

    for /l %%a in (1 1 20) do call :Separate %%a

    for /f "tokens=1,2 delims==" %%i in ('type %ARG_FILE%') do (
        if %%i==%ArgName% (
            set ArgValue=%%j
        )
    )
    call :DeleteFile %ARG_FILE%
    goto :EOF

:Separate
    for /f "tokens=%1 delims=;" %%i in ("%ArgInput%") do (
        echo %%i>> %ARG_FILE%
    )
    goto :EOF

:DeleteFile
    set FileName=%1
    if exist %FileName% (del /f /q %FileName%)
    
    goto :EOF
    
:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOG_FILE%
    )
    call %AGENT_BIN_PATH%\agent_func.bat %LOG_FILE%
    goto :EOF

:error
    call :log "Sql server operation failed."
    call :Log "####################End sqlserver operations.####################"
    exit /b %ERR_CODE%
    
:end
    call :Log "####################End sqlserver operations.####################"
    endlocal

