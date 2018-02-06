/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_TASK_POOL_H_
#define _AGENT_TASK_POOL_H_

#include <vector>
#include "pluginfx/IPlugin.h"
#include "pluginfx/PluginCfgParse.h"
#include "pluginfx/PluginManager.h"
#include "agent/TaskWorker.h"
#include "agent/TaskVssWorker.h"
#include "agent/TaskDispatchWorker.h"
#include "common/Types.h"
#include "common/Thread.h"

//工作线程总数
#define MAX_WORKER_NUM 10

/*
typedef struct tag_worker_thread
{
    thread_id_t id;
    mp_bool closed;
}worker_thread_t;
*/

class CTaskPool:public IPluginCallback
{
private:
    CTaskWorker* m_pWorkers[MAX_WORKER_NUM];
    CTaskDispatchWorker* m_pDispatchWorker;
    //mp_int32 m_iStatus;
    CPluginCfgParse* m_plugCfgParse;
    //IPluginManager* m_plugMgr;
    CPluginManager* m_plugMgr;
#ifdef WIN32
    CTaskVssWorker* m_pVssWorker;
#endif
    
public:
    CTaskPool();
    ~CTaskPool();
 
    mp_int32 Init();
    //mp_int32 Run();
    mp_void Exit();

    //IPluginCallback 虚方法实现
    virtual mp_bool CanUnload(IPlugin* pOldPlg);
    virtual mp_void OnUpgraded(IPlugin* pOldPlg, IPlugin* pNewPlg);
    virtual mp_void SetOptions(IPlugin* plg);
    virtual mp_char* GetReleaseVersion(const mp_char* pszLib, mp_char* pszVer, mp_size sz);

private: 
    mp_int32 CreateWorkers();
    mp_int32 CreateProtectWorker(); 
    mp_int32 CreateDispatchWorker();
    mp_int32 CreatePlgConfParse();
    mp_int32 CreatePlugMgr();
#ifdef WIN32 
    mp_int32 CreateVssWorker();
#endif
};

#endif //_AGENT_TASK_POOL_H_

