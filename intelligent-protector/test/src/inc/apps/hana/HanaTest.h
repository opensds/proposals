/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __HANATEST_H__
#define __HANATEST_H__

#define private public

#include "apps/hana/Hana.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "array/Array.h"
#include <sstream>
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CHanaTest : public testing::Test
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
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CHanaTest::m_stub;

//定义函数类型
/*类成员函数的类型命名：Class名+函数名+Type后缀
 *类成员函数的Stub函数的类型命名：Stub前缀+Class名+函数名+Type后缀
 *类成员函数的Stub函数参数：是void* pthis + 原函数参数，这么写是因为，可能会改原函数传入的输出参数。静态函数与原函数参数一致。
 *静态函数、全局函数、库函数的类型命名：函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的类型命名：Stub前缀+函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的参数：与原函数参数一致，这么写是因为，可能会改原函数传入的输出参数。
*/

#endif
