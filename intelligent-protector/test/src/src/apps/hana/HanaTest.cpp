/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/hana/HanaTest.h"

static mp_int32 g_iHanaCount = 0;

mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}

mp_int32 StubHanaRootCallerExec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (g_iHanaCount++ == 0)
    {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 StubHanaRootCallerExecReturnVecter(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (g_iHanaCount++ == 0)
    {
        return MP_FAILED;
    }
    pvecResult->push_back("0");
    return MP_SUCCESS;
}


typedef mp_int32 (*CRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);

TEST_F(CHanaTest, BuildScriptParam)
{
    CHana hana;
    hana_db_info_t dbInfo;
    mp_string strOperType;
    mp_string strParam;

    dbInfo.strDbName = "123";
    dbInfo.strDBPassword = "123";
    dbInfo.strDBUsername = "123";
    dbInfo.strInstNum = "123";
    strOperType = "xxx";
    
    hana.BuildScriptParam(dbInfo, strOperType, strParam);  //这两个函数没有返回值，只有组装字符串，没有异常场景

    hana.BuildScriptParam(dbInfo, strParam);
}

TEST_F(CHanaTest, StartDB)
{
    CHana hana;
    hana_db_info_t dbInfo;
    g_iHanaCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubHanaRootCallerExec);
    mp_int32 rst = hana.StartDB(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = hana.StartDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CHanaTest, StopDB)
{
    CHana hana;
    hana_db_info_t dbInfo;
    g_iHanaCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubHanaRootCallerExec);
    mp_int32 rst = hana.StopDB(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = hana.StopDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CHanaTest, Test)
{
    CHana hana;
    hana_db_info_t dbInfo;
    g_iHanaCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubHanaRootCallerExec);
    mp_int32 rst = hana.Test(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = hana.Test(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CHanaTest, FreezeDB)
{
    CHana hana;
    hana_db_info_t dbInfo;
    g_iHanaCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubHanaRootCallerExec);
    mp_int32 rst = hana.FreezeDB(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = hana.FreezeDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CHanaTest, ThawDB)
{
    CHana hana;
    hana_db_info_t dbInfo;
    g_iHanaCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubHanaRootCallerExec);
    mp_int32 rst = hana.ThawDB(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = hana.ThawDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CHanaTest, GetFreezeStatus)
{
    CHana hana;
    hana_db_info_t dbInfo;
    mp_int32 istate;
    g_iHanaCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubHanaRootCallerExecReturnVecter);
    mp_int32 rst = hana.GetFreezeStatus(dbInfo, istate);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    
    rst = hana.GetFreezeStatus(dbInfo, istate);
    EXPECT_EQ(rst, MP_SUCCESS);
}

