/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_UNIQUEID_H__
#define __AGENT_UNIQUEID_H__

#include "common/Types.h"
#include "common/Defines.h"
#include "common/Thread.h"


//临时文件id相关
const mp_int32 MAX_UNIQUE_ID = 1000000;
class AGENT_API CUniqueID
{
public:
    static CUniqueID& GetInstance()
    {
        return m_instance;
    }
    ~CUniqueID()
    {
        CMpThread::DestroyLock(&m_uniqueIDMutex);
    }
    mp_void Init()
    {
    }
    mp_string GetString();
    mp_int32 GetInt();
private:
    CUniqueID()
    {
        m_iUniqueID = 0;
        CMpThread::InitLock(&m_uniqueIDMutex);
    }
    static CUniqueID m_instance;   //单例对象
    mp_int32 m_iUniqueID;
    thread_lock_t m_uniqueIDMutex;
};

#endif //__AGENT_UNIQUEID_H__

