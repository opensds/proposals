/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/db2/Db2PluginTest.h"

static mp_int32 stubGetInfo(void *ptr, std::vector<db2_inst_info_t> &vecdbInstInfo)
{
    static int iCounter_inner = 0;
    if (iCounter_inner++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        db2_inst_info_t dbInfo;
        dbInfo.strversion = "11.1";
        dbInfo.strinstName = "agent";
        dbInfo.strdbName = "agentdb";
        dbInfo.istate = 1;

        vecdbInstInfo.push_back(dbInfo);

        return MP_SUCCESS;
    }
}

static mp_int32 stubGetLunInfo(mp_void *ptr, db2_db_info_t stdbinfo, vector<db2_lun_info_t>& vecLunInfos)
{
    static int iCounter_inner = 0;
    if (iCounter_inner++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        db2_lun_info_t dbLunInfo;
        vecLunInfos.push_back(dbLunInfo);

        return MP_SUCCESS;
    }
}


TEST_F(CDb2PluginTest, DoAction)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;

    iRet = plugObj.DoAction(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);    
}

TEST_F(CDb2PluginTest, QueryInfo)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;

    typedef mp_int32 (CDB2::*pOrgGetInfo)(std::vector<db2_inst_info_t> &vecdbInstInfo);
    typedef mp_int32 (*pStubGetInfo)(mp_void *ptr, std::vector<db2_inst_info_t> &vecdbInstInfo);
    Stub<pOrgGetInfo, pStubGetInfo, mp_void> StubCDB2(&CDB2::GetInfo, &stubGetInfo);

    iRet = plugObj.QueryInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.QueryInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CDb2PluginTest, QueryLunInfo)
{
    FCGX_Request fcgiReq;
    CRequestMsg req;
    CResponseMsg rsp(&fcgiReq);
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.QueryLunInfo(&req, &rsp);

    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg3(&CHttpRequest::GetHead, &stub_return_cstring);

    typedef mp_int32 (CDB2::*pOrgGetLunInfo)(db2_db_info_t stdbinfo, vector<db2_lun_info_t>& vecLunInfos);
    typedef mp_int32 (*pStubGetLunInfo)(mp_void *ptr, db2_db_info_t stdbinfo, vector<db2_lun_info_t>& vecLunInfos);
    Stub<pOrgGetLunInfo, pStubGetLunInfo, mp_void> stubCDB2(&CDB2::GetLunInfo, &stubGetLunInfo);

    iRet = plugObj.QueryLunInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);  

    iRet = plugObj.QueryLunInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet); 

    iRet = plugObj.QueryLunInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet); 
}

TEST_F(CDb2PluginTest, Start)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    //plugObj.Start(&req, &rsp);

    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg3(&CHttpRequest::GetHead, &stub_return_cstring);
    
    iRet = plugObj.Start(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);   
    
    iRet = plugObj.Start(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);   
}

TEST_F(CDb2PluginTest, Stop)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;
    
    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    //plugObj.Stop(&req, &rsp);

    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg3(&CHttpRequest::GetHead, &stub_return_cstring);

    iRet = plugObj.Stop(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet); 
    
    iRet = plugObj.Stop(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);   
}

TEST_F(CDb2PluginTest, Test)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;
    
    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    //plugObj.Test(&req, &rsp);

    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg3(&CHttpRequest::GetHead, &stub_return_cstring);

    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet); 
    
    iRet = plugObj.Test(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);   
}

TEST_F(CDb2PluginTest, Freeze)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    //plugObj.Freeze(&req, &rsp);

    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg3(&CHttpRequest::GetHead, &stub_return_cstring);

    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);   
    
    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);   
}

TEST_F(CDb2PluginTest, UnFreeze)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    //plugObj.UnFreeze(&req, &rsp);

    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg3(&CHttpRequest::GetHead, &stub_return_cstring);

    iRet = plugObj.UnFreeze(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);    

    iRet = plugObj.UnFreeze(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_EXEC_FAILED, iRet);   
}


TEST_F(CDb2PluginTest, QueryFreezeState)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDb2Plugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    //plugObj.QueryFreezeState(&req, &rsp);

    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg3(&CHttpRequest::GetHead, &stub_return_cstring);
    
    typedef mp_int32 (CDB2::*pOrgQueryFreezeState)(db2_db_info_t stdbInfo, mp_int32& iFreezeState);
    Stub<pOrgQueryFreezeState, pStubIntType, mp_void> stubCDB2(&CDB2::QueryFreezeState, &stub_return_ret);

    iRet = plugObj.QueryFreezeState(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);    

    iRet = plugObj.QueryFreezeState(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);  

    iRet = plugObj.QueryFreezeState(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);  
}



