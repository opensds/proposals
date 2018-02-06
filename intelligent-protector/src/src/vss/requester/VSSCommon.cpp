/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifdef WIN32
#include "vss/requester/VSSCommon.h"

bool IsEqual(wstring str1, wstring str2)
{
    return (_wcsicmp(str1.c_str(), str2.c_str()) == 0);
}

bool FindStringInList(wstring str, vector<wstring> stringList)
{
    for (unsigned i = 0; i < stringList.size(); i++)
    {
        if (IsEqual(str, stringList[i]))
        {
            return true;
        }
    }

    return false;
}

wstring AppendBackslash(wstring str)
{
    if (str.length() == 0)
    {
        return wstring(L"\\");
    }

    if (str[str.length() - 1] == L'\\')
    {
        return str;
    }

    return str.append(L"\\");
}

wstring BSTR2WString(BSTR bstr)
{
    return (bstr == NULL)? wstring(L""): wstring(bstr);
}

wstring Guid2WString(GUID guid)
{
    wstring guidString(100, L'\0');
    CHECK_COM(StringCchPrintfW(WString2Buffer(guidString), guidString.length(), WSTR_GUID_FMT, GUID_PRINTF_ARG(guid)));

    return guidString;
}

GUID& WString2Guid(wstring src)
{
    // Check if this is a GUID
    static GUID result;
    HRESULT hr = ::CLSIDFromString(W2OLE(const_cast<WCHAR*>(src.c_str())), &result);
    if (FAILED(hr))
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"The string '%s' is not formatted as a GUID.", src.c_str());

        throw(E_INVALIDARG);
    }

    return result;
}

wstring GetUniqueVolumeNameForPath(wstring path)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin get unique volume name for path %s.", path.c_str());
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Get volume path name for %s ...", path.c_str());

    // Add the backslash termination, if needed
    path = AppendBackslash(path);

    wstring volumeRootPath(MAX_PATH, L'\0');
    wstring volumeName(MAX_PATH, L'\0');
    wstring volumeUniqueName(MAX_PATH, L'\0');

    // Get the root path of the volume
    CHECK_WIN32(GetVolumePathNameW((LPCWSTR)path.c_str(), WString2Buffer(volumeRootPath), (DWORD)volumeRootPath.length()));
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Path name: %s.", volumeRootPath.c_str());


    // Get the volume name alias (might be different from the unique volume name in rare cases)
    CHECK_WIN32(GetVolumeNameForVolumeMountPointW((LPCWSTR)volumeRootPath.c_str(), WString2Buffer(volumeName),
        (DWORD)volumeName.length()));
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Volume name for path: %s.", volumeName.c_str());

    // Get the unique volume name
    CHECK_WIN32(GetVolumeNameForVolumeMountPointW((LPCWSTR)volumeName.c_str(), WString2Buffer(volumeUniqueName),
        (DWORD)volumeUniqueName.length()));
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"- Unique volume name: %s.", volumeUniqueName.c_str());

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get unique volume name for path succ.");

    return volumeUniqueName;
}

// Returns TRUE if this is a real volume (for eample C:\ or C:)
// - The backslash termination is optional
bool IsVolume(wstring volumePath)
{
    COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Checking is %s is a real volume path.", volumePath.c_str());

    // If the last character is not '\\', append one
    volumePath = AppendBackslash(volumePath);

    // Get the volume name
    wstring volumeName(MAX_PATH, L'\0');
    if (!GetVolumeNameForVolumeMountPointW(volumePath.c_str(), WString2Buffer(volumeName), (DWORD)volumeName.length()))
    {
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Get volume name for volume mount point(%s) failed with %d.",
            volumePath.c_str(), GetLastError());

        return false;
    }

    return true;
}

// Get the displayable root path for the given volume name
wstring GetDisplayNameForVolume(wstring volumeName)
{
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin get display name for volume %s.", volumeName.c_str());

    DWORD dwRequired = 0;
    wstring volumeMountPoints(MAX_PATH, L'\0');
    if (!GetVolumePathNamesForVolumeNameW((LPCWSTR)volumeName.c_str(), WString2Buffer(volumeMountPoints),
        (DWORD)volumeMountPoints.length(), &dwRequired))
    {
        // If not enough, retry with a larger size
        volumeMountPoints.resize(dwRequired, L'\0');
        CHECK_WIN32(!GetVolumePathNamesForVolumeNameW((LPCWSTR)volumeName.c_str(), WString2Buffer(volumeMountPoints),
            (DWORD)volumeMountPoints.length(), &dwRequired));
    }

    // compute the smallest mount point by enumerating the returned MULTI_SZ
    wstring mountPoint = volumeMountPoints;
    for(LPWSTR pwszString = (LPWSTR)volumeMountPoints.c_str(); pwszString[0]; pwszString += wcslen(pwszString) + 1)
    {
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get mount point %s.", pwszString);

        if (mountPoint.length() > wcslen(pwszString))
        {
            mountPoint = pwszString;
            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Set mount point %s.", mountPoint.c_str());
        }
    }

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get display name for volume succ.");

    return mountPoint;
}

int GetVolumePaths(PWCHAR pwVolumeName, vector<wstring>& vecVolumePaths)
{
    DWORD dwCharCount = MAX_PATH + 1;
    PWCHAR pwNames = NULL;
    PWCHAR pwNameIdx = NULL;
    BOOL bSuccess = FALSE;
    DWORD dwError = ERROR_SUCCESS;

    for (;;)
    {
        //  Allocate a buffer to hold the paths.
        try
        {
            pwNames = (PWCHAR)new BYTE[dwCharCount * sizeof(WCHAR)];
        }
        catch(...)
        {
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"new pwNames failed.");
            pwNames = NULL;
        }
        
        if (!pwNames) 
        {
            //  If memory can't be allocated, return.
            return -1;
        }

        //  Obtain all of the paths
        //  for this volume.
        bSuccess = GetVolumePathNamesForVolumeNameW(pwVolumeName, pwNames, dwCharCount, &dwCharCount);
        if (bSuccess)
        {
            break;
        }

        dwError = GetLastError();
        if (dwError != ERROR_MORE_DATA)
        {
            delete [] pwNames;
            pwNames = NULL;
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Get volume paths names for volume name %s failed with error code %d.", 
                pwVolumeName, dwError);
            return -1;
        }

        //  Try again with the
        //  new suggested size.
        delete [] pwNames;
        pwNames = NULL;
    }

    if (bSuccess)
    {
        //  Display the various paths.
        for (pwNameIdx = pwNames; pwNameIdx[0] != L'\0'; pwNameIdx += wcslen(pwNameIdx) + 1 ) 
        {
            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"  %s.", pwNameIdx);
            vecVolumePaths.push_back(pwNameIdx);
        }
    }

    if (pwNames != NULL)
    {
        delete [] pwNames;
        pwNames = NULL;
    }

    return 0;
}

int GetSystemDrivePath(wstring& strSysDrive)
{
    WCHAR acPath[MAX_PATH] = L"";
    DWORD dwError = ERROR_SUCCESS;
    WCHAR drive[_MAX_DRIVE] = {0};
    WCHAR dir[_MAX_DIR] = {0};
    WCHAR fname[_MAX_FNAME] = {0};
    WCHAR ext[_MAX_EXT] = {0};
    
    if (0 == GetSystemDirectoryW(acPath, ARRAYSIZE(acPath)))
    {
        dwError = GetLastError();
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Get system direcotry failed with error code %d.", dwError);
        return -1;
    }
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get system directory %s.", acPath);

    _wsplitpath_s(acPath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get system drive %s.", drive);
    strSysDrive = drive;
    
    return 0;
}

BOOL IsIncludeSysDrive(vector<wstring>& vecVolumePaths, wstring& strSysDrive)
{ 
    vector<wstring>::iterator iter;

    for (iter = vecVolumePaths.begin(); iter != vecVolumePaths.end(); iter++)
    {
        if (strSysDrive == *iter)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int GetAllVolumes(vector<wstring>& vecVolumesList)
{
    DWORD  dwCharCount = 0;
    HANDLE hFindHandle = INVALID_HANDLE_VALUE;
    WCHAR acVolumeName[MAX_PATH] = L"";
    WCHAR acDeviceName[MAX_PATH] = L"";
    DWORD dwError = ERROR_SUCCESS;
    size_t iIndex = 0;
    unsigned int uiDriveType = 0;
    BOOL bSuccess = FALSE;
    vector<wstring> vecVolumePaths;

    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Begin get all volumes.");
    hFindHandle = FindFirstVolumeW(acVolumeName, ARRAYSIZE(acVolumeName));
    if (hFindHandle == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"FindFirstVolumeW failed with error code %d.", dwError);
        return -1;
    }

    for (;;)
    {
        COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get one volume %s.", acVolumeName);
        //  Skip the \\?\ prefix and remove the trailing backslash.
        iIndex = wcslen(acVolumeName) - 1;
        mp_bool bVolumeName= (acVolumeName[0]     != L'\\' ||
            acVolumeName[1]     != L'\\' ||
            acVolumeName[2]     != L'?'  ||
            acVolumeName[3]     != L'\\' ||
            acVolumeName[iIndex] != L'\\'   ); 
        if (bVolumeName) 
        {
            FindVolumeClose(hFindHandle);
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s.", 
                acVolumeName);
            return -1;
        }

        //  QueryDosDeviceW does not allow a trailing backslash, so temporarily remove it.
        acVolumeName[iIndex] = L'\0';
        dwCharCount = QueryDosDeviceW(&acVolumeName[4], acDeviceName, ARRAYSIZE(acDeviceName));
        acVolumeName[iIndex] = L'\\';
        if (dwCharCount == 0)
        {
            dwError = GetLastError();
            FindVolumeClose(hFindHandle);
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"QueryDosDeviceW failed with error code %d.", dwError);
            return -1;
        }

        //Found a device \Device\HarddiskVolume5
        //Volume name: \\?\Volume{05b1aa0c-031d-11e5-ae2b-005056b12f3a}\
        //Paths:
        //"G:\"
        COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Found a device %s", acDeviceName);
        COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Volume name: %s", acVolumeName);
        COMMLOGW(OS_LOG_INFO, LOG_COMMON_INFO, L"Paths:");
        if (0 != GetVolumePaths(acVolumeName, vecVolumePaths))
        {
            COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"Get volume paths failed, volume name %s.", acVolumeName);
            FindVolumeClose(hFindHandle);
            return -1;
        }
        
        uiDriveType = GetDriveTypeW(acVolumeName);
        if (DRIVE_FIXED == uiDriveType)
        {
            vecVolumesList.push_back(acVolumeName);
        }
        
        //  Move on to the next volume.
        bSuccess = FindNextVolumeW(hFindHandle, acVolumeName, ARRAYSIZE(acVolumeName));
        if (!bSuccess)
        {
            dwError = GetLastError();
            if (dwError != ERROR_NO_MORE_FILES) 
            {
                FindVolumeClose(hFindHandle);
                COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"FindNextVolumeW failed with error code %d.", dwError);
                return -1;
            }

            COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Finished iterating through all the volumes.");
            break;
        }
    }

    FindVolumeClose(hFindHandle);
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Get all volumes succ.");
    return 0;
}

// Convert a failure type into a string
wstring HResult2String(HRESULT hrError)
{
    switch (hrError)
    {
        CHECK_CASE_FOR_CONSTANT(VSS_E_BAD_STATE);
        CHECK_CASE_FOR_CONSTANT(VSS_E_PROVIDER_ALREADY_REGISTERED);
        CHECK_CASE_FOR_CONSTANT(VSS_E_PROVIDER_NOT_REGISTERED);
        CHECK_CASE_FOR_CONSTANT(VSS_E_PROVIDER_VETO);
        CHECK_CASE_FOR_CONSTANT(VSS_E_PROVIDER_IN_USE);
        CHECK_CASE_FOR_CONSTANT(VSS_E_OBJECT_NOT_FOUND);
        CHECK_CASE_FOR_CONSTANT(VSS_S_ASYNC_PENDING);
        CHECK_CASE_FOR_CONSTANT(VSS_S_ASYNC_FINISHED);
        CHECK_CASE_FOR_CONSTANT(VSS_S_ASYNC_CANCELLED);
        CHECK_CASE_FOR_CONSTANT(VSS_E_VOLUME_NOT_SUPPORTED);
        CHECK_CASE_FOR_CONSTANT(VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER);
        CHECK_CASE_FOR_CONSTANT(VSS_E_OBJECT_ALREADY_EXISTS);
        CHECK_CASE_FOR_CONSTANT(VSS_E_UNEXPECTED_PROVIDER_ERROR);
        CHECK_CASE_FOR_CONSTANT(VSS_E_CORRUPT_XML_DOCUMENT);
        CHECK_CASE_FOR_CONSTANT(VSS_E_INVALID_XML_DOCUMENT);
        CHECK_CASE_FOR_CONSTANT(VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED);
        CHECK_CASE_FOR_CONSTANT(VSS_E_FLUSH_WRITES_TIMEOUT);
        CHECK_CASE_FOR_CONSTANT(VSS_E_HOLD_WRITES_TIMEOUT);
        CHECK_CASE_FOR_CONSTANT(VSS_E_UNEXPECTED_WRITER_ERROR);
        CHECK_CASE_FOR_CONSTANT(VSS_E_SNAPSHOT_SET_IN_PROGRESS);
        CHECK_CASE_FOR_CONSTANT(VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED);
        CHECK_CASE_FOR_CONSTANT(VSS_E_WRITER_INFRASTRUCTURE);
        CHECK_CASE_FOR_CONSTANT(VSS_E_WRITER_NOT_RESPONDING);
        CHECK_CASE_FOR_CONSTANT(VSS_E_WRITER_ALREADY_SUBSCRIBED);
        CHECK_CASE_FOR_CONSTANT(VSS_E_UNSUPPORTED_CONTEXT);
        CHECK_CASE_FOR_CONSTANT(VSS_E_VOLUME_IN_USE);
        CHECK_CASE_FOR_CONSTANT(VSS_E_MAXIMUM_DIFFAREA_ASSOCIATIONS_REACHED);
        CHECK_CASE_FOR_CONSTANT(VSS_E_INSUFFICIENT_STORAGE);

        // Regular COM errors
        CHECK_CASE_FOR_CONSTANT(S_OK);
        CHECK_CASE_FOR_CONSTANT(S_FALSE);
        CHECK_CASE_FOR_CONSTANT(E_UNEXPECTED);
        CHECK_CASE_FOR_CONSTANT(E_OUTOFMEMORY);

        default:
            break;
    }

    PWCHAR pwszBuffer = NULL;
    DWORD dwRet = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, hrError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pwszBuffer, 0, NULL);

    // No message found for this error. Just return <Unknown>
    if (dwRet == 0)
    {
        return wstring(L"<Unknown error code>");
    }

    // Convert the message into wstring
    wstring errorText(pwszBuffer);
    LocalFree(pwszBuffer);

    return errorText;
}

#endif

