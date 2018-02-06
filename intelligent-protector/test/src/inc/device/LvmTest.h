/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _LVMTEST_H_
#define _LVMTEST_H_

#define private public

#include "device/Lvm.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "array/Array.h"

#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLvmTestLogVoid(mp_void* pthis);

class CLvmTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLvmTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CLvmTest::m_stub;

//******************************************************************************
typedef mp_int32 (*CLvmGetDevNameByWWNType)(mp_string& strDevName, mp_string& strWWN);
typedef mp_int32 (*StubCLvmGetDevNameByWWNType)(mp_string& strDevName, mp_string& strWWN);

typedef mp_int32 (CLvm::*GetVgName_LLVMType)(mp_string& strDevice, vector<mp_string> &vecVgName);
typedef mp_int32 (*StubGetVgName_LLVMType)(mp_string& strDevice, vector<mp_string> &vecVgName);

typedef mp_int32 (CLvm::*IsVgExportedType)(mp_string& strVgName, mp_bool& bIsExported);
typedef mp_int32 (*StubIsVgExportedType)(mp_void* pthis,mp_string& strVgName, mp_bool& bIsExported);

typedef mp_bool (CLvm::*WriteVgMapInfoType)(mp_string& strMapInfo, mp_string& strMapInfoFile);
typedef mp_bool (*StubWriteVgMapInfoType)(mp_string& strMapInfo, mp_string& strMapInfoFile);

typedef mp_int32 (*GetSecPvNameType)(mp_string& strPriPvName, mp_string& strLegacyDisk, mp_string& strSecPvName);
typedef mp_int32 (*StubGetSecPvNameType)(mp_string& strPriPvName, mp_string& strLegacyDisk, mp_string& strSecPvName);

typedef mp_int32 (CLvm::*GetVgName_HLVMType)(mp_string& strDevice, mp_string& strVgName);
typedef mp_int32 (*StubGetVgName_HLVMType)(mp_string& strDevice, mp_string& strVgName);

typedef mp_int32 (*CLvmSysExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCLvmSysExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
//******************************************************************************
mp_int32 StubCLvmSysExec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return 0;
}

mp_int32 StubCLvmSysExect(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    (*pvecResult).push_back("test");
    return 0;
}

mp_int32 StubGetVgName_HLVMt(mp_string& strDevice, mp_string& strVgName){
    strVgName = "test";
    return 0;
}

mp_int32 StubGetVgName_HLVM(mp_string& strDevice, mp_string& strVgName){
    strVgName = "";
    return 0;
}

mp_int32 StubGetSecPvName(mp_string& strPriPvName, mp_string& strLegacyDisk, mp_string& strSecPvName){
    return 0;
}

mp_bool StubWriteVgMapInfo(mp_string& strMapInfo, mp_string& strMapInfoFile){
    return 0;
}

mp_int32 StubIsVgExported0(mp_void* pthis,mp_string& strVgName, mp_bool& bIsExported){
    strVgName = "test";
    return 0;
}

mp_int32 StubIsVgExported(mp_void* pthis,mp_string& strVgName, mp_bool& bIsExported){
    bIsExported = MP_TRUE;
    return 0;
}

mp_int32 StubGetVgName_LLVM(mp_string& strDevice, vector<mp_string> &vecVgName){
    return 0;
}

mp_int32 StubCLvmGetDevNameByWWN(mp_string& strDevName, mp_string& strWWN){
    return 0;
}

mp_void StubCLvmTestLogVoid(mp_void* pthis){
    return;
}

#endif
