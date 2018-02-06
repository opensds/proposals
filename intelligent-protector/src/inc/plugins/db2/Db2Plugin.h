/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_DB2_PLUGIN_H__
#define __AGENT_DB2_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/db2/Db2.h"

#define REST_PARAM_DB2_INSTNAME           "instName"
#define REST_PARAM_DB2_DBNAME              "dbName"
#define REST_PARAM_DB2_VERSION             "version"
#define REST_PARAM_DB2_STATE                  "state"
#define REST_PARAM_DB2_LUNID                  "lunId"
#define REST_PARAM_DB2_UUID                    "uuid"
#define REST_PARAM_DB2_ARRAYSN             "arraySn"
#define REST_PARAM_DB2_WWN                    "wwn"
#define REST_PARAM_DB2_VOLTYPE              "volType"
#define REST_PARAM_DB2_VGNAME               "vgName"
#define REST_PARAM_DB2_VOLNAME             "volName"
#define REST_PARAM_DB2_STORAGETYPE     "deviceType"
#define REST_PARAM_DB2_DEVICENAME       "deviceName"
#define REST_PARAM_DB2_PVNAME               "pvName"

class CDb2Plugin : public CServicePlugin
{
private:
    CDB2 m_db2;

public:
    CDb2Plugin();
    ~CDb2Plugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);

private:
    mp_int32 QueryInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryLunInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Start(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Stop(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Test(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Freeze(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UnFreeze(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp);
};

#endif //__AGENT_DB2_PLUGIN_H__

