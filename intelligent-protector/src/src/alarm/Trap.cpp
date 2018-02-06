/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "alarm/Trap.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Mac.h"
#include "common/Path.h"
#include "rest/Interfaces.h"

/*------------------------------------------------------------
Function Name:SendAlarm
Description  :发送告警
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CTrapSender::SendAlarm(alarm_param_t &alarmParam)
{
    LOGGUARD("");
    //通过告警参数查询db中是否存在同样告警
    alarm_Info_t alarmInfo;

    mp_int32 iRet = CAlarmDB::GetAlarmInfoByParam(alarmParam.iAlarmID, alarmParam.strAlarmParam, alarmInfo);
    if (MP_SUCCESS != iRet)
    {
        //记录日志
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetAlarmInfoByParam failed, alarmID = %d, alarmParam = %s",
             alarmParam.iAlarmID, alarmParam.strAlarmParam.c_str());
        return iRet;
    }

    //如果db中没有相同告警，新生成一条
    if (alarmInfo.iAlarmSN == -1)
    {
        iRet = NewAlarmRecord(alarmParam, alarmInfo);
        if (MP_SUCCESS != iRet)
        {
            //记录日志
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "NewAlarmRecord failed, alarmID = %d, alarmParam=%s",
                 alarmParam.iAlarmID, alarmParam.strAlarmParam.c_str());
            return iRet;
        }
        else
        {
            //记录日志
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "newAlarmRecord success, alarmID = %d, alarmParam=%s",
                 alarmParam.iAlarmID, alarmParam.strAlarmParam.c_str());
        }
    }

    //获取trap server地址信息
    vector<trap_server> vecServerInfo;
    iRet = CAlarmDB::GetAllTrapInfo(vecServerInfo);
    if (MP_SUCCESS != iRet)
    {
        //记录日志
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get TRAP server info from database failed.");
        return iRet;
    }

    Pdu pdu;
    ConstructPDU(alarmInfo, pdu);
    SendTrap(pdu, vecServerInfo);

    //记录日志
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Send alarm(ID = %d, Param= \"%s\") to server sucess.",
                 alarmInfo.iAlarmID, alarmInfo.strAlarmParam.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:SendAlarm
Description  :发送恢复告警
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CTrapSender::ResumeAlarm(alarm_param_t &stAlarm)
{
    LOGGUARD("");
    alarm_Info_t alarmInfo;
    alarmInfo.iAlarmID = stAlarm.iAlarmID;
    alarmInfo.strAlarmParam = stAlarm.strAlarmParam;
    //通过告警参数查询db中是否存在同样告警
    mp_int32 iRet = CAlarmDB::GetAlarmInfoByParam(stAlarm.iAlarmID, stAlarm.strAlarmParam, alarmInfo);
    if (MP_SUCCESS != iRet)
    {
        //记录日志
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetAlarmInfoByParam failed, alarmID = %d, alarmParam = %s",
             stAlarm.iAlarmID, stAlarm.strAlarmParam.c_str());
        return iRet;
    }

    //找到了告警记录，直接删除
    if (alarmInfo.iAlarmSN != -1)
    {
        iRet = CAlarmDB::DeleteAlarmInfo(alarmInfo.iAlarmSN, alarmInfo.iAlarmID);
        if (MP_SUCCESS != iRet)
        {
            //记录日志
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete alarm failed, alarmID = %d, alarmSN = %d",
                 alarmInfo.iAlarmID, alarmInfo.iAlarmSN);
            return iRet;
        }
        else
        {
            //记录日志
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete alarm success, alarmID = %d, alarmSN = %d",
                 alarmInfo.iAlarmID, alarmInfo.iAlarmSN);
        }
    }

    alarmInfo.iAlarmCategoryType = ALARM_CATEGORY_RESUME;
    //获取trap server地址信息
    vector<trap_server> vecServerInfo;
    iRet = CAlarmDB::GetAllTrapInfo(vecServerInfo);
    if (MP_SUCCESS != iRet)
    {
        //记录日志
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get TRAP server info from database failed.");
        return iRet;
    }

    Pdu pdu;
    ConstructPDU(alarmInfo, pdu);
    SendTrap(pdu, vecServerInfo);

    //记录日志
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Resume alarm(ID = %d, Param = \"%s\") to server sucess.",
                 stAlarm.iAlarmID, stAlarm.strAlarmParam.c_str());
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 发送单个Trap到Trap server
Input        : 
Output       : 
Return       :  
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CTrapSender::SendSingleTrap(Pdu &pdu, trap_server& trapServer, mp_int32 securityModel, OctetStr& securityName)
{
	//CodeDex误报,KLOCWORK.RH.LEAK
    UdpAddress address(trapServer.strServerIP.c_str() );
    address.set_port((unsigned short)trapServer.iPort);
    Snmp *snmp;
    mp_int32 status = 0;

    try
    {
        if (address.get_ip_version() == Address::version_ipv4)
        {
            if (trapServer.strListenIP.compare(UNKNOWN) == 0)
            {
                snmp = new Snmp(status, "0.0.0.0");
            }
            else   //排除获取Agent监听IP失败的情况
            {
                snmp = new Snmp(status, trapServer.strListenIP.c_str());
            }
        }
        else
        {
            snmp = new Snmp(status, "::");
        }
    }
    catch(...)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New Snmp failed.");
        return;
    }

    if (NULL == snmp)
    {
        return;
    }

    if (SNMP_CLASS_SUCCESS != status)
    {
        delete(snmp);
        return;
    }

    UTarget utarget(address);
    utarget.set_version((snmp_version)trapServer.iVersion);    //目前只实现SNMPV3，故写死
    utarget.set_security_model(securityModel);
    utarget.set_security_name(securityName);

    COMMLOG(OS_LOG_INFO,LOG_COMMON_INFO,"Send Trap to:%s with %s.", trapServer.strServerIP.c_str(), trapServer.strListenIP.c_str());
    status = snmp->trap(pdu,utarget);
    delete(snmp);
}

/*------------------------------------------------------------
Function Name:SendTrap
Description  :发送trap到trap server
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CTrapSender::SendTrap(Pdu &pdu, vector<trap_server> &vecServerInfo)
{
    ////CodeDex误报,KLOCWORK.RH.LEAK
    LOGGUARD("");
    Snmp::socket_startup();
    snmp_v3_param stSnmpV3Param;
    CAlarmConfig::GetSnmpV3Param(stSnmpV3Param);

    //V3参数
    OctetStr privPassword(stSnmpV3Param.strPrivPassword.c_str());
    OctetStr authPassword(stSnmpV3Param.strAuthPassword.c_str());
    OctetStr securityName(stSnmpV3Param.strSecurityName.c_str());
    mp_int32 securityModel = stSnmpV3Param.iSecurityModel;
    mp_int64 authProtocol = stSnmpV3Param.iAuthProtocol;
    mp_int64 privProtocol = stSnmpV3Param.iPrivProtocol;
    v3MP *v3_MP;

    mp_string strTmpEngineId = GetLocalNodeCode();
    //引擎计数器写死为1，不做文件操作处理
    mp_uint32 snmpEngineBoots = 1;

    mp_int32 construct_status = 0;
    try
    {
        v3_MP = new v3MP(strTmpEngineId.c_str(), snmpEngineBoots, construct_status);
    }
    catch(...)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New v3MP failed.");
        v3_MP = NULL;
    }
    
    if (!v3_MP)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "v3MP is NULL.");
        return;
    }
    if (SNMPv3_MP_OK != construct_status)
    {
        delete(v3_MP);
        return;
    }

    //对v3_MP空指针判断不需要，coverity检查提示是死代码，屏蔽对应pclint告警。
    USM *usm = v3_MP->get_usm();  //lint !e613
    usm->add_usm_user(securityName,
        (long)authProtocol, (long)privProtocol,
        authPassword, privPassword);

    vector<trap_server>::iterator it = vecServerInfo.begin();
    for (;vecServerInfo.end() != it;  ++it)
    {
        SendSingleTrap(pdu, *it, securityModel, securityName);
    }
    delete(v3_MP);
    Snmp::socket_cleanup();
}//lint !e429

/*------------------------------------------------------------
Function Name:ConstructPDUCommon
Description  :根据告警信息构造pdu数据，供ConstructPDU调用
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CTrapSender::ConstructPDUCommon(Pdu &pdu)
{
    Vb vb;
    vb.set_oid(OID_ISM_ALARM_REPORTING_NODECODE);
    vb.set_value(GetLocalNodeCode().c_str());
    pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_LOCATIONINFO);
    //vb.set_value("");
    //pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_RESTOREADVICE);
    //vb.set_value("");
    //pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTTITLE);
    //vb.set_value("");
    //pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTTYPE);
    vb.set_value(ALARM_TYPE_EQUPMENTFAULT);
    pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTLEVEL);
    //vb.set_value(ALARM_LEVEL_MAJOR);
    //pdu += vb;
    
    vb.clear();
    mp_time time;
    CMpTime::Now(&time);
    mp_string strNowTime = CMpTime::GetTimeString(&time);
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTTIME);
    vb.set_value(strNowTime.c_str());
    pdu += vb;
}

/*------------------------------------------------------------
Function Name:ConstructPDU
Description  :根据告警信息构造pdu数据
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CTrapSender::ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu)
{
    LOGGUARD("");
    //华为trap上报模块OID
    Oid oid( OID_HUAWEI_TRAP_MODEL );
    pdu_security_info stPduSecurInfo;
    (mp_void)GetPduSecurInfo(stPduSecurInfo);

    int securityLevel = stPduSecurInfo.iSecurityLevel;
    OctetStr contextName(stPduSecurInfo.strContextName.c_str());
    OctetStr contextEngineID(stPduSecurInfo.strContextEngineID.c_str());
    Vb vb;
    
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_ALARMID);
    vb.set_value(stAlarm.iAlarmID);
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_SERIALNO);
    vb.set_value(stAlarm.iAlarmSN);
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_ADDITIONINFO);
    vb.set_value(stAlarm.strAlarmParam.c_str());
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTCATEGORY);
    vb.set_value(stAlarm.iAlarmCategoryType);
    pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_LOCATIONID);
    //vb.set_value("");
    //pdu += vb;
    ConstructPDUCommon(pdu);
    pdu.set_notify_id(oid);
    pdu.set_notify_enterprise(oid);
    pdu.set_security_level(securityLevel);
    pdu.set_context_name (contextName);
    pdu.set_context_engine_id(contextEngineID);
    return MP_TRUE;
}

/*------------------------------------------------------------
Function Name:ConstructPDU
Description  :从配置文件中读取snmp相关安全配置
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CTrapSender::GetPduSecurInfo(pdu_security_info &stPduSecurInfo)
{
    LOGGUARD("");
    //从配置文件中获取安全级别
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SNMP_SECTION, CFG_SECURITY_LEVEL, stPduSecurInfo.iSecurityLevel);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:security_level value from xml config failed");
        stPduSecurInfo.iSecurityLevel = 3;
    }
    //从配置文件中获取context name
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_CONTEXT_NAME, stPduSecurInfo.strContextName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:context_name value from xml config failed");
        stPduSecurInfo.strContextName = "";
    }
    //从配置文件中获取context engine id
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_ENGINE_ID, stPduSecurInfo.strContextEngineID);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:engine_id value from xml config failed");
        stPduSecurInfo.strContextEngineID = "";
    }

    //COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "TrapSender::GetPduSecurInfo Get secritylevel is %d, contextname is %s,contextengineid is %s",
        //stPduSecurInfo.iSecurityLevel, stPduSecurInfo.strContextName.c_str(), stPduSecurInfo.strContextEngineID.c_str());
    return MP_TRUE;
}

/*------------------------------------------------------------
Function Name:GetLocalNodeCode
Description  :获取本主机mac地址
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_string CTrapSender::GetLocalNodeCode()
{
    LOGGUARD("");
    vector<mp_string> vecMacs; 
    mp_string strHostsnFile = CPath::GetInstance().GetConfFilePath(HOSTSN_FILE);

    mp_int32 iRet = CMpFile::ReadFile(strHostsnFile, vecMacs);
    if (MP_SUCCESS != iRet || 0 == vecMacs.size())
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host sn failed, iRet = %d, size of vecMacs is %d", iRet, vecMacs.size());
         return "";
    }
    return vecMacs.front();
}

/*------------------------------------------------------------
Function Name:NewAlarmRecord
Description  :新产生一条告警记录，并存入到sqlite数据库
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CTrapSender::NewAlarmRecord(alarm_param_t &alarmParam, alarm_Info_t &alarmInfo)
{
    //获取新的sn
    LOGGUARD("");
    mp_int32 iCurrSN = 0;
    mp_int32 iRet = CAlarmDB::GetSN(iCurrSN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get CurSN Failed iRet is: %d", iRet);
        return iRet;
    }

    alarmInfo.iAlarmSN = (iCurrSN == MAX_ALARM_ID) ? 0 : iCurrSN + 1; //新流水号
    alarmInfo.iAlarmID = alarmParam.iAlarmID;
    //alarmInfo.iAlarmLevel = ALARM_LEVEL_MAJOR;
    alarmInfo.strAlarmParam = alarmParam.strAlarmParam;
    alarmInfo.iAlarmType = ALARM_TYPE_EQUPMENTFAULT;
    alarmInfo.strEndTime = "";
    mp_time time;
    CMpTime::Now(&time);
    mp_string strNowTime = CMpTime::GetTimeString(&time);
    alarmInfo.strStartTime = strNowTime;
    alarmInfo.iAlarmCategoryType = ALARM_CATEGORY_FAULT;
    iRet = CAlarmDB::InsertAlarmInfo(alarmInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "InsertAlarmInfo failed");
        return iRet;
    }

    //更新流水号
    iRet = CAlarmDB::SetSN(alarmInfo.iAlarmSN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SetSN Failed iRet is: %d",iRet);
        return iRet;
    }

    return MP_SUCCESS;
}


