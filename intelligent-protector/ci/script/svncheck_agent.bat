echo off
set currentpath=%cd%
set repopath_source="D:\pom\ReplicationDirector_V1R3C00_Agent\code\current\Agent"

D:
rem cd %repopath%

set yy=%date:~0,4%&set month=%date:~5,2%&set day=%date:~8,2%
set hh=%time:~0,2%&set mm=%time:~3,2%
if /i %hh% LSS 10 (set hh=0%time:~1,1%)
rem 如果小时数小于10，会出现空格，运行出错，上一行补0

svn status --depth infinity %repopath_source% > %currentpath%\Agent_%yy%_%month%_%day%_%hh%_%mm%_检查更新.txt
svn info %repopath_source% > %currentpath%\Agent_%yy%_%month%_%day%_%hh%_%mm%_副本状态.txt

pause