/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/SignTest.h"

TEST(CheckScriptSign,normal){
    Stub<SignTestCLoggerLogType, StubSignTestCLoggerLogType, mp_void> stublog(&CLogger::Log, &SignTestStubCLoggerLog);
    mp_string strFileName = "test";

    CheckScriptSign(strFileName);
    
    Stub<SignTestFileExistType, SignTestStubFileExistType, mp_void> stub1(&CMpFile::FileExist, &SignTestStubFileExist);
    CheckScriptSign(strFileName);
    
    Stub<SignTestReadFileType, SignTestStubReadFileType, mp_void> mystub1(&CMpFile::ReadFile, &SignTestStubReadFile);
    CheckScriptSign(strFileName);
}

TEST(CheckFileSign,normal){
    Stub<SignTestCLoggerLogType, StubSignTestCLoggerLogType, mp_void> stublog(&CLogger::Log, &SignTestStubCLoggerLog);
    mp_string strFileName = "test";
    vector<mp_string> vecSigns;
    
    CheckFileSign(strFileName,vecSigns);
    
    vecSigns.push_back("test");
    CheckFileSign(strFileName,vecSigns);
    
    vecSigns.push_back("test==");
    CheckFileSign(strFileName,vecSigns);
}

TEST(GetScriptNames,normal){
    Stub<SignTestCLoggerLogType, StubSignTestCLoggerLogType, mp_void> stublog(&CLogger::Log, &SignTestStubCLoggerLog);
    mp_string strFileName = "test";
    map<mp_string,mp_int32> mapScriptNames;
    
    GetScriptNames(strFileName,mapScriptNames);
}

TEST(CheckSign,normal){
    Stub<SignTestCLoggerLogType, StubSignTestCLoggerLogType, mp_void> stublog(&CLogger::Log, &SignTestStubCLoggerLog);
    mp_string strFileName = "test";
    mp_string strSignEncrypt;
    
    CheckSign(strFileName,strSignEncrypt);
    
    strSignEncrypt = "test";
    CheckSign(strFileName,strSignEncrypt);
    
    Stub<SignTestReadFileType, SignTestStubReadFileType, mp_void> mystub1(&CMpFile::ReadFile, &SignTestStubReadFile);
    CheckSign(strFileName,strSignEncrypt);
    
    Stub<SignTestPBKDF2HashType, SignTestStubPBKDF2HashType, mp_void> mystub2(&PBKDF2Hash, &SignTestStubPBKDF2Hash);
    CheckSign(strFileName,strSignEncrypt);
}
