/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "rootexec/SystemCallTest.h"
#include "array/Array.h"


TEST_F(CCommandMapTest, xFunc)
{
    CCommandMap commonMap;

    commonMap.InitDB2ScriptMap();
    commonMap.InitOracleScriptMap();
    commonMap.InitSysCmdMap1();
    commonMap.InitSysCmdMap2();
    commonMap.InitSysCmdMap3();
    commonMap.InitSysCmdMap4();
    commonMap.InitSysCmdMap5();
    commonMap.InitSysCmdMap6();
    commonMap.InitNeedEchoCmdMap1();
    commonMap.InitNeedEchoCmdMap2();
}


TEST_F(CCommandMapTest, GetCommandString)
{
    mp_int32 iCommandID = 0;
    CCommandMap commonMap;
    
    commonMap.GetCommandString(iCommandID);
    
    commonMap.m_mapCommand.insert(map<mp_int32, mp_string>::value_type(0, "hagrp"));
    commonMap.GetCommandString(iCommandID);
}


TEST_F(CCommandMapTest, NeedEcho)
{
    mp_int32 iCommandID = 0;
    CCommandMap commonMap;
    
    commonMap.NeedEcho(iCommandID);
    
    commonMap.m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(0, MP_TRUE));
    commonMap.NeedEcho(iCommandID);
}


TEST_F(CSystemCallTest, ExecSysCmd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strUniqueID;
    mp_int32 iCommandID;
    CSystemCall caller;

    typedef mp_int32 (*pOrgGetParamFromTmpFile)(mp_string& strUniqueID, mp_string& strParam);
    Stub<pOrgGetParamFromTmpFile, pStubIntType, mp_void> StubCSystemCall(&CSystemCall::GetParamFromTmpFile, &stub_return_ret);
    
    iRet = caller.ExecSysCmd(strUniqueID, iCommandID);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (*pOrgNeedEcho)(mp_string& strUniqueID, mp_string& strParam);
    Stub<pOrgNeedEcho, pStubBoolType, mp_void> StubCSystemCall1(&CSystemCall::GetParamFromTmpFile, &stub_return_bool);
    
    iRet = caller.ExecSysCmd(strUniqueID, iCommandID);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = caller.ExecSysCmd(strUniqueID, iCommandID);
    EXPECT_EQ(1, iRet);

    iRet = caller.ExecSysCmd(strUniqueID, iCommandID);
    EXPECT_EQ(1, iRet);
}


TEST_F(CSystemCallTest, ExecScript)
{
    mp_int32 iRet = 0;
    mp_string strUniqueID;
    mp_int32 iCommandID;
    CSystemCall caller;

    typedef mp_bool (*pOrgFileExist)(const mp_char* pszFilePath);
    Stub<pOrgFileExist, pStubBoolType, mp_void> StubCMFile(&CMpFile::FileExist, &stub_return_bool);

    iRet = caller.ExecScript(strUniqueID, iCommandID);
    EXPECT_EQ(255, iRet);

    iRet = caller.ExecScript(strUniqueID, iCommandID);
    EXPECT_EQ(5, iRet);

    iRet = caller.ExecScript(strUniqueID, iCommandID);
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CSystemCallTest, GetDisk80Page)
{
    mp_int32 iRet = 0;
    mp_string strUniqueID;
    CSystemCall caller;

    typedef mp_int32 (*pOrgGetParamFromTmpFile)(mp_string& strUniqueID, mp_string& strParam);
    Stub<pOrgGetParamFromTmpFile, pStubIntType, mp_void> StubCSystemCall(&CSystemCall::GetParamFromTmpFile, &stub_return_ret);

    typedef mp_int32 (*pOrgGetDisk80Page)(mp_string& strDevice, mp_string& strSN);
    Stub<pOrgGetDisk80Page, pStubIntType, mp_void> StubCArray(&CArray::GetDisk80Page, &stub_return_ret);

    iRet = caller.GetDisk80Page(strUniqueID);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = caller.GetDisk80Page(strUniqueID);
    EXPECT_TRUE(1);
}


TEST_F(CSystemCallTest, GetDisk83Page)
{
    mp_int32 iRet = 0;
    mp_string strUniqueID;
    CSystemCall caller;

    typedef mp_int32 (*pOrgGetParamFromTmpFile)(mp_string& strUniqueID, mp_string& strParam);
    Stub<pOrgGetParamFromTmpFile, pStubIntType, mp_void> StubCSystemCall(&CSystemCall::GetParamFromTmpFile, &stub_return_ret);

    typedef mp_int32 (*pOrgGetDisk83Page)(mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID);
    Stub<pOrgGetDisk83Page, pStubIntType, mp_void> StubCArray(&CArray::GetDisk83Page, &stub_return_ret);

    iRet = caller.GetDisk83Page(strUniqueID);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = caller.GetDisk83Page(strUniqueID);
    EXPECT_TRUE(1);
}

TEST_F(CSystemCallTest, GetDiskCapacity)
{
    mp_int32 iRet = 0;
    mp_string strUniqueID;
    CSystemCall caller;

    typedef mp_int32 (*pOrgGetParamFromTmpFile)(mp_string& strUniqueID, mp_string& strParam);
    Stub<pOrgGetParamFromTmpFile, pStubIntType, mp_void> StubCSystemCall(&CSystemCall::GetParamFromTmpFile, &stub_return_ret);

    typedef mp_int32 (*pOrgGetDiskCapacity)(mp_string& strDevice, mp_string& strBuf);
    Stub<pOrgGetDiskCapacity, pStubIntType, mp_void> StubCDisk(&CDisk::GetDiskCapacity, &stub_return_ret);
      
    iRet = caller.GetDiskCapacity(strUniqueID);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = caller.GetDiskCapacity(strUniqueID);
    EXPECT_TRUE(1);
}

TEST_F(CSystemCallTest, GetVendorAndProduct)
{
    mp_int32 iRet = 0;
    mp_string strUniqueID;
    CSystemCall caller;

    typedef mp_int32 (*pOrgGetParamFromTmpFile)(mp_string& strUniqueID, mp_string& strParam);
    Stub<pOrgGetParamFromTmpFile, pStubIntType, mp_void> StubCSystemCall(&CSystemCall::GetParamFromTmpFile, &stub_return_ret);

    typedef mp_int32 (*pOrgGetDiskArrayInfo)(mp_string& strDevice, mp_string& strVendor, mp_string& strProduct);
    Stub<pOrgGetDiskArrayInfo, pStubIntType, mp_void> StubCCArray(&CArray::GetDiskArrayInfo, &stub_return_ret);

    iRet = caller.GetVendorAndProduct(strUniqueID);
    EXPECT_EQ(MP_FAILED, iRet);   

    iRet = caller.GetVendorAndProduct(strUniqueID);
    EXPECT_TRUE(1);
}

TEST_F(CSystemCallTest, ExecThirdPartyScript)
{
    mp_int32 iRet = 0;
    mp_string strUniqueID;
    CSystemCall caller;

    iRet = caller.ExecThirdPartyScript(strUniqueID);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);     
}

TEST_F(CSystemCallTest, GetRawMajorMinor)
{
    mp_int32 iRet = 0;
    mp_string strUniqueID;
    CSystemCall caller;

    iRet = caller.GetRawMajorMinor(strUniqueID);
    EXPECT_EQ(MP_FAILED, iRet);     
}


TEST_F(CSystemCallTest, GetParamFromTmpFile)
{
    mp_int32 iRet = 0;
    mp_string strUniqueID;
    mp_string strParam;
    CSystemCall caller;

    typedef mp_bool (*pOrgFileExist)(const mp_char* pszFilePath);
    Stub<pOrgFileExist, pStubBoolType, mp_void> StubCMFile(&CMpFile::FileExist, &stub_return_bool);
    
    typedef mp_int32 (*pOrgReadInput)(mp_string& strUniqueID, mp_string& strInput);
    Stub<pOrgReadInput, pStubIntType, mp_void> StubCIPCFile(&CIPCFile::ReadInput, &stub_return_ret);
    
    iRet = caller.GetParamFromTmpFile(strUniqueID, strParam);
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = caller.GetParamFromTmpFile(strUniqueID, strParam);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = caller.GetParamFromTmpFile(strUniqueID, strParam);
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CCommandMapTest, GetDisk00Page)
{
    mp_string strUniqueID = "0x123234";
    CSystemCall caller;

    typedef mp_bool (*pOrgFileExistType)(const mp_char* pszFilePath);
    Stub<pOrgFileExistType, pStubBoolType, mp_void> stubCMpFile(&CMpFile::FileExist, &stub_return_bool);
    caller.GetDisk00Page(strUniqueID);

    typedef mp_int32 (*pOrgReadInput)(mp_string& strUniqueID, mp_string& strInput);
    Stub<pOrgReadInput, pStubRetType, mp_void> stubCIPCFile(&CIPCFile::ReadInput, &stub_return_ret);
    caller.GetDisk00Page(strUniqueID);

    typedef mp_int32 (*pOrgGetDisk00Page)(mp_string& strDevice, vector<mp_string>& vecResult);
    Stub<pOrgGetDisk00Page, pStubRetType, mp_void> stubCArray(&CArray::GetDisk00Page, &stub_return_ret);
    caller.GetDisk00Page(strUniqueID);
}


TEST_F(CCommandMapTest, GetDiskC8Page)
{
    mp_string strUniqueID = "0x123234";
    CSystemCall caller;

    typedef mp_bool (*pOrgFileExistType)(const mp_char* pszFilePath);
    Stub<pOrgFileExistType, pStubBoolType, mp_void> stubCMpFile(&CMpFile::FileExist, &stub_return_bool);
    caller.GetDiskC8Page(strUniqueID);

    typedef mp_int32 (*pOrgReadInput)(mp_string& strUniqueID, mp_string& strInput);
    Stub<pOrgReadInput, pStubRetType, mp_void> stubCIPCFile(&CIPCFile::ReadInput, &stub_return_ret);
    caller.GetDiskC8Page(strUniqueID);

    typedef mp_int32 (*pOrgGetDiskC8Page)(mp_string& strDevice, mp_string& strLunID);
    Stub<pOrgGetDiskC8Page, pStubRetType, mp_void> stubCArray(&CArray::GetDiskC8Page, &stub_return_ret);
    caller.GetDiskC8Page(strUniqueID);
}

static mp_void StubStrSplit(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
    static int i = 0;
    if (i++ >= 1)
    {
         vecTokens.push_back("6688");
    }
}


TEST_F(CCommandMapTest, BatchGetLUNInfo)
{
    mp_string strUniqueID = "0x123234";
    CSystemCall caller;

    typedef mp_int32 (*pOrgGetParamFromTmpFile)(mp_string& strUniqueID, mp_string& strParam);
    Stub<pOrgGetParamFromTmpFile, pStubRetType, mp_void> stubCSystemCall(&CSystemCall::GetParamFromTmpFile, &stub_return_ret);
    caller.BatchGetLUNInfo(strUniqueID);

    typedef mp_void (*pOrgStrSplit)(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep);
    typedef mp_void (*pStubStrSplit)(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep);
    Stub<pOrgStrSplit, pStubStrSplit, mp_void> stubCMpString(&CMpString::StrSplit, StubStrSplit);
    caller.BatchGetLUNInfo(strUniqueID);   

    typedef mp_int32 (*pOrgGetDiskArrayInfo)(mp_string& strDevice, mp_string& strVendor, mp_string& strProduct);
    Stub<pOrgGetDiskArrayInfo, pStubRetType, mp_void> stubCArray(&CArray::GetDiskArrayInfo, &stub_return_ret);
    caller.BatchGetLUNInfo(strUniqueID);
}


    

