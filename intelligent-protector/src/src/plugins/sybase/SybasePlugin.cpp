/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/


#include "plugins/sybase/SybasePlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "rest/MessageProcess.h"

REGISTER_PLUGIN(CSybasePlugin); //lint !e19
CSybasePlugin::CSybasePlugin()
{
    REGISTER_ACTION(REST_SYBASE_STARTDB, REST_URL_METHOD_PUT, &CSybasePlugin::StartDB);
    REGISTER_ACTION(REST_SYBASE_STOPDB, REST_URL_METHOD_PUT, &CSybasePlugin::StopDB);
    REGISTER_ACTION(REST_SYBASE_TESTDB, REST_URL_METHOD_PUT, &CSybasePlugin::Test);
    REGISTER_ACTION(REST_SYBASE_FREEZE, REST_URL_METHOD_PUT, &CSybasePlugin::FreezeDB);
    REGISTER_ACTION(REST_SYBASE_THAW, REST_URL_METHOD_PUT, &CSybasePlugin::ThawDB);
    REGISTER_ACTION(REST_SYBASE_GET_FREEZE_STATE, REST_URL_METHOD_GET, &CSybasePlugin::GetFreezeStatus);
}

CSybasePlugin::~CSybasePlugin()
{
}

/*------------------------------------------------------------ 
Description  :获取下发的Sybase认证信息
Input        : req -- rest请求结构体
Output       :  stDBInfo --数据库信息，获取信息放到此结构体
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CSybasePlugin::GetDBAuthParam(CRequestMsg* req, sybase_db_info_t &stDBInfo)
{
    const mp_char *pHeadStr = NULL;
    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBUSERNAME);
    if (pHeadStr)
    {
        stDBInfo.strDBUsername = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)stDBInfo.strDBUsername.c_str());
    }
    else
    {
        stDBInfo.strDBUsername = "";
    }

    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBPASSWORD);
    if (pHeadStr)
    {
        stDBInfo.strDBPassword = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)stDBInfo.strDBPassword.c_str());
    }
    else
    {
        stDBInfo.strDBPassword = "";
    }

    mp_bool bParamCheck = (!stDBInfo.strDBPassword.empty() && stDBInfo.strDBUsername.empty());
    if (bParamCheck)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check parameter failed, strDBPassword is null and strDBUsername is not null.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :Sybase组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybasePlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CSybasePlugin, req, rsp);
}

/*------------------------------------------------------------ 
Description  :测试数据库
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybasePlugin::Test(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    sybase_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test sybase.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_INSTNAME, strinstName);
    stdbInfo.strinstName = strinstName;
    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_DBNAME, strdbName);
    stdbInfo.strdbName = strdbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strinstName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test sybase failed, instance name is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strinstName, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sybase DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance %s, database %s.", stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());

    iRet = m_sybase.Test(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test sybase failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test sybase succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :启动数据库
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybasePlugin::StartDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    sybase_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start sybase.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_INSTNAME, strinstName);
    stdbInfo.strinstName = strinstName;
    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_DBNAME, strdbName);
    stdbInfo.strdbName = strdbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strinstName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start sybase failed, instance name is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strinstName, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sybase DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance %s, database %s.", stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());

    iRet = m_sybase.StartDB(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start sybase failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start sybase succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :停止数据库
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybasePlugin::StopDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    sybase_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to stop sybase.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_INSTNAME, strinstName);
    stdbInfo.strinstName = strinstName;
    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_DBNAME, strdbName);
    stdbInfo.strdbName = strdbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strinstName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop sybase failed, instance name is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strinstName, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sybase DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance %s, database %s.", stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());


    iRet = m_sybase.StopDB(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop sybase failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop sybase succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :测试数据库
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybasePlugin::FreezeDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    sybase_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to freeze sybase.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_INSTNAME, strinstName);
    stdbInfo.strinstName = strinstName;
    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_DBNAME, strdbName);
    stdbInfo.strdbName = strdbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strinstName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze sybase failed, instance name is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strinstName, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sybase DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance %s, database %s.", stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());

    iRet = m_sybase.FreezeDB(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze sybase failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze sybase succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :测试数据库
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybasePlugin::ThawDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    sybase_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to thaw sybase.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_INSTNAME, strinstName);
    stdbInfo.strinstName = strinstName;
    GET_JSON_STRING(jReqValue, REST_PARAM_SYBASAE_DBNAME, strdbName);
    stdbInfo.strdbName = strdbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strinstName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Thaw sybase failed, instance name is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strinstName, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sybase DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance %s, database %s.", stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());


    iRet = m_sybase.ThawDB(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Thaw sybase failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw sybase succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :测试数据库
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybasePlugin::GetFreezeStatus(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    sybase_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;
    mp_int32 iFreezeState = 0;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get sybase freeze status.");

    CRequestURL& vrequrl= req->GetURL();
    map<mp_string, mp_string>& vreqal = vrequrl.GetQueryParam();
    map<mp_string, mp_string>::iterator iter = vreqal.begin();
    for (; iter != vreqal.end(); ++iter)
    {
        if (iter->first == REST_PARAM_SYBASAE_INSTNAME)
        {
            stdbInfo.strinstName = iter->second;
        }
        
        if (iter->first == REST_PARAM_SYBASAE_DBNAME)
        {
            stdbInfo.strdbName = iter->second;
        }
    }
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strinstName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sybase freeze status failed, instance name is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strinstName, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sybase DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance %s, database %s.", stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());


    iRet = m_sybase.GetFreezeStatus(stdbInfo, iFreezeState);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sybase freeze status failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jValue = rsp->GetJsonValueRef();
    //oracle冻结状态和Define.h中冻结状态宏定义相同
    jValue[REST_PARAM_SYBASE_STATE] = iFreezeState;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get sybase freeze status succ.");
    return MP_SUCCESS;
}


