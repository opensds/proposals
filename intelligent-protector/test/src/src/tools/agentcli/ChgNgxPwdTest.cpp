/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/ChgNgxPwdTest.h"

static mp_bool stubCheckAdminOldPwd(mp_void)
{
        return MP_TRUE;
}


static mp_int32 stubInputNginxInfo(mp_void)
{
    static int iCounter = 0;
    if (++iCounter <= 1)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}


static mp_int32 stubChgNginxInfo(mp_void)
{
    static int iCounter = 0;
    if (++iCounter <= 1)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}


TEST_F(CChgNgxPwdTest, Handle)
{
    mp_int32 iRet = 0;
    CChgNgxPwd PwdObj;
    
    typedef mp_void (*pOrgstubGetInput)(mp_string strHint, mp_string& strInput, mp_int32 iInputLen);
    Stub<pOrgstubGetInput, pStubVoidType, mp_void> stubCPassword00(&CPassword::GetInput, stub_return_nothing);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    /* ºÏ≤‚—≠ª∑; */
    typedef mp_bool (*pOrgCheckAdminOldPwd)(const mp_string& strOldPwd);
    typedef mp_bool (*pStubCheckAdminOldPwd)(mp_void);
    Stub<pOrgCheckAdminOldPwd, pStubCheckAdminOldPwd, mp_void> stubCPassword(&CPassword::CheckAdminOldPwd, stubCheckAdminOldPwd);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    /* InputNginxInfo; */
    typedef mp_int32 (*pOrgInputNginxInfo)(mp_string &strCertificate, mp_string &strKeyFile, mp_string &strNewPwd);
    typedef mp_int32 (*pStubInputNginxInfo)(mp_void);
    Stub<pOrgInputNginxInfo, pStubInputNginxInfo, mp_void> stubCChgNgxPwd00(&CChgNgxPwd::InputNginxInfo, stubInputNginxInfo);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    /* ChgNginxInfo; */
    typedef mp_int32 (*pOrgstubChgNginxInfo)(mp_string strCertificate, mp_string strKeyFile, mp_string strNewPwd);
    typedef mp_int32 (*pStubstubChgNginxInfo)(mp_void);
    Stub<pOrgstubChgNginxInfo, pStubstubChgNginxInfo, mp_void> stubCChgNgxPwd01(&CChgNgxPwd::ChgNginxInfo, stubChgNginxInfo);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}


TEST_F(CChgNgxPwdTest, InputNginxInfo)
{
    mp_int32 iRet = 0;
    mp_string strCertificate;
    mp_string strKeyFile;
    mp_string strNewPwd;
    CChgNgxPwd PwdObj;

    typedef mp_void (*pOrgstubGetInput)(mp_string strHint, mp_string& strInput, mp_int32 iInputLen);
    Stub<pOrgstubGetInput, pStubVoidType, mp_void> stubCPassword00(&CPassword::GetInput, stub_return_nothing);

    typedef mp_bool (*pOrgstubFileExist)(const mp_char* pszFilePath);
    Stub<pOrgstubFileExist, pStubBoolType, mp_void> stubCMpFile(&CMpFile::FileExist, stub_return_bool);
    
    PwdObj.InputNginxInfo(strCertificate, strKeyFile, strNewPwd);

    typedef mp_bool (*pOrgChgPwdNoCheck)(mp_string& strPwd);
    Stub<pOrgChgPwdNoCheck, pStubRetType, mp_void> stubCPassword01(&CPassword::ChgPwdNoCheck, stub_return_ret);
    PwdObj.InputNginxInfo(strCertificate, strKeyFile, strNewPwd);
    PwdObj.InputNginxInfo(strCertificate, strKeyFile, strNewPwd);

    return ;
}


static mp_int32 StubReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static int iCounter = 0;
    if (++iCounter <= 1)
    {
        return MP_FAILED;
    }
    else
    {
        vecOutput.push_back("asdfasdfdf");
        return MP_SUCCESS;
    }
}

TEST_F(CChgNgxPwdTest, ChgNginxInfo)
{
    mp_int32 iRet = 0;
    mp_string strCertificate;
    mp_string strKeyFile;
    mp_string strNewPwd;
    
    CChgNgxPwd PwdObj;

    typedef mp_bool (*pOrgstubFileExist)(const mp_char* pszFilePath);
    Stub<pOrgstubFileExist, pStubBoolType, mp_void> stubCMpFile00(&CMpFile::FileExist, stub_return_bool);
    PwdObj.ChgNginxInfo(strCertificate, strKeyFile, strNewPwd);

    typedef mp_int32 (*pOrgstubReadFile)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    typedef mp_int32 (*pStubReadFile)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    Stub<pOrgstubReadFile, pStubReadFile, mp_void> stubCMpFile01(&CMpFile::ReadFile, StubReadFile);
    PwdObj.ChgNginxInfo(strCertificate, strKeyFile, strNewPwd);
    PwdObj.ChgNginxInfo(strCertificate, strKeyFile, strNewPwd);

    return ;
}




