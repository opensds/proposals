/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/host/HostPluginTest.h"


static mp_int32 stubGetInitiators(void *ptr, initiator_info_t& initInfo)
{
    static mp_int32 icounter = 0;
    if (icounter++ == 0)
    {
        return MP_FAILED;    
    }
    else
    {
        initInfo.iscsis.push_back("1123");
        initInfo.fcs.push_back("1123");
        return MP_SUCCESS;
    }
}

static mp_int32 stubGetPartisions(void *ptr, vector<partitisions_info_t>& partisioninfos)
{
    static mp_int32 icounter = 0;
    if (icounter++ == 0)
    {
        return MP_FAILED;    
    }
    else
    {
        partisioninfos.push_back(partitisions_info_t());
        return MP_SUCCESS;
    }
}

mp_int32 stubpQueryThirdPartyScripts(mp_void *ptr, vector<mp_string> & vectFileList)
{
    static mp_int32 icounter = 0;
    if (icounter++ == 0)
    {
        return MP_FAILED;    
    }
    else
    {
        vectFileList.push_back("123");
        return MP_SUCCESS;
    }
}

static mp_int32 stubGetDiskInfo(void *ptr, vector<host_lun_info_t> & vecLunInfo)
{
    static mp_int32 icounter = 0;
    if (icounter++ == 0)
    {
        return MP_FAILED;    
    }
    else
    {
        vecLunInfo.push_back(host_lun_info_t());
        return MP_SUCCESS;
    }
}

static mp_int32 HostPluginTest_bCounter=0;
static mp_int32 HostPluginTest_iCounter=0;
static mp_int32 INVALID_PARAM_iCounter=0;
static mp_int32 HostPluginTest_iCounter2=0;
static bool stub_HostPluginTest_bool(void)
{
    if (HostPluginTest_bCounter++ == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

static mp_int32 stub_HostPluginTest_InvalidParam(mp_void)
{
    if (INVALID_PARAM_iCounter++ == 0)
    {
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 stub_HostPluginTest_Int(mp_void)
{
    if (HostPluginTest_iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 stub_HostPluginTest_IntWithResult(mp_string fileName, mp_string paramValues,vector<mp_string>& vecResult)
{
    if (HostPluginTest_iCounter2++ == 0)
    { 
        return MP_FAILED;
    }
    else
    {
        vecResult.push_back("1");
        return MP_SUCCESS;
    }
}

static mp_void reset_HostPluginTest_Counter()
{
    HostPluginTest_bCounter=0;
    INVALID_PARAM_iCounter=0;
    HostPluginTest_iCounter=0;
    HostPluginTest_iCounter2=0;
}


TEST_F(CHostPluginTest, DoAction)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    iRet = plugObj.DoAction(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);
}

TEST_F(CHostPluginTest, QueryAgentVersion)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (CHost::*pOrgGetAgentVersion)(mp_string& strAgentVersion, mp_string& strBuildNum);
    Stub<pOrgGetAgentVersion, pStubIntType, mp_void> stubCHost(&CHost::GetAgentVersion, &stub_return_ret);

    iRet = plugObj.QueryAgentVersion(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_READ_CONFIG_FAILED, iRet);

    iRet = plugObj.QueryAgentVersion(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, QueryHostInfo)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (CHost::*pOrgGetInfo)(host_info_t& hostInfo);
    Stub<pOrgGetInfo, pStubIntType, mp_void> stubCHost(&CHost::GetInfo, &stub_return_ret);

    iRet = plugObj.QueryHostInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.QueryHostInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, QueryDiskInfo)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (CHost::*pOrgGetDiskInfo)(vector<host_lun_info_t> & vecLunInfo);
    typedef mp_int32 (*pStubGetDiskInfo)(void *ptr, vector<host_lun_info_t> & vecLunInfo);
    Stub<pOrgGetDiskInfo, pStubGetDiskInfo, mp_void> stubCHost(&CHost::GetDiskInfo, &stubGetDiskInfo);

    iRet = plugObj.QueryDiskInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

     iRet = plugObj.QueryDiskInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, QueryTimeZone)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (CHost::*pOrgGetTimeZone)(timezone_info_t& sttimezone);
    Stub<pOrgGetTimeZone, pStubIntType, mp_void> stubCHost(&CHost::GetTimeZone, &stub_return_ret);

    iRet = plugObj.QueryTimeZone(&req, &rsp);
    EXPECT_EQ(ERROR_HOST_LOG_IS_BEENING_COLLECTED, iRet);
    
    iRet = plugObj.QueryTimeZone(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, QueryInitiators)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (CHost::*pOrgGetInitiators)(initiator_info_t& initInfo);
    typedef mp_int32 (*pStubGetInitiators)(void *ptr, initiator_info_t& initInfo);
    Stub<pOrgGetInitiators, pStubGetInitiators, mp_void> stubCHost(&CHost::GetInitiators, &stubGetInitiators);

    iRet = plugObj.QueryInitiators(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.QueryInitiators(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CHostPluginTest, ScanDisk)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (CHost::*pOrgScanDisk)(mp_void);    
    Stub<pOrgScanDisk, pStubIntType, mp_void> stubCHost(&CHost::ScanDisk, &stub_return_ret);    

    iRet = plugObj.ScanDisk(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.ScanDisk(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, QueryThirdPartyScripts)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

     
    typedef mp_int32 (CHost::*pOrgQueryThirdPartyScripts)(vector<mp_string> & vectFileList);
    typedef mp_int32 (*pStubpQueryThirdPartyScripts)(mp_void *ptr, vector<mp_string> & vectFileList);
    Stub<pOrgQueryThirdPartyScripts, pStubpQueryThirdPartyScripts, mp_void> stubCHost(&CHost::QueryThirdPartyScripts, &stubpQueryThirdPartyScripts);    
    
    iRet = plugObj.QueryThirdPartyScripts(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.QueryThirdPartyScripts(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, ExecThirdPartyScript)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    iRet = plugObj.ExecThirdPartyScript(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.ExecThirdPartyScript(&req, &rsp);
    EXPECT_TRUE(1); 

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.ExecThirdPartyScript(&req, &rsp);

    typedef mp_bool (*pOrgCheckCmdDelimiter)(mp_string& str);
    Stub<pOrgCheckCmdDelimiter, pStubBoolType, mp_void> stub02(CheckCmdDelimiter, &stub_return_bool);
    plugObj.ExecThirdPartyScript(&req, &rsp);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub00(CheckPathString, &stub_return_ret);
    plugObj.ExecThirdPartyScript(&req, &rsp);      

    typedef mp_int32 (CHost::*pOrgExecThirdPartyScript)(mp_string fileName, mp_string paramValues,vector<mp_string>& vecResult);
    Stub<pOrgExecThirdPartyScript, pStubIntType, mp_void> stubCHost(&CHost::ExecThirdPartyScript, &stub_return_ret);   

    iRet = plugObj.ExecThirdPartyScript(&req, &rsp);
    EXPECT_TRUE(1);
}

#if 0
TEST_F(CHostPluginTest, RegTrapServer)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    iRet = plugObj.RegTrapServer(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.RegTrapServer(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    typedef mp_int32 (CHost::*pOrgRegTrapServer)(trap_server& stTrapServer);
    Stub<pOrgRegTrapServer, pStubIntType, mp_void> stubCHost(&CHost::RegTrapServer, &stub_return_ret); 

    iRet = plugObj.RegTrapServer(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}
#endif

TEST_F(CHostPluginTest, UnRegTrapServer)
{
    CRequestMsg req;
    CResponseMsg rsp;
    CRequestMsgBody reqBody;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (CHost::*pOrgUnRegTrapServer)(trap_server& stTrapServer);
    Stub<pOrgUnRegTrapServer, pStubIntType, mp_void> stubCHost(&CHost::UnRegTrapServer, &stub_return_ret); 

    typedef bool (Json::Value::*pOrgIsMemory)(const char *key) const;
    typedef bool (*pStubIsMemory)(void);
    Stub<pOrgIsMemory, pStubIsMemory, mp_void> stubJsonIsMember(&Json::Value::isMember, &stub_HostPluginTest_bool);

    typedef mp_int32 (*pOrgCheckParamStringIsIP)(mp_string &paramValue);
    Stub<pOrgCheckParamStringIsIP, pStubIntType, mp_void> stubCheckIP(&CheckParamStringIsIP, &stub_HostPluginTest_Int);
    
    typedef mp_int32 (*pOrgCheckParamInteger32)(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude);
    Stub<pOrgCheckParamInteger32, pStubIntType, mp_void> stubCheckInt(&CheckParamInteger32, &stub_HostPluginTest_Int);
    
    reset_cunit_counter();
    reset_HostPluginTest_Counter();

    iRet = plugObj.UnRegTrapServer(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

        iRet = plugObj.UnRegTrapServer(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.UnRegTrapServer(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

}

TEST_F(CHostPluginTest, VerifySnmp)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;
    
    typedef const mp_char*  (CHttpRequest::*pOrgGetHead)(const mp_char* name);
    Stub<pOrgGetHead, pStubCstringType, mp_void> stubCRequestMsg(&CHttpRequest::GetHead, &stub_return_cstring);

    typedef bool (Json::Value::*pOrgIsMemory)(const char *key) const;
    typedef bool (*pStubIsMemory)(void);
    Stub<pOrgIsMemory, pStubIsMemory, mp_void> stubJsonIsMember(&Json::Value::isMember, &stub_HostPluginTest_bool);

    typedef mp_int32 (*pOrgCheckParamInteger32)(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude);
    Stub<pOrgCheckParamInteger32, pStubIntType, mp_void> stubCheckInt(&CheckParamInteger32, &stub_HostPluginTest_Int);
    
    typedef mp_int32 (CHost::*pOrgVerifySnmp)(snmp_v3_param& stParam);
    Stub<pOrgVerifySnmp, pStubIntType, mp_void> stubCHost(&CHost::VerifySnmp, &stub_return_ret); 

    reset_cunit_counter();
    reset_HostPluginTest_Counter();

    iRet = plugObj.VerifySnmp(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    
    iRet = plugObj.VerifySnmp(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    
    iRet = plugObj.VerifySnmp(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, ExecFreezeScript)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_bool (*pOrgFileExistType)(const mp_char* pszFilePath);
    Stub<pOrgFileExistType, pStubBoolType, mp_void> stubSign(&CMpFile::FileExist, &stub_return_bool);
    plugObj.ExecFreezeScript(&req, &rsp);

    typedef bool (Json::Value::*pOrgIsMemory)(const char *key) const;
    Stub<pOrgIsMemory, pStubBoolType, void> stubJson1(&Json::Value::isMember, stub_return_bool);

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    
    typedef mp_bool (*pOrgCheckCmdDelimiter)(mp_string& str);
    Stub<pOrgCheckCmdDelimiter, pStubBoolType, mp_void> stub00(CheckCmdDelimiter, &stub_return_bool);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub02(CheckPathString, &stub_return_ret);
    plugObj.ExecFreezeScript(&req, &rsp);

    typedef mp_int32 (CHost::*pOrgExecThirdPartyScript)(mp_string fileName, mp_string paramValues,vector<mp_string>& vecResult);
    Stub<pOrgExecThirdPartyScript, pStubIntType, mp_void> stubCHost(&CHost::ExecThirdPartyScript, &stub_return_ret); 

    reset_cunit_counter();
    iRet = plugObj.ExecFreezeScript(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.ExecFreezeScript(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_SCRIPT_FILE_NOT_EXIST, iRet);
    
    iRet = plugObj.ExecFreezeScript(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, ExecThawScript)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);

    typedef mp_bool (*pOrgCheckCmdDelimiter)(mp_string& str);
    Stub<pOrgCheckCmdDelimiter, pStubBoolType, mp_void> stub00(CheckCmdDelimiter, &stub_return_bool);
    plugObj.ExecThawScript(&req, &rsp);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub02(CheckPathString, &stub_return_ret);
    plugObj.ExecThawScript(&req, &rsp);

    typedef mp_int32 (CHost::*pOrgExecThirdPartyScript)(mp_string fileName, mp_string paramValues,vector<mp_string>& vecResult);
    Stub<pOrgExecThirdPartyScript, pStubIntType, mp_void> stubCHost(&CHost::ExecThirdPartyScript, &stub_return_ret); 
    
    reset_cunit_counter();
    iRet = plugObj.ExecThawScript(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = plugObj.ExecThawScript(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    
    iRet = plugObj.ExecThawScript(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, QueryFreezeStatusScript)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);

    typedef mp_bool (*pOrgCheckCmdDelimiter)(mp_string& str);
    Stub<pOrgCheckCmdDelimiter, pStubBoolType, mp_void> stub00(CheckCmdDelimiter, &stub_return_bool);
    plugObj.QueryFreezeStatusScript(&req, &rsp);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub02(CheckPathString, &stub_return_ret);
    plugObj.QueryFreezeStatusScript(&req, &rsp);

    typedef mp_int32 (CHost::*pOrgExecThirdPartyScript)(mp_string fileName, mp_string paramValues,vector<mp_string>& vecResult);
    typedef mp_int32 (*pStubExecThirdPartyScript)(mp_string fileName, mp_string paramValues,vector<mp_string>& vecResult);
    Stub<pOrgExecThirdPartyScript, pStubExecThirdPartyScript, mp_void> stubCHost(&CHost::ExecThirdPartyScript, &stub_HostPluginTest_IntWithResult); 

    reset_cunit_counter();
    iRet = plugObj.QueryFreezeStatusScript(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.QueryFreezeStatusScript(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);

    iRet = plugObj.QueryFreezeStatusScript(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(CHostPluginTest, CollectAgentLog)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    typedef mp_int32 (CHost::*pOrgCollectLog)(mp_void);
    Stub<pOrgCollectLog, pStubIntType, mp_void> stubCHost(&CHost::CollectLog, &stub_return_ret);     

    iRet = plugObj.CollectAgentLog(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = plugObj.CollectAgentLog(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, ExportAgentLog)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CHostPlugin plugObj;

    reset_cunit_counter();
    reset_HostPluginTest_Counter();
    
    typedef mp_bool (CHost::*pOrgCanDownloadLog)(mp_void);
    Stub<pOrgCanDownloadLog, pStubBoolType, mp_void> stubCHost(&CHost::CanDownloadLog, &stub_return_bool); 
    
    iRet = plugObj.ExportAgentLog(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef bool (Json::Value::*pOrgIsMemory)(const char *key) const;
    Stub<pOrgIsMemory, pStubBoolType, void> stubJson1(&Json::Value::isMember, stub_return_bool);

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    
    typedef mp_int32 (*pOrgCheckParamStringEnd)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strEnd);
    Stub<pOrgCheckParamStringEnd, pStubIntType, mp_void> stub00(CheckParamStringEnd, &stub_return_ret);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub02(CheckPathString, &stub_return_ret);

    iRet = plugObj.ExportAgentLog(&req, &rsp);
    EXPECT_EQ(ERROR_HOST_GET_TIMEZONE_FAILED, iRet);

    iRet = plugObj.ExportAgentLog(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CHostPluginTest, GetLogName)
{
    CRequestMsg req;
    CResponseMsg rsp;
    CHostPlugin plugObj;

    plugObj.GetLogName();
}



