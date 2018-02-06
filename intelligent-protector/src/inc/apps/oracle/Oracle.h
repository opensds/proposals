/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_ORACLE_H__
#define __AGENT_ORACLE_H__

#include <vector>
#include <list>
#include "common/Types.h"
#include "host/Host.h"

// URL参数
#define RESPOND_ORACLE_PARAM_INSTNAME               "instName"
#define RESPOND_ORACLE_PARAM_DBNAME                 "dbName"
#define RESPOND_ORACLE_PARAM_VERSION                "version"
#define RESPOND_ORACLE_PARAM_STATE                  "state"
#define RESPOND_ORACLE_PARAM_ISASMDB                "isAsmInst"
#define RESPOND_ORACLE_PARAM_ORACLE_HOME            "oracleHome"

#define RESPOND_ORACLE_PARAM_CONID                	"conID"
#define RESPOND_ORACLE_PARAM_PDBNAME                "pdbName"
#define RESPOND_ORACLE_PARAM_PDBSTATUS              "status"


#define RESPOND_ORACLE_PARAM_ASMINSTNAME            "asmInstName"
#define RESPOND_ORACLE_PARAM_SEARCHARCHIVE          "isArchive"

#define RESPOND_ORACLE_PARAM_ARCHIVETHRESHOLD       "threshold"

#define RESPOND_ORACLE_PARAM_CDBTYPE       "type"

#define RESPOND_ORACLE_LUNID                        "lunId"
#define RESPOND_ORACLE_UUID                         "uuid"
#define RESPOND_ORACLE_ARRAYSN                      "arraySn"
#define RESPOND_ORACLE_WWN                          "wwn"
#define RESPOND_ORACLE_VOLTYPE                      "volType"
#define RESPOND_ORACLE_VGNAME                       "vgName"
#define RESPOND_ORACLE_DEVICENAME                   "deviceName"
#define RESPOND_ORACLE_PVNAME                       "pvName"
#define RESPOND_ORACLE_STORMAINTYPE                 "deviceType"
#define RESPOND_ORACLE_STORSUBTYPE                  "volType"
#define RESPOND_ORACLE_DEVICEPATH                   "devicePath"
#define RESPOND_ORACLE_UDEVRULES                    "udevRules"
#define RESPOND_ORACLE_LBA                          "LBA"
#define RESPOND_ORACLE_ASMDISKGROUPS                "asmDiskGroups"
#define RESPOND_ORACLE_ASMDISKGROUP                 "asmDiskGroup"
#define RESPOND_ORACLE_ISASM                        "isASM"
#define REST_PARAM_ORACLE_STATE                     "state"
#define REST_PARAM_ORACLE_IS_INCLUDE_ARCH           "isIncludeArchLog"

#define REST_PARAM_ORACLE_TIME                      "time"
#define REST_PARAM_ORACLE_LOGTYPE                   "logType"
#define REST_PARAM_ORACLE_ERRORCODE                 "errorCode"
#define REST_PARAM_ORACLE_ERRORMESSAGE              "errorMessage"
#define REST_PARAM_ORACLE_INSTANCEARRAY             "instances"
#define RESPOND_ORACLE_PARAM_INST_LIST              "instNames"
#define RESPOND_ORACLE_VSS_WRITER_STATUS            "vssWriterStatus"

//脚本定义参数名称
#define ORACLE_SCRIPTPARAM_INSTNAME                   "InstanceName="
#define ORACLE_SCRIPTPARAM_DBNAME                     "AppName="
#define ORACLE_SCRIPTPARAM_DBUSERNAME                 "UserName="
#define ORACLE_SCRIPTPARAM_DBPASSWORD                 "Password="
#define ORACLE_SCRIPTPARAM_TABLESPACENAME             "TableSpaceName="
#define ORACLE_SCRIPTPARAM_ASMINSTANCE                "ASMInstanceName="
#define ORACLE_SCRIPTPARAM_ASMUSERNAME                "ASMUserName="
#define ORACLE_SCRIPTPARAM_ASMPASSWOD                 "ASMPassword="
#define ORACLE_SCRIPTPARAM_ORACLE_HOME                "OracleHome="
#define ORACLE_SCRIPTPARAM_FRUSHTYPE                  "FrushType="
#define ORACLE_SCRIPTPARAM_ARCHIVETHRESHOLD           "ArchiveThreshold="
#define ORACLE_SCRIPTPARAM_ASMDISKGROUPS              "ASMDiskGroups="
#define ORACLE_SCRIPTPARAM_ISASM                      "IsASM="
#define ORACLE_SCRIPTPARAM_ACTION                     "Action=" // 停止和开始动作 0:start  1:stop
#define ORACLE_SCIPRTPARAM_IS_INCLUDE_ARCH            "IsIncludeArchLog="
#define ORACLE_SCIPRTPARAM_TRUNCATE_LOG_TIME          "TruncateLogTime="
#define ORACLE_SCIPRTPARAM_PDBNAME          		  "PDBName="


#define ORACLE_SCRIPTPARAM_FREEZEDB                   "0"
#define ORACLE_SCRIPTPARAM_THAWDB                     "1"
#define ORACLE_SCRIPTPARAM_FREEZESTATUS               "2"
#define ORACLE_SCRIPTPARAM_ARCHIVEDB                  "3"
#define ORACLE_SCRIPTPARAM_TRUNCATEARCHIVELOG         "4"
#define ORACLE_SCRIPTPARAM_GET_ARCHIVE_LOG_MODE       "5"
#define ORACLE_SCRIPTPARAM_CHECK_ORACLE       		  "6"


//分割符定义暂时放此，稍后移植到公共类中
#define DBADAPTIVE_PRAMA_MUST            "must"
#define DBADAPTIVE_PRAMA_OPTION          "option"
#define DBADAPTIVE_PRAMA_ARCHIVE         "archive"

//定义PDB状态
#define PDB_MOUNTED						0
#define PDB_READ_ONLY					1
#define PDB_READ_WRITE					2

#define INIT_PDB_STATUS_MOUNTED			"MOUNTED"
#define INIT_PDB_STATUS_READ_ONLY		"READ ONLY"
#define INIT_PDB_STATUS_READ_WRITE		"READ WRITE"

//windows下的脚本列表
#define WIN_ORACLE_INFO                  "oracleinfo.bat"
#define WIN_ORACLE_LUN_INFO              "oracleluninfo.bat"
#define WIN_ORACLE_TEST                  "oracletest.bat"
#define WIN_ORACLE_CONSISTENT            "oracleconsistent.bat"
#define WIN_ORACLE_ASMACTION             "oraasmaction.bat"
#define WIN_ORACLE_CHECK_ARCHIVE         "oraclecheckarchive.bat"
#define WIN_ORACLE_DB_ACTION             "oradbaction.bat"
#define WIN_ORACLE_CHECK_CDB             "oraclecheckcdb.bat"

#define STORAGE_TYPE_FS                  "0"
#define STORAGE_TYPE_RAW                 "1"
#define STORAGE_TYPE_ASMLIB              "2"
#define STORAGE_TYPE_ASMRAW              "3"
#define STORAGE_TYPE_ASMLINK             "4"
#define STORAGE_TYPE_ASMUDEV             "5"
#define STORAGE_TYPE_ASMWIN              "6"

#define VOLTYPE_NOVOL                    0
#define ORACLE_QUERY_ARCHIVE_LOGS        1

#define NON_ARCHIVE_LOG_MODE             0
#define ARCHIVE_LOG_MODE                 1

#define ORACLE_TYPE_NON_CDB             0
#define ORACLE_TYPE_CDB                 1

#define ORA_INSTANCE_DATABASE_CONFIGFILE "oracleinfo.cfg"
#define ORACLE_REG_KEY               	 "SOFTWARE\\ORACLE"

typedef struct tag_oracle_instance_info
{
    mp_string   strInstName;                //实例
    mp_string   strDBName;                  //数据库名称
    mp_string   strVersion;                 //数据库版本
    mp_int32    iState;                     //状态,0-在线,1-离线
    mp_int32    iIsASMDB;                   //是否是ASM数据库
    mp_int32    iArchiveLogMode;            // oracle的归档模式
#ifdef WIN32
    mp_int32    iVssWriterStatus;           //VssWriter状态,0-停止,1-启动,2-其他
#endif
    mp_string   strOracleHome;              //ORACLE_HOME环境变量的值
} oracle_inst_info_t;

typedef struct tag_oracle_error_info
{
    mp_int32    iErrorCode;                 //错误码
    mp_string   strInstName;                //实例
    //mp_string   strDBName;                  //数据库名称
} oracle_error_info_t;

typedef struct tag_oracle_dataBase_info
{
    mp_string strInstName;
    mp_string strDBName;
    mp_string strDBUsername;
    mp_string strDBPassword;
    mp_string strASMInstance;
    mp_string strASMUserName;
    mp_string strASMPassword;
    mp_int32 iGetArchiveLUN;       //查询oracle的lun信息是否查询归档日志所在lun， 1 -- 查询， 0 -- 不查询
    mp_int32 iIncludeArchLog;      //启动时是否包含归档日志，1 -- 包含，0 -- 不包含
    mp_string strOracleHome;
    mp_string strArchThreshold;
    mp_string strTableSpaceName;
    mp_string strASMDiskGroup;
    mp_string strIsASM;
} oracle_db_info_t;

typedef struct tag_oracle_storage_script_info
{
    mp_string strStorMainType;
    mp_string strStorSubType;
    mp_string strSystemDevice;
    mp_string strDeviceName;
    mp_string strDevicePath;
    mp_string strVgName;
    mp_string strASMDiskGroup;
    mp_string strUDEVRes;
    mp_string strUDEVName;
    mp_string strLBA;

} oracle_storage_script_info;

typedef struct tag_oracle_lun_info
{
    mp_string strInstName;
    mp_string strDBName;
    mp_string strLUNId;
    mp_string strUUID;
    mp_string strArraySn;
    mp_string strWWN;
    mp_string strVgName;
    mp_string strDeviceName;
    mp_string strPvName;
    mp_int32  iStorMainType;
    mp_int32  iStorSubType;
    mp_string strDevicePath;
    mp_string strUDEVRules;
    mp_string strLBA;
    mp_string strASMDiskGroup;
} oracle_lun_info_t;

typedef struct tag_oracle_rsp_pdb_info
{
	mp_int32 iConID;
	mp_string strPdbName;
	mp_int32 iStatus;
} oracle_pdb_rsp_info_t;

typedef struct tag_oracle_req_pdb_info
{
	mp_string strOracleHome;
	mp_string strInstName;
	mp_string strPdbName;
	mp_string strDBUsername;
    mp_string strDBPassword;
} oracle_pdb_req_info_t;


class COracle
{
public:
    COracle();
    ~COracle();

    mp_int32 IsInstalled(mp_bool &bIsInstalled);
    mp_int32 GetDBInfo(list<oracle_inst_info_t> &lstOracleInstInfo);
    mp_int32 GetArchiveLogMode(oracle_db_info_t& stDBInfo, mp_int32& iArvhiceLogMode);    
    mp_int32 GetInstances(oracle_db_info_t& stDBInfo, list<oracle_inst_info_t>& lstOracleInsts);
    mp_int32 GetDBLUNInfo(oracle_db_info_t &stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos);
    mp_int32 Test(oracle_db_info_t &stDBInfo);
    mp_int32 CheckArchiveThreshold(oracle_db_info_t &stDBInfo);
    mp_int32 Freeze(oracle_db_info_t &stDBInfo);
    mp_int32 Thaw(oracle_db_info_t &stDBInfo);
    mp_int32 ArchiveDB(oracle_db_info_t &stDBInfo);
    mp_int32 StartASMInstance(oracle_db_info_t &stDBInfo);
    mp_int32 StartOracleInstance(oracle_db_info_t &stDBInfo);
    mp_int32 StopOracleInstance(oracle_db_info_t &stDBInfo);
    mp_int32 StopASMInstance(oracle_db_info_t &stDBInfo);
    mp_int32 QueryFreezeState(oracle_db_info_t &stdbInfo, mp_int32 &iFreezeState);
    mp_int32 TruncateArchiveLog(oracle_db_info_t &stDBInfo, mp_time truncTime);
    mp_int32 CheckCDB(oracle_db_info_t &stDBInfo, mp_int32& iCDBType);
	mp_int32 GetPDBInfo(oracle_pdb_req_info_t &stPdbReqInfo, vector<oracle_pdb_rsp_info_t> &vecOraclePdbInfo);
	mp_int32 StartPluginDB(oracle_pdb_req_info_t &stPdbReqInfo);

private:
    mp_int32 AnalyseInstInfoScriptRst(vector<mp_string> &vecResult, list<oracle_inst_info_t> &lstOracleInstInfo);
    mp_int32 AnalyseLunInfoScriptRST(vector<mp_string> &vecResult, vector<oracle_storage_script_info> &vecDBStorageScriptInfo);
    mp_int32 AnalyseLunInfoByScriptRST(vector<oracle_storage_script_info> &vecDBStorageScriptInfo,
        vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType);
	mp_int32 AnalysePDBInfoScriptRst(vector<mp_string> &vecResult, vector<oracle_pdb_rsp_info_t> &vecOraclePdbInfo);

	mp_void BuildPDBInfoScriptParam(oracle_pdb_req_info_t &stPdbInfo, mp_string &strParam);
    mp_void BuildLunInfoScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam);
    mp_void BuildTestScriptParam(oracle_db_info_t stDBInfo, mp_string& strParam);
    mp_void BuildConsistentScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam, mp_string strFrushType);
    mp_int32 BuildTruncateLogScriptParam(oracle_db_info_t &stDBInfo, mp_time truncTime, mp_string &strParam);
    mp_void BuildCheckThresholdScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam);
    mp_void BuildStartASMScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam);
    mp_void BuildStopASMScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam);
    mp_void BuildStartOracleScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam);
    mp_void BuildStopOracleScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam);
    mp_void BuildCheckCDBScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam);

    mp_int32 GetLunInfoByStorageType(oracle_db_info_t stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos,
        mp_string strStorageType);
    mp_bool CheckLUNInfoExists(vector<oracle_lun_info_t> &vecLUNInfos, oracle_lun_info_t &oracle_lun_info);

    mp_int32 AnalyseDatabaseNameByConfFile(list<oracle_inst_info_t> &lstOracleInstInfo);
	mp_int32 TranslatePDBStatus(mp_string &strStatus, mp_int32 &iStatus);
#ifdef LINUX
    mp_int32 GetUDEVConfig(mp_string &strUDEVConfDir, mp_string &strUDEVRoot);
    mp_int32 GetUDEVInfo(mp_string strUdevRulesFileDir, mp_string strUdevName,
        mp_string strUdevResult, mp_string &strUdevDeviceRecord);

#endif

#ifdef WIN32
    mp_int32 GetLUNInfoWin(mp_string &strPath, sub_area_Info_t &stSubareaInfoWin, 
        vector<disk_info> &vecDiskInfoWinRes, oracle_lun_info_t &stDBLUNInfo);
    mp_int32 GetLUNInfoByPathWin(mp_string &strPath, vector<sub_area_Info_t> &vecSubareaInfoWin,
        vector<disk_info> &vecDiskInfoWinRes, vector<oracle_lun_info_t> &vecLUNInfo);
    mp_int32 GetDBLUNInfoWin(vector<mp_string> &vecDiskPath,
        vector<oracle_storage_script_info> &vecAdaptiveLUNInfo, vector<oracle_lun_info_t> &vecLUNInfo);
    mp_int32 GetDBLUNFSInfoWin(vector<mp_string> &vecDiskPath, 
        vector<disk_info> &vecDiskInfoWinRes, vector<sub_area_Info_t> &vecSubareaInfoWin, 
        vector<oracle_lun_info_t> &vecLUNInfos);
    mp_int32 GetDBLUNASMInfoWin(vector<oracle_storage_script_info> &vecAdaptiveLUNInfo, 
        vector<disk_info> &vecDiskInfoWinRes, vector<sub_area_Info_t> &vecSubareaInfoWin, 
        vector<oracle_lun_info_t> &vecLUNInfos);
    mp_int32 AnalyseLunInfoByScriptRSTWIN(vector<oracle_storage_script_info> &vecDBStorageScriptInfo,
        vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType);
#else
    mp_int32 GetAndCheckArraySN(mp_string strDev, mp_string &strArraySN, mp_string strStorageType);
    mp_int32 GetVendorAndProduct(mp_string strDev, mp_string &strVendor, mp_string &strProduct, mp_string strStorageType);
    mp_int32 AnalyseLunInfoByScriptRSTNoWIN(vector<oracle_storage_script_info> &vecDBStorageScriptInfo,
        vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType);
#endif

};

#endif //__AGENT_ORACLE_H__

