/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/ChgSnmpTest.h"

static mp_bool stubCheckAdminOldPwd(mp_void)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ <= MAX_FAILED_COUNT)
    {
        return MP_FALSE;
    }
    else
    {
        return MP_TRUE;
    }
}

TEST_F(CChgSnmpTest, Handle)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    
    iRet = snmp.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_bool (*pOrgCheckAdminOldPwd)(const mp_string& strOldPwd);
    typedef mp_bool (*pStubCheckAdminOldPwd)(mp_void);
    Stub<pOrgCheckAdminOldPwd, pStubBoolType, mp_void> stubCPassword0(&CPassword::CheckAdminOldPwd, &stubCheckAdminOldPwd);
    
    iRet = snmp.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (*pOrgHandleInner)(mp_void);
    Stub<pOrgHandleInner, pStubIntType, mp_void> stubCChgSnmp(&CChgSnmp::HandleInner, &stub_return_ret);   

    iRet = snmp.Handle();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = snmp.Handle();
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CChgSnmpTest, HandleInner)
{
    mp_int32 iRet;
    CChgSnmp snmp;

    typedef SNMP_CHOOSE_TYPE (*pOrgGetChoice)(mp_void);
    Stub<pOrgGetChoice, pStubIntType, mp_void> stubCChgSnmp(&CChgSnmp::GetChoice, &stub_return_number);

    typedef mp_bool (*pOrgCheckSNMPPwd)(PASSWOD_TYPE eType);
    Stub<pOrgCheckSNMPPwd, pStubBoolType, mp_void> stubCChgSnmp8(&CChgSnmp::CheckSNMPPwd, &stub_return_bool);
    
    typedef mp_int32 (*pOrgChgPwd)(PASSWOD_TYPE eType);
    Stub<pOrgChgPwd, pStubIntType, mp_void> stubCPassword(&CPassword::ChgPwd, &stub_return_ret);
    
    typedef mp_int32 (*pOrgSetx)(mp_void);
    Stub<pOrgSetx, pStubIntType, mp_void> stubCChgSnmp1(&CChgSnmp::ChgAuthProtocol, &stub_return_ret);
    Stub<pOrgSetx, pStubIntType, mp_void> stubCChgSnmp2(&CChgSnmp::ChgPrivateProtocol, &stub_return_ret);
    Stub<pOrgSetx, pStubIntType, mp_void> stubCChgSnmp3(&CChgSnmp::ChgSecurityName, &stub_return_ret);
    Stub<pOrgSetx, pStubIntType, mp_void> stubCChgSnmp4(&CChgSnmp::ChgSecurityLevel, &stub_return_ret);
    Stub<pOrgSetx, pStubIntType, mp_void> stubCChgSnmp5(&CChgSnmp::ChgSecurityModel, &stub_return_ret);
    Stub<pOrgSetx, pStubIntType, mp_void> stubCChgSnmp6(&CChgSnmp::ChgContextEngineID, &stub_return_ret);
    Stub<pOrgSetx, pStubIntType, mp_void> stubCChgSnmp7(&CChgSnmp::ChgContextName, &stub_return_ret);
    
    mp_int32 ret_table[SNMP_CHOOSE_SET_BUTT + 1] = 
    {
        MP_SUCCESS,
        MP_SUCCESS,
        MP_FAILED, 
        MP_FAILED, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS, 
        MP_SUCCESS
    };
    
    for (mp_int32 i = 1; i <= SNMP_CHOOSE_SET_BUTT; i++)
    {
            iRet = snmp.HandleInner();
            printf("%d____%d_____%d\n", i, ret_table[i], iRet);
            EXPECT_EQ(ret_table[i], iRet);
    }
}


TEST_F(CChgSnmpTest, GetChoice)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    Stub<pOrgGetInput, StubGetInputType, mp_void> stubCPassword(&CPassword::GetInput, &stub_set_cpasswdString);

    iRet = snmp.GetChoice();
    EXPECT_EQ(SNMP_CHOOSE_SET_BUTT, iRet);

    iRet = snmp.GetChoice();
    EXPECT_EQ(SNMP_CHOOSE_SET_PRI_PASSWD, iRet);
}


TEST_F(CChgSnmpTest, ChgAuthProtocol)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    Stub<pOrgGetInput, StubGetInputType, mp_void> stubCPassword_GetInput(&CPassword::GetInput, &stub_set_cpasswdString);

    typedef mp_int32 (CConfigXmlParser::*pOrgSetValue)(mp_string strSection, mp_string strKey, mp_string strValue);
    Stub<pOrgSetValue, pStubIntType, mp_void> stubCConfigXmlParser(&CConfigXmlParser::SetValue, &stub_return_ret);

    iRet = snmp.ChgAuthProtocol();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgAuthProtocol();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgAuthProtocol();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgAuthProtocol();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgPrivateProtocol)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    Stub<pOrgGetInput, StubGetInputType, mp_void> stubCPassword_GetInput(&CPassword::GetInput, &stub_set_cpasswdString);    

    iRet = snmp.ChgPrivateProtocol();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgPrivateProtocol();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = snmp.ChgPrivateProtocol();
    EXPECT_EQ(ERROR_COMMON_READ_CONFIG_FAILED, iRet);
    
    iRet = snmp.ChgPrivateProtocol();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgSecurityName)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    Stub<pOrgGetInput, StubGetInputType, mp_void> stubCPassword_GetInput(&CPassword::GetInput, &stub_set_cpasswdLongString);

    iRet = snmp.ChgSecurityName();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = snmp.ChgSecurityName();
    EXPECT_EQ(ERROR_COMMON_READ_CONFIG_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgSecurityLevel)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    Stub<pOrgGetInput, StubGetInputType, mp_void> stubCPassword_GetInput(&CPassword::GetInput, &stub_set_cpasswdString);
    
    iRet = snmp.ChgSecurityLevel();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgSecurityLevel();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgSecurityModel)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    Stub<pOrgGetInput, StubGetInputType, mp_void> stubCPassword_GetInput(&CPassword::GetInput, &stub_set_cpasswdString);

    iRet = snmp.ChgSecurityModel();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgSecurityModel();
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgContextEngineID)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    Stub<pOrgGetInput, StubGetInputType, mp_void> stubCPassword_GetInput(&CPassword::GetInput, &stub_set_cpasswdString);

    iRet = snmp.ChgContextEngineID();
    EXPECT_EQ(ERROR_COMMON_READ_CONFIG_FAILED, iRet);

    iRet = snmp.ChgContextEngineID();
    EXPECT_EQ(ERROR_COMMON_READ_CONFIG_FAILED, iRet);
}


TEST_F(CChgSnmpTest, ChgContextName)
{
    mp_int32 iRet;
    CChgSnmp snmp;
    Stub<pOrgGetInput, StubGetInputType, mp_void> stubCPassword_GetInput(&CPassword::GetInput, &stub_set_cpasswdLongString);

    iRet = snmp.ChgContextName();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = snmp.ChgContextName();
    EXPECT_EQ(ERROR_COMMON_READ_CONFIG_FAILED, iRet);
}



