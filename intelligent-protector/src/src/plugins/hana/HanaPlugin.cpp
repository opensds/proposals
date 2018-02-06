/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/


#include "plugins/hana/HanaPlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "rest/MessageProcess.h"

REGISTER_PLUGIN(CHanaPlugin); //lint !e19
CHanaPlugin::CHanaPlugin()
{
    REGISTER_ACTION(REST_HANA_STARTDB, REST_URL_METHOD_PUT, &CHanaPlugin::StartDB);
    REGISTER_ACTION(REST_HANA_STOPDB, REST_URL_METHOD_PUT, &CHanaPlugin::StopDB);
    REGISTER_ACTION(REST_HANA_TESTDB, REST_URL_METHOD_PUT, &CHanaPlugin::Test);
    REGISTER_ACTION(REST_HANA_FREEZE, REST_URL_METHOD_PUT, &CHanaPlugin::FreezeDB);
    REGISTER_ACTION(REST_HANA_THAW, REST_URL_METHOD_PUT, &CHanaPlugin::ThawDB);
    REGISTER_ACTION(REST_HANA_GET_FREEZE_STATE, REST_URL_METHOD_GET, &CHanaPlugin::GetFreezeStatus);
}

CHanaPlugin::~CHanaPlugin()
{
}

/*------------------------------------------------------------ 
Description  :获取下发的Hana认证信息
Input        : req -- rest请求结构体
Output       :  stDBInfo --数据库信息，获取信息放到此结构体
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHanaPlugin::GetDBAuthParam(CRequestMsg* req, hana_db_info_t &stDBInfo)
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
Description  :HANA组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CHanaPlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CHanaPlugin, req, rsp);
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
mp_int32 CHanaPlugin::Test(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    hana_db_info_t stdbInfo;
    mp_string strinstNum;
    mp_string strDbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test hana.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_INSTNUM, strinstNum);
    stdbInfo.strInstNum = strinstNum;
    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_DBNAME, strDbName);
    stdbInfo.strDbName = strDbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strInstNum.empty() || stdbInfo.strDbName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test hana failed, instance num(%s) is empty or Db name(%s) is empty.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strInstNum, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get hana DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance num %s, database %s.", stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());

    iRet = m_hana.Test(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test hana failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test hana succ.");
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
mp_int32 CHanaPlugin::StartDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    hana_db_info_t stdbInfo;
    mp_string strinstNum;
    mp_string strDbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start hana.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_INSTNUM, strinstNum);
    stdbInfo.strInstNum = strinstNum;
    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_DBNAME, strDbName);
    stdbInfo.strDbName = strDbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strInstNum.empty() || stdbInfo.strDbName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start hana failed, instance num(%s) is empty or Db name(%s) is empty.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strInstNum, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get hana DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance num %s, database %s.", stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());

    iRet = m_hana.StartDB(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start hana failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start hana succ.");
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
mp_int32 CHanaPlugin::StopDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    hana_db_info_t stdbInfo;
    mp_string strinstNum;
    mp_string strDbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to stop hana.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_INSTNUM, strinstNum);
    stdbInfo.strInstNum = strinstNum;
    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_DBNAME, strDbName);
    stdbInfo.strDbName = strDbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strInstNum.empty() || stdbInfo.strDbName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop hana failed, instance num(%s) is empty or Db name(%s) is empty.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strInstNum, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get hana DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance num %s, database %s.", stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());


    iRet = m_hana.StopDB(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop hana failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop hana succ.");
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
mp_int32 CHanaPlugin::FreezeDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    hana_db_info_t stdbInfo;
    mp_string strInstNum;
    mp_string strDbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to freeze hana.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_INSTNUM, strInstNum);
    stdbInfo.strInstNum = strInstNum;
    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_DBNAME, strDbName);
    stdbInfo.strDbName = strDbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strInstNum.empty() || stdbInfo.strDbName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze hana failed, instance num(%s) is empty or Db name(%s) is empty.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strInstNum, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get hana DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance num %s, database %s.", stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());

    iRet = m_hana.FreezeDB(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze hana failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze hana succ.");
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
mp_int32 CHanaPlugin::ThawDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    hana_db_info_t stdbInfo;
    mp_string strInstNum;
    mp_string strDbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to thaw hana.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_INSTNUM, strInstNum);
    stdbInfo.strInstNum = strInstNum;
    GET_JSON_STRING(jReqValue, REST_PARAM_HANA_DBNAME, strDbName);
    stdbInfo.strDbName = strDbName;
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strInstNum.empty() || stdbInfo.strDbName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Thaw hana failed, instance num(%s) is empty or Db name(%s) is empty.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strInstNum, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get hana DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance num %s, database %s.", stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());


    iRet = m_hana.ThawDB(stdbInfo);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Thaw hana failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw hana succ.");
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
mp_int32 CHanaPlugin::GetFreezeStatus(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    hana_db_info_t stdbInfo;
    mp_string strInstNum;
    mp_string strDbName;
    mp_int32 iFreezeState = 0;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get hana freeze status.");

    CRequestURL& vrequrl= req->GetURL();
    map<mp_string, mp_string>& vreqal = vrequrl.GetQueryParam();
    map<mp_string, mp_string>::iterator iter = vreqal.begin();
    for (; iter != vreqal.end(); ++iter)
    {
        if (iter->first == REST_PARAM_HANA_INSTNUM)
        {
            stdbInfo.strInstNum = iter->second;
        }
        
        if (iter->first == REST_PARAM_HANA_DBNAME)
        {
            stdbInfo.strDbName = iter->second;
        }
    }
    
    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    if(stdbInfo.strInstNum.empty() || stdbInfo.strDbName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get hana freeze status failed, instance num(%s) is empty or Db name(%s) is empty.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    else
    {
        CHECK_FAIL_EX(CheckParamString(stdbInfo.strInstNum, 1, 254, strInclude, strExclude));
    }
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get hana DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instance num %s, database %s.", stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());


    iRet = m_hana.GetFreezeStatus(stdbInfo, iFreezeState);
    stdbInfo.strDBPassword.replace(0, stdbInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get hana freeze status failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jValue = rsp->GetJsonValueRef();
    //hana冻结状态和Define.h中冻结状态宏定义相同
    jValue[REST_PARAM_HANA_STATE] = iFreezeState;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get hana freeze status succ.");
    return MP_SUCCESS;
}


