/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __HOSTTEST_H__
#define __HOSTTEST_H__

#define private public

#include "host/Host.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Uuid.h"
#include "common/SystemExec.h"
#include "common/Defines.h"
#include "common/RootCaller.h"
#include "common/Path.h"
#include "array/Array.h"
#include "common/Ip.h"
#include "common/Sign.h"
#include "common/Utils.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCHostTestLogVoid(mp_void* pthis);

class CHostTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCHostTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CHostTest::m_stub;

//******************************************************************************
typedef mp_int32 (*CHostTestExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCHostTestExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);

typedef mp_int32 (*CHostTestReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
typedef mp_int32 (*StubCHostTestReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);

typedef mp_int32 (*CHostGetAllDiskNameType)(vector<mp_string>& vecDiskName);
typedef mp_int32 (*StubCHostGetAllDiskNameType)(vector<mp_string>& vecDiskName);

typedef mp_int32 (*CHostGetLunInfoType)(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);
typedef mp_int32 (*StubCHostGetLunInfoType)(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);

typedef mp_int32 (*CHostGetArraySNType)(mp_string& strDev, mp_string& strSN);
typedef mp_int32 (*StubCHostGetArraySNType)(mp_string& strDev, mp_string& strSN);

typedef mp_int32 (*CHostGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);
typedef mp_int32 (*StubCHostGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);

typedef mp_void (*GetSnmpV3ParamType)(snmp_v3_param& stSnmpV3Param);
typedef mp_void (*StubGetSnmpV3ParamType)(snmp_v3_param& stSnmpV3Param);

typedef mp_int32 (*CHostExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);
typedef mp_int32 (*StubCHostExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);

typedef mp_int32 (*PackageLogType)(mp_string strLogName);
typedef mp_int32 (*StubPackageLogType)(mp_string strLogName);

typedef mp_string (CLogCollector::*GetLogNameType)();
typedef mp_string (*StubGetLogNameType)();

typedef mp_int32 (*WriteFileType)(mp_string& strFilePath, vector<mp_string>& vecInput);
typedef mp_int32 (*StubWriteFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
//******************************************************************************
mp_int32 StubWriteFile(mp_string& strFilePath, vector<mp_string>& vecOutput){
    return MP_SUCCESS;
}

mp_int32 StubCHostExecSystemWithEcho(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect){
    strEcho.push_back("test");
    strEcho.push_back("test");
    return 0;
}

mp_string StubGetLogName(){
    return "test";
}

mp_int32 StubPackageLog(mp_string strLogName){
    return -1;
}

mp_void StubGetSnmpV3Param0(snmp_v3_param& stSnmpV3Param){
    stSnmpV3Param.strAuthPassword = "test1";
    stSnmpV3Param.strPrivPassword = "test";
    stSnmpV3Param.strSecurityName = "test";
    stSnmpV3Param.iAuthProtocol = 1;
    stSnmpV3Param.iPrivProtocol = 1;
    
    return;
}

mp_void StubGetSnmpV3Param(snmp_v3_param& stSnmpV3Param){
    stSnmpV3Param.strAuthPassword = "test";
    stSnmpV3Param.strPrivPassword = "test";
    stSnmpV3Param.strSecurityName = "test";
    stSnmpV3Param.iAuthProtocol = 1;
    stSnmpV3Param.iPrivProtocol = 1;
    
    return;
}

mp_int32 StubCHostGetLunInfo(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID){
    return 0;
}

mp_int32 StubCHostGetArraySN(mp_string& strDev, mp_string& strSN){
    return 0;
}

mp_int32 StubCHostGetArrayVendorAndProduct(mp_string& strDev, mp_string& strvendor, mp_string& strproduct){
    return 0;
}

mp_int32 StubCHostGetArrayVendorAndProductt(mp_string& strDev, mp_string& strvendor, mp_string& strproduct){
    strvendor = "HUAWEI";
    return 0;
}

mp_int32 StubCHostGetAllDiskName(vector<mp_string>& vecDiskName){
    vecDiskName.push_back("");
    return 0;
}

mp_int32 StubCHostGetAllDiskNamet(vector<mp_string>& vecDiskName){
    vecDiskName.push_back("test");
    return 0;
}

mp_int32 StubCHostTestReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput){
    //vecOutput.push_back("");
    return 0;
}

mp_int32 StubCHostTestReadFilet(mp_string& strFilePath, vector<mp_string>& vecOutput){
    vecOutput.push_back("test");
    return 0;
}

mp_int32 StubCHostTestExecl(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    (*pvecResult).push_back("l");
    return 0;
}

mp_int32 StubCHostTestExec0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult){
    return 0;
}

mp_void StubCHostTestLogVoid(mp_void* pthis){
    return;
}

#endif
