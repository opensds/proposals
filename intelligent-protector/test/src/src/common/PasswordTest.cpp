/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/PasswordTest.h"

TEST_F(CPasswordTest,ChgPwd){
    mp_string strPwd = "test";
    
    Stub<InputUserPwdType, StubInputUserPwdType, mp_void> mystub8(&CPassword::InputUserPwd, &StubInputUserPwd);
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL,strPwd);
    }
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<CheckNewPwdType, StubCheckNewPwdType, mp_void> mystub3(&CPassword::CheckNewPwd, &StubCheckNewPwdPwd);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL,strPwd);
    }
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<CheckNewPwdType, StubCheckNewPwdType, mp_void> mystub3(&CPassword::CheckNewPwd, &StubCheckNewPwdPwd0);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL,strPwd);
    }
}

/*
TEST_F(CPasswordTest,ChgPwdNoCheck){
	mp_string strPwd = "Admin@123";

    CPassword::ChgPwdNoCheck(strPwd);
}*/

TEST_F(CPasswordTest,CheckPasswordOverlap){
	mp_string strPasswd = "Admin@123";

    CPassword::CheckPasswordOverlap(strPasswd);
}

TEST_F(CPasswordTest,ChgAdminPwd){
    Stub<GetChType, StubGetChType, mp_void> mystub7(&CMpString::GetCh, &StubGetCh);
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString);
        CPassword::ChgAdminPwd();
    }
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<VerifyOldUserPwdType, StubVerifyOldUserPwdType, mp_void> mystub3(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd);
        CPassword::ChgAdminPwd();
    }

    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub3(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<VerifyOldUserPwdType, StubVerifyOldUserPwdType, mp_void> mystub4(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd0);
        Stub<InputNewUserPwdType, StubInputNewUserPwdType, mp_void> mystub5(&CPassword::InputNewUserPwd, &StubInputNewUserPwd);
        CPassword::ChgAdminPwd();
    }
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub6(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<VerifyOldUserPwdType, StubVerifyOldUserPwdType, mp_void> mystub7(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd0);
        Stub<InputNewUserPwdType, StubInputNewUserPwdType, mp_void> mystub8(&CPassword::InputNewUserPwd, &StubInputNewUserPwd0);
        Stub<ConfirmNewUserPwdType, StubConfirmNewUserPwdType, mp_void> mystub9(&CPassword::ConfirmNewUserPwd, &StubConfirmNewUserPwd);
        CPassword::ChgAdminPwd();
    }

    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub10(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<VerifyOldUserPwdType, StubVerifyOldUserPwdType, mp_void> mystub11(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd0);
        Stub<InputNewUserPwdType, StubInputNewUserPwdType, mp_void> mystub12(&CPassword::InputNewUserPwd, &StubInputNewUserPwd0);
        Stub<ConfirmNewUserPwdType, StubConfirmNewUserPwdType, mp_void> mystub13(&CPassword::ConfirmNewUserPwd, &StubConfirmNewUserPwd0);
        Stub<SaveAdminPwdType, StubSaveAdminPwdType, mp_void> mystub14(&CPassword::SaveAdminPwd, &StubSaveAdminPwd);
        CPassword::ChgAdminPwd();
    }
}

TEST_F(CPasswordTest,VerifyOldUserPwd){
    mp_string strUserName = "test";
    
    Stub<InputUserPwdType, StubInputUserPwdType, mp_void> mystub8(&CPassword::InputUserPwd, &StubInputUserPwd);
    {
        Stub<CheckAdminOldPwdType, StubCheckAdminOldPwdType, mp_void> mystub1(&CPassword::CheckAdminOldPwd, &StubCheckAdminOldPwd);
        CPassword::VerifyOldUserPwd(strUserName);
    }
    
    {
        Stub<CheckAdminOldPwdType, StubCheckAdminOldPwdType, mp_void> mystub1(&CPassword::CheckAdminOldPwd, &StubCheckAdminOldPwd0);
        CPassword::VerifyOldUserPwd(strUserName);
    }
}

TEST_F(CPasswordTest,InputNewUserPwd){
    mp_string strUserName = "test";
    mp_string strNewPwd = "test";
    
    Stub<InputUserPwdType, StubInputUserPwdType, mp_void> mystub8(&CPassword::InputUserPwd, &StubInputUserPwd);
    {
        Stub<CheckNewPwdType, StubCheckNewPwdType, mp_void> mystub1(&CPassword::CheckNewPwd, &StubCheckNewPwd);
        CPassword::InputNewUserPwd(strUserName,strNewPwd);
    }
    
    {
        Stub<CheckNewPwdType, StubCheckNewPwdType, mp_void> mystub1(&CPassword::CheckNewPwd, &StubCheckNewPwd0);
        CPassword::InputNewUserPwd(strUserName,strNewPwd);
    }
}

TEST_F(CPasswordTest,ConfirmNewUserPwd){
    mp_string strUserName = "test";
    mp_string strNewPwd = "test";
    
    Stub<GetChType, StubGetChType, mp_void> mystub1(&CMpString::GetCh, &StubGetCh);
    CPassword::ConfirmNewUserPwd(strUserName,strNewPwd);
}

TEST_F(CPasswordTest,InputUserPwd){
    mp_string strUserName = "test";
    mp_string strUserPwd = "test";
    INPUT_TYPE eType;
    
    Stub<GetChType, StubGetChType, mp_void> mystub1(&CMpString::GetCh, &StubGetCh);
    {
        eType = INPUT_GET_ADMIN_OLD_PWD;
        CPassword::InputUserPwd(strUserName,strUserPwd,eType);
    }
    
    {
        eType = INPUT_SNMP_OLD_PWD;
        CPassword::InputUserPwd(strUserName,strUserPwd,eType);
    }
    
    {
        eType = INPUT_DEFAULT;
        CPassword::InputUserPwd(strUserName,strUserPwd,eType);
    }
}

TEST_F(CPasswordTest,CheckAdminOldPwd){
    mp_string strOldPwd = "test";
    
    Stub<GetChType, StubGetChType, mp_void> mystub3(&CMpString::GetCh, &StubGetCh);
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString);
        CPassword::CheckAdminOldPwd(strOldPwd);
    }
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<PBKDF2HashType, StubPBKDF2HashType, mp_void> mystub2(&PBKDF2Hash, &StubPBKDF2Hash);
        CPassword::CheckAdminOldPwd(strOldPwd);
    }
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<PBKDF2HashType, StubPBKDF2HashType, mp_void> mystub2(&PBKDF2Hash, &StubPBKDF2Hash);
        CPassword::CheckAdminOldPwd(strOldPwd);
    }
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        Stub<GetSha256HashType, StubGetSha256HashType, mp_void> mystub2(&GetSha256Hash, &StubGetSha256Hash);
        CPassword::CheckAdminOldPwd(strOldPwd);
    }
}

TEST_F(CPasswordTest,CheckOtherOldPwd){
    PASSWOD_TYPE eType;
    mp_string strPwd = "test";
    
    {
        eType = PASSWORD_INPUT;
        CPassword::CheckOtherOldPwd(eType,strPwd);
    }
    
    {
        eType = PASSWORD_INPUT;
        CPassword::CheckOtherOldPwd(eType,strPwd);
    }
    
    {
        eType = PASSWORD_SNMP_PRIVATE;
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString);
        CPassword::CheckOtherOldPwd(eType,strPwd);
    }
    
    {
        eType = PASSWORD_SNMP_AUTH;
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        CPassword::CheckOtherOldPwd(eType,strPwd);
    }
}

TEST_F(CPasswordTest,CheckNginxOldPwd){
    mp_string strOldPwd = "test";
    
    {
        Stub<CPasswordFileExistType, CPasswordStubFileExistType, mp_void> mystub1(&CMpFile::FileExist, &CPasswordStubFileExist);
        CPassword::CheckNginxOldPwd(strOldPwd);
    }
    
    {
        Stub<CPasswordFileExistType, CPasswordStubFileExistType, mp_void> mystub1(&CMpFile::FileExist, &CPasswordStubFileExist0);
        Stub<ReadFileType, StubReadFileType, mp_void> mystub2(&CIPCFile::ReadFile, &StubReadFile);
        CPassword::CheckNginxOldPwd(strOldPwd);
    }
    
    {
        Stub<CPasswordFileExistType, CPasswordStubFileExistType, mp_void> mystub1(&CMpFile::FileExist, &CPasswordStubFileExist0);
        Stub<ReadFileType, StubReadFileType, mp_void> mystub2(&CIPCFile::ReadFile, &StubReadFile0);
        CPassword::CheckNginxOldPwd(strOldPwd);
    }
}

TEST_F(CPasswordTest,GetNginxKey){
    mp_string strKey = "test";
    vector<mp_string> vecResult;
    vecResult.push_back("ssl_certificate_key_password");
    
    CPassword::GetNginxKey(strKey,vecResult);
}

TEST_F(CPasswordTest,CheckNewPwd){
    PASSWOD_TYPE eType = PASSWORD_ADMIN;
    mp_string strPwd = "test";
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString);
        CPassword::CheckNewPwd(eType,strPwd);
    }
        
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueStringl);
        CPassword::CheckNewPwd(eType,strPwd);
    }
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub3(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        CPassword::CheckNewPwd(eType,strPwd);
    }
    
    {
        eType = PASSWORD_SNMP_PRIVATE;
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub4(&CConfigXmlParser::GetValueString, &StubCPasswordGetValueString0);
        CPassword::CheckNewPwd(eType,strPwd);
    } 
}

TEST_F(CPasswordTest,SaveAdminPwd){
    mp_string strPwd = "test";
    
    {
        Stub<GetRandomType, StubGetRandomType, mp_void> mystub1(&GetRandom, &StubGetRandom);
        CPassword::SaveAdminPwd(strPwd);
    }
    
    {
        Stub<GetRandomType, StubGetRandomType, mp_void> mystub2(&GetRandom, &StubGetRandom0);
        Stub<PBKDF2HashType, StubPBKDF2HashType, mp_void> mystub3(&PBKDF2Hash, &StubPBKDF2Hash);
        CPassword::SaveAdminPwd(strPwd);
    }
    
    {
        Stub<GetRandomType, StubGetRandomType, mp_void> mystub4(&GetRandom, &StubGetRandom0);
        Stub<PBKDF2HashType, StubPBKDF2HashType, mp_void> mystub5(&PBKDF2Hash, &StubPBKDF2Hash0);
        Stub<SetValueType, StubSetValueType, mp_void> mystub6(&CConfigXmlParser::SetValue, &StubSetValue);
        CPassword::SaveAdminPwd(strPwd);
    }
    
    {
        Stub<GetRandomType, StubGetRandomType, mp_void> mystub7(&GetRandom, &StubGetRandom0);
        Stub<PBKDF2HashType, StubPBKDF2HashType, mp_void> mystub8(&PBKDF2Hash, &StubPBKDF2Hash0);
        Stub<SetValueType, StubSetValueType, mp_void> mystub9(&CConfigXmlParser::SetValue, &StubSetValue0);
        CPassword::SaveAdminPwd(strPwd);
    }
}

TEST_F(CPasswordTest,SaveOtherPwd){
    PASSWOD_TYPE eType;
    mp_string strPwd = "test";
    
    {
        eType = PASSWORD_NGINX_SSL;
        CPassword::SaveOtherPwd(eType,strPwd);
    }
    
    {
        eType = PASSWORD_SNMP_AUTH;
        Stub<SetValueType, StubSetValueType, mp_void> mystub1(&CConfigXmlParser::SetValue, &StubSetValue);
        CPassword::SaveOtherPwd(eType,strPwd);
    }
    
    {
        eType = PASSWORD_SNMP_PRIVATE;
        Stub<SetValueType, StubSetValueType, mp_void> mystub2(&CConfigXmlParser::SetValue, &StubSetValue0);
        CPassword::SaveOtherPwd(eType,strPwd);
    }    
}

TEST_F(CPasswordTest,SaveNginxPwd){
    mp_string strPwd = "test";
    
    {
        Stub<CPasswordFileExistType, CPasswordStubFileExistType, mp_void> mystub1(&CMpFile::FileExist, &CPasswordStubFileExist0);
        CPassword::SaveNginxPwd(strPwd);
    }
    
    {
        Stub<CPasswordFileExistType, CPasswordStubFileExistType, mp_void> mystub1(&CMpFile::FileExist, &CPasswordStubFileExist);
        Stub<ReadFileType, StubReadFileType, mp_void> mystub2(&CIPCFile::ReadFile, &StubReadFile);
        CPassword::SaveNginxPwd(strPwd);
    }
    
    {
        Stub<CPasswordFileExistType, CPasswordStubFileExistType, mp_void> mystub1(&CMpFile::FileExist, &CPasswordStubFileExist);
        Stub<ReadFileType, StubReadFileType, mp_void> mystub2(&CIPCFile::ReadFile, &StubReadFile0);
        CPassword::SaveNginxPwd(strPwd);
    }
}

TEST_F(CPasswordTest,CalComplexity){
    mp_string strPwd;
    mp_int32 iNum;
    mp_int32 iUppercase;
    mp_int32 iLowcase;
    mp_int32 iSpecial;
    
    strPwd ="1";
    CPassword::CalComplexity(strPwd,iNum,iUppercase,iLowcase,iSpecial);
    
    strPwd ="A";
    CPassword::CalComplexity(strPwd,iNum,iUppercase,iLowcase,iSpecial);
    
    strPwd ="a";
    iSpecial = 0;
    CPassword::CalComplexity(strPwd,iNum,iUppercase,iLowcase,iSpecial);
}

TEST_F(CPasswordTest,CalculateComplexity){
    mp_int32 iNumber = 1;
    mp_int32 iUppercase = 1;
    mp_int32 iLowcase = 1;
    
    CPassword::CalculateComplexity( iNumber,  iUppercase,  iLowcase);
}
/*
TEST_F(CPasswordTest,CheckPasswordOverlap){
    mp_string strPasswd = "Admin@123";
    
    CPassword::CheckPasswordOverlap(strPasswd);
}*/

TEST_F(CPasswordTest,GetInput){
    mp_string strHint = "test";
    mp_string strInput = "test";
    
    Stub<GetChType, StubGetChType, mp_void> mystub3(&CMpString::GetCh, &StubGetCh);
    CPassword::GetInput( strHint, strInput);
}

TEST_F(CPasswordTest,GetLockTime){
    CPassword::GetLockTime();
}

TEST_F(CPasswordTest,ClearLock){
    CPassword::ClearLock();
}

