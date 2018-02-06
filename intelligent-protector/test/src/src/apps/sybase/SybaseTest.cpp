/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/sybase/SybaseTest.h"

static mp_int32 g_iSybaseCount = 0;

mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}

mp_int32 StubSybaseRootCallerExec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (g_iSybaseCount++ == 0)
    {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 StubSybaseRootCallerExecReturnVecter(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (g_iSybaseCount++ == 0)
    {
        return MP_FAILED;
    }
    pvecResult->push_back("0");
    return MP_SUCCESS;
}


typedef mp_int32 (*CRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);

TEST_F(CSybaseTest, BuildScriptParam)
{
    CSybase sybase;
    sybase_db_info_t dbInfo;
    mp_string strOperType;
    mp_string strParam;

    dbInfo.strdbName = "123";
    dbInfo.strDBPassword = "123";
    dbInfo.strDBUsername = "123";
    dbInfo.strinstName = "123";
    strOperType = "xxx";
    
    sybase.BuildScriptParam(dbInfo, strOperType, strParam);  //这两个函数没有返回值，只有组装字符串，没有异常场景

    sybase.BuildScriptParam(dbInfo, strParam);
}

TEST_F(CSybaseTest, StartDB)
{
    CSybase sybase;
    sybase_db_info_t dbInfo;
    g_iSybaseCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubSybaseRootCallerExec);
    mp_int32 rst = sybase.StartDB(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = sybase.StartDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CSybaseTest, StopDB)
{
    CSybase sybase;
    sybase_db_info_t dbInfo;
    g_iSybaseCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubSybaseRootCallerExec);
    mp_int32 rst = sybase.StopDB(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = sybase.StopDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CSybaseTest, Test)
{
    CSybase sybase;
    sybase_db_info_t dbInfo;
    g_iSybaseCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubSybaseRootCallerExec);
    mp_int32 rst = sybase.Test(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = sybase.Test(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CSybaseTest, FreezeDB)
{
    CSybase sybase;
    sybase_db_info_t dbInfo;
    g_iSybaseCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubSybaseRootCallerExec);
    mp_int32 rst = sybase.FreezeDB(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = sybase.FreezeDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CSybaseTest, ThawDB)
{
    CSybase sybase;
    sybase_db_info_t dbInfo;
    g_iSybaseCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubSybaseRootCallerExec);
    mp_int32 rst = sybase.ThawDB(dbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = sybase.ThawDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CSybaseTest, GetFreezeStatus)
{
    CSybase sybase;
    sybase_db_info_t dbInfo;
    mp_int32 istate;
    g_iSybaseCount = 0;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubSybaseRootCallerExecReturnVecter);
    mp_int32 rst = sybase.GetFreezeStatus(dbInfo, istate);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    
    rst = sybase.GetFreezeStatus(dbInfo, istate);
    EXPECT_EQ(rst, MP_SUCCESS);
}

