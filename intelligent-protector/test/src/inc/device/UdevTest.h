/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __UDEVTEST_H__
#define __UDEVTEST_H__

#define private public

#include "device/Udev.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/String.h"
#include "common/RootCaller.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubUdevTestLogVoid(mp_void* pthis);

class UdevTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubUdevTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* UdevTest::m_stub;

//******************************************************************************
typedef mp_int32 (*UdevTestExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubUdevTestExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
//******************************************************************************
mp_int32 StubUdevTestExec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return -1;
}

mp_int32 StubUdevTestExecl(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    (*pvecResult).push_back("l");
    return 0;
}

mp_int32 StubUdevTestExec0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return 0;
}

mp_void StubUdevTestLogVoid(mp_void* pthis){
    return;
}

#endif
