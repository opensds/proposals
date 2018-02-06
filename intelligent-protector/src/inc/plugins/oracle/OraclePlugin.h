/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_ORACLE_PLUGIN_H__
#define __AGENT_ORACLE_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/oracle/Oracle.h"

#define REST_PARAM_ORACLE_ARCHIVE_LOG_MODE       "archiveLogMode"

class COraclePlugin : public CServicePlugin
{
private:
    COracle m_oracle;
    
public:
    COraclePlugin();
    ~COraclePlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);

private:
    mp_int32 QueryInfo(CRequestMsg* req, CResponseMsg* rsp);
	mp_int32 QueryPDBInfo(CRequestMsg* req, CResponseMsg* rsp);
	mp_int32 StartPDB(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryLunInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StartDB(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StopDB(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Test(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 CheckArchiveThreshold(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Freeze(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 Thaw(CRequestMsg* req, CResponseMsg* rsp);    
    mp_int32 ArchiveDB(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StartASMInstance(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StopASMInstance(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StartRACCluster(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 GetDBFreezeState(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 GetDBAuthParam(CRequestMsg* req, oracle_db_info_t &stDBInfo);
	mp_int32 GetPDBAuthParam(CRequestMsg* req, oracle_pdb_req_info_t &stPDBInfo);
};

#endif //__AGENT_ORACLE_PLUGIN_H__

