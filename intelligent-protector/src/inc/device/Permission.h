/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_PERMISSION_H__
#define __AGENT_PERMISSION_H__

#ifndef WIN32
#include "common/Types.h"
#include "common/Thread.h"

typedef struct tag_permission_info
{
    mp_string strDevName;  //设备名称
    mp_string strUserName;  //设备的用户名
    mp_string strMod;      //设备的访问权限
}permission_info_t;


class CPermission
{
public:
    CPermission()
    {
        CMpThread::InitLock(&m_Mutex);
    }
    ~CPermission()
    {
        CMpThread::DestroyLock(&m_Mutex);
    }
    mp_int32 Set(permission_info_t& permissionInfo);

private:
    mp_int32 Chown(permission_info_t& permissionInfo);
    mp_int32 Chmod(permission_info_t& permissionInfo);
    mp_int32 GetGroupName_R(mp_string& strUserName, mp_string& strGroupName);

private:
    thread_lock_t m_Mutex;
};

#endif //WIN32
#endif //

