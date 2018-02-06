/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_TASK_DISPATCH_WORKER_H_
#define _AGENT_TASK_DISPATCH_WORKER_H_

#include "common/Types.h"
#include "common/Thread.h"
#include "agent/Communication.h"
#include "agent/TaskWorker.h"

#define MAX_PUSHED_WORKERS_CNT 2147483640   // int -> +2147483647  防止溢出

class CTaskDispatchWorker
{
private:
    thread_id_t m_dispatchTid;
    mp_int32 m_iWorkerCount;
    mp_int32 m_lastPushedWorker;
    volatile mp_bool m_bNeedExit;                     //线程退出标识
    volatile mp_int32 m_iThreadStatus;                //接收线程状态
    //m_pWorkers和m_pVssWorker由外部分配和释放
    CTaskWorker** m_pWorkers;
#ifdef WIN32
    CTaskWorker* m_pVssWorker;
#endif
    
public:
    CTaskDispatchWorker();
    ~CTaskDispatchWorker();

    thread_id_t& GetThreadId();
    mp_bool NeedExit();
    mp_void SetRecvThreadStatus(mp_int32 iThreadStatus);
#ifdef WIN32
    mp_int32 Init(CTaskWorker** pTaskWorkers, mp_int32 iCount, CTaskWorker* pVssWorker);
#else
    mp_int32 Init(CTaskWorker** pTaskWorkers, mp_int32 iCount);
#endif
    mp_void Exit();
 
private:
    mp_void PushMsgToWorker(message_pair_t& msg);
#ifdef WIN32
    mp_bool IsVSSRequst(message_pair_t& msg);
    static DWORD WINAPI DispacthProc(void* pThis);
#else
    static void* DispacthProc(void* pThis);
#endif
};

#endif //_AGENT_TASK_DISPATCH_WORKER_H_

