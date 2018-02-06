/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/SystemExecTest.h"

TEST_F(CSystemExecTest,ExecSystemWithoutEcho){
    mp_string strCommand = "test";
    mp_bool bNeedRedirect = MP_TRUE;
    
    Stub<CSystemExecTestCheckCmdDelimiterType, CSystemExecTestStubCheckCmdDelimiterType, mp_void> stub2(&CheckCmdDelimiter, &CSystemExecTestStubCheckCmdDelimiter);
    CSystemExec::ExecSystemWithoutEcho(strCommand,bNeedRedirect);
}

TEST_F(CSystemExecTest,ExecSystemWithEcho){
    mp_string strCommand = "test";
    vector<mp_string> strEcho;
    mp_bool bNeedRedirect = MP_TRUE;
    
    CSystemExec::ExecSystemWithEcho(strCommand,strEcho,bNeedRedirect);
    
    Stub<CSystemExecTestCheckCmdDelimiterType, CSystemExecTestStubCheckCmdDelimiterType, mp_void> stub1(&CheckCmdDelimiter, &CSystemExecTestStubCheckCmdDelimiter);
    CSystemExec::ExecSystemWithEcho(strCommand,strEcho,bNeedRedirect);
}

TEST_F(CSystemExecTest,ExecScript){
    mp_string strScriptFileName = "test";
    mp_string strParam;
    vector<mp_string> pvecResult;
    mp_bool bNeedCheckSign = MP_TRUE;
    
    CSystemExec::ExecScript(strScriptFileName,strParam,&pvecResult,bNeedCheckSign);
    
    Stub<CSystemExecTestFileExistType, CSystemExecTestStubFileExistType, mp_void> stub1(&CMpFile::FileExist, &CSystemExecTestStubFileExist);
    CSystemExec::ExecScript(strScriptFileName,strParam,&pvecResult,bNeedCheckSign);
    
    bNeedCheckSign = MP_FALSE;
    CSystemExec::ExecScript(strScriptFileName,strParam,&pvecResult,bNeedCheckSign);
    
    pvecResult.push_back("test");
    Stub<CSystemExecExecSystemWithoutEchoType, CSystemExecStubExecSystemWithoutEchoType, mp_void> stub2(&CSystemExec::ExecSystemWithoutEcho, &CSystemExecStubExecSystemWithoutEcho);
    CSystemExec::ExecScript(strScriptFileName,strParam,&pvecResult,bNeedCheckSign);
    
    strParam = "test";
    CSystemExec::ExecScript(strScriptFileName,strParam,&pvecResult,bNeedCheckSign);
}
