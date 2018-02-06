/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "pluginfx/PluginCfgParse.h"
#include "common/Log.h"
#include "securec.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"

CPluginCfgParse::CPluginCfgParse()
{
   m_tFileTime = 0;
   CMpThread::InitLock(&m_tLock);
}

CPluginCfgParse::~CPluginCfgParse()
{
    CMpThread::DestroyLock(&m_tLock);
}

mp_int32 CPluginCfgParse::Init(mp_char* pszFileName)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin init plugin conf parse.");
    if (NULL == pszFileName)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is null.");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    CThreadAutoLock tlock(&m_tLock);
    if (!LoadCfg(pszFileName))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Load cfg file failed, file %s.", BaseFileName(pszFileName));
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Init plugin conf parse succ.");
    return MP_SUCCESS;
}

mp_int32 CPluginCfgParse::GetPluginByService(mp_char* pszServiceName, plugin_def_t& plgDef)
{
    CThreadAutoLock tlock(&m_tLock);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "#### pszServiceName:%s", pszServiceName);
    for (std::vector<plugin_def_t>::iterator iter = m_vecPlgDefs.begin();
        iter != m_vecPlgDefs.end(); ++iter)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "#### name:%s, server:%s, version:%s", 
            iter->name.c_str(), iter->service.c_str(), iter->version.c_str());
    }

    for (mp_uint32 i = 0; i < m_vecPlgDefs.size(); i++)
    {
        if (0 == strcmp(pszServiceName, m_vecPlgDefs[i].service.c_str()))
        {
            plgDef = m_vecPlgDefs[i];
            return MP_SUCCESS;
        }
    }

    return MP_FAILED;
}

mp_int32 CPluginCfgParse::GetPluginVersion(const mp_char* pszPlgName, mp_string& strVersion)
{
    plugin_def_t* pPlgDef = NULL;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get plugin version.");
    if (NULL == pszPlgName)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Input param is null.");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    CThreadAutoLock tlock(&m_tLock);
    pPlgDef = GetPlugin(pszPlgName);
    if (NULL == pPlgDef)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    strVersion = pPlgDef->version;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get plugin version succ, name %s, version %s.", pszPlgName,
        strVersion.c_str());
    return MP_SUCCESS;
}

void CPluginCfgParse::PrintPluginDef()
{
    vector<plugin_def_t>::iterator iter;

    CThreadAutoLock tlock(&m_tLock);
    for (iter = m_vecPlgDefs.begin(); iter != m_vecPlgDefs.end(); iter++)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "------Plugin : name %s, version %s-------", iter->name.c_str(),
            iter->version.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "");
}

mp_bool CPluginCfgParse::LoadCfg(mp_char* pszFileName)
{
    TiXmlDocument xdoc;
    TiXmlElement* pPlgList = NULL;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin load cfg file.");

    if (NULL == pszFileName)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is null.");
        return MP_FALSE;
    }
    //CodeDex误报，Dead Code
    if (MP_SUCCESS != CMpFile::GetlLastModifyTime(pszFileName, m_tFileTime))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get last modify time failed, file %s.", BaseFileName(pszFileName));
        return MP_FALSE;
    }
    //CodeDex误报,KLOCWORK.NPD.FUNC.MUST
    if (!xdoc.LoadFile(pszFileName) || NULL == xdoc.RootElement())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid config file '%s', error: %s, %s.", BaseFileName(pszFileName), xdoc.Value(),
            xdoc.ErrorDesc());
        return MP_FALSE;
    }

    pPlgList = xdoc.RootElement()->FirstChildElement();
    if (NULL == pPlgList)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "No 'ModuleList' in config file '%s'.", BaseFileName(m_strFileName.c_str()));
    }
    else
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "start load <%s><%s>", XML_NODE_PLUGIN_LIST, XML_NODE_PLUGIN);

        if (MP_FALSE == LoadPluginDefs(pPlgList))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Load plugins failed.");
            return MP_FALSE;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load cfg file succ.");

    return MP_TRUE;
}

mp_bool CPluginCfgParse::LoadPluginDefs(TiXmlElement* pTiPlugins)
{
    TiXmlElement* plugin = NULL;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin load plugin defs.");
	//CodeDex误报，Dead Code
    if (NULL == pTiPlugins)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is null.");
        return MP_FALSE;
    }

    for(plugin = pTiPlugins->FirstChildElement(); plugin; plugin = plugin->NextSiblingElement())
    {
        if (MP_FALSE == LoadPluginDef(plugin))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Load plugin failed.");
            return MP_FALSE;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load plugin defs succ.");

    return MP_TRUE;
}

mp_bool CPluginCfgParse::LoadPluginDef(TiXmlElement* pTiPlugin)
{
    plugin_def_t plgDef;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin load plugin def.");
	//CodeDex误报，Dead Code
    if (NULL == pTiPlugin)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is null.");
        return MP_FALSE;
    }

    const mp_char* pszName = pTiPlugin->Attribute(XML_ATTR_PLUGIN_NAME);
    const mp_char* pszVersion = pTiPlugin->Attribute(XML_ATTR_PLUGIN_VERSION);
    const mp_char* pszService = pTiPlugin->Attribute(XML_ATTR_PLUGIN_SERVICE);

    if (strempty(pszName) || strempty(pszVersion) || strempty(pszService))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "No attribute 'name', 'version' or 'service' in plugin's config, row %d, col %d.",
            pTiPlugin->Row(), pTiPlugin->Column());
        return MP_FALSE;
    }

    plgDef.name = pszName;
    plgDef.version = pszVersion;
    plgDef.service = pszService;
    AddPluginDef(plgDef);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load plugin def succ.");

    return MP_TRUE;
}

mp_void CPluginCfgParse::AddPluginDef(plugin_def_t& plgDef)
{
    if (NULL == GetPlugin((mp_char*)plgDef.name.c_str()))
    {
        m_vecPlgDefs.push_back(plgDef);
    }
}

plugin_def_t* CPluginCfgParse::GetPlugin(const mp_char* pszPluginName)
{
    plugin_def_t* pPlgDef = NULL;

    for (mp_uint32 i = 0; i < m_vecPlgDefs.size(); i++)
    {
        if (0 == strcmp(pszPluginName, m_vecPlgDefs[i].name.c_str()))
        {
            pPlgDef = &m_vecPlgDefs[i];
        }
    }

    return pPlgDef;
}

