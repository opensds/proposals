/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_DEVICE_PLUGIN_H__
#define __AGENT_DEVICE_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "device/FileSys.h"
#include "device/Lvm.h"
#include "device/Udev.h"
#include "device/Asm.h"
#include "device/Raw.h"
#include "device/Link.h"
#include "device/Permission.h"

//Json请求参数宏定义
#define REST_PARAM_DEVICE_DEV_NAME        "deviceName"
#define REST_PARAM_DEVICE_MOUNT_POINT     "mountPoint"
#define REST_PARAM_DEVICE_VOL_TYPE        "volType"
#define REST_PARAM_DEVICE_ERROR_CODE      "errorCode"
#define REST_PARAM_DEVICE_FILE_SYS_TYPE   "fileSystemType"
#define REST_PARAM_DEVICE_CAPACITY        "capacity"
#define REST_PARAM_DEVICE_DISK_NUMBER     "diskNumber"
#define REST_PARAM_DEVICE_OFFSET          "startingOffSet"
#define REST_PARAM_DEVICE_WWN             "wwn"
#define REST_PARAM_DEVICE_LINKNAME        "linkName"
#define REST_PARAM_DEVICE_UDEVRULE        "udevRule"
#define REST_PARAM_DEVICE_DISK_NAMES      "diskNames"
#define REST_PARAM_DEVICE_VGNAME          "vgName"
#define REST_PARAM_DEVICE_VGNAMES         "vgNames"
#define REST_PARAM_DEVICE_ACTIVEMODE      "vgActiveMode"
#define REST_PARAM_DEVICE_RECOVERTYPE     "recoverType"
#define REST_PARAM_DEVICE_VOLTYPE         "volType"
#define REST_PARAM_DEVICE_VGSTATE         "state"
#define REST_PARAM_DEVICE_PVNAME          "pvName"
#define REST_PARAM_DEVICE_MAPINFO         "mapInfo"
#define REST_PARAM_DEVICE_PVINFO          "pvInfo"
#define REST_PARAM_DEVICE_PVS             "pvs"
#define REST_PARAM_DEVICE_DEV_PATH        "devicePath"
#define REST_PARAM_DEVICE_DEV_USERNAME    "userName"
#define REST_PARAM_DEVICE_DEV_PRIMODE     "privMode"
#define REST_PARAM_DEVICE_BACKUP_SUCC     "backupSucc"
#define REST_PARAM_DEVICE_FREEZE_STAT     "state"
#define REST_PARAM_DEVICE_FREEZE_TIME     "time"




//设备插件实现类
class CDevicePlugin : public CServicePlugin
{
public:
    CDevicePlugin();
    ~CDevicePlugin();
    
    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);

private:
    CFileSys m_fileSys;
#ifndef WIN32
    CLink m_link;
    CUdev m_udev;
    CAsm m_deviceASM;
    CLvm m_lvm;
    CRaw m_raw;
    CPermission m_permission;
#endif

private:
    //文件系统相关
    mp_int32 FileSysQueryInfo(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 FileSysMount(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 FileSysBatchMount(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 FileSysUmount(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 FileSysBatchUmount(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 DriveLetterBatchDelete(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp);
        
    mp_int32 Freeze(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UnFreeze(CRequestMsg* req, CResponseMsg* rsp);
    
#ifndef WIN32
    mp_int32 ScanASMLib(CRequestMsg* req, CResponseMsg* rsp);

    //软连接
    mp_int32 LinkCreate(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LinkDelete(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LinkBatchCreate(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LinkBatchDelete(CRequestMsg* req, CResponseMsg* rsp);
    
    //LVM相关
    mp_int32 LVMQueryVgs(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LVMExportVgs(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LVMImportVgs(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LVMActivateVgs(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LVMDeactivateVgs(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LVMQueryLVs(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 LVMScanDisks(CRequestMsg* req, CResponseMsg* rsp);

    //UDEV相关
    mp_int32 UDEVCreateRules(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UDEVDeleteRules(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UDEVBatchCreateRules(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 UDEVBatchDeleteRules(CRequestMsg* req, CResponseMsg* rsp);

    //Raw device相关
    mp_int32 RawDeviceCreate(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 RawDeviceDelete(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 RawDeviceBatchCreate(CRequestMsg* req, CResponseMsg* rsp);
    mp_int32 RawDeviceBatchDelete(CRequestMsg* req, CResponseMsg* rsp);

    //权限相关
    mp_int32 Permission(CRequestMsg* req, CResponseMsg* rsp);

#endif //WIN32
};

#endif //__AGENT_DEVICE_PLUGIN_H__

