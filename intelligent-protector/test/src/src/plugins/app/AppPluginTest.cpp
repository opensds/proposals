/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/app/AppPluginTest.h"


TEST_F(CAppPluginTest, DoAction)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;
    
    iRet = plugObj.DoAction(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);
}


TEST_F(CAppPluginTest, AddAppInfosToResult)
{
    CAppPlugin plugObj;
    Json::Value jValResult;
    vector<app_info_t> vecAppInfos;
    mp_int32 iRet = MP_SUCCESS;

    vecAppInfos.push_back(app_info_t());

    typedef mp_int32 (CAppPlugin::*pOrgAddAppInfoToResult)(Json::Value& valResult, app_info_t& appInfo);
    Stub<pOrgAddAppInfoToResult, pStubIntType, mp_void> stubCAppPlugin(&CAppPlugin::AddAppInfoToResult, stub_return_ret);
    plugObj.AddAppInfosToResult(jValResult, vecAppInfos);

    plugObj.AddAppInfosToResult(jValResult, vecAppInfos);
}


TEST_F(CAppPluginTest, AddAppInfoToResult)
{
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;
    Json::Value valResult;
    app_info_t appInfo;

    appInfo.enAppType = APP_TYPE_BUTT;
    plugObj.AddAppInfoToResult(valResult, appInfo);

    appInfo.enAppType = APP_TYPE_ORACLE;
    plugObj.AddAppInfoToResult(valResult, appInfo);

    appInfo.enAppType = APP_TYPE_SQLSERVER;
    plugObj.AddAppInfoToResult(valResult, appInfo);
    
    appInfo.enAppType = APP_TYPE_EXCHANGE;
    plugObj.AddAppInfoToResult(valResult, appInfo);
    
    appInfo.enAppType = APP_TYPE_DB2;
    plugObj.AddAppInfoToResult(valResult, appInfo);
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
TEST_F(CAppPluginTest, GetDBAuthInfo)
{
    CAppPlugin plugObj;
    CRequestMsg req;
    app_auth_info_t stAppAuthInfo;

    typedef const mp_char* (CHttpRequest::*pOrgGetHeadNoCheck)(const mp_char* name);
    typedef const mp_char* (*pStubGetHeadNoCheck)(mp_void *ptr, const mp_char * name);
    Stub<pOrgGetHeadNoCheck, pStubGetHeadNoCheck, mp_void> stubCHttpRequest(&CHttpRequest::GetHeadNoCheck, &stubGetHeadNoCheck);
    
    plugObj.GetDBAuthInfo(&req, stAppAuthInfo);

    plugObj.GetDBAuthInfo(&req, stAppAuthInfo);
}


static mp_int32 StubQueryInfo(void *ptr, vector<app_info_t>& vecAppInfos)
{
    static int i = 0;
    if (i++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        vecAppInfos.push_back(app_info_t());
        return MP_SUCCESS;
    }
}

TEST_F(CAppPluginTest, QueryInfo)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;

    typedef mp_int32 (CApp::*pOrgQueryInfo)(vector<app_info_t>& vecAppInfos);
    typedef mp_int32 (*pStubQueryInfo)(void *ptr, vector<app_info_t>& vecAppInfos);
    Stub<pOrgQueryInfo, pStubQueryInfo, mp_void> stubCApp(&CApp::QueryInfo, StubQueryInfo);
    plugObj.QueryInfo(&req, &rsp);
    
    plugObj.QueryInfo(&req, &rsp);
}


static mp_int32 StubFreeze(void *p, app_auth_info_t& appAuthInfo, mp_time& tFreezeTime, vector<app_failed_info_t>& vecAppFailedList)
{
    static int i = 0;
    if (i++ == 0)
    {
        vecAppFailedList.push_back(app_failed_info_t());
        return MP_FAILED;
    }
    else
    {
         return MP_SUCCESS;
    }
}
TEST_F(CAppPluginTest, Freeze)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;

    typedef mp_int32 (CAppPlugin::*pOrgGetDBAuthInfo)(CRequestMsg* req, app_auth_info_t& stAppAuthInfo);
    Stub<pOrgGetDBAuthInfo, pStubVoidType, mp_void> stubCAppPlugin(&CAppPlugin::GetDBAuthInfo, stub_return_nothing);

    typedef mp_int32 (CApp::*pOrgFreeze)(app_auth_info_t& appAuthInfo, mp_time& tFreezeTime, vector<app_failed_info_t>& vecAppFailedList);
    typedef mp_int32 (*pStubFreeze)(void *p, app_auth_info_t& appAuthInfo, mp_time& tFreezeTime, vector<app_failed_info_t>& vecAppFailedList);
    Stub<pOrgFreeze, pStubFreeze, mp_void> stubCApp(&CApp::Freeze, StubFreeze);
    plugObj.Freeze(&req, &rsp);
}


static mp_int32 StubUnFreeze(void *ptr, app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList)
{
    static int i = 0;
    if (i++ == 0)
    {
        vecAppFailedList.push_back(app_failed_info_t());
        return MP_FAILED;
    }
    else
    {
         return MP_SUCCESS;
    }
}
TEST_F(CAppPluginTest, UnFreeze)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;

    typedef mp_int32 (CAppPlugin::*pOrgGetDBAuthInfo)(CRequestMsg* req, app_auth_info_t& stAppAuthInfo);
    Stub<pOrgGetDBAuthInfo, pStubVoidType, mp_void> stubCAppPlugin(&CAppPlugin::GetDBAuthInfo, stub_return_nothing);

    typedef mp_int32 (CApp::*pOrgUnFreeze)(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList);
    typedef mp_int32 (*pStubUnFreeze)(void *ptr, app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList);
    Stub<pOrgUnFreeze, pStubUnFreeze, mp_void> stubCApp(&CApp::UnFreeze, StubUnFreeze);
    plugObj.UnFreeze(&req, &rsp);

    plugObj.UnFreeze(&req, &rsp);
}



TEST_F(CAppPluginTest, EndBackup)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;

    typedef mp_int32 (CAppPlugin::*pOrgGetDBAuthInfo)(CRequestMsg* req, app_auth_info_t& stAppAuthInfo);
    Stub<pOrgGetDBAuthInfo, pStubVoidType, mp_void> stubCAppPlugin(&CAppPlugin::GetDBAuthInfo, stub_return_nothing);
    /* GET_JSON_INT32 ∑µªÿ ß∞‹; */
    plugObj.EndBackup(&req, &rsp);

    typedef mp_int32 (*pOrgCheckParamInteger32)(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude);
    Stub<pOrgCheckParamInteger32, pStubIntType, mp_void> stub(CheckParamInteger32, stub_return_ret);
    plugObj.EndBackup(&req, &rsp);

    plugObj.EndBackup(&req, &rsp);
    typedef mp_int32 (CApp::*pOrgEndBackup)(app_auth_info_t& appAuthInfo, mp_int32 iBackupSucc, vector<app_failed_info_t>& vecAppFailedList);
    Stub<pOrgEndBackup, pStubIntType, mp_void> stubCApp(&CApp::EndBackup, stub_return_ret);
    plugObj.EndBackup(&req, &rsp);
    
}


TEST_F(CAppPluginTest, QueryFreezeState)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;

    typedef mp_int32 (CAppPlugin::*pOrgGetDBAuthInfo)(CRequestMsg* req, app_auth_info_t& stAppAuthInfo);
    Stub<pOrgGetDBAuthInfo, pStubVoidType, mp_void> stubCAppPlugin(&CAppPlugin::GetDBAuthInfo, stub_return_nothing);
    
    typedef mp_int32 (CApp::*pOrgQueryFreezeState)(app_auth_info_t& appAuthInfo, mp_int32& iState);
    Stub<pOrgQueryFreezeState, pStubIntType, mp_void> stubCApp(&CApp::QueryFreezeState, stub_return_ret);
    plugObj.QueryFreezeState(&req, &rsp);

    plugObj.QueryFreezeState(&req, &rsp);
}



TEST_F(CAppPluginTest, UnFreezeEx)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;

    typedef mp_int32 (CAppPlugin::*pOrgGetDBAuthInfo)(CRequestMsg* req, app_auth_info_t& stAppAuthInfo);
    Stub<pOrgGetDBAuthInfo, pStubVoidType, mp_void> stubCAppPlugin(&CAppPlugin::GetDBAuthInfo, stub_return_nothing);
    
    plugObj.UnFreezeEx(&req, &rsp);
}



TEST_F(CAppPluginTest, TruncateLog)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CAppPlugin plugObj;

    typedef mp_int32 (CAppPlugin::*pOrgGetDBAuthInfo)(CRequestMsg* req, app_auth_info_t& stAppAuthInfo);
    Stub<pOrgGetDBAuthInfo, pStubVoidType, mp_void> stubCAppPlugin(&CAppPlugin::GetDBAuthInfo, stub_return_nothing);

    typedef mp_int32 (*pOrgCheckParamInteger64)(mp_int64 paramValue, mp_int64 begValue, mp_int64 endValue, vector<mp_int64> &vecExclude);
    Stub<pOrgCheckParamInteger64, pStubIntType, mp_void> stub(&CheckParamInteger64, stub_return_ret);
    plugObj.TruncateLog(&req, &rsp);
}

