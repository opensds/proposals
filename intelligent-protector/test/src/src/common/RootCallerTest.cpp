/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/RootCallerTest.h"

TEST_F(CRootCallerTest,Exec){
    mp_int32 iCommandID;
    mp_string strParam;
    vector<mp_string> pvecResult;
    
    CRootCaller::Exec(iCommandID,strParam,&pvecResult);
    
    strParam = "test";
    Stub<ReadResultType, StubReadResultType, mp_void> mystub2(&CIPCFile::ReadResult, &StubReadResult);
    Stub<ExecSystemWithoutEchoType, StubExecSystemWithoutEchoType, mp_void> mystub3(&CSystemExec::ExecSystemWithoutEcho, &StubExecSystemWithoutEcho);
    CRootCaller::Exec(iCommandID,strParam,&pvecResult);
    
    Stub<WriteInputType, StubWriteInputType, mp_void> mystub1(&CIPCFile::WriteInput, &StubWriteInput);
    CRootCaller::Exec(iCommandID,strParam,&pvecResult);
}
