/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_PLUGIN_MANAGER_IMPL_H
#define _AGENT_PLUGIN_MANAGER_IMPL_H

#include <map>
#include "pluginfx/IPlugin.h"
#include "common/Thread.h"
#include "common/FileSearcher.h"

typedef IPlugin *(*QUERY_INTERFACE)();

//////////////////////////////////////////////////////////////////////////////////////////
class CModule
{
public:
    CModule(CFileSearcher* pSearcher, const mp_char* pszName, const mp_char* pszVersion);
    virtual ~CModule();

    IPlugin* GetPlugin();
    const mp_char* Name() const;
    const mp_char* Version() const;
    
    mp_int32 Load();
    mp_int32 Unload();

    mp_time LoadTime() const { return m_LoadTime; }

private:

    mp_time m_LoadTime;
    mp_handle_t m_hLib;
    IPlugin* m_Plugin;

    CFileSearcher* m_FileSearcher;
    mp_string m_Name;
    mp_string m_Version;
};

//////////////////////////////////////////////////////////////////////////////////////////
class CPluginManager
{
    //friend class CPluginManager;
public:
    CPluginManager();
    virtual ~CPluginManager();

    mp_int32 Initialize(IPluginCallback* pCallback);
    mp_uint64 GetSCN();

    IPlugin* GetPlugin(const mp_char* pszPlg);
    IPlugin* LoadPlugin(const mp_char* pszPlg);
    mp_void UnloadPlugin(const mp_char* pszPlg);
    mp_int32 Upgrade();

    mp_void SetPluginPath(const mp_char* pszPath) {
        m_FileSearcher->SetPath(pszPath);
    }

    const mp_string& GetPluginPath() const {
        return m_FileSearcher->GetPath();
    }

protected:
    mp_uint64 m_scn;
    //查询锁，GetModule时使用，在更新m_Modules
    //时必须加m_QueryX锁，保证查询时的数据一致性
    thread_lock_t m_QueryX; 
    thread_lock_t m_LoadX;     //更新锁
    thread_lock_t m_UpgradeX;  //保证upgrade可以并发

    virtual mp_char* GetModuleVersion(const mp_char* pszModule, mp_char* pszVer, mp_int32 sz);
    CModule* Load(const mp_char* pszName);
    CModule* Load(const mp_char* pszName, const mp_char* szVersion);
    CModule* GetModule(const mp_char* pszName);
    mp_void UnloadOldModules();
    mp_bool NeedReload(CModule& module);
    mp_bool InitModule(CModule& module);
    mp_bool Reload(CModule* pModule);

    mp_void Clear();

    //typedef map<const mp_char*, CModule*> MODULES_MAP;
    typedef map<mp_string, CModule*> MODULES_MAP;
    MODULES_MAP m_Modules;
    MODULES_MAP m_OldModules;

    CFileSearcher* m_FileSearcher;
    IPluginCallback* m_Callback;

private:
    mp_void PrintModules();
};
//////////////////////////////////////////////////////////////////////////////////////////

#endif //_AGENT_PLUGIN_MANAGER_IMPL_H

