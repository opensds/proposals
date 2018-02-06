@echo off

net use Z: \\10.183.243.153\RDV2R1C20_AgentPackage Huawei@123 /user:Administrator /Y
SET ICP_AGENT=..\..\..\..
rem set ICP_AGENT=C:\build\RD_V200R001C10_Agent\code\current
set AGENT_PKG=%ICP_AGENT%\build\ms\package

copy /Y "%AGENT_PKG%\*_Agent-WIN64.zip" "Z:\pkg"
net use Z: /delete /Y
