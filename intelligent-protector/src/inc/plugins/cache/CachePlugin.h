/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_CACHE_PLUGIN_H__
#define __AGENT_CACHE_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/cache/Cache.h"

#define REST_PARAM_CACHE_INSTNAME           "instName"
#define REST_PARAM_CACHE_DBNAME              "dbName"
#define REST_PARAM_CACHE_VERSION             "version"
#define REST_PARAM_CACHE_STATE                  "state"

#define REST_PARAM_CACHE_LUNID                  "lunId"
//#define REST_PARAM_CACHE_UUID                    "uuid"
#define REST_PARAM_CACHE_ARRAYSN             "arraySn"
#define REST_PARAM_CACHE_WWN                    "wwn"
//#define REST_PARAM_CACHE_VOLTYPE              "volType"
#define REST_PARAM_CACHE_VGNAME               "vgName"
//#define REST_PARAM_CACHE_VOLNAME             "volName"
//#define REST_PARAM_CACHE_STORAGETYPE     "deviceType"
#define REST_PARAM_CACHE_DEVICENAME       "deviceName"
//#define REST_PARAM_CACHE_PVNAME               "pvName"
#define REST_PARAM_CACHE_DEVICEPATH       "devicePath"

class CCachePlugin : public CServicePlugin
{
private:
    CCache m_cache;

public:
    CCachePlugin();
    ~CCachePlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);

private:
    mp_int32 QueryInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryLunInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Test(CRequestMsg* req, CResponseMsg* rsp);
};

#endif //__AGENT_CACHE_PLUGIN_H__

