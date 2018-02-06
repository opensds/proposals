/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/cluster/ClusterPluginTest.h"

static CRequestURL& stubGetURL(mp_void)
{
    static CRequestURL requestUrlObj;
    return requestUrlObj;
}


static std::map<mp_string, mp_string>& stubGetQueryParam(mp_void)
{
    static std::map<mp_string, mp_string> urlMapObj;
    
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_RESGRPNAME, "123"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_CLUSTERTYPE, "oracleCLuster"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_INSTNAME, "oracleInstance"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_DBNAME, "agent"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_CLUSTERTYPE, "666"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_APPTYPE, "555"));
    
    return urlMapObj;
}


static CHttpRequest& stubGetHttpReq(mp_void)
{
    static CHttpRequest RequestObj;
    
    RequestObj.m_strURL = "www.ohxf.com";
    RequestObj.m_strQueryParam = "oxhf"; 
    mp_string m_strMethod = "OK";
    FCGX_Request *m_pFcgRequest = new(std::nothrow) FCGX_Request;
    return RequestObj;
}


static mp_int32 StubQueryClusterInfo(void *ptr, db_info_t& stdbInfo, mp_string strClusterType, mp_string strDBType, vector<cluster_info_t>& vecClusterInfo)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        cluster_info_t cluster;
        cluster.strClusterName = "mscluster";
        cluster.strResGrpName = "group";
        cluster.vecResourceName.push_back("abc");
        cluster.strVgActiveMode = "asdf";
        cluster.strNetWorkName = "sqlsluster";
        
        vecClusterInfo.push_back(cluster);

        return MP_SUCCESS;
    }
}


static mp_int32  StartResouceGroup_bool_iCounter = 0;
static mp_int32  StopResouceGroup_bool_iCounter = 0;
static mp_int32  QueryActiveHost_rCounter = 0;
static mp_int32  StartResouceGroup_rCounter = 0;
static mp_int32  StartCluster_rCounter = 0;
static mp_int32  StopResouceGroup_rCounter = 0;
static bool  stub_return_bool_StartResouceGroup(void)
{
    if (StartResouceGroup_bool_iCounter <= 1)
    {
        StartResouceGroup_bool_iCounter++;
        return MP_FALSE;
    }
    else
    {
        StartResouceGroup_bool_iCounter++;
        return MP_TRUE;
    }
}
static bool  stub_return_bool_StopResouceGroup(void)
{
    return MP_TRUE;
}
static mp_int32  stub_return_QueryActiveHost (void)
{
    if (QueryActiveHost_rCounter <= 1)
    {  
        QueryActiveHost_rCounter++;
        return MP_FAILED;
    }
    QueryActiveHost_rCounter++;
    return MP_SUCCESS;
}
static mp_int32  stub_return_StartCluster(void)
{
    if (StartCluster_rCounter == 0)
    {
        StartCluster_rCounter++;
        return MP_FAILED;
    }
    StartCluster_rCounter++;
    return MP_SUCCESS;
}

static mp_int32  stub_return_StartResouceGroup(void)
{
    if (StartResouceGroup_rCounter++ == 0)
    {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

static mp_int32  stub_return_StopResouceGroup(void)
{
    if (StopResouceGroup_rCounter++ == 0)
    {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}


TEST_F(CClusterPluginTest, DoAction)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CClusterPlugin plugObj;

    iRet = plugObj.DoAction(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);
}

TEST_F(CClusterPluginTest, QueryActiveHost)
{
    FCGX_Request fcgiReq;
    CRequestMsg req;
    CResponseMsg rsp(&fcgiReq);
    mp_int32 iRet = MP_SUCCESS;
    CClusterPlugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_QueryActiveHost);
    plugObj.QueryActiveHost(&req, &rsp);

    typedef CRequestURL& (CRequestMsg::*pOrgGetURL)(mp_void);
    typedef CRequestURL& (*pStubGetURL)(mp_void);
    Stub<pOrgGetURL, pStubGetURL, mp_void> stubCRequestMsg(&CRequestMsg::GetURL, &stubGetURL);

    typedef std::map<mp_string, mp_string>& (CRequestURL::*pOrgGetQueryParam)(mp_void);
    typedef std::map<mp_string, mp_string>& (*pStubGetQueryParam)(mp_void);
    Stub<pOrgGetQueryParam, pStubGetQueryParam, mp_void> stubCRequestMsg1(&CRequestURL::GetQueryParam, &stubGetQueryParam);

    typedef mp_int32 (CCluster::*pOrgIsActiveNode)(mp_string& strResGrpName, mp_int32 iClusterType, mp_bool& bIsActive);
    Stub<pOrgIsActiveNode, pStubIntType, mp_void> stubCCluster(&CCluster::IsActiveNode, &stub_return_QueryActiveHost);

    iRet = plugObj.QueryActiveHost(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.QueryActiveHost(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

static mp_void StubGetDBInfo(void *ptr, map<mp_string, mp_string>& vreqal, db_info_t &stdbInfo, mp_string &strDBType, mp_string &strClusterType)
{
    strDBType = "1"; 
}

TEST_F(CClusterPluginTest, QueryClusterInfo)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CClusterPlugin plugObj;

    typedef CRequestURL& (CRequestMsg::*pOrgGetURL)(mp_void);
    typedef CRequestURL& (*pStubGetURL)(mp_void);
    Stub<pOrgGetURL, pStubGetURL, mp_void> stubCRequestMsg(&CRequestMsg::GetURL, &stubGetURL);

    typedef CHttpRequest& (CRequestMsg::*pOrgGetHttpReq)(mp_void);
    typedef CHttpRequest& (*pStubGetHttpReq)(mp_void);
    Stub<pOrgGetHttpReq, pStubGetHttpReq, mp_void> stubCRequestMsg1(&CRequestMsg::GetHttpReq, &stubGetHttpReq);

    typedef mp_int32 (CClusterPlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, mp_string &strdbUsername, mp_string &strdbPassword);
    Stub<pOrgGetDBAuthParam, pStubVoidType, mp_void> stubCClusterPlugin(&CClusterPlugin::GetDBAuthParam, &stub_return_nothing);
    
    typedef mp_void (CClusterPlugin::*pOrgGetDBInfo)(map<mp_string, mp_string>& vreqal, db_info_t &stdbInfo, mp_string &strDBType, mp_string &strClusterType);
    typedef mp_void (*pStubGetDBInfo)(void *ptr, map<mp_string, mp_string>& vreqal, db_info_t &stdbInfo, mp_string &strDBType, mp_string &strClusterType);
    Stub<pOrgGetDBInfo, pStubGetDBInfo, mp_void> stubCClusterPlugin01(&CClusterPlugin::GetDBInfo, &StubGetDBInfo);

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.QueryClusterInfo(&req, &rsp);

    typedef std::map<mp_string, mp_string>& (CRequestURL::*pOrgGetQueryParam)(mp_void);
    typedef std::map<mp_string, mp_string>& (*pStubGetQueryParam)(mp_void);
    Stub<pOrgGetQueryParam, pStubGetQueryParam, mp_void> stubCRequestMsg2(&CRequestURL::GetQueryParam, &stubGetQueryParam);
    
    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg3(&CHttpRequest::GetHead, &stub_return_cstring);

    typedef mp_int32 (CCluster::*pOrgQueryClusterInfo)(db_info_t& stdbInfo, mp_string strClusterType, mp_string strDBType, vector<cluster_info_t>& vecClusterInfo);
    typedef mp_int32 (*pStubQueryClusterInfo)(void *ptr, db_info_t& stdbInfo, mp_string strClusterType, mp_string strDBType, vector<cluster_info_t>& vecClusterInfo);
    Stub<pOrgQueryClusterInfo, pStubQueryClusterInfo, mp_void> stubCCluster(&CCluster::QueryClusterInfo, &StubQueryClusterInfo);
    plugObj.QueryClusterInfo(&req, &rsp);

    plugObj.QueryClusterInfo(&req, &rsp);
}


TEST_F(CClusterPluginTest, StartCluster)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CClusterPlugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_StartCluster);
    plugObj.StartCluster(&req, &rsp);

    iRet = plugObj.StartCluster(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.StartCluster(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
}



TEST_F(CClusterPluginTest, StartResouceGroup)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CClusterPlugin plugObj;
    Json::Value myValue(5);

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_StartResouceGroup);
    plugObj.StartResouceGroup(&req, &rsp);
    
    typedef bool (Json::Value::*pOrgIsMemory)(const char *key) const;
    typedef bool (*pStub_return_bool_innser)(void);
    Stub<pOrgIsMemory, pStub_return_bool_innser, void> stubJson1(&Json::Value::isMember, &stub_return_bool_StartResouceGroup);

    iRet = plugObj.StartResouceGroup(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    iRet = plugObj.StartResouceGroup(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);   
    
    iRet = plugObj.StartResouceGroup(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
}


TEST_F(CClusterPluginTest, StopResouceGroup)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CClusterPlugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_StopResouceGroup);
    plugObj.StopResouceGroup(&req, &rsp);

    typedef bool (Json::Value::*pOrgIsMemory)(const char *key) const;
    typedef bool (*pStub_return_bool_innser)(void);
    Stub<pOrgIsMemory, pStub_return_bool_innser, void> stubJson1(&Json::Value::isMember, &stub_return_bool_StopResouceGroup);

    typedef mp_int32 (CCluster::*pOrgStopResGrp)(mp_string strResGrpName, vector<mp_string> vecDevGrpName, mp_string strClusterType, mp_string strDBType, vector<mp_string> &vecResourceName);
    Stub<pOrgStopResGrp, pStubIntType, void> stubCClusterPlugin(&CCluster::StopResGrp, &stub_return_StopResouceGroup);
    
    iRet = plugObj.StopResouceGroup(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.StopResouceGroup(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = plugObj.StopResouceGroup(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CClusterPluginTest, GetDBAuthParam)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_string strdbUsername;
    mp_string strdbPassword;
    mp_int32 iRet = MP_SUCCESS;
    CClusterPlugin plugObj;

    typedef const mp_char* (CHttpRequest::*pOrgGetHeadNoCheck)(const mp_char* name);
    Stub<pOrgGetHeadNoCheck, pStubCstringType, void> stubCHttpRequest(&CHttpRequest::GetHeadNoCheck, stub_return_cstring);
    plugObj.GetDBAuthParam(&req, strdbUsername, strdbPassword);

    plugObj.GetDBAuthParam(&req, strdbUsername, strdbPassword);
}

TEST_F(CClusterPluginTest, GetDBInfo)
{
    std::map<mp_string, mp_string> vreqal;
    db_info_t stdbInfo;
    mp_string strDBType;
    mp_string strClusterType;
    mp_int32 iRet = MP_SUCCESS;
    CClusterPlugin plugObj;

    vreqal.insert(std::map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_INSTNAME, "instance"));
    vreqal.insert(std::map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_DBNAME, "dbname"));
    vreqal.insert(std::map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_CLUSTERTYPE, "sqlserver"));
    vreqal.insert(std::map<mp_string, mp_string>::value_type(REST_PARAM_CLUSTER_APPTYPE, "xx"));

    plugObj.GetDBInfo(vreqal, stdbInfo, strDBType, strClusterType);
}


