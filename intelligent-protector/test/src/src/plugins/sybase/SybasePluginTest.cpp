/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/sybase/SybasePluginTest.h"

static int iSybaseCounter = 0;
static int iSybaseRet = 0;

static std::map<mp_string, mp_string> urlMapObj;
static std::map<mp_string, mp_string>& stubSybaseGetQueryParam(mp_void)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return urlMapObj;
    }
    
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_SYBASAE_INSTNAME, "SybaseInstance"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_SYBASAE_DBNAME,   "SybaseDB"));
    return urlMapObj;
}


static const mp_char* stubGetHeadNoCheck(mp_void *ptr, const mp_char * name)
{                                                               
    static int iCounter = 0;
    iCounter++;
    switch (iCounter)
    {   //1. 用户名为空密码不空 -- ERROR_COMMON_INVALID_PARAM
        case 1: return NULL;
        case 2: return "123";
        //2. 用户名为空密码为空 -- success
        case 3: return NULL;
        case 4: return NULL;
        //3. 用户名不为空密码为空 -- sucess
        case 5: return "123";
        case 6: return NULL;
        //4. 都不空 -- success
        case 7: return "123";
        case 8: return "123";
        
        default: return "123";
    }
}

static int g_iSybaseCounter1 = 0;
static int g_iSybaseCounter2 = 0;
static int g_iSybaseJsonStringCounter = 0;

static mp_int32 SybaseStubJsonString(const Json::Value& jsValue, mp_string strKey, mp_string& strValue)
{
    if (g_iSybaseJsonStringCounter++ == 0)
    {
        strValue = "";
        return MP_SUCCESS;
    }
    strValue = "test123";
    
    return MP_SUCCESS;
}

mp_int32 SybaseStubReturnRet1(mp_void)
{
    if (g_iSybaseCounter1++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

mp_int32 SybaseStubReturnRet2(mp_void)
{
    if (g_iSybaseCounter2++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

TEST_F(CSybasePluginTest, DoAction)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CSybasePlugin plugObj;

    iRet = plugObj.DoAction(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);
}

TEST_F(CSybasePluginTest, GetDBAuthParam)    /*1. 用户名为空密码为空 2. 用户名为空密码不空 3. 用户名不为空密码为空 4. 都不空*/
{
    CRequestMsg req;
    CResponseMsg rsp;
    sybase_db_info_t stDBInfo;
    mp_int32 iRet = MP_SUCCESS;
    CSybasePlugin plugObj;

    typedef const mp_char* (CHttpRequest::*pOrgGetHeadNoCheck)(const mp_char* name);
    typedef const mp_char* (*pStubGetHeadNoCheck)(mp_void *ptr, const mp_char * name);
    Stub<pOrgGetHeadNoCheck, pStubGetHeadNoCheck, mp_void> stubCHttpRequest0(&CHttpRequest::GetHeadNoCheck, &stubGetHeadNoCheck);

    iRet = plugObj.GetDBAuthParam(&req, stDBInfo);     
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);   //1. 用户名为空密码不空 -- ERROR_COMMON_INVALID_PARAM
    
    iRet = plugObj.GetDBAuthParam(&req, stDBInfo); 
    EXPECT_EQ(MP_SUCCESS, iRet);                   //2. 用户名为空密码为空 -- success
    
    iRet = plugObj.GetDBAuthParam(&req, stDBInfo); 
    EXPECT_EQ(MP_SUCCESS, iRet);                   //3. 用户名不为空密码为空 -- sucess

    iRet = plugObj.GetDBAuthParam(&req, stDBInfo); 
    EXPECT_EQ(MP_SUCCESS, iRet);                   //4. 都不空 -- success
}  

typedef mp_int32 (*pOrgGetJsonString)(const Json::Value& jsValue, mp_string strKey, mp_string& strValue);
typedef mp_int32 (*pStubGetJsonString)(const Json::Value& jsValue, mp_string strKey, mp_string& strValue);
static Stub<pOrgGetJsonString, pStubGetJsonString, mp_void> *gp_GetJsonString;

TEST_F(CSybasePluginTest, StartDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CSybasePlugin plugObj;
    g_iSybaseJsonStringCounter = 0;             //1. 参数为空的情况
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &SybaseStubJsonString);

    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    g_iSybaseCounter1 = 0;                       //2. 校验用户名密码错误
    typedef mp_int32 (CSybasePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, sybase_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCSybasePlugin1(&CSybasePlugin::GetDBAuthParam, &SybaseStubReturnRet1);
    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iSybaseCounter2 = 0;
    typedef mp_int32 (CSybase::*pOrgStartSybase)(sybase_db_info_t &stDBInfo);
    Stub<pOrgStartSybase, pStubIntType, mp_void> stubCSybasePlugin2(&CSybase::StartDB, &SybaseStubReturnRet2);
    iRet = plugObj.StartDB(&req, &rsp);         //3. 执行启动错误
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CSybasePluginTest, StopDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CSybasePlugin plugObj;
    
    g_iSybaseJsonStringCounter = 0;
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &SybaseStubJsonString);
    
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    g_iSybaseCounter1 = 0;
    typedef mp_int32 (CSybasePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, sybase_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCSybasePlugin1(&CSybasePlugin::GetDBAuthParam, &SybaseStubReturnRet1);
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iSybaseCounter2 = 0;   
    typedef mp_int32 (CSybase::*pOrgStopSybase)(sybase_db_info_t &stDBInfo);
    Stub<pOrgStopSybase, pStubIntType, mp_void> stubCSybasePlugin2(&CSybase::StopDB, &SybaseStubReturnRet2);
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CSybasePluginTest, Test)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CSybasePlugin plugObj;
    
    g_iSybaseJsonStringCounter = 0;
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &SybaseStubJsonString);
    
    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    g_iSybaseCounter1 = 0;
    typedef mp_int32 (CSybasePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, sybase_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCSybasePlugin1(&CSybasePlugin::GetDBAuthParam, &SybaseStubReturnRet1);
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iSybaseCounter2 = 0;   
    typedef mp_int32 (CSybase::*pOrgTestSybase)(sybase_db_info_t &stDBInfo);
    Stub<pOrgTestSybase, pStubIntType, mp_void> stubCSybasePlugin2(&CSybase::Test, &stub_return_ret);
    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

}

TEST_F(CSybasePluginTest, FreezeDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CSybasePlugin plugObj;
    g_iSybaseJsonStringCounter = 0;
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &SybaseStubJsonString);
    
    iRet = plugObj.FreezeDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    g_iSybaseCounter1 = 0;
    typedef mp_int32 (CSybasePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, sybase_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCSybasePlugin1(&CSybasePlugin::GetDBAuthParam, &SybaseStubReturnRet1);
    iRet = plugObj.FreezeDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iSybaseCounter2 = 0;  

    typedef mp_int32 (CSybase::*pOrgFreezeSybase)(sybase_db_info_t &stDBInfo);
    Stub<pOrgFreezeSybase, pStubIntType, mp_void> stubCSybasePlugin2(&CSybase::FreezeDB, &SybaseStubReturnRet2);
    iRet = plugObj.FreezeDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.FreezeDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CSybasePluginTest, Thaw)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CSybasePlugin plugObj;
    g_iSybaseJsonStringCounter = 0;
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &SybaseStubJsonString);
    
    iRet = plugObj.ThawDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    g_iSybaseCounter1 = 0;
    typedef mp_int32 (CSybasePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, sybase_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCSybasePlugin1(&CSybasePlugin::GetDBAuthParam, &SybaseStubReturnRet1);
    iRet = plugObj.ThawDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iSybaseCounter2 = 0;   
    typedef mp_int32 (CSybase::*pOrgThawSybase)(sybase_db_info_t &stDBInfo);
    Stub<pOrgThawSybase, pStubIntType, mp_void> stubCSybasePlugin2(&CSybase::ThawDB, &SybaseStubReturnRet2);
    iRet = plugObj.ThawDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.ThawDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

}

TEST_F(CSybasePluginTest, GetFreezeStatus)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CSybasePlugin plugObj;
    g_iSybaseCounter1 = 0;

    typedef std::map<mp_string, mp_string>& (CRequestURL::*pOrgSybaseGetQueryParam)(mp_void);
    typedef std::map<mp_string, mp_string>& (*pstubSybaseGetQueryParam)(mp_void);
    Stub<pOrgSybaseGetQueryParam, pstubSybaseGetQueryParam, mp_void> stubCSybasePlugin0(&CRequestURL::GetQueryParam, &stubSybaseGetQueryParam);
    iRet = plugObj.GetFreezeStatus(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    g_iSybaseCounter1 = 0;
    typedef mp_int32 (CSybasePlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, sybase_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCSybasePlugin1(&CSybasePlugin::GetDBAuthParam, &SybaseStubReturnRet1);
    iRet = plugObj.GetFreezeStatus(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);   

    g_iSybaseCounter2 = 0;
    typedef mp_int32 (CSybase::*pOrgGetFreezeStatus)(sybase_db_info_t &stdbInfo, mp_int32 &iFreezeState);
    Stub<pOrgGetFreezeStatus, pStubIntType, mp_void> stubCSybasePlugin2(&CSybase::GetFreezeStatus, &SybaseStubReturnRet2);
    iRet = plugObj.GetFreezeStatus(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.GetFreezeStatus(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

}


