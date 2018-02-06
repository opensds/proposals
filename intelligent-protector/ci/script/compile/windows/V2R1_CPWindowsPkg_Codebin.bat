@echo off

net use Z: \\10.183.192.206\RDV2R1C00_AgentPackage_Codebin /Y
set ICP_AGENT=C:\build\RD_V200R001C00_Agent_Codebin\code\current
set AGENT_PKG=%ICP_AGENT%\Agent\build\ms\package

copy /Y "%AGENT_PKG%\*_Agent-WIN64.zip" "Z:\pkg"
net use Z: /delete /Y
