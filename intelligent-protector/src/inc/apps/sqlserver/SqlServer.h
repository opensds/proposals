/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_SQL_SERVER_H__
#define __AGENT_SQL_SERVER_H__
#include <vector>
#include <list>
#include "common/Types.h"
#include "vss/requester/Requester.h"

#ifdef WIN32
#define SQL_SERVER_INFO_TOTAL_COUNT             6
#define SQL_SERVER_ONLINE                       0
#define SQL_SERVER_OFFLINE                      1
#define SQL_SERVER_RECOVERY_MODEL_UNKNOW        0
#define SQL_SERVER_RECOVERY_MODEL_FULL          1
#define SQL_SERVER_RECOVERY_MODEL_BULK_LOG      2
#define SQL_SERVER_RECOVERY_MODEL_SIMPLE        3

// 定义 SQLServer 插件中数据库操作码;
#define SQL_SERVER_OPTCODE_STOP                 "0"
#define SQL_SERVER_OPTCODE_START                "1"
#define SQL_SERVER_OPTCODE_TESTCONN             "2"
#define SQL_SERVER_OPTCODE_TRUNCATE             "3"

#define SQL_SERVER_QUERY_REG_ERR_NOT_EXIST      2
#define SQL_SERVER_REG_KEY                      "SOFTWARE\\Microsoft\\Microsoft SQL Server\\Instance Names\\SQL"

// 定义SQLServer数据库中的系统数据库
#define MASTER									"master"
#define TEMPDB									"tempdb"
#define MODEL									"model"
#define MSDB									"msdb"	

// sqlserver rest接口参数集合;
typedef struct tag_sqlserver_info
{
    mp_string strInstName;   // 实例名称;
    mp_string strDBName;     // 数据库名;
    mp_string strUser;       // 用户名;
    mp_string strPasswd;     // 密码;
    mp_string strVersion;    // 版本;
    mp_string strIsCluster;  // 集群标示;
    mp_string strCheckType;  // 操作码,适用启停数据库;
    mp_string strState;      // 数据库状态;
    mp_int32 iRecoveryModel; //恢复模式 0 -- unkonw；1 -- full, 2 -- bulk logged, 3 -- simple；
}sqlserver_info_t;


// 查询SQLServer阵列的反包信息, 数据库磁盘详细信息;
typedef struct tag_sqlserver_lun_info
{
    mp_string strLunID;         // lun id;
    mp_string strArraySN;       // Lun 所在阵列SN;
    mp_string strWWN;           // Lun 所在的WWN;
    mp_string strDeviceName;    // 存储附加信息;
    mp_string strVOLName;       // 卷名; \\?\Volume{997dc5aa-351b-11e3-89f5-000c29d2c650}
    mp_string strLBA;           // 分区LBA首地址; 
    mp_int32  iDeviceType;      // 存储类型;
                                //    0 – 文件系统   备注:SQLServer使用
                                //    1 – 裸设备
                                //    2 – ASMLib磁盘；
                                //    3 – ASM裸设备；
                                //    4 – ASM软链接；
                                //    5 – ASMOnUdev（仅Linux使用，在udev中使用）
                                //    6 – windows ASM磁盘标识符
}sqlserver_lun_info_t;

// SQLServer 阵列基本信息;
typedef struct tag_storage_basic
{
    mp_string strLunID;     // LUN ID;
    mp_string strArraySN;   // LUN所在阵列的序列号;
    mp_string strWWN;       // LUN的WWN;
    mp_int32  iDiskNum;     // LUN在主机上序号;
}storage_basic_t;

typedef struct tag_sqlserver_freeze_info
{
    mp_string strDBName;
    vector<mp_string> vecDriveLetters;
}sqlserver_freeze_info_t;

typedef struct tag_sqlserver_unfreeze_info
{
    mp_string strDBName;
    vector<mp_string> vecDriveLetters;
}sqlserver_unfreeze_info_t;

class CSqlServer
{
private:
    VSSRequester* m_pVssRequester;
public:
    CSqlServer();
    ~CSqlServer();

    mp_int32 IsInstalled(mp_bool& bIsInstalled);
    mp_int32 GetInfo(vector<sqlserver_info_t> &vecdbInstInfo);
    mp_int32 GetLunInfo(sqlserver_info_t& stdbinfo, vector<sqlserver_lun_info_t>& vecLunInfos);
    mp_int32 Start(sqlserver_info_t& stdbInfo);
    mp_int32 Stop(sqlserver_info_t& stdbInfo);
    mp_int32 Test(sqlserver_info_t& stdbInfo);
    mp_int32 TruncateTransLog(sqlserver_info_t& dbInfo);
    mp_int32 Freeze(vector<sqlserver_freeze_info_t> vecFreezeInfos);  
    mp_int32 UnFreeze(vector<sqlserver_unfreeze_info_t> vecUnFreezeInfos);
    mp_int32 GetFreezeStat();

private:
    mp_int32 AnalyseDBQueryResult(vector<sqlserver_info_t>& vecDbInfo, const vector<mp_string>& vecResult);
    mp_int32 QueryDBTableSpaceLUNInfo(sqlserver_info_t& stdbinfo, vector<sqlserver_lun_info_t>& vecLunInfos);
    mp_int32 GetDBFilePath(vector<mp_string> &lstPath, const mp_string &strParam);
    mp_int32 GetDBLUNInfoByPath(vector<sqlserver_lun_info_t> &vecLunInfos, const mp_string &path);
    mp_int32 GetDiskInfoList(vector<storage_basic_t> &rlstDiskInfoWin);
    mp_void  BuildScriptParams(mp_string &rstrParam, const sqlserver_info_t& stdbinfo);
    mp_void  ReplaceStr(const mp_string &oldStr, mp_string &newStr, const mp_string &old_value, const mp_string &new_value);

};

#endif
#endif //__AGENT_SQL_SERVER_H__

