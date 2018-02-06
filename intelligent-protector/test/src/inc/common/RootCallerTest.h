/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _ROOTCALLER_H_
#define _ROOTCALLER_H_

#define private public

#include "common/RootCaller.h"
#include "common/Utils.h"
#include "common/UniqueId.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/SystemExec.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CRootCallerTest: public testing::Test{
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

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CRootCallerTest::m_stub;

//*******************************************************************************
typedef mp_int32 (*WriteInputType)(mp_string& strUniqueID, mp_string& strInput);
typedef mp_int32 (*StubWriteInputType)(mp_string& strUniqueID, mp_string& strInput);

typedef mp_int32 (*ReadResultType)(mp_string& strUniqueID, vector<mp_string>& vecRlt);
typedef mp_int32 (*StubReadResultType)(mp_string& strUniqueID, vector<mp_string>& vecRlt);

typedef mp_int32 (*ExecSystemWithoutEchoType)(mp_string& strCommand, mp_bool bNeedRedirect);
typedef mp_int32 (*StubExecSystemWithoutEchoType)(mp_string& strCommand, mp_bool bNeedRedirect);
//*******************************************************************************
mp_int32 StubWriteInput(mp_string& strUniqueID, mp_string& strInput){
    return -1;
}

mp_int32 StubReadResult(mp_string& strUniqueID, vector<mp_string>& vecRlt){
    return -1;
}

mp_int32 StubExecSystemWithoutEcho(mp_string& strCommand, mp_bool bNeedRedirect){
    return 0;
}

#endif
