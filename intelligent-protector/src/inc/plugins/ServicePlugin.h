/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_SERVICE_PLUGIN_H_
#define _AGENT_SERVICE_PLUGIN_H_

#include "pluginfx/IPlugin.h"
#include "common/Types.h"
#include "rest/MessageProcess.h"
#include "common/Log.h"
#include <map>

template <class T>
class CRestActionMap
{
public:
    typedef mp_int32(T::*ACT)(CRequestMsg*, CResponseMsg*);
    typedef struct tag_rest_action
    {
        mp_string url;
        mp_string method;
        ACT action;
    } rest_action_t;
    typedef map<mp_string, rest_action_t> ACTIONS_MAP;
    
private:
    ACTIONS_MAP m_mapActions;

public:  
    CRestActionMap(){};
    ~CRestActionMap(){};

    mp_void Add(const mp_char* pszUrl, const mp_char* pszHttpMethod, ACT act)
    {
        rest_action_t restAction;
        mp_string strKey;

        strKey = pszUrl;
        strKey += "_";
        strKey += pszHttpMethod;
        restAction.url = pszUrl;
        restAction.method = pszHttpMethod;
        restAction.action = act;
        m_mapActions[strKey] = restAction;
    }
    
    mp_int32 GetAction(mp_string& strUrl, mp_string strMethod, rest_action_t& restAction)
    {
        mp_string strKey;
        strKey = strUrl + "_" + strMethod;
        typename map<mp_string, rest_action_t>::iterator iter = m_mapActions.find(strKey);
        if(iter == m_mapActions.end())
        {
            return MP_FAILED;
        }

        restAction = iter->second;
        return MP_SUCCESS;
    }

    mp_void PrintMap()
    {
        typename map<mp_string, rest_action_t>::iterator iter;

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Print cached actions.");
        for (iter = m_mapActions.begin(); iter != m_mapActions.end(); iter++)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Key %s, url %s, method %s.", iter->first.c_str(), 
                iter->second.url.c_str(), iter->second.method.c_str());
        }
    }
};

//业务插件基础类，具体插件实现继承该类
class CServicePlugin : public IPlugin
{
public:   
    enum
    {
        APP_PUGIN_ID = 2001
    };
    
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //插件框架动态升级功能预留
    ////////////////////////////////////////////////////////////////////////////////////////////////
    mp_int32 GetTypeId() const {return APP_PUGIN_ID;}
    mp_void Initialize(IPluginManager* pMgr);
    mp_int32 Destroy();
    mp_void SetOption(const mp_char* pszName, mp_void* pvVal);
    mp_bool GetOption(const mp_char* pszName, mp_void* pvVal, mp_int32 sz);
    mp_void* CreateObject(const mp_char* pszName);
    mp_int32 GetClasses(IPlugin::DYN_CLASS *pClasses, mp_int32 sz);
    const mp_char* GetName();
    const mp_char* GetVersion();
    mp_size GetSCN();
    ////////////////////////////////////////////////////////////////////////////////////////////////

    //调用插件，执行请求
    virtual mp_int32 Invoke(CRequestMsg* req, CResponseMsg* rsp);
    //具体插件实现类需要实现该方法
    virtual mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp) = 0;
};


////////////////////////////////////////////////////////////////////////////////////////////////
//插件注册宏定义
////////////////////////////////////////////////////////////////////////////////////////////////
//注册插件
#define REGISTER_PLUGIN(clsname) \
    extern "C" AGENT_EXPORT IPlugin* QueryInterface() \
    { \
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create new plugin obj %s.", #clsname); \
        return new clsname(); \
    }\
    CRestActionMap<clsname> restActionMap;

//注册插件Action
#define REGISTER_ACTION(url, httpmethod, act) \
    restActionMap.Add(url, httpmethod, act);

//获取注册的Action
#define GET_REQUEST_ACTION(url, http_method, restaction) \
    { \
        mp_int32 iRetx = restActionMap.GetAction(url, http_method, restaction);\
        if (MP_SUCCESS != iRetx) \
        { \
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unimplement action, url %s, http method %s.", url.c_str(), \
                http_method.c_str()); \
            return ERROR_COMMON_FUNC_UNIMPLEMENT; \
        }\
    }

//执行特定Action
#define DO_ACTION(clsname, req, rsp) \
    {\
        mp_int32 iRet = MP_SUCCESS; \
        mp_string strUrl; \
        mp_string strMethod; \
        CRestActionMap<clsname>::rest_action_t restAction; \
        \
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin do rest action.");\
        restActionMap.PrintMap();\
        strUrl = req->GetURL().GetProcURL();\
        strMethod = req->GetHttpReq().GetMethod();\
        GET_REQUEST_ACTION(strUrl, strMethod, restAction);\
        iRet = (this->*restAction.action)(req, rsp);\
        if (MP_SUCCESS != iRet)\
        {\
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Do rest action failed, url %s, http method %s, iRet %d.", \
                strUrl.c_str(), strMethod.c_str(), iRet);\
            return iRet;\
        }\
        \
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Do rest action succ.");\
        return MP_SUCCESS;\
    }
    
#endif //_AGENT_SERVICE_PLUGIN_H_

