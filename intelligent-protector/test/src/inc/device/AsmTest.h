/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __ASMTEST_H__
#define __ASMTEST_H__

#include "device/Asm.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCAsmTestLogVoid(mp_void* pthis);

class CAsmTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCAsmTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CAsmTest::m_stub;

//******************************************************************************
typedef mp_int32 (*ExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
//******************************************************************************
mp_void StubCAsmTestLogVoid(mp_void* pthis){
    return;
}

mp_int32 StubExec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return 0;
}

#endif

