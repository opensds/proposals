<# Copyright (c) Huawei Corporation. All rights reserved.
#######################################################################################################################
Recovery Exchange DB:
#######################################################################################################################
Input params : 
    OperType        : 0
    Exchange Version: 1 -- 2007, 2 -- 2010, 3 -- 2013
    Old StorageGroup Name
    Old MailboxDB Name
    New StroageGrup Name
    New MailboxDB Name
    ServerName
    Edb File Path
    Log Dir Path
    Sys Dir Path
    Recovery Type    : 0 -- Recovery, 1 -- Test
    
Return value : 
    0 -- succ, 1 -- failed
#######################################################################################################################

#######################################################################################################################
Clear Exchange DB:
#######################################################################################################################
Input params :
    OperType        : 1
    Exchange Version: 1 -- 2007, 2 -- 2010, 3 -- 2013
    Storage Group Name
    MailboxDB Name
    ServerName
    
Return value : 
    0 -- succ, 1 -- failed
#######################################################################################################################
#>

$gOperType
$gVersion
#Clear parameters
$gStorageGroupName
$gMailBoxDBName
$gServerName
#Recovery paramters
$gOldStorageGroupName
$gOldMailboxDBName
$gNewStorageGroupName
$gNewMailboxDBName
$gServerName
$gEdbFilePath
$gLogDirPath
$gSysDirPath
$gRecoveryType

$gLogPareFolder

$NeedLog = 1
$RecoveryParamsCount = 12
$ClearParamsCount = 6
$Exchange2007Version = 1
$Exchange2010Version = 2
$Exchange2013Version = 3
$OperTypeRecovery = 0
$OperTypeClear = 1
$OpertypeDismount=3
$RecoverTypeRestore = 0
$RecoverTypeRollback = 2

#0:NewMailboxDbName not exist; 1:NewMailboxDbName exist
$bDBNotExit = 0
#Mount db try 5 times in total
$TryTotalTimes = 5

#error code
$ERR_CODE = 5
$ERR_SCRIPT_EXEC_FAILED = 5
$ERR_FILE_IS_EMPTY = 8
#parameter wrong
$ERR_PARAM_NUM = 9
$ERR_REMOVE_FAILED = 100
$ERR_SOFTRECOVERY_FAILED = 101
$ERR_RECOVERY_DB_FAILED = 102
$ERR_MOUNT_INMULTIAD_FAIL = 103


$CurrentDir = Split-Path $MyInvocation.MyCommand.Definition
$CurrentPath = Split-Path -Parent $MyInvocation.MyCommand.Definition
$AgentRoot = Split-Path -Parent $CurrentPath
$AgentLogPath = join-path -path $AgentRoot -childpath log
$LogFilePath = join-path -path $AgentLogPath -childpath operexchangedb.log
$InfoSeparator = "/"
$CurrentServerName = $env:COMPUTERNAME

function Log($InputLog)
{
    if (1 -eq $NeedLog)
    {
        "[$(Get-Date)] [$env:username] $InputLog" | Out-File $LogFilePath -Encoding UTF8 -Append
    }

    . "$CurrentDir\agent_func.ps1" $LogFilePath
}

function ParseInputParams($InputParams)
{
    $InputParamCount = $InputParams.count
    Set-Variable gOperType $InputParams[0] -scope 1
    Set-Variable gVersion $InputParams[1] -scope 1
    Log "Oper type $gOperType, version $gVersion."

    if ($gOperType -eq $OperTypeRecovery)
    {
        if ($InputParamCount -ne $RecoveryParamsCount)
        {
            Log "The count of input params is wrong, params count $InputParamCount."
            exit($ERR_PARAM_NUM)
        }
        
        Set-Variable gOldStorageGroupName $InputParams[2] -scope 1
        Set-Variable gOldMailboxDBName $InputParams[3] -scope 1
        Set-Variable gNewStorageGroupName $InputParams[4] -scope 1
        Set-Variable gNewMailboxDBName $InputParams[5] -scope 1
        Set-Variable gServerName $InputParams[6] -scope 1
        Set-Variable gEdbFilePath $InputParams[7] -scope 1
        Set-Variable gLogDirPath $InputParams[8] -scope 1
        Set-Variable gSysDirPath $InputParams[9] -scope 1
        Set-Variable gRecoveryType $InputParams[10] -scope 1
        Set-Variable gResultFile $InputParams[11] -scope 1
        $ResultFileName = Split-Path $gResultFile -leaf
        Log "Parse params succ, Old storage group name $gOldStorageGroupName, old mailbox db name $gOldMailboxDBName, new storage group name $gNewStorageGroupName, new mailbox db name $gNewMailboxDBName, server name $gServerName, edb file path $gEdbFilePath, log dir path $gLogDirPath sys dir path $gSysDirPath, recoverty type $gRecoveryType, result file $ResultFileName"
    }
    else
    {
        if ($InputParamCount -ne $ClearParamsCount)
        {
            Log "The count of input params is wrong, params count $InputParamCount."
            exit($ERR_PARAM_NUM)
        }

        Set-Variable gStorageGroupName $InputParams[2] -scope 1
        Set-Variable gMailBoxDBName $InputParams[3] -scope 1
        Set-Variable gServerName $InputParams[4] -scope 1
        Set-Variable gResultFile $InputParams[5] -scope 1
        $ResultFileName = Split-Path $gResultFile -leaf
        Log "Parse parasm succ, storage group name $gStorageGroupName, Mailbox db name $gMailBoxDBName, server name $gServerName, result file $ResultFileName."
    }
}

function CheckEx201XDBExist($gMailBoxDBName)
{

    $arrMailBoxDb = @(Get-MailboxDatabase -Server $CurrentServerName -ErrorAction Stop)
    $iFlag = 0
    foreach ($DbInfo in $arrMailBoxDb)
    {
        $DbName = $DbInfo.Name
        if("$gMailBoxDBName" -eq "$DbName")  
        {
            $iFlag = 1
            break
        }
    }
    
    if($bDBNotExit -eq $iFlag)
    {
        Log "Mailbox Data base $gMailBoxDBName is not exist. So return 0" 
        exit(0)
    }
}

function CheckEx2007StorGrpExist($gStorageGroupName)
{
 
    $arrStorageGroup = @(Get-StorageGroup -Server $CurrentServerName -ErrorAction Stop)
    $iFlag = 0
    foreach ($StorGrpInfo in $arrStorageGroup)
    {
        $GrpName = $StorGrpInfo.Name
        if("$gStorageGroupName" -eq "$GrpName")  
        {
            $iFlag = 1
            break
        }
    }
    
    if ($bDBNotExit -eq $iFlag)
    {
        Log "Storage Group $gMailBoxDBName is not exist. So return 0" 
        exit(0) 
    }
}

function RemoveExChange201XDB($gMailBoxDBName)
{
    #Dismount db
    Log "Dismount mailbox db $gMailBoxDBName."
    $ERR_CODE = $ERR_REMOVE_FAILED
    Dismount-Database -Identity "$gMailBoxDBName" -Confirm:$false -ErrorAction Stop

    #Remove db
    Log "Remove mailbox db $gMailBoxDBName."
    $ERR_CODE = $ERR_REMOVE_FAILED
    Remove-MailboxDatabase -Identity "$gMailBoxDBName" -Confirm:$false -ErrorAction Stop
    
    Log "Dismount MailBox db $gMailBoxDBName succ."
}

function DismountExChange2007DB($gStorageGroupName)
{
    #Query db in group
    Log "Get Mailbox dbs in $gMailBoxDBName."
    $ERR_CODE = $ERR_REMOVE_FAILED
    $MailBoxDBs = @(Get-MailboxDatabase -StorageGroup "$gStorageGroupName" -ErrorAction Stop)
   
    foreach ($MailBoxDB in $MailBoxDBs)
    {
        $Exdbname = $MailBoxDB.Name
        #Dismount db
        Log "Dismount mailbox db $Exdbname."
        Dismount-Database -Identity "$gStorageGroupName`\$Exdbname" -Confirm:$false -ErrorAction Stop
    }
    
    Log "Dismount MailBox dbs in $gMailBoxDBName succ."
}

function RemoveExChange2007DB($gStorageGroupName)
{
    #Query db in group
    Log "Get MailBox dbs in $gStorageGroupName"
    $ERR_CODE = $ERR_REMOVE_FAILED
    $MailBoxDBs = @(Get-MailboxDatabase -StorageGroup "$gStorageGroupName"  -ErrorAction Stop)
    
    foreach ($MailBoxDB in $MailBoxDBs)
    {
        $Exdbname = $MailBoxDB.Name
        #Remove db
        $tmpDbName = "$CurrentServerName`\$gStorageGroupName`\$Exdbname"

        Log "Remove MailBox db $tmpDbName."
        Remove-MailboxDatabase -Identity "$tmpDbName" -Confirm:$false -ErrorAction Stop
    }
    
    #Remove storage group
    Log "Remove storage group $gStorageGroupName."
    Remove-StorageGroup -Identity "$CurrentServerName\$gStorageGroupName" -Confirm:$false -ErrorAction Stop
    Log "Remove StorageGroup $gStorageGroupName succ."
}

function DismountExchange($ExchangeVersion)
{
    $ERR_CODE = $ERR_REMOVE_FAILED
    if ($ExchangeVersion -eq $Exchange2007Version)
    {
        CheckEx2007StorGrpExist $gStorageGroupName
        
        DismountExChange2007DB $gStorageGroupName
    }
    elseif ($ExchangeVersion -eq $Exchange2010Version)
    {
        CheckEx201XDBExist $gMailBoxDBName
       
        Dismount-Database -Identity "$gMailBoxDBName" -Confirm:$false -ErrorAction Stop
        
        Log "Dismount MailBox db $gMailBoxDBName in Exchange2010 succ."
    }
    elseif ($ExchangeVersion -eq $Exchange2013Version)
    {
        CheckEx201XDBExist $gMailBoxDBName
        
        Dismount-Database -Identity "$gMailBoxDBName" -Confirm:$false -ErrorAction Stop
        
        Log "Dismount MailBox db $gMailBoxDBName in Exchange2013 succ."
    }
    else
    {
        Log "Ivalid exchange Version $ExchangeVersion."
        exit(1)
    }
}

function ClearExchange($ExchangeVersion)
{ 
    if ($ExchangeVersion -eq $Exchange2007Version)
    {
        CheckEx2007StorGrpExist $gStorageGroupName
        DismountExChange2007DB $gStorageGroupName
        RemoveExChange2007DB $gStorageGroupName
    }
    elseif ($ExchangeVersion -eq $Exchange2010Version)
    {
        CheckEx201XDBExist $gMailBoxDBName
        RemoveExChange201XDB $gMailBoxDBName
        
        Log "Clear MailBox db $gMailBoxDBName in Exchange2010 succ."
    }
    elseif ($ExchangeVersion -eq $Exchange2013Version)
    {
        CheckEx201XDBExist $gMailBoxDBName
        RemoveExChange201XDB $gMailBoxDBName
        
        #Log "Stop HostControllerService."
        #Stop-Service HostControllerService -Confirm:$false -ErrorAction Stop
        
        Log "Clear MailBox db $gMailBoxDBName in Exchange2013 succ."
    }
    else
    {
        Log "Ivalid exchange Version $ExchangeVersion."
        exit(1)
    }
}

#Check the Edb file directory contain the log directory or not, 0:contain 1:not contain
function EdbDirContainLogDir($EdbFileDir, $LogDirPath)
{
    $TmpPath = $LogDirPath
    while ("" -ne "$TmpPath")
    {
        if ("$TmpPath" -eq "$EdbFileDir")
        {
            Log "Edb file directory contain Log directory, so return 0."
            return 0
        }
        
        #echange2013 must remove the empty log folder $gLogPareFolder
        $LogPareFolder = Split-Path $TmpPath -leaf

        Set-Variable gLogPareFolder $LogPareFolder -scope 1

        $TmpPath = Split-Path $TmpPath
    }
    
    Log "Edb file directory not contain Log directory, so return 1."
    return 1
}

function ExecSoftRecovery($LogDirPath, $EdbFilePath, $MailboxDBName, $LogPreFix)
{
    Log  "Begin exec soft recovery, log dir path $LogDirPath, edb file path $EdbFilePath, mailbox db name $MailboxDBName, logprefix $LogPreFix."
    
    if ("" -eq "$EdbFilePath")
    {
        Log "Edb file path is empty."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }
    
    $ERR_CODE = $ERR_SOFTRECOVERY_FAILED
    $StateStr = eseutil /mh "$EdbFilePath" | findstr "State:"
    $StateTrim = $StateStr.Trim()
    Log "Get state info $StateTrim."
    
    if ("$StateTrim" -ne "State: Clean Shutdown")
    {
        Log "Exec soft recovery."

        $EdbFileDir = Split-Path "$EdbFilePath"
        #"$?" is true no matter the result is succ or failed.
        if ("" -eq "$EdbFileDir")    
        {
            Log "Get edb dir path failed."
            "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
            exit($ERR_FILE_IS_EMPTY)
        }
        Log "Get edb file dir $EdbFileDir."

        cd "$LogDirPath"
        Log "cd log dir path $LogDirPath."
        
        $ResultInfo = eseutil /r "$LogPreFix" /d $EdbFileDir 

        Log "$ResultInfo"
        
        $StateStr = eseutil /mh "$EdbFilePath" | findstr "State:"
        Log "Get state info $StateStr after soft recovery."
        
        cd "$CurrentDir"
        Log "return current dir $CurrentDir."
    }
    else
    {
        Log "No need to do soft recovery."
    }

    Log "Exec soft recovery succ."
}

function ExecSoftRecovery201x($LogDirPath, $EdbFilePath, $MailboxDBName)
{
    Log "Begin exec soft recovery 201x, log dir path $LogDirPath, edb file path $EdbFilePath, mailbox db name $MailboxDBName."
    
    $ERR_CODE = $ERR_SOFTRECOVERY_FAILED
    $MailboxDB = Get-Mailboxdatabase -identity "$MailboxDBName" -ErrorAction Stop
    $LogPreFix = $MailboxDB.LogFilePrefix
    Log "Get log pre fix $LogPreFix."

    ExecSoftRecovery $LogDirPath $EdbFilePath $MailboxDBName $LogPreFix
    
    Log "Exec soft recovery 201x succ."
}

function ExecSoftRecovery2007($LogDirPath, $EdbFilePaths, $MailboxDBNames, $StorageGroupName, $ServerName)
{
    $ERR_CODE = $ERR_SOFTRECOVERY_FAILED
    $MailboxDB = Get-StorageGroup "$ServerName\$StorageGroupName" -ErrorAction Stop
    $LogPreFix = $MailboxDB.LogFilePrefix
    Log "Get log pre fix $LogPreFix."

    for ($i = 0; $i -lt $MailboxDBNames.count; $i++)
    {
        $EdbFilePath = $EdbFilePaths[$i]
        $MailboxDBName = $MailboxDBNames[$i]
        
        ExecSoftRecovery $LogDirPath $EdbFilePath $MailboxDBName $LogPreFix
    }
    
    Log "Exec soft recovery 2007 succ."
}

function BackupDir($SrcDir)
{
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    $DesDir = $SrcDir + "_Recovery"
    Log "Move dir $SrcDir  to $DesDir."
    rename-item -path "$SrcDir" "$DesDir" -ErrorAction Stop
}

function Backup201xEdbAndLogFiles($EdbFilePath, $LogDirPath)
{
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    Log "Begin backup exchange edb and log files."

    if ("" -eq "$EdbFilePath")
    {
        Log "Edb file path is empty."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }

    $EdbFileDir = Split-Path "$EdbFilePath"
    if ("" -eq "$EdbFileDir")
    {
        Log "Get edb dir path failed."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }
    Log "Get edb file dir $EdbFileDir."

    $NewEdbFileDir = $EdbFileDir + "_Recovery"
    Log "Move edb dir $EdbFileDir to $NewEdbFileDir."
    rename-item -path "$EdbFileDir" "$NewEdbFileDir" -ErrorAction Stop

    Log "Create dir $EdbFileDir."
    New-Item -ItemType directory -Path "$EdbFileDir" -ErrorAction Stop

    $ContTmp = EdbDirContainLogDir $EdbFileDir $LogDirPath
    if (0 -ne $ContTmp)
    {
        $NewLogDirPath = $LogDirPath + "_Recovery"
        Log "Move Log dir $LogDirPath to $NewLogDirPath."
        rename-item -path "$LogDirPath" "$NewLogDirPath" -ErrorAction Stop

        Log "Create Log dir $LogDirPath."
        New-Item -ItemType directory -Path "$LogDirPath" -ErrorAction Stop
    }
    
    Log "Backup exchange edb and log files succ."
}

function Backup2013Files($EdbFilePath, $LogDirPath)
{
    Log "Begin backup exchange 2013 files."

    Backup201xEdbAndLogFiles $EdbFilePath $LogDirPath

    Log "Backup exchange 2013 files succ."
}

function Backup2010Files($EdbFilePath, $LogDirPath)
{
    Log "Begin backup exchange 2010 files."

    Backup201xEdbAndLogFiles $EdbFilePath $LogDirPath

    Log "Backup exchange 2010 files succ."
}

function Backup2007Files($EdbFilePathArray, $LogDirPath, $SysDirPath)
{
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    Log "Begin backup exchange 2007 files."

    #get edb dir path and remove the reduplicate entry
    #create empty array
    $EdbDirArray = @()
    for ($i = 0; $i -lt $EdbFilePathArray.count; $i++)
    {
        $EdbFilePath = $EdbFilePathArray[$i]
        $EdbFileDir = Split-Path "$EdbFilePath"
        if ("" -eq "$EdbFileDir")
        {
            Log "Edb file path is empty."
            "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
            exit($ERR_FILE_IS_EMPTY)
        }
        
        if ($EdbDirArray.count -eq 0)
        {
            Log "Add edb file dir $EdbFileDir."
            $EdbDirArray += $EdbFileDir
        }
        else
        {
            for ($j = 0; $j -lt $EdbDirArray.count; $j++)
            {
                if ("$EdbDirArray[$j]" -eq "$EdbFileDir")
                {
                    break;
                }
            }
            
            if ($j -eq $EdbDirArray.count)
            {
                Log "Add edb file dir $EdbFileDir."
                $EdbDirArray += $EdbFileDir
            }
        }
    }
    
    for ($i = 0; $i -lt $EdbDirArray.count; $i++)
    {
        $EdbFilePath = $EdbDirArray[$i]
        if ("$EdbFilePath" -ne "$LogDirPath" -and "$EdbFilePath" -ne "$SysDirPath")
        {
            $NewEdbFileDir = $EdbFilePath + "_Recovery"
            Log "Move edb dir $EdbFilePath to $NewEdbFileDir."
            rename-item -path "$EdbFilePath" "$NewEdbFileDir" -ErrorAction Stop

            Log "Create dir $EdbFilePath."
            New-Item -ItemType directory -Path "$EdbFilePath" -ErrorAction Stop
        }
    }

    $NewLogDirPath = $LogDirPath + "_Recovery"
    Log "Move edb dir $LogDirPath to $NewLogDirPath."
    rename-item -path "$LogDirPath" "$NewLogDirPath" -ErrorAction Stop

    Log "Create dir $LogDirPath."
    New-Item -ItemType directory -Path "$LogDirPath" -ErrorAction Stop
    
    if ("$LogDirPath" -ne "$SysDirPath")
    {
        $NewSysDirPath = $SysDirPath + "_Recovery"
        
        Log "Move sys dir $SysDirPath to $NewSysDirPath."
        rename-item -path "$SysDirPath" "$NewSysDirPath" -ErrorAction Stop
        
        Log "Create dir $SysDirPath."
        New-Item -ItemType directory -Path "$SysDirPath" -ErrorAction Stop
    }

    Log "Backup exchange 2007 files succ."
}

function Restore2007Files($EdbFilePathArray, $LogDirPath, $SysDirPath)
{ 
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    Log "Begin restore exchange 2007 files."

    #remove the reduplicate entry
    #create empty array
    $EdbDirArray = @()
    for ($i = 0; $i -lt $EdbFilePathArray.count; $i++)
    {
        $EdbFilePath = $EdbFilePathArray[$i]
        $EdbFileDir = Split-Path "$EdbFilePath"
        
        if ($EdbDirArray.count -eq 0)
        {
            Log "Add edb file dir $EdbFileDir."
            $EdbDirArray += $EdbFileDir
        }
        else
        {
            for ($j = 0; $j -lt $EdbDirArray.count; $j++)
            {
                if ("$EdbDirArray[$j]" -eq "$EdbFileDir")
                {
                    break;
                }
            }
            
            if ($j -eq $EdbDirArray.count)
            {
                Log "Add edb file dir $EdbFileDir."
                $EdbDirArray += $EdbFileDir
            }
        }
    }

    for ($i = 0; $i -lt $EdbDirArray.count; $i++)
    {
        $EdbFilePath = $EdbDirArray[$i]
        if ("$EdbFilePath" -ne "$LogDirPath" -and "$EdbFilePath" -ne "$SysDirPath")
        {
            Log "Remove temp edb file dir $EdbFilePath."
            Remove-Item -Path "$EdbFilePath" -Recurse -ErrorAction Stop

            $NewEdbFileDir = $EdbFilePath + "_Recovery"
            Log "Move edb dir $NewEdbFileDir to $EdbFilePath."
            rename-item -path "$NewEdbFileDir" "$EdbFilePath" -ErrorAction Stop
        }
    }

    Log "Remove temp log dir $LogDirPath."
    Remove-Item -Path "$LogDirPath" -Recurse -ErrorAction Stop

    $NewLogDirPath = $LogDirPath + "_Recovery"
    Log "Move edb dir $NewLogDirPath to $LogDirPath."
    rename-item -path "$NewLogDirPath" "$LogDirPath" -ErrorAction Stop

    if ("$LogDirPath" -ne "$SysDirPath")
    {
        Log "Remove temp sys dir $SysDirPath."
        Remove-Item -Path "$SysDirPath" -Recurse -ErrorAction Stop

        $NewSysDirPath = $SysDirPath + "_Recovery"
        Log "Move sys dir $NewSysDirPath to $SysDirPath."
        rename-item -path "$NewSysDirPath" "$SysDirPath" -ErrorAction Stop
    }

    Log "Restore exchange 2007 files succ."
}

function Restore2010Files($EdbFilePath, $LogDirPath)
{
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    Log "Begin restore exchange 2010 files."

    if ("" -eq "$EdbFilePath")
    {
        Log "Edb file path is empty."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }

    $EdbFileDir = Split-Path "$EdbFilePath"
    if ("" -eq "$EdbFileDir")
    {
        Log "Get dir path for edb file path failed."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }
    
    Log "Remove temp edb file dir $EdbFileDir."
    Remove-Item -Path "$EdbFileDir" -Recurse -ErrorAction Stop
    
    $NewEdbFileDir = $EdbFileDir + "_Recovery"
    Log "Move edb dir $NewEdbFileDir to $EdbFileDir."
    rename-item -path "$NewEdbFileDir" "$EdbFileDir" -ErrorAction Stop

    $ContTmp = EdbDirContainLogDir $EdbFileDir $LogDirPath
    if (0 -ne $ContTmp)
    {
        Log "Remove temp log dir $LogDirPath."
        Remove-Item -Path "$LogDirPath" -Recurse -ErrorAction Stop
        
        $NewLogDirPath = $LogDirPath + "_Recovery"
        Log "Move edb dir $NewLogDirPath to $LogDirPath."
        rename-item -path "$NewLogDirPath" "$LogDirPath" -ErrorAction Stop
    }

    Log "Restore exchange 2010 files succ."
}

function Restore2013Files($EdbFilePath, $LogDirPath)
{
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    Log "Begin restore exchange 2013 files."

    if ("" -eq "$EdbFilePath")
    {
        Log "Edb file path is empty."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }

    #move edb files to new location
    $EdbFileDir = Split-Path "$EdbFilePath"
    if ("" -eq "$EdbFileDir")
    {
        Log "Get dir path for edb file path failed."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }
    $NewEdbFileDir = $EdbFileDir + "_Recovery"
    Log "Get edb file dir $EdbFileDir and edb backup dir $NewEdbFileDir."

    $EdbFileName = Split-Path -Leaf "$EdbFilePath"
    if ("" -eq "$EdbFileName")
    {
        Log "Get edb file name failed."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }
    
    $ContTmp = EdbDirContainLogDir $EdbFileDir $LogDirPath
    if (0 -ne $ContTmp)
    {
        #move files in log dir to new location
        $NewLogDirPath = $LogDirPath + "_Recovery"
        Log "Move files in $NewLogDirPath to $LogDirPath."
        Move-Item -path "$NewLogDirPath\*" -destination "$LogDirPath" -ErrorAction Stop
    
        Log "Remove temp log dir $NewLogDirPath."
        Remove-Item -Path "$NewLogDirPath" -Recurse -ErrorAction Stop
    }
    else
    {
        if ("$EdbFileDir" -ne "$LogDirPath")
        {
            $LogTmpFolder = $EdbFileDir + "\\" + $gLogPareFolder

            Log "Remove empty log dir $LogTmpFolder."
            
            Remove-Item -Path "$LogTmpFolder" -Recurse -ErrorAction Stop
        }
    }
    
    Log "Get edb file name $EdbFileName."

    $NewEdbFilePath = $NewEdbFileDir + "\\" + $EdbFileName
    Log "Move edb file $NewEdbFilePath to $EdbFilePath."
    Move-Item -path "$NewEdbFileDir\*" -destination "$EdbFileDir" -ErrorAction Stop
    
    #delete tmp files
    Log "Remove temp edb dir $NewEdbFileDir."
    Remove-Item -Path "$NewEdbFileDir" -Recurse -ErrorAction Stop

    Log "Restore exchange 2013 files succ."
}

function RehomeUsers($NewMailboxDbName, $OldMailboxDbName)
{
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    Log "Begin rehome users in mailbox db."
 
    Get-Mailbox -Database "$OldMailboxDbName" |where {$_.ObjectClass -NotMatch '(SystemAttendantMailbox|ExOleDbSystemMailbox)'}| Set-Mailbox -Database "$NewMailboxDbName" -force -ErrorAction Stop

    Log "Rehome users in mailbox db succ."
}

function RemoveCatalogData($EdbFilePath, $Filtrate)
{
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    Log "Begin remove $Filtrate files."

    if ("" -eq "$EdbFilePath")
    {
        Log "Edb file path is empty."
        "$ERR_FILE_IS_EMPTY" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_FILE_IS_EMPTY)
    }

    $EdbFileDir = Split-Path "$EdbFilePath"
    
    $CataLogDirNames = @(Get-ChildItem -Path "$EdbFileDir" -Filter $Filtrate -ErrorAction Stop)
    
    foreach ($CataLogDirName in $CataLogDirNames)
    {
        $CataLogName = $CataLogDirName.Name
        
        $CataLogDir = join-path -path "$EdbFileDir" -childpath "$CataLogName"
        
        Log "Remove the file $Filtrate."
        Remove-Item -Path "$CataLogDir" -Recurse -ErrorAction Stop
    }

    Log "Remove $Filtrate files succ."
}

#0:NewMailboxDbName not exist; 1:NewMailboxDbName exist
function CheckDbFor201x($NewMailboxDbName, $CurrentServerName)
{
    $ERR_CODE = $ERR_SCRIPT_EXEC_FAILED
    Log "Check db $NewMailboxDbName in server $CurrentServerName."
    $MailBoxDbs = @(Get-Mailboxdatabase -status -server "$CurrentServerName" -ErrorAction Stop)
    
    foreach ($mailboxdb in $MailBoxDbs)
    {
        $DBName =  $mailboxdb.Name
        $DBMount = $mailboxdb.Mounted
        
        if ("$DBName" -eq "$NewMailboxDbName")
        {
            Log "DB $NewMailboxDbName in server $CurrentServerName is exist, status $DBMount then continue."
            
            if ("$DBMount" -eq "True")
            {
                Log "DB $NewMailboxDbName in server $CurrentServerName is mounted, so operator succ, then exit 0."
                exit(0)
            }
            return 1
        }
    }
    
    Log "DB $NewMailboxDbName in server $CurrentServerName is not exist, then continue."
    return 0
}

#try mount database in multi-AD if mount database failed
#try 5 times in 10 min
function TryMountDBInMultiAD($TryTimes, $NewMailboxDbName)
{
    $TryTimes++
    if ($TryTimes -lt $TryTotalTimes)
    {
        try
        {
            Log "Try to mount db $NewMailboxDbName in times[$TryTimes]."
            Mount-Database "$NewMailboxDbName" -Confirm:$false -ErrorAction Stop
        }
        catch
        {            
            Start-Sleep -s 120
            TryMountDBInMultiAD $TryTimes "$NewMailboxDbName"
        }
    }
    else
    {
        Log "Mount db $NewMailboxDbName out of time."
        "$ERR_MOUNT_INMULTIAD_FAIL" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_MOUNT_INMULTIAD_FAIL)
    }
}

function RecoveryExchange201x($NewMailboxDbName, $OldMailboxDbName, $EdbFilePath, $LogDirPath, $CurrentServerName, $RecoveryType)
{
    Log "Begin recovery exchange 201x."
    
    $iFlag = CheckDbFor201x $NewMailboxDbName $CurrentServerName
    
    ExecSoftRecovery201x $LogDirPath $EdbFilePath $OldMailboxDbName

    if ($bDBNotExit -eq $iFlag)
    {
        RemoveCatalogData $EdbFilePath "Catalog*"
        
        Backup2010Files $EdbFilePath $LogDirPath
    
        Log "Create new mailbox db $NewMailboxDbName."
        $ERR_CODE = $ERR_RECOVERY_DB_FAILED
        New-MailboxDatabase -Name "$NewMailboxDbName" -Server "$CurrentServerName" -EdbFilePath "$EdbFilePath" -LogFolderPath "$LogDirPath"  -ErrorAction Stop
    }
    
    Log "Set mailbox db $NewMailboxDbName allow file restore property"
    Set-MailboxDatabase "$NewMailboxDbName" -AllowFileRestore:$true  -ErrorAction Stop

    if ($bDBNotExit -eq $iFlag)
    {    
        Restore2010Files $EdbFilePath $LogDirPath
    }

    Log "Mount mailbox db $NewMailboxDbName."
    try
    {
        $ERR_CODE = $ERR_RECOVERY_DB_FAILED
        Mount-Database "$NewMailboxDbName" -Confirm:$false -ErrorAction Stop
    }
    catch
    {
        $TryTimes = 0
        $MountFailErr = $Error[0]
        
        $MountFailErrorFilter1 = "*(hr=0x8004010f, ec=-2147221233)*"
        $MountFailErrorFilter2 = "The operation couldn't be performed because object '" + $NewMailboxDbName + "' couldn't be found on*"
        
        #try to mount db in Multi-AD
        if (($MountFailErr -like $MountFailErrorFilter1) -or ($MountFailErr -like $MountFailErrorFilter2))
        {   
            Log "$MountFailErr"
            TryMountDBInMultiAD $TryTimes $NewMailboxDbName
        }
        else
        {
            $lineNumber = $Error[0].InvocationInfo.scriptlinenumber    
            Log "ERROR LINE NUMBER $lineNumber, DESCRIPTION $MountFailErr"
            "$ERR_RECOVERY_DB_FAILED" | Out-File "$gResultFile" -Encoding ASCII -Append
            exit($ERR_RECOVERY_DB_FAILED)
        }
    }
    
    if ($RecoveryType -eq $RecoverTypeRestore)
    {
        RehomeUsers $NewMailboxDBName $OldMailboxDBName
    }

    Log "Recovery exchange 201x succ."
}

function RecoveryExchange2010($NewMailboxDbName, $OldMailboxDbName, $EdbFilePath, $LogDirPath, $CurrentServerName, $RecoveryType)
{
    RecoveryExchange201x $NewMailboxDbName $OldMailboxDbName $EdbFilePath $LogDirPath $CurrentServerName $RecoveryType
}

function RecoveryExchange2013($NewMailboxDbName, $OldMailboxDbName, $EdbFilePath, $LogDirPath, $CurrentServerName, $RecoveryType)
{  
    Log "Begin recovery exchange 2013."
    
    $iFlag = CheckDbFor201x $NewMailboxDbName $CurrentServerName

    ExecSoftRecovery201x $LogDirPath $EdbFilePath $OldMailboxDbName
    
    if ($bDBNotExit -eq $iFlag)
    {   
        RemoveCatalogData $EdbFilePath "*Single"
        
        Backup2013Files $EdbFilePath $LogDirPath
    
        Log "Create new mailbox db $NewMailboxDbName, Edb file path $EdbFilePath, Log folder path $LogDirPath."
        $ERR_CODE = $ERR_RECOVERY_DB_FAILED
        New-MailboxDatabase -Name "$NewMailboxDbName" -Server "$CurrentServerName" -EdbFilePath "$EdbFilePath" -LogFolderPath "$LogDirPath" -ErrorAction Stop
    }

    Log "Set mailbox db $NewMailboxDbName allow file restore property"
    Set-MailboxDatabase "$NewMailboxDbName" -AllowFileRestore:$true -ErrorAction Stop
    
    if ($bDBNotExit -eq $iFlag)
    {
        Restore2013Files $EdbFilePath $LogDirPath
    }

    Log "Mount mailbox db $NewMailboxDbName."
    $ERR_CODE = $ERR_RECOVERY_DB_FAILED
    Mount-Database -Identity "$NewMailboxDbName" -Confirm:$false -ErrorAction Stop
    Log "Mount mailbox db $NewMailboxDbName end."

    if ($RecoveryType -eq $RecoverTypeRestore)
    {
        RehomeUsers $NewMailboxDBName $OldMailboxDBName
    }

    Log "Recovery exchange 2013 succ."
}

function CheckStorageGroup($NewStorageGroupName, $CurrentServerName)
{
    Log "Check storagegroup $NewStorageGroupName in server $CurrentServerName."
    $StorageGroups = @(Get-StorageGroup -Server "$CurrentServerName" -ErrorAction Stop)
    
    foreach ($strageGroup in $StorageGroups)
    {
        $strGroupname = $strageGroup.Name
        
        if ("$strGroupname" -eq "$NewStorageGroupName")
        {
            Log "StorageGroup $NewMailboxDbName in server $CurrentServerName is exist, then continue."
            return 1
        }
    }
    
    Log "StorageGroup $NewMailboxDbName in server $CurrentServerName is not exist, then continue."
    return 0
}

function CheckDbFor2007($NewStorageGroupName, $CurrentServerName, $NewMailboxDBName)
{
    Log "Check DB $NewMailboxDBName in storage group $NewStorageGroupName."
    $DBInfos = @(Get-MailBoxDataBase -StorageGroup "$CurrentServerName\$NewStorageGroupName" -ErrorAction Stop)
    
    foreach ($DBInfo in $DBInfos)
    {
        $DBName = $DBInfo.Name
        
        if ("$DBName" -eq "$NewMailboxDBName")
        {
            Log "DB $NewMailboxDbName in storage group $NewStorageGroupName is exist, then continue."
            return 1
        }
    }
    
    Log "DB $NewMailboxDbName in storage group $NewStorageGroupName is not exist, then continue."
    return 0
}

function RecoveryExchange2007($NewStorageGroupName, $OldStorageGroupName, $NewMailboxDBNames, $EdbFilePaths, $LogDirPath, $SysDirPath, $ServerName)
{
    Log "Begin recovery exchange 2007."

    $MailboxDBNameArray = $NewMailboxDBNames -split $InfoSeparator
    $EdbFilePathArray = $EdbFilePaths -split $InfoSeparator
    
    ExecSoftRecovery2007 $LogDirPath $EdbFilePathArray $MailboxDBNameArray $OldStorageGroupName $ServerName

    Backup2007Files $EdbFilePathArray $LogDirPath $SysDirPath

    $iFlag = CheckStorageGroup $NewStorageGroupName $CurrentServerName
    
    if ($bDBNotExit -eq $iFlag)
    {
        Log "Create new storage group, storage group $CurrentServerName, server name $NewStorageGroupName."
        New-StorageGroup -Name "$NewStorageGroupName" -Server "$CurrentServerName" -LogFolderPath:"$LogDirPath" -SystemFolderPath:"$SysDirPath"  -ErrorAction Stop
    }
    
    for ($i = 0; $i -lt $MailboxDBNameArray.count; $i++)
    {
        $MailboxDBName = $MailboxDBNameArray[$i]
        $EdbFilePath = $EdbFilePathArray[$i]
        
        $iFlag = CheckDbFor2007 $NewStorageGroupName $CurrentServerName $MailboxDBName
        
        if ($bDBNotExit -eq $iFlag)
        {
            Log "Create new MailboxDatabase $MailboxDBName in storage group $NewStorageGroupName."
            New-MailboxDatabase -StorageGroup "$CurrentServerName\$NewStorageGroupName" -Name "$MailboxDBName" -EdbFilePath "$EdbFilePath" -ErrorAction Stop
        }
        
        Log "Set mailbox db $MailboxDBName allow file restore property."
        Set-MailboxDatabase "$MailboxDBName" -AllowFileRestore:$true -ErrorAction Stop
    }
    
    Restore2007Files $EdbFilePathArray $LogDirPath $SysDirPath
    
    for ($i = 0; $i -lt $EdbFilePathArray.count; $i++)
    {
        $EdbFilePa = $EdbFilePathArray[$i]
        
        RemoveCatalogData $EdbFilePa
    }

    for ($i = 0; $i -lt $MailboxDBNameArray.count; $i++)
    {
        $MailboxDBName = $MailboxDBNameArray[$i]

        Log "Mount mailbox db $NewMailboxDbName."
        Mount-Database "$MailboxDBName" -Confirm:$false -ErrorAction Stop

        Get-Mailbox -Database "$ServerName\$OldStorageGroupName\$MailboxDBName" |where {$_.ObjectClass -NotMatch '(SystemAttendantMailbox|ExOleDbSystemMailbox)'}| Move-Mailbox -ConfigurationOnly -TargetDatabase "$CurrentServerName\$NewStorageGroupName\$MailboxDBName"
    }
    
    Log "Recovery exchange 2007 succ."
}

function RecoveryExchange($ExchangeVersion)
{
    if ($ExchangeVersion -eq $Exchange2007Version)
    {
        RecoveryExchange2007 $gNewStorageGroupName $gOldStorageGroupName $gNewMailboxDBName $gEdbFilePath $gLogDirPath $gSysDirPath $gServerName $gRecoveryType
    }
    elseif ($ExchangeVersion -eq $Exchange2010Version)
    {
        RecoveryExchange2010 $gNewMailboxDBName $gOldMailboxDBName $gEdbFilePath $gLogDirPath $CurrentServerName $gRecoveryType
    }
    elseif ($ExchangeVersion -eq $Exchange2013Version)
    {
        RecoveryExchange2013 $gNewMailboxDBName $gOldMailboxDBName $gEdbFilePath $gLogDirPath $CurrentServerName $gRecoveryType
    }
    else
    {
        Log "Ivalid exchange Version $ExchangeVersion."
        exit(1)
    }
}

function ExchangeOper($OperType, $ExhcangeVersion)
{
    try
    {
        if ($OperType -eq $OperTypeRecovery)
        {
            if ($gRecoveryType -eq  $RecoverTypeRollback)
            {
                Log "Begin rollback exchange 201x."
                $iFlag = CheckDbFor201x $gNewMailboxDBName $CurrentServerName
                
                #$gNewMailboxDBName is not exist, then exit
                if ($iFlag -eq 0)
                {
                    Log "$gNewMailboxDBName is not exist, can't exec roolback, then exit."
                    "$ERR_RECOVERY_DB_FAILED" | Out-File "$gResultFile" -Encoding ASCII -Append
                    exit($ERR_RECOVERY_DB_FAILED)
                }
                
                $ERR_CODE = $ERR_MOUNT_INMULTIAD_FAIL
                Mount-Database -Identity "$gNewMailboxDBName" -Confirm:$false -ErrorAction Stop
                
                Log "Rollback exchange 201x succ."
            }
            else
            {
                RecoveryExchange $ExhcangeVersion
            }
        }
        elseif($OperType -eq $OpertypeDismount)
        {
            DismountExchange $ExhcangeVersion
        }
        elseif ($OperType -eq $OperTypeClear)
        {
            ClearExchange $ExhcangeVersion
        }
        else
        {
            Log "Ivalid exchange oper type $OperType."
            exit(1)
        }
    }
    catch
    {
        $lineNumber = $Error[0].InvocationInfo.scriptlinenumber
        $ErrorInfos = $Error[0]
        Log "ERROR LINE NUMBER $lineNumber, DESCRIPTION $ErrorInfos"
        "$ERR_CODE" | Out-File "$gResultFile" -Encoding ASCII -Append
        exit($ERR_CODE)
    }
}

ParseInputParams $args
ExchangeOper $gOperType $gVersion

exit(0)

