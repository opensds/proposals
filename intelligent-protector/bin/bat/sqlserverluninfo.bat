@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal EnableDelayedExpansion
rem  ******************************************************************************************
rem  *program name:          SqlServerAdaptive.bat  
rem  *function:              SqlServer Adaptive Disk info
rem  *author:                
rem  *time:                  2010-12-14
rem  *function and description:
rem  *rework:                First Programming
 
rem  *rework:modified for adaptive don't work on english OS issue,translate Chinese comments to English
rem  *author:  
rem  *time:   2012-7-18
rem  *explain:translate Chinese comments to English                            
rem  ******************************************************************************************

setlocal DisableDelayedExpansion
rem *********************parameter*******************************
set RANDOMID=%2

rem ************ErrorCode*****************
set /a OPERATION_RTN_SUCCESS=0
set /a ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG=10
set /a ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR=130
set /a ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED=131
set /a ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED=134
set /a ERROR_SCRIPT_SQLSERVER_DB_OFFLINE_OR_NOT_EXIST=135
set /a ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST=136
set /a ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST=137
set /a ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED=138
set /a ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED=139
set /a ERROR_SCRIPT_COMMON_INSTANCE_NOSTART=11

rem *********************variable*******************************
set AGENT_ROOT=%~1
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\

set ExitValues=0
set AllSize=0
set FreeSize=0
set FilePath=--
set ArgValue=--
set DATABASENAE=--
set MoRenShiLi=MSSQLSERVER
set CLUSTERNAME=
set SelectSize="sp_spaceused @updateusage = 'TRUE'"
set SelectPath="select physical_name from sys.database_files"

rem *********************file define*******************************
set RST="%AGENT_TMP_PATH%result_tmp%RANDOMID%"
set PATHFILE="%AGENT_TMP_PATH%PATH%RANDOMID%.txt"
set EXITFILE="%AGENT_TMP_PATH%EXIT%RANDOMID%.txt"
set ARGFILE="%AGENT_TMP_PATH%ARG%RANDOMID%.txt"
set TMPFILE="%AGENT_TMP_PATH%TMP%RANDOMID%.txt"
set SERVERNAMEFILE="%AGENT_TMP_PATH%SVNAME%RANDOMID%.txt"
set LOGFILENAME="%AGENT_LOG_PATH%sqlserverluninfo.log"

set SQLSERVERDBINFO="%AGENT_TMP_PATH%GetDBInfoFile%RANDOMID%.txt"
set DBSTATUSSQL="%AGENT_TMP_PATH%DBSatus%RANDOMID%.sql"
set SQLSERVERDBSTATUS="%AGENT_TMP_PATH%GetDBStatusFile%RANDOMID%.txt"
set DBSqlService="%AGENT_TMP_PATH%DBSqlService%RANDOMID%.txt"
set InstanceListTmp="%AGENT_TMP_PATH%InstanceListTmp%RANDOMID%.txt"
set InstanceList="%AGENT_TMP_PATH%InstanceList%THREADID%.txt"
set DEFAULTINSTANCENAME=MSSQLSERVER
rem ERROR file
set CHECKCLUSTER="%AGENT_TMP_PATH%CheckCluster%RANDOMID%.txt"

set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%RANDOMID%"
set INPUTMSG=
for /f "delims=" %%a in ('type %PARAM_FILE%') do (
    if not "%%a" == "" (
        set INPUTMSG=%%a
    )
)
if exist %PARAM_FILE% (del /f /q %PARAM_FILE%)

call :GetValue "%INPUTMSG%" INSTNAME
set SubAppName=%ArgValue%

call :GetValue "%INPUTMSG%" DBNAME
set DATABASENAE=%ArgValue%

call :GetValue "%INPUTMSG%" DBUSERNAME
set UserNam=%ArgValue%

call :GetValue "%INPUTMSG%" DBPASSWORD
set OSQLPASSWORD=%ArgValue%

rem add new item DBClusterFlag & DBClusteName;
call :GetValue "%INPUTMSG%"  ISCLUSTER
set CLUSTERFLAG=%ArgValue%

echo RANDOMID=%RANDOMID%;HOSTNAME=%hostname%;SUBAPPNAME=%SubAppName%;DBNAME=%DATABASENAE%;DBUSER=%UserNam%;CLUSTERFLAG=%CLUSTERFLAG%;CLUSTERNAME=%CLUSTERNAME%
call :Log "RANDOMID=%RANDOMID%;HOSTNAME=%hostname%;SUBAPPNAME=%SubAppName%;DBNAME=%DATABASENAE%;DBUSER=%UserNam%;CLUSTERFLAG=%CLUSTERFLAG%;CLUSTERNAME=%CLUSTERNAME%"

setlocal EnableDelayedExpansion

echo exit > %EXITFILE%
echo localhost > %SERVERNAMEFILE%

if %CLUSTERFLAG%==1 (
    call :GetDBClusterName
)

for /f "tokens=1" %%i in ('type %SERVERNAMEFILE%') do (
    set SERVERNAME=%%i
)
if exist %SERVERNAMEFILE% (del /f /q %SERVERNAMEFILE%)

rem check cluster status
if %CLUSTERFLAG%==1 (
    powershell "Get-Cluster" > %CHECKCLUSTER%
    if !errorlevel! neq 0 (
        call :Log "get cluster info failed."
        more %CHECKCLUSTER% >> %LOGFILENAME%
        if exist %CHECKCLUSTER% (del /f /q %CHECKCLUSTER%)
        if exist %SERVERNAMEFILE% (del /f /q %SERVERNAMEFILE%)
        if exist %EXITFILE% (del /f /q %EXITFILE%)
        exit %ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED%
    )
    if exist %CHECKCLUSTER% (del /f /q %CHECKCLUSTER%)
    
)

for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\ComputerName\ComputerName" /v "ComputerName"') do set "SERVERNAME=%%k" 

set YZFS=-U %UserNam%

if %CLUSTERFLAG%==0 (
    if %MoRenShiLi%==%SubAppName% (
        set YZFS=-U %UserNam%
    ) else (
        set YZFS=-S %SERVERNAME%\%SubAppName% -U %UserNam%
    )
) else (
    if %MoRenShiLi%==%SubAppName% (
        set YZFS=-S %CLUSTERNAME% -U %UserNam%
    ) else (
        set YZFS=-S %CLUSTERNAME%\%SubAppName% -U %UserNam% -P %OSQLPASSWORD%
    )
)
for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server\90\Tools\ClientSetup" /v "Path"') do set "OSQLPATH=%%k"
if "!OSQLPATH!"=="" (
    for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server\100\Tools\ClientSetup" /v "Path"') do set "OSQLPATH=%%k"
)
if "!OSQLPATH!"=="" (
    for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server\110\Tools\ClientSetup" /v "Path"') do set "OSQLPATH=%%k"
)
if "!OSQLPATH!"=="" (
    for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server\120\Tools\ClientSetup" /v "Path"') do set "OSQLPATH=%%k"
)
if "!OSQLPATH!"=="" (
    for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server\130\Tools\ClientSetup" /v "Path"') do set "OSQLPATH=%%k"
)

set OSQLPATH="%OSQLPATH%\osql"

rem check instance
call :Checkinstance

rem check user password
call :CheckUserPassword %DATABASENAE% RetStatus RetCode
if !RetCode! neq 0 (
    call :Log "CheckUserPassword exec failed, RetCode is !RetCode!."
    if exist %EXITFILE% (del /f /q %EXITFILE%)
    exit 5
)

if !RetStatus! neq 0 (
    call :Log "CheckUserPassword exec failed, RetStatus is !RetStatus!."
    if exist %EXITFILE% (del /f /q %EXITFILE%)
    exit !RetStatus!
)
call :Log "DBNAME:%DATABASENAE% End to check user password."

call :Log "DBNAME:%DATABASENAE% Begin to check db status."
rem check db status
call :GetDBStatus %DATABASENAE% RetStatus RetCode
if !RetCode! neq 0 (
    call :Log "GetDBStatus exec failed."
    if exist %EXITFILE% (del /f /q %EXITFILE%)
    exit 5
)

if !RetStatus! neq 0 (
    call :Log "DB is offline."
    if exist %EXITFILE% (del /f /q %EXITFILE%)
    exit %ERROR_SCRIPT_SQLSERVER_DB_OFFLINE_OR_NOT_EXIST%
)
call :Log "DBNAME:%DATABASENAE% End to check db status."

call :Log "DBNAME:%DATABASENAE% Begin to get db path."
rem *********************get path*******************************
%OSQLPATH% %YZFS% -d %DATABASENAE% -q %SelectPath% -i%EXITFILE% | findstr /i ":\\" > %TMPFILE%

if !errorlevel! EQU 0 (
    set N=0
    set SkipNum=1
    set Count=0

    rem *********************get disk*******************************
    set N=0
    for /f "tokens=1 delims=:" %%a in ('type %TMPFILE%') do (
        set tmpmsg=%%a
        set tmpmsg=!tmpmsg: =!
            set /a N += 1
        if !N! equ 1 (
                echo !tmpmsg! > %PATHFILE%
        ) else (
            echo !tmpmsg! >> %PATHFILE%
        )
    )
    
    rem *********************get unified disk*******************************
    set N=0
    for /f "tokens=1 delims=" %%a in ('type %PATHFILE%') do (
        set tmpmsg=%%a
        set tmpmsg=!tmpmsg: =!
            set /a N += 1
        if !N! equ 1 (
            echo !tmpmsg! > %TMPFILE%
        ) else (
            set IsHave=0
            for /f "tokens=1 delims=" %%i in ('type %TMPFILE%') do (
                set panfu=%%i
                set panfu=!panfu: =!
                if !panfu!==!tmpmsg! (
                    set IsHave=1
                )
            )

            if !IsHave! equ 0 (
                echo !tmpmsg! >> %TMPFILE%
            )
        )
    )

    rem *********************write result*******************************
    set N=0
    for /f "tokens=1 delims=" %%i in ('type %TMPFILE%') do (
        set tmpmsg=%%i
        set tmpmsg=!tmpmsg: =!
            set /a N += 1

        if !N! equ 1 (
            set FilePath=!tmpmsg!
        ) else (
            set FilePath=!FilePath!;!tmpmsg!
        )
    )
    echo !FilePath!>%RST%
    call :Log "The Path of %DATABASENAE% is !FilePath!."

) else (
    call :Log "Adaptive LUN error."
)

call :Log "DBNAME:%DATABASENAE% End to get db path."

:END
if exist %TMPFILE% (del /f /q %TMPFILE%)
if exist %EXITFILE% (del /f /q %EXITFILE%)
if exist %PATHFILE% (del /f /q %PATHFILE%)
if exist %ARGFILE% (del /f /q %ARGFILE%)
exit %ExitValues%

::Print log function, controled by "NEEDLOGFLG".
:Log
    echo %date:~0,10% %time:~0,8% [%username%] %~1 >> %LOGFILENAME%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILENAME%
goto :eof


:GetValue
    if exist %ARGFILE% (del /f /q %ARGFILE%)
    set ArgInput=%~1
    set ArgName=%~2
    set ArgValue=--
    for /l %%a in (1 1 20) do call :Separate %%a

    for /f "tokens=1,2 delims==" %%i in ('type %ARGFILE%') do (
        if %%i==%ArgName% (
            set ArgValue=%%j
        )
    )
    if exist %ARGFILE% (del /f /q %ARGFILE%)
goto :EOF

:Separate
    for /f "tokens=%1 delims=;" %%i in ("%ArgInput%") do (
        echo %%i>> %ARGFILE%
    )

goto :EOF

rem #############################################################
rem osql to database to obtain the status of SqlServer function.
rem #############################################################
:GetDBStatus
set DataBaseName=%~1
echo sp_helpdb [!DataBaseName!] > %DBSTATUSSQL%
if !CLUSTERFLAG!==0 (
    if %SubAppName% == %DEFAULTINSTANCENAME% (
        !OSQLPATH! -b -U %UserNam% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
    ) else (
        !OSQLPATH! -b -U %UserNam% -S %SERVERNAME%\%SubAppName% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
    )
) else (
    if %SubAppName% == %DEFAULTINSTANCENAME% (
        !OSQLPATH! -b -U %UserNam% -S %CLUSTERNAME% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
    ) else (
        !OSQLPATH! -b -U %UserNam% -S %CLUSTERNAME%\%SubAppName% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
    )
)

if !errorlevel! EQU 0 (
    set /a %~2=1
    type %SQLSERVERDBINFO% |findstr /i "Status=" > %SQLSERVERDBSTATUS%
    if !errorlevel! EQU 0 (
        for /f "delims=," %%a in ('type %SQLSERVERDBSTATUS%') do (
            for /f "tokens=2 delims==" %%b in ("%%a") do (
                if %%b==ONLINE (
                    set /a %~2=0
                    call :Log "Query db[!DataBaseName!] of %SubAppName% instance status:%%b, RetStatus[0]."
                ) else (
                    set /a %~2=1
                    call :Log "Query db[!DataBaseName!] of %SubAppName% instance status:%%b, RetStatus[1]."
                )
            )
        )
    ) else (
        set /a %~2=1
        call :Log "Get database !DataBaseName! of !SERVERNAME! instance status not ONLINE, RetStatus[1]."
    )
    set /a %~3=0
) else (
    set /a %~3=1
    call :Log "osql -E -S %SERVERNAME% -q sp_helpdb !DataBaseName! failed."
)

if exist %SQLSERVERDBSTATUS% (del /f /q %SQLSERVERDBSTATUS%)
if exist %SQLSERVERDBINFO% (del /f /q %SQLSERVERDBINFO%)
if exist %DBSTATUSSQL% (del /f /q %DBSTATUSSQL%)
goto :EOF 

rem #############################################################
rem osql to database to obtain the status of SqlServer function.
rem #############################################################
:CheckUserPassword
set DataBaseName=%~1
echo exit > %DBSTATUSSQL%
if %CLUSTERFLAG%==0 (
    if %SubAppName% == %DEFAULTINSTANCENAME% (
        !OSQLPATH! -b -U%UserNam% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
    ) else (
        !OSQLPATH! -b -U%UserNam% -S %SERVERNAME%\%SubAppName% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
    )
) else (
    if "%CLUSTERNAME%"=="" (
        call :Log "DBNAME:%DATABASENAE% cluster network name is Null"
        if exist %EXITFILE% (del /f /q %EXITFILE%)
        if exist %DBSTATUSSQL% (del /f /q %DBSTATUSSQL%)
        exit %ERROR_SCRIPT_COMMON_INSTANCE_NOSTART%
    )
    if %SubAppName% == %DEFAULTINSTANCENAME% (
        !OSQLPATH! -b -U%UserNam% -S %CLUSTERNAME% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
    ) else (
        !OSQLPATH! -b -U%UserNam% -S %CLUSTERNAME%\%SubAppName% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
    )
)

type %SQLSERVERDBINFO% | findstr "[xFFFFFFFF]" > nul
if !errorlevel! EQU 0 (
    call :Log "Instance %SubAppName% is not start. return str contents [xFFFFFFFF]"
    set /a %~2=%ERROR_SCRIPT_COMMON_INSTANCE_NOSTART%
) else (
    rem Login failed for user 'xxx'.
    type %SQLSERVERDBINFO% | findstr /i -c:"'%UserNam%'" > nul
    if !errorlevel! EQU 0 (
        rem user password error
        call :Log "Username %UserNam% or password is wrong."
        set /a %~2=%ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG%
    ) else (
        set /a %~2=0
    )
) 

if exist %SQLSERVERDBINFO% (
    del /f /q %SQLSERVERDBINFO%
) else (
    rem success
    call :Log "Instance %SubAppName% Login sucessful"
    set /a %~2=0
)
if exist %DBSTATUSSQL% (del /f /q %DBSTATUSSQL%)
goto :eof

rem *************************Isinstance exist***********************************
:Checkinstance
if %SubAppName% == %DEFAULTINSTANCENAME% (
    sc query state= all | findstr -c:"SERVICE_NAME: MSSQLSERVER" > %InstanceListTmp%
    if !errorlevel! equ 0 (
        call :Log "DBNAME:%DATABASENAE% Begin to check DEFAULTINSTANCENAME."
    ) else (
        call :log "DEFAULTINSTANCENAME %SubAppName% is not-exist!"
        if exist %InstanceListTmp% del /f /q %InstanceListTmp%
        exit %ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST%
    )
) else (
    sc query state= all | findstr -c:"SERVICE_NAME: MSSQL$" > %InstanceListTmp%
    if !errorlevel! equ 0 (
        for /f "tokens=1,2 delims=$" %%i in ('type %InstanceListTmp%') do (
            echo %%j>> %InstanceList%
        )
        for /f "tokens=1,2 delims=$" %%i in ('type %InstanceList%') do (
            if %%i == %SubAppName% (
                if exist %InstanceListTmp% del /f /q %InstanceListTmp%
                if exist %InstanceList% del /f /q %InstanceList%
                goto end;
            )
        )
        call :log "InstanceList %SubAppName% is not-exist!"
        if exist %InstanceListTmp% del /f /q %InstanceListTmp%
        if exist %InstanceList% del /f /q %InstanceList%
        exit %ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST%
    ) else (
        call :log "%SubAppName% is not-exist!"
        if exist %InstanceListTmp% del /f /q %InstanceListTmp%
        exit %ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST%
    )
)

if exist %InstanceListTmp% del /f /q %InstanceListTmp%
call :Log "Isinstance end."

goto end;
rem ###########################################################################
rem get Cluster Name; We Can Use DB Instance;
rem ###########################################################################
:GetDBClusterName
powershell "get-clusterResource | ? {$_.ResourceType -eq \"SQL Server\"} | get-clusterparameter | select Name, Value | ? {$_.Name -eq \"VirtualServerName\" -or $_.Name -eq \"InstanceName\"}" > %DBSqlService%
if !errorlevel! equ 0 (
    for /f "tokens=1,* delims= " %%i in ('type %DBSqlService%') do (
        set title=%%i
        if "!title!" == "VirtualServerName" (
            set CLUSTERNAME=%%j
            set CLUSTERNAME=!CLUSTERNAME: =!
        )
        
        if "!title!" == "InstanceName" (
            set DBInstanceName=%%j
            set DBInstanceName=!DBInstanceName: =!
            if "!DBInstanceName!" == "%SubAppName%" (
                if exist %DBSqlService% (del /f /q %DBSqlService%)
                goto end;
            )
        )
    )
    set CLUSTERNAME=
    call :Log "DBNAME:%DATABASENAE% Get Cluster Name Error, DBInstance: !SubAppName!"
    for /f "delims=," %%a in ('type %DBSqlService%') do (
        call :log "DBNAME:%DATABASENAE% DBSqlService:%%a"
    )
) else (
    call :Log "Exec Get Cluster Name Error, DBInstance: !SubAppName!"
)

if exist %DBSqlService% (del /f /q %DBSqlService%)
:end
goto :eof

endlocal