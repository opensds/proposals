/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "agent/TaskPool.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "securec.h"

CTaskPool::CTaskPool()
{
    (mp_void)memset_s(&m_pWorkers, sizeof(m_pWorkers), 0, sizeof(m_pWorkers));
    //Coverity&Fortify误报:UNINIT_CTOR
    //Coveirty&Fortify不认识公司安全函数memset_s，提示m_pWorkers未初始化
    m_pDispatchWorker = NULL;
    m_plugCfgParse = NULL;
    m_plugMgr = NULL;
#ifdef WIN32
    m_pVssWorker = NULL;
#endif
}

CTaskPool::~CTaskPool()
{
    Exit();
}//lint !e1579

/*------------------------------------------------------------
Description  : 初始化线程池
               如下方法中调用顺序不能随便调整，有依赖关系
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskPool::Init()
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin init task pool.");
    iRet = CreatePlgConfParse();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create plg conf parse failed.");
        return iRet;
    }

    iRet = CreatePlugMgr();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create plg manager failed.");
        return iRet;
    }

    iRet = CreateWorkers();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create task workers failed.");
        return iRet;
    }

#ifdef WIN32
    iRet = CreateVssWorker();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create vss workers failed.");
        return iRet;
    }
#endif

    iRet = CreateDispatchWorker();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create dispatch worker failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End init task pool.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 退出线程池
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskPool::Exit()
{
    CTaskWorker* pWorker = NULL;

    for (mp_uint32 i = 0; i < MAX_WORKER_NUM; i++)
    {
        pWorker = m_pWorkers[i];
        if (NULL != pWorker)
        {
            pWorker->Exit();
            delete pWorker;
        }
        m_pWorkers[i] = NULL;
    }

    if (NULL != m_plugCfgParse)
    {
        delete m_plugCfgParse;
        m_plugCfgParse = NULL;
    }

    if (NULL != m_plugMgr)
    {
        delete m_plugMgr;
        m_plugMgr = NULL;
    }

#ifdef WIN32
    if (NULL != m_pVssWorker)
    {
        m_pVssWorker->Exit();
        delete m_pVssWorker;
        m_pVssWorker = NULL;
    }
#endif
}

/*------------------------------------------------------------
Description  : 创建task worker线程
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskPool::CreateWorkers()
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create task workers.");

    for (mp_uint32 i = 0; i < MAX_WORKER_NUM; i++)
    {	
    	//CodeDex误报，Memory Leak
        NEW_CATCH_RETURN_FAILED(m_pWorkers[i], CTaskWorker);
        iRet = m_pWorkers[i]->Init(m_plugCfgParse, m_plugMgr);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init task worker[%d] failed, iRet %d.", i, iRet);
            return iRet;
        }
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Init task worker[%d] succ.", i + 1);
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create task workers succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------
Description  : 创建protect worker线程
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskPool::CreateProtectWorker()
{
    return MP_SUCCESS;
}

#ifdef WIN32
/*------------------------------------------------------------
Description  : 创建vss worker线程，仅windows下存在该线程
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskPool::CreateVssWorker()
{
    mp_int32 i = 0;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create task vss worker.");
    NEW_CATCH_RETURN_FAILED(m_pVssWorker, CTaskVssWorker);
    iRet = m_pVssWorker->Init(m_plugCfgParse, m_plugMgr);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init task vss worker failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Init task vss worker succ.");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create task vss worker succ.");
    return MP_SUCCESS;
}
#endif //WIN32

/*------------------------------------------------------------
Description  : 创建dispatch worker线程
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskPool::CreateDispatchWorker()
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 i = 0;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create task dispatch workers.");
	//CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(m_pDispatchWorker, CTaskDispatchWorker);
#ifdef WIN32
    iRet = m_pDispatchWorker->Init(&m_pWorkers[0], MAX_WORKER_NUM, m_pVssWorker);
#else
    iRet = m_pDispatchWorker->Init(&m_pWorkers[0], MAX_WORKER_NUM);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init task dispatch worker[%d] failed, iRet %d.", i, iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Init task dispatch worker succ.");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create task dispatch workers succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 创建插件配置文件解析对象
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskPool::CreatePlgConfParse()
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strPlgCfg;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create plg conf parse.");
    //CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(m_plugCfgParse, CPluginCfgParse);
    strPlgCfg = CPath::GetInstance().GetConfFilePath(AGENT_PLG_CONF);
    iRet = m_plugCfgParse->Init((mp_char*)strPlgCfg.c_str());
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init plugin conf parse failed, iRet %d.", iRet);
        return iRet;
    }
    m_plugCfgParse->PrintPluginDef();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create plg conf parse succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 创建插件管理对象
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskPool::CreatePlugMgr()
{
    mp_string strPlgPath;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create plugin manager.");
    strPlgPath = CPath::GetInstance().GetPluginsPath();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Plugin directory is %s.", strPlgPath.c_str());
    //CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(m_plugMgr, CPluginManager);
    iRet = m_plugMgr->Initialize(this);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init plugin manager failed.");
        delete m_plugMgr;
        m_plugMgr = NULL;
        return iRet;
    }
    m_plugMgr->SetPluginPath(strPlgPath.c_str());
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create plugin manager succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 判断插件是否可以卸载
               IPluginCallback 虚方法实现
Input        : pOldPlg -- 判断是否可卸载的插件指针
Output       : 
Return       : MP_TRUE -- 可以卸载
               MP_FALSE -- 不可以卸载
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CTaskPool::CanUnload(IPlugin* pOldPlg)
{
    mp_uint64 scn;
    const mp_char* plgName = NULL;
	//CodeDex误报，Dead Code
    if (NULL == m_plugMgr)
    {
        return MP_FALSE;
    }
    scn = m_plugMgr->GetSCN();
    
    plgName = pOldPlg->GetName();
    if (NULL == plgName)
    {
        return MP_FALSE;
    }

    for (mp_uint32 i = 0; i < MAX_WORKER_NUM; i++)
    {
        if (MP_FALSE == m_pWorkers[i]->CanUnloadPlugin(scn, plgName))
        {
            return MP_FALSE;
        }
    }

    return MP_TRUE;
}

/*------------------------------------------------------------
Description  : 升级事件处理函数(插件框架动态升级预留)
               IPluginCallback 虚方法实现
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskPool::OnUpgraded(IPlugin* pOldPlg, IPlugin* pNewPlg)
{
    return;
}

/*------------------------------------------------------------
Description  : 设置插件选项(插件框架动态升级预留)
               IPluginCallback 虚方法实现
Input        :
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskPool::SetOptions(IPlugin* plg)
{
    return;
}

/*------------------------------------------------------------
Description  : 获取插件版本信息
               IPluginCallback 虚方法实
Input        : pszLib -- 插件名称
Output       : pszVer -- 用于保存版本信息的缓冲区指针
               sz -- 用于保存版本信息的缓冲区长度
Return       : 成功返回pszVer，失败返回NULL
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_char* CTaskPool::GetReleaseVersion(const mp_char* pszLib, mp_char* pszVer, mp_size sz)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strVersion;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get release version.");
    if (NULL == m_plugCfgParse)
    {
        return NULL;
    }
    
    iRet = m_plugCfgParse->GetPluginVersion(pszLib, strVersion);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get release version failed.");
        return NULL;
    }

    iRet = strncpy_s(pszVer, sz, strVersion.c_str(), strlen(strVersion.c_str()));
    if (EOK != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Strncpy_s failed, iRet %d.", iRet);
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get release version succ, name %s, version %s.", pszLib, pszVer);
    return pszVer;
}

