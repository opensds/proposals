/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

******************************************************************************
File Name     : Authentication.h
Version       : Initial Draft
Author        : 
Created       : 2015/01/19
Last Modified :
Description   : 鉴权类定义
History       :
1.Date        :
Author      :
Modification:
******************************************************************************/
#ifndef _AGENT_AUTHENTICATION_H_
#define _AGENT_AUTHENTICATION_H_

#include "common/Types.h"
#include "common/Thread.h"
#include <vector>

#define MAX_TRY_TIME 3  //连续失败次数 3次
#define LOCKED_TIME 15 * 60   //在15分钟内连续失败3次，锁定15分钟
#define CONTINUOUS_FAILURE_TIME   15 * 60   //两次失败登录登录如果超过30分钟，则不计算连续登录失败次数

typedef struct st_locked_client_info_st
{
    mp_bool isLocked;        //是否锁定，true 锁定，false 未锁定
    mp_int64 failedTimes;    //连续登录失败次数
    mp_uint64 lastFailedTime;//上一次鉴权失败时间
    mp_uint64 lockedTime;      //锁定时间，非锁定情况下，其值是0
    mp_string strClientIP;   //客户端ip地址
}locked_client_info;

class CAuthentication
{
public:
    static CAuthentication& GetInstance()
    {
        return m_instance;
    }
    mp_int32 Init();
    mp_int32 Auth(mp_string& strClientIP, mp_string& strUsr, mp_string& strPw);
    ~CAuthentication()
    {
        CMpThread::DestroyLock(&m_lockedIPListMutex);
    }
private:
    CAuthentication()
    {
        CMpThread::InitLock(&m_lockedIPListMutex);
    }
    mp_bool IsLocked(mp_string& strClientIP);
    mp_bool Check(mp_string& strUsr, mp_string& strPw);
    mp_void Lock(mp_string& strClientIP);
    mp_void Unlock(mp_string& strClientIP);

private:
    mp_string m_strUsr;  //保存的用户名，sha256
    mp_string m_strPwd;  //保存的pw，sha256
    static CAuthentication m_instance;   //单例对象
    vector<locked_client_info> m_lockedIPList;    //锁定http客户端ip地址列表
    thread_lock_t m_lockedIPListMutex;  //m_lockedIPList访问互斥锁
};
#endif
