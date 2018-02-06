/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __FILESYSTEST_H__
#define __FILESYSTEST_H__

#define private public

#include "device/FileSys.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/File.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "array/Array.h"
#ifdef LIN_FRE_SUPP
#include "alarm/AppFreezeStatus.h"
#endif
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCFileSysTestLogVoid(mp_void* pthis);

class CFileSysTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCFileSysTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CFileSysTest::m_stub;

//******************************************************************************
typedef mp_int32 (*CFileSysExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCFileSysExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);

typedef mp_int32 (*ExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);
typedef mp_int32 (*StubExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);

typedef mp_int32 (*Exec0Type)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubExec0Type)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (CFileSys::*CheckMountedStatusType)(mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType, mp_bool& bIsMounted);
typedef mp_int32 (*StubCheckMountedStatusType)(void* pthis,mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType, mp_bool& bIsMounted);
  
typedef mp_bool (*CFileFileExistType)(const mp_char* pszFilePath);
typedef mp_bool (*StubCFileExistExecType)(const mp_char* pszFilePath);

typedef mp_int32 (*GetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);
typedef mp_int32 (*StubGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);


typedef mp_int32 (CFileSys::*CheckDevStatusType)(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted);
typedef mp_int32 (*StubCheckDevStatusType)(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted);

typedef mp_int32 (CFileSys::*CheckUdevDevStatusType)(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted);
typedef mp_int32 (*StubCheckUdevDevStatusType)(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted);

typedef mp_int32 (CFileSys::*GetAllMountInfoType)(vector<linux_mount_info_t> &vecMountInfo);
typedef mp_int32 (*StubGetAllMountInfoType)(vector<linux_mount_info_t> &vecMountInfo);

//******************************************************************************
mp_int32 StubGetArrayVendorAndProduct(mp_string& strDev, mp_string& strvendor, mp_string& strproduct){
    return -1;
}

mp_int32 StubGetArrayVendorAndProductl(mp_string& strDev, mp_string& strvendor, mp_string& strproduct){
    //strVendor = "test";
    return 0;
}

mp_int32 StubGetArrayVendorAndProduct0(mp_string& strDev, mp_string& strvendor, mp_string& strproduct){
    //strVendor = ARRAY_VENDER_HUAWEI;
    return 0;
}

mp_void StubCFileSysTestLogVoid(mp_void* pthis){
    return;
}

mp_int32 StubCFileSysExec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return -1;
}

mp_int32 StubCFileSysExecl(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    (*pvecResult).push_back("l");
    return 0;
}

mp_int32 StubCFileSysExec0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return 0;
}

mp_int32 StubExecSystemWithEchol(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect){
    return 0;
}

mp_int32 StubExecSystemWithEcho0(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect){
    strEcho.push_back("test");
    return 0;
}

mp_int32 StubExec0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return 0;
}
mp_int32 StubExecSystemWithEcho(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect){
    strEcho.push_back("test");
    return -1;
}

mp_int32 StubCheckMountedStatus0(void* pthis,mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType, mp_bool& bIsMounted){
    bIsMounted = MP_TRUE;
    return 0;
}

mp_int32 StubCheckMountedStatus1(void* pthis, mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType, mp_bool& bIsMounted){
    bIsMounted = MP_FALSE;
    return 0;
}

mp_int32 StubCheckMountedStatus(void* pthis, mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType, mp_bool& bIsMounted){
    return -1;
}

mp_bool StubCFileExistExec(const mp_char* pszFilePath){
    return 1;
}

mp_bool StubCFileExistExec0(const mp_char* pszFilePath){
    return 0;
}


mp_int32 StubCheckDevStatus(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted){
	return -1;
}

mp_int32 StubCheckUdevDevStatus0(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted){
	return 0;
}

mp_int32 StubCheckUdevDevStatus(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted){
	return -1;
}

mp_int32 StubCheckDevStatus0(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted){
	return 0;
}


mp_int32 StubGetAllMountInfo(vector<linux_mount_info_t> &vecMountInfo){
	return -1;
}

mp_int32 StubGetAllMountInfo0(vector<linux_mount_info_t> &vecMountInfo){
	return 0;
}


#endif
