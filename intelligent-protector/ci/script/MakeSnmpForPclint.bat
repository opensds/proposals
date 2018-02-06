@echo off

setlocal EnableDelayedExpansion


set Drive=%~d0
set CurBatPath=%~dp0

set AgentPath=%CurBatPath%..\..\..\Agent

setlocal DisableDelayedExpansion

    if exist %AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h1 del /f /q %AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h1

    set /a n=0
    set str=
    for /f "delims=" %%i in ('findstr /n .* %AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h') do (
        set "str=%%i"
        set /a "n=n+1"
        setlocal EnableDelayedExpansion
        set str=!str:*:=!
        if !n!==55 ( 
            echo.!str!>>%AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h1
            set str=#define STDCXX_98_HEADERS 1
        )
        if !n!==60 ( 
            echo.!str!>>%AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h1
            set str=#undef SNMP_PP_NAMESPACE
        )
        echo.!str!>>%AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h1
        endlocal
    )

    copy /y %AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h %AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h.bak
    move /y %AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h1 %AgentPath%\open_src\snmp++\include\snmp_pp\snmp_pp.h
endlocal