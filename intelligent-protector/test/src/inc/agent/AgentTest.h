/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENTTEST_H__
#define __AGENTTEST_H__

#define private public

#include "agent/Agent.h"
#include "agent/Authentication.h"
#include "agent/Communication.h"
#include "agent/FTExceptionHandle.h"
#include "agent/TaskDispatchWorker.h"
#include "agent/TaskPool.h"
#include "agent/TaskProtectWorker.h"
#include "agent/TaskVssWorker.h"
#include "agent/TaskWorker.h"
#include "agent/TaskPool.h"
#include "agent/Communication.h"
#include "agent/Authentication.h"
#include "agent/FTExceptionHandle.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/UniqueId.h"
#include "common/String.h"
#include "common/ConfigXmlParse.h"
#include "common/SystemExec.h"
#include "securec.h"
#include "common/Log.h"
#include "common/Types.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "alarm/Db.h"
#include "rest/Interfaces.h"
#include "plugins/db2/Db2Plugin.h"
#include "fcgi/include/fcgios.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CAuthenticationTest : public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CAuthenticationTest::m_stub;

class CCommunicationTest : public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CCommunicationTest::m_stub;

class CFTExceptionHandleTest : public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CFTExceptionHandleTest::m_stub;

class CTaskDispatchWorkerTest : public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CTaskDispatchWorkerTest::m_stub;

class CTaskPoolTest : public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CTaskPoolTest::m_stub;

class CTaskProtectWorkerTest : public testing::Test
{
};

class CTaskWorkerTest : public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CTaskWorkerTest::m_stub;

//定义函数类型
/*类成员函数的类型命名：Class名+函数名+Type后缀
 *类成员函数的Stub函数的类型命名：Stub前缀+Class名+函数名+Type后缀
 *类成员函数的Stub函数参数：是void* pthis + 原函数参数，这么写是因为，可能会改原函数传入的输出参数。静态函数与原函数参数一致。
 *静态函数、全局函数、库函数的类型命名：函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的类型命名：Stub前缀+函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的参数：与原函数参数一致，这么写是因为，可能会改原函数传入的输出参数。
*/
typedef mp_int32 (CConfigXmlParser::*CConfigXmlParserGetValueStringType)(mp_string strSection, mp_string strKey, mp_string& strValue);
typedef mp_int32 (*StubCConfigXmlParserGetValueStringType)(mp_void* pthis, mp_string strSection, mp_string strKey, mp_string& strValue);

typedef mp_int32 (*FCGX_InitType)(mp_void);
typedef mp_int32 (*StubFCGX_InitType)(mp_void);

typedef mp_int32 (*FCGX_OpenSocketType)(const mp_char *path, mp_int32 backlog);
typedef mp_int32 (*StubFCGX_OpenSocketType)(const mp_char *path, mp_int32 backlog);

typedef mp_char* (*FCGX_GetParamType)(const mp_char *name, FCGX_ParamArray envp);
typedef mp_char* (*StubFCGX_GetParamType)(const mp_char *name, FCGX_ParamArray envp);

typedef mp_int32 (*OS_CloseType)(mp_int32 fd, mp_int32 shutdown);
typedef mp_int32 (*StubOS_CloseType)(mp_int32 fd, mp_int32 shutdown);

typedef mp_int32 (*fcntlType)(mp_int32 fd, mp_int32 cmd, ...); 
typedef mp_int32 (*StubfcntlType)(mp_int32 fd, mp_int32 cmd); 

typedef mp_int32 (CCommunication::*CCommunicationInitRequestType)(mp_int32 handler);
typedef mp_int32 (*StubCCommunicationInitRequestType)(mp_void* pthis, mp_int32 handler);

typedef mp_int32 (*CMpThreadCreateType)(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize);
typedef mp_int32 (*StubCMpThreadCreateType)(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize);

typedef mp_int32 (CAuthentication::*CAuthenticationInitType)();
typedef mp_int32 (*StubCAuthenticationInitType)(mp_void* pthis);

typedef mp_int32 (CRequestMsg::*CRequestMsgParseType)();
typedef mp_int32 (*StubCRequestMsgParseType)(mp_void* pthis);

typedef mp_int32 (CResponseMsg::*CResponseMsgSendType)();
typedef mp_int32 (*StubCResponseMsgSendType)(mp_void* pthis);

typedef mp_int32 (CDB::*CDBQueryTableType)(mp_string strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount);
typedef mp_int32 (*StubCDBQueryTableType)(mp_void* pthis, mp_string strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount);

typedef mp_int32 (CDB::*CDBExecSqlType)(mp_string strSql, DbParamStream &dpl);
typedef mp_int32 (*StubCDBExecSqlType)(mp_void* pthis, mp_string strSql, DbParamStream &dpl);

typedef mp_uint64 (*CMpTimeGetTimeSecType)();
typedef mp_uint64 (*StubCMpTimeGetTimeSecType)();

typedef mp_int32 (*CMpThreadWaitForEndType)(thread_id_t* id, mp_void** retValue);
typedef mp_int32 (*StubCMpThreadWaitForEndType)(thread_id_t* id, mp_void** retValue);

typedef mp_int32 (CPluginCfgParse::*CPluginCfgParseInitType)(mp_char* pszFileName);
typedef mp_int32 (*StubCPluginCfgParseInitType)(mp_void* pthis, mp_char* pszFileName);

typedef mp_int32 (CPluginManager::*CPluginManagerInitializeType)(IPluginCallback* pCallback);
typedef mp_int32 (*StubCPluginManagerInitializeType)(mp_void* pthis, IPluginCallback* pCallback);

typedef mp_int32 (CTaskWorker::*CTaskWorkerInitType)(CPluginCfgParse* pPlgCfgParse, CPluginManager* pPlgMgr);
typedef mp_int32 (*StubCTaskWorkerInitType)(mp_void* pthis, CPluginCfgParse* pPlgCfgParse, CPluginManager* pPlgMgr);

typedef mp_int32 (CTaskDispatchWorker::*CTaskDispatchWorkerInitType)(CTaskWorker** pTaskWorkers, mp_int32 iCount);
typedef mp_int32 (*StubCTaskDispatchWorkerInitType)(mp_void* pthis, CTaskWorker** pTaskWorkers, mp_int32 iCount);

typedef mp_void (CTaskWorker::*CTaskWorkerExitType)();
typedef mp_void (*StubCTaskWorkerExitType)(mp_void* pthis);

typedef mp_bool (CTaskWorker::*CTaskWorkerNeedExitType)();
typedef mp_bool (*StubCTaskWorkerNeedExitType)(mp_void* pthis);

typedef mp_int32 (CPluginCfgParse::*CPluginCfgParseGetPluginByServiceType)(mp_char* pszServiceName, plugin_def_t& plgDef);
typedef mp_int32 (*StubCPluginCfgParseGetPluginByServiceType)(mp_void* pthis, mp_char* pszServiceName, plugin_def_t& plgDef);

typedef IPlugin* (CPluginManager::*CPluginManagerGetPluginType)(const mp_char* pszPlg);
typedef IPlugin* (*StubCPluginManagerGetPluginType)(mp_void* pthis, const mp_char* pszPlg);

/* Stub 函数的取名规则：Stub+(Class名+)原函数名+需要改的结果说明(+特殊用处)
 * 比如：StubopenEq0。是用来取代open函数的，返回值为0。
 * Lt：小于    Eq：等于  Ok：有返回值和输出
 * 参数参照类型命名处说明
*/
mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}
mp_int32 StubCConfigXmlParserGetValueStringLt0(mp_void* pthis, mp_string strSection, mp_string strKey, mp_string& strValue)
{
    return -1;
}
mp_int32 StubCConfigXmlParserGetValueStringLt0AuthInit(mp_void* pthis, mp_string strSection, mp_string strKey, mp_string& strValue)
{
    if (CFG_USER_NAME == strKey)
    {
        return 0;
    }
    return -1;
}
mp_int32 StubCConfigXmlParserGetValueStringLt0AuthAuth(mp_void* pthis, mp_string strSection, mp_string strKey, mp_string& strValue)
{
    if (CFG_USER_NAME == strKey)
    {
        return 0;
    }
    return -1;
}
mp_int32 StubCConfigXmlParserGetValueStringEq0AuthAuth(mp_void* pthis, mp_string strSection, mp_string strKey, mp_string& strValue)
{
    if (CFG_USER_NAME == strKey)
    {
        strValue = "admin";
    }
    else if (CFG_HASH_VALUE == strKey)
    {
        strValue = "e86f78a8a3caf0b60d8e74e5942aa6d86dc150cd3c03338aef25b7d2d7e3acc7";//Admin@123
    }
    else if (CFG_PORT == strKey)
    {
        strValue = "8091";
    }
    else
    {}
    return 0;
}
mp_int32 StubCConfigXmlParserGetValueStringEq0(mp_void* pthis, mp_string strSection, mp_string strKey, mp_string& strValue)
{
    return 0;
}
mp_int32 StubFCGX_InitLt0(mp_void)
{
    return -1;
}
mp_int32 StubFCGX_InitEq0(mp_void)
{
    return 0;
}
mp_int32 StubFCGX_OpenSocketLt0(const mp_char *path, mp_int32 backlog)
{
    return -1;
}
mp_int32 StubFCGX_OpenSocketEq0(const mp_char *path, mp_int32 backlog)
{
    return 0;
}
mp_char* StubFCGX_GetParamNULL(const mp_char *name, FCGX_ParamArray envp)
{
    return NULL;
}
mp_char* StubFCGX_GetParamOk(const mp_char *name, FCGX_ParamArray envp)
{
    if (strcmp(name, HTTPPARAM_DBUSERNAME) == 0)
    {
        static mp_char name[10] = "";
        return name;
    }
    if (strcmp(name, HTTPPARAM_DBPASSWORD) == 0)
    {
        static mp_char pdword[10] = "huawei";
        return pdword;
    }
    return NULL;
}
mp_int32 StubOS_CloseEq0(mp_int32 fd, mp_int32 shutdown)
{
    return 0;
}
mp_int32 StubfcntlLt0(mp_int32 fd, mp_int32 cmd)
{
    return -1;
}
mp_int32 StubfcntlEq0(mp_int32 fd, mp_int32 cmd)
{
    return 0;
}
mp_int32 StubCCommunicationInitRequestLt0(mp_void* pthis, mp_int32 handler)
{
    return -1;
}
mp_int32 StubCMpThreadCreateLt0(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return -1;
}
mp_int32 StubCMpThreadCreateEq0(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return 0;
}
mp_int32 StubCAuthenticationInitLt0(mp_void* pthis)
{
    return -1;
}
mp_int32 StubCRequestMsgParseEq0(mp_void* pthis)
{
    return 0;
}
mp_int32 StubCRequestMsgParseLt0(mp_void* pthis)
{
    return -1;
}
mp_int32 StubCResponseMsgSendEq0(mp_void* pthis)
{
    return 0;
}
mp_int32 StubCDBQueryTableLt0(mp_void* pthis, mp_string strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    return -1;
}
mp_int32 StubCDBQueryTableEq0(mp_void* pthis, mp_string strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    return 0;
}
mp_int32 StubCDBQueryTableOk(mp_void* pthis, mp_string strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    iRowCount = 1;
    mp_string str = "1";
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    return 0;
}
mp_int32 StubCDBExecSqlLt0(mp_void* pthis, mp_string strSql, DbParamStream &dpl)
{
    return -1;
}
mp_int32 StubCDBExecSqlEq0(mp_void* pthis, mp_string strSql, DbParamStream &dpl)
{
    return 0;
}
mp_uint64 StubCMpTimeGetTimeSec6H()
{
    return 3600*6;
}
mp_uint64 StubCMpTimeGetTimeSec50s()
{
    return 50;
}
mp_uint64 StubCMpTimeGetTimeSec100s()
{
    return 100;
}
mp_int32 StubCMpThreadWaitForEndEq0(thread_id_t* id, mp_void** retValue)
{
    return 0;
}
mp_int32 StubCPluginCfgParseInitLt0(mp_void* pthis, mp_char* pszFileName)
{
    return -1;
}
mp_int32 StubCPluginCfgParseInitEq0(mp_void* pthis, mp_char* pszFileName)
{
    return 0;
}
mp_int32 StubCPluginManagerInitializeLt0(mp_void* pthis, IPluginCallback* pCallback)
{
    return -1;
}
mp_int32 StubCTaskWorkerInitLt0(mp_void* pthis, CPluginCfgParse* pPlgCfgParse, CPluginManager* pPlgMgr)
{
    return -1;
}
mp_int32 StubCTaskWorkerInitEq0(mp_void* pthis, CPluginCfgParse* pPlgCfgParse, CPluginManager* pPlgMgr)
{
    return 0;
}
mp_int32 StubCTaskDispatchWorkerInitLt0(mp_void* pthis, CTaskWorker** pTaskWorkers, mp_int32 iCount)
{
    return -1;
}
mp_void StubCTaskWorkerExitNull(mp_void* pthis)
{
    return;
}
mp_bool StubCTaskWorkerNeedExitEq1(mp_void* pthis)
{
    return 1;
}
mp_int32 StubCPluginCfgParseGetPluginByServiceEq0(mp_void* pthis, mp_char* pszServiceName, plugin_def_t& plgDef)
{
    return 0;
}
IPlugin* StubCPluginManagerGetPluginOK(mp_void* pthis, const mp_char* pszPlg)
{
    return (new CDb2Plugin);
}
#endif

