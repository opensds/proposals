/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_CONFIG_XML_PARSE_H__
#define __AGENT_CONFIG_XML_PARSE_H__

#include "tinyxml.h"
#include "common/Types.h"
#include <common/Thread.h>
#include <map>
#include <vector>

//agent_cfg.xml 配置文件字段名称
#define CFG_SYSTEM_SECTION       "System"
#define CFG_PORT                 "port"
#define CFG_LOG_LEVEL            "log_level"
#define CFG_LOG_COUNT            "log_count"
#define CFG_LOG_CACHE_THRESHOLD  "log_cache_threshold"
#define CFG_USER_NAME            "name"
#define CFG_SALT_VALUE           "sl"
#define CFG_HASH_VALUE           "hash"
#define CFG_MONITOR_SECTION      "Monitor"
#define CFG_RETRY_TIME           "retry_time"
#define CFG_MONITOR_INTERVAL     "monitor_interval"
#define CFG_RDAGENT_SECTION      "rdagent"
#define CFG_NGINX_SECTION        "nginx"
#define CFG_THREAD_COUNT         "monitor_interval"
#define CFG_HANDLE_COUNT         "handle_count"
#define CFG_PM_SIZE              "pm_size"
#define CFG_VM_SIZE              "vm_size"
#define CFG_CPU_USAGE            "cpu_usage"
#define CFG_TMP_FILE_SIZE        "tmpfile_size"
#define CFG_SNMP_SECTION         "SNMP"
#define CFG_PRIVATE_PASSWOD      "private_password"
#define CFG_PRIVATE_PROTOCOL     "private_protocol"
#define CFG_AUTH_PASSWORD        "auth_password"
#define CFG_AUTH_PROTOCOL        "auth_protocol"
#define CFG_SECURITY_NAME        "security_name"
#define CFG_CONTEXT_NAME         "context_name"
#define CFG_ENGINE_ID            "engine_id"
#define CFG_SECURITY_LEVEL       "security_level"
#define CFG_SECURITY_MODEL       "security_model"

typedef map<mp_string, mp_string> NodeValue;

class AGENT_API CConfigXmlParser
{
public:
    static CConfigXmlParser& GetInstance()
    {
        return m_instance;
    }
    ~CConfigXmlParser()
    {
        CMpThread::DestroyLock(&m_cfgValueMutexLock);
        if (m_pDoc != NULL)
        {
            delete m_pDoc;
            m_pDoc = NULL;
        }
    }

    mp_int32 Init(mp_string strCfgFilePath);
    mp_bool IsModified();
    mp_int32 GetValueString(mp_string strSection, mp_string strKey, mp_string& strValue);
    mp_int32 GetValueBool(mp_string strSection, mp_string strKey, mp_bool& bValue);
    mp_int32 GetValueInt32(mp_string strSection, mp_string strKey, mp_int32& iValue);
    mp_int32 GetValueInt64(mp_string strSection, mp_string strKey, mp_int64& lValue);
    mp_int32 GetValueFloat(mp_string strSection, mp_string strKey, mp_float& fValue);
    mp_int32 GetValueString(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_string& strValue);
    mp_int32 GetValueBool(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_bool& bValue);
    mp_int32 GetValueInt32(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_int32& iValue);
    mp_int32 GetValueInt64(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_int64& lValue);
    mp_int32 GetValueFloat(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_float& fValue);

    mp_int32 SetValue(mp_string strSection, mp_string strKey, mp_string strValue);
    mp_int32 SetValue(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_string strValue);

private:
    CConfigXmlParser()
    {
        CMpThread::InitLock(&m_cfgValueMutexLock);
        m_pDoc = NULL;
        m_lastTime = 0;
    }
    mp_int32 Load();
    mp_void ParseNodeValue(TiXmlElement *pCfgSec, NodeValue& nodeValue);
    TiXmlElement* GetChildElement(TiXmlElement* pParentElement, mp_string strSection);

private:
    static CConfigXmlParser m_instance;  //单例对象
    mp_string m_strCfgFilePath;
    mp_time m_lastTime;  //加载xml文件的上次修改时间
    thread_lock_t m_cfgValueMutexLock; //m_cfgValue访问互斥锁
    TiXmlDocument* m_pDoc;
};

#endif //__AGENT_CONFIG_XML_PARSE_H__

