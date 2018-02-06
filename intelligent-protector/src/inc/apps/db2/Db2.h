/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_DB2_H__
#define __AGENT_DB2_H__
#include <vector>
#include <list>
#include "common/Types.h"

#define SCRIPTPARAM_INSTNAME              "INSTNAME="
#define SCRIPTPARAM_DBNAME                "DBNAME="
#define SCRIPTPARAM_DBUSERNAME            "DBUSERNAME="
#define SCRIPTPARAM_DBPASSWORD            "DBPASSWORD="
#define SCRIPTPARAM_OPERTYPE              "OPERTYPE="

#define SCRIPTPARAM_STARTDB               "1"
#define SCRIPTPARAM_STOPDB                "0"

#define SCRIPTPARAM_TESTDB                "2"
#define SCRIPTPARAM_FREEZEDB              "0"
#define SCRIPTPARAM_THAWDB                "1"
#define SCRIPTPARAM_FREEZESTATE           "3"

typedef struct tag_db2_instance_info
{
    mp_string  strinstName;               //实例
    mp_string  strdbName;                 //数据库名称
    mp_string strversion;                 //数据库版本
    mp_int32 istate;                      //状态,0-在线,1-离线
}db2_inst_info_t;

typedef struct tag_db2_dataBase_info
{
    mp_string  strinstName; 
    mp_string  strdbName;
    mp_string  strdbUsername;
    mp_string  strdbPassword;
}db2_db_info_t;

typedef struct tag_db2_storage_info
{
    mp_string strvolName;
    mp_string strvgName;
    mp_string strdeviceName;
    mp_string strdiskName;
    mp_int32 ivolType;
    mp_int32 istorageType;
} db2_storage_info_t;

typedef struct tag_db2_lun_info
{
    mp_string strlunId;
    mp_string struuid;
    mp_string strarraySn;
    mp_string strwwn;
    mp_string strdeviceName;
    mp_string strdbName;
    mp_string strinstName;
    mp_int32 ivolType;
    mp_string strvgName;
    mp_string strvolName;
    mp_string strpvName;
    mp_int32 istorageType;  
}db2_lun_info_t;

class CDB2
{
public:
    CDB2();
    ~CDB2();

    mp_int32 GetInfo(vector<db2_inst_info_t> &vecdbInstInfo);
    
    mp_int32 GetLunInfo(db2_db_info_t stdbinfo, vector<db2_lun_info_t>& vecLunInfos);
    
    mp_int32 Start(db2_db_info_t stdbInfo);
    
    mp_int32 Stop(db2_db_info_t stdbInfo);
    
    mp_int32 Test(db2_db_info_t stdbInfo);
    
    mp_int32 Freeze(db2_db_info_t stdbInfo);
    
    mp_int32 UnFreeze(db2_db_info_t stdbInfo);
    
    mp_int32 QueryFreezeState(db2_db_info_t stdbInfo, mp_int32& iFreezeState);
private:
    mp_int32 GetDiskLunInfo(vector<db2_storage_info_t> vecdbStorageInfo, vector<db2_lun_info_t> &vecdbLunInfo);

    void BuildScriptParam(db2_db_info_t stdbinfo, mp_string strOperType, mp_string& strParam);

    void BuildLunInfoScriptParam(db2_db_info_t stdbinfo, mp_string & strParam);
    
    mp_void AnalyseInstInfoScriptRst(vector<mp_string> vecResult, vector<db2_inst_info_t> &vecdbInstInfo); 
    
    mp_void AnalyseLunInfoScriptRST(vector < mp_string > vecResult, vector < db2_storage_info_t > & vecdbStorInfo);
 
};

#endif //__AGENT_DB2_H__

