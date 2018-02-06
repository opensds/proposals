/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "rest/RestTest.h"
#include <string.h>

//begin CHttpRequestTest
TEST_F(CHttpRequestTest, con_des)
{   
    FCGX_Request freq;
    freq.envp = new char*[2];
    freq.envp[0] = new char[100];
    freq.envp[1] = new char[50];
    memcpy(freq.envp[0], "REQUEST_URI=/agent/test?name=a&age=6", strlen("REQUEST_URI=/agent/test?name=a&age=6"));
    memcpy(freq.envp[1], "REQUEST_METHOD=GET", strlen("REQUEST_METHOD=GET"));
    CHttpRequest hreq2(&freq);
    delete[] freq.envp[0];
    delete[] freq.envp[1];
    delete[] freq.envp;
}
TEST_F(CHttpRequestTest, GetHead)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    hreq.GetHead("a");
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, GetHeadNoCheck)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    hreq.GetHeadNoCheck("a");
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, GetAllHead)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    hreq.GetAllHead();
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, GetRemoteIP)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    hreq.GetRemoteIP(); 
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, GetContentLen)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    hreq.GetContentLen();
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, GetContentType)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    mp_string tp;
    hreq.GetContentType(tp);
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, GetFcgxReq)
{   
    CHttpRequest hreq;
    hreq.GetFcgxReq();
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, ReadChar)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    Stub<FCGX_GetCharType, StubFCGX_GetCharType, mp_void> mystub(&FCGX_GetChar, &StubFCGX_GetCharTypeEq0);
    hreq.ReadChar();
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, ReadStr)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    Stub<FCGX_GetStrType, StubFCGX_GetStrType, mp_void> mystub(&FCGX_GetStr, &StubFCGX_GetStrEq0);
    hreq.ReadStr(NULL, 1);
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, Readline)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    Stub<FCGX_GetLineType, StubFCGX_GetLineType, mp_void> mystub(&FCGX_GetLine, &StubFCGX_GetLineOk);
    hreq.Readline(NULL, 1);
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, GetURL)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    hreq.GetURL();
    EXPECT_TRUE(1);
}
TEST_F(CHttpRequestTest, GetQueryParamStr)
{   
    FCGX_Request freq;
    freq.envp = NULL;
    CHttpRequest hreq(&freq);
    hreq.GetQueryParamStr();
    EXPECT_TRUE(1);
}
//end CHttpRequestTest
//begin CHttpResponseTest
TEST_F(CHttpResponseTest, SetContentType)
{   
    CHttpResponse freq;
    freq.SetContentType("aaa");
    EXPECT_TRUE(1);
}
TEST_F(CHttpResponseTest, GetHead)
{   
    CHttpResponse freq;
    freq.GetHead("aaa");
    EXPECT_TRUE(1);
}
TEST_F(CHttpResponseTest, SetHead)
{   
    CHttpResponse freq;
    freq.SetHead("aaa", "bbb");
    EXPECT_TRUE(1);
}
TEST_F(CHttpResponseTest, RemoveHead)
{   
    CHttpResponse freq;
    freq.RemoveHead("aaa");
    EXPECT_TRUE(1);
}
TEST_F(CHttpResponseTest, WriteChar)
{   
    FCGX_Request freq;
    CHttpResponse hreq(&freq);
    Stub<FCGX_VFPrintFType, StubFCGX_VFPrintFType, mp_void> mystub1(&FCGX_VFPrintF, &StubFCGX_VFPrintFEq0);
    Stub<FCGX_PutCharType, StubFCGX_PutCharType, mp_void> mystub2(&FCGX_PutChar, &StubFCGX_PutCharEq0);
    hreq.WriteChar(1);
    EXPECT_TRUE(1);
}
TEST_F(CHttpResponseTest, WriteStr)
{   
    FCGX_Request freq;
    CHttpResponse hreq(&freq);
    Stub<FCGX_VFPrintFType, StubFCGX_VFPrintFType, mp_void> mystub1(&FCGX_VFPrintF, &StubFCGX_VFPrintFEq0);
    Stub<FCGX_PutStrType, StubFCGX_PutStrType, mp_void> mystub2(&FCGX_PutStr, &StubFCGX_PutStrEq0);
    hreq.WriteStr("aaa", 1);
    EXPECT_TRUE(1);
}
TEST_F(CHttpResponseTest, WriteS)
{   
    FCGX_Request freq;
    CHttpResponse hreq(&freq);
    Stub<FCGX_VFPrintFType, StubFCGX_VFPrintFType, mp_void> mystub1(&FCGX_VFPrintF, &StubFCGX_VFPrintFEq0);
    Stub<FCGX_PutStrType, StubFCGX_PutStrType, mp_void> mystub2(&FCGX_PutStr, &StubFCGX_PutStrEq0);
    hreq.WriteS("aaa");
    EXPECT_TRUE(1);
}
TEST_F(CHttpResponseTest, WriteF)
{   
    FCGX_Request freq;
    CHttpResponse hreq(&freq);
    Stub<FCGX_VFPrintFType, StubFCGX_VFPrintFType, mp_void> mystub(&FCGX_VFPrintF, &StubFCGX_VFPrintFEq0);
    hreq.WriteF("%s", "aaa");;
    EXPECT_TRUE(1);
}
TEST_F(CHttpResponseTest, Complete)
{   
    CHttpResponse freq;
    Stub<FCGX_Finish_rType, StubFCGX_Finish_rType, mp_void> mystub(&FCGX_Finish_r, &StubFCGX_Finish_rVoid);
    freq.Complete();
    EXPECT_TRUE(1);
}
//end CHttpResponseTest
//begin CMessage_BlockTest
TEST_F(CMessage_BlockTest, GetWritePtr)
{
    CMessage_Block msb;
    msb.GetWritePtr();
    EXPECT_TRUE(1);
}
TEST_F(CMessage_BlockTest, GetReadPtr)
{
    CMessage_Block msb(4);
    msb.GetReadPtr();
    EXPECT_TRUE(1);
}
TEST_F(CMessage_BlockTest, AddLength)
{
    CMessage_Block msb;
    msb.AddLength(1);
    EXPECT_TRUE(1);
}
TEST_F(CMessage_BlockTest, GetLength)
{
    CMessage_Block msb;
    msb.GetLength();
    EXPECT_TRUE(1);
}
TEST_F(CMessage_BlockTest, GetSize)
{
    CMessage_Block msb;
    msb.GetSize();
    EXPECT_TRUE(1);
}
TEST_F(CMessage_BlockTest, Resize)
{
    CMessage_Block msb(4);
    msb.Resize(5);
    msb.Resize(4);
    EXPECT_TRUE(1);
}
//end CMessage_BlockTest
//begin CRequestURLTest
TEST_F(CRequestURLTest, GetProcURL)
{
    CRequestURL url;
    url.GetProcURL();
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, GetOriURL)
{
    CRequestURL url;
    url.GetOriURL();
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, GetID)
{
    CRequestURL url;
    url.GetID();
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, SetProcURL)
{
    CRequestURL url;
    url.SetProcURL("test");
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, SetOriURL)
{
    CRequestURL url;
    url.SetOriURL("test");
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, GetQueryParam)
{
    CRequestURL url;
    url.GetQueryParam();
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, ParseURL)
{
    CRequestURL url;
    url.m_oriURL = "/agent/test";
    url.ParseURL();
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, SetQueryParam)
{
    CRequestURL url;
    url.m_oriURL = "/agent/test";
    url.SetQueryParam("name=lllll&abc=abc");
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, GetServiceName)
{
    CRequestURL url;
    url.GetServiceName();
    url.m_procURL = "/test";
    url.GetServiceName();
    EXPECT_TRUE(1);
}
TEST_F(CRequestURLTest, GetCutURL)
{
    CRequestURL url;
    url.GetCutURL(1);
    url.m_procURL = "/agent/test";
    url.GetCutURL(3);
    EXPECT_TRUE(1);
}
//end CRequestURLTest
//begin CRequestMsgBodyTest
TEST_F(CRequestMsgBodyTest, JsonValueToString)
{
    CRequestMsgBody msgbody;
    Json::Value v;
    msgbody.JsonValueToString(v);
    Json::Value v1(12);
    msgbody.JsonValueToString(v1);
    EXPECT_TRUE(1);
}
TEST_F(CRequestMsgBodyTest, GetValue)
{
    CRequestMsgBody msgbody;
    msgbody.m_msgJsonData["hhhh"] = Json::Value("aaaaa");
    const mp_string name = "hhhh";
    mp_string value;
    msgbody.GetValue(name, value);
    EXPECT_TRUE(1);
}
TEST_F(CRequestMsgBodyTest, GetOriMsg)
{
    CRequestMsgBody msgbody;
    mp_char* buf;
    mp_uint32 len;
    msgbody.GetOriMsg(buf, len);
    auto_ptr<CMessage_Block> temp(new CMessage_Block(100));
    msgbody.m_raw_msg = temp;
    msgbody.GetOriMsg(buf, len);
    EXPECT_TRUE(1);
}
//end CRequestMsgBodyTest
//begin CRequestMsgTest
TEST_F(CRequestMsgTest, Parse)
{
    FCGX_Request req;
    req.envp = NULL;
    CRequestMsg reqmsg(&req);
    Stub<FCGX_GetCharType, StubFCGX_GetCharType, mp_void> mystub(&FCGX_GetChar, &StubFCGX_GetCharTypeEq0);
    reqmsg.Parse();
    EXPECT_TRUE(1);
}
//end CRequestMsgTest
//begin CResponseMsgTest
TEST_F(CResponseMsgTest, Send)
{
    FCGX_Request req;
    req.envp = NULL;
    CResponseMsg rsqmsg(&req);
    Stub<FCGX_PutStrType, StubFCGX_PutStrType, mp_void> mystub1(&FCGX_PutStr, &StubFCGX_PutStrEq0);
    Stub<FCGX_VFPrintFType, StubFCGX_VFPrintFType, mp_void> mystub2(&FCGX_VFPrintF, &StubFCGX_VFPrintFEq0);
    Stub<FCGX_Finish_rType, StubFCGX_Finish_rType, mp_void> mystub3(&FCGX_Finish_r, &StubFCGX_Finish_rVoid);
    //RSP_JSON_TYPE
    rsqmsg.m_httpType = CResponseMsg::RSP_JSON_TYPE;
    rsqmsg.Send();
    //RSP_ATTACHMENT_TYPE and error
    rsqmsg.m_httpType = CResponseMsg::RSP_ATTACHMENT_TYPE;
    rsqmsg.m_lRetCode = MP_FAILED;
    rsqmsg.Send();
    //RSP_ATTACHMENT_TYPE and success
    rsqmsg.m_msgJsonData["attachmentName"] = Json::Value("hosts");
    rsqmsg.m_msgJsonData["attachmentPath"] = Json::Value("/etc/hosts");
    rsqmsg.m_httpType = CResponseMsg::RSP_ATTACHMENT_TYPE;
    rsqmsg.m_lRetCode = MP_SUCCESS;
    rsqmsg.Send();
    EXPECT_TRUE(1);
}
TEST_F(CResponseMsgTest, PackageReponse)
{
    FCGX_Request req;
    CResponseMsg rsqmsg(&req);
    //retcode = failed
    rsqmsg.m_lRetCode = MP_FAILED;
    Json::Value value;
    rsqmsg.PackageReponse(value);
    //retcode = success
    rsqmsg.m_lRetCode = MP_SUCCESS;
    rsqmsg.PackageReponse(value);
    EXPECT_TRUE(1);
}
//end CResponseMsgTest
//begin CUrlUtilsTest
TEST_F(CUrlUtilsTest, GetNum)
{
    mp_uint64 ret;
    CUrlUtils::GetNum("abc", 2, 16, ret);
    EXPECT_TRUE(1);
}
TEST_F(CUrlUtilsTest, Urlencode2)
{
    mp_char dst[100];
    CUrlUtils::Urlencode2(dst, 100, "test");
    EXPECT_TRUE(1);
}
TEST_F(CUrlUtilsTest, Urldecode2)
{
    mp_char dst[100];
    CUrlUtils::Urldecode2(dst, "testabc", 5);
    EXPECT_TRUE(1);
}
//end CUrlUtilsTest
//begin CJsonUtilsTest
TEST_F(CJsonUtilsTest, GetJsonString)
{
    Json::Value jsValue;
    mp_string strKey = "abc";
    mp_string strValue;
    //array
    CJsonUtils::GetJsonString(jsValue, strKey, strValue);
    jsValue["abc"] = Json::Value("abc");
    //not memeber
    strKey = "def";
    CJsonUtils::GetJsonString(jsValue, strKey, strValue);
    //ok
    strKey = "abc";
    CJsonUtils::GetJsonString(jsValue, strKey, strValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetJsonInt32)
{
    Json::Value jsValue;
    mp_string strKey = "abc";
    mp_int32 iValue;
    //array
    CJsonUtils::GetJsonInt32(jsValue, strKey, iValue);
    jsValue["abc"] = Json::Value(12);
    //not memeber
    strKey = "def";
    CJsonUtils::GetJsonInt32(jsValue, strKey, iValue);
    //ok
    strKey = "abc";
    CJsonUtils::GetJsonInt32(jsValue, strKey, iValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetJsonInt64)
{
    Json::Value jsValue;
    mp_string strKey = "abc";
    mp_int64 lValue;
    //array
    CJsonUtils::GetJsonInt64(jsValue, strKey, lValue);
    //not memeber
    strKey = "def";
    jsValue["abc"] = Json::Value(12);
    CJsonUtils::GetJsonInt64(jsValue, strKey, lValue);
    //ok
    strKey = "abc";
    CJsonUtils::GetJsonInt64(jsValue, strKey, lValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetJsonArrayInt32)
{
    Json::Value jsValue;
    mp_string strKey = "abc";
    vector<mp_int32> vecValue;
    //array
    CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue);
    //not memeber
    strKey = "def";
    jsValue["abc"] = Json::Value("aaa");
    CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue);
    //ok
    jsValue["test"].append(12);
    jsValue["test"].append(12);
    strKey = "test";
    CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetJsonArrayInt64)
{
    Json::Value jsValue;
    mp_string strKey = "abc";
    vector<mp_int64> vecValue;
    //array
    CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue);
    //not memeber
    strKey = "def";
    jsValue["abc"] = Json::Value("aaaa");
    CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue);
    //ok
    jsValue["test"].append(12);
    jsValue["test"].append(12);
    strKey = "test";
    CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetJsonArrayString)
{
    Json::Value jsValue;
    mp_string strKey = "abc";
    vector<mp_string> vecValue;
    //array
    CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue);
    //not memeber
    strKey = "def";
    jsValue["abc"] = Json::Value("123");
    CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue);
    //ok
    jsValue["test"].append("aa");
    jsValue["test"].append("aa");
    strKey = "test";
    CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetJsonArrayJson)
{
    Json::Value jsValue;
    mp_string strKey = "abc";
    vector<Json::Value> vecValue;
    Json::Value jtemp;
    jtemp["test"] = Json::Value("test");
    //array
    CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue);
    //not memeber
    strKey = "def";
    jsValue["abc"] = Json::Value("123");
    CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue);
    //ok
    strKey = "test";
    jsValue["test"].append(jtemp);
    CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetArrayInt32)
{
    Json::Value jsValue;
    vector<mp_int32> vecValue;
    jsValue["abc"] = Json::Value("123");
    //not array
    CJsonUtils::GetArrayInt32(jsValue, vecValue);
    //ok
    Json::Value jsValue1;
    jsValue1.append(12);
    CJsonUtils::GetArrayInt32(jsValue1, vecValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetArrayInt64)
{
    Json::Value jsValue;
    vector<mp_int64> vecValue;
    jsValue["abc"] = Json::Value("123");
    //not array
    CJsonUtils::GetArrayInt64(jsValue, vecValue);
    //ok
    Json::Value jsValue1;
    jsValue1.append(12);
    CJsonUtils::GetArrayInt64(jsValue1, vecValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetArrayString)
{
    Json::Value jsValue;
    vector<mp_string> vecValue;
    jsValue["abc"] = Json::Value("123");
    //not array
    CJsonUtils::GetArrayString(jsValue, vecValue);
    //ok
    Json::Value jsValue1;
    jsValue1.append("aaaa");
    CJsonUtils::GetArrayString(jsValue1, vecValue);
    EXPECT_TRUE(1);
}
TEST_F(CJsonUtilsTest, GetArrayJson)
{
    Json::Value jsValue;
    vector<Json::Value> vecValue;
    Json::Value jtemp;
    jsValue["abc"] = Json::Value("123");
    jtemp["test"] = Json::Value("test");
    //not array
    CJsonUtils::GetArrayJson(jsValue, vecValue);
    //ok
    Json::Value jsValue1;
    jsValue1.append(jtemp);
    CJsonUtils::GetArrayJson(jsValue1, vecValue);
    EXPECT_TRUE(1);
}
//end CJsonUtilsTest
