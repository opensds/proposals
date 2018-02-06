/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_IPLUGIN_H_
#define _AGENT_IPLUGIN_H_

#include "common/Types.h"

class IPluginManager;

///////////////////////////////////////////////////////////////////////////
//基础插件接口定义
class IPlugin
{
public:
    typedef struct {
        mp_void* Creator;           //Creator函数指针
        const mp_char* ClassName;   //类名字
    }DYN_CLASS;
    
public:
    //插件初始化方法
    //param [in]  pMgr  PluginManager对象指针
    virtual mp_void Initialize(IPluginManager* pMgr) = 0;  //初始化方法

    //卸载并销毁
    virtual mp_int32 Destroy() = 0;
  
    //设置插件运行环境
    //[in]  pszName  参数名称
    //[in]  pvVal    参数指针
    //插件运行环境设置方法，插件运行所需要的资源、配置参数
    //通过这个方法“Push”到插件，这个方法先于initialize()方法
    virtual mp_void SetOption(const mp_char* pszName, mp_void* pvVal) = 0;
    
    //获取插件配置的方法，可以不实现，仅用于后续扩展
    //[in]  pszName  参数名称
    //[out] pvVal    参数指针
    //[in]  sz       参数Buffer长度
    //retval true  参数有效
    //retval false 不支持该参数
    virtual mp_bool GetOption(const mp_char* pszName, mp_void* pvVal, mp_int32 sz) = 0;
    
    //根据输入类名创建一个对象（组件 or 具体实现）
    //[in]  pszName  对象类名
    //retval  !=NULL 成功创建的对象的指针
    //retval  NULL   创建对象失败
    virtual mp_void* CreateObject(const mp_char* pszName) = 0;

    //获取插件中“发布”的类信息。
    //[out]  pClasses  存放类定义的Buffer
    //[in]   sz        Buffer长度
    //return  插件内支持动态创建的类的个数。
    //返回值为类的个数，如果入参内存不足，则返回-1，同时填充pClassses
    virtual mp_int32 GetClasses(IPlugin::DYN_CLASS *pClasses, mp_int32 sz) = 0;

    //获取加载插件的名称
    virtual const mp_char* GetName() = 0;

    //获取加载插件的版本
    virtual const mp_char* GetVersion() = 0;
    
    //获取加载插件的SCN
    virtual mp_size GetSCN() = 0;
    
    //获取插件类型
    virtual mp_int32 GetTypeId() const  { return 0; }
    
protected:
    virtual ~IPlugin() {}
};


class IPluginCallback
{
public:
    //插件卸载前回调的方法，判断该插件是否可以卸载
    //[in]  pOldPlg 插件接口
    //retval true  可以卸载
    //retval false 不可以卸载
    virtual mp_bool CanUnload(IPlugin* pOldPlg) = 0;

    //新插件加载通知函数
    //[in] pOldPlg 旧插件接口
    //[in] pNewPlg 新插件接口
    //当新插件加载成功后，PluginManager会回调这个函数，
    //相关模块可以做相应处理     
    virtual mp_void OnUpgraded(IPlugin* pOldPlg, IPlugin* pNewPlg) = 0;

    //设置插件选项的接口
    //[in]  plg  插件接口
    //当新插件加载成功后，PluginManager会回调这个函数
    virtual mp_void SetOptions(IPlugin* plg) = 0;

    //获取lib的发布版本
    //[in]  pszLib  lib名字，注意不包含后缀（.so or .dll）
    //[out] pszVer  版本
    //[in] sz       pszVer的长度
    //retval NULL   没有版本信息
    //retval !=NULL 版本信息
    virtual mp_char* GetReleaseVersion(const mp_char* pszLib, mp_char* pszVer, mp_size sz) = 0;

protected:
    virtual ~IPluginCallback() {}
};


//PluginManager接口定义
class IPluginManager
{
public:
    //初始化函数
    //[in]  pCallback 回调钩子
    virtual mp_void Initialize(IPluginCallback* pCallback) = 0;
    
    //销毁
    //子类可以实现一个“假”销毁的函数
    virtual mp_void Destroy() = 0;

    //获取系统编号序号
    //return  返回scn（system change number）
    //每个Plugin加载一次scn就应该加1
    virtual mp_size GetSCN() = 0;
    
    //根据名字获取插件接口
    //[in]  pszPlg 插件名称
    //retval != NULL  返回插件接口
    //retval NULL     插件还没有加载
    virtual IPlugin* GetPlugin(const mp_char* pszPlg) = 0;
    
    virtual IPlugin* LoadPlugin(const mp_char* pszPlg) = 0;
    
    virtual mp_void UnloadPlugin(const mp_char* pszPlg) = 0;

    //检查已经加载的插件需要升级，如果需要则重新Load插件
    //return  成功升级的插件的个数。
    virtual mp_int32 Upgrade() = 0;

protected:
    virtual ~IPluginManager() {}
};

#endif //_AGENT_IPLUGIN_H_

