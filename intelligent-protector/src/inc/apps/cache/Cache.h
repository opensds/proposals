/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_CACHE_H__
#define __AGENT_CACHE_H__

#include <vector>
#include <list>
#include "common/Types.h"

#define SCRIPTPARAM_INSTNAME              "INSTNAME="
#define SCRIPTPARAM_DBNAME                "DBNAME="
//#define SCRIPTPARAM_OPERTYPE              "OPERTYPE="

#define SCRIPTPARAM_TESTDB                "2"

typedef struct tag_cache_instance_info
{
    mp_string  strinstName;               //实例
//    mp_string  strdbName;                 //数据库名称，目前不使用
    mp_string strversion;                 //数据库版本
    mp_int32 istate;                      //状态,0-在线,1-离线
}cache_inst_info_t;

typedef struct tag_cache_dataBase_info
{
    mp_string  strinstName; 
//    mp_string  strdbName;
}cache_db_info_t;

typedef struct tag_cache_storage_info
{
    mp_string strvolName;
    mp_string strvgName;
    mp_string strdeviceName;
    mp_string strdiskName;
    mp_int32 ivolType;
    mp_int32 istorageType;
} cache_storage_info_t;

typedef struct tag_cache_lun_info
{
    mp_string strlunId;
    mp_string struuid;
    mp_string strarraySn;
    mp_string strwwn;
    mp_string strdeviceName;
//   mp_string strdbName;
//   mp_string strinstName;
    mp_int32 ivolType;
    mp_string strvgName;
    mp_string strvolName;
    mp_string strpvName;
    mp_int32 istorageType;  
}cache_lun_info_t;

class CCache
{
public:
    CCache();
    ~CCache();

    mp_int32 GetInfo(vector<cache_inst_info_t> &vecdbInstInfo);
    
    mp_int32 GetLunInfo(cache_db_info_t stdbinfo, vector<cache_lun_info_t>& vecLunInfos);
  
    mp_int32 Test(cache_db_info_t stdbInfo);
    
private:
    mp_int32 GetDiskLunInfo(vector<cache_storage_info_t> vecdbStorageInfo, vector<cache_lun_info_t> &vecdbLunInfo);

    void BuildScriptParam(cache_db_info_t stdbinfo, mp_string strOperType, mp_string& strParam);

    void BuildLunInfoScriptParam(cache_db_info_t stdbinfo, mp_string & strParam);
    
    mp_void AnalyseInstInfoScriptRst(vector<mp_string> vecResult, vector<cache_inst_info_t> &vecdbInstInfo); 
    
    mp_void AnalyseLunInfoScriptRST(vector < mp_string > vecResult, vector < cache_storage_info_t > & vecdbStorInfo);
 
};

#endif //__AGENT_CACHE_H__

