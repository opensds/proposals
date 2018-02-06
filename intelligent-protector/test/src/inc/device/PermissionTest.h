/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __PERMISSIONTEST_H__
#define __PERMISSIONTEST_H__

#define private public

#ifndef WIN32
#include "device/Permission.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "array/Array.h"

#include <pwd.h>
#include <grp.h>
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCPermissionTestLogVoid(mp_void* pthis);

class CPermissionTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCPermissionTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CPermissionTest::m_stub;

//******************************************************************************
typedef mp_int32 (CPermission::*ChmodType)(permission_info_t& permissionInfo);
typedef mp_int32 (*StubChmodType)(mp_void* pthis,permission_info_t& permissionInfo);
//******************************************************************************
mp_int32 StubChmod(mp_void* pthis,permission_info_t& permissionInfo){
    permissionInfo.strUserName = "test";
    
    return 0;
}

mp_void StubCPermissionTestLogVoid(mp_void* pthis){
    return;
}

#endif
#endif
