/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_PLUGIN_CFG_PARSE_H
#define _AGENT_PLUGIN_CFG_PARSE_H

#include <vector>
#include "common/Types.h"
#include "common/Thread.h"
#include "tinyxml/tinyxml.h"

#define XML_NODE_PLUGIN_LIST      "PluginList"
#define XML_NODE_PLUGIN           "Plugin"
#define XML_ATTR_PLUGIN_NAME      "name"
#define XML_ATTR_PLUGIN_VERSION   "version"
#define XML_ATTR_PLUGIN_SERVICE   "service"


typedef struct tag_plugin_def_tag
{
    mp_string name;
    mp_string version;
    mp_string service;
} plugin_def_t;

class CPluginCfgParse
{
private:
    mp_string m_strFileName;
    mp_time m_tFileTime;
    vector<plugin_def_t> m_vecPlgDefs;
    thread_lock_t m_tLock;
    
public:
    CPluginCfgParse();
    ~CPluginCfgParse();

    mp_int32 Init(mp_char* pszFileName);
    mp_int32 GetPluginByService(mp_char* pszServiceName, plugin_def_t& plgDef);
    mp_int32 GetPluginVersion(const mp_char* pszPlgName, mp_string& strVersion);
    void PrintPluginDef();

private:
    mp_bool LoadCfg(mp_char* pszFileName);
    mp_bool LoadPluginDefs(TiXmlElement* pTiPlugins);
    mp_bool LoadPluginDef(TiXmlElement* pTiPlugin);
    mp_void AddPluginDef(plugin_def_t& plgDef);
    plugin_def_t* GetPlugin(const mp_char* pszPluginName);
};

#endif //_AGENT_PLUGIN_CFG_PARSE_H

