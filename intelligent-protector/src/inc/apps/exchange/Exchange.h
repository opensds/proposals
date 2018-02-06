/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_EXCHANGE_H__
#define __AGENT_EXCHANGE_H__
#ifdef WIN32

#include <vector>
#include <list>
#include "common/Types.h"
#include "vss/requester/Requester.h"

#define EXCHANGE_STR_LEN              10    //版本和操作类型的字符串长度

#define OPER_START_SERVICE            0
#define OPER_STOP_SERVICE             1

#define OPER_TYPE_RECOVERY            "0"
#define OPER_TYPE_CLEAR               "1"
#define OPER_TYPE_QUERY               "2"
#define OPER_TYPE_DISMOUNT            "3"

#define EXCHANGE_OPER_SCRIPT_NAME     "exchange.bat"

#define PARAM_OPER_TYPE               "OperType="
#define PARAM_RECOVERY_TYPE           "RecoveryType="
#define PARAM_VERSION                 "Version="
#define PARAM_NEW_STORGRPNAME         "NewStorageGroupName="
#define PARAM_NEW_MAILBOXDBNAME       "NewMailBoxDBName="
#define PARAM_OLD_STORGRPNAME         "OldStorageGroupName="
#define PARAM_OLD_MAILBOXDBNAME       "OldMailBoxDBName="
#define PARAM_HOSTNAME                "HostName="

#define PARAM_EDBPATH                 "EdbFilePath="
#define PARAM_LOGPATH                 "LogFilePath="
#define PARAM_SYSPATH                 "SysFilePath="

#define EXCHANGE_2007_MAJOR_VERSION   8
#define EXCHANGE_2010_MAJOR_VERSION   14
#define EXCHANGE_2013_MAJOR_VERSION   15

//查询AppInfo，增加DAG时，需做相应修改
#define EXCHANGE_INFO_COUNT           7
#define POS_STORAGE_GROUP_NAME        0
#define POS_MAILBOX_NAME              1
#define POS_MOUNT_STATUS              2
#define POS_EDB_FILE_PATH             3
#define POS_LOG_DIR_PATH              4
#define POS_SYS_DIR_PATH              5
#define POS_PUBLIC_FOLDER_FLAG        6

#define EXCHANGE_MOUTNED_STATUS       "True"
#define EXCHANGE_UNMOUNTED_STATUS     "False"

enum E_EXCHANGE_VER
{
    EXCHANGE_VER_2007 = 1,
    EXCHANGE_VER_2010,
    EXCHANGE_VER_2013
};

enum E_MAILBOX_DB_STATUS
{
    EXC_STATUS_DISMOUNTED = 0,    //卸除状态
    EXC_STATUS_MOUNTED,           //挂载状态
    EXC_STATUS_UNKNOWN            //未知状态(在某些系统服务未开启时无法查询出具体状态，
                                  //例如:Microsoft Exchange Information Store服务未开启)
};

typedef struct tag_exchange_db_info
{
    mp_int32 iVersion;
    //mp_int32 iServerType;
    mp_int32 iMountState;
    mp_int32 iIsCommon;
    mp_string strStorageGroup;
    mp_string strDbName;
    mp_string strEdbPath;
    mp_string strLogPath;
    mp_string strSystemPath;
    //mp_string strClusterType;
    //mp_string strCopyState;

}exchange_db_info_t;

typedef struct tag_exchange_lun_info
{
    mp_string strLunId;
    mp_string strArraySn;
    mp_string strDevName;
    mp_string strWwn;
    mp_string strVolName;
    mp_string strLba;
}exchange_lun_info_t;

typedef struct tag_exchange_strorage_info
{
    mp_int32 iDiskNumber;
    mp_string strLunId;
    mp_string strArraySn;
    mp_string strWwn;
}exchange_strorage_info_t;

typedef struct tag_ex_querlun_input_info
{
    mp_string  strStorageGroup;
    mp_string  strDbNames;
    mp_string  strVersion;
}ex_querlun_input_info_t;

typedef struct tag_exchange_param
{
    mp_int32 iVersion;
    mp_int32 iStarType;
    mp_string strDbName;          //主端数据库名称
    mp_string strStrGrpName;      //主端存储组名称，exchange2007
    mp_string strServName;        //主端服务器名称
    mp_string strSlaveDbName;     //备端数据库名称
    mp_string strSlaveStrGrpName; //备端存储组名称
    mp_string strEdbPath;
    mp_string strLogPath;
    mp_string strSysPath;
}exchange_param_t;

typedef struct tag_exchange_freeze_info
{
    mp_string strDBName;    //2007 -- 存储组名称；2010、2013 -- 邮箱数据库名称；
    vector<mp_string> vecDriveLetters;
}exchange_freeze_info_t;

typedef struct tag_exchange_unfreeze_info
{
    mp_string strDBName;    //2007 -- 存储组名称；2010、2013 -- 邮箱数据库名称；
    vector<mp_string> vecDriveLetters;
}exchange_unfreeze_info_t;

class CExchange
{
private:
    VSSRequester* m_pVssRequester;
public:
    CExchange();
    ~CExchange();

    mp_int32 GetInfo(vector<exchange_db_info_t> &vecExchangeDbInfo);
    mp_int32 GetLunInfo(ex_querlun_input_info_t exchangeInputInfo, vector<exchange_lun_info_t>& vecLunInfos);
    mp_int32 StartExchangeService();
    mp_int32 Stop(exchange_param_t& exstopparam, mp_string strStopType);
    mp_int32 Start(exchange_param_t& exstartparam);
    mp_int32 OperHostContorllerService(mp_int32 OperTyp);
    mp_int32 Freeze(vector<exchange_freeze_info_t> vecFreezeInfos);  
    mp_int32 UnFreeze(vector<exchange_unfreeze_info_t> vecUnFreezeInfos);
    mp_int32 GetFreezeStat();

private:
    mp_int32 AnalyseResultInfo(vector<mp_string>& vecResult, vector<exchange_db_info_t>& vecExchangeDbInfo);
    mp_int32 ConvertVersion(mp_string& strVersion, mp_int32& iVersion);
    mp_int32 AnalyseExchangeAppInfo(mp_string& strAppInfo, exchange_db_info_t& stExhangeInfo);
    mp_int32 AnalyseLunInfos(ex_querlun_input_info_t exchangeInputInfo, vector<exchange_db_info_t> vecExchangeDbInfo, vector<exchange_lun_info_t>& vecLunInfos);
    mp_int32 Get2007VolumePathList(mp_int32 iSpecifiedVersion, mp_string& strStorageGroupName,
        vector<exchange_db_info_t>& vecExchangeDbInfo, list<mp_string>& lstVolumePaths);
    mp_int32 Get201xVolumePathList(mp_int32 iSpecifiedVersion,  mp_string& strMailboxDbName,
        vector<exchange_db_info_t>& vecExchangeDbInfo, list<mp_string>& lstVolumePaths);
    mp_int32 GetExchangeLunInfos(list<mp_string>& lstVolumePath, vector<exchange_lun_info_t>& lstExchangeLunInfos);
    mp_int32 GetExhcangeLunInfo(mp_string& strVolumePath, exchange_lun_info_t& stExchangeLunInfo);
    mp_void ConvertMountStatus(mp_string& strStatus, mp_int32& iStatus);
    mp_void AddVolumePathToList(list<mp_string>& lstVolumePaths, mp_string& strVolumePath);
    mp_int32 GetDiskInfoList(vector<exchange_strorage_info_t>& exstrorageinfos);

};
#endif

#endif //__AGENT_EXCHANGE_H__

