/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "alarm/Db.h"
#include "common/Log.h"
#include "common/Path.h"


CDB CDB::m_Instance;
/*------------------------------------------------------------
Function Name:DBReader
Description  :DBReader构造函数
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
DBReader::DBReader()
{
    m_lstResult.clear();
}

/*------------------------------------------------------------
Function Name:~DBReader
Description  :DBReader析构函数
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
DBReader::~DBReader()
{
    m_lstResult.clear();
}

/*------------------------------------------------------------
Function Name:Clear
Description  :清除list成员变量内容
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void DBReader::Clear()
{
    m_lstResult.clear();
}

/*------------------------------------------------------------
Function Name:Empty
Description  :判断list成员变量是否为空
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool DBReader::Empty()
{
    return m_lstResult.empty();
}

/*------------------------------------------------------------
Function Name:operator<<
Description  :重载<<操作符
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_string DBReader::operator<<(mp_string& strResult)
{
    m_lstResult.push_back(strResult);
    return strResult;
}

/*------------------------------------------------------------
Function Name:operator>>
Description  :重载>>操作符
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_string DBReader::operator>>(mp_string& strResult)
{
	//codedex误报CHECK_CONTAINER_EMPTY,容器ite在之前的代码能保证不为空，此处可以不判断
    list<mp_string>::iterator ite = m_lstResult.begin();
    if (m_lstResult.end() != ite)
    {
        strResult = *ite;
        m_lstResult.erase(ite);
    }
    else
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "%s", "There has no param.");
    }
    return strResult;
}

/*------------------------------------------------------------
Function Name:Connect
Description  :连接sqlite数据库
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CDB::Connect()
{
    mp_int32 iRet;

    //连接之前首先释放以前的连接
    iRet = Disconnect();
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "close db connection failed. errno is %d:", iRet);
        return MP_FAILED;
    }

    //打开sqlite连接
    mp_string strDbFile = CPath::GetInstance().GetDbFilePath("AgentDB.db");
    iRet = sqlite3_open(strDbFile.c_str(), &m_pDB);
    if (SQLITE_OK != iRet)
    {
        //记录日志
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db open failed.errno is: %d", iRet);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:Disconnect
Description  :断开连接sqlite数据库
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CDB::Disconnect()
{
    if (NULL == m_pDB)
    {
        return MP_SUCCESS;
    }

    mp_int32 iRet = sqlite3_close(m_pDB);
    if (SQLITE_OK != iRet)
    {
        //记录日志
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "close db connection failed.errno is : %d", iRet);
        return MP_FAILED;
    }
    m_pDB = NULL;
    return MP_SUCCESS;
}

//采用预编译方式进行sqlite访问，
//首先需要使用sqlite3_prepare_v2对sql语句进行预编译
//预编译后会返回一个sqlite3_stmt指针
//然后将要操作的内容插入到预编译后的sql语句中打问号的字段(sqlite_bind_*函数)
//最后使用sqlite_step执行
sqlite3_stmt* CDB::SqlPrepare(mp_string sql)
{
	//CodeDex误报，SQL Injection
	//SQL语句是内部写死的，非用户输入
    sqlite3_stmt *stmt = NULL;
    mp_int32 iRet = sqlite3_prepare_v2(m_pDB, sql.c_str(), sql.size() , &stmt, NULL);
    if(SQLITE_OK != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "sqlite3_prepare_v2 DB failed, iRet = %d", iRet);
        if(MP_SUCCESS!= Disconnect())
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disconnect DB failed, iRet = %d", iRet);
        return NULL;
    }
    return stmt;
}


mp_int32 CDB::SqlBind(sqlite3_stmt* stmt,DbParamStream &dps)
{
    if(!m_stringList.empty())
    {
        m_stringList.clear();
    }
    mp_int32 iRet = SQLITE_OK;
    for(mp_int32 i = 1; !dps.Empty(); i++) //lint !e441
    {
        DbParam dp;
        dps >> dp;
        
        switch(dp.m_type)
        {
        case DB_PARAM_TYPE_INT32:
        case DB_PARAM_TYPE_UINT32:
            iRet = sqlite3_bind_int(stmt, i, atoint32(dp.m_value.c_str()));
            break;
        case DB_PARAM_TYPE_INT64:
        case DB_PARAM_TYPE_UINT64: 
            iRet = sqlite3_bind_int64(stmt, i, atoint64(dp.m_value.c_str()));
            break;
        case DB_PARAM_TYPE_STRING: 
            m_stringList.push_front(dp.m_value);
            iRet = sqlite3_bind_text(stmt, i, m_stringList.front().c_str(), m_stringList.front().size(), NULL);
            break;
        default:
            iRet = SQLITE_ERROR;
        }
        
        if(SQLITE_OK != iRet)
        {
            FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_bind_* DB failed")
        }
    }
    
    return MP_SUCCESS;
}

mp_int32 CDB::SqlExecute(sqlite3_stmt* stmt)
{
    mp_int32  iRet = sqlite3_step(stmt);
    if(SQLITE_DONE != iRet)
    {
        FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_step failed")
    }
    
    if(!m_stringList.empty())
    {
        m_stringList.clear();
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:ExecSql
Description  :预编译方式执行sql语句
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CDB::ExecSql(mp_string strSql, DbParamStream &dps)
{
    CThreadAutoLock lock(&m_InstanceLock);
    LOGGUARD("");
    mp_int32 iRet = Connect();
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Connect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }
    
    sqlite3_stmt* stmt = SqlPrepare(strSql);
    if(NULL == stmt)
        return MP_FAILED;

    iRet = SqlBind(stmt, dps);
    if(MP_FAILED == iRet)
        return MP_FAILED;

    iRet = SqlExecute(stmt);
    if(MP_FAILED == iRet)
        return MP_FAILED;

    if(SQLITE_OK != sqlite3_finalize(stmt))
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "sqlite3_finalize failed"); 
    
    iRet = Disconnect();
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disconnect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }
    dps.Clear();
    return MP_SUCCESS;
}


/*------------------------------------------------------------
Function Name:SqlQuery
Description  :预编译方式QueryTable子函数
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CDB::SqlQuery(sqlite3_stmt* stmt, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    mp_int32 iRet = SQLITE_ERROR;
    mp_int32 colCount = sqlite3_column_count(stmt); //列数
    if(0 >= colCount)
    {
        FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_column_count return <= 0")     
    }
    mp_int32 rowCount = 0; //行数
    while(SQLITE_ROW == (iRet = sqlite3_step(stmt)))
    {
        for(mp_int32 i = 0; i < colCount; i++)
        {
            const char* temp = (const char*)sqlite3_column_text(stmt, i);
            if(NULL == temp)
            {
                FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_column_text return NULL")
            }
            mp_string text = temp;
            readBuff << text;
        }
        rowCount++;
    }

    if(SQLITE_DONE != iRet)
    {
        FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED("sqlite3_step not return SQLITE_DONE")         
    }
    
    iRowCount = rowCount;
    iColCount = colCount;
    if(!m_stringList.empty())
    {
        m_stringList.clear();
    }
    return MP_SUCCESS;
}


/*------------------------------------------------------------
Function Name:QueryTable
Description  :预编译方式执行sqlite获取某条表数据
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CDB::QueryTable(mp_string strSql, DbParamStream& dps, 
    DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    CThreadAutoLock lock(&m_InstanceLock);
    LOGGUARD("");
    mp_int32 iRet = Connect();
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Connect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }

    sqlite3_stmt* stmt = SqlPrepare(strSql);
    if(NULL == stmt)
        return MP_FAILED;

    iRet = SqlBind(stmt, dps);
    if(MP_FAILED == iRet)
        return MP_FAILED;
    
    iRet = SqlQuery(stmt, readBuff, iRowCount, iColCount);
    if(MP_FAILED == iRet)
        return MP_FAILED;
    
    if(SQLITE_OK != sqlite3_finalize(stmt))
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "sqlite3_finalize failed"); 
    
    iRet = Disconnect();
    if(MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disconnect DB failed, iRet = %d", iRet);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:operator>>
Description  :>>操作符
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
DbParam DbParamStream::operator>>(DbParam& param)
{
    if(Empty())
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "%s", "There has no param.");
        return param;
    }
    param = m_ParamList.front();
    m_ParamList.pop_front();
    return param;
}

DbParam::DbParam(mp_int32 value)
{
    m_type = DB_PARAM_TYPE_INT32;
    IntToString(value, m_value);
}
DbParam::DbParam(mp_int64 value)
{
    m_type = DB_PARAM_TYPE_INT64;
    IntToString(value, m_value);
}
DbParam::DbParam(mp_uint32 value)
{
    m_type = DB_PARAM_TYPE_UINT32;
    IntToString(value, m_value);
}
DbParam::DbParam(mp_uint64 value)
{
    m_type = DB_PARAM_TYPE_UINT64;
    IntToString(value, m_value);
}


