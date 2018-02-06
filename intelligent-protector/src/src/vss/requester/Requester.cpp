/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "vss/requester/Requester.h"
#include "common/String.h"
#include <algorithm>

#ifdef WIN32
VSSRequester::VSSRequester()
{
    m_pVssObject = NULL;
    m_pAsyncSnapshot = NULL;
    m_hEventFrozen = NULL;
    m_hEventThaw = NULL;
    m_hEventTimeOut = NULL;
}

VSSRequester::~VSSRequester()
{
    CleanUp();
}

int VSSRequester::Init()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin Init vss requester.");

    COMInitializer initializer;
    HRESULT hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hr))
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Com security init failed, Returned HRESULT = 0x%08lx", hr);

        return -1;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Com security init succ.");

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Init vss requester succ.");

    return 0;
}

int VSSRequester::GetWriterName(vss_freeze_type eFreezeType, wstring& wstrWriterName)
{
    if (vss_freeze_sqlserver == eFreezeType)
    {
        wstrWriterName = CMpString::String2WString(VSS_SQLSERVER_WRITER_NAME);
    }
    else if (vss_freeze_exchange == eFreezeType)
    {
        wstrWriterName = CMpString::String2WString(VSS_EXCHANGE_WRITER_NAME);
    }
    else
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Invalid freeze type %d.", eFreezeType);
        return MP_FAILED;
    }

    return 0;
}

int VSSRequester::Freeze(vector<vss_db_oper_info_t>& vecVssDBInfos, vss_freeze_type eFreezeType)
{
    int iRet = 0;
    vector<string> vecDrvieLetters;
    vector<wstring> vecVolumes;
    list<string> lstDBNames;
    list<string>::iterator iter;
    wstring wstrDBName;
    vector<wstring> vecComponents;
    int iErrCode = 0;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze db, freeze type %d.", eFreezeType);
    m_vecDbInfos = vecVssDBInfos;
    m_eFreezeType = eFreezeType;
    GetDriveLetterList(vecVssDBInfos, vecDrvieLetters);
    GetDbNameList(vecVssDBInfos, lstDBNames);
    for (iter = lstDBNames.begin(); iter != lstDBNames.end(); iter++)
    {
        wstrDBName = CMpString::String2WString(*iter);
        m_vecDBNames.push_back(wstrDBName);
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get unicode db name %s.", wstrDBName.c_str());
    }

    iRet = GetWriterName(eFreezeType, m_wstrWriterName);
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Get writer name failed.");
        return iRet;
    }
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Specify vss writer, writer name %s.", m_wstrWriterName.c_str());

    iRet = GetVolumes(vecDrvieLetters, vecVolumes);
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, OS_LOG_ERROR, L"Get volume paths for drive letters failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = CreateEvents();
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Create events failed.");
        return iRet;
    }

    iRet = FreezeProc(vecVolumes, eFreezeType);
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, OS_LOG_ERROR, L"Freeze volumes failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze db succ");
    return iRet;
}

int VSSRequester::Freeze(vector<string>& vecDriveLetters, vss_freeze_type eFreezeType)
{
    int iRet = 0;
    list<string> lstDrvieLetters;
    vector<wstring> vecVolumes;
    int iErrCode = 0;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze file sys, freeze type %d.", eFreezeType);
    m_eFreezeType = eFreezeType;
    iRet = GetVolumes(vecDriveLetters, vecVolumes);
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, OS_LOG_ERROR, L"Get volume paths for drive letters failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = CreateEvents();
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Create events failed.");
        return iRet;
    }

    iRet = FreezeProc(vecVolumes, eFreezeType);
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, OS_LOG_ERROR, L"Freeze volumes failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Freeze file sys, succ.");

    return iRet;
}

int VSSRequester::FreezeAll()
{
    int iRet = 0;
    vector<wstring> vecVolumesList;

    iRet = GetAllVolumes(vecVolumesList);
    if (0 != iRet)
    {
        return iRet;
    }

    iRet = CreateEventsEx();
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Create events failed.");
        return iRet;
    }

    m_eFreezeType = vss_freeze_filesys;
    iRet = FreezeProc(vecVolumesList, vss_freeze_filesys);
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, OS_LOG_ERROR, L"Freeze volumes failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Freeze file sys, succ.");

    return iRet;
}

int VSSRequester::FreezeProc(vector<wstring> vecVolumePaths, vss_freeze_type eFreezeType)
{
    int iRet = 0;
    list<string>::iterator iter;
    wstring wstrDBName;
    vector<wstring> vecIncComponents;
    int iErrCode = 0;

    // 打开日志缓存
    //CLogger::GetInstance().OpenLogCache();
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze volume, freeze type %d.", eFreezeType);
    try
    {
        CoInitialize(NULL);
        if (vss_freeze_filesys != eFreezeType)
        {
            GetComponents(m_wstrWriterName, m_vecDBNames, vecIncComponents);
        }

        //文件系统不指定必须包含的vecIncComponents
        CreateSnapShot(vecVolumePaths, vecIncComponents);
    }
    //需要给server返回特定错误码的操作都需要抛出该异常
    catch (ReqException reqExcption)
    {
        iErrCode = reqExcption.GetErrCode();
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Create snapshot set failed, errcode: 0x%x", iErrCode);
        FreezeFailedProcess();
        //关闭日志缓存
        //CLogger::GetInstance().CloseLogCache();
        return iErrCode;
    }
    catch (HRESULT hr)
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Create snapshot set failed, HRESULT Error caught: 0x%08lx", hr);
        FreezeFailedProcess();
        //关闭日志缓存
        //CLogger::GetInstance().CloseLogCache();
        return MP_FAILED;
    }

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Freeze volume succ.");

    return iRet;
}

int VSSRequester::UnFreeze(vector<vss_db_oper_info_t>& vecVssDBInfos)
{
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Begin thaw.");
    int iRet = 0;
    bool bIsMatch = true;

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Begin unfreeze db.");
    bIsMatch = IsMatch(m_vecDbInfos, vecVssDBInfos);
    if (!bIsMatch)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The unfreeze request dose not match.");
        return ERROR_INNER_THAW_NOT_MATCH;
    }

    iRet = UnFreezeProc();
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, OS_LOG_ERROR, L"Unfreeze volumes failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Unfreeze db succ.");
    return iRet;
}

int VSSRequester::UnFreeze(vector<string>& vecDriveLetters)
{
    int iRet = 0;
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Begin unfreeze file sys.");

    iRet = UnFreezeProc();
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, OS_LOG_ERROR, L"Unfreeze volumes failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Unfreeze file sys succ.");
    return iRet;
}

int VSSRequester::UnFreezeAll()
{
    int iRet = 0;
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Begin unfreeze all.");
    
    SetEvent(m_hEventThaw);
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Set unfreeze event.");
    try
    {
        WaitThawReturn(false);
    }
    catch (ReqException reqExcption)
    {
        iRet = reqExcption.GetErrCode();
        m_pVssObject->AbortBackup();
        CleanUpEx();
        CoUninitialize();
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Wait unfreeze failed, errcode: 0x%x", iRet);
    }
    catch (HRESULT hr)
    {
        m_pVssObject->AbortBackup();
        iRet = ERROR_COMMON_OPER_FAILED;
        CleanUpEx();
        CoUninitialize();
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Wait unfreeze failed, HRESULT Error caught: 0x%08lx", hr);
    }

    //关闭日志缓存
    //CLogger::GetInstance().CloseLogCache();

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Unfreeze all succ.");
    return iRet;
}

int VSSRequester::EndBackup(int iBackupSucc)
{
    int iRet = 0;
    bool bSucceeded = true;
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Begin end bakcup.");

    bSucceeded = iBackupSucc ? true : false;
    try
    {
        BackupComplete(bSucceeded);
    }
    catch (ReqException reqExcption)
    {
        iRet = reqExcption.GetErrCode();
        m_pVssObject->AbortBackup();
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Wait unfreeze failed, errcode: 0x%x", iRet);
    }
    catch (HRESULT hr)
    {
        m_pVssObject->AbortBackup();
        iRet = ERROR_COMMON_OPER_FAILED;
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Wait unfreeze failed, HRESULT Error caught: 0x%08lx", hr);
    }
    
    CleanUpEx();
    CoUninitialize();
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"End backup succ.");
    return iRet;
}

int VSSRequester::UnFreezeProc()
{
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Begin thaw.");
    int iRet = 0;
    
    SetEvent(m_hEventThaw);
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Set thaw event.");

    try
    {
        WaitThawReturn(true);
    }
    catch (ReqException reqExcption)
    {
        iRet = reqExcption.GetErrCode();
        m_pVssObject->AbortBackup();
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Wait thaw failed, errcode: 0x%x", iRet);
    }
    catch (HRESULT hr)
    {
        m_pVssObject->AbortBackup();
        iRet = MP_FAILED;
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Wait thaw failed, HRESULT Error caught: 0x%08lx", hr);
    }

    CleanUp();
    CoUninitialize();
    
    //关闭日志缓存
    //CLogger::GetInstance().CloseLogCache();
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Thaw succ.");

    return iRet;
}

bool VSSRequester::IsMatch(vector<vss_db_oper_info_t>& vecDbInfoSrc, vector<vss_db_oper_info_t>& vecDbInfoDes)
{
    list<string> lstSrc;
    list<string> lstDes;
    list<string>::iterator iter;
    int iRet = 0;

    if (vecDbInfoSrc.size() != vecDbInfoDes.size())
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The count of elements is not equal.");

        return false;
    }
    
    return true;
}

void VSSRequester::GetDriveLetterList(vector<vss_db_oper_info_t>& vecVssDBInfos, vector<string>& vecDriveLetters)
{
    vector<vss_db_oper_info_t>::iterator iterDbInfo;
    vector<string>::iterator iterDriveLetter;
    mp_string strDriveLetter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get drive letter list.");
    for (iterDbInfo = vecVssDBInfos.begin(); iterDbInfo != vecVssDBInfos.end(); iterDbInfo++)
    {
        for (iterDriveLetter = iterDbInfo->vecDriveLetters.begin(); iterDriveLetter != iterDbInfo->vecDriveLetters.end();
            iterDriveLetter++)
        {
            strDriveLetter = *iterDriveLetter;
            if (find(vecDriveLetters.begin(), vecDriveLetters.end(), strDriveLetter) != vecDriveLetters.end())
            {
                COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Drive letter %s exist in list, skip it.", strDriveLetter.c_str());
                continue;
            }

            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Add new drive letter to list, drive letter %s.", strDriveLetter.c_str());
            vecDriveLetters.push_back(strDriveLetter);
        }
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End get drive letter list.");
}

void VSSRequester::GetDbNameList(vector<vss_db_oper_info_t>& vecVssDBInfos, list<string>& lstDbNames)
{
    vector<vss_db_oper_info_t>::iterator iterDbInfo;
    mp_string strDbName;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End get db name list.");
    for (iterDbInfo = vecVssDBInfos.begin(); iterDbInfo != vecVssDBInfos.end(); iterDbInfo++)
    {
        strDbName = iterDbInfo->strDbName;
        if (find(lstDbNames.begin(), lstDbNames.end(), strDbName) != lstDbNames.end())
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Db name %s exist in list, skip it.", strDbName.c_str());
            continue;
        }

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Add new db name to list, db name %s.", strDbName.c_str());
        lstDbNames.push_back(strDbName);
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End get db name list.");
}

void VSSRequester::GetComponents(wstring& wstrWriterName, vector<wstring>& vecDBNames,
    vector<wstring>& vecIncComponents)
{
    wstring wstrComponentName;
    vector<wstring>::iterator iter;

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin get components, writer name %s.", wstrWriterName.c_str());

    for (iter = vecDBNames.begin(); iter != vecDBNames.end(); iter++)
    {
        wstrComponentName = wstrWriterName + L":" + *iter;
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get unicode component name %s.",
            wstrComponentName.c_str());

        vecIncComponents.push_back(wstrComponentName);
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get components succ.");
}

int VSSRequester::GetVolumes(vector<string>& vecDriveLetters, vector<wstring>& vecVolumes)
{
    vector<string>::iterator iter;
    wstring wstrVolume;
    wstring wstrPartition;
    string strTmp;
    size_t iPos;

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin get volumes.");

    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); iter++)
    {
        strTmp = *iter;
        iPos = strTmp.find(":");
        if (string::npos != iPos)
        {
            strTmp = strTmp.substr(0, iPos);
        }

        strTmp = strTmp.append(":\\");
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Curr partition is %s.", strTmp.c_str());

        wstrPartition = CMpString::String2WString(strTmp);
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get unicode partition name %s.", wstrPartition.c_str());
        if (IsVolume(wstrPartition))
        {
            COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Volume %s was added as parameter.", wstrPartition.c_str());

            wstrVolume = GetUniqueVolumeNameForPath(wstrPartition);
            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get volume name %s.", wstrVolume.c_str());

            vecVolumes.push_back(wstrVolume);
        }
        else
        {
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"ERROR: invalid parameters %s", wstrPartition.c_str());
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"- Parameter %s is expected to be a volume!",
                wstrPartition.c_str());
            //throw(E_INVALIDARG);
            return MP_FAILED;
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get volumes succ.");
    return 0;
}

int VSSRequester::CreateEvents()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin create events.");

    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    m_hEventFrozen = CreateEventW(&sa, TRUE, FALSE, EVENT_NAME_FROZEN);
    if (!m_hEventFrozen)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Failed to create event %s, err %d.",
            EVENT_NAME_FROZEN, GetLastError());
        return MP_FAILED;
    }
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Create frozen events.");


    m_hEventThaw = CreateEventW(&sa, TRUE, FALSE, EVENT_NAME_THAW);
    if (!m_hEventThaw)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Failed to create event %s, err %d.",
            EVENT_NAME_THAW, GetLastError());
        return MP_FAILED;
    }
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Create thaw events.");

    m_hEventTimeOut = CreateEventW(&sa, TRUE, FALSE, EVENT_NAME_TIMEOUT);
    if (!m_hEventTimeOut)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Failed to create event %s, err %d.",
            EVENT_NAME_TIMEOUT, GetLastError());
        return MP_FAILED;
    }
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Create timeout events.");

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Create events succ.");

    return 0;
}

int VSSRequester::CreateEventsEx()
{
    int iRet = 0;

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin create all events.");
    
    iRet = CreateEvents();
    if (0 != iRet)
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Create events failed.");
        return iRet;
    }
    
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    m_hEventEndBackup = CreateEventW(&sa, TRUE, FALSE, EVENT_NAME_ENDBACKUP);
    if (!m_hEventTimeOut)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Failed to create event %s, err %d.",
            EVENT_NAME_ENDBACKUP, GetLastError());

        return ERROR_COMMON_OPER_FAILED;
    }
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Create endbackup events.");

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Create all events succ.");
    return 0;
}

void VSSRequester::InitializeWriterMetadata(wstring* pwstrWriterName)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin init writer metadata.");

    unsigned cWriters = 0;
    CHECK_COM(m_pVssObject->GetWriterMetadataCount(&cWriters));
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get writer metadata count %u.", cWriters);

    for (unsigned iWriter = 0; iWriter < cWriters; iWriter++)
    {
        VSS_ID idInstance = GUID_NULL;
        CComPtr<IVssExamineWriterMetadata> pMetadata;
        CHECK_COM(m_pVssObject->GetWriterMetadata(iWriter, &idInstance, &pMetadata));

        VssWriter writer;
        bool bSucc = true;
        bSucc = writer.Initialize(pMetadata, pwstrWriterName);
        if (bSucc)
        {
            m_vecWriters.push_back(writer);
            COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Add writer %s to list.", writer.m_wstrName.c_str());
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Init writer metadata succ.");
}

void VSSRequester::GatherWriterMetadata(wstring* pwstrWriterName)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin gather writer metadata.");
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Gathering writer metadata...");

    // Gathers writer metadata
    // WARNING: this call can be performed only once per IVssBackupComponents instance!
    CComPtr<IVssAsync> pAsync;
    CHECK_COM(m_pVssObject->GatherWriterMetadata(&pAsync));
    // Waits for the async operation to finish and checks the result
    WaitForAsync(pAsync);

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Initialize writer metadata ...");
    // Initialize the internal metadata data structures
    InitializeWriterMetadata(pwstrWriterName);

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Gather writer metadata succ.");
}

// Discover excluded components that have file groups outside the shadow set
void VSSRequester::DiscoverNonShadowedExcludedComponents(vector<wstring> shadowSourceVolumes)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin discover components that reside outside the shadow set.");
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Discover components that reside outside the shadow set ...");

    // Discover components that should be excluded from the shadow set
    // This means components that have at least one File Descriptor requiring
    // volumes not in the shadow set.
    for (unsigned iWriter = 0; iWriter < m_vecWriters.size(); iWriter++)
    {
        VssWriter& writer = m_vecWriters[iWriter];

        // Check if the writer is excluded
        if (writer.m_bIsExcluded)
        {
            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Writer %s is excluded, skip it.", writer.m_wstrName.c_str());

            continue;
        }

        // Check if the component is excluded
        for (unsigned iComponent = 0; iComponent < writer.m_vecComponents.size(); iComponent++)
        {
            VssComponent& component = writer.m_vecComponents[iComponent];

            // Check to see if this component is explicitly excluded
            if (component.m_bIsExcluded)
            {
                COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component(name %s, caption %s) is excluded, skip it.",
                    component.m_wstrName.c_str(), component.m_wstrCaption.c_str());

                continue;
            }

            // Try to find an affected volume outside the shadow set
            // If yes, exclude the component
            for (unsigned iVol = 0; iVol < component.m_vecAffectedVolumes.size(); iVol++)
            {

                if (!FindStringInList(component.m_vecAffectedVolumes[iVol], shadowSourceVolumes))
                {
                    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Component '%s' from writer '%s' is excluded from backup "
                        L"(it requires %s in the shadow set)", component.m_wstrFullPath.c_str(), writer.m_wstrName.c_str(),
                        GetDisplayNameForVolume(component.m_vecAffectedVolumes[iVol]).c_str());

                    component.m_bIsExcluded = true;

                    break;
                }
            }
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Discover components that reside outside the shadow set succ.");
}

// Discover the components that should not be included (explicitly or implicitly)
// These are componenets that are have directly excluded descendents
void VSSRequester::DiscoverAllExcludedComponents()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin discover all excluded components.");
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Discover all excluded components ...");

    // Discover components that should be excluded from the shadow set
    // This means components that have at least one File Descriptor requiring
    // volumes not in the shadow set.
    for (unsigned iWriter = 0; iWriter < m_vecWriters.size(); iWriter++)
    {
        VssWriter& writer = m_vecWriters[iWriter];
        if (writer.m_bIsExcluded)
        {
            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Writer %s is excluded, skip it.", writer.m_wstrName.c_str());

            continue;
        }

        // Enumerate all components
        for (unsigned i = 0; i < writer.m_vecComponents.size(); i++)
        {
            VssComponent& component = writer.m_vecComponents[i];

            // Check if this component has any excluded children
            // If yes, deselect it
            for (unsigned j = 0; j < writer.m_vecComponents.size(); j++)
            {
                VssComponent& descendent = writer.m_vecComponents[j];
                if (component.IsAncestorOf(descendent) && descendent.m_bIsExcluded)
                {
                    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Component '%s'(name %s, caption %s) from writer '%s' is "
                        L"excluded from backup (it has an excluded descendent: '%s' (caption %s))",
                        component.m_wstrFullPath.c_str(), component.m_wstrName.c_str(), component.m_wstrCaption.c_str(),
                        writer.m_wstrName.c_str(), descendent.m_wstrName.c_str(), descendent.m_wstrCaption.c_str());

                    component.m_bIsExcluded = true;

                    break;
                }
            }
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Discover all excluded components succ.");
}

// Discover excluded writers. These are writers that:
// - either have a top-level nonselectable excluded component
// - or do not have any included components (all its components are excluded)
void VSSRequester::DiscoverExcludedWriters()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin discover excluded writers.");
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Discover excluded writers ...");

    // Enumerate writers
    for (unsigned iWriter = 0; iWriter < m_vecWriters.size(); iWriter++)
    {
        VssWriter& writer = m_vecWriters[iWriter];
        if (writer.m_bIsExcluded)
        {
            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Writer %s is excluded, skip it.", writer.m_wstrName.c_str());

            continue;
        }

        // Discover if we have any:
        // - non-excluded selectable components
        // - or non-excluded top-level non-selectable components
        // If we have none, then the whole writer must be excluded from the backup
        writer.m_bIsExcluded = true;
        for (unsigned i = 0; i < writer.m_vecComponents.size(); i++)
        {
            VssComponent& component = writer.m_vecComponents[i];
            if (component.CanBeExplicitlyIncluded())
            {
                writer.m_bIsExcluded = false;
                COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component(name %s, caption %s) can be explicitly included, "
                    L"writer %s is selectable.", component.m_wstrName.c_str(), component.m_wstrCaption.c_str(),
                    writer.m_wstrName.c_str());

                break;
            }
        }

        // No included components were found
        if (writer.m_bIsExcluded)
        {
            COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- The writer '%s' is now entirely excluded from the backup:",
                writer.m_wstrName.c_str());
            COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"  (it does not contain any components that can be potentially included in the backup)");

            continue;
        }

        // Now, discover if we have any top-level excluded non-selectable component
        // If this is true, then the whole writer must be excluded from the backup
        for (unsigned i = 0; i < writer.m_vecComponents.size(); i++)
        {
            VssComponent& component = writer.m_vecComponents[i];

            if (component.m_bIsTopLevel && !component.m_bIsSelectable && component.m_bIsExcluded)
            {
                COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- The writer '%s' is now entirely excluded from the backup:",
                    writer.m_wstrName.c_str());
                COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"  (the top-level non-selectable component '%s' is an excluded component)",
                    component.m_wstrFullPath.c_str());

                writer.m_bIsExcluded = true;
                break;
            }
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Discover excluded writers succ.");
}

// Discover the components that should be explicitly included
// These are any included top components
void VSSRequester::DiscoverExplicitelyIncludedComponents()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin discover explicitly included components.");
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Discover explicitly included components ...");

    // Enumerate all writers
    for (unsigned iWriter = 0; iWriter < m_vecWriters.size(); iWriter++)
    {
        VssWriter& writer = m_vecWriters[iWriter];
        if (writer.m_bIsExcluded)
        {
            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Writer %s is excluded, skip it.", writer.m_wstrName.c_str());

            continue;
        }

        // Compute the roots of included components
        for (unsigned i = 0; i < writer.m_vecComponents.size(); i++)
        {
            VssComponent& component = writer.m_vecComponents[i];

            if (!component.CanBeExplicitlyIncluded())
            {
                COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component(name %s, caption %s) can not be explicitly included, skip it.",
                    component.m_wstrName.c_str(), component.m_wstrCaption.c_str());

                continue;
            }

            // Test if our component has a parent that is also included
            component.m_bIsExplicitlyIncluded = true;
            for (unsigned j = 0; j < writer.m_vecComponents.size(); j++)
            {
                VssComponent& ancestor = writer.m_vecComponents[j];
                if (ancestor.IsAncestorOf(component) && ancestor.CanBeExplicitlyIncluded())
                {
                    // This cannot be explicitely included since we have another
                    // ancestor that that must be (implictely or explicitely) included
                    component.m_bIsExplicitlyIncluded = false;
                    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component(name %s, caption %s) cannot be explicitely included "
                        L"since we have another ancestor that must be included.", component.m_wstrName.c_str(),
                        component.m_wstrCaption.c_str());

                    break;
                }
            }
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Discover explicitly included components succ.");
}

// Discover the components that should be explicitly included
// These are any included top components
void VSSRequester::SelectExplicitelyIncludedComponents()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin select explicitly included components.");
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Select explicitly included components ...");

    // Enumerate all writers
    for (unsigned iWriter = 0; iWriter < m_vecWriters.size(); iWriter++)
    {
        VssWriter& writer = m_vecWriters[iWriter];
        if (writer.m_bIsExcluded)
        {
            continue;
        }

        COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L" * Writer '%s':", writer.m_wstrName.c_str());

        // Compute the roots of included components
        for (unsigned i = 0; i < writer.m_vecComponents.size(); i++)
        {
            VssComponent& component = writer.m_vecComponents[i];

            if (!component.m_bIsExplicitlyIncluded)
            {
                continue;
            }

            COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"   - Add component %s(name %s, caption %s)",
                component.m_wstrFullPath.c_str(), component.m_wstrName.c_str(), component.m_wstrCaption.c_str());
            // Add the component
            CHECK_COM(m_pVssObject->AddComponent(WString2Guid(writer.m_wstrInstanceId), WString2Guid(writer.m_wstrId),
                component.m_enType, component.m_wstrLogicalPath.c_str(), component.m_wstrName.c_str()));
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Select explicitly included components succ.");
}

bool VSSRequester::VerifyOneComponent(VssComponent& component, VssWriter& writer, wstring& wstrIncludedComponent)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin verify one component '%s'.", component.m_wstrFullPath.c_str());
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Found component '%s' from writer '%s'",
        component.m_wstrFullPath.c_str(), writer.m_wstrName.c_str());

    // If not explicitly included, check to see if there is an explicitly included ancestor
    bool isIncluded = component.m_bIsExplicitlyIncluded;
    if (!isIncluded)
    {
        for (unsigned k = 0; k < writer.m_vecComponents.size(); k++)
        {
            VssComponent& ancestor = writer.m_vecComponents[k];
            if (ancestor.IsAncestorOf(component) && ancestor.m_bIsExplicitlyIncluded)
            {
                isIncluded = true;
                break;
            }
        }
    }

    if (isIncluded)
    {
        COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"  - The component \"%s\" is selected",
            wstrIncludedComponent.c_str());

        return true;
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"End verify one component.");
    return false;
}

bool VSSRequester::VerifyCompsInOneWirter(wstring& wstrIncludedComponent, VssWriter& writer)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"- Verifing \"%s\" included in writer \"%s\" ...",
        wstrIncludedComponent.c_str(), writer.m_wstrName.c_str());

    // Find the associated component
    for (unsigned j = 0; j < writer.m_vecComponents.size(); j++)
    {
        VssComponent& component = writer.m_vecComponents[j];
        // Ignore explicitly excluded components
        if (component.m_bIsExcluded)
        {
            continue;
        }

        wstring componentNameWithWriterName;
        if (writer.m_wstrName == wstring(VSS_SQLSERVER_WRITER_NAME_W))
        {
            componentNameWithWriterName = writer.m_wstrName + L":" + component.m_wstrName;
        }
        else if (writer.m_wstrName == wstring(VSS_EXCHANGE_WRITER_NAME_W))
        {
            componentNameWithWriterName = writer.m_wstrName + L":" + component.m_wstrCaption;
        }
        else
        {
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR,
                L"ERROR: Not support this writer %s! Aborting backup ...", writer.m_wstrName.c_str());

            throw(E_INVALIDARG);
        }

        // Check to see if this component is (implicitly or explicitly) included
        if (IsEqual(componentNameWithWriterName, wstrIncludedComponent))
        {
            COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Found component '%s' from writer '%s'",
                component.m_wstrFullPath.c_str(), writer.m_wstrName.c_str());

            // If not explicitly included, check to see if there is an explicitly included ancestor
            if (VerifyOneComponent(component, writer, wstrIncludedComponent))
            {
                return true;
            }
            else
            {
                COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR,
                    L"ERROR: The component \"%s\" was not included in the backup! Aborting backup ...",
                    wstrIncludedComponent.c_str());

                throw(E_INVALIDARG);
            }
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"The component \"%s\" was not found in the writer %s.",
        wstrIncludedComponent.c_str(), writer.m_wstrName.c_str());

    return false;
}

void VSSRequester::VerifyIncludedComponent(wstring& wstrIncludedComponent, vector<VssWriter>& vecWriterList)
{
    bool bIsValid = false;

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Verifing component \"%s\" ...", wstrIncludedComponent.c_str());

    // Enumerate all writers
    for (unsigned iWriter = 0; iWriter < vecWriterList.size(); iWriter++)
    {
        VssWriter& writer = vecWriterList[iWriter];
        // Ignore explicitly excluded writers
        if (writer.m_bIsExcluded)
        {
            continue;
        }

        bIsValid = VerifyCompsInOneWirter(wstrIncludedComponent, writer);
        if (bIsValid)
        {
            return;
        }
    }

    COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR,
        L"ERROR: The component \"%s\" was not found in the writer components list! Aborting backup ...",
        wstrIncludedComponent.c_str());

    throw ReqException(ERROR_VSS_EXCHANGE_DB_NOT_EXIST);
}

void VSSRequester::CheckIncludedList(vector<wstring>& vecIncludedList)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin check included writer and component list.");
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Verifying explicitly specified components ...");

    unsigned int i = 0;
    for (i = 0; i < vecIncludedList.size(); i++)
    {
        VerifyIncludedComponent(vecIncludedList[i], m_vecWriters);
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"End check included writer and component list.");
}

void VSSRequester::GetIncludeComponentFullPaths(vector<VssWriter>& vecWriterList,
        vector<wstring>& vecIncludedComponentList, vector<wstring>& vecComponentFullPaths)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin get include component full paths.");

    for (unsigned iWriter = 0; iWriter < vecWriterList.size(); iWriter++)
    {
        VssWriter& writer = vecWriterList[iWriter];

        for (unsigned iComponent = 0; iComponent < writer.m_vecComponents.size(); iComponent++)
        {
            VssComponent& component = writer.m_vecComponents[iComponent];
            wstring componentNameWithWriterName;
            if (writer.m_wstrName == wstring(VSS_SQLSERVER_WRITER_NAME_W))
            {
                componentNameWithWriterName = writer.m_wstrName + L":" + component.m_wstrName;
            }
            else if (writer.m_wstrName == wstring(VSS_EXCHANGE_WRITER_NAME_W))
            {
                componentNameWithWriterName = writer.m_wstrName + L":" + component.m_wstrCaption;
            }
            else
            {
                COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR,
                    L"ERROR: Not support this writer %s! Aborting backup ...", writer.m_wstrName.c_str());

                throw(E_INVALIDARG);
            }

            if (FindStringInList(componentNameWithWriterName, vecIncludedComponentList))
            {
                vecComponentFullPaths.push_back(component.m_wstrFullPath);
                COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Get full path %s for component(name %s, caption %s).",
                    component.m_wstrFullPath.c_str(), component.m_wstrName.c_str(), component.m_wstrCaption.c_str());

                continue;
            }
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get include component full paths succ.");
}

bool VSSRequester::IsDescendent(wstring& wstrDescendentFullPath, vector<wstring> vecAcestorFullPaths)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin check is descendent, descendent full path %s.",
        wstrDescendentFullPath.c_str());

    for (unsigned i = 0; i < vecAcestorFullPaths.size(); i++)
    {
       if (IsAncestor(vecAcestorFullPaths[i], wstrDescendentFullPath))
       {
           COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"%s is descendent of %s.", wstrDescendentFullPath.c_str(),
                vecAcestorFullPaths[i].c_str());

           return true;
       }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"End discover directly included components.");

    return false;
}

bool VSSRequester::IsAncestor(wstring& wstrAcestorFullPath, wstring& wstrDescendentFullPath)
{
    // The child must have a longer full path
    if (wstrDescendentFullPath.length() <= wstrAcestorFullPath.length())
    {
        return false;
    }

    wstring fullPath = AppendBackslash(wstrAcestorFullPath);
    wstring descendentPath = AppendBackslash(wstrDescendentFullPath);

    // Return TRUE if the current full path is a prefix of the child full path
    return IsEqual(fullPath, descendentPath.substr(0, fullPath.length()));
}

void VSSRequester::DiscoverDirectlyIncludedComponents(vector<wstring>& vecIncludedComponentList,
    vector<VssWriter>& vecWriterList)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin discover directly included components.");
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Discover directly included components ...");

    vector<wstring> vecComponentFullPaths;
    GetIncludeComponentFullPaths(vecWriterList, vecIncludedComponentList, vecComponentFullPaths);

    for (unsigned iWriter = 0; iWriter < vecWriterList.size(); iWriter++)
    {
        VssWriter& writer = vecWriterList[iWriter];

        // Check if the component is excluded
        for (unsigned iComponent = 0; iComponent < writer.m_vecComponents.size(); iComponent++)
        {
            VssComponent& component = writer.m_vecComponents[iComponent];
            //wstring componentNameWithWriterName = writer.m_wstrName + L":" + component.m_wstrName;
            wstring componentNameWithWriterName;
            if (writer.m_wstrName == wstring(VSS_SQLSERVER_WRITER_NAME_W))
            {
                componentNameWithWriterName = writer.m_wstrName + L":" + component.m_wstrName;
            }
            else if (writer.m_wstrName == wstring(VSS_EXCHANGE_WRITER_NAME_W))
            {
                componentNameWithWriterName = writer.m_wstrName + L":" + component.m_wstrCaption;
            }
            else
            {
                COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR,
                    L"ERROR: Not support this writer %s! Aborting backup ...", writer.m_wstrName.c_str());

                throw(E_INVALIDARG);
            }

            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get component name witch writer name %s.",
                componentNameWithWriterName.c_str());
            // Check to see if this component is explicitly excluded
            if (!FindStringInList(componentNameWithWriterName, vecIncludedComponentList))
            {
                if (!IsDescendent(component.m_wstrFullPath, vecComponentFullPaths))
                {
                    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Component '%s' from writer '%s' is explicitly excluded from backup ",
                        component.m_wstrFullPath.c_str(), writer.m_wstrName.c_str());

                    component.m_bIsExcluded = true;
                }

                continue;
            }
        }

        // Now, discover if we have any selected components. If none, exclude the whole writer
        bool nonExcludedComponents = false;
        for (unsigned i = 0; i < writer.m_vecComponents.size(); i++)
        {
            VssComponent& component = writer.m_vecComponents[i];

            if (!component.m_bIsExcluded)
            {
                COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component(name %s, caption %s) is not excluded.",
                    component.m_wstrName.c_str(), component.m_wstrCaption.c_str());

                nonExcludedComponents = true;
            }
        }

        // If all components are missing or excluded, then exclude the writer too
        if (!nonExcludedComponents)
        {
            COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Excluding writer '%s' since it has no selected components.",
                writer.m_wstrName.c_str());

            writer.m_bIsExcluded = true;
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Discover directly included components succ.");
}


// Select the maximum number of components such that their
// file descriptors are pointing only to volumes to be shadow copied
void VSSRequester::SelectComponentsForBackup(vector<wstring>& shadowSourceVolumes,
    vector<wstring>& includedComponentList)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin select components for backup.");

    // Just include the specified components
    //DiscoverDirectlyIncludedComponents(includedComponentList, m_vecWriters);

    // Discover excluded components that have file groups outside the shadow set
    DiscoverNonShadowedExcludedComponents(shadowSourceVolumes);

    // Now, exclude all componenets that are have directly excluded descendents
    DiscoverAllExcludedComponents();

    // Next, exclude all writers that:
    // - either have a top-level nonselectable excluded component
    // - or do not have any included components (all its components are excluded)
    DiscoverExcludedWriters();

    // Now, discover the components that should be included (explicitly or implicitly)
    // These are the top components that do not have any excluded children
    DiscoverExplicitelyIncludedComponents();

    // Check if the specified components were included
    if (includedComponentList.size() > 0)
    {
        CheckIncludedList(includedComponentList);
    }

    // Finally, select the explicitly included components
    SelectExplicitelyIncludedComponents();

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Select components for backup succ.");
}

// Add volumes to the shadow set
void VSSRequester::AddToSnapshotSet(vector<wstring> volumeList)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin add to snapshot set.");

    // *** BEGIN *** DTS2014103005542 y00275736 20014-11-21
    //This provider id must as the same as g_gProviderId defined in rdvss.cpp.
    // {ab23308d-639e-41ed-9b76-691b52409acb}
    GUID rdProviderId = { 0xab23308d, 0x639e, 0x41ed, { 0x9b, 0x76, 0x69, 0x1b, 0x52, 0x40, 0x9a, 0xcb } };

    // Add volumes to the shadow set
    for (unsigned i = 0; i < volumeList.size(); i++)
    {
        wstring volume = volumeList[i];
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Add volume %s [%s] to the shadow set.", volume.c_str(),
            GetDisplayNameForVolume(volume).c_str());

        VSS_ID SnapshotID;
        CHECK_COM(m_pVssObject->AddToSnapshotSet((LPWSTR)volume.c_str(), rdProviderId, &SnapshotID));
    }
    // *** END *** DTS2014103005542 y00275736 20014-11-21

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Add to snapshot set succ.");
}

// Prepare the shadow for backup
void VSSRequester::PrepareForBackup()
{
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_DEBUG, L"Begin prepare for backup.");

    CComPtr<IVssAsync> pAsync;
    CHECK_COM(m_pVssObject->PrepareForBackup(&pAsync));

    // Waits for the async operation to finish and checks the result
    WaitForAsync(pAsync);

    // Check selected writer status
    CheckSelectedWriterStatus();

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_DEBUG, L"Prepare for backup succ.");
}

// Effectively creating the shadow (calling DoSnapshotSet)
void VSSRequester::DoSnapshotSet()
{
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_DEBUG, L"Begin create the shadow (Do SnapShot Set).");

    CHECK_COM(m_pVssObject->DoSnapshotSet(&m_pAsyncSnapshot));

    unsigned i = 0;
    HRESULT hrRet = S_OK;
    HRESULT hrResult = S_OK;
    DWORD iWaitStatus;
    for (i = 0; i < VSS_TIMEOUT_FREEZE_MSEC/VSS_TIMEOUT_EVENT_MSEC; i++)
    {
        hrRet = m_pAsyncSnapshot->QueryStatus(&hrResult, NULL);
        if (FAILED(hrRet))
        {
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Failed to do snapshot set HRESULT = 0x%08lx.", hrRet);
            throw(hrRet);
        }

        if (hrResult != VSS_S_ASYNC_PENDING)
        {
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Do snapshot set exited without frozen event.");
            throw(E_FAIL);
        }

        iWaitStatus = WaitForSingleObject(m_hEventFrozen, VSS_TIMEOUT_EVENT_MSEC);
        if (WAIT_TIMEOUT != iWaitStatus)
        {
            break;
        }
    }

    if (WAIT_OBJECT_0 != iWaitStatus)
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Couldn't receive frozen event from VSS provider.");
        // *** BEGIN *** DTS2014071801749 y00275736 20014-07-24
        m_pAsyncSnapshot->Cancel();
        throw ReqException(ERROR_VSS_FREEZE_TIMEOUT);
        // *** END *** DTS2014071801749 y00275736 20014-07-24
    }

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_DEBUG, L"Create the shadow (Do SnapShot Set) succ.");
}

void VSSRequester::CreateSnapShot(vector<wstring>& vecVolumes, vector<wstring>& vecIncComponents)
{
    VSS_ID latestSnapshotSetID;
    LONG ctx;
    HRESULT hr;
    
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin create snapshot.");

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Create vss backup componets.");
    CHECK_COM(CreateVssBackupComponents(&m_pVssObject));

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Init for backup.");
    CHECK_COM(m_pVssObject->InitializeForBackup());

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Set backup state.");
    CHECK_COM(m_pVssObject->SetBackupState(true, true, VSS_BT_FULL, false));

    //To prevent the final commit (which requires to write to snapshots),
    //ATTR_NO_AUTORECOVERY and ATTR_TRANSPORTABLE are specified here.
    ctx = VSS_CTX_BACKUP | VSS_VOLSNAP_ATTR_TRANSPORTABLE | VSS_VOLSNAP_ATTR_NO_AUTORECOVERY | VSS_VOLSNAP_ATTR_TXF_RECOVERY;
    hr = m_pVssObject->SetContext(ctx);
    if (hr == (HRESULT)VSS_E_UNSUPPORTED_CONTEXT)
    {
        COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"This version of windows doesn't support the specified volume attrs.");
        //VSS_VOLSNAP_ATTR_TRANSPORTABLE -- Windows Server 2003, Standard Edition, Windows Server 2003, Web Edition, 
        //and Windows XP:  This attribute is not supported. All editions of Windows Server 2003 with SP1 support this attribute.
        ctx &= ~VSS_VOLSNAP_ATTR_TRANSPORTABLE;
        //VSS_VOLSNAP_ATTR_NO_AUTORECOVERY -- Windows Server 2003 and Windows XP:  This value is not supported until Windows Vista.
        ctx &= ~VSS_VOLSNAP_ATTR_NO_AUTORECOVERY;
        //VSS_VOLSNAP_ATTR_TXF_RECOVERY -- Windows Vista, Windows Server 2003, and Windows XP:  This value is not 
        //supported until Windows Server 2008.
        ctx &= ~VSS_VOLSNAP_ATTR_TXF_RECOVERY;
        hr = m_pVssObject->SetContext(ctx);
    }
    CHECK_COM_ERROR(hr, "SetContext");

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Gather writer metadata.");
    if (vss_freeze_filesys != m_eFreezeType)
    {
        GatherWriterMetadata(&m_wstrWriterName);
    }
    else
    {
        GatherWriterMetadata(NULL);
    }

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Select components for backup.");
    SelectComponentsForBackup(vecVolumes, vecIncComponents);

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Start snap shot set.");
    CHECK_COM(m_pVssObject->StartSnapshotSet(&latestSnapshotSetID));

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Add to snapshot set.");
    AddToSnapshotSet(vecVolumes);

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Prepare for backup.");
    PrepareForBackup();

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Do snapshot set.");
    DoSnapshotSet();

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Create snapshot succ.");
}

// Gather writers status
void VSSRequester::GatherWriterStatus()
{
    // Gathers writer status
    // WARNING: GatherWriterMetadata must be called before
    CComPtr<IVssAsync>  pAsync;
    CHECK_COM(m_pVssObject->GatherWriterStatus(&pAsync));

    // Waits for the async operation to finish and checks the result
    WaitForAsync(pAsync);
}

bool VSSRequester::IsWriterSelected(GUID guidInstanceId)
{
    // If this writer was not selected for backup, ignore it
    wstring instanceId = Guid2WString(guidInstanceId);
    for (unsigned i = 0; i < m_vecWriters.size(); i++)
    {
        if ((instanceId == m_vecWriters[i].m_wstrInstanceId) && !m_vecWriters[i].m_bIsExcluded)
        {
            return true;
        }
    }

    return false;
}

wstring VSSRequester::GetStringFromWriterStatus(VSS_WRITER_STATE eWriterStatus)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Interpreting constant %d", (int)eWriterStatus);

    switch (eWriterStatus)
    {
        CHECK_CASE_FOR_CONSTANT(VSS_WS_STABLE);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_WAITING_FOR_FREEZE);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_WAITING_FOR_THAW);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_WAITING_FOR_POST_SNAPSHOT);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_WAITING_FOR_BACKUP_COMPLETE);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_IDENTIFY);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_PREPARE_BACKUP);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_PREPARE_SNAPSHOT);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_FREEZE);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_THAW);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_POST_SNAPSHOT);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_BACKUP_COMPLETE);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_PRE_RESTORE);
        CHECK_CASE_FOR_CONSTANT(VSS_WS_FAILED_AT_POST_RESTORE);

        default:
            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Unknown constant: %d", eWriterStatus);

            return wstring(L"Undefined");
    }
}

// Check the status for all selected writers
void VSSRequester::CheckSelectedWriterStatus()
{
    // Gather writer status to detect potential errors
    GatherWriterStatus();

    // Gets the number of writers in the gathered status info
    // (WARNING: GatherWriterStatus must be called before)
    unsigned cWriters = 0;
    CHECK_COM(m_pVssObject->GetWriterStatusCount(&cWriters));

    // Enumerate each writer
    for(unsigned iWriter = 0; iWriter < cWriters; iWriter++)
    {
        VSS_ID idInstance = GUID_NULL;
        VSS_ID idWriter= GUID_NULL;
        VSS_WRITER_STATE eWriterStatus = VSS_WS_UNKNOWN;
        CComBSTR bstrWriterName;
        HRESULT hrWriterFailure = S_OK;

        // Get writer status
        CHECK_COM(m_pVssObject->GetWriterStatus(iWriter, &idInstance, &idWriter, &bstrWriterName, &eWriterStatus,
            &hrWriterFailure));

        // If the writer is not selected, just continue
        if (!IsWriterSelected(idInstance))
            continue;

        // If the writer is in non-stable state, break
        mp_bool bNonStableState = ( VSS_WS_FAILED_AT_IDENTIFY == eWriterStatus) || 
            ( VSS_WS_FAILED_AT_PREPARE_BACKUP == eWriterStatus) || 
            ( VSS_WS_FAILED_AT_PREPARE_SNAPSHOT == eWriterStatus) || 
            ( VSS_WS_FAILED_AT_FREEZE == eWriterStatus) ||
            ( VSS_WS_FAILED_AT_THAW == eWriterStatus) || 
            ( VSS_WS_FAILED_AT_POST_SNAPSHOT == eWriterStatus) || 
            ( VSS_WS_FAILED_AT_BACKUP_COMPLETE == eWriterStatus) || 
            ( VSS_WS_FAILED_AT_PRE_RESTORE == eWriterStatus) || 
            ( VSS_WS_FAILED_AT_POST_RESTORE == eWriterStatus);
        if ( !bNonStableState ) 
            continue;

        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"ERROR: Selected writer '%s' is in failed state!\n"
            L"   - Status: %d (%s)\n"
            L"   - Writer Failure code: 0x%08lx (%s)\n"
            L"   - Writer ID: " WSTR_GUID_FMT L"\n"
            L"   - Instance ID: " WSTR_GUID_FMT L"\n",
            (PWCHAR)bstrWriterName,
            eWriterStatus, GetStringFromWriterStatus(eWriterStatus).c_str(),
            hrWriterFailure, HResult2String(hrWriterFailure).c_str(),
            GUID_PRINTF_ARG(idWriter),
            GUID_PRINTF_ARG(idInstance)
            );

        // Stop here
        throw(E_UNEXPECTED);
    }
}


// Marks all selected components as succeeded for backup
void VSSRequester::SetBackupSucceeded(bool succeeded)
{
    // Enumerate writers
    for (unsigned iWriter = 0; iWriter < m_vecWriters.size(); iWriter++)
    {
        VssWriter& writer = m_vecWriters[iWriter];

        // Enumerate components
        for(unsigned iComponent = 0; iComponent < writer.m_vecComponents.size(); iComponent++)
        {
            VssComponent& component = writer.m_vecComponents[iComponent];

            // Test that the component is explicitely selected and requires notification
            if (!component.m_bIsExplicitlyIncluded || !component.m_bNotifyOnBackupComplete)
            {
                continue;
            }

            // Call SetBackupSucceeded for this component
            CHECK_COM(m_pVssObject->SetBackupSucceeded(WString2Guid(writer.m_wstrInstanceId), WString2Guid(writer.m_wstrId),
                component.m_enType, component.m_wstrLogicalPath.c_str(), component.m_wstrName.c_str(), succeeded));
        }
    }
}

// Ending the backup (calling BackupComplete)
void VSSRequester::BackupComplete(bool succeeded)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin backup complete.");

    if (succeeded)
    {
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"- Mark all writers as succesfully backed up.");
    }
    else
    {
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"- Backup failed. Mark all writers as not succesfully backed up.");
    }

    SetBackupSucceeded(succeeded);

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Completing the backup (BackupComplete).");

    CComPtr<IVssAsync> pAsync;
    CHECK_COM(m_pVssObject->BackupComplete(&pAsync));

    // Waits for the async operation to finish and checks the result
    WaitForAsync(pAsync);

    // Check selected writer status
    CheckSelectedWriterStatus();

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Backup complete succ.");
}

void VSSRequester::FreezeFailedProcess()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin freeze failed process");

    if (m_pVssObject)
    {
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin abort vss backup");
        m_pVssObject->AbortBackup();
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"End abort vss backup");
    }
    CleanUp();
    CoUninitialize();

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"End freeze failed process");
}

void VSSRequester::WaitThawReturn(bool bAutoEndBackup)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin wait thaw return.");
    HRESULT hr = S_OK;

    hr = WaitForAsyncEx(m_pAsyncSnapshot);
    switch (hr)
    {
        case VSS_S_ASYNC_FINISHED:
            COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"VSS async finished.");
            if (bAutoEndBackup)
            {
                BackupComplete(true);
            }
            break;
        //Cprovider::GetSnapshotProperties方法返回VSS_E_OBJECT_NOT_FOUND时在win2003下会走到VSS_E_UNEXPECTED_PROVIDER_ERROR分支
        //case (HRESULT)VSS_E_OBJECT_NOT_FOUND:
            //On Windows earlier than 2008 SP2 which does not support
            //VSS_VOLSNAP_ATTR_NO_AUTORECOVERY context, the final commit is not
            //skipped and VSS is aborted by VSS_E_OBJECT_NOT_FOUND. However, as
            //the system had been frozen until fsfreeze-thaw command was issued,
            //we ignore this error.
            //COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Ignore the object not found error.");
            //WriteLog("Ignore the object not found error.");
            //m_pVssObject->AbortBackup();
            //break;
        case VSS_E_UNEXPECTED_PROVIDER_ERROR:
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Unexpected provider error.");
            if (WaitForSingleObject(m_hEventTimeOut, 0) != WAIT_OBJECT_0)
            {
                COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Unexpected error in VSS provider.");
                throw(hr);
                break;
            }

            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Couldn't hold writes, timeout.");
            //throw(hr);
            throw ReqException(ERROR_VSS_TIME_OUT);
            break;
        case (HRESULT)VSS_E_HOLD_WRITES_TIMEOUT:
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Couldn't hold writes: freeze is limited up to 10 secondes.");
            //throw(hr);
            throw ReqException(ERROR_VSS_TIME_OUT);
            break;
        default:
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Failed to do snapshot set.");
            throw(hr);
            break;
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Wait thaw return succ.");
}

void VSSRequester::CleanUp()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin clean up.");

    if (m_hEventFrozen)
    {
        CloseHandle(m_hEventFrozen);
        m_hEventFrozen = NULL;
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Close frozen event.");
    }

    if (m_hEventThaw)
    {
        CloseHandle(m_hEventThaw);
        m_hEventThaw = NULL;
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Close thaw event.");
    }

    if (m_hEventTimeOut)
    {
        CloseHandle(m_hEventTimeOut);
        m_hEventTimeOut = NULL;
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Close timeout event.");
    }

    if (m_pAsyncSnapshot)
    {
        m_pAsyncSnapshot = NULL;
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Release async snapshot.");
    }

    if (m_pVssObject)
    {
        m_pVssObject = NULL;
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Release vss backup components.");
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Clean up succ.");
}

void VSSRequester::CleanUpEx()
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin clean up all.");
    CleanUp();
    if (m_hEventEndBackup)
    {
        CloseHandle(m_hEventEndBackup);
        m_hEventEndBackup = NULL;
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Close endbackup event.");
    }
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Clean up all succ.");
}

void VSSRequester::WaitForAsync(IVssAsync* pAsync)
{
    HRESULT hrReturned = S_OK;

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin wait for async.");

    CHECK_COM(pAsync->Wait());

    CHECK_COM(pAsync->QueryStatus(&hrReturned, NULL));

    if (FAILED(hrReturned))
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Error during the last asynchronous operation.");
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"- Returned HRESULT = 0x%08lx.", hrReturned);
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"- Error text: %s.", HResult2String(hrReturned).c_str());

        throw(hrReturned);
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Wait for async succ.");
}

HRESULT VSSRequester::WaitForAsyncEx(IVssAsync* pAsync)
{
    HRESULT ret = S_OK;
    HRESULT hr = S_OK;

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Begin wait for async ex.");

    do
    {
        hr = pAsync->Wait();
        if (FAILED(hr))
        {
            ret = hr;

            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Wait for async ex failed, Returned HRESULT = 0x%08lx.",
                ret);

            break;
        }
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Wait for async operation.");

        hr = pAsync->QueryStatus(&ret, NULL);
        if (FAILED(hr))
        {
            ret = hr;

            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Query status failed, Returned HRESULT = 0x%08lx.", ret);

            break;
        }
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Query status, 0x%08lx.", ret);
    }
    while (ret == VSS_S_ASYNC_PENDING);

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"End wait for async ex, return 0x%08lx (%s).", ret,
        HResult2String(ret).c_str());

    return ret;
}

#endif //WIN32

