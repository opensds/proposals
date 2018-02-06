/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "alarm/AppFreezeStatus.h" 
#include "common/Defines.h"
#include "common/Log.h"
#include <sstream>
/*------------------------------------------------------------ 
Description  : 插入应用状态表到sqlite数据库
Input        : stStatus---冻结状态
Output       : 
Return       :  MP_SUCCESS---状态表中已经存在或执行sql成功
               iRet---执行sql失败，返回对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CAppFreezeStatus::Insert(const freeze_status& stStatus)
{
    LOGGUARD("");
    if (IsExist(stStatus))
    {
        //已经存在，打印日志
        COMMLOG(OS_LOG_INFO,LOG_COMMON_INFO,"%s is exist",stStatus.strKey.c_str());
        return MP_SUCCESS;
    }

    ostringstream buff;
    buff << "insert into " << AppStatusTable <<"(Key) values(?);";
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = stStatus.strKey;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
    }
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 删除应用状态表中的某条信息
Input        : stStatus---冻结状态
Output       : 
Return       :  MP_SUCCESS---状态表中不存在或执行sql成功
               iRet---执行sql失败，返回对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CAppFreezeStatus::Delete(const freeze_status& stStatus)
{
    LOGGUARD("");
    if (!IsExist(stStatus))
    {
        //不存在，打印日志
        COMMLOG(OS_LOG_INFO,LOG_COMMON_INFO,"%s is not exist",stStatus.strKey.c_str());
        return MP_SUCCESS;
    }

    std::ostringstream buff;
    buff<<"delete from " << AppStatusTable <<" where Key == ?";
    mp_string sql = buff.str();
    
    DbParamStream dps;
    DbParam dp = stStatus.strKey;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
    }
    return iRet;
}
/*------------------------------------------------------------ 
Description  : 获取应用状态
Input        : 
Output       : stStatus---冻结状态
Return       :  
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CAppFreezeStatus::Get(freeze_status& stStatus)
{
    LOGGUARD("");
    if (IsExist(stStatus))
    {
        stStatus.iStatus = DB_FREEZE;
    }
    else
    {
        stStatus.iStatus = DB_UNFREEZE;
    }
}
/*------------------------------------------------------------ 
Description  : 获取所有应用状态
Input        : 
Output       : vecStatus---状态列表
Return       :  MP_SUCCESS---获取状态成功
               iRet---获取失败，返回错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CAppFreezeStatus::GetAll(vector<freeze_status>& vecStatus)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select Key from " << AppStatusTable;
    
    DbParamStream dps;
    
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return iRet;
    }

    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow)
    {
        freeze_status stStatus;
        readBuff >> stStatus.strKey;
        stStatus.iStatus = DB_FREEZE;
        vecStatus.push_back(stStatus);
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 查询应用状态是否存在表中
Input        : stStatus---冻结状态
Output       : 
Return       :  MP_TRUE---查询到应用状态
               MP_FALSE---iRowCount>=0，false---查询表失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CAppFreezeStatus::IsExist(const freeze_status& stStatus)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select Key from " << AppStatusTable <<" where Key == ?";

    mp_string strSql = buff.str();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "buff is = %s.", strSql.c_str());
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    DbParamStream dps;
    DbParam dp = stStatus.strKey;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return MP_FALSE;
    }

    if (0 < iRowCount)
    {
        return MP_TRUE;
    }
    else
    {
        return MP_FALSE;
    }
}


