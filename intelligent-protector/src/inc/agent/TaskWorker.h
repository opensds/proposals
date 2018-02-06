/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_TASK_WORKER_H_
#define _AGENT_TASK_WORKER_H_

#include "pluginfx/IPlugin.h"
#include "pluginfx/PluginCfgParse.h"
#include "pluginfx/PluginManager.h"
#include "plugins/ServicePlugin.h"
#include "common/Types.h"
#include "common/Thread.h"
#include "agent/Communication.h" 

enum WORK_STATUS
{
    IDLE = 0,
    RUNNING = 1
};

class CTaskWorker
{
private:
    thread_lock_t m_tPlgLock;
    thread_lock_t m_tReqLock;
    thread_id_t m_threadId;
    mp_bool m_bClosed;
    vector<message_pair_t> m_vecReqQueue;
    mp_uint64 m_SCN;
    const mp_char* m_plgName;
    const mp_char* m_plgVersion;
    CPluginCfgParse* m_plgCfgParse;
    CPluginManager* m_plgMgr;
	volatile mp_bool m_bProcReq;
    volatile mp_bool m_bNeedExit;                     //线程退出标识
    volatile mp_int32 m_iThreadStatus;                //接收线程状态

    
public:
    CTaskWorker();
    virtual ~CTaskWorker();
    
    virtual mp_int32 ReqProc();
    mp_bool NeedExit();
    mp_void SetThreadStatus(mp_int32 iThreadStatus);
	mp_bool GetThreadProcReqStatus();
    mp_int32 PopRequest(message_pair_t& msg);
    mp_void PushRequest(message_pair_t& msg);
    mp_int32 Init(CPluginCfgParse* pPlgCfgParse, CPluginManager* pPlgMgr);
    mp_void Exit();
    mp_bool CanUnloadPlugin(mp_uint64 newSCN, const char* plgName);
 
private:
    CServicePlugin* GetPlugin(mp_char* pszService);
    mp_void SetPlugin(CServicePlugin* pPlug);
#ifdef WIN32
    static DWORD WINAPI WorkProc(void* pThis);
#else
    static void* WorkProc(void* pThis);
#endif
};

#endif //_AGENT_TASK_WORKER_H_

