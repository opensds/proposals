/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifdef WIN32
#include "plugins/exchange/ExchangePlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "common/Path.h"
#include "common/Defines.h"

REGISTER_PLUGIN(CExchangePlugin);
CExchangePlugin::CExchangePlugin()
{
    REGISTER_ACTION(REST_EXCHANGE_QUERY_DB_INFO, REST_URL_METHOD_GET, &CExchangePlugin::QueryInfo);
    REGISTER_ACTION(REST_EXCHANGE_QUERY_LUN_INFO, REST_URL_METHOD_GET, &CExchangePlugin::QueryLunInfo);
    REGISTER_ACTION(REST_EXCHANGE_START, REST_URL_METHOD_PUT, &CExchangePlugin::Start);
    REGISTER_ACTION(REST_EXCHANGE_DISMOUNT, REST_URL_METHOD_PUT, &CExchangePlugin::Dismount);
    REGISTER_ACTION(REST_EXCHANGE_CLEAR, REST_URL_METHOD_PUT, &CExchangePlugin::Clear);
    REGISTER_ACTION(REST_EXCHANGE_HOSTCONTROLLER_START, REST_URL_METHOD_PUT, &CExchangePlugin::StartHostContrService);
    REGISTER_ACTION(REST_EXCHANGE_HOSTCONTROLLER_STOP, REST_URL_METHOD_PUT, &CExchangePlugin::StopHostContrService);
    REGISTER_ACTION(REST_EXCHANGE_FREEZE_DB, REST_URL_METHOD_PUT, &CExchangePlugin::Freeze);
    REGISTER_ACTION(REST_EXCHANGE_UNFREEZE_DB, REST_URL_METHOD_PUT, &CExchangePlugin::UnFreeze);
    REGISTER_ACTION(REST_EXCHANGE_GET_FREEZE_STAT, REST_URL_METHOD_GET, &CExchangePlugin::QueryFreezeState);
}

CExchangePlugin::~CExchangePlugin()
{
}
/*------------------------------------------------------------ 
Description  :Exchange组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 输入信息
Output       : rsp -- 返回信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CExchangePlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CExchangePlugin, req, rsp);
}

/*------------------------------------------------------------ 
Description  : 查询数据库应用信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::QueryInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector < exchange_db_info_t > vecExchangeDbInfo;
    vector < exchange_db_info_t >::iterator iter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query exchange database info.");

    iRet = m_exchange.GetInfo(vecExchangeDbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Exchange db info failed, iRet %d", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    for (iter = vecExchangeDbInfo.begin(); iter != vecExchangeDbInfo.end(); ++iter)
    {
        Json::Value jValue;
        jValue[REST_PARAM_EXCHANGE_ISCOMM] = iter->iIsCommon;
        jValue[REST_PARAM_EXCHANGE_STATE] = iter->iMountState;
        jValue[REST_PARAM_EXCHANGE_VERSION] = iter->iVersion;
        jValue[REST_PARAM_EXCHANGE_DB_NAME] = iter->strDbName;
        jValue[REST_PARAM_EXCHANGE_EDBPATH] = iter->strEdbPath;
        jValue[REST_PARAM_EXCHANGE_LOGPATH] = iter->strLogPath;
        jValue[REST_PARAM_EXCHANGE_STRGRP] = iter->strStorageGroup;
        jValue[REST_PARAM_EXCHANGE_SYSPATH] = iter->strSystemPath;
        val.append(jValue);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query Exchange info succ.");
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 查询数据库所在的Lun信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::QueryLunInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query exchange lun info.");
    ex_querlun_input_info_t queryluninputinfo;
    vector<exchange_lun_info_t> exchluninfo;
    vector<exchange_lun_info_t>::iterator iterinfo;

    CRequestURL& vrequrl= req->GetURL();

    queryluninputinfo.strDbNames = vrequrl.GetSpecialQueryParam(REST_PARAM_EXCHANGE_DBNAME);
    queryluninputinfo.strVersion = vrequrl.GetSpecialQueryParam(REST_PARAM_EXCHANGE_VERSION);
    queryluninputinfo.strStorageGroup = vrequrl.GetSpecialQueryParam(REST_PARAM_EXCHANGE_STRGRP);

    //参数校验
    mp_string strInclude("123");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamString(queryluninputinfo.strVersion, 1, 1, strInclude, strExclude));
    strInclude = mp_string("");
    strExclude = mp_string("\\/=;,");
    if("1" == queryluninputinfo.strVersion)
    {
        CHECK_FAIL_EX(CheckParamString(queryluninputinfo.strStorageGroup, 1, 64, strInclude, strExclude));
    }
    mp_int32 iRet = m_exchange.GetLunInfo(queryluninputinfo, exchluninfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Exchange lun info failed, iRet %d", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    for (iterinfo = exchluninfo.begin(); iterinfo != exchluninfo.end(); ++iterinfo)
    {
        Json::Value jValue;
        jValue[REST_PARAM_EXCHANGE_LUNID] = iterinfo->strLunId;
        jValue[REST_PARAM_EXCHANGE_ARRAYSN] = iterinfo->strArraySn;
        jValue[REST_PARAM_EXCHANGE_DEVNAME] = iterinfo->strDevName;
        jValue[REST_PARAM_EXCHANGE_WWN] = iterinfo->strWwn;
        jValue[REST_PARAM_EXCHANGE_VOLNAME] = iterinfo->strVolName;
        jValue[REST_PARAM_EXCHANGE_LBA] = iterinfo->strLba;
        val.append(jValue);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query Exchange lun info succ.");
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 启动数据库
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::Start(CRequestMsg* req, CResponseMsg* rsp)
{
    exchange_param_t exstartparam;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start exchange.");

    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_INT32(jReqValue, REST_PARAM_EXCHANGE_VERSION, exstartparam.iVersion);
    GET_JSON_INT32(jReqValue, REST_PARAM_EXCHANGE_STARTYPE, exstartparam.iStarType);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_MASTDBNAME, exstartparam.strDbName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_MASTRGRPNAME, exstartparam.strStrGrpName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_SLAVDBNAME, exstartparam.strSlaveDbName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_SALVSTRGRPNAME, exstartparam.strSlaveStrGrpName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_MASTSERVNAME, exstartparam.strServName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_EDBPATH, exstartparam.strEdbPath);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_LOGPATH, exstartparam.strLogPath);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_SYSPATH, exstartparam.strSysPath);

    //参数校验
    vector<mp_int32> vecExclude;
    mp_string strInclude("");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamInteger32(exstartparam.iVersion, 1, 3, vecExclude));
    strInclude = mp_string("");
    strExclude = mp_string("\\/=;,");
    if(1 == exstartparam.iVersion)
    {
        CHECK_FAIL_EX(CheckParamString(exstartparam.strStrGrpName, 1, 64, strInclude, strExclude));
        CHECK_FAIL_EX(CheckParamString(exstartparam.strSlaveStrGrpName, 1, 64, strInclude, strExclude));
    }
    strInclude = mp_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-");
    strExclude = mp_string("");
    CHECK_FAIL_EX(CheckParamString(exstartparam.strServName, 1, 15, strInclude, strExclude));
    strInclude = mp_string("");
    strExclude = mp_string("/*?\"<>|");
    CHECK_FAIL_EX(CheckParamString(exstartparam.strEdbPath, 1, 254, strInclude, strExclude));    
    mp_string strEnd(".edb");
    CHECK_FAIL_EX(CheckParamStringEnd(exstartparam.strEdbPath, 1, 254, strEnd));
    CHECK_FAIL_EX(CheckParamString(exstartparam.strLogPath, 1, 254, strInclude, strExclude)); 
    if(1 == exstartparam.iVersion)
    {
        CHECK_FAIL_EX(CheckParamString(exstartparam.strSysPath, 1, 254, strInclude, strExclude)); 
    }
    CHECK_FAIL_EX(CheckParamInteger32(exstartparam.iStarType, 0, 2, vecExclude));
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,
        "Input param Version:%d, starType:%d, masterDb:%s, masterStrGrp:%s, slaveDb:%s, slaveStrGrp:%s, masterServer:%s.",
        exstartparam.iVersion, exstartparam.iStarType, exstartparam.strDbName.c_str(), exstartparam.strStrGrpName.c_str(),
        exstartparam.strSlaveDbName.c_str(), exstartparam.strSlaveStrGrpName.c_str(), exstartparam.strServName.c_str());

    iRet = m_exchange.Start(exstartparam);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start exchange database %s failed, iRet %d.",
            exstartparam.strSlaveDbName.c_str(), iRet);
        rsp->SetRetCode(iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End start exchange database %s", exstartparam.strSlaveDbName.c_str());
    return iRet;

}

/*------------------------------------------------------------ 
Description  : 去挂载数据库
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::Dismount(CRequestMsg* req, CResponseMsg* rsp)
{
    exchange_param_t exstopparam;
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iVersion = 0;
    mp_string strDbName, strStrGrpName, strServName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to dismount exchange.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_INT32(jReqValue, REST_PARAM_EXCHANGE_VERSION, iVersion);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_DBNAME, strDbName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_STRGRP, strStrGrpName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_SERNAME, strServName);

    exstopparam.iVersion = iVersion;
    exstopparam.strDbName = strDbName;
    exstopparam.strServName = strServName;
    exstopparam.strStrGrpName = strStrGrpName;

    //参数校验
    vector<mp_int32> vecExclude;
    mp_string strInclude("");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamInteger32(exstopparam.iVersion, 1, 3, vecExclude));
    strInclude = mp_string("");
    strExclude = mp_string("\\/=;,");
    if(1 == exstopparam.iVersion)
    {
        CHECK_FAIL_EX(CheckParamString(exstopparam.strStrGrpName, 1, 64, strInclude, strExclude));
    }
    strInclude = mp_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-");
    strExclude = mp_string("");
    CHECK_FAIL_EX(CheckParamString(exstopparam.strServName, 1, 15, strInclude, strExclude));

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Input param Version:%d, dbname:%s, storageGroup:%s, serverName:%s.",
        exstopparam.iVersion, exstopparam.strDbName.c_str(), exstopparam.strStrGrpName.c_str(),  exstopparam.strServName.c_str());

    iRet = m_exchange.Stop(exstopparam, OPER_TYPE_DISMOUNT);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Dismount exchange database %s failed, iRet %d.",
            exstopparam.strDbName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End dismount exchange database %s", exstopparam.strDbName.c_str());

    return iRet;
}

/*------------------------------------------------------------ 
Description  : 移除数据库
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::Clear(CRequestMsg* req, CResponseMsg* rsp)
{
    exchange_param_t exstopparam;
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iVersion = 0;
    mp_string strDbName, strStrGrpName, strServName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to clear exchange.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();

    GET_JSON_INT32(jReqValue, REST_PARAM_EXCHANGE_VERSION, iVersion);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_DBNAME, strDbName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_STRGRP, strStrGrpName);
    GET_JSON_STRING(jReqValue, REST_PARAM_EXCHANGE_SERNAME, strServName);

    exstopparam.iVersion = iVersion;
    exstopparam.strDbName = strDbName;
    exstopparam.strServName = strServName;
    exstopparam.strStrGrpName = strStrGrpName;

    //参数校验
    vector<mp_int32> vecExclude;
    mp_string strInclude("");
    mp_string strExclude("");
    CHECK_FAIL_EX(CheckParamInteger32(exstopparam.iVersion, 1, 3, vecExclude));
    strInclude = mp_string("");
    strExclude = mp_string("\\/=;,");
    if(1 == exstopparam.iVersion)
    {
        CHECK_FAIL_EX(CheckParamString(exstopparam.strStrGrpName, 1, 64, strInclude, strExclude));
    }
    strInclude = mp_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-");
    strExclude = mp_string("");
    CHECK_FAIL_EX(CheckParamString(exstopparam.strServName, 1, 15, strInclude, strExclude));

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Input param Version:%d, dbname:%s, storageGroup:%s, serverName:%s.",
        exstopparam.iVersion, exstopparam.strDbName.c_str(), exstopparam.strStrGrpName.c_str(),  exstopparam.strServName.c_str());

    iRet = m_exchange.Stop(exstopparam, OPER_TYPE_CLEAR);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Clear exchange database %s failed, iRet %d.",
            exstopparam.strDbName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Clear exchange database %s succ.", exstopparam.strDbName.c_str());

    return iRet;
}

/*------------------------------------------------------------ 
Description  : 启动Exchange2013 HostContrService服务
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::StartHostContrService(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 OperTyp = OPER_START_SERVICE;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start exchange 2013 service.");
    iRet = m_exchange.OperHostContorllerService(OperTyp);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start Exchange 2013 Service failed, iRet %d.", iRet);

        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start exchange 2013 service succ.");

    return iRet;
}

/*------------------------------------------------------------ 
Description  : 停止Exchange2013 HostContrService服务
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::StopHostContrService(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 OperTyp = OPER_STOP_SERVICE;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin stop exchange 2013 service.");
    iRet = m_exchange.OperHostContorllerService(OperTyp);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop Exchange 2013 Service failed, iRet %d.", iRet);

        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop exchange 2013 service succ.");

    return iRet;
}

/*------------------------------------------------------------ 
Description  : 冻结数据库
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::Freeze(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    vector<exchange_freeze_info_t> vecFreezInfos;
    exchange_freeze_info_t freezInfo;
    mp_string strDriveLetter;
    mp_uint32 uiDriveLetterCount = 0;
    mp_uint32 uiDBCount = 0;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze exchange.");
    CHECK_JSON_ARRAY(jReqValue);
    uiDBCount = jReqValue.size();
    mp_string strInclude("");
    mp_string strExclude("");
    for (mp_uint32 i = 0; i < uiDBCount; i++)
    {
        const Json::Value& jvTmp = jReqValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_EXCHANGE_DB_NAME, freezInfo.strDBName);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get db name %s.", freezInfo.strDBName.c_str());
        CHECK_JSON_VALUE(jvTmp, REST_PARAM_EXCHANGE_DISK_NAMES);
        CHECK_JSON_ARRAY(jvTmp[REST_PARAM_EXCHANGE_DISK_NAMES]);
        
        uiDriveLetterCount = jvTmp[REST_PARAM_EXCHANGE_DISK_NAMES].size();
        strInclude = mp_string("BCDEFGHIJKLMNOPQRSTUVWXYZ");
        for (mp_uint32 j = 0; j < uiDriveLetterCount; j++)
        {
            strDriveLetter = jvTmp[REST_PARAM_EXCHANGE_DISK_NAMES][j].asString();
            CHECK_FAIL_EX(CheckParamString(strDriveLetter, 1, 1, strInclude, strExclude));
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive letter %s.", strDriveLetter.c_str());
            freezInfo.vecDriveLetters.push_back(strDriveLetter);
        }
        vecFreezInfos.push_back(freezInfo);
    }

    iRet = m_exchange.Freeze(vecFreezInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze exchange failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze exchange succ.");
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 解冻数据库
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::UnFreeze(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    vector<exchange_unfreeze_info_t> vecUnfreezInfos;
    exchange_unfreeze_info_t unfreezInfo;
    mp_string strDriveLetter;
    mp_uint32 uiDriveLetterCount = 0;
    mp_uint32 uiDBCount = 0;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin unfreeze exchange.");
    CHECK_JSON_ARRAY(jReqValue);
    uiDBCount = jReqValue.size();
    mp_string strInclude("");
    mp_string strExclude("");
    for (mp_uint32 i = 0; i < uiDBCount; i++)
    {
        const Json::Value& jvTmp = jReqValue[i];
        GET_JSON_STRING(jvTmp, REST_PARAM_EXCHANGE_DB_NAME, unfreezInfo.strDBName);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get db name %s.", unfreezInfo.strDBName.c_str());
        CHECK_JSON_VALUE(jvTmp, REST_PARAM_EXCHANGE_DISK_NAMES);
        CHECK_JSON_ARRAY(jvTmp[REST_PARAM_EXCHANGE_DISK_NAMES]);
        
        uiDriveLetterCount = jvTmp[REST_PARAM_EXCHANGE_DISK_NAMES].size();
        strInclude = mp_string("BCDEFGHIJKLMNOPQRSTUVWXYZ");
        for (mp_uint32 j = 0; j < uiDriveLetterCount; j++)
        {
            strDriveLetter = jvTmp[REST_PARAM_EXCHANGE_DISK_NAMES][j].asString();
            CHECK_FAIL_EX(CheckParamString(strDriveLetter, 1, 1, strInclude, strExclude));
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive letter %s.", strDriveLetter.c_str());
            unfreezInfo.vecDriveLetters.push_back(strDriveLetter);
        }
        vecUnfreezInfos.push_back(unfreezInfo);
    }

    iRet = m_exchange.UnFreeze(vecUnfreezInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unfreeze exchange failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unfreeze exchange succ.");
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 查询数据库冻结状态
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchangePlugin::QueryFreezeState(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iStat;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query exchange freeze stat.");
    iStat = m_exchange.GetFreezeStat();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Exchange freeze stat is %d.", iStat);

    Json::Value& jValue= rsp->GetJsonValueRef();
    jValue[REST_PARAM_EXCHANGE_FREEZE_STAT] = iStat;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query exchange freeze stat:%d.", iStat);
    return MP_SUCCESS;
}
#endif

