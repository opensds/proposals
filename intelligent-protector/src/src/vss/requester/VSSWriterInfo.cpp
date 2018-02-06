/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "vss/requester/VSSWriterInfo.h"
#include <algorithm>
#ifdef WIN32

wstring VssFileDescriptor::GetVolumeName(wstring wstrPath)
{
    wstring wstrAffectedVolume;

    try
    {
        wstrAffectedVolume = GetUniqueVolumeNameForPath(wstrPath);
    }
    catch (HRESULT hr)
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Get volume name for %s failed, HRESULT Error caught: 0x%08lx",
            wstrPath.c_str(), hr);

        throw ReqException(ERROR_VSS_INIT_FILEDES_GETVOLUME_FAILED);
    }

    return wstrAffectedVolume;
}

mp_bool VssFileDescriptor::Initialize(IVssWMFiledesc* pFileDesc, VSS_DESCRIPTOR_TYPE typeParam)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin init vss file descriptor.");

    // Set the type
    m_enType = typeParam;

    CComBSTR bstrPath;
    CHECK_COM(pFileDesc->GetPath(&bstrPath));

    CComBSTR bstrFilespec;
    CHECK_COM(pFileDesc->GetFilespec(&bstrFilespec));

    bool bRecursive = false;
    CHECK_COM(pFileDesc->GetRecursive(&bRecursive));

    CComBSTR bstrAlternate;
    CHECK_COM(pFileDesc->GetAlternateLocation(&bstrAlternate));

    // Initialize local data members
    m_wstrPath = BSTR2WString(bstrPath);
    m_wstrFilespec = BSTR2WString(bstrFilespec);
    m_bIsRecursive = bRecursive;

    // Get the expanded path
    m_wstrExpandedPath.resize(MAX_PATH, L'\0');
    CHECK_WIN32(ExpandEnvironmentStringsW(bstrPath, (PWCHAR)m_wstrExpandedPath.c_str(), (DWORD)m_wstrExpandedPath.length()));
    m_wstrExpandedPath = AppendBackslash(m_wstrExpandedPath);

    mp_string strPath = CMpString::WString2String(m_wstrExpandedPath);
    if (strPath.find("\\EFI\\") != mp_string::npos)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "- Path name: %s, this is an EFI partition...", strPath.c_str());
        return false;
    }

    // Get the affected volume
    //m_wstrAffectedVolume = GetUniqueVolumeNameForPath(m_wstrExpandedPath);
    m_wstrAffectedVolume = GetVolumeName(m_wstrExpandedPath);

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get file descriptor info, path %s, file spec %s, recursive %d, expanded path %s, "
        L"affected volume %s.", m_wstrPath.c_str(), m_wstrFilespec.c_str(), m_bIsRecursive, m_wstrExpandedPath.c_str(),
        m_wstrAffectedVolume.c_str());

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Init vss file descriptor succ.");
    return true; 
}

bool VssComponent::NeedInit(PVSSCOMPONENTINFO pInfo, wstring writerName, vector<wstring>& vecDBNames)
{
    wstring wstrSQLServerWriter = VSS_SQLSERVER_WRITER_NAME_W;
    wstring wstrExchangeWriter = VSS_EXCHANGE_WRITER_NAME_W;
    wstring wstrTmpName;
    wstring wstrName;

    m_wstrName = BSTR2WString(pInfo->bstrComponentName);
    m_wstrCaption = BSTR2WString(pInfo->bstrCaption);

    if (writerName == wstrSQLServerWriter)
    {
        wstrTmpName = BSTR2WString(pInfo->bstrComponentName);
    }
    else if (writerName == wstrExchangeWriter)
    {
        wstrTmpName = BSTR2WString(pInfo->bstrCaption);
    }
    else
    {
        return false;
    }

    if (find(vecDBNames.begin(), vecDBNames.end(), wstrTmpName) == vecDBNames.end())
    {

        return false;
    }

    return true;
}

void VssComponent::Initialize(wstring writerNameParam, IVssWMComponent* pComponent)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin init vss component.");

    m_wstrWriterName = writerNameParam;
    mp_bool bRet = false;
    
    // Get the component info
    PVSSCOMPONENTINFO pInfo = NULL;
    CHECK_COM(pComponent->GetComponentInfo(&pInfo));

    // Initialize local members
    m_wstrName = BSTR2WString(pInfo->bstrComponentName);
    m_wstrLogicalPath = BSTR2WString(pInfo->bstrLogicalPath);
    m_wstrCaption = BSTR2WString(pInfo->bstrCaption);
    m_enType = pInfo->type;
    m_bIsSelectable = pInfo->bSelectable;
    m_bNotifyOnBackupComplete = pInfo->bNotifyOnBackupComplete;

    // Compute the full path
    m_wstrFullPath = AppendBackslash(m_wstrLogicalPath) + m_wstrName;
    if (m_wstrFullPath[0] != L'\\')
    {
        m_wstrFullPath = wstring(L"\\") + m_wstrFullPath;
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component full path %s, name %s, logical path %s, caption %s, type %d, "
        L"is selectable %s.", m_wstrFullPath.c_str(), m_wstrName.c_str(), m_wstrLogicalPath.c_str(), m_wstrCaption.c_str(),
        m_enType, m_bIsSelectable?L"true":L"false");

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get file list descriptors.");
    // Get file list descriptors
    for(unsigned i = 0; i < pInfo->cFileCount; i++)
    {
        CComPtr<IVssWMFiledesc> pFileDesc;
        CHECK_COM(pComponent->GetFile(i, &pFileDesc));

        VssFileDescriptor desc;
        bRet = desc.Initialize(pFileDesc, VSS_FDT_FILELIST);
        if (!bRet)
        {
            continue;
        }
        m_vecDescriptors.push_back(desc);
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get database descriptors.");
    // Get database descriptors
    for(unsigned i = 0; i < pInfo->cDatabases; i++)
    {
        CComPtr<IVssWMFiledesc> pFileDesc;
        CHECK_COM(pComponent->GetDatabaseFile (i, &pFileDesc));

        VssFileDescriptor desc;
        bRet = desc.Initialize(pFileDesc, VSS_FDT_DATABASE);
        if (!bRet)
        {
            continue;
        }
        m_vecDescriptors.push_back(desc);
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get log descriptors.");
    // Get log descriptors
    for(unsigned i = 0; i < pInfo->cLogFiles; i++)
    {
        CComPtr<IVssWMFiledesc> pFileDesc;
        CHECK_COM(pComponent->GetDatabaseLogFile (i, &pFileDesc));

        VssFileDescriptor desc;
        bRet = desc.Initialize(pFileDesc, VSS_FDT_DATABASE_LOG);
        if (!bRet)
        {
            continue;
        }
        m_vecDescriptors.push_back(desc);
    }

    pComponent->FreeComponentInfo(pInfo);

    // Compute the affected paths and volumes
    for(unsigned i = 0; i < m_vecDescriptors.size(); i++)
    {
        if (!FindStringInList(m_vecDescriptors[i].m_wstrExpandedPath, m_vecAffectedPaths))
        {
            m_vecAffectedPaths.push_back(m_vecDescriptors[i].m_wstrExpandedPath);
        }

        if (!FindStringInList(m_vecDescriptors[i].m_wstrAffectedVolume, m_vecAffectedVolumes))
        {
            m_vecAffectedVolumes.push_back(m_vecDescriptors[i].m_wstrAffectedVolume);
        }
    }

    sort(m_vecAffectedPaths.begin(), m_vecAffectedPaths.end());

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Init vss component succ.");
}

// Return TRUE if the current component is parent of the given component
bool VssComponent::IsAncestorOf(VssComponent& descendent)
{
    // The child must have a longer full path
    if (descendent.m_wstrFullPath.length() <= m_wstrFullPath.length())
    {
        return false;
    }

    wstring fullPath = AppendBackslash(m_wstrFullPath);
    wstring descendentPath = AppendBackslash(descendent.m_wstrFullPath);

    // Return TRUE if the current full path is a prefix of the child full path
    return IsEqual(fullPath, descendentPath.substr(0, fullPath.length()));
}

// Return TRUE if the current component is parent of the given component
bool VssComponent::CanBeExplicitlyIncluded()
{
    if (m_bIsExcluded)
    {
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component is exclued, it's can't be explicitly included.");

        return false;
    }

    // selectable can be explictly included
    if (m_bIsSelectable)
    {
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component is selectable, it's can be explicitly included.");

        return true;
    }

    // Non-selectable top level can be explictly included
    if (m_bIsTopLevel)
    {
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"component is top level, it's can be explicitly included.");

        return true;
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Component can't be explicitly included.");

    return false;
}

bool VssWriter::Initialize(IVssExamineWriterMetadata* pMetadata, wstring* pwstrWriterName)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin init vss writer info.");

    // Get writer identity information
    VSS_ID idInstance = GUID_NULL;
    VSS_ID idWriter = GUID_NULL;
    CComBSTR bstrWriterName;
    VSS_USAGE_TYPE usage = VSS_UT_UNDEFINED;
    VSS_SOURCE_TYPE source = VSS_ST_UNDEFINED;
    CComBSTR bstrService;
    CComBSTR bstrUserProcedure;
    UINT iMappings;

    // Get writer identity
    CHECK_COM(pMetadata->GetIdentity(&idInstance, &idWriter, &bstrWriterName, &usage, &source));

    // Initialize local members
    m_wstrName = (LPWSTR)bstrWriterName;
    m_wstrId = Guid2WString(idWriter);
    m_wstrInstanceId = Guid2WString(idInstance);
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Get writer identity, name %s, id %s, instance id %s.", m_wstrName.c_str(),
        m_wstrId.c_str(), m_wstrInstanceId.c_str());


    if ((NULL != pwstrWriterName)&&(m_wstrName != *pwstrWriterName))
    {
        COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"No need init detail info for writer %s.", m_wstrName.c_str());

        return false;
    }

    // Get file counts
    unsigned cIncludeFiles = 0;
    unsigned cExcludeFiles = 0;
    unsigned cComponents = 0;
    CHECK_COM(pMetadata->GetFileCounts(&cIncludeFiles, &cExcludeFiles, &cComponents));

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Enumerate components in writer.");
    // Enumerate components
    for(unsigned iComponent = 0; iComponent < cComponents; iComponent++)
    {
        // Get component
        CComPtr<IVssWMComponent> pComponent;
        CHECK_COM(pMetadata->GetComponent(iComponent, &pComponent));

        // Add this component to the list of components
        VssComponent component;
        component.Initialize(m_wstrName, pComponent);
        m_vecComponents.push_back(component);
    }

    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Discover top level components in writer.");
    // Discover toplevel components
    for(unsigned i = 0; i < m_vecComponents.size(); i++)
    {
        m_vecComponents[i].m_bIsTopLevel = true;
        for(unsigned j = 0; j < m_vecComponents.size(); j++)
        {
            if (m_vecComponents[j].IsAncestorOf(m_vecComponents[i]))
            {
                m_vecComponents[i].m_bIsTopLevel = false;
            }
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Init vss writer info succ.");

    return true;
}

#endif

