/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/hana/HanaPluginTest.h"

static int iHanaCounter = 0;
static int iHanaRet = 0;

static std::map<mp_string, mp_string> urlMapObj;
static std::map<mp_string, mp_string>& stubHanaGetQueryParam(mp_void)
{
    static int iCounter = 0;
    if (iCounter++ == 0)
    {
        return urlMapObj;
    }
    
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_HANA_INSTNUM, "HanaInstance"));
    urlMapObj.insert(map<mp_string, mp_string>::value_type(REST_PARAM_HANA_DBNAME,   "HanaDB"));
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

static int g_iHanaCounter1 = 0;
static int g_iHanaCounter2 = 0;
static int g_iHanaJsonStringCounter = 0;

static mp_int32 HanaStubJsonString(const Json::Value& jsValue, mp_string strKey, mp_string& strValue)
{
    if (g_iHanaJsonStringCounter++ == 0)
    {
        strValue = "";
        return MP_SUCCESS;
    }
    strValue = "test123";
    
    return MP_SUCCESS;
}

mp_int32 HanaStubReturnRet1(mp_void)
{
    if (g_iHanaCounter1++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

mp_int32 HanaStubReturnRet2(mp_void)
{
    if (g_iHanaCounter2++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

TEST_F(CHanaPluginTest, DoAction)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHanaPlugin plugObj;

    iRet = plugObj.DoAction(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);
}

TEST_F(CHanaPluginTest, GetDBAuthParam)    /*1. 用户名为空密码为空 2. 用户名为空密码不空 3. 用户名不为空密码为空 4. 都不空*/
{
    CRequestMsg req;
    CResponseMsg rsp;
    hana_db_info_t stDBInfo;
    mp_int32 iRet = MP_SUCCESS;
    CHanaPlugin plugObj;

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

TEST_F(CHanaPluginTest, StartDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHanaPlugin plugObj;
    g_iHanaJsonStringCounter = 0;             //1. 参数为空的情况
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &HanaStubJsonString);

    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    g_iHanaCounter1 = 0;                       //2. 校验用户名密码错误
    typedef mp_int32 (CHanaPlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, hana_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCHanaPlugin1(&CHanaPlugin::GetDBAuthParam, &HanaStubReturnRet1);
    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iHanaCounter2 = 0;
    typedef mp_int32 (CHana::*pOrgStartHana)(hana_db_info_t &stDBInfo);
    Stub<pOrgStartHana, pStubIntType, mp_void> stubCHanaPlugin2(&CHana::StartDB, &HanaStubReturnRet2);
    iRet = plugObj.StartDB(&req, &rsp);         //3. 执行启动错误
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.StartDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHanaPluginTest, StopDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHanaPlugin plugObj;
    
    g_iHanaJsonStringCounter = 0;
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &HanaStubJsonString);
    
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    g_iHanaCounter1 = 0;
    typedef mp_int32 (CHanaPlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, hana_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCHanaPlugin1(&CHanaPlugin::GetDBAuthParam, &HanaStubReturnRet1);
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iHanaCounter2 = 0;   
    typedef mp_int32 (CHana::*pOrgStopHana)(hana_db_info_t &stDBInfo);
    Stub<pOrgStopHana, pStubIntType, mp_void> stubCHanaPlugin2(&CHana::StopDB, &HanaStubReturnRet2);
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHanaPluginTest, Test)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHanaPlugin plugObj;
    
    g_iHanaJsonStringCounter = 0;
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &HanaStubJsonString);
    
    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    g_iHanaCounter1 = 0;
    typedef mp_int32 (CHanaPlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, hana_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCHanaPlugin1(&CHanaPlugin::GetDBAuthParam, &HanaStubReturnRet1);
    iRet = plugObj.StopDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iHanaCounter2 = 0;   
    typedef mp_int32 (CHana::*pOrgTestHana)(hana_db_info_t &stDBInfo);
    Stub<pOrgTestHana, pStubIntType, mp_void> stubCHanaPlugin2(&CHana::Test, &stub_return_ret);
    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

}

TEST_F(CHanaPluginTest, FreezeDB)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHanaPlugin plugObj;
    g_iHanaJsonStringCounter = 0;
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &HanaStubJsonString);
    
    iRet = plugObj.FreezeDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    g_iHanaCounter1 = 0;
    typedef mp_int32 (CHanaPlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, hana_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCHanaPlugin1(&CHanaPlugin::GetDBAuthParam, &HanaStubReturnRet1);
    iRet = plugObj.FreezeDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iHanaCounter2 = 0;  

    typedef mp_int32 (CHana::*pOrgFreezeHana)(hana_db_info_t &stDBInfo);
    Stub<pOrgFreezeHana, pStubIntType, mp_void> stubCHanaPlugin2(&CHana::FreezeDB, &HanaStubReturnRet2);
    iRet = plugObj.FreezeDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.FreezeDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHanaPluginTest, Thaw)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHanaPlugin plugObj;
    g_iHanaJsonStringCounter = 0;
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubGetJsonString, mp_void>(&CJsonUtils::GetJsonString, &HanaStubJsonString);
    
    iRet = plugObj.ThawDB(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    g_iHanaCounter1 = 0;
    typedef mp_int32 (CHanaPlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, hana_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCHanaPlugin1(&CHanaPlugin::GetDBAuthParam, &HanaStubReturnRet1);
    iRet = plugObj.ThawDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    g_iHanaCounter2 = 0;   
    typedef mp_int32 (CHana::*pOrgThawHana)(hana_db_info_t &stDBInfo);
    Stub<pOrgThawHana, pStubIntType, mp_void> stubCHanaPlugin2(&CHana::ThawDB, &HanaStubReturnRet2);
    iRet = plugObj.ThawDB(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.ThawDB(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

}

TEST_F(CHanaPluginTest, GetFreezeStatus)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHanaPlugin plugObj;
    g_iHanaCounter1 = 0;

    typedef std::map<mp_string, mp_string>& (CRequestURL::*pOrgHanaGetQueryParam)(mp_void);
    typedef std::map<mp_string, mp_string>& (*pstubHanaGetQueryParam)(mp_void);
    Stub<pOrgHanaGetQueryParam, pstubHanaGetQueryParam, mp_void> stubCHanaPlugin0(&CRequestURL::GetQueryParam, &stubHanaGetQueryParam);
    iRet = plugObj.GetFreezeStatus(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    g_iHanaCounter1 = 0;
    typedef mp_int32 (CHanaPlugin::*pOrgGetDBAuthParam)(CRequestMsg* req, hana_db_info_t &stDBInfo);
    Stub<pOrgGetDBAuthParam, pStubIntType, mp_void> stubCHanaPlugin1(&CHanaPlugin::GetDBAuthParam, &HanaStubReturnRet1);
    iRet = plugObj.GetFreezeStatus(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);   

    g_iHanaCounter2 = 0;
    typedef mp_int32 (CHana::*pOrgGetFreezeStatus)(hana_db_info_t &stdbInfo, mp_int32 &iFreezeState);
    Stub<pOrgGetFreezeStatus, pStubIntType, mp_void> stubCHanaPlugin2(&CHana::GetFreezeStatus, &HanaStubReturnRet2);
    iRet = plugObj.GetFreezeStatus(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.GetFreezeStatus(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

}


