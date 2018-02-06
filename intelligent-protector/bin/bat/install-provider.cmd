@rem @echo off
@setlocal

@set FOUND_FILES=0
@set CMD_DIR=%~dp0
@set DLL_DIR=%CMD_DIR%
@set VBS_DIR=%CMD_DIR%
@call :Checkfiles
@if %FOUND_FILES% EQU 0 (goto :missingfiles) else (goto :goodproc)


:goodproc
rem Remove existing installation
call "%CMD_DIR%\uninstall-provider.cmd"

@rem Get the complete %DLL_DIR% and %VBS_DIR%
@pushd %DLL_DIR%
@set DLL_DIR=%CD%
@popd
@pushd %VBS_DIR%
@set VBS_DIR=%CD%
@popd

rem Register RD VSS provider
cscript "%VBS_DIR%\register_app.vbs" -register "RdProvider" "%DLL_DIR%\rdvss.dll" "OceanStor BCManager Agent VSS Software Provider"

echo.
goto :EOF

:checkfiles
@if not exist "%DLL_DIR%\rdvss.dll"             goto :EOF
@if not exist "%VBS_DIR%\register_app.vbs"      goto :EOF
@set FOUND_FILES=1
@goto :EOF


:missingfiles
@echo.
@echo One or more important files are missing. 
@echo.
@echo   rdvss.dll
@echo   register_app.vbs
@echo   install-provider.cmd
@echo   uninstall-provider.cmd
@echo.
@goto :EOF
