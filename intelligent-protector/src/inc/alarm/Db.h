/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _DB_H_
#define _DB_H_

#include <list>
#include "common/Types.h"
#include "common/Thread.h"
#include "sqlite/sqlite3.h"
#include <stdlib.h>
#include <sstream>

//sqlite数据库表名
#define TrapServerTable "TrapInfoTable"   //记录注册trap server信息表
#define AlarmTable "AlarmTable"           //记录上报告警信息表
#define AlarmTypeTable "AlarmTypeTable"   //记录流水号信息表
#define AppStatusTable "AppStatusTable"    //记录应用冻结状态信息表
#define FreezeObjTable "FreezeObjTable"    //记录冻结对象

//sqlite数据库各表单字段名称
//AlarmTable各字段名
#define titleAlarmID "AlarmID"
#define titleAlarmLevel "AlarmLevel"
#define titleAlarmParam "AlarmParam"
#define titleAlarmType "AlarmType"
#define titleEndTime "AlarmClearTime"
#define titleStartTime "AlarmBeginTime"
#define titleAlarmSerialNo "AlarmSerialNo"

//AlarmTypeTable各字段名称
#define titleAlarmSN   "AlarmSN"

//FreezeObjTable各字段名称
#define InstanceName "InstanceName"  //数据库实例名称
#define DBName "DBName"              //数据库名称
#define BeginStatus "BeginStatus"    //数据库开始状态
#define LoopTime "LoopTime"          //监控轮询时间
#define User "User"                  //数据库访问
#define MP "MP"                      //数据库访问
#define JsonData "JsonData"          //请求的json消息
#define AppType "AppType"            //应用类型
#define BeginTime "BeginTime"        //监控开始时间


#define atoint32(x)  mp_int32(atoi(x)) 
#define atoint64(x)  mp_int64(atol(x))
//#define atouint32(x)  mp_uint32(atoi(x))
//#define atouint64(x)  mp_uint64(atol(x))
#define IntToString(i, s) {std::stringstream ss;ss << i;ss >> s;}

 //使用此宏后不需要分号
#define FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED(x) { \
  COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, x); \
  if(SQLITE_OK != sqlite3_finalize(stmt)) \
     COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "sqlite3_finalize failed"); \
  if(MP_SUCCESS!= Disconnect()) \
     COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disconnect DB failed, iRet = %d", iRet); \
  return MP_FAILED; \
}


typedef enum
{
    DB_PARAM_TYPE_INT32,
    DB_PARAM_TYPE_INT64,
    DB_PARAM_TYPE_UINT32,
    DB_PARAM_TYPE_UINT64,
    DB_PARAM_TYPE_STRING
}DbParamType;

struct DbParam
{
    DbParamType m_type;
    mp_string m_value;
    DbParam()
    {
        m_type = DB_PARAM_TYPE_STRING;
        m_value = "";
    }
    DbParam(mp_string value)
    {
        m_type = DB_PARAM_TYPE_STRING;
        m_value = value;
    }
    DbParam(mp_int32 value);
    DbParam(mp_int64 value);
    DbParam(mp_uint32 value);
    DbParam(mp_uint64 value);
};
//使用预编译模式查询时的入参
class DbParamStream
{
public:
    DbParamStream(){}
    ~DbParamStream(){}
    mp_void Clear()
    {
        m_ParamList.clear();
    }
    mp_bool Empty()
    {
        return m_ParamList.empty();
    }
    DbParam operator>>(DbParam& param);
    DbParam operator<<(DbParam& param)
    {
        m_ParamList.push_back(param);
        return param;
    }
private:
    list<DbParam> m_ParamList;
};

class DBReader
{
public:
    DBReader();
    ~DBReader();
    mp_string operator>>(mp_string& strResult);
    mp_string operator<<(mp_string& strResult);
    mp_void Clear();
    mp_bool Empty();
private:
    list<mp_string> m_lstResult;
};


class CDB
{
public:
  
    //sql语句预编译方式
    mp_int32 ExecSql(mp_string strSql, DbParamStream &dpl);
    mp_int32 QueryTable(mp_string strSql, DbParamStream &dpl, 
                                         DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount);

    static CDB& GetInstance(void)
    {
        return m_Instance;
    }

    virtual ~CDB()
    {
        CMpThread::DestroyLock(&m_InstanceLock);
    }
private:
    mp_int32 Connect();
    mp_int32 Disconnect();

    //ExecSql函数拆分，降低函数复杂度
    sqlite3_stmt* SqlPrepare(mp_string sql);
    mp_int32 SqlBind(sqlite3_stmt* stmt, DbParamStream &dps);
    mp_int32 SqlExecute(sqlite3_stmt* stmt);
    mp_int32 SqlQuery(sqlite3_stmt* stmt, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount);
    
    sqlite3 * m_pDB; //m_pDB的内存管理由sqlit自己保证，sqlite3_open时申请，sqlite3_close时释放
    list<mp_string> m_stringList;//用于存放sqlite_bind_text的字符串
    CDB(CDB & cdb){}
    CDB & operator = (const CDB& cdb);
    CDB() : m_pDB(NULL)
    {
        CMpThread::InitLock(&m_InstanceLock);
    }

private:
    static CDB m_Instance;
    thread_lock_t m_InstanceLock;
    
};

#endif

