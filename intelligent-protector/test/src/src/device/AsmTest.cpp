/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/AsmTest.h"

TEST_F(CAsmTest,AsmLibScan){
    mp_int32 ret;
    CAsm m_CAsm;
    
    {
        ret = m_CAsm.AsmLibScan();
        EXPECT_EQ(ret,ERROR_DEVICE_ASM_SCAN_ASMLIB_FAILED);
    }
    
    {
        Stub<ExecType, StubExecType, mp_void> mystub1(&CRootCaller::Exec, &StubExec);
        ret = m_CAsm.AsmLibScan();
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}
