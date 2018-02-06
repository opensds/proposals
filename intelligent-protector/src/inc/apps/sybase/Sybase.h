/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_SYBASE_H__
#define __AGENT_SYBASE_H__

#include <vector>
#include <list>
#include <map>
#include "common/Types.h"

using namespace std;

#define SCRIPTPARAM_INSTNAME              "INSTNAME="
#define SCRIPTPARAM_DBNAME                "DBNAME="
#define SCRIPTPARAM_DBUSERNAME            "DBUSERNAME="
#define SCRIPTPARAM_DBPASSWORD            "DBPASSWORD="
#define SCRIPTPARAM_OPERTYPE              "OPERTYPE="
#define REST_PARAM_SYBASE_STATE           "state"

#define SCRIPTPARAM_FREEZEDB              "0"
#define SCRIPTPARAM_THAWDB                "1"
#define SCRIPTPARAM_FREEZESTATUS          "2"
#define SCRIPTPARAM_STARTDB               "0"
#define SCRIPTPARAM_STOPDB                "1"


typedef struct tag_sybase_instance_info
{
    mp_string  strinstName;               //实例
    mp_string  strdbName;                 //数据库名称，目前不使用
    mp_string strversion;                 //数据库版本
    mp_int32 istate;                      //状态,0-在线,1-离线
}sybase_inst_info_t;

typedef struct tag_sybase_dataBase_info
{
    mp_string strinstName; 
    mp_string strdbName;
    mp_string strDBUsername;
    mp_string strDBPassword;
}sybase_db_info_t;


class CSybase
{
public:
    CSybase();
    ~CSybase();

    mp_int32 StartDB(sybase_db_info_t &stdbInfo);
    
    mp_int32 StopDB(sybase_db_info_t &stdbInfo);

    mp_int32 FreezeDB(sybase_db_info_t &vecdbInfo);
    
    mp_int32 ThawDB(sybase_db_info_t &vecdbInfo);
    
    mp_int32 GetFreezeStatus(sybase_db_info_t &stdbInfo, mp_int32 &iFreezeState);
  
    mp_int32 Test(sybase_db_info_t &stdbInfo);
    
private:
    mp_void BuildScriptParam(sybase_db_info_t &stdbInfo, mp_string strOperType, mp_string &strParam);
    mp_void BuildScriptParam(sybase_db_info_t &stdbInfo, mp_string &strParam);
    
};

#endif //__AGENT_SYBASE_H__

