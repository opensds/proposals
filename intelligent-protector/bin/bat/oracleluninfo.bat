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

set STORAGE_FS=0
set STORAGE_RAW=1
set STORAGE_ASMDISK=2
set STORAGE_ASMRAW=3
set STORAGE_ASMLINK=4
set STORAGE_ASMUDEV=5
set STORAGE_WIN_ASMDISK=6

set DB_LOGIN=
set ASM_LOGIN=

rem **************************Set file name and path***************************************
set RESULTFILEPRE=ResultFile
set RESULTFILE="%AGENT_TMP_PATH%%RESULTFILEPRE%%PID%.txt"

set ASMDISKINFOTMPFILE="%AGENT_TMP_PATH%AsmDiskInfoTmpFile.txt"

set RSLTMPFILEPRE=RslTmpFile
set RSLTMPFILE="%AGENT_TMP_PATH%%RSLTMPFILEPRE%%PID%.txt"

set LOCATIONLISTFILEPRE=LocationListFile
set LOCALLISTFILE="%AGENT_TMP_PATH%%LOCATIONLISTFILEPRE%%PID%.txt"

set LOCALTMPFILEPRE=LocalTmpFile
set LOCATTMPFILE="%AGENT_TMP_PATH%%LOCALTMPFILEPRE%%PID%.txt"
set LOCAL_RECOVERY_TMP_FILE="%AGENT_TMP_PATH%LocalRecoveryTmpFile%PID%.txt"

set QUERYFILENAMEPRE=QueryFileName
set QUERYNAMECRIPT="%AGENT_TMP_PATH%%QUERYFILENAMEPRE%%PID%.sql"

set LOGFILE=oracleluninfo.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"

set QUERYASMDiskCRIPT="%AGENT_TMP_PATH%QueryASMDisk%PID%.sql"
set QUERYASMDiskRSTCRIPT="%AGENT_TMP_PATH%QueryASMDiskRST%PID%.txt"
set QUREYSERVICE="%AGENT_TMP_PATH%QueryService%PID%.txt"
set QUERY_RECOVERY_SCRIPT="%AGENT_TMP_PATH%QueryRecovery%PID%.sql"
set QUERY_RECOVERY_RESULT_FILE="%AGENT_TMP_PATH%QueryRecoveryRST%PID%.txt"

set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"

rem check instance status
set CHECKINSTANCESTATUS="%AGENT_TMP_PATH%CheckInstanceStatus%PID%.sql"
set CHECKINSTANCESTATUSRST="%AGENT_TMP_PATH%CheckInstanceStatusRST%PID%.txt"

rem check archivelog mdoe
set GETARCHIVEINFOFILE="%AGENT_TMP_PATH%GetArchive%PID%.sql"
set GETARCHIVEINFORST="%AGENT_TMP_PATH%GetArchiveRST%PID%.txt"

set CURRENTPIDRST="%AGENT_TMP_PATH%PIDTmp%PID%.txt"
set CURRENTCMDLineRST="%AGENT_TMP_PATH%ProcessCommandLine%PID%.txt"
set /a CURRENTPID=0

set PARAM_FILE="%AGENT_TMP_PATH%input_tmp%PID%"
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
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "TableSpaceName" DBTABLESPACENAME
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMUserName" ASMUSER
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMPassword" ASMUSERPWD
call %COMMONFUNC% "%AGENT_ROOT%" %CMD_GETVALUE% %PID% %LOGFILE% "!INPUTINFO!" "ASMInstanceName" ASMINSTANCENAME

set AUTHMODE=0
if "%DBUSERPWD%" == "" (
    set AUTHMODE=1
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
            more %CURRENTCMDLineRST% | findstr /c:"oracleluninfo.bat\" \"%AGENT_ROOT%\" %PID%"
        ) else (
            more %CURRENTCMDLineRST% | findstr /c:"oracleluninfo.bat %AGENT_ROOT% %PID%"
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

call :Log "Begin get tablespace information for %DBNAME%!"
call :Log "AppName=%DBNAME%;InstanceName=%DBINSTANCE%;UserName=%DBUSER%;ASMUSER=%ASMUSER%;ASMINSTANCENAME=%ASMINSTANCENAME%;DBTABLESPACENAME=!DBTABLESPACENAME!;AUTHMODE=!AUTHMODE!;CURRENTPID=!CURRENTPID!;"

rem check asm intance status
call :Log "Start to check oracle instance status."
call :GetInstanceStatus INSTStatus RetCode
if "!RetCode!" NEQ "0" (
    call :Log "Get instance status failed."
    exit !RetCode!
)

rem STARTED - After STARTUP NOMOUNT
rem MOUNTED - After STARTUP MOUNT or ALTER DATABASE CLOSE
rem OPEN - After STARTUP or ALTER DATABASE OPEN
rem OPEN MIGRATE - After ALTER DATABASE OPEN { UPGRADE | DOWNGRADE }
if not "!INSTStatus:~0,4!" == "OPEN" (
    call :Log "Instance status !INSTStatus! no open"
    exit %ERROR_RECOVER_INSTANCE_NOSTART%
)
call :Log "End to check oracle instance status."

rem check archivelog mode

rem try to get ASM instance name
sc query state= all | findstr /i "OracleASMService%ASMINSTANCENAME%" > %QUREYSERVICE%
for /f "tokens=1,2 delims=:" %%a in ('type %QUREYSERVICE%') do (
    if "SERVICE_NAME"=="%%a" (
        for /f "tokens=1,2 delims=+" %%i in ("%%b") do (
            set ASMINSTANCENAME=+%%j
        )
    )
)
call :DeleteFile %QUREYSERVICE%
call :DeleteFile %RSTFILE%
call :Log "check ASM Instance=!ASMINSTANCENAME!"
rem *************************Entry of script to query  oracle database file********************
call :CreatFileNameSql %QUERYNAMECRIPT%

call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %QUERYNAMECRIPT% %RESULTFILE% !DBINSTANCE! "!DB_LOGIN!" 60 RetCode
call :DeleteFile %QUERYNAMECRIPT%
if !RetCode! NEQ 0 (
    call :DeleteFile %RESULTFILE%
    exit !RetCode!
) else (
    findstr "dba_data_files$" %RESULTFILE% > nul
    if !errorlevel! EQU 0 (
          call :Log "Query database %DBNAME% location failed."
          call :DeleteFile %RESULTFILE%
          exit %ERROR_SCRIPT_EXEC_FAILED%
      ) else (
        for /f %%i in ('type %RESULTFILE%') do (
            if not defined %%i set %%i=A & echo %%i >> %RSLTMPFILE%
        )
        call :DeleteFile %RESULTFILE%

        if "%DBTABLESPACENAME%" == "archive" (
            call :Log "Begin to check oracle archivelog mode."
            call :CheckArchiveMode
            call :Log "End to check oracle archivelog mode."
            
            call :Log "Get device names for archive logs."
            rem get file system
            call :GetDeviceNamesOnArchDir %RSLTMPFILE%
        ) else (
            call :Log "Get device names for data files."
            rem get file system
            call :GetDeviceNamesOnFS %RSLTMPFILE%
            rem manage asm disks
            call :GetDeviceNamesOnASM %RSLTMPFILE%
        )
        call :DeleteFile %RSLTMPFILE%
    )
)
    
call :Log "Success to stat. oracle database file information."
exit 0

:GetDeviceNamesOnArchDir
    set resultFile="%~1"
    call :GetDeviceNamesOnRecovery !resultFile!
    set resultFile="%~1"
    call :GetDeviceNamesOnFS !resultFile!
    set resultFile="%~1"
    call :GetDeviceNamesOnASM !resultFile!
    goto :EOF

:GetDeviceNamesOnRecovery
    call :Log "Begin get device names on recovery."
    set resultFile="%~1"
    type %resultFile% | findstr /i "USE_DB_RECOVERY_FILE_DEST" > %LOCAL_RECOVERY_TMP_FILE%
    for /f %%i in ('type %LOCAL_RECOVERY_TMP_FILE%') do (
        call :Log "Get recovery file %%i."
        call :CreateQueryRecoverySql %QUERY_RECOVERY_SCRIPT%
        call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %QUERY_RECOVERY_SCRIPT% %QUERY_RECOVERY_RESULT_FILE% !DBINSTANCE! "!DB_LOGIN!" 60 RetCode
        call :DeleteFile %QUERY_RECOVERY_SCRIPT%
        if !RetCode! NEQ 0 (
            call :DeleteFile %QUERY_RECOVERY_RESULT_FILE%
            call :DeleteFile %LOCAL_RECOVERY_TMP_FILE%
            call :Log "Get device names on recovery failed."
            exit !RetCode!
        ) else (
            call :Log "Get device names on file system for recovery file."
            call :GetDeviceNamesOnFS %QUERY_RECOVERY_RESULT_FILE%
            call :Log "Get device names on asm for recovery file."
            call :GetDeviceNamesOnASM %QUERY_RECOVERY_RESULT_FILE%
            call :DeleteFile %QUERY_RECOVERY_RESULT_FILE%
        )
    )
    call :DeleteFile %LOCAL_RECOVERY_TMP_FILE%
    call :Log "End get device names on recovery."
    goto :EOF

:GetDeviceNamesOnFS
    set resultFile="%~1"
    call :Log "Begin get device names on file system, result file !resultFile!."
    rem Archive log dir can be root dir(e.g., E:, F:), and colon can't used in ASM instance name and dir name.
    type %resultFile% | findstr /i "^[a-zA-Z]:" > %LOCATTMPFILE%
    for /f %%i in ('type %LOCATTMPFILE%') do (
        set filePath=%%i
        call :Log "Get file path !filePath!."
        call :GetDeviceNameOnFS !filePath!
    )
    call :DeleteFile %LOCATTMPFILE%
    call :Log "End get device names on file system."
    goto :EOF
    
:GetDeviceNameOnFS
    set tmpPath=%~1
    call :Log "Begin get one device name on file system, file path !tmpPath!."
    for /f "tokens=1 delims=:" %%i in ("!tmpPath!") do (
        call :Log "Get driver letter %%i."
        call :Log "%%i;;%STORAGE_FS%;"
        echo %%i;;%STORAGE_FS%; >> %RSTFILE%
    )
    call :Log "End get one device name on file system."
    goto :EOF

:GetDeviceNamesOnASM
    set resultFile="%~1"
    call :Log "Begin get device names on asm, result file !resultFile!."
    type %resultFile% | findstr /i "^+" > %LOCATTMPFILE%
    for /f "tokens=1 delims=/" %%i in ('type %LOCATTMPFILE%') do (
        echo %%i>> %LOCALLISTFILE%
    )
    call :DeleteFile %LOCATTMPFILE%
    
    rem get asm disk
    for /f %%i in ('type %LOCALLISTFILE%') do (
        set dgName=%%i
        set dgName=!dgName:~1!
        call :Log "Get disk group !dgName!."
        call :GetDeviceNameOnASM !dgName!
    )
    call :DeleteFile %LOCALLISTFILE%
    call :Log "End get device names on asm."
    goto :EOF

:GetDeviceNameOnASM
    set dgName=%~1
    call :Log "Begin get one device name on asm, dg name !dgName!."
    call :CreatASMDiskSql %QUERYASMDiskCRIPT% !dgName!
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECASMSQL% %PID% %LOGFILE% %QUERYASMDiskCRIPT% %QUERYASMDiskRSTCRIPT% %ASMINSTANCENAME% "!ASM_LOGIN!" 60 RetCode
    if !RetCode! NEQ 0 (
        call :DeleteFile %QUERYASMDiskCRIPT%
        call :DeleteFile %QUERYASMDiskRSTCRIPT%
        exit !RetCode!
    ) else (
        rem get disk list
        for /f "tokens=2 delims=\" %%a in ('type %QUERYASMDiskRSTCRIPT%') do (
            set dName=%%a
            asmtool -list|findstr "!dName!">%ASMDISKINFOTMPFILE%
            if !errorlevel! EQU 0 (
            for /f "tokens=1,* delims=\" %%x in ('type %ASMDISKINFOTMPFILE%') do (
                set a1=%%y
                for /f "tokens=1" %%m in ("!a1!") do (
                   set devicepath=\%%m
                   echo !devicepath!;!dName!;%STORAGE_WIN_ASMDISK%;!dgName! >> %RSTFILE%
                   call :Log "!devicepath!;!dName!;%STORAGE_WIN_ASMDISK%;!dgName!" 
                )
            )
          )
        )
        
        call :DeleteFile %QUERYASMDiskCRIPT%
        call :DeleteFile %QUERYASMDiskRSTCRIPT%
        call :DeleteFile %ASMDISKINFOTMPFILE%
    )
    call :Log "End get one device name on asm."
    goto :EOF

rem ************************************************************************
rem function name: Log
rem aim:           Print log function, controled by "NEEDLOGFLG"
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
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
rem function name: CreatFileNameSql
rem aim:           Create temporary sql script for quering file_name function
rem input:         the sql script name
rem output:        QUERYNAMECRIPT
rem ************************************************************************
:CreatFileNameSql
    set tmppath="%~1"
    
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
    if "%DBTABLESPACENAME%" == "must" (
        call :Log "Generate query data files sql."
        echo select file_name from dba_data_files; > !tmppath!
        echo select MEMBER from v$logfile; >> !tmppath!
        echo select name from v$controlfile; >> !tmppath!
    ) else (
        if "%DBTABLESPACENAME%" == "option" (
            call :Log "Generate query temp files sql."
            echo select file_name from dba_temp_files; > !tmppath!
        ) else (
            call :Log "Generate query archive log files sql."
            echo select DESTINATION from v$archive_dest where STATUS='VALID';> !tmppath!
        )
    )
    echo exit >> !tmppath!
    goto :EOF

:CreateQueryRecoverySql
    set tmpPath="%~1"    
    call :Log "Generate query recovery file sql."
    echo select NAME from V$RECOVERY_FILE_DEST; > !tmppath!

    echo exit >> !tmppath!
    goto :EOF

rem ************************************************************************
rem function name: CreatASMDiskSql
rem aim:           Create temporary sql script for quering file_name function
rem input:         the sql script name
rem output:        QUERYNAMECRIPT
rem ************************************************************************
:CreatASMDiskSql
    set tmppath="%~1"
    set dgName=%~2
    echo select A.PATH from v$asm_disk A, v$asm_diskgroup B where A.GROUP_NUMBER = B.GROUP_NUMBER and B.Name='!dgName!'; > !tmppath!
    echo exit >> !tmppath!
goto :EOF

rem Convert str to Low
:UperTOLow
    set CONVERTSTR=%~1
    for %%a in (a b c d e f g h i j d l m n o p q r s t u v w x y z) do (
        call set CONVERTSTR=%%CONVERTSTR:%%a=%%a%%
    )
    
    set %~2=!CONVERTSTR!
goto :EOF

rem ************************** check instance status ***********************************
:GetInstanceStatus
    echo select status from v$instance; > %CHECKINSTANCESTATUS%
    echo exit >> %CHECKINSTANCESTATUS%

    call :Log "Exec sql to get status of database."
    call %COMMONFUNC% "%AGENT_ROOT%" %CMD_EXECSQL% %PID% %LOGFILE% %CHECKINSTANCESTATUS% %CHECKINSTANCESTATUSRST% !DBINSTANCE! "!DB_LOGIN!" 30 RetCode
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


rem ************************************************************************
rem function name: checkArchiveModeAndArchiveUsing
rem aim:           checkArchiveModeAndArchiveUsing
rem input:         
rem output:        
rem ************************************************************************
:CheckArchiveMode
    rem *************************begin 2014-12-15 check archive directory******************
    set /a IS_RECOVERY_FILE_DEST=0
    
    call :Log "Begin check archive dest directory."
    set TITLE_LOGMODE=Database log mode
    set TITLE_ARCHIVE_DEST=Archive destination
    set STRARCHIVEDEST=

    echo ALTER SESSION SET NLS_LANGUAGE='AMERICAN'; > %GETARCHIVEINFOFILE%
    echo select LOG_MODE from v$database; >> %GETARCHIVEINFOFILE%
    echo exit >> %GETARCHIVEINFOFILE%
    
    call :Log "Exec sql to get archive mode."
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
        set ASM_LOGIN=%ASMUSER%/"%ASMUSERPWD%" as sysasm
    )
goto :eof

endlocal

