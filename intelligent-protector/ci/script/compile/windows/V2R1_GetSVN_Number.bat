@echo off
setlocal EnableDelayedExpansion

set ICP_AGENT=C:\build\RD_V200R001C00_Agent\code\current
set AGENT_ROOT=%ICP_AGENT%\Agent
set SVN_FILE=%ICP_AGENT%\scmInfo.properties
set SVN_VERSION_FILE=%AGENT_ROOT%\conf\svn
set SVN_NUM=
for /f "tokens=1,2 delims==" %%a in ('type %SVN_FILE%') do (
    if "%%a" == "Agent.svnVersion" (
        set SVN_NUM=%%b
        goto :ECHOSVN
    )
)

:ECHOSVN
echo !SVN_NUM!> !SVN_VERSION_FILE!

endlocal
