/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_HOST_PLUGIN_H__
#define __AGENT_HOST_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "host/Host.h"
#include "cluster/Cluster.h"

#define REST_PARAM_HOST_NAME                       "name"
#define REST_PARAM_HOST_OS                         "os"
#define REST_PARAM_HOST_SN                         "sn"
#define REST_PARAM_HOST_VERSION                    "version"
#define REST_PARAM_HOST_BUILD_NUM                  "buildNum"
#define REST_PARAM_HOST_DISK_NUM                   "diskNumber"
#define REST_PARAM_HOST_DISK_NUMS                  "diskNumbers"
#define REST_PARAM_HOST_INIT_ISCSI                 "iscsis"
#define REST_PARAM_HOST_INIT_FC                    "fcs"
#define REST_PARAM_HOST_LUN_ID                     "lunId"
#define REST_PARAM_HOST_WWN                        "wwn"
#define REST_PARAM_HOST_ARRAY_SN                   "arraySn"
#define REST_PARAM_HOST_ARRAY_VENDOR               "arrayVendor"
#define REST_PARAM_HOST_ARRAY_MODEL                "arrayModel"
#define REST_RARAM_HOST_DEVICE_NAME                "deviceName"
#define REST_PARAM_HOST_DEVICE_DISKNUM             "diskNumber"
#define REST_PARAM_HOST_PARTISIONNAME              "partitionName"
#define REST_PARAM_HOST_CAPACITY                   "capacity"
#define REST_PARAM_HOST_DISKNAME                   "diskName"
#define REST_PARAM_HOST_LBA_ADDR                   "LBA"

#define REST_PARAM_HOST_FILENAME                   "fileName"
#define REST_PARAM_HOST_PARAMS                     "params"
#define REST_PARAM_HOST_IP                         "ip"
#define REST_PARAM_HOST_PORT                       "port"
#define REST_PARAM_HOST_SNMPTYPE                   "snmpType"
#define REST_PARAM_HOST_SNMP_AUTHTYPE              "authType"
#define REST_PARAM_HOST_SNMP_ENCRYPTYPE            "encryptType"

#define REST_PARAM_HOST_TIMEZONE_BIAS              "tzBias"
#define REST_PARAM_HOST_TIMEZONE_ISDST             "isDST"

#define REST_PARAM_HOST_THIRDPARTY_STATE           "state"
#define REST_PARAM_HOST_LOG_EXPORT_NAME            "sysInfoFile"



static const mp_char* SNMP_PROTOCOL_USER =  "HTTP_SNMPUSERNAME";
static const mp_char* SNMP_AUTH_PW       =  "HTTP_AUTHPASSWORD";
static const mp_char* SNMP_ENCRYPT_PW    =  "HTTP_ENCRYPTPASSWORD";

class CHostPlugin : public CServicePlugin
{
private:
    CHost m_host;
    
public:
    CHostPlugin();
    ~CHostPlugin();
    
    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);
    
private:
    mp_int32 QueryAgentVersion(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryHostInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryDiskInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryTimeZone(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryInitiators(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryPartisions(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 ScanDisk(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryThirdPartyScripts(CRequestMsg* req, CResponseMsg * rsp);
    mp_int32 ExecThirdPartyScript(CRequestMsg * req, CResponseMsg * rsp);
    mp_int32 RegTrapServer(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UnRegTrapServer(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 VerifySnmp(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 ExecFreezeScript(CRequestMsg* req, CResponseMsg * rsp);
    mp_int32 ExecThawScript(CRequestMsg* req, CResponseMsg * rsp);
    mp_int32 QueryFreezeStatusScript(CRequestMsg* req, CResponseMsg * rsp);
    mp_int32 CollectAgentLog(CRequestMsg* req, CResponseMsg * rsp);
    mp_int32 ExportAgentLog(CRequestMsg* req, CResponseMsg * rsp);
    mp_string GetLogName()
    {
        return m_host.GetLogName();
    }
#ifdef WIN32    
    mp_int32 DeviceOnline(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 DeviceBatchOnline(CRequestMsg* req, CResponseMsg* rsp);
#endif
};

#endif //__AGENT_HOST_PLUGIN_H__

