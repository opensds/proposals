/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_SQL_SERVER_PLUGIN_H__
#define __AGENT_SQL_SERVER_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/sqlserver/SqlServer.h"

#ifdef WIN32
#define REST_PARAM_SQLSERVER_INSTNAME      "instName"
#define REST_PARAM_SQLSERVER_DBNAME        "dbName"
#define REST_PARAM_SQLSERVER_VERSION       "version"
#define REST_PARAM_SQLSERVER_STATE         "state"
#define REST_PARAM_SQLSERVER_ISCLUSTER     "isCluster"
#define REST_PARAM_SQLSERVER_LUNID         "lunId"
#define REST_PARAM_SQLSERVER_ARRAYSN       "arraySn"
#define REST_PARAM_SQLSERVER_WWN           "wwn"
#define REST_PARAM_SQLSERVER_DEVICENAME    "deviceName"
#define REST_PARAM_SQLSERVER_VOLNAME       "volName"
#define REST_PARAM_SQLSERVER_DEVICETYPE    "deviceType"
#define REST_PARAM_SQLSERVER_LBA           "Lba"
#define REST_PARAM_SQLSERVER_DISKNAMES     "diskNames"
#define REST_PARAM_SQLSERVER_FREEZE_STAT   "state"
#define REST_PARAM_SQLSERVER_NETWORK_NAME   "networkName"


class CSqlServerPlugin : public CServicePlugin
{
private:
    CSqlServer m_sqlserver;

public:
    CSqlServerPlugin();
    ~CSqlServerPlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);

private:
    mp_int32 QueryDBBasicInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryDBLUNInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StartDB(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StopDB(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 TestDB(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Freeze(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UnFreeze(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp);
};

#endif
#endif //__AGENT_SQL_SERVER_PLUGIN_H__

