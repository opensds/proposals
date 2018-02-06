/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_EXCHANGE_PLUGIN_H__
#define __AGENT_EXCHANGE_PLUGIN_H__
#ifdef WIN32

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/exchange/Exchange.h"

#define REST_PARAM_EXCHANGE_DB_NAME           "dbName"
#define REST_PARAM_EXCHANGE_VERSION           "version"
#define REST_PARAM_EXCHANGE_STATE             "state"
#define REST_PARAM_EXCHANGE_STRGRP            "storageGroup"
#define REST_PARAM_EXCHANGE_EDBPATH           "edbPath"
#define REST_PARAM_EXCHANGE_LOGPATH           "logPath"
#define REST_PARAM_EXCHANGE_SYSPATH           "systemPath"
#define REST_PARAM_EXCHANGE_ISCOMM            "isCommon"
#define REST_PARAM_EXCHANGE_LUNID             "lunId"
#define REST_PARAM_EXCHANGE_ARRAYSN           "arraySn"
#define REST_PARAM_EXCHANGE_DEVNAME           "deviceName"
#define REST_PARAM_EXCHANGE_WWN               "wwn"
#define REST_PARAM_EXCHANGE_VOLNAME           "volName"
#define REST_PARAM_EXCHANGE_LBA               "LBA"
#define REST_PARAM_EXCHANGE_DBNAME            "dbNames"
#define REST_PARAM_EXCHANGE_SERNAME           "serverName"
#define REST_PARAM_EXCHANGE_DEVTYPE           "deviceType"
#define REST_PARAM_EXCHANGE_MASTDBNAME        "masterDbNames"
#define REST_PARAM_EXCHANGE_MASTRGRPNAME      "masterStorageGroup"
#define REST_PARAM_EXCHANGE_SLAVDBNAME        "slaveDbNames"
#define REST_PARAM_EXCHANGE_SALVSTRGRPNAME    "slaveStorageGroup"
#define REST_PARAM_EXCHANGE_MASTSERVNAME      "masterServerName"
#define REST_PARAM_EXCHANGE_STARTYPE          "starType"
#define REST_PARAM_EXCHANGE_DISK_NAMES        "diskNames"
#define REST_PARAM_EXCHANGE_FREEZE_STAT       "state"

class CExchangePlugin : public CServicePlugin
{
public:
    CExchangePlugin();
    ~CExchangePlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);
private:
    mp_int32 QueryInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryLunInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Start(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Dismount(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 CExchangePlugin::Clear(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StartHostContrService(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StopHostContrService(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Freeze(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UnFreeze(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp);
private:
    CExchange m_exchange;
};

#endif
#endif //__AGENT_EXCHANGE_PLUGIN_H__

