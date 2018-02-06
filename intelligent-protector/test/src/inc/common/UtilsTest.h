/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _UTILSTEST_H_
#define _UTILSTEST_H_

#ifndef WIN32
#include <signal.h>
#include <libgen.h>
#endif
#include <sstream>
#include "common/Utils.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/UniqueId.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/RootCaller.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubUtilsVoid(mp_void* pthis);

class UtilsTest: public testing::Test{
protected:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubUtilsVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* UtilsTest::m_stub;

//*******************************************************************************
typedef mp_int32 (CPath::*CPathInitType)(mp_char* pszFullBinPath);
typedef mp_int32 (*StubCPathInitType)(mp_char* pszFullBinPath);

typedef mp_int32 (CConfigXmlParser::*CConfigXmlParserInitType)(mp_string strInput);
typedef mp_int32 (*StubCConfigXmlParserInitType)(mp_string strInput);
//*******************************************************************************
mp_void StubUtilsVoid(mp_void* pthis){
    return;
}

mp_int32 StubCPathInit(mp_char* pszFullBinPath){
    return 0;
}

mp_int32 StubCConfigXmlParserInit(mp_string strInput){
    return 0;
}

#endif
