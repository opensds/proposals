/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "pluginfx/PluginCfgParseTest.h"

using namespace std;

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(CMpPluginCfgParseTest, PluginCfgParseTest)
{
    try
    {
        //  打桩Log 函数，防止出现Segmentation fault错误
        typedef mp_void (*StubFuncType)(void);
        typedef mp_void (CLogger::*LogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
        Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);

        /********Begin PluginCfgParse.cpp test********/

        // write config information to temp file
        vector<mp_string> vecTmpCfgFile;
        vecTmpCfgFile.push_back("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        vecTmpCfgFile.push_back("<Config>");
        vecTmpCfgFile.push_back("    <PluginList>");
        vecTmpCfgFile.push_back("        <Plugin name=\"liboracle\" version=\"V100R005C00\" service=\"oracle\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libdb2\" version=\"V100R005C00\" service=\"db2\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libhost\" version=\"V100R005C00\" service=\"host\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libdevice\" version=\"V100R005C00\" service=\"device\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libexchange\" version=\"V100R005C00\" service=\"exchange\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libcluster\" version=\"V100R005C00\" service=\"cluster\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("        <Plugin name=\"libsqlserver\" version=\"V100R005C00\" service=\"sqlserver\">");
        vecTmpCfgFile.push_back("        </Plugin>");
        vecTmpCfgFile.push_back("    </PluginList>");
        vecTmpCfgFile.push_back("    <ServiceList>");
        vecTmpCfgFile.push_back("        <Service name=\"oracle\" plugin=\"liboracle\">");
        vecTmpCfgFile.push_back("        </Service>");
        vecTmpCfgFile.push_back("        <Service name=\"db2\" plugin=\"libdb2\">");
        vecTmpCfgFile.push_back("        </Service>");
        vecTmpCfgFile.push_back("        <Service name=\"exchange\" plugin=\"libexchange\">");
        vecTmpCfgFile.push_back("        </Service>");
        vecTmpCfgFile.push_back("    </ServiceList>");
        vecTmpCfgFile.push_back("</Config>");
        mp_string tmpCfgFile("/tmp/gtestcfg.xml");
        
        // delete file
        CMpFile::DelFile(tmpCfgFile.c_str());

        CPluginCfgParse pluginParse;
        // NULL file
        mp_int32 iRet = pluginParse.Init(NULL);
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

        // not exist file
        iRet = pluginParse.Init((mp_char *)tmpCfgFile.c_str());
        //EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

        // exist file
        CIPCFile::WriteFile(tmpCfgFile, vecTmpCfgFile);
        iRet = pluginParse.Init((mp_char *)tmpCfgFile.c_str());
        EXPECT_EQ(iRet, MP_SUCCESS);

        // get not exists services
        plugin_def_t pluginInfo;
        iRet = pluginParse.GetPluginByService("sqlserver1111", pluginInfo);
        EXPECT_EQ(iRet, MP_FAILED);

        // check exists services
        iRet = pluginParse.GetPluginByService("oracle", pluginInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);
        EXPECT_EQ("oracle", pluginInfo.service);
        EXPECT_EQ("liboracle", pluginInfo.name);
        EXPECT_EQ("V100R005C00", pluginInfo.version);

        mp_string pluginVersion;
        // get no exists plugin
        iRet = pluginParse.GetPluginVersion("sqlserver", pluginVersion);
        EXPECT_EQ(iRet, ERROR_COMMON_READ_CONFIG_FAILED);

        // get exists plugin
        iRet = pluginParse.GetPluginVersion("liboracle", pluginVersion);
        EXPECT_EQ(iRet, MP_SUCCESS);
        EXPECT_EQ("V100R005C00", pluginVersion);
        
        iRet = pluginParse.GetPluginVersion(NULL, pluginVersion);
        /********End PluginCfgParse.cpp test********/
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    }
}

TEST_F(CMpPluginCfgTest, GetPluginVersion){
    CPluginCfgParse m_CPluginCfgParse;
    mp_char pszPlgName;
    mp_string strVersion;
    
    m_CPluginCfgParse.GetPluginVersion(NULL,strVersion);
}

TEST_F(CMpPluginCfgTest, LoadCfg){
    CPluginCfgParse m_CPluginCfgParse;
    mp_char pszFileName;
    
    m_CPluginCfgParse.LoadCfg(NULL);
}

TEST_F(CMpPluginCfgTest, LoadPluginDefs){
    CPluginCfgParse m_CPluginCfgParse;
    
    m_CPluginCfgParse.LoadPluginDefs(NULL);
}

TEST_F(CMpPluginCfgTest, LoadPluginDef){
    CPluginCfgParse m_CPluginCfgParse;
    
    m_CPluginCfgParse.LoadPluginDef(NULL);
}

