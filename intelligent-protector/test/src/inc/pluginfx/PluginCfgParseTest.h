/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __PLUGINCFG_PARSE_H__
#define __PLUGINCFG_PARSE_H__

#define private public
#define protected public

#include "pluginfx/PluginCfgParse.h"
#include "common/Log.h"
#include "securec.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"
#include "pluginfx/PluginCfgParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class CMpPluginCfgParseTest: public testing::Test{
protected:
};

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCMpPluginCfgTestLogVoid(mp_void* pthis);

class CMpPluginCfgTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCMpPluginCfgTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>*  CMpPluginCfgTest::m_stub;

mp_void StubCMpPluginCfgTestLogVoid(mp_void* pthis){
    return;
}

#endif



