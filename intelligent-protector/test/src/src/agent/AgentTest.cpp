/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "agent/AgentTest.h"

//Begin CAuthenticationTest
TEST_F(CAuthenticationTest, Init)
{
    mp_int32 rst = 0;
    //CFG_USER_NAME < 0
    {
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringLt0);
        rst = CAuthentication::GetInstance().Init();
        EXPECT_EQ(rst, MP_FAILED);
    }
    //CFG_HASH_VALUE < 0
    {
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringLt0AuthInit);
        rst = CAuthentication::GetInstance().Init();
        EXPECT_EQ(rst, MP_FAILED);
    }
}
TEST_F(CAuthenticationTest, Auth)
{
    mp_string strClientIP = "127.0.0.1";
    mp_string strUsr = "admin";
    mp_string strPw = "Admin@123";
    mp_int32 rst = 0;
    //CFG_USER_NAME < 0
    {
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringLt0);
        rst = CAuthentication::GetInstance().Auth(strClientIP, strUsr, strPw);
        EXPECT_EQ(rst, ERROR_COMMON_READ_CONFIG_FAILED);
    }
    //CFG_HASH_VALUE < 0
    {
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringLt0AuthAuth);
        rst = CAuthentication::GetInstance().Auth(strClientIP, strUsr, strPw);
        EXPECT_EQ(rst, ERROR_COMMON_READ_CONFIG_FAILED);
    }
    //ckeck != MP_FALSE
    {
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringEq0AuthAuth);
        rst = CAuthentication::GetInstance().Auth(strClientIP, strUsr, strPw);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    //ckeck = MP_FALSE and unlock
    {
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringEq0AuthAuth);
        strPw = "error";
        rst = CAuthentication::GetInstance().Auth(strClientIP, strUsr, strPw);
        EXPECT_EQ(rst, ERROR_COMMON_USER_OR_PASSWD_IS_WRONG);
        strPw = "Admin@123";
        rst = CAuthentication::GetInstance().Auth(strClientIP, strUsr, strPw);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    //ckeck = MP_FALSE and lock
    {
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringEq0AuthAuth);
        strPw = "error";
        for (int i = 0; i <= MAX_TRY_TIME; i++)
        {
            rst = CAuthentication::GetInstance().Auth(strClientIP, strUsr, strPw);
        }
        EXPECT_EQ(rst, ERROR_COMMON_CLIENT_IS_LOCKED);
    }
}
//End CAuthenticationTest
//Begin CCommunicationTest
TEST_F(CCommunicationTest, Init)
{
    mp_string strUsr = "test";
    mp_string strPwd = "test";
    mp_int32 rst = 0;
    //FCGX_Init < 0
    {
        Stub<FCGX_InitType, StubFCGX_InitType, mp_void> mystub1(&FCGX_Init, &StubFCGX_InitLt0);
        rst = CCommunication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
    //CFG_PORT < 0
    {
        Stub<FCGX_InitType, StubFCGX_InitType, mp_void> mystub2(&FCGX_Init, &StubFCGX_InitEq0);
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub3(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringLt0);
        rst = CCommunication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_READ_CONFIG_FAILED);
    }
    //FCGX_OpenSocket < 0
    {
        Stub<FCGX_InitType, StubFCGX_InitType, mp_void> mystub4(&FCGX_Init, &StubFCGX_InitEq0);
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub5(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringEq0);
        Stub<FCGX_OpenSocketType, StubFCGX_OpenSocketType, mp_void> mystub6(&FCGX_OpenSocket, &StubFCGX_OpenSocketLt0);
        rst = CCommunication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
    //fcntl < 0
    {
        Stub<FCGX_InitType, StubFCGX_InitType, mp_void> mystub7(&FCGX_Init, &StubFCGX_InitEq0);
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub8(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringEq0);
        Stub<FCGX_OpenSocketType, StubFCGX_OpenSocketType, mp_void> mystub9(&FCGX_OpenSocket, &StubFCGX_OpenSocketEq0);
        Stub<fcntlType, StubfcntlType, mp_void> mystub10(&fcntl, &StubfcntlLt0);
        Stub<OS_CloseType, StubOS_CloseType, mp_void> mystub11(&OS_Close, &StubOS_CloseEq0);
        rst = CCommunication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
    //Create < 0
    {
        Stub<FCGX_InitType, StubFCGX_InitType, mp_void> mystub12(&FCGX_Init, &StubFCGX_InitEq0);
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub13(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringEq0);
        Stub<FCGX_OpenSocketType, StubFCGX_OpenSocketType, mp_void> mystub14(&FCGX_OpenSocket, &StubFCGX_OpenSocketEq0);
        Stub<fcntlType, StubfcntlType, mp_void> mystub15(&fcntl, &StubfcntlEq0);
        Stub<OS_CloseType, StubOS_CloseType, mp_void> mystub16(&OS_Close, &StubOS_CloseEq0);
        Stub<CMpThreadCreateType, StubCMpThreadCreateType, mp_void> mystub17(&CMpThread::Create, &StubCMpThreadCreateLt0);
        rst = CCommunication::GetInstance().Init();
        EXPECT_EQ(rst, -1);
    }
    //init < 0
    {
        Stub<FCGX_InitType, StubFCGX_InitType, mp_void> mystub18(&FCGX_Init, &StubFCGX_InitEq0);
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub19(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringEq0);
        Stub<FCGX_OpenSocketType, StubFCGX_OpenSocketType, mp_void> mystub20(&FCGX_OpenSocket, &StubFCGX_OpenSocketEq0);
        Stub<fcntlType, StubfcntlType, mp_void> mystub21(&fcntl, &StubfcntlEq0);
        Stub<OS_CloseType, StubOS_CloseType, mp_void> mystub22(&OS_Close, &StubOS_CloseEq0);
        Stub<CMpThreadCreateType, StubCMpThreadCreateType, mp_void> mystub23(&CMpThread::Create, &StubCMpThreadCreateEq0);
        Stub<CAuthenticationInitType, StubCAuthenticationInitType, mp_void> mystub24(&CAuthentication::Init, &StubCAuthenticationInitLt0);
        rst = CCommunication::GetInstance().Init();
        EXPECT_EQ(rst, -1);
        CCommunication::GetInstance().GetFcgxReq();
    }
    
    //InitRequest < 0
    {
        Stub<FCGX_InitType, StubFCGX_InitType, mp_void> mystub25(&FCGX_Init, &StubFCGX_InitEq0);
        Stub<CConfigXmlParserGetValueStringType, StubCConfigXmlParserGetValueStringType, mp_void> mystub26(&CConfigXmlParser::GetValueString, &StubCConfigXmlParserGetValueStringEq0);
        Stub<FCGX_OpenSocketType, StubFCGX_OpenSocketType, mp_void> mystub27(&FCGX_OpenSocket, &StubFCGX_OpenSocketEq0);
        Stub<fcntlType, StubfcntlType, mp_void> mystub28(&fcntl, &StubfcntlEq0);
        Stub<OS_CloseType, StubOS_CloseType, mp_void> mystub29(&OS_Close, &StubOS_CloseEq0);
        Stub<CCommunicationInitRequestType, StubCCommunicationInitRequestType, mp_void> mystub30(&CCommunication::InitRequest, &StubCCommunicationInitRequestLt0);
        rst = CCommunication::GetInstance().Init();
        EXPECT_EQ(rst, -1);
    }
}
TEST_F(CCommunicationTest, NeedExit)
{
    mp_int32 rst = CCommunication::GetInstance().NeedExit();
    EXPECT_EQ(rst, CCommunication::GetInstance().m_bNeedExit);
}
TEST_F(CCommunicationTest, SetRecvThreadStatus)
{
    CCommunication::GetInstance().SetRecvThreadStatus(1);
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, SetSendThreadStatus)
{
    CCommunication::GetInstance().SetSendThreadStatus(1);
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, ReleaseRequest)
{
    FCGX_Request req;
    CCommunication::GetInstance().ReleaseRequest(&req);
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, SendFailedMsg)
{
    FCGX_Request req;
    Stub<CResponseMsgSendType, StubCResponseMsgSendType, mp_void> mystub(&CResponseMsg::Send, &StubCResponseMsgSendEq0);
    CCommunication::GetInstance().SendFailedMsg(NULL, NULL, 1, 1);
    CCommunication::GetInstance().SendFailedMsg(&(CCommunication::GetInstance()), &req, 1, 1);
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, HandleReceiveMsg)
{
    FCGX_Request req;
    CCommunication::GetInstance().HandleReceiveMsg(NULL, NULL);
    {
        Stub<FCGX_GetParamType, StubFCGX_GetParamType, mp_void> mystub1(&FCGX_GetParam, &StubFCGX_GetParamNULL);
        Stub<CRequestMsgParseType, StubCRequestMsgParseType, mp_void> mystub2(&CRequestMsg::Parse, &StubCRequestMsgParseLt0);
        Stub<CResponseMsgSendType, StubCResponseMsgSendType, mp_void> mystub3(&CResponseMsg::Send, &StubCResponseMsgSendEq0);
        CCommunication::GetInstance().HandleReceiveMsg(&(CCommunication::GetInstance()), &req);
    }
    {
        Stub<FCGX_GetParamType, StubFCGX_GetParamType, mp_void> mystub4(&FCGX_GetParam, &StubFCGX_GetParamNULL);
        Stub<CRequestMsgParseType, StubCRequestMsgParseType, mp_void> mystub5(&CRequestMsg::Parse, &StubCRequestMsgParseEq0);
        CCommunication::GetInstance().HandleReceiveMsg(&(CCommunication::GetInstance()), &req);
    }
    EXPECT_TRUE(1);
    vector<message_pair_t>::iterator it = CCommunication::GetInstance().m_reqMsgQueue.begin();
    delete it->pReqMsg;
    delete it->pRspMsg;
    CCommunication::GetInstance().m_reqMsgQueue.erase(it);
}
TEST_F(CCommunicationTest, ReceiveThreadFunc)
{
    CCommunication::GetInstance().m_bNeedExit = 1;
    CCommunication::GetInstance().ReceiveThreadFunc(&(CCommunication::GetInstance()));
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, SendThreadFunc)
{
    CCommunication::GetInstance().m_bNeedExit = 1;
    CCommunication::GetInstance().SendThreadFunc(&(CCommunication::GetInstance()));
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, PushReq_PopReq)
{
    message_pair_t pair;
    CCommunication::GetInstance().PushReqMsgQueue(pair);
    mp_int32 rst = CCommunication::GetInstance().PopReqMsgQueue(pair);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CCommunication::GetInstance().PopReqMsgQueue(pair);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CCommunicationTest, PushRsp_PopRsp)
{
    CResponseMsg rsp;
    CRequestMsg *req = NULL;
    message_pair_t pair(req, &rsp);
    CCommunication::GetInstance().PushRspMsgQueue(pair);
    mp_int32 rst = CCommunication::GetInstance().PopRspMsgQueue(pair);
    EXPECT_EQ(rst, MP_FAILED);
    rst = CCommunication::GetInstance().PopRspInternalMsgQueue(pair);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CCommunication::GetInstance().PopRspMsgQueue(pair);
    EXPECT_EQ(rst, MP_FAILED);
    
    FCGX_Request pfcg;
    CResponseMsg rsp2(&pfcg);
    message_pair_t pair2(req, &rsp2);
    CCommunication::GetInstance().PushRspMsgQueue(pair2);
    rst = CCommunication::GetInstance().PopRspMsgQueue(pair2);
    EXPECT_EQ(rst, MP_SUCCESS);
    
    CCommunication::GetInstance().PushRspMsgQueue(pair2);
    rst = CCommunication::GetInstance().PopRspInternalMsgQueue(pair2);
    EXPECT_EQ(rst, MP_FAILED);
    rst = CCommunication::GetInstance().PopRspMsgQueue(pair2);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CCommunication::GetInstance().PopRspInternalMsgQueue(pair2);
    EXPECT_EQ(rst, MP_FAILED);
}
//End CCommunicationTest
//Begin CFTExceptionHandleTest
TEST_F(CFTExceptionHandleTest, HandleMonitorObjsProc)
{
    CFTExceptionHandle::GetInstance().m_bNeedExit = true;
    CFTExceptionHandle::GetInstance().HandleMonitorObjsProc(&(CFTExceptionHandle::GetInstance()));
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, GetThreadStatus)
{
    mp_int32 rst = CFTExceptionHandle::GetInstance().GetThreadStatus();
    EXPECT_EQ(rst, CFTExceptionHandle::GetInstance().m_iThreadStatus);
}
TEST_F(CFTExceptionHandleTest, GetThreadLock)
{
    CFTExceptionHandle::GetInstance().GetThreadLock();
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, HandleMonitorObjs)
{
    CFTExceptionHandle::GetInstance().HandleMonitorObjs();
    CResponseMsg *rsp = new CResponseMsg;
    CRequestMsg *req = new CRequestMsg;
    req->m_url.m_procURL = REST_DB2_UNFREEZE;
    message_pair_t pair(req, rsp);
    CCommunication::GetInstance().PushRspMsgQueue(pair);
    CFTExceptionHandle::GetInstance().HandleMonitorObjs();
    
    rsp = new CResponseMsg;
    req = new CRequestMsg;
    req->m_url.m_procURL = REST_DB2_FREEZESTATE;
    message_pair_t pair1(req, rsp);
    MONITOR_OBJ obj;
    obj.uiStatus = MONITOR_STATUS_FREEZED;
    obj.iAppType = TYPE_APP_UNKNOWN;
    CFTExceptionHandle::GetInstance().m_vecMonitors.push_back(obj);
    CCommunication::GetInstance().PushRspMsgQueue(pair1);
    CFTExceptionHandle::GetInstance().HandleMonitorObjs();
    EXPECT_TRUE(1);
    CFTExceptionHandle::GetInstance().m_vecMonitors.pop_back();
}
TEST_F(CFTExceptionHandleTest, ProccessUnFreezeRsp)
{
    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = REST_DB2_FREEZESTATE;
    req.m_msgBody.m_msgJsonData["instName"] = "db2inst1";
    mp_int32 rst = CFTExceptionHandle::GetInstance().ProccessUnFreezeRsp(&req, &rsp);
    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
    
    req.m_msgBody.m_msgJsonData["dbName"] = "db_sss";
    rsp.m_lRetCode = MP_SUCCESS;
    rst = CFTExceptionHandle::GetInstance().ProccessUnFreezeRsp(&req, &rsp);
    EXPECT_EQ(rst, MP_SUCCESS);
    
    MONITOR_OBJ obj;
    obj.iAppType = TYPE_APP_DB2;
    obj.strInstanceName = "db2inst1";
    obj.strDBName = "db_sss";
    obj.uiStatus = MONITOR_STATUS_FREEZED;
    CFTExceptionHandle::GetInstance().m_vecMonitors.push_back(obj);
    rst = CFTExceptionHandle::GetInstance().ProccessUnFreezeRsp(&req, &rsp);
    EXPECT_EQ(rst, MP_SUCCESS);
    CFTExceptionHandle::GetInstance().m_vecMonitors.pop_back();
    
    rsp.m_lRetCode = -1;
    obj.uiStatus = MONITOR_STATUS_GETSTATUSING;
    CFTExceptionHandle::GetInstance().m_vecMonitors.push_back(obj);
    rst = CFTExceptionHandle::GetInstance().ProccessUnFreezeRsp(&req, &rsp);
    EXPECT_EQ(rst, MP_FAILED);
    CFTExceptionHandle::GetInstance().m_vecMonitors.pop_back();
}
TEST_F(CFTExceptionHandleTest, ProcessQueryStatusRsp)
{
    CRequestMsg req;
    CResponseMsg rsp;
    req.m_url.m_procURL = REST_DB2_FREEZESTATE;
    req.m_msgBody.m_msgJsonData["instName"] = "db2inst1";
    mp_int32 rst = CFTExceptionHandle::GetInstance().ProcessQueryStatusRsp(&req, &rsp);
    
    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
    req.m_msgBody.m_msgJsonData["dbName"] = "db_sss";
    rsp.m_lRetCode = MP_SUCCESS;
    rsp.m_msgJsonData["state"] = 1;
    rst = CFTExceptionHandle::GetInstance().ProcessQueryStatusRsp(&req, &rsp);
    EXPECT_EQ(rst, MP_SUCCESS);
    
    MONITOR_OBJ obj;
    obj.iAppType = TYPE_APP_DB2;
    obj.strInstanceName = "db2inst1";
    obj.strDBName = "db_sss";
    obj.uiStatus = MONITOR_STATUS_FREEZED;
    CFTExceptionHandle::GetInstance().m_vecMonitors.push_back(obj);
    rst = CFTExceptionHandle::GetInstance().ProcessQueryStatusRsp(&req, &rsp);
    EXPECT_EQ(rst, MP_SUCCESS);
    CFTExceptionHandle::GetInstance().m_vecMonitors.pop_back();
    
    rsp.m_msgJsonData["state"] = DB_FREEZE;
    obj.uiStatus = MONITOR_STATUS_GETSTATUSING;
    obj.iAppType = TYPE_APP_UNKNOWN;
    CFTExceptionHandle::GetInstance().m_vecMonitors.push_back(obj);
    rst = CFTExceptionHandle::GetInstance().ProcessQueryStatusRsp(&req, &rsp);
    EXPECT_EQ(rst, MP_SUCCESS);
    CFTExceptionHandle::GetInstance().m_vecMonitors.pop_back();
}
TEST_F(CFTExceptionHandleTest, HandleUnFreezingMonitorObj)
{
    MONITOR_OBJ obj;
    obj.uiStatus = MONITOR_STATUS_GETSTATUSING;
    obj.uiLoopTime = 60;
    mp_int32 rst = CFTExceptionHandle::GetInstance().HandleUnFreezingMonitorObj(&obj, 0);
    EXPECT_EQ(rst, MP_FAILED);
    rst = CFTExceptionHandle::GetInstance().HandleUnFreezingMonitorObj(&obj, 1);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CFTExceptionHandleTest, HandleQueryStatusMonitorObj)
{
    MONITOR_OBJ obj;
    obj.uiStatus = MONITOR_STATUS_GETSTATUSING;
    obj.iAppType = TYPE_APP_DB2;
    mp_int32 rst = CFTExceptionHandle::GetInstance().HandleQueryStatusMonitorObj(&obj, DB_UNFREEZE);
    EXPECT_EQ(rst, MP_SUCCESS);
    CRequestMsg req;
    obj.pReqMsg = &req;
    rst = CFTExceptionHandle::GetInstance().HandleQueryStatusMonitorObj(&obj, DB_FREEZE);
    EXPECT_EQ(rst, MP_SUCCESS);
    message_pair_t stPair;
    CCommunication::GetInstance().PopReqMsgQueue(stPair);
    if (stPair.pReqMsg)
    {
        delete stPair.pReqMsg;
        stPair.pReqMsg = NULL;
    }
    if (stPair.pRspMsg)
    {
        delete stPair.pRspMsg;
        stPair.pRspMsg = NULL;
    }
    rst = CFTExceptionHandle::GetInstance().HandleQueryStatusMonitorObj(&obj, DB_UNKNOWN);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CFTExceptionHandleTest, HandleFreezedMonitorObj)
{
    MONITOR_OBJ obj;
    obj.ulBeginTime = 0;
    obj.uiLoopTime = 60;
    mp_int32 rst = 0;
    {
        Stub<CMpTimeGetTimeSecType, StubCMpTimeGetTimeSecType, mp_void> mystub1(&CMpTime::GetTimeSec, &StubCMpTimeGetTimeSec6H);
        rst = CFTExceptionHandle::GetInstance().HandleFreezedMonitorObj(&obj);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    {
        Stub<CMpTimeGetTimeSecType, StubCMpTimeGetTimeSecType, mp_void> mystub2(&CMpTime::GetTimeSec, &StubCMpTimeGetTimeSec50s);
        rst = CFTExceptionHandle::GetInstance().HandleFreezedMonitorObj(&obj);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    Stub<CMpTimeGetTimeSecType, StubCMpTimeGetTimeSecType, mp_void> mystub3(&CMpTime::GetTimeSec, &StubCMpTimeGetTimeSec100s);
    obj.iAppType = TYPE_APP_UNKNOWN;
    rst = CFTExceptionHandle::GetInstance().HandleFreezedMonitorObj(&obj);
    EXPECT_EQ(rst, MP_FAILED);
    obj.iAppType = TYPE_APP_DB2;
    CRequestMsg remsg;
    CRequestURL url;
    url.m_procURL = "test";
    url.m_oriURL = "test";
    url.m_id = "test";
    url.m_queryParam["test"] = "test";
    remsg.m_url = url;
    
    CRequestMsgBody reqbody;
    reqbody.m_msgBodyType = BODY_DECODE_JSON;
    reqbody.m_msgLen = 0;
    auto_ptr<CMessage_Block> test(new CMessage_Block());
    reqbody.m_raw_msg = test;
    Json::Value jvalue;
    jvalue["test"] = "test";
    reqbody.m_msgJsonData = jvalue;
    remsg.m_msgBody = reqbody;
    
    CHttpRequest req;
    req.m_strURL = "test";
    req.m_strQueryParam = "test";
    req.m_strMethod = "test";
    FCGX_Request freq;
    req.m_pFcgRequest = &freq;
    remsg.m_httpReq = req;
    
    obj.pReqMsg = &remsg;
    
    rst = CFTExceptionHandle::GetInstance().HandleFreezedMonitorObj(&obj);
    EXPECT_EQ(rst, MP_SUCCESS);
    message_pair_t stPair;
    CCommunication::GetInstance().PopReqMsgQueue(stPair);
    if (stPair.pReqMsg)
    {
        delete stPair.pReqMsg;
        stPair.pReqMsg = NULL;
    }
    if (stPair.pRspMsg)
    {
        delete stPair.pRspMsg;
        stPair.pRspMsg = NULL;
    }
}
TEST_F(CFTExceptionHandleTest, WaitForExit)
{
    CFTExceptionHandle::GetInstance().m_iThreadStatus = THREAD_STATUS_EXITED;
    CFTExceptionHandle::GetInstance().WaitForExit();
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, MonitorFreezeOper)
{
    CRequestMsg req;
    Stub<CDBQueryTableType, StubCDBQueryTableType, mp_void> mystub(&CDB::QueryTable, &StubCDBQueryTableEq0);
    req.m_url.m_procURL = "/device/filesystems/freezestate";
    CFTExceptionHandle::GetInstance().MonitorFreezeOper(&req);
    req.m_msgBody.m_msgJsonData[DISKNAMES].append("test");
    req.m_msgBody.m_msgJsonData[DISKNAMES].append("test");
    CFTExceptionHandle::GetInstance().MonitorFreezeOper(&req);
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, MonitorSingleFreezeOper)
{
    FCGX_Request fcg;
    Stub<FCGX_GetParamType, StubFCGX_GetParamType, mp_void> mystub1(&FCGX_GetParam, &StubFCGX_GetParamOk);
    CRequestMsg req(&fcg);
    {
        Stub<CDBExecSqlType, StubCDBExecSqlType, mp_void> mystub2(&CDB::ExecSql, &StubCDBExecSqlEq0);
        //CreateMonitorObj < 0
        CFTExceptionHandle::GetInstance().MonitorSingleFreezeOper(&req);
        EXPECT_TRUE(1);
        req.m_url.m_procURL = REST_DB2_FREEZE;
        req.m_msgBody.m_msgJsonData[INST_NAME] = "db2inst1";
        req.m_msgBody.m_msgJsonData[DBNAME] = "db_sss";
        req.m_msgBody.m_msgJsonData[LOOP_TIME] = 60;
        CFTExceptionHandle::GetInstance().MonitorSingleFreezeOper(&req);
        EXPECT_TRUE(1);
    }
    //savetodb < 0
    {
        Stub<CDBExecSqlType, StubCDBExecSqlType, mp_void> mystub3(&CDB::ExecSql, &StubCDBExecSqlLt0);
        CFTExceptionHandle::GetInstance().MonitorSingleFreezeOper(&req);
        EXPECT_TRUE(1);
    }
    //AddMonitorObj <0
    {
        Stub<CDBExecSqlType, StubCDBExecSqlType, mp_void> mystub4(&CDB::ExecSql, &StubCDBExecSqlEq0);
        MONITOR_OBJ obj;
        obj.iAppType = TYPE_APP_DB2;
        obj.strInstanceName = "db2inst1";
        obj.strDBName = "db_sss";
        CFTExceptionHandle::GetInstance().m_vecMonitors.push_back(obj);
        CFTExceptionHandle::GetInstance().MonitorSingleFreezeOper(&req);
        CFTExceptionHandle::GetInstance().m_vecMonitors.pop_back();
    }
}
TEST_F(CFTExceptionHandleTest, CreateMonitorObj)
{
    FCGX_Request fcg;
    Stub<FCGX_GetParamType, StubFCGX_GetParamType, mp_void> mystub1(&FCGX_GetParam, &StubFCGX_GetParamOk);
    CRequestMsg reqmsg(&fcg);
    MONITOR_OBJ obj;
    //vss
    reqmsg.m_url.m_procURL = REST_SQLSERVER_UNFREEZE_DB;
    mp_int32 rst = CFTExceptionHandle::GetInstance().CreateMonitorObj(&reqmsg, obj);
    EXPECT_EQ(rst, MP_SUCCESS);
    //fs
    reqmsg.m_url.m_procURL = REST_DEVICE_FILESYS_FREEZE;
    reqmsg.m_msgBody.m_msgJsonData[DISKNAMES].append("disk0");
    reqmsg.m_msgBody.m_msgJsonData[DISKNAMES].append("disk1");
    rst = CFTExceptionHandle::GetInstance().CreateMonitorObj(&reqmsg, obj);
    EXPECT_EQ(rst, MP_SUCCESS);
    //other
    reqmsg.m_url.m_procURL = "test";
    rst = CFTExceptionHandle::GetInstance().CreateMonitorObj(&reqmsg, obj);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CFTExceptionHandleTest, CreateVSSMonitorObj)
{
    FCGX_Request fcg;
    Stub<FCGX_GetParamType, StubFCGX_GetParamType, mp_void> mystub1(&FCGX_GetParam, &StubFCGX_GetParamOk);
    CRequestMsg req(&fcg);
    MONITOR_OBJ obj;
    mp_int32 rst = CFTExceptionHandle::GetInstance().CreateVSSMonitorObj(&req, obj);
}
TEST_F(CFTExceptionHandleTest, CreateDBMonitorObj)
{
    FCGX_Request fcg;
    Stub<FCGX_GetParamType, StubFCGX_GetParamType, mp_void> mystub1(&FCGX_GetParam, &StubFCGX_GetParamOk);
    CRequestMsg req(&fcg);
    MONITOR_OBJ obj;
    req.m_url.m_procURL = REST_DB2_FREEZE;
    //no inst
    mp_int32 rst = CFTExceptionHandle::GetInstance().CreateDBMonitorObj(&req, obj);
    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
    req.m_msgBody.m_msgJsonData[INST_NAME] = "db2inst1";
    //no db
    rst = CFTExceptionHandle::GetInstance().CreateDBMonitorObj(&req, obj);
    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
    req.m_msgBody.m_msgJsonData[DBNAME] = "db_sss";
    req.m_msgBody.m_msgJsonData[LOOP_TIME] = 60;
}
TEST_F(CFTExceptionHandleTest, CreateFSMonitorObj)
{
    FCGX_Request fcg;
    Stub<FCGX_GetParamType, StubFCGX_GetParamType, mp_void> mystub1(&FCGX_GetParam, &StubFCGX_GetParamOk);
    CRequestMsg req(&fcg);
    MONITOR_OBJ obj;
    mp_int32 rst = CFTExceptionHandle::GetInstance().CreateFSMonitorObj(&req, obj);
    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
}
TEST_F(CFTExceptionHandleTest, UpdateFreezeOper)
{
    FCGX_Request fcg;
    Stub<FCGX_GetParamType, StubFCGX_GetParamType, mp_void> mystub1(&FCGX_GetParam, &StubFCGX_GetParamOk);
    CRequestMsg reqMsg(&fcg);
    CResponseMsg rspMsg;
    //not freeze or unfreeze obj
    CFTExceptionHandle::GetInstance().UpdateFreezeOper(&reqMsg, &rspMsg);
    //GetMonitorObj < 0
    reqMsg.m_url.m_procURL = REST_DB2_FREEZE;
    reqMsg.m_msgBody.m_msgJsonData[INST_NAME] = "db2inst1";
    reqMsg.m_msgBody.m_msgJsonData[DBNAME] = "db_sss";
    CFTExceptionHandle::GetInstance().UpdateFreezeOper(&reqMsg, &rspMsg);
    //freeze opt
    MONITOR_OBJ obj;
    obj.iAppType = TYPE_APP_DB2;
    obj.strInstanceName = "db2inst1";
    obj.strDBName = "db_sss";
    CFTExceptionHandle::GetInstance().m_vecMonitors.push_back(obj);
    CFTExceptionHandle::GetInstance().UpdateFreezeOper(&reqMsg, &rspMsg);
    //unfreeze opt
    reqMsg.m_url.m_procURL = REST_DB2_UNFREEZE;
    CFTExceptionHandle::GetInstance().UpdateFreezeOper(&reqMsg, &rspMsg);
    CFTExceptionHandle::GetInstance().m_vecMonitors.pop_back();
    //no disks
    reqMsg.m_url.m_procURL = REST_DEVICE_FILESYS_FREEZE;
    CFTExceptionHandle::GetInstance().UpdateFreezeOper(&reqMsg, &rspMsg);
    //disks
    reqMsg.m_msgBody.m_msgJsonData[DISKNAMES].append("disk0");
    reqMsg.m_msgBody.m_msgJsonData[DISKNAMES].append("disk1");
    CFTExceptionHandle::GetInstance().UpdateFreezeOper(&reqMsg, &rspMsg);
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, PushUnFreezeReq)
{
    MONITOR_OBJ obj;
    mp_int32 rst = CFTExceptionHandle::GetInstance().PushUnFreezeReq(&obj);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CFTExceptionHandleTest, Add_DelMonitorObjs)
{
    MONITOR_OBJ obj;
    obj.iAppType = TYPE_APP_DB2;
    obj.strInstanceName = "db2inst1";
    obj.strDBName = "db_sss";
    CRequestMsg* reqMsg = new CRequestMsg;
    reqMsg->m_httpReq.m_pFcgRequest = new FCGX_Request;
    reqMsg->m_httpReq.m_pFcgRequest->envp = new char*[2];
    reqMsg->m_httpReq.m_pFcgRequest->envp[0] = new char[3];
    reqMsg->m_httpReq.m_pFcgRequest->envp[1] = new char[3];
    obj.pReqMsg = reqMsg;
    vector<MONITOR_OBJ> vec;
    vec.push_back(obj);
    MONITOR_OBJ obj2;
    obj2.iAppType = TYPE_APP_ORACLE;
    obj2.strInstanceName = "db2inst1";
    obj2.strDBName = "db_sss";
    CRequestMsg* reqMsg1 = new CRequestMsg;
    reqMsg1->m_httpReq.m_pFcgRequest = new FCGX_Request;
    reqMsg1->m_httpReq.m_pFcgRequest->envp = new char*[2];
    reqMsg1->m_httpReq.m_pFcgRequest->envp[0] = new char[3];
    reqMsg1->m_httpReq.m_pFcgRequest->envp[1] = new char[3];
    obj2.pReqMsg = reqMsg1;
    vec.push_back(obj2);
    CFTExceptionHandle::GetInstance().AddMonitorObjs(vec);
    CFTExceptionHandle::GetInstance().DelMonitorObj(&obj2);
    CFTExceptionHandle::GetInstance().DelMonitorObj(&obj);
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, LoadFromDB)
{
    vector<MONITOR_OBJ> vecObj;
    Stub<CDBQueryTableType, StubCDBQueryTableType, mp_void> mystub(&CDB::QueryTable, &StubCDBQueryTableOk);
    mp_int32 rst = CFTExceptionHandle::GetInstance().LoadFromDB(vecObj);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CFTExceptionHandleTest, GetDBNameFromObj)
{
    MONITOR_OBJ obj;
    CRequestMsg req;
    obj.pReqMsg = &req;
    obj.iAppType = TYPE_APP_FILESYSTEM;
    obj.strInstanceName = "test";
    mp_string rst = CFTExceptionHandle::GetInstance().GetDBNameFromObj(&obj);
    EXPECT_EQ(rst, "test");
}
TEST_F(CFTExceptionHandleTest, GetRequestInstanceName)
{
    CRequestMsg req;
    mp_string inst;
    //vss
    req.m_url.m_procURL = REST_SQLSERVER_FREEZE_DB;
    mp_int32 rst = CFTExceptionHandle::GetInstance().GetRequestInstanceName(&req, inst);
    EXPECT_EQ(rst, MP_SUCCESS);
    //fs
    req.m_url.m_procURL = REST_DEVICE_FILESYS_FREEZE;
    req.m_msgBody.m_msgJsonData[DISKNAMES].append("disk0");
    req.m_msgBody.m_msgJsonData[DISKNAMES].append("disk1");
    rst = CFTExceptionHandle::GetInstance().GetRequestInstanceName(&req, inst);
    EXPECT_EQ(rst, MP_SUCCESS);
    //other
    req.m_url.m_procURL = "no support";
    rst = CFTExceptionHandle::GetInstance().GetRequestInstanceName(&req, inst);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CFTExceptionHandleTest, GetRequestDbName)
{
    CRequestMsg req;
    mp_string db;
    //vss
    req.m_url.m_procURL = REST_SQLSERVER_FREEZE_DB;
    mp_int32 rst = CFTExceptionHandle::GetInstance().GetRequestDbName(&req, db);
    EXPECT_EQ(rst, MP_SUCCESS);
    //fs
    req.m_url.m_procURL = REST_DEVICE_FILESYS_FREEZE;
    rst = CFTExceptionHandle::GetInstance().GetRequestDbName(&req, db);
    EXPECT_EQ(rst, MP_SUCCESS);
    //other
    req.m_url.m_procURL = "no support";
    rst = CFTExceptionHandle::GetInstance().GetRequestDbName(&req, db);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CFTExceptionHandleTest, GetQueryStatusUrl)
{
    mp_string rst = CFTExceptionHandle::GetInstance().GetQueryStatusUrl(TYPE_APP_ORACLE);
    EXPECT_EQ(rst, REST_ORACLE_FREEZESTATE);
    rst = CFTExceptionHandle::GetInstance().GetQueryStatusUrl(TYPE_APP_SQL);
    EXPECT_EQ(rst, REST_SQLSERVER_GET_FREEZE_STAT);
    rst = CFTExceptionHandle::GetInstance().GetQueryStatusUrl(TYPE_APP_EXCHANGE);
    EXPECT_EQ(rst, REST_EXCHANGE_GET_FREEZE_STAT);
    rst = CFTExceptionHandle::GetInstance().GetQueryStatusUrl(TYPE_APP_FILESYSTEM);
    EXPECT_EQ(rst, REST_DEVICE_FILESYS_FREEZESTATUS);
}
TEST_F(CFTExceptionHandleTest, GetUnFreezeUrl)
{
    mp_string rst = CFTExceptionHandle::GetInstance().GetUnFreezeUrl(TYPE_APP_ORACLE);
    EXPECT_EQ(rst, REST_ORACLE_UNFREEZE);
    rst = CFTExceptionHandle::GetInstance().GetUnFreezeUrl(TYPE_APP_SQL);
    EXPECT_EQ(rst, REST_SQLSERVER_UNFREEZE_DB);
    rst = CFTExceptionHandle::GetInstance().GetUnFreezeUrl(TYPE_APP_EXCHANGE);
    EXPECT_EQ(rst, REST_EXCHANGE_UNFREEZE_DB);
    rst = CFTExceptionHandle::GetInstance().GetUnFreezeUrl(TYPE_APP_FILESYSTEM);
    EXPECT_EQ(rst, REST_DEVICE_FILESYS_UNFREEZE);
}
TEST_F(CFTExceptionHandleTest, Init)
{
    mp_int32 rst = 0;
    //LoadFromDB < 0
    {
        Stub<CDBQueryTableType, StubCDBQueryTableType, mp_void> mystub1(&CDB::QueryTable, &StubCDBQueryTableLt0);
        rst = CFTExceptionHandle::GetInstance().Init();
        EXPECT_EQ(rst, -1);
    }
    //Create < 0
    {
        Stub<CDBQueryTableType, StubCDBQueryTableType, mp_void> mystub2(&CDB::QueryTable, &StubCDBQueryTableEq0);
        Stub<CMpThreadCreateType, StubCMpThreadCreateType, mp_void> mystub3(&CMpThread::Create, &StubCMpThreadCreateLt0);
        rst = CFTExceptionHandle::GetInstance().Init();
        EXPECT_EQ(rst, -1);
    }
}
//End CFTExceptionHandleTest
//Begin CTaskDispatchWorkerTest
TEST_F(CTaskDispatchWorkerTest, Init)
{
    CTaskDispatchWorker dspwrk;
    Stub<CMpThreadCreateType, StubCMpThreadCreateType, mp_void> mystub(&CMpThread::Create, &StubCMpThreadCreateLt0);
    mp_int32 rst = dspwrk.Init(NULL, 0);
    EXPECT_EQ(rst, -1);
}
TEST_F(CTaskDispatchWorkerTest, NeedExit)
{
    CTaskDispatchWorker dspwrk;
    mp_bool rst = dspwrk.NeedExit();
    EXPECT_EQ(rst, dspwrk.m_bNeedExit);
}
TEST_F(CTaskDispatchWorkerTest, PushMsgToWorker)
{
    CTaskDispatchWorker dspwrk;
    message_pair_t stPair;
    //m_iWorkerCount <= 0
    dspwrk.PushMsgToWorker(stPair);
    //
    dspwrk.m_iWorkerCount = 1;
    CTaskWorker *wrk = new CTaskWorker;
    dspwrk.m_pWorkers = &wrk;
    dspwrk.PushMsgToWorker(stPair);
    EXPECT_TRUE(1);
    delete wrk;
}
TEST_F(CTaskDispatchWorkerTest, DispacthProc)
{
    CTaskDispatchWorker dspwrk;
    dspwrk.m_bNeedExit = 1;
    mp_void* rst = dspwrk.DispacthProc(&dspwrk);
    mp_void* pnull = NULL;
    EXPECT_EQ(rst, pnull);
}
TEST_F(CTaskDispatchWorkerTest, Exit)
{
    CTaskDispatchWorker dspwrk;
    Stub<CMpThreadWaitForEndType, StubCMpThreadWaitForEndType, mp_void> mystub(&CMpThread::WaitForEnd, &StubCMpThreadWaitForEndEq0);
    dspwrk.Exit();
    EXPECT_TRUE(1);
}
//End CTaskDispatchWorkerTest
//Begin CTaskPoolTest
TEST_F(CTaskPoolTest, Init_Exit)
{
    mp_int32 rst = 0;
    Stub<CTaskWorkerExitType, StubCTaskWorkerExitType, mp_void> mystub(&CTaskWorker::Exit, &StubCTaskWorkerExitNull);
    //CreatePlgConfParse < 0
    {
        CTaskPool tskpool;
        Stub<CPluginCfgParseInitType, StubCPluginCfgParseInitType, mp_void> mystub1(&CPluginCfgParse::Init, &StubCPluginCfgParseInitLt0);
        rst = tskpool.Init();
        EXPECT_EQ(rst, -1);
    }
    //CreatePlugMgr < 0
    {
        CTaskPool tskpool;
        Stub<CPluginCfgParseInitType, StubCPluginCfgParseInitType, mp_void> mystub2(&CPluginCfgParse::Init, &StubCPluginCfgParseInitEq0);
        Stub<CPluginManagerInitializeType, StubCPluginManagerInitializeType, mp_void> mystub3(&CPluginManager::Initialize, &StubCPluginManagerInitializeLt0);
        rst = tskpool.Init();
        EXPECT_EQ(rst, -1);
    }
    //CreateWorkers < 0
    {
        CTaskPool tskpool;
        Stub<CPluginCfgParseInitType, StubCPluginCfgParseInitType, mp_void> mystub4(&CPluginCfgParse::Init, &StubCPluginCfgParseInitEq0);
        Stub<CTaskWorkerInitType, StubCTaskWorkerInitType, mp_void> mystub6(&CTaskWorker::Init, &StubCTaskWorkerInitLt0);
        rst = tskpool.Init();
        EXPECT_EQ(rst, -1);
    }
    //CreateDispatchWorker < 0
    {
        CTaskPool tskpool;
        Stub<CPluginCfgParseInitType, StubCPluginCfgParseInitType, mp_void> mystub7(&CPluginCfgParse::Init, &StubCPluginCfgParseInitEq0);
        Stub<CTaskWorkerInitType, StubCTaskWorkerInitType, mp_void> mystub9(&CTaskWorker::Init, &StubCTaskWorkerInitEq0);
        Stub<CTaskDispatchWorkerInitType, StubCTaskDispatchWorkerInitType, mp_void> mystub10(&CTaskDispatchWorker::Init, &StubCTaskDispatchWorkerInitLt0);
        rst = tskpool.Init();
        EXPECT_EQ(rst, -1);
    }
}
TEST_F(CTaskPoolTest, CanUnload)
{
    CTaskPool tskpool;
    mp_int32 rst = tskpool.CanUnload(NULL);
    EXPECT_EQ(rst, 0);
}
TEST_F(CTaskPoolTest, OnUpgraded)
{
    CTaskPool tskpool;
    tskpool.OnUpgraded(NULL, NULL);
    EXPECT_TRUE(1);
}
TEST_F(CTaskPoolTest, SetOptions)
{
    CTaskPool tskpool;
    tskpool.SetOptions(NULL);
    EXPECT_TRUE(1);
}
TEST_F(CTaskPoolTest, GetReleaseVersion)
{
    CTaskPool tskpool;
    mp_char* pc = tskpool.GetReleaseVersion(NULL, NULL, 0);
    mp_char* pnull = NULL;
    EXPECT_EQ(pc, pnull);
}
//End CTaskPoolTest
//Begin CTaskProtectWorkerTest
TEST_F(CTaskProtectWorkerTest, Con_Des)
{
    CTaskProtectWorker pwrk;
}
//End CTaskProtectWorkerTest
//Begin CTaskWorkerTest
TEST_F(CTaskWorkerTest, Init)
{
    CTaskWorker work;
    mp_int32 rst  = work.Init(NULL, NULL);
    EXPECT_EQ(rst, -1);
    CPluginCfgParse pPlgCfgParse;
    CPluginManager pPlgMgr;
    Stub<CMpThreadCreateType, StubCMpThreadCreateType, mp_void> mystub(&CMpThread::Create, &StubCMpThreadCreateLt0);
    rst  = work.Init(&pPlgCfgParse, &pPlgMgr);
    EXPECT_EQ(rst, -1);
}
TEST_F(CTaskWorkerTest, Exit)
{
    CTaskWorker work;
    Stub<CMpThreadWaitForEndType, StubCMpThreadWaitForEndType, mp_void> mystub(&CMpThread::WaitForEnd, &StubCMpThreadWaitForEndEq0);
    work.Exit();
    EXPECT_TRUE(1);
}
TEST_F(CTaskWorkerTest, WorkProc)
{
    CTaskWorker work;
    Stub<CTaskWorkerNeedExitType, StubCTaskWorkerNeedExitType, mp_void> mystub(&CTaskWorker::NeedExit, &StubCTaskWorkerNeedExitEq1);
    mp_void* rst = work.WorkProc(&work);
    mp_void* pnull = NULL;
    EXPECT_EQ(rst, pnull);
}
TEST_F(CTaskWorkerTest, CanUnloadPlugin)
{
    CTaskWorker work;
    mp_int32 rst = work.CanUnloadPlugin(0, NULL);
    EXPECT_EQ(rst, 1);
}
TEST_F(CTaskWorkerTest, push_popreq)
{
    CTaskWorker work;
    message_pair_t stPair;
    mp_int32 rst = work.PopRequest(stPair);
    EXPECT_EQ(rst, MP_FAILED);
    work.PushRequest(stPair);
    rst = work.PopRequest(stPair);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CTaskWorkerTest, NeedExit)
{
    CTaskWorker work;
    mp_bool rst = work.NeedExit();
    EXPECT_EQ(rst, work.m_bNeedExit);
}
TEST_F(CTaskWorkerTest, GetPlugin)
{
    CTaskWorker work;
    CPluginCfgParse cfgprs;
    CPluginManager mng;
    work.m_plgCfgParse = &cfgprs;
    work.m_plgMgr = &mng;
    //NULL
    mp_void* pnull = NULL;
    CServicePlugin* rst = work.GetPlugin(NULL);
    EXPECT_EQ(rst, pnull);
    //
    Stub<CPluginCfgParseGetPluginByServiceType, StubCPluginCfgParseGetPluginByServiceType, mp_void> mystub1(&CPluginCfgParse::GetPluginByService, &StubCPluginCfgParseGetPluginByServiceEq0);
    Stub<CPluginManagerGetPluginType, StubCPluginManagerGetPluginType, mp_void> mystub2(&CPluginManager::GetPlugin, &StubCPluginManagerGetPluginOK);
    rst = work.GetPlugin("db2");
    if (rst)
    {
        delete rst;
    }
}
TEST_F(CTaskWorkerTest, SetPlugin)
{
    CTaskWorker work;
    CDb2Plugin plg;
    work.SetPlugin(&plg);
    EXPECT_TRUE(1);
}
//End CTaskWorkerTest
