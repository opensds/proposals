/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/app/AppPlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "common/Path.h"
#include "common/Defines.h"

REGISTER_PLUGIN(CAppPlugin); //lint !e19
CAppPlugin::CAppPlugin()
{
    REGISTER_ACTION(REST_APP_QUERY_DB_INFO, REST_URL_METHOD_GET, &CAppPlugin::QueryInfo);
    REGISTER_ACTION(REST_APP_FREEZE, REST_URL_METHOD_PUT, &CAppPlugin::Freeze);
    REGISTER_ACTION(REST_APP_UNFREEZE, REST_URL_METHOD_PUT, &CAppPlugin::UnFreeze);
    REGISTER_ACTION(REST_APP_ENDBACKUP, REST_URL_METHOD_PUT, &CAppPlugin::EndBackup);
    REGISTER_ACTION(REST_APP_TRUNCATE_LOG, REST_URL_METHOD_PUT, &CAppPlugin::TruncateLog);
    REGISTER_ACTION(REST_APP_QUERY_DB_FREEZESTATE, REST_URL_METHOD_GET, &CAppPlugin::QueryFreezeState);
    REGISTER_ACTION(REST_APP_UNFREEZEEX, REST_URL_METHOD_PUT, &CAppPlugin::UnFreezeEx);
}

CAppPlugin::~CAppPlugin()
{
}

mp_int32 CAppPlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CAppPlugin, req, rsp);
}

mp_int32 CAppPlugin::AddAppInfosToResult(Json::Value& jValResult, vector<app_info_t>& vecAppInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 i = 0;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Add app info list to result succ");
    for (i = 0; i < vecAppInfos.size(); i++)
    {
        iRet = AddAppInfoToResult(jValResult, vecAppInfos[i]);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get app info to list failed, iRet %d.", iRet);
            return iRet;
        }
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Add app info list to result succ");

    return iRet;
}

mp_int32 CAppPlugin::AddAppInfoToResult(Json::Value& valResult, app_info_t& appInfo)
{
    Json::Value jValue;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin add app info to result, app type %d, version %s, inst name %s, "
        "db name %s, storage group %s.", appInfo.enAppType, appInfo.strVersion.c_str(), appInfo.strInstName.c_str(), 
        appInfo.strDBName.c_str(), appInfo.strStorageGroup.c_str());
    jValue[REST_PARAM_APP_VERSION] = appInfo.strVersion;
    jValue[REST_PARAM_APP_DBNAME] = appInfo.strDBName;
    if (APP_TYPE_ORACLE == appInfo.enAppType)
    {
        jValue[REST_PARAM_APP_INSTNAME] = appInfo.strInstName;
        valResult[REST_PARAM_APP_ORALCE].append(jValue);
    }
    else if (APP_TYPE_SQLSERVER == appInfo.enAppType)
    {
        jValue[REST_PARAM_APP_INSTNAME] = appInfo.strInstName;
        valResult[REST_PARAM_APP_SQLSERVER].append(jValue);
    }
    else if (APP_TYPE_EXCHANGE == appInfo.enAppType)
    {
        jValue[REST_PARAM_APP_STORAGE_GROUP] = appInfo.strStorageGroup;
        valResult[REST_PARAM_APP_EXCHANGE].append(jValue);
    }
    else if (APP_TYPE_DB2 == appInfo.enAppType)
    {
        jValue[REST_PARAM_APP_INSTNAME] = appInfo.strInstName;
        valResult[REST_PARAM_APP_DB2].append(jValue);
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid app type, app type %d.", appInfo.enAppType);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Add app info to result succ");
    return MP_SUCCESS;
}

mp_int32 CAppPlugin::GetDBAuthInfo(CRequestMsg* req, app_auth_info_t& stAppAuthInfo)
{
    const mp_char *pHeadStr = NULL;
    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBUSERNAME);
    if (pHeadStr)
    {
        stAppAuthInfo.strUserName = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char*)stAppAuthInfo.strUserName.c_str());
    }

    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBPASSWORD);
    if (pHeadStr)
    {
        stAppAuthInfo.strPasswd = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char*)stAppAuthInfo.strPasswd.c_str());
    }
    
    return MP_SUCCESS;
}

mp_int32 CAppPlugin::QueryInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<app_info_t> vecAppInfos;
    vector<app_info_t>::iterator iter;
    Json::Value& valResult = rsp->GetJsonValueRef();
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query app info.");
    iRet = m_app.QueryInfo(vecAppInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sqlserver database info failed, iRet %d.", iRet);
        return iRet;
    }

    if (0 == vecAppInfos.size())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Can't find any app info on this computer.");
        return MP_SUCCESS;
    }

    AddAppInfosToResult(valResult, vecAppInfos);
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End Query app info.");
    return MP_SUCCESS;
}

mp_int32 CAppPlugin::Freeze(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    //CodeDex误报，UNINIT
    mp_time tFreezeTime;
    app_auth_info_t appAuthInfo;
    vector<app_failed_info_t> vecAppFailedList;
    vector<app_failed_info_t>::iterator iter;
    Json::Value& jRspValue= rsp->GetJsonValueRef();
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze app.");
    (mp_void)GetDBAuthInfo(req, appAuthInfo);
    //CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    iRet = m_app.Freeze(appAuthInfo, tFreezeTime, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        iter = vecAppFailedList.begin();
        Json::Value jValue;
        jValue[REST_PARAM_APP_ERROR_CODE] = iter->iErrorCode;
        jValue[REST_PARAM_APP_DBNAME] = iter->strDbName;
        jRspValue.append(jValue);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze app failed, iRet %d.", iRet);
        return iRet;
    }
    jRspValue[REST_PARAM_APP_TIME] = (mp_int64)tFreezeTime;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze app succ.");
    return MP_SUCCESS;
}

mp_int32 CAppPlugin::UnFreeze(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    app_auth_info_t appAuthInfo;
    vector<app_failed_info_t> vecAppFailedList;
    vector<app_failed_info_t>::iterator iter;
    Json::Value& jRspValue= rsp->GetJsonValueRef();
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin unfreeze app.");
    (mp_void)GetDBAuthInfo(req, appAuthInfo);
    //CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    iRet = m_app.UnFreeze(appAuthInfo, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        iter = vecAppFailedList.begin();
        Json::Value jValue;
        jValue[REST_PARAM_APP_ERROR_CODE] = iter->iErrorCode;
        jValue[REST_PARAM_APP_DBNAME] = iter->strDbName;
        jRspValue.append(jValue);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze app failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unfreeze app succ.");
    return MP_SUCCESS;
}

mp_int32 CAppPlugin::EndBackup(CRequestMsg* req, CResponseMsg* rsp)
{    
    mp_int32 iRet = MP_SUCCESS;
    app_auth_info_t appAuthInfo;
    vector<app_failed_info_t> vecAppFailedList;
    vector<app_failed_info_t>::iterator iter;
    Json::Value& jRspValue= rsp->GetJsonValueRef();
    mp_int32 iBackupSucc = 0;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin end backup.");
    (mp_void)GetDBAuthInfo(req, appAuthInfo);
    
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_INT32(jReqValue, REST_PARAM_APP_BACKUP_SUCC, iBackupSucc);

    //参数校验
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iBackupSucc, 0, 1, vecExclude));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get backupSucc param %d.", iBackupSucc);
    //CodeDex误报，Dead Code
    iRet = m_app.EndBackup(appAuthInfo, iBackupSucc, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        iter = vecAppFailedList.begin();
        Json::Value jValue;
        jValue[REST_PARAM_APP_ERROR_CODE] = iter->iErrorCode;
        jValue[REST_PARAM_APP_DBNAME] = iter->strDbName;
        jRspValue.append(jValue);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze app failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End backup succ.");
    return MP_SUCCESS;
}

mp_int32 CAppPlugin::TruncateLog(CRequestMsg* req, CResponseMsg* rsp)
{    
    mp_int32 iRet = MP_SUCCESS;
    mp_time tTruncateTime;
    app_auth_info_t appAuthInfo;    
    vector<app_failed_info_t> vecAppFailedList;
    vector<app_failed_info_t>::iterator iter;
    Json::Value& jRspValue= rsp->GetJsonValueRef();
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin truncage log.");    
    (mp_void)GetDBAuthInfo(req, appAuthInfo);

    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    mp_int64 i64Tmp = 0;
    GET_JSON_INT64(jReqValue, REST_PARAM_APP_TIME, i64Tmp);

    //参数校验
    vector<mp_int64> vecExclude;
    mp_int64 begValue, endValue;
    begValue = 1;
#ifdef HP_UX_IA
    endValue = LONG_LONG_MAX - 1;
#else
    endValue = LLONG_MAX - 1;
#endif
    CHECK_FAIL_EX(CheckParamInteger64(i64Tmp, begValue, endValue, vecExclude));

    tTruncateTime = (mp_time)i64Tmp;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get truncate time param %lld.", tTruncateTime);
    //CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    iRet = m_app.TruncateLog(appAuthInfo, tTruncateTime, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        iter = vecAppFailedList.begin();
        Json::Value jValue;
        jValue[REST_PARAM_APP_ERROR_CODE] = iter->iErrorCode;
        jValue[REST_PARAM_APP_DBNAME] = iter->strDbName;
        jRspValue.append(jValue);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Truncate log failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Truncate log succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: QueryFreezeState
Description  : 冻结保护机制查询当前主机上应用的冻结状态，查询失败返回未知，全部都已解冻返回1，存在一个未解冻应用返回0
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CAppPlugin::QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp)
{
    app_auth_info_t appAuthInfo;
    mp_int32  iRet = MP_SUCCESS;
    (mp_void)GetDBAuthInfo(req, appAuthInfo);

    mp_int32 iFreezeState = DB_UNFREEZE;
    iRet = m_app.QueryFreezeState(appAuthInfo, iFreezeState);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query freeze state failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jRspValue= rsp->GetJsonValueRef();
    jRspValue[REST_PARAM_ORACLE_STATE] = iFreezeState;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query freeze state succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: UnFreezeEx
Description  : 冻结保护机制使用，执行解冻和结束备份操作，重复解冻时不返回错误
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CAppPlugin::UnFreezeEx(CRequestMsg* req, CResponseMsg* rsp)
{
    app_auth_info_t appAuthInfo;
    (mp_void)GetDBAuthInfo(req, appAuthInfo);

    return m_app.UnFreezeEx(appAuthInfo);
}


