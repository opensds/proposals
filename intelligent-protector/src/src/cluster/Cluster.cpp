/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include <fstream>

#include "cluster/Cluster.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "common/String.h"
#include "host/Host.h"
#include "device/FileSys.h"
#include "apps/sqlserver/SqlServer.h"

CCluster::CCluster()
{
}

CCluster::~CCluster()
{
}

/*------------------------------------------------------------ 
Description  : 分析获取集群信息脚本返回结果
Input        :vecResult -- 脚本返回结果字符串
Output       : vecClusterInfo -- 集群信息
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CCluster::AnalyseClusterInfoScriptRst(vector<mp_string> vecResult, vector<cluster_info_t>& vecClusterInfo)
{
    mp_string strAllDevGrp;
    vector<mp_string> vecDevGrp;
    vector<mp_string>::iterator iter;
    size_t idxSep, idxSepSec, idxSepThr;
    cluster_info_t stClusterInfo;
    const mp_string SEPARATOR = STR_COLON;    
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to analyse cluster info script result.");

    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
    
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db cluster info failed when find 1nd separator, cluster info=%s.", (*iter).c_str());
            continue;
        }
        stClusterInfo.strClusterName = iter->substr(0, idxSep);
        
        //find 2nd separator(;)            
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db cluster info failed when find 2nd separator, cluster info=%s.", (*iter).c_str());
            continue;
        }      
        stClusterInfo.strResGrpName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);  

        //find 3nd separator(;)            
        idxSepThr = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepThr)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db cluster info failed when find 3nd separator, cluster info=%s.", (*iter).c_str());
            continue;
        }

        strAllDevGrp = iter->substr(idxSepSec + 1, (idxSepThr - idxSepSec) - 1);
        CMpString::StrSplit(stClusterInfo.vecDevGrpName, strAllDevGrp, '+');
        stClusterInfo.strVgActiveMode = iter->substr(idxSepThr + 1);

        vecClusterInfo.push_back(stClusterInfo);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to analyse cluster info script result.");
}


/*------------------------------------------------------------ 
Description  : 构建资源组操作脚本输入参数
Input        :strResGrp -- 资源组名称
              vecDevGrp -- 设备组名称列表
              strClusterType -- 集群类型
              strOperType -- 操作类型
Output       :strParam -- 脚本输入参数
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCluster::BuildResGrpScriptParam(mp_string strResGrp, vector<mp_string> vecDevGrp, mp_string strClusterType, mp_string strOperType, 
    mp_string& strParam)
{
    vector<mp_string>::iterator iter;
    mp_string strAllDevGrp;
    for(iter = vecDevGrp.begin();iter != vecDevGrp.end();iter++)
    {
        if(strAllDevGrp.empty())
        {
            strAllDevGrp = *iter;
        }
        else
        {
            strAllDevGrp = strAllDevGrp + mp_string("+") + *iter;
        }
    }
    strParam = SCRIPTPARAM_RESGRPNAME + strResGrp + STR_COLON
        + SCRIPTPARAM_DEVGRPNAME + strAllDevGrp + STR_COLON
        + SCRIPTPARAM_CLUSTERTYPE + strClusterType + STR_COLON
        + SCRIPTPARAM_OPERTYPE+ strOperType ;

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 构建获取集群信息脚本输入参数
Input        :stdbInfo -- 数据库信息
              strClusterType -- 集群类型
Output       :strParam -- 脚本输入参数
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CCluster::BuildClusterInfoScriptParam(db_info_t& stdbInfo, mp_string strClusterType, mp_string &strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbInfo.strinstName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbInfo.strdbName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbInfo.strdbUsername+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbInfo.strdbPassword+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_CLUSTERTYPE) + strClusterType;
}

/*------------------------------------------------------------ 
Description  : 检查集群是否启动成功
Input        :iCmd -- 查询集群状态命令
                 strQueryParam -- 查询集群命令参数
                 strNormalState -- 正常状态名称
Output       :
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCluster::CheckClusterState(mp_int32& iCmd, mp_string& strQueryParam, mp_string& strNormalState)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_uint32 uiCount = 0;
    mp_string strNodeState;
    vector<mp_string> vecResult;

    for (uiCount = 0; uiCount < 10; uiCount++)
    {
        vecResult.clear();
        iRet = CRootCaller::Exec((mp_int32)iCmd, strQueryParam, &vecResult);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd of check cluster service state failed.");
            return ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED;
        }

        if (!vecResult.empty())
        {
            strNodeState = vecResult.front();
            if(strNodeState == strNormalState)
            {
                break;
            }
        }
        DoSleep(WAIT_AFTER_QUERY_STATUS);
    }

    if (10 == uiCount)
    {
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 检查serviceguard程序包状态是否稳定
Input        :iCmd -- 查询serviceguard程序包状态命令
                 strQueryParam -- 查询serviceguard程序包命令参数
                 strNormalState -- 正常状态名称
Output       :
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCluster::CheckResGrpState(mp_int32& iCmd, mp_string& strQueryParam, mp_string& strStableState)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iCount = 0;
    mp_string strResGrpState;
    vector<mp_string> vecResult;

    for (iCount = 0; iCount < 10; iCount++)
    {
        vecResult.clear();
        iRet = CRootCaller::Exec((mp_int32)iCmd, strQueryParam, &vecResult);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd of check package service state failed.");
            return ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED;
        }

        if (!vecResult.empty())
        {
            strResGrpState = vecResult.front();
            if(mp_string::npos != strStableState.find(strResGrpState))
            {
                break;
            }
        }
        DoSleep(WAIT_AFTER_QUERY_STATUS);
    }

    if (10 == iCount)
    {
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取RHCS集群状态
Input        :    strHostName---主机名
Output       :  bIsStart---是否启动
Return       :   MP_SUCCESS---获取成功
                 iRet---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CCluster::GetRHCSClusterState(const mp_string& strHostName, mp_bool& bIsStart)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strNodeState;
    mp_string strParam = "|awk '$1==\"" + strHostName + "\" {print $3$4$5}'";

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get RHCS cluster state, host name %s.", strHostName.c_str());
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RHCS_CLUSTAT, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec get RHCS cluster cmd failed, iRet %d.", iRet);
        return iRet;
    }
    
    if (!vecResult.empty())
    {
        strNodeState = vecResult.front();
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The cluster service status is %s.", strNodeState.c_str());
        if ((strNodeState.find("Online") != mp_string::npos) && (strNodeState.find("rgmanager") != mp_string::npos))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Cluster service has been started.");
            bIsStart = MP_TRUE;
            return MP_SUCCESS;
        }
    }

    bIsStart = MP_FALSE;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End get RHCS cluster state.");
#endif
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :等待RHCS集群启动
Input        :    strHostName---主机名
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CCluster::WaitRHCSClusterStart(const mp_string& strHostName)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_uint32 uiCount = 0;
    mp_bool bIsStart = MP_FALSE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin wait start RHCS cluster.");
    for (uiCount = 0; uiCount < 10; uiCount++)
    {
        iRet = GetRHCSClusterState(strHostName, bIsStart);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get RHCS cluster state failed, iRet %d.", iRet);
            return ERROR_CLUSTER_START_SERVICE_FAILED;
        }

        if (bIsStart)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Cluster service has been started.");
            break;
        }

        DoSleep(WAIT_AFTER_QUERY_STATUS);
    }

    if (10 == uiCount)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Wait start RHCS failed.");
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }
#endif
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Wait start RHCS cluster succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :启动RHCS服务
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CCluster::StartRHCSService()
{
#ifndef WIN32
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Start RHCS Service.");
    //启动集群
    mp_string strParam = "cman start";
    mp_int32 iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RHCS_SERVICE, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "service %s failed, iRet = %d", strParam.c_str(), iRet);
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }

    //启动CLVM 
    iRet = StartCLvm();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "start clvm failed, iRet %d.", iRet);
        return iRet;
    }

    strParam = "rgmanager start";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RHCS_SERVICE, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "service %s failed, iRet = %d", strParam.c_str(), iRet);
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }
#endif

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Start RHCS Service succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 上线资源组
Input        :strResGrpName -- 资源组名称
                 strClusterType -- 集群类型
                 strDBType -- 数据库类型
                 vecResourceName -- 资源名称
Output       :
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCluster::StartResGrp(mp_string strResGrpName, vector<mp_string> vecDevGrpName, mp_string strClusterType, mp_string strDBType, 
    vector<mp_string> &vecResourceName)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,  "Begin to excute start resourcgroup script.");
    BuildResGrpScriptParam(strResGrpName, vecDevGrpName, strClusterType, SCRIPTPARAM_STARTRESGRP, strParam);
    if (DB_ORACLE == strDBType)
    {
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLERESOURCEGROUP, strParam, NULL);
    }
    else if (DB_DB2 == strDBType)
    {
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_DB2RESOURCEGROUP, strParam, NULL);
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The db type(%s) is not be supported.", strDBType.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    TRANSFORM_RETURN_CODE(iRet, ERROR_CLUSTER_PACKAGE_ONLINE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec start resourcegroup script failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,  "Excute start resourcgroup script succ.");
#else
    mp_int32 iRet = MP_SUCCESS;
    if (mp_string(CLUSTER_TYPE_MSFC) == strClusterType)
    {
        //上线& 修复磁盘资源
        iRet = OnlineClusterDiskResources(vecResourceName,strResGrpName);
        TRANSFORM_RETURN_CODE(iRet, ERROR_CLUSTER_PACKAGE_ONLINE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Online Disk resource failed.");
            return iRet;
        }
        //上线实例对应的资源组
        iRet = StartSQLServerResGrp(strResGrpName);
        TRANSFORM_RETURN_CODE(iRet, ERROR_CLUSTER_PACKAGE_ONLINE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start sqlserver cluster group failed.");
            return iRet;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Cluster Type is not support, input cluster type is: %s.", 
            strClusterType.c_str());
        return ERROR_COMMON_INVALID_PARAM; //参数错误的错误码
    }
#endif

    return iRet;
}


/*------------------------------------------------------------ 
Description  : 下线资源组
Input        :strResGrpName -- 资源组名称
                 strClusterType -- 集群类型
                 strDBType -- 数据库类型
                 vecResourceName -- 资源名称
Output       :
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::StopResGrp(mp_string strResGrpName, vector<mp_string> vecDevGrpName, mp_string strClusterType, mp_string strDBType, 
    vector<mp_string> &vecResourceName)
{
    mp_int32 iRet = MP_SUCCESS;
#ifndef WIN32
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,  "Begin to excute stop resourcgroup script.");

    BuildResGrpScriptParam(strResGrpName, vecDevGrpName, strClusterType, SCRIPTPARAM_STOPRESGRP, strParam);
    if (DB_ORACLE == strDBType)
    {
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLERESOURCEGROUP, strParam, NULL);
    }
    else if (DB_DB2 == strDBType)
    {
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_DB2RESOURCEGROUP, strParam, NULL);
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The db type(%s) is not be supported.", strDBType.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    TRANSFORM_RETURN_CODE(iRet, ERROR_CLUSTER_PACKAGE_ONLINE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec stop resourcegroup script failed.");
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,  "Excute stop resourcgroup script succ.");
#else
    if (mp_string(CLUSTER_TYPE_MSFC) == strClusterType)
    {
        //资源进入维护模式
        iRet = SuspendClusterDiskResources(vecResourceName);
        TRANSFORM_RETURN_CODE(iRet, ERROR_CLUSTER_SUSPEND_DISK_RESOURCE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec stop resourcegroup script failed.");
            return iRet;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unknown cluster type.");
        return ERROR_CLUSTER_PACKAGE_OFFLINE_FAILED;
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 本机是否活动节点
Input        :strResGrpName -- 资源组名称
                 iClusterType -- 集群类型
Output       :bIsActive -- 是否是活动节点
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::IsActiveNode(mp_string& strResGrpName, mp_int32 iClusterType, mp_bool& bIsActive)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strParam;
    mp_string strActiveHostName;
    mp_string strLocalHostName;
    mp_bool bRet = MP_FALSE;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,  "Begin to get active host of cluster.");
    //获取本机名
    iRet = GetHostName(strLocalHostName);
    if (MP_SUCCESS != iRet)
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host name failed.");
         return ERROR_CLUSTER_QUERY_NODE_FAILED;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The local host name is %s.", strLocalHostName.c_str());

    if(CLUSTER_VCS == iClusterType)
    {
        strParam = " -state|awk '{if ($1==\"" + strResGrpName + "\") print $0}'|grep 'ONLINE'|awk '{print $3}'";
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_HARES, strParam, &vecResult);
    }
    else if(CLUSTER_POWERHA == iClusterType)
    {
        strParam = " " + strResGrpName + " | grep -n -e 'ONLINE' | awk '{print $3}'";
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CLFINDRES, strParam, &vecResult);
    }
    else if(CLUSTER_SERVICEGUARD == iClusterType)
    {
        //strParam = "-l node | grep -n -e 'running' | awk '{print $2}' ";
        strParam = " -p " + strResGrpName + " | grep '" + strResGrpName + "'|awk '{print $NF}' ";
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CMVIEWCL, strParam, &vecResult);
    }
    else if (CLUSTER_RHCS == iClusterType)
    {
        strParam = " -s " + strResGrpName + " | grep '" + strResGrpName + "'|awk '{print $2}' "; 
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RHCS_CLUSTAT, strParam, &vecResult);
    }
    else if (CLUSTER_SUNCLUSTER== iClusterType)
    {
        strParam = " status " + strResGrpName + " | grep Online | nawk '{print $(NF/2)}'"; 
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMNAD_CLRG, strParam, &vecResult);
    }
    
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd of query active host failed.", strResGrpName.c_str());
        return iRet;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of query active host is empty, param %s.", strParam.c_str());
        return ERROR_CLUSTER_QUERY_ACTIVE_HOST_FAILED;
    }

    strActiveHostName = vecResult.front();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The active host name of resourcegroup %s is %s.",
        strResGrpName.c_str(), strActiveHostName.c_str());

    //RHCS集群去掉前后的括号
    bRet = ((CLUSTER_RHCS == iClusterType) && (strActiveHostName.length() >= 2) 
        && strActiveHostName[0] == '(' && strActiveHostName[strActiveHostName.length() - 1] == ')');
    if (bRet)
    {
        mp_string strTmp = strActiveHostName.substr(1, strActiveHostName.length() - 2);
        strActiveHostName = strTmp;
    }

    if(strActiveHostName != strLocalHostName)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The host is not active host.");
        bIsActive = MP_FALSE;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get active host of cluster succ.");
#else
    if(CLUSTER_MSFC == iClusterType)
    {
        bIsActive = IsSQLServerActiveNode(strResGrpName);
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Cluster Type is not support, input cluster type is: %d.", iClusterType);
        bIsActive = MP_FALSE;
        return ERROR_COMMON_INVALID_PARAM; //参数错误的错误码
    }

#endif
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 启动集群
Input        :iClusterType -- 集群类型
Output       :
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::StartCluster(mp_int32 iClusterType, mp_string strResGrpName)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    if (CLUSTER_VCS == iClusterType)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The cluster is vcs cluster.");
        iRet = StartVCSCluster();
    }
    else if (CLUSTER_POWERHA == iClusterType)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The cluster is powerha cluster.");
        iRet = StartPowerHACluster();
    }
    else if (CLUSTER_SERVICEGUARD == iClusterType)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The cluster is serviceguard cluster.");
        iRet = StartServiceGuardCluster(strResGrpName);
    }
    else if (CLUSTER_RHCS == iClusterType)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The cluster is rhcs cluster.");
        iRet = StartRHCSCluster();
    }
    else if (CLUSTER_SUNCLUSTER== iClusterType)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The cluster is suncluster.");
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The cluster type(%d) is not supported.", iClusterType);
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start cluster failed.");
        return iRet;
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询集群信息
Input        :stdbInfo -- 数据库信息
                 strClusterType -- 集群类型
                 strDBType -- 数据库类型
Output       :vecClusterInfo -- 集群信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::QueryClusterInfo(db_info_t& stdbInfo, mp_string strClusterType, mp_string strDBType, 
    vector<cluster_info_t>& vecClusterInfo)
{
#ifndef WIN32
    mp_string strParam;
    vector<mp_string> vecResult;

    BuildClusterInfoScriptParam(stdbInfo, strClusterType, strParam);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get cluster info of db.");

    //db2和oracle下获取获取集群信息命令必须在root下执行
    if (DB_ORACLE == strDBType)
    {
        CHECK_MP_RETURN(CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYORACLECLUSTERINFO, strParam, &vecResult), 
            ERROR_COMMON_SCRIPT_EXEC_FAILED);
    }
    else if (DB_DB2 == strDBType)
    {
        CHECK_MP_RETURN(CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYDB2CLUSTERINFO, strParam, &vecResult), 
            ERROR_COMMON_SCRIPT_EXEC_FAILED);
    }
    else if (DB_CACHE == strDBType)
    {
        CHECK_MP_RETURN(CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYCACHECLUSTERINFO, strParam, &vecResult), 
            ERROR_COMMON_SCRIPT_EXEC_FAILED);
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The db type(%s) is not be supported.", strDBType.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    AnalyseClusterInfoScriptRst(vecResult, vecClusterInfo);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get cluster info of db succ.");
#else
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get cluster info of db, cluster type %s, db type %s.",
        strClusterType.c_str(), strDBType.c_str());
    
    if (mp_string(CLUSTER_TYPE_MSFC) == strClusterType)
    {
        iRet = QuerySQLServerClusterInfo(stdbInfo, strClusterType, vecClusterInfo);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get cluster info of db succ.");

#endif
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 启动VCS集群服务
Input        :
Output       :
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::StartVCSCluster()
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_string strHostName;
    mp_string strNodeState;
    mp_string strParam;
    mp_string strNormalState = "RUNNING";
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start vcs cluster.");

    //获取本机名
    iRet = GetHostName(strHostName);
    if (MP_SUCCESS != iRet)
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host name failed.");
         return ERROR_CLUSTER_QUERY_NODE_FAILED;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The local host name is %s.", strHostName.c_str());

    //查询本节点上集群服务状态
    strParam = " -state " + strHostName;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_HASYS, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd of get cluster service state failed.");
        return ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED;
    }

    if (!vecResult.empty())
    {
        strNodeState = vecResult.front();
        if (strNodeState == strNormalState)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Cluster service has been started.");
            return MP_SUCCESS;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "The result of query cluster service state is empty.");
        return ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED;
    }

    //启动集群
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_HASTART, "", NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd of start cluster service failed.");
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }

    mp_int32 iCmd = ROOT_COMMAND_HASYS;
    iRet = CheckClusterState(iCmd, strParam, strNormalState);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start vcs cluster failed or timeout.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start vcs cluster succ.");
#endif
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 启动ServiceGuard集群服务
Input        :
Output       :
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::StartServiceGuardCluster(mp_string strResGrpName)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    mp_string strLocalHostName;
    mp_string strNodeState;
    mp_string strNormalState("running");
    mp_string strStableState("running+halted+failed");
    vector<mp_string> vecResult;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start serviceguard cluster.");
    //获取本机名
    iRet = GetHostName(strLocalHostName);
    if (MP_SUCCESS != iRet)
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host name failed.");
         return ERROR_CLUSTER_QUERY_NODE_FAILED;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The local host name is %s.", strLocalHostName.c_str());
        
    strParam = " -l node | grep " + strLocalHostName + " |awk '{print $NF}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CMVIEWCL, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd of get node state of serviceguard failed.");
        return ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED;
    }

    if (!vecResult.empty())
    {
        strNodeState = vecResult.front();
        if (strNodeState == strNormalState)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Cluster service has been started.");
            return MP_SUCCESS;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "The result of query cluster service state is empty.");
        return ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The cluster service status is %s.", strNodeState.c_str());

    //启动集群
    strParam =  " -v";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CMRUNCL, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start serviceguard cluster service failed.");
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }

    strParam = " -l node | grep " + strLocalHostName + " |awk '{print $NF}'";
    mp_int32 iCmd = ROOT_COMMAND_CMVIEWCL;
    iRet = CheckClusterState(iCmd, strParam, strNormalState);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start serviceguard cluster failed or timeout.");
        return iRet;
    }


    strParam = " -p " + strResGrpName + " |grep " + strResGrpName + " |awk '{print $3}'";
    iCmd = ROOT_COMMAND_CMVIEWCL;
    iRet = CheckResGrpState(iCmd, strParam, strStableState);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start serviceguard cluster failed or timeout.");
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start serviceguard cluster succ.");
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 启动PowerHA集群服务
Input        :
Output       :
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::StartPowerHACluster()
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;;
    mp_string strNodeState;
    mp_string strNodeList; //Hacmp集群节点名称
    mp_string strParam;
    mp_string strNormalState = "ST_STABLE";
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start powerha cluster.");

    //获取集群中节点列表
    strParam = "| grep Node |awk '{print $2}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CLLSNODE, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd of get node list of powerha failed.");
        return ERROR_CLUSTER_QUERY_NODE_FAILED;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The resulte of get node list of powerha is empty.");
        return ERROR_CLUSTER_QUERY_NODE_FAILED;
    }

    vector<mp_string>::iterator iter = vecResult.begin();
    strNodeList = *iter;
    iter++;
    for ( ; iter != vecResult.end(); ++iter)
    {
        strNodeList += ",";
        strNodeList += *iter;
    }
    vecResult.clear();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The node list of cluster is %s.", strNodeList.c_str());

    //查询集群服务状态
    strParam = " -ls clstrmgrES | sed -n '1p' | awk '{print $3}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LSSRC, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd of query cluster service state failed.");
        return ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED;
    }

    if (!vecResult.empty())
    {
        strNodeState = vecResult.front();
        if (strNormalState == strNodeState)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Powerha cluster service has been started.");
            return MP_SUCCESS;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of query cluster service state is empty.");
        return ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The cluster service status is %s.", strNodeState.c_str());

    //启动集群
    strParam =  " '-N' -cspoc-n '" + strNodeList + "' '-A' '-b' '-i' '-C yes'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_STARTPOWERHA, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start powerha cluster service failed.");
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }

    strParam = " -ls clstrmgrES | sed -n '1p' | awk '{print $3}'";
    mp_int32 iCmd = ROOT_COMMAND_LSSRC;
    iRet = CheckClusterState(iCmd, strParam, strNormalState);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start powerha cluster failed or timeout.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start powerha cluster succ.");
#endif
    return MP_SUCCESS;
}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::IsSQLServerActiveNode
Description  : 通过资源组名称查询集群活动主机;
Input        : strClusterResourceGroup,集群资源组名称;
Output       : MP_BOOL;
Return       : MP_BOOL
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_bool CCluster::IsSQLServerActiveNode(const mp_string& strClusterResourceGroup)
{
    mp_bool bRet = MP_FALSE;
#ifdef WIN32
    mp_int32 iRet = MP_SUCCESS;
    if (strClusterResourceGroup.empty())
    {
        return bRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,"Begin Check Cluster Active Host.");

    // 获取本地主机名称;
    mp_string strLocalName, strHostName;
    vector<mp_string> vecEcho;
    mp_string strCmd = "hostname";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_FAILED != iRet)
    {
        strLocalName = vecEcho.empty() ? "" : vecEcho.front();
    }
    else
    {
        return bRet;
    }
    
    // 从集群获取活动主机;
    strCmd = "cmd.exe /c powershell \"get-clusterGroup \\\"";
    strCmd += strClusterResourceGroup;
    strCmd += "\\\" | select OwnerNode | fl * ";
    vector<mp_string> vecReturn;

    iRet = ExecPowershell(strCmd, "OwnerNode", strHostName);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Active node of %s is failed.", strClusterResourceGroup.c_str());
        
        return MP_FALSE;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,"strLocalName:%s, active hostname:%s", strLocalName.c_str(), strHostName.c_str());

    if (strcmp(strHostName.c_str(), strLocalName.c_str()) == 0)
    {
        bRet = MP_TRUE;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,"End Query Cluster Active Host.");
#endif

    return bRet;
}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::StartSQLServerResGrp
Description  : 启动集群资源组;
Input        : strResGrpName,集群资源组名称;
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::StartSQLServerResGrp(const mp_string &strResGrpName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = ERROR_CLUSTER_PACKAGE_ONLINE_FAILED;
    mp_string strStatus;
    mp_ulong ulRetCode = 0;
    mp_string::size_type iFindPos = 0;

    if (strResGrpName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Empty Parameter.");
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Try to online resourceGroup:[%s]", strResGrpName.c_str());

    mp_string strCmd = "cmd.exe /c powershell \"Start-ClusterGroup \\\"";
    strCmd += strResGrpName;
    strCmd += "\\\" \" ";

    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_FAILED != iRet)
    {
        while (std::string::npos != iFindPos)
        {
            Sleep(3000);

            iRet = GetSQLServerResourceGroupStatus(strResGrpName, strStatus);
            if (MP_FAILED == iRet)
            {
                return iRet;
            }
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Cluster Instance Status: %s", strStatus.c_str());

            iFindPos = strStatus.find("Failed");
            if (std::string::npos != iFindPos)
            {
                //Error:启动资源组失败
                return ERROR_CLUSTER_PACKAGE_ONLINE_FAILED;
            }

            iFindPos = strStatus.find("Pending");
            if (std::string::npos == iFindPos)
            {
                break;
            }
        }

        if ("Online" == strStatus)
        {
            return MP_SUCCESS;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Online Cluster Resource Group Fail. ResourceGroup: %s.", strResGrpName.c_str());
    }
    //Error: 启动资源组失败
    return iRet;
#endif

}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetSQLServerResourceGroupStatus
Description  : 获取资源组状态;
Input        : const mp_string &strResourceGroup, 资源组名称;
               mp_string &strStatus, 资源组状态;
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetSQLServerResourceGroupStatus(const mp_string &strResourceGroup, mp_string &strStatus)
{
    strStatus.clear();
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = ERROR_CLUSTER_PACKAGE_ONLINE_FAILED;
    mp_string strLine;
    mp_string strTitle;
    std::list<mp_string> lstResourceStatus;
    vector<mp_string> vecReturn;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get ResourceGroup: %s Status Info.", strResourceGroup.c_str());

    // 获取实例资源名称;
    mp_string strCmd = "";
    strCmd += "cmd.exe /c powershell \"Get-ClusterResource | ? {$_.ResourceType -eq \\\"SQL Server\\\" -and $_.OwnerGroup -eq  \\\"";
    strCmd += strResourceGroup;
    strCmd += "\\\"} | select State | fl * ";

    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecReturn);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command failed.");
        //Error:获取实例对应资源的状态失败
        return ERROR_CLUSTER_PACKAGE_ONLINE_FAILED;
    }
    mp_size lenReturn = vecReturn.size();
    for (mp_uint32 i = 0; i < lenReturn; ++i)
    {
        mp_string strLine = vecReturn[i];
        vector<mp_string> lstResourceStatus;
        (void)CMpString::StrSplit(lstResourceStatus, strLine, ' ');
        if (lstResourceStatus.empty())
        {
            continue;
        }
        
        strTitle = lstResourceStatus.front();
        if ("State" == strTitle)
        {
            char cstrStatus[256] = {0};
            strStatus = lstResourceStatus.back();
            iRet = SNPRINTF_S(cstrStatus, sizeof(cstrStatus), sizeof(cstrStatus) - 1, "%s", strStatus.c_str());
            if (MP_FAILED == iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call _snprintf_s failed, ret %d.", iRet);
                break;
            }
        
            (mp_void)CMpString::Trim(cstrStatus);
            strStatus = cstrStatus;
            iRet = MP_SUCCESS;
        }

    }
    
    if (strStatus.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get DB Cluster Instance Status Fail.");
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get DB Cluster Instance Status: %s", strStatus.c_str());

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get ResourceGroup:[%s] Status Info.", strResourceGroup.c_str());
    //Error:获取实例对应资源的状态失败
    return iRet;
#endif
}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetSQLServerClusterName
Description  : 获取SQLServer集群名称;
Input        : mp_string &strClusterName; 集群名称;
Output       : vecClusterName;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetSQLServerClusterName(mp_string &strClusterName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = ERROR_CLUSTER_QUERY_FAILED;
    vector<mp_string> vecEcho;
    mp_string strCmd = "cmd.exe /c powershell \"get-cluster | select Name | fl *\"";
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get SQLServer Cluster Name.");

    iRet = ExecPowershell(strCmd, "Name", strClusterName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SQLServer Cluster Name failed, strCmd is %s.", strCmd.c_str());
        return ERROR_CLUSTER_QUERY_FAILED;
    }

    // 获取集群名称列表;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get SQLServer ClusterName:[%s]", strClusterName.c_str());

    return iRet;
#endif
}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetSQLServerClusterGroup
Description  : 获取SQLServer 所有的集群资源名称;
Input        : vector<mp_string> &vecResources, 资源名称列表;
Output       : strResourceGroup;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetSQLServerClusterResources(vector<mp_string> &vecResources)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = ERROR_CLUSTER_PACKAGE_ONLINE_FAILED;
    mp_string strCmd = "cmd.exe /c powershell \"get-clusterResource | ?{$_.ResourceType -eq \\\"SQL Server\\\"} | select Name | fl * ";
    mp_string strFilter = "Name";
    vector<mp_string> vecReturn;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get SQLServer Cluster Resource ");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "## strCmd:%s", strCmd.c_str());
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecReturn);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command failed.");
        //Error:获取sqlserver 集群资源失败
        return ERROR_CLUSTER_PACKAGE_ONLINE_FAILED;
    }
    mp_size lenReturn = vecReturn.size();
    for (mp_uint32 i = 0; i < lenReturn; ++i)
    {
        vector<mp_string> vecEcho;
        mp_string strTmp = vecReturn[i];
        (void)CMpString::StrSplit(vecEcho, strTmp, ':');
        if (!vecEcho.empty())
        {
            mp_string strTitle = vecEcho.front();
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "title:%s", strTitle.c_str());
            (mp_void)CMpString::Trim((mp_char*)strTitle.c_str());
            (mp_void)CMpString::TotallyTrimRight((mp_char*)strTitle.c_str());
            if (strcmp(strFilter.c_str(), strTitle.c_str()) == 0)
            {
                mp_string strValueTmp = vecEcho.back();
                (mp_void)CMpString::Trim((mp_char*)strValueTmp.c_str());
                (mp_void)CMpString::TotallyTrimRight((mp_char*)strValueTmp.c_str());
                mp_string strValue(strValueTmp, 0, strlen(strValueTmp.c_str()));
                vecResources.push_back(strValue);
                COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get value:%s", strValue.c_str());
            }
        }
    }
    // 查询结果为空，也为错误;
    if (vecResources.empty())
    {
        //Error:不存在sqlserver的资源组
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SQLServer Cluster Resource is not exist, iRet %d", 
            ERROR_CLUSTER_SQLSERVER_RESOURCE_NOT_EXIST);
        
        return ERROR_CLUSTER_SQLSERVER_RESOURCE_NOT_EXIST;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get SQLServer Cluster Resource");
    return iRet;
#endif
}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetVrlServerName
Description  : 查询虚拟服务器;
Input        : const mp_string & strResName, 资源名称;
                mp_string & strServerName, 虚拟服务器名称;
Output       : strServerName;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/

mp_int32 CCluster::GetVrlServerName(const mp_string & strResName, mp_string & strServerName)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get Virtual Server name");
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = ERROR_CLUSTER_GET_CLUSTER_NETWORK_NAME_FAILED;
    mp_string strCmd = "cmd.exe /c powershell \"get-clusterResource -Name \\\"";
    strCmd += strResName;
    strCmd += "\\\" | get-clusterParameter | ? {$_.Name -eq \\\"VirtualServerName\\\"} | select Value | fl * ";
    mp_string strFilter = "Value";

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get vrl server name, cmd:%s.", strCmd.c_str());
    //获取实例名称
    iRet = ExecPowershell(strCmd, strFilter, strServerName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get networkname failed, exec powershell command[%s] failed.", 
            strCmd.c_str());
        //Error:获取集群网络资源名称失败
        return ERROR_CLUSTER_GET_CLUSTER_NETWORK_NAME_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Virtual Server name succ, VrlServerName:%s", strServerName.c_str());
    return iRet;
#endif
}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::BeInstanceResource
Description  : 判断是否是对应实例的资源并且直接查询虚拟服务器;
Input        : const mp_string & strResName, 资源名称;
                const mp_string & strInstName,  实例名称;
                mp_string & strServerName, 虚拟服务器名称;
Output       : strResourceGroup;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/

mp_bool CCluster::BeInstanceResource(const mp_string & strResName, const mp_string & strInstName)
{
    mp_bool bRet = MP_FALSE;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin check Instance Resource");
#ifdef WIN32
    mp_string strCmd = "cmd.exe /c powershell \"get-clusterResource -Name \\\"";
    strCmd += strResName;
    strCmd += "\\\" | get-clusterParameter | ? {$_.Name -eq \\\"InstanceName\\\"} | select Value | fl *\"";
    mp_string strFilter = "Value";
    mp_string strTmpInst;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "## strCmd:%s", strCmd.c_str());
    //获取实例名称
    mp_int32 iRet = ExecPowershell(strCmd, strFilter, strTmpInst);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get resource instance failed, exec powershell command[%s] failed.", 
            strCmd.c_str());
        //Error: 查询数据库实例对应的资源名称失败

        return bRet;
    }
    
    //本次value为实例名称
    
    if (strInstName == strTmpInst)
    {
        bRet = MP_TRUE;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "resource (%s) is in instance :%s", strResName.c_str(), 
            strTmpInst.c_str());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Input instance:%s, query instance:%s.", strInstName.c_str(), 
            strTmpInst.c_str()); 
#endif

    return bRet;
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetClusterGroupByResource
Description  : 获取指定资源所在的SQLServer集群资源组;
Input        : cconst mp_string & strResName, 资源名称;
                mp_string & strResourceGroup, 资源组名称;
Output       : strResourceGroup;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/

mp_int32 CCluster::GetClusterGroupByResource(const mp_string & strResName, mp_string & strResourceGroup)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = ERROR_CLUSTER_QUERY_GROUP_INFO_FAILED;
    mp_string strCmd = "cmd.exe /c powershell \"get-clusterResource -Name \\\"";
    strCmd += strResName;
    strCmd += "\\\" | select OwnerGroup | fl * ";
    mp_string strFilter = "OwnerGroup";

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get cluster group, cmd:%s.", strCmd.c_str());
    //获取集群资源组
    iRet = ExecPowershell(strCmd, strFilter, strResourceGroup);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get cluster group failed, exec powershell command[%s] failed.", 
            strCmd.c_str());
                //Error:通过资源查找对应的资源组失败
        return ERROR_CLUSTER_QUERY_GROUP_INFO_FAILED;
    }
    
    // 查询结果为空，也为错误;
    if (strResourceGroup.empty())
    {
        //Error:通过资源查找对应的资源组失败，为空
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get cluster group failed, the strResourceGroup is empty.");
        iRet = ERROR_CLUSTER_QUERY_GROUP_INFO_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get SQLServer Cluster Resource Group");
    return iRet;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetClusterGroupByinstName
Description  : 获取SQLServer集群资源组;
Input        : const mp_string & strInstName, 数据库实例名称;
                mp_string &strResourceGroup,  资源组名称;
                mp_string & strServerName, 虚拟服务器名称;
Output       : strResourceGroup;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetClusterGroupByinstName(const mp_string & strInstName, mp_string &strResourceGroup, 
    mp_string & strServerName)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get SQLServer Cluster Resource Group");
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = MP_FAILED;
    //获取sqlserver 所有的资源(资源类型为 sqlserver的所有资源)
    vector<mp_string> vecResources;
    iRet = GetSQLServerClusterResources(vecResources);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SQLServer Cluster Group Failed");
        return iRet;
    }

    mp_size ResSize = vecResources.size();
    for (mp_uint32 i = 0; i < ResSize; ++i)
    {
        //遍历资源，判断哪个资源是当前查询的数据库实例的资源
        mp_bool bInstanceRes = BeInstanceResource(vecResources[i], strInstName);
        if (bInstanceRes)
        {
            //根据对应的资源查找所在资源组
            iRet = GetClusterGroupByResource(vecResources[i], strResourceGroup);
            if (MP_SUCCESS != iRet) 
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Group by resource failed");
                return iRet;
            }
            //获取虚拟网络资源名称
            iRet = GetVrlServerName(vecResources[i], strServerName);
            if (MP_SUCCESS != iRet) 
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vritual Server name by resource failed");
                return iRet;
            }

            break;
        }
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get SQLServer Cluster Resource Group");
    return iRet;
#endif   
}

/*--------------------------------------------------------------------------------------------------------
Function Name : CCluster::ExecPowershell
Description   : 执行powershell 命令;
Input         : const mp_string & strcmd,powershell 命令;
                const mp_string & filter, 过滤条件，比如我想获取名称，则输入name;
                mp_string strValue, 获取的信息;
Output        : mp_int32
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::ExecPowershell(mp_string & strCmd, const mp_string & strFilter, mp_string &strValue)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Exec Powershell command.");
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecReturn;
    iRet = CSystemExec::ExecSystemWithEcho(strCmd,vecReturn);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command failed.");
        return iRet;
    }

    mp_size lenReturn = vecReturn.size();
    for (mp_uint32 i = 0; i < lenReturn; ++i)
    {
        vector<mp_string> vecEcho;
        mp_string strTmp = vecReturn[i];
        (void)CMpString::StrSplit(vecEcho, strTmp, ':');
        if (!vecEcho.empty())
        {
            mp_string strTitle = vecEcho.front();
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "title:%s", strTitle.c_str());
            (mp_void)CMpString::Trim((mp_char*)strTitle.c_str());
            (mp_void)CMpString::TotallyTrimRight((mp_char*)strTitle.c_str());
            if (strcmp(strFilter.c_str(), strTitle.c_str()) == 0)
            {
                mp_string strValueTmp = vecEcho.back();
                (mp_void)CMpString::Trim((mp_char*)strValueTmp.c_str());
                (mp_void)CMpString::TotallyTrimRight((mp_char*)strValueTmp.c_str());
                mp_string strValueFin(strValueTmp, 0, strlen(strValueTmp.c_str()));
                strValue = strValueFin;
                COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get value:%s.", strValue.c_str());
                break;
            }
        }
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Exec Powershell command.");
    return iRet;
#endif
}

/*--------------------------------------------------------------------------------------------------------
Function Name : CCluster::ReplaceStr
Description   : 替换字符串;
Input         : const mp_string &oldStr, 输入串;
                mp_string &newStr, 输出串;
                const mp_string &old_value, 原始替换值;
                const mp_string &new_value, 替换值;
Output        : mp_string &newStr
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_void CCluster::ReplaceStr(const mp_string &oldStr, mp_string &newStr, const mp_string &old_value, 
    const mp_string &new_value)
{
    mp_size offset = 0;
    mp_size strLen = old_value.size();
    newStr.clear();

    mp_size idx = oldStr.find(old_value, offset);
    while (idx != string::npos)
    {
        newStr = newStr + oldStr.substr(offset, idx - offset);
        newStr = newStr + new_value;

        offset = idx + strLen;
        idx = oldStr.find(old_value, offset);
    }

    newStr = newStr + oldStr.substr(offset);
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::CutResourceName
Description  : 上线MSFC集群中的磁盘资源
Input        : mp_string strResourceName, 待上线的磁盘资源信息
                    内容如: "[磁盘资源名称];[盘符]"
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::CutResourceName(const mp_string &strResourceName, mp_string &strResName, mp_string &strDiskMountPoint, mp_string &strDiskSignature)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin CutResourceName.");
    vector<mp_string> vecTokens;
	//codedex误报CHECK_CONTAINER_EMPTY,容器vecTokens在之前的代码能保证不为空，此处可以不判断
    CMpString::StrSplit(vecTokens, strResourceName, ';');
    mp_size lenTokens = vecTokens.size();
    if (3 == lenTokens) //正常参数会有3个参数
    {
        strResName = vecTokens[0];
        strDiskMountPoint = vecTokens[1];
        strDiskSignature = vecTokens[2];
    }
    else
    {
        //如果传入资源不是正确的格式，则打印到日志，此处不做处理    
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "ResourceName input [%s] is not correct.", strResourceName.c_str());
        //Error:
        return ERROR_COMMON_INVALID_PARAM;  //参数错误的错误码
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetResourceStatus
Description  : 获取MSFC 资源状态
Input        : const mp_string & strResName, 检测的资源名称
                    mp_string & strStatus, 资源最终状态
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetResourceStatus(const mp_string & strResName, mp_string & strStatus)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get resource status");
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = MP_SUCCESS;
    mp_string strStatusTmp;
    mp_string strTitle = "State"; //状态标题
    
    mp_string strCmd = "cmd.exe /c powershell \"get-ClusterResource -Name \\\"";
    strCmd += strResName;
    strCmd += "\\\" | select State | fl * ";

    iRet = ExecPowershell(strCmd, strTitle, strStatusTmp);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get status failed, exec powershell command[%s] failed.", 
            strCmd.c_str());
        //Error:获取磁盘资源状态失败
        return ERROR_CLUSTER_PACKAGE_ONLINE_FAILED;
    }
    strStatus = strStatusTmp;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get resource status");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::WaitResourceStatusChanged
Description  : 等待磁盘资源状态变更，返回最终的状态信息
Input        : const mp_string & strResName, 检测的资源名称
                    mp_string & strStatus, 资源最终状态
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_void CCluster::WaitResourceStatusChanged(const mp_string & strResName, mp_string & strStatus)
{
    strStatus = mp_string(MSFC_RESOURCE_STATUS_ONLINE);
#ifdef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_string strStatusTmp;
    mp_size iFindPos = 0;
    while (mp_string::npos != iFindPos)
    {
        //获取资源状态
        iRet = GetResourceStatus(strResName, strStatusTmp);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get disk resource Status failed.");
            return;
        }
        // 正在上线/下线状态，此时为过程量，继续查询;
        iFindPos = strStatusTmp.find(mp_string(MSFC_RESOURCE_STATUS_PENDING));
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "get resource[%s] status is: %s.", strResName.c_str(), 
        strStatusTmp.c_str());
    strStatus = strStatusTmp;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::ResumeDiskResource
Description  : 将磁盘资源从维护状态转换为上线状态
Input        : const mp_string & strResName, 待恢复的磁盘资源名称
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::ResumeDiskResource(const mp_string & strResName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin resume disk resource.");
    mp_int32 iRet = MP_SUCCESS;
    
    mp_string strCmd = "cmd.exe /c powershell \"resume-clusterResource \\\"";
    strCmd += strResName;
    strCmd += "\\\" \" ";
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        //Error:恢复磁盘资源失败
        return ERROR_CLUSTER_RESUME_DISK_RESOURCE_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End resume disk resource.");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::ResetClusterDiskResource
Description  : 修复磁盘资源，重新指定磁盘资源指向的盘符
Input        : const mp_string & strResName, 资源名称
                const mp_string & strDiskMountPoint, 挂载点
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::ResetClusterDiskResource(const mp_string & strResName, const mp_string & strDiskMountPoint)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Reset disk resource.");
    mp_int32 iRet = MP_SUCCESS;
    
    mp_string strCmd = "cmd.exe /c powershell \"get-clusterResource \\\"";
    strCmd += strResName;
    strCmd += "\\\" | Set-ClusterParameter DiskPath  \\\"";
    strCmd += strDiskMountPoint;
    strCmd += ":\\\" \" ";

    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        //Error:修复磁盘资源失败
        return ERROR_CLUSTER_REPAIR_DISK_RESOURCE_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Reset disk resource.");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::onlineResource
Description  : 上线MSFC集群中的资源
Input        : const mp_string & strResName, 资源名称
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::onlineResource(const mp_string & strResName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin online cluster resource.");
    mp_int32 iRet = MP_SUCCESS;
    
    mp_string strCmd = "cmd.exe /c powershell \"Start-ClusterResource \\\"";
    strCmd += strResName;
    strCmd += "\\\" \" ";

    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        //Error:上线磁盘资源失败
        return ERROR_CLUSTER_ONLINE_DISK_RESOURCE_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End online cluster resource.");
    return MP_SUCCESS;
#endif

}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::onlineResource
Description  : 停止MSFC集群中的资源
Input        : const mp_string & strResName, 资源名称
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::SuspendResource(const mp_string &strResName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin suspend cluster resource.");
    mp_int32 iRet = MP_SUCCESS;
    
    mp_string strCmd = "cmd.exe /c powershell \"Suspend-ClusterResource \\\"";
    strCmd += strResName;
    strCmd += "\\\" \" ";


    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        //Error:挂起磁盘资源失败
        return ERROR_CLUSTER_SUSPEND_DISK_RESOURCE_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End suspend cluster resource.");
    return MP_SUCCESS;
#endif

}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::OnlineClusterDiskResource
Description  : 上线MSFC集群中的磁盘资源
Input        : mp_string strResourceName, 待上线的磁盘资源信息
                    内容如: "[磁盘资源名称];[盘符]"
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::OnlineClusterDiskResource(const mp_string &strResourceName, const mp_string &strResGrpName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin online disk resource.");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strResName;  //磁盘资源名称
    mp_string strDiskMountPoint; //磁盘挂载点
    mp_string strDiskSignature;
    mp_string strDiskPartitionType(DISK_TYPE_MBR);
    mp_string strSqlDependency;
    mp_string strWWN;
    mp_bool   IsResource = MP_FALSE;

    iRet = CutResourceName(strResourceName, strResName, strDiskMountPoint,strWWN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Cut disk resource Name failed.");
        return iRet;
    }

    iRet = GetSignatureByWWN(strWWN,strDiskSignature);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk Signature failed.");
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Disk Signature %s.",strDiskSignature.c_str());

    if(mp_string::npos != strDiskSignature.find("{"))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "strDiskPartitionType is GPT.");
        strDiskPartitionType = DISK_TYPE_GPT;
    }

    //资源名称是否存在
    iRet = IsClusterResource(strResName,IsResource);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Cluster resource failed.");
        return iRet;
    }

    iRet = IsDiskSignature(strDiskSignature,strDiskPartitionType);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "DiskSignature has one more or none.");
        return iRet;
    }

    iRet = DiskListSignature(strResName,strDiskSignature,strDiskPartitionType);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "DiskListSignature failed.");
        return iRet;
    }
    
    iRet = QueryDependencyResource(strResGrpName,strSqlDependency);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "QueryDependencyResource failed.");
        return iRet;
    }

    if(MP_FALSE == IsResource)
    {
        iRet = DiskAndMountPoint(strDiskSignature,strDiskMountPoint,strDiskPartitionType);
        if(MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "DiskAndMountPoint failed.");
            return iRet;
        }
        
        iRet = AddNewResourcetoCluster(strResName,strResGrpName,strDiskSignature,strDiskPartitionType,strDiskMountPoint);
        if(MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Add cluster resource failed.");
            return iRet;
        }

        iRet = AddClusterDependency(strResName,strSqlDependency);
        if(MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Add Cluster Dependency failed.");
            return iRet;
        }
        
        return iRet;
    }

    iRet = RemoveClusterDependency(strResName,strSqlDependency);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Remove cluster resource failed.");
        return iRet;
    }

    iRet = ClusterResourceReset(strResName,strResGrpName,strDiskSignature,strDiskMountPoint,strDiskPartitionType);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ClusterResourceReset Failed.");
        return iRet;
    }
    
    iRet = AddClusterDependency(strResName,strSqlDependency);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Add Cluster Dependency failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End online disk resource.");
    return iRet;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::SuspendClusterDiskResources
Description  : 挂起集群磁盘资源，进入维护状态
Input        : const vector<mp_string> & vecResourceName, 待挂起的磁盘资源信息
                    内容如: "[磁盘资源名称];[盘符]"
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::SuspendClusterDiskResources(const vector<mp_string> & vecResourceName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin suspend disk resource.");
    mp_int32 iRet = MP_SUCCESS;
    mp_size sizeResourceName = vecResourceName.size();
    for (mp_uint32 i = 0; i < sizeResourceName; ++i)
    {
        mp_string strResourceNameTmp = vecResourceName[i];
        mp_string strResName;
        mp_string strDiskPath;
        mp_string strDiskSignature;
        
        iRet = CutResourceName(strResourceNameTmp, strResName, strDiskPath, strDiskSignature);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Cut resource name failed.");
            return iRet;
        }
        
        iRet = SuspendResource(strResName);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "suspend cluster disk resource failed, resource name is: %s.", 
                strResName.c_str());
            //Error:挂起磁盘资源失败
            return iRet;
        }
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End suspend disk resource.");
    return iRet;
#endif

}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::OnlineClusterDiskResources
Description  : 上线MSFC集群中的磁盘资源
Input        : const vector<mp_string> & vecResourceName, 待上线的磁盘资源信息
                    内容如: "[磁盘资源名称];[盘符]"
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::OnlineClusterDiskResources(const vector<mp_string> & vecResourceName,const mp_string &strResGrpName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin online disk resource.");
    mp_int32 iRet = MP_SUCCESS;
    mp_size sizeResourceName = vecResourceName.size();
    for (mp_uint32 i = 0; i < sizeResourceName; ++i)
    {
        mp_string strResourceNameTmp = vecResourceName[i];
        iRet = OnlineClusterDiskResource(strResourceNameTmp,strResGrpName);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Online cluster disk resource failed, resource name is: %s.", 
                strResourceNameTmp.c_str());
            //Error:上线磁盘资源失败
            return iRet;
        }
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End online disk resource.");
    return iRet;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetSqlServerFilePath
Description  : 根据查询信息 获取sqlserver所占用的磁盘
Input        : db_info_t& stdbInfo, 数据库信息;
               vector<mp_string> vecDiskName, 磁盘名称列表;
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/

mp_int32 CCluster::GetSqlServerFilePath(vector<mp_string> &lstPath, const mp_string &strParam)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query sqlserver app info.");
    mp_string strParamTmp;
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strScriptName = SQLSERVER_SCRIPT_SQLSERVERADAPTIVE;

    LOGGUARD("");
    ReplaceStr(strParam, strParamTmp, "%", "%%");
    iRet = CSystemExec::ExecScript(strScriptName, strParamTmp, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        RETURN_MAP_ERRCODE(iRet, "get sqlserver lun path");
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Cann't find any disk.");
        //Error:获取sqlserver所在磁盘为空
        return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    }

    for (vector<string>::iterator iter = vecResult.begin();
        iter != vecResult.end();
        ++iter)
    {
        vector<mp_string> vecPathTmp;
        (void)CMpString::StrSplit(vecPathTmp, *iter, ';');
        for (vector<mp_string>::const_iterator iterPath = vecPathTmp.begin();
            iterPath != vecPathTmp.end();
            ++iterPath)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Find sqlserver path:%s", iterPath->c_str());
            lstPath.push_back(*iterPath);
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query sqlserver app info succ.");

    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetSQLServerClusterDisk
Description  : 获取sqlserver数据库所占用的磁盘
Input        : db_info_t& stdbInfo, 数据库信息;
                const mp_string & strVrlServerName, 虚拟服务器名称;
               vector<mp_string> vecDiskName, 磁盘名称列表;
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/

mp_int32 CCluster::GetSQLServerClusterDisk(db_info_t& stdbInfo, const mp_string & strVrlServerName, 
    vector<mp_string> & vecDiskName)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get SQLServer Cluster Disk");
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = MP_SUCCESS;
    //构造调用脚本的参数
    mp_string strParam = mp_string(SCRIPTPARAM_DBNAME) + stdbInfo.strdbName + mp_string(NODE_SEMICOLON)
        + mp_string(SCRIPTPARAM_INSTNAME) + stdbInfo.strinstName  + mp_string(NODE_SEMICOLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbInfo.strdbUsername + mp_string(NODE_SEMICOLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbInfo.strdbPassword + mp_string(NODE_SEMICOLON)
        + mp_string(SQLSERVER_SCRIPTPARAM_CLUSTERFLAG) + mp_string(BE_CLUSTER_SIGN) + mp_string(NODE_SEMICOLON) 
        + mp_string(SCRIPTPARAM_CLUSTERNAME) + strVrlServerName;

    iRet = GetSqlServerFilePath(vecDiskName, strParam);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Sql Server File path faild.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get SQLServer Cluster Disk succ.");
    return iRet;
#endif
}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetDiskNum
Description  : 获取磁盘序号
Input        : const mp_string &strDiskName, 磁盘盘符名称;
               mp_string &strDiskNumber, 磁盘序号;
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetDiskNum(const mp_string &strDiskName, mp_string &strDiskNumber)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get Disk Number");
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd = "cmd.exe /c powershell \"get-partition | ?{$_.DriveLetter -eq \\\"";
    strCmd += strDiskName;
    strCmd += "\\\"} | select DiskNumber | fl * ";
    mp_string strFilter = "DiskNumber";
    //获取磁盘序号
    iRet = ExecPowershell(strCmd, strFilter, strDiskNumber);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get disk number failed, exec powershell command[%s] failed.", 
            strCmd.c_str());
        //Error:获取磁盘序号失败
        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get Disk Number");
    return iRet;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetDiskSignature
Description  : 根据磁盘序号获取磁盘签名以及磁盘类型
Input        : mp_string &strDiskSignature, 磁盘信号量;
               mp_string &strDiskNumber, 磁盘序号;
               mp_string &strDiskPatitionType, 磁盘类型(如: MBR)
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetDiskSignature(const mp_string &strDiskNumber, mp_string &strDiskSignature, 
    mp_string &strDiskPatitionType)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get Disk Signature");
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmdtmp, strCmd;
    mp_string strSignatureFilter = DISK_ID_GPT;
    mp_string strPartitionStyleFilter = "PartitionStyle";

    strCmdtmp = "cmd.exe /c powershell \"get-disk " + strDiskNumber;
    strCmd = strCmdtmp + " | select PartitionStyle | fl * ";

    //获取磁盘分区类型
    iRet = ExecPowershell(strCmd, strPartitionStyleFilter, strDiskPatitionType);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get disk partition failed, exec powershell command[%s] failed.", 
            strCmd.c_str());
        //Error:
        return ERROR_CLUSTER_GET_DISK_PATITION_TYPE_FAILED;
    }

    if (DISK_TYPE_MBR == strDiskPatitionType)
    {
        strSignatureFilter = "Signature";
    }

    //获取磁盘标签
    strCmd = strCmdtmp + " | select " + strSignatureFilter + " | fl * ";
    iRet = ExecPowershell(strCmd, strSignatureFilter, strDiskSignature);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get disk sig failed, exec powershell command[%s] failed.", 
            strCmd.c_str());
        //Error:获取磁盘的标签失败
        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Disk Signature:%s", strDiskSignature.c_str());
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get Disk Signature");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetDiskResBySignature
Description  : 通过Signature 获取磁盘资源
Input        : mp_string &strDiskSignature, 磁盘信号量;
               mp_string &strResource, 资源名称;
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetDiskResBySignature(const mp_string &strDiskSignature, const mp_string &strDiskPartitionType, mp_string &strResource)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get Disk Resource by Signature");
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else

    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd;
    mp_string strFilter = "ClusterObject";
    if (DISK_TYPE_MBR == strDiskPartitionType)
    {
        //如果分区类型为MBR ， 则磁盘标签为16进制的数字
        mp_int64 iSignature = 0;
        mp_char acuSignature[256] = {0};

        mp_string strDiskSigTmp = CMpString::Trim((mp_char*)strDiskSignature.c_str());
        iSignature = _atoi64(strDiskSigTmp.c_str());
        iRet = sprintf_s(acuSignature, sizeof(acuSignature), "%llx", iSignature);
        if (MP_FAILED == iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call sprintf_s Failed, ret %d.", iRet);
            //Error:获取磁盘资源失败
            return ERROR_CLUSTER_GET_DISK_RESOURCE_FAILED;
        }
        mp_string strDiskSignatureTmp = "0x" + mp_string(acuSignature);

        strCmd = "cmd.exe /c powershell \"get-clusterResource | get-clusterParameter | ? {$_.Name -eq \\\"DiskSignature\\\"} ";
        strCmd += "| ? {$_.Value -eq \\\"";
        strCmd += strDiskSignatureTmp;
        strCmd += "\\\"} | select ClusterObject |fl * ";
    }
    else
    {
        strCmd = "cmd.exe /c powershell \"get-clusterResource | get-clusterParameter | ? {$_.Name -eq \\\"DiskIdGuid\\\"} ";
        strCmd += "| ? {$_.Value -eq \\\"";
        strCmd += strDiskSignature;
        strCmd += "\\\"} | select ClusterObject |fl * ";
    }


    iRet = ExecPowershell(strCmd, strFilter, strResource);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get resource name by disksig failed, exec powershell command[%s] failed.", 
            strCmd.c_str());
        //Error:获取磁盘资源失败
        return ERROR_CLUSTER_GET_DISK_RESOURCE_FAILED;
    }
      
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get Disk Resource by Signature");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::GetSQLServerClusterResource
Description  : 根据磁盘盘符获取资源列表
Input        : const mp_string &strDiskName, 盘符(如: "D" "E")
               mp_string &strResource, 资源名称;
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::GetSQLServerClusterResource(const mp_string &strDiskName, mp_string &strResource)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Get Cluster Resource");
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDiskNumber;
    mp_string strDiskSignature;
    mp_string strDiskPartitionType;

    //根据盘符获取当前卷的序号
    iRet = GetDiskNum(strDiskName, strDiskNumber);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk Number failed.");
        return iRet;
    }

    //根据序号获取磁盘的signature
    iRet = GetDiskSignature(strDiskNumber, strDiskSignature, strDiskPartitionType);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk Signature failed.");
        return iRet;
    }

    //根据 signature 获取资源名称
    iRet = GetDiskResBySignature(strDiskSignature, strDiskPartitionType, strResource);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk Resource by Signature failed.");
        return iRet;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get Cluster Resource");
    return iRet;    
#endif
}


/*------------------------------------------------------------------------------------------------------------
Function Name: CCluster::QuerySQLServerClusterInfo
Description  : 获取集群信息;
Input        : db_info_t& stdbInfo, 数据库信息;
               mp_string strClusterType, 集群类型;
               vector<cluster_info_t>& vecClusterInfo, 集群信息;
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CCluster::QuerySQLServerClusterInfo(db_info_t& stdbInfo, mp_string strClusterType, 
    vector<cluster_info_t>& vecClusterInfo)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    mp_int32 iRet = MP_FAILED;
    mp_string strWWN;
    cluster_info_t clusterInfo;
    sqlserver_info_t stdbluninfo;
    vector<sqlserver_lun_info_t> vecLunInfos;
    CSqlServer SqlServer;

    stdbluninfo.strInstName = stdbInfo.strinstName;
    stdbluninfo.strDBName   = stdbInfo.strdbName;
    stdbluninfo.strUser = stdbInfo.strdbUsername;
    stdbluninfo.strPasswd = stdbInfo.strdbPassword;
    stdbluninfo.strIsCluster = "1";
    iRet = SqlServer.GetLunInfo(stdbluninfo,vecLunInfos);
    // 获取集群名称;
    mp_string strClusterName;
    iRet = GetSQLServerClusterName(strClusterName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SQLServer Cluster Name Failed");
        return iRet;
    }
    clusterInfo.strClusterName = strClusterName;
    
    // 根据数据库实例查找资源组;
    mp_string strResourceGroup;
    mp_string strVrlServerName;
    iRet = GetClusterGroupByinstName(stdbInfo.strinstName, strResourceGroup, strVrlServerName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SQLServer Cluster Group Failed");
        return iRet;
    }
    clusterInfo.strResGrpName = strResourceGroup;  //资源组名称
    clusterInfo.strNetWorkName = strVrlServerName; //网络资源名称

    mp_bool bActiveNode = IsSQLServerActiveNode(strResourceGroup);
    //如果传入的数据库为空，则认为不需要查询资源列表
    if (("" == stdbInfo.strdbName)||(!bActiveNode))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "db name is null or not activeNode, no need to get cluster resource");
        vecClusterInfo.push_back(clusterInfo);
        return MP_SUCCESS;
    }
    // 查找数据库所在磁盘的盘符;
    vector<mp_string> vecDiskName; 
    iRet = GetSQLServerClusterDisk(stdbInfo, strVrlServerName, vecDiskName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SQLServer Cluster Disk Failed");
        return iRet;
    }

    // 根据盘符查找资源列表;
    mp_string strResourceList = "";
    mp_size lenDiskName = vecDiskName.size();
    for (mp_uint32 i = 0; i < lenDiskName; ++i)
    {
        mp_string strDiskName = vecDiskName[i];
        mp_string strDiskResource;

        iRet = GetSQLServerClusterResource(strDiskName, strDiskResource);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk Signature failed.");
            return iRet;
        }

        for(vector<sqlserver_lun_info_t>::iterator iter = vecLunInfos.begin();iter != vecLunInfos.end();++iter)
        {
            if(strDiskName == (*iter).strDeviceName)
            {
                strWWN = (*iter).strWWN;
            }
        }
        //返回的信息前面是资源名称 后面是对应的盘符
        strDiskResource += mp_string(STR_SEMICOLON) + strDiskName + mp_string(STR_SEMICOLON) + strWWN;
        clusterInfo.vecResourceName.push_back(strDiskResource);
    }
    //将数据插入到集群资源列表
    vecClusterInfo.push_back(clusterInfo);
    return iRet;
#endif
}
/*------------------------------------------------------------ 
Description  :启动CLvm
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::StartCLvm()
{
#ifndef WIN32    
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd = "cat /etc/lvm/lvm.conf | grep locking_type | grep -v \"#\"";
    mp_string strParam;
    vector<mp_string> vecRlt;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin start clvm.");
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (MP_SUCCESS == iRet && vecRlt.size() == 1 && vecRlt.front().find("3") != mp_string::npos)
    {
        strParam = "clvmd start";
        strCmd = "ps -ef | grep clvmd | grep -v grep";
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RHCS_SERVICE, strParam, NULL);
        mp_int32 iRet1 = CSystemExec::ExecSystemWithoutEcho(strCmd);
        if (MP_SUCCESS != iRet && MP_SUCCESS != iRet1)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "service %s failed, iRet = %d, iRet1 = %d", 
                  strParam.c_str(), iRet, iRet1);
            return ERROR_CLUSTER_START_SERVICE_FAILED;
        }
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Start clvm succ.");
#endif
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :启动RHCS集群
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::StartRHCSCluster()
{
#ifndef WIN32
    vector<mp_string> vecResult;
    mp_string strLocalHostName;
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsStart = MP_FALSE;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start RHCS cluster.");
    //获取本机名    
    iRet = GetHostName(strLocalHostName);
    if (MP_SUCCESS != iRet)
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host name failed, iRet %d.", iRet);
         return ERROR_CLUSTER_START_SERVICE_FAILED;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The local host name is %s.", strLocalHostName.c_str());
   
    iRet = GetRHCSClusterState(strLocalHostName, bIsStart);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get RHCS cluster state failed, iRet %d.", iRet);
        return ERROR_CLUSTER_START_SERVICE_FAILED;
    }

    if (bIsStart)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Cluster service has been started.");
        return MP_SUCCESS;
    }

    iRet = StartRHCSService();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start RHCS service failed, iRet = %d", iRet);
        return iRet;
    }

    //检查集群状态
    iRet = WaitRHCSClusterStart(strLocalHostName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start RHCS cluster failed or timeout.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start RHCS cluster succ.");
#endif
    return MP_SUCCESS;
}

#ifdef WIN32
/*------------------------------------------------------------ 
Description  :添加新资源到集群
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::AddNewResource(const mp_string & strResName, const mp_string & strResourceGroup)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin add cluster resource.");
    mp_int32 iRet = MP_SUCCESS;
    
    mp_string strCmd = "cmd.exe /c powershell \"Add-ClusterResource -name \\\"";
    strCmd += strResName;
    strCmd += "\\\" -ResourceType \\\"Physical disk\\\" -Group \\\"";
    strCmd += strResourceGroup;
    strCmd += "\\\"\"";
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End add cluster resource.");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------ 
Description  :设置资源标签
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::SetClusterSignature(const mp_string & strResName, const mp_string & strDiskSignature, const mp_string &strDiskPartitionType)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin set cluster resource signature.");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDiskType("DiskSignature");

    if(DISK_TYPE_GPT == strDiskPartitionType)
    {
        strDiskType = "DiskIdGuid";
    }
    
    mp_string strCmd = "cmd.exe /c powershell \"Get-ClusterResource \\\"";
    strCmd += strResName;
    strCmd += "\\\" | set-clusterparameter ";
    strCmd += strDiskType;
    strCmd += " \\\"";
    strCmd += strDiskSignature;
    strCmd += "\\\"\"";
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End set cluster resource signature.");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------ 
Description  :添加资源依赖
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::AddClusterDependency(const mp_string & strResName,mp_string &strSqlDependency)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin add cluster resource dependency.");
    mp_int32 iRet = MP_SUCCESS;
    
    mp_string strCmd = "cmd.exe /c powershell \"Add-ClusterResourceDependency \\\"";
    strCmd += strSqlDependency;
    strCmd += "\\\" \\\"";
    strCmd += strResName;
    strCmd += "\\\"\"";
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End add cluster resource dependency.");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------ 
Description  :移除资源依赖
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::RemoveClusterDependency(const mp_string & strResName,mp_string &strSqlDependency)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin remove cluster resource dependency.");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bExist = MP_FALSE;
    
    mp_string strCmd = "cmd.exe /c powershell \"Remove-ClusterResourceDependency \\\"";
    strCmd += strSqlDependency;
    strCmd += "\\\" \\\"";
    strCmd += strResName;
    strCmd += "\\\"\"";
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        iRet = IsClusterDependency(strResName,strSqlDependency, bExist);
        if (MP_FAILED == iRet )
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
            return MP_FAILED;
        }

        if (MP_FALSE == bExist)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The Resource has no Dependency.");
            return MP_SUCCESS;
        }
        else
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The Resource has Dependency.");
            return MP_FAILED; 
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End remove cluster resource dependency.");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------ 
Description  :移除资源
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::RemoveClusterResource(const mp_string & strResName)
{
#ifndef WIN32
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin remove cluster resource.");
    mp_int32 iRet = MP_SUCCESS;
    
    mp_string strCmd = "cmd.exe /c powershell \"Remove-ClusterResource \\\"";
    strCmd += strResName;
    strCmd += "\\\" -force\"";

    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End remove cluster resource.");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------ 
Description  :判断资源是否存在
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::IsClusterResource(const mp_string & strResName,mp_bool &IsResource)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query resource.");
    mp_int32 iRet = MP_SUCCESS;
    mp_size first = 0;
    mp_string str1;
    IsResource = MP_FALSE;
    
    mp_string strCmd = "cmd.exe /c powershell \"get-clusterResource | select Name | fl *\"";
    vector<mp_string> vecEcho;
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec Powershell command [%s] failed.", strCmd.c_str());
        return MP_FAILED;
    }

    for(vector<mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if((mp_string::npos != first) && (strResName == str1.substr(first + 2, str1.size())))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "ClusterResource is alive.");
            IsResource = MP_TRUE;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End query resource.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :添加新资源到集群，并上线
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::AddNewResourcetoCluster(const mp_string &strResourceName,const mp_string &strResGrpName,const mp_string &strDiskSignature,const mp_string &strDiskPartitionType,const mp_string &strDiskMountPoint)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strStatus;
    mp_string strDiskNum;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to add new resource to Cluster.");
    
    iRet = AddNewResource(strResourceName, strResGrpName);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Add New Resource failed.");
        return iRet;
    }
    
    iRet = SetClusterSignature(strResourceName, strDiskSignature, strDiskPartitionType);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set Cluster Signature failed.");
        return iRet;
    }
    
    iRet = onlineResource(strResourceName);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "online Resource failed.");
        return iRet;
    }

    WaitUntilOnline(strResourceName,strStatus);
    if (mp_string(MSFC_RESOURCE_STATUS_ONLINE) != strStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disk resource status is offline.");
        return MP_FAILED;
    }
    
    return iRet;
}

/*------------------------------------------------------------ 
Description  :等待资源上线
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CCluster::WaitUntilOnline(const mp_string &strResName, mp_string &strStatus)
{
    mp_int32 iRet = MP_SUCCESS;
    
    WaitResourceStatusChanged(strResName, strStatus);
    if (mp_string(MSFC_RESOURCE_STATUS_ONLINE) == strStatus)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Reset disk resouce success, status is: %s", strStatus.c_str());
        return;
    }
    //如果没有上线，再重新执行一次上线操作
    //此处由于修复磁盘资源后会有一个状态转换过程
    //可能存在上线命令失败的情况，故此处设置一个重试5次的操作
    for (mp_uint32 i = 0; i < 5; ++i)
    {
        iRet = onlineResource(strResName);
        if (MP_SUCCESS == iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "online Resource success, times is: %d.", i);
            break;
        }
        DoSleep(1000); //如果上线命令失败，说明此时状态不对，等待1秒再次重试
    }
    
    WaitResourceStatusChanged(strResName, strStatus);
}
/*------------------------------------------------------------ 
Description  :查看资源是否正常在线
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::ResourceListSignature(const mp_string &strResName, const mp_string &strDiskSignature)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query resource.");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strResDisknature;
    mp_string strDiskType(DISK_ID_MBR);
    mp_string str1;
    mp_size  first = 0;
    vector<mp_string> vecEcho;
    
    mp_string strCmd = "cmd.exe /c powershell \"get-clusterResource \\\"";
    strCmd += strResName;
    strCmd += "\\\" | Get-ClusterParameter | ? {$_.Name -eq \\\"DiskSignature\\\" -or $_.Name -eq \\\"DiskIdGuid\\\"} | select value | fl *\"";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get ClusterResource Signature failed, exec powershell command[%s] failed.", 
            strCmd.c_str());

        return MP_FAILED;
    }
    for(vector<mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if(mp_string::npos == first)
        {
            continue;
        }

        if(mp_string::npos != str1.find("{"))
        {
            strResDisknature = str1.substr(first + 2, str1.size());
            strDiskType = DISK_ID_GPT;
        }
        
        if(mp_string::npos != str1.find("0x"))
        {
            StringHexToDec(str1.substr(first + 4, str1.size()),strResDisknature);
        }

        if(strDiskSignature == strResDisknature)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "ClusterResource Signature is equal.");
            return iRet;
        }
    }

    strCmd = "cmd.exe /c powershell \"Get-Disk | select ";
    strCmd += strDiskType;
    strCmd += " | fl *\"";
    vector<mp_string> vecEcho1;
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get ClusterResource Signature failed, exec powershell command[%s] failed.", 
            strCmd.c_str());

        return MP_FAILED;
    }

    for(vector<mp_string>::iterator iter = vecEcho1.begin();iter != vecEcho1.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if((mp_string::npos != first) && (strResDisknature == str1.substr(first + 2, str1.size())))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ClusterResource is used by Signature[%s].", str1.c_str());
            return MP_FAILED;
        }
    }
    
    return iRet;
}
/*------------------------------------------------------------ 
Description  :查看磁盘是否与正常在线的资源对应
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::DiskListSignature(const mp_string &strResName,const mp_string &strDiskSignature,const mp_string strDiskPartitionType)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query clusterResources's signature.");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strResDisknature;
    mp_string strResource;
    mp_string str1;
    mp_size  first = 0;
    vector<mp_string> vecEcho;
    
    mp_string strCmd = "cmd.exe /c powershell \"get-clusterResource ";
    strCmd += "| Get-ClusterParameter | ? {$_.Name -eq \\\"DiskSignature\\\" -or $_.Name -eq \\\"DiskIdGuid\\\"} | select value | fl *\"";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get ClusterResource Signature failed, exec powershell command[%s] failed.", 
            strCmd.c_str());

        return MP_FAILED;
    }
    for(vector<mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if(mp_string::npos == first)
        {
            continue;
        }

        if(mp_string::npos != str1.find("{"))
        {
            strResDisknature = str1.substr(first + 2, str1.size());
        }
        
        if(mp_string::npos != str1.find("0x"))
        {
            StringHexToDec(str1.substr(first + 4, str1.size()),strResDisknature);
        }

        if(strDiskSignature == strResDisknature)
        {
            iRet = GetDiskResBySignature(strDiskSignature,strDiskPartitionType,strResource);
            if(MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDiskResBySignature failed.");
                return MP_FAILED;
            }
            
            if(strResource != strResName)
            {
                COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Signature is used by ClusterResource[%s].",strResDisknature.c_str());
                return MP_FAILED;
            }
        }
    }
    
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :string类型十六进制转换为十进制
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CCluster::StringHexToDec(const mp_string &str,mp_string &strout)
{
    mp_int32 sz = 0;
    mp_uint64 st = 0;
    ostringstream fd;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get str is [%s].", str.c_str());
    for(sz = 0;sz < str.size();sz++)
    {
        if(str[sz]>='a' && str[sz]<='z')
        {
            st = st*16 + str[sz]-'a'+10;
        }
        else if(str[sz]>='A' && str[sz]<='Z')
        {
            st = st*16 + str[sz]-'A'+10;
        }
        else
        {
            st = st*16 + str[sz]-'0';
        }
    }

    fd << st;
    strout = fd.str();
}
/*------------------------------------------------------------ 
Description  :集群资源复位
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::ClusterResourceReset(const mp_string &strResName, const mp_string &strResGrpName,const mp_string &strDiskSignature, const mp_string &strDiskMountPoint, const mp_string &strDiskPartitionType)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strStatus = mp_string(MSFC_RESOURCE_STATUS_OFFLINE);//磁盘资源状态信息
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin ClusterResourceReset.");
    iRet = ResourceListSignature(strResName,strDiskSignature);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "List cluster resource signature failed.");
        return iRet;
    }
      
    iRet = ResumeDiskResource(strResName);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Resume disk resource failed, do not return, countinue to repair itself."); 
    }
    else
    {
        WaitResourceStatusChanged(strResName, strStatus);
        if (mp_string(MSFC_RESOURCE_STATUS_ONLINE) == strStatus)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Disk resource success online.");
            return MP_SUCCESS;
        }
    }

    iRet = ResetResourceBySignature(strResName,strDiskSignature,strDiskPartitionType);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "RemoveClusterResource failed.");
        return iRet;
    }

    WaitUntilOnline(strResName,strStatus);
    if (mp_string(MSFC_RESOURCE_STATUS_ONLINE) == strStatus)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Disk resource success online.");
        return MP_SUCCESS;
    }
    
    iRet = RemoveClusterResource(strResName);;
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "RemoveClusterResource failed.");
        return iRet;
    }
    
    iRet = DiskAndMountPoint(strDiskSignature,strDiskMountPoint,strDiskPartitionType);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "DiskAndMountPoint failed.");
        return iRet;
    }

    iRet = AddNewResourcetoCluster(strResName,strResGrpName,strDiskSignature,strDiskPartitionType,strDiskMountPoint);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Add Disk Cluster Resource failed.");
        return iRet;
    }

    return iRet;
}
/*------------------------------------------------------------ 
Description  :上线磁盘并设置盘符
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::DiskAndMountPoint(const mp_string &strDiskSignature,const mp_string &strDiskMountPoint,const mp_string &strDiskPartitionType)
{
    mp_int32 iRet = MP_SUCCESS;
    mount_info_t mountInfo;
    umount_info_t umountInfo;
    file_sys_info_t file_sys;
    CHost m_Host;
    CFileSys m_FileSys;
    mp_string strDiskNum;
    vector<file_sys_info_t> vecFileInfos;

    iRet = GetDiskNumBySignature(strDiskSignature,strDiskPartitionType,strDiskNum);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk Num failed.strDiskNum[%s]",strDiskNum.c_str());
        return iRet;
    }
    
    iRet = m_Host.DeviceOnline(strDiskNum);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Online Disk failed.");
        return iRet;
    }
    
    m_FileSys.QueryFileSysInfo(vecFileInfos);
    for(vector<file_sys_info_t>::iterator iter = vecFileInfos.begin();iter != vecFileInfos.end();++iter)
    {
        if(atoi(strDiskNum.c_str()) == (*iter).diskNumber)
        {
            mountInfo.deviceName = (*iter).deviceName;
            file_sys = *iter;
            break;
        }
    }
    
    if(("" != mountInfo.deviceName) && (strDiskMountPoint != file_sys.mountpoint))
    {
        umountInfo.deviceName = mountInfo.deviceName;
        umountInfo.mountPoint = file_sys.mountpoint;
        umountInfo.volumeType = 0;
        iRet = m_FileSys.UMount(umountInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unmount point failed.");
            return MP_FAILED;
        }
    }
    
    mountInfo.mountPoint = strDiskMountPoint;
    mountInfo.volumeType = 0;
    iRet = m_FileSys.Mount(mountInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set volume mount point failed.");
        return ERROR_DEVICE_FILESYS_MOUNT_FAILED;
    }

    return iRet;
}
/*------------------------------------------------------------ 
Description  :通过标签获取磁盘号
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::GetDiskNumBySignature(const mp_string &strDiskSignature,const mp_string &strDiskPartitionType,mp_string & strDiskNum)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd;
    mp_string strDiskType(DISK_ID_MBR);
    vector<mp_string> vecEcho;
    mp_string str1;
    mp_size  first = 0;
    mp_string strResDisknature;

    if(DISK_TYPE_GPT == strDiskPartitionType)
    {
        strDiskType = DISK_ID_GPT;
    }

    strCmd = "cmd.exe /c powershell \"Get-Disk | select ";
    strCmd += strDiskType;
    strCmd += ",Number | fl *\"";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get ClusterResource Signature failed, exec powershell command[%s] failed.", 
            strCmd.c_str());

        return MP_FAILED;
    }

    for(vector<mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if(mp_string::npos == first)
        {
            continue;
        }

        strResDisknature = str1.substr(first + 2, str1.size());

        if(strDiskSignature == strResDisknature)
        {
            ++iter;
            str1 = *iter;
            first = str1.find(':');
            if(mp_string::npos == first)
            {
                ++iter;
                str1 = *iter;
                first = str1.find(':');
            }
            strDiskNum = str1.substr(first + 2, str1.size());
            return MP_SUCCESS;
        }
    }
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDiskNumBySignature failed strDiskNum-%s.",strDiskNum.c_str());
    return MP_FAILED;
}
/*------------------------------------------------------------ 
Description  :通过标签复位资源
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::ResetResourceBySignature(const mp_string &strResName,const mp_string &strDiskSignature,const mp_string & strDiskPartitionType)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strStatus;
    
    iRet = SetClusterSignature(strResName, strDiskSignature, strDiskPartitionType);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set Cluster Signature failed.");
        return iRet;
    }
    
    iRet = onlineResource(strResName);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "online Resource failed.strResName[%s]",strResName.c_str());
        return iRet;
    }

    return iRet;
}
/*------------------------------------------------------------ 
Description  :查询资源依赖
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::QueryDependencyResource(const mp_string & strResourceGroup,mp_string &strSqlDependency)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string str1;
    mp_size  first = 0;
    vector<mp_string> vecEcho;
    
    mp_string strCmd = "cmd.exe /c powershell \"Get-ClusterResource | select Name,OwnerGroup,ResourceType | ?{$_.OwnerGroup -eq \\\"";
    strCmd += strResourceGroup;
    strCmd += "\\\"} | ?{$_.ResourceType -eq \\\"SQL Server\\\"} | select Name | fl *";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get ClusterResource Signature failed, exec powershell command[%s] failed.", 
            strCmd.c_str());

        return MP_FAILED;
    }
    for(vector<mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if(mp_string::npos == first)
        {
            continue;
        }

        strSqlDependency = str1.substr(first + 2, str1.size());
        if("" == strSqlDependency)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "QueryDependencyResource failed.");
            return MP_FAILED;
        }
    }
    
    return MP_SUCCESS;   
}

/*------------------------------------------------------------ 
Description  :通过WWN获取磁盘signature
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::GetSignatureByWWN(const mp_string &strWWN,mp_string &strDiskSignature)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string str1;
    mp_string strGetWWN;
    mp_string strLowerWWN;
    mp_size  first = 0;
    vector<mp_string> vecEcho;

    mp_string strCmd = "cmd.exe /c powershell \"Get-Disk | select UniqueId,Guid,Signature | ?{$_.UniqueId -eq \\\"";
    strCmd += strWWN;
    strCmd += "\\\"} | select Guid,Signature | fl *\"";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get ClusterResource Signature failed, exec powershell command[%s] failed.", 
            strCmd.c_str());

        return MP_FAILED;
    }
    
    for(vector<mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if(mp_string::npos == first)
        {
            continue;
        }
            
        strDiskSignature = str1.substr(first + 2, str1.size());
        if("" != strDiskSignature)
        {
            return MP_SUCCESS;
        }
    }
    return MP_FAILED;
}

/*------------------------------------------------------------ 
Description  :判断磁盘标签是否存在，且是唯一的
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::IsDiskSignature(const mp_string &strDiskSignature,const mp_string &strDiskPartitionType)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iNum = 0;
    mp_string str1("");
    mp_size  first = 0;
    vector<mp_string> vecEcho;
    mp_string strDiskType(DISK_ID_MBR);

    if(DISK_TYPE_GPT == strDiskPartitionType)
    {
        strDiskType = DISK_ID_GPT;
    }
    
    mp_string strCmd = "cmd.exe /c powershell \"Get-Disk | select ";
    strCmd += strDiskType;
    strCmd += " | fl *\"";

    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get ClusterResource Signature failed, exec powershell command[%s] failed.", 
            strCmd.c_str());

        return MP_FAILED;
    }

    for(vector<mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if((mp_string::npos != first) && (strDiskSignature == str1.substr(first + 2, str1.size())))
        {
            iNum++;
        }
    }
    
    if(1 == iNum)
    {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

/*------------------------------------------------------------ 
Description  :查询资源是否存在于依赖中
Input        :     
Output       :   
Return       :   MP_SUCCESS---获取成功
                 非MP_SUCCESS---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CCluster::IsClusterDependency(const mp_string & strResName,const mp_string &strSqlDependency, mp_bool &bExist)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string str1;
    mp_string str = "[" + strResName + "]";
    mp_size first = 0;
    vector<mp_string> vecEcho;

    bExist = MP_FALSE;
    mp_string strCmd = "cmd.exe /c powershell \"Get-ClusterResourceDependency \\\"";
    strCmd += strSqlDependency;
    strCmd += "\\\" | fl *\"";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecEcho);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get ClusterResource Dependency failed, exec powershell command[%s] failed.", 
            strCmd.c_str());

        return MP_FAILED;
    }
    for(vector<mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        str1 = *iter;
        first = str1.find(':');
        if(mp_string::npos == first)
        {
            continue;
        }

        if(string::npos != str1.find(str))
        {
            bExist = MP_TRUE;
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Success.");
            
            return MP_SUCCESS;
        }
    }

    return MP_SUCCESS;   
}

#endif

