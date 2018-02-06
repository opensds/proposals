@echo off
set yy=%date:~0,4%&set month=%date:~5,2%&set day=%date:~8,2%
set hh=%time:~0,2%&set mm=%time:~3,2%&set ss=%time:~6,2%

net use Y: \\10.156.163.53\Cloud_Archive_V100R003C00_Agent
set archiveDirectory="Y:\Archive\%yy%%month%%day% %hh%%mm%%ss%"

rem create directory
md %archiveDirectory%\AIX
md %archiveDirectory%\RHEL5
md %archiveDirectory%\RHEL6
md %archiveDirectory%\SLES10
md %archiveDirectory%\SLES11
md %archiveDirectory%\Windows

copy "Y:\temp\AIX\*" %archiveDirectory%\AIX\ /Y
copy "Y:\temp\RHEL5\*" %archiveDirectory%\RHEL5\ /Y
copy "Y:\temp\RHEL6\*" %archiveDirectory%\RHEL6\ /Y
copy "Y:\temp\SLES10\*" %archiveDirectory%\SLES10\ /Y
copy "Y:\temp\SLES11\*" %archiveDirectory%\SLES11\ /Y 
copy "Y:\temp\Windows\*" %archiveDirectory%\Windows\ /Y 

rem copy to package
copy /Y Y:\temp\SLES10\ReplicationDirector-Agent-*.rpm Y:\package\
copy /Y Y:\temp\SLES11\ReplicationDirector-Agent-*.rpm Y:\package\
"C:\Program Files (x86)\7-Zip\7z.exe" a "Y:\temp\Windows\ReplicationDirector-Agent-V100R003C00-WIN64.zip" "Y:\temp\Windows\ReplicationDirector-Agent-V100R003C00-WIN64.exe"
copy /Y Y:\temp\Windows\ReplicationDirector-Agent-V100R003C00-WIN64.zip Y:\package\
copy /Y Y:\temp\AIX\ReplicationDirector-Agent-*.bff Y:\package\
copy /Y Y:\temp\RHEL5\ReplicationDirector-Agent-*.rpm Y:\package\
copy /Y Y:\temp\RHEL6\ReplicationDirector-Agent-*rpm Y:\package\

net use Y: /delete /Y