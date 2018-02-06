/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "alarm/Alarm.h"
#include "alarm/Db.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"

#include <sstream>

/*------------------------------------------------------------
Function Name: InsertAlarmInfo
Description  : 将alarm信息插入到sqlite数据库
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::InsertAlarmInfo(alarm_Info_t &stAlarmInfo)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff<<"insert into " <<AlarmTable <<"("<< titleAlarmSerialNo <<"," << titleAlarmID<<"," << titleAlarmType<<"," \
         << titleAlarmLevel<<","<< titleStartTime<<","<<titleEndTime<<","<<titleAlarmParam <<") values(?,?,?,?,?,?,?);";
    mp_string sql = buff.str();
    
    CDB &db = CDB::GetInstance();

    DbParamStream dps;
    DbParam dp = stAlarmInfo.iAlarmSN;
    dps << dp;
    dp = stAlarmInfo.iAlarmID;
    dps << dp;
    dp = stAlarmInfo.iAlarmType;
    dps << dp;
    dp = stAlarmInfo.iAlarmLevel;
    dps << dp;
    dp = stAlarmInfo.strStartTime;
    dps << dp;
    dp = stAlarmInfo.strEndTime;
    dps << dp;
    dp = stAlarmInfo.strAlarmParam;
    dps << dp;
    
    mp_int32 iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
    }
    
    return iRet;
}

/*------------------------------------------------------------
Function Name: DeleteAlarmInfo
Description  : 删除sqlite数据库中某条告警数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::DeleteAlarmInfo(mp_int32 iAlarmSN, mp_int32 iAlarmID)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff<<"delete from " <<AlarmTable <<" where " << titleAlarmSerialNo <<" == ?" 
        <<" and "<<titleAlarmID <<" == ?;";
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = iAlarmSN;
    dps << dp;
    dp = iAlarmID;
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
Function Name: UpdateAlarmInfo
Description  : 更新sqlite数据库中某条告警数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::UpdateAlarmInfo(alarm_Info_t &stAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"update " <<AlarmTable <<" set " << titleAlarmLevel << "= ?,"
            << titleAlarmParam << "= ?," << titleAlarmType << "= ?,"
            << titleEndTime << "= ?," << titleStartTime << "= ?"<<" where "
            << titleAlarmSerialNo << " == ? and " << titleAlarmID << " == ?;";
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = stAlarmInfo.iAlarmLevel;
    dps << dp;
    dp = stAlarmInfo.strAlarmParam;
    dps << dp;
    dp = stAlarmInfo.iAlarmType;
    dps << dp;
    dp = stAlarmInfo.strEndTime;
    dps << dp;
    dp = stAlarmInfo.strStartTime;
    dps << dp;
    dp = stAlarmInfo.iAlarmSN;
    dps << dp;
    dp = stAlarmInfo.iAlarmID;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
    }
    return iRet;
}

/*------------------------------------------------------------
Function Name: UpdateAlarmInfo
Description  : 查询sqlite数据库中所有告警数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::GetAllAlarmInfo(vector<alarm_Info_t> &vecAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select "<< titleAlarmSerialNo <<"," << titleAlarmID<<"," << titleAlarmType<<"," \
        << titleAlarmLevel<<","<< titleStartTime<<","<<titleEndTime<<","<<titleAlarmParam <<" from " <<AlarmTable;
    mp_string sql = buff.str();

    DbParamStream dps;
    
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return iRet;
    }

    for(mp_int32 iRow = 1; iRow <= iRowCount; ++iRow)
    {
        alarm_Info_t stAlarmInfo;
        mp_string strAlarmSN = "";
        mp_string strAlarmID = "";
        mp_string strAlarmType = "";
        mp_string strAlarmLevel = "";
        readBuff >> strAlarmSN;
        readBuff >> strAlarmID;
        readBuff >> strAlarmType;
        readBuff >> strAlarmLevel;
        stAlarmInfo.iAlarmID = atoi(strAlarmID.c_str());
        stAlarmInfo.iAlarmSN = atoi(strAlarmSN.c_str());
        stAlarmInfo.iAlarmLevel = atoi(strAlarmLevel.c_str());
        stAlarmInfo.iAlarmType = atoi(strAlarmType.c_str());
        readBuff >> stAlarmInfo.strStartTime;
        readBuff >> stAlarmInfo.strEndTime;
        readBuff >> stAlarmInfo.strAlarmParam;
        vecAlarmInfo.push_back(stAlarmInfo);
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetAlarmInfoBySNAndID
Description  : 根据sn和id获取sqlite数据中某条告警数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::GetAlarmInfoBySNAndID(mp_int32 iAlarmSN, mp_int32 iAlarmID, alarm_Info_t &stAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select "<<titleAlarmSerialNo<<"," << titleAlarmID<<"," << titleAlarmType<<"," \
        << titleAlarmLevel<<","<< titleStartTime<<","<<titleEndTime<<","<<titleAlarmParam <<" from " \
        <<AlarmTable <<" where "<< titleAlarmSerialNo <<" == ? and " <<titleAlarmID << " == ?;";

    mp_string sql = buff.str();
    
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    DbParamStream dps;
    DbParam dp = iAlarmSN;
    dps << dp;
    dp = iAlarmID;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return iRet;
    }

    for(mp_int32 iRow = 1; iRow <= iRowCount; ++iRow)
    {
        mp_string strAlarmSN = "";
        mp_string strAlarmID = "";
        mp_string strAlarmType = "";
        mp_string strAlarmLevel = "";
        readBuff >> strAlarmSN;
        readBuff >> strAlarmID;
        readBuff >> strAlarmType;
        readBuff >> strAlarmLevel;
        stAlarmInfo.iAlarmID = atoi(strAlarmID.c_str());
        stAlarmInfo.iAlarmSN = atoi(strAlarmSN.c_str());
        stAlarmInfo.iAlarmLevel = atoi(strAlarmLevel.c_str());
        stAlarmInfo.iAlarmType = atoi(strAlarmType.c_str());
        readBuff >> stAlarmInfo.strStartTime;
        readBuff >> stAlarmInfo.strEndTime;
        readBuff >> stAlarmInfo.strAlarmParam;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetCurrentAlarmInfoByAlarmID
Description  : 根据sn和id获取sqlite数据中某条当前告警数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::GetCurrentAlarmInfoByAlarmID(mp_string strAlarmID,alarm_Info_t &stAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select "<<titleAlarmSerialNo<<"," << titleAlarmID<<"," << titleAlarmType<<"," \
            << titleAlarmLevel<<","<< titleStartTime<<","<<titleEndTime<<","<<titleAlarmParam <<" from " \
            <<AlarmTable <<" where "<< titleAlarmID <<" == ?;";

    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = strAlarmID ;
    dps << dp;
    
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return iRet;
    }
    for(mp_int32 iRow = 1; iRow <= iRowCount; iRow++)
    {
        mp_string strAlarmSN = "";
        mp_string strReadAlarmID = "";
        mp_string strAlarmType = "";
        mp_string strAlarmLevel = "";
        readBuff >> strAlarmSN;
        readBuff >> strReadAlarmID;
        readBuff >> strAlarmType;
        readBuff >> strAlarmLevel;
        stAlarmInfo.iAlarmID = atoi(strAlarmID.c_str());
        stAlarmInfo.iAlarmSN = atoi(strAlarmSN.c_str());
        stAlarmInfo.iAlarmLevel = atoi(strAlarmLevel.c_str());
        stAlarmInfo.iAlarmType = atoi(strAlarmType.c_str());
        readBuff >> stAlarmInfo.strStartTime;
        readBuff >> stAlarmInfo.strEndTime;
        readBuff >> stAlarmInfo.strAlarmParam;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetAlarmInfoByParam
Description  : 根据告警参数获取sqlite数据中某条告警数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::GetAlarmInfoByParam(mp_int32 iAlarmID, mp_string strAlarmParam, alarm_Info_t &stAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select "<<titleAlarmSerialNo<<"," << titleAlarmID<<"," << titleAlarmType<<"," \
        << titleAlarmLevel<<","<< titleStartTime<<","<<titleEndTime<<","<<titleAlarmParam <<" from " \
        <<AlarmTable <<" where "<< titleAlarmParam <<" == ? and " <<titleAlarmID << " == ?;"; 
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = strAlarmParam;
    dps << dp;
    dp = iAlarmID;
    dps << dp;
    
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return iRet;
    }

    mp_string strAlarmSN = "";
    mp_string strAlarmID = "";
    mp_string strAlarmType = "";
    mp_string strAlarmLevel = "";
    readBuff >> strAlarmSN;
    if (!strAlarmSN.empty())
    {
        COMMLOG(OS_LOG_DEBUG,LOG_COMMON_DEBUG, "find record");
        readBuff >> strAlarmID;
        readBuff >> strAlarmType;
        readBuff >> strAlarmLevel;
        stAlarmInfo.iAlarmID = atoi(strAlarmID.c_str());
        stAlarmInfo.iAlarmSN = atoi(strAlarmSN.c_str());
        stAlarmInfo.iAlarmLevel = atoi(strAlarmLevel.c_str());
        stAlarmInfo.iAlarmType = atoi(strAlarmType.c_str());
        readBuff >> stAlarmInfo.strStartTime;
        readBuff >> stAlarmInfo.strEndTime;
        readBuff >> stAlarmInfo.strAlarmParam;
    }
    else
    {
        COMMLOG(OS_LOG_DEBUG,LOG_COMMON_DEBUG, "can not find record");
        stAlarmInfo.iAlarmSN = -1; //无效sn
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetSN
Description  : 获取告警流水号
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::GetSN(mp_int32& iAlarmSn)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select "<<titleAlarmSN<<" from "<<AlarmTypeTable;
    mp_string sql = buff.str();

    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    DbParamStream dps;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return iRet;
    }

    mp_string strAlarmSN = "";
    if (iRowCount >= 1)
    {
        readBuff >> strAlarmSN;
        iAlarmSn = atoi(strAlarmSN.c_str());
    }
    else
    {
        //如果数据库中没有，则插入一个alarmID的条目，SN从0开始计数
        buff.str("");
        buff <<"insert into "<<AlarmTypeTable <<"( "<<titleAlarmSN<<") values(?);";
        dps.Clear();
        DbParam dp = 0;
        dps << dp;
        iRet = db.ExecSql(buff.str(), dps);
        if (0 != iRet )
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"db.ExecSql failed,iRet = %d.",iRet);
            return iRet;
        }
        iAlarmSn = 0;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: SetSN
Description  : 将告警流水号存入sqlite数据库
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::SetSN(mp_int32 iAlarmSn)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select "<<titleAlarmSN<<" from "<<AlarmTypeTable;

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

    if (iRowCount >= 1)
    {
        COMMLOG(OS_LOG_DEBUG,LOG_COMMON_DEBUG,"find record in AlarmType table");
        buff.str("");
        buff <<"update "<<AlarmTypeTable <<" set "<<titleAlarmSN<<" = ?;";
        mp_string sql = buff.str();
        COMMLOG(OS_LOG_DEBUG,LOG_COMMON_DEBUG,"sql:%s",sql.c_str());
        
        dps.Clear();
        DbParam dp = iAlarmSn;
        dps << dp;
        iRet = db.ExecSql(sql, dps);
        
        if (0 != iRet)
        {
            COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR,"db.ExecSql failed, iRet = %d.",iRet);
            return iRet;
        }
    }
    else
    {
        COMMLOG(OS_LOG_DEBUG,LOG_COMMON_DEBUG,"can not find record in AlarmType table");
        //如果数据库中没有，则插入一个alarmID的条目
        buff.str("");
        buff <<"insert into "<<AlarmTypeTable <<"( "<<titleAlarmSN<<") values(?);";
        mp_string sql = buff.str();
        dps.Clear();
        DbParam dp = iAlarmSn;
        dps << dp;
        iRet = db.ExecSql(sql, dps);
        if (0 != iRet )
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"db.ExecSql failed, iRet = %d.",iRet);
            return iRet;
        }
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: InsertTrapServer
Description  : 在数据库中插入一条trap server信息
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::InsertTrapServer(trap_server& stTrapServer)
{
    LOGGUARD("");
    mp_int32 iRet = CheckTrapInfoTable();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CheckTrapInfoTable failed, iRet = %d.",iRet);
        return iRet;
    }
    
    if (BeExistInTrapInfo(stTrapServer))
    {
        //已经存在，打印日志
        COMMLOG(OS_LOG_INFO,LOG_COMMON_INFO,"%s is exist",stTrapServer.strServerIP.c_str());
        return MP_SUCCESS;
    }

    ostringstream buff;
    buff << "insert into " << TrapServerTable << "(TrapServerIP, TrapPort, SnmpVersion, AgentIP) values(?, ?, ?, ?);";
    mp_string sql = buff.str();
    CDB &db = CDB::GetInstance();
    DbParamStream dps;
    DbParam dp = stTrapServer.strServerIP;
    dps << dp;
    dp = stTrapServer.iPort;
    dps << dp;
    dp = stTrapServer.iVersion;
    dps << dp;
    dp = stTrapServer.strListenIP;
    dps << dp;
    iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
    }
    return iRet;
}

/*------------------------------------------------------------
Function Name: DeleteTrapServer
Description  : 在数据库中删除一条trap server信息
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::DeleteTrapServer(trap_server& stTrapServer)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "delete from " <<TrapServerTable <<" where TrapServerIP == ? and TrapPort == ? and SnmpVersion == ?;";
    mp_string sql = buff.str();
    
    DbParamStream dps;
    DbParam dp = stTrapServer.strServerIP;
    dps << dp;
    dp = stTrapServer.iPort;
    dps << dp;
    dp = stTrapServer.iVersion;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.ExecSql(sql ,dps);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
    }
    return iRet;
}

/*------------------------------------------------------------
Function Name: GetAllTrapInfo
Description  : 查询数据中所有的trap server信息
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::GetAllTrapInfo(vector<trap_server>& vecStServerInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select TrapServerIP,TrapPort,SnmpVersion,AgentIP from " <<TrapServerTable;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    CDB &db = CDB::GetInstance();
    DbParamStream dps;

    mp_int32 iRet = CheckTrapInfoTable();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CheckTrapInfoTable failed, iRet = %d.",iRet);
        return iRet;
    }
    
    iRet = db.QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return iRet;
    }

    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow)
    {
        trap_server stServerInfo;
        mp_string strServerIP = "";
        mp_string strPort = "";
        mp_string strVersion = "";
        mp_string strListenIP = "";
        readBuff >> strServerIP;
        readBuff >> strPort;
        readBuff >> strVersion;
        readBuff >> strListenIP;
        stServerInfo.strServerIP = strServerIP;
        stServerInfo.iPort = atoi(strPort.c_str());
        stServerInfo.iVersion = atoi(strVersion.c_str());
        stServerInfo.strListenIP = strListenIP;
        vecStServerInfo.push_back(stServerInfo);
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetAllTrapInfo
Description  : 查询数据中是否存在指定的trap server信息
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CAlarmDB::BeExistInTrapInfo(trap_server& stTrapServer)
{
    LOGGUARD("");
    ostringstream buff;
    buff <<"select TrapServerIP,TrapPort,SnmpVersion from " <<TrapServerTable 
          <<" where TrapServerIP == ? and TrapPort == ? and SnmpVersion == ?;";

    DbParamStream dps;
    DbParam dp = stTrapServer.strServerIP;
    dps << dp;
    dp = stTrapServer.iPort;
    dps << dp;
    dp = stTrapServer.iVersion;
    dps << dp;

    mp_string strSql = buff.str();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "buff is = %s.", strSql.c_str());
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return false;
    }

    if (0 < iRowCount)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*------------------------------------------------------------
Function Name: CheckTrapInfoTable
Description  : 检查表中是否存在AgentIP字段，如果不存在AgentIP则创建此列
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAlarmDB::CheckTrapInfoTable()
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
        
    // check col if exists
    CDB &db = CDB::GetInstance();
    DbParamStream dps;
    iRet = db.QueryTable(CHECKT_TRAP_COL_EXISTS, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query col AgentIP from TrapInfoTable failed, iRet = %d.", iRet);
        return iRet;
    }

    // 如果没有AgentIP列，直接生成对应的列，并完成数据内容更新
    if (iRowCount == 0)
    { 
        dps.Clear();
        iRet = db.ExecSql(ADD_TRAP_COL, dps);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "Add column AgentIP in TrapInfoTable failed, iRet = %d.", iRet);
            return iRet;
        }
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Add AgentIP column in table TrapInfoTable succeeded.");
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Table TrapInfoTable have AgentIP column, do nothing.");
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetSnmpV3Param
Description  : 从配置文件中获取snmp v3相关参数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CAlarmConfig::GetSnmpV3Param(snmp_v3_param& stSnmpV3Param)
{
    LOGGUARD("");
    mp_string strPrivatePw;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_PRIVATE_PASSWOD, strPrivatePw);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:private_password value from xml config failed");
        stSnmpV3Param.strPrivPassword = "";
    }
    else
    {
        //解密
        DecryptStr(strPrivatePw, stSnmpV3Param.strPrivPassword);
    }

    mp_string strAuthPw;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_AUTH_PASSWORD, strAuthPw);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:auth_password value from xml config failed");
        stSnmpV3Param.strAuthPassword = "";
    }
    else
    {
        //解密
        DecryptStr(strAuthPw, stSnmpV3Param.strAuthPassword);
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_SECURITY_NAME, stSnmpV3Param.strSecurityName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:security_name value from xml config failed");
        stSnmpV3Param.strSecurityName = "";
    }

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SNMP_SECTION, CFG_SECURITY_MODEL, stSnmpV3Param.iSecurityModel);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:security_model value from xml config failed");
        stSnmpV3Param.iSecurityModel = 0;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SNMP_SECTION, CFG_AUTH_PROTOCOL, stSnmpV3Param.iAuthProtocol);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:auth_protocol value from xml config failed");
        stSnmpV3Param.iAuthProtocol = 0;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SNMP_SECTION, CFG_PRIVATE_PROTOCOL, stSnmpV3Param.iPrivProtocol);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:private_protocol value from xml config failed");
        stSnmpV3Param.iPrivProtocol = 0;
    }
}


