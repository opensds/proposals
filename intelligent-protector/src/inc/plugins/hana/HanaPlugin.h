/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_HANA_PLUGIN_H__
#define __AGENT_HANA_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/hana/Hana.h"

#define REST_PARAM_HANA_INSTNUM            "instNum"
#define REST_PARAM_HANA_DBNAME             "dbName"
#define REST_PARAM_HANA_DBNAMES            "dbNames"


class CHanaPlugin : public CServicePlugin
{
private:
    CHana m_hana;

public:
    CHanaPlugin();
    ~CHanaPlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);

private:
    mp_int32 StartDB(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 StopDB(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 Test(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 FreezeDB(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 ThawDB(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 GetFreezeStatus(CRequestMsg* req, CResponseMsg *rsp);

    mp_int32 GetDBAuthParam(CRequestMsg* req, hana_db_info_t &stDBInfo);

};

#endif //__AGENT_HANA_PLUGIN_H__

