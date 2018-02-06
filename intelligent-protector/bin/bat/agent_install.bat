@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################


rem ########################################################################
rem #
rem #  upgrade: %~1=/up; push upgrade: %~1=/up/r; 
rem #  %~2: "100":V100; "200":V200
rem #  %~3: /up : upgrade; /rb: rollback
rem #
rem #  push installation: %~1=/r; push install: %~2 agent_conf file; 
rem #
rem ########################################################################

setlocal EnableDelayedExpansion

cd /d %~dp0
set CURRENT_PATH=%~dp0
set AGENT_ROOT_PATH=%CURRENT_PATH%..\
set AGENT_BIN_PATH=%AGENT_ROOT_PATH%\bin\
set AGENT_LOG_PATH=%AGENT_ROOT_PATH%log\
set LOGFILE_PATH=%AGENT_LOG_PATH%agent_install.log
set REGULAR_NAME_FILE=%AGENT_ROOT_PATH%tmp\user.info
set REGULAR_PASSWD_FILE=%AGENT_ROOT_PATH%tmp\en_tmp
set REGULAR_PORT_FILE=%AGENT_ROOT_PATH%tmp\port.info
set REGULAR_IP_FILE=%AGENT_ROOT_PATH%tmp\ip.info
set REGULAR_PATH_FILE=%AGENT_ROOT_PATH%tmp\path.info
set SET_IP_TMP_FILE=%AGENT_ROOT_PATH%tmp\setiptmppath.info
set INSTALL_TMP_FILE=%AGENT_ROOT_PATH%tmp\installtmpinfo
set SESERV_LOGON_RIGHT_FILE=%AGENT_ROOT_PATH%tmp\logonright.inf
set SESERV_LOGON_RIGHT_TMP_DB=%AGENT_ROOT_PATH%tmp\logonright.sdb

set WORKING_USER=rdadmin
set WORKING_USER_PWD_EN=401C8F36B7DF2DA9336BAF749790A3F31386F8483E3163FE2D6661900A445B3E66F5F1F5213CB99437546D71987AEC97

set NEED_LOG=1
set WIN08_VER=6
set DEFAULT_PORT=0
set AGENT_PORT=0
set PORT_NAME=rdagent
set PARAM_NAME=listen
set KEY_USERNAME=username
set KEY_USERPWD=userpwd
set KEY_IPADDRESS=ipaddress
set KEY_PORT=port
set /a NUMBER=0
rem contains calculation deviation
set /a FREE_SPACE_MIN=512
set USER_REX="^[a-zA-Z][a-zA-Z0-9_]*$"
set PORT_REX="^[1-9][0-9]*$"
set IP_REX="^[0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*$"
set NUM_REX="^[1-9][0-9]*[\\s]*$"
set PATH_REX="^[-_\\sA-Za-z0-9:\\]*$" 
set LINE="^.*$"
set CMD_PAUSE=pause
set INPUT_PARAM_FLAG=0
set DEL_WORKING_USER=0
set EXEC_FLAG=%~1
set AGENT_INFO=

rem -------------error code----------------------
set /a ERR_INPUT_PARAM_ERR=10
set /a ERR_SERVICE_IS_EXIST=11
set /a ERR_SERVICE_REGISTER_FAILED=12
set /a ERR_DISK_FREE_ISLESS_500MB=13
set /a ERR_WORKINGUSER_ADD_FAILED=14
set /a ERR_CHECK_INSTALLATION_PATH_FAILED=15
set /a ERR_USERNAME_SET_FAILED=16
set /a ERR_PASSWORD_SET_FAILED=17
set /a ERR_IPADDR_SET_FAILED=18
set /a ERR_PORT_SET_FAILED=19
rem -------------error code----------------------


call :Log "####################agent_install##########################"

if not "%EXEC_FLAG%" == "" (
    rem upgrade
    if "%EXEC_FLAG%" == "/up" (
        set INPUT_PARAM_FLAG=1
        set CMD_PAUSE=
        if "" == "%~3" (
            echo Input param is error. 
            call :Log "Input param is empty, exit %ERR_INPUT_PARAM_ERR%."
            exit /b %ERR_INPUT_PARAM_ERR%
        )
    )
    
    rem push upgrade
    if "%EXEC_FLAG%" == "/up/r" (
        echo Input param is error
        exit /b %ERR_INPUT_PARAM_ERR%
        
        set INPUT_PARAM_FLAG=1
        set CMD_PAUSE=
        if "" == "%~3" (
            echo Input param is error. 
            call :Log "Input param is empty, exit %ERR_INPUT_PARAM_ERR%."
            exit /b %ERR_INPUT_PARAM_ERR%
        )
    )
    
    rem push Installation
    if "%EXEC_FLAG%" == "/r" (
        set INPUT_PARAM_FLAG=2
        set CMD_PAUSE=
        set AGENT_INFO=%~2
        if "!AGENT_INFO!" == "" (
            echo Input param is error. 
            call :Log "Input param is empty, exit %ERR_INPUT_PARAM_ERR%."
            exit /b %ERR_INPUT_PARAM_ERR%
        )
        
        if not exist "!AGENT_INFO!" (
            echo User info conf file is not exist.
            call :Log "User info conf file is not exist, exit %ERR_INPUT_PARAM_ERR%."
            exit /b %ERR_INPUT_PARAM_ERR%
        )
    )
    
    if 0 EQU !INPUT_PARAM_FLAG! (
        echo Input Param is error, exit.
        call :Log "Input Param is error, exit %ERR_INPUT_PARAM_ERR%."
        %CMD_PAUSE%
        exit /b %ERR_INPUT_PARAM_ERR%
    )
    
    if 1 EQU !INPUT_PARAM_FLAG! (
        call :Log "Begin to install new OceanStor BCManager Agent for upgrade."

        call :checkServices rdagent
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)

        call :checkServices rdnginx
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)

        call :checkServices rdmonitor
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)
        
        rem user add
        if "100" == "%~2" (
            call :addworkinguser
            if not "!errorlevel!" == "0" (exit /b %ERR_WORKINGUSER_ADD_FAILED%)
        ) else (
            echo !WORKING_USER_PWD_EN!>"!REGULAR_PASSWD_FILE!"
            for /f "delims=" %%i in ('call "!AGENT_BIN_PATH!crypto.exe" -a 1 -i "!REGULAR_PASSWD_FILE!"') do (set WORKING_USER_PWD=%%i)
            if "!WORKING_USER_PWD!" == "" (
                call :Log "Decryption the password of working user !WORKING_USER! failed."
                exit /b %ERR_SERVICE_REGISTER_FAILED%
            )
        )    
        
        call :registServices rdagent rdagent
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_REGISTER_FAILED%)

        if not "100" == "%~2" (
            call :registServices rdnginx nginx !WORKING_USER!
        ) else (
            set DEL_WORKING_USER=1
            if "/rb" == "%~3" (
                call :registServices rdnginx nginx
            ) else (
                call :registServices rdnginx nginx !WORKING_USER!
            )
        )
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_REGISTER_FAILED%)
        
        
        if not "100" == "%~2" (
            call :registServices rdmonitor monitor !WORKING_USER!
        ) else (
            if "/rb" == "%~3" (
                call :registServices rdmonitor monitor
            ) else (
                call :registServices rdmonitor monitor !WORKING_USER!
            )
        )
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_REGISTER_FAILED%)
        
        if exist "!AGENT_BIN_PATH!crypto.exe" (del /f /q "!AGENT_BIN_PATH!crypto.exe")
        if exist "!AGENT_BIN_PATH!..\log\crypto.log" (del /f /q "!AGENT_BIN_PATH!..\log\crypto.log")
        
        call :Log "Install new OceanStor BCManager Agent for upgrade end."
        exit /b 0
    )
    
    rem push Installation
    if 2 EQU !INPUT_PARAM_FLAG! (
        call :Log "Begin to install OceanStor BCManager Agent, input param %EXEC_FLAG%."
        
        call :checkServices rdagent
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)
        
        call :checkServices rdnginx
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)
        
        call :checkServices rdmonitor
        if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)
        
        call :checkdiskspace

        net user !WORKING_USER! 1>nul 2>nul
        if "!errorlevel!" == "0" (
            echo Agent working user !WORKING_USER! exist.
            echo The installation of OceanStor BCManager Agent will be stopped.
            call :Log "Agent working user !WORKING_USER! exist, exit."
            exit /b %ERR_WORKINGUSER_ADD_FAILED%
        )
        call :addworkinguser
        if not "!errorlevel!" == "0" (exit /b %ERR_WORKINGUSER_ADD_FAILED%)
        
        set AGENT_ROOT_PATH=!DRIVER_LETTER!\Program Files\Huawei\BCManager\Agent\
        set AGENT_BIN_PATH=!AGENT_ROOT_PATH!bin\
        set REGULAR_PASSWD_FILE=!AGENT_ROOT_PATH!tmp\en_tmp
        
        if not exist "!AGENT_ROOT_PATH!" (
            mkdir "!AGENT_ROOT_PATH!"
        ) else (
            for /f "tokens=1" %%i in ('2^>nul dir /a/b "!AGENT_ROOT_PATH!"') do (set DIR_ISEMPTY=%%i)
            if not "" == "!DIR_ISEMPTY!" (
                call :seservicelogonright /d
                call :deleteworkinguser
                call :Log "Installation Path !AGENT_ROOT_PATH! exist, and not empty, then delete user !WORKING_USER! and exist 1."
                exit /b %ERR_CHECK_INSTALLATION_PATH_FAILED%
            )
        )
        
        xcopy /e/h/v/q/y "!CURRENT_PATH!..\*"  "!AGENT_ROOT_PATH!" >nul
        
        set /a NUMBER=2
        call :setusername
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_USERNAME_SET_FAILED%
        )
        
        call :Log "call xmlcfg.exe"
        call "!AGENT_BIN_PATH!xmlcfg.exe" write System name !USER_NAME!
        if not "!errorlevel!" == "0" (
            call :seservicelogonright /d
            call :deleteworkinguser
            call :Log "Set user name(!USER_NAME!) failed."
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_USERNAME_SET_FAILED%
        )
        call :Log "Set user name(!USER_NAME!) successfully."
        
        
        call :setpassword
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_PASSWORD_SET_FAILED%
        )
        
        rem IP set
        call :setipforPush
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_IPADDR_SET_FAILED%
        )
        
        set /a NUMBER=2
        set PORT_NUM=
        set DEFAULT_PORT=8091
        set PORT_NAME=rdagent
        call :setports
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_PORT_SET_FAILED%
        )
        
        set PARAM_NAME=fastcgi_pass
        call "!AGENT_BIN_PATH!xmlcfg.exe" write System port !PORT_NUM!
        if not "!errorlevel!" == "0" (
            call :seservicelogonright /d
            call :deleteworkinguser
            call :Log "Set agent port number(!PORT_NUM!) failed."
            %CMD_PAUSE%
            exit /b %ERR_PORT_SET_FAILED%
        )

        call :modifyconffile
        
        set /a NUMBER=2
        set PORT_NUM=
        set DEFAULT_PORT=59526
        set PORT_NAME=nginx
        call :setports
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_PORT_SET_FAILED%
        )
        
        set PARAM_NAME=listen
        call :modifyconffile
        
        rem register service
        call :registServices rdagent rdagent
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_SERVICE_REGISTER_FAILED%
        )

        call :registServices rdnginx nginx !WORKING_USER!
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_SERVICE_REGISTER_FAILED%
        )

        call :registServices rdmonitor monitor !WORKING_USER!
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_SERVICE_REGISTER_FAILED%
        )

        call :registprovider
        if not "!errorlevel!" == "0" (
            if exist "!AGENT_ROOT_PATH!" (rd /s /q "!AGENT_ROOT_PATH!")
            exit /b %ERR_SERVICE_REGISTER_FAILED%
        )
        
        if exist "!AGENT_BIN_PATH!crypto.exe" (del /f /q "!AGENT_BIN_PATH!crypto.exe")
        if exist "!AGENT_BIN_PATH!..\log\crypto.log" (del /f /q "!AGENT_BIN_PATH!..\log\crypto.log")
        
        echo.
        echo OceanStor BCManager Agent was installed successfully.
        call :Log "OceanStor BCManager Agent was installed successfully."
        exit /b 0
    )
)


call :Log "Begin to install OceanStor BCManager Agent."

rem Check installation path
set PATH_TMP=%CURRENT_PATH: =%
echo %PATH_TMP%>"%REGULAR_PATH_FILE%"

for /f "delims=" %%i in ('findstr /i %PATH_REX% "%REGULAR_PATH_FILE%"') do (set PATH_CHECK=%%i)

if exist "%REGULAR_PATH_FILE%"  del /f /q "%REGULAR_PATH_FILE%"

if "%PATH_CHECK%" == "" (
    echo The installation path of OceanStor BCManager Agent can only contain characters such as '-_ a-zA-Z0-9'.
    echo The installation of OceanStor BCManager Agent will be stopped.
    call :Log "The installation path of OceanStor BCManager Agent contain the unsupport characters, install failed."
    %CMD_PAUSE%
    exit %ERR_CHECK_INSTALLATION_PATH_FAILED%
)

call :checkdiskspace

call :log "Begin to set OceanStor BCManager Agent conf."

rem===========check Service exist begin==============

call :checkServices rdagent
if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)

call :checkServices rdnginx
if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)

call :checkServices rdmonitor
if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_IS_EXIST%)

rem============check Service exist end==============


rem=============Add working user begin==============

net user !WORKING_USER! 1>nul 2>nul
if "!errorlevel!" == "0" (
    echo Agent working user !WORKING_USER! exist.
    echo The installation of OceanStor BCManager Agent will be stopped.
    call :Log "Agent working user !WORKING_USER! exist, exit."
    %CMD_PAUSE%
    exit /b %ERR_WORKINGUSER_ADD_FAILED%
)

call :addworkinguser
if not "!errorlevel!" == "0" (exit /b %ERR_WORKINGUSER_ADD_FAILED%)

rem=============Add working user end================


rem================set username begin===============

set /a NUMBER=0
call :setusername
if not "!errorlevel!" == "0" (exit /b %ERR_USERNAME_SET_FAILED%)

call "%AGENT_BIN_PATH%xmlcfg.exe" write System name !USER_NAME!
if not "!errorlevel!" == "0" (
    call :seservicelogonright /d
    call :deleteworkinguser
    call :Log "Set user name(!USER_NAME!) failed."
    %CMD_PAUSE%
    exit /b %ERR_USERNAME_SET_FAILED%
)

call :Log "Set user name(!USER_NAME!) successfully."

rem================set username end==================


rem================set user password begin===========

call :setpassword
if not "!errorlevel!" == "0" (exit /b %ERR_PASSWORD_SET_FAILED%)

rem================set user password end==============


rem================set ip begin=======================

call :Log "Set IP of OceanStor BCManager Agent."  
set SYSTEM_VER=0
set /a NUMBER=0
for /f "tokens=1,2,3,4,5 delims=. " %%a in ('ver') do (set SYSTEM_VER=%%d)

if !SYSTEM_VER! LSS %WIN08_VER% (
    set IP_FILTER=ip.*:
) else (
    set IP_FILTER=ipv4
)

call :setipaddress
   
if exist "%REGULAR_IP_FILE%"  del /f /q "%REGULAR_IP_FILE%"

call :Log "Set user IP successfully!"

rem================set ip end=========================


rem================set agent port begin===============

call :Log "Set port of OceanStor BCManager Agent."
set /a NUMBER=0
set PORT_NUM=
set DEFAULT_PORT=8091
set PORT_NAME=rdagent
call :setports
if not "!errorlevel!" == "0" (exit /b %ERR_PORT_SET_FAILED%)

set PARAM_NAME=fastcgi_pass
call "%AGENT_BIN_PATH%xmlcfg.exe" write System port !PORT_NUM!
if not "!errorlevel!" == "0" (
    call :seservicelogonright /d
    call :deleteworkinguser
    call :Log "Set agent port number(!PORT_NUM!) failed."
    %CMD_PAUSE%
    exit /b %ERR_PORT_SET_FAILED%
)

call :modifyconffile


call :Log "Set user agent port (!PORT_NUM!) successfully!"

rem================set agent port end==================


rem================set nginx port begin================

set /a NUMBER=0
set PORT_NUM=
set DEFAULT_PORT=59526
set PORT_NAME=nginx
call :setports
if not "!errorlevel!" == "0" (exit /b %ERR_PORT_SET_FAILED%)

set PARAM_NAME=listen
call :modifyconffile

call :Log "Set user nginx port (!PORT_NUM!) successfully!"

rem================set nginx port end==================

rem================set file privilege begin==================
set AGENT_PRIV_PATH=!AGENT_ROOT_PATH:~0,-1!
echo Y | Cacls "!AGENT_PRIV_PATH!" /T /E /R Users >> "%LOGFILE_PATH%"
rem================set file privilege end==================

call :registServices rdagent rdagent
if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_REGISTER_FAILED%)

call :registServices rdnginx nginx !WORKING_USER!
if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_REGISTER_FAILED%)

call :registServices rdmonitor monitor !WORKING_USER!
if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_REGISTER_FAILED%)

call :registprovider
if not "!errorlevel!" == "0" (exit /b %ERR_SERVICE_REGISTER_FAILED%)

if exist "!AGENT_BIN_PATH!crypto.exe" (del /f /q "!AGENT_BIN_PATH!crypto.exe")
if exist "!AGENT_BIN_PATH!..\log\crypto.log" (del /f /q "!AGENT_BIN_PATH!..\log\crypto.log")

echo.
echo OceanStor BCManager Agent was installed successfully.
call :Log "OceanStor BCManager Agent was installed successfully."

ping -n 4 127.0>nul

exit /b 0


:setusername
    if %NUMBER% LSS 3 (
        set USER_NAME=
        if "%EXEC_FLAG%" == "/r" (
            call :getagentinfo "!KEY_USERNAME!" USER_NAME
        ) else (
            echo Please input your name:
            set /p USER_NAME=">>"
        )
        
        if "!USER_NAME!" == "" (
            echo The name should contain 4 to 16 characters, including case-sensitive letters, digits or underscores ^(_^), and must start with a letter.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setusername
        )
        
        echo !USER_NAME!>"%REGULAR_NAME_FILE%"
        
        set USER_CHECK=
        for /f "delims=" %%i in ('findstr /r "[^a-zA-Z0-9_]" "%REGULAR_NAME_FILE%"') do (set USER_CHECK=%%i)
        
        if not "!USER_CHECK!" == "" (            
            echo The name should contain 4 to 16 characters, including case-sensitive letters, digits or underscores ^(_^), and must start with a letter.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setusername
        )
        
        set USER_CHECK=
        for /f "delims=" %%i in ('findstr /r %USER_REX% "%REGULAR_NAME_FILE%"') do (set USER_CHECK=%%i)
  
        if "!USER_CHECK!" == "" (
            echo The name should contain 4 to 16 characters, including case-sensitive letters, digits or underscores ^(_^), and must start with a letter.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setusername
        )
        
        set STR_LEN=
        set a=!USER_CHECK!
        for /l %%a in (0,1,1000) do if "!a:~%%a,1!"=="" set STR_LEN=%%a&&goto usernamelen
        
        :usernamelen
        if !STR_LEN! LSS 4 (
            echo The name should contain 4 to 16 characters.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setusername
        )
        
        if !STR_LEN! GTR 16 (
            echo The name should contain 4 to 16 characters.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setusername
        )
         
        call :Log "Get user !USER_NAME! of OceanStor BCManager Agent succ."
        if exist "%REGULAR_NAME_FILE%"  del /f /q "%REGULAR_NAME_FILE%"
        exit /b 0
        
    ) else (
        if exist "%REGULAR_NAME_FILE%"  del /f /q "%REGULAR_NAME_FILE%"
        call :seservicelogonright /d
        call :deleteworkinguser
        call :Log "Get user of OceanStor BCManager Agent failed."
        echo The installation of OceanStor BCManager Agent will be stopped.
        %CMD_PAUSE%
        exit /b 1
    )
    
    exit /b 0
    
:setpassword
    if "%EXEC_FLAG%" == "/r" (
        call "%AGENT_BIN_PATH%getinput.exe" "!AGENT_INFO!"
    ) else (
        call "%AGENT_BIN_PATH%getinput.exe"
    )
    
    if not "!errorlevel!" == "0" (
        if exist "%REGULAR_PASSWD_FILE%"  del /f /q "%REGULAR_PASSWD_FILE%"
        call :seservicelogonright /d
        call :deleteworkinguser
        echo The installation of OceanStor BCManager Agent will be stopped.
        call :Log "The installation of OceanStor BCManager Agent will be stopped."
        %CMD_PAUSE%
        exit /b 1
    )

    if not exist "%REGULAR_PASSWD_FILE%" (
        call :seservicelogonright /d
        call :deleteworkinguser
        echo The temporary file en_tmp does not exist, installation of OceanStor BCManager Agent will be stopped.
        call :Log "The temporary file en_tmp does not exist, installation of OceanStor BCManager Agent will be stopped."
        %CMD_PAUSE%
        exit /b 1
    )

    for /f "delims=" %%i in ('type "%REGULAR_PASSWD_FILE%"') do (set ENCODE_PASSWD=%%i)
    if exist "%REGULAR_PASSWD_FILE%"  del /f /q "%REGULAR_PASSWD_FILE%"

    call "%AGENT_BIN_PATH%xmlcfg.exe" write System hash !ENCODE_PASSWD!
    if not "!errorlevel!" == "0" (
        echo Set password of user !USER_NAME! failed.
        call :seservicelogonright /d
        call :deleteworkinguser
        call :Log "Set password of user !USER_NAME! failed."
        %CMD_PAUSE%
        exit /b 1
    )

    call :Log "Set password of user !USER_NAME! successfully."
    exit /b 0
    
:setipaddress
    if %NUMBER% LSS 3 (
        rem get ALL IP in this system
        echo Please choose IP Address binded by nginx:
        set /a IPCOUNT=1
        for /f "tokens=1,2,3 delims=:" %%i in ('2^>nul ipconfig ^| findstr /i "!IP_FILTER!"') do (
            if "%%j" NEQ " 127.0.0.1" (
                echo    !IPCOUNT! %%j
                set IP_CHECK=%%j
                echo !IP_CHECK!>>"%REGULAR_IP_FILE%"
                set /a IPCOUNT+=1
            )
        )
        set /p IPCHOICE=">>"
        
        if "!IPCHOICE!" == "" ( 
            echo Your choice is invalid
            goto :setipaddress
        )
        
        :trimblank
        if "!IPCHOICE:~,1!"==" " (set "IPCHOICE=!IPCHOICE:~1!"&goto :trimblank)
        if "!IPCHOICE:~-1!"==" " (set "IPCHOICE=!IPCHOICE:~,-1!"&goto :trimblank)

        echo !IPCHOICE!>"%SET_IP_TMP_FILE%"
        rem check the input number
        set IPCHOICE=
        for /f "delims=" %%i in ('findstr /i "^0$" "%SET_IP_TMP_FILE%"') do (set IPCHOICE=%%i)
        if "!IPCHOICE!" NEQ "0" (
            for /f "delims=" %%i in ('findstr /i %NUM_REX% "%SET_IP_TMP_FILE%"') do (set IPCHOICE=%%i)
        )
        del "%SET_IP_TMP_FILE%"
        
        if "!IPCHOICE!" == "" (
            echo Your choice is invalid
            set /a NUMBER+=1
            goto :setipaddress
        )
        if !IPCHOICE! GEQ !IPCOUNT! (
            echo Your choice is invalid
            set /a NUMBER+=1
            goto :setipaddress
        )
        
        if !IPCHOICE! == 0 (
            echo Your choice is invalid
            goto :setipaddress
        )
        set /a IPINDEX=1
        for /f "delims= " %%i in ( 'findstr /i %LINE% "%REGULAR_IP_FILE%"' ) do (
            if !IPINDEX! == !IPCHOICE! ( 
                set IP_ADDRESS=%%i 
                set IP_ADDRESS=!IP_ADDRESS: =!
                if !SYSTEM_VER! LSS %WIN08_VER% (
                    set IP_ADDRESS=!IP_ADDRESS:~0,-1!
                )
                goto :EOF
            )
            set /a IPINDEX+=1
        )
       
        goto :EOF
       
    ) else (
        if exist "%REGULAR_IP_FILE%"  del /f /q "%REGULAR_IP_FILE%"
        call :seservicelogonright /d
        call :deleteworkinguser
        echo The installation of OceanStor BCManager Agent will be stopped.
        call :Log "The IP you entered(!IP_ADDRESS!) is error, so installation was stopped!"
        %CMD_PAUSE%
        exit  %ERR_IPADDR_SET_FAILED%
    )
    
    goto :EOF

rem push install
:setipforPush
    set SYSTEM_VER=0
    set /a NUMBER=0
    for /f "tokens=1,2,3,4,5 delims=. " %%a in ('ver') do (set SYSTEM_VER=%%d)

    if !SYSTEM_VER! LSS %WIN08_VER% (
        set IP_FILTER=ip
    ) else (
        set IP_FILTER=ipv4
    )

    set IP_ADDRESS=
    call :getagentinfo "!KEY_IPADDRESS!" IP_ADDRESS
    
    if "!IP_ADDRESS!" == "" (
        call :seservicelogonright /d
        call :deleteworkinguser
        call :Log "IP address is empty, exits 1."
        exit /b 1
    )
    
    set IP_ADDRESS=!IP_ADDRESS: =!
    if "!IP_ADDRESS!" == "" (
        call :seservicelogonright /d
        call :deleteworkinguser
        call :Log "IP address format is error, exits 1."
        exit /b 1
    )
    
    if "!IP_ADDRESS!" == "0.0.0.0" (exit /b 0)
    
    echo !IP_ADDRESS!>"%REGULAR_IP_FILE%"
    
    set IP_CHECK=
    for /f "delims=" %%i in ('findstr /i %IP_REX% "%REGULAR_IP_FILE%"') do (set IP_CHECK=%%i)
    if exist "%REGULAR_IP_FILE%" (del /f /q "%REGULAR_IP_FILE%")
    if "!IP_CHECK!" == "" (
        echo IP address !IP_ADDRESS! format is error.
        call :seservicelogonright /d
        call :deleteworkinguser
        call :Log "IP address !IP_ADDRESS! format is error."
        exit /b 1
    )
    
    set IPFLAG=0
    set IP_FRINUM=0
    set IP_SECNUM=0
    set IP_TTINUM=0
    set IP_FOUNUM=0
    for /f "tokens=1,2,3,4 delims=." %%a in ('echo !IP_ADDRESS!') do (
        set IP_FRINUM=%%a
        set IP_SECNUM=%%b
        set IP_TTINUM=%%c
        set IP_FOUNUM=%%d
    )
    
    if !IP_FRINUM! LSS 0 set IPFLAG=1
    if !IP_FRINUM! GTR 255 set IPFLAG=1

    if !IP_SECNUM! LSS 0 set IPFLAG=1
    if !IP_SECNUM! GTR 255 set IPFLAG=1

    if !IP_TTINUM! LSS 0 set IPFLAG=1
    if !IP_TTINUM! GTR 255 set IPFLAG=1

    if !IP_FOUNUM! LSS 0 set IPFLAG=1
    if !IP_FOUNUM! GTR 255 set IPFLAG=1
    
    if !IPFLAG! EQU 1 (
        call :seservicelogonright /d
        call :deleteworkinguser
        echo The IP Address number must be more than 0 and less than 255.
        call :Log "The IP Address number must be more than 0 and less than 255."
        exit /b 1
    )
    
    if "!IP_ADDRESS!" == "127.0.0.1" (
        call :seservicelogonright /d
        call :deleteworkinguser
        echo The IP Address number can not be 127.0.0.1.
        call :Log "The IP Address number can not be 127.0.0.1."
        exit /b 1
    )
    
    set IPFLAG=0
    set IP_CHECK=
    for /f "tokens=1,2,3 delims=:" %%i in ('2^>nul ipconfig ^| findstr /i "!IP_FILTER!"') do (
        set IP_CHECK=%%j
        set IP_CHECK=!IP_CHECK: =!
    
        if !SYSTEM_VER! LSS %WIN08_VER% (
            set IP_CHECK=!IP_CHECK:~0,-1!
        )
    
        if "!IP_ADDRESS!" == "!IP_CHECK!" (
          set IPFLAG=1  
        )    
    )
    
    if !IPFLAG! EQU 0 (
        call :seservicelogonright /d
        call :deleteworkinguser
        echo The IP Address !IP_ADDRESS! is not a local IP Address.
        call :Log "The IP Address !IP_ADDRESS! is not a local IP Address."
        exit /b 1
    )
    
    echo Set IP Address !IP_ADDRESS! succ.
    call :Log "Set IP Address !IP_ADDRESS! succ."
    exit /b 0
    
:setports
    if %NUMBER% LSS 3 (
        set PORT_NUM=
        if "%EXEC_FLAG%" == "/r" (
            if "nginx" == "%PORT_NAME%" (
                call :getagentinfo "!KEY_PORT!" PORT_NUM
            )
        ) else (
            echo Please input %PORT_NAME% listening port number 1024-65535, default port number is %DEFAULT_PORT%:
            set /p PORT_NUM=">>"
        )
        
        
        if "!PORT_NUM!" == "" (
            echo You choose %PORT_NAME% default port number [%DEFAULT_PORT%].
            set PORT_NUM=!DEFAULT_PORT!
        ) else (
            set PORT_NUM=!PORT_NUM: =!
        )
        
        echo !PORT_NUM!>"%REGULAR_PORT_FILE%"
        
        set PORT_CHECK=
        for /f "delims=" %%i in ('findstr /i %PORT_REX% "%REGULAR_PORT_FILE%"') do (set PORT_CHECK=%%i)
        if exist "%REGULAR_PORT_FILE%"   del "%REGULAR_PORT_FILE%"
        
        if "!PORT_CHECK!" == "" (
            echo The port should contain 1 to 5 digits and start with 1-9.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setports
        )
        
        if !PORT_NUM! LEQ 1024  (
            echo The port number should be more than 1024 and less than or equal to 65535.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setports
        )
        
        if !PORT_NUM! GTR 65535 (
            echo The port number should be more than 1024 and less than or equal to 65535.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setports
        )
        
        set PORT_EXIST=
        set PORT_FLAG=0
        for /f "tokens=1,2,3* delims=: " %%a in ('2^>nul netstat -an ^| findstr !PORT_NUM!') do (
            if "%%c" == "!PORT_NUM!" (
                set PORT_FLAG=1
            )
        )
         
        if !PORT_FLAG! EQU 1 (
            echo The port number is used by other process!
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setports
        )
        
        if "%AGENT_PORT%" == "!PORT_NUM!" (
            echo nginx port number !PORT_NUM! is same with agent port number %AGENT_PORT%.
            set /a NUMBER+=1
            ping -n 2 127.0>nul
            goto :setports
        )
        
        set AGENT_PORT=!PORT_NUM!
        
        exit /b 0
        
    ) else (
        call :seservicelogonright /d
        call :deleteworkinguser
        echo The installation of OceanStor BCManager Agent will be stopped.
        call :Log "The %PORT_NAME% port you entered(!PORT_NUM!) is error, so installation was stopped!"
        %CMD_PAUSE%
        exit /b 1
    )
    
    exit /b 0
    
:modifyconffile
    set ISPECIL_NUM=0
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "!AGENT_BIN_PATH!nginx\conf\nginx.conf"') do (
        
        for /f %%j in ('2^>nul echo.%%b ^| findstr  !PARAM_NAME!') do (
            set IFLAF_NUM=%%a 
        )
        
        for /f %%j in ('2^>nul echo.%%b ^| findstr  ssl_ciphers') do (
            set ISPECIL_NUM=%%a 
        )
    )

    if !IFLAF_NUM! equ 0 (
        call :Log "Don't find the !PARAM_NAME! in the nginx.conf, exit 1."
        %CMD_PAUSE%
        exit 1
    )
    
    if !ISPECIL_NUM! equ 0 (
        call :Log "Don't find the ssl_ciphers in the nginx.conf, exit 1."
        %CMD_PAUSE%
        exit 1
    )
      
    for /f "tokens=1* delims=:" %%a in ('findstr /n .* "!AGENT_BIN_PATH!nginx\conf\nginx.conf"') do (
        
        if %%a equ %IFLAF_NUM% (
            if "!PARAM_NAME!" == "listen" (
                
                echo.        !PARAM_NAME!       %IP_ADDRESS%:%PORT_NUM% ssl;>>"!AGENT_ROOT_PATH!tmp\nginx.conf.bak"
                
            ) else (
                
                echo.            !PARAM_NAME!   127.0.0.1:%PORT_NUM%;>>"!AGENT_ROOT_PATH!tmp\nginx.conf.bak"
                
            )
        ) else (
            echo.%%b >>"!AGENT_ROOT_PATH!tmp\nginx.conf.bak"
        )   
    )
    
    MOVE "!AGENT_ROOT_PATH!tmp\nginx.conf.bak" "!AGENT_BIN_PATH!nginx\conf\nginx.conf" >nul
    
    call :Log "Set %PORT_NAME% port %PORT_NUM% successfully."
    
    goto :EOF
    
:registServices
    set SERVICE_NAME=%~1
    set SERVICE_PARAM=%~2
    set REGISTER_USER=
    if not "" == "%~3" (set REGISTER_USER=.\%~3)
    
    call :Log "Register Service !SERVICE_NAME! of OceanStor BCManager Agent."
    
    sc stop !SERVICE_NAME! >> "%AGENT_LOG_PATH%\agent_install.log" 2>&1
    
    call "%AGENT_BIN_PATH%\winservice.exe" !SERVICE_PARAM! uninstall
    if not "!errorlevel!" == "0" (
        call :unregisterall
        if "1" == "!INPUT_PARAM_FLAG!" (
            if "1" == "!DEL_WORKING_USER!" (
                call :seservicelogonright /d
                call :deleteworkinguser
            )
        ) else (
            call :seservicelogonright /d
            call :deleteworkinguser
        )
        
        echo Service !SERVICE_NAME! of OceanStor BCManager Agent was uninstalled failed.
        call :Log "Service !SERVICE_NAME! of OceanStor BCManager Agent uninstall failed."
        %CMD_PAUSE%
        exit /b 1
    )
    
    if "" == "%~3" (
        call "%AGENT_BIN_PATH%\winservice.exe" !SERVICE_PARAM! install
    ) else (
        call "%AGENT_BIN_PATH%\winservice.exe" !SERVICE_PARAM! install !REGISTER_USER! !WORKING_USER_PWD!
    )
    
    if not "!errorlevel!" == "0" (
        call :unregisterall
        if "1" == "!INPUT_PARAM_FLAG!" (
            if "1" == "!DEL_WORKING_USER!" (
                call :seservicelogonright /d
                call :deleteworkinguser
            )
        ) else (
            call :seservicelogonright /d
            call :deleteworkinguser
        )

        echo Service !SERVICE_NAME! of OceanStor BCManager Agent was registered failed.
        call :Log "Register Service !SERVICE_NAME! of OceanStor BCManager Agent failed."
        %CMD_PAUSE%
        exit /b 1
    )
    
    call sc start !SERVICE_NAME! >> "%AGENT_LOG_PATH%\agent_install.log" 2>&1
    
    call :Log "Service !SERVICE_NAME! of OceanStor BCManager Agent was registered successfully."
    echo Service !SERVICE_NAME! of OceanStor BCManager Agent was registered successfully.
    
    exit /b 0
    
:registprovider
    call :Log "Register Service provider of OceanStor BCManager Agent."
    
    call "%AGENT_BIN_PATH%\install-provider.cmd">> "%AGENT_LOG_PATH%\agent_install.log" 2>&1
    if not "!errorlevel!" == "0" (
        call :unregisterall
        call :seservicelogonright /d
        call :deleteworkinguser
        echo Service provider of OceanStor BCManager Agent was registered failed.
        call :Log "Register Service provider of OceanStor BCManager Agent failed."
        %CMD_PAUSE%
        exit /b 1
    )
    
    call :Log "Register Service provider of OceanStor BCManager Agent successfully."
    echo Register Service provider of OceanStor BCManager Agent successfully.
    
    exit /b 0
    
:unregisterall
    call "%AGENT_BIN_PATH%\winservice.exe" monitor uninstall
    call "%AGENT_BIN_PATH%\winservice.exe" nginx   uninstall
    call "%AGENT_BIN_PATH%\winservice.exe" rdagent uninstall
    call "%AGENT_BIN_PATH%\uninstall-provider.cmd">nul
    
    goto :EOF
   
rem %~1 rdagent, rdmonitor, rdprovider   
:checkServices
    call :Log "Check Service %~1."
    set SERVICE_CHECK=
    for /f "delims=" %%i in ('2^>nul sc query %~1 ^| findstr /i "%~1"') do (set SERVICE_CHECK=%%i)
    if not "!SERVICE_CHECK!" == "" (
        call :Log "Service %~1 is exist."
        echo Service %~1 is exist.
        %CMD_PAUSE%
        exit /b %ERR_SERVICE_IS_EXIST%
    )
    
    exit /b 0
  
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
    
:addworkinguser
    set WORKING_USER_PWD=

    echo !WORKING_USER_PWD_EN!>"!REGULAR_PASSWD_FILE!"
    for /f "delims=" %%i in ('call "!AGENT_BIN_PATH!crypto.exe" -a 1 -i "!REGULAR_PASSWD_FILE!"') do (set WORKING_USER_PWD=%%i)
    if "!WORKING_USER_PWD!" == "" (
        call :Log "Get working user !WORKING_USER! password failed."
        echo Get working user !WORKING_USER! password failed.
        echo The installation of OceanStor BCManager Agent will be stopped.
        %CMD_PAUSE%
        exit /b 1
    )

    rem add user
    net user !WORKING_USER! !WORKING_USER_PWD! /add /passwordchg:no /expires:never >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        echo Add user !WORKING_USER! failed.
        echo The installation of OceanStor BCManager Agent will be stopped.
        call :Log "Add user !WORKING_USER! failed."
        %CMD_PAUSE%
        exit /b 1
    )

    rem set password never expires
    wmic useraccount where "Name='!WORKING_USER!'" set PasswordExpires=FALSE 1>nul 2>nul
    call :Log "Add user !WORKING_USER! succ."

    net localgroup administrators !WORKING_USER! /add >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        echo Add user !WORKING_USER! to localgroup administrators failed.
        call :Log "Add user !WORKING_USER! to localgroup administrators failed."
        call :deleteworkinguser
        echo The installation of OceanStor BCManager Agent will be stopped.
        call :Log "Add user !WORKING_USER! failed."
        %CMD_PAUSE%
        exit /b 1
    )

    call :seservicelogonright
    if not "!errorlevel!" == "0" (
        echo Set user !WORKING_USER! to Log on as a service failed.
        call :deleteworkinguser
        echo The installation of OceanStor BCManager Agent will be stopped.
        call :Log "Add user !WORKING_USER! failed."
        %CMD_PAUSE%
        exit /b 1
    )
    
    call :Log "Add user !WORKING_USER! to localgroup administrators succ."
    exit /b 0

:deleteworkinguser    
    net user !WORKING_USER! /delete >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        call :Log "Delete user !WORKING_USER! failed."
        exit /b 1
    )
    
    call :Log "Delete the working user !WORKING_USER! infos."
    for /f "tokens=*" %%a in ('2^>nul reg query "hklm\software\microsoft\windows nt\currentversion\profilelist" ^| findstr /i "s-1-5-21"') do (call :deleteuserinfos "%%a")
    
    call :Log "Delete user !WORKING_USER! succ."

    exit /b 0
    
:deleteuserinfos
    set USER_REG_PATH=
    for /f "tokens=1,2,*" %%a in ('2^>nul reg query "%~1" /v ProfileImagePath') do (set USER_REG_PATH=%%c)
    
    if "!USER_REG_PATH!" == "" (
        call :Log "Query user home director failed."
        exit /b 1
    )
    
    for /f "tokens=3 delims=\" %%e in ('echo.!USER_REG_PATH!') do (set USER_REG=%%e)
    for /f "tokens=1 delims=." %%f in ('echo.!USER_REG!') do (set USER_REG_PARSE=%%f)
    
    rem other users, do not deal
    if not "!USER_REG_PARSE!" == "!WORKING_USER!" (exit /b 0)
    
    rem clear regeditor
    reg delete "%~1" /f 1>nul 2>>"%LOGFILE_PATH%"
    
    rem delete home director
    if exist "!USER_REG_PATH!" (rd /s /q "!USER_REG_PATH!") 1>nul 2>>"%LOGFILE_PATH%"
    
    rem check infos
    reg query "%~1" 1>nul 2>&1
    if "!errorlevel!" == "0" (
        call :Log "The regeditor of %~1 delete failed."
        exit /b 1
    )
    
    if exist "!USER_REG_PATH!" (
        call :Log "Delete the !USER_REG_PATH! failed."
        exit /b 1
    )
    
    call :Log "Clear working user infos succ."
    exit /b 0
    
rem %~1:/d delete !WORKING_USER! from the seservicelogonright
:seservicelogonright
    set UNICODE_VAL=
    set SIGNATURE_VAL=
    set REVISION_VAL=
    set SESLOR_VAL=
    set SEDRILOR_VAL=
    set SEDILOR_VAL=
    
    rem export seservicelogonright config
    if exist "!SESERV_LOGON_RIGHT_FILE!"  del /f /q "!SESERV_LOGON_RIGHT_FILE!"

    secedit /export /cfg "!INSTALL_TMP_FILE!" >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        call :Log "Export seservicelogonright config failed."
        if exist "!INSTALL_TMP_FILE!"  del /f /q "!INSTALL_TMP_FILE!"
        exit /b 1
    )

    rem get the info that Log on as a service need
    for /f "delims=" %%a in ('type "!INSTALL_TMP_FILE!"') do (
        set V_KEY=
        for /f "tokens=1 delims==" %%i in ('echo.%%a') do (set V_KEY=%%i)
        set V_KEY=!V_KEY: =!
        
        if "Unicode" == "!V_KEY!" (set UNICODE_VAL=%%a)

        if "signature" == "!V_KEY!" (set SIGNATURE_VAL=%%a)

        if "Revision" == "!V_KEY!" (set REVISION_VAL=%%a)
        
        if "SeServiceLogonRight" == "!V_KEY!" (
            if "%~1" == "/d" (
                set SESLOR_VAL=%%a
                set SESLOR_VAL=!SESLOR_VAL:,%WORKING_USER%=!
                set SESLOR_VAL=!SESLOR_VAL:%WORKING_USER%,=!
                set SESLOR_VAL=!SESLOR_VAL:%WORKING_USER%=!
            ) else (
                set SESLOR_VAL=%%a,%WORKING_USER%
            )
        )
        
        if "SeDenyRemoteInteractiveLogonRight" == "!V_KEY!" (
            if "%~1" == "/d" (
                set SEDRILOR_VAL=%%a
                set SEDRILOR_VAL=!SEDRILOR_VAL:,%WORKING_USER%=!
                set SEDRILOR_VAL=!SEDRILOR_VAL:%WORKING_USER%,=!
                set SEDRILOR_VAL=!SEDRILOR_VAL:%WORKING_USER%=!
            ) else (
                set SEDRILOR_VAL=%%a,%WORKING_USER%
            )
        )
        
        if "SeDenyInteractiveLogonRight" == "!V_KEY!" (
            if "%~1" == "/d" (
                set SEDILOR_VAL=%%a
                set SEDILOR_VAL=!SEDILOR_VAL:,%WORKING_USER%=!
                set SEDILOR_VAL=!SEDILOR_VAL:%WORKING_USER%,=!
                set SEDILOR_VAL=!SEDILOR_VAL:%WORKING_USER%=!
            ) else (
                set SEDILOR_VAL=%%a,%WORKING_USER%
            )
        )
    )
    
    rem the right working user need is empty
    if not "%~1" == "/d" (
        if "!SESLOR_VAL!" == "" (
            set SESLOR_VAL=SeServiceLogonRight = %WORKING_USER%
        )
        if "!SEDRILOR_VAL!" == "" (
            set SEDRILOR_VAL=SeDenyRemoteInteractiveLogonRight = %WORKING_USER%
        )
        if "!SEDILOR_VAL!" == "" (
            set SEDILOR_VAL=SeDenyInteractiveLogonRight = %WORKING_USER%
        )
    )
    
    echo.[Unicode]> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!UNICODE_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.[Version]>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!SIGNATURE_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!REVISION_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.[Privilege Rights]>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!SESLOR_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!SEDRILOR_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    echo.!SEDILOR_VAL!>> "!SESERV_LOGON_RIGHT_FILE!"
    
    call :Log "get the infos that log on as a service need from the installtmpinfo"
    if exist "!INSTALL_TMP_FILE!"  del /f /q "!INSTALL_TMP_FILE!"
    
    rem import seservicelogonright config to .sdb file
    secedit /import /db "!SESERV_LOGON_RIGHT_TMP_DB!" /cfg "!SESERV_LOGON_RIGHT_FILE!" >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        call :Log "Import the !SESERV_LOGON_RIGHT_FILE! to the !SESERV_LOGON_RIGHT_TMP_DB! failed."
        if exist "!SESERV_LOGON_RIGHT_FILE!"  del /f /q "!SESERV_LOGON_RIGHT_FILE!"
        exit /b 1
    )
    
    if exist "!SESERV_LOGON_RIGHT_FILE!"    del /f /q "!SESERV_LOGON_RIGHT_FILE!"
    
    rem add user seservicelogonright Log on as a service
    secedit /configure /db "!SESERV_LOGON_RIGHT_TMP_DB!" >>"%LOGFILE_PATH%"
    if not "!errorlevel!" == "0" (
        call :Log "Add user !WORKING_USER! seservicelogonright to log on as a service failed."
        if exist "!SESERV_LOGON_RIGHT_TMP_DB!"  del /f /q "!SESERV_LOGON_RIGHT_TMP_DB!"
        exit /b 1
    )

    if exist "!SESERV_LOGON_RIGHT_TMP_DB!"  del /f /q "!SESERV_LOGON_RIGHT_TMP_DB!"
    call :Log "Set user !WORKING_USER! seservicelogonright to log on as a service succ."
    exit /b 0
    
rem %~1:key %~2:value    
:getagentinfo
    set AGENT_KEY=%~1
    for /f "tokens=1,2 delims==" %%a in ('type "!AGENT_INFO!"') do (if "%%a" == "!AGENT_KEY!" (set %~2=%%b))
    
    goto :EOF
    
:Log
    if %NEED_LOG% EQU 1 (
        echo %date:~0,10% %time:~0,8% [%username%] "%~1" >> "%LOGFILE_PATH%"
    )
    call "%AGENT_BIN_PATH%\agent_func.bat" "%LOGFILE_PATH%"
    goto :EOF

:end
    endlocal