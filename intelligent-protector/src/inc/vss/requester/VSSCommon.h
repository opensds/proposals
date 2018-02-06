/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

  Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : VSSCommon.h
  Version       : Initial Draft
  Author        : yangwenun 00275736
  Created       : 2014/04/30
  Last Modified :
  Description   : VSS相关通用函数代码，目前只有Requester使用，放在Requester目录下
  History       :
  1.Date        :
    Author      :
    Modification:
******************************************************************************/

#ifndef _RD_VSS_COMMON_
#define _RD_VSS_COMMON_

//#pragma comment(lib, "advapi32.lib")
#pragma warning(disable:4995)

//define this macro for COM
#ifndef _WIN32_WINNT   // Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501
#endif

#ifdef WIN32
//#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
//#include <aclapi.h>
#include <vector>
#include <atlbase.h>
#include <list>
//#include <comutil.h>
#include "vss.h"
#include "vswriter.h"
#include "vsbackup.h"
#include "common/Log.h"
//#include "Commf.h"

using namespace std;

#define GEN_STRINGIZE_ARG(X) #X
#define GEN_MERGE(A, B) A##B
#define GEN_MAKE_W(A) GEN_MERGE(L, A)
#define GEN_WSTRINGIZE(X) GEN_MAKE_W(GEN_STRINGIZE_ARG(X))

#define CHECK_COM( Call ) CHECK_COM_ERROR( Call, #Call )

#define CHECK_COM_ERROR( ErrorCode, Text )                                                  \
{                                                                                           \
    HRESULT hrInternal = ErrorCode;                                                         \
    if (FAILED(hrInternal))                                                                 \
    {                                                                                       \
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"ERROR: COM call %s failed.", GEN_WSTRINGIZE(Text));\
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"- Returned HRESULT = 0x%08lx", hrInternal);\
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"- Error text: %s", HResult2String(hrInternal).c_str());\
        throw(hrInternal);                                                                  \
    }                                                                                       \
}

#define CHECK_WIN32( Call )                                                                 \
{                                                                                           \
    BOOL bRes = Call;                                                                       \
    if (!bRes)                                                                              \
        CHECK_WIN32_ERROR(GetLastError(), #Call);                                           \
}

#define CHECK_WIN32_ERROR( ErrorCode, Text )                                                \
{                                                                                           \
    COMMLOGW(OS_LOG_DEBUG, LOG_COMMON_DEBUG, L"Executing Win32 call '%s'", GEN_WSTRINGIZE(Text));    \
    DWORD dwLastError = ErrorCode;                                                          \
    HRESULT hrInternal = HRESULT_FROM_WIN32(dwLastError);                                   \
    if (dwLastError != NOERROR)                                                             \
    {                                                                                       \
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"\nERROR: Win32 call %s failed.", GEN_WSTRINGIZE(Text)); \
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"- GetLastError() == %ld", dwLastError); \
        COMMLOGW(OS_LOG_ERROR, LOG_COMMON_ERROR, L"- Error text: %s", HResult2String(hrInternal).c_str()); \
        throw(hrInternal);                                                                  \
    }                                                                                       \
}

// Helper macros to print a GUID using printf-style formatting
#define WSTR_GUID_FMT  L"{%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x}"

#define GUID_PRINTF_ARG( X )                                \
    (X).Data1,                                              \
    (X).Data2,                                              \
    (X).Data3,                                              \
    (X).Data4[0], (X).Data4[1], (X).Data4[2], (X).Data4[3], \
    (X).Data4[4], (X).Data4[5], (X).Data4[6], (X).Data4[7]

#define CHECK_CASE_FOR_CONSTANT(value)                      \
    case value: return wstring(GEN_MAKE_W(#value));


class ReqException
{
    private:
        int m_iErrCode;

    public:
        ReqException(int iErrCode){m_iErrCode = iErrCode;};
        ~ReqException(){};

        int GetErrCode(){return m_iErrCode;};
};

class WString2Buffer
{
public:

    WString2Buffer(wstring & s) : m_s(s), m_sv(s.length() + 1, L'\0')
    {
        // Move data from wstring to the temporary vector
        std::copy(m_s.begin(), m_s.end(), m_sv.begin());
    }

    ~WString2Buffer()
    {
        // Move data from the temporary vector to the string
        m_sv.resize(wcslen(&m_sv[0]));
        m_s.assign(m_sv.begin(), m_sv.end());
    }

    // Return the temporary WCHAR buffer
    operator WCHAR* () {return &(m_sv[0]);}

    // Return the available size of the temporary WCHAR buffer
    size_t length() {return m_s.length();}

private:
    wstring& m_s;
    vector<WCHAR> m_sv;
};


bool IsEqual(wstring str1, wstring str2);

bool FindStringInList(wstring str, vector<wstring> stringList);

wstring AppendBackslash(wstring str);

wstring BSTR2WString(BSTR bstr);

wstring Guid2WString(GUID guid);

GUID& WString2Guid(wstring src);

wstring GetUniqueVolumeNameForPath(wstring path);

bool IsVolume(wstring volumePath);

wstring GetDisplayNameForVolume(wstring volumeName);

int GetAllVolumes(vector<wstring>& vecVolumesList);

wstring HResult2String(HRESULT hrError);

#endif
#endif

