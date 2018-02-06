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

set /a ERROR_SCRIPT_ORACLE_INST_NOT_CDB=47
set /a ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT=48
set /a ERROR_SCRIPT_START_PDB_FAILED=49

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

set ORACLE_CDB_TYPE=
set RET_RESULT_NOT_CDB=0
set RET_RESULT_CDB=1
set EXIT_CODE_NORMAL=0
rem **************************Set file name and path***************************************
rem the interval of querying is 2 second
set QUERYINTERVAL=200
rem the circling time is 60 tim
set /a CIRCLETIME=120
set /a COMSTATE=1
set TNSNAME=TNSLSNR

set LOGININPRE=LoginInSql
set LOGININSCRIPT="%AGENT_TMP_PATH%%LOGININPRE%%PID%.sql"

set LOGININRSPRE=LoginInTmp
set LOGININRST="%AGENT_TMP_PATH%%LOGININRSPRE%%PID%.txt"

rem ***************************2012-05-25 modify problem:**************************
set TNSLSNRPRE=TNSLSNRTmp
set TNSLSNRRST="%AGENT_TMP_PATH%%TNSLSNRPRE%%PID%.txt"

set LOGFILE=oraclecheckcdb.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

rem check instance status
set CHECKINSTANCESTATUS="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set CHECKINSTANCESTATUSRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"

set CHECKORACLECDB="%AGENT_TMP_PATH%CheckOracleCDB%PID%.sql"
set CHECKORACLECDBRST="%AGENT_TMP_PATH%CheckOracleCDBType%PID%.txt"

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

set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
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
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "OracleHome" IN_ORACLE_HOME
set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
)

call :Log "SubAppName=!DBINSTANCE!;AppName=!DBNAME!;UserName=!DBUSER!;IN_ORACLE_HOME=!IN_ORACLE_HOME!"
call :SetDBAuth
call :CheckInstanceStatusIsOk
call :CheckOracleVersionIsOk
call :CheckIsCDB
call :GetRet

:GetRet
	if "%ORACLE_CDB_TYPE%" EQU "YES" (
		echo "%RET_RESULT_CDB%" > %RSTFILE%
		call :Log "end to check oracle instance type, oracle is CDB."
		exit %EXIT_CODE_NORMAL%
	) else (
		echo "%RET_RESULT_NOT_CDB%" > %RSTFILE%
		call :Log "oracle is _not CDB."
		exit %EXIT_CODE_NORMAL%
	)
goto :eof
rem ----------------------------------------------------------------------
rem                     check instance status
rem ----------------------------------------------------------------------
:CheckInstanceStatusIsOk
	call :Log "Start to check oracle instance status."
	call :GetInstanceStatus INSTStatus RetCode
	if !RetCode! NEQ 0 (
		call :Log: "get oracle instance status failed."
		exit !RetCode!
	)
	call :Log: "get oracle instance status: !INSTStatus!."
	rem STARTED - After STARTUP NOMOUNT
	rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
	rem OPEN - After STARTUP or ALTER DATABASE OPEN
	rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
	if "!INSTStatus:~0,4!" NEQ "OPEN" (
		call :Log: "Error: INSTANCE_STATUS isn't open"
		exit %ERROR_INSTANCE_NOSTART%
	)
	call :Log "end to check oracle instance status."
goto :eof
rem ----------------------------------------------------------------------
rem                     get oracle version
rem ----------------------------------------------------------------------
:CheckOracleVersionIsOk
	call :Log "Start to check oracle version."
	call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVERSION% %PID% %LOGFILE% ORA_VERSION
	set PREDBVERSION=%ORA_VERSION:~0,2%
	if "!PREDBVERSION!" LSS "12" (
		call :Log "Oracle version is %PREDBVERSION%, none cdb."
		echo "%RET_RESULT_NOT_CDB%" > %RSTFILE%
		exit %EXIT_CODE_NORMAL%
	)
	call :Log "end to check oracle instance version(%PREDBVERSION%)."
goto :eof 
rem ----------------------------------------------------------------------
rem                     check db type: is CDB ?
rem ----------------------------------------------------------------------
:CheckIsCDB
	call :Log "Start to check cdb."
	echo select cdb from v$database; > %CHECKORACLECDB%
	echo exit >> %CHECKORACLECDB%
	
	call :Log "Exec sql to get oracle db _type."
	call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %CHECKORACLECDB% %CHECKORACLECDBRST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
	if "!RetCode!" NEQ "0" (
		call :Log "Excute sql script to get db _type failed."
		call :DeleteFile %CHECKORACLECDB%
		call :DeleteFile %CHECKORACLECDBRST%
		exit !RetCode!
	) else (
		call :Log "CheckIsCDB Exec sql success."
		call :Log "----------------------------------"
		type %CHECKORACLECDBRST% >> %LOGFILEPATH%
		call :Log "----------------------------------"
		set /a FLG_FIND=0
		for /f "skip=12" %%i in ('type %CHECKORACLECDBRST%') do (
			if "!FLG_FIND!" EQU "1" (
				set ORACLE_CDB_TYPE=%%i
				call :DeleteFile %CHECKORACLECDB%
				call :DeleteFile %CHECKORACLECDBRST%
				call :Log "ORACLE_CDB_TYPE=%%i"
				goto :eof
			)
			
			echo %%i | findstr \-\-\-
			if "!errorlevel!" EQU "0" (
				set /a FLG_FIND=1
			)
		)
	)
	call :DeleteFile %CHECKORACLECDB%
	call :DeleteFile %CHECKORACLECDBRST%

goto :eof
rem ----------------------------------------------------------------------
rem                         The END
rem ----------------------------------------------------------------------

rem ************************** check instance status ***********************************
:GetInstanceStatus
    echo select status from v$instance; > %CHECKINSTANCESTATUS%
    echo exit >> %CHECKINSTANCESTATUS%

    call :Log "Exec sql to get status of database."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %CHECKINSTANCESTATUS% %CHECKINSTANCESTATUSRST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
    if "!RetCode!" NEQ "0" (
		call :Log "GetInstanceStatus Exec sql failed."
        set /a %~2 = !RetCode!
        call :DeleteFile %CHECKINSTANCESTATUS%
        call :DeleteFile %CHECKINSTANCESTATUSRST%
    ) else (
        rem STARTED - After STARTUP NOMOUNT
        rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
        rem OPEN - After STARTUP or ALTER DATABASE OPEN
        rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
		call :Log "GetInstanceStatus Exec sql success."
		call :Log "----------------------------------"
		type %CHECKINSTANCESTATUSRST% >> %LOGFILEPATH%
		call :Log "----------------------------------"
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
call :DeleteFile %CHECKINSTANCESTATUS%
call :DeleteFile %CHECKINSTANCESTATUSRST%
goto :eof

:SetDBAuth
    if !AUTHMODE!==1 (
        set DB_LOGIN=/ as sysdba
    )
    if !AUTHMODE!==0 (
        if "!DBUSER!"=="sys" (
            set DB_LOGIN=!DBUSER!/"!DBUSERPWD!" as sysdba
        ) else (
            set DB_LOGIN=!DBUSER!/"!DBUSERPWD!"
        )
    )
goto :eof

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

endlocal