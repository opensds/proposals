/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

// provider.cpp : Cprovider 的实现
#ifdef WIN32

#include <time.h>
#include <atlstr.h>
#include "vss/provider/stdafx.h"
#include "vss/provider/provider.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"

extern WCHAR *g_wszProviderName;

void LockModule(BOOL lock)
{
    if (lock)
    {
        InterlockedIncrement(&g_nComObjsInUse);
    }
    else
    {
        InterlockedDecrement(&g_nComObjsInUse);
    }
}

RDVSSEnumObject::RDVSSEnumObject()
{
    m_nRefCount = 0;
    LockModule(TRUE);
}

RDVSSEnumObject::~RDVSSEnumObject()
{
    LockModule(FALSE);
}

STDMETHODIMP RDVSSEnumObject::QueryInterface(REFIID riid, void** ppObj)
{
    if (riid == IID_IUnknown || riid == IID_IVssEnumObject)
    {
        *ppObj = static_cast<void*>(static_cast<IVssEnumObject*>(this));
        AddRef();
        return S_OK;
    }

    *ppObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) RDVSSEnumObject::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

STDMETHODIMP_(ULONG) RDVSSEnumObject::Release()
{
    long nRefCount = InterlockedDecrement(&m_nRefCount);
    if (m_nRefCount == 0)
    {
        delete this;
    }

    return nRefCount;
}

STDMETHODIMP RDVSSEnumObject::Next(ULONG celt, VSS_OBJECT_PROP* rgelt, ULONG* pceltFetched)
{
    *pceltFetched = 0;
    return S_FALSE;
}

STDMETHODIMP RDVSSEnumObject::Skip(ULONG celt)
{
    return S_FALSE;
}

STDMETHODIMP RDVSSEnumObject::Reset(void)
{
    return S_OK;
}

STDMETHODIMP RDVSSEnumObject::Clone(IVssEnumObject** ppenum)
{
    return E_NOTIMPL;
}

Cprovider::Cprovider()
{
    (void)Init();
}

Cprovider::~Cprovider()
{
}

int Cprovider::InitConf(char* pszConfPath)
{
    return 0;
}

int Cprovider::InitLogger(char* pszLogPath)
{
    return 0;
}

bool Cprovider::GetServicePath(const wchar_t* wpszServiceName, string& strServicePath)
{
    DWORD dwBytesNeeded;
    CString cstrPathName;
    wstring wstrPathName;
    string strPathName;
    LPQUERY_SERVICE_CONFIG lpqscBuf;
    char* pszRes = NULL;
    size_t iIndex, iCount;
    char drive[_MAX_DRIVE] = {0};
    char dir[_MAX_DIR] = {0};
    char fname[_MAX_FNAME] = {0};
    char ext[_MAX_EXT] = {0};
    int iRet = 0;

    SC_HANDLE schSCManager = OpenSCManager(NULL,NULL, SC_MANAGER_ALL_ACCESS);
    if(schSCManager == NULL)
    {
        return false;
    }

    SC_HANDLE schService = OpenService(schSCManager, wpszServiceName, SERVICE_ALL_ACCESS);
    if(schService == NULL)
    {
        CloseServiceHandle(schSCManager);

        return false;
    }

    lpqscBuf = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4096);
    if (NULL == lpqscBuf)
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return false;
    }

    if(!QueryServiceConfig(schService, lpqscBuf, 4096, &dwBytesNeeded))
    {
        LocalFree(lpqscBuf);
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return false;
    }

    cstrPathName = lpqscBuf->lpBinaryPathName;
    //某些环境上路径前后有双引号
    cstrPathName.Replace('"', ' ');
    wstrPathName = cstrPathName.GetString();
    strPathName = CMpString::WString2String(wstrPathName);
    iIndex = strPathName.find(".exe");
    strPathName = strPathName.substr(0, iIndex + 4);
    _splitpath_s(strPathName.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
    iCount = strlen(drive) + strlen(dir) + 1;
    pszRes = (char*)malloc(iCount);
    if (NULL == pszRes)
    {
        LocalFree(lpqscBuf);
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return false;
    }

    iRet = memset_s(pszRes, iCount, 0, iCount);
    if (0 != iRet)
    {
        free(pszRes);
        LocalFree(lpqscBuf);
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return false;
    }

    iRet = strcpy_s(pszRes, iCount, drive);
    if (0 != iRet)
    {
        free(pszRes);
        LocalFree(lpqscBuf);
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return false;
    }

    iRet = strcat_s(pszRes, iCount, dir);
    if (0 != iRet)
    {
        free(pszRes);
        LocalFree(lpqscBuf);
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        return false;
    }

    strServicePath = pszRes;

    free(pszRes);
    LocalFree(lpqscBuf);
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return true;
}

void Cprovider::Init()
{
}

//
// IVssProviderCreateSnapshotSet methods
//

STDMETHODIMP Cprovider::EndPrepareSnapshots(IN VSS_ID SnapshotSetId)
{
    return S_OK;
}

STDMETHODIMP Cprovider::PreCommitSnapshots(IN VSS_ID SnapshotSetId)
{
    return S_OK;
}

STDMETHODIMP Cprovider::CommitSnapshots(IN VSS_ID SnapshotSetId)
{
    HRESULT hr = S_OK;
    HANDLE hEventFrozen, hEventThaw, hEventTimeout;

    hEventFrozen = OpenEventW(EVENT_ALL_ACCESS, FALSE, EVENT_NAME_FROZEN);
    if (!hEventFrozen)
    {
        return E_FAIL;
    }

    hEventThaw = OpenEventW(EVENT_ALL_ACCESS, FALSE, EVENT_NAME_THAW);
    if (!hEventThaw)
    {
        CloseHandle(hEventFrozen);
        return E_FAIL;
    }

    hEventTimeout = OpenEventW(EVENT_ALL_ACCESS, FALSE, EVENT_NAME_TIMEOUT);
    if (!hEventTimeout)
    {
        CloseHandle(hEventFrozen);
        CloseHandle(hEventThaw);
        return E_FAIL;
    }

    SetEvent(hEventFrozen);

    if (WaitForSingleObject(hEventThaw, VSS_TIMEOUT_MSEC) != WAIT_OBJECT_0)
    {
        SetEvent(hEventTimeout);
        hr = E_ABORT;
    }

    CloseHandle(hEventThaw);
    CloseHandle(hEventFrozen);
    CloseHandle(hEventTimeout);

    return hr;
}

STDMETHODIMP Cprovider::PostCommitSnapshots(IN VSS_ID SnapshotSetId, IN LONG)
{
    return S_OK;
}

//
// These two methods are stubs for Windows Server 2003, and should merely return S_OK
//
STDMETHODIMP Cprovider::PreFinalCommitSnapshots(IN VSS_ID)
{
    return S_OK;
}

STDMETHODIMP Cprovider::PostFinalCommitSnapshots(IN VSS_ID)
{
    return S_OK;
}

STDMETHODIMP Cprovider::AbortSnapshots(IN VSS_ID)
{
    return S_OK;
}

STDMETHODIMP Cprovider::BeginPrepareSnapshot(IN VSS_ID SnapshotSetId, IN VSS_ID SnapshotId,
    IN VSS_PWSZ pwszVolumeName, IN LONG lNewContext)
{
    return S_OK;
}

STDMETHODIMP Cprovider::DeleteSnapshots(IN VSS_ID SourceObjectId, IN VSS_OBJECT_TYPE eSourceObjectType,
    IN BOOL bForceDelete, OUT LONG *plDeletedSnapshots, OUT VSS_ID *pNondeletedSnapshotID)
{
    return S_OK;
}

STDMETHODIMP Cprovider::GetSnapshotProperties(IN VSS_ID SnapshotId, OUT VSS_SNAPSHOT_PROP *pProp)
{
    //return VSS_E_OBJECT_NOT_FOUND;
    return S_OK;
}

STDMETHODIMP Cprovider::IsVolumeSnapshotted(IN VSS_PWSZ pwszVolumeName, OUT BOOL *pbSnapshotsPresent,
    OUT LONG *plSnapshotCompatibility)
{
    return S_OK;
}

STDMETHODIMP Cprovider::IsVolumeSupported(IN VSS_PWSZ pwszVolumeName, OUT BOOL *pbSupportedByThisProvider)
{
    HRESULT hr = S_OK;
    *pbSupportedByThisProvider = TRUE;

    return hr;
}

STDMETHODIMP Cprovider::Query(IN VSS_ID QueriedObjectId, IN VSS_OBJECT_TYPE eQueriedObjectType,
    IN VSS_OBJECT_TYPE eReturnedObjectsType, OUT IVssEnumObject **ppEnum)
{
    HRESULT hr = S_OK;
    
    try
    {
        *ppEnum = new RDVSSEnumObject;
    }
    catch (...)
    {
        return E_OUTOFMEMORY;
    }

    (*ppEnum)->AddRef();

    return hr;
}

STDMETHODIMP Cprovider::SetContext(IN LONG lContext)
{
    return S_OK;
}

STDMETHODIMP Cprovider::QueryRevertStatus(IN VSS_PWSZ pwszVolume, OUT IVssAsync **ppAsync)
{
    return E_NOTIMPL;
}

STDMETHODIMP Cprovider::RevertToSnapshot(IN VSS_ID SnapshotId)
{
    return E_NOTIMPL;
}

STDMETHODIMP Cprovider::SetSnapshotProperty(IN VSS_ID SnapshotId, IN VSS_SNAPSHOT_PROPERTY_ID eSnapshotPropertyId,
    IN VARIANT vProperty)
{
    return E_NOTIMPL;
}

#endif

