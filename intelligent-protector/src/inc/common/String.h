/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_STRING_H__
#define __AGENT_STRING_H__

#include <stdarg.h>
#include <stdlib.h>
#ifndef WIN32
#include <strings.h>
#include<termios.h>
#endif
#include <list>
#include <vector>
#include "common/Types.h"
#include "common/Defines.h"

class AGENT_API CMpString
{
public:
    static mp_char* Trim(mp_char* str);
    static mp_char* TrimLeft(mp_char* str);
    static mp_char* TrimRight(mp_char* str);
    static mp_bool HasSpace(mp_char* str);
    static mp_char* ToUpper(mp_char* str);
    static mp_char* ToLower(mp_char* str);
    static mp_char* TotallyTrimRight(mp_char* str);
    static mp_void FormatLUNID(mp_string& rstrINLUNID, mp_string& rstrOUTLUNID);

    static mp_void StrToken(mp_string strToken, mp_string strSeparator, list<mp_string>& plstStr);
    //根据指定分割符分割字符串，并将结果存放vector中
    static mp_void StrSplit(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep);
    //字符串中包含空格，前后添加引号，解决路径中包含空格问题
    static mp_string BlankComma(const mp_string& strPath);
#ifdef WIN32
    static mp_string UnicodeToANSI(const mp_wstring &wStrSrc);
    static mp_wstring ANSIToUnicode(const mp_string &strSrc);
    static mp_string WString2String(const mp_wstring src);
    static mp_wstring String2WString(const mp_string src);
    static mp_string ANSIToUTF8(const mp_string &strSrc);
    static mp_string UTF8ToANSI(const mp_string &strSrc);
#else
    static mp_int32 GetCh();
#endif
};

#endif //__AGENT_STRING_H__

