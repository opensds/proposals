/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/String.h"
#include "common/Log.h"
#include "securec.h"
/*------------------------------------------------------------ 
Description  :整理字符串
Input        :  str--- 字符串
Output       :      
Return       :  返回字符串
                   NULL--- 字符串为空
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_char* CMpString::Trim(mp_char* str)
{
    if (NULL == str)
    {
        return NULL;
    }

    return TrimRight(TrimLeft(str));
}
/*------------------------------------------------------------ 
Description  :消除从左侧起所遇到的所有空格字符
Input        :  str--- 字符串
Output       :      
Return       :  返回整理后字符串
                   NULL--- 字符串为空
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_char* CMpString::TrimLeft(mp_char* str)
{
    if (NULL == str)
    {
        return NULL;
    }

    mp_char* ptr = str;
    mp_char* pret = str;
    while(' ' == *ptr)
    {
        ++ptr;
    }

    if(ptr == str)
    {
        return ptr;
    }

    while(*ptr)
    {
        *str = *ptr;
        ++str;
        ++ptr;
    }

    *str = 0;
    return pret;
}
/*------------------------------------------------------------ 
Description  :消除从右侧起所遇到的所有空格字符
Input        :  str--- 字符串
Output       :      
Return       :  返回整理后字符串
                   NULL--- 字符串为空
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_char* CMpString::TrimRight(mp_char* str)
{
    if (NULL == str)
    {
        return NULL;
    }

    mp_char* p1 = str;
    mp_char* p2 = str;

    do
    {
        while(*p1 != ' ' && *p1 != '\0')
        {
            p1++;
        }
        p2 = p1;

        while(*p1 == ' ')
        {
            p1++;
        }
    }while(*p1 != '\0');
    *p2 = '\0';

    return str;
}
/*------------------------------------------------------------ 
Description  :消除 所遇到的所有空格\t \n \r
Input        :  str--- 字符串
Output       :      
Return       :  返回整理后字符串
                   NULL--- 字符串为空
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_char* CMpString::TotallyTrimRight(mp_char* str)
{
    mp_int32 iWhile = 0;

    iWhile = (mp_int32)strlen(str);
    if (0 == iWhile)
    {
        return NULL;
    }
    while ((iWhile > 0) && ((' ' == str[iWhile - 1])
                            || ('\t' == str[iWhile - 1])
                            || ('\n' == str[iWhile - 1])
                            || ('\r' == str[iWhile - 1])))
    {
        iWhile--;
    }

    str[iWhile] = 0;
    return str;
}

//将查询出的LUN ID前面多余的0去掉
mp_void CMpString::FormatLUNID(mp_string& rstrINLUNID, mp_string& rstrOUTLUNID)
{
    mp_string strNumerics("123456789");
    mp_string::size_type strIndex;

    strIndex =  rstrINLUNID.find_first_of(strNumerics);
    if (mp_string::npos != strIndex)
    {
        rstrOUTLUNID = rstrINLUNID.substr(strIndex);
    }
    else
    {
        rstrOUTLUNID = "0";
    }
}

/*------------------------------------------------------------ 
Description  :判断空格
Input        :  str--- 字符串
Output       :      
Return       :  MP_TRUE---有空格
                   MP_FALSE---没有空格
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CMpString::HasSpace(mp_char* str)
{
    if (NULL == str)
    {
        return MP_FALSE;
    }

    for(; *str != '\0'; ++str)
    {
        if(*str == ' ')
        {
            return MP_TRUE;
        }
    }

    return MP_FALSE;
}
/*------------------------------------------------------------ 
Description  :转换为大写
Input        :  str--- 字符串
Output       :      
Return       :   返回转换后的字符串
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_char* CMpString::ToUpper(mp_char* str)
{
    mp_char* ptr = str;
    while(*ptr)
    {
        *ptr = (mp_char)toupper(*ptr);
        ++ptr;
    }

    return str;
}
/*------------------------------------------------------------ 
Description  :转换为小写
Input        :  str--- 字符串
Output       :      
Return       :   返回转换后的字符串
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_char* CMpString::ToLower(mp_char* str)
{
    mp_char* ptr = str;
    while(*ptr)
    {
        *ptr = (mp_char)tolower(*ptr);
        ++ptr;
    }

    return str;
}
/*------------------------------------------------------------ 
Description  : 提取字符串
Input        :  strToken--- 字符串，strSeparator---分割符
Output       :      
Return       :    
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CMpString::StrToken(mp_string strToken, mp_string strSeparator, list<mp_string>& plstStr)
{
    mp_char* pszTmpToken;
    mp_char* pNextToken;

    pszTmpToken = NULL;
    pNextToken = NULL;

    pszTmpToken = strtok_s((mp_char *)strToken.c_str(), strSeparator.c_str(),&pNextToken);
    while (NULL != pszTmpToken)
    {
        plstStr.push_back(mp_string(pszTmpToken));
        pszTmpToken = strtok_s(NULL, strSeparator.c_str(),&pNextToken);
    }
}
/*------------------------------------------------------------ 
Description  : 分割字符串
Input        :   
Output       :      
Return       :    
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CMpString::StrSplit(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
    mp_string::size_type start = 0, end = 0;
    while ((end = strText.find(cSep, start)) != mp_string::npos)
    {
        vecTokens.push_back(strText.substr(start, end - start));
        start = end + 1;
    }

    vecTokens.push_back(strText.substr(start));
}

/*------------------------------------------------------------
Function Name: BlankComma
Description  : 对于有空格的字符串，前后加引号，用于路径处理，解决路径包含空格问题
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_string CMpString::BlankComma(const mp_string& strPath)
{
    if (strPath.find(" ") != mp_string::npos && strPath[0] != '"')
    {
        return "\"" + strPath + "\"";
    }
    else
    {
        return strPath;
    }
}


#ifdef WIN32
/*------------------------------------------------------------ 
Description  : unicode转换为ansi
Input        :   
Output       :      
Return       :    返回ansi值
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_string CMpString::UnicodeToANSI(const mp_wstring &wStrSrc)
{
    mp_string strText("");
    mp_char* pElementText;
    mp_int32 iRet = EOK;
    mp_int32 iTextLen;

    // wide char to multi char
    iTextLen = WideCharToMultiByte(CP_ACP, 0, wStrSrc.c_str(), -1, NULL, 0, NULL, NULL);

    NEW_ARRAY_CATCH(pElementText, mp_char, iTextLen + 1);
    if (pElementText == NULL)
    {
        return strText;
    }

    iRet = memset_s((mp_void*)pElementText, sizeof(mp_char) * (iTextLen + 1), 0, sizeof(mp_char) * (iTextLen + 1));
    if (EOK != iRet)
    {
        delete[] pElementText;
        return strText;
    }

    ::WideCharToMultiByte(CP_ACP, 0, wStrSrc.c_str(), -1, pElementText, iTextLen, NULL, NULL);

    strText = pElementText;
    delete[] pElementText;
    return strText;
}
/*------------------------------------------------------------ 
Description  : ansi转换为unicode
Input        :   
Output       :      
Return       :    返回unicode值
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_wstring CMpString::ANSIToUnicode(const mp_string &strSrc)
{
    mp_wstring wStrTmp;
    mp_int32 dwNum = 0;
    mp_wchar *pElementText;
    mp_int32 iRet = EOK;

    dwNum = MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, NULL, 0);
    if (dwNum <= 0)
    {
        return wStrTmp;
    }

    NEW_ARRAY_CATCH(pElementText, mp_wchar, dwNum + 1)
    if (pElementText == NULL)
    {
        return wStrTmp;
    }

    iRet = memset_s( ( mp_void* )pElementText, sizeof( mp_wchar ) * ( dwNum + 1 ), 0, sizeof( mp_wchar ) * ( dwNum + 1 ) );
    if (EOK != iRet)
    {
        delete[] pElementText;
        return wStrTmp;
    }

    dwNum = MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, pElementText, dwNum);
    if (0 == dwNum)
    {
        delete[] pElementText;
        return wStrTmp;
    }

    wStrTmp = pElementText;
    delete[] pElementText;
    return wStrTmp;
}
/*------------------------------------------------------------ 
Description  : WideChar to String
Input        :   
Output       :      
Return       :    
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_string CMpString::WString2String(const mp_wstring src)
{
    vector<CHAR> chBuffer;
    
    mp_int32 iChars = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, NULL, 0, NULL, NULL);
    if (iChars <= 0)
    {
        return "";
    }

    chBuffer.resize(iChars);
    iChars = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, &chBuffer.front(), (mp_int32)chBuffer.size(), NULL, NULL);
    if (iChars <= 0)
    {
        return "";
    }

    return std::string(&chBuffer.front());
}

/*------------------------------------------------------------ 
Description  : String to WideChar
Input        :   
Output       :      
Return       :     
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_wstring CMpString::String2WString(const mp_string src)
{
    wstring wStrTmp;
    mp_int32 dwNum = 0;
    wchar_t* pElementText;
    mp_int32 iRet = EOK;
    dwNum = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0);
    if (dwNum <= 0)
    {
        return wStrTmp;
    }

    NEW_ARRAY_CATCH(pElementText, wchar_t, dwNum + 1);
    if (pElementText == NULL)
    {
        return wStrTmp;
    }
    iRet = memset_s((mp_void*)pElementText, sizeof(wchar_t) * (dwNum + 1), 0, sizeof(wchar_t) * (dwNum + 1));
    if (EOK != iRet)
    {
        delete[] pElementText;
        return wStrTmp;
    }

    dwNum = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, pElementText, dwNum);
    if (0 == dwNum)
    {
        delete[] pElementText;
        return wStrTmp;
    }

    wStrTmp = pElementText;
    delete[] pElementText;

    return wStrTmp;
}

/*------------------------------------------------------------ 
Description  : ansi to utf-8
Input        :   
Output       :      
Return       :     
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_string CMpString::ANSIToUTF8(const mp_string &strSrc)
{
    mp_string strUTF8;
    mp_wstring wstrSrc = CMpString::String2WString(strSrc);
    mp_int32 u8Len = WideCharToMultiByte(CP_UTF8, NULL, wstrSrc.c_str(), -1, NULL, 0, NULL, NULL);
    if (u8Len <= 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "u8Len is litter than 0, u8Len is %d.", u8Len);
        return strUTF8;
    }

    mp_char* cUTF8 = NULL;
    NEW_ARRAY_CATCH(cUTF8, mp_char, u8Len + 1);
    if (cUTF8 == NULL)
    {
        return strUTF8;
    }
    mp_int32 iRet = memset_s(cUTF8, u8Len + 1, 0, u8Len + 1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] cUTF8;;
        return strUTF8;
    }
    
    u8Len = WideCharToMultiByte(CP_UTF8, NULL, wstrSrc.c_str(), -1, cUTF8, u8Len, NULL, NULL);
    if (u8Len <= 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "u8Len is litter than 0, u8Len is %d.", u8Len);
        delete[] cUTF8;
        return strUTF8;
    }

    cUTF8[u8Len] = '\0';
    strUTF8 = cUTF8;
    delete[] cUTF8;
    return strUTF8;
}

/*------------------------------------------------------------ 
Description  : utf-8 to ansi
Input        :   
Output       :      
Return       :     
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_string CMpString::UTF8ToANSI(const mp_string &strSrc)
{
    mp_string strAnsi;
    mp_int32 ansiLen = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, NULL, 0);
    if (ansiLen <= 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ansiLen is litter than 0, ansiLen is %d.", ansiLen);
        return strAnsi;
    }

    mp_wchar* wAnsi = NULL;
    NEW_ARRAY_CATCH(wAnsi, mp_wchar, ansiLen + 1);
    mp_int32 iRet = memset_s(wAnsi, sizeof(wchar_t) * (ansiLen + 1), 0, sizeof(wchar_t) * (ansiLen + 1));
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] wAnsi;;
        return strAnsi;
    }

    ansiLen = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, wAnsi, ansiLen);
    if (ansiLen <= 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ansiLen is litter than 0, ansiLen is %d.", ansiLen);
        return strAnsi;
    }

    return CMpString::WString2String(wAnsi);
}


#else
/*------------------------------------------------------------ 
Description  : 获取字符
Input        :   
Output       :      
Return       :    返回字符
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpString::GetCh()
{
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);// 得到原来的终端属性
    newt = oldt;
    newt.c_lflag &= ~ICANON;// 设置非正规模式，如果程序每次要从终端读取一个字符的话，这是必须的
    newt.c_lflag &= ~ECHO;// 关闭回显
    newt.c_cc[VMIN] = 1;// 设置非正规模式下的最小字符数
    newt.c_cc[VTIME] = 0;// 设置非正规模式下的读延时
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);// 设置新的终端属性
    mp_int32 iChar = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);// 恢复旧属性
    return iChar;
}
#endif

