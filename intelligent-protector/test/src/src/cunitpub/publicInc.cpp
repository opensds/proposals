/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "cunitpub/publicInc.h"
#include "jsoncpp/include/json/value.h"
#include "jsoncpp/include/json/json.h"
#include "rest/MessageProcess.h"

/* 日志; */
typedef mp_void (CLogger::*pOrgLog)(mp_int32 iLevel, const mp_int32 iFileLine, mp_uint64 ulCode,const mp_char* pszFileName, const mp_char* pszFormat, ...);
typedef mp_void (*pStubLog)(mp_void);
static Stub<pOrgLog, pStubLog, mp_void> *gp_stubLog = NULL;

/* xml解析;*/
typedef mp_int32 (CConfigXmlParser::*pOrgGetValueString0)(mp_string strSection, mp_string strKey, mp_string& strValue);
typedef mp_int32 (*pStubGetValueString)(mp_void);
static Stub<pOrgGetValueString0, pStubGetValueString, mp_void> *gp_CConfigXmlParser_0 = NULL;

typedef mp_int32 (CConfigXmlParser::*pOrgGetValueString1)(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_string& strValue);
static Stub<pOrgGetValueString1, pStubGetValueString, mp_void> *gp_CConfigXmlParser_1 = NULL;

typedef mp_int32 (*pOrgGetJsonString)(const Json::Value& jsValue, mp_string strKey, mp_string& strValue);
static Stub<pOrgGetJsonString, pStubIntType, mp_void> *gp_GetJsonString = NULL;

typedef mp_int32 (*pOrgGetJsonInt32)(const Json::Value& jsValue, mp_string strKey, mp_int32& iValue);
static Stub<pOrgGetJsonInt32, pStubIntType, mp_void> *gp_GetJsonInt32 = NULL;

typedef mp_int32 (*pOrgGetJsonArrayString)(const Json::Value& jsValue, mp_string strKey, std::vector<mp_string>& vecValue);
static Stub<pOrgGetJsonArrayString, pStubIntType, mp_void> *gp_GetJsonArrayString  = NULL;

typedef mp_int32 (*pOrgGetJsonInt64)(const Json::Value& jsValue, mp_string strKey, mp_int64& lValue);
static Stub<pOrgGetJsonInt64, pStubIntType, mp_void> *gp_GetJsonInt64  = NULL;

typedef mp_int32 (*pOrgGetJsonArrayJson)(const Json::Value& jsValue, mp_string strKey, vector<Json::Value>& vecValue);
static Stub<pOrgGetJsonArrayJson, pStubIntType, mp_void> *gp_GetJsonArrayJson  = NULL;

typedef mp_int32 (*pOrgExecSystemWithEcho)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);
typedef mp_int32 (*pStubExecSystemWithEcho)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);
static Stub<pOrgExecSystemWithEcho, pStubExecSystemWithEcho, mp_void> *gp_ExecSystemWithEcho  = NULL;

typedef mp_int32 (*pOrgExecSystemWithoutEcho)(mp_string& strCommand, mp_bool bNeedRedirect);
typedef mp_int32 (*pStubExecSystemWithoutEcho)(mp_void);
static Stub<pOrgExecSystemWithoutEcho, pStubExecSystemWithoutEcho, mp_void> *gp_ExecSystemWithoutEcho  = NULL;

typedef mp_int32 (*pOrgExecScript)(mp_string strScriptFileName, mp_string strParam, vector<mp_string>* pvecResult, mp_bool bNeedCheckSign);
typedef mp_int32 (*pStubExecScript)(mp_string strScriptFileName, mp_string strParam, vector<mp_string>* pvecResult, mp_bool bNeedCheckSign);
static Stub<pOrgExecScript, pStubExecScript, mp_void> *gp_ExecScript  = NULL;

typedef mp_int32 (*pOrgCheckScriptSign)(const mp_string strFileName);
typedef mp_int32 (*pStubCheckScriptSign)(mp_void);
static Stub<pOrgCheckScriptSign, pStubCheckScriptSign, mp_void> *gp_CheckScriptSign  = NULL;

/* 用户输入; */
typedef mp_void (*pOrgInputUserPwd)(mp_string strUserName, mp_string &strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen);
static Stub<pOrgInputUserPwd, pStubVoidType, mp_void> *gp_InputUserPwd  = NULL;

static  mp_int32  CUNIT_iCounter = 0;
static  mp_int32  CUNIT_bCounter = 0;
static  mp_int32  CUNIT_cCounter = 0;
static  mp_int32  CUNIT_sCounter = 0;
static  mp_int32  CUNIT_rCounter = 0;
static  mp_int32  CUNIT_nCounter = 0;
static  mp_int32  CUNIT_pCounter = 0;
static  mp_int32  CUNIT_SystemWithEchoCounter = 0;
static  mp_int32  CUNIT_SystemWithoutEchoCounter = 0;
static  mp_int32  CUNIT_ExecScriptCounter = 0;
static  mp_int32  CUNIT_CheckScriptSignCounter = 0;


static mp_int32 stub_return_success()
{
    return MP_SUCCESS;
}

static mp_int32 stubCheckScriptSign(mp_void)
{
    if (CUNIT_CheckScriptSignCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 stubExecSystemWithoutEcho()
{
    if (CUNIT_SystemWithoutEchoCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 stubExecScript(mp_string strScriptFileName, mp_string strParam, vector<mp_string>* pvecResult, mp_bool bNeedCheckSign)
{
    if (CUNIT_ExecScriptCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        if (pvecResult)  pvecResult->push_back("123");
        return MP_SUCCESS;
    }
}


static mp_int32 stubSystemWithEcho(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    if (CUNIT_SystemWithEchoCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        strEcho.push_back("123");
        return MP_SUCCESS;
    }
}

mp_void stub_set_cpasswdString(mp_string strHint, mp_string& strInput)
{
    if (CUNIT_pCounter++ == 0)
    {
        strInput = "15";
    }
    else
    {
        strInput = toString<mp_int32>(CUNIT_pCounter);
    }
}


mp_void stub_set_cpasswdLongString(mp_string strHint, mp_string& strInput)
{
    if (CUNIT_pCounter++ == 0)
    {
        strInput = "Agent&12345679891234567891234567989123456789123456789123456789123456789";
    }
    else
    {
        strInput = "Agent&123";
    }
}


mp_void stub_set_string(mp_string &strInput)
{
    if (CUNIT_sCounter++)
    {
        strInput = "Agent&123";
    }
    else
    {
        strInput = "Agent&12345679891234567891234567989123456789123456789123456789123456789";
    }
}


mp_void stub_set_numberStr(mp_string &strInput)
{
    if (CUNIT_iCounter== 0)
    {
        strInput = "15";
    }
    strInput = toString<mp_int32>(CUNIT_iCounter++);
}


mp_bool stub_return_bool(mp_void)
{
    if (CUNIT_bCounter++ == 0)
    {
        return MP_FALSE;
    }
    else
    {
        return MP_TRUE;
    }
}


mp_int32 stub_return_ret(mp_void)
{
    if (CUNIT_rCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

mp_int32 stub_return_number(mp_void)
{
    return CUNIT_nCounter++;
}


mp_string stub_return_string(mp_void)
{   
    mp_string strInput;
    stub_set_numberStr(strInput);

    return strInput;
}


const mp_char * stub_return_cstring(void)
{
    if (CUNIT_cCounter++ == 0)
    {
        return "Unknown";
    }
    else
    {
        return "Agent123";
    }
}


mp_void reset_cunit_counter(mp_void)
{
    CUNIT_iCounter = 0;
    CUNIT_bCounter = 0;
    CUNIT_cCounter = 0;
    CUNIT_sCounter = 0;
    CUNIT_rCounter = 0;
    CUNIT_nCounter = 0;
    CUNIT_pCounter = 0;
    CUNIT_SystemWithEchoCounter = 0;
    CUNIT_SystemWithoutEchoCounter = 0;
    CUNIT_ExecScriptCounter = 0;
    CUNIT_CheckScriptSignCounter = 0;
}

mp_void stub_return_nothing(mp_void)
{
    return ;
}

mp_void init_cunit_data(mp_void)
{
    gp_stubLog = new Stub<pOrgLog, pStubLog, mp_void>(&CLogger::Log, &stub_return_nothing);
    gp_CConfigXmlParser_0 = new Stub<pOrgGetValueString0, pStubGetValueString, mp_void>(&CConfigXmlParser::GetValueString, &stub_return_success);
    gp_CConfigXmlParser_1 = new Stub<pOrgGetValueString1, pStubGetValueString, mp_void>(&CConfigXmlParser::GetValueString, &stub_return_success);
    gp_GetJsonString = new Stub<pOrgGetJsonString, pStubIntType, mp_void>(&CJsonUtils::GetJsonString, &stub_return_ret);
    gp_GetJsonInt32 = new Stub<pOrgGetJsonInt32, pStubIntType, mp_void>(&CJsonUtils::GetJsonInt32, &stub_return_ret);
    gp_GetJsonArrayString = new Stub<pOrgGetJsonArrayString, pStubIntType, mp_void>(&CJsonUtils::GetJsonArrayString, &stub_return_ret);
    gp_GetJsonInt64 = new Stub<pOrgGetJsonInt64, pStubIntType, mp_void>(&CJsonUtils::GetJsonInt64, &stub_return_ret);
    gp_ExecSystemWithEcho = new Stub<pOrgExecSystemWithEcho, pStubExecSystemWithEcho, mp_void>(&CSystemExec::ExecSystemWithEcho, &stubSystemWithEcho);
    gp_ExecSystemWithoutEcho = new Stub<pOrgExecSystemWithoutEcho, pStubExecSystemWithoutEcho, mp_void>(&CSystemExec::ExecSystemWithoutEcho, &stubExecSystemWithoutEcho);
    gp_ExecScript = new Stub<pOrgExecScript, pStubExecScript, mp_void>(&CSystemExec::ExecScript, &stubExecScript);
    gp_CheckScriptSign = new Stub<pOrgCheckScriptSign, pStubCheckScriptSign, mp_void>(&CheckScriptSign, &stubCheckScriptSign);
    gp_GetJsonArrayJson = new Stub<pOrgGetJsonArrayJson, pStubIntType, mp_void>(&CJsonUtils::GetJsonArrayJson, &stub_return_ret);
    gp_InputUserPwd = new Stub<pOrgInputUserPwd, pStubVoidType, mp_void>(&CPassword::InputUserPwd, &stub_return_nothing);
}

mp_void destroy_cunit_data(mp_void)
{
    if (gp_stubLog) delete gp_stubLog;
    if (gp_CConfigXmlParser_0)    delete gp_CConfigXmlParser_0;
    if (gp_CConfigXmlParser_1)  delete gp_CConfigXmlParser_1;
    if (gp_GetJsonString) delete gp_GetJsonString;
    if (gp_GetJsonInt32) delete gp_GetJsonInt32;
    if (gp_GetJsonArrayString) delete gp_GetJsonArrayString;
    if (gp_GetJsonInt64) delete gp_GetJsonInt64;
    if (gp_ExecSystemWithEcho) delete gp_ExecSystemWithEcho;
    if (gp_ExecSystemWithoutEcho) delete gp_ExecSystemWithoutEcho;
    if (gp_ExecScript) delete gp_ExecScript;
    if (gp_CheckScriptSign) delete gp_CheckScriptSign;
    if (gp_GetJsonArrayJson) delete gp_GetJsonArrayJson;
    if (gp_InputUserPwd) delete gp_InputUserPwd;
}

