/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _TRAP_H_
#define _TRAP_H_

#include "common/Types.h"
#include "snmp_pp/snmp_pp.h"
#include "alarm/Alarm.h"

#define MAX_ALARM_ID 0x7FFFFFFF
#define OID_HUAWEI_TRAP_MODEL "1.3.6.1.4.1.2011.2.91.10.2.1.0.1"

//告警相关
#define ALARM_ID_CPUUSAGE 0x3230015  //CPU利用率告警ID
#define ALARM_ID_THAWFAILED 0x3230019  //数据库解冻失败告警ID
#define ALARM_TYPE_EQUPMENTFAULT 2 //设备告警

//告警级别
typedef enum euAlarmLevel
{
    ALARM_LEVEL_NULL,
    ALARM_LEVEL_CRITICAL,
    ALARM_LEVEL_MAJOR,
    ALARM_LEVEL_MINOR,
    ALARM_LEVEL_WARNING
}euAlarmLevel;

//告警故障类别
typedef enum euAlarmCategory
{
    ALARM_CATEGORY_NULL,   //空
    ALARM_CATEGORY_FAULT,  //故障告警
    ALARM_CATEGORY_RESUME //恢复告警
}euAlarmCategory;

//CPU MIBOID
#define OID_ISM_ALARM_REPORTING "1.3.6.1.4.1.2011.2.91.10.2.1.0.1"  //hwIsmAlarmReporting
#define OID_ISM_ALARM_REPORTING_NODECODE "1.3.6.1.4.1.2011.2.91.10.3.1.1.1"   //hwIsmReportingAlarmNodeCode
#define OID_ISM_ALARM_REPORTING_LOCATIONINFO "1.3.6.1.4.1.2011.2.91.10.3.1.1.2" //hwIsmReportingAlarmLocationInfo
#define OID_ISM_ALARM_REPORTING_RESTOREADVICE "1.3.6.1.4.1.2011.2.91.10.3.1.1.3" //hwIsmReportingAlarmRestoreAdvice
#define OID_ISM_ALARM_REPORTING_FAULTTITLE "1.3.6.1.4.1.2011.2.91.10.3.1.1.4" //hwIsmReportingAlarmFaultTitle
#define OID_ISM_ALARM_REPORTING_FAULTTYPE "1.3.6.1.4.1.2011.2.91.10.3.1.1.5" //hwIsmReportingAlarmFaultType
#define OID_ISM_ALARM_REPORTING_FAULTLEVEL "1.3.6.1.4.1.2011.2.91.10.3.1.1.6" //hwIsmReportingAlarmFaultLevel
#define OID_ISM_ALARM_REPORTING_ALARMID "1.3.6.1.4.1.2011.2.91.10.3.1.1.7" //hwIsmReportingAlarmAlarmID
#define OID_ISM_ALARM_REPORTING_FAULTTIME "1.3.6.1.4.1.2011.2.91.10.3.1.1.8" //hwIsmReportingAlarmFaultTime
#define OID_ISM_ALARM_REPORTING_SERIALNO "1.3.6.1.4.1.2011.2.91.10.3.1.1.9" //hwIsmReportingAlarmSerialNo
#define OID_ISM_ALARM_REPORTING_ADDITIONINFO "1.3.6.1.4.1.2011.2.91.10.3.1.1.10" //hwIsmReportingAlarmAdditionInfo
#define OID_ISM_ALARM_REPORTING_FAULTCATEGORY "1.3.6.1.4.1.2011.2.91.10.3.1.1.11" //hwIsmReportingAlarmFaultCategory
#define OID_ISM_ALARM_REPORTING_LOCATIONID "1.3.6.1.4.1.2011.2.91.10.3.1.1.12" //hwIsmReportingAlarmLocationAlarmID

typedef struct pdu_security_info_st
{
    pdu_security_info_st()
    {
        iSecurityLevel = 0;
        strContextName = "";
        strContextEngineID = "";
    }
    mp_int32 iSecurityLevel;
    mp_string strContextName;
    mp_string strContextEngineID;
}pdu_security_info;

//发送告警的必须参数
typedef struct alarm_param_st
{
    alarm_param_st()
    {
        iAlarmID = -1;
        strAlarmParam = "";
    }
    mp_int32 iAlarmID;
    mp_string strAlarmParam;
}alarm_param_t;

class CTrapSender
{
public:
    static mp_int32 SendAlarm(alarm_param_t& alarmParam);
    static mp_int32 ResumeAlarm(alarm_param_t& alarmParam);

private:
    static mp_void SendSingleTrap(Pdu &pdu, trap_server& trapServer, mp_int32 securityModel, OctetStr& securityName);
    static mp_void SendTrap(Pdu &pdu, vector<trap_server> &vecServerInfo);
    static mp_void ConstructPDUCommon(Pdu &pdu);
    static mp_bool ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu);
    static mp_bool GetPduSecurInfo(pdu_security_info &stPduSecurInfo);
    static mp_string GetLocalNodeCode();
    static mp_int32 NewAlarmRecord(alarm_param_t &alarmParam, alarm_Info_t &alarmInfo);
};

#endif

