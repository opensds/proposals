/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _ALARM_H_
#define _ALARM_H_

#include "common/Types.h"
#include <vector>

#define DEFAULT_PORT_NUM 162
#define DEFAULT_VERSION 3

#define CHECKT_TRAP_COL_EXISTS "select * from [sqlite_master] where name = 'TrapInfoTable' and sql like '%AgentIP%';"
#define ADD_TRAP_COL "alter table [TrapInfoTable] add AgentIP VARCHAR(30) NOT NULL DEFAULT ('Unknown');"

//SNMP协议类型
enum
{
    SNMP_V1 = 1,
    SNMP_V2C,
    SNMP_V3
};

typedef struct alarm_Info_st
{
    mp_int32 iAlarmSN;
    mp_int32 iAlarmID;
    mp_int32 iAlarmType;
    mp_int32 iAlarmLevel;
    mp_int32 iAlarmCategoryType;
    mp_string strStartTime;
    mp_string strEndTime;
    mp_string strAlarmParam;
    alarm_Info_st():iAlarmSN(-1),iAlarmID(-1),
        iAlarmType(-1),iAlarmLevel(-1),iAlarmCategoryType(-1),
        strStartTime(""),strEndTime(""),
        strAlarmParam("")
    {
    }
}alarm_Info_t;

typedef struct trap_server_st
{
    mp_int32 iPort;
    mp_int32 iVersion;
    mp_string strServerIP;
    mp_string strListenIP;  //本机监听IP，当本机存在多个IP时，注册TRAP时使用注册时的IP，防止trap收不到信息
    trap_server_st(): strServerIP("0.0.0.0"),iPort(DEFAULT_PORT_NUM),iVersion(DEFAULT_VERSION)
    {
    }

}trap_server;

typedef struct snmp_v3_param_st
{
    mp_string strPrivPassword;
    mp_string strAuthPassword;
    mp_string strSecurityName;
    mp_int32 iSecurityModel;
    mp_int32 iAuthProtocol;
    mp_int32 iPrivProtocol;
    mp_string strEngineId;
    mp_string strFilename;
}snmp_v3_param;

class CAlarmDB
{
public:
    static mp_int32 InsertAlarmInfo(alarm_Info_t &stAlarmInfo);
    static mp_int32 DeleteAlarmInfo(mp_int32 iAlarmSN, mp_int32 iAlarmID);
    static mp_int32 UpdateAlarmInfo(alarm_Info_t &stAlarmInfo);
    static mp_int32 GetAllAlarmInfo(vector<alarm_Info_t> &vecAlarmInfo);
    static mp_int32 GetAlarmInfoBySNAndID(mp_int32 iAlarmSN, mp_int32 iAlarmID, alarm_Info_t &stAlarmInfo);
    static mp_int32 GetCurrentAlarmInfoByAlarmID(mp_string strAlarmID,alarm_Info_t &stAlarmInfo);
    static mp_int32 GetAlarmInfoByParam(mp_int32 iAlarmID, mp_string strAlarmParam, alarm_Info_t &stAlarmInfo);
    static mp_int32 GetSN(mp_int32& iAlarmSn);
    static mp_int32 SetSN(mp_int32 iAlarmSn);
    static mp_int32 InsertTrapServer(trap_server& stTrapServer);
    static mp_int32 DeleteTrapServer(trap_server& stTrapServer);
    static mp_int32 GetAllTrapInfo(vector<trap_server>& vecStServerInfo);

private:
    static mp_bool BeExistInTrapInfo(trap_server& stTrapServer);
	static mp_int32 CheckTrapInfoTable();
};

class CAlarmConfig
{
public:
    static mp_void GetSnmpV3Param(snmp_v3_param& stSnmpV3Param);
};
#endif

