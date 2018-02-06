@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

rem result file format: instance name;db name;version;online status;is cluster flag;recovery model;
setlocal enabledelayedexpansion
cd /d %~dp0

set AGENT_ROOT=%~1
set RANDOM_ID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\
set NEED_LOG=1
set LOG_FILE="%AGENT_LOG_PATH%slqserverinfo.log"
set DEFAULT_INSTANCE=MSSQLSERVER
set CLUSTER_NAME_LABEL=ClusterName
set TRIM_STRING=
set HOST_NAME=localhost
set /a ERR_CODE=0
rem 2014(version=12); 2012(version=11); 2008(version=10); 2005(version=9);
rem 2016(version=13);
set /a MIN_SQL_SERVER_VER_NUMBER=9
set /a MAX_SQL_SERVER_VER_NUMBER=13
set /a MAX_DATABASE_NAME_LENGTH=129
set /a DB_STATUS_ONLINE=0
set /a DB_STATUS_OFFLINE=1
set /a DB_RECOVERY_MODEL_UNKNOW=0
set TMP_FILE="%AGENT_TMP_PATH%sql_info_tmp%RANDOM_ID%"
set RESULT_FILE="%AGENT_TMP_PATH%result_tmp%RANDOM_ID%"
set INSTANCE_LIST_FILE="%AGENT_TMP_PATH%sql_info_instance_list%RANDOM_ID%"
set DATABASE_LIST_FILE="%AGENT_TMP_PATH%sql_info_db_list%RANDOM_ID%"
set OSQL_EXIT_SQL_FILE="%AGENT_TMP_PATH%sql_info_osql_exit%RANDOM_ID%"
set SQL_STATUS_FILE="%AGENT_TMP_PATH%sql_info_status%RANDOM_ID%"

set /a ERROR_SCRIPT_COMMON_PATH_WRONG=8


call :Log "####################Begin get sql server info.####################"
call :GetOsqlCmdPath RET_CODE OSQL_PATH
if %RET_CODE% NEQ 0 (
    set /a ERR_CODE=!ERROR_SCRIPT_COMMON_PATH_WRONG!
    call :Log "Get osql cmd path failed, exit code !ERROR_SCRIPT_COMMON_PATH_WRONG!."
    goto :error
)
call :Log "Get osql path !OSQL_PATH!."

call :GetALLInstanceInfo RET_CODE
if %RET_CODE% NEQ 0 (
    set /a ERR_CODE=!RET_CODE!
    call :Log "Get all instance info failed, exit code !RET_CODE!."
    goto :error
)
goto :end

rem input param: ret code;
:GetALLInstanceInfo
    call :Log "Begin get all instance info."
    call :GetInstanceList RET_CODE
    if %RET_CODE% NEQ 0 (
        set /a %1=1
        call :DeleteFile %INSTANCE_LIST_FILE%
        call :Log "Get instance list failed."
        goto :eof
    )
    
    for /f "tokens=*" %%a in ('type %INSTANCE_LIST_FILE%') do (
        call :GetInstanceInfo RET_CODE %%a
        if !RET_CODE! NEQ 0 (
            call :Log "Get instance info failed, continue to get next one, current instance name %%a."
        )
    )
    set /a %1=0
    call :DeleteFile %INSTANCE_LIST_FILE%
    call :Log "End get all instance info."
    goto :eof

rem input param: ret code; instance name;
:GetInstanceInfo
    call :Log "Begin get instance info, instance name %2."
    call :GetClusterInstanceInfo %2 IS_CLUSTER CLUSTER_NAME

    call :GetDBVersion RET_CODE %2 DB_VERSION !IS_CLUSTER! !CLUSTER_NAME!
    if !RET_CODE! NEQ 0 (
        set /a %1=1
        call :Log "Get db version failed."
        goto :eof
    )
    
    call :GetDBNameList RET_CODE %2 !IS_CLUSTER! !CLUSTER_NAME!
    if !RET_CODE! NEQ 0 (
        set /a %1=1
        call :DeleteFile %DATABASE_LIST_FILE%
        call :Log "Get db name list failed."
        goto :eof
    )
    
    for /f "tokens=1" %%i in ('type %DATABASE_LIST_FILE%') do (
        call :Log "Get details info for db %%i."
        call :GetDBOnlineStatus %2 %%i DB_ONLINE_STATUS !IS_CLUSTER! !CLUSTER_NAME!
        call :GetDBRecoveryModel %2 %%i DB_RECOVERY_MODEL !IS_CLUSTER! !CLUSTER_NAME!
        call :Log "Get db info, instance name %2, database name %%i, version !DB_VERSION!, online status !DB_ONLINE_STATUS!, is cluster !IS_CLUSTER!, recovery model !DB_RECOVERY_MODEL!."
        echo %2;%%i;!DB_VERSION!;!DB_ONLINE_STATUS!;!IS_CLUSTER!;!DB_RECOVERY_MODEL! >> %RESULT_FILE%
    )

    call :DeleteFile %DATABASE_LIST_FILE%
    call :Log "End get instance info."
    goto :eof

rem input param: ret code; instance name; is cluster flag; cluster name;
:GetDBNameList
    call :Log "Begin get db name list, instance name %2, is cluster flag %3, cluster name %4."
    call :GetDBCount RET_CODE %2 DB_COUNT %3 %4
    if !RET_CODE! NEQ 0 (
        set /a %1=1
        call :Log "Get db count failed."
        goto :eof
    )

    call :ExecSqlWithWidth RET_CODE %2 "select name from sys.databases" %MAX_DATABASE_NAME_LENGTH% %3 %4
    if !RET_CODE! NEQ 0 (
        set /a %1=1
        call :DeleteFile %TMP_FILE%
        call :Log "Get db name failed."
        goto :eof
    )

    call :DeleteFile %DATABASE_LIST_FILE%
    set /a TMP_COUNT=0
    for /f "skip=2 delims=" %%a in ('type %TMP_FILE%') do (
        if !TMP_COUNT! LSS !DB_COUNT! (
            set TRIM_STRING=%%a
            call :TrimSpace
            call :Log "Get db name !TRIM_STRING![%%a]."
            echo !TRIM_STRING! >> %DATABASE_LIST_FILE%
        )
        set /a TMP_COUNT += 1
    )
    
    set /a %1=0
    call :DeleteFile %TMP_FILE%
    call :Log "Get db name list succ."
    goto :eof

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

rem input param: instance name; db name; recovery model(0 -- unknow, 1 -- full, 2 -- bulk logged, 3 -- simple); is cluster flag; cluster name;
:GetDBRecoveryModel
    call :Log "Begin get db recovery model, instance name %1, db name %2."
    call :ExecSql RET_CODE %1 "select recovery_model from sys.databases where name = '%2'" %4 %5
    if !RET_CODE! NEQ 0 (
        set /a %3=%DB_RECOVERY_MODEL_UNKNOW%
        call :DeleteFile %TMP_FILE%
        call :Log "Execute the select recovery model sql failed, set the model to unknow."
        goto :eof
    )
    
    set /a GET_EXPECTED_LINE=0
    for /f "skip=2 delims=" %%a in ('type %TMP_FILE%') do (
        if !GET_EXPECTED_LINE! EQU 1 (
            call :DeleteFile %TMP_FILE%
            call :Log "Get db recovery model succ, skip line %%a, recovery model !%3!."
            goto :eof
        )
        
        set TRIM_STRING=%%a
        call :TrimSpace
        call :Log "Get db recovery model !TRIM_STRING![%%a]."
        set /a %3=%%a
        set /a GET_EXPECTED_LINE=1
    )

    set /a %3=%DB_RECOVERY_MODEL_UNKNOW%
    call :DeleteFile %TMP_FILE%
    call :Log "Can't get the db recovery model, set the model to unknow."
    goto :eof

rem input param: instance name; db name; online status(0 -- online, 1 -- offline); is cluster flag; cluster name;
:GetDBOnlineStatus
    call :Log "Begin get db online status, instance name %1, db name %2."
    call :ExecSql RET_CODE %1 "sp_helpdb [%2]" %4 %5
    if !RET_CODE! NEQ 0 (
        set /a %3=%DB_STATUS_OFFLINE%
        call :DeleteFile %TMP_FILE%
        call :Log "Can't get db online status, set the online status to offline."
        goto :eof
    )

    rem if the database if offline, we there is no "Status" string exist in SQL_STATUS_FILE file
    type %TMP_FILE% | findstr /i "Status=" > %SQL_STATUS_FILE%
    if !errorlevel! NEQ 0 (
        set /a %3=%DB_STATUS_OFFLINE%
        call :DeleteFile %TMP_FILE%
        call :DeleteFile %SQL_STATUS_FILE%
        call :Log "Status string dose not exist, set the online status to offline."
        goto :eof
    )
    
    for /f "delims=," %%a in ('type %SQL_STATUS_FILE%') do (
        call :Log "Find status string %%a."
        for /f "tokens=2 delims==" %%b in ("%%a") do (
            if %%b==ONLINE (
                set /a %3=%DB_STATUS_ONLINE%
            ) else (
                set /a %3=%DB_STATUS_OFFLINE%
            )
        )
    )

    call :DeleteFile %TMP_FILE%
    call :DeleteFile %SQL_STATUS_FILE%
    call :Log "Get db online status succ, status !%3!."
    goto :eof

rem input param: ret code; instance name; db version; is cluster flag; cluster name;
:GetDBVersion
    call :Log "Begin get db version, instance name %2."
    call :ExecSql RET_CODE %2 "select @@version" %4 %5
    if !RET_CODE! NEQ 0 (
        set /a %1=1
        call :DeleteFile %TMP_FILE%
        call :Log "Get db version failed."
        goto :eof
    )

    set /a TMP_INDEX=0
    for /f "skip=10 delims=" %%a in ('type %TMP_FILE%') do (
        set /a TMP_INDEX+=1
        if !TMP_INDEX! EQU 1 (
            set %3=%%a
        )
    )
    set /a %1=0
    call :DeleteFile %TMP_FILE%
    call :Log "Get db version succ, version !%3!."
    goto :eof

rem input param lsit: ret code; instance name; db count; is cluster flag; cluster name;
:GetDBCount
    call :Log "Begin get db count, instance name %2."
    call :ExecSql RET_CODE %2 "select count(*) from master..sysdatabases" %4 %5
    if !RET_CODE! NEQ 0 (
        set /a %1=1
        call :DeleteFile %TMP_FILE%
        call :Log "Get db count failed."
        goto :eof
    )
    
    set /a TMP_INDEX=0
    for /f "skip=2 eol=" %%a in ('type %TMP_FILE%') do (
        set /a TMP_INDEX+=1
        if !TMP_INDEX! EQU 1 (
            set /a %3=%%a
        )
    )
    set /a %1=0
    call :DeleteFile %TMP_FILE%
    call :Log "Get db count succ, db count !%3!."
    goto :eof

rem input param list: ret code; instance name; sql; width; is cluster flag; cluster name;
rem the caller need to delete the "TMP_FILE" outside of this function
:ExecSqlWithWidth
    call :Log "Begin execute sql with width, instance name %2, sql %~3, width %4, cluster flag %5, cluster name %6."
    echo exit > %OSQL_EXIT_SQL_FILE%
    rem non cluster instance
    if %5==0 (
        if %~2 EQU %DEFAULT_INSTANCE% (
            call :Log "Execute sql on default non cluster instance."
            "!OSQL_PATH!" -b -E -q %3 -w %4 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        ) else (
            call :Log "Execute sql on non cluster instance."
            "!OSQL_PATH!" -b -E -S !HOST_NAME!\%~2 -q %3 -w %4 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        )
    rem cluster instance
    ) else (
        if %~2 EQU %DEFAULT_INSTANCE% (
            call :Log "Execute sql on default cluster instance."
            "!OSQL_PATH!" -b -E -S%6 -q %3 -w %4 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        ) else (
            call :Log "Execute sql on cluster instance."
            "!OSQL_PATH!" -b -E -S%6\%~2 -q %3 -w %4 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        )
    )

    rem set ret code
    set /a %1=!errorlevel!
    call :DeleteFile %OSQL_EXIT_SQL_FILE%
    call :Log "End execute sql with width, ret code !%1!."
    goto :eof

rem input param list: ret code; instance name; sql; is cluster flag; cluster name;
rem the caller need to delete the "TMP_FILE" outside of this function
:ExecSql
    call :Log "Begin execute sql, instance name %2, sql %~3, cluster flag %4, cluster name %5."
    echo exit > %OSQL_EXIT_SQL_FILE%
    rem non cluster instance
    if %4==0 (
        if %~2 EQU %DEFAULT_INSTANCE% (
            call :Log "Execute sql on default non cluster instance."
            "!OSQL_PATH!" -b -E -q %3 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        ) else (
            call :Log "Execute sql on non cluster instance."
            "!OSQL_PATH!" -b -E -S !HOST_NAME!\%~2 -q %3 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        )
    rem cluster instance
    ) else (
        if %~2 EQU %DEFAULT_INSTANCE% (
            call :Log "Execute sql on default cluster instance."
            "!OSQL_PATH!" -b -E -S%5 -q %3 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
        ) else (
            call :Log "Execute sql on cluster instance."
            "!OSQL_PATH!" -b -E -S%5\%~2 -q %3 -i%OSQL_EXIT_SQL_FILE% > %TMP_FILE%
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

rem input param: ret code;
:GetInstanceList
    set /a %1=1
    call :Log "Begin get instance list."
    rem get default instance
    sc query state= all | findstr -c:"SERVICE_NAME: MSSQLSERVER"
    if !errorlevel! equ 0 (
        call :Log "Get default instance MSSQLSERVER."
        echo %DEFAULT_INSTANCE% >> %INSTANCE_LIST_FILE%
        set /a %1=0
    )

    rem get other instances
    sc query state= all | findstr -c:"SERVICE_NAME: MSSQL$" >> %TMP_FILE%
    if !errorlevel! equ 0 (
        for /f "tokens=1,2 delims=$" %%i in ('type %TMP_FILE%') do (
            call :Log "Get sql instance %%j."
            echo %%j >> %INSTANCE_LIST_FILE%
            set /a %1=0
        )
    )
    
    call :DeleteFile %TMP_FILE%
    call :Log "End get instance list."
    goto :eof

:DeleteFile
    set FileName=%~1
    if exist "%FileName%" (del /f /q "%FileName%")
    goto :eof

:Log
    if %NEED_LOG%==1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOG_FILE%
    )
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOG_FILE%
    goto :eof

:error
    call :Log "####################Get sql server info failed.####################"
    exit /b %ERR_CODE%
    
:end
    call :Log "####################End get sql server info.####################"
    endlocal
    exit 0

