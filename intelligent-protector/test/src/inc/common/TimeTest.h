/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _TIMETEST_H_
#define _TIMETEST_H_

#include "common/Time.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCMpTimeLogVoid(mp_void* pthis);

class CMpTimeTest: public testing::Test{
protected:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCMpTimeLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CMpTimeTest::m_stub;

//******************************************************************************
typedef mp_tm* (*LocalTimeRType)(mp_time* pTime, mp_tm* pTm);
typedef mp_tm* (*StubLocalTimeRType)(mp_time* pTime, mp_tm* pTm);
//******************************************************************************
mp_tm* StubCMpTimeLocalTimeR(mp_time* pTime, mp_tm* pTm){
    pTm = NULL;
    return NULL;
}

mp_void StubCMpTimeLogVoid(mp_void* pthis){
    return;
}
#endif
