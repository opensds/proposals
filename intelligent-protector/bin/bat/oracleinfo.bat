@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


setlocal EnableDelayedExpansion

set QUOTE=;
set ORACLESERVICEPRE=ORACLESERVICE
set ORACLEVSSWRITERPRE=ORACLEVSSWRITER

set AGENT_ROOT=%~1
set PID=%~2
set AGENT_BIN_PATH=%AGENT_ROOT%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT%\log\
set AGENT_TMP_PATH=%AGENT_ROOT%\tmp\

set CMD_GETVERSION=getversion
set CMD_EXECSQL=execsql
set CMD_GETVALUE=getvalue
set CMD_GETORAPATH=getoraclepath
set CMD_GETGRIDPATH=getgridpath
set LOGFILE=oracleinfo.log
set LOGFILEPATH="%AGENT_LOG_PATH%%LOGFILE%"
set COMMONFUNC="%AGENT_BIN_PATH%oraclefunc.bat"
set RSTFILE="%AGENT_TMP_PATH%result_tmp%PID%"
set ORATMPINFO="%AGENT_TMP_PATH%ORATMPINFO%PID%"
set SIDTMPINFO="%AGENT_TMP_PATH%SIDTMPINFO%PID%"
set /a ERROR_SCRIPT_EXEC_FAILED=5

call :DeleteFile %RSTFILE%
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

call :Log "Begin to check oracle version."
set MAIN_VERSION=%ORA_VERSION:~0,2%
if not "!MAIN_VERSION!" == "10" (
    if not "!MAIN_VERSION!" == "11" (
        if not "!MAIN_VERSION!" == "12" (
            call :Log "Oracle version-!ORA_VERSION! is not supported."
            exit 1
        )
    )
)

call :Log "Begin to read oracle mapping info of database and instance info."
call :DeleteFile %ORATMPINFO%

for /f %%a in ('dir /B !ORACLEBASEPATH!\admin') do (
    set DB_NAME=%%a
    if "!MAIN_VERSION!" == "10" (
        for /f %%b in ('dir /B !ORACLEBASEPATH!\admin\!DB_NAME!\bdump\alert_*.log') do (
            set ALTER_FILE=%%b
            set SID_TMP=!ALTER_FILE:~6!
            set SID_NAME=!SID_TMP:~0,-4!
            echo !SID_NAME! !DB_NAME! >> !ORATMPINFO!
        )
    ) else (
        for /f %%b in ('dir /B /A:D !ORACLEBASEPATH!\diag\rdbms\!DB_NAME!') do (
            set SID_NAME=%%b
            if not "!SID_NAME!" == "" (
                echo !SID_NAME! !DB_NAME! >> !ORATMPINFO!
            )
        )
    )
)

call :Log "Begin to read oracle instance list."
call :DeleteFile %SIDTMPINFO%
for /f %%a in ('dir /B !ORACLEHOMEPATH!\database\init*.ora') do (
    set INIT_FILE=%%a
    set SID_TMP=!INIT_FILE:~4!
    set SID_NAME=!SID_TMP:~0,-4!
    echo ;!SID_NAME!; >> !SIDTMPINFO!
)

for /f %%a in ('dir /B !ORACLEHOMEPATH!\database\spfile*.ora') do (
    set SPFILE_FILE=%%a
    set SID_TMP=!SPFILE_FILE:~6!
    set SID_NAME=!SID_TMP:~0,-4!
    more !SIDTMPINFO! | findstr /i ";!SID_NAME!;" > NUL
    if !errorlevel! NEQ 0 (
        echo ;!SID_NAME!; >> !SIDTMPINFO!
    )
)

call :Log "Begin to read oracle all information."
call :DeleteFile %RSTFILE%
set /p=<nul>%RSTFILE%
for /f "tokens=1 delims=;" %%a in ('more !SIDTMPINFO!') do (
    set SID_NAME=%%a
    set PLUG_FLG=!SID_NAME:~0,1!
    if not "!PLUG_FLG!" == "+" (
        set /a FLG_DBNAME=0
        if not "!SID_NAME!" == "" (
            rem check ASM flag
            set DBISASM=0
            call :CheckIsASM !SID_NAME!
            
            rem check running status
            set STATUSTMP=1
            for /f "tokens=2 delims=:" %%i in ('sc query %ORACLESERVICEPRE%!SID_NAME! ^| findstr /i "STATE"') do (
                for /f "tokens=1 delims= " %%j in ("%%i") do (
                    if %%j EQU 4 (
                       set /a STATUSTMP=0
                    ) else if %%j equ 1 (
                       set /a STATUSTMP=1
                    ) else (
                       set /a STATUSTMP=1
                    )
                )
            )
            
            rem check vss status
            set /a VSS_STATUS=2
            set TMPSTR=
            for /f "delims=" %%i in ('2^>nul sc query "%ORACLEVSSWRITERPRE%!SID_NAME!" ^| findstr /i "STATE"') do (set "TMPSTR=%%i")
            if not "!TMPSTR!" == "" (
                set VSSSTATE=
                for /f "tokens=2 delims=: " %%i in ("!TMPSTR!") do (set "VSSSTATE=%%i")
                if "!VSSSTATE!" == "1" (
                    set /a VSS_STATUS=0
                ) else (
                    if "!VSSSTATE!" == "4" (
                        set VSS_WRITER=
                        for /f "delims=" %%i in ('2^>nul vssadmin list writers ^| findstr /i /c:"Oracle VSS Writer - !SID_NAME!"') do (set "VSS_WRITER=%%i")
                        if not "!VSS_WRITER!" == "" (
                            set /a VSS_STATUS=1
                        )
                    ) 
                )
            )
            
            rem get database name, must have a black space
            for /f "tokens=1,2 delims= " %%i in ('more !ORATMPINFO!') do (
                set SID_NAME_TMP=%%i
                if /i "!SID_NAME_TMP!" == "!SID_NAME!" (
                    set DB_NAME=%%j
                    set /a FLG_DBNAME=1
                    call :GetDBNameLength !DB_NAME! NAMELENGTH
                    if !NAMELENGTH! GTR 8 (
                        set "DB_NAME=!DB_NAME:~0,8!"
                    )
                    call :Log "!ORA_VERSION!%QUOTE%!SID_NAME!%QUOTE%!DB_NAME!%QUOTE%!STATUSTMP!%QUOTE%!DBISASM!%QUOTE%!VSS_STATUS!%QUOTE%!ORACLEHOMEPATH!"
                    echo !ORA_VERSION!%QUOTE%!SID_NAME!%QUOTE%!DB_NAME!%QUOTE%!STATUSTMP!%QUOTE%!DBISASM!%QUOTE%!VSS_STATUS!%QUOTE%!ORACLEHOMEPATH!>> %RSTFILE%
                )
            )
            
            if "!FLG_DBNAME!" EQU "0" (
                call :Log "Instance !SID_NAME! can not found database name."
                call :Log "!ORA_VERSION!%QUOTE%!SID_NAME!%QUOTE%%QUOTE%!STATUSTMP!%QUOTE%!DBISASM!%QUOTE%!VSS_STATUS!%QUOTE%!ORACLEHOMEPATH!"
                echo !ORA_VERSION!%QUOTE%!SID_NAME!%QUOTE%%QUOTE%!STATUSTMP!%QUOTE%!DBISASM!%QUOTE%!VSS_STATUS!%QUOTE%!ORACLEHOMEPATH!>> %RSTFILE%
            )
        )
    )
)

call :DeleteFile !SIDTMPINFO!
call :DeleteFile !ORATMPINFO!
exit 0

rem ************************************************************************
rem function name GetDBNameLength
rem aim:          get the dbname length
rem input:        DBName
rem output:       Length   
rem ************************************************************************
:GetDBNameLength
    set str=%~1
    set /a num=0
    :next
          if not "%str%"=="" (
            set /a num+=1
              set "str=%str:~1%"
              goto next
          )
      set /a %~2=%num%
goto :EOF

:CheckParameterFile
    rem first copy init file to tmp directory
    if not exist "%~1" (
        call :Log "%~1 not exists."
        goto :CHECKEXIT
    )
    
    set ParamfileName="%~1"
    set tmpParamFile="%AGENT_TMP_PATH%\paramFile%STRSID%%PID%.ORA"
    copy /Y !ParamfileName! !tmpParamFile!

    for /f "tokens=1" %%a in ('type !tmpParamFile!') do (
        echo %%a | findstr /I "SPFILE" > nul
        if !errorlevel! EQU 0 (
            for /f "tokens=1,2 delims==" %%b in ("%%a") do (
                set STRTMP=%%c
                set STRTMP=!STRTMP:~1,1!
                if "!STRTMP!" == "+" (
                    set DBISASM=1
                    call :DeleteFile !tmpParamFile!
                    goto :EOF
                )
            )
        )
        
        echo %%a | findstr /I "control_files=" > nul
        if !errorlevel! EQU 0 (
            for /f "tokens=1,2 delims==" %%b in ("%%a") do (
                set STRTMP=%%c
                set STRTMP=!STRTMP:~1,1!
                if "!STRTMP!" == "+" (
                    set DBISASM=1
                    call :DeleteFile !tmpParamFile!
                    goto :EOF
                )
            )
        )        
    )
    call :DeleteFile !tmpParamFile!
goto :EOF

:CheckIsASM
    set STRSID=%1
    set DBISASM=0
    
    call :CheckParameterFile "%ORACLEHOMEPATH%\database\init%STRSID%.ORA"
    if "!DBISASM!" EQU "0" (
        call :CheckParameterFile "%ORACLEHOMEPATH%\database\spfile%STRSID%.ORA"
    )
    
goto :EOF

rem ************************************************************************
rem function name: DeleteFile
rem aim:           Delete file function
rem input:         the deleted file
rem output:        
rem ************************************************************************
:DeleteFile
    set FileName=%~1
    if exist "%FileName%" (del /f /q "%FileName%")
    
goto :EOF

rem ************************************************************************
rem function name: Log
rem input:         the recorded log
rem output:        LOGFILENAME
rem ************************************************************************
:Log
    echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> %LOGFILEPATH%
    call "%AGENT_BIN_PATH%\agent_func.bat" %LOGFILEPATH%
goto :EOF

endlocal
