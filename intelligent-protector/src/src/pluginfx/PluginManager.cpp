/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include <stdlib.h>
#include <vector>
#include "pluginfx/PluginManager.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "securec.h"

//////////////////////////////////////////////////////////////////////////////////////////
CPluginManager::CPluginManager()
{
    m_scn = 0;
    m_Callback = NULL;
    m_FileSearcher = NULL;
    CMpThread::InitLock(&m_QueryX);
    CMpThread::InitLock(&m_LoadX);
    CMpThread::InitLock(&m_UpgradeX);
}

CPluginManager::~CPluginManager()
{
    Clear();
    m_Callback = NULL;
    if (NULL != m_FileSearcher)
    {
        delete m_FileSearcher;
        m_FileSearcher = NULL;
    }

    CMpThread::DestroyLock(&m_QueryX);
    CMpThread::DestroyLock(&m_LoadX);
    CMpThread::DestroyLock(&m_UpgradeX);
}

mp_void CPluginManager::Clear()
{
    MODULES_MAP::iterator it = m_OldModules.begin();
    for(; it != m_OldModules.end(); ++it)
    {
        it->second->Unload();
        //it->second->Destroy();
        delete it->second;
    }

    it = m_Modules.begin();
    for(; it != m_Modules.end(); ++it)
    {
        it->second->Unload();
        //it->second->Destroy();
        delete it->second;
    }

    m_Modules.clear();
    m_OldModules.clear();
}

mp_int32 CPluginManager::Initialize(IPluginCallback* pCallback)
{
    m_Callback = pCallback;
    //CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(m_FileSearcher, CFileSearcher);
    return MP_SUCCESS;
}

mp_uint64 CPluginManager::GetSCN()
{
    return m_scn;
}

IPlugin* CPluginManager::GetPlugin(const mp_char* pszPlg)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get plugin, plg %s.", pszPlg);
    CModule* pModule = GetModule(pszPlg);
    if(pModule != NULL)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get plugin succ, this plugin has been loaded.");
        return pModule->GetPlugin();
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get plugin failed.");
    return NULL;
}

IPlugin* CPluginManager::LoadPlugin(const mp_char* pszPlg)
{
    CThreadAutoLock tlock(&m_LoadX);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin load plugin, plg %s.", pszPlg);
    CModule* pModule = GetModule(pszPlg);
    if(pModule != NULL)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get plugin from from cache succ.");
        return pModule->GetPlugin();
    }

    pModule = Load(pszPlg);
    if(NULL == pModule)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Load plugin failed, plg %s.", pszPlg);
        return NULL;
    }
    
    ++m_scn;
    (mp_void)InitModule(*pModule);

    //真正变更时需要对m_QueryX加锁
    CThreadAutoLock qlock(&m_QueryX);
    ++m_scn;
    m_Modules[pModule->Name()] = pModule;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load plugin succ, scn %d, module name %s.", m_scn, pModule->Name());
    return pModule->GetPlugin();
}

mp_void CPluginManager::UnloadPlugin(const mp_char* pszPlg)
{
    CModule* pModule = NULL;
    do
    {
        CThreadAutoLock tlock(&m_LoadX);
        pModule = GetModule(pszPlg);
        if(NULL == pModule)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unload plugin '%s' failed, the plugin has not been loaded.", pszPlg);
            return;
        }

        if(!m_Callback->CanUnload(pModule->GetPlugin()))
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unload plugin '%s' failed, the plugin can not be unload now.", pszPlg);
            return;
        }

        do
        {
            CThreadAutoLock qlock(&m_QueryX);
            ++m_scn;
            m_Modules.erase(pModule->Name());
        }while(0);
    }while(0);

    pModule->Unload();
    //pModule->Destroy();
    delete pModule;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unload plugin '%s' successfully.", pszPlg);
}

mp_int32 CPluginManager::Upgrade()
{
    mp_int32 iRet = 0;
    vector<CModule*> modules(256);

    //这里的锁仅仅是为了保证upgrade方法是线程安全的。
    CThreadAutoLock tlock(&m_UpgradeX);
    UnloadOldModules();

    do
    {
        //首先将当前加载的库备份到本地vector中
        //然后再逐个处理，这样可以保证m_LoadX的加锁时间最短
        //这里主要是减少同load_plugin的锁冲突
        CThreadAutoLock ldLock(&m_LoadX);
        MODULES_MAP::iterator it = m_Modules.begin();
        for(; it != m_Modules.end(); ++it)
        {
            modules.push_back(it->second);
        }
    } while (0);


    //依次检查是否需要重新加载
    for(mp_size i = 0; i<modules.size(); i++)
    {
        if(!NeedReload(*modules[i]))
        {
            continue;
        }

        Reload(modules[i]);
        iRet++;
    }

    return iRet;
}

mp_bool CPluginManager::Reload(CModule* pModule)
{
    CThreadAutoLock ldLock(&m_LoadX);
    CModule* pNewModule = Load(pModule->Name());
    if(NULL == pNewModule)
    {
        return MP_FALSE;
    }

    //++m_scn;
    (mp_void)InitModule(*pNewModule);
    
    m_Callback->OnUpgraded(pModule->GetPlugin(), pNewModule->GetPlugin());

    CThreadAutoLock qryLock(&m_QueryX);
    ++m_scn;

    //这里必须首先删除原来的，因为Map的可以是用const char*做key值
    //而且这个Key指针是放在second的对象中存放，如果不删除，则在执行
    //下一行代码时，这个Key值不会被替换，仍然使用旧对象中的指针，这样
    //当旧对象被删除后，这个Key就成为一个“野指针”，再访问这个Map时
    //很容易造成非法内存访问
    m_Modules.erase(pModule->Name());
    m_Modules[pNewModule->Name()] = pNewModule;

    //将旧插件放到另一MAP中，等待后续自动卸载
    m_OldModules[pModule->Name()] = pModule;

    return MP_TRUE;
}

mp_bool CPluginManager::InitModule(CModule& module)
{
    /*
    try
    {
        m_Callback->set_options(module.GetPlugin());
        module.GetPlugin()->Initialize(this);
    }
    catch(ECustom &e)
    {
        mpr_set_error(e.ErrNo, "Initialize plugin('%s') failed: %s", module.Name(), e.Message);
        DSF_RUN_ERROR(mpr_last_strerr());
        return MP_FALSE;
    }
    */

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Initialize plugin '%s'(%s) completed.", module.Name(), module.Version());
    return MP_TRUE;
}

mp_bool CPluginManager::NeedReload(CModule& module)
{
    mp_char szVersion[128];
    //没有版本定义不需要升级
    if(GetModuleVersion(module.Name(), szVersion, sizeof(szVersion)) == NULL)
    {
        return MP_FALSE;
    }

    //版本没有变化不需要升级
    if(strcmp(szVersion, module.Version()) == 0)
    {
        return MP_FALSE;
    }

    //动态库仍存在旧版本没有卸载的情况下，不进行版本更新
    //本函数和m_OldModules只有在upgrade过程中会被访问，所以不需要加锁
    MODULES_MAP::iterator it = m_OldModules.find(module.Name());
    if(it != m_OldModules.end())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Plugin(%s)'s old version(%s) still be running, new version %s reload later.",
            module.Name(), it->second->Version(), szVersion);
        return MP_FALSE;
    }

    return MP_TRUE;
}

mp_void CPluginManager::UnloadOldModules()
{
    CModule* pModule = NULL;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin unload old modules.");
    MODULES_MAP::iterator iter = m_OldModules.begin();
    while(iter != m_OldModules.end())
    {
        pModule = iter->second;
        if(!m_Callback->CanUnload(pModule->GetPlugin())) //lint !e613
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Plugin '%s' can not unload.", pModule->Name());
            ++iter;
            continue;
        }

        pModule->Unload();
        m_OldModules.erase(iter++);
        //pModule->Destroy();
        delete pModule;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unload old modules succ.");
}

CModule* CPluginManager::Load(const mp_char* pszName)
{
    mp_char szVersion[64] = {0};

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin load module, name %s.", pszName);

    if(GetModuleVersion(pszName, szVersion, sizeof(szVersion)) == NULL)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Can't get moudle version, name %s.", pszName);
        return Load(pszName, NULL);
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load moudle version succ, name %s, version %s.",
        pszName, szVersion);
    return Load(pszName, szVersion);
}

CModule* CPluginManager::Load(const mp_char* pszName, const mp_char* pszVer)
{
    mp_int32 iRet = MP_SUCCESS;
    CModule* pModule;
    try
    {
        //CodeDex误报，Memory Leak
        pModule = new CModule(m_FileSearcher, pszName, pszVer);
    }
    catch(...)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New CModule failed.");
        pModule = NULL;
    }

    if (!pModule)
    {
        return NULL;
    }
    iRet = pModule->Load();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Load plugin '%s' failed, iRet %d.", pszName, iRet);
        pModule->Unload();
        delete pModule;
        return NULL;
    }

    return pModule;
}

mp_char* CPluginManager::GetModuleVersion(const mp_char* pszModule, mp_char* pszVer, mp_int32 sz)
{
    return m_Callback->GetReleaseVersion(pszModule, pszVer, (mp_size)sz); //lint !e613
}

mp_void CPluginManager::PrintModules()
{
    MODULES_MAP::iterator iter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Print cached moudles.");
    for (iter = m_Modules.begin(); iter != m_Modules.end(); iter++)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Moudle name %s, version %s.", iter->first.c_str(), iter->second->Version());
    }
}

CModule* CPluginManager::GetModule(const mp_char* pszName)
{
    CThreadAutoLock tlock(&m_QueryX);
    PrintModules();
    MODULES_MAP::iterator it = m_Modules.find(pszName);
    if(m_Modules.end() == it)
    {
        return NULL;
    }
    return it->second;
}

//////////////////////////////////////////////////////////////////////////////////////////
CModule::CModule(CFileSearcher* pSearcher, const mp_char* pszName, const mp_char* pszVersion)
{
    m_Name = pszName;
    if (NULL != pszVersion)
        m_Version = pszVersion;
    m_FileSearcher = pSearcher;
    m_hLib = NULL;
    m_Plugin = NULL;
    time(&m_LoadTime);
}

CModule::~CModule()
{
    Unload();
    m_hLib = NULL;
    m_Plugin = NULL;
}//lint !e1579

IPlugin* CModule::GetPlugin()
{
    return m_Plugin;
}

const mp_char* CModule::Name() const
{
    return m_Name.c_str();
}

const mp_char* CModule::Version() const
{
    return m_Version.c_str();
}

mp_int32 CModule::Load()
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin load plugin.");

    if(m_hLib != NULL)
    {
        return ERROR_COMMON_DLL_LOAD_FAILED;
    }

    mp_char szName[256] = {0};
    if(m_Version.empty())
    {
        CHECK_FAIL(SNPRINTF_S(szName, sizeof(szName), sizeof(szName) - 1, "%s" LIB_SUFFIX, m_Name.c_str()));
    }
    else
    {
        CHECK_FAIL(SNPRINTF_S(szName, sizeof(szName), sizeof(szName) - 1, "%s-%s%s", m_Name.c_str(), 
            m_Version.c_str(), LIB_SUFFIX));
    }

    mp_string strPath;
    if(!m_FileSearcher->Search(szName, strPath))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Not found file '%s' from path '%s'.",
            szName, m_FileSearcher->GetPath().c_str());
        return ERROR_COMMON_DLL_LOAD_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin load library, plugin='%s'(%s), library='%s'.",
        m_Name.c_str(), m_Version.c_str(), strPath.c_str());
    mp_handle_t hLib = DlibOpenEx(strPath.c_str(), MP_TRUE);
    if(NULL == hLib)
    {
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Load library '%s' failed: %s.",
            strPath.c_str(), DlibError(szErr, sizeof(szErr)));
        return ERROR_COMMON_DLL_LOAD_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load library '%s' succ.", strPath.c_str());

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get 'QueryInterface' in library '%s'.", strPath.c_str());
    QUERY_INTERFACE fp = (QUERY_INTERFACE)DlibDlsym(hLib, "QueryInterface");  //lint !e611
    if(NULL == fp)
    {
        DlibClose(hLib);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "No symbol 'QueryInterface' in library '%s'.",
            strPath.c_str());
        return ERROR_COMMON_DLL_LOAD_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get 'QueryInterface' in library '%s' succ.", strPath.c_str());

    m_hLib = hLib;
    m_Plugin = fp();
    m_Plugin->SetOption("PLUGIN_NAME",     (void*)m_Name.c_str());
    m_Plugin->SetOption("PLUGIN_VERSION",  (void*)m_Version.c_str());
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load plugin succ, plugin='%s'(%s), library='%s'.",
        m_Name.c_str(), m_Version.c_str(), strPath.c_str());

    return MP_SUCCESS;
}

mp_int32 CModule::Unload()
{
    mp_int32 iRet = MP_SUCCESS;
    if(m_Plugin != NULL)
    {
        //CodeDex误报，Dead Code
        iRet = m_Plugin->Destroy();
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unload plugin '%s' failed, iRet %d.", m_Name.c_str(), iRet);
            return iRet;
        }
    }

    if(m_hLib != NULL)
    {
        DlibClose(m_hLib);
    }

    m_hLib = NULL;
    m_Plugin = NULL;

    return MP_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////////////////

