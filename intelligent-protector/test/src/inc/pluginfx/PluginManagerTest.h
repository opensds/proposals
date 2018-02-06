/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __PLUGINCFG_MANAGER_H__
#define __PLUGINCFG_MANAGER_H__

#define private public
#define protected public

#include <stdlib.h>
#include <vector>
#include "pluginfx/PluginManager.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "securec.h"
#include "pluginfx/IPlugin.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCMpPluginManagerTestLogVoid(mp_void* pthis);

class CMpPluginManagerTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCMpPluginManagerTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>*  CMpPluginManagerTest::m_stub;

//******************************************************************************

class CPluginManagerCBTest: public IPluginCallback
{
public:
    CPluginManagerCBTest();
    ~CPluginManagerCBTest();
 
    //IPluginCallback 虚方法实现
    virtual mp_bool CanUnload(IPlugin* pOldPlg);
    virtual mp_void OnUpgraded(IPlugin* pOldPlg, IPlugin* pNewPlg);
    virtual mp_void SetOptions(IPlugin* plg);
    virtual mp_char* GetReleaseVersion(const mp_char* pszLib, mp_char* pszVer, mp_size sz);
};

//******************************************************************************
typedef CModule* (CPluginManager::*GetModuleType)(const mp_char* pszName);
typedef CModule* (*StubGetModuleType)(mp_void* pthis,const mp_char* pszName);
  
typedef mp_bool (CPluginManagerCBTest::*CanUnloadType)(IPlugin* pOldPlg);
typedef mp_bool (*StubCanUnloadType)(IPlugin* pOldPlg);

typedef CModule* (CPluginManager::*LoadType)(const mp_char* pszName);
typedef CModule* (*StubLoadType)(const mp_char* pszName);

typedef mp_bool (CFileSearcher::*SearchType)(const mp_char* pszFile, mp_string& strPath);
typedef mp_bool (*StubSearchType)(const mp_char* pszFile, mp_string& strPath);

typedef mp_handle_t (*DlibOpenExType)(const mp_char* pszLibName, mp_bool bLocal);
typedef mp_handle_t (*StubDlibOpenExType)(const mp_char* pszLibName, mp_bool bLocal);

typedef mp_void* (*DlibDlsymType)(mp_handle_t hDlib, const mp_char* pszFname);
typedef mp_void* (*StubDlibDlsymType)(mp_handle_t hDlib, const mp_char* pszFname);

typedef mp_void (*DlibCloseType)(mp_handle_t hDlib);
typedef mp_void (*StubDlibCloseType)(mp_handle_t hDlib);
//******************************************************************************
mp_void StubDlibClose(mp_handle_t hDlib){
    return;
}

mp_void* StubDlibDlsym0(mp_handle_t hDlib, const mp_char* pszFname){
    return NULL;
}

mp_void* StubDlibDlsym(mp_handle_t hDlib, const mp_char* pszFname){
    
    return (mp_void*)"test";
}

mp_handle_t StubDlibOpenEx0(const mp_char* pszLibName, mp_bool bLocal){
    return NULL;
}

mp_handle_t StubDlibOpenEx(const mp_char* pszLibName, mp_bool bLocal){
    return (mp_handle_t)"test";
}

mp_bool StubSearch0(const mp_char* pszFile, mp_string& strPath){
    return MP_TRUE;
}

mp_bool StubSearch(const mp_char* pszFile, mp_string& strPath){
    return MP_FALSE;
}

CModule* StubLoad(const mp_char* pszName){
    pszName = NULL;
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_char pszName1;
    mp_char pszVersion;
    pModule = new CModule(&m_FileSearcher,&pszName1,&pszVersion);
    
    return pModule;
}

CModule* StubLoad0(const mp_char* pszName){
    pszName = NULL;
    
    return NULL;
}

CModule* StubGetModule(mp_void* pthis,const mp_char* pszName){
    pszName = NULL;
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_char pszName1;
    mp_char pszVersion;
    pModule = new CModule(&m_FileSearcher,&pszName1,&pszVersion);
    
    return pModule;
}

CModule* StubGetModulet(mp_void* pthis,const mp_char* pszName){
    pszName = "test";
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_char pszName1;
    mp_char pszVersion;
    pModule = new CModule(&m_FileSearcher,&pszName1,&pszVersion);
    
    return pModule;
}

CModule* StubGetModule0(mp_void* pthis,const mp_char* pszName){    
    return NULL;
}

mp_void StubCMpPluginManagerTestLogVoid(mp_void* pthis){
    return;
}

#endif


