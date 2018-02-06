/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _SYSTEMEXECTEST_H_
#define _SYSTEMEXECTEST_H_

#define private public

#ifndef WIN32
#include <signal.h>
#include <libgen.h>
#endif
#include <sstream>
#include "securec.h"
#include "common/SystemExec.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/CryptAlg.h"
#include "common/UniqueId.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/Sign.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CSystemExecTest: public testing::Test{
protected:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CSystemExecTest::m_stub;

//*******************************************************************************
typedef mp_bool (*CSystemExecTestFileExistType)(const mp_char* pszFilePath);
typedef mp_bool (*CSystemExecTestStubFileExistType)(const mp_char* pszFilePath);

typedef mp_bool (*CSystemExecTestCheckCmdDelimiterType)(mp_string& str);
typedef mp_bool (*CSystemExecTestStubCheckCmdDelimiterType)(mp_string& str);

typedef mp_int32 (*CSystemExecExecSystemWithoutEchoType)(mp_string& strCommand, mp_bool bNeedRedirect);
typedef mp_int32 (*CSystemExecStubExecSystemWithoutEchoType)(mp_string& strCommand, mp_bool bNeedRedirect);
//*******************************************************************************
mp_bool CSystemExecTestStubFileExist(const mp_char* pszFilePath){
    return 1;
}

mp_bool CSystemExecTestStubCheckCmdDelimiter(mp_string& str){
    return 0;
}

mp_int32 CSystemExecStubExecSystemWithoutEcho(mp_string& strCommand, mp_bool bNeedRedirect){
    return 0;
}

#endif
