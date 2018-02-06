/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/ChangeIPTest.h"

static mp_void StubGetInput(mp_string strHint, mp_string& strInput)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        strInput = "192.168.100.110";
    }
    else
    {
        strInput = "a.1";
    }
}

static mp_bool StubIsLocalIP(mp_string strIP)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        return MP_TRUE;
    }
    else
    {
        return MP_FALSE;
    }
}

static mp_int32 StubGetIPAddress(mp_string &strIP)
{
    static mp_int32 iCounter = 0;
    
    if (iCounter++ != 0)
    {
        if ( iCounter > 1)
        {
            strIP = "192.168.100.11";
        }
        else
        {
            strIP = "192.168.100.110";
        }
        
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_int32 StubSetIPAddress(void)
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

static mp_int32 StubSheckScriptSign(mp_void)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}


static mp_int32 stub_ReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static mp_int32 iCounter = 0;
    if ( iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        vecOutput.push_back("listen:192.168.100.170");
        return MP_SUCCESS;
    }
}



TEST_F(CChangeIpTest, Handle)
{
    typedef mp_void (*pOrgType)(mp_string strHint, mp_string& strInput, mp_int32 iInputLen);
    typedef mp_void (*StubFuncType)(mp_string strHint, mp_string& strInput);
    Stub<pOrgType, StubFuncType, mp_void> stubCPassword(&CPassword::GetInput, &StubGetInput);

    typedef mp_bool (*pOrgIsLocalIP)(mp_string strIP);
    typedef mp_bool (*StubIsLocalIPType)(mp_string strIP);
    Stub<pOrgIsLocalIP, StubIsLocalIPType, mp_void> stubCChangeIP(&CChangeIP::IsLocalIP, &StubIsLocalIP);

    typedef mp_int32 (*pOrgGetIPAddress)(mp_string &strIP);
    typedef mp_int32 (*StubGetIPAddressType)(mp_string &strIP);
    Stub<pOrgGetIPAddress, StubGetIPAddressType, mp_void> stubCChangeIP1(&CChangeIP::GetIPAddress, &StubGetIPAddress);

    typedef mp_int32 (*pOrgSetIPAddress)(mp_string strIP);
    typedef mp_int32 (*StubSetIPAddressType)(void);
    Stub<pOrgSetIPAddress, StubSetIPAddressType, mp_void> stubCChangeIP2(&CChangeIP::SetIPAddress, &StubSetIPAddress);
    
    mp_int32 iRet = MP_SUCCESS;
    CChangeIP IPObj;

    iRet = IPObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = IPObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = IPObj.Handle();
    EXPECT_EQ(MP_SUCCESS, iRet);
    
}


TEST_F(CChangeIpTest, GetIPAddress)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string ip;
    CChangeIP IPObj;
    
    typedef mp_bool (*pOrgFileExistType)(const mp_char* pszFilePath);
    Stub<pOrgFileExistType, pStubBoolType, mp_void> stubSign(&CMpFile::FileExist, &stub_return_bool);
    
    iRet = IPObj.GetIPAddress(ip);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = IPObj.GetIPAddress(ip);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (*pOrgReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    typedef mp_int32 (*pStubReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    Stub<pOrgReadFileType, pStubReadFileType, mp_void> stubSign1(&CMpFile::ReadFile, &stub_ReadFile);

    iRet = IPObj.GetIPAddress(ip);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = IPObj.GetIPAddress(ip);
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}

TEST_F(CChangeIpTest, SetIPAddress)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string ip;
    CChangeIP IPObj;
    typedef mp_bool (*pOrgFileExistType)(const mp_char* pszFilePath);
    Stub<pOrgFileExistType, pStubBoolType, mp_void> stubSign(&CMpFile::FileExist, &stub_return_bool);
    
    iRet = IPObj.SetIPAddress(ip);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = IPObj.SetIPAddress(ip);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (*pOrgReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    typedef mp_int32 (*pStubReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    Stub<pOrgReadFileType, pStubReadFileType, mp_void> stubSign1(&CMpFile::ReadFile, &stub_ReadFile);

    iRet = IPObj.SetIPAddress(ip);
    EXPECT_EQ(MP_FAILED, iRet);
    
    return;
}

TEST_F(CChangeIpTest, GetLocalIPs)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecIPs;   
    CChangeIP IPObj;
    
    iRet = IPObj.GetLocalIPs(vecIPs);
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}

TEST_F(CChangeIpTest, IsLocalIP)
{
    mp_bool bRet = MP_FALSE;
    mp_string strIP = "192.168.100.127";
    CChangeIP IPObj;

    typedef mp_int32 (*pOrgType)(vector<mp_string>& vecIPs);
    Stub<pOrgType, pStubIntType, mp_void> stubSign(&CChangeIP::GetLocalIPs, &stub_return_ret);
    
    bRet = IPObj.IsLocalIP(strIP);
    EXPECT_EQ(MP_FALSE, bRet);

    bRet = IPObj.IsLocalIP(strIP);
    EXPECT_EQ(MP_FALSE, bRet);

    return;
}

TEST_F(CChangeIpTest, RestartNginx)
{
    mp_int32 iRet = 0;
    CChangeIP IPObj;
    
    /* ºÏ≤‚ ß∞‹; */
    iRet = IPObj.RestartNginx();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = IPObj.RestartNginx();
    EXPECT_EQ(-1, iRet);

    iRet = IPObj.RestartNginx();
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}



