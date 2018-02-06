/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "pluginfx/PluginManagerTest.h"
    
using namespace std;

static mp_void StubCLoggerLog(mp_void)
{
    return;
}

CPluginManagerCBTest::CPluginManagerCBTest()
{
}

CPluginManagerCBTest::~CPluginManagerCBTest()
{
}

mp_bool CPluginManagerCBTest::CanUnload(IPlugin* pOldPlg)
{
    return MP_TRUE;
}

mp_void CPluginManagerCBTest::OnUpgraded(IPlugin* pOldPlg, IPlugin* pNewPlg)
{
}

mp_void CPluginManagerCBTest::SetOptions(IPlugin* plg)
{
}

mp_char* CPluginManagerCBTest::GetReleaseVersion(const mp_char* pszLib, mp_char* pszVer, mp_size sz)
{
    return "";
}


TEST_F(CMpPluginManagerTest, PluginManagerTest)
{
    try
    {
        //  打桩Log 函数，防止出现Segmentation fault错误
        typedef mp_void (*StubFuncType)(void);
        typedef mp_void (CLogger::*LogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
        Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);

        /********Begin PluginManager.cpp test********/
        CPluginManagerCBTest pluginMCBTset;
        CPluginManager pluginManager;

        // initialize
        mp_int32 iRet = pluginManager.Initialize(&pluginMCBTset);
        EXPECT_EQ(iRet, MP_SUCCESS);
        
        pluginManager.SetPluginPath("./");

        // get no exists plugin
        IPlugin* pPlg = pluginManager.GetPlugin("libtest");
        EXPECT_EQ(pPlg, (IPlugin *)NULL);

        // loal plugin
        pPlg = pluginManager.LoadPlugin("libtest");
        //EXPECT_NE(pPlg, (IPlugin *)NULL);

        if (pPlg != NULL)
        {
            // check plugin
            pPlg = pluginManager.GetPlugin("libtest");
            EXPECT_NE(pPlg, (IPlugin *)NULL);

            // unload plugin
            pluginManager.UnloadPlugin("libtest");
        
            // check plugin
            pPlg = pluginManager.GetPlugin("libtest");
            EXPECT_EQ(pPlg, (IPlugin *)NULL);
        }
        
        // check upgrade
        //iRet = pluginManager.Upgrade();
        //EXPECT_EQ(iRet, MP_SUCCESS);
        /********End PluginManager.cpp test********/
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    }
}

TEST_F(CMpPluginManagerTest,GetPlugin){
    CPluginManager m_CPluginManager;
    mp_char pszPlg;
    
    {
        Stub<GetModuleType, StubGetModuleType, mp_void> mystub1(&CPluginManager::GetModule, &StubGetModule0);
        m_CPluginManager.GetPlugin(&pszPlg);
    }
    
    {
        Stub<GetModuleType, StubGetModuleType, mp_void> mystub1(&CPluginManager::GetModule, &StubGetModule);
        m_CPluginManager.GetPlugin(&pszPlg);
    }
}

TEST_F(CMpPluginManagerTest,LoadPlugin){
    CPluginManager m_CPluginManager;
    mp_char pszPlg;
    
    {
        Stub<GetModuleType, StubGetModuleType, mp_void> mystub1(&CPluginManager::GetModule, &StubGetModule);
        m_CPluginManager.LoadPlugin(&pszPlg);
    }
    
    {
        Stub<GetModuleType, StubGetModuleType, mp_void> mystub1(&CPluginManager::GetModule, &StubGetModule0);
        Stub<LoadType, StubLoadType, mp_void> mystub2(&CPluginManager::Load, &StubLoad);
        m_CPluginManager.LoadPlugin(&pszPlg);
    }
}

TEST_F(CMpPluginManagerTest,GetSCN){
    CPluginManager m_CPluginManager;
    
    m_CPluginManager.GetSCN();
}

TEST_F(CMpPluginManagerTest,UnloadPlugin){
    CPluginManager m_CPluginManager;
    CPluginManagerCBTest m_CPluginManagerCBTest;
    m_CPluginManager.m_Callback = &m_CPluginManagerCBTest;
    mp_char pszPlg;
    
    {
        m_CPluginManager.UnloadPlugin(&pszPlg);
    }
    
    {
        Stub<GetModuleType, StubGetModuleType, mp_void> mystub1(&CPluginManager::GetModule, &StubGetModule);
        m_CPluginManager.UnloadPlugin(&pszPlg);
    }
}

TEST_F(CMpPluginManagerTest,Load){
    CFileSearcher m_FileSearcher;
    mp_char pszName;
    mp_char pszVersion;
    CModule* m_CModule = new CModule(&m_FileSearcher,&pszName,&pszVersion);
    
    {
        Stub<SearchType, StubSearchType, mp_void> mystub1(&CFileSearcher::Search, &StubSearch0);
        Stub<DlibOpenExType, StubDlibOpenExType, mp_void> mystub2(&DlibOpenEx, &StubDlibOpenEx0);
        m_CModule->Load();
    }
    
    {
        Stub<SearchType, StubSearchType, mp_void> mystub1(&CFileSearcher::Search, &StubSearch0);
        Stub<DlibOpenExType, StubDlibOpenExType, mp_void> mystub2(&DlibOpenEx, &StubDlibOpenEx);
        Stub<DlibDlsymType, StubDlibDlsymType, mp_void> mystub3(&DlibDlsym, &StubDlibDlsym0);
        Stub<DlibCloseType, StubDlibCloseType, mp_void> mystub4(&DlibClose, &StubDlibClose);
        m_CModule->Load();
    }
/*    
    {
        Stub<SearchType, StubSearchType, mp_void> mystub1(&CFileSearcher::Search, &StubSearch0);
        Stub<DlibOpenExType, StubDlibOpenExType, mp_void> mystub2(&DlibOpenEx, &StubDlibOpenEx);
        Stub<DlibDlsymType, StubDlibDlsymType, mp_void> mystub3(&DlibDlsym, &StubDlibDlsym);
        m_CModule->Load();
    }*/
}

TEST_F(CMpPluginManagerTest,Reload){
    CPluginManager m_CPluginManager;
    CPluginManagerCBTest m_CPluginManagerCBTest;
    m_CPluginManager.m_Callback = &m_CPluginManagerCBTest;
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_char pszName;
    mp_char pszVersion;
    pModule = new CModule(&m_FileSearcher,&pszName,&pszVersion);
    
    {
        Stub<LoadType, StubLoadType, mp_void> mystub1(&CPluginManager::Load, &StubLoad0);
        m_CPluginManager.Reload(pModule);
    }

    {
        Stub<LoadType, StubLoadType, mp_void> mystub1(&CPluginManager::Load, &StubLoad);
        m_CPluginManager.Reload(pModule);
    }
}

TEST_F(CMpPluginManagerTest,InitModule){
    CPluginManager m_CPluginManager;
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_char pszName;
    mp_char pszVersion;
    pModule = new CModule(&m_FileSearcher,&pszName,&pszVersion);
    
    m_CPluginManager.InitModule(*pModule);
}

TEST_F(CMpPluginManagerTest,NeedReload){
    CPluginManager m_CPluginManager;
    CPluginManagerCBTest m_CPluginManagerCBTest;
    m_CPluginManager.m_Callback = &m_CPluginManagerCBTest;
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_char pszName;
    mp_char pszVersion;
    pModule = new CModule(&m_FileSearcher,&pszName,&pszVersion);
    
    m_CPluginManager.NeedReload(*pModule);
}

TEST_F(CMpPluginManagerTest,UnloadOldModules){
    CPluginManager m_CPluginManager;
    CPluginManagerCBTest m_CPluginManagerCBTest;
    m_CPluginManager.m_Callback = &m_CPluginManagerCBTest;
    CFileSearcher m_FileSearcher;
    mp_char pszName;
    mp_char pszVersion;
    
    m_CPluginManager.m_OldModules.insert(pair<mp_string,CModule*>("test",new CModule(&m_FileSearcher,&pszName,&pszVersion)));
    
    m_CPluginManager.UnloadOldModules();
}
