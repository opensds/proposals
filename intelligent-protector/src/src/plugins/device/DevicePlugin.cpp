/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/device/DevicePlugin.h"
#include "common/String.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"

//注册插件
REGISTER_PLUGIN(CDevicePlugin); //lint !e19
CDevicePlugin::CDevicePlugin()
{
    //注册Device插件Action
    REGISTER_ACTION(REST_DEVICE_FILESYS_QUERY, REST_URL_METHOD_GET, &CDevicePlugin::FileSysQueryInfo);
    REGISTER_ACTION(REST_DEVICE_FILESYS_MOUNT, REST_URL_METHOD_PUT, &CDevicePlugin::FileSysMount);
    REGISTER_ACTION(REST_DEVICE_FILESYS_UNMOUNT, REST_URL_METHOD_PUT, &CDevicePlugin::FileSysUmount);
    REGISTER_ACTION(REST_DEVICE_FILESYS_MOUTN_BATCH, REST_URL_METHOD_PUT, &CDevicePlugin::FileSysBatchMount);
    REGISTER_ACTION(REST_DEVICE_FILESYS_UMOUNT_BATCH, REST_URL_METHOD_PUT, &CDevicePlugin::FileSysBatchUmount);
    REGISTER_ACTION(REST_DEVICE_DRIVELETTER_DELETE_BATCH, REST_URL_METHOD_PUT, &CDevicePlugin::DriveLetterBatchDelete);
    REGISTER_ACTION(REST_DEVICE_FILESYS_FREEZE, REST_URL_METHOD_PUT, &CDevicePlugin::Freeze);
    REGISTER_ACTION(REST_DEVICE_FILESYS_UNFREEZE, REST_URL_METHOD_PUT, &CDevicePlugin::UnFreeze);
    REGISTER_ACTION(REST_DEVICE_FILESYS_FREEZESTATUS, REST_URL_METHOD_GET, &CDevicePlugin::QueryFreezeState);
    
#ifndef WIN32
    REGISTER_ACTION(REST_DEVICE_LINK, REST_URL_METHOD_POST, &CDevicePlugin::LinkCreate);
    REGISTER_ACTION(REST_DEVICE_LINK, REST_URL_METHOD_DELETE, &CDevicePlugin::LinkDelete);
    REGISTER_ACTION(REST_DEVICE_BATCHLINKS, REST_URL_METHOD_POST, &CDevicePlugin::LinkBatchCreate);
    REGISTER_ACTION(REST_DEVICE_BATCHLINKS, REST_URL_METHOD_DELETE, &CDevicePlugin::LinkBatchDelete);
    REGISTER_ACTION(REST_DEVICE_LVM_QUERY_VGS, REST_URL_METHOD_GET, &CDevicePlugin::LVMQueryVgs);
    REGISTER_ACTION(REST_DEVICE_LVM_EXPORT_VGS, REST_URL_METHOD_PUT, &CDevicePlugin::LVMExportVgs);
    REGISTER_ACTION(REST_DEVICE_LVM_IMPORT_VGS, REST_URL_METHOD_PUT, &CDevicePlugin::LVMImportVgs);
    REGISTER_ACTION(REST_DEVICE_LVM_ACTIVATE, REST_URL_METHOD_PUT, &CDevicePlugin::LVMActivateVgs);
    REGISTER_ACTION(REST_DEVICE_LVM_DEACTIVATE, REST_URL_METHOD_PUT, &CDevicePlugin::LVMDeactivateVgs);
    REGISTER_ACTION(REST_DEVICE_LVM_QUERY_LVS, REST_URL_METHOD_GET, &CDevicePlugin::LVMQueryLVs);
    REGISTER_ACTION(REST_DEVICE_LVM_SCAN_DISKS, REST_URL_METHOD_PUT, &CDevicePlugin::LVMScanDisks);
    REGISTER_ACTION(REST_DEVICE_UDEV, REST_URL_METHOD_POST, &CDevicePlugin::UDEVCreateRules);
    REGISTER_ACTION(REST_DEVICE_UDEV, REST_URL_METHOD_DELETE, &CDevicePlugin::UDEVDeleteRules);
    REGISTER_ACTION(REST_DEVICE_UDEV_BATCH, REST_URL_METHOD_POST, &CDevicePlugin::UDEVBatchCreateRules);
    REGISTER_ACTION(REST_DEVICE_UDEV_BATCH, REST_URL_METHOD_DELETE, &CDevicePlugin::UDEVBatchDeleteRules);
    REGISTER_ACTION(REST_DEVICE_ASMLIB_SCAN, REST_URL_METHOD_PUT, &CDevicePlugin::ScanASMLib);
    REGISTER_ACTION(REST_DEVICE_RAW, REST_URL_METHOD_POST, &CDevicePlugin::RawDeviceCreate);
    REGISTER_ACTION(REST_DEVICE_RAW, REST_URL_METHOD_DELETE, &CDevicePlugin::RawDeviceDelete);
    REGISTER_ACTION(REST_DEVICE_RAW_BATCH, REST_URL_METHOD_POST, &CDevicePlugin::RawDeviceBatchCreate);
    REGISTER_ACTION(REST_DEVICE_RAW_BATCH, REST_URL_METHOD_DELETE, &CDevicePlugin::RawDeviceBatchDelete);
    REGISTER_ACTION(REST_DEVICE_PERMISSION, REST_URL_METHOD_POST, &CDevicePlugin::Permission);
    
#endif
}

CDevicePlugin::~CDevicePlugin()
{
}
/*------------------------------------------------------------ 
Description  :设备组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 输入信息
Output       : rsp -- 返回信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    //调用插件具体Action
    DO_ACTION(CDevicePlugin, req, rsp);
}


/*------------------------------------------------------------ 
Description  : 查询文件系统信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::FileSysQueryInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector<file_sys_info_t> vecFileInfos;
    vector<file_sys_info_t>::iterator iter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query file sys info.");
    iRet = m_fileSys.QueryFileSysInfo(vecFileInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query file sys info failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jValue = rsp->GetJsonValueRef();
    for (iter = vecFileInfos.begin(); iter != vecFileInfos.end(); ++iter)
    {
        Json::Value jInfo;
        jInfo[REST_PARAM_DEVICE_DEV_NAME] = iter->deviceName;
        jInfo[REST_PARAM_DEVICE_MOUNT_POINT] = iter->mountpoint;
        jInfo[REST_PARAM_DEVICE_FILE_SYS_TYPE] = iter->fileSysType;
        jInfo[REST_PARAM_DEVICE_VOL_TYPE] = iter->volType;
        jInfo[REST_PARAM_DEVICE_CAPACITY] = iter->capacity;
#ifdef WIN32
        jInfo[REST_PARAM_DEVICE_OFFSET] = iter->offSet;
        jInfo[REST_PARAM_DEVICE_DISK_NUMBER] = iter->diskNumber;
#endif

        jValue.append(jInfo);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query file sys info succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 挂载文件系统
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::FileSysMount(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mount_info_t mountInfo;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin mount file sys.");

    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_DEV_NAME, mountInfo.deviceName);
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_MOUNT_POINT, mountInfo.mountPoint);
    GET_JSON_INT32(jValue, REST_PARAM_DEVICE_VOL_TYPE, mountInfo.volumeType);

    CHECK_FAIL_EX(CheckFileSysMountParam(mountInfo.deviceName, mountInfo.volumeType, mountInfo.mountPoint));
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get mount info, dev path %s, mount point %s, voltype %d.",
        mountInfo.deviceName.c_str(), mountInfo.mountPoint.c_str(), mountInfo.volumeType);
    iRet = m_fileSys.Mount(mountInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Mount file sys failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Mount file sys succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 批量挂载文件系统
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::FileSysBatchMount(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    mount_info_t mountInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin batch mount file sys.");

    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_uint32 uiCount = jValue.size();
    for (mp_uint32 i = 0; i < uiCount; i++)
    {
        const Json::Value& jvTmp = jValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_DEV_NAME, mountInfo.deviceName);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_MOUNT_POINT, mountInfo.mountPoint);
        GET_JSON_INT32(jvTmp, REST_PARAM_DEVICE_VOL_TYPE, mountInfo.volumeType);
        
        CHECK_FAIL_EX(CheckFileSysMountParam(mountInfo.deviceName, mountInfo.volumeType, mountInfo.mountPoint));
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get mount info, dev path %s, mount point %s, voltype %d.",
            mountInfo.deviceName.c_str(), mountInfo.mountPoint.c_str(), mountInfo.volumeType);
        iRet = m_fileSys.Mount(mountInfo);
        if (MP_SUCCESS != iRet)
        {
            Json::Value jv;
            jv[REST_PARAM_DEVICE_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_DEV_NAME] = mountInfo.deviceName;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Mount file sys failed, add to failed list, dev %s, errcode %d.",
                mountInfo.deviceName.c_str(), iRet);
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End batch mount file sys.");
    return iRetRst;
}

/*------------------------------------------------------------ 
Description  : 去挂载文件系统
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::FileSysUmount(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    umount_info_t umountInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin umount file sys.");

    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_DEV_NAME, umountInfo.deviceName);
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_MOUNT_POINT, umountInfo.mountPoint);
    GET_JSON_INT32(jValue, REST_PARAM_DEVICE_VOL_TYPE, umountInfo.volumeType);

    CHECK_FAIL_EX(CheckFileSysMountParam(umountInfo.deviceName, umountInfo.volumeType, umountInfo.mountPoint));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get umount info, dev path %s, mount point %s, volume type %d.",
        umountInfo.deviceName.c_str(), umountInfo.mountPoint.c_str(), umountInfo.volumeType);
    iRet = m_fileSys.UMount(umountInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Umount file sys failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Umount file sys succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 批量去挂载文件系统
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::FileSysBatchUmount(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    umount_info_t umountInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin batch umount file sys.");

    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_uint32 uiCount = jValue.size();
    for (mp_uint32 i = 0; i < uiCount; i++)
    {
        const Json::Value& jvTmp = jValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_DEV_NAME, umountInfo.deviceName);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_MOUNT_POINT, umountInfo.mountPoint);
        GET_JSON_INT32(jvTmp, REST_PARAM_DEVICE_VOL_TYPE, umountInfo.volumeType);

        CHECK_FAIL_EX(CheckFileSysMountParam(umountInfo.deviceName, umountInfo.volumeType, umountInfo.mountPoint));

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get umount info, dev path %s, mount point %s, volume type %d.",
            umountInfo.deviceName.c_str(), umountInfo.mountPoint.c_str(), umountInfo.volumeType);
        iRet = m_fileSys.UMount(umountInfo);
        if (MP_SUCCESS != iRet)
        {
            Json::Value jv;
            jv[REST_PARAM_DEVICE_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_DEV_NAME] = umountInfo.deviceName;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Umount file sys failed, add to failed list, dev %s, errcode %d.",
                umountInfo.deviceName.c_str(), iRet);
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Edn batch umount file sys.");
    return iRetRst;
}

/*------------------------------------------------------------ 
Description  : 冻结文件系统
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::Freeze(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecDriveLetters;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze file sys.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    
    GET_JSON_ARRAY_STRING(jReqValue, REST_PARAM_DEVICE_DISK_NAMES, vecDriveLetters);

    //参数校验
    vector<mp_string>::iterator iter;
    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); ++iter)
    {
        CHECK_FAIL_EX(CheckFileSysFreezeParam(*iter));
    }
    iRet = m_fileSys.Freeze(vecDriveLetters);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze file sys failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze file sys succ.");
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 解冻文件系统
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::UnFreeze(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecDriveLetters;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin unfreeze file sys.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_ARRAY_STRING(jReqValue, REST_PARAM_DEVICE_DISK_NAMES, vecDriveLetters);

    //参数校验
    vector<mp_string>::iterator iter;
    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); ++iter)
    {
        CHECK_FAIL_EX(CheckFileSysFreezeParam(*iter));
    }
    iRet = m_fileSys.UnFreeze(vecDriveLetters);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze file sys failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unfreeze file sys succ.");
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 批量删除驱器动号
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::DriveLetterBatchDelete(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
#ifdef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    umount_info_t driveletterinfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin batch delete drive letters.");

    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_uint32 uiCount = jValue.size();

    vector<mp_int32> vecExclude;
    for (mp_uint32 i = 0; i < uiCount; i++)
    {
        const Json::Value& jvTmp = jValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_DEV_NAME, driveletterinfo.deviceName);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_MOUNT_POINT, driveletterinfo.mountPoint);
        GET_JSON_INT32(jvTmp, REST_PARAM_DEVICE_VOL_TYPE, driveletterinfo.volumeType);

        //参数校验
        mp_string strPre("\\");
        mp_int32 lenBeg, lenEnd;
        lenBeg = lenEnd = 49;
        CHECK_FAIL_EX(CheckParamString(driveletterinfo.deviceName, lenBeg, lenEnd, strPre));

        CHECK_FAIL_EX(CheckParamInteger32(driveletterinfo.volumeType, 0, 0, vecExclude));

        lenBeg = 1;
        lenEnd = 1;
        mp_string strInclude("BCDEFGHIJKLMNOPQRSTUVWXYZ");
        mp_string strExclude;
        CHECK_FAIL_EX(CheckParamString(driveletterinfo.mountPoint, lenBeg, lenEnd, strInclude, strExclude));
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive letter input info, dev path %s, mount point %s, volume type %d.",
            driveletterinfo.deviceName.c_str(), driveletterinfo.mountPoint.c_str(), driveletterinfo.volumeType);
        
        iRet = m_fileSys.DeleteDriveLetter(driveletterinfo);
        if (MP_SUCCESS != iRet)
        {
            Json::Value jv;
            jv[REST_PARAM_DEVICE_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_DEV_NAME] = driveletterinfo.deviceName;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;     
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete drive letter failed, add to failed list, dev %s, mount point %s, errcode %d.",
                driveletterinfo.deviceName.c_str(), driveletterinfo.mountPoint.c_str(), iRet);
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End batch delete drive letters.");
    
    return iRetRst;
#else

    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupport batch delete drive letters.");

    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif
}

/*------------------------------------------------------------ 
Description  : 查询文件系统状态
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iStatus = 0;
    mp_string strdiskNames;
    vector<mp_string> vecDriveLetters;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query file system status.");
    
    CRequestURL& vrequrl= req->GetURL();
    strdiskNames = vrequrl.GetSpecialQueryParam(REST_PARAM_DEVICE_DISK_NAMES);

    //参数形式 "/mnt/tyj:/mnt/xiao:"
    CMpString::StrSplit(vecDriveLetters, strdiskNames, CHAR_COLON);

    //参数校验
    vector<mp_string>::iterator iter;
    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); ++iter)
    {
        //CodeDex误报，Dead Code
        if (!iter->empty())
        {
            CHECK_FAIL_EX(CheckFileSysFreezeParam(*iter));
        }
    }
    iStatus = m_fileSys.QueryFreezeState(vecDriveLetters);

    if (iStatus != ERROR_COMMON_FUNC_UNIMPLEMENT)
    {
    
        Json::Value& jValue= rsp->GetJsonValueRef();
        jValue[REST_PARAM_DEVICE_FREEZE_STAT] = iStatus;
    
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query file sys status succ, status %d.", iStatus);
        return MP_SUCCESS;
    }

    
    return iStatus;
}

#ifndef WIN32

/*------------------------------------------------------------ 
Description  : 创建软连接
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::LinkCreate(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    link_info_t linkInfo;
    mp_int32 iRet = MP_SUCCESS;
    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin create link.");
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_WWN, linkInfo.wwn);
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_DEV_NAME, linkInfo.slaveDevName);
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_LINKNAME, linkInfo.softLinkName);

    //参数校验
    mp_string strInclude("0123456789abcdefABCDEF");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamString(linkInfo.wwn, 1, 64, strInclude, strExclude));
    mp_string strPre("/");
    CHECK_FAIL_EX(CheckPathString(linkInfo.slaveDevName, strPre));
    CHECK_FAIL_EX(CheckPathString(linkInfo.softLinkName, strPre));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Link info, wwn %s, device name %s, link name %s.",
            linkInfo.wwn.c_str(), linkInfo.slaveDevName.c_str(), linkInfo.softLinkName.c_str());

    iRet = m_link.Create(linkInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create link failed, iRet is %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create link succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 删除软连接
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::LinkDelete(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    link_info_t linkInfo;
    mp_int32 iRet = MP_SUCCESS;
    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin delete link.");
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_WWN, linkInfo.wwn);
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_DEV_NAME, linkInfo.slaveDevName);
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_LINKNAME, linkInfo.softLinkName);

    //参数校验
    mp_string strInclude("0123456789abcdefABCDEF");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamString(linkInfo.wwn, 1, 64, strInclude, strExclude));
    mp_string strPre("/");
    CHECK_FAIL_EX(CheckPathString(linkInfo.slaveDevName, strPre));
    CHECK_FAIL_EX(CheckPathString(linkInfo.softLinkName, strPre));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Link info, wwn %s, device name %s, link name %s.",
            linkInfo.wwn.c_str(), linkInfo.slaveDevName.c_str(), linkInfo.softLinkName.c_str());

    iRet = m_link.Delete(linkInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete link failed, iRet is %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete link succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 批量创建软连接
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::LinkBatchCreate(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    link_info_t linkInfo;
    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin batch create link.");
    mp_uint32 uiCount = jValue.size();
    
    mp_string strInclude("0123456789abcdefABCDEF");
    mp_string strExclude("");
    mp_string strPre("/");
    
    for (mp_uint32 i = 0; i < uiCount; ++i)
    {
        const Json::Value& jvTmp = jValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_WWN, linkInfo.wwn);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_DEV_NAME, linkInfo.slaveDevName);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_LINKNAME, linkInfo.softLinkName);

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Link info, wwn %s, device name %s, link name %s.",
            linkInfo.wwn.c_str(), linkInfo.slaveDevName.c_str(), linkInfo.softLinkName.c_str());

        //参数校验
        CHECK_FAIL_EX(CheckParamString(linkInfo.wwn, 1, 64, strInclude, strExclude));
        CHECK_FAIL_EX(CheckPathString(linkInfo.slaveDevName, strPre));
        CHECK_FAIL_EX(CheckPathString(linkInfo.softLinkName, strPre));

        iRet = m_link.Create(linkInfo);
        if (MP_SUCCESS != iRet)
        {
            Json::Value jv;
            jv[REST_PARAM_DEVICE_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_WWN] = linkInfo.wwn;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create link %s failed, errcode %d.",
                linkInfo.softLinkName.c_str(), iRet);

        }
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Batch create link succ.");
    return iRetRst;
}

/*------------------------------------------------------------ 
Description  : 批量删除软连接
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDevicePlugin::LinkBatchDelete(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    link_info_t linkInfo;
    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin batch delete link.");
    mp_uint32 uiCount = jValue.size();

    mp_string strInclude("0123456789abcdefABCDEF");
    mp_string strExclude("");
    mp_string strPre("/");
    
    for (mp_uint32 i = 0; i < uiCount; ++i)
    {
        const Json::Value& jvTmp = jValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_WWN, linkInfo.wwn);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_DEV_NAME, linkInfo.slaveDevName);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_LINKNAME, linkInfo.softLinkName);

        //参数校验
        CHECK_FAIL_EX(CheckParamString(linkInfo.wwn, 1, 64, strInclude, strExclude));
        CHECK_FAIL_EX(CheckPathString(linkInfo.slaveDevName, strPre));
        CHECK_FAIL_EX(CheckPathString(linkInfo.softLinkName, strPre));

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Link info, wwn %s, device name %s, link name %s.",
            linkInfo.wwn.c_str(), linkInfo.slaveDevName.c_str(), linkInfo.softLinkName.c_str());

        iRet = m_link.Delete(linkInfo);
        if (MP_SUCCESS != iRet)
        {
            Json::Value jv;
            jv[REST_PARAM_DEVICE_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_WWN] = linkInfo.wwn;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete link %s failed, errcode %d.",
                linkInfo.softLinkName.c_str(), iRet);
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Batch delete link succ.");
    return iRetRst;
}
/*------------------------------------------------------------ 
Description  : 查询vg信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::LVMQueryVgs(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string>::iterator iterpv;
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    vg_info_t struVgInfo;
    CRequestURL& vrequrl= req->GetURL();
    
    struVgInfo.strVgName = vrequrl.GetSpecialQueryParam(REST_PARAM_DEVICE_VGNAME);
    struVgInfo.strVolType = vrequrl.GetSpecialQueryParam(REST_PARAM_DEVICE_VOLTYPE);

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("\\/:*?\"<>|");
    CHECK_FAIL_EX(CheckParamString(struVgInfo.strVgName, 1, 254, strInclude, strExclude));
    strInclude = mp_string("01234");
    strExclude = mp_string("");
    CHECK_FAIL_EX(CheckParamString(struVgInfo.strVolType, 1, 1, strInclude, strExclude));
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query vg(%s) info.", struVgInfo.strVgName.c_str());
    
    iRet = m_lvm.QueryVgInfo(struVgInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query vg info failed, vg is %s, voltype is %s, iRet %d.",
          struVgInfo.strVgName.c_str(), struVgInfo.strVolType.c_str(), iRet);
        
        return iRet;
    }

    jRspValue[REST_PARAM_DEVICE_VGNAME] = struVgInfo.strVgName; 
    jRspValue[REST_PARAM_DEVICE_VOLTYPE] = struVgInfo.strVolType;
    jRspValue[REST_PARAM_DEVICE_MAPINFO] = struVgInfo.strMapInfo;

    for (iterpv = struVgInfo.vecPvs.begin(); iterpv != struVgInfo.vecPvs.end(); ++iterpv)
    {
        jRspValue[REST_PARAM_DEVICE_PVS].append(*iterpv);

        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The vg(%s) pv is %s.", struVgInfo.strVgName.c_str(), iterpv->c_str());
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query vg info succ.");
    
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 导出vg
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::LVMExportVgs(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jValue = req->GetMsgBody().GetJsonValueRef();
    mp_string strVgName;
    mp_int32 iVolType;
    GET_JSON_STRING(jValue, REST_PARAM_DEVICE_VGNAME, strVgName);
    GET_JSON_INT32(jValue, REST_PARAM_DEVICE_VOLTYPE, iVolType);

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("\\/:*?\"<>|");
    CHECK_FAIL_EX(CheckParamString(strVgName, 1, 254, strInclude, strExclude));
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iVolType, 0, 4, vecExclude));
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin export vgs.");
    mp_int32 iRet = m_lvm.ExportVg(strVgName, iVolType);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Export Vg \"%s\" failed, iRet %d.", strVgName.c_str(), iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Export vgs succ.");
    }
    return iRet;
}
/*------------------------------------------------------------ 
Description  : 导入vg
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::LVMImportVgs(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    mp_string strPriPvName;
    mp_string strVgName;
    mp_int32 iVolType;
    mp_string strWWN;
    mp_string strMapInfo;
    vector<mp_string> vecPriPvName;
    vector<mp_string> vecWWN;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin import vgs.");
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_VGNAME, strVgName);
    GET_JSON_INT32(jReqValue, REST_PARAM_DEVICE_VOLTYPE, iVolType);
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_MAPINFO, strMapInfo);

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("\\/:*?\"<>|");
    CHECK_FAIL_EX(CheckParamString(strVgName, 1, 254, strInclude, strExclude));
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iVolType, 0, 4, vecExclude));
    mp_string strPre("/");
    
    vector<Json::Value> vecValue;
    GET_JSON_ARRAY_JSON(jReqValue, REST_PARAM_DEVICE_PVINFO, vecValue);
    for (vector<Json::Value>::iterator it = vecValue.begin(); it != vecValue.end(); it++)
    {
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_PVNAME, strPriPvName);
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_WWN, strWWN);

        //参数校验
        strInclude = mp_string("0123456789ABCDEFabcdef");
        strExclude = mp_string("");
        CHECK_FAIL_EX(CheckParamString(strWWN, 1, 64, strInclude, strExclude));
        vecPriPvName.push_back(strPriPvName);
        vecWWN.push_back(strWWN);
    }
    
    mp_int32 iRet = m_lvm.ImportVg(vecPriPvName, strVgName, iVolType, strMapInfo, vecWWN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Import Vg \"%s\" failed, iRet %d.", strVgName.c_str(), iRet);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Import vgs succ.");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :激活vg
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::LVMActivateVgs(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_int32 iVolType;
    mp_int32 iRecoverType;
    mp_string strVgActiveMode;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin active vgs.");
    GET_JSON_INT32(jReqValue, REST_PARAM_DEVICE_VOLTYPE, iVolType);
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_ACTIVEMODE, strVgActiveMode);
    GET_JSON_INT32(jReqValue, REST_PARAM_DEVICE_RECOVERTYPE, iRecoverType);

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iVolType, 0, 4, vecExclude));
    CHECK_FAIL_EX(CheckParamInteger32(iRecoverType, 0, 3, vecExclude));
    if(!strVgActiveMode.empty())
    {
        strInclude = mp_string(" -ae");
        strExclude = mp_string("");
        CHECK_FAIL_EX(CheckParamString(strVgActiveMode, 1, 254, strInclude, strExclude));         
    }

    strInclude = mp_string("");
    strExclude = mp_string("\\/:*?\"<>|");
    vector<mp_string> vecValue;
    GET_JSON_ARRAY_STRING(jReqValue, REST_PARAM_DEVICE_VGNAMES, vecValue);
    for (vector<mp_string>::iterator it = vecValue.begin(); it != vecValue.end(); it++)
    {
        //参数校验
        CHECK_FAIL_EX(CheckParamString(*it, 1, 254, strInclude, strExclude));
        
        iRet = m_lvm.ActivateVg(*it, iVolType, strVgActiveMode, iRecoverType);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Activate Vg \"%s\" failed, Vol Type is %d, iRet %d.",
                it->c_str(), iVolType, iRet);
            Json::Value jv;
            jv[REST_PARAM_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_VGNAME] = *it;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Active vgs succ.");
    return iRetRst;
}
/*------------------------------------------------------------ 
Description  : 去激活vg
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::LVMDeactivateVgs(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_int32 iVolType;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin deactive vgs.");
    GET_JSON_INT32(jReqValue, REST_PARAM_DEVICE_VOLTYPE, iVolType);

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("\\/:*?\"<>|");
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iVolType, 0, 4, vecExclude));
    
    vector<mp_string> vecValue;
    GET_JSON_ARRAY_STRING(jReqValue, REST_PARAM_DEVICE_VGNAMES, vecValue);
    for (vector<mp_string>::iterator it = vecValue.begin(); it != vecValue.end(); it++)
    {
        //参数校验
        CHECK_FAIL_EX(CheckParamString(*it, 1, 254, strInclude, strExclude));
            
        iRet = m_lvm.DeActivateVg(*it, iVolType);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Deactivate Vg \"%s\" failed, Vol Type is %d, iRet %d.",
                  it->c_str(), iVolType, iRet);
            Json::Value jv;
            jv[REST_PARAM_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_VGNAME] = *it;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Deactive vgs succ.");
    return iRetRst;
}

/*------------------------------------------------------------ 
Description  : 查询lv信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::LVMQueryLVs(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询lv信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::LVMScanDisks(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to scan vxvm disks.");
    iRet = m_lvm.ScanDisks_VXVM();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Scan vxvm disks failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to scan vxvm disks.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 创建udev规则
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::UDEVCreateRules(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    mp_string strWWN;
    mp_string strUdevRule;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to create udev rule.");

    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_WWN, strWWN);
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_UDEVRULE, strUdevRule);

    //参数校验
    mp_string strExclude;
    mp_string strInclude("0123456789abcdefABCDEF");
    mp_int32 lenEnd = 64;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(strWWN, lenBeg, lenEnd, strInclude, strExclude));
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get udev rule  info, wwn is [%s], udev rule is [%s].",
              strWWN.c_str(), strUdevRule.c_str());

    iRet = m_udev.Create(strUdevRule, strWWN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create udev rule failed, udev rule is %s, wwn is %s, iRet %d.",
              strUdevRule.c_str(), strWWN.c_str(), iRet);
        return iRet;
    }

    // 重新加载udev规则文件,如果失败继续进行
    iRet = m_udev.ReloadRules();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Reload udev rules failed, but try to create udev device.");
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create udev rule succ.");
    return iRet;
}
/*------------------------------------------------------------ 
Description  : 删除udev规则
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDevicePlugin::UDEVDeleteRules(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    mp_string strWWN;
    mp_string strUdevRule;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to delete udev rule.");

    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_WWN, strWWN);
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_UDEVRULE, strUdevRule);

    //参数校验
    mp_string strExclude;
    mp_string strInclude("0123456789abcdefABCDEF");
    mp_int32 lenEnd = 64;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(strWWN, lenBeg, lenEnd, strInclude, strExclude));

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get udev rule  info, wwn is [%s], udev rule is [%s].",
              strWWN.c_str(), strUdevRule.c_str());

    iRet = m_udev.Delete(strUdevRule, strWWN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create udev rule failed, udev rule is %s, wwn is %s, iRet %d.",
              strUdevRule.c_str(), strWWN.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete udev rule succ.");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :批量 创建udev规则
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::UDEVBatchCreateRules(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_string strWWN;
    mp_string strUdevRule;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to batch create udev rules.");
    CHECK_JSON_ARRAY(jReqValue);
    mp_uint32 uiCount = jReqValue.size();
    for (mp_uint32 i = 0; i < uiCount; i++)
    {
        const Json::Value& jvTmp = jReqValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_WWN, strWWN);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_UDEVRULE, strUdevRule);

        //参数校验
        mp_string strExclude;
        mp_string strInclude("0123456789abcdefABCDEF");
        mp_int32 lenEnd = 64;
        mp_int32 lenBeg = 1;
        CHECK_FAIL_EX(CheckParamString(strWWN, lenBeg, lenEnd, strInclude, strExclude));

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get udev rule  info, wwn is [%s], udev rule is [%s].",
              strWWN.c_str(), strUdevRule.c_str());

        iRet = m_udev.Create(strUdevRule, strWWN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create udev rule failed, udev rule is %s, wwn is %s, iRet %d.",
                  strUdevRule.c_str(), strWWN.c_str(), iRet);
            Json::Value jv;
            jv[REST_PARAM_DEVICE_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_UDEVRULE] = strUdevRule;
            jv[REST_PARAM_DEVICE_WWN] = strWWN;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }

    // 重新加载udev规则文件,如果失败继续进行
    iRet = m_udev.ReloadRules();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Reload udev rules failed, but try to create udev device.");
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Batch create udev rule succ.");
    return iRetRst;
}
/*------------------------------------------------------------ 
Description  :批量 删除udev规则
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::UDEVBatchDeleteRules(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_string strWWN;
    mp_string strUdevRule;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to batch delete udev rules.");
    CHECK_JSON_ARRAY(jReqValue);
    mp_uint32 uiCount = jReqValue.size();
    for (mp_uint32 i = 0; i < uiCount; i++)
    {
        const Json::Value& jvTmp = jReqValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_WWN, strWWN);
        GET_JSON_STRING(jvTmp, REST_PARAM_DEVICE_UDEVRULE, strUdevRule);

       //参数校验
        mp_string strExclude;
        mp_string strInclude("0123456789abcdefABCDEF");
        mp_int32 lenEnd = 64;
        mp_int32 lenBeg = 1;
        CHECK_FAIL_EX(CheckParamString(strWWN, lenBeg, lenEnd, strInclude, strExclude));

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get udev rule  info, wwn is [%s], udev rule is [%s].",
              strWWN.c_str(), strUdevRule.c_str());

        iRet = m_udev.Delete(strUdevRule, strWWN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create udev rule failed, udev rule is %s, wwn is %s, iRet %d.",
                  strUdevRule.c_str(), strWWN.c_str(), iRet);
            Json::Value jv;
            jv[REST_PARAM_DEVICE_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_UDEVRULE] = strUdevRule;
            jv[REST_PARAM_DEVICE_WWN] = strWWN;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Batch delete udev rule succ.");
    return iRetRst;
}
/*------------------------------------------------------------ 
Description  :扫描asmlib磁盘
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::ScanASMLib(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin scan asmlib disks.");
    iRet = m_deviceASM.AsmLibScan();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Scan asmlib disks failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End scan asmlib disks.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :创建裸设备
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::RawDeviceCreate(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    raw_info_t stRawDevice;
    mp_int32 iRet;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin create raw device.");
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_WWN, stRawDevice.strLunWWN);
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_DEV_NAME, stRawDevice.strDevName);
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_DEV_PATH, stRawDevice.strRawDevPath);

    //参数校验
    mp_string strExclude;
    mp_string strInclude("0123456789abcdefABCDEF");
    mp_string strPre = "/";
    
    CHECK_FAIL_EX(CheckParamString(stRawDevice.strLunWWN, 1, 64, strInclude, strExclude));
    CHECK_FAIL_EX(CheckPathString(stRawDevice.strDevName,strPre));
    CHECK_FAIL_EX(CheckPathString(stRawDevice.strRawDevPath,strPre));
    
    iRet = m_raw.Create(stRawDevice);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create raw device failed, iRet = %d.", iRet);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create raw device succ.");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :删除裸设备
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::RawDeviceDelete(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    raw_info_t stRawDevice;
    mp_int32 iRet;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin delete raw device.");
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_WWN, stRawDevice.strLunWWN);
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_DEV_NAME, stRawDevice.strDevName);
    GET_JSON_STRING(jReqValue, REST_PARAM_DEVICE_DEV_PATH, stRawDevice.strRawDevPath);

    //参数校验
    mp_string strExclude;
    mp_string strInclude("0123456789abcdefABCDEF");
    mp_string strPre = "/";
    
    CHECK_FAIL_EX(CheckParamString(stRawDevice.strLunWWN, 1, 64, strInclude, strExclude));
    CHECK_FAIL_EX(CheckPathString(stRawDevice.strDevName,strPre));
    CHECK_FAIL_EX(CheckPathString(stRawDevice.strRawDevPath,strPre));

    iRet = m_raw.Delete(stRawDevice);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete raw device failed, iRet = %d.", iRet);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete raw device succ.");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :批量创建裸设备
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::RawDeviceBatchCreate(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    vector<Json::Value> vecValue;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin batch create raw device.");
    GET_ARRAY_JSON(jReqValue, vecValue);
    for (vector<Json::Value>::iterator it = vecValue.begin(); it != vecValue.end(); it++)
    {
        raw_info_t stRawDevice;
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_WWN, stRawDevice.strLunWWN);
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_DEV_NAME, stRawDevice.strDevName);
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_DEV_PATH, stRawDevice.strRawDevPath);

        //参数校验
        mp_string strExclude;
        mp_string strInclude("0123456789abcdefABCDEF");
        mp_string strPre = "/";
    
        CHECK_FAIL_EX(CheckParamString(stRawDevice.strLunWWN, 1, 64, strInclude, strExclude));
        CHECK_FAIL_EX(CheckPathString(stRawDevice.strDevName,strPre));
        CHECK_FAIL_EX(CheckPathString(stRawDevice.strRawDevPath,strPre));

        iRet = m_raw.Create(stRawDevice);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Batch create raw device failed, wwn is \"%s\", device name is \"%s\", \
                device path is \"%s\", iRet %d.", stRawDevice.strLunWWN.c_str(), stRawDevice.strDevName.c_str(),
                stRawDevice.strRawDevPath.c_str(), iRet);
            Json::Value jv;
            jv[REST_PARAM_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_WWN] = stRawDevice.strLunWWN;
            jv[REST_PARAM_DEVICE_DEV_NAME] = stRawDevice.strDevName;
            jv[REST_PARAM_DEVICE_DEV_PATH] = stRawDevice.strRawDevPath;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End batch create raw device.");
    return iRetRst;
}
/*------------------------------------------------------------ 
Description  :批量删除裸设备
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::RawDeviceBatchDelete(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    vector<Json::Value> vecValue;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin batch delete raw device.");
    GET_ARRAY_JSON(jReqValue, vecValue);
    for (vector<Json::Value>::iterator it = vecValue.begin(); it != vecValue.end(); it++)
    {
        raw_info_t stRawDevice;
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_WWN, stRawDevice.strLunWWN);
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_DEV_NAME, stRawDevice.strDevName);
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_DEV_PATH, stRawDevice.strRawDevPath);

        //参数校验
        mp_string strExclude;
        mp_string strInclude("0123456789abcdefABCDEF");
        mp_string strPre = "/";
    
        CHECK_FAIL_EX(CheckParamString(stRawDevice.strLunWWN, 1, 64, strInclude, strExclude));
        CHECK_FAIL_EX(CheckPathString(stRawDevice.strDevName,strPre));
        CHECK_FAIL_EX(CheckPathString(stRawDevice.strRawDevPath,strPre));

        iRet = m_raw.Delete(stRawDevice);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Batch delete raw device failed, wwn is \"%s\", device name is \"%s\", \
                device path is \"%s\", iRet %d.", stRawDevice.strLunWWN.c_str(), stRawDevice.strDevName.c_str(),
                stRawDevice.strRawDevPath.c_str(), iRet);
            Json::Value jv;
            jv[REST_PARAM_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_WWN] = stRawDevice.strLunWWN;
            jv[REST_PARAM_DEVICE_DEV_NAME] = stRawDevice.strDevName;
            jv[REST_PARAM_DEVICE_DEV_PATH] = stRawDevice.strRawDevPath;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End batch delete raw device.");
    return iRetRst;
}
/*------------------------------------------------------------ 
Description  :设置设备权限
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDevicePlugin::Permission(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;
    vector<Json::Value> vecValue;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin set permission.");
    GET_ARRAY_JSON(jReqValue, vecValue);
    for (vector<Json::Value>::iterator it = vecValue.begin(); it != vecValue.end(); it++)
    {
        permission_info_t stPermission;
        GET_JSON_STRING((*it), REST_PARAM_DEVICE_DEV_NAME, stPermission.strDevName);
        GET_JSON_STRING_OPTION((*it), REST_PARAM_DEVICE_DEV_USERNAME, stPermission.strUserName);
        GET_JSON_STRING_OPTION((*it), REST_PARAM_DEVICE_DEV_PRIMODE, stPermission.strMod);

        //参数校验
        mp_string strExclude;
        mp_string strInclude("");
        mp_string strPre = "/";
    
        CHECK_FAIL_EX(CheckPathString(stPermission.strDevName,strPre));
        CHECK_FAIL_EX(CheckParamString(stPermission.strUserName,1,8, strInclude, strExclude));
        strInclude = "01234567";
        CHECK_FAIL_EX(CheckParamString(stPermission.strMod,3, 3, strInclude, strExclude));
        
        iRet = m_permission.Set(stPermission);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set permission chown failed, device name is \"%s\", \
                user name is \"%s\", iRet %d.", stPermission.strDevName.c_str(),
                stPermission.strUserName.c_str(), iRet);
            Json::Value jv;
            jv[REST_PARAM_ERROR_CODE] = iRet;
            jv[REST_PARAM_DEVICE_DEV_NAME] = stPermission.strDevName;
            jv[REST_PARAM_DEVICE_DEV_USERNAME] = stPermission.strUserName;
            jv[REST_PARAM_DEVICE_DEV_PRIMODE] = stPermission.strMod;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End set permission.");
    return iRetRst;
}

#endif //WIN32

