/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/host/HostPlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "rest/MessageProcess.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/RootCaller.h"
#include "common/Defines.h"
#include "common/String.h"

REGISTER_PLUGIN(CHostPlugin); //lint !e19
CHostPlugin::CHostPlugin()
{
    //host
    REGISTER_ACTION(REST_HOST_QUERY_AGENT_VERSION, REST_URL_METHOD_GET, &CHostPlugin::QueryAgentVersion);
    REGISTER_ACTION(REST_HOST_QUERY_INFO, REST_URL_METHOD_GET, &CHostPlugin::QueryHostInfo);
    REGISTER_ACTION(REST_HOST_QUERY_INITIATOR, REST_URL_METHOD_GET, &CHostPlugin::QueryInitiators);
    REGISTER_ACTION(REST_HOST_SCAN_DISK, REST_URL_METHOD_PUT, &CHostPlugin::ScanDisk);
    REGISTER_ACTION(REST_HOST_QUERY_DISKS, REST_URL_METHOD_GET, &CHostPlugin::QueryDiskInfo);
    REGISTER_ACTION(REST_HOST_QUERY_TIMEZONE, REST_URL_METHOD_GET, &CHostPlugin::QueryTimeZone);
    REGISTER_ACTION(REST_HOST_COLLECT_LOG, REST_URL_METHOD_POST, &CHostPlugin::CollectAgentLog);
    REGISTER_ACTION(REST_HOST_EXPORT_LOG, REST_URL_METHOD_POST, &CHostPlugin::ExportAgentLog);
    //thirdpart script
    REGISTER_ACTION(REST_HOST_THIRDPARTY_QUERY_FILE_INFO, REST_URL_METHOD_GET, &CHostPlugin::QueryThirdPartyScripts);
    REGISTER_ACTION(REST_HOST_THIRDPARTY_EXEC_FILE, REST_URL_METHOD_POST, &CHostPlugin::ExecThirdPartyScript);
    REGISTER_ACTION(REST_HOST_FREEZE_SCRIPT, REST_URL_METHOD_PUT, &CHostPlugin::ExecFreezeScript);
    REGISTER_ACTION(REST_HOST_UNFREEZE_SCRIPT, REST_URL_METHOD_PUT, &CHostPlugin::ExecThawScript);
    REGISTER_ACTION(REST_HOST_QUERY_STATUS_SCRIPT, REST_URL_METHOD_GET, &CHostPlugin::QueryFreezeStatusScript);
    //snmp
    REGISTER_ACTION(REST_HOST_REG_TRAP_SERVER, REST_URL_METHOD_POST, &CHostPlugin::RegTrapServer);
    REGISTER_ACTION(REST_HOST_UNREG_TRAP_SERVER, REST_URL_METHOD_POST, &CHostPlugin::UnRegTrapServer);
    REGISTER_ACTION(REST_HOST_VERIFY_SNMP, REST_URL_METHOD_PUT, &CHostPlugin::VerifySnmp);
#ifdef WIN32
    REGISTER_ACTION(REST_HOST_ONLINE, REST_URL_METHOD_PUT, &CHostPlugin::DeviceOnline);
    REGISTER_ACTION(REST_HOST_BATCH_ONLINE, REST_URL_METHOD_PUT, &CHostPlugin::DeviceBatchOnline);
    REGISTER_ACTION(REST_HOST_QUERY_PARTITIONS, REST_URL_METHOD_GET, &CHostPlugin::QueryPartisions);
#endif
}

CHostPlugin::~CHostPlugin()
{
}
/*------------------------------------------------------------ 
Description  :Host组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 输入信息
Output       : rsp -- 返回信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CHostPlugin, req, rsp);
}

/*------------------------------------------------------------ 
Description  : 查询第三方脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::QueryThirdPartyScripts(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<string> vectFileList;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query thirdparty files.");
    iRet = m_host.QueryThirdPartyScripts(vectFileList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query thirdparty files failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    
    for (vector<mp_string>::iterator iter = vectFileList.begin(); iter != vectFileList.end(); ++ iter)
    {
        val.append(iter->c_str());
    }
   
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query thirdparty files succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :执行三方脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::ExecThirdPartyScript(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string fileName;//第三方文件的名称
    mp_string paramValues;//脚本文件的参数值集合
    vector <string> vecResult;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin execute thirdpart script.");
    
    const Json::Value& jv = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jv, REST_PARAM_HOST_FILENAME,fileName);
    GET_JSON_STRING(jv, REST_PARAM_HOST_PARAMS, paramValues);

    //参数校验
    mp_string strExclude("\\/:*?\"<>|");
    mp_string strInclude;
    mp_int32 lenEnd = 254;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(fileName, lenBeg, lenEnd, strInclude, strExclude));
    if (CheckCmdDelimiter(paramValues) == MP_FALSE)
    {
        //打印日志
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "strCommand %s contain invalid character.", paramValues.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    mp_string strFilePath = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + 
        mp_string(AGENT_THIRDPARTY_DIR) + mp_string(PATH_SEPARATOR) + fileName;
    CHECK_FAIL_EX(CheckPathString(strFilePath));
    
    iRet = m_host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Rootcaller thirdparty script exec failed, iRet %d.", iRet);
        
        Json::Value& jRspValue= rsp->GetJsonValueRef();
        for (vector <string>::iterator it = vecResult.begin(); it != vecResult.end(); it++)
        {
            Json::Value jValue;
            jValue["errorMessage"] = *it;
            jRspValue.append(jValue);
        }
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Execute thirdpart script succ.");
    }
    return iRet;
}

/*------------------------------------------------------------ 
Description  :查询Agent版本信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::QueryAgentVersion(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strBuildNum;
    mp_string strAgentVersion;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query Agent version.");

    iRet = m_host.GetAgentVersion(strAgentVersion, strBuildNum);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query Agent version failed, iRet %d.", ERROR_COMMON_READ_CONFIG_FAILED);
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    
    Json::Value& jValue = rsp->GetJsonValueRef();
    jValue[REST_PARAM_HOST_VERSION]   = strAgentVersion;
    jValue[REST_PARAM_HOST_BUILD_NUM] = strBuildNum;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End query Agent version.");
    return iRet;
}

/*------------------------------------------------------------ 
Description  :执行客户定制的三方冻结应用脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::ExecFreezeScript(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string fileName, strScriptName;//第三方文件的名称
    mp_string paramValues;//脚本文件的参数值集合
    vector <string> vecResult;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin execute freeze script.");
    
    const Json::Value& jv = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jv, REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME, fileName);
    strScriptName = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + 
        mp_string(AGENT_THIRDPARTY_DIR) + mp_string(PATH_SEPARATOR) + fileName;
    if (!CMpFile::FileExist(strScriptName.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze file %s is not exists.", fileName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    if (jv.isMember(REST_PARAM_HOST_FREEZE_SCRIPT_PARAM))
    {
        GET_JSON_STRING(jv, REST_PARAM_HOST_FREEZE_SCRIPT_PARAM, paramValues);
    }

    // check script exists
    GET_JSON_STRING(jv, REST_PARAM_HOST_UNFREEZE_SCRIPT_FILENAME, strScriptName);
    strScriptName = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + 
        mp_string(AGENT_THIRDPARTY_DIR) + mp_string(PATH_SEPARATOR) + strScriptName;
    if (!CMpFile::FileExist(strScriptName.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Unfreeze file %s is not exists.", strScriptName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    GET_JSON_STRING(jv, REST_PARAM_HOST_QUERY_SCRIPT_FILENAME, strScriptName);
    strScriptName = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + 
        mp_string(AGENT_THIRDPARTY_DIR) + mp_string(PATH_SEPARATOR) + strScriptName;
    if (!CMpFile::FileExist(strScriptName.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query status file %s is not exists.", strScriptName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    //参数校验
    mp_string strExclude("\\/:*?\"<>|");
    mp_string strInclude;
    mp_int32 lenEnd = 254;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(fileName, lenBeg, lenEnd, strInclude, strExclude));
    if(CheckCmdDelimiter(paramValues) == MP_FALSE)
    {
        //打印日志
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "strCommand %s contain invalid character.", paramValues.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    mp_string strFilePath = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + 
        mp_string(AGENT_THIRDPARTY_DIR) + mp_string(PATH_SEPARATOR) + fileName;
    CHECK_FAIL_EX(CheckPathString(strFilePath));
    
    iRet = m_host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Exec freeze script failed, iRet %d.", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Exec freeze script succ.");
    }
    //对脚本不存在错误码做特殊处理
    return iRet;
}

/*------------------------------------------------------------ 
Description  :执行客户定制的三方解冻应用脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::ExecThawScript(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string fileName;//第三方文件的名称
    mp_string paramValues;//脚本文件的参数值集合
    vector <string> vecResult;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin execute thaw script.");
    
    const Json::Value& jv = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jv, REST_PARAM_HOST_UNFREEZE_SCRIPT_FILENAME, fileName);
    if (jv.isMember(REST_PARAM_HOST_UNFREEZE_SCRIPT_PARAM))
    {
        GET_JSON_STRING(jv, REST_PARAM_HOST_UNFREEZE_SCRIPT_PARAM, paramValues);
    }

    //参数校验
    mp_string strExclude("\\/:*?\"<>|");
    mp_string strInclude;
    mp_int32 lenEnd = 254;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(fileName, lenBeg, lenEnd, strInclude, strExclude));
    if(CheckCmdDelimiter(paramValues) == MP_FALSE)
    {
        //打印日志
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "strCommand %s contain invalid character.", paramValues.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    mp_string strFilePath = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + 
        mp_string(AGENT_THIRDPARTY_DIR) + mp_string(PATH_SEPARATOR) + fileName;
    CHECK_FAIL_EX(CheckPathString(strFilePath));
    
    iRet = m_host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Exec thaw script failed, iRet %d.", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Exec thaw script succ.");
    }
    return iRet;
}

/*------------------------------------------------------------ 
Description  :执行客户定制的查询三方应用冻结状态脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::QueryFreezeStatusScript(CRequestMsg* req, CResponseMsg * rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string fileName;//第三方文件的名称
    mp_string paramValues;//脚本文件的参数值集合
    vector <string> vecResult;
    mp_int32 iFreezeState = DB_UNKNOWN;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin execute query script.");

    fileName = req->GetURL().GetSpecialQueryParam(REST_PARAM_HOST_QUERY_SCRIPT_FILENAME);
    paramValues = req->GetURL().GetSpecialQueryParam(REST_PARAM_HOST_QUERY_SCRIPT_PARAM);

    //参数校验
    mp_string strExclude("\\/:*?\"<>|");
    mp_string strInclude;
    mp_int32 lenEnd = 254;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(fileName, lenBeg, lenEnd, strInclude, strExclude));
    if(CheckCmdDelimiter(paramValues) == MP_FALSE)
    {
        //打印日志
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "strCommand %s contain invalid character.", paramValues.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    mp_string strFilePath = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + 
        mp_string(AGENT_THIRDPARTY_DIR) + mp_string(PATH_SEPARATOR) + fileName;
    CHECK_FAIL_EX(CheckPathString(strFilePath));
    
    //第三方脚本状态通过脚本返回码返回
    iRet = m_host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Exec query script failed, iRet %d.", iRet);
        return iRet;
    }
    else
    {
        if (vecResult.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of get freeze state by scritp failed.");
            return ERROR_COMMON_OPER_FAILED;
        }

        iFreezeState = atoi(vecResult.front().c_str());
        Json::Value& jValue = rsp->GetJsonValueRef();
        jValue[REST_PARAM_HOST_THIRDPARTY_STATE] = iFreezeState;
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Exec query script succ[%d].", iFreezeState);
        return MP_SUCCESS;
    }
}

/*------------------------------------------------------------ 
Description  :查询主机信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::QueryHostInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    host_info_t hostInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query host info.");
    iRet = m_host.GetInfo(hostInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host info failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jValue = rsp->GetJsonValueRef();
    jValue[REST_PARAM_HOST_NAME]    = hostInfo.name;
    jValue[REST_PARAM_HOST_OS]      = hostInfo.os;
    jValue[REST_PARAM_HOST_SN]      = hostInfo.sn;
    jValue[REST_PARAM_HOST_VERSION] = hostInfo.version;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query host info succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :查询Lun信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::QueryDiskInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector < host_lun_info_t > vecLunInfo;
    vector < host_lun_info_t >::iterator iter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query host Lun info.");
    iRet = m_host.GetDiskInfo(vecLunInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host Lun info failed, iRet %d.", iRet);
        
        return iRet;
    }

    Json::Value& jValue = rsp->GetJsonValueRef();
    for (iter = vecLunInfo.begin(); iter != vecLunInfo.end(); ++iter)
    {
         Json::Value jInfo;
         jInfo[REST_PARAM_HOST_LUN_ID] = iter->lunId;
         jInfo[REST_PARAM_HOST_WWN] = iter->wwn;
         jInfo[REST_PARAM_HOST_ARRAY_SN] = iter->arraySn;
         jInfo[REST_PARAM_HOST_ARRAY_VENDOR] = iter->arrayVendor;
         jInfo[REST_PARAM_HOST_ARRAY_MODEL] = iter->arrayModel;
         jInfo[REST_RARAM_HOST_DEVICE_NAME] = iter->deviceName;
         jInfo[REST_PARAM_HOST_DEVICE_DISKNUM] = iter->diskNumber;
   
         jValue.append(jInfo);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query host Lun info succ.");
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :查询时区信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::QueryTimeZone(CRequestMsg* req, CResponseMsg* rsp)
{
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query host time zone info.");
    mp_int32 iRet = MP_SUCCESS;
    timezone_info_t sttimezone;
    iRet = m_host.GetTimeZone(sttimezone);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query host time zone info failed, iRet is %d.", ERROR_HOST_GET_TIMEZONE_FAILED);

        return ERROR_HOST_GET_TIMEZONE_FAILED;
    }

    Json::Value& jValue = rsp->GetJsonValueRef();
    jValue[REST_PARAM_HOST_TIMEZONE_ISDST] = sttimezone.iIsDST;
    jValue[REST_PARAM_HOST_TIMEZONE_BIAS]  = sttimezone.strTzBias;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query host time zone info succ.");

    return iRet;
    
}


/*------------------------------------------------------------ 
Description  :查询启动器
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::QueryInitiators(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string>::iterator iter;
    initiator_info_t initInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query initialtor infos.");
    iRet = m_host.GetInitiators(initInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get initiator info failed, iRet %d.", iRet);
        
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    for (iter = initInfo.fcs.begin(); iter != initInfo.fcs.end(); iter++)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Append to json, fc wwn %s.", iter->c_str());
        val[REST_PARAM_HOST_INIT_FC].append(*iter);
    }

    for (iter = initInfo.iscsis.begin(); iter != initInfo.iscsis.end(); iter++)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Append to json, iscsi iqn %s.", iter->c_str());
        val[REST_PARAM_HOST_INIT_ISCSI].append(*iter);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query initialtor infos succs.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :扫描磁盘
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::ScanDisk(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin scan disk info.");
    iRet = m_host.ScanDisk();
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Scan disk faield, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Scan disk succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :注册trap
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::RegTrapServer(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jv = req->GetMsgBody().GetJsonValueRef();
    trap_server stTrapServer;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin register trap server.");
    GET_JSON_STRING(jv, REST_PARAM_HOST_IP, stTrapServer.strServerIP);
    GET_JSON_INT32(jv, REST_PARAM_HOST_PORT, stTrapServer.iPort);
    GET_JSON_INT32(jv, REST_PARAM_HOST_SNMPTYPE, stTrapServer.iVersion);
    stTrapServer.strListenIP = req->GetHttpReq().GetHead(LISTEN_ADDR);
    if (stTrapServer.strListenIP.empty())
    {
        stTrapServer.strListenIP = UNKNOWN;
    }

    //参数校验
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamStringIsIP(stTrapServer.strServerIP));
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iPort, 0, 65535, vecExclude));
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iVersion, 1, 3, vecExclude));
    
    mp_int32 iRet = m_host.RegTrapServer(stTrapServer);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "RegTrapServer faield, iRet %d.", iRet);
    }
    else
    {
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Register trap server succ.");
    }
    return iRet;
}
/*------------------------------------------------------------ 
Description  :解除trap
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::UnRegTrapServer(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    const Json::Value& jv = req->GetMsgBody().GetJsonValueRef();
    trap_server stTrapServer;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin unregister trap server succ.");
    GET_JSON_STRING(jv, REST_PARAM_HOST_IP, stTrapServer.strServerIP);
    GET_JSON_INT32(jv, REST_PARAM_HOST_PORT, stTrapServer.iPort);
    GET_JSON_INT32(jv, REST_PARAM_HOST_SNMPTYPE, stTrapServer.iVersion);
    
    //参数校验
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamStringIsIP(stTrapServer.strServerIP));
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iPort, 0, 65535, vecExclude));
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iVersion, 1, 3, vecExclude));
    
    mp_int32 iRet = m_host.UnRegTrapServer(stTrapServer);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "RegTrapServer faield, iRet %d.", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "RegTrapServer success");
    }
    return iRet;
}
/*------------------------------------------------------------ 
Description  :校验SNMP
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::VerifySnmp(CRequestMsg* req, CResponseMsg* rsp)
{
    snmp_v3_param stParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin verify snmp.");
    //从消息头中获取数据
    stParam.strSecurityName = req->GetHttpReq().GetHead(SNMP_PROTOCOL_USER);
    stParam.strAuthPassword = req->GetHttpReq().GetHead(SNMP_AUTH_PW);
    stParam.strPrivPassword = req->GetHttpReq().GetHead(SNMP_ENCRYPT_PW);
    const Json::Value& jv = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_INT32(jv, REST_PARAM_HOST_SNMP_AUTHTYPE, stParam.iAuthProtocol); 
    GET_JSON_INT32(jv, REST_PARAM_HOST_SNMP_ENCRYPTYPE, stParam.iPrivProtocol); 

    //参数校验
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(stParam.iAuthProtocol, 1, 5, vecExclude));
    CHECK_FAIL_EX(CheckParamInteger32(stParam.iPrivProtocol, 1, 4, vecExclude));
    
    mp_int32 iRet = m_host.VerifySnmp(stParam);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "VerifySnmp faield, iRet %d.", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "VerifySnmp success");
    }

    stParam.strAuthPassword.replace(0, stParam.strAuthPassword.length(), "");
    stParam.strPrivPassword.replace(0, stParam.strPrivPassword.length(), "");
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 收集Agent日志
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::CollectAgentLog(CRequestMsg* req, CResponseMsg * rsp)
{
    mp_int32 iRet = m_host.CollectLog();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CollectLog faield, iRet %d.", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "CollectAgentLog success");
        Json::Value& jValue = rsp->GetJsonValueRef();
        jValue[REST_PARAM_HOST_LOG_EXPORT_NAME] = m_host.GetLogName(); 
    }
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 导出Agent日志
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::ExportAgentLog(CRequestMsg* req, CResponseMsg * rsp)
{
    mp_string strLogFileName;

    const Json::Value& jv = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jv, REST_PARAM_HOST_LOG_EXPORT_NAME, strLogFileName);

    //参数校验
    mp_string strExclude("\\/:*?\"<>|");
    mp_string strInclude;
    mp_int32 lenEnd = 254;
    mp_int32 lenBeg = 1;
    mp_string strEnd ="";
    
    CHECK_FAIL_EX(CheckParamString(strLogFileName, lenBeg, lenEnd, strInclude, strExclude));
    #ifdef WIN32
    strEnd = ".zip";
    CHECK_FAIL_EX(CheckParamStringEnd(strLogFileName, lenBeg, lenEnd, strEnd));
    #else
    strEnd = ".tar.gz";
    CHECK_FAIL_EX(CheckParamStringEnd(strLogFileName, lenBeg, lenEnd, strEnd));
    #endif
    mp_string strFilePath = CPath::GetInstance().GetTmpPath() + mp_string(PATH_SEPARATOR) + strLogFileName;
    CHECK_FAIL_EX(CheckPathString(strFilePath));
    
    //判断日志是否收集完成
    if (!m_host.CanDownloadLog())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Log is beening collected.");
        return ERROR_HOST_LOG_IS_BEENING_COLLECTED;
    }
    Json::Value& jValue = rsp->GetJsonValueRef();
    jValue[REST_PARAM_ATTACHMENT_NAME] = strLogFileName;
    jValue[REST_PARAM_ATTACHMENT_PATH] = CPath::GetInstance().GetTmpFilePath(strLogFileName);
    rsp->SetHttpType(CResponseMsg::RSP_ATTACHMENT_TYPE);
    return MP_SUCCESS;
}

#ifdef WIN32
/*------------------------------------------------------------ 
Description  :设备上线
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::DeviceOnline(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDiskNum;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin online device.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqValue, REST_PARAM_HOST_DISK_NUM, strDiskNum);

    //参数校验
    mp_int32 iDiskNum = atoi(strDiskNum.c_str());
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iDiskNum, 0, 25, vecExclude));
    
    iRet = m_host.DeviceOnline(strDiskNum);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Online device failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Online device succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :批量设备上线
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::DeviceBatchOnline(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin batch online device.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp->GetJsonValueRef();
    CHECK_JSON_VALUE(jReqValue, REST_PARAM_HOST_DISK_NUMS);
    CHECK_JSON_ARRAY(jReqValue[REST_PARAM_HOST_DISK_NUMS]);
    mp_uint32 uiSize = jReqValue[REST_PARAM_HOST_DISK_NUMS].size();

    vector<mp_int32> vecExclude;
    for (mp_uint32 i = 0; i < uiSize; i++)
    {
        mp_string strDiskNum = jReqValue[REST_PARAM_HOST_DISK_NUMS][i].asString();
        //参数校验
        mp_int32 iDiskNum = atoi(strDiskNum.c_str());

        CHECK_FAIL_EX(CheckParamInteger32(iDiskNum, 0, 25, vecExclude));
        
        iRet = m_host.DeviceOnline(strDiskNum);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Online device failed, disk num %s, iRet %d.", strDiskNum.c_str(),
                iRet);
            Json::Value jv;
            jv[REST_PARAM_ERROR_CODE] = iRet;
            jv[REST_PARAM_HOST_DISK_NUM] = strDiskNum;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End batch online device.");
    return iRetRst;
}

/*------------------------------------------------------------ 
Description  :查询磁盘分区
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHostPlugin::QueryPartisions(CRequestMsg* req, CResponseMsg* rsp)
{
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query  partisions.");
    vector<partitisions_info_t> partisioninfos;
    vector<partitisions_info_t>::iterator iter;
    Json::Value& jRspValue = rsp->GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query partisions.");
    mp_int32 iRet = m_host.GetPartisions(partisioninfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query  partisions failed, iRet %d.", iRet);

        return iRet;
    }

    for (iter = partisioninfos.begin(); iter != partisioninfos.end(); ++iter)
    {
        Json::Value jv;
        jv[REST_PARAM_HOST_PARTISIONNAME] = iter->strPartitionName;
        jv[REST_PARAM_HOST_CAPACITY] = iter->lCapacity;
        jv[REST_PARAM_HOST_DISKNAME] = iter->strDiskNumber;
        jv[REST_RARAM_HOST_DEVICE_NAME] = iter->strVolName;
        jv[REST_PARAM_HOST_LBA_ADDR] = iter->strLba;

        jRspValue.append(jv);
        
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query partisions succ.");
    
    return MP_SUCCESS;
}

#endif

