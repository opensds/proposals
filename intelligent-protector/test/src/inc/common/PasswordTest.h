/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _PASSWORDTEST_H_
#define _PASSWORDTEST_H_

#define private public

#include "common/Password.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Path.h"
#include <sstream>
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCPasswordCLoggerLogVoid(mp_void* pthis);

class CPasswordTest: public testing::Test{
protected:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCPasswordCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CPasswordTest::m_stub;

//*******************************************************************************
typedef mp_int32 (CConfigXmlParser::*GetValueStringType)(mp_string strSection, mp_string strKey, mp_string& strValue);
typedef mp_int32 (*StubGetValueStringType)(mp_string strSection, mp_string strKey, mp_string& strValue);

typedef mp_int32 (CConfigXmlParser::*SetValueType)(mp_string strSection, mp_string strKey, mp_string strValue);
typedef mp_int32 (*StubSetValueType)(mp_string strSection, mp_string strKey, mp_string strValue);

typedef mp_bool (*CheckNewPwdType)(PASSWOD_TYPE eType, const mp_string& strNewPwd);
typedef mp_bool (*StubCheckNewPwdType)(PASSWOD_TYPE eType, const mp_string& strNewPwd);

typedef mp_int32 (*VerifyOldUserPwdType)(mp_string& strUserName);
typedef mp_int32 (*StubVerifyOldUserPwdType)(mp_string& strUserName);

typedef mp_int32 (*InputNewUserPwdType)(mp_string& strUserName, mp_string& strNewPwd);
typedef mp_int32 (*StubInputNewUserPwdType)(mp_string& strUserName, mp_string& strNewPwd);

typedef mp_int32 (*ConfirmNewUserPwdType)(mp_string& strUserName, mp_string& strNewPwd);
typedef mp_int32 (*StubConfirmNewUserPwdType)(mp_string& strUserName, mp_string& strNewPwd);

typedef mp_int32 (*ReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
typedef mp_int32 (*StubReadFileType)(mp_void* pthis,mp_string& strFilePath, vector<mp_string>& vecOutput);

typedef mp_bool (*SaveAdminPwdType)(const mp_string& strPwd);
typedef mp_bool (*StubSaveAdminPwdType)(const mp_string& strPwd);

typedef mp_int32 (*GetChType)();
typedef mp_int32 (*StubGetChType)(mp_void* pthis);

typedef mp_int32 (*GetSha256HashType)(const mp_char* buff, const mp_int32 len, mp_char* hashHex, mp_int32 hexLen);
typedef mp_int32 (*StubGetSha256HashType)(const mp_char* buff, const mp_int32 len, mp_char* hashHex, mp_int32 hexLen);

typedef mp_int32 (*PBKDF2HashType)(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText);
typedef mp_int32 (*StubPBKDF2HashType)(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText);

typedef mp_bool (*CPasswordFileExistType)(const mp_char* pszFilePath);
typedef mp_bool (*CPasswordStubFileExistType)(const mp_char* pszFilePath);

typedef mp_int32 (*GetRandomType)(mp_uint64 &num);
typedef mp_int32 (*StubGetRandomType)(mp_uint64 &num);

typedef mp_bool (*CheckAdminOldPwdType)(const mp_string& strUserName);
typedef mp_bool (*StubCheckAdminOldPwdType)(mp_string& strUserName);

typedef mp_bool (*CheckNewPwdType)(PASSWOD_TYPE eType, const mp_string& strNewPwd);
typedef mp_bool (*StubCheckNewPwdType)(PASSWOD_TYPE eType, const mp_string& strNewPwd);

typedef mp_void (*InputUserPwdType)(mp_string strUserName, mp_string &strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen);
typedef mp_void (*StubInputUserPwdType)(mp_string strUserName, mp_string &strUserPwd, INPUT_TYPE eType);
//*******************************************************************************
mp_int32 StubCPasswordGetValueString(mp_string strSection, mp_string strKey, mp_string& strValue){
    return -1;
}

mp_int32 StubCPasswordGetValueString0(mp_string strSection, mp_string strKey, mp_string& strValue){
    return 0;
}

mp_int32 StubCPasswordGetValueStringl(mp_string strSection, mp_string strKey, mp_string& strValue){
    strValue = "test";
    return 0;
}
//
mp_int32 StubSetValue(mp_string strSection, mp_string strKey, mp_string strValue){
    return -1;
}

mp_int32 StubSetValue0(mp_string strSection, mp_string strKey, mp_string strValue){
    return 0;
}
//
mp_bool StubCheckNewPwdPwd(PASSWOD_TYPE eType, const mp_string& strNewPwd){
    return 1;
}

mp_bool StubCheckNewPwdPwd0(PASSWOD_TYPE eType, const mp_string& strNewPwd){
    return 0;
}
//
mp_int32 StubVerifyOldUserPwd(mp_string& strUserName){
    return -1;
}

mp_int32 StubVerifyOldUserPwd0(mp_string& strUserName){
    return 0;
}
//
mp_int32 StubInputNewUserPwd(mp_string& strUserName, mp_string& strNewPwd){
    return -1;
}

mp_int32 StubInputNewUserPwd0(mp_string& strUserName, mp_string& strNewPwd){
    return 0;
}
//
mp_int32 StubConfirmNewUserPwd(mp_string& strUserName, mp_string& strNewPwd){
    return -1;
}

mp_int32 StubConfirmNewUserPwd0(mp_string& strUserName, mp_string& strNewPwd){
    return 0;
}
//
mp_int32 StubReadFile(mp_void* pthis,mp_string& strFilePath, vector<mp_string>& vecOutput){
    return -1;
}

mp_int32 StubReadFile0(mp_void* pthis,mp_string& strFilePath, vector<mp_string>& vecOutput){
    vecOutput.push_back("test");
    return 0;
}

mp_void StubCPasswordCLoggerLogVoid(mp_void* pthis){
    return;
}

mp_int32 StubGetCh(mp_void* pthis){
    static mp_int32 retv = 0;
    if (0 == retv){
        retv++;
        return 8;
    } else {
        return 10;
    }
}

mp_bool StubSaveAdminPwd(const mp_string& strPwd){
    return 1;
}

mp_int32 StubGetSha256Hash(const mp_char* buff, const mp_int32 len, mp_char* hashHex, mp_int32 hexLen){
    return 1;
}

mp_int32 StubGetSha256Hash0(const mp_char* buff, const mp_int32 len, mp_char* hashHex, mp_int32 hexLen){
    return 0;
}

mp_int32 StubPBKDF2Hash(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText){
    return 1;
}

mp_int32 StubPBKDF2Hash0(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText){
    return 0;
}

mp_bool CPasswordStubFileExist(const mp_char* pszFilePath){
    return 1;
}

mp_bool CPasswordStubFileExist0(const mp_char* pszFilePath){
    return 0;
}

mp_int32 StubGetRandom(mp_uint64 &num){
    return 1;
}

mp_int32 StubGetRandom0(mp_uint64 &num){
    return 0;
}

mp_bool StubCheckAdminOldPwd(mp_string& strUserName){
    return 1;
}

mp_bool StubCheckAdminOldPwd0(mp_string& strUserName){
    return 0;
}

mp_bool StubCheckNewPwd(PASSWOD_TYPE eType, const mp_string& strNewPwd){
    return 1;
}

mp_bool StubCheckNewPwd0(PASSWOD_TYPE eType, const mp_string& strNewPwd){
    return 0;
}

mp_void StubInputUserPwd(mp_string strUserName, mp_string &strUserPwd, INPUT_TYPE eType){
    return;
}

#endif
