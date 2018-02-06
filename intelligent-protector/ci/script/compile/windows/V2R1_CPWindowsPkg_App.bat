@echo off

net use G: \\10.183.243.153\RDV2R1C10_AgentPackage_App Huawei@123 /user:Administrator /Y
SET ICP_AGENT=..\..\..\..
rem set ICP_AGENT=C:\build\RD_V200R001C10_Agent\code\current
set AGENT_PKG=%ICP_AGENT%\build\ms\package

copy /Y "%AGENT_PKG%\*_Agent-WIN64.zip" "G:\pkg"
net use G: /delete /Y
