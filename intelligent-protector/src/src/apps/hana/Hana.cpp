/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/


#include "apps/hana/Hana.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/Defines.h"
#include "common/RootCaller.h"

CHana::CHana()
{
}

CHana::~CHana()
{
}


/*------------------------------------------------------------ 
Description  : 构建执行保护策略和启动停止脚本输入参数
Input        : stdbinfo -- 数据库信息
                  strOperType --操作类型
Output       : strParam -- 输入参数字符串
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CHana::BuildScriptParam(hana_db_info_t &stdbInfo, mp_string strOperType, mp_string &strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNUM) + stdbInfo.strInstNum + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbInfo.strDbName + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_OPERTYPE) + strOperType;
}


/*------------------------------------------------------------ 
Description  : 构建执行保护策略和启动停止脚本输入参数
Input        : stdbinfo -- 数据库信息
Output       : strParam -- 输入参数字符串
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CHana::BuildScriptParam(hana_db_info_t &stdbInfo, mp_string &strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNUM) + stdbInfo.strInstNum + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbInfo.strDbName + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbInfo.strDBPassword;
}

/*------------------------------------------------------------ 
Description  : 启动数据库
Input        :stdbInfo -- 数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CHana::StartDB(hana_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start hana database, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
    BuildScriptParam(stdbInfo, SCRIPTPARAM_STARTDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STARTHANA, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute start hana database, instance num %s, database %s script failed, iRet %d.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "start hana database succ, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :停止数据库
Input        :stdbInfo -- 数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CHana::StopDB(hana_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to stop hana database, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
    BuildScriptParam(stdbInfo, SCRIPTPARAM_STOPDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STOPHANA, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute stop hana database, instance num %s, database %s script failed, iRet %d.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop hana database succ, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :冻结数据库，调用数据库名命令停顿数据库
Input        :vecdbInfo --待冻结的数据库列表
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CHana::FreezeDB(hana_db_info_t &stdbInfo)
{
    mp_string strParam = "";
    mp_string strFileParam = "";
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to freeze hana database, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
    BuildScriptParam(stdbInfo, SCRIPTPARAM_FREEZEDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_FREEZEHANA, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_FREEZE_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute freeze hana database failed, instance num %s, database %s script failed, iRet %d.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str(), iRet);
        return iRet;
    }
    
    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Sync data file failed, file list is empty.");
        return ERROR_COMMON_APP_FREEZE_FAILED;
    }
    
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        strFileParam = *iter;
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SYNC_DATA_FILE, strFileParam, NULL);
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_FREEZE_FAILED);
        if (MP_SUCCESS != iRet)
        {  
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
                "Excute freeze hana database failed, instance num %s, database %s script failed, iRet %d.", 
                stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str(), iRet);
            return iRet;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze hana database succ, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :解冻数据库，调用数据库名命令释放数据库
Input        :vecdbInfo --待释放的数据库列表
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CHana::ThawDB(hana_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to thaw hana database, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_THAWDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_THAWHANA, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute thaw hana database failed, instance num %s, database %s script failed, iRet %d.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw hana database succ, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :查询数据库冻结状态，调用数据库名命令查询数据库状态
Input        :stdbInfo --数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CHana::GetFreezeStatus(hana_db_info_t &stdbInfo, mp_int32 &iFreezeState)
{
    mp_string strParam;
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get hana database freeze status, instance num %s, database %s.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_FREEZESTATUS, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYHANAFREEZESTATUS, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute get hana database freeze status, instance num %s, database %s script failed, iRet %d.", 
            stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str(), iRet);
        return iRet;
    }
    
    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of get hana freeze state is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }
    iFreezeState = atoi(vecResult.front().c_str());
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get hana database status succ, instance num %s, database %s, state %d.", 
        stdbInfo.strInstNum.c_str(), stdbInfo.strDbName.c_str(), iFreezeState);
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 测试连接数据库
Input        :stdbInfo -- 数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CHana::Test(hana_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test hana instance num(%s).", stdbInfo.strInstNum.c_str());    
    BuildScriptParam(stdbInfo, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_TESTHANA, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute test instance num (%s) script failed, iRet %d.", stdbInfo.strInstNum.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test hana instance num (%s) succ.", stdbInfo.strInstNum.c_str());
    return MP_SUCCESS;
}

