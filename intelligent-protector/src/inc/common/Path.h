/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_PATH_H__
#define __AGENT_PATH_H__

#include "common/Types.h"
#include "common/Defines.h"

//路径相关
class AGENT_API CPath
{
public:
    static CPath& GetInstance()
    {
        return m_instance;
    }

    ~CPath()
    {
    }

    //pszBinFilePath的格式为/home/rdadmin/Agent/bin/rdagent
    mp_int32 Init(mp_char* pszBinFilePath);
    //SetRootPath提供通过AgentRoot路径支持初始化的方式
    mp_void SetRootPath(mp_string& strRootPath)
    {
        m_strAgentRootPath = strRootPath;
    }
    //获取agent路径，如安装路径为/home/rdagent/agent，则返回/home/rdagent/agent
    mp_string GetRootPath()
    {
        return m_strAgentRootPath;
    }
    //获取agent安装路径下的bin文件夹路径
    mp_string GetBinPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR;
    }
    //获取插件路径
    mp_string GetPluginsPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR + PATH_SEPARATOR + AGENT_PLUGIN_DIR;
    }
    //获取agent安装路径下的conf文件夹路径
    mp_string GetConfPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_CONF_DIR;
    }
    //获取agent安装路径下的log文件夹路径
    mp_string GetLogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LOG_DIR;
    }
    //获取agent安装路径下的tmp文件夹路径
    mp_string GetTmpPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_TMP_DIR;
    }
    //获取agent安装路径下的thirdparty文件夹路径
    mp_string GetThirdPartyPath()
    {
        return GetBinPath() + PATH_SEPARATOR + AGENT_THIRDPARTY_DIR;
    }
    //获取agent安装路径下的DB文件夹路径
    mp_string GetDbPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_DB;
    }
    //获取agent安装路径下bin文件夹下nginx子文件夹路径
    mp_string GetNginxPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR + PATH_SEPARATOR + AGENT_NGINX;
    }
    //获取agent安装路径下的bin文件夹下某个文件路径
    mp_string GetBinFilePath(mp_string strFileName)
    {
        return GetBinPath() + PATH_SEPARATOR + strFileName;
    }
    //获取agent安装路径下的conf文件夹下某个文件路径
    mp_string GetConfFilePath(mp_string strFileName)
    {
        return GetConfPath() + PATH_SEPARATOR + strFileName;
    }
    //获取agent安装路径下的log文件夹下某个文件路径
    mp_string GetLogFilePath(mp_string strFileName)
    {
        return GetLogPath() + PATH_SEPARATOR + strFileName;
    }
    //获取agent安装路径下的tmp文件夹下某个文件路径
    mp_string GetTmpFilePath(mp_string strFileName)
    {
        return GetTmpPath() + PATH_SEPARATOR + strFileName;
    }
    //获取agent安装路径下的DB文件夹下某个文件路径
    mp_string GetDbFilePath(mp_string strFileName)
    {
        return GetDbPath() + PATH_SEPARATOR + strFileName;
    }
    //获取agent安装路径下的thirdparty文件夹下某个文件路径
    mp_string GetThirdPartyFilePath(mp_string strFileName)
    {
        return GetThirdPartyPath() + PATH_SEPARATOR + strFileName;
    }
    //获取nginx目录下某个文件的路径
    mp_string GetNginxFilePath(mp_string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + strFileName;
    }
    //获取gninx logs目录下某个文件的路径
    mp_string GetNginxLogsFilePath(mp_string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + AGENT_NGINX_LOGS + PATH_SEPARATOR + strFileName;
    }
    //获取gninx conf目录下某个文件的路径
    mp_string GetNginxConfFilePath(mp_string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + AGENT_NGINX_CONF + PATH_SEPARATOR + strFileName;
    }

private:
    CPath()
    {
    }

private:
    static CPath m_instance;   //单例对象
    mp_string m_strAgentRootPath;
};

#endif //__AGENT_PATH_H__

