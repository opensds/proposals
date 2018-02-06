<# Copyright (c) Huawei Corporation. All rights reserved.
Query exchange app info result format:
###########################################################################################################################
ExchangeVersion
Storage Group Name/MailboxDB Name/Mounted Status/Edb File Path/Log Folder Path/System Folder Path/Public Folder Mailbox Flag
############################################################################################################################

Query exchange lun info result format:
###########################################################################################################################
ExchangeVersion
Storage Group Name/MailboxDB Name/Edb File Path/Log Folder Path/System Folder Path
############################################################################################################################
#>

#0 -- query exchange app info; 1 -- query exchange lun info
$gServerName = $args[0]
$gResultFile = $args[1]

$ERR_SCRIPT_EXEC_FAILED = 5

#major version, 8 -- Exchange 2007, 14 -- Exchange 2010, 15 -- Exchange 2013
$Exchange2007MajorVersion = 8
$Exchange2010MajorVersion = 14
$OperQueryExchangeAppInfo = 0
$OperQueryExchangeLunInfo = 1
$NeedLog = 1

$CurrentDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$AgentRoot = Split-Path -Parent $CurrentDir
$AgentLogPath = join-path -path $AgentRoot -childpath log
$LogFilePath = join-path -path $AgentLogPath -childpath queryexchangeinfo.log

function Log($InputLog)
{
    if (1 -eq $NeedLog)
    {
        "[$(Get-Date)] [$env:username] $InputLog" | Out-File $LogFilePath -Encoding UTF8 -Append
    }

    . "$CurrentDir\agent_func.ps1" $LogFilePath
}

function GetExchange2007AppInfo($ServerName, $ResultFile)
{
    Log "Get MailBox db in server $ServerName."
    $MailBoxDBs = Get-MailboxDatabase -status -server $ServerName -ErrorAction Stop
    foreach ($MailBoxDB in $MailBoxDBs)
    {
        $ResultString = $MailBoxDB.StorageGroup.Name + "/" + $MailBoxDB.Name + "/" + $MailBoxDB.Mounted + "/" + ($MailBoxDB.EdbFilePath).pathname + "/"
        $ResultString += ((Get-StorageGroup $MailBoxDB.StorageGroup | select LogFolderPath).LogFolderPath).pathname + "/" 
        $ResultString += ((Get-StorageGroup $MailBoxDB.StorageGroup | select SystemFolderPath).SystemFolderPath).pathname + "/" + "0"
        $ResultString | Out-File $ResultFile -Encoding ASCII -Append
        
        Log "Exchange2007 mail box db $MailBoxDB infos: $ResultString"
    }
    Log "Get Exchange2007 App info succ."
}

function GetExchange2010AppInfo($ServerName, $ResultFile)
{
    Log "Get MailBox db in server $ServerName."
    $MailBoxDBs = Get-MailboxDatabase -status -server $ServerName -ErrorAction Stop
    foreach ($MailBoxDB in $MailBoxDBs)
    {
        $ResultString = "--" + "/" + $MailBoxDB.Name + "/" + $MailBoxDB.Mounted + "/" + $MailBoxDB.EdbFilePath.pathName + "/"
        $ResultString += $MailBoxDB.LogFolderPath.pathName + "/" + "--" + "/" + "0"
        $ResultString | Out-File $ResultFile -Encoding ASCII -Append
        
        Log "Exchange2010 mail box db $MailBoxDB infos: $ResultString"
    }
    Log "Get Exchange2010 App info succ."
}

function GetExchange2013AppInfo($ServerName, $ResultFile)
{
    Log "Get MailBox db in server $ServerName."
    $MailBoxDBs = Get-MailboxDatabase -status -server $ServerName -ErrorAction Stop
    foreach ($MailBoxDB in $MailBoxDBs)
    {
        $PublicFolderMailBoxCount = (Get-Mailbox -PublicFolder | Where-Object {$_.database -eq $MailBoxDB.Name} | Measure-Object).Count

        $ResultString = "--" + "/" + $MailBoxDB.Name + "/" + $MailBoxDB.Mounted + "/" + $MailBoxDB.EdbFilePath.pathName + "/"
        $ResultString += $MailBoxDB.LogFolderPath.pathName + "/" + "--" + "/" + $PublicFolderMailBoxCount
        $ResultString | Out-File $ResultFile -Encoding ASCII -Append
        
        Log "Exchange2013 mail box db $MailBoxDB infos: $ResultString"
    }
    Log "Get Exchange2013 App info succ."
}

function QueryExchangeAppInfo($ExcMajorVersion)
{
    #write version to result file
    $ExcMajorVersion | Out-File $gResultFile -Encoding ASCII
    
    #write other info to result file
    if ($ExcMajorVersion -gt $Exchange2007MajorVersion)
    {
        if ($ExcMajorVersion -gt $Exchange2010MajorVersion)
        {
            GetExchange2013AppInfo $gServerName $gResultFile
        }
        else
        {
            GetExchange2010AppInfo $gServerName $gResultFile
        }
    }
    else
    {
        GetExchange2007AppInfo $gServerName $gResultFile
    }
}

try
{
    #get exchange major version
    Log "######################################"
    Log "get exchange major version, server name $gServerName."
    $ExchangeServer = Get-ExchangeServer -identity $gServerName -ErrorAction Stop
    $MajorVersion = $ExchangeServer.admindisplayversion.major

    QueryExchangeAppInfo $MajorVersion
}
catch
{
    $lineNumber = $Error[0].InvocationInfo.scriptlinenumber
    $ErrorInfos = $Error[0]
    Log "ERROR LINE NUMBER $lineNumber, DESCRIPTION $ErrorInfos"
    exit($ERR_SCRIPT_EXEC_FAILED)
}

exit(0)

