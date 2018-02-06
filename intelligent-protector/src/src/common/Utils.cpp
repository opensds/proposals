/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef WIN32
#include <signal.h>
#include <libgen.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#endif
#include <sstream>
#include <vector>
#include "common/Utils.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/UniqueId.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "common/String.h"
#include "securec.h"

thread_lock_t g_GetUidByUName_Mutex;
thread_lock_t g_DlibError_Mutex;
/*------------------------------------------------------------ 
Description  :睡眠函数
Input        : ms -- 时间
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void DoSleep(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    struct timeval stTimeOut;

    stTimeOut.tv_sec  = ms / 1000;
    stTimeOut.tv_usec = (ms % 1000) * 1000;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}
/*------------------------------------------------------------ 
Description  :注册信号
Input        : signo -- 信号
                  func -- 跟信号绑定的接口
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 SignalRegister(mp_int32 signo, signal_proc func)
{
#ifndef WIN32
    struct sigaction act;
    struct sigaction oact;

    (mp_void)memset_s(&act, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    if (0 != sigemptyset(&act.sa_mask))
    {
        return MP_FAILED;
    }
    act.sa_flags = 0;
    act.sa_handler = func;

    if (0 != sigaction(signo, &act, &oact))
    {
        return MP_FAILED;
    }
#endif

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :命令行输入检查
Input        : str -- 命令行输入
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
//不包含命令分隔符("&&"、"||"、"&"、";")则返回true，包含则返回false
mp_bool CheckCmdDelimiter(mp_string& str)
{
    size_t idx;

    idx = str.find(STR_SEMICOLON, 0);
    if (mp_string::npos != idx)
    {
        return MP_FALSE;
    }

    idx = str.find(STR_DOUBLE_VERTICAL_LINE, 0);
    if (mp_string::npos != idx)
    {
        return MP_FALSE;
    }

    idx = str.find(STR_ADDRESS, 0);
    if (mp_string::npos != idx)
    {
        return MP_FALSE;
    }

    idx = str.find(STR_DOUBLE_ADDRESS, 0);
    if (mp_string::npos != idx)
    {
        return MP_FALSE;
    }

    return MP_TRUE;
}

mp_int32 InitAgentRoot(mp_char* pszBinFilePath)
{
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :获取系统错误码
Input        : 
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 GetOSError()
{
#ifdef WIN32
    return GetLastError();
#else
    return errno;
#endif
}
/*------------------------------------------------------------ 
Description  :获取系统错误描述
Input        : 
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_char* GetOSStrErr(mp_int32 err, mp_char* buf, mp_size buf_len)
{
#ifdef WIN32
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
        );

    mp_int32 iRet = strncpy_s(buf, buf_len, (mp_char*)lpMsgBuf, strlen((mp_char*)lpMsgBuf));
    if (EOK != iRet)
    {
        LocalFree(lpMsgBuf);
        return NULL;
    }

    //strncpy(buf, (mp_char*)lpMsgBuf, buf_len - 1);
    buf[buf_len-1] = 0;

    LocalFree(lpMsgBuf);
#else
    //strerror_r(err, buf, buf_len);
    buf = strerror(err);
    buf[buf_len - 1] = 0;
#endif
    return buf;
}
/*------------------------------------------------------------ 
Description  :打开lib
Input        : pszLibName -- lib名
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_handle_t DlibOpen(const mp_char* pszLibName)
{
    LOGGUARD("");
#ifdef WIN32
    mp_handle_t h_dlib = 0;
    h_dlib = LoadLibrary(pszLibName);
    return h_dlib;
#else
    return DlibOpenEx(pszLibName, MP_TRUE);
#endif
}
/*------------------------------------------------------------ 
Description  :打开lib
Input        : pszLibName -- lib名
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_handle_t DlibOpenEx(const mp_char* pszLibName, mp_bool bLocal)
{
    LOGGUARD("");
#ifdef WIN32
    return DlibOpen(pszLibName);
#else
    mp_int32 flag = bLocal? DFLG_LOCAL : DFLG_GLOBAL;
    //Coverity&Fortify误报:FORTIFY.Process_Control
    //pszLibName引用出传递都是绝对路径
    return dlopen(pszLibName, flag);
#endif
}
/*------------------------------------------------------------ 
Description  :关闭lib
Input        : hDlib -- 打开lib时的句柄
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void DlibClose(mp_handle_t hDlib)
{
    LOGGUARD("");
#ifdef WIN32
    if(0 == hDlib)
    {
        return;
    }

    FreeLibrary(hDlib);
#else
    if(0 == hDlib)
    {
        return;
    }

    dlclose(hDlib);
#endif
}
/*------------------------------------------------------------ 
Description  :取得符号pszFname的地址
Input        : hDlib -- 句柄
                  pszFname -- 符号名
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void* DlibDlsym(mp_handle_t hDlib, const mp_char* pszFname)
{
    LOGGUARD("");
#ifdef WIN32
    return GetProcAddress(hDlib, pszFname);
#else
    return dlsym(hDlib, pszFname);
#endif
}
/*------------------------------------------------------------ 
Description  :取得lib里方法执行出错信息
Input        : 
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
const mp_char* DlibError(mp_char* szMsg, mp_uint32 isz)
{
#ifdef WIN32
    mp_int32 iErr = GetOSError();
    return GetOSStrErr(iErr, szMsg, isz);
#else
    
    CThreadAutoLock cLock(&g_DlibError_Mutex);
    const mp_char* pszErr = dlerror();
    if(NULL == pszErr)
    {
        szMsg[0] = 0;
        return NULL;
    }
    
    mp_int32 iRet = strncpy_s(szMsg, isz, pszErr, isz-1);
    if (EOK != iRet)
    {
        return NULL;
    }
    szMsg[isz - 1] = 0;
    return szMsg;
#endif
}
/*------------------------------------------------------------ 
Description  :初始化公共模块
Input        : 
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 InitCommonModules(mp_char* pszFullBinPath)
{
    mp_int32 iRet = MP_SUCCESS;

    //初始化Agent路径
    iRet = CPath::GetInstance().Init(pszFullBinPath);
    if (MP_SUCCESS != iRet)
    {
        printf("Init agent path failed.\n");
        return iRet;
    }

    //初始化配置文件模块
    iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (MP_SUCCESS != iRet)
    {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF);
        return iRet;
    }

    //初始化日志模块
    CLogger::GetInstance().Init(AGENT_LOG_NAME, CPath::GetInstance().GetLogPath());

    //在主程序初始化
    CUniqueID::GetInstance().Init();
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :获取主机名
Input        : 
Output       : strHostName -- 主机名
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 GetHostName(mp_string& strHostName)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_char szHostName[MAX_HOSTNAME_LEN] = {0};

#ifdef WIN32
    iRet = gethostname(szHostName, sizeof(szHostName));
    if (MP_SUCCESS != iRet)
    {
        iRet = WSAGetLastError();
    }
#else
    iRet = gethostname(szHostName, sizeof(szHostName));
    if (MP_SUCCESS != iRet)
    {
        iRet = GetOSError();
    }
#endif
    strHostName = szHostName;
    return iRet;
}

/*------------------------------------------------------------ 
Description  :获取用户名
Input        : 
Output       : strUserName -- 用户名
                  iErrCode    --   错误码
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 GetCurrentUserName(mp_string &strUserName, mp_ulong &iErrCode) 
{
//CodeDex误报，UNUSED_VALUE
//CodeDex误报，NEGATIVE_RETURNS，不用检查iErrCode入参的正负
#ifdef WIN32
    mp_ulong size = WINDOWS_USERNAME_LEN;   
    char *pUsername = new char[size];
    if (!GetUserName(pUsername, &size))
    {
        delete[] pUsername;
        iErrCode = GetOSError();
        // 日志模块使用，无法记录日志
        return MP_FAILED;
    }
    strUserName = pUsername;
    iErrCode = 0;
    delete[] pUsername;
    
#elif defined SOLARIS
    struct passwd pwd;
    struct passwd *result = NULL;
    mp_char *pbuf = NULL;
    mp_size size;

    //初始化buf的大小,通过系统函数进行获取
    size = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (MP_FALSE == size)
    {
        size = LINUX_USERNAME_LEN;
    }

    pbuf = (char *)malloc(size);
    if (NULL == pbuf) 
    {
        iErrCode = GetOSError();
        return MP_FAILED;
    }
    
    result = getpwuid_r(getuid(), &pwd, pbuf, size);
    if (NULL == result)
    {
        iErrCode = GetOSError();
        free(pbuf);
        pbuf = NULL;
        return MP_FAILED;
    }
    strUserName = pwd.pw_name;
    
    free(pbuf);
    pbuf = NULL;
    
#else
    struct passwd pwd;
    struct passwd *result = NULL;
    mp_char *pbuf = NULL;
    mp_int32 error;
    mp_size size;

    //初始化buf的大小,通过系统函数进行获取
    size = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (MP_FALSE == size)
    {
        size = LINUX_USERNAME_LEN;
    }

    pbuf = (char *)malloc(size);
    if (NULL == pbuf) 
    {
        iErrCode = GetOSError();
        return MP_FAILED;
    }
    
    error = getpwuid_r(getuid(), &pwd, pbuf, size, &result);
    if (MP_FAILED == error || NULL == result)
    {
        iErrCode = GetOSError();
        free(pbuf);
        pbuf = NULL;
        return MP_FAILED;
    }
    strUserName = pwd.pw_name;
    
    free(pbuf);
    pbuf = NULL;
#endif

    return MP_SUCCESS;
}

#ifdef WIN32
mp_int32 GetCurrentUserNameW(mp_wstring &strUserName, mp_ulong &iErrCode) 
{
    mp_ulong size = WINDOWS_USERNAME_LEN;   
    mp_wchar *pUsername = new mp_wchar[size];
    if (!GetUserNameW(pUsername, &size))
    {
        delete[] pUsername;
        iErrCode = GetOSError();
        // 日志模块使用，无法记录日志
        return MP_FAILED;
    }
    strUserName = pUsername;
    iErrCode = 0;
    delete[] pUsername;
    return MP_SUCCESS;
}

const mp_wchar* BaseFileNameW(const mp_wchar* pszFileName)
{
    const mp_wchar* p = pszFileName;
    mp_int64 len = wcslen(pszFileName);
    while (len > 0)
    {
        p = pszFileName + len;
        if (*p == L'\\')
            return &pszFileName[len + 1];

        len--;
    }

    return pszFileName;
}

#endif

const mp_char* BaseFileName(const mp_char* pszFileName)
{
    const mp_char* p = pszFileName;
    mp_int64 len = strlen(pszFileName);
    while (len > 0)
    {
        p = pszFileName + len;
#ifdef WIN32
        if (*p == '\\')
#else
        if (*p == '/')
#endif
            return &pszFileName[len + 1];

        len--;
    }

    return pszFileName;
}


/*---------------------------------------------------------------------------
Function Name: RemoveFullPathForLog
Description  : 去掉命令中的全路径方便日志打印， 
    如/usr/bin/ls filename,变成ls filename
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void RemoveFullPathForLog(mp_string strCmd, mp_string &strLogCmd)
{
    vector<mp_string> vecCmdParams;
    CMpString::StrSplit(vecCmdParams, strCmd, ' ');

    if (vecCmdParams.size() == 0)
    {
        strLogCmd = strCmd;
    }
    else
    {
        strLogCmd = BaseFileName(vecCmdParams[0].c_str());
        vector<mp_string>::iterator iter = vecCmdParams.begin();
        ++iter;
        for (; iter != vecCmdParams.end(); ++iter)
        {
            strLogCmd = strLogCmd + " " + *iter;
        }
    }
}

/*---------------------------------------------------------------------------
Function Name: PackageLog
Description  : 打包日志
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 PackageLog(mp_string strLogName)
{
    //调用脚本打包日志
    //部分日志是root权限，需要提升权限才可以导出，日志名称后缀有脚本添加
    COMMLOG(OS_LOG_INFO, OS_LOG_INFO, "Package Log Name is %s.", strLogName.c_str());
    //日志收集脚本输入日志打包名称不带后缀
    mp_string::size_type pos = strLogName.find(ZIP_SUFFIX);
    if (pos == mp_string::npos)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "strLogName is invalid.", strLogName.c_str());
    }
    mp_string strLogNameWithoutSuffix = strLogName.substr(0, pos);
#ifdef WIN32
    return CSystemExec::ExecScript(SCRIPT_PACKLOG_WIN, strLogNameWithoutSuffix, NULL);
#else
    ROOT_EXEC((mp_int32)ROOT_COMMAND_SCRIPT_PACKAGELOG, strLogNameWithoutSuffix, NULL);
    return MP_SUCCESS;
#endif
}

#ifndef WIN32
/*---------------------------------------------------------------------------
Function Name: GetUidByUserName
Description  : 根据用户名称获取UID和GID
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 GetUidByUserName(mp_string strUserName, mp_int32 &uid, mp_int32 &gid)
{
    struct passwd *user;
    uid = -1;
    gid = -1;

    CThreadAutoLock cLock(&g_GetUidByUName_Mutex);
    setpwent();
    while((user = getpwent()) != 0)
    {
        mp_string strUser = mp_string(user->pw_name);
        if (strUser.compare(strUserName) == 0)
        {
            uid = user->pw_uid;
            gid = user->pw_gid;
            break;
        }
    }
    endpwent();

    if (uid == -1)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Get uid of user(%s) failed.", strUserName.c_str());
        return MP_FAILED;
    }
    else
    {
        COMMLOG(OS_LOG_DEBUG, OS_LOG_DEBUG, "User(%s) info: uid=%d, gid=%d.", strUserName.c_str(), uid, gid);
        return MP_SUCCESS;
    }
}

/*---------------------------------------------------------------------------
Function Name: ChownFile
Description  : 设置文件的用户和组权限
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 ChownFile(mp_string strFileName, mp_int32 uid, mp_int32 gid)
{
    mp_int32 iRet = MP_SUCCESS;

    iRet = chown(strFileName.c_str(), uid, gid);
    if (0 != iRet)
    {   
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Chmod file(%s) failed, errno[%d]: %s.", BaseFileName(strFileName.c_str()), 
            errno, strerror(errno));
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

#endif


//以下临时使用固定函数实现，后续开源软件选型后采用开源软件优化
/*---------------------------------------------------------------------------
Function Name: CheckParamString
Description  : 检查string类型的参数
Input        : paramValue -- 输入字符串
                 lenBeg        -- 字符串最小长度
                 lenEnd        -- 字符串最大长度
                 strInclude   -- 必须包含字符串
                 strExclude   --不能包含字符串
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckParamString(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, 
    mp_string &strExclude)
{
    //CodeDex误报，Dead Code
    if(lenBeg == -1 && lenEnd == -1)
    {
        COMMLOG(OS_LOG_INFO, OS_LOG_INFO, "This string(%s) has no length restrictions.", paramValue.c_str());
    }
    // check length
    else if (paramValue.length() < lenBeg || paramValue.length() > lenEnd)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "The len of string(%s) is not between %d and %d.", paramValue.c_str(), lenBeg, lenEnd);
        return MP_FAILED;
    }

    // check include and exclude
    for (mp_string::iterator iter = paramValue.begin(); iter != paramValue.end(); ++iter)
    {
        // check exclude
        if (strExclude.find(*iter) != string::npos)
        {   
            COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "The string(%s) have exclude char %c.", paramValue.c_str(), *iter);
            return MP_FAILED; 
        }

        // check include
        if(!strInclude.empty())
        {
            if (strInclude.find(*iter) == string::npos)
            {   
                COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Char %c is not in include string(%s).", *iter, paramValue.c_str());
                return MP_FAILED; 
            }
        }
    }
    
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckParamString
Description  : 检查string类型的参数
Input        : paramValue -- 输入字符串
                 lenBeg        -- 字符串最小长度
                 lenEnd        -- 字符串最大长度
                 strPre         -- 字符串前缀
Return       :
Call           :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckParamString(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strPre)
{
    if (paramValue.length() < lenBeg || paramValue.length() > lenEnd)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "The len of string(%s) is not between %d and %d.", paramValue.c_str(), lenBeg, lenEnd);
        return MP_FAILED;
    }

    size_t idxPre = paramValue.find_first_of(strPre);
    if (idxPre != 0)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "String(%s) is not begin with %s.", paramValue.c_str(), strPre.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckParamString
Description  : 检查string类型的参数
Input        : paramValue -- 输入字符串
                 lenBeg        -- 字符串最小长度
                 lenEnd        -- 字符串最大长度
                 strPre         -- 字符串后缀
Return       :
Call           :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckParamStringEnd(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strEnd)
{
    if (paramValue.length() < lenBeg || paramValue.length() > lenEnd)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "The len of string(%s) is not between %d and %d.", paramValue.c_str(), lenBeg, lenEnd);
        return MP_FAILED;
    }

    size_t idxEnd = paramValue.rfind(strEnd);
    if (idxEnd != (paramValue.length() - strEnd.length()))
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "String(%s) is not end with %s.", paramValue.c_str(), strEnd.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckParamInteger32
Description  : 检查int32类型的参数
Input        : paramValue       -- 输入数字
                 begValue            -- 数字最小值
                 endValue            -- 数字最大值
                 vecExclude         -- 数字不能包含值
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckParamInteger32(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude)
{
    if (begValue != -1 && paramValue < begValue)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Check failed, %d < %d.", paramValue, begValue);
        return MP_FAILED;
    }
    
    if (endValue != -1 && paramValue > endValue)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Check failed, %d > %d.", paramValue, endValue);
        return MP_FAILED;
    }

    vector<mp_int32>::iterator iter = vecExclude.begin();
    for (; iter != vecExclude.end(); ++iter)
    {
        if (paramValue == *iter)
        {
            COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Check failed, %d = %d.", paramValue, *iter);
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckParamInteger64
Description  : 检查int64类型的参数
Input        : paramValue       -- 输入数字
                 begValue            -- 数字最小值
                 endValue            -- 数字最大值
                 vecExclude         -- 数字不能包含值
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckParamInteger64(mp_int64 paramValue, mp_int64 begValue, mp_int64 endValue, vector<mp_int64> &vecExclude)
{
    if (begValue != -1 && paramValue < begValue)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Check failed, %d < %d.", paramValue, begValue);
        return MP_FAILED;
    }
    
    if (endValue != -1 && paramValue > endValue)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Check failed, %d > %d.", paramValue, endValue);
        return MP_FAILED;
    }

    vector<mp_int64>::iterator iter = vecExclude.begin();
    for (; iter != vecExclude.end(); ++iter)
    {
        if (paramValue == *iter)
        {
            COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Check failed, %d = %d.", paramValue, *iter);
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckParamInteger
Description  : 检查参数是否是IP形式
Input        : paramValue       -- 输入IP字符串
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckParamStringIsIP(mp_string &paramValue)
{

    mp_int32 iIpnum1, iIpnum2, iIpnum3, iIpnum4;
    mp_int32 iIpnum;
    iIpnum=sscanf_s(paramValue.c_str(),"%d.%d.%d.%d",&iIpnum1,&iIpnum2,&iIpnum3,&iIpnum4); 
    if(iIpnum==4&&(iIpnum1>=1&&iIpnum1<=233)&&(iIpnum2>=0&&iIpnum2<=255)&&(iIpnum3>=0&&iIpnum3<=255)&&(iIpnum4>=0&&iIpnum4<=255)) 
    { 
        return MP_SUCCESS;
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "The string(%s) is not ip.", paramValue.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckPathString
Description  : 检查路径字符串是否大于最大长度
Input        : pathValue       -- 路径字符串
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckPathString(mp_string &pathValue)
{
#ifdef WIN32
    mp_int32 imaxpath = MAX_PATH;
#else
    mp_int32 imaxpath = PATH_MAX;
#endif
    if(pathValue.length() >= imaxpath) 
    { 
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "The path string(%s) len is not less than %d.", pathValue.c_str(), imaxpath);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckPathString
Description  : 检查路径字符串是否大于最大长度
Input        : pathValue       -- 路径字符串
               strPre          -- 前缀
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckPathString(mp_string &pathValue, mp_string strPre)
{
#ifdef WIN32
    mp_int32 imaxpath = MAX_PATH;
#else
    mp_int32 imaxpath = PATH_MAX;
#endif
    if(pathValue.length() >= imaxpath) 
    { 
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "The path string(%s) len is not less than %d.", pathValue.c_str(), imaxpath);
        return MP_FAILED;
    }

    size_t idxPre = pathValue.find_first_of(strPre);
    if (idxPre != 0)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "String(%s) is not begin with %s.", pathValue.c_str(), strPre.c_str());
        return MP_FAILED;
    }

    mp_string strInclude("");
    mp_string strExclude("");
    size_t idxSep = pathValue.find_last_of("/");
    if(mp_string::npos == idxSep)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
          "The string %s does not contain / character.", pathValue.c_str());
        return MP_FAILED;
    }
    mp_string strFileName = pathValue.substr(idxSep + 1);
    CHECK_FAIL_EX(CheckParamString(strFileName, 1, 254, strInclude, strExclude));
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckFileSysMountParam
Description  : 检查文件系统挂载参数
Input        : pathValue       -- 路径字符串
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckFileSysMountParam(mp_string strDeviceName, mp_int32 volumeType, mp_string strMountPoint)
{
    //参数校验
    mp_string strFileName;
    mp_string strPre("/\\");
    mp_int32 lenBeg, lenEnd;
    size_t idxSep;
    vector<mp_int32> vecExclude;

#ifdef WIN32
    lenBeg = lenEnd = 49;
#else
    lenBeg = 1;
    lenEnd = PATH_MAX - 1;
#endif    
    CHECK_FAIL_EX(CheckParamString(strDeviceName, lenBeg, lenEnd, strPre));
#ifndef WIN32
    idxSep = strDeviceName.find_last_of("/");
    if(mp_string::npos == idxSep)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
          "The string(%s) does not contain / character.", strDeviceName.c_str());
        return MP_FAILED;
    }
    strFileName = strDeviceName.substr(idxSep + 1);
    mp_string strInclude("");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamString(strFileName, 1, 254, strInclude, strExclude));
#endif

    CHECK_FAIL_EX(CheckParamInteger32(volumeType, 0, 4, vecExclude));

#ifdef WIN32
    lenBeg = 1;
    lenEnd = 1;
    mp_string strInclude("BCDEFGHIJKLMNOPQRSTUVWXYZ");
    mp_string strExclude;
    CHECK_FAIL_EX(CheckParamString(strMountPoint, lenBeg, lenEnd, strInclude, strExclude));
#else
    lenBeg = 1;
    lenEnd = PATH_MAX - 1;
    strPre = mp_string("/");
    CHECK_FAIL_EX(CheckParamString(strMountPoint, lenBeg, lenEnd, strPre));

    idxSep = strMountPoint.find_last_of("/");
    if(mp_string::npos == idxSep)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
          "The string(%s) does not contain / character.", strMountPoint.c_str());
        return MP_FAILED;
    }
    strFileName = strMountPoint.substr(idxSep + 1);
    strInclude = mp_string("");
    strExclude = mp_string("");
    if(!strFileName.empty())
    {
        CHECK_FAIL_EX(CheckParamString(strFileName, 1, 254, strInclude, strExclude));
    }
#endif
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CheckFileSysFreezeParam
Description  : 检查文件系统冻结参数
Input        : pathValue       -- 路径字符串
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CheckFileSysFreezeParam(mp_string strDiskNames)
{
    mp_int32 lenBeg, lenEnd;
#ifdef WIN32
    lenBeg = 1;
    lenEnd = 1;
    mp_string strInclude("BCDEFGHIJKLMNOPQRSTUVWXYZ");
    mp_string strExclude;
    CHECK_FAIL_EX(CheckParamString(strDiskNames, lenBeg, lenEnd, strInclude, strExclude));
#else
    lenBeg = 1;
    lenEnd = PATH_MAX - 1;
    mp_string strPre("/");
    CHECK_FAIL_EX(CheckParamString(strDiskNames, lenBeg, lenEnd, strPre));
#endif

#ifndef WIN32
    size_t idxSep = strDiskNames.find_last_of("/");
    if(mp_string::npos == idxSep)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
          "The string(%s) does not contain / character.", strDiskNames.c_str());
        return MP_FAILED;
    }
    mp_string strFileName = strDiskNames.substr(idxSep + 1);
    mp_string strInclude("");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamString(strFileName, 1, 254, strInclude, strExclude));
#endif
    return MP_SUCCESS;
}


/*---------------------------------------------------------------------------
Function Name: GetOSType
Description  : 获取OS类型，需要使用root权限执行
Input        : OS_TYPE_E       -- 操作系统类型，获取后通过当前字段返回
Return       : 
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void GetOSType(mp_int32 &iOSType)
{
    LOGGUARD("");
#ifdef WIN32
    iOSType = HOST_OS_WINDOWS;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Host is windows.");
#elif defined(AIX)
    iOSType = HOST_OS_AIX;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Host is AIX.");
#elif defined(HP_UX)
    iOSType = HOST_OS_HP_UX;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Host is HP-UX.");
#elif defined(SOLARIS)
    iOSType = HOST_OS_SOLARIS;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Host is Solaris.");
#elif defined(LINUX)
    //条件判断顺序不能变化，因为oracle linux下也存在redhat-release文件
    if (MP_TRUE == CMpFile::FileExist("/etc/SuSE-release"))
    {
        iOSType = HOST_OS_SUSE;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Host is SUSE.");
    }
    else if (MP_TRUE == CMpFile::FileExist("/etc/oracle-release"))
    {
        iOSType = HOST_OS_ORACLE_LINUX;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Host is oracle linux.");
    }
    else if (MP_TRUE == CMpFile::FileExist("/etc/redhat-release"))
    {
        iOSType = HOST_OS_REDHAT;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Host is redhat.");
    }
    else
    {
        iOSType = HOST_OS_OTHER_LINUX;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Host is other linux.");
    }
#else
    iOSType = HOST_OS_UNKNOWN;
    COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Canot not get OS type.");
#endif
}

//获取OS版本信息
mp_int32 GetOSVersion(mp_int32 iOSType, mp_string &strOSVersion)
{
    strOSVersion = "";
#ifdef LINUX
    // 使用cat命令，简单处理
    mp_string strExecCmd;
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_FAILED;
    
    if (HOST_OS_SUSE == iOSType)
    {
        strExecCmd = "cat /etc/SuSE-release | grep VERSION | awk  -F '=' '{print $2}' | sed 's/^[ \t]*//g'";
    }
    // redhat和oracle linux都使用redhat-release获取版本号
    else if (HOST_OS_ORACLE_LINUX == iOSType || HOST_OS_REDHAT == iOSType)
    {
        strExecCmd = "cat /etc/redhat-release | awk -F '.' '{print $1}' | awk '{print $NF}'";
    }
    else
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "OS type is %d, not support to get os version.", iOSType);
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "cmd '%s' will be excute.", strExecCmd.c_str());
    iRet = CSystemExec::ExecSystemWithEcho(strExecCmd, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute getting os version failed, iRet %d.", iRet);
        return iRet;
    }

    // 执行cat命令执行结果获取版本号
    if (vecResult.size() > 0)
    {
        strOSVersion = vecResult.front();
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of getting os version is empty.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
#else
    COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "OS type is %d, not support to get os version.", iOSType);
    return MP_SUCCESS;
#endif
}


