/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/CollectLogTest.h"

static mp_void stubGetInput(mp_string strHint, mp_string& strInput)
{
    static mp_int32 ival = 0;
    if (ival++ == 0)
    {
        strInput = "n";
    }
    else
    {
        strInput = "y";
    }
}


TEST_F(CCollectLogTest, Handle)
{
    mp_int32 iRet = MP_SUCCESS;
    CCollectLog Obj;

    typedef mp_void (*pOrgGetInput)(mp_string strHint, mp_string& strInput, mp_int32 iInputLen);
    typedef mp_void (*pStubGetInput)(mp_string strHint, mp_string& strInput);
    Stub<pOrgGetInput, pStubGetInput, mp_void> stubCPasswd(&CPassword::GetInput, &stubGetInput);

    printf("\n#######################   01");
    
    iRet = Obj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    printf("\n#######################   02");
    
    typedef mp_int32 (*pPackageLog)(mp_string strLogName);
    Stub<pPackageLog, pStubIntType, mp_void> stubPackage(&PackageLog, &stub_return_ret); 
    
    typedef mp_string (*pGetTimeString)(mp_time* pTime);
    Stub<pGetTimeString, pStubStringType, mp_void> stubCMpTime(&CMpTime::GetTimeString, &stub_return_string);

    iRet = Obj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    printf("\n#######################   03");

    iRet = Obj.Handle();
    EXPECT_TRUE(1);
    
    printf("\n#######################   04");
    
}

