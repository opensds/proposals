/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _SIGNTEST_H_
#define _SIGNTEST_H_

#define private public

#include "common/Sign.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Path.h"
#include "common/Log.h"
#include "common/UniqueId.h"
#include "common/ErrorCode.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"

//*******************************************************************************
typedef mp_void (CLogger::*SignTestCLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubSignTestCLoggerLogType)(mp_void);

typedef mp_int32 (*SignTestReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
typedef mp_int32 (*SignTestStubReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);

typedef mp_int32 (*SignTestPBKDF2HashType)(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText);
typedef mp_int32 (*SignTestStubPBKDF2HashType)(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText);

typedef mp_bool (*SignTestFileExistType)(const mp_char* pszFilePath);
typedef mp_bool (*SignTestStubFileExistType)(const mp_char* pszFilePath);
//*******************************************************************************
mp_void SignTestStubCLoggerLog(mp_void){
    return;
}

mp_int32 SignTestStubReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput){
    vecOutput.push_back("test");
    return 0;
}

mp_int32 SignTestStubPBKDF2Hash(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText){
    return 0;
}

mp_bool SignTestStubFileExist(const mp_char* pszFilePath){
    return 1;
}

#endif
