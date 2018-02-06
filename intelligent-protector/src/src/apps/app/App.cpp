/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/app/App.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"

CApp::CApp()
{
#ifdef WIN32    
    m_pVssRequester = NULL;
#endif
}

CApp::~CApp()
{
}

mp_int32 CApp::QueryInfo(vector<app_info_t>& vecAppInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Query app info.");
    iRet = QuerySqlInfo(vecAppInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sql info failed, iRet %d", iRet);
        return iRet;
    }

    iRet = QueryOracleInfo(vecAppInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle info failed, iRet %d", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query app info succ.");
    return MP_SUCCESS;
}

mp_int32 CApp::Freeze(app_auth_info_t& appAuthInfo, mp_time& tFreezeTime, vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet = MP_SUCCESS;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin freeze app.");
    CMpTime::Now(&tFreezeTime);
#ifdef WIN32
    iRet = FreezeVss(vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Vss freeze failed, iRet %d.", iRet);
        return iRet;
    }
#else
    iRet = FreezeOracle(appAuthInfo, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Oracle freeze failed, iRet %d.", iRet);
        return iRet;
    }
#endif
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End freeze app.");
    return MP_SUCCESS;
}

mp_int32 CApp::UnFreeze(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet = MP_SUCCESS;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin unfreeze app.");
#ifdef WIN32
    iRet = UnFreezeVss(vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Vss unfreeze failed, iRet %d.", iRet);
        return iRet;
    }
#else
    iRet = UnFreezeOracle(appAuthInfo, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Oracle unfreeze failed, iRet %d.", iRet);
        return iRet;
    }
#endif    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Unfreeze app.");
    return MP_SUCCESS;
}

mp_int32 CApp::EndBackup(app_auth_info_t& appAuthInfo, mp_int32 iBackupSucc, vector<app_failed_info_t>& vecAppFailedList)
{    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin end backup.");
#ifdef WIN32
    mp_int32 iRet = MP_SUCCESS;

    iRet = EndBackupVss(iBackupSucc, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Vss end backup failed, iRet %d.", iRet);
        return iRet;
    }
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Do nothing for non windows.");
#endif
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End endbackup.");
    return MP_SUCCESS;
}

mp_int32 CApp::TruncateLog(app_auth_info_t& appAuthInfo, mp_time tTruncateTime, vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet = MP_SUCCESS;
    app_info_t appInfo;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin truncate log.");
#ifdef WIN32
    iRet = TruncateSqlLog(appAuthInfo, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Truncate sql log failed, iRet %d.", iRet);
        return iRet;
    }
#endif
    iRet = TruncateOracleLog(appAuthInfo, tTruncateTime, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Truncate oracle log failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End truncate log.");
    return MP_SUCCESS;
}

mp_int32 CApp::QuerySqlInfo(vector<app_info_t>& vecAppInfos)
{
#ifdef WIN32
    mp_int32 iRet = MP_SUCCESS;
    app_info_t appInfo;
    mp_bool bIsInstalled = MP_FALSE;
    vector<sqlserver_info_t> vecSqlserverDBInfo;
    vector<sqlserver_info_t>::iterator iter;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query sql info.");
    iRet = m_sqlserver.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check sql server is installed or not failed, iRet %d", iRet);
        return iRet;
    }

    if (!bIsInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Sql server isn't installed on this machine, skip it.");
        return MP_SUCCESS;
    }
    
    iRet = m_sqlserver.GetInfo(vecSqlserverDBInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sqlserver database info failed, iRet %d", iRet);
        return iRet;
    }

    for (iter = vecSqlserverDBInfo.begin(); iter != vecSqlserverDBInfo.end(); ++iter)
    {
        appInfo.enAppType = APP_TYPE_SQLSERVER;
        appInfo.strVersion = iter->strVersion;
        appInfo.strInstName = iter->strInstName;
        appInfo.strDBName = iter->strDBName;
        vecAppInfos.push_back(appInfo);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query sql info succ.");
    return iRet;
#else
    return MP_SUCCESS;
#endif
}

mp_int32 CApp::QueryOracleFreezeState(app_auth_info_t& appAuthInfo, mp_int32& iState)
{
    mp_bool bIsInstalled = MP_FALSE;
    mp_int32 iRet = m_oracle.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check Oracle installed failed, iRet %d.", iRet);
        return iRet;
    }

    if (MP_FALSE == bIsInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Oracle is not installed on this machine.");
        iState = DB_UNFREEZE;
        return MP_SUCCESS;
    }

    list<oracle_inst_info_t> lstOracleInfo;
    iRet = m_oracle.GetDBInfo(lstOracleInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle database info failed, iRet %d", iRet);
        return iRet;
    }
    for (list<oracle_inst_info_t>::iterator iter = lstOracleInfo.begin(); iter != lstOracleInfo.end(); ++iter)
    {
        mp_int32 iStateTmp = DB_UNFREEZE;
        oracle_db_info_t oracleDBInfo;
        oracleDBInfo.iGetArchiveLUN = 0;
        oracleDBInfo.iIncludeArchLog = 0;
        oracleDBInfo.strDBUsername = appAuthInfo.strUserName;
        oracleDBInfo.strDBPassword = appAuthInfo.strPasswd;
        oracleDBInfo.strInstName = iter->strInstName;
        iRet = m_oracle.QueryFreezeState(oracleDBInfo, iStateTmp);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query freeze state failed, iRet = %d.", iRet);
        }
        else if (iStateTmp != DB_UNFREEZE)
        {
            iState = iStateTmp;
            return MP_SUCCESS;
        }
    }

    //所有实例状态均为未冻结
    iState = DB_UNFREEZE;
    return MP_SUCCESS;
}


mp_int32 CApp::QueryOracleInfo(vector<app_info_t>& vecAppInfos)
{
    app_info_t appInfo;
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsInstalled = MP_FALSE;
    list<oracle_inst_info_t> lstOracleInfo;
    list<oracle_inst_info_t>::iterator iter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query oracle info.");

    iRet = m_oracle.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check Oracle installed failed, iRet %d.", iRet);
        return iRet;
    }

    if (MP_FALSE == bIsInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Oracle is not installed on this machine.");
        return MP_SUCCESS;
    }
    
    iRet = m_oracle.GetDBInfo(lstOracleInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle database info failed, iRet %d", iRet);
        return iRet;
    }

    for (iter = lstOracleInfo.begin(); iter != lstOracleInfo.end(); ++iter)
    {
        appInfo.enAppType = APP_TYPE_ORACLE;
        appInfo.strVersion = iter->strVersion;
        appInfo.strInstName = iter->strInstName;
        appInfo.strDBName = iter->strDBName;
        vecAppInfos.push_back(appInfo);
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query oracle info succ."); 
    return MP_SUCCESS;
}

mp_int32 CApp::FreezeOracle(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsInstalled = MP_FALSE;
    app_failed_info_t errorInfo;
    oracle_db_info_t oracleDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to check oracle installed."); 

    iRet = m_oracle.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check Oracle installed failed, iRet %d.", iRet);
        return iRet;
    }

    if (MP_FALSE == bIsInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Oracle is not installed in this host.");
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze all oracle instances.");   
    oracleDBInfo.iGetArchiveLUN = 0;
    oracleDBInfo.iIncludeArchLog = 0;
    oracleDBInfo.strDBUsername = appAuthInfo.strUserName;
    oracleDBInfo.strDBPassword = appAuthInfo.strPasswd;
    
    list<oracle_inst_info_t> lstOracleInsts;
    iRet = m_oracle.GetDBInfo(lstOracleInsts);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query all oracle instances failed, iRet %d.", iRet);
        return iRet;
    }

    list<oracle_db_info_t> lstOracleDBInfo;
    for (list<oracle_inst_info_t>::iterator iter = lstOracleInsts.begin(); iter != lstOracleInsts.end(); ++iter)
    {
        oracleDBInfo.strInstName = iter->strInstName;
        lstOracleDBInfo.push_back(oracleDBInfo);
    }

    for (list<oracle_db_info_t>::iterator iter = lstOracleDBInfo.begin(); iter != lstOracleDBInfo.end(); ++iter)
    {
        oracleDBInfo = (*iter);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Freeze: instname:%s, username:%s.", oracleDBInfo.strInstName.c_str(), 
            oracleDBInfo.strDBUsername.c_str());
        iRet = m_oracle.Freeze(oracleDBInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze oracle instance (%s) failed, iRet %d.",
                oracleDBInfo.strInstName.c_str(), iRet);
            errorInfo.iErrorCode = iRet;
            errorInfo.strDbName = oracleDBInfo.strInstName;
            vecAppFailedList.push_back(errorInfo);
            return iRet;
        }
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End freeze all oracle instances.");
    return MP_SUCCESS;
}

mp_int32 CApp::UnFreezeOracle(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsInstalled = MP_FALSE;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to check oracle installed."); 

    iRet = m_oracle.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check Oracle installed failed, iRet %d.", iRet);
        return iRet;
    }

    if (MP_FALSE == bIsInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Oracle is not installed in this host.");
        return MP_SUCCESS;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin unfreeze all oracle instances.");      
    list<oracle_inst_info_t> lstOracleInsts;
    iRet = m_oracle.GetDBInfo(lstOracleInsts);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query all oracle instances failed, iRet %d.", iRet);
        return iRet;
    }

    list<oracle_db_info_t> lstOracleDBInfo;
    for (list<oracle_inst_info_t>::iterator iter = lstOracleInsts.begin(); iter != lstOracleInsts.end(); ++iter)
    {
        oracle_db_info_t oracleDBInfo;
        app_failed_info_t errorInfo;
        oracleDBInfo.iGetArchiveLUN = 0;
        oracleDBInfo.iIncludeArchLog = 0;
        oracleDBInfo.strDBUsername = appAuthInfo.strUserName;
        oracleDBInfo.strDBPassword = appAuthInfo.strPasswd;
        oracleDBInfo.strInstName = iter->strInstName;

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unfreeze: instname:%s, username:%s.", oracleDBInfo.strInstName.c_str(), 
            oracleDBInfo.strDBUsername.c_str());
        iRet = m_oracle.Thaw(oracleDBInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze oracle database (%s) failed, iRet %d.",
                oracleDBInfo.strInstName.c_str(), iRet);
            errorInfo.iErrorCode = iRet;
            errorInfo.strDbName = oracleDBInfo.strInstName;
            vecAppFailedList.push_back(errorInfo);
            continue;
        }

        // archive DB
        iRet = m_oracle.ArchiveDB(oracleDBInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Archive oracle database (%s) failed, iRet %d.",
                    oracleDBInfo.strInstName.c_str(), iRet);
            errorInfo.iErrorCode = iRet;
            errorInfo.strDbName = oracleDBInfo.strInstName;
            vecAppFailedList.push_back(errorInfo);
        }
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End unfreeze all oracle instances.");
    return vecAppFailedList.empty() ? MP_SUCCESS : ERROR_COMMON_APP_THAW_FAILED;
}


mp_int32 CApp::UnFreezeExOracle(app_auth_info_t& appAuthInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsInstalled = MP_FALSE;
    app_failed_info_t errorInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to check oracle installed."); 

    iRet = m_oracle.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check Oracle installed failed, iRet %d.", iRet);
        return iRet;
    }

    if (MP_FALSE == bIsInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Oracle is not installed in this host.");
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin unfreeze all oracle instances.");      
    list<oracle_inst_info_t> lstOracleInsts;
    iRet = m_oracle.GetDBInfo(lstOracleInsts);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query all oracle instances failed, iRet %d.", iRet);
        return iRet;
    }

    list<oracle_db_info_t> lstOracleDBInfo;
    for (list<oracle_inst_info_t>::iterator iter = lstOracleInsts.begin(); iter != lstOracleInsts.end(); ++iter)
    {
        oracle_db_info_t oracleDBInfo;
        oracleDBInfo.iGetArchiveLUN = 0;
        oracleDBInfo.iIncludeArchLog = 0;
        oracleDBInfo.strDBUsername = appAuthInfo.strUserName;
        oracleDBInfo.strDBPassword = appAuthInfo.strPasswd;
        oracleDBInfo.strInstName = iter->strInstName;
        mp_int32 iState = DB_UNFREEZE;
        iRet = m_oracle.QueryFreezeState(oracleDBInfo, iState);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query freeze state failed, iRet = %d.", iRet);
        }
        else if (iState == DB_UNFREEZE)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "db instance:%s is unfreeze.", oracleDBInfo.strInstName.c_str());
            continue;
        }
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unfreeze: instname:%s, username:%s.", oracleDBInfo.strInstName.c_str(), 
            oracleDBInfo.strDBUsername.c_str());
        iRet = m_oracle.Thaw(oracleDBInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze oracle database (%s) failed, iRet %d.",
                oracleDBInfo.strInstName.c_str(), iRet);
            return iRet;
        }

        //startup instance
        iRet = m_oracle.StartOracleInstance(oracleDBInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start oracle database (%s) failed, iRet %d.",
                oracleDBInfo.strInstName.c_str(), iRet);
            return iRet;
        }
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End unfreezeEx all oracle instances.");
    return iRet;
}


mp_int32 CApp::TruncateOracleLog(app_auth_info_t& appAuthInfo, mp_time tTruncateTime, vector<app_failed_info_t>& vecAppFailedList)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsInstalled = MP_FALSE;
    app_failed_info_t errorInfo;
    oracle_db_info_t oracleDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to check oracle installed."); 

    iRet = m_oracle.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check Oracle installed failed, iRet %d.", iRet);
        return iRet;
    }

    if (MP_FALSE == bIsInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Oracle is not installed in this host.");
        return MP_SUCCESS;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin truncate all oracle instances log.");
    oracleDBInfo.iGetArchiveLUN = 0;
    oracleDBInfo.iIncludeArchLog = 0;
    oracleDBInfo.strDBUsername = appAuthInfo.strUserName;
    oracleDBInfo.strDBPassword = appAuthInfo.strPasswd;
    
    list<oracle_inst_info_t> lstOracleInsts;
    iRet = m_oracle.GetDBInfo(lstOracleInsts);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query all oracle instances failed, iRet %d.", iRet);
        return iRet;
    }

    list<oracle_db_info_t> lstOracleDBInfo;
    for (list<oracle_inst_info_t>::iterator iter = lstOracleInsts.begin(); iter != lstOracleInsts.end(); ++iter)
    {
        oracleDBInfo.strInstName = iter->strInstName;
        lstOracleDBInfo.push_back(oracleDBInfo);
    }

    for (list<oracle_db_info_t>::iterator iter = lstOracleDBInfo.begin(); iter != lstOracleDBInfo.end(); ++iter)
    {
        oracleDBInfo = (*iter);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Truncate Log: instname:%s, username:%s.", oracleDBInfo.strInstName.c_str(), 
            oracleDBInfo.strDBUsername.c_str());
        iRet = m_oracle.TruncateArchiveLog(oracleDBInfo, tTruncateTime);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Truncate oracle instance (%s) log failed, iRet %d.",
                oracleDBInfo.strInstName.c_str(), iRet);
            errorInfo.iErrorCode = iRet;
            errorInfo.strDbName = oracleDBInfo.strInstName;
            vecAppFailedList.push_back(errorInfo);
        }
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End truncate all oracle instances log.");
    return iRet;
}

/*---------------------------------------------------------------------------
Function Name: QueryFreezeState
Description  : 冻结保护机制查询当前主机上应用的冻结状态，查询失败返回未知，全部都已解冻返回1，存在一个未解冻应用返回0
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CApp::QueryFreezeState(app_auth_info_t& appAuthInfo, mp_int32& iState)
{
#ifdef WIN32
    NULL == m_pVssRequester ? iState = DB_UNFREEZE : DB_FREEZE;
    return MP_SUCCESS;
#else
    //目前之后oracle数据库，如果有其他数据库，要遍历完所有数据库
    return QueryOracleFreezeState(appAuthInfo, iState);
#endif    
}

/*---------------------------------------------------------------------------
Function Name: UnFreezeEx
Description  : 冻结保护机制使用，执行解冻和结束备份操作，重复解冻时不返回错误
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CApp::UnFreezeEx(app_auth_info_t& appAuthInfo)
{
#ifdef WIN32
    if (NULL == m_pVssRequester)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "VSS is already unfreezed.");
        //根据测试结果。当vss超时后会自动结束备份状态，此处不做处理
        return MP_SUCCESS;
    }

    vector<app_failed_info_t> vecAppFailedList;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin UnfreezeEx app.");
    mp_int32 iRet = UnFreezeVss(vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Vss unfreeze failed, iRet %d.", iRet);
        return iRet;
    }
    vecAppFailedList.clear();
    iRet = EndBackup(appAuthInfo, 0, vecAppFailedList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "EndBackup failed, iRet %d.", iRet);
        return iRet;
    }
#else 
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin unfreeze all oracle instances.");   
    //目前之后oracle数据库，如果有其他数据库，要遍历完所有数据库
    mp_int32 iRet = UnFreezeExOracle(appAuthInfo);
#endif
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End UnfreezeEx app.");
    return iRet;
}


#ifdef WIN32
mp_int32 CApp::IsAppInstalled(mp_bool& bIsAppInstalled)
{
    mp_bool bIsInstalled = MP_FALSE;
    mp_int32 iRet = MP_SUCCESS;
    iRet = m_sqlserver.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {        
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check sql server is installed or not failed, iRet %d", iRet);
        return iRet;
    }

    if (bIsInstalled)
    {
        bIsAppInstalled = MP_TRUE;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Sql server is installed on this machine.");
        return MP_SUCCESS;
    }

    iRet = m_oracle.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check oracle is installed or not failed, iRet %d.", iRet);
        return iRet;
    }

    if (bIsInstalled)
    {
        bIsAppInstalled = MP_TRUE;
        COMMLOG(OS_LOG_DEBUG, OS_LOG_DEBUG, "Oracle is installed on this machine.");
        return MP_SUCCESS;
    }

    bIsAppInstalled = MP_FALSE;
    return MP_SUCCESS;
}

mp_int32 CApp::FreezeVss(vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet = MP_SUCCESS;
    app_failed_info_t appFailedInfo;
    mp_bool bIsAppInstalled = MP_FALSE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin vss freeze.");
    iRet = IsAppInstalled(bIsAppInstalled);
    if (MP_SUCCESS != iRet)
    {    
        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check whether app is installed failed, iRet %d", iRet);
        return iRet;
    }

    if (!bIsAppInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Can't find any app info on this computer.");
        return MP_SUCCESS;
    }
    
    if (NULL != m_pVssRequester)
    {        
        appFailedInfo.iErrorCode = ERROR_VSS_OTHER_FREEZE_RUNNING;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Other freeze opertion is running.");
        return ERROR_VSS_OTHER_FREEZE_RUNNING;
    }

    NEW_CATCH(m_pVssRequester, VSSRequester);
    if (NULL == m_pVssRequester)
    {        
        appFailedInfo.iErrorCode = ERROR_COMMON_OPER_FAILED;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        return MP_FAILED;
    }

    iRet = m_pVssRequester->FreezeAll();
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        
        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze all volumes failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End vss freeze.");

    return MP_SUCCESS;
}

mp_int32 CApp::UnFreezeVss(vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet = MP_SUCCESS;
    app_failed_info_t appFailedInfo;
    mp_bool bIsAppInstalled = MP_FALSE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin vss unfreeze.");
    iRet = IsAppInstalled(bIsAppInstalled);
    if (MP_SUCCESS != iRet)
    {        
        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check whether app is installed failed, iRet %d", iRet);
        return iRet;
    }

    if (!bIsAppInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Can't find any app info on this computer.");
        return MP_SUCCESS;
    }
    
    if (NULL == m_pVssRequester)
    {
        appFailedInfo.iErrorCode = ERROR_COMMON_APP_THAW_FAILED;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "No need to unfreeze.");
        return ERROR_COMMON_APP_THAW_FAILED;
    }

    iRet = m_pVssRequester->UnFreezeAll();
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        
        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze all volumes failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin vss unfreeze.");

    return MP_SUCCESS;
}

mp_int32 CApp::EndBackupVss(mp_int32 iBackupSucc, vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet = MP_SUCCESS;
    app_failed_info_t appFailedInfo;
    mp_bool bIsAppInstalled = MP_FALSE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin vss endbakup.");
    iRet = IsAppInstalled(bIsAppInstalled);
    if (MP_SUCCESS != iRet)
    {        
        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check whether app is installed failed, iRet %d", iRet);
        return iRet;
    }

    if (!bIsAppInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Can't find any app info on this computer.");
        return MP_SUCCESS;
    }
    
    if (NULL == m_pVssRequester)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "No need to end backup.");
        return MP_SUCCESS;
    }

    iRet = m_pVssRequester->EndBackup(iBackupSucc);
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        
        appFailedInfo.iErrorCode = iRet;
        appFailedInfo.strDbName = DBNAME_FOR_VSS_IN_ERR_RESPONSE;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "End backup failed, iRet %d.", iRet);
        return iRet;
    }
    
    delete m_pVssRequester;
    m_pVssRequester = NULL;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End vss endbakup.");
    
    return MP_SUCCESS;
}

mp_int32 CApp::TruncateSqlLog(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList)
{
    mp_int32 iRet = MP_SUCCESS;
    app_failed_info_t appFailedInfo;
    vector<sqlserver_info_t> vecSqlserverDBInfo;
    vector<sqlserver_info_t>::iterator iter;
    mp_int32 iOnlineStatus;
    mp_bool bIsInstalled = MP_TRUE;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin truncate sql log.");
    iRet = m_sqlserver.IsInstalled(bIsInstalled);
    if (MP_SUCCESS != iRet)
    {        
        appFailedInfo.iErrorCode = iRet;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check sql server is installed or not failed, iRet %d", iRet);
        return iRet;
    }

    if (!bIsInstalled)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Sql server isn't installed on this machine, skip it.");
        return MP_SUCCESS;
    }
    
    iRet = m_sqlserver.GetInfo(vecSqlserverDBInfo);
    if (MP_SUCCESS != iRet)
    {
        appFailedInfo.iErrorCode = iRet;
        vecAppFailedList.push_back(appFailedInfo);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sql server database info failed, iRet %d", iRet);
        return iRet;
    }
    
    for (iter = vecSqlserverDBInfo.begin(); iter != vecSqlserverDBInfo.end(); ++iter)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Truncate database trans log , iRet %d, instance name %s, db name %s, "
            "is cluster %s, online status %s, recovery model %d.", iRet, iter->strInstName.c_str(), iter->strDBName.c_str(),
            iter->strIsCluster.c_str(), iter->strState.c_str(), iter->iRecoveryModel);
        
        iter->strUser = appAuthInfo.strUserName;
        iter->strPasswd = appAuthInfo.strPasswd;
        iter->strCheckType = SQL_SERVER_OPTCODE_TRUNCATE;
        iOnlineStatus = atoi(iter->strState.c_str());
        if ((SQL_SERVER_RECOVERY_MODEL_FULL != iter->iRecoveryModel && SQL_SERVER_RECOVERY_MODEL_BULK_LOG != iter->iRecoveryModel)
            || SQL_SERVER_ONLINE != iOnlineStatus)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Current database is not online, or the recovery model is not full and "
                "bulk log, skip it, instance name %s, db name %s.", iter->strInstName.c_str(), iter->strDBName.c_str());
            continue;
        }
        
        iRet = m_sqlserver.TruncateTransLog(*iter);
        if (MP_SUCCESS != iRet)
        {            
            appFailedInfo.iErrorCode = iRet;
            appFailedInfo.strDbName = iter->strDBName.c_str();
            vecAppFailedList.push_back(appFailedInfo);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Truncate trans log failed, instance name %s, db name %s.",
                iter->strInstName.c_str(), iter->strDBName.c_str());
            return iRet;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Truncate all transaction log succ.");
    return MP_SUCCESS;
}
#endif

