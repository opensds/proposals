/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "stub.h"
#include "alarm/AlarmTest.h"
#include "alarm/Alarm.h"
#include "alarm/AppFreezeStatus.h"
#include "alarm/Db.h"
#include "alarm/Trap.h"
#include "common/Log.h"
#include "common/Types.h"
#include "common/ConfigXmlParse.h"
#include "common/Types.h"
#include "common/Path.h"
#include <cstdlib>
#include <vector>

using namespace std;

static mp_void StubCLoggerLog(mp_void){
    return;
}

static mp_int32 StubCConXmlPGetVaStr(mp_string strin, mp_string strin2, mp_string& strValue)
{
    strValue = "401C8F36B7DF2DA9336BAF749790A3F31386F8483E3163FE2D6661900A445B3E66F5F1F5213CB99437546D71987AEC97";
    return MP_FAILED;
}

static mp_int32 StubExecSql()
{
   return MP_FAILED;
    
}

static mp_int32 StubExecSqlSU()
{
   return MP_SUCCESS;
    
}


static mp_int32 GetValIntT()
{
    return MP_FAILED;
}


static mp_int32 GetValStrT()
{
    return MP_FAILED;
}

static mp_int32 StubSqlite3_open()
{
    cout << "exec stub sqlite3_open." << endl;
    return SQLITE_OK;
}

static mp_string StubGetDbFilePath()
{
    return mp_string("zwgTest.db");
}

static mp_int32 StubQueryTable(void* This, mp_string strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    readBuff.Clear();
    readBuff.Empty();
    mp_string strInput1 = "1";
    readBuff << strInput1;
    readBuff << strInput1;
    readBuff << strInput1;
    readBuff << strInput1;
    readBuff << strInput1;
    readBuff << strInput1;
    readBuff << strInput1;
    iRowCount = 1;
    iColCount = 1;
    return MP_SUCCESS;
}


static mp_int32 StubQueryTableZero(void* This, mp_string strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    iRowCount = 0;
    iColCount = 0;
    return MP_SUCCESS;
}

static mp_int32 CreateTable(mp_string strSql)
{
    LOGGUARD("");
    CDB &db = CDB::GetInstance();
    DbParamStream dps;
    mp_int32 iRet = db.ExecSql(strSql, dps);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "Create table failed, iRet = %d.", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create table succeeded.");
    }
    return iRet;
}

TEST_F(CAlarmDBTest, OperSqlittest)
{
    try
    {
        //  打桩Log 函数，防止出现Segmentation fault错误
        typedef mp_void (*StubFuncType)(void);
        typedef mp_void (CLogger::*LogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
        Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);

        /********Begin Db.cpp test********/
        CDB &db = CDB::GetInstance();
        DBReader readBuff;
        mp_int32 iRowCount = 0;
        mp_int32 iColCount = 0;

        DbParamStream dps;
        mp_int32 iRet = db.QueryTable("sss",dps, readBuff, iRowCount, iColCount);
        EXPECT_EQ(iRet, MP_FAILED); 

        iRet = db.ExecSql("ssss",dps);
        EXPECT_EQ(iRet, MP_FAILED);

        typedef mp_int32 (*TypeStubConn)(mp_void);
        typedef mp_int32 (*Typesqlit)(const char *, sqlite3 **);
        Stub<Typesqlit, TypeStubConn, void> StubCon(&sqlite3_open, &StubSqlite3_open);

        iRet = db.QueryTable("sss", dps, readBuff, iRowCount, iColCount);
        EXPECT_EQ(iRet, MP_FAILED); 

        iRet = db.ExecSql("ssss", dps);
        EXPECT_EQ(iRet, MP_FAILED);
        /********End Db.cpp test********/
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    }
}


TEST_F(CAlarmDBTest, OperDBtest)
{
   try
    {
        //  打桩Log 函数，防止出现Segmentation fault错误
        typedef mp_void (*StubFuncType)(void);
        typedef mp_void (CLogger::*LogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
        Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);
    
        alarm_Info_t stAlarmInfo;
        trap_server stTrapServer;
        typedef mp_int32 (*TypeEx)(mp_void);
        typedef mp_int32 (CDB::*TypeExecSql)(mp_string, DbParamStream&);

        typedef mp_int32 (CDB::*TypeQueryT)(mp_string, DbParamStream&, DBReader&, mp_int32&, mp_int32&);
        typedef mp_int32 (*StubTypeQu)(void*, mp_string, DbParamStream&, DBReader&, mp_int32&, mp_int32&);

        Stub<TypeQueryT, StubTypeQu, void> StubQuta(&CDB::QueryTable, &StubQueryTable);
        Stub<TypeExecSql, TypeEx, void> StubExSql(&CDB::ExecSql, &StubExecSql);

        /********Begin Alarm.cpp test********/
        
        stAlarmInfo.iAlarmCategoryType = 1;
        stAlarmInfo.iAlarmID =2;
        stAlarmInfo.iAlarmLevel = 3;
        stAlarmInfo.iAlarmSN = 4;
        stAlarmInfo.iAlarmType = 5;
        stAlarmInfo.strAlarmParam = "this is alarm param";
        stAlarmInfo.strEndTime = "00:00:00";
        stAlarmInfo.strStartTime = "00:00:00";

        mp_int32 iRet = CAlarmDB::InsertAlarmInfo(stAlarmInfo);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = CAlarmDB::DeleteAlarmInfo(1, 2);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = CAlarmDB::DeleteAlarmInfo(1, 2);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = CAlarmDB::UpdateAlarmInfo(stAlarmInfo);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = CAlarmDB::DeleteTrapServer(stTrapServer);
        EXPECT_EQ(iRet, MP_FAILED);

        stTrapServer.iPort = 59526;
        stTrapServer.iVersion = 3;
        stTrapServer.strServerIP = "100.136.25.95";
        iRet = CAlarmDB::InsertTrapServer(stTrapServer);
        EXPECT_EQ(iRet, MP_SUCCESS);

        /******************************************/
        vector<alarm_Info_t> vecAlarmInfo;

        iRet = CAlarmDB::GetAllAlarmInfo(vecAlarmInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = CAlarmDB::GetAlarmInfoBySNAndID(1, 2, stAlarmInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = CAlarmDB::GetCurrentAlarmInfoByAlarmID("1", stAlarmInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

       
        iRet = CAlarmDB::GetAlarmInfoByParam(1, "this is param", stAlarmInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        vector<trap_server> vecStServerInfo;
        iRet = CAlarmDB::GetAllTrapInfo(vecStServerInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        mp_int32 iAlarmSn = 0;
        iRet = CAlarmDB::GetSN(iAlarmSn);
        EXPECT_EQ(iRet, MP_SUCCESS);
       

        iRet = CAlarmDB::SetSN(1);
        EXPECT_EQ(iRet, MP_FAILED);  

        /********End Alarm.cpp test********/

        /********Begin AppFreezeStatus.cpp test********/
        CAppFreezeStatus oCAppFree;
        freeze_status stStatus;
        stStatus.strKey = "/home/tyj";
        stStatus.iStatus = 0;

        iRet = oCAppFree.Insert(stStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = oCAppFree.Delete(stStatus);
        EXPECT_EQ(iRet, MP_FAILED);

        oCAppFree.Get(stStatus);
        EXPECT_EQ(stStatus.iStatus, DB_FREEZE);

        vector<freeze_status> vecStatus;
        iRet = oCAppFree.GetAll(vecStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);
        /********End AppFreezeStatus.cpp test********/

        /********Begin Trap.cpp test********/

        typedef mp_int32 (CConfigXmlParser::*TypeGetInt)(mp_string, mp_string, mp_int32&);
        typedef mp_int32 (CConfigXmlParser::*TypeGetStr)(mp_string, mp_string, mp_string&);

        Stub<TypeExecSql, TypeEx, void> StubExSqlSu(&CDB::ExecSql, &StubExecSqlSU);
        Stub<TypeGetInt, TypeEx, void> StubGetInt(&CConfigXmlParser::GetValueInt32, &GetValIntT);
        Stub<TypeGetStr, TypeEx, void> StubGetStr(&CConfigXmlParser::GetValueString, &GetValStrT);
        
        alarm_param_t stAlarm;
        stAlarm.iAlarmID = 1;
        stAlarm.strAlarmParam = "this is param";
        iRet = CTrapSender::SendAlarm(stAlarm);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = CTrapSender::ResumeAlarm(stAlarm);
        EXPECT_EQ(iRet, MP_SUCCESS);

        Stub<TypeQueryT, StubTypeQu, void> StubQutaZer(&CDB::QueryTable, &StubQueryTableZero);

        iRet = CTrapSender::SendAlarm(stAlarm);
        EXPECT_EQ(iRet, MP_SUCCESS);

        /********End Trap.cpp test********/

        iRet = oCAppFree.Insert(stStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    } 
}

TEST_F(CAlarmDBTest, DbParam_int64_uint32_uint64)
{
    mp_int64 itmp64;
    DbParam tmp1(itmp64);
    mp_int64 utmp32;
    DbParam tmp2(utmp32);
    mp_int64 utmp64;
    DbParam tmp3(utmp64);
}

TEST_F(CMpAlarmTest, GetSnmpV3Paramtest)
{
    try
    {
        snmp_v3_param stLocalParam;
        typedef mp_int32 (*GetVaStrType)(mp_string, mp_string, mp_string&);
        typedef mp_int32 (CConfigXmlParser::*GetVaStr)(mp_string, mp_string, mp_string&);
        Stub<GetVaStr, GetVaStrType, void> StubVa(&CConfigXmlParser::GetValueString, &StubCConXmlPGetVaStr);
        
        CAlarmConfig::GetSnmpV3Param(stLocalParam);
        
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    }
}

TEST_F(CMpAlarmTest, PreCompileExecSqlTest)
{
    typedef mp_void (*StubFuncType)(void);
    typedef mp_void (CLogger::*LogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
    Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);
    typedef mp_string (CPath::*GetDbPathType)(mp_string);
    typedef mp_string (*StubTypeReturnString)(void);
    Stub<GetDbPathType, StubTypeReturnString, void> stubGetPath(&CPath::GetDbFilePath, StubGetDbFilePath);

    //create table test
    mp_string create_sql="CREATE TABLE IF NOT EXISTS [AlarmTable] ([AlarmSerialNo] INTEGER(4) NOT NULL ON CONFLICT ABORT COLLATE BINARY DEFAULT (0),[AlarmID] INTEGER(4) COLLATE BINARY DEFAULT (0), [AlarmType] INTEGER(4) DEFAULT (0),[AlarmLevel] INTEGER(4) DEFAULT (2), [AlarmBeginTime] VARCHAR(20),[AlarmClearTime] VARCHAR(20),[AlarmParam] VARCHAR(100), CONSTRAINT [] PRIMARY KEY ([AlarmSerialNo] COLLATE BINARY, [AlarmID]));";
    ASSERT_EQ(MP_SUCCESS, CreateTable(create_sql));

    //insert test
    alarm_Info_t  alarmInfo;
    alarmInfo.iAlarmSN = 7;
    alarmInfo.iAlarmID = 52625429;
    alarmInfo.iAlarmType = ALARM_TYPE_EQUPMENTFAULT;
    alarmInfo.strEndTime = "xxxx";
    mp_time time;
    CMpTime::Now(&time);
    mp_string strNowTime = CMpTime::GetTimeString(&time);
    alarmInfo.strStartTime = strNowTime;
    alarmInfo.iAlarmCategoryType = ALARM_CATEGORY_FAULT;
    alarmInfo.strAlarmParam = "test";
    ASSERT_EQ(MP_SUCCESS, CAlarmDB::InsertAlarmInfo(alarmInfo));

    //get all test
    vector<alarm_Info_t> infoList;
    CAlarmDB::GetAllAlarmInfo(infoList);
    EXPECT_EQ(1, infoList.size());
    alarm_Info_t queryAlarmInfo;
    queryAlarmInfo = infoList.front();
    EXPECT_EQ(queryAlarmInfo.iAlarmSN, 7);
    EXPECT_EQ(queryAlarmInfo.iAlarmID,  52625429);
    EXPECT_EQ(queryAlarmInfo.iAlarmType, ALARM_TYPE_EQUPMENTFAULT);
    EXPECT_EQ(queryAlarmInfo.strEndTime, "xxxx");
    EXPECT_EQ(queryAlarmInfo.strAlarmParam, "test");

    //update test
    alarmInfo.strEndTime = "change";
    ASSERT_EQ(MP_SUCCESS, CAlarmDB::UpdateAlarmInfo(alarmInfo));

    //getAlarmInfoBySNAndID
    alarmInfo.strEndTime = "";
    ASSERT_EQ(MP_SUCCESS, CAlarmDB::GetAlarmInfoBySNAndID(7, 52625429, alarmInfo));
    EXPECT_EQ(alarmInfo.strEndTime, "change");
    
    //delete test
    ASSERT_EQ(MP_SUCCESS, CAlarmDB::DeleteAlarmInfo(7, 52625429));
    infoList.clear();
    EXPECT_EQ(infoList.size(), 0);
    CAlarmDB::GetAllAlarmInfo(infoList);
    EXPECT_EQ(infoList.size(), 0);
    
    remove("zwgTest.db");
}

