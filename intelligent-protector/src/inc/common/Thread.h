/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_THREAD_H__
#define __AGENT_THREAD_H__

#include <stdlib.h>
#ifdef WIN32
//#include <windows.h>
#else
#include <pthread.h>
#endif
#include "common/Types.h"
#include "common/Defines.h"

#ifdef WIN32
    typedef CRITICAL_SECTION thread_lock_t;
    typedef mp_uint32        thread_local_var_t;
    typedef DWORD            thread_os_id_t;   
#else
    typedef pthread_mutex_t  thread_lock_t;
    typedef pthread_key_t    thread_local_var_t;
    typedef pthread_t        thread_os_id_t;
#endif

#ifdef WIN32
#define CM_INFINITE INFINITE
#else
#define CM_INFINITE 0xFFFFFFFF
#endif

typedef struct tag_thread_cond
{
#ifdef WIN32
    HANDLE              sem;
    mp_uint32           count;
#else
    pthread_mutex_t     lock;
    pthread_cond_t      cond;
#endif
} thread_cond_t;

typedef struct tag_thread_id
{
    thread_os_id_t  os_id;
#ifdef WIN32
    HANDLE          handle;
#endif
}thread_id_t;

#ifdef WIN32
typedef LPTHREAD_START_ROUTINE thread_proc_t;
#else
typedef mp_void* (*thread_proc_t)(mp_void* arg);
#endif

#define DEFAULE_THREAD_STACK_SIZE      (512 * 1024)
#define THREAD_LOCK_BUSY               -2

enum THREAD_STATUS_E
{
    THREAD_STATUS_IDLE = 0,
    THREAD_STATUS_RUNNING,
    THREAD_STATUS_EXITED
};

class AGENT_API CMpThread
{
public:
    static mp_int32 Create(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize = DEFAULE_THREAD_STACK_SIZE);
    static mp_int32 WaitForEnd(thread_id_t* id, mp_void** retValue);
    static mp_int32 InitLock(thread_lock_t* plock);
    static mp_int32 DestroyLock(thread_lock_t* plock);
    static mp_int32 InitCond(thread_cond_t* pcond);
    static mp_int32 DestroyCond(thread_cond_t* pcond);
    static mp_int32 Lock(thread_lock_t* plock);
    static mp_int32 TryLock(thread_lock_t* plock);
    static mp_int32 TimedLock(thread_lock_t* plock, mp_uint32 uiTimeoutInMillisecs);
    static mp_int32 Unlock(thread_lock_t* plock);
    static thread_os_id_t GetThreadId();

private:
    static mp_uint32 CalcRetryInterval(mp_uint32 uiTimeoutInMillisecs);
    static mp_bool IsTimeout(mp_uint64 ulStartTime, mp_uint32 uiTimeoutInMillisecs);
};

class AGENT_API CThreadAutoLock
{
public:
    CThreadAutoLock(thread_lock_t* pLock);
    ~CThreadAutoLock();

private:
    thread_lock_t* m_plock;
};

#endif //__AGENT_THREAD_H__

