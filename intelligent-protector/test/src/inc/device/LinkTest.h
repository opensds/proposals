/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __LINKTEST_H__
#define __LINKTEST_H__

#define private public

#include "device/Link.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "array/Array.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLinkTestLogVoid(mp_void* pthis);

class CLinkTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLinkTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CLinkTest::m_stub;

//******************************************************************************
typedef mp_bool (*CLinkTestCFileFileType)(const mp_char* pszFilePath);
typedef mp_bool (*StubCLinkTestCFileExistType)(const mp_char* pszFilePath);

typedef mp_bool (*CLinkTestDirExistType)(const mp_char* pszFilePath);
typedef mp_bool (*StubCLinkTestDirExistType)(const mp_char* pszFilePath);

typedef mp_int32 (CLink::*GetDeviceUsedByLinkType)(mp_string & strLinkFileName, mp_string & strUsedDeviceName);
typedef mp_int32 (*StubGetDeviceUsedByLinkType)(mp_void* pthis,mp_string & strLinkFileName, mp_string & strUsedDeviceName);
//******************************************************************************
mp_int32 StubGetDeviceUsedByLink(mp_void* pthis,mp_string & strLinkFileName, mp_string & strUsedDeviceName){
    strLinkFileName = "test";
    strUsedDeviceName = "test";
    
    return 0;
}

mp_int32 StubGetDeviceUsedByLink0(mp_void* pthis,mp_string & strLinkFileName, mp_string & strUsedDeviceName){
    strLinkFileName = "test";
    
    return 0;
}

mp_int32 StubGetDeviceUsedByLink1(mp_void* pthis,mp_string & strLinkFileName, mp_string & strUsedDeviceName){
    strLinkFileName = "test";
    
    return -1;
}

mp_bool StubCLinkTestCFileExist(const mp_char* pszFilePath){
    return 1;
}

mp_bool StubCLinkTestCFileExist0(const mp_char* pszFilePath){
    return 0;
}

mp_bool StubCLinkTestDirExist(const mp_char* pszFilePath){
    return 1;
}

mp_bool StubCLinkTestDirExist0(const mp_char* pszFilePath){
    return 0;
}

mp_void StubCLinkTestLogVoid(mp_void* pthis){
    return;
}

#endif
