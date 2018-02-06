/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_APP_PLUGIN_H__
#define __AGENT_APP_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/app/App.h"

#define REST_PARAM_APP_ORALCE              "oracle"
#define REST_PARAM_APP_SQLSERVER           "sqlserver"
#define REST_PARAM_APP_DB2                 "db2"
#define REST_PARAM_APP_EXCHANGE            "exchange"
#define REST_PARAM_APP_INSTNAME            "instName"
#define REST_PARAM_APP_DBNAME              "dbName"
#define REST_PARAM_APP_VERSION             "version"
#define REST_PARAM_APP_STORAGE_GROUP       "storageGroup"
#define REST_PARAM_APP_BACKUP_SUCC         "backupSucc"
#define REST_PARAM_APP_TIME                "time"
#define REST_PARAM_APP_ERROR_CODE          "errorCode"

class CAppPlugin : public CServicePlugin
{
private:
    CApp m_app;

public:
    CAppPlugin();
    ~CAppPlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);
private:    
    mp_int32 AddAppInfosToResult(Json::Value& jValResult, vector<app_info_t>& vecAppInfos);
    mp_int32 AddAppInfoToResult(Json::Value& jValResult, app_info_t& appInfo);
    mp_int32 GetDBAuthInfo(CRequestMsg* req, app_auth_info_t& stAppAuthInfo);
    mp_int32 QueryInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Freeze(CRequestMsg* req, CResponseMsg* rsp);    
    mp_int32 UnFreeze(CRequestMsg* req, CResponseMsg* rsp);    
    mp_int32 EndBackup(CRequestMsg* req, CResponseMsg* rsp);    
    mp_int32 TruncateLog(CRequestMsg* req, CResponseMsg* rsp);    
    mp_int32 GetDBFreezeState(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UnFreezeEx(CRequestMsg* req, CResponseMsg* rsp);
};

#endif //__AGENT_APP_PLUGIN_H__

