/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __CLUSTERTEST_H__
#define __CLUSTERTEST_H__

#define private public

#include "cluster/Cluster.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "common/String.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CClusterTest : public testing::Test
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
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CClusterTest::m_stub;

//定义函数类型
/*类成员函数的类型命名：Class名+函数名+Type后缀
 *类成员函数的Stub函数的类型命名：Stub前缀+Class名+函数名+Type后缀
 *类成员函数的Stub函数参数：是void* pthis + 原函数参数，这么写是因为，可能会改原函数传入的输出参数。静态函数与原函数参数一致。
 *静态函数、全局函数、库函数的类型命名：函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的类型命名：Stub前缀+函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的参数：与原函数参数一致，这么写是因为，可能会改原函数传入的输出参数。
*/

typedef mp_int32 (*CRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);

typedef mp_int32 (*CSystemExecExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);
typedef mp_int32 (*StubCSystemExecExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);

typedef mp_void (*DoSleepType)(mp_uint32 ms);
typedef mp_void (*StubDoSleepType)(mp_uint32 ms);

/* Stub 函数的取名规则：Stub+(Class名+)原函数名+需要改的结果说明(+特殊用处)
 * 比如：StubopenEq0。是用来取代open函数的，返回值为0。
 * Lt：小于    Eq：等于  Ok：有返回值和输出
 * 参数参照类型命名处说明
*/
mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}
mp_int32 StubCRootCallerExecEq0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    return 0;
}
mp_int32 StubCRootCallerExecActiveNode(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("test");
    }
    return 0;
}
mp_int32 StubCRootCallerExecClusterInfo(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("strClusterName:strResGrpName:strVgActiveMode");
    }
    return 0;
}
mp_int32 StubCRootCallerExecStart(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        switch (iCommandID)
        {
            case ROOT_COMMAND_HASYS:
                pvecResult->push_back("RUNNING");
            break;
            case ROOT_COMMAND_LSSRC:
            case ROOT_COMMAND_CLLSNODE:
                pvecResult->push_back("ST_STABLE");
            break;
            case ROOT_COMMAND_CMVIEWCL:
                pvecResult->push_back("running");
            break;
            case ROOT_COMMAND_RHCS_CLUSTAT:
                pvecResult->push_back("Online,Local,rgmanager");
            break;
            default:
            break;
        }
    }
    return 0;
}

mp_int32 StubCRootCallerExecStartErr(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("ErrStatus");
    }
    return 0;
}
mp_int32 StubCSystemExecExecSystemWithEchoEq0(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    strEcho.push_back("locking_type = 3"); 
    return 0;
}
mp_void StubDoSleepVoid(mp_uint32 ms)
{
    return;
}
#endif
