/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/db2/Db2Plugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "rest/MessageProcess.h"

REGISTER_PLUGIN(CDb2Plugin); //lint !e19
CDb2Plugin::CDb2Plugin()
{
    REGISTER_ACTION(REST_DB2_QUERY_DB_INFO, REST_URL_METHOD_GET, &CDb2Plugin::QueryInfo);
    REGISTER_ACTION(REST_DB2_QUERY_LUN_INFO, REST_URL_METHOD_GET, &CDb2Plugin::QueryLunInfo);
    REGISTER_ACTION(REST_DB2_SART, REST_URL_METHOD_PUT, &CDb2Plugin::Start);
    REGISTER_ACTION(REST_DB2_STOP, REST_URL_METHOD_PUT, &CDb2Plugin::Stop);
    REGISTER_ACTION(REST_DB2_TEST, REST_URL_METHOD_PUT, &CDb2Plugin::Test);
    REGISTER_ACTION(REST_DB2_FREEZE, REST_URL_METHOD_PUT, &CDb2Plugin::Freeze);
    REGISTER_ACTION(REST_DB2_UNFREEZE, REST_URL_METHOD_PUT, &CDb2Plugin::UnFreeze);
    REGISTER_ACTION(REST_DB2_FREEZESTATE, REST_URL_METHOD_GET, &CDb2Plugin::QueryFreezeState);
}

CDb2Plugin::~CDb2Plugin()
{
}
/*------------------------------------------------------------ 
Description  :DB2组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDb2Plugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CDb2Plugin, req, rsp);
}
/*------------------------------------------------------------ 
Description  :查询db2信息
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDb2Plugin::QueryInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<db2_inst_info_t> vecdbInstInfo;
    vector<db2_inst_info_t>::iterator iter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query db2 info.");

    iRet = m_db2.GetInfo(vecdbInstInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query db2 info failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    for (iter = vecdbInstInfo.begin(); iter != vecdbInstInfo.end(); ++ iter)
    {
        Json::Value jValue;
        jValue[REST_PARAM_DB2_INSTNAME] = iter->strinstName;
        jValue[REST_PARAM_DB2_DBNAME] = iter->strdbName;
        jValue[REST_PARAM_DB2_VERSION] = iter->strversion;
        jValue[REST_PARAM_DB2_STATE] = iter->istate;
        val.append(jValue);      
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query db2 info succ.");
    
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
mp_int32 CDb2Plugin::QueryLunInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    db2_db_info_t stdbInfo;
    vector < db2_lun_info_t > vecLunInfos;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query db2 lun info.");
    CRequestURL& vrequrl= req->GetURL();

    stdbInfo.strinstName = vrequrl.GetSpecialQueryParam(REST_PARAM_DB2_INSTNAME);
    stdbInfo.strdbName= vrequrl.GetSpecialQueryParam(REST_PARAM_DB2_DBNAME);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    mp_int32 lenBeg = 1;
    mp_int32 lenEnd = 8;
    CHECK_FAIL_EX(CheckParamString(stdbInfo.strinstName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(stdbInfo.strdbName, lenBeg, lenEnd, strInclude, strExclude));
    
    stdbInfo.strdbUsername = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stdbInfo.strdbPassword = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), stdbInfo.strdbUsername.c_str());

    iRet = m_db2.GetLunInfo(stdbInfo, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get db2 lun info failed, iRet %d.", iRet);
        return iRet;
    }

    vector < db2_lun_info_t >::iterator iterdbLun;
    Json::Value& val= rsp->GetJsonValueRef(); 
    for (iterdbLun = vecLunInfos.begin(); iterdbLun != vecLunInfos.end(); ++ iterdbLun)
    {
        Json::Value jValue;
        jValue[REST_PARAM_DB2_LUNID] = iterdbLun->strlunId;
        jValue[REST_PARAM_DB2_UUID] = iterdbLun->struuid;
        jValue[REST_PARAM_DB2_ARRAYSN] = iterdbLun->strarraySn;
        jValue[REST_PARAM_DB2_WWN] = iterdbLun->strwwn;
        jValue[REST_PARAM_DB2_DEVICENAME] = iterdbLun->strdeviceName;
        jValue[REST_PARAM_DB2_VOLTYPE] = iterdbLun->ivolType;
        jValue[REST_PARAM_DB2_VGNAME] = iterdbLun->strvgName;
        jValue[REST_PARAM_DB2_VOLNAME] = iterdbLun->strvolName;
        jValue[REST_PARAM_DB2_STORAGETYPE] = iterdbLun->istorageType;
        jValue[REST_PARAM_DB2_PVNAME] = iterdbLun->strpvName;
        val.append(jValue);      
    }

    stdbInfo.strdbPassword.replace(0, stdbInfo.strdbPassword.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query db2 lun info succ.");
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
mp_int32 CDb2Plugin::Start(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strinstName;
    mp_string strdbName;
    db2_db_info_t stdbInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start db2.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_INSTNAME, strinstName);
    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_DBNAME, strdbName);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    mp_int32 lenBeg = 1;
    mp_int32 lenEnd = 8;
    CHECK_FAIL_EX(CheckParamString(strinstName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(strdbName, lenBeg, lenEnd, strInclude, strExclude));
    
    stdbInfo.strinstName = strinstName;
    stdbInfo.strdbName = strdbName;
    
    stdbInfo.strdbUsername = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stdbInfo.strdbPassword = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s",
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), stdbInfo.strdbUsername.c_str());

    iRet = m_db2.Start(stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start db2 failed, iRet %d.", iRet);
        return iRet;
    }

    stdbInfo.strdbPassword.replace(0, stdbInfo.strdbPassword.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start db2 succ.");
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
mp_int32 CDb2Plugin::Stop(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    db2_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to stop db2.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_INSTNAME, strinstName);
    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_DBNAME, strdbName);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    mp_int32 lenBeg = 1;
    mp_int32 lenEnd = 8;
    CHECK_FAIL_EX(CheckParamString(strinstName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(strdbName, lenBeg, lenEnd, strInclude, strExclude));
    
    stdbInfo.strinstName = strinstName;
    stdbInfo.strdbName = strdbName;
    
    stdbInfo.strdbUsername = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stdbInfo.strdbPassword = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), stdbInfo.strdbUsername.c_str());

    iRet = m_db2.Stop(stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop db2 failed, iRet %d.", iRet);
        return iRet;
    }

    stdbInfo.strdbPassword.replace(0, stdbInfo.strdbPassword.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop db2 succ.");
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
mp_int32 CDb2Plugin::Test(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    db2_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test db2.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_INSTNAME, strinstName);
    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_DBNAME, strdbName);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    mp_int32 lenBeg = 1;
    mp_int32 lenEnd = 8;
    CHECK_FAIL_EX(CheckParamString(strinstName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(strdbName, lenBeg, lenEnd, strInclude, strExclude));
    
    stdbInfo.strinstName = strinstName;
    stdbInfo.strdbName = strdbName;

    stdbInfo.strdbUsername = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stdbInfo.strdbPassword = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), stdbInfo.strdbUsername.c_str());

    iRet = m_db2.Test(stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test db2 failed, iRet %d.", iRet);
        return iRet;
    }

    stdbInfo.strdbPassword.replace(0, stdbInfo.strdbPassword.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test db2 succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :冻结数据库
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDb2Plugin::Freeze(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    db2_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to freeze db2.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_INSTNAME, strinstName);
    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_DBNAME, strdbName);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    mp_int32 lenBeg = 1;
    mp_int32 lenEnd = 8;
    CHECK_FAIL_EX(CheckParamString(strinstName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(strdbName, lenBeg, lenEnd, strInclude, strExclude));
    
    stdbInfo.strinstName = strinstName;
    stdbInfo.strdbName = strdbName;
    
    stdbInfo.strdbUsername = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stdbInfo.strdbPassword = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), stdbInfo.strdbUsername.c_str());

    iRet = m_db2.Freeze(stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze db2 failed, iRet %d.", iRet);
        return iRet;
    }

    stdbInfo.strdbPassword.replace(0, stdbInfo.strdbPassword.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze db2 succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :解冻数据库
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDb2Plugin::UnFreeze(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    db2_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to thaw db2.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_INSTNAME, strinstName);
    GET_JSON_STRING(jReqValue, REST_PARAM_DB2_DBNAME, strdbName);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    mp_int32 lenBeg = 1;
    mp_int32 lenEnd = 8;
    CHECK_FAIL_EX(CheckParamString(strinstName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(strdbName, lenBeg, lenEnd, strInclude, strExclude));
    
    stdbInfo.strinstName = strinstName;
    stdbInfo.strdbName = strdbName;
    
    stdbInfo.strdbUsername = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stdbInfo.strdbPassword = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), stdbInfo.strdbUsername.c_str());

    iRet = m_db2.UnFreeze(stdbInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Thaw db2 failed, iRet %d.", iRet);
        return iRet;
    }

    stdbInfo.strdbPassword.replace(0, stdbInfo.strdbPassword.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw db2 succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :查询冻结状态
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDb2Plugin::QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    db2_db_info_t stdbInfo;
    mp_string strinstName;
    mp_string strdbName;
    mp_int32 iFreezeState;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query db2 freeze state.");
    CRequestURL& vrequrl= req->GetURL();

    stdbInfo.strinstName = vrequrl.GetSpecialQueryParam(REST_PARAM_DB2_INSTNAME);
    stdbInfo.strdbName= vrequrl.GetSpecialQueryParam(REST_PARAM_DB2_DBNAME);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    mp_int32 lenBeg = 1;
    mp_int32 lenEnd = 8;
    CHECK_FAIL_EX(CheckParamString(stdbInfo.strinstName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(stdbInfo.strdbName, lenBeg, lenEnd, strInclude, strExclude));
    
    stdbInfo.strdbUsername = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stdbInfo.strdbPassword = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s",
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), stdbInfo.strdbUsername.c_str());

    iRet = m_db2.QueryFreezeState(stdbInfo, iFreezeState);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query db2 freeze state failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jValue= rsp->GetJsonValueRef();
    //DB2冻结状态和Define.h中冻结状态宏定义相同
    jValue[REST_PARAM_DB2_STATE] = iFreezeState;

    stdbInfo.strdbPassword.replace(0, stdbInfo.strdbPassword.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query db2 freeze state succ.");
    return MP_SUCCESS;
}

