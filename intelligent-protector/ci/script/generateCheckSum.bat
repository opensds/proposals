echo off
setlocal EnableDelayedExpansion

net use Y: \\10.156.163.53\Cloud_Archive_V100R003C00_Agent
set GenerateType=%1
Y:
cd Package
call :generateCheckSum

C:
net use Y: /delete /Y
goto :eof

rem ************************************************************************
rem function name: generateCheckSum
rem aim:           generate CheckSum
rem input:         NA
rem output:        NA
rem ************************************************************************
:generateCheckSum
    set strFileName=
    for /f "tokens=*" %%f in ('dir /b') do (
        set strFileName=%%f
        set prefix=!strFileName:~0,5!
        set suffix=!strFileName:~-6,6!
        
        if NOT "!suffix!" == "sha256" (
            echo %%f begin...
            set sha256File=!strFileName!.sha256
            D:\windows_buildcloud-agent\tools\sha256sum.exe !strFileName! > !sha256File!
                
            set checkSum=
            set shaFile=
            call :clearStar !sha256File! checkSum shaFile
            echo !checkSum! !shaFile! > !sha256File!
            echo %%f end...
        )
    )

goto :eof

rem ************************************************************************
rem function name: clearStar
rem aim:           clear star
rem input:         the package file name
rem output:        NA
rem ************************************************************************
:clearStar
    
    set packageFile=%1

    if exist %packageFile% (
        for /f "tokens=1,2 delims=*" %%m in (%packageFile%) do (
            set %~2=%%m
            set %~3=%%n
            rem 中间两个空格
            rem echo %%m  %pkgFile:~1%> %packageFile%
        )
    ) else (
        echo file not exists!
    )

goto :eof
