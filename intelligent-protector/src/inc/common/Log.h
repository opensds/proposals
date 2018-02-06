/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_LOG_H_
#define _AGENT_LOG_H_

#include <stdarg.h>
#include <list>
#include <queue>
#include <iomanip>
#include <sstream>
#include "common/Types.h"
#include "common/LogCode.h"
#include "common/Thread.h"
#include "common/File.h"
#include "common/Time.h"
#include "common/String.h"
#include "common/Utils.h"

//日志级别
#define OS_LOG_DEBUG        0
#define OS_LOG_INFO         1
#define OS_LOG_WARN         2
#define OS_LOG_ERROR        3
#define OS_LOG_CRI          4
#define MAX_HEAD_SIZE       16
#define MAX_MSG_SIZE        65536 * 6
#define LOG_HEAD_LENGTH     10
#define DEFAULT_LOG_COUNT   5
#define UNIT_K              1024
#define UNIT_M              1048576
#define DEFAULT_LOG_SIZE    UNIT_M * 10
#define MAX_LOG_FILE_SIZE   UNIT_M * 30
#define ADDLOG(x)           ADDLLU(x)
#define ADDLLU(x)           x##LLU
#define LOG_CACHE_ON		1
#define LOG_CACHE_OFF		0
#define LOG_CACHE_ASCII		1
#define LOG_CACHE_UNICODE	0
#define LOG_CACHE_MAXSIZE	30


#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#ifdef WIN32
#define LOGGUARD(pszFormat, ...)  CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, pszFormat, __VA_ARGS__)
#define COMMLOG(iLevel, ullCode, pszFormat, ...) CLogger::GetInstance().Log(iLevel, __LINE__, ullCode, __FILE__,  pszFormat, __VA_ARGS__)
#define COMMLOGW(iLevel, ullCode, pszFormat, ...) CLogger::GetInstance().LogW(iLevel, __LINE__, ullCode, __WFILE__,  pszFormat, __VA_ARGS__)
#elif defined SOLARIS
#define LOGGUARD(...)  CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define COMMLOG(iLevel, ullCode, ...) CLogger::GetInstance().Log(iLevel, __LINE__, ullCode, __FILE__, __VA_ARGS__)
#else
#define COMMLOG(iLevel, ullCode, pszFormat, args...) CLogger::GetInstance().Log(iLevel, __LINE__, ADDLOG(ullCode), __FILE__, pszFormat, ##args)
#define LOGGUARD(pszFormat, args...) CLogGuard logGuard(__LINE__, __FUNCTION__, __FILE__, pszFormat, ##args)
#endif

class AGENT_API CLogger
{
private:
    CLogger();

	typedef struct _CacheData
	{
		mp_int32 codeType;
		mp_string logCache;
		mp_wstring logCacheW;
		mp_uint32 cacheLen;
	} CacheData;

    static CLogger m_instance;  //单例对象
    //日志类线程互斥变量
    thread_lock_t m_tLock;
    //日志文件名
    mp_string m_strFileName;
    //日志文件路径
    mp_string m_strFilePath;
    //日志最大大小
    mp_int32 m_iMaxSize;
    //日志记录的当前级别
    mp_int32 m_iLogLevel;
    //日志个数
    mp_int32 m_iLogCount;
	//是否开启日志缓存功能
	mp_int32 m_cacheFlg;
	//日志缓存阈值大小(单位字节，配置文件单位为M)
	mp_uint64 m_cacheThreshold;
	//当前日志缓存大小(单位字节)
	mp_uint64 m_cacheSize;
	//日志缓存
	queue <CacheData> m_cacheContent;
	//日志缓存打开计数器
	mp_int32 m_OpenCacheNum;

    mp_int32 MkHead(mp_int32 iLevel, mp_char* pszHeadBuf, mp_int32 iBufLen);
    mp_int32 SwitchLogFile(const mp_char* pszLogPath, const mp_char* pszLogName, mp_int32 iLogCount);
    mp_int32 CreateLogFile(const mp_char* pszLogFile);
    FILE* OpenLogFile();
#ifdef WIN32
    mp_int32 ExecWinCmd(mp_char* pszCmdBuf, mp_uint32* uiRetCode);
    mp_int32 MkHeadW(mp_int32 iLevel, mp_wchar* pszHeadBuf, mp_int32 iBufLen);
	mp_void WriteLog2Cache(wostringstream &strWMsg);
#endif

	mp_void WriteLog2Cache(ostringstream &strMsg);
public:
    ~CLogger();
    static CLogger& GetInstance()
    {
        return m_instance;
    }
    void Init(const char* pcLogFileName, mp_string strFilePath)
    {
        m_strFilePath = strFilePath;
        m_strFileName = pcLogFileName;
    }

   	mp_int32 SetLogLevel(mp_int32 iLogLevel);
	mp_int32 SetLogCount(mp_int32 iLogCount);
	//打开日志缓存
	mp_void OpenLogCache();
	//关闭日志缓存
	mp_int32 CloseLogCache();
    mp_void ReadLevelAndCount();
	//临时放到public,为了UT打桩方便
	mp_void ReadLogCacheThreshold();
    mp_void Log(mp_int32 iLevel, const mp_int32 iFileLine, mp_uint64 ulCode, const mp_char* pszFileName,
        const mp_char* pszFormat, ...);
#ifdef WIN32
    mp_void LogW(mp_int32 iLevel, const mp_int32 iFileLine, mp_uint64 ulCode, const mp_wchar* pszFileName,
        const mp_wchar* pszFormat, ...);
#endif
};

class AGENT_API CLogGuard
{
public:
    CLogGuard(mp_int32 iLine, mp_string strFunctionName, mp_string strFileName, const char* pszFormat, ...);
    ~CLogGuard();
private:
    mp_int32 m_iLine;
    mp_string m_strFunctionName;
    mp_string m_strFileName;
};

#endif //_AGENT_LOG_H_

