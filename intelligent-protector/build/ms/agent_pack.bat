@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

rem 对工程应该首先编译，然后拷贝必要的文件，然后做安装包放置到Agent相关out目录下

rem 设置变量
@echo off

set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set AGENTPATH=%CurBatPath%..\..\
SET INSTALLSHIELD_PACKAGE_DIR=%AGENTPATH%build\ms\package
set BUILD_LOG="%CurBatPath%"build.log

set VERSION_PARAM="define AGENT_PACKAGE_VERSION"

set VERSION=
call :FindAgentVERSION VERSION
echo VERSION is %VERSION%

set VERSION_PARAM="define AGENT_VERSION"

set AGENT_VERSION=
call :FindAgentVERSION AGENT_VERSION
echo AGENT_VERSION is %AGENT_VERSION%

set VERSION_PARAM="define AGENT_BUILD_NUM"

set AGENT_BUILD_NUM=
call :FindAgentVERSION AGENT_BUILD_NUM
echo AGENT_BUILD_NUM is %AGENT_BUILD_NUM%

call :RemoveSVNDIRECTORY
echo RemoveSVNDIRECTORY called and finished

rem 获得系统的日期和时间
set DAY=%date:~0,2%
set MOUNT=%date:~3,2%
set YEAR=%date:~6,4%
set TIME=%YEAR%-%MOUNT%-%DAY%

rem file name must be like OceanStor BCManager Agent V100R002C10B010-WIN64-BIN-13-09-14.zip
set BIN_NAME_BASE=OceanStor BCManager %AGENT_VERSION%_Agent-WIN64

echo BIN ZIP Name will be %BIN_NAME_BASE%
set SRC_NAME_BASE=OceanStor BCManager %AGENT_VERSION%_Agent-SRC-%TIME%

rem=====================源码打包======================

"C:\Program Files (x86)\7-Zip\7z.exe" a "%INSTALLSHIELD_PACKAGE_DIR%\%SRC_NAME_BASE%.zip" "%AGENTPATH%\*"

echo  %DATE% %TIME% %1 zip agent src >> %BUILD_LOG%

rem===================================================

rem unzip the open_src files
call %CurBatPath%agent_prepare.bat

echo 1 is %1

if "%1" EQU "Clean" (GOTO Clean)

rem call 64 compile
call %AGENTPATH%build\ms\agent_make.bat compile64agent

SET PACKAGE_FILE_DIR=%AGENTPATH%pkg
SET SOURCE_FILE_DIR=%AGENTPATH%bin

IF EXIST %PACKAGE_FILE_DIR%\bin                         RMDIR  /S/Q  %PACKAGE_FILE_DIR%\bin
IF EXIST %PACKAGE_FILE_DIR%\bin\thirdparty              RMDIR  /S/Q  %PACKAGE_FILE_DIR%\bin\thirdparty
IF EXIST %PACKAGE_FILE_DIR%\conf                        RMDIR  /S/Q  %PACKAGE_FILE_DIR%\conf
IF EXIST %PACKAGE_FILE_DIR%\log                         RMDIR  /S/Q  %PACKAGE_FILE_DIR%\log
IF EXIST %PACKAGE_FILE_DIR%\tmp                         RMDIR  /S/Q  %PACKAGE_FILE_DIR%\tmp
IF EXIST %PACKAGE_FILE_DIR%\db                          RMDIR  /S/Q  %PACKAGE_FILE_DIR%\db

IF NOT EXIST %PACKAGE_FILE_DIR%                         MKDIR  %PACKAGE_FILE_DIR%
IF NOT EXIST %PACKAGE_FILE_DIR%\bin                     MKDIR  %PACKAGE_FILE_DIR%\bin
IF NOT EXIST %PACKAGE_FILE_DIR%\conf                    MKDIR  %PACKAGE_FILE_DIR%\conf
IF NOT EXIST %PACKAGE_FILE_DIR%\bin\plugins             MKDIR  %PACKAGE_FILE_DIR%\bin\plugins
IF NOT EXIST %PACKAGE_FILE_DIR%\bin\thirdparty          MKDIR  %PACKAGE_FILE_DIR%\bin\thirdparty
IF NOT EXIST %PACKAGE_FILE_DIR%\bin\thirdparty\sample   MKDIR  %PACKAGE_FILE_DIR%\bin\thirdparty\sample
IF NOT EXIST %PACKAGE_FILE_DIR%\log                     MKDIR  %PACKAGE_FILE_DIR%\log
IF NOT EXIST %PACKAGE_FILE_DIR%\tmp                     MKDIR  %PACKAGE_FILE_DIR%\tmp
IF NOT EXIST %PACKAGE_FILE_DIR%\db                      MKDIR  %PACKAGE_FILE_DIR%\db




rem 拷贝需要的文件到srcfiles目录下，供InstallShield打包使用

IF EXIST %PACKAGE_FILE_DIR%\bin\*.exe          DEL /F  %PACKAGE_FILE_DIR%\bin\*.exe
IF EXIST %PACKAGE_FILE_DIR%\bin\*.dll          DEL /F  %PACKAGE_FILE_DIR%\bin\*.dll
IF EXIST %PACKAGE_FILE_DIR%\bin\plugins\*.dll  DEL /F  %PACKAGE_FILE_DIR%\bin\plugins\*.dll
IF EXIST "%PACKAGE_FILE_DIR%\Open Source Software Notice.docx"          DEL /F  "%PACKAGE_FILE_DIR%\bin\Open Source Software Notice.docx"

rem==================1. set conf files begin======================

rem 生成version文件
echo %AGENT_VERSION%>%AGENTPATH%\conf\version
echo %AGENT_BUILD_NUM%>>%AGENTPATH%\conf\version

rem 更新插件版本信息
IF NOT EXIST %AGENTPATH%\conf\pluginmgr.xml   call :FileNotFound pluginmgr.xml
MOVE %AGENTPATH%\conf\pluginmgr.xml           %AGENTPATH%\conf\pluginmgr_bak.xml

for /f tokens^=4*^ delims^=^" %%i in ('type "%AGENTPATH%\conf\pluginmgr_bak.xml" ^| findstr "version=" ^| findstr "service="') do (
    set OLD_PLUGIN_VER=%%i
)

echo "OLD_PLUGIN_VER=%OLD_PLUGIN_VER%"


setlocal EnableDelayedExpansion

for /f "delims=" %%i in ('findstr .* "%AGENTPATH%\conf\pluginmgr_bak.xml"') do (
    set PLUGIN_LINE=%%i
    
    set PLUGIN_LINE=!PLUGIN_LINE:%OLD_PLUGIN_VER%=%AGENT_BUILD_NUM%!
    
    echo.!PLUGIN_LINE!>>%AGENTPATH%\conf\pluginmgr.xml  
)

DEL /F "%AGENTPATH%\conf\pluginmgr_bak.xml"

endlocal

echo  %DATE% %TIME% %1 Set nginx.conf file >> %BUILD_LOG%
rem==================1. set conf files end========================


rem==================2. copy bin file begin=======================

IF NOT EXIST %SOURCE_FILE_DIR%\rdagent.exe                           call :FileNotFound rdagent.exe
COPY /Y %SOURCE_FILE_DIR%\rdagent.exe                                %PACKAGE_FILE_DIR%\bin\rdagent.exe

IF NOT EXIST %SOURCE_FILE_DIR%\crypto.exe                            call :FileNotFound crypto.exe
COPY /Y %SOURCE_FILE_DIR%\crypto.exe                                 %PACKAGE_FILE_DIR%\bin\crypto.exe

IF NOT EXIST %SOURCE_FILE_DIR%\scriptsign.exe                        call :FileNotFound scriptsign.exe
COPY /Y %SOURCE_FILE_DIR%\scriptsign.exe                             %PACKAGE_FILE_DIR%\bin\scriptsign.exe

IF NOT EXIST %SOURCE_FILE_DIR%\datamigration.exe                     call :FileNotFound datamigration.exe
COPY /Y %SOURCE_FILE_DIR%\datamigration.exe                          %PACKAGE_FILE_DIR%\bin\datamigration.exe

IF NOT EXIST %SOURCE_FILE_DIR%\monitor.exe                           call :FileNotFound monitor.exe
COPY /Y %SOURCE_FILE_DIR%\monitor.exe                                %PACKAGE_FILE_DIR%\bin\monitor.exe

IF NOT EXIST %SOURCE_FILE_DIR%\winservice.exe                        call :FileNotFound winservice.exe
COPY /Y %SOURCE_FILE_DIR%\winservice.exe                             %PACKAGE_FILE_DIR%\bin\winservice.exe

IF NOT EXIST %SOURCE_FILE_DIR%\xmlcfg.exe                            call :FileNotFound xmlcfg.exe
COPY /Y %SOURCE_FILE_DIR%\xmlcfg.exe                                 %PACKAGE_FILE_DIR%\bin\xmlcfg.exe

IF NOT EXIST %SOURCE_FILE_DIR%\agentcli.exe                          call :FileNotFound agentcli.exe
COPY /Y %SOURCE_FILE_DIR%\agentcli.exe                               %PACKAGE_FILE_DIR%\bin\agentcli.exe

IF NOT EXIST %SOURCE_FILE_DIR%\getinput.exe                          call :FileNotFound getinput.exe
COPY /Y %SOURCE_FILE_DIR%\getinput.exe                               %PACKAGE_FILE_DIR%\bin\getinput.exe


IF NOT EXIST %SOURCE_FILE_DIR%\libcommon.dll                         call :FileNotFound libcommon.dll
COPY /Y %SOURCE_FILE_DIR%\libcommon.dll                              %PACKAGE_FILE_DIR%\bin\libcommon.dll

IF NOT EXIST %SOURCE_FILE_DIR%\rdvss.dll                             call :FileNotFound rdvss.dll
COPY /Y %SOURCE_FILE_DIR%\rdvss.dll                                  %PACKAGE_FILE_DIR%\bin\rdvss.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\libcluster.dll                call :FileNotFound libcluster.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\libcluster.dll                     %PACKAGE_FILE_DIR%\bin\plugins\libcluster-%AGENT_BUILD_NUM%.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\libdevice.dll                 call :FileNotFound libdevice.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\libdevice.dll                      %PACKAGE_FILE_DIR%\bin\plugins\libdevice-%AGENT_BUILD_NUM%.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\libexchange.dll               call :FileNotFound libexchange.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\libexchange.dll                    %PACKAGE_FILE_DIR%\bin\plugins\libexchange-%AGENT_BUILD_NUM%.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\libhost.dll                   call :FileNotFound libhost.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\libhost.dll                        %PACKAGE_FILE_DIR%\bin\plugins\libhost-%AGENT_BUILD_NUM%.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\liboracle.dll                 call :FileNotFound liboracle.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\liboracle.dll                      %PACKAGE_FILE_DIR%\bin\plugins\liboracle-%AGENT_BUILD_NUM%.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\libsqlserver.dll              call :FileNotFound libsqlserver.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\libsqlserver.dll                   %PACKAGE_FILE_DIR%\bin\plugins\libsqlserver-%AGENT_BUILD_NUM%.dll

IF NOT EXIST %SOURCE_FILE_DIR%\plugins\libapp.dll                    call :FileNotFound libapp.dll
COPY /Y %SOURCE_FILE_DIR%\plugins\libapp.dll                         %PACKAGE_FILE_DIR%\bin\plugins\libapp-%AGENT_BUILD_NUM%.dll

COPY /Y %SOURCE_FILE_DIR%\bat\thirdparty\sample\*                    %PACKAGE_FILE_DIR%\bin\thirdparty\sample\

COPY /Y %AGENTPATH%\third_party_groupware\msvc80\lib\*               %PACKAGE_FILE_DIR%\bin\

echo  %DATE% %TIME% %1 copy bin files >> %BUILD_LOG%

rem==================2. copy bin file end=======================

rem==================3. copy  nginx file begin=================

set NGINX=nginx
IF NOT EXIST %PACKAGE_FILE_DIR%\bin\nginx                 MKDIR  %PACKAGE_FILE_DIR%\bin\nginx
IF NOT EXIST %PACKAGE_FILE_DIR%\bin\nginx\conf          MKDIR  %PACKAGE_FILE_DIR%\bin\nginx\conf
IF NOT EXIST %PACKAGE_FILE_DIR%\bin\nginx\html          MKDIR  %PACKAGE_FILE_DIR%\bin\nginx\html
IF NOT EXIST %PACKAGE_FILE_DIR%\bin\nginx\logs          MKDIR  %PACKAGE_FILE_DIR%\bin\nginx\logs

IF NOT EXIST %AGENTPATH%\open_src\nginx_tmp\objs\nginx.exe      call :FileNotFound nginx.exe
rem rename nginx to rdnginx
COPY /Y %AGENTPATH%\open_src\nginx_tmp\objs\nginx.exe           %PACKAGE_FILE_DIR%\bin\nginx\rdnginx.exe

COPY /Y %AGENTPATH%\conf\*                          %PACKAGE_FILE_DIR%\bin\nginx\conf\
DEL /F %PACKAGE_FILE_DIR%\bin\nginx\conf\svn
DEL /F %PACKAGE_FILE_DIR%\bin\nginx\conf\*.xml
DEL /F %PACKAGE_FILE_DIR%\bin\nginx\conf\version
DEL /F %PACKAGE_FILE_DIR%\bin\nginx\conf\kmc*.txt

XCOPY /Y /E %AGENTPATH%\open_src\nginx_tmp\docs\html %PACKAGE_FILE_DIR%\bin\nginx\html

echo  %DATE% %TIME% %1 copy nginx files >> %BUILD_LOG%

rem==================3. copy  nginx file end==================


rem==================4. copy bat file begin====================
COPY /Y %SOURCE_FILE_DIR%\bat\*.bat %PACKAGE_FILE_DIR%\bin\

COPY /Y %SOURCE_FILE_DIR%\bat\*.cmd %PACKAGE_FILE_DIR%\bin\

COPY /Y %SOURCE_FILE_DIR%\bat\*.ps1 %PACKAGE_FILE_DIR%\bin\

COPY /Y %SOURCE_FILE_DIR%\bat\*.vbs %PACKAGE_FILE_DIR%\bin\

echo  %DATE% %TIME% %1 copy bat script files >> %BUILD_LOG%

rem==================4. copy bat file end======================


rem==================5. copy conf file begin===================

IF EXIST %AGENTPATH%\conf\svn ( COPY /Y %AGENTPATH%\conf\svn %PACKAGE_FILE_DIR%\conf\ )
COPY /Y %AGENTPATH%\conf\*.xml                                %PACKAGE_FILE_DIR%\conf\
COPY /Y %AGENTPATH%\conf\version                              %PACKAGE_FILE_DIR%\conf\
COPY /Y %AGENTPATH%\conf\KMC_Crt.conf                         %PACKAGE_FILE_DIR%\conf\

echo  %DATE% %TIME% %1 copy xml files in conf dir >> %BUILD_LOG%

rem==================5. copy conf file end=====================

rem==================6. copy db file begin=====================

COPY /Y %AGENTPATH%\selfdevelop\*.db                          %PACKAGE_FILE_DIR%\db\

echo  %DATE% %TIME% %1 copy db file in db dir >> %BUILD_LOG%

rem==================6. copy db file end=======================

rem==================7. gen sign file begin=====================

"%PACKAGE_FILE_DIR%\bin\scriptsign.exe"
DEL /F "%PACKAGE_FILE_DIR%\bin\scriptsign.exe"
DEL /F "%PACKAGE_FILE_DIR%\log\scriptsign.log"

echo  %DATE% %TIME% %1 gen script sign file in conf dir >> %BUILD_LOG%

rem==================7. gen sign file end=======================

rem==================8. copy Open Source Software Notice.docx============

COPY /Y "%AGENTPATH%build\copyRight\Open Source Software Notice.docx"  %PACKAGE_FILE_DIR%\

rem==================8. copy Open Source Software Notice.docx end========

rem==================9. copy 7zip begin========================

COPY /Y %AGENTPATH%\third_party_groupware\7Zip\*   %PACKAGE_FILE_DIR%\bin\

rem==================9. copy 7zip end==========================

rem==================10. initialize nginx config begin======================

set INPUT_PATH=%PACKAGE_FILE_DIR%\tmp\input
if exist %INPUT_PATH% (del /f /q %INPUT_PATH%)
echo BCM@DataProtect123>%INPUT_PATH%
for /f "delims=" %%i in ('call %PACKAGE_FILE_DIR%\bin\crypto.exe -a 0 -i %INPUT_PATH%') do (set NGINX_CONFIG=%%i)
if "%NGINX_CONFIG%" == "" (
    echo  %DATE% %TIME% %1 crypto NGINX_CONFIG failed.>>%BUILD_LOG% 
    if exist %INPUT_PATH% (del /f /q %INPUT_PATH%)
    exit 1
)

call %PACKAGE_FILE_DIR%\bin\xmlcfg.exe write Monitor nginx ssl_key_password "%NGINX_CONFIG%"

rem==================10. initialize nginx config end======================

rem==================10. initialize snmp config begin======================

if exist %INPUT_PATH% (del /f /q %INPUT_PATH%)
echo BCM@DataProtect8>%INPUT_PATH%
for /f "delims=" %%i in ('call %PACKAGE_FILE_DIR%\bin\crypto.exe -a 0 -i %INPUT_PATH%') do (set SNMP_CONFIG=%%i)
if "%SNMP_CONFIG%" == "" (
    echo  %DATE% %TIME% %1 crypto SNMP_CONFIG failed.>>%BUILD_LOG% 
    if exist %INPUT_PATH% (del /f /q %INPUT_PATH%)
    exit 1
)

call %PACKAGE_FILE_DIR%\bin\xmlcfg.exe write SNMP private_password "%SNMP_CONFIG%"

if exist %INPUT_PATH% (del /f /q %INPUT_PATH%)
echo BCM@DataProtect6>%INPUT_PATH%
for /f "delims=" %%i in ('call %PACKAGE_FILE_DIR%\bin\crypto.exe -a 0 -i %INPUT_PATH%') do (set SNMP_CONFIG=%%i)
if "%SNMP_CONFIG%" == "" (
    echo  %DATE% %TIME% %1 crypto SNMP_CONFIG failed.>>%BUILD_LOG% 
    if exist %INPUT_PATH% (del /f /q %INPUT_PATH%)
    exit 1
)

call %PACKAGE_FILE_DIR%\bin\xmlcfg.exe write SNMP auth_password "%SNMP_CONFIG%"
if exist %INPUT_PATH% (del /f /q %INPUT_PATH%)
del /f /q %PACKAGE_FILE_DIR%\log\*

rem==================10. initialize snmp config end========================

rem==================10. zip Agent begin========================

"C:\Program Files (x86)\7-Zip\7z.exe" a "%INSTALLSHIELD_PACKAGE_DIR%\%BIN_NAME_BASE%.zip" "%PACKAGE_FILE_DIR%\*"


echo  %DATE% %TIME% %1 zip Agent >> %BUILD_LOG%

rem==================10. zip Agent end==========================

echo Release Success. 
exit 0


rem Find the Visual Studio Tool's Installed Path by query enviroment
rem  %1: return value, the path where Visual Studio Tool installed, if not found, return "n/a"
rem
:FindAgentVERSION
    set VERSIONTemp=n/a

    rem Query the UltraAgent Version 
    set %1=n/a
    set fileWithInfo=%AGENTPATH%src\inc\common\AppVersion.h
    if not exist %fileWithInfo% (
        goto :eof
    )
    for /f tokens^=2*^ delims^=^" %%i in ('type %fileWithInfo% ^| findstr /C:%VERSION_PARAM%') do (
        rem Because the content of %%1 is a sentence, so we use "" to make it as a whole
        rem the side effect is the %1 become as "the content of %%1", in fact, "" is redundant
        set %1=%%i
    )
goto :eof

rem
rem Remove all the .svn directory and subdirectory in the agent code directory
rem
:RemoveSVNDIRECTORY
    
    %Drive%
    cd %AGENTPATH%\
    for /r . %%a in (.) do @if exist "%%a\.svn" @echo "%%a\.svn"
    for /r . %%a in (.) do @if exist "%%a\.svn" rd /s /q "%%a\.svn"
    for /r . %%a in (.) do @if exist "%%a\.gitignore" @echo "%%a\.gitignore"
    for /r . %%a in (.) do @if exist "%%a\.gitignore" rd /s /q "%%a\.gitignore"
    echo completed remove .svn file
    cd %CurBatPath%
    
goto :eof

:FileNotFound
echo  %DATE% %TIME% %1 Not Found >> %BUILD_LOG%
goto Exit 

:Clean
call %AGENTPATH%build\ms\agent_make.bat Clean64Openssl
goto Exit

:Exit
exit 1
            