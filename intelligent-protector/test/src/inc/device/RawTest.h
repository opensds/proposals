/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _RAWTEST_H_
#define _RAWTEST_H_

#define private public

#ifndef WIN32
#include "device/Raw.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "array/Array.h"

#include <sys/stat.h>
#include <sstream>
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCRawTestLogVoid(mp_void* pthis);

class CRawTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCRawTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CRawTest::m_stub;

//******************************************************************************
typedef mp_bool (*CRawExistType)(const mp_char* pszFilePath);
typedef mp_bool (*StubCRawExistType)(const mp_char* pszFilePath);

typedef mp_int32 (*CRawExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCRawExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);

typedef mp_int32 (CRaw::*GetDeviceUsedByRawType)(mp_string& strRawDevPath, mp_string& strUsedDevName);
typedef mp_int32 (*StubGetDeviceUsedByRawType)(mp_void* pthis,mp_string& strRawDevPath, mp_string& strUsedDevName);

typedef mp_int32 (CRaw::*GetBoundedDevVersionsType)(mp_string& strRawDevPath, mp_int32& iBoundMajor, mp_int32& iBoundMinor);
typedef mp_int32 (*StubGetBoundedDevVersionsType)(mp_string& strRawDevPath, mp_int32& iBoundMajor, mp_int32& iBoundMinor);

typedef mp_int32 (*GetFolderFileType)(mp_string& strFolder, vector<mp_string>& vecFileList);
typedef mp_int32 (*StubGetFolderFileType)(mp_string& strFolder, vector<mp_string>& vecFileList);

typedef mp_int32 (*GetDevNameByWWNType)(mp_string& strDevName, mp_string& strWWN);
typedef mp_int32 (*StubGetDevNameByWWNType)(mp_string& strDevName, mp_string& strWWN);

typedef mp_int32 (CRaw::*GetDeviceNumberType)(mp_string& rstrDeviceName, mp_int32& iMajor, mp_int32& iMinor);
typedef mp_int32 (*StubGetDeviceNumberType)(mp_string& rstrDeviceName, mp_int32& iMajor, mp_int32& iMinor);
//******************************************************************************
mp_int32 StubGetDeviceNumber0(mp_string& rstrDeviceName, mp_int32& iMajor, mp_int32& iMinor){
    return 0;
}

mp_int32 StubGetDeviceNumber(mp_string& rstrDeviceName, mp_int32& iMajor, mp_int32& iMinor){
    return -1;
}

mp_int32 StubGetDevNameByWWN(mp_string& strDevName, mp_string& strWWN){
    return 0;
}

mp_int32 StubGetFolderFilet(mp_string& strFolder, vector<mp_string>& vecFileList){
    vecFileList.push_back("test");
    return 0;
}

mp_int32 StubGetFolderFile(mp_string& strFolder, vector<mp_string>& vecFileList){
    return -1;
}

mp_int32 StubGetFolderFile0(mp_string& strFolder, vector<mp_string>& vecFileList){
    return 0;
}

mp_int32 StubGetBoundedDevVersions(mp_string& strRawDevPath, mp_int32& iBoundMajor, mp_int32& iBoundMinor){
    return 0;
}

mp_int32 StubGetDeviceUsedByRaw(mp_void* pthis,mp_string& strRawDevPath, mp_string& strUsedDevName){
    return 0;
}

mp_int32 StubGetDeviceUsedByRawt(mp_void* pthis,mp_string& strRawDevPath, mp_string& strUsedDevName){
    strUsedDevName = "test";
    return 0;
}

mp_int32 StubGetDeviceUsedByRaw0(mp_void* pthis,mp_string& strRawDevPath, mp_string& strUsedDevName){
    strUsedDevName = "goal";
    return 0;
}

mp_int32 StubCRawExec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return -1;
}

mp_int32 StubCRawExecl(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    (*pvecResult).push_back("l");
    return 0;
}

mp_int32 StubCRawExec0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return 0;
}

mp_bool StubCRawExist(const mp_char* pszFilePath){
    return 1;
}

mp_bool StubCRawExist0(const mp_char* pszFilePath){
    return 0;
}

mp_void StubCRawTestLogVoid(mp_void* pthis){
    return;
}

#endif
#endif
