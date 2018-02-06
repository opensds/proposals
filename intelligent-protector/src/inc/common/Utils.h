/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_UTILS_H__
#define __AGENT_UTILS_H__

#include "common/Types.h"
#include "common/Defines.h"
#include "common/Thread.h"
#include "common/Log.h"
#include <vector>
#ifndef WIN32
#include <dlfcn.h>
#else
#endif

#define SCRIPT_PACKLOG_WIN "packlog.bat"
#define AGENT_RUNNING_USER "rdadmin"
#define WINDOWS_USERNAME_LEN 512
#define LINUX_USERNAME_LEN 1024

#define HOST_OS_UNKNOWN         0
#define HOST_OS_WINDOWS         1
#define HOST_OS_REDHAT          2
#define HOST_OS_HP_UX           3
#define HOST_OS_SOLARIS         4
#define HOST_OS_AIX             5
#define HOST_OS_SUSE            6
#define HOST_OS_ORACLE_LINUX    7
#define HOST_OS_OTHER_LINUX     8

//不要直接使用signal注册信号
typedef mp_void (*signal_proc)(mp_int32);
AGENT_API mp_int32 SignalRegister(mp_int32 signo, signal_proc func);

AGENT_API mp_void DoSleep(mp_uint32 ms);
AGENT_API mp_bool CheckCmdDelimiter(mp_string& str);
AGENT_API mp_int32 GetOSError();
AGENT_API mp_char* GetOSStrErr(mp_int32 err, mp_char* buf, mp_size buf_len);
AGENT_API mp_int32 InitCommonModules(mp_char* pszFullBinPath);
AGENT_API mp_int32 GetHostName(mp_string& strHostName);

//动态库操作相关
#define DFLG_LOCAL   (RTLD_NOW | RTLD_LOCAL)
#define DFLG_GLOBAL  (RTLD_NOW | RTLD_GLOBAL)
AGENT_API mp_handle_t DlibOpen(const mp_char* pszLibName);
AGENT_API mp_handle_t DlibOpenEx(const mp_char* pszLibName, mp_bool bLocal);
AGENT_API mp_void DlibClose(mp_handle_t hDlib);
AGENT_API mp_void* DlibDlsym(mp_handle_t hDlib, const char *pszFname);
AGENT_API const mp_char* DlibError(mp_char* szMsg, mp_uint32 isz);
AGENT_API mp_int32 PackageLog(mp_string strLogName);
AGENT_API mp_int32 GetCurrentUserName(mp_string &strUserName, mp_ulong &iErrCode);
AGENT_API const mp_char* BaseFileName(const mp_char* pszFileName);
AGENT_API mp_void RemoveFullPathForLog(mp_string strCmd, mp_string &strLogCmd);

#ifdef WIN32
AGENT_API mp_int32 GetCurrentUserNameW(mp_wstring &strUserName, mp_ulong &iErrCode);
AGENT_API const mp_wchar* BaseFileNameW(const mp_wchar* pszFileName);
#endif

#ifndef WIN32
AGENT_API mp_int32 GetUidByUserName(mp_string strUserName, mp_int32 &uid, mp_int32 &gid);
AGENT_API mp_int32 ChownFile(mp_string strFileName, mp_int32 uid, mp_int32 gid);
#endif

//以下临时使用固定函数实现，后续开源软件选型后采用开源软件优化
AGENT_API mp_int32 CheckParamString(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, 
    mp_string &strExclude);
AGENT_API mp_int32 CheckParamString(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strPre);
AGENT_API mp_int32 CheckParamStringEnd(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strEnd);
AGENT_API mp_int32 CheckParamInteger32(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude);
AGENT_API mp_int32 CheckParamInteger64(mp_int64 paramValue, mp_int64 begValue, mp_int64 endValue, vector<mp_int64> &vecExclude);
AGENT_API mp_int32 CheckParamStringIsIP(mp_string &paramValue);
AGENT_API mp_int32 CheckPathString(mp_string &pathValue);
AGENT_API mp_int32 CheckPathString(mp_string &pathValue, mp_string strPre);
AGENT_API mp_int32 CheckFileSysMountParam(mp_string strDeviceName, mp_int32 volumeType, mp_string strMountPoint);
AGENT_API mp_int32 CheckFileSysFreezeParam(mp_string strDiskNames);
//获取OS类型
AGENT_API mp_void GetOSType(mp_int32 &iOSType);
//获取OS版本信息
AGENT_API mp_int32 GetOSVersion(mp_int32 iOSType, mp_string &strOSVersion);

#define NEW_CATCH(pObj, CobjClassName) try\
{ \
    pObj = new CobjClassName; \
} \
catch (...) \
{ \
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New %s failed", #CobjClassName); \
    pObj = NULL; \
} \

#define NEW_CATCH_RETURN_FAILED(pObj, CobjClassName) try\
{ \
    pObj = new CobjClassName; \
} \
catch (...) \
{ \
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New %s failed", #CobjClassName); \
    pObj = NULL; \
} \
if (!pObj) \
{\
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "pObj is NULL."); \
    return MP_FAILED; \
}\


#define NEW_ARRAY_CATCH(pObj, CobjClassName, iNum) try\
{ \
    pObj = new CobjClassName[iNum]; \
} \
catch (...) \
{ \
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New %s failed", #CobjClassName); \
    pObj = NULL; \
} \

#define NEW_ARRAY_CATCH_RETURN_FAILED(pObj, CobjClassName, iNum) try\
{ \
    pObj = new CobjClassName[iNum]; \
} \
catch (...) \
{ \
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New %s failed", #CobjClassName); \
    pObj = NULL; \
} \
if (!pObj) \
{\
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "pObj is NULL."); \
    return MP_FAILED; \
}\


#endif //__AGENT_UTILS_H__

