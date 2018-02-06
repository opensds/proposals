@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


rem #######################################################
rem #
rem #  push upgrade: %~1=/r
rem #   
rem #######################################################

setlocal EnableDelayedExpansion

cd /d %~dp0
set NEED_LOG=1
set WORKING_USER=rdadmin
rem 1 for V1R3, 2 for V1R5
set UPGRADE_TYPE=0
set CUR_VERSION_VR=100005
set OLD_VER_V=100
rem contains calculation deviation
set /a FREE_SPACE_MIN=512
set CURRENT_PATH=%~dp0
set AGENT_BIN_PATH_FORLOG=%~dp0
set CMD_PAUSE=pause
set UNINSTALL_FLAG=/q
set INSTALL_FLAG=/up

rem -------------error code----------------------
set /a ERR_INPUT_PARAM_ERR=10
set /a ERR_DISK_FREE_ISLESS_500MB=13
set /a ERR_CHECK_INSTALLATION_PATH_FAILED=15
set /a ERR_CHECK_VSERSION_FAILED=20
set /a ERR_CHECK_WORKING_USER_FAILED=21
set /a ERR_AGENT_FILE_MIGRATE_FALIED=22
set /a ERR_AGENT_STOP_FAILED=23
set /a EER_AGENT_UNINSTALL_FAILED=24
set /a ERR_REGISTER_SERVICE_FALIED=25
set /a ERR_SET_CONF_FILE_FAILED=26
rem -------------error code----------------------

set AGENT_LOG_PATH=%CURRENT_PATH%..\log\
set LOGFILE_PATH=%AGENT_LOG_PATH%agent_upgrade.log

set UPGRADE_FLAG=%~1

if not "%UPGRADE_FLAG%" == "" (
    echo Input param is error.
    exit /b %ERR_INPUT_PARAM_ERR%

    if not "%UPGRADE_FLAG%" == "/r" (
        echo Input param is error.
        exit /b %ERR_INPUT_PARAM_ERR%
    )
)

if "%UPGRADE_FLAG%" == "/r" (
    set CMD_PAUSE=
    set UNINSTALL_FLAG=/r
    set INSTALL_FLAG=/up/r
    set AGENT_INSTALL_PACKAGE="OceanStor BCManager V"*_Agent-WIN64.zip
)

set AGENT_INSTALL_PATH=%CURRENT_PATH%\..\
set AGENT_BIN_PATH=%AGENT_INSTALL_PATH%\bin\

for /f "delims=" %%a in ('echo %CURRENT_PATH%\..') do (set AGENT_BACKUP_TMP=%%~fa)
set AGENT_BACKUP_PATH=%AGENT_BACKUP_TMP%_bak
 

if not "%UPGRADE_FLAG%" == "/r" (
    set REGULAR_PATH_FILE=%AGENT_INSTALL_PATH%tmp\path.info

    set PATH_REX="^[-_\\sA-Za-z0-9:\\]*$"

    rem Check installation path
    set PATH_TMP=%CURRENT_PATH: =%
    echo !PATH_TMP!>"!REGULAR_PATH_FILE!"
    
    for /f "delims=" %%i in ('findstr /i !PATH_REX! "!REGULAR_PATH_FILE!"') do (set PATH_CHECK=%%i)

    if exist "!REGULAR_PATH_FILE!"  del /f /q "!REGULAR_PATH_FILE!"

    if "!PATH_CHECK!" == "" (
        echo The upgrade path of OceanStor BCManager Agent can only contain characters '-_ a-zA-Z0-9'.
        echo The upgrade OceanStor BCManager Agent will be stopped.
        call :Log "The upgrade path of OceanStor BCManager Agent contain the unsupport characters, install failed."
        %CMD_PAUSE%
        exit 1
    )

    set /a NUMBER=0
    echo You are about to upgrade the OceanStor BCManager Agent. This operation suspends the OceanStor BCManager Agent, causing the applications on the host to be out of protection during the upgrade.
    echo.
    echo Suggestion: Upgrade the OceanStor BCManager Agent after confirming that no application is during the implementation of a protection task.
    echo.
        
    :makesureagentnoworks
        set UP_FLAG=1

        echo Please make sure OceanStor BCManager Agent is not working now. ^(y/n^):
        set /p UPGRADE_UP=">>"
            
        if "!UPGRADE_UP!" == "y"  set UP_FLAG=0
        if "!UPGRADE_UP!" == "n"  set UP_FLAG=0
        if "!UP_FLAG!" == "1" (
            set /a NUMBER+=1
            if !NUMBER! LSS 3 (
                echo Please enter y or n.
                goto :makesureagentnoworks
            ) else (
                echo Input invalid value over 3 times.
                echo The upgrade OceanStor BCManager Agent will be stopped.
                call :Log "The upgrade OceanStor BCManager Agent will be stopped."
                %CMD_PAUSE%
                exit 1
            )
        )
          
        if "!UPGRADE_UP!" == "n" (
            echo The upgrade OceanStor BCManager Agent will be stopped.
            call :Log "The upgrade OceanStor BCManager Agent will be stopped."
            %CMD_PAUSE%
            exit 1
        )
)

if exist "!REGULAR_PATH_FILE!"  del /f /q "!REGULAR_PATH_FILE!"

echo Begin to upgrade OceanStor BCManager Agent...
echo.

echo Check free space of installation path...
call :checkdiskspace

echo Check version of previous OceanStor BCManager Agent...
call :checkversion

echo Check Working user !WORKING_USER! of OceanStor BCManager Agent...
call :checkworkinguser

rem push upgrade
if "%UPGRADE_FLAG%" == "/r" (
    call :upgradeprepare
)

echo Check files in new OceanStor BCManager Agent...
call :checknewagentfiles

echo Check files in previous OceanStor BCManager Agent...
call :checkpreagentfilse

echo Stop previous OceanStor BCManager Agent...
call :stopprocess

echo Backup previous OceanStor BCManager Agent...
call :backupagentdir

echo Move the configure data from previous to new OceanStor BCManager Agent...
call :Log "Move the configure data from previous to new OceanStor BCManager Agent."
call :agentupgrade

echo Install new OceanStor BCManager Agent...
call :installagent

echo New OceanStor BCManager Agent installation path is %CURRENT_PATH%.
echo.
echo OceanStor BCManager Agent was upgraded successfully.

timeout /T 3 /NOBREAK >nul
call :Log "OceanStor BCManager Agent was upgraded successfully."

if not "%UPGRADE_FLAG%" == "/r" (
    start /b "" cmd /c del /q /f "%AGENT_BIN_PATH%agent_upgrade.bat" &exit /b 0
) else (
    del /q /f "%AGENT_BIN_PATH%agent_upgrade.bat"
)

exit /b 0


:upgradeprepare
    set NEW_INSTALL_PATH=!DRIVER_LETTER!\Program Files\Huawei\BCManager\Agent!NEW_VERSION!!NEW_BULID_NUM!
    if not exist "!NEW_INSTALL_PATH!\" (
        mkdir "!NEW_INSTALL_PATH!"
    ) else (
        for /f "tokens=1" %%i in ('2^>nul dir /a/b "!NEW_INSTALL_PATH!"') do (set DIR_ISEMPTY=%%i)
        if not "" == "!DIR_ISEMPTY!" (
            call :Log "Installation Path !NEW_INSTALL_PATH! is exist, and not empty, exit."
            exit %ERR_CHECK_INSTALLATION_PATH_FAILED%
        )
    )
    
    xcopy /e/h/v/q/y "!CURRENT_PATH!..\*"                  "!NEW_INSTALL_PATH!"   >nul
    
    move "!OLD_AGENT_BIN_PATH!agent_stop.bat"        "!OLD_AGENT_BIN_PATH!agent_stop.bat.bak" >nul
    move "!OLD_AGENT_BIN_PATH!agent_start.bat"       "!OLD_AGENT_BIN_PATH!agent_start.bat.bak" >nul
    move "!OLD_AGENT_BIN_PATH!agent_install.bat"     "!OLD_AGENT_BIN_PATH!agent_install.bat.bak" >nul
    move "!OLD_AGENT_BIN_PATH!agent_uninstall.bat"   "!OLD_AGENT_BIN_PATH!agent_uninstall.bat.bak" >nul
    
    xcopy /e/h/v/q/y "!CURRENT_PATH!agent_stop.bat"        "!OLD_AGENT_BIN_PATH!" >nul
    xcopy /e/h/v/q/y "!CURRENT_PATH!agent_start.bat"       "!OLD_AGENT_BIN_PATH!" >nul
    xcopy /e/h/v/q/y "!CURRENT_PATH!agent_install.bat"     "!OLD_AGENT_BIN_PATH!" >nul
    xcopy /e/h/v/q/y "!CURRENT_PATH!agent_uninstall.bat"   "!OLD_AGENT_BIN_PATH!" >nul
    
    set CURRENT_PATH=!NEW_INSTALL_PATH!\bin\
    
    set AGENT_INSTALL_PATH=!NEW_INSTALL_PATH!\
    set AGENT_BIN_PATH=%AGENT_INSTALL_PATH%bin\
    set AGENT_BACKUP_PATH=!DRIVER_LETTER!\Program Files\Huawei\BCManager\Agent!NEW_VERSION!!NEW_BULID_NUM!_bak
    
    goto :EOF

:checknewagentfiles
    rem check bin files
    call :Log "Check files in %AGENT_BIN_PATH% new OceanStor BCManager Agent."
    
    set CHECK_FILE_PATH=%AGENT_BIN_PATH%

    call :checkperfile agent_func.bat
    call :checkperfile agent_func.ps1
    call :checkperfile agent_install.bat
    call :checkperfile agent_start.bat
    call :checkperfile agent_stop.bat
    call :checkperfile agent_uninstall.bat
    call :checkperfile agentcli.exe
    call :checkperfile atl100.dll
    call :checkperfile crypto.exe
    call :checkperfile datamigration.exe
    call :checkperfile exchange.bat
    call :checkperfile getinput.exe
    call :checkperfile initiator.bat
    call :checkperfile install-provider.cmd
    call :checkperfile libcommon.dll
    call :checkperfile monitor.exe
    call :checkperfile msvcp100.dll
    call :checkperfile msvcr100.dll
    call :checkperfile online.bat
    call :checkperfile operexchangedb.ps1
    call :checkperfile oraasmaction.bat
    call :checkperfile oraclecheckarchive.bat
    call :checkperfile oracleconsistent.bat
    call :checkperfile oraclefunc.bat
    call :checkperfile oracleinfo.bat
    call :checkperfile oracleluninfo.bat
    call :checkperfile oracletest.bat
    call :checkperfile oradbaction.bat
    call :checkperfile process_start.bat
    call :checkperfile process_stop.bat
    call :checkperfile procmonitor.bat
    call :checkperfile queryexchangeinfo.ps1
    call :checkperfile rdagent.exe
    call :checkperfile rdvss.dll
    call :checkperfile register_app.vbs
    call :checkperfile sqlserverinfo.bat
    call :checkperfile sqlserverluninfo.bat
    call :checkperfile sqlserverrecover.bat
    call :checkperfile sqlserversample.bat
    call :checkperfile uninstall-provider.cmd
    call :checkperfile winservice.exe
    call :checkperfile xmlcfg.exe
    
    rem check plugins files
    set CHECK_FILE_PATH=%AGENT_BIN_PATH%plugins
    call :checkperfile libcluster-*.dll
    call :checkperfile libdevice-*.dll
    call :checkperfile libexchange-*.dll
    call :checkperfile libhost-*.dll
    call :checkperfile liboracle-*.dll
    call :checkperfile libsqlserver-*.dll
    call :checkperfile libapp-*.dll
    
    rem check conf files
    set CHECK_FILE_PATH=%AGENT_INSTALL_PATH%conf
    call :checkperfile agent_cfg.xml
    call :checkperfile pluginmgr.xml
    call :checkperfile script.sig
    
    set CHECK_FILE_PATH=%AGENT_BIN_PATH%nginx
    call :checkperfile rdnginx.exe
    
    set CHECK_FILE_PATH=%AGENT_BIN_PATH%nginx\html
    call :checkperfile 50x.html
    call :checkperfile index.html
    
    set CHECK_FILE_PATH=%AGENT_BIN_PATH%nginx\conf
    call :checkperfile fastcgi_params
    call :checkperfile nginx.conf
    rem if upgrage from v2r1, check the server.crt and server.key at 'bin/nginx/conf'
    if "!NEW_VER_VR!" == "200001" (
        call :checkperfile server.crt
        call :checkperfile server.key
        call :checkperfile bcmagentca.crt
    )
    
    goto :EOF
    
:checkpreagentfilse
    rem check v1r3
    call :Log "Check files in %OLD_AGENT_BIN_PATH% previous OceanStor BCManager Agent."

    if !UPGRADE_TYPE! EQU 1 (
        set CHECK_FILE_PATH=%OLD_AGENT_BIN_PATH%..\conf
        call :checkperfile rdagent.ini
        call :checkperfile rdagentSign.ini
        call :checkperfile rdagent.db
        call :checkperfile server.pem
        
        set CHECK_FILE_PATH=%OLD_AGENT_BIN_PATH%..\db
        call :checkperfile AgentDB.db
        
        set CHECK_FILE_PATH=%OLD_AGENT_BIN_PATH%
        call :checkperfile agent_func.bat
        call :checkperfile rdagentService.bat
        call :checkperfile install-provider.cmd
        call :checkperfile uninstall-provider.cmd
        call :checkperfile register_app.vbs
        
        goto :EOF
    )
    
    rem check v1r5
    set CHECK_FILE_PATH=%OLD_AGENT_BIN_PATH%..\conf
    call :checkperfile agent_cfg.xml
    call :checkperfile pluginmgr.xml
    
    set CHECK_FILE_PATH=%OLD_AGENT_BIN_PATH%nginx\conf
    call :checkperfile fastcgi_params
    call :checkperfile nginx.conf
    rem if upgrage from v1r5, check the server.pem at 'bin/nginx/conf'
    if "!CUR_VERSION_VR!" == "100005" (
        call :checkperfile server.pem
    )
    
    set CHECK_FILE_PATH=%OLD_AGENT_BIN_PATH%..\db
    call :checkperfile AgentDB.db
    
    set CHECK_FILE_PATH=%OLD_AGENT_BIN_PATH%
    call :checkperfile agent_func.bat
    call :checkperfile agent_install.bat
    call :checkperfile agent_uninstall.bat
    call :checkperfile register_app.vbs
    call :checkperfile agent_stop.bat
    call :checkperfile agent_start.bat
    call :checkperfile install-provider.cmd
    call :checkperfile uninstall-provider.cmd
        
    goto :EOF
    
:checkperfile
    set CHECK_FILE_NAME=%~1
    if not exist "!CHECK_FILE_PATH!\!CHECK_FILE_NAME!" (
        echo Check file !CHECK_FILE_NAME! failed, which is missed in OceanStor BCManager Agent.
        call :Log "Check file !CHECK_FILE_NAME! failed, which is missed in OceanStor BCManager Agent."
        echo The upgrade OceanStor BCManager Agent will be stopped.
        %CMD_PAUSE%
        call :deletefolder
        exit %ERR_AGENT_FILE_MIGRATE_FALIED%
    ) else (
        call :Log "Check file !CHECK_FILE_NAME! ....ok"
    )
    goto :EOF
    
:installagent
    if !UPGRADE_TYPE! EQU 2 (
        call :Log "Uninstall previous OceanStor BCManager Agent."
        call "!OLD_AGENT_BIN_PATH!agent_uninstall.bat" !UNINSTALL_FLAG! 1
        
        if not !errorlevel! EQU 0 (
            call :Log "Previous OceanStor BCManager Agent was uninstalled failed."
            echo Previous OceanStor BCManager Agent was uninstalled failed.
            echo Begin to roll back previous OceanStor BCManager Agent...
            call "!OLD_AGENT_BIN_PATH!agent_start.bat"
            
            if not !errorlevel! EQU 0  (
                echo Previous OceanStor BCManager Agent was rolled back failed.
                call :Log "Previous OceanStor BCManager Agent start failed, rollback failed."
            ) else (
                echo Previous OceanStor BCManager Agent was rolled back successfully.
                call :Log "Previous OceanStor BCManager Agent was rolled back successfully."
            )
            
            echo OceanStor BCManager Agent was upgraded failed.
            call :Log "OceanStor BCManager Agent was upgraded failed."
            %CMD_PAUSE%
            call :deletefolder
            exit %EER_AGENT_UNINSTALL_FAILED%
        ) 
    )
    
    echo Begin to install new OceanStor BCManager Agent...
    call :Log "Install new OceanStor BCManager Agent, install flag is !INSTALL_FLAG!, OLD_VER_V is !OLD_VER_V!."

    call "%AGENT_BIN_PATH%agent_install.bat" !INSTALL_FLAG! !OLD_VER_V! /up
    set /a ERR_LEVEL=!errorlevel!
    
    if not !ERR_LEVEL! EQU 0 (
        goto :gotorollback
    )
    
    if !UPGRADE_TYPE! EQU 2 (
        call "%AGENT_BIN_PATH%install-provider.cmd" >> "%LOGFILE_PATH%"
        if not !errorlevel! EQU 0 (
            set /a ERR_LEVEL=%ERR_REGISTER_SERVICE_FALIED%
            goto :gotorollback
        )
        
        call :Log "Delete previous OceanStor BCManager Agent and bak dir."

        if exist "!OLD_AGENT_ROOT_PATH!" (
            call :Log "Delete dir !OLD_AGENT_ROOT_PATH!."
            rd /s /q  "!OLD_AGENT_ROOT_PATH!"
        )
        
        if exist "!OLD_AGENT_ROOT_PATH!_bak" (
            call :Log "Delete dir !OLD_AGENT_ROOT_PATH!_bak."
            rd /s /q  "!OLD_AGENT_ROOT_PATH!_bak"
        )
        
        goto :EOF
    )

    echo Begin to uninstall previous OceanStor BCManager Agent...
    call :Log "Uninstall previous OceanStor BCManager Agent."
    call "C:\Program Files (x86)\InstallShield Installation Information\{D4280104-0E77-47E3-9986-D5C91452341F}\setup.exe" -runfromtemp -l0x0409/M{D4280104-0E77-47E3-9986-D5C91452341F}
        
    :getinput
    echo Make sure previous OceanStor BCManager Agent was uninstalled completed? ^(y/n^):
    set /p UNINSTALL=">>"
        
    if not "!UNINSTALL!" == "y" (
        goto :getinput
    )
    
    echo.
    echo Register Service provider of new OceanStor BCManager Agent... 
    call "%AGENT_BIN_PATH%install-provider.cmd" >> "%LOGFILE_PATH%"
    if not !errorlevel! EQU 0 (
        echo Previous OceanStor BCManager Agent was installed failed, rollback failed.
        call :Log "Previous OceanStor BCManager Agent was installed failed, rollback failed."
        %CMD_PAUSE%
        exit %ERR_REGISTER_SERVICE_FALIED%
    )

    goto :EOF

:gotorollback
    echo New OceanStor BCManager Agent was installed failed.
    echo Begin to roll back previous OceanStor BCManager Agent...
    call :Log "New OceanStor BCManager Agent was installed failed, begin rollback."
        
    call "%AGENT_BIN_PATH%agent_uninstall.bat" %UNINSTALL_FLAG% 1
    if not !errorlevel! EQU 0 (
        echo New OceanStor BCManager Agent was uninstalled failed, rollback failed.
        call :Log "New OceanStor BCManager Agent was uninstalled failed, rollback failed."
        echo OceanStor BCManager Agent was upgraded failed.
        call :Log "OceanStor BCManager Agent was upgraded failed."
        %CMD_PAUSE%
        call :deletefolder
        exit !ERR_LEVEL!
    )
    
    if !UPGRADE_TYPE! EQU 1 (
        call "!OLD_AGENT_BIN_PATH!rdagentService.bat" start
        if not !errorlevel! EQU 0 (
            echo Previous OceanStor BCManager Agent was started failed, rollback failed.
            call :Log "Previous OceanStor BCManager Agent was started failed, rollback failed."
            echo OceanStor BCManager Agent was upgraded failed.
            call :Log "OceanStor BCManager Agent was upgraded failed."
            %CMD_PAUSE%
            exit !ERR_LEVEL!
        )
    ) else (
        call "!OLD_AGENT_BIN_PATH!agent_install.bat" %INSTALL_FLAG% !OLD_VER_V! /rb
        if not !errorlevel! EQU 0 (
            echo Previous OceanStor BCManager Agent was installed failed, rollback failed.
            call :Log "Previous OceanStor BCManager Agent was installed failed, rollback failed."
        )
            
        call "!OLD_AGENT_BIN_PATH!install-provider.cmd" >> "%LOGFILE_PATH%"
        if not !errorlevel! EQU 0 (
            echo Previous OceanStor BCManager Agent was installed failed, rollback failed.
            call :Log "Previous OceanStor BCManager Agent was installed failed, rollback failed."
        )
    )        
        
    echo Previous OceanStor BCManager Agent was rolled back successfully.
    call :Log "Previous OceanStor BCManager Agent was rolled back successfully."
    echo OceanStor BCManager Agent was upgraded failed.
    call :Log "OceanStor BCManager Agent was upgraded failed."
    %CMD_PAUSE%
    call :deletefolder
    exit !ERR_LEVEL!

:agentupgrade
    if !UPGRADE_TYPE! EQU 2 (
        call :gev1r5config
        call :Log "Move the configure data from previous to new OceanStor BCManager Agent successfully."
        
        goto :EOF
    )
    
    call :v1r3config
    call :Log "Move the configure data from previous to new OceanStor BCManager Agent successfully."
    
    goto :EOF
    
:v1r3config
    call :setconfvalue LOG_LEVEL log_level System
    
    call :setconfvalue LOG_COUNT log_count System
    
    call :setconfvalue THREADCNT thread_count Monitor rdagent
    call :setconfvalue THREADCNT thread_count Monitor nginx
    
    call :setconfvalue HANDLECNT handle_count Monitor rdagent
    call :setconfvalue HANDLECNT handle_count Monitor nginx
    
    call :setconfvalue PYHMEMSIZE pm_size Monitor rdagent
    call :setconfvalue PYHMEMSIZE pm_size Monitor nginx
    
    call :setconfvalue VIRMEMSIZE vm_size Monitor rdagent
    call :setconfvalue VIRMEMSIZE vm_size Monitor nginx
    
    call :setconfvalue CPUUSAGE cpu_usage Monitor rdagent
    call :setconfvalue CPUUSAGE cpu_usage Monitor nginx
    
    call :setconfvalue TMPFILETOTALSIZE tmpfile_size Monitor rdagent
    
    call :setconfvalue BASERETRYTIMES retry_time Monitor
    
    call :setconfvalue BASECYCTIME monitor_interval Monitor
    
    call :setconfvalue SECURITYLEVEL security_level SNMP
    
    call :setconfvalue CONTEXTNAME context_name SNMP
    
    call :setconfvalue CONTEXTENGINEID engine_id SNMP
    
    call :setconfvalue PRIVATEPWD private_password SNMP
    
    call :setconfvalue AUTHPWD auth_password SNMP
    
    call :setconfvalue AUTHPROTOCOL auth_protocol SNMP
    
    call :setconfvalue PRIVATEPROTOCOL private_protocol SNMP
    
    call :setconfvalue SECURITYNAME security_name SNMP
    
    call :setconfvalue SECURITYMODEL security_model SNMP
    
    for /f "tokens=1,2 delims== " %%i in ('2^>nul type "%AGENT_BACKUP_PATH%\conf\rdagentSign.ini" ^| findstr SALT') do (set AGENT_SALT=%%j)
    
    set /p AGENT_NAME=<"%AGENT_BACKUP_PATH%\conf\rdagent.db"
    for /f "delims=" %%i in ('type "%AGENT_BACKUP_PATH%\conf\rdagent.db"') do (set AGENT_HASH=%%i)
    
    call "%AGENT_INSTALL_PATH%\bin\xmlcfg.exe" write System sl !AGENT_SALT!
    call "%AGENT_INSTALL_PATH%\bin\xmlcfg.exe" write System name !AGENT_NAME!
    call "%AGENT_INSTALL_PATH%\bin\xmlcfg.exe" write System hash !AGENT_HASH!
    
    for /f "tokens=1,2 delims== " %%i in ('2^>nul type "%AGENT_BACKUP_PATH%\conf\rdagent.ini" ^| findstr IP') do (set IP_ADDRESS=%%j)
    for /f "tokens=1,2 delims== " %%i in ('2^>nul type "%AGENT_BACKUP_PATH%\conf\rdagent.ini" ^| findstr PORT') do (set NGINX_PORT=%%j)
    for /f "tokens=1,2 delims== " %%i in ('2^>nul type "%AGENT_BACKUP_PATH%\conf\rdagent.ini" ^| findstr KEY_PWD') do (set VKEY_PWD=%%j)
    
    if not "!IP_ADDRESS!" == "" (set IP_ADDRESS=!IP_ADDRESS: =!)
    
    set PARAM_NAME=listen
    call :modifyconffile
    set PARAM_NAME=fastcgi_pass
    call :modifyconffile
    
    call :Log "Set nginx.config successfully."
    
    if exist "%AGENT_BACKUP_PATH%\conf\HostSN" (
        xcopy /e/h/v/q/y "%AGENT_BACKUP_PATH%\conf\HostSN"   "%AGENT_INSTALL_PATH%\conf\">nul
        echo.>>"%AGENT_INSTALL_PATH%\conf\HostSN"
    )
    
    
    if exist "%AGENT_INSTALL_PATH%\db\AgentDB.db"                  del /f /q "%AGENT_INSTALL_PATH%\db\AgentDB.db"
    xcopy /e/h/v/q/y "%AGENT_BACKUP_PATH%\db\AgentDB.db"         "%AGENT_INSTALL_PATH%\db\">nul
    call :Log "Move AgentDB.db to new OceanStor BCManager Agent."

    rem create table
    call "%AGENT_INSTALL_PATH%\bin\datamigration.exe" table
    set ERR_RESULT=!errorlevel!
    
    rem reencrypt db record
    call "%AGENT_INSTALL_PATH%\bin\datamigration.exe" record
    set ERR_TMP=!errorlevel!
    if "!ERR_RESULT!" == "0" (set ERR_RESULT=!ERR_TMP!)
    
    rem reencrypt conf passwd
    call "%AGENT_INSTALL_PATH%\bin\datamigration.exe" pass
    set ERR_TMP=!errorlevel!
    if "!ERR_RESULT!" == "0" (set ERR_RESULT=!ERR_TMP!)
    
    call "%AGENT_INSTALL_PATH%\bin\datamigration.exe" salt
    set ERR_TMP=!errorlevel!
    if "!ERR_RESULT!" == "0" (set ERR_RESULT=!ERR_TMP!)
    
    if not "!ERR_RESULT!" == "0" (
        echo Datas migrate to new OceanStor BCManager Agent was failed.
        call :Log "Datas migrate to new OceanStor BCManager Agent was failed."
        
        echo The previous OceanStor BCManager Agent is starting...
        call "!OLD_AGENT_BIN_PATH!rdagentService.bat" start
        if not "!errorlevel!" == "0" (
            echo The previous OceanStor BCManager Agent start failed, rollback failed.
            call :Log "The previous OceanStor BCManager Agent start failed, rollback failed."
        )
        
        call :deletefolder
        if exist "!NEW_INSTALL_PATH!_bak"    rd /s /q "!NEW_INSTALL_PATH!_bak"
        echo The upgrade OceanStor BCManager Agent will be stopped.
        %CMD_PAUSE%
        exit %ERR_AGENT_FILE_MIGRATE_FALIED% 
    )
    
    if exist "%AGENT_BACKUP_PATH%\bin\thirdparty\" (
        if exist "%AGENT_INSTALL_PATH%\bin\thirdparty\"            del /f /q "%AGENT_INSTALL_PATH%\bin\thirdparty\*"
        xcopy /e/h/v/q/y "%AGENT_BACKUP_PATH%\bin\thirdparty\*"  "%AGENT_INSTALL_PATH%\bin\thirdparty\">nul
    )
    call :Log "Move third scripts to new OceanStor BCManager Agent."
    
    goto :EOF
    
:setconfvalue
    set ARG_NAME=%~1
    set INPUT_NAME=%~2
    set PARENT_TAG=%~3
    set CHILD_TAG=%~4
    set ARG_VALUE=
    for /f "tokens=1,2 delims== " %%i in ('2^>nul type "%AGENT_BACKUP_PATH%\conf\rdagent.ini" ^| findstr !ARG_NAME!') do (set ARG_VALUE=%%j)
    
    call "%AGENT_INSTALL_PATH%\bin\xmlcfg.exe" write !PARENT_TAG! !CHILD_TAG! !INPUT_NAME! !ARG_VALUE!
    goto :EOF
    
:modifyconffile
    set ISPECIL_NUM=0
    set IFLAF_NUM=0
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "%AGENT_INSTALL_PATH%\bin\nginx\conf\nginx.conf"') do (
        for /f %%j in ('2^>nul echo.%%b ^| findstr  !PARAM_NAME!') do (
            set IFLAF_NUM=%%a
        )

        for /f %%j in ('2^>nul echo.%%b ^| findstr  ssl_ciphers') do (
            set ISPECIL_NUM=%%a
        )
    )
    
    if !IFLAF_NUM! equ 0 (
        echo Move the configure data from previous to new OceanStor BCManager Agent failed.
        call :Log "Do not find the !PARAM_NAME! in the nginx.conf, exit 1."
        echo The upgrade OceanStor BCManager Agent will be stopped.
        %CMD_PAUSE%
        exit 1
    )
    
    if !ISPECIL_NUM! equ 0 (
        call :Log "Don't find the ssl_ciphers in the nginx.conf, exit 1."
        call :Log "Do not find the ssl_ciphers in the nginx.conf, exit 1."
        echo The upgrade OceanStor BCManager Agent will be stopped.
        %CMD_PAUSE%
        exit 1
    )
        
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "%AGENT_INSTALL_PATH%\bin\nginx\conf\nginx.conf"') do (
        if %%a equ %IFLAF_NUM% (
            if "!PARAM_NAME!" == "listen" (
                echo.        !PARAM_NAME!       %IP_ADDRESS%:%NGINX_PORT% ssl;>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
            ) else if "!PARAM_NAME!" == "ssl_certificate_key_password" (
                echo.        !PARAM_NAME! %VKEY_PWD%;>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
            ) else (
                echo.            !PARAM_NAME!   127.0.0.1:!AGENT_PORT!;>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
            )
        ) else if %%a equ %ISPECIL_NUM% (
            echo.        ssl_ciphers  ECDHE-RSA-AES256-SHA384:AES256-SHA256:HIGH:^^^!DES:^^^!MD5:^^^!3DES:^^^!EDH:^^^!aNULL:^^^!AESGCM:^^^!CAMELLIA:^^^!DSS;>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
        ) else (
            echo.%%b >>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
        )  
    )
    
    move "%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak" "%AGENT_INSTALL_PATH%\bin\nginx\conf\nginx.conf" >nul
    
    goto :EOF
    
:gev1r5config
    call :setV1R5confvalue port System
    call :setV1R5confvalue log_level System
    call :setV1R5confvalue log_count System
    call :setV1R5confvalue name System
    call :setV1R5confvalue sl System
    call :setV1R5confvalue hash System
    
    call :setV1R5confvalue retry_time Monitor
    call :setV1R5confvalue monitor_interval Monitor
    call :setV1R5confvalue rdagent Monitor
    call :setV1R5confvalue thread_count Monitor rdagent
    call :setV1R5confvalue handle_count Monitor rdagent
    call :setV1R5confvalue pm_size Monitor rdagent
    call :setV1R5confvalue vm_size Monitor rdagent
    call :setV1R5confvalue cpu_usage Monitor rdagent
    call :setV1R5confvalue tmpfile_size Monitor rdagent
    
    call :setV1R5confvalue nginx Monitor
    call :setV1R5confvalue thread_count Monitor nginx
    call :setV1R5confvalue handle_count Monitor nginx
    call :setV1R5confvalue pm_size Monitor nginx
    call :setV1R5confvalue vm_size Monitor nginx
    call :setV1R5confvalue cpu_usage Monitor nginx
    
    call :setV1R5confvalue context_name SNMP
    call :setV1R5confvalue engine_id SNMP
    call :setV1R5confvalue private_password SNMP
    call :setV1R5confvalue private_protocol SNMP
    call :setV1R5confvalue auth_password SNMP
    call :setV1R5confvalue auth_protocol SNMP
    call :setV1R5confvalue security_level SNMP
    call :setV1R5confvalue security_name SNMP
    call :setV1R5confvalue security_model SNMP
    
    if !OLD_VER_V! GEQ 200 (
        call :setV1R5confvalue log_cache_threshold System
        call :setV1R5confvalue nginx_log_size Monitor nginx
    )
    
    if exist "%AGENT_BACKUP_PATH%\conf\HostSN"    copy /y "%AGENT_BACKUP_PATH%\conf\HostSN"   "%AGENT_INSTALL_PATH%\conf\">nul
    call :Log "Move agent conf files to new OceanStor BCManager Agent."
    
    del /f /q "%AGENT_INSTALL_PATH%\db\*"
    copy /y "%AGENT_BACKUP_PATH%\db\*"  "%AGENT_INSTALL_PATH%\db\">nul
    call :Log "Move file AgentDB.db to new OceanStor BCManager Agent."
    
    if exist "%AGENT_BACKUP_PATH%\bin\thirdparty\" (
        if exist "%AGENT_INSTALL_PATH%\bin\thirdparty"               del /f/q "%AGENT_INSTALL_PATH%\bin\thirdparty\*"
        xcopy /e/h/v/q/y "%AGENT_BACKUP_PATH%\bin\thirdparty\*"    "%AGENT_INSTALL_PATH%\bin\thirdparty\">nul
    )
    call :Log "Move third scripts to new OceanStor BCManager Agent." 
    
    if !OLD_VER_V! GEQ 200 (
        copy /y "%AGENT_BACKUP_PATH%\bin\nginx\conf\kmc*.txt"  "%AGENT_BIN_PATH%nginx\conf\" 1>nul 2>nul
        copy /y "%AGENT_BACKUP_PATH%\conf\kmc*.txt"            "%AGENT_INSTALL_PATH%\conf\" 1>nul 2>nul
        call :Log "Copy kmc files to new OceanStor BCManager Agent succ." 
    )
    
    call :Log "Move file nginx conf files to new OceanStor BCManager Agent."
    
    rem read old nginx.conf
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "%AGENT_BACKUP_PATH%\bin\nginx\conf\nginx.conf"') do (
        for /f %%j in ('2^>nul echo.%%b ^| findstr  listen') do (
            set STR_LISTEN=%%b 
        )
                
        for /f "tokens=1,2 delims= " %%j in ('2^>nul echo.%%b ^| findstr /C:"ssl_certificate "') do (
            set STR_CRT_FILE=%%b  
        )
        
        for /f "tokens=1,2 delims= " %%j in ('2^>nul echo.%%b ^| findstr /C:"ssl_certificate_key "') do (
            set STR_CRT_KEY_FILE=%%b  
        )
        
        for /f %%j in ('2^>nul echo.%%b ^| findstr  fastcgi_pass') do (
            set STR_FASTCGI_PASS=%%b 
        )
    )
    
    rem set new nginx.conf
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "%AGENT_INSTALL_PATH%\bin\nginx\conf\nginx.conf"') do (
        for /f %%j in ('2^>nul echo.%%b ^| findstr  listen') do (
            set LISTEN_NUM=%%a
        )
        
        for /f %%j in ('2^>nul echo.%%b ^| findstr /C:"ssl_certificate "') do (
            set CRT_FILE_NUM=%%a
        )
        
        for /f %%j in ('2^>nul echo.%%b ^| findstr /C:"ssl_certificate_key "') do (
            set CRT_KEY_FILE_NUM=%%a
        )
        
        for /f %%j in ('2^>nul echo.%%b ^| findstr  fastcgi_pass') do (
            set FASTCGI_PASS_NUM=%%a
        )

        for /f %%j in ('2^>nul echo.%%b ^| findstr  ssl_ciphers') do (
            set SSL_CIPHERS_NUM=%%a
        )
    )
    
    rem current version is v2r1, need to replace certificate, certificate key and certificate key password with current version information
    rem current version is v1r5, do not to replace information, but using the new install version information
    if "!CUR_VERSION_VR!" == "200001" (
        for /f "tokens=1-3 delims=; " %%a in ("%STR_CRT_FILE%") do (
            set CRT_FILE=%%b
        )
        
        for /f "tokens=1-3 delims=; " %%a in ("%STR_CRT_KEY_FILE%") do (
            set CRT_KEY_FILE=%%b
        )
        
        rem check crt and key file exists
        set /a FLG_FILE=0
        if not exist "!OLD_AGENT_BIN_PATH!/nginx/conf/!CRT_FILE!" (
            call :Log "!OLD_AGENT_BIN_PATH!/nginx/conf/!CRT_FILE! is not exists."
            echo "!OLD_AGENT_BIN_PATH!/nginx/conf/!CRT_FILE! is not exists."
            set /a FLG_FILE=1
        )
        
        if not exist "!OLD_AGENT_BIN_PATH!/nginx/conf/!CRT_KEY_FILE!" (
            call :Log "!OLD_AGENT_BIN_PATH!/nginx/conf/!CRT_KEY_FILE! is not exists."
            echo "!OLD_AGENT_BIN_PATH!/nginx/conf/!CRT_KEY_FILE! is not exists."
            set /a FLG_FILE=1
        )
        
        if !FLG_FILE! equ 1 (
            echo The previous OceanStor BCManager Agent is starting...
            call "!OLD_AGENT_BIN_PATH!agent_start.bat"
            if not "!errorlevel!" == "0" (
                echo The previous OceanStor BCManager Agent start failed, rollback failed.
                call :Log "The previous OceanStor BCManager Agent start failed, rollback failed."
            )
            
            call :deletefolder
            if exist "!NEW_INSTALL_PATH!_bak"    rd /s /q "!NEW_INSTALL_PATH!_bak"
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_AGENT_FILE_MIGRATE_FALIED% 
        ) else (
            xcopy /e/h/v/q/y "!OLD_AGENT_BIN_PATH!\nginx\conf\!CRT_FILE!"       "%AGENT_INSTALL_PATH%\bin\nginx\conf\">nul
            xcopy /e/h/v/q/y "!OLD_AGENT_BIN_PATH!\nginx\conf\!CRT_KEY_FILE!"   "%AGENT_INSTALL_PATH%\bin\nginx\conf\">nul
        )
    )
    
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "%AGENT_INSTALL_PATH%\bin\nginx\conf\nginx.conf"') do (
        if %%a equ %LISTEN_NUM% (
            echo.%STR_LISTEN%>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
            call :Log "Reconfig listen in nginx.conf ....ok"
        ) else if %%a equ %SSL_CIPHERS_NUM% (
            echo.        ssl_ciphers  ECDHE-RSA-AES256-SHA384:AES256-SHA256:HIGH:^^^!DES:^^^!MD5:^^^!3DES:^^^!EDH:^^^!aNULL:^^^!AESGCM:^^^!CAMELLIA:^^^!DSS;>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
            call :Log "Reconfig ssl_ciphers in nginx.conf ....ok"
        ) else if %%a equ %FASTCGI_PASS_NUM% (
            echo.%STR_FASTCGI_PASS%>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
            call :Log "Reconfig fastcgi_pass in nginx.conf ....ok"
        ) else (
            rem current version is v2r1, need to replace certificate, certificate key and certificate key password with current version information
            rem current version is v1r5, do not to replace information, but using the new install version information
            if "!CUR_VERSION_VR!" == "200001" (
                if %%a equ %CRT_FILE_NUM% (
                    echo.%STR_CRT_FILE%>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
                    call :Log "Reconfig ssl_certificate in nginx.conf ....ok"
                ) else if %%a equ %CRT_KEY_FILE_NUM% (
                    echo.%STR_CRT_KEY_FILE%>>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
                    call :Log "Reconfig ssl_certificate_key in nginx.conf ....ok"
                ) else (
                    echo.%%b >>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
                )
            ) else (
                echo.%%b >>"%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak"
            )
        )
    )
    
    move "%AGENT_INSTALL_PATH%\tmp\nginx.conf.bak" "%AGENT_INSTALL_PATH%\bin\nginx\conf\nginx.conf" >nul
    
    if !OLD_VER_V! LSS 200 (
        rem reencrypt db record
        call "%AGENT_INSTALL_PATH%\bin\datamigration.exe" record
        set ERR_RESULT=!errorlevel!
        
        rem reencrypt conf passwd
        call "%AGENT_INSTALL_PATH%\bin\datamigration.exe" pass
        set ERR_TMP=!errorlevel!
        if "!ERR_RESULT!" == "0" (set ERR_RESULT=!ERR_TMP!)
        
        if not "!ERR_RESULT!" == "0" (
            echo Datas migrate to new OceanStor BCManager Agent was failed.
            call :Log "Datas migrate from !OLD_VERSION! to !NEW_VERSION! was failed."
            
            echo The previous OceanStor BCManager Agent is starting...
            call "!OLD_AGENT_BIN_PATH!agent_start.bat"
            if not "!errorlevel!" == "0" (
                echo The previous OceanStor BCManager Agent start failed, rollback failed.
                call :Log "The previous OceanStor BCManager Agent start failed, rollback failed."
            )
            
            call :deletefolder
            if exist "!NEW_INSTALL_PATH!_bak"    rd /s /q "!NEW_INSTALL_PATH!_bak"
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_AGENT_FILE_MIGRATE_FALIED% 
        )
    )
    
    goto :EOF
    
:setV1R5confvalue
    set INPUT_NAME=%~1
    set PARENT_TAG=%~2
    set CHILD_TAG=%~3
    set ARG_VALUE=
    for /f %%i in ('call "!OLD_AGENT_BIN_PATH!\xmlcfg.exe" read !PARENT_TAG! !CHILD_TAG! !INPUT_NAME!') do (set ARG_VALUE=%%i)
    
    call "%AGENT_INSTALL_PATH%\bin\xmlcfg.exe" write !PARENT_TAG! !CHILD_TAG! !INPUT_NAME! !ARG_VALUE!
    call :Log "Reconfig !CHILD_TAG! !PARENT_TAG! !INPUT_NAME! in agent_cfg.xml ....ok"
    goto :EOF
    
    
:backupagentdir
    if exist "%AGENT_BACKUP_PATH%"    rd /s/q "%AGENT_BACKUP_PATH%"
    mkdir "%AGENT_BACKUP_PATH%"
    
    xcopy /e/h/v/q/y "!OLD_AGENT_BIN_PATH!\..\*"   "%AGENT_BACKUP_PATH%">nul
    
    call :Log "Backup previous OceanStor BCManager Agent to %AGENT_BACKUP_PATH%."
    
    goto :EOF

:stopprocess
    rem stop V1R3
    call :Log "Stop previous OceanStor BCManager Agent."
    if "!UPGRADE_TYPE!" == "1" (
        
        call "!OLD_AGENT_BIN_PATH!rdagentService.bat" stop
        if not "!errorlevel!" == "0" (
            echo The previous OceanStor BCManager Agent stop failed.
            call :Log "The previous OceanStor BCManager Agent stop failed."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_AGENT_STOP_FAILED%
        )
        
        call :Log "Previous OceanStor BCManager Agent stop successfully."
        goto :EOF
    )
    
    rem stop V1R5 and later
    call "!OLD_AGENT_BIN_PATH!agent_stop.bat"
    if not "!errorlevel!" == "0" (
        echo The previous OceanStor BCManager Agent stop failed.
        call :Log "The previous OceanStor BCManager Agent stop failed."
        echo The upgrade OceanStor BCManager Agent will be stopped.
        %CMD_PAUSE%
        call :deletefolder
        exit %ERR_AGENT_STOP_FAILED%
    )
    
    call :Log "Previous OceanStor BCManager Agent stop successfully."
    goto :EOF
    
:checkworkinguser
    rem 200:V200R001
    if !OLD_VER_V! LSS 200 (
        net user !WORKING_USER! 1>nul 2>nul
        if "!errorlevel!" == "0" (
            call :Log "Working user !WORKING_USER! is exist in !OLD_VERSION! of OceanStor BCManager Agent, then exist."
            echo Working user !WORKING_USER! is exist in !OLD_VERSION! of OceanStor BCManager Agent.
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_WORKING_USER_FAILED%
        )
    ) else (
        net user !WORKING_USER! 1>nul 2>nul
        if not "!errorlevel!" == "0" (
            call :Log "Working user !WORKING_USER! is not exist in !OLD_VERSION! of OceanStor BCManager Agent, then exist."
            echo Working user !WORKING_USER! is not exist in !OLD_VERSION! of OceanStor BCManager Agent.
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_WORKING_USER_FAILED%
        )
    )
    
    goto :EOF

:checkversion
    rem Check new Agent, which is ready to install 
    for /f "tokens=1,2 delims=: " %%i in ('2^>nul "!AGENT_BIN_PATH!agentcli.exe" show ^| findstr /i Version') do (set NEW_VERSION=%%j)
    for /f "tokens=1,2,3,4 delims=VRCBSPT" %%a in ('echo !NEW_VERSION!') do (
        set NEW_VER_V=%%a
        set NEW_VER_VR=%%a%%b
        set NEW_VER_VRC=%%a%%b%%c
        set NEW_VER_VRCSPC=%%a%%b%%c%%d
    )
    
    for /f "tokens=1,2 delims=:" %%i in ('2^>nul "!AGENT_BIN_PATH!agentcli.exe" show ^| findstr /i "Build^ Number"') do (set NEW_BULID_NUM=%%j)
    set NEW_BULID_NUM=!NEW_BULID_NUM: =!
    set NEW_BULID_NUM=!NEW_BULID_NUM:.=!

    if "!NEW_VERSION!" == "" (
        echo Query version of current OceanStor BCManager Agent failed.
        call :Log "Query version of current OceanStor BCManager Agent failed."
        echo The upgrade OceanStor BCManager Agent will be stopped.
        %CMD_PAUSE%
        exit %ERR_CHECK_VSERSION_FAILED%
    )
    
    call :Log "Get the version !NEW_VERSION! of current OceanStor BCManager Agent."

    rem Get old version of installed Agent
    set OLD_VERSION=
    set CHECK_VER_VRC=
    set CHECK_VER_VRCSPC=

    rem Check V1R5,or later
    set CHECK_VERSION=rdagent
    call :queryagentpath

    if "!OLD_AGENT_BIN_PATH!" == "" (
        rem V1R5 is not exist.Check V1R3
        if "%UPGRADE_FLAG%" == "/r" (
            echo NO supported OceanStor BCManager Agent to upgrade in this System.
            call :Log "NO supported OceanStor BCManager Agent to upgrade in this System."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )

        set CHECK_VERSION=ReplicationDirector Agent
        set QUOTATION_MARKS=
        call :queryagentpath
        if "!OLD_AGENT_BIN_PATH!" == "" (
            echo NO supported OceanStor BCManager Agent to upgrade in this System.
            call :Log "NO supported OceanStor BCManager Agent to upgrade in this System."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )

        if not exist "!OLD_AGENT_BIN_PATH!rdagent.exe" (
            echo File rdagent.exe was missed in previous OceanStor BCManager Agent.
            call :Log "rdagent.exe was missed in previous OceanStor BCManager Agent."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )

        for /f "tokens=1,2 delims=: " %%i in ('2^>nul "!OLD_AGENT_BIN_PATH!rdagent.exe" -v ^| findstr /i Version:') do (set OLD_VERSION=%%j)
        for /f "tokens=1,2,3 delims=VRCSPB" %%i in ('echo !OLD_VERSION!') do (set OLD_VERSION_NUM=%%i%%j%%k)
        if "!OLD_VERSION_NUM!" == "" (
            echo Check version of previous OceanStor BCManager Agent failed.
            call :Log "Check Version of previous OceanStor BCManager Agent failed."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )
        
       
        if not "!OLD_VERSION_NUM!" == "10000310" (
            echo Unsupport Version !OLD_VERSION! of OceanStor BCManager Agent.
            call :Log "Unsupport version !OLD_VERSION! of OceanStor BCManager Agent."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )
        
        call :Log "Get the version !OLD_VERSION! of previous OceanStor BCManager Agent."
        
        rem check agent port
        for /f "delims=" %%i in ('"!AGENT_BIN_PATH!xmlcfg.exe" read System port') do (set AGENT_PORT=%%i)
        if "!AGENT_PORT!" == "" (
            echo Query agent prot number failed in previous OceanStor BCManager Agent.
            call :Log "Query agent prot number failed in previous OceanStor BCManager Agent."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )
        
        call :Log "Get the agent port number !AGENT_PORT! of new OceanStor BCManager Agent."
        
        if !AGENT_PORT! LEQ 1024  (
            echo The port number should be more than 1024 and less than or equal to 65535.
            call :Log "The port number should be more than 1024 and less than or equal to 65535, agent port number !AGENT_PORT!"
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )
        
        if !AGENT_PORT! GTR 65535 (
            echo The port number should be more than 1024 and less than or equal to 65535.
            call :Log "The port number should be more than 1024 and less than or equal to 65535, agent port number !AGENT_PORT!"
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )
        
        call :Log "Check the agent port number !AGENT_PORT! be used in this system."
        
        for /f "tokens=1,2,3* delims=: " %%a in ('2^>nul netstat -an ^| findstr !AGENT_PORT!') do (
            if "%%c" == "!AGENT_PORT!" (
                set PORT_FLAG=1
            )
        )
        
        if "!PORT_FLAG!" == "1" (
            echo The port number !AGENT_PORT! in agent_cfg.xml is used by other process, Please modify it."
            call :Log "The port number ${AGENT_PORT} in agent_cfg.xml is used by other process."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )
            
        set UPGRADE_TYPE=1
    ) else (
        rem V1R5 and later
        if not exist "!OLD_AGENT_BIN_PATH!agentcli.exe" (
            echo File agentcli.exe was missed in previous OceanStor BCManager Agent.
            call :Log "agentcli.exe was missed in previous OceanStor BCManager Agent."
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_CHECK_VSERSION_FAILED%
        )

        if exist "!OLD_AGENT_BIN_PATH!agentcli.exe" (
            for /f "tokens=1,2 delims=: " %%i in ('2^>nul "!OLD_AGENT_BIN_PATH!agentcli.exe" show ^| findstr /i Version') do (set OLD_VERSION=%%j)
            
            for /f "tokens=1,2,3,4 delims=VRCBSPT" %%a in ('echo !OLD_VERSION!') do (
                set OLD_VER_V=%%a
                set OLD_VER_VR=%%a%%b
                set OLD_VER_VRC=%%a%%b%%c
                set OLD_VER_VRCSPC=%%a%%b%%c%%d
            )
            
            if "!OLD_VERSION!" == "" (
                echo Query version of previous OceanStor BCManager Agent failed.
                call :Log "Query version of previous OceanStor BCManager Agent failed."
                echo The upgrade OceanStor BCManager Agent will be stopped.
                %CMD_PAUSE%
                exit %ERR_CHECK_VSERSION_FAILED%
            )
            
            call :Log "Get the version !OLD_VERSION! of previous OceanStor BCManager Agent."
            
            set UPGRADE_TYPE=2
            set CUR_VERSION_VR=%OLD_VER_VR%
            call :Log "Get the version !OLD_VERSION! of previous OceanStor BCManager Agent."
        )
    )

    set OLD_VERSION=!OLD_VERSION!
    for /f "tokens=1,2,3 delims='SPBT'" %%a in ('echo !OLD_VERSION!') do (
        set CHECK_VER_VRC=%%a
        set CHECK_VER_VRCSPC=%%b
    )
    if not "!CHECK_VER_VRCSPC!" == "" (
        if "!CHECK_VER_VRCSPC:~0,1!" == "C" (
            set CHECK_VER_VRCSPC=SP!CHECK_VER_VRCSPC!
        ) else ( 
            set CHECK_VER_VRCSPC=
        )
    )
    
    rem have a T version, set T at last
    @echo !OLD_VERSION! | findstr "T" >nul 2>nul
    if "!errorlevel!" == "0" (
        set CHECK_VER_VRCSPC=!CHECK_VER_VRCSPC!T
    )
    
    call :LOG "CHECK_VER_VRC=!CHECK_VER_VRC!"
    call :LOG "CHECK_VER_VRCSPC=!CHECK_VER_VRCSPC!"
    
    @"!AGENT_BIN_PATH!xmlcfg.exe" read Upgrade !CHECK_VER_VRC! version >nul 2>nul
    if not "!errorlevel!" == "0" (
        echo Can not get available Version !CHECK_VER_VRC! to upgrade from list file.
        call :LOG "Can not get available Version !CHECK_VER_VRC! to upgrade from list file."
        %CMD_PAUSE%
        exit %ERR_CHECK_VSERSION_FAILED%
    )

    if "!CHECK_VER_VRCSPC!" == "" (
        for /f "tokens=1" %%a in ('"!AGENT_BIN_PATH!xmlcfg.exe" read Upgrade !CHECK_VER_VRC! version') do (
            set SUPORT_VER=%%a
            
            if "!SUPORT_VER!" == "all" (
                echo "Current version is !CHECK_VER_VRC!, all versions in !CHECK_VER_VRC! are allowed to upgrade."
                call :LOG "Current version is !CHECK_VER_VRC!, all versions in !CHECK_VER_VRC! are allowed to upgrade."
            ) else (
                @echo "!SUPORT_VER!" | findstr "||" >nul 2>nul
                set SUPORT_VER_RET=!errorlevel!
                
                set SUPORT_VER_START=!SUPORT_VER:~0,1!
                set SUPORT_VER_END=!SUPORT_VER:~-1,1!
                
                if not "!SUPORT_VER_RET!" == "0" (
                    if not "!SUPORT_VER_START!" == "|" (
                        if not "!SUPORT_VER_END!" == "|" (
                            echo "Can not get available Version !CHECK_VER_VRC! to upgrade from list file."
                            call :LOG "Can not get available Version !CHECK_VER_VRC! to  upgrade from list file."
                            %CMD_PAUSE%
                            exit %ERR_CHECK_VSERSION_FAILED%
                        )
                    )
                )
            )
        )
    ) else (
        for /f "tokens=1" %%a in ('"!AGENT_BIN_PATH!xmlcfg.exe" read Upgrade !CHECK_VER_VRC! version') do (
            set SUPORT_VER="|%%a|"
            set SUPORT_VER=!SUPORT_VER: =!
            set SUPORT_VER=!SUPORT_VER:"=!

            if "!SUPORT_VER!" == "|all|" (
                echo "Current version is !CHECK_VER_VRC!, all versions in !CHECK_VER_VRC! are allowed to upgrade."
                call :LOG "Current version is !CHECK_VER_VRC!, all versions in !CHECK_VER_VRC! are allowed to upgrade."
            ) else (
                @echo "!SUPORT_VER!" | findstr "|!CHECK_VER_VRCSPC!|" >nul 2>nul
                
                if not "!errorlevel!" == "0" (
                    echo "Can not get available Version !CHECK_VER_VRC! to upgrade from list file."
                    call :LOG "Can not get available Version !CHECK_VER_VRC! to upgrade from list file."
                    %CMD_PAUSE%
                    exit %ERR_CHECK_VSERSION_FAILED%
                )
                echo "Get available Version !CHECK_VER_VRC!!CHECK_VER_VRCSPC! to upgrade from list file !SUPORT_VER!."
                call :LOG "Get available Version !CHECK_VER_VRC!!CHECK_VER_VRCSPC! to upgrade from list file !SUPORT_VER!."
            )
        )
    )

    goto :EOF

:checkdiskspace
    call :Log "Check free space of OceanStor BCManager Agent installation."
    
    for /f "delims=:" %%i in ('echo %CURRENT_PATH%') do (set DRIVER_LETTER=%%i:)
    for /f "tokens=3 delims= " %%i in ('2^>nul dir "%CURRENT_PATH%..\tmp"') do (set FREE_SPACE=%%i)
    set FREE_SPACE=!FREE_SPACE:,=!
    set FREE_SPACE=!FREE_SPACE:~0,-3!
    
    rem free space is too large
    if not "!FREE_SPACE!" == "-1" (
        set /a FREE_SPACE=!FREE_SPACE! / 1024
    )
    
    if not "!FREE_SPACE!" == "-1" (
        if !FREE_SPACE! LSS !FREE_SPACE_MIN! (
            echo The installation path free space !FREE_SPACE! is less than the minimum space requirements !FREE_SPACE_MIN!.
            call :Log "The installation path free space !FREE_SPACE! is less than the minimum space requirements !FREE_SPACE_MIN!, then exit.
            echo The upgrade OceanStor BCManager Agent will be stopped.
            %CMD_PAUSE%
            exit %ERR_DISK_FREE_ISLESS_500MB%
        )
    )
    
    call :Log "Free space !FREE_SPACE! of OceanStor BCManager Agent installation."
    goto :EOF
    
:queryagentpath
    for /f "delims=" %%a in ('2^>nul sc qc "!CHECK_VERSION!" ^| 2^>nul findstr /i "BINARY_PATH_NAME" ^| findstr ^"^"^"') do (set QUOTATION_MARKS=%%a)
    
    rem processing quotation marks and without quotation marks in path
    if not "!QUOTATION_MARKS!" == "" (
        for /f tokens^=1-4^ delims^=:.^" %%a in ('2^>nul sc qc "!CHECK_VERSION!" ^| findstr /i "BINARY_PATH_NAME"') do (set OLD_AGENT_BIN_PATH=%%c:%%d\..\)
    ) else (
        for /f tokens^=1-4^ delims^=:.^" %%a in ('2^>nul sc qc "!CHECK_VERSION!" ^| findstr /i "BINARY_PATH_NAME"') do (set OLD_AGENT_BIN_PATH=%%b:%%c\..\)
    )
    
    if "!OLD_AGENT_BIN_PATH!" == "" goto :EOF

    call :trimblank
    
    for /f "delims=" %%a in ('echo !OLD_AGENT_BIN_PATH!') do (set OLD_AGENT_BIN_PATH=%%~fa)
    for /f "delims=" %%a in ('echo !OLD_AGENT_BIN_PATH!..') do (set OLD_AGENT_ROOT_PATH=%%~fa)

    call :Log "Query installation path !OLD_AGENT_BIN_PATH! of previous OceanStor BCManager Agent installation."
    goto :EOF

rem Remove the spaces on both sides 
:trimblank
    if "!OLD_AGENT_BIN_PATH:~,1!"==" " (set "OLD_AGENT_BIN_PATH=!OLD_AGENT_BIN_PATH:~1!"&goto :trimblank)
    if "!OLD_AGENT_BIN_PATH:~-1!"==" " (set "OLD_AGENT_BIN_PATH=!OLD_AGENT_BIN_PATH:~,-1!"&goto :trimblank)
    goto :EOF
    

:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] %~1 >> "%LOGFILE_PATH%"
    )
    call "%AGENT_BIN_PATH_FORLOG%\agent_func.bat" "%LOGFILE_PATH%"
    goto :EOF
    
:deletefolder
    if "%UPGRADE_FLAG%" == "/r" (
        if exist "!NEW_INSTALL_PATH!"        rd /s /q "!NEW_INSTALL_PATH!"
        if exist "!NEW_INSTALL_PATH!_bak"    rd /s /q "!NEW_INSTALL_PATH!_bak"

        move "!OLD_AGENT_BIN_PATH!agent_stop.bat.bak"        "!OLD_AGENT_BIN_PATH!agent_stop.bat" >nul
        move "!OLD_AGENT_BIN_PATH!agent_start.bat.bak"       "!OLD_AGENT_BIN_PATH!agent_start.bat" >nul
        move "!OLD_AGENT_BIN_PATH!agent_install.bat.bak"     "!OLD_AGENT_BIN_PATH!agent_install.bat" >nul
        move "!OLD_AGENT_BIN_PATH!agent_uninstall.bat.bak"   "!OLD_AGENT_BIN_PATH!agent_uninstall.bat" >nul
    )
    
    goto :EOF
    
:end
    endlocal