/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "agent/TaskWorker.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"

CTaskWorker::CTaskWorker()
{
    m_plgCfgParse = NULL;
    m_bClosed = MP_FALSE;
    m_SCN = 0;
    m_plgName = 0;
    m_plgVersion = 0;
    m_plgCfgParse = 0;
    m_plgMgr = 0;
    (mp_void)memset_s(&m_threadId, sizeof(m_threadId), 0, sizeof(m_threadId));
    //Coverity&Fortify误报:UNINIT_CTOR
    //Coveirty&Fortify不认识公司安全函数memset_s，提示m_threadId.os_id未初始化
    CMpThread::InitLock(&m_tPlgLock);
    CMpThread::InitLock(&m_tReqLock);
    m_iThreadStatus = THREAD_STATUS_IDLE;
    m_bNeedExit = MP_FALSE;
    m_bProcReq = MP_FALSE;
}

CTaskWorker::~CTaskWorker()
{
    CMpThread::DestroyLock(&m_tPlgLock);
    CMpThread::DestroyLock(&m_tReqLock);
}

/*------------------------------------------------------------
Description  : 初始化task worker线程
Input        : pPlgCfgParse -- 插件配置文件解析对象指针
               pPlgMgr -- 插件管理对象指针
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskWorker::Init(CPluginCfgParse* pPlgCfgParse, CPluginManager* pPlgMgr)
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin init task worker.");

    if (NULL == pPlgCfgParse || NULL == pPlgMgr)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is null.");
        return MP_FAILED;
    }

    m_plgCfgParse = pPlgCfgParse;
    m_plgMgr =pPlgMgr;
    m_SCN = m_plgMgr->GetSCN();

    iRet = CMpThread::Create(&m_threadId, WorkProc, this);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init task worker failed, ret %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Init task worker succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 退出task worker线程
Input        :
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskWorker::Exit()
{
    //暂时忽略线程返回值
    CMpThread::WaitForEnd(&m_threadId, NULL);
}

/*------------------------------------------------------------
Description  : 从消息队列中获取消息
Input        :
Output       : msg -- 获取的消息
Return       : MP_SUCCESS -- 成功
               MP_FAILED -- 请求队列中没有请求
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskWorker::PopRequest(message_pair_t& msg)
{
    vector<message_pair_t>::iterator iter;

    CThreadAutoLock tlock(&m_tPlgLock);
    if (m_vecReqQueue.empty())
    {
        return MP_FAILED;
    }

    iter = m_vecReqQueue.begin();
    msg = *iter;
    m_vecReqQueue.erase(iter);

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 把消息保存到消息队列
Input        : msg -- 要保存到队列的消息
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskWorker::PushRequest(message_pair_t& msg)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    m_vecReqQueue.push_back(msg);
}

mp_bool CTaskWorker::NeedExit()
{
    return m_bNeedExit;
}

mp_bool CTaskWorker::GetThreadProcReqStatus()
{
    return m_bProcReq;
}

mp_void CTaskWorker::SetThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}

/*------------------------------------------------------------
Description  : 进行消息处理，task worker线程回调函数调用该函数处理请求
Input        :
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskWorker::ReqProc()
{
    CServicePlugin* pPlugin = NULL;
    mp_string strService;
    message_pair_t msg;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin process request.");

    SetThreadStatus(THREAD_STATUS_RUNNING);
    while (!NeedExit())
    {
        m_SCN = m_plgMgr->GetSCN(); //lint !e613
        iRet = PopRequest(msg);
        if (MP_SUCCESS != iRet)
        {
            m_bProcReq = MP_FALSE;
            DoSleep(100);
            continue;
        }
        m_bProcReq = MP_TRUE;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get request succ.");

        strService = msg.pReqMsg->GetURL().GetServiceName();
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get service %s.", strService.c_str());
        
        pPlugin = GetPlugin((mp_char*)strService.c_str());
        if (NULL == pPlugin)
        {
            msg.pRspMsg->SetRetCode((mp_int64)ERROR_COMMON_PLUGIN_LOAD_FAILED);
            CCommunication::GetInstance().PushRspMsgQueue(msg);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get plugin failed, type %s.", strService.c_str());
            DoSleep(100);
            continue;
        }

        //获取当前的插件名称及scn
        SetPlugin(pPlugin);
        iRet = pPlugin->Invoke(msg.pReqMsg, msg.pRspMsg);
        if (MP_SUCCESS != iRet)
        {
            //如果插件某些分支没有设置返回码，这里统一设置
            if (msg.pRspMsg->GetRetCode() == MP_SUCCESS)
            {
                msg.pRspMsg->SetRetCode((mp_int64)iRet);
            }
            CCommunication::GetInstance().PushRspMsgQueue(msg);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invoke service plugin failed, iRet %d.", iRet);
            continue;
        }

        CCommunication::GetInstance().PushRspMsgQueue(msg);
    }
    
    SetThreadStatus(THREAD_STATUS_EXITED);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Process request succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 判断插件是否可以卸载(插件框架动态升级预留)
Input        : newSCN -- scn号
               plgName -- 插件名
Output       : 
Return       : MP_TRUE -- 可以卸载
               MP_FALSE -- 不可以卸载
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CTaskWorker::CanUnloadPlugin(mp_uint64 newSCN, const char* plgName)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    //work没有工作或者当前使用的插件不是需要删除的插件则可以删除
    if (NULL == m_plgName || 0 != strcmp(plgName, m_plgName))
    {
        return MP_TRUE;
    }

    //work当前使用的插件和需要删除的一致，scn一致说明work已经在使用新的插件则可以删除
    if (newSCN == m_SCN)
    {
        return MP_TRUE;
    }

    //当前work还在使用旧的插件，不允许删除
    return MP_FALSE;
}

/*------------------------------------------------------------
Description  : 根据服务名称获取插件
Input        : pszService -- 服务名
Output       : 
Return       : 成功返回获取的插件指针，失败返回NULL
Create By    :
Modification : 
-------------------------------------------------------------*/
CServicePlugin* CTaskWorker::GetPlugin(mp_char* pszService)
{
    mp_int32 iRet = MP_SUCCESS;
    plugin_def_t plgDef;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get plugin.");
    if (NULL == pszService)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is null.");
        return NULL;
    }
    //CodeDex误报，Dead Code
    iRet = m_plgCfgParse->GetPluginByService(pszService, plgDef);  //lint !e613
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get plugin failed.");
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get plugin %s.", plgDef.name.c_str());
    IPlugin* pPlg = m_plgMgr->GetPlugin(plgDef.name.c_str());  //lint !e613
    if (NULL == pPlg)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Load new plugin %s.", plgDef.name.c_str());
        pPlg = m_plgMgr->LoadPlugin(plgDef.name.c_str());  //lint !e613
        if (NULL == pPlg)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load plugin failed, name %s.", plgDef.name.c_str());
            return NULL;
        }
    }

    if (pPlg->GetTypeId() != CServicePlugin::APP_PUGIN_ID)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Plugin's type is wrong. expect = %d, actual = %d.",
            CServicePlugin::APP_PUGIN_ID, pPlg->GetTypeId());
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get plugin succ.");
    return (CServicePlugin*)pPlg;
}

/*------------------------------------------------------------
Description  : 保存插件先关信息(插件框架动态升级预留)
Input        : pPlug -- 插件指针
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskWorker::SetPlugin(CServicePlugin* pPlug)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    //m_SCN = CPluginManager::GetImpl()->GetSCN();
    m_plgVersion = pPlug->GetVersion();
    m_plgName = pPlug->GetName();
    //m_workState = workStat_work;
    //COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Set plugin info, scn %d, version %s, name %s.", m_SCN,
    //    m_plgVersion, m_plgName);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Set plugin info, scn %d.", m_SCN);
}

/*------------------------------------------------------------
Description  : Task worker线程回调函数
Input        : pThis -- 线程回调函数参数
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
#ifdef WIN32
DWORD WINAPI CTaskWorker::WorkProc(void* pThis)
#else
void* CTaskWorker::WorkProc(void* pThis)
#endif
{
    CTaskWorker* pTaskWorker = (CTaskWorker*)pThis;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin request process.");

    (void)pTaskWorker->ReqProc();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End request process.");

#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}

