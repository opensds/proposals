/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_HANA_H__
#define __AGENT_HANA_H__

#include <vector>
#include <list>
#include <map>
#include "common/Types.h"

using namespace std;

#define SCRIPTPARAM_INSTNUM               "INSTNUM="
#define SCRIPTPARAM_DBNAME                "DBNAME="
#define SCRIPTPARAM_DBUSERNAME            "DBUSERNAME="
#define SCRIPTPARAM_DBPASSWORD            "DBPASSWORD="
#define SCRIPTPARAM_OPERTYPE              "OPERTYPE="
#define REST_PARAM_HANA_STATE             "state"

#define SCRIPTPARAM_FREEZEDB              "0"
#define SCRIPTPARAM_THAWDB                "1"
#define SCRIPTPARAM_FREEZESTATUS          "2"
#define SCRIPTPARAM_STARTDB               "0"
#define SCRIPTPARAM_STOPDB                "1"


typedef struct tag_hana_instance_info
{
    mp_string  strinstNum;                   //实例编号
    mp_string  strdbName;                    //数据库名称
    mp_string  strversion;                   //数据库版本
    mp_int32   istate;                       //状态,0-在线,1-离线
}hana_inst_info_t;

typedef struct tag_hana_dataBase_info
{
    mp_string strInstNum;
    mp_string strDbName;
    mp_string strDBUsername;
    mp_string strDBPassword;
}hana_db_info_t;


class CHana
{
public:
    CHana();
    ~CHana();

    mp_int32 StartDB(hana_db_info_t &stdbInfo);
    
    mp_int32 StopDB(hana_db_info_t &stdbInfo);

    mp_int32 FreezeDB(hana_db_info_t &vecdbInfo);
    
    mp_int32 ThawDB(hana_db_info_t &vecdbInfo);
    
    mp_int32 GetFreezeStatus(hana_db_info_t &stdbInfo, mp_int32 &iFreezeState);
  
    mp_int32 Test(hana_db_info_t &stdbInfo);
    
private:
    mp_void BuildScriptParam(hana_db_info_t &stdbInfo, mp_string strOperType, mp_string &strParam);
    mp_void BuildScriptParam(hana_db_info_t &stdbInfo, mp_string &strParam);
    
};

#endif //__AGENT_HANA_H__

