/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/oracle/OraclePlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "rest/MessageProcess.h"
#include <sstream>

REGISTER_PLUGIN(COraclePlugin); //lint !e19
COraclePlugin::COraclePlugin()
{
    REGISTER_ACTION(REST_ORACLE_QUERY_DB_INFO, REST_URL_METHOD_GET, &COraclePlugin::QueryInfo);  
    REGISTER_ACTION(REST_ORACLE_QUERY_PDB_INFO, REST_URL_METHOD_GET, &COraclePlugin::QueryPDBInfo);
    REGISTER_ACTION(REST_ORACLE_START_PDB, REST_URL_METHOD_PUT, &COraclePlugin::StartPDB);   
    REGISTER_ACTION(REST_ORACLE_QUERY_LUN_INFO, REST_URL_METHOD_GET, &COraclePlugin::QueryLunInfo);
    REGISTER_ACTION(REST_ORACLE_START, REST_URL_METHOD_PUT, &COraclePlugin::StartDB);
    REGISTER_ACTION(REST_ORACLE_STOP, REST_URL_METHOD_PUT, &COraclePlugin::StopDB);
    REGISTER_ACTION(REST_ORACLE_CHECKARCHIVETHRESHOLD, REST_URL_METHOD_PUT, &COraclePlugin::CheckArchiveThreshold);
    REGISTER_ACTION(REST_ORACLE_TEST, REST_URL_METHOD_PUT, &COraclePlugin::Test);
    REGISTER_ACTION(REST_ORACLE_FREEZE, REST_URL_METHOD_PUT, &COraclePlugin::Freeze);
    REGISTER_ACTION(REST_ORACLE_UNFREEZE, REST_URL_METHOD_PUT, &COraclePlugin::Thaw);
    REGISTER_ACTION(REST_ORACLE_ARCHIVE, REST_URL_METHOD_PUT, &COraclePlugin::ArchiveDB);
    REGISTER_ACTION(REST_ORACLE_STARTASMINSTANCE, REST_URL_METHOD_PUT, &COraclePlugin::StartASMInstance);
    REGISTER_ACTION(REST_ORACLE_STOPASMINSTANCE, REST_URL_METHOD_PUT, &COraclePlugin::StopASMInstance);
    REGISTER_ACTION(REST_ORACLE_STARTRACCLUSTER, REST_URL_METHOD_PUT, &COraclePlugin::StartRACCluster);
    REGISTER_ACTION(REST_ORACLE_FREEZESTATE, REST_URL_METHOD_GET, &COraclePlugin::GetDBFreezeState);
}

COraclePlugin::~COraclePlugin()
{
}
/*------------------------------------------------------------ 
Description  :Oracle组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 输入信息
Output       : rsp -- 返回信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(COraclePlugin, req, rsp);
}

/*------------------------------------------------------------ 
Description  : 查询oracle数据库信息入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 

mp_int32 COraclePlugin::QueryInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iCDBType=ORACLE_TYPE_NON_CDB;
    oracle_db_info_t stDBInfo;
    list<oracle_inst_info_t> lstOracleInstInfo;
    list<oracle_inst_info_t>::iterator iter;

    CRequestURL& vrequrl = req->GetURL();
    stDBInfo.strInstName = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_INSTNAME);
    stDBInfo.strOracleHome = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_ORACLE_HOME);

    //if InstName is not empty, turn to check cdb
    if ( !stDBInfo.strInstName.empty() )
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to check oracle is CDB...");
        //get username and password in http head
        iRet = GetDBAuthParam(req, stDBInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
            return iRet;
        }
        iRet = m_oracle.CheckCDB(stDBInfo, iCDBType);
        Json::Value& jValue= rsp->GetJsonValueRef();
        if ( MP_SUCCESS  == iRet)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Check oracle CDB succ.");
            jValue[RESPOND_ORACLE_PARAM_CDBTYPE] = iCDBType;
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Error occur in checking CDB.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query oracle info..");
    iRet = m_oracle.GetDBInfo(lstOracleInstInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle info failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    for (iter = lstOracleInstInfo.begin(); iter != lstOracleInstInfo.end(); ++iter)
    {
        Json::Value jValue;
        jValue[RESPOND_ORACLE_PARAM_INSTNAME] = iter->strInstName;
        jValue[RESPOND_ORACLE_PARAM_DBNAME] = iter->strDBName;
        jValue[RESPOND_ORACLE_PARAM_VERSION] = iter->strVersion;
        jValue[RESPOND_ORACLE_PARAM_STATE] = iter->iState;
        jValue[RESPOND_ORACLE_PARAM_ISASMDB] = iter->iIsASMDB;
        jValue[RESPOND_ORACLE_PARAM_ORACLE_HOME] = iter->strOracleHome;
        val.append(jValue);      
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query oracle info succ.");
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询oracle12c pdb信息入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 COraclePlugin::QueryPDBInfo(CRequestMsg* req, CResponseMsg* rsp)

{
    mp_int32 iRet = MP_SUCCESS;
    oracle_pdb_req_info_t stPdbReqInfo;
    vector<oracle_pdb_rsp_info_t> vecOraclePdbInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query oracle pdb info..");
    CRequestURL& vrequrl = req->GetURL();
    stPdbReqInfo.strOracleHome = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_ORACLE_HOME);
    stPdbReqInfo.strInstName = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_INSTNAME);
    stPdbReqInfo.strPdbName = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_PDBNAME);

    if(stPdbReqInfo.strInstName.empty()) // || stPdbReqInfo.oracleHome.empty()
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle pdb info failed, instName is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    //获取消息头中的用户名密码
    iRet = GetPDBAuthParam(req, stPdbReqInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = m_oracle.GetPDBInfo(stPdbReqInfo, vecOraclePdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle pdb info failed, iRet %d.", iRet);
        return iRet;
    }

    stPdbReqInfo.strDBPassword.replace(0, stPdbReqInfo.strDBPassword.length(), "");
    stPdbReqInfo.strDBUsername.replace(0, stPdbReqInfo.strDBUsername.length(), "");
    
    vector<oracle_pdb_rsp_info_t>::iterator iter;
    Json::Value& val= rsp->GetJsonValueRef();
    for (iter = vecOraclePdbInfo.begin(); iter != vecOraclePdbInfo.end(); ++iter)
    {
        Json::Value jValue;
        jValue[RESPOND_ORACLE_PARAM_CONID] = iter->iConID;
        jValue[RESPOND_ORACLE_PARAM_PDBNAME] = iter->strPdbName;
        jValue[RESPOND_ORACLE_PARAM_PDBSTATUS] = iter->iStatus;
        val.append(jValue);      
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query oracle pdb info succ.");
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 启动oracle12c pdb入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 COraclePlugin::StartPDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_pdb_req_info_t stPdbReqInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start oracle pdb.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stPdbReqInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_PDBNAME, stPdbReqInfo.strPdbName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stPdbReqInfo.strOracleHome);

    if(stPdbReqInfo.strInstName.empty()) 
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start oracle pdb failed, instName is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    //获取消息头中的用户名密码
    iRet = GetPDBAuthParam(req, stPdbReqInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = m_oracle.StartPluginDB(stPdbReqInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start oracle pdb failed, iRet %d.", iRet);
        return iRet;
    }
    
    stPdbReqInfo.strDBPassword.replace(0, stPdbReqInfo.strDBPassword.length(), "");
    stPdbReqInfo.strDBUsername.replace(0, stPdbReqInfo.strDBUsername.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start oracle pdb succ.");
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取下发的oracle认证信息
Input        : req -- rest请求结构体
Output       :  stDBInfo --数据库信息，获取信息放到此结构体
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::GetPDBAuthParam(CRequestMsg* req, oracle_pdb_req_info_t &stPDBInfo)
{
    const mp_char *pHeadStr = NULL;
    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBUSERNAME);
    if (pHeadStr)
    {
        stPDBInfo.strDBUsername = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)stPDBInfo.strDBUsername.c_str());
    }
    else
    {
        stPDBInfo.strDBUsername = "";
    }

    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBPASSWORD);
    if (pHeadStr)
    {
        stPDBInfo.strDBPassword = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)stPDBInfo.strDBPassword.c_str());
    }
    else
    {
        stPDBInfo.strDBPassword = "";
    }

    if (!stPDBInfo.strDBPassword.empty() && stPDBInfo.strDBUsername.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check parameter failed, strDBPassword is null and strDBUsername is not null.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 查询oracle数据库LUN信息入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 

mp_int32 COraclePlugin::QueryLunInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;
    vector <oracle_lun_info_t> vecLunInfos;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query oracle lun info.");
    CRequestURL& vrequrl = req->GetURL();
    stDBInfo.strInstName = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_INSTNAME);
    stDBInfo.strDBName = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_DBNAME);
    stDBInfo.strASMInstance = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_ASMINSTNAME);
    stDBInfo.iGetArchiveLUN = atoi(vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_SEARCHARCHIVE).c_str());
    stDBInfo.strOracleHome = vrequrl.GetSpecialQueryParam(RESPOND_ORACLE_PARAM_ORACLE_HOME);
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s, ASM instance name:%s, ASM username:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), stDBInfo.strDBUsername.c_str(), 
        stDBInfo.strASMInstance.c_str(), stDBInfo.strASMUserName.c_str());

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    mp_string strPre("+");
    vector<mp_int32> vecExclude;
    if(!(stDBInfo.strASMInstance.empty()))
    {
        CHECK_FAIL_EX(CheckParamString(stDBInfo.strASMInstance, 1, 254, strPre));
    }
    
    CHECK_FAIL_EX(CheckParamInteger32(stDBInfo.iGetArchiveLUN, 0, 1, vecExclude));
    

    iRet = m_oracle.GetDBLUNInfo(stDBInfo, vecLunInfos);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle lun info failed, iRet %d.", iRet);
        return iRet;
    }

    vector < oracle_lun_info_t >::iterator iterDBLun;
    Json::Value& val= rsp->GetJsonValueRef(); 
    for (iterDBLun = vecLunInfos.begin(); iterDBLun != vecLunInfos.end(); ++iterDBLun)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "lunid:%s, uudid:%s, arraysn:%s, wwn:%s, vgname:%s, devicename:%s,"
            " mainType:%d,subType:%d,devicepath=%s, udevrules:%s, lba=%s, asmdg=%s.",
            iterDBLun->strLUNId.c_str(), iterDBLun->strUUID.c_str(), iterDBLun->strArraySn.c_str(), iterDBLun->strWWN.c_str(),
            iterDBLun->strVgName.c_str(), iterDBLun->strDeviceName.c_str(), iterDBLun->iStorMainType, iterDBLun->iStorSubType,
            iterDBLun->strDevicePath.c_str(), iterDBLun->strUDEVRules.c_str(), iterDBLun->strLBA.c_str(), iterDBLun->strASMDiskGroup.c_str());

        Json::Value jValue;
        jValue[RESPOND_ORACLE_LUNID]        = iterDBLun->strLUNId;
        jValue[RESPOND_ORACLE_UUID]         = iterDBLun->strUUID;
        jValue[RESPOND_ORACLE_ARRAYSN]      = iterDBLun->strArraySn;
        jValue[RESPOND_ORACLE_WWN]          = iterDBLun->strWWN;
        jValue[RESPOND_ORACLE_VGNAME]       = iterDBLun->strVgName;
        jValue[RESPOND_ORACLE_DEVICENAME]   = iterDBLun->strDeviceName;
        jValue[RESPOND_ORACLE_PVNAME]          = iterDBLun->strPvName;
        jValue[RESPOND_ORACLE_STORMAINTYPE] = iterDBLun->iStorMainType;
        jValue[RESPOND_ORACLE_STORSUBTYPE]  = iterDBLun->iStorSubType;
        jValue[RESPOND_ORACLE_DEVICEPATH]   = iterDBLun->strDevicePath;
        jValue[RESPOND_ORACLE_UDEVRULES]    = iterDBLun->strUDEVRules;
        jValue[RESPOND_ORACLE_LBA]          = iterDBLun->strLBA;
        jValue[RESPOND_ORACLE_ASMDISKGROUP] = iterDBLun->strASMDiskGroup;
        val.append(jValue);      
    }

    
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query oracle lun info succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 启动oracle数据库入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 COraclePlugin::StartDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iISASM = 0;
    oracle_db_info_t stDBInfo;
    ostringstream oss;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start oracle instance.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_DBNAME, stDBInfo.strDBName);
    if (sendParam.isMember(RESPOND_ORACLE_PARAM_ASMINSTNAME))
    {
       GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ASMINSTNAME, stDBInfo.strASMInstance); 
    }
    if (sendParam.isMember(RESPOND_ORACLE_ASMDISKGROUPS))
    {
        GET_JSON_STRING(sendParam, RESPOND_ORACLE_ASMDISKGROUPS, stDBInfo.strASMDiskGroup);
    }
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);
    GET_JSON_INT32(sendParam, RESPOND_ORACLE_ISASM, iISASM);
    oss << iISASM;
    stDBInfo.strIsASM = oss.str();
    GET_JSON_INT32(sendParam, REST_PARAM_ORACLE_IS_INCLUDE_ARCH, stDBInfo.iIncludeArchLog);

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    mp_string strPre("+");
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iISASM, 0, 1, vecExclude));
    if(!(stDBInfo.strASMInstance.empty()))
    {
        CHECK_FAIL_EX(CheckParamString(stDBInfo.strASMInstance, 1, 254, strPre));
    }
    
    CHECK_FAIL_EX(CheckParamInteger32(stDBInfo.iIncludeArchLog, 0, 1, vecExclude));  
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, asmInstanceName:%s, asmdiskgroup:%s, username:%s.",
      stDBInfo.strInstName.c_str(), stDBInfo.strASMInstance.c_str(), 
      stDBInfo.strASMDiskGroup.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.StartOracleInstance(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start oracle instace(%s) failed, iRet %d.", 
          stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }

    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start oracle instance succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 关闭oracle数据库入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 COraclePlugin::StopDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iISASM = 0;
    oracle_db_info_t stDBInfo;
    ostringstream oss;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin stop oracle instance.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_DBNAME, stDBInfo.strDBName);
    if (sendParam.isMember(RESPOND_ORACLE_PARAM_ASMINSTNAME))
    {
       GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ASMINSTNAME, stDBInfo.strASMInstance); 
    }
    if (sendParam.isMember(RESPOND_ORACLE_ASMDISKGROUPS))
    {
        GET_JSON_STRING(sendParam, RESPOND_ORACLE_ASMDISKGROUPS, stDBInfo.strASMDiskGroup);
    }
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);
    GET_JSON_INT32(sendParam, RESPOND_ORACLE_ISASM, iISASM);
    oss << iISASM;
    stDBInfo.strIsASM = oss.str();

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    mp_string strPre("+");
    vector<mp_int32> vecExclude;

    CHECK_FAIL_EX(CheckParamInteger32(iISASM, 0, 1, vecExclude));
    if(!(stDBInfo.strASMInstance.empty()))
    {
        CHECK_FAIL_EX(CheckParamString(stDBInfo.strASMInstance, 1, 254, strPre));
    }

    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, asmdiskgroup:%s, username:%s.",
      stDBInfo.strInstName.c_str(), stDBInfo.strASMDiskGroup.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.StopOracleInstance(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop oracle instace(%s) failed, iRet %d.", 
          stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop oracle instance succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 测试连接oracle数据库入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 COraclePlugin::Test(CRequestMsg* req, CResponseMsg* rsp)

{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin test oracle database info.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_DBNAME, stDBInfo.strDBName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);

    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s, ASM username:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), stDBInfo.strDBUsername.c_str(), 
        stDBInfo.strASMUserName.c_str());

    iRet = m_oracle.Test(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test oracle database (%s-%s) failed, iRet %d.", 
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test oracle database succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 检查oracle数据库归档阈值入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 COraclePlugin::CheckArchiveThreshold(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;
    vector <oracle_lun_info_t> vecLunInfos;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin check oracle archive threshold.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ARCHIVETHRESHOLD, stDBInfo.strArchThreshold);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ASMINSTNAME, stDBInfo.strASMInstance);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);

    //参数校验
    mp_string strInclude("");
    mp_string strExclude("");
    mp_string strPre("+");
    vector<mp_int32> vecExclude;

    strInclude = mp_string("0123456789");
    CHECK_FAIL_EX(CheckParamString(stDBInfo.strArchThreshold, 1, 254, strInclude, strExclude));

    if(!(stDBInfo.strASMInstance.empty()))
    {
        CHECK_FAIL_EX(CheckParamString(stDBInfo.strASMInstance, 1, 254, strPre));
    }

    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, username:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.CheckArchiveThreshold(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check oracle archive threshold (%s) failed, iRet %d.", 
            stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Check oracle archive threshold succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 冻结oracle数据库入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::Freeze(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze oracle database info.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_DBNAME, stDBInfo.strDBName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);


    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.Freeze(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze oracle database (%s-%s) failed, iRet %d.", 
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze oracle database succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 解冻oracle数据库入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::Thaw(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin thaw oracle database info.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_DBNAME, stDBInfo.strDBName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);


    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.Thaw(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Thaw oracle database (%s-%s) failed, iRet %d.", 
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw oracle database succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :强制归档oracle数据库入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::ArchiveDB(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin archive oracle database info.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_DBNAME, stDBInfo.strDBName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_ORACLE_HOME, stDBInfo.strOracleHome);


    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s.",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.ArchiveDB(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Archive oracle database (%s-%s) failed, iRet %d.", 
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Archive oracle database succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 启动ASM实例入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::StartASMInstance(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start asm instance.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_ASMDISKGROUPS, stDBInfo.strASMDiskGroup);
    
    
    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, asmdiskgroup:%s, username:%s.",
      stDBInfo.strInstName.c_str(), stDBInfo.strASMDiskGroup.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.StartASMInstance(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start ASM instace(%s) failed, iRet %d.", 
          stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start asm instance succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 停止ASM实例入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::StopASMInstance(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin stop ASM instance.");
    const Json::Value& sendParam = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(sendParam, RESPOND_ORACLE_PARAM_INSTNAME, stDBInfo.strInstName);    

    //获取消息头中的用户名密码
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, asmdiskgroup:%s, username:%s.",
      stDBInfo.strInstName.c_str(), stDBInfo.strASMDiskGroup.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.StopASMInstance(stDBInfo);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop ASM instace(%s) failed, iRet %d.", 
          stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop ASM instance succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 启动RAC集群入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::StartRACCluster(CRequestMsg* req, CResponseMsg* rsp)
{
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取Oracle冻结状态入口函数
Input        : req -- rest请求结构体
                rsp -- rest返回结果结构体
Output       : 
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::GetDBFreezeState(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;
    oracle_db_info_t stDBInfo;
    mp_int32 iFreezeState = 0;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query oracle freeze state.");
    CRequestURL& vrequrl= req->GetURL();
    map<mp_string, mp_string>& vreqal = vrequrl.GetQueryParam();
    map<mp_string, mp_string>::iterator iter = vreqal.begin();
    for (; iter != vreqal.end(); ++iter)
    {
        if (iter->first == RESPOND_ORACLE_PARAM_INSTNAME)
        {
            stDBInfo.strInstName = iter->second;
        }
        
        if (iter->first == RESPOND_ORACLE_PARAM_DBNAME)
        {
            stDBInfo.strDBName = iter->second;
        }
    }
    
    iRet = GetDBAuthParam(req, stDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB auth parameter failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "instname:%s, dbname:%s, username:%s",
        stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), stDBInfo.strDBUsername.c_str());

    iRet = m_oracle.QueryFreezeState(stDBInfo, iFreezeState);
    stDBInfo.strASMPassword.replace(0, stDBInfo.strASMPassword.length(), "");
    stDBInfo.strDBPassword.replace(0, stDBInfo.strDBPassword.length(), "");
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query oracle freeze state failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jValue= rsp->GetJsonValueRef();
    //oracle冻结状态和Define.h中冻结状态宏定义相同
    jValue[REST_PARAM_ORACLE_STATE] = iFreezeState;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query oracle freeze state succ.");
    return MP_SUCCESS;

}

/*------------------------------------------------------------ 
Description  :获取下发的oracle认证信息
Input        : req -- rest请求结构体
Output       :  stDBInfo --数据库信息，获取信息放到此结构体
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COraclePlugin::GetDBAuthParam(CRequestMsg* req, oracle_db_info_t &stDBInfo)
{
    const mp_char *pHeadStr = NULL;
    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBUSERNAME);
    if (pHeadStr)
    {
        stDBInfo.strDBUsername = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)stDBInfo.strDBUsername.c_str());
    }
    else
    {
        stDBInfo.strDBUsername = "";
    }

    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBPASSWORD);
    if (pHeadStr)
    {
        stDBInfo.strDBPassword = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)stDBInfo.strDBPassword.c_str());
    }
    else
    {
        stDBInfo.strDBPassword = "";
    }

    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_ASMSERNAME);
    if (pHeadStr)
    {
        stDBInfo.strASMUserName = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)stDBInfo.strASMUserName.c_str());
    }
    else
    {
        stDBInfo.strASMUserName = "";
    }

    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_ASMPASSWORD);
    if (pHeadStr)
    {
        stDBInfo.strASMPassword = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)stDBInfo.strASMPassword.c_str());
    }
    else
    {
        stDBInfo.strASMPassword = "";
    }


    mp_bool bParamCheck = (!stDBInfo.strASMPassword.empty() && stDBInfo.strASMUserName.empty())
                        || (!stDBInfo.strDBPassword.empty() && stDBInfo.strDBUsername.empty());
    if (bParamCheck)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check parameter failed, strASMPassword is null"
            " and strASMUserName is not null, strDBPassword is null and strDBUsername is not null.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    return MP_SUCCESS;
}

