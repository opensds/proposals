/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_ClUSTER_PLUGIN_H__
#define __AGENT_ClUSTER_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "cluster/Cluster.h"

#define REST_PARAM_CLUSTER_RESGRPNAME            "resourceGroup"
#define REST_PARAM_CLUSTER_DEVGRPNAME            "deviceGroup"
#define REST_PARAM_CLUSTER_CLUSTERTYPE           "clusterType"
#define REST_PARAM_CLUSTER_RESOURCENAME          "resourceName"
#define REST_PARAM_CLUSTER_DBNAME                "dbName"
#define REST_PARAM_CLUSTER_INSTNAME              "instName"
#define REST_PARAM_CLUSTER_APPTYPE                "appType"
#define REST_PARAM_CLUSTER_ISACTIVE               "isActive"
#define REST_PARAM_CLUSTER_CLUSTERNAME           "clusterName" 
#define REST_PARAM_CLUSTER_VGACTIVEMODE          "vgActiveMode" 
#define REST_PARAM_CLUSTER_NETWORKNAME          "networkName" 

class CClusterPlugin : public CServicePlugin
{
private:
    CCluster m_cluster;
    
public:
    CClusterPlugin();
    ~CClusterPlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);
    
private:
    mp_int32 QueryActiveHost(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryClusterInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StartCluster(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StartResouceGroup(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 StopResouceGroup(CRequestMsg* req, CResponseMsg* rsp);
	mp_int32 GetDBAuthParam(CRequestMsg* req, mp_string &strdbUsername, mp_string &strdbPassword);
	mp_void GetDBInfo(map<mp_string, mp_string>& vreqal, db_info_t &stdbInfo, mp_string &strDBType, mp_string &strClusterType);
};

#endif //__AGENT_CLUSTER_PLUGIN_H__

