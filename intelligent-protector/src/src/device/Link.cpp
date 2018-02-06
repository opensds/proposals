/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/Link.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "array/Array.h"

#ifndef WIN32
CLink::CLink()
{
}

CLink::~CLink()
{
}

/*------------------------------------------------------------ 
Description  : 创建软连接
Input        : linkInfo -- 创建软连接所需参数
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLink::Create(link_info_t& linkInfo)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin creat link operator, link name is %s.", linkInfo.softLinkName.c_str());

    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    mp_string strUsedDevByLink;
#ifdef SOLARIS
    mp_string strRawDiskName;
    iRet = CDisk::GetSolarisRawDiskName(linkInfo.slaveDevName, strRawDiskName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get raw disk name of disk(%s) failed, ret %d.",
            linkInfo.slaveDevName.c_str(), iRet);

        return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
    }
	linkInfo.slaveDevName = strRawDiskName;
#endif
    //判断slaveDevName是否存在
    if (!CMpFile::FileExist(linkInfo.slaveDevName.c_str()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The dev %s is not exist, return err %d", 
            linkInfo.slaveDevName.c_str(), ERROR_COMMON_DEVICE_NOT_EXIST);

        return ERROR_COMMON_DEVICE_NOT_EXIST;
    }
    
    //server能保证软连接参数正确
    //链接文件存在且为目录，则直接返回错误
    if (CMpFile::DirExist(linkInfo.softLinkName.c_str()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The dev used by link %s is directory, return err %d", 
            linkInfo.softLinkName.c_str(), ERROR_DEVICE_LINK_USED_BY_OTHER_DEV);
        
        return ERROR_DEVICE_LINK_USED_BY_OTHER_DEV;
    }

    if (CMpFile::FileExist(linkInfo.softLinkName.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The link name %s is exist.", linkInfo.softLinkName.c_str());
        
        iRet =  GetDeviceUsedByLink(linkInfo.softLinkName, strUsedDevByLink);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_GET_DEV_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get block device failed, soft link name %s.",
                linkInfo.softLinkName.c_str());

            return iRet;
        }

        if (strUsedDevByLink != linkInfo.slaveDevName)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Link file used wrong dev. expected dev %s, curr dev %s.",
                linkInfo.slaveDevName.c_str(), strUsedDevByLink.c_str());

            return ERROR_DEVICE_LINK_USED_BY_OTHER_DEV;
        }
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Link file used the right dev.");

            return MP_SUCCESS;
        }
    }

    strParam = linkInfo.slaveDevName + " " + linkInfo.softLinkName;

    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_LN, strParam, NULL, ERROR_DEVICE_LINK_CREATE_FAILED);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Exec creat link operator succ, link name is %s.",
        linkInfo.softLinkName.c_str());
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 删除软连接
Input        : linkInfo -- 删除软连接所需参数
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLink::Delete(link_info_t& linkInfo)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin delete link %s.", linkInfo.softLinkName.c_str());

    if (!CMpFile::FileExist(linkInfo.softLinkName.c_str()))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The soft link %s dose not exist.",
            linkInfo.softLinkName.c_str());

        return MP_SUCCESS;
    }

    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_RM, linkInfo.softLinkName, NULL, ERROR_DEVICE_LINK_DELETE_FAILED);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The soft link %s delete succ.",
        linkInfo.softLinkName.c_str());

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 查询软连接的设备名
Input        : strLinkFileName -- 软连接名称
Output       : strUsedDeviceName -- 设备名称
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CLink::GetDeviceUsedByLink(mp_string & strLinkFileName, mp_string & strUsedDeviceName)
{
	//CodeDex误报，UNUSED_VALUE
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_char * pszBuff = NULL;
    mp_size sPathMax = 1024;
	//CodeDex误报， String Termination Error申请内存时已经+1
    pszBuff = (mp_char*)malloc(sPathMax + 1);
    if (NULL == pszBuff)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Malloc memory failed, count %d.",
            sPathMax + 1);

        return MP_FAILED;
    }
	
    iRet = memset_s(pszBuff, sPathMax + 1, 0, sPathMax + 1);
    if (EOK != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, iRet %d.", iRet);
        free(pszBuff);
        pszBuff = NULL;
        return MP_FAILED;
    }
	//CodeDex误报，Often Misused:File System
    iRet = readlink(strLinkFileName.c_str(), pszBuff, sPathMax);
    if (iRet < 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read link file %s failed， errcode %d.",
            strLinkFileName.c_str(), iRet);

        free(pszBuff);
        pszBuff = NULL;
        return iRet;
    }

    strUsedDeviceName = pszBuff;
    free(pszBuff);
    pszBuff = NULL;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get device for link file succ, link file %s, device name %s.",
        strLinkFileName.c_str(), strUsedDeviceName.c_str());

    return MP_SUCCESS;
}

#endif //WIN32

