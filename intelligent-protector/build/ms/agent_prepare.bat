@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal DisableDelayedExpansion

rem 使用7z解压时，现将*gz格式解压为*tar格式，然后再次解压，jsoncpp是解压至\dist目录下
set Drive=%~d0
set CURBAT_PATH=%~dp0

set AGENT_OPEN_SRC_PATH=%CURBAT_PATH%..\..\open_src
set AGENT_ROOT_PATH=%CURBAT_PATH%..\..
set AGENT_OPEN_SRC_TMP=%AGENT_OPEN_SRC_PATH%\tmp

IF NOT EXIST "%AGENT_OPEN_SRC_TMP%"  MKDIR "%AGENT_OPEN_SRC_TMP%"

MOVE "%AGENT_OPEN_SRC_PATH%\*" "%AGENT_OPEN_SRC_TMP%"

cd /d "%AGENT_OPEN_SRC_TMP%"

set FILE_NAME=fcgi
set NEW_NAME=fcgi
call :UNZIPGZFILES

set FILE_NAME=openssl
set NEW_NAME=openssl
call :UNZIPGZFILES

set FILE_NAME=snmp++
set NEW_NAME=snmp++-3.3.7
set MV_NEW_NAME=snmp++
call :UNZIPGZFILES
MOVE "%AGENT_OPEN_SRC_PATH%\%NEW_NAME%" "%AGENT_OPEN_SRC_PATH%\%MV_NEW_NAME%"

set FILE_NAME=tinyxml
set NEW_NAME=tinyxml
call :UNZIPGZFILES

set FILE_NAME=SQLite
set NEW_NAME=sqlite
call :UNZIPZIPFILES

set FILE_NAME=jsoncpp
set NEW_NAME=jsoncpp
call :UNZIPZIPFILES

set FILE_NAME=nginx
set NEW_NAME=nginx_tmp
call :UNZIPGZFILES

MOVE "%AGENT_OPEN_SRC_TMP%\*.gz" "%AGENT_OPEN_SRC_PATH%"
MOVE "%AGENT_OPEN_SRC_TMP%\*.zip" "%AGENT_OPEN_SRC_PATH%"
MOVE "%AGENT_OPEN_SRC_TMP%\*.tgz" "%AGENT_OPEN_SRC_PATH%"
cd /d %CURBAT_PATH%
RD /Q /S "%AGENT_OPEN_SRC_TMP%"

call :PrepareNginx
call :PrepareFcgi

endlocal
goto :EOF

:UNZIPGZFILES
    IF EXIST "%AGENT_OPEN_SRC_TMP%\%FILE_NAME%*gz" (
        IF NOT EXIST "%AGENT_OPEN_SRC_PATH%\%NEW_NAME%" (
            "C:\Program Files (x86)\7-Zip\7z.exe" x %FILE_NAME%*gz -y -o"%AGENT_OPEN_SRC_TMP%"
            IF EXIST "%AGENT_OPEN_SRC_TMP%\dist"  (
                MOVE "%AGENT_OPEN_SRC_TMP%\dist\*" "%AGENT_OPEN_SRC_TMP%"
                RD /Q /S "%AGENT_OPEN_SRC_TMP%\dist
            )
            
            "C:\Program Files (x86)\7-Zip\7z.exe" x %FILE_NAME%*.tar -y -o"%AGENT_OPEN_SRC_PATH%"
            IF NOT EXIST "%AGENT_OPEN_SRC_PATH%\%NEW_NAME%" (
                MOVE "%AGENT_OPEN_SRC_PATH%\%FILE_NAME%*" "%AGENT_OPEN_SRC_PATH%\%NEW_NAME%" 
            )
        )
    )
    goto :EOF
    
:UNZIPZIPFILES
    IF EXIST "%AGENT_OPEN_SRC_TMP%\%FILE_NAME%*.zip" (
        IF NOT EXIST "%AGENT_OPEN_SRC_PATH%\%NEW_NAME%" (
            "C:\Program Files (x86)\7-Zip\7z.exe" x %FILE_NAME%*.zip -y -o"%AGENT_OPEN_SRC_PATH%"
            
            IF EXIST "%AGENT_OPEN_SRC_PATH%\%FILE_NAME%*" (
                MOVE "%AGENT_OPEN_SRC_PATH%\%FILE_NAME%*" "%AGENT_OPEN_SRC_PATH%\%NEW_NAME%"
            )
        )
    )
    goto :EOF
    
:PrepareNginx
    XCOPY /Y /E %AGENT_OPEN_SRC_PATH%\objs_ngx %AGENT_OPEN_SRC_PATH%\nginx_tmp
    
    goto :EOF
    
:PrepareFcgi
    copy /y %AGENT_ROOT_PATH%\src\patch\fcgi\os_win32.c %AGENT_OPEN_SRC_PATH%\fcgi\libfcgi\os_win32.c
    
    goto :EOF