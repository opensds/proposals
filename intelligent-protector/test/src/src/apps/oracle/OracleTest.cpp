/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/oracle/OracleTest.h"

TEST_F(COracleTest, GetDBInfo)
{
    COracle orl;
    list<oracle_inst_info_t> instInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetDBInfo);
    mp_int32 rst = orl.GetDBInfo(instInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, GetDBLUNInfo)
{
    COracle orl;
    oracle_db_info_t stDBInfo;
    vector<oracle_lun_info_t> vecLUNInfos;

    //typedef mp_int32 (COracle::*pOrgGetLunInfoByStorageType)(oracle_db_info_t stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType);
    //typedef mp_int32 (*pStubGetLunInfoByStorageType)(oracle_db_info_t stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType); 
    //Stub<pOrgGetLunInfoByStorageType, pStubGetLunInfoByStorageType, mp_void> stubCOracle1(&COracle::GetLunInfoByStorageType, &StubGetLunInfoByStorageType);

    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecGetDBLUNInfo);
    Stub<CArrayGetArrayVendorAndProductType, StubCArrayGetArrayVendorAndProductType, mp_void> mystub2(&CArray::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOk);
    Stub<CArrayGetArraySNType, StubCArrayGetArraySNType, mp_void> mystub3(&CArray::GetArraySN, &StubCArrayGetArraySNEq0);
    Stub<CArrayGetLunInfoType, StubCArrayGetLunInfoType, mp_void> mystub4(&CArray::GetLunInfo, &StubCArrayGetLunInfoEq0);
    stDBInfo.iGetArchiveLUN = 0;
    mp_int32 rst = orl.GetDBLUNInfo(stDBInfo, vecLUNInfos);
    stDBInfo.iGetArchiveLUN = 1;
    rst = orl.GetDBLUNInfo(stDBInfo, vecLUNInfos);
    EXPECT_EQ(rst, ERROR_COMMON_QUERY_APP_LUN_FAILED);

}
TEST_F(COracleTest, StartOracleInstance)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.StartOracleInstance(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, StopOracleInstance)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.StopOracleInstance(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, Test)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.Test(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, CheckArchiveThreshold)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.CheckArchiveThreshold(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, Freeze)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.Freeze(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, Thaw)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.Thaw(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, QueryFreezeState)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    mp_int32 istate;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.QueryFreezeState(dbInfo, istate);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, StartASMInstance)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.StartASMInstance(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, StopASMInstance)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.StopASMInstance(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, ArchiveDB)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.ArchiveDB(dbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, GetInstances)
{
    COracle orl;
    oracle_db_info_t stDBInfo;
    list<oracle_inst_info_t> lstOracleInsts;
    oracle_inst_info_t test;
    lstOracleInsts.push_back(test);
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetInstances);
    mp_int32 rst = orl.GetInstances(stDBInfo, lstOracleInsts);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, TruncateArchiveLog)
{
    COracle orl;
    oracle_db_info_t dbInfo;
    mp_time tm;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    mp_int32 rst = orl.TruncateArchiveLog(dbInfo, tm);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, AnalyseInstInfoScriptRst)
{
    COracle orl;
    vector<mp_string> vecResult;
    list<oracle_inst_info_t> lstOracleInstInfo;
    vecResult.push_back("test");
    mp_int32 rst = orl.AnalyseInstInfoScriptRst(vecResult, lstOracleInstInfo);
    vecResult[0] = "test;test";
    rst = orl.AnalyseInstInfoScriptRst(vecResult, lstOracleInstInfo);
    vecResult[0] = "test:test:test";
    rst = orl.AnalyseInstInfoScriptRst(vecResult, lstOracleInstInfo);
    vecResult[0] = "test:test:test:test";
    rst = orl.AnalyseInstInfoScriptRst(vecResult, lstOracleInstInfo);
    vecResult[0] = "test:test:test:test:test";
    rst = orl.AnalyseInstInfoScriptRst(vecResult, lstOracleInstInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, AnalyseLunInfoScriptRST)
{
    COracle orl;
    vector<mp_string> vecResult;
    vector<oracle_storage_script_info> vecDBStorageScriptInfo;
    vecResult.push_back("test");
    mp_int32 rst = orl.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    vecResult.push_back("test;test");
    rst = orl.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    vecResult.push_back("test;test;test");
    rst = orl.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    vecResult.push_back("test;test;test;test");
    rst = orl.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    vecResult.push_back("test;test;test;test;test");
    rst = orl.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    vecResult.push_back("test;test;test;test;test;test");
    rst = orl.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    vecResult.push_back("test;test;test;test;test;test;test");
    rst = orl.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    vecResult.push_back("test;test;test;test;test;test;test;test");
    rst = orl.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(COracleTest, GetUDEVInfo)
{
    COracle orl;
    mp_string strUdevRulesFileDir;
    mp_string strUdevName;
    mp_string strUdevResult;
    mp_string strUdevDeviceRecord;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetUDEVInfo);
    orl.GetUDEVInfo(strUdevRulesFileDir, strUdevName, strUdevResult, strUdevDeviceRecord);
    EXPECT_TRUE(1);
    
}

TEST_F(COracleTest, IsInstalled)
{
    COracle orl;
    mp_bool bIsInstalled;

    {
        orl.IsInstalled(bIsInstalled);
        EXPECT_TRUE(1);
    }

    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetUDEVInfo);
        orl.IsInstalled(bIsInstalled);
        EXPECT_TRUE(1);
    }

    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetUDEVInfo1);
        orl.IsInstalled(bIsInstalled);
        EXPECT_TRUE(1);
    }

    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetUDEVInfo2);
        orl.IsInstalled(bIsInstalled);
        EXPECT_TRUE(1);
    }
}

TEST_F(COracleTest, CheckLUNInfoExists)
{
    COracle orl;
    vector<oracle_lun_info_t> vecLUNInfos;
    oracle_lun_info_t oracle_lun_info;

    //vecLUNInfos.push_back("test");
    orl.CheckLUNInfoExists(vecLUNInfos,oracle_lun_info);
    EXPECT_TRUE(1);
    
}

TEST_F(COracleTest, GetAndCheckArraySN)
{
    COracle orl;
    mp_string strDev;
    mp_string strArraySN;
    mp_string strStorageType;

    orl.GetAndCheckArraySN(strDev,strArraySN,strStorageType);
    EXPECT_TRUE(1);

    strStorageType = DBADAPTIVE_PRAMA_MUST;
    orl.GetAndCheckArraySN(strDev,strArraySN,strStorageType);
    EXPECT_TRUE(1);
}

TEST_F(COracleTest, GetVendorAndProduct)
{
    COracle orl;
    mp_string strDev;
    mp_string strVendor;
    mp_string strProduct;
    mp_string strStorageType;

    orl.GetVendorAndProduct(strDev,strVendor,strProduct,strStorageType);
    EXPECT_TRUE(1);

    Stub<CArrayGetArrayVendorAndProductType, StubCArrayGetArrayVendorAndProductType, mp_void> mystub2(&CArray::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOk);
    orl.GetVendorAndProduct(strDev,strVendor,strProduct,strStorageType);
    EXPECT_TRUE(1);
}

TEST_F(COracleTest, AnalyseLunInfoByScriptRSTNoWIN)
{
    COracle orl;
    vector<oracle_storage_script_info> vecDBStorageScriptInfo;
    vector<oracle_lun_info_t> vecLUNInfos;
    mp_string strStorageType;

    //vecLUNInfos.push_back("test");

    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetUDEVInfo2);
        orl.AnalyseLunInfoByScriptRSTNoWIN(vecDBStorageScriptInfo,vecLUNInfos,strStorageType);
        EXPECT_TRUE(1);
    }

    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetUDEVInfo);
        orl.AnalyseLunInfoByScriptRSTNoWIN(vecDBStorageScriptInfo,vecLUNInfos,strStorageType);
        EXPECT_TRUE(1);
    }

    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetUDEVInfo);
        Stub<StrSplitType, StubStrSplitType, mp_void> mystub1(&CMpString::StrSplit, &StubStrSplit0);
        orl.AnalyseLunInfoByScriptRSTNoWIN(vecDBStorageScriptInfo,vecLUNInfos,strStorageType);
        EXPECT_TRUE(1);
    }

    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecGetUDEVInfo);
        Stub<StrSplitType, StubStrSplitType, mp_void> mystub1(&CMpString::StrSplit, &StubStrSplit);
        orl.AnalyseLunInfoByScriptRSTNoWIN(vecDBStorageScriptInfo,vecLUNInfos,strStorageType);
        EXPECT_TRUE(1);
    }
}

TEST_F(COracleTest, GetPDBInfo)
{
    COracle orl;
    oracle_pdb_req_info_t stPdbReqInfo;
    vector<oracle_pdb_rsp_info_t> vecOraclePdbInfo;  
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecGetPDBInfo);
    stPdbReqInfo.strInstName = "oracle12c";
    
    mp_int32 rst = orl.GetPDBInfo(stPdbReqInfo, vecOraclePdbInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);

    rst = orl.GetPDBInfo(stPdbReqInfo, vecOraclePdbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
    
    rst = orl.GetPDBInfo(stPdbReqInfo, vecOraclePdbInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(COracleTest, AnalysePDBInfoScriptRst)
{
    COracle orl;
    vector<mp_string> vecResult;
    vector<oracle_pdb_rsp_info_t> vecOraclePdbInfo;

    mp_int32 iRstNull = orl.AnalysePDBInfoScriptRst(vecResult, vecOraclePdbInfo);

    vector<mp_string> vecResult2;
    vecResult2.push_back("-1;pdbname;READ ONLY");
    mp_int32 iConIdInvalid = orl.AnalysePDBInfoScriptRst(vecResult2, vecOraclePdbInfo);

    vector<mp_string> vecResult3;
    vecResult3.push_back("1;;READ ONLY");
    mp_int32 iPdbNameNull = orl.AnalysePDBInfoScriptRst(vecResult3, vecOraclePdbInfo);

    vector<mp_string> vecResult4;
    vecResult4.push_back("1;pdbname;xx");
    mp_int32 iStatusInvalid = orl.AnalysePDBInfoScriptRst(vecResult4, vecOraclePdbInfo);

    vector<mp_string> vecResult5;
    vecResult5.push_back("1;pdbname;READ ONLY");
    mp_int32 iRet = orl.AnalysePDBInfoScriptRst(vecResult5, vecOraclePdbInfo);

    EXPECT_EQ(iRstNull, ERROR_SQLSERVER_DB_NOT_EXIST);
    EXPECT_EQ(iConIdInvalid, ERROR_SQLSERVER_DB_NOT_EXIST);
    EXPECT_EQ(iPdbNameNull, ERROR_SQLSERVER_DB_NOT_EXIST);
    EXPECT_EQ(iStatusInvalid, ERROR_SQLSERVER_DB_NOT_EXIST);
    EXPECT_EQ(iRet, MP_SUCCESS); 
} 

TEST_F(COracleTest, BuildPDBInfoScriptParam)
{
    COracle orl;
    oracle_pdb_req_info_t stPdbReqInfo;
    stPdbReqInfo.strDBPassword = "123456";
    stPdbReqInfo.strDBUsername = "xxx";
    stPdbReqInfo.strInstName = "aaa";
    stPdbReqInfo.strOracleHome = "bbb";
    stPdbReqInfo.strPdbName = "ccc";
    mp_string strParam("InstanceName=aaa:OracleHome=bbb:UserName=xxx:Password=123456:PDBName=ccc");
    mp_string strParamRST;
  
    orl.BuildPDBInfoScriptParam(stPdbReqInfo, strParamRST);
    mp_int32 rst = (strParamRST == strParam ? MP_SUCCESS : MP_FAILED);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(COracleTest, TranslatePDBStatus)
{
    COracle orl;
    mp_string strStatus;
    mp_int32 iStatus;

    strStatus = INIT_PDB_STATUS_READ_ONLY;
    mp_int32 iReadOnly = orl.TranslatePDBStatus(strStatus, iStatus);
    EXPECT_EQ(iStatus, PDB_READ_ONLY);

    strStatus = INIT_PDB_STATUS_READ_WRITE;
    mp_int32 iReadWrite = orl.TranslatePDBStatus(strStatus, iStatus);
    EXPECT_EQ(iStatus, PDB_READ_WRITE);
    
    strStatus = INIT_PDB_STATUS_MOUNTED;
    mp_int32 iMounted = orl.TranslatePDBStatus(strStatus, iStatus);
    EXPECT_EQ(iStatus, PDB_MOUNTED);

    strStatus = "error";
    mp_int32 iError = orl.TranslatePDBStatus(strStatus, iStatus);
    EXPECT_EQ(iError, MP_FAILED);
}

TEST_F(COracleTest, StartPluginDB)
{
    COracle orl;
    oracle_pdb_req_info_t stPdbReqInfo;
    vector<oracle_pdb_rsp_info_t> vecOraclePdbInfo;  
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecStartPDB);
    
    mp_int32 rst = orl.StartPluginDB(stPdbReqInfo);
    EXPECT_EQ(rst, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    
    rst = orl.StartPluginDB(stPdbReqInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(COracleTest, CheckCDB)
{
    COracle orl;
    mp_int32 vecResult;
    oracle_db_info_t dbInfo;
    mp_int32 iRet;

    //脚本返回成功，返回内容1
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecCheckCDB);
    iRet = orl.CheckCDB(dbInfo, vecResult);
    EXPECT_EQ(MP_SUCCESS, iRet);
    EXPECT_EQ(ORACLE_TYPE_CDB, vecResult);
    //脚本返回成功，返回内容0
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecCheckCDB2);
    iRet = orl.CheckCDB(dbInfo, vecResult);
    EXPECT_EQ(MP_SUCCESS, iRet);
    EXPECT_EQ(ORACLE_TYPE_NON_CDB, vecResult);
    //脚本返回错误码: 脚本数据库实例未启动
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub2(&CRootCaller::Exec, &StubCRootCallerExecCheckCDB3);
    iRet = orl.CheckCDB(dbInfo, vecResult);
    EXPECT_EQ(ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART, iRet);
}
