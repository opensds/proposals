@echo off

setlocal EnableDelayedExpansion


set Drive=%~d0
set CurBatPath=%~dp0

set VCPath="D:\VS2012\VC\"

set AgentPath=%CurBatPath%..\..\..\Agent


%Drive%

copy /y %AgentPath%\open_src\fcgi\include\fcgi_config_x86.h %AgentPath%\open_src\fcgi\include\fcgi_config.h

call %VCPath%\bin\vcvars32.bat

cd /d %AgentPath%\open_src\openssl

call perl configure VC-WIN64A
call ms\do_win64a
call nmake -f ms\nt.mak

rem call %VCPath%vcvarsall.bat

cd /d %AgentPath%\open_src\sqlite

call nmake /f Makefile.msc sqlite3.c clean
call nmake /f Makefile.msc sqlite3.c


setlocal DisableDelayedExpansion

    if exist %AgentPath%\open_src\tinyxml\tinystr.h1 del /f /q %AgentPath%\open_src\tinyxml\tinystr.h1

    set /a n=0
    set str=
    for /f "delims=" %%i in ('findstr /n .* %AgentPath%\open_src\tinyxml\tinystr.h') do (
        set "str=%%i"
        set /a "n=n+1"
        setlocal EnableDelayedExpansion
        set str=!str:*:=!
        if !n!==92 ( 
            set str=virtual !str!
        )
        echo.!str!>>%AgentPath%\open_src\tinyxml\tinystr.h1
        endlocal
    )

    copy /y %AgentPath%\open_src\tinyxml\tinystr.h %AgentPath%\open_src\tinyxml\tinystr.h.bak
    move /y %AgentPath%\open_src\tinyxml\tinystr.h1 %AgentPath%\open_src\tinyxml\tinystr.h
endlocal