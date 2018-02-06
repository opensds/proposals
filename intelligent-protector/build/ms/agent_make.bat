@echo off
rem #################################################################################
rem # Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
rem #################################################################################

setlocal EnableDelayedExpansion

set Drive=%~d0
set CurBatPath=%~dp0
echo CurBatPath is %CurBatPath%

set AGENTPATH=%CurBatPath%..\..\
set SOLUTION=%AGENTPATH%vsprj\agentprj\agentprj.sln
set SDP_SOLUTION=%AGENTPATH%vsprj\agentprj\sdpngx\sdpngx.sln

echo AGENTPATH is %AGENTPATH%
echo SOLUTION is %SOLUTION%

set VCTOOL_HOME=
call :FindVCTOOLHome VCTOOL_HOME

echo VCTOOL_HOME is %VCTOOL_HOME%
set DEVEVN_FILE=%VCTOOL_HOME%\..\IDE\devenv.exe
set BUILD_LOG="%CurBatPath%build.log"

echo include is %INCLUDE%

echo 1 is "%1"

if "%1" EQU "Clean64Openssl" (call :Clean64Openssl)
if "%1" EQU "compile64agent" (call :Compile64Agent)
echo End of Compile
goto :eof

:Clean64Openssl
rem clean openssl 64 lib file

rem setting the enviroment with VC command amd64
echo amd64 is %VCTOOL_HOME%..\..\VC\bin\x86_amd64\vcvarsx86_amd64.bat
call %VCTOOL_HOME%..\..\VC\bin\x86_amd64\vcvarsx86_amd64.bat

%Drive%
cd %AGENTPATH%\open_src\openssl
rem Adding the 3 lines below to avoid NMAKE : fatal error U1077: 'del' : return code '0x1'
IF NOT EXIST %AGENTPATH%\open_src\openssl\inc32 MKDIR  %AGENTPATH%\open_src\openssl\inc32
IF NOT EXIST %AGENTPATH%\open_src\openssl\out32 MKDIR  %AGENTPATH%\open_src\openssl\out32
IF NOT EXIST %AGENTPATH%\open_src\openssl\tmp32 MKDIR  %AGENTPATH%\open_src\openssl\tmp32

perl configure VC-WIN64A > %BUILD_LOG%
call ms\do_win64a

nmake -f ms/nt.mak clean >> %BUILD_LOG%
echo End to Clean 64 Openssl
goto :eof

:Compile64Agent
rem build 32 bit openssl for 32 bit nginx
set SDP_SOLUTION_CONFIG="Release|x64"
set ACTION=Rebuild

%Drive%
cd %AGENTPATH%\open_src\nginx_tmp
call %VCTOOL_HOME%vsvars32.bat

rem add no-asm option for nmake  because the masm don't not support compile the newest openssl
nmake -f auto/lib/openssl/makefile.msvc	OPENSSL="../openssl" OPENSSL_OPT="no-asm" >> %BUILD_LOG%
echo %DEVEVN_FILE% "%SDP_SOLUTION%" /clean
%DEVEVN_FILE% "%SDP_SOLUTION%" /clean

echo %DEVEVN_FILE% "%SDP_SOLUTION%" /%ACTION% %SDP_SOLUTION_CONFIG% /useenv
%DEVEVN_FILE% "%SDP_SOLUTION%" /%ACTION% %SDP_SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%

cd %AGENTPATH%\open_src\nginx_tmp
call %VCTOOL_HOME%vsvars32.bat
nmake -f objs/Makefile >> %BUILD_LOG%

RD /Q /S %AGENTPATH%\open_src\openssl
IF NOT EXIST "%AGENTPATH%\open_src\tmp"  MKDIR "%AGENTPATH%\open_src\tmp"
MOVE "%AGENTPATH%\open_src\openssl*.tar.gz" "%AGENTPATH%\open_src\tmp"
cd "%AGENTPATH%\open_src\tmp"
"C:\Program Files (x86)\7-Zip\7z.exe" x openssl*gz -y -o"%AGENTPATH%\open_src\tmp"
"C:\Program Files (x86)\7-Zip\7z.exe" x openssl*.tar -y -o"%AGENTPATH%\open_src"
MOVE "%AGENTPATH%\open_src\openssl*" "%AGENTPATH%\open_src\openssl" >> %BUILD_LOG%
MOVE "%AGENTPATH%\open_src\tmp\openssl*.tar.gz" "%AGENTPATH%\open_src" >> %BUILD_LOG%
cd "%AGENTPATH%\open_src"
RD /Q /S "%AGENTPATH%\open_src\tmp"

rem setting the enviroment with VC command amd64
echo amd64 is %VCTOOL_HOME%..\..\VC\bin\x86_amd64\vcvarsx86_amd64.bat
call %VCTOOL_HOME%..\..\VC\bin\x86_amd64\vcvarsx86_amd64.bat
rem begin to make openssl 64 bit lib
cd %AGENTPATH%\open_src\openssl
perl configure VC-WIN64A >> %BUILD_LOG%
call ms\do_win64a

rem nmake -f ms/nt.mak clean >> %BUILD_LOG%
nmake -f ms\nt.mak >> %BUILD_LOG%

rem make sqlite3.c file
cd %AGENTPATH%\open_src\sqlite
call nmake /f Makefile.msc sqlite3.c clean >> %BUILD_LOG%
call nmake /f Makefile.msc sqlite3.c >> %BUILD_LOG%

cd %CurBatPath%

if not exist %AGENTPATH%\open_src\openssl\out32\libeay32.lib (goto CompileFailed)

if not exist %AGENTPATH%\open_src\openssl\out32\ssleay32.lib (GOTO CompileFailed)

set SOLUTION_CONFIG="Release|x64"
echo Begin to clean compile >> %BUILD_LOG%
echo %DEVEVN_FILE% "%SOLUTION%" /clean

%DEVEVN_FILE% "%SOLUTION%" /clean
echo Begin to rebuild the Agent >> %BUILD_LOG%
echo %DEVEVN_FILE% "%SOLUTION%" /%ACTION% %SOLUTION_CONFIG% /useenv

%DEVEVN_FILE% "%SOLUTION%" /%ACTION% %SOLUTION_CONFIG% /useenv /Out %BUILD_LOG%

echo End to build x64 Agent
goto :eof

rem
rem Find the Visual Studio Tool's Installed Path by query enviroment
rem  %1: return value, the path where Visual Studio Tool installed, if not found, return "n/a"
rem
:FindVCTOOLHome
    set VCTOOLTemp=n/a

    rem Query the registry to find the VC's install Path 
    set | findstr "VS100COMNTOOLS" >temp.txt 2>nul
    for /f "tokens=2,* delims==" %%i in (temp.txt) do set VCTOOLTemp=%%i
        
    if "%VCTOOLTemp%" NEQ "n/a" (set %1="%VCTOOLTemp%")
    call :DeleteFile temp.txt
goto :eof

rem
rem Delete a file
rem
:DeleteFile
    set FileName=%~1
    if exist "%FileName%" (del /f /q "%FileName%")
goto :eof

:CompileFailed
echo  %DATE% %TIME% openssl file Not Found,Compile Failed. >> %BUILD_LOG%
goto Exit

:Exit

