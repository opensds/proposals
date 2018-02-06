/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_CLUSTER_H__
#define __AGENT_CLUSTER_H__

#include <vector>
#include "common/Types.h"

#define DB_ORACLE                        "1"
#define DB_DB2                           "3"
#define DB_CACHE                       "15"
#define CLUSTER_VCS                      2
#define CLUSTER_MSFC                     3
#define CLUSTER_POWERHA                  4
#define CLUSTER_SERVICEGUARD             5
#define CLUSTER_RHCS                     6
#define CLUSTER_SUNCLUSTER               7
#define SCRIPTPARAM_OPERTYPE             "OPERTYPE="
#define SCRIPTPARAM_STARTRESGRP          "1"
#define SCRIPTPARAM_STOPRESGRP           "0"
#define WAIT_AFTER_QUERY_STATUS          30 * 1000
#define BE_CLUSTER_SIGN                  "1"
#define CLUSTER_TYPE_MSFC                "3"
//MSFC 集群资源状态
#define MSFC_RESOURCE_STATUS_ONLINE      "Online"
#define MSFC_RESOURCE_STATUS_OFFLINE     "Offline"
#define MSFC_RESOURCE_STATUS_FAILED      "Failed"
#define MSFC_RESOURCE_STATUS_PENDING     "Pending"


#ifdef WIN32
#define DISK_TYPE_GPT "GPT"
#define DISK_ID_GPT "Guid"
#define DISK_TYPE_MBR "MBR"
#define DISK_ID_MBR "Signature"
#define SQLSERVER_SCRIPT_SQLSERVERADAPTIVE      "sqlserverluninfo.bat"
// 返回映射的错误码;
#define RETURN_MAP_ERRCODE(iRet, strOptInfo)                                                \
    CErrorCodeMap errorCode;                                                                \
    mp_int32 iNewRet = errorCode.GetErrorCode(iRet);                                        \
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,                                                 \
        "Exec %s failed, original code: %d, map code: %d", strOptInfo, iRet, iNewRet);      \
    return iNewRet
#endif

typedef struct tag_dataBase_info
{
    mp_string  strinstName; 
    mp_string  strdbName;
    mp_string  strdbUsername;
    mp_string  strdbPassword;
}db_info_t;

typedef struct tag_cluster_info
{
    mp_string strClusterName;           //集群名称
    mp_string strResGrpName;            //资源组名称
    vector<mp_string> vecDevGrpName;    //设备组名称
    vector<mp_string> vecResourceName;  //资源名称列表
    mp_string strVgActiveMode;          //
    mp_string strNetWorkName;           //网络资源名称
}cluster_info_t;

class CCluster
{
public:
    CCluster();
    ~CCluster();

    mp_int32 StartCluster(mp_int32 iClusterType, mp_string strResGrpName);
    mp_int32 IsActiveNode(mp_string& strResGrpName, mp_int32 iClusterType, mp_bool& bIsActive);
    mp_int32 StartResGrp(mp_string strResGrpName, vector<mp_string> vecDevGrpName, mp_string strClusterType, mp_string strDBType, 
        vector<mp_string> &vecResourceName);
    mp_int32 StopResGrp(mp_string strResGrpName, vector<mp_string> vecDevGrpName, mp_string strClusterType, mp_string strDBType, 
        vector<mp_string> &vecResourceName);
    mp_int32 QueryClusterInfo(db_info_t& stdbInfo, mp_string strClusterType, mp_string strDBType, 
        vector<cluster_info_t>& vecClusterInfo);
    
private:
    mp_void AnalyseClusterInfoScriptRst(vector<mp_string> vecResult, vector<cluster_info_t>& vecClusterInfo);
    mp_int32 BuildResGrpScriptParam(mp_string strResGrp, vector<mp_string> vecDevGrp, mp_string strClusterType, mp_string strOperType, 
        mp_string& strParam);
    mp_void BuildClusterInfoScriptParam(db_info_t& stdbInfo, mp_string strClusterType, mp_string &strParam);
    mp_int32 CheckClusterState(mp_int32& iCmd, mp_string& strQueryParam, mp_string& strNormalState);
    mp_int32 CheckResGrpState(mp_int32& iCmd, mp_string& strQueryParam, mp_string& strStableState);
    mp_int32 GetRHCSClusterState(const mp_string& strHostName, mp_bool& bIsStart);
    mp_int32 WaitRHCSClusterStart(const mp_string& strHostName);
    mp_int32 StartRHCSService();
    mp_int32 StartVCSCluster();
    mp_int32 StartServiceGuardCluster(mp_string strResGrpName);
    mp_int32 StartPowerHACluster();

private:
    mp_bool  IsSQLServerActiveNode(const mp_string& strClusterResourceGroup);
    mp_int32 StartSQLServerResGrp(const mp_string &strResGrpName);
    mp_int32 GetSQLServerResourceGroupStatus(const mp_string &strResourceGroup, mp_string &strStatus);
    mp_int32 GetSQLServerClusterName(mp_string &strClusterName);
    mp_int32 GetSQLServerClusterGroup(vector<mp_string> &vecResourceGroup);
    mp_int32 GetSQLServerClusterResources(vector<mp_string> &vecResources);
    mp_int32 GetSQLServerClusterResource(const mp_string &vecDiskName, mp_string &strResource);
    mp_int32 GetDiskSignature(const mp_string &strDiskNumber, mp_string &strDiskSignature, 
        mp_string &strDiskPartitionType);
    mp_int32 GetDiskResBySignature(const mp_string &strDiskSignature, const mp_string &strDiskPartitionType, 
        mp_string &strResource);
    mp_int32 GetSQLServerClusterDisk(db_info_t& stdbInfo, const mp_string & strclusterName, 
        vector<mp_string>& vecDiskName);
    mp_int32 QuerySQLServerClusterInfo(db_info_t& stdbInfo, mp_string strClusterType, 
        vector<cluster_info_t>& vecClusterInfo);
    mp_void ReplaceStr(const mp_string &oldStr, mp_string &newStr, const mp_string &old_value, 
        const mp_string &new_value);
    mp_int32 GetSqlServerFilePath(vector<mp_string> &vecDiskName, const mp_string &strParam);
    mp_int32 GetDiskNum(const mp_string &strDiskName, mp_string &strDiskNumber);
    mp_int32 GetVrlServerName(const mp_string & strResName, mp_string & strServerName);
    mp_bool BeInstanceResource(const mp_string & strResName, const mp_string & strInstName);
    mp_int32 GetClusterGroupByResource(const mp_string & strResName, mp_string & strResourceGroup);
    mp_int32 GetClusterGroupByinstName(const mp_string & strInstName, mp_string &strResourceGroup, 
        mp_string & strServerName);
    mp_int32 OnlineClusterDiskResources(const vector<mp_string> & vecResourceName, const mp_string &strResGrpName);
    mp_int32 SuspendClusterDiskResources(const vector<mp_string> & vecResourceName);
    mp_int32 OnlineClusterDiskResource(const mp_string & strResourceName, const mp_string &strResGrpName);
    mp_int32 onlineResource(const mp_string & strResName);
    mp_int32 SuspendResource(const mp_string &strResName);
    mp_int32 ResetClusterDiskResource(const mp_string & strResName, const mp_string & strDiskMountPoint);
    mp_int32 ResumeDiskResource(const mp_string & strResName);
    mp_void WaitResourceStatusChanged(const mp_string & strResName, mp_string & strStatus);
    mp_int32 GetResourceStatus(const mp_string & strResName, mp_string & strStatus);
    mp_int32 CutResourceName(const mp_string & strResourceName, mp_string & strResName, mp_string& strDiskMountPoint, mp_string &strDiskSignature);
    mp_int32 ExecPowershell(mp_string & strCmd, const mp_string & strFilter, mp_string &strValue);
    mp_int32 StartRHCSCluster();
    mp_int32 StartCLvm();

#ifdef WIN32
private:
    mp_int32 IsClusterResource(const mp_string & strResName, mp_bool &IsResource);
    mp_int32 AddNewResource(const mp_string & strResName, const mp_string & strResourceGroup);
    mp_int32 SetClusterSignature(const mp_string &strResName, const mp_string &strDiskSignature, const mp_string &strDiskPartitionType);
    mp_int32 AddClusterDependency(const mp_string & strResName,mp_string &strSqlDependency);
    mp_int32 RemoveClusterDependency(const mp_string & strResName,mp_string &strSqlDependency);
    mp_int32 RemoveClusterResource(const mp_string & strResName);
    mp_int32 AddNewResourcetoCluster(const mp_string &strResourceName,const mp_string &strResGrpName,const mp_string &strDiskSignature,const mp_string &strDiskPartitionType,const mp_string &strDiskMountPoint);
    mp_void  WaitUntilOnline(const mp_string &strResName, mp_string &strStatus);
    mp_int32 ResourceListSignature(const mp_string &strResName, const mp_string &strDiskSignature);
    mp_void  StringHexToDec(const mp_string &str,mp_string &strout);
    mp_int32 ClusterResourceReset(const mp_string &strResName, const mp_string &strResGrpName,const mp_string &strDiskSignature, const mp_string &strDiskMountPoint,const mp_string &strDiskPartitionType);
    mp_int32 DiskListSignature(const mp_string &strResName,const mp_string &strDiskSignature,const mp_string strDiskPartitionType);
    mp_int32 DiskAndMountPoint(const mp_string &strDiskSignature,const mp_string &strDiskMountPoint,const mp_string &strDiskPartitionType);
    mp_int32 GetDiskNumBySignature(const mp_string &strDiskSignature,const mp_string &strDiskPartitionType,mp_string & strDiskNum);
    mp_int32 ResetResourceBySignature(const mp_string &strResName,const mp_string &strDiskSignature,const mp_string & strDiskPartitionType);
    mp_int32 QueryDependencyResource(const mp_string & strResourceGroup,mp_string &strSqlDependency);
    mp_int32 GetSignatureByWWN(const mp_string &strWWN,mp_string &strDiskSignature);
    mp_int32 IsDiskSignature(const mp_string &strDiskSignature,const mp_string &strDiskPartitionType);
    mp_int32 IsClusterDependency(const mp_string & strResName,const mp_string &strSqlDependency, mp_bool &bExist);
#endif
};

#endif //__AGENT_CLUSTER_H__

