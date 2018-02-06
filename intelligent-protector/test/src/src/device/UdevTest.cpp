/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/UdevTest.h"

TEST_F(UdevTest,GetUdevRulesFileName){
    mp_int32 ret;
    CUdev m_CUdev;
    mp_string strUdevRulesFileName;
    
    {
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExec);
        ret = m_CUdev.GetUdevRulesFileName(strUdevRulesFileName);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExec0);
        ret = m_CUdev.GetUdevRulesFileName(strUdevRulesFileName);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExecl);
        ret = m_CUdev.GetUdevRulesFileName(strUdevRulesFileName);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(UdevTest,Create){
    mp_int32 ret;
    CUdev m_CUdev;
    mp_string strUdevRule;
    mp_string strWWN;
    
    {
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExec);
        ret = m_CUdev.Create(strUdevRule,strWWN);
        EXPECT_EQ(ret,1073949062);
    }
    
    {
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExec0);
        ret = m_CUdev.Create(strUdevRule,strWWN);
        EXPECT_EQ(ret,ERROR_DEVICE_UDEV_CREATE_FAILED);
    }
    
    {
        strUdevRule = "RESULT==\"3";
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExecl);
        ret = m_CUdev.Create(strUdevRule,strWWN);
        EXPECT_EQ(ret,ERROR_DEVICE_UDEV_CREATE_FAILED);
    }
    
    {
        strUdevRule = "RESULT==\"3RESULT==\"3";
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExecl);
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub2(&CRootCaller::Exec,&StubUdevTestExec);
        ret = m_CUdev.Create(strUdevRule,strWWN);
        EXPECT_EQ(ret,1073949062);
    }
    
    {
        strUdevRule = "RESULT==\"3RESULT==\"3";
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExecl);
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub2(&CRootCaller::Exec,&StubUdevTestExec);
        ret = m_CUdev.Create(strUdevRule,strWWN);
        EXPECT_EQ(ret,1073949062);
    }
    
    {
        strUdevRule = "RESULT==\"3RESULT==\"3";
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExecl);
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub2(&CRootCaller::Exec,&StubUdevTestExec0);
        ret = m_CUdev.Create(strUdevRule,strWWN);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        strUdevRule = "RESULT==\"3RESULT==\"3";
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExecl);
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub2(&CRootCaller::Exec,&StubUdevTestExecl);
        ret = m_CUdev.Create(strUdevRule,strWWN);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(UdevTest,Delete){
    mp_int32 ret;
    CUdev m_CUdev;
    mp_string strUdevRule;
    mp_string strWWN;
    
    {
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExec);
        ret = m_CUdev.Delete(strUdevRule,strWWN);
        EXPECT_EQ(ret,1073949063);
    }
    
    {
        Stub<UdevTestExecType,StubUdevTestExecType,void>mystub1(&CRootCaller::Exec,&StubUdevTestExec0);
        ret = m_CUdev.Delete(strUdevRule,strWWN);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}
