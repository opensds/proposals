/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifdef WIN32
#include "plugins/sqlserver/SqlServerPlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "common/Path.h"
#include "common/Defines.h"
#include "common/String.h"

REGISTER_PLUGIN(CSqlServerPlugin);
CSqlServerPlugin::CSqlServerPlugin()
{
    REGISTER_ACTION(REST_SQLSERVER_QUERY_DB_INFO, REST_URL_METHOD_GET, &CSqlServerPlugin::QueryDBBasicInfo);
    REGISTER_ACTION(REST_SQLSERVER_QUERY_LUN_INFO, REST_URL_METHOD_GET, &CSqlServerPlugin::QueryDBLUNInfo);
    REGISTER_ACTION(REST_SQLSERVER_START, REST_URL_METHOD_PUT, &CSqlServerPlugin::StartDB);
    REGISTER_ACTION(REST_SQLSERVER_STOP, REST_URL_METHOD_PUT, &CSqlServerPlugin::StopDB);
    REGISTER_ACTION(REST_SQLSERVER_TEST, REST_URL_METHOD_PUT, &CSqlServerPlugin::TestDB);
    REGISTER_ACTION(REST_SQLSERVER_FREEZE_DB, REST_URL_METHOD_PUT, &CSqlServerPlugin::Freeze);
    REGISTER_ACTION(REST_SQLSERVER_UNFREEZE_DB, REST_URL_METHOD_PUT, &CSqlServerPlugin::UnFreeze);
    REGISTER_ACTION(REST_SQLSERVER_GET_FREEZE_STAT, REST_URL_METHOD_GET, &CSqlServerPlugin::QueryFreezeState);
}

CSqlServerPlugin::~CSqlServerPlugin()
{
}
/*------------------------------------------------------------ 
Description  :SqlServer组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 输入信息
Output       : rsp -- 返回信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CSqlServerPlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CSqlServerPlugin, req, rsp);
}

/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServerPlugin::QueryDBBasicInfo
Description   : 查询SQLServer数据库信息;
Input         : CRequestMsg* req,  rest 请求消息;
                CResponseMsg* rsp, rest 应答消息;
Output        : CResponseMsg* rsp;
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServerPlugin::QueryDBBasicInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<sqlserver_info_t> vecSqlserverDBInfo;
    vector<sqlserver_info_t>::iterator iter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query sqlserver database info.");

    iRet = m_sqlserver.GetInfo(vecSqlserverDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sqlserver database info failed, iRet %d", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    for (iter = vecSqlserverDBInfo.begin(); iter != vecSqlserverDBInfo.end(); ++iter)
    {
        Json::Value jValue;
        jValue[REST_PARAM_SQLSERVER_INSTNAME]  = iter->strInstName;
        jValue[REST_PARAM_SQLSERVER_DBNAME]    = iter->strDBName;
        jValue[REST_PARAM_SQLSERVER_VERSION]   = iter->strVersion;
        jValue[REST_PARAM_SQLSERVER_STATE]     = iter->strState;
        jValue[REST_PARAM_SQLSERVER_ISCLUSTER] = iter->strIsCluster;
        val.append(jValue);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query sqlserver database info succ.");
    return iRet;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServerPlugin::QueryDBLUNInfo
Description   : 查询SQLServer 挂在的数据库LUN信息;
Input         : CRequestMsg* req,  rest 请求消息;
                CResponseMsg* rsp, rest 应答消息;
Output        : CResponseMsg* rsp;
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServerPlugin::QueryDBLUNInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    sqlserver_info_t stdbinfo;
    vector<sqlserver_lun_info_t> vecLunInfos;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query sqlserver lun info.");

    CRequestURL& vrequrl= req->GetURL();
    map<mp_string, mp_string>& vreqal= vrequrl.GetQueryParam();
    map<mp_string, mp_string>::iterator iter=vreqal.begin();
    for (; iter != vreqal.end(); ++iter)
    {
        if (REST_PARAM_SQLSERVER_INSTNAME == iter->first)
        {
            stdbinfo.strInstName = iter->second;
            continue;
        }

        if (REST_PARAM_SQLSERVER_DBNAME == iter->first)
        {
            stdbinfo.strDBName = iter->second;
            continue;
        }

        if (REST_PARAM_SQLSERVER_ISCLUSTER == iter->first)
        {
            stdbinfo.strIsCluster = iter->second;
            continue;
        }
    }

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    CHECK_FAIL_EX(CheckParamString(stdbinfo.strInstName, 1, 123, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(stdbinfo.strDBName, 1, 123, strInclude, strExclude));
    strInclude=mp_string("01");
    CHECK_FAIL_EX(CheckParamString(stdbinfo.strIsCluster, 1, 1, strInclude, strExclude));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
        "sqlsercver query lun param: instname:%s, dbname:%s, iscluster:%s.",
        stdbinfo.strInstName.c_str(), stdbinfo.strDBName.c_str(),
        stdbinfo.strIsCluster.c_str());

    stdbinfo.strUser = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stdbinfo.strPasswd = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));

    iRet = m_sqlserver.GetLunInfo(stdbinfo, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sqlserver lun info failed, iRet %d", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    for (vector<sqlserver_lun_info_t>::iterator iter = vecLunInfos.begin();
        iter != vecLunInfos.end();
        ++ iter)
    {
        Json::Value jValue;
        jValue[REST_PARAM_SQLSERVER_LUNID]         = iter->strLunID;
        jValue[REST_PARAM_SQLSERVER_ARRAYSN]       = iter->strArraySN;
        jValue[REST_PARAM_SQLSERVER_WWN]           = iter->strWWN;
        jValue[REST_PARAM_SQLSERVER_DEVICENAME]    = iter->strDeviceName;
        jValue[REST_PARAM_SQLSERVER_VOLNAME]       = iter->strVOLName;
        jValue[REST_PARAM_SQLSERVER_DEVICETYPE]    = iter->iDeviceType;
        jValue[REST_PARAM_SQLSERVER_LBA]           = iter->strLBA;

        val.append(jValue);
    }

    stdbinfo.strPasswd.replace(0, stdbinfo.strPasswd.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query sqlserver lun info succ.");
    return iRet;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServerPlugin::StartDB
Description   : 启动sqlserver数据库;
Input         : CRequestMsg* req,  rest 请求消息;
                CResponseMsg* rsp, rest 应答消息;
Output        : CResponseMsg* rsp;
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServerPlugin::StartDB(CRequestMsg* req, CResponseMsg* rsp)
{
    sqlserver_info_t stDBInfo;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start sqlserver database.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_DBNAME, stDBInfo.strDBName);
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_VERSION, stDBInfo.strVersion);
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_ISCLUSTER, stDBInfo.strIsCluster);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strInstName, 1, 123, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strDBName, 1, 123, strInclude, strExclude));
    strInclude=mp_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.()- ");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strVersion, 1, 255, strInclude, strExclude));
    strInclude=mp_string("01");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strIsCluster, 1, 1, strInclude, strExclude));
    
    stDBInfo.strUser = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stDBInfo.strPasswd = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));
    stDBInfo.strCheckType = SQL_SERVER_OPTCODE_START;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Received sqlparam. instname:%s, dbname:%s, user:%s, version:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), stDBInfo.strUser.c_str(), stDBInfo.strVersion.c_str());

    iRet = m_sqlserver.Start(stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start sqlserver database[%s] failed, iRet %d.",
            stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    stDBInfo.strPasswd.replace(0, stDBInfo.strPasswd.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start sqlserver database[%s] succ.", stDBInfo.strDBName.c_str());

    return iRet;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServerPlugin::StartDB
Description   : 启动sqlserver数据库;
Input         : CRequestMsg* req,  rest 请求消息;
                CResponseMsg* rsp, rest 应答消息;
Output        : CResponseMsg* rsp;
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServerPlugin::StopDB(CRequestMsg* req, CResponseMsg* rsp)
{
    sqlserver_info_t stDBInfo;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin stop sqlserver database.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_DBNAME, stDBInfo.strDBName);
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_VERSION, stDBInfo.strVersion);
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_ISCLUSTER, stDBInfo.strIsCluster);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strInstName, 1, 123, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strDBName, 1, 123, strInclude, strExclude));
    strInclude=mp_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.()- ");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strVersion, 1, 255, strInclude, strExclude));
    strInclude=mp_string("01");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strIsCluster, 1, 1, strInclude, strExclude));
    
    stDBInfo.strUser = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stDBInfo.strPasswd = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));
    stDBInfo.strCheckType = SQL_SERVER_OPTCODE_STOP;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Received sqlparam. instname:%s, dbname:%s, user:%s, version:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), stDBInfo.strUser.c_str(), stDBInfo.strVersion.c_str());

    iRet = m_sqlserver.Stop(stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop sqlserver database[%s] failed, iRet %d.",
            stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    stDBInfo.strPasswd.replace(0, stDBInfo.strPasswd.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop sqlserver database[%s] succ.", stDBInfo.strDBName.c_str());

    return iRet;
}

/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServerPlugin::StartDB
Description   : 测试sqlserver数据库;
Input         : CRequestMsg* req,  rest 请求消息;
                CResponseMsg* rsp, rest 应答消息;
Output        : CResponseMsg* rsp;
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServerPlugin::TestDB(CRequestMsg* req, CResponseMsg* rsp)
{
    sqlserver_info_t stDBInfo;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin test sqlserver database.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_DBNAME, stDBInfo.strDBName);
    GET_JSON_STRING(jReqValue, REST_PARAM_SQLSERVER_ISCLUSTER, stDBInfo.strIsCluster);

    //参数校验
    mp_string strExclude("");
    mp_string strInclude("");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strInstName, 1, 123, strInclude, strExclude));
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strDBName, 1, 123, strInclude, strExclude));
    strInclude=mp_string("01");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strIsCluster, 1, 1, strInclude, strExclude));

    stDBInfo.strUser = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME));
    stDBInfo.strPasswd = mp_string(req->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD));
    stDBInfo.strCheckType = SQL_SERVER_OPTCODE_TESTCONN;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
        "Received sqlparam. instname:%s, dbname:%s, user:%s, version:%s, iscluster:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(),
        stDBInfo.strUser.c_str(),     stDBInfo.strVersion.c_str(),  
        stDBInfo.strIsCluster.c_str());

    iRet = m_sqlserver.Test(stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test sqlserver database[%s] failed, iRet %d.",
            stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    stDBInfo.strPasswd.replace(0, stDBInfo.strPasswd.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test sqlserver database[%s] succ.", stDBInfo.strDBName.c_str());

    return iRet;
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
mp_int32 CSqlServerPlugin::Freeze(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    vector<sqlserver_freeze_info_t> vecFreezInfos;
    sqlserver_freeze_info_t freezInfo;
    mp_string strDriveLetter;
    mp_uint32 uiDriveLetterCount = 0;
    mp_uint32 uiDBCount = 0;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze sqlserver.");
    CHECK_JSON_ARRAY(jReqValue);
    uiDBCount = jReqValue.size();
    mp_string strExclude("");
    mp_string strInclude("");
    for (mp_uint32 i = 0; i < uiDBCount; i++)
    {
        const Json::Value& jvTmp = jReqValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_SQLSERVER_DBNAME, freezInfo.strDBName);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get db name %s.", freezInfo.strDBName.c_str());
        CHECK_JSON_VALUE(jvTmp, REST_PARAM_SQLSERVER_DISKNAMES);
        CHECK_JSON_ARRAY(jvTmp[REST_PARAM_SQLSERVER_DISKNAMES]);

        //参数校验
        strInclude = mp_string("");
        CHECK_FAIL_EX(CheckParamString(freezInfo.strDBName, 1, 123, strInclude, strExclude));
        
        uiDriveLetterCount = jvTmp[REST_PARAM_SQLSERVER_DISKNAMES].size();
        strInclude = mp_string("BCDEFGHIJKLMNOPQRSTUVWXYZ");
        for (mp_uint32 j = 0; j < uiDriveLetterCount; j++)
        {
            strDriveLetter = jvTmp[REST_PARAM_SQLSERVER_DISKNAMES][j].asString();
            CHECK_FAIL_EX(CheckParamString(strDriveLetter, 1, 1, strInclude, strExclude));
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive letter %s.", strDriveLetter.c_str());
            freezInfo.vecDriveLetters.push_back(strDriveLetter);
        }
        vecFreezInfos.push_back(freezInfo);
    }

    iRet = m_sqlserver.Freeze(vecFreezInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze sqlserver failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze sqlserver succ.");
    return iRet;
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
mp_int32 CSqlServerPlugin::UnFreeze(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    vector<sqlserver_unfreeze_info_t> vecUnfreezInfos;
    sqlserver_unfreeze_info_t unfreezInfo;
    mp_string strDriveLetter;
    mp_uint32 uiDriveLetterCount = 0;
    mp_uint32 uiDBCount = 0;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin unfreeze sqlserver.");
    CHECK_JSON_ARRAY(jReqValue);
    uiDBCount = jReqValue.size();
    mp_string strExclude("");
    mp_string strInclude("");
    for (mp_uint32 i = 0; i < uiDBCount; i++)
    {
        const Json::Value& jvTmp = jReqValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_SQLSERVER_DBNAME, unfreezInfo.strDBName);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get db name %s.", unfreezInfo.strDBName.c_str());
        CHECK_JSON_VALUE(jvTmp, REST_PARAM_SQLSERVER_DISKNAMES);
        CHECK_JSON_ARRAY(jvTmp[REST_PARAM_SQLSERVER_DISKNAMES]);

        //参数校验
        strInclude = mp_string("");
        CHECK_FAIL_EX(CheckParamString(unfreezInfo.strDBName, 1, 123, strInclude, strExclude));
        
        uiDriveLetterCount = jvTmp[REST_PARAM_SQLSERVER_DISKNAMES].size();
        strInclude = mp_string("BCDEFGHIJKLMNOPQRSTUVWXYZ");
        for (mp_uint32 j = 0; j < uiDriveLetterCount; j++)
        {
            strDriveLetter = jvTmp[REST_PARAM_SQLSERVER_DISKNAMES][j].asString();
            CHECK_FAIL_EX(CheckParamString(strDriveLetter, 1, 1, strInclude, strExclude));
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive letter %s.", strDriveLetter.c_str());
            unfreezInfo.vecDriveLetters.push_back(strDriveLetter);
        }
        vecUnfreezInfos.push_back(unfreezInfo);
    }

    iRet = m_sqlserver.UnFreeze(vecUnfreezInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unfreeze sqlserver failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unfreeze sqlserver succ.");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :查询数据库冻结状态
Input        : req -- 请求信息
Output       : rsp -- 响应信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSqlServerPlugin::QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iStat;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query sqlserver freeze stat.");
    iStat = m_sqlserver.GetFreezeStat();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Sqlserver freeze stat is %d.", iStat);

    Json::Value& jValue= rsp->GetJsonValueRef();
    jValue[REST_PARAM_SQLSERVER_FREEZE_STAT] = iStat;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query sqlserver freeze stat:%d.", iStat);
    return MP_SUCCESS;
}

#endif

