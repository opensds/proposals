/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/oracle/OraclePluginTest.h"

static int iOracleCounter = 0;
static int iOracleRet = 0;
static int iOracleCDBCounter = 0;
    
static mp_int32 stubGetDBInfo(void *ptr, list<oracle_inst_info_t> &lstOracleInstInfo)
{
    if (iOracleCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        lstOracleInstInfo.push_back(oracle_inst_info_t());
        return MP_SUCCESS;
    }
}


mp_int32 stubGetDBLUNInfo(mp_void *ptr, oracle_db_info_t &stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        vecLUNInfos.push_back(oracle_lun_info_t());
        return MP_SUCCESS;
    }
}

static std::map<mp_string, mp_string> urlMapObj;
static std::map<mp_string, mp_string>& stubGetQueryParam(mp_void)
{
    urlMapObj.insert(map<mp_string, mp_string>::value_type(RESPOND_ORACLE_PARAM_INSTNAME, "oracleInstance"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(RESPOND_ORACLE_PARAM_DBNAME, "oracle"));
    
    return urlMapObj;
}


static const mp_char* stubGetHeadNoCheck(mp_void *ptr, const mp_char * name)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return NULL;
    }
    else
    {
        return "123";
    }
}


static mp_int32 stubGetInstances(mp_void *ptr, oracle_db_info_t& stDBInfo, list<oracle_inst_info_t>& lstOracleInsts)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        lstOracleInsts.push_back(oracle_inst_info_t());
        return MP_SUCCESS;
    }
}

static mp_int32 stubReturnRet(mp_void)
{
    if (iOracleRet++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}


static mp_int32 stubCheckCDB(void *ptr, oracle_db_info_t &stDBInfo, mp_int32& iCDBType)
{
    if (iOracleCDBCounter == 0)
    {
        iCDBType=0;
        iOracleCDBCounter++;
        return MP_SUCCESS;
    }
    else if (iOracleCDBCounter == 1)
    {
        iCDBType=1;
        iOracleCDBCounter++;
        return MP_SUCCESS;
    }
    else
    {
        return ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART;
    }
}


static mp_string stubGetSpecialQueryParam(mp_void)
{
    return "oracle12c";
}

static mp_int32 stubGetDBAuthParamTrue(mp_void)
{
    return MP_SUCCESS;
}

static mp_int32 stubGetDBAuthParamFalse(mp_void)
{
    return ERROR_COMMON_INVALID_PARAM;
}

mp_int32 stubGetPDBInfo(mp_void *ptr, oracle_pdb_req_info_t &stPdbReqInfo, vector<oracle_pdb_rsp_info_t> &vecOraclePdbInfo)
{
    static int iCounter = 0;
    oracle_pdb_rsp_info_t stOraclePdbInfo;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        stOraclePdbInfo.iConID = 0;
        stOraclePdbInfo.iStatus = 0;
        stOraclePdbInfo.strPdbName = "aaa";
        vecOraclePdbInfo.push_back(stOraclePdbInfo);
        return MP_SUCCESS;
    }
}

mp_int32 stubStartPluginDB(mp_void *ptr, oracle_pdb_req_info_t &stPdbReqInfo)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

mp_string StubGetSpecialQueryParam(const mp_string& strKey)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        mp_string strParam;
        return strParam;
    }
    else
    {   
        mp_string strParam("123");
        return strParam;
    }

}

mp_int32 StubGetJsonString(mp_void *ptr, const Json::Value& jsValue, mp_string strKey, mp_string& strValue)
{
    static int iCounter = 0;
    if (iCounter++ < 3)
    {
        return MP_FAILED;
    }
    else
    {   
        strValue = "123";
        return MP_SUCCESS;
    }
}

mp_int32 stub_return_ret_xxx(mp_void)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static const mp_char* stubPDBGetHeadNoCheck(mp_void *ptr, const mp_char * name)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return NULL;
    }
    else
    {
        return "123";
    }
}

TEST_F(COraclePluginTest, DoAction)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;

    iRet = plugObj.DoAction(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);
}

TEST_F(COraclePluginTest, QueryInfo)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    CRequestURL testurl;
    testurl.m_procURL = "test";
    testurl.m_oriURL = "test";
    testurl.m_id = "test";
    testurl.m_queryParam["instName"] = "cdb12c";

    typedef mp_int32 (COracle::*pOrgGetDBInfo)(list<oracle_inst_info_t> &lstOracleInstInfo);
    typedef mp_int32 (*pStubGetDBInfo)(void *ptr, list<oracle_inst_info_t> &lstOracleInstInfo);
    Stub<pOrgGetDBInfo, pStubGetDBInfo, mp_void> stubCOracle(&COracle::GetDBInfo, &stubGetDBInfo);
    
    iRet = plugObj.QueryInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
        
    iRet = plugObj.QueryInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);


    typedef mp_int32 (COracle::*pOrgCheckCDB)(oracle_db_info_t &stDBInfo, mp_int32& iCDBType);
    typedef mp_int32 (*pStubCheckCDB)(void *ptr, oracle_db_info_t &stDBInfo, mp_int32& iCDBType);
    Stub<pOrgCheckCDB, pStubCheckCDB, mp_void> stubCOracle1(&COracle::CheckCDB, &stubCheckCDB);
    
    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle4(&COraclePlugin::GetDBAuthParam, &stubGetDBAuthParamTrue);

    req.m_url = testurl;
    Json::Value jvalue;
    jvalue["test"] = "test";
    rsp.m_msgJsonData = jvalue;
    iRet = plugObj.QueryInfo(&req, &rsp);
    EXPECT_EQ(ORACLE_TYPE_NON_CDB, rsp.m_msgJsonData["type"].asInt());
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = plugObj.QueryInfo(&req, &rsp);
    EXPECT_EQ(ORACLE_TYPE_CDB, rsp.m_msgJsonData["type"].asInt() );
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = plugObj.QueryInfo(&req, &rsp);
    EXPECT_EQ(ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART, iRet);

    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle3(&COraclePlugin::GetDBAuthParam, &stubGetDBAuthParamFalse);
    iRet = plugObj.QueryInfo(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
}


TEST_F(COraclePluginTest, QueryLunInfo)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle(&COraclePlugin::GetDBAuthParam, &stub_return_ret);

    iRet = plugObj.QueryLunInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgGetDBLUNInfo)(oracle_db_info_t &stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos);
    typedef mp_int32 (*pStubGetDBLUNInfo)(mp_void *ptr, oracle_db_info_t &stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos);
    Stub<pOrgGetDBLUNInfo, pStubGetDBLUNInfo, mp_void> stubCOracle1(&COracle::GetDBLUNInfo, &stubGetDBLUNInfo);

    iRet = plugObj.QueryLunInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.QueryLunInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, StartDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    typedef bool (Json::Value::*pOrgisMember)( const char *key ) const;
    Stub<pOrgisMember, pStubBoolType, mp_void> stubValue(&Json::Value::isMember, &stub_return_bool);

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOraclePlugin(&COraclePlugin::GetDBAuthParam, &stub_return_ret);
    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (*pOrgCheckParamInteger32)(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude);
    Stub<pOrgCheckParamInteger32, pStubIntType, mp_void> stubCOraclePluginx1(CheckParamInteger32, &stub_return_ret);
    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle0(&COraclePlugin::GetDBAuthParam, &stubReturnRet);
    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);


    typedef mp_int32 (COracle::*pOrgStartOracleInstance)(oracle_db_info_t &stDBInfo);
    Stub<pOrgStartOracleInstance, pStubIntType, mp_void> stubCOracle1(&COracle::StartOracleInstance, &stub_return_ret);
    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, StopDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    typedef bool (Json::Value::*pOrgisMember)( const char *key ) const;
    Stub<pOrgisMember, pStubBoolType, mp_void> stubValue(&Json::Value::isMember, &stub_return_bool);
    
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOraclePlugin(&COraclePlugin::GetDBAuthParam, &stubReturnRet);
    
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);    

    typedef mp_int32 (COracle::*pOrgStopOracleInstance)(oracle_db_info_t &stDBInfo);
    Stub<pOrgStopOracleInstance, pStubIntType, mp_void> stubCOracle(&COracle::StopOracleInstance, &stub_return_ret);
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, Test)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;
    
    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOraclePlugin(&COraclePlugin::GetDBAuthParam, &stubReturnRet);

    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgTest)(oracle_db_info_t &stDBInfo);
    Stub<pOrgTest, pStubIntType, mp_void> stubCOracle(&COracle::Test, &stub_return_ret);
    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, CheckArchiveThreshold)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    iRet = plugObj.CheckArchiveThreshold(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle0(&COraclePlugin::GetDBAuthParam, &stubReturnRet);
    iRet = plugObj.CheckArchiveThreshold(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    iRet = plugObj.CheckArchiveThreshold(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgCheckArchiveThreshold)(oracle_db_info_t &stDBInfo);
    Stub<pOrgCheckArchiveThreshold, pStubIntType, mp_void> stubCOracle1(&COracle::CheckArchiveThreshold, &stub_return_ret);
    iRet = plugObj.CheckArchiveThreshold(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);  
}

TEST_F(COraclePluginTest, Freeze)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle0(&COraclePlugin::GetDBAuthParam, &stubReturnRet);

    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgFreeze)(oracle_db_info_t &stDBInfo);
    Stub<pOrgFreeze, pStubIntType, mp_void> stubCOracle1(&COracle::Freeze, &stub_return_ret);
    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);   
}

TEST_F(COraclePluginTest, Thaw)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    iRet = plugObj.Thaw(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle0(&COraclePlugin::GetDBAuthParam, &stubReturnRet);  
    
    iRet = plugObj.Thaw(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
      
    iRet = plugObj.Thaw(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgThaw)(oracle_db_info_t &stDBInfo);
    Stub<pOrgThaw, pStubIntType, mp_void> stubCOracle1(&COracle::Thaw, &stub_return_ret);    
    iRet = plugObj.Thaw(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, ArchiveDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    iRet = plugObj.ArchiveDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
         
    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle0(&COraclePlugin::GetDBAuthParam, &stubReturnRet);   
    
    iRet = plugObj.ArchiveDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.ArchiveDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgArchiveDB)(oracle_db_info_t &stDBInfo);
    Stub<pOrgArchiveDB, pStubIntType, mp_void> stubCOracle1(&COracle::ArchiveDB, &stub_return_ret);       
    iRet = plugObj.ArchiveDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, StartASMInstance)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    iRet = plugObj.StartASMInstance(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet); 

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle0(&COraclePlugin::GetDBAuthParam, &stubReturnRet);       
    
    iRet = plugObj.StartASMInstance(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.StartASMInstance(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgStartASMInstance)(oracle_db_info_t &stDBInfo);
    Stub<pOrgStartASMInstance, pStubIntType, mp_void> stubCOracle1(&COracle::StartASMInstance, &stub_return_ret); 
    iRet = plugObj.StartASMInstance(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, StopASMInstance)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0;

    iRet = plugObj.StopASMInstance(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle0(&COraclePlugin::GetDBAuthParam, &stubReturnRet);  
    
    iRet = plugObj.StopASMInstance(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.StopASMInstance(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgStopASMInstance)(oracle_db_info_t &stDBInfo);
    Stub<pOrgStopASMInstance, pStubIntType, mp_void> stubCOracle1(&COracle::StopASMInstance, &stub_return_ret); 
    iRet = plugObj.StopASMInstance(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, StartRACCluster)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;

    plugObj.StartRACCluster(&req, &rsp);
}

TEST_F(COraclePluginTest, GetDBFreezeState)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;

    typedef std::map<mp_string, mp_string>& (CRequestURL::*pOrgGetQueryParam)(mp_void);
    typedef std::map<mp_string, mp_string>& (*pStubGetQueryParam)(mp_void);
    Stub<pOrgGetQueryParam, pStubGetQueryParam, mp_void> stubCRequestURL(&CRequestURL::GetQueryParam, &stubGetQueryParam);

    typedef mp_int32 (COraclePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, oracle_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCOracle0(&COraclePlugin::GetDBAuthParam, &stub_return_ret);  

    iRet = plugObj.GetDBFreezeState(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.GetDBFreezeState(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgQueryFreezeState)(oracle_db_info_t &stDBInfo, mp_int32 &iFreezeState);
    Stub<pOrgQueryFreezeState, pStubIntType, mp_void> stubCOracle1(&COracle::QueryFreezeState, &stub_return_ret); 
    iRet = plugObj.GetDBFreezeState(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
    
}

TEST_F(COraclePluginTest, GetDBAuthParam)
{
    CRequestMsg req;
    CResponseMsg rsp;
    oracle_db_info_t stDBInfo;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;

    typedef const mp_char* (CHttpRequest::*pOrgGetHeadNoCheck)(const mp_char* name);
    typedef const mp_char* (*pStubGetHeadNoCheck)(mp_void *ptr, const mp_char * name);
    Stub<pOrgGetHeadNoCheck, pStubGetHeadNoCheck, mp_void> stubCHttpRequest(&CHttpRequest::GetHeadNoCheck, &stubGetHeadNoCheck);

    iRet = plugObj.GetDBAuthParam(&req, stDBInfo);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    iRet = plugObj.GetDBAuthParam(&req, stDBInfo);
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(COraclePluginTest, QueryPDBInfo)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;
    iOracleRet= 0;
    iOracleCounter = 0; 

    iRet = plugObj.QueryPDBInfo(&req, &rsp);  //inst_name is null
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    typedef mp_string (CRequestURL::*pOrgGetSpecialQueryParam)(const mp_string& strKey);
    typedef mp_string (*pStubGetSpecialQueryParam)(const mp_string& strKey); 
    Stub<pOrgGetSpecialQueryParam, pStubGetSpecialQueryParam, mp_void> stubCOracle1(&CRequestURL::GetSpecialQueryParam, &StubGetSpecialQueryParam);

    typedef mp_int32 (COraclePlugin::*pOrgGetPDBAuthParam)(CRequestMsg* req, oracle_pdb_req_info_t &stPDBInfo);
    Stub<pOrgGetPDBAuthParam, pStubIntType, mp_void> stubCOracle(&COraclePlugin::GetPDBAuthParam, &stub_return_ret);
    iRet = plugObj.QueryPDBInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (COracle::*pOrgGetPDBInfo)(oracle_pdb_req_info_t &stPdbReqInfo, vector<oracle_pdb_rsp_info_t> &vecOraclePdbInfo);
    typedef mp_int32 (*pStubGetPDBInfo)(mp_void *ptr, oracle_pdb_req_info_t &stPdbReqInfo, vector<oracle_pdb_rsp_info_t> &vecOraclePdbInfo);
    Stub<pOrgGetPDBInfo, pStubGetPDBInfo, mp_void> stubCOracle2(&COracle::GetPDBInfo, &stubGetPDBInfo);

    iRet = plugObj.QueryPDBInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.QueryPDBInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, GetPDBAuthParam)
{
    CRequestMsg req;
    CResponseMsg rsp;
    oracle_pdb_req_info_t stPDBInfo;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj;

    typedef const mp_char* (CHttpRequest::*pOrgGetHeadNoCheck)(const mp_char* name);
    typedef const mp_char* (*pStubGetHeadNoCheck)(mp_void *ptr, const mp_char * name);
    Stub<pOrgGetHeadNoCheck, pStubGetHeadNoCheck, mp_void> stubCHttpRequest(&CHttpRequest::GetHeadNoCheck, &stubPDBGetHeadNoCheck);

    iRet = plugObj.GetPDBAuthParam(&req, stPDBInfo);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    iRet = plugObj.GetPDBAuthParam(&req, stPDBInfo);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(COraclePluginTest, StartPDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    COraclePlugin plugObj; 

    iRet = plugObj.StartPDB(&req, &rsp);  //inst_name is null
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.StartPDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    Json::Value jvalue;
    jvalue["pdbName"] = "xxx";
    jvalue["oracleHome"] = "xxx";
    jvalue["instName"] = "xxx";

    CRequestMsgBody reqBody;
    reqBody.m_msgJsonData = jvalue;
    req.m_msgBody = reqBody;

    iRet = plugObj.StartPDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    typedef mp_int32 (COraclePlugin::*pOrgGetPDBAuthParam)(CRequestMsg* req, oracle_pdb_req_info_t &stPDBInfo);
    Stub<pOrgGetPDBAuthParam, pStubIntType, mp_void> stubCOracle(&COraclePlugin::GetPDBAuthParam, &stub_return_ret_xxx);
        
    typedef mp_int32 (COracle::*pOrgStartPluginDB)(oracle_pdb_req_info_t &stPdbReqInfo);
    typedef mp_int32 (*pStubStartPluginDB)(mp_void *ptr, oracle_pdb_req_info_t &stPdbReqInfo);
    Stub<pOrgStartPluginDB, pStubStartPluginDB, mp_void> stubCOracle2(&COracle::StartPluginDB, &stubStartPluginDB);

    iRet = plugObj.StartPDB(&req, &rsp);
    
    iRet = plugObj.StartPDB(&req, &rsp);
}


