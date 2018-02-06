/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/StartNginxTest.h"

static mp_int32 StubGetCiphertext(mp_string &CipherStr)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        CipherStr = "test";
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_int32 StubExecNginxStart(void)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_int32 StubWriteFile(mp_string& strPath, vector<mp_string> &strPWD)
{
    static mp_int32 iCounter = 0;
    if ( iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 StubDelFile(const mp_char* strFilePath)
{
    static mp_int32 iCounter = 0;
    
    if ( iCounter == 1 )
    {
        return MP_FAILED;
    } 
    
    if ( iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 StubCheckScriptSign(const mp_string strFileName)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_int32 StubGetValueString(mp_void* pthis, mp_string strSection, mp_string strKey, mp_string& strValue)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}


TEST_F(CStartNginxTest, Handle)
{
    mp_int32 iRet = 0;
    CStartNginx startObj;
    
    typedef mp_int32 (*pOrgstubGetCiphertext)(mp_string& strInput);
    typedef mp_int32 (*StubGetCiphertextType)(mp_string& strInput);
    Stub<pOrgstubGetCiphertext, StubGetCiphertextType, mp_void> stubCStartNginx1(&CStartNginx::GetPassword, StubGetCiphertext);
    
    typedef mp_int32 (*pOrgstubWriteFile)(mp_string &strPath, vector<mp_string> &strPWD);
    typedef mp_int32 (*StubWritFileType)(mp_string &strPath, vector<mp_string> &strPWD);
    Stub<pOrgstubWriteFile, StubWritFileType, mp_void> stubCStartNginx2(&CIPCFile::WriteFile, &StubWriteFile);
    
    typedef mp_int32 (*pOrgstubExecNginxStart)(void);
    typedef mp_int32 (*StubExecNginxStartType)(void);
    Stub<pOrgstubExecNginxStart, StubExecNginxStartType, mp_void> stubCStartNginx3(&CStartNginx::ExecNginxStart, StubExecNginxStart);
        
    typedef mp_int32 (*pOrgstubDelFile)(const mp_char* strFilePath);
    typedef mp_int32 (*StubDelFileType)(const mp_char* strFilePath);
    Stub<pOrgstubDelFile, StubDelFileType, mp_void> stubCStartNginx4(&CMpFile::DelFile, StubDelFile);
    
    iRet = startObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);
    
}

TEST_F(CStartNginxTest, ExecNginxStart)
{
    mp_int32 iRet = 0;
    CStartNginx startObj;
    
    typedef mp_int32 (*pOrgCheckScriptSign)(const mp_string strFileName);
    typedef mp_int32 (*StubCheckScriptSignType)(const mp_string strFileName);
    Stub<pOrgCheckScriptSign, StubCheckScriptSignType, mp_void> stub0(&CheckScriptSign, StubCheckScriptSign);
    
    iRet = startObj.ExecNginxStart();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.ExecNginxStart();
    EXPECT_EQ(MP_FAILED, iRet);
    
}

TEST_F(CStartNginxTest, GetPassword)
{
    mp_int32 iRet = 0;
    CStartNginx startObj;
    mp_string pCipherStr;
    
    typedef mp_int32 (CConfigXmlParser::*pOrgGetValueString)(mp_string strSection, mp_string strKey, mp_string& strValue);
    typedef mp_int32 (*StubGetValueStringType)(mp_void* pthis, mp_string strSection, mp_string strKey, mp_string& strValue);
    Stub<pOrgGetValueString, StubGetValueStringType, mp_void> mystub0(&CConfigXmlParser::GetValueString, &StubGetValueString);
    
    iRet = startObj.GetPassword(pCipherStr);
    EXPECT_EQ(MP_SUCCESS, iRet);
    
    iRet = startObj.GetPassword(pCipherStr);
    EXPECT_EQ(MP_SUCCESS, iRet);
    
}

