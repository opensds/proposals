@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


rem ***************************************************************************************
rem program name:          SqlserverRecover.bat     
rem function:              start or stop sql server
rem author:                 
rem time:                  2012-3-28
rem function and description:  
rem function              description
rem GetTreadId            Get the specified value from input parameter
rem                            parameter: %~1¡¢%~2   result£ºArgValue
rem GetValue              Get the specified value from input parameter
rem                            parameter£º%~1¡¢%~2   result£ºArgValue
rem Separate              Separate the input parameter
rem                            parameter:serial number; result:ARGFILENAME
rem WaitService           Waitting until the status of database is running
rem Log                   Print log function, controled by "NEEDLOGFLG"
rem                            parameter:record log;
rem DeleteFile            Delete file function
rem                            parameter:deleted file;
rem rework:               First Programming
rem author:
rem time:
rem explain:
rem ***************************************************************************************

set NEEDLOGFLG=1

set AGENT_ROOT=%~1
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\

set RESULTID=%2
set HOSTNAME=
set DEFAULTINSTANCENAME=MSSQLSERVER
set DBClusterName=

rem query intervals time(units:10millisecond)
set /a QUERYTIME=200
set /a RETRYQUERYTIME=3000

rem cycle times
set /a CYCLETIME=60

cd /d %~dp0

rem ************ErrorCode*****************
set /a OPERATION_RTN_SUCCESS=0
set /a ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG=10
set /a ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG=15
set /a ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR=130
set /a ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED=131
set /a ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED=134
set /a ERROR_SCRIPT_SQLSERVER_DB_OFFLINE_OR_NOT_EXIST=135
set /a ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST=136
set /a ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST=137
set /a ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED=138
set /a ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED=139
set /a ERROR_SCRIPT_COMMON_INSTANCE_NOSTART=11

rem **************************Set file name and path***************************************
set ARGFILENAME="%AGENT_TMP_PATH%ArgFile%RESULTID%.txt"
set DBSERVICESTATUSFILE="%AGENT_TMP_PATH%DbSCStatusFile%RESULTID%.txt"
set DBSTATUSRSTFILE="%AGENT_TMP_PATH%DBSTATUSFILE%RESULTID%.txt"
set DBSTATUSRSTFILETMP="%AGENT_TMP_PATH%DBSTATUSFILETMP%RESULTID%.txt"
set SQLTESTFILE="%AGENT_TMP_PATH%SQLTESTFILE%RESULTID%.sql"
set SQLONLINEFILE="%AGENT_TMP_PATH%SQLONLINEFILE%RESULTID%.sql"
set SQLOFFLINFILE="%AGENT_TMP_PATH%SQLOFFLINEFILE%RESULTID%.sql"
set DBSTATUSSQL="%AGENT_TMP_PATH%DBSatus%RESULTID%.sql"
set SQLSERVERDBINFO="%AGENT_TMP_PATH%GetDBInfoFile%RESULTID%.txt"
set SQLGETSTATUSFILE="%AGENT_TMP_PATH%SQLGETSTATUSFILE%RESULTID%.sql"
set LOGFILEPATH="%AGENT_LOG_PATH%sqlserverrecover.log"
set ONRESULTFILE="%AGENT_TMP_PATH%ONRESULTFILE%RESULTID%.txt"
set DBSqlService="%AGENT_TMP_PATH%DBSqlService%RESULTID%.txt"
set DBCMDRESULT="%AGENT_TMP_PATH%DBCMDRESULT%RESULTID%.txt"
set ERRORTMPINFO="%AGENT_TMP_PATH%errortmpinfo%RESULTID%.txt"
set InstanceListTmp="%AGENT_TMP_PATH%InstanceListTmp%RESULTID%.txt"
set InstanceList="%AGENT_TMP_PATH%InstanceList%RESULTID%.txt"

set TMPHOSTFILE="%AGENT_TMP_PATH%SqlserverHostName%RESULTID%.txt"
echo localhost>%TMPHOSTFILE%
set /p HOSTNAME=<%TMPHOSTFILE%
if exist %TMPHOSTFILE% (del /f /q %TMPHOSTFILE%) 
rem ****************************************get host name**********************************************

set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%RESULTID%"
set INPUTINFO=
for /f "delims=" %%a in ('type %PARAM_FILE%') do (
    if not "%%a" == "" (
        set INPUTINFO=%%a
    )
)
if exist %PARAM_FILE% (del /f /q %PARAM_FILE%) 

setlocal DisableDelayedExpansion
rem *****************************Get dbname username and password**************************
call :GetValue "%INPUTINFO%" INSTNAME
set INSTANCENAME=%ArgValue%

call :GetValue "%INPUTINFO%" DBNAME
set DBNAME=%ArgValue%

call :GetValue "%INPUTINFO%" DBUSERNAME
set DBUSER=%ArgValue%

call :GetValue "%INPUTINFO%" DBPASSWORD
set OSQLPASSWORD=%ArgValue%

call :GetValue "%INPUTINFO%" ISCLUSTER
set CLUSTERFLAG=%ArgValue%

setlocal EnableDelayedExpansion

call :GetValue "%INPUTINFO%" CHECKTYPE
set CHECKTYPE=%ArgValue%

call :Log "INSTANCENAME=%INSTANCENAME%;DBNAME=%DBNAME%;DBUSER=%DBUSER%;HOSTNAME=%HOSTNAME%;CHECKTYPE=%CHECKTYPE%;CLUSTERFLAG=%CLUSTERFLAG%;"

rem try to find osql cmd;
set /a OSQLFindFlag=0
set MinSqlServerVerNumber=9
set MaxSqlServerVerNumber=13
call :GETOSQLCMDPATH OSQLPath OSQLFindFlag
if not %OSQLFindFlag% equ 0 (
    call :Log "Cann't fined tool:osql"
    DeleteAllFile
    exit %ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED%
)
set osqlexe="%OSQLPath%"

rem **********get service name***************
if %INSTANCENAME% EQU %DEFAULTINSTANCENAME% (
    set SERVICENAME=%INSTANCENAME%
    set SERVERNAME=
) else (
    set SERVICENAME=MSSQL$%INSTANCENAME%
    set SERVERNAME=%INSTANCENAME%
)

call :Log "SERVERNAME=%SERVERNAME%;SERVICENAME=%SERVICENAME%"

echo exit > %SQLTESTFILE%

rem Cluster Opt;
if %ClusterFlag% equ 1 (
    call :GetDBClusterName
)
rem check instance
call :Checkinstance
rem **********************check user name  and password********************** 
call :CheckUserPassword RetStatus
if !RetStatus! neq 0 (
    call :DeleteAllFile
    exit !RetStatus!
)

rem **********************set database offline*******************************
if %CHECKTYPE% EQU 0 (
    ::*******************check service if start 0:start 1:stop********************
    call :CheckServiceIfStart %SERVICENAME% RetCode
    if !RetCode! EQU 1 (
        call :Log "Service %SERVICENAME% is not started."
        call :DeleteAllFile
        exit 0
    ) else (
        ::**********************create query database status sql file******************
        call :CreateDBStatusSql %SQLGETSTATUSFILE%
        ::**********************get database status file*************************
        call :GetDBStatusFile RetCode
        if !RetCode! NEQ 0 (
            call :DeleteAllFile
            exit !RetCode!
        ) else (   
            for /l %%a in (1 1 32) do (
                ::*******************set database offline process****************************
                call :SetDBOffLinePro %%a RetCode
                if !RetCode! NEQ 0 (
                    call :Log "Set database offline failed."
                    call :DeleteAllFile
                    exit !RetCode!
                ) else (
                    call :Log "Set database offline successful."
                    call :DeleteAllFile     
                    exit 0
                )
            )
        )
    )
) else (
    ::************************set database online****************************
    ::*******************check service if start 0:start 1:stop***************
    call :CheckServiceIfStart %SERVICENAME% RetCode
    if !RetCode! NEQ 0 (
        call :StartServicePro %%a RetCode
        if !RetCode! NEQ 0 (
            call :Log "Start sqlserver service %SERVICENAME% failed."
            call :DeleteAllFile
            exit !RetCode!
        )
    )
    
    ::**********************create query database status sql file******************
    call :CreateDBStatusSql %SQLGETSTATUSFILE%
    
    ::**********************get database status file*************************
    call :GetDBStatusFile RetCode
    if !RetCode! NEQ 0 (
        call :DeleteAllFile
        exit !RetCode!
    ) else (
        for /l %%a in (1 1 32) do (
            call :TestDBPro RetCode %%a
            if !RetCode! EQU 0 (
                call :DeleteAllFile
                exit 0
            ) else (
                call :DeleteAllFile
                exit !RetCode!
            ) 
        )
    ) 
)

rem ************************************************************************
rem function name: TestDBPro
rem aim:           set database online process after database services has been started
rem input:         seprate number (eg:1,2,3)
rem output:        0,successful;1,failed
rem ************************************************************************
:TestDBPro
    ::query database status 0,offline;1,online;2,fail
    call :CheckDBStatus %~2 TestDBProRetCode CheckDBName
    if !TestDBProRetCode! EQU 3 (
        set /a %~1 = %ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST%
    )
    if !TestDBProRetCode! EQU 4 (
        set /a %~1 = %ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR%
    )
    if !TestDBProRetCode! EQU 1 (
        ::**************test database if can connect*********************
        call :TestDB %~2 %~1
    ) 
    if !TestDBProRetCode! EQU 0 (
        call :Log "Database !CheckDBName! is offline."
        call :ExecOnLine %~2 ExecOnLineRetCode
        if !ExecOnLineRetCode! EQU 0 (
            ::**************test database if can connect*********************
            call :TestDB %~2 %~1
        ) else (
            set /a %~1 = !ExecOnLineRetCode!
        )
     )
     if !TestDBProRetCode! EQU 2 (
         call :Log "Database !CheckDBName! is RECOVERY_PENDING."
         call :ExecOffLine %~2 ExecOffRetCode
         call :ExecOnLine %~2 ExecOnRetCode
         if !ExecOnRetCode! EQU 0 (
             ::**************test database if can connect*********************
            call :TestDB %~2 %~1
         ) else (
            set /a %~1 = !ExecOnRetCode!
         )
     )        

goto :EOF

rem ************************************************************************
rem function name: StartServicePro
rem aim:           set database online process after database services has been not started
rem input:         seprate number (eg:1,2,3)
rem output:        0,successful;1,failed
rem ************************************************************************
:StartServicePro
    sc start %SERVICENAME% > nul
    if !errorlever! NEQ 0 (
        call :Log "Execute sc start %SERVICENAME% command failed."
        set /a %~2 = %ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED%
    ) else (
        set /a DelayCount=0
        call :WaitService !QUERYTIME! !CYCLETIME! %SERVICENAME%
        if !EXITVALUE! EQU 1 (
            set /a %~2 = %ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED%
        ) else (
            call :Log "Execute sc start %SERVICENAME% command successful."
            set /a %~2 = %OPERATION_RTN_SUCCESS%
        )
    )
goto :EOF

:EXITFLAG
    set EXITVALUE = 1
goto :EOF

rem ************************************************************************
rem function name: SetDBOffLinePro
rem aim:           set database offline process
rem input:         seprate number (eg:1,2,3)
rem output:        0,successful;1,failed
rem ************************************************************************
:SetDBOffLinePro
    ::query database status 0,offline;1,online;other,fail
    call :CheckDBStatus %~1 RetCode CheckDBName
    if !RetCode! EQU 3 (
        set /a %~2 = %ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST%
        call :Log "The database is not exist."
    ) 
    if !RetCode! EQU 4 (
        set /a %~2 = %ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR%
    )
    if !RetCode! EQU 0 (
        call :Log "The status of database !CheckDBName! is offline."
        set /a %~2 = 0
    ) 
    if !RetCode! EQU 1 (
        call :ExecOffLine %~1 %~2
    )
    if !RetCode! EQU 2 (
        call :Log "Database !CheckDBName! is RECOVERY_PENDING."
        call :ExecOffLine %~1 %~2
    )
goto :EOF

rem ************************************************************************
rem function name: ExecOffLine
rem aim:           set database offline
rem input:         separate number
rem output:        0,successful;1,failed
rem ************************************************************************
:ExecOffLine
    for /f "tokens=%~1 delims=," %%i in ("%DBNAME%") do (
        call :CreateSetOffLineSql %%i %SQLOFFLINFILE%
        if %ClusterFlag% equ 0 (
            call :Log "%osqlexe% -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -d%%i -i SQLOFFLINEFILE%RESULTID%.sql"
            %osqlexe% -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -i %SQLOFFLINFILE% > %DBCMDRESULT%
        ) else (
            %osqlexe% -b -U %DBUSER% -P %OSQLPASSWORD% -S%DBClusterName%\%SERVERNAME% -i %SQLOFFLINFILE% > %DBCMDRESULT%
        )
        
        if !errorlevel! NEQ 0 (
            type %DBCMDRESULT% |findstr "5011" > nul
            if !errorlevel! EQU 0 (
                set /a %~2=%ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG%
                call :Log "User does not have permission to alter database."
                goto :EOF
            )
            
            call :GetDBStatusFile FileRetCode
            if !FileRetCode! NEQ 0 (
                call :DeleteAllFile
                set /a %~2 = !FileRetCode!
                call :Log "Execute set database %%i offline failed." 
            ) else (
                call :CheckDBStatus %~1 CheckStatusRetCode
                if !CheckStatusRetCode! EQU 0 (
                    set /a %~2 = %OPERATION_RTN_SUCCESS%
                    call :Log "Execute set database %%i offline successful."
                ) else (
                    set /a %~2 = !CheckStatusRetCode!
                    call :Log "Execute set database %%i offline failed." 
                )
            )
        ) else (
            set /a %~2 = %OPERATION_RTN_SUCCESS%
            call :Log "Execute set database %%i offline successful."
        )
        call :DeleteFile %DBCMDRESULT%
    )
goto :EOF

rem ************************************************************************
rem function name: RetryOnLine
rem aim:           set database online retry
rem input:         separate number
rem output:        0,successful;1,failed
rem ************************************************************************
:RetryOnLine
    set RETRYDBNAME=%~1
    set /a RETRYCOUNT+=1
    if !RETRYCOUNT! GEQ 4 (
        call :Log "Execute set database !RETRYDBNAME! online failed."
        set /a %~2=%ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR%
    ) else (
        call :ProcDelay %RETRYQUERYTIME%
        
        if %ClusterFlag% equ 0 (
            %osqlexe% -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -i %SQLONLINEFILE% -o %ONRESULTFILE%> nul
        ) else (
            %osqlexe% -b -U%DBUSER% -P%OSQLPASSWORD% -S%DBClusterName%\%SERVERNAME% -i %SQLONLINEFILE% -o %ONRESULTFILE%> nul
        )
        if !errorlevel! NEQ 0 (
            call :Log "Retry count is %RETRYCOUNT%"
            call :DeleteFile %ONRESULTFILE%
            goto :RetryOnLine %~1 %~2
        ) else (
            set /a %~2=%OPERATION_RTN_SUCCESS%
            call :CheckCmdResult %ONRESULTFILE% %~2
            if %~2 EQU 0 (
                call :Log "Execute set database !RETRYDBNAME! online successful."
            )
            
        )
        
    )
goto :EOF    

rem ************************************************************************
rem function name: ExecOnLine
rem aim:           set database online
rem input:         separate number
rem output:        0,successful;1,failed
rem ************************************************************************
:ExecOnLine
    for /f "tokens=%~1 delims=," %%i in ("%DBNAME%") do (
        set /a RETRYCOUNT=0    
        call :CreateSetOnLineSql %%i %SQLONLINEFILE%
        if %ClusterFlag% equ 0 (
            call :Log "%osqlexe%  -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -i SQLONLINEFILE%RESULTID%.sql"
            %osqlexe% -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -i %SQLONLINEFILE% -o %ONRESULTFILE%> nul
        ) else (
            %osqlexe% -b -U%DBUSER% -P%OSQLPASSWORD% -S%DBClusterName%\%SERVERNAME% -i %SQLONLINEFILE% -o %ONRESULTFILE%> nul            
        )

        if !errorlevel! NEQ 0 (
            call :Log "Execute set database %%i online failed, start to retry." 
            call :RetryOnLine %%i %~2
        ) else (
            type %ONRESULTFILE% | findstr "5011 5069"
            if !errorlevel! EQU 0 (
                call :Log "Execute set database %%i online failed, start to retry." 
                call :RetryOnLine %%i retryCode
                set /a %~2 = !retryCode!
            ) else (
                set /a %~2 = 0
                call :Log "Execute set database %%i online successful." 
            )            
        )
    )
goto :EOF

rem *****************************startup database**************************
if %INSTANCENAME% EQU %DEFAULTINSTANCENAME% (
        set SERVICENAME=%INSTANCENAME%
) else (
        set SERVICENAME=MSSQL$%INSTANCENAME%
)

rem ************************************************************************
rem function name: CheckServiceIfStart
rem aim:           check sqlserver service if start
rem input:         service name
rem output:        0,start;1,not start
rem ************************************************************************
:CheckServiceIfStart
    sc query %~1 | findstr /i "STATE" > %DBSERVICESTATUSFILE%
    set /p DBSERVICESTATUSTMP= < %DBSERVICESTATUSFILE%
    if "!DBSERVICESTATUSTMP!" == "" (
        call :DeleteFile %DBSERVICESTATUSFILE%
        call :Log "Service %~1 is not exist."
        call :DeleteAllFile
        exit %ERROR_SCRIPT_COMMON_INSTANCE_NOSTART%
    )
    
    for /f "tokens=2 delims=:" %%i in ("%DBSERVICESTATUSTMP%") do (
        echo %%i > %DBSERVICESTATUSFILE%
        for /f "tokens=1 delims= " %%j in ('type %DBSERVICESTATUSFILE%') do (
            if %%j EQU 4 (
                set /a %~2 = 0
            ) else (
                set /a %~2 = 1
            )
        )
    )
    
goto :EOF

rem ************************************************************************
rem function name: GetDBStatusFile
rem aim:           get database status files
rem input:         
rem output:        0,successful;other,fail
rem ************************************************************************
:GetDBStatusFile
    if %CLUSTERFLAG% equ 0 (
        echo %osqlexe% -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -i %SQLGETSTATUSFILE% -o %DBSTATUSRSTFILETMP% -w 300 > %ERRORTMPINFO%
        
        %osqlexe% -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -i %SQLGETSTATUSFILE% -o %DBSTATUSRSTFILETMP% -w 300 >> %ERRORTMPINFO%
    ) else (
        echo %osqlexe% -b -U %DBUSER% -P %OSQLPASSWORD% -S%DBClusterName%\%SERVERNAME% -i %SQLGETSTATUSFILE% -o %DBSTATUSRSTFILETMP% -w 300 > %ERRORTMPINFO%
        
        %osqlexe% -b -U %DBUSER% -P %OSQLPASSWORD% -S%DBClusterName%\%SERVERNAME% -i %SQLGETSTATUSFILE% -o %DBSTATUSRSTFILETMP% -w 300 >> %ERRORTMPINFO%
    )
    
    if !errorlevel! NEQ 0 (
        set /a %~1 = %ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED%
        call :Log "Query database status failed."
        more %ERRORTMPINFO% >> %LOGFILEPATH%
    ) else (
        set /a %~1 = 0
        call :Log "Query database status successful."
    )
    
    if exist %ERRORTMPINFO% del /f /q %ERRORTMPINFO%
    
    ::**********************delete space line**************************************
    for /f "tokens=*" %%a in ('type %DBSTATUSRSTFILETMP%') do (
        if not "%%a" == "" (
            echo %%a >> %DBSTATUSRSTFILE%
        )
    )    
    call :CheckCmdResult %DBSTATUSRSTFILE% %~1
goto :EOF

rem ************************************************************************
rem function name: CheckDBStatus
rem aim:           check database status
rem input:         parmeter 1: seprate number(eg:1,2,3)
rem output:        0,offline;1,online;2,RECOVERY_PENDING;3,no find;4,error
rem ************************************************************************
:CheckDBStatus
    set /a FINDFLAG=0
    set /a %~2 = 4
    for /f "tokens=%~1 delims=," %%i in ("%DBNAME%") do (
        set DBNAMETMPTMP=%%i
        set DBNAMETMP=!DBNAMETMPTMP: =!
        
        for /f "tokens=1,2 skip=2 delims= " %%i in ('type %DBSTATUSRSTFILE%') do (
            if "!DBNAMETMP!" == "%%i" (
                set /a FINDFLAG+=1
                set %~3=!DBNAMETMP!

                set STATUSFLG=%%j
                if "!STATUSFLG!" == "ONLINE" (
                    set /a %~2 = 1                        
                )                    
                if "!STATUSFLG!" == "OFFLINE" (
                    set /a %~2 = 0
                )
                if "!STATUSFLG!" == "RECOVERY_PENDING" (
                    set /a %~2 = 2
                )
            )
        )
    )
    
    if !FINDFLAG! EQU 0 (
        set /a %~2 = 3
    )

goto :EOF
    
rem ************************************************************************
rem function name: CreateDBStatusSql
rem aim:           create query database status sql file
rem input:         the sql script name to execute
rem output:        SQLGETSTATUSFILE
rem ************************************************************************
:CreateDBStatusSql
    echo select name, state_desc from sys.databases > "%~1"
    echo go >> "%~1"
    echo exit >> "%~1"
goto :EOF

rem ************************************************************************
rem function name: CreateSetOnLineSql
rem aim:           create set database online sql file
rem input:         the sql script name to execute
rem output:        SQLONLINEFILE
rem ************************************************************************
:CreateSetOnLineSql
    echo alter database [%~1] set online > "%~2"
    echo go >> "%~2"
    echo exit >> "%~2"
goto :EOF

rem ************************************************************************
rem function name: CreateSetOffLineSql
rem aim:           create set database offline sql file
rem input:         the sql script name to execute
rem output:        SQLOFFLINEFILE
rem ************************************************************************
:CreateSetOffLineSql
    echo alter database [%~1] set offline > "%~2"
    echo go >> "%~2"
    echo exit >> "%~2"
goto :EOF

rem ************************************************************************
rem function name: WaitService
rem aim:           Waitting until the status of database is running
rem input:         
rem output:        
rem ************************************************************************
:WaitService
    if !DelayCount! GEQ %~2 (
        call :DeleteFile %DBSERVICESTATUSFILE%
          call :Log "Start the database %SERVICENAME% service failed."
          goto :EXITFLAG
    )

sc query %~3 | findstr /i "STATE" > %DBSERVICESTATUSFILE%
set /p DBSERVICESTATUSTMP= < %DBSERVICESTATUSFILE%
if "!DBSERVICESTATUSTMP!" == "" (
    call :Log "Service %~3 is not existent."
    goto :EXITFLAG
)

for /f "tokens=2 delims=:" %%i in ('sc query %~3 ^| findstr /i "STATE"') do (
    echo %%i > %DBSERVICESTATUSFILE%
    for /f "tokens=1 delims= " %%j in ('type %DBSERVICESTATUSFILE%') do (                
        if %%j NEQ 4 (
            set /a DelayCount+=1
            call :ProcDelay %~1
            goto :WaitService    %~1 %~2 %~3
        )
    )
)    
goto :EOF

rem ************************************************************************
rem function name: ProcDelay
rem aim:           Sleep 
rem input:         
rem output:        
rem ************************************************************************
:ProcDelay delayMSec_
    FOR /f "tokens=1-4 delims=:. " %%h IN ("%TIME%") DO SET start_=%%h%%i%%j%%k
:_procwaitloop
    FOR /f "tokens=1-4 delims=:. " %%h IN ("%TIME%") DO SET now_=%%h%%i%%j%%k
    SET /a diff_=%now_%-%start_%
    IF %diff_% LSS %1 GOTO _procwaitloop
    ENDLOCAL & GOTO :EOF
:EOF

rem ************************************************************************
rem function name: GetValue
rem aim:           Get the specified value from input parameter
rem input:         $2 and ArgName
rem output:        ArgValue
rem ************************************************************************
:GetValue
    call :DeleteFile %ARGFILENAME%
    set ArgInput=%~1
    set ArgName=%~2

    for /l %%a in (1 1 20) do call :Separate %%a

    for /f "tokens=1,2 delims==" %%i in ('type %ARGFILENAME%') do (
        if %%i==%ArgName% (
            set ArgValue=%%j
        )
    )

    call :DeleteFile %ARGFILENAME%

goto :EOF

rem ************************************************************************
rem function name: CheckCmdResult
rem aim:           Check command result
rem input:         cmdresultfile, errorcode
rem output:        errorcode
rem ************************************************************************
:CheckCmdResult
set TMPCmdResultFile="%~1"
rem 5011:User does not have permission to alter database
type %TMPCmdResultFile% |findstr "5011" > nul
if !errorlevel! EQU 0 (
    rem have no priv when stop db
    set /a %~2=%ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG%
    call :Log "User does not have permission to alter database."
)
goto :eof

rem #############################################################
rem osql to database to obtain the status of SqlServer function.
rem errorcode
rem 136: get clusterworkname failed or instance is not exist
rem 10: username or password wrong
rem 0: success
rem #############################################################
:CheckUserPassword
echo exit > %DBSTATUSSQL%
if !CLUSTERFLAG!==0 (
    %osqlexe% -b -U%DBUSER% -S %HOSTNAME%\%SERVERNAME% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
) else (
    if "%DBClusterName%"=="" (
        call :Log "cluster network name is Null"
        call :DeleteAllFile
        exit %ERROR_SCRIPT_COMMON_INSTANCE_NOSTART%
    )
    %osqlexe% -b -U%DBUSER% -S %DBClusterName%\%SERVERNAME% -i%DBSTATUSSQL% > %SQLSERVERDBINFO%
)
rem Cannot open database "dbd" requested by the login. The login failed.
type %SQLSERVERDBINFO% |findstr "[xFFFFFFFF]" > nul
if !errorlevel! EQU 0 (
    call :Log "instance is not start. return str contents [xFFFFFFFF]"
    set /a %~1=%ERROR_SCRIPT_COMMON_INSTANCE_NOSTART%
) else (
    rem Login failed for user 'xxx'.
    type %SQLSERVERDBINFO% |findstr "'%DBUSER%'" > nul
    if !errorlevel! EQU 0 (
        rem user password error
        call :Log "username or password is wrong."
        set /a %~1=%ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG%
    ) else (
        set /a %~1=%OPERATION_RTN_SUCCESS%
    )
) 
call :DeleteFile %SQLSERVERDBINFO%
call :DeleteFile %DBSTATUSSQL%
goto :eof

rem ************************************************************************
rem function name: Separate
rem aim:           Separate the input parameter
rem input:         parameter serial number
rem output:        ARGFILENAME
rem ************************************************************************
:Separate
    setlocal DisableDelayedExpansion
    for /f "tokens=%~1 delims=;" %%i in ("%ArgInput%") do (
        echo %%i>> %ARGFILENAME%
    )
    endlocal
goto :EOF

rem ************************************************************************
rem function name: Log
rem aim:           Print log function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    if %NEEDLOGFLG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILEPATH%
    )
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
rem function name: TestDb
rem aim:           Test Database Connection
rem input:         the deleted file
rem output:        
rem ************************************************************************
:TestDb
rem for SQLSERVER2005
if "%DBUSER%" EQU " " (
    for /f "tokens=%1 delims=," %%i in ("%DBNAME%") do (
        if %ClusterFlag% equ 0 (
            call :Log "%osqlexe% -b -E -S%HOSTNAME%\%SERVERNAME% -d%%i -i SQLTESTFILE%RESULTID%.sql"
            %osqlexe% -b -E -S%HOSTNAME%\%SERVERNAME% -d%%i -i %SQLTESTFILE% > %DBCMDRESULT%
        ) else (
            %osqlexe% -b -U%DBUSER% -P%OSQLPASSWORD% -S%DBClusterName%\%SERVERNAME% -d%%i -i %SQLTESTFILE% > %DBCMDRESULT%
        )

        if !errorlevel! NEQ 0 (
            set /a %~2 = %ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR%
            call :Log "Test to connect %%i failed."
        ) else (
            set /a %~2 = %OPERATION_RTN_SUCCESS%
            call :Log "Test to connect %%i succeeded."
        )
        
    )
) else (
    rem for SQLSERVER2000
    for /f "tokens=%1 delims=," %%i in ("%DBNAME%") do (
        if %ClusterFlag%==0 (
            call :Log "%osqlexe% -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -d%%i -i SQLTESTFILE%RESULTID%.sql"
            %osqlexe% -b -S%HOSTNAME%\%SERVERNAME% -U%DBUSER% -d%%i -i %SQLTESTFILE% > %DBCMDRESULT%    
        ) else (
            %osqlexe% -b -S%DBClusterName%\%SERVERNAME% -U%DBUSER% -P%OSQLPASSWORD% -d%%i -i %SQLTESTFILE% > %DBCMDRESULT%    
        ) 

        if !errorlevel! NEQ 0 (
            set /a %~2 = %ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR%
            call :Log "Test to connect %%i by user %DBUSER% failed."
        ) else (
            set /a %~2 = %OPERATION_RTN_SUCCESS%
            call :Log "Test to connect %%i by user %DBUSER% succeeded."
        )
    )
)
call :CheckCmdResult %DBCMDRESULT% %~2
call :DeleteFile %DBCMDRESULT%
goto :EOF

rem *************************Isinstance exist***********************************
:Checkinstance
if %INSTANCENAME% == %DEFAULTINSTANCENAME% (
    sc query state= all | findstr -c:"SERVICE_NAME: MSSQLSERVER" > %InstanceListTmp%
    if !errorlevel! equ 0 (
        call :Log "DBNAME:%DBNAME% Begin to check DEFAULTINSTANCENAME."
    ) else (
        call :log "DEFAULTINSTANCENAME %INSTANCENAME% is not-exist!"
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
            if %%i == %INSTANCENAME% (
                if exist %InstanceListTmp% del /f /q %InstanceListTmp%
                if exist %InstanceList% del /f /q %InstanceList%
                goto end;
            )
        )
        call :log "InstanceList %INSTANCENAME% is not-exist!"
        if exist %InstanceListTmp% del /f /q %InstanceListTmp%
        if exist %InstanceList% del /f /q %InstanceList%
        exit %ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST%
    ) else (
        call :log "%INSTANCENAME% is not-exist!"
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
            set DBClusterName=%%j
            set DBClusterName=!DBClusterName: =!
        )
        
        if "!title!" == "InstanceName" (
            set DBInstanceName=%%j
            set DBInstanceName=!DBInstanceName: =!
            if "!DBInstanceName!" == "%INSTANCENAME%" (
                goto end;
            )
        )
    )
    set DBClusterName=
    call :Log "Get Cluster Name Error, DBInstance: !INSTANCENAME!"
) else (
    more %DBSqlService% >> %LOGFILEPATH%
    call :Log "Exec Get Cluster Name Error, DBInstance: !INSTANCENAME!"
)

:end
goto :eof

rem ###########################################################################
rem get osql cmd path;
rem ###########################################################################
:GETOSQLCMDPATH
set %OSQLFindFlag%=1
for /l %%a in (%MinSqlServerVerNumber% 1 %MaxSqlServerVerNumber%) do (
    for /f "tokens=1,2,* " %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server\%%a0\Tools\ClientSetup" /v "Path"') do (
        set "OSQLPathTmp=%%k"
        set %~1=!OSQLPathTmp!\osql
        set /a %~2=%OPERATION_RTN_SUCCESS%
    )
)
goto :eof

rem ************************************************************************
rem function name: DeleteAllFile
rem aim:           delete all tmp file
rem input:         
rem output:        
rem ************************************************************************
:DeleteAllFile
    call :DeleteFile %ARGFILENAME%
    call :DeleteFile %DBSERVICESTATUSFILE%
    call :DeleteFile %DBSTATUSRSTFILETMP%
    call :DeleteFile %DBSTATUSRSTFILE%
    call :DeleteFile %SQLTESTFILE%
    call :DeleteFile %SQLONLINEFILE%
    call :DeleteFile %SQLOFFLINFILE%
    call :DeleteFile %SQLGETSTATUSFILE%
    call :DeleteFile %ONRESULTFILE%
    call :DeleteFile %DBSqlService%
    call :DeleteFile %DBSTATUSSQL%
    call :DeleteFile %SQLSERVERDBINFO%
    call :DeleteFile %DBCMDRESULT%
    call :DeleteFile %InstanceListTmp%
    call :Log "Delete tmp file successful."
goto :EOF    
 
endlocal
