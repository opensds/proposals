/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/


#include "plugins/cache/CachePlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "rest/MessageProcess.h"

REGISTER_PLUGIN(CCachePlugin); //lint !e19
CCachePlugin::CCachePlugin()
{
    REGISTER_ACTION(REST_CACHE_QUERY_DB_INFO, REST_URL_METHOD_GET, &CCachePlugin::QueryInfo);
    REGISTER_ACTION(REST_CACHE_QUERY_LUN_INFO, REST_URL_METHOD_GET, &CCachePlugin::QueryLunInfo);
    REGISTER_ACTION(REST_CACHE_TEST, REST_URL_METHOD_PUT, &CCachePlugin::Test);
}

CCachePlugin::~CCachePlugin()
{
}
/*------------------------------------------------------------ 
Description  :CACHE组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCachePlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CCachePlugin, req, rsp);
}
/*------------------------------------------------------------ 
Description  :查询cache信息
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCachePlugin::QueryInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<cache_inst_info_t> vecdbInstInfo;
    vector<cache_inst_info_t>::iterator iter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query cache info.");

    iRet = m_cache.GetInfo(vecdbInstInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query cache info failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    for (iter = vecdbInstInfo.begin(); iter != vecdbInstInfo.end(); ++ iter)
    {
        Json::Value jValue;
        jValue[REST_PARAM_CACHE_INSTNAME] = iter->strinstName;
        jValue[REST_PARAM_CACHE_VERSION] = iter->strversion;
        jValue[REST_PARAM_CACHE_STATE] = iter->istate;
        val.append(jValue);      
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query cache info succ.");
    
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :查询lun信息
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCachePlugin::QueryLunInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    cache_db_info_t stdbInfo;
    vector < cache_lun_info_t > vecLunInfos;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query cache lun info.");
    CRequestURL& vrequrl= req->GetURL();

    stdbInfo.strinstName = vrequrl.GetSpecialQueryParam(REST_PARAM_CACHE_INSTNAME);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s.", stdbInfo.strinstName.c_str());

    iRet = m_cache.GetLunInfo(stdbInfo, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get cache lun info failed, iRet %d.", iRet);
        return iRet;
    }

    vector < cache_lun_info_t >::iterator iterdbLun;
    Json::Value& val= rsp->GetJsonValueRef(); 
    for (iterdbLun = vecLunInfos.begin(); iterdbLun != vecLunInfos.end(); ++ iterdbLun)
    {
        Json::Value jValue;
        jValue[REST_PARAM_CACHE_LUNID] = iterdbLun->strlunId;
        jValue[REST_PARAM_CACHE_ARRAYSN] = iterdbLun->strarraySn;
        jValue[REST_PARAM_CACHE_WWN] = iterdbLun->strwwn;
        jValue[REST_PARAM_CACHE_DEVICENAME] = iterdbLun->strvolName;;
        jValue[REST_PARAM_CACHE_VGNAME] = iterdbLun->strvgName;
        jValue[REST_PARAM_CACHE_DEVICEPATH] = iterdbLun->strdeviceName;
        val.append(jValue);      
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query cache lun info succ.");
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
mp_int32 CCachePlugin::Test(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    cache_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test cache.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_CACHE_INSTNAME, strinstName);
    stdbInfo.strinstName = strinstName;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s.",
        stdbInfo.strinstName.c_str());

    iRet = m_cache.Test(stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test cache failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test cache succ.");
    return MP_SUCCESS;
}

