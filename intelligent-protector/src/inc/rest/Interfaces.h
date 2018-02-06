/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_REST_INTERFACES_H__
#define __AGENT_REST_INTERFACES_H__

//URL Method
#define REST_URL_METHOD_GET                       "GET"
#define REST_URL_METHOD_PUT                       "PUT"
#define REST_URL_METHOD_POST                      "POST"
#define REST_URL_METHOD_DELETE                    "DELETE"

//Host
#define REST_HOST_QUERY_AGENT_VERSION             "/host/agent"
#define REST_HOST_QUERY_INFO                      "/host"
#define REST_HOST_QUERY_INITIATOR                 "/host/initiators"
#define REST_HOST_QUERY_DISKS                     "/host/disks"
#define REST_HOST_SCAN_DISK                       "/host/disks/action/scan"
#define REST_HOST_QUERY_PARTITIONS                "/host/disks/partitions"
#define REST_HOST_QUERY_TIMEZONE                  "/host/timezone"
#define REST_HOST_EXPORT_LOG                      "/host/agent/systeminfo"
#define REST_HOST_COLLECT_LOG                     "/host/agent/action/collectsysteminfo"

//Cluster
#define REST_CLUSTER_QUERY_ACTIVEHOST             "/cluster/activehost"
#define REST_CLUSTER_QUERY_CLUSTERINFO            "/cluster/services"
#define REST_CLUSTER_START_CLUSTER                "/cluster/action/start"
#define REST_CLUSTER_START_RESOURCEGROUP          "/cluster/resourcegroup/action/start"
#define REST_CLUSTER_STOP_RESOURCEGROUP           "/cluster/resourcegroup/action/stop"

//ThirdParty
#define REST_HOST_THIRDPARTY_QUERY_FILE_INFO      "/host/thirdparty/files"
#define REST_HOST_THIRDPARTY_EXEC_FILE            "/host/thirdparty/action/excute"
#define REST_HOST_REG_TRAP_SERVER                 "/host/trapserver/register"
#define REST_HOST_UNREG_TRAP_SERVER               "/host/trapserver/unregister"
#define REST_HOST_VERIFY_SNMP                     "/host/snmp/params/action/verify"
#define REST_HOST_ONLINE                          "/host/disks/action/online"
#define REST_HOST_BATCH_ONLINE                    "/host/disks/action/batchonline"
#define REST_HOST_FREEZE_SCRIPT                   "/host/thirdparty/action/freeze"
#define REST_HOST_UNFREEZE_SCRIPT                 "/host/thirdparty/action/unfreeze"
#define REST_HOST_QUERY_STATUS_SCRIPT             "/host/thirdparty/freezestate"

//Device
//FileSystem
#define REST_DEVICE_FILESYS_QUERY                 "/device/filesystems"
#define REST_DEVICE_FILESYS_MOUNT                 "/device/filesystems/action/mount"
#define REST_DEVICE_FILESYS_MOUTN_BATCH           "/device/filesystems/action/batchmount"
#define REST_DEVICE_FILESYS_UNMOUNT               "/device/filesystems/action/unmount"
#define REST_DEVICE_FILESYS_UMOUNT_BATCH          "/device/filesystems/action/batchunmount"
#define REST_DEVICE_FILESYS_FREEZE                "/device/filesystems/action/freeze"
#define REST_DEVICE_FILESYS_UNFREEZE              "/device/filesystems/action/unfreeze"
#define REST_DEVICE_FILESYS_FREEZESTATUS          "/device/filesystems/freezestate"
#define REST_DEVICE_DRIVELETTER_DELETE_BATCH      "/device/driveletter/action/batchdel"

//LVM
#define REST_DEVICE_LVM_QUERY_VGS                 "/device/lvm/vgs"
#define REST_DEVICE_LVM_EXPORT_VGS                "/device/lvm/vgs/action/export"
#define REST_DEVICE_LVM_IMPORT_VGS                "/device/lvm/vgs/action/import"
#define REST_DEVICE_LVM_ACTIVATE                  "/device/lvm/vgs/action/activate"
#define REST_DEVICE_LVM_DEACTIVATE                "/device/lvm/vgs/action/deactivate"
#define REST_DEVICE_LVM_QUERY_LVS                 "/device/lvm/vgs/lvs"
#define REST_DEVICE_LVM_SCAN_DISKS                "/device/lvm/action/scandisks"

//ASMLIB
#define REST_DEVICE_ASMLIB_SCAN                   "/device/asmlib"
//UDEV
#define REST_DEVICE_UDEV                          "/device/udev/rule"
#define REST_DEVICE_UDEV_BATCH                    "/device/udev/batchrules"
//RAW
#define REST_DEVICE_RAW                           "/device/raw"
#define REST_DEVICE_RAW_BATCH                     "/device/batchraws"

//Link
#define REST_DEVICE_LINK                          "/device/link"
#define REST_DEVICE_BATCHLINKS                    "/device/batchlinks"

//权限设置
#define REST_DEVICE_PERMISSION                    "/device/permission"

//DB2
#define REST_DB2_QUERY_DB_INFO                    "/db2/databases"
#define REST_DB2_QUERY_LUN_INFO                   "/db2/databases/action/getluns"
#define REST_CLUSTER_VCS_RESGRP_START             "/cluster/resourceGroup/start"
#define REST_DB2_SART                             "/db2/databases/action/start"
#define REST_DB2_STOP                             "/db2/databases/action/stop"
#define REST_DB2_TEST                             "/db2/databases/action/testconnection"
#define REST_DB2_FREEZE                           "/db2/databases/action/freeze"
#define REST_DB2_UNFREEZE                         "/db2/databases/action/unfreeze"
#define REST_DB2_FREEZESTATE                      "/db2/databases/freezestate"

//Oracle
#define REST_ORACLE_QUERY_DB_INFO                 "/oracle/databases"
#define REST_ORACLE_QUERY_PDB_INFO				  "/oracle/pdbs"
#define REST_ORACLE_START_PDB					  "/oracle/pdbs/action/start"
#define REST_ORACLE_QUERY_LUN_INFO                "/oracle/databases/action/getluns"
#define REST_ORACLE_START                         "/oracle/databases/action/start"
#define REST_ORACLE_STOP                          "/oracle/databases/action/stop"
#define REST_ORACLE_CHECKARCHIVETHRESHOLD         "/oracle/databases/action/checkthreshold"
#define REST_ORACLE_TEST                          "/oracle/databases/action/testconnection"
#define REST_ORACLE_FREEZE                        "/oracle/databases/action/freeze"
#define REST_ORACLE_UNFREEZE                      "/oracle/databases/action/unfreeze"
#define REST_ORACLE_ARCHIVE                       "/oracle/databases/action/archivelog"
#define REST_ORACLE_STARTASMINSTANCE              "/oracle/action/startasminstance"
#define REST_ORACLE_STOPASMINSTANCE               "/oracle/action/stopasminstance"
#define REST_ORACLE_STARTRACCLUSTER               "/cluster/action/start"
#define REST_ORACLE_FREEZESTATE                   "/oracle/databases/freezestate"

//Exchange
#define REST_EXCHANGE_QUERY_DB_INFO               "/exchange/databases"
#define REST_EXCHANGE_QUERY_LUN_INFO              "/exchange/databases/action/getluns"
#define REST_EXCHANGE_START                       "/exchange/databases/action/load"
#define REST_EXCHANGE_DISMOUNT                    "/exchange/databases/action/dismount"
#define REST_EXCHANGE_CLEAR                       "/exchange/databases/action/clear"
#define REST_EXCHANGE_QUERY_DB_COPYSTATE          "/exchange/dabases/copies/states"
#define REST_EXCHANGE_SUSPEND_PASSIVE_DB          "/exchange/dabases/copies/states/action/suspend"
#define REST_EXCHANGE_UPDATE_PASSIVE_DB           "/exchange/dabases/copies/states/action/update"
#define REST_EXCHANGE_ACTIVE_PASSIVE_DB           "/exchange/databases/copies/action/translate"
#define REST_EXCHANGE_HOSTCONTROLLER_START        "/exchange/hostcontroller/action/start"
#define REST_EXCHANGE_HOSTCONTROLLER_STOP         "/exchange/hostcontroller/action/stop"
#define REST_EXCHANGE_FREEZE_DB                   "/exchange/databases/action/freeze"
#define REST_EXCHANGE_UNFREEZE_DB                 "/exchange/databases/action/unfreeze"
#define REST_EXCHANGE_GET_FREEZE_STAT             "/exchange/databases/freezestate"

//Sqlserver
#define REST_SQLSERVER_QUERY_DB_INFO              "/sqlserver/databases"
#define REST_SQLSERVER_QUERY_LUN_INFO             "/sqlserver/databases/action/getluns"
#define REST_SQLSERVER_START                      "/sqlserver/databases/action/start"
#define REST_SQLSERVER_STOP                       "/sqlserver/databases/action/stop"
#define REST_SQLSERVER_TEST                       "/sqlserver/databases/action/testconnection"
#define REST_SQLSERVER_FREEZE_DB                  "/sqlserver/databases/action/freeze"
#define REST_SQLSERVER_UNFREEZE_DB                "/sqlserver/databases/action/unfreeze"
#define REST_SQLSERVER_GET_FREEZE_STAT            "/sqlserver/databases/freezestate"

//Cache
#define REST_CACHE_QUERY_DB_INFO                    "/cache/instances"
#define REST_CACHE_QUERY_LUN_INFO                   "/cache/instances/action/getluns"
#define REST_CACHE_TEST                             "/cache/instances/action/testconnection"

//App
#define REST_APP_QUERY_DB_INFO                    "/app"
#define REST_APP_FREEZE                           "/app/action/freezeall"
#define REST_APP_UNFREEZE                         "/app/action/unfreezeall"
#define REST_APP_ENDBACKUP                        "/app/action/endbackup"
#define REST_APP_TRUNCATE_LOG                     "/app/action/truncatelog"
#define REST_APP_QUERY_DB_FREEZESTATE             "/app/freezestate"
#define REST_APP_UNFREEZEEX                       "/app/action/unfreezeex"

//Sybase
#define REST_SYBASE_STARTDB                       "/sybase/databases/action/start"
#define REST_SYBASE_STOPDB                        "/sybase/databases/action/stop"
#define REST_SYBASE_TESTDB                        "/sybase/databases/action/testconnection"
#define REST_SYBASE_FREEZE                        "/sybase/databases/action/freeze"
#define REST_SYBASE_THAW                          "/sybase/databases/action/unfreeze"
#define REST_SYBASE_GET_FREEZE_STATE              "/sybase/databases/freezestate"

//Hana
#define REST_HANA_STARTDB                       "/hana/databases/action/start"
#define REST_HANA_STOPDB                        "/hana/databases/action/stop"
#define REST_HANA_TESTDB                        "/hana/databases/action/testconnection"
#define REST_HANA_FREEZE                        "/hana/databases/action/freeze"
#define REST_HANA_THAW                          "/hana/databases/action/unfreeze"
#define REST_HANA_GET_FREEZE_STATE              "/hana/databases/freezestate"

//错误消息体
#define REST_PARAM_ERROR_CODE                     "errorCode"

//CodeDex误报，Password Management:Hardcoded Password
//消息头key
static const char* HTTPPARAM_DBUSERNAME         = "HTTP_DBUSERNAME";
static const char* HTTPPARAM_DBPASSWORD         = "HTTP_DBPASSWORD";
static const char* HTTPPARAM_ASMSERNAME         = "HTTP_ASMUSERNAME";
static const char* HTTPPARAM_ASMPASSWORD        = "HTTP_ASMPASSWORD";
static const char* HTTPPARAM_SNMPAUTHPW         = "HTTP_AUTHPASSWORD";
static const char* HTTPPARAM_SNMPENCRYPW        = "HTTP_ENCRYPTPASSWORD";
static const char* UNKNOWN                      = "Unknown";
static const char* REMOTE_ADDR                  = "REMOTE_ADDR";
static const char* REQUEST_URI                  = "REQUEST_URI";
static const char* REQUEST_METHOD               = "REQUEST_METHOD";
static const char* CONTENT_LENGTH               = "CONTENT_LENGTH";
static const char* QUERY_STRING                 = "QUERY_STRING";
static const char* STATUS                       = "Status";
static const char* CONTENT_TYPE                 = "CONTENT_TYPE";
static const char* CACHE_CONTROL                = "Cache-Control";
static const char* CONTENT_ENCODING             = "Content-Encoding";
static const char* UNAME                        = "HTTP_X_AUTH_USER";
static const char* PW                           = "HTTP_X_AUTH_KEY";
static const char* LISTEN_ADDR					= "SERVER_ADDR";

//如下json消息key定义，agent主进程和so均会用到，提取在这里定义
#define REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME     "freezeFile"
#define REST_PARAM_HOST_FREEZE_SCRIPT_PARAM        "freezeParams"
#define REST_PARAM_HOST_UNFREEZE_SCRIPT_FILENAME   "unfreezeFile"
#define REST_PARAM_HOST_UNFREEZE_SCRIPT_PARAM      "unfreezeParam"
#define REST_PARAM_HOST_QUERY_SCRIPT_FILENAME      "queryFile"
#define REST_PARAM_HOST_QUERY_SCRIPT_PARAM         "queryParam"
#define REST_PARAM_HOST_FREEZE_TIMEOUT             "freezeTimeOut"
#endif //__AGENT_REST_INTERFACES_H__

