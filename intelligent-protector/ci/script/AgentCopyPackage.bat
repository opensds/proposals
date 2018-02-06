@echo off
net use Z: \\10.156.163.53\temp /Y
copy /Y "%AGENT_ROOT%\build\ms\package\*.exe" "Z:\Windows"
net use Z: /delete /Y
