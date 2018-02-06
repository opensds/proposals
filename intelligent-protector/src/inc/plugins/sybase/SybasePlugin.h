/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_SYBASE_PLUGIN_H__
#define __AGENT_SYBASE_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/sybase/Sybase.h"

#define REST_PARAM_SYBASAE_INSTNAME            "instName"
#define REST_PARAM_SYBASAE_DBNAME              "dbName"
#define REST_PARAM_SYBASAE_DBNAMES             "dbNames"


class CSybasePlugin : public CServicePlugin
{
private:
    CSybase m_sybase;

public:
    CSybasePlugin();
    ~CSybasePlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);

private:
    mp_int32 StartDB(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 StopDB(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 Test(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 FreezeDB(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 ThawDB(CRequestMsg* req, CResponseMsg* rsp);
    
    mp_int32 GetFreezeStatus(CRequestMsg* req, CResponseMsg *rsp);

    mp_int32 GetDBAuthParam(CRequestMsg* req, sybase_db_info_t &stDBInfo);

};

#endif //__AGENT_SYBASE_PLUGIN_H__

