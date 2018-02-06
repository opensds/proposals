/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/db2/Db2Test.h"

TEST_F(CDB2Test, GetInfo)
{
    CDB2 db;
    vector<db2_inst_info_t> vecdbInstInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecDbinfo);
    mp_int32 rst = db.GetInfo(vecdbInstInfo);
    EXPECT_EQ(rst ,MP_SUCCESS);
}
TEST_F(CDB2Test, GetLunInfo)
{
    CDB2 db;
    db2_db_info_t dbinfo;
    dbinfo.strinstName = "db2inst1";
    dbinfo.strdbName = "db_sss";
    dbinfo.strdbUsername = "db2inst1";
    dbinfo.strdbPassword = "huawei";
    vector<db2_lun_info_t> vecLunInfos;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecDbLuninfo);
    Stub<CArrayGetArrayVendorAndProductType, StubCArrayGetArrayVendorAndProductType, mp_void> mystub2(&CArray::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOk);
    Stub<CArrayGetArraySNType, StubCArrayGetArraySNType, mp_void> mystub3(&CArray::GetArraySN, &StubCArrayGetArraySNEq0);
    Stub<CArrayGetLunInfoType, StubCArrayGetLunInfoType, mp_void> mystub4(&CArray::GetLunInfo, &StubCArrayGetLunInfoEq0);
    mp_int32 rst = db.GetLunInfo(dbinfo, vecLunInfos);
    EXPECT_EQ(rst ,MP_SUCCESS);
}
TEST_F(CDB2Test, Start)
{
    CDB2 db;
    db2_db_info_t dbinfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = db.Start(dbinfo);
    EXPECT_EQ(rst ,MP_SUCCESS);
}
TEST_F(CDB2Test, Stop)
{
    CDB2 db;
    db2_db_info_t dbinfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = db.Stop(dbinfo);
    EXPECT_EQ(rst ,MP_SUCCESS);
}
TEST_F(CDB2Test, Test)
{
    CDB2 db;
    db2_db_info_t dbinfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = db.Test(dbinfo);
    EXPECT_EQ(rst ,MP_SUCCESS);
}
TEST_F(CDB2Test, Freeze)
{
    CDB2 db;
    db2_db_info_t dbinfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = db.Freeze(dbinfo);
    EXPECT_EQ(rst ,MP_SUCCESS);
}
TEST_F(CDB2Test, UnFreeze)
{
    CDB2 db;
    db2_db_info_t dbinfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = db.UnFreeze(dbinfo);
    EXPECT_EQ(rst ,MP_SUCCESS);
}
TEST_F(CDB2Test, QueryFreezeState)
{
    CDB2 db;
    db2_db_info_t dbinfo;
    mp_int32 state;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = db.QueryFreezeState(dbinfo, state);
    EXPECT_EQ(rst ,MP_SUCCESS);
}
