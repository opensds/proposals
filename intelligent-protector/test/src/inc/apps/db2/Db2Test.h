/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __DB2TEST_H__
#define __DB2TEST_H__

#define private public

#include "apps/db2/Db2.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/Defines.h"
#include "common/RootCaller.h"
#include "array/Array.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CDB2Test : public testing::Test
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
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CDB2Test::m_stub;

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

typedef mp_int32 (*CArrayGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);
typedef mp_int32 (*StubCArrayGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);

typedef mp_int32 (*CArrayGetArraySNType)(mp_string& strDev, mp_string& strSN);
typedef mp_int32 (*StubCArrayGetArraySNType)(mp_string& strDev, mp_string& strSN);

typedef mp_int32 (*CArrayGetLunInfoType)(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);
typedef mp_int32 (*StubCArrayGetLunInfoType)(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);

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
    if (pvecResult)
    {
        pvecResult->push_back("0");
    }
    return 0;
}
mp_int32 StubCRootCallerExecDbinfo(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    pvecResult->push_back("db2inst1:db_sss:v9.7:1");
    return 0;
}
mp_int32 StubCRootCallerExecDbLuninfo(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    pvecResult->push_back("strvolName:strvgName:strdeviceName:strdiskName:strvolType:1");
    return 0;
}
mp_int32 StubCArrayGetArrayVendorAndProductOk(mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    strvendor = ARRAY_VENDER_HUAWEI;
    strproduct = ARRAY_VENDER_HUAWEI;
    return 0;
}
mp_int32 StubCArrayGetArraySNEq0(mp_string& strDev, mp_string& strSN)
{
    return 0;
}
mp_int32 StubCArrayGetLunInfoEq0(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    return 0;
}
#endif
