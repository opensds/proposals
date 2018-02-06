/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef WIN32
#include <signal.h>
#include <sys/wait.h>
#endif

#include "common/Log.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "securec.h"

CLogger CLogger::m_instance;

CLogger::CLogger()
{
    CMpThread::InitLock(&m_tLock);
    m_iMaxSize = DEFAULT_LOG_SIZE;
    //设置日志默认级别和个数
    m_iLogLevel = OS_LOG_INFO;
    m_iLogCount = DEFAULT_LOG_COUNT;
    m_cacheThreshold = (mp_uint64)LOG_CACHE_MAXSIZE * 1024 * 1024;
    m_cacheFlg = LOG_CACHE_OFF;
    //设置打开日志次数
    m_OpenCacheNum = 0;
    m_cacheSize = 0;
}

CLogger::~CLogger()
{
    CMpThread::DestroyLock(&m_tLock);
    while (!m_cacheContent.empty())
    {
        m_cacheContent.pop();
    }
}
/*------------------------------------------------------------ 
Description  :设置log级别
Input        :      iLogLevel---log级别
Output       :     
Return       :    MP_SUCCESS---设置成功
                   MP_FAILED---设置失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLogger::SetLogLevel(mp_int32 iLogLevel)
{
    if (iLogLevel < OS_LOG_DEBUG || iLogLevel > OS_LOG_CRI)
    {
        return MP_FAILED;
    }

    m_iLogLevel = iLogLevel;

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :设置log计数
Input        :      iLogCount---log计数
Output       :     
Return       :    MP_SUCCESS---设置成功
                   MP_FAILED---设置失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLogger::SetLogCount(mp_int32 iLogCount)
{
    if (iLogCount <= 0)
    {
        return MP_FAILED;
    }

    m_iLogCount = iLogCount;

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :打开日志缓存
Input        :    
Output       :     
Return       :    MP_SUCCESS---设置成功
                   MP_FAILED---设置失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CLogger::OpenLogCache()
{
    m_cacheFlg = LOG_CACHE_ON;
    ++m_OpenCacheNum;
}

/*------------------------------------------------------------ 
Description  :关闭日志缓存
Input        :    
Output       :     
Return       :    MP_SUCCESS---设置成功
                   MP_FAILED---设置失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLogger::CloseLogCache()
{
    //适配多次打开日志缓存的情况，
    //必须需要等到全部关闭，才去关闭缓存
    --m_OpenCacheNum;
    if (m_OpenCacheNum > 0)
    {
        return MP_SUCCESS;
    }
    
    m_cacheFlg = LOG_CACHE_OFF;

    // 写入日志缓存到日志文件中
    FILE* pFile = OpenLogFile();
    if (NULL == pFile)
    {
        return MP_FAILED;
    }

    while (!m_cacheContent.empty())
    {
        if (m_cacheContent.front().codeType == LOG_CACHE_ASCII)
        {
            fprintf(pFile, "%s", m_cacheContent.front().logCache.c_str());
        }
        else
        {
        	//CodeDex误报,KLOCWORK.SV.FMT_STR.PRINT_FORMAT_MISMATCH.UNDESIRED
            fwprintf(pFile, L"%s", m_cacheContent.front().logCacheW.c_str()); //lint !e559
        }
        m_cacheContent.pop();
    }
    m_cacheSize = 0;

    fflush(pFile);
    fclose(pFile);

    return MP_SUCCESS;
}

//创建日志信息的"标题头"
mp_int32 CLogger::MkHead(mp_int32 iLevel, mp_char* pszHeadBuf, mp_int32 iBufLen)
{
    mp_int32 iRet = MP_SUCCESS;

    switch (iLevel)
    {
        case OS_LOG_DEBUG:
            iRet = SNPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "DBG");
            break;
        case OS_LOG_INFO:
            iRet = SNPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "INFO");
            break;
        case OS_LOG_WARN:
            iRet = SNPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "WARN");
            break;
        case OS_LOG_ERROR:
            iRet = SNPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "ERR");
            break;
        case OS_LOG_CRI:
            iRet = SNPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "CRI");
            break;
        default:
            iRet= SNPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, "%s", "");
            break;
    }


    if (MP_FAILED == iRet)
    {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :选择log文件
Input        :      pszLogPath---log存放路径，pszLogName---log名，iLogCount---log计数行
Output       :     
Return       :     
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLogger::SwitchLogFile(const mp_char* pszLogPath, const mp_char* pszLogName, mp_int32 iLogCount)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_char acDestFile[MAX_FULL_PATH_LEN] = {0};
    mp_char acBackFile[MAX_FULL_PATH_LEN] = {0};
    mp_string strSuffix = "zip";
    mp_string strCommand;
	//CodeDex误报，Command Injection
    mp_string strLogFile = mp_string("") + pszLogPath + PATH_SEPARATOR + pszLogName;
    mp_int32 i = iLogCount;

#ifndef WIN32
    strSuffix = "gz";
#endif

    //删除时间最久的一个备份日志文件
    iRet = SNPRINTF_S(acBackFile, MAX_FULL_PATH_LEN, MAX_FULL_PATH_LEN - 1, \
        "%s%s%s.%d.%s", pszLogPath, PATH_SEPARATOR, pszLogName, i, strSuffix.c_str());
    if (MP_FAILED == iRet)
    {
        return iRet;
    }

    (void)remove(acBackFile);

    //一个一个更改日志文件名称
    i--;
    for (; i >= 0; i--)
    {
        //第一个文件名中不包括0
        if (0 == i)
        {
            iRet = SNPRINTF_S(acBackFile, MAX_FULL_PATH_LEN, MAX_FULL_PATH_LEN - 1, "%s%s%s.%s", \
                pszLogPath, PATH_SEPARATOR, pszLogName, strSuffix.c_str());
        }
        else
        {
            iRet = SNPRINTF_S(acBackFile, MAX_FULL_PATH_LEN, MAX_FULL_PATH_LEN - 1, "%s%s%s.%d.%s", \
                pszLogPath, PATH_SEPARATOR, pszLogName, i, strSuffix.c_str());
        }

        if (MP_FAILED == iRet)
        {
            return iRet;
        }

        if (0 == i)
        {            
            //压缩文件备份文件
#ifndef WIN32
            strCommand = mp_string("gzip -f -q -9 \"") + strLogFile + "\"";
            if (CheckCmdDelimiter(strCommand) == MP_FALSE)
            {
                return MP_FAILED;
            }

            //Coverity&Fortify误报:FORTIFY.Command_Injection 
            //已经按修复建议增加命令分隔符判断CheckCmdDelimiter
            iRet = system(strCommand.c_str());
            if(!WIFEXITED(iRet))
            {
                //system异常返回
                return MP_FAILED;
            }
            
#else // windows
            mp_uint32 uiRetCode = 0;
            mp_string strZipTool = CPath::GetInstance().GetBinFilePath("7z.exe");
            strCommand = mp_string("\"") + strZipTool + "\" a -y -tzip \"" + acBackFile + "\" \"" + strLogFile + "\" -mx=9";

            if (CheckCmdDelimiter(strCommand) == MP_FALSE)
            {
                return MP_FAILED;
            }

            iRet = ExecWinCmd((mp_char*)strCommand.c_str(), &uiRetCode);
            if ((MP_SUCCESS != iRet) || (0 != uiRetCode))
            {
                return MP_FAILED;
            }

            (void)remove(strLogFile.c_str());
#endif            

        }

        iRet = SNPRINTF_S(acDestFile, MAX_FULL_PATH_LEN, MAX_FULL_PATH_LEN - 1, "%s%s%s.%d.%s", \
            pszLogPath, PATH_SEPARATOR, pszLogName, (i + 1), strSuffix.c_str());
        if (MP_FAILED == iRet)
        {
            return iRet;
        }
        //CodeDex误报，SECURE_CODING
        (void)rename(acBackFile, acDestFile);

#ifndef WIN32
        (void)chmod(acDestFile, S_IRUSR);
#endif


    }

    iRet = CreateLogFile(strLogFile.c_str());
    if (MP_FAILED == iRet)
    {
        return iRet;
    }

    return iRet;
}
/*------------------------------------------------------------ 
Description  :创建log文件
Input        :      pszLogFile---log文件名
Output       :     
Return       :    MP_SUCCESS---创建成功
                   MP_FAILED---创建失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLogger::CreateLogFile(const mp_char* pszLogFile)
{
#ifdef WIN32
    FILE* pFile = NULL;
    mp_string pszLogFileName;
    mp_int32 itmp = 0;
    //CodeDex误报，Race Condition:File System Access
    pFile = fopen(pszLogFile, "w");
    if (NULL == pFile)
    {
        return MP_FAILED;
    }
    fclose(pFile);

    itmp = ((mp_string)pszLogFile).find_last_of(PATH_SEPARATOR);
    if (string::npos == itmp)
    {
        return MP_FAILED;
    }
    pszLogFileName = ((mp_string)pszLogFile).substr(itmp + 1);

    string strCommand = "cmd.exe /c echo Y | cacls.exe \"" + string(pszLogFile) + "\" /E /R Users > NUL";
    mp_uint32 uiRetCode = 0;

    mp_int32 iRtn = ExecWinCmd((mp_char*)strCommand.c_str(), &uiRetCode);
    if ((MP_SUCCESS != iRtn) || (0 != uiRetCode))
    {
        return MP_FAILED;
    }
#else
    mp_int32 fd = 0;
    fd = open(pszLogFile, O_CREAT, S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        return MP_FAILED;
    }
    close(fd);
    // 设置权限
    (void)chmod(pszLogFile, S_IRUSR | S_IWUSR);
#endif

    return MP_SUCCESS;
}
 
/*------------------------------------------------------------ 
Description  :读取log级别和计数
Input        :      
Output       :     
Return       :     
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CLogger::ReadLevelAndCount()
{
    mp_int32 iLogLevel = 0;
    mp_int32 iLogCount = 0;

    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_LEVEL, iLogLevel);
    if (MP_SUCCESS != iRet)
    {
        return;
    }
    m_iLogLevel = iLogLevel;

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_COUNT, iLogCount);
    if (MP_SUCCESS != iRet)
    {
        return;
    }
    m_iLogCount = iLogCount;
}

/*------------------------------------------------------------ 
Description  :读取log缓存的阈值
Input        :      
Output       :     
Return       :     
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CLogger::ReadLogCacheThreshold()
{
    mp_int32 iLogCacheThreshold;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_CACHE_THRESHOLD, iLogCacheThreshold);
    if (MP_SUCCESS != iRet)
    {
        m_cacheThreshold = (mp_uint64)LOG_CACHE_MAXSIZE * 1024 * 1024;
        return;
    }
    m_cacheThreshold = (mp_uint64)iLogCacheThreshold * 1024 * 1024;
}

/*------------------------------------------------------------ 
Description  :打开log文件
Input        :      
Output       :     
Return       :     strMsg---字符流
                    NULL---打开失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CLogger::WriteLog2Cache(ostringstream &strMsg)
{
    // 检查当前缓存是否超过阈值，有日志线程锁，此处为单线程操作
    // 超过阈值则删除最旧的日志直到满足阈值
    m_cacheSize += strMsg.str().length();
    while (m_cacheSize > m_cacheThreshold)
    {
        m_cacheSize -= m_cacheContent.front().cacheLen;
        if (!m_cacheContent.empty())
        {
            m_cacheContent.pop();
        }
    }

    CacheData LogCache;
    LogCache.codeType = LOG_CACHE_ASCII;
    LogCache.logCache = strMsg.str();
    LogCache.cacheLen = strMsg.str().length();
    m_cacheContent.push(LogCache); 
}

#ifdef WIN32
/*------------------------------------------------------------ 
Description  :打开log文件
Input        :      
Output       :     
Return       :     strWMsg---长字节字符流
                    NULL---打开失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CLogger::WriteLog2Cache(wostringstream &strWMsg)
{
    // 检查当前缓存是否超过阈值，有日志线程锁，此处为单线程操作
    // 超过阈值则删除最旧的日志直到满足阈值
    m_cacheSize += strWMsg.str().length();
    while (m_cacheSize > m_cacheThreshold)
    {
        m_cacheSize -= m_cacheContent.front().cacheLen;
        if (!m_cacheContent.empty())
        {
            m_cacheContent.pop();
        }
    }

    CacheData LogCacheW;
    LogCacheW.codeType = LOG_CACHE_UNICODE;
    LogCacheW.logCacheW = strWMsg.str();
    LogCacheW.cacheLen = strWMsg.str().length();
    m_cacheContent.push(LogCacheW);
}

/*------------------------------------------------------------ 
Description  :打开log文件
Input        :      
Output       :     
Return       :     pFile---文件描述符
                    NULL---打开失败
Create By    :
Modification : 
-------------------------------------------------------------*/
FILE* CLogger::OpenLogFile()
{
    //codedex误报CANONICAL_FILEPATH，日志路径不存在这个问题
    mp_string strLogFilePath;
    mp_uint32 uiFileSize = 0;
    FILE* pFile = NULL;
    mp_int32 iRet = 0;

    strLogFilePath = m_strFilePath + PATH_SEPARATOR + m_strFileName;
    if (MP_TRUE != CMpFile::FileExist(strLogFilePath.c_str()))
    {
        iRet = CreateLogFile(strLogFilePath.c_str());
        if (MP_FAILED == iRet)
        {
            return NULL;
        }
    }
    else
    {
        if (MP_SUCCESS != CMpFile::FileSize(strLogFilePath.c_str(), uiFileSize))
        {
            return NULL;
        }

        if (uiFileSize >= (mp_uint32)m_iMaxSize)
        {
            iRet = SwitchLogFile(m_strFilePath.c_str(), m_strFileName.c_str(), m_iLogCount);
            if (MP_FAILED == iRet)
            {
                return NULL;
            }
        }
    }

    pFile = fopen(strLogFilePath.c_str(), "a+");
    if (NULL == pFile)
    {
        return NULL;
    }

    return pFile;
}
#else
/*------------------------------------------------------------ 
Description  :打开log文件
Input        :      
Output       :     
Return       :     pFile---文件描述符
                    NULL---打开失败
Create By    :
Modification : 
-------------------------------------------------------------*/
FILE* CLogger::OpenLogFile()
{
    //codedex误报CANONICAL_FILEPATH，日志路径不存在这个问题
    mp_string strLogFilePath;
    FILE* pFile = NULL;
    mp_int32 fd;
    struct stat st;
    mp_int32 iRet = 0;

    strLogFilePath = m_strFilePath + PATH_SEPARATOR + m_strFileName;
    fd = open(strLogFilePath.c_str(), O_RDONLY);
    if (-1 == fd)
    {
        iRet = CreateLogFile(strLogFilePath.c_str());
        if (MP_FAILED == iRet)
        {
            return NULL;
        }
    }
    else
    {
        (void)fstat(fd, &st);
        close(fd);
        //日志超过大小切换日志
        if (st.st_size >= m_iMaxSize)
        {
            iRet = SwitchLogFile(m_strFilePath.c_str(), m_strFileName.c_str(), m_iLogCount);
            if (MP_FAILED == iRet)
            {
                return NULL;
            }
        }
    }

    pFile = fopen(strLogFilePath.c_str(), "a+");
    if (NULL == pFile)
    {
        return NULL;
    }

    return pFile;
}

#endif

#ifdef WIN32
/*------------------------------------------------------------ 
Description  :执行Windows命令
Input        :      pszCmdBuf---命令缓存
Output       :     uiRetCode---返回码
Return       :     MP_SUCCESS---执行成功
                    MP_FAILED---执行失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLogger::ExecWinCmd(mp_char* pszCmdBuf, mp_uint32* uiRetCode)
{
    list<mp_string> lstContents;
    mp_int32 iRet = 0;
    mp_string strCmd = pszCmdBuf;
    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
    DWORD dwCode = 0;

    if (!CheckCmdDelimiter(strCmd))
    {
        return MP_FAILED;
    }

    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    stStartupInfo.wShowWindow = SW_HIDE;
    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));

    if (!CreateProcess(NULL, TEXT(pszCmdBuf), NULL, NULL, MP_FALSE, 0, NULL, NULL, &stStartupInfo, &stProcessInfo))
    {
        *uiRetCode = GetLastError();

        return MP_FAILED;
    }

    if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess, 1000 * 3600))
    {
        *uiRetCode = 1;
    }
    else
    {
        GetExitCodeProcess(stProcessInfo.hProcess, &dwCode);
        *uiRetCode = dwCode;
    }

    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :制作 head打印信息
Input        :      iLevel---日志级别
Output       :     pszHeadBuf---head，iBufLen---head长度
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLogger::MkHeadW(mp_int32 iLevel, mp_wchar* pszHeadBuf, mp_int32 iBufLen)
{
    mp_int32 iRet = MP_SUCCESS;
    switch (iLevel)
    {
        case OS_LOG_DEBUG:
            iRet = SNWPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, L"%s", L"DBG");
            break;
        case OS_LOG_INFO:
            iRet = SNWPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, L"%s", L"INFO");
            break;
        case OS_LOG_WARN:
            iRet = SNWPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, L"%s", L"WARN");
            break;
        case OS_LOG_ERROR:
            iRet = SNWPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, L"%s", L"ERR");
            break;
        case OS_LOG_CRI:
            iRet = SNWPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, L"%s", L"CRI");
            break;
        default:
            iRet= SNWPRINTF_S(pszHeadBuf, iBufLen, iBufLen - 1, L"%s", L"");
            break;
    }
    
    if (MP_FAILED == iRet)
    {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :打印日志信息
Input        :      iLevel---日志级别，iFileLine---文件行号，ulCode---ul码，pszFormat---格式
Output       :     
Return       :      
                     
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CLogger::LogW(mp_int32 iLevel, const mp_int32 iFileLine, mp_uint64 ulCode,
    const mp_wchar* pszFileName, const mp_wchar* pszFormat, ...)
{
    mp_wchar acMsgHead[LOG_HEAD_LENGTH];
    static mp_wchar acMsg[MAX_MSG_SIZE];
    FILE* pFile = NULL;
    mp_tm* pstCurTime = NULL;
    mp_tm stCurTime;
    mp_time tLongTime;
    va_list pszArgp;
    mp_int32 iRet = 0;
    wostringstream strWMsg;
    mp_wstring strUserName;
    mp_ulong iErrCode = 0;

    //支持动态修改日志级别
    ReadLevelAndCount();
    // 支持动态修改日志缓存阈值大小
    ReadLogCacheThreshold();
    //根据定义的日志级别记录日志
    if (iLevel < m_iLogLevel)
    {
        return;
    }

    va_start(pszArgp, pszFormat);
    CMpTime::Now(&tLongTime);
    pstCurTime = CMpTime::LocalTimeR(&tLongTime, &stCurTime);
    if (NULL == pstCurTime)
    {
        va_end(pszArgp);
        return;
    }

    CThreadAutoLock tlock(&m_tLock);
    //初始化日志缓冲区
    acMsg[0] = 0;
    iRet = VSNWPRINTF_S(acMsg, MAX_MSG_SIZE, MAX_MSG_SIZE - 1, pszFormat, pszArgp);
    if (MP_FAILED == iRet)
    {
        va_end(pszArgp);
        return;
    }

    if (MP_SUCCESS != MkHeadW(iLevel, acMsgHead, LOG_HEAD_LENGTH))
    {
        va_end(pszArgp);
        return;
    }
    
    strWMsg <<L"[" 
       <<std::setfill(L'0') 
       <<std::setw(4) <<(pstCurTime->tm_year + 1900) <<L"-"
       <<std::setw(2) <<(pstCurTime->tm_mon + 1) <<L"-" 
       <<std::setw(2) <<pstCurTime->tm_mday <<L" "
       <<std::setw(2) <<pstCurTime->tm_hour <<L":" 
       <<std::setw(2) <<pstCurTime->tm_min <<L":" 
       <<std::setw(2) <<pstCurTime->tm_sec <<L"][0x" 
       <<std::setw(16) <<std::hex <<ulCode <<L"]["
       <<std::dec <<CMpThread::GetThreadId()<<L"][";
    
    // get user name
    if (MP_SUCCESS == GetCurrentUserNameW(strUserName, iErrCode))
    {
        strWMsg <<strUserName <<L"][";; 
    }
    else
    {
        strWMsg <<L"e-" <<std::dec <<iErrCode <<L"][";; 
    }

    strWMsg <<acMsgHead <<L"][" <<BaseFileNameW(pszFileName) <<L"," <<iFileLine <<L"]" <<acMsg <<std::endl;

    // 判断日志缓存是否开启
    if (LOG_CACHE_ON == m_cacheFlg)
    {
        WriteLog2Cache(strWMsg);
    }
    else
    {  
        pFile = OpenLogFile();
        if (NULL == pFile)
        {
            va_end(pszArgp);
            return;
        }
        
        fwprintf(pFile, L"%s", strWMsg.str().c_str());
        fflush(pFile);
        fclose(pFile);
    }

    va_end(pszArgp);
}
#endif
/*------------------------------------------------------------ 
Description  :打印日志信息
Input        :      iLevel---日志级别，iFileLine---文件行号，ulCode---ul码，pszFormat---格式
Output       :     
Return       :      
                     
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CLogger::Log(mp_int32 iLevel, const mp_int32 iFileLine, mp_uint64 ulCode,
    const mp_char* pszFileName, const mp_char* pszFormat, ...)
{
    mp_char acMsgHead[LOG_HEAD_LENGTH] = {0};
    static mp_char acMsg[MAX_MSG_SIZE] = {0};
    FILE* pFile = NULL;
    mp_tm* pstCurTime = NULL;
    mp_tm stCurTime;
    mp_time tLongTime;
    va_list pszArgp;
    mp_int32 iRet = 0;
    ostringstream strMsg;
    mp_string strUserName;
    mp_ulong iErrCode = 0;

    //支持动态修改日志级别
    ReadLevelAndCount();
    // 支持动态修改日志缓存阈值大小
    ReadLogCacheThreshold();
    //根据定义的日志级别记录日志
    if (iLevel < m_iLogLevel)
    {
        return;
    }

    va_start(pszArgp, pszFormat);
    CMpTime::Now(&tLongTime);
    pstCurTime = CMpTime::LocalTimeR(&tLongTime, &stCurTime);
    if (NULL == pstCurTime)
    {
        va_end(pszArgp);
        return;
    }

    CThreadAutoLock tlock(&m_tLock);
    //初始化日志缓冲区
    acMsg[0] = 0;
    //Coverity&Fortify误报:FORTIFY.Format_String
    //pszFormat都是程序内记录日志硬编码的
    iRet = VSNPRINTF_S(acMsg, sizeof(acMsg),sizeof(acMsg) - 1, pszFormat, pszArgp); //lint !e530
    if (MP_FAILED == iRet)
    {
        va_end(pszArgp);
        return;
    }

    iRet = MkHead(iLevel, acMsgHead, sizeof(acMsgHead));
    if (MP_SUCCESS != iRet)
    {
        va_end(pszArgp);
        return;
    }

    strMsg <<"[" 
       <<std::setfill('0') 
       <<std::setw(4) <<(pstCurTime->tm_year + 1900) <<"-"
       <<std::setw(2) <<(pstCurTime->tm_mon + 1) <<"-" 
       <<std::setw(2) <<pstCurTime->tm_mday <<" "
       <<std::setw(2) <<pstCurTime->tm_hour <<":" 
       <<std::setw(2) <<pstCurTime->tm_min <<":" 
       <<std::setw(2) <<pstCurTime->tm_sec <<"][0x" 
       <<std::setw(16) <<std::hex <<ulCode <<"][";

#ifdef WIN32
    strMsg <<std::dec <<CMpThread::GetThreadId();
    if (MP_SUCCESS == GetCurrentUserName(strUserName, iErrCode))
    {
        strMsg <<"][" <<strUserName; 
    }
    else
    {
        strMsg <<"][e-" <<std::dec <<iErrCode; 
    }
#else
	//CodeDex误报，System Information Leak:Internal
    if (MP_SUCCESS == GetCurrentUserName(strUserName, iErrCode))
    {
        strMsg <<std::dec <<getpid() <<"][" <<CMpThread::GetThreadId() <<"][" <<strUserName;
    }
    else
    {
        strMsg <<std::dec <<getpid() <<"][" <<CMpThread::GetThreadId() <<"][u:" <<std::dec <<getuid() <<"e:" <<std::dec <<iErrCode;
    }
#endif

    strMsg <<"][" <<acMsgHead <<"][" <<BaseFileName(pszFileName) <<"," <<iFileLine <<"]" <<acMsg <<std::endl;

    // 判断日志缓存是否开启
    if (LOG_CACHE_ON == m_cacheFlg)
    {
        WriteLog2Cache(strMsg);
    }
    else
    {   
        pFile = OpenLogFile();
        if (NULL == pFile)
        {
            va_end(pszArgp);
            return;
        }
        
        fprintf(pFile, "%s", strMsg.str().c_str());
        fflush(pFile);
        fclose(pFile);
    }

    va_end(pszArgp);
}
 
CLogGuard::CLogGuard(mp_int32 iLine, mp_string strFunctionName, mp_string strFileName, const char* pszFormat, ...)
     : m_iLine(iLine), m_strFileName(strFileName)
{
    mp_char acMsg[1024] = {0};
    va_list pszArgp;
    
    va_start(pszArgp, pszFormat);
    //Coverity&Fortify误报:FORTIFY.Format_String
    //pszFormat都是程序内记录日志硬编码的
    mp_int32 iRet = VSNPRINTF_S(acMsg, sizeof(acMsg),sizeof(acMsg) - 1, pszFormat, pszArgp); //lint !e530
    if (MP_FAILED == iRet)
    {
        va_end(pszArgp);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "VSNPRINTF_S failed.");
        return;
    }
    //不打印函数参数
    mp_string::size_type pos = strFunctionName.find("(");
    (pos != mp_string::npos) ? m_strFunctionName = strFunctionName.substr(0, pos) : m_strFunctionName = strFunctionName;

#ifdef WIN32
        CLogger::GetInstance().Log(OS_LOG_DEBUG, m_iLine, LOG_COMMON_DEBUG,
              m_strFileName.c_str(),  "Enter %s...%s", m_strFunctionName.c_str(), acMsg);
#else
        CLogger::GetInstance().Log(OS_LOG_DEBUG, m_iLine, ADDLOG(LOG_COMMON_DEBUG),
              m_strFileName.c_str(), "Enter %s...%s", m_strFunctionName.c_str(), acMsg);
#endif
    va_end(pszArgp);
}

CLogGuard::~CLogGuard()
{
#ifdef WIN32
    CLogger::GetInstance().Log(OS_LOG_DEBUG, 0xffff, LOG_COMMON_DEBUG,
                  m_strFileName.c_str(), "Leave %s...", m_strFunctionName.c_str());
#else
    CLogger::GetInstance().Log(OS_LOG_DEBUG, 0xffff, ADDLOG(LOG_COMMON_DEBUG),
               m_strFileName.c_str(), "Leave %s...", m_strFunctionName.c_str());
#endif
}

