/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef WIN32
#include "device/Permission.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "array/Array.h"

#include <pwd.h>
#include <grp.h>

/*------------------------------------------------------------
Function Name: Set
Description  : 设置权限
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CPermission::Set(permission_info_t& permissionInfo)
{
    mp_int32 iRet;
    if (!permissionInfo.strMod.empty())
    {
        iRet = Chmod(permissionInfo);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_PERMISSION_SET_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "chmod failed, mode: \"%s\", devName: \"%s\".",
                permissionInfo.strMod.c_str(), permissionInfo.strDevName.c_str());
            return iRet;
        }
    }
    if (!permissionInfo.strUserName.empty())
    {
        iRet = Chown(permissionInfo);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_PERMISSION_SET_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "chown failed, user name: \"%s\", devName: \"%s\".",
                permissionInfo.strUserName.c_str(), permissionInfo.strDevName.c_str());
            return iRet;
        }
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Chown
Description  : 通过chown设置所属权限
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CPermission::Chown(permission_info_t& permissionInfo)
{
    LOGGUARD("");
    //获取用户所在组信息
    mp_string strGroupName;
    mp_int32 iRet = GetGroupName_R(permissionInfo.strUserName, strGroupName);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get group name of \"%s\" failed.", permissionInfo.strUserName.c_str());
        return iRet;
    }
    mp_string strParam = permissionInfo.strUserName + ":" + strGroupName + " " + permissionInfo.strDevName;
    ROOT_EXEC(ROOT_COMMAND_CHOWN, strParam, NULL);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Chmod
Description  : 通过chmod设置读写权限
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CPermission::Chmod(permission_info_t& permissionInfo)
{
    LOGGUARD("");
    mp_string strParam = permissionInfo.strMod + " " + permissionInfo.strDevName;
    ROOT_EXEC(ROOT_COMMAND_CHMOD, strParam, NULL);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetGroupName_R
Description  : 获取所属组信息，线程安全
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CPermission::GetGroupName_R(mp_string& strUserName, mp_string& strGroupName)
{
    struct passwd* pPwd = NULL;
    struct group* pGrp = NULL;
    CThreadAutoLock cLock(&m_Mutex);
    pPwd = getpwnam(strUserName.c_str());
    if (NULL == pPwd)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get pw failed.");
        return MP_FAILED;
    }

    pGrp = getgrgid(pPwd->pw_gid);
    if (NULL == pGrp)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get gid failed.");
        return MP_FAILED;
    }

    strGroupName = pGrp->gr_name;
    return MP_SUCCESS;
}

#endif //WIn32

