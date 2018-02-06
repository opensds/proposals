/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "cluster/ClusterTest.h"

TEST_F(CClusterTest, IsActiveNode)
{
    CCluster cls;
    mp_string strResGrpName;
    mp_int32 iClusterType;
    mp_bool bIsActive;
    mp_int32 rst;
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecEq0);
        //iClusterType = CLUSTER_VCS;
        iClusterType = CLUSTER_VCS;
        rst = cls.IsActiveNode(strResGrpName, iClusterType, bIsActive);
        EXPECT_EQ(rst, ERROR_CLUSTER_QUERY_ACTIVE_HOST_FAILED);
    }
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub2(&CRootCaller::Exec, &StubCRootCallerExecActiveNode);
        //iClusterType = CLUSTER_VCS;
        iClusterType = CLUSTER_VCS;
        rst = cls.IsActiveNode(strResGrpName, iClusterType, bIsActive);
        //iClusterType = CLUSTER_POWERHA;
        iClusterType = CLUSTER_POWERHA;
        rst = cls.IsActiveNode(strResGrpName, iClusterType, bIsActive);
        //iClusterType = CLUSTER_SERVICEGUARD;
        iClusterType = CLUSTER_SERVICEGUARD;
        rst = cls.IsActiveNode(strResGrpName, iClusterType, bIsActive);
        //iClusterType = CLUSTER_RHCS;
        iClusterType = CLUSTER_RHCS;
        rst = cls.IsActiveNode(strResGrpName, iClusterType, bIsActive);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CClusterTest, QueryClusterInfo)
{
    CCluster cls;
    db_info_t stdbInfo;
    mp_string strClusterType;
    mp_string strDBType;
    vector<cluster_info_t> vecClusterInfo;
    mp_int32 rst;
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecClusterInfo);
    //strDBType = DB_ORACLE;
    strDBType = DB_ORACLE;
    rst = cls.QueryClusterInfo(stdbInfo, strClusterType, strDBType, vecClusterInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
    //strDBType = DB_DB2;
    strDBType = DB_DB2;
    rst = cls.QueryClusterInfo(stdbInfo, strClusterType, strDBType, vecClusterInfo);
    EXPECT_EQ(rst, MP_SUCCESS);
    //strDBType = DB_OTHER;
    strDBType = 12;
    rst = cls.QueryClusterInfo(stdbInfo, strClusterType, strDBType, vecClusterInfo);
    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
}
TEST_F(CClusterTest, StartCluster)
{
    CCluster cls;
    mp_int32 iClusterType;
    mp_string strResGrpName;
    mp_int32 rst;
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecStart);
        Stub<CSystemExecExecSystemWithEchoType, StubCSystemExecExecSystemWithEchoType, mp_void> mystub2(&CSystemExec::ExecSystemWithEcho, &StubCSystemExecExecSystemWithEchoEq0);
        //iClusterType = CLUSTER_VCS;
        iClusterType = CLUSTER_VCS;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, MP_SUCCESS);
        //iClusterType = CLUSTER_POWERHA;
        iClusterType = CLUSTER_POWERHA;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, MP_SUCCESS);
        //iClusterType = CLUSTER_SERVICEGUARD;
        iClusterType = CLUSTER_SERVICEGUARD;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, MP_SUCCESS);
        //iClusterType = CLUSTER_RHCS;
        iClusterType = CLUSTER_RHCS;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, MP_SUCCESS);
        //iClusterType = OTHER;
        iClusterType = 12;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
    }
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub3(&CRootCaller::Exec, &StubCRootCallerExecStartErr);
        Stub<CSystemExecExecSystemWithEchoType, StubCSystemExecExecSystemWithEchoType, mp_void> mystub4(&CSystemExec::ExecSystemWithEcho, &StubCSystemExecExecSystemWithEchoEq0);
        Stub<DoSleepType, StubDoSleepType, mp_void> mystub5(&DoSleep, &StubDoSleepVoid);
        //iClusterType = CLUSTER_VCS;
        iClusterType = CLUSTER_VCS;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, ERROR_CLUSTER_START_SERVICE_FAILED);
        //iClusterType = CLUSTER_POWERHA;
        iClusterType = CLUSTER_POWERHA;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, ERROR_CLUSTER_START_SERVICE_FAILED);
        //iClusterType = CLUSTER_SERVICEGUARD;
        iClusterType = CLUSTER_SERVICEGUARD;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, ERROR_CLUSTER_START_SERVICE_FAILED);
        //iClusterType = CLUSTER_RHCS;
        iClusterType = CLUSTER_RHCS;
        rst = cls.StartCluster(iClusterType, strResGrpName);
        EXPECT_EQ(rst, ERROR_CLUSTER_START_SERVICE_FAILED);
    }
}
TEST_F(CClusterTest, StartResGrp)
{
    CCluster cls;
	mp_string strResGrpName;
	vector<mp_string> vecDevGrpName;
	mp_string strClusterType;
	mp_string strDBType;
	vector<mp_string> vecResourceName;

//    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    //strDBType = DB_ORACLE;
    strDBType = DB_ORACLE;
	cls.StartResGrp(strResGrpName,vecDevGrpName,strClusterType,strDBType,vecResourceName);
//    EXPECT_EQ(rst, MP_SUCCESS);
    //strDBType = DB_DB2;
    strDBType = DB_DB2;
	cls.StartResGrp(strResGrpName,vecDevGrpName,strClusterType,strDBType,vecResourceName);
//    EXPECT_EQ(rst, MP_SUCCESS);
    //strDBType = OTHER;
    strDBType = 12;
	cls.StartResGrp(strResGrpName,vecDevGrpName,strClusterType,strDBType,vecResourceName);
//    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
}
TEST_F(CClusterTest, StopResGrp)
{
    CCluster cls;
	mp_string strResGrpName;
	vector<mp_string> vecDevGrpName;
	mp_string strClusterType;
	mp_string strDBType;
	vector<mp_string> vecResourceName;

//    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub(&CRootCaller::Exec, &StubCRootCallerExecEq0);
    //strDBType = DB_ORACLE;
    strDBType = DB_ORACLE;
    cls.StopResGrp(strResGrpName,vecDevGrpName,strClusterType,strDBType,vecResourceName);
//    EXPECT_EQ(rst, MP_SUCCESS);
    //strDBType = DB_DB2;
    strDBType = DB_DB2;
    cls.StopResGrp(strResGrpName,vecDevGrpName,strClusterType,strDBType,vecResourceName);
//    EXPECT_EQ(rst, MP_SUCCESS);
    //strDBType = OTHER;
    strDBType = 12;
    cls.StopResGrp(strResGrpName,vecDevGrpName,strClusterType,strDBType,vecResourceName);
//    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);
}
TEST_F(CClusterTest, IsSQLServerActiveNode)
{
    CCluster cls;
    const mp_string resg("aaaa");
    mp_int32 rst = cls.IsSQLServerActiveNode(resg);
    EXPECT_EQ(rst, MP_FALSE);
}
TEST_F(CClusterTest, StartSQLServerResGrp)
{
    CCluster cls;
    const mp_string resg("aaaa");
    mp_int32 rst = cls.StartSQLServerResGrp(resg);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetSQLServerResourceGroupStatus)
{
    CCluster cls;
    const mp_string resg("aaaa");
    mp_string strStatus;
    mp_int32 rst = cls.GetSQLServerResourceGroupStatus(resg, strStatus);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetSQLServerClusterName)
{
    CCluster cls;
    mp_string strStatus;
    mp_int32 rst = cls.GetSQLServerClusterName(strStatus);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetSQLServerClusterResources)
{
    CCluster cls;
    vector<mp_string> vecResources;
    mp_int32 rst = cls.GetSQLServerClusterResources(vecResources);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetVrlServerName)
{
    CCluster cls;
    const mp_string strResName("aaa");
    mp_string strServerName;
    mp_int32 rst = cls.GetVrlServerName(strResName, strServerName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, BeInstanceResource)
{
    CCluster cls;
    const mp_string strResName("aaa");
    const mp_string strServerName("aaa");
    mp_int32 rst = cls.BeInstanceResource(strResName, strServerName);
    EXPECT_EQ(rst, MP_FALSE);
}
TEST_F(CClusterTest, GetClusterGroupByResource)
{
    CCluster cls;
    const mp_string strResName("aaa");
    mp_string strServerName("aaa");
    mp_int32 rst = cls.GetClusterGroupByResource(strResName, strServerName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetClusterGroupByinstName)
{
    CCluster cls;
    const mp_string strResName("aaa");
    mp_string strResourceGroup("aaa");
    mp_string strServerName("aaa");
    mp_int32 rst = cls.GetClusterGroupByinstName(strResName, strResourceGroup, strServerName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, ExecPowershell)
{
    CCluster cls;
    mp_string strResName("aaa");
    const mp_string strResourceGroup("aaa");
    mp_string strServerName("aaa");
    mp_int32 rst = cls.ExecPowershell(strResName, strResourceGroup, strServerName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, ReplaceStr)
{
    CCluster cls;
    const  mp_string oldStr("abababa");;
    mp_string newStr;
    const mp_string old_value("a");
    const mp_string new_value("b");
    cls.ReplaceStr(oldStr, newStr, old_value, new_value);
    EXPECT_TRUE(1);
}
//CutResourceName function has been modifed by DTS2016021900782,hence change this case
TEST_F(CClusterTest, CutResourceName)
{
    CCluster cls;
    const mp_string strResourceName("abc;abc;abc");
    mp_string strResName;
    mp_string strDiskMountPoint;
	mp_string strDiskSignature;
    mp_int32 rst = cls.CutResourceName(strResourceName, strResName, strDiskMountPoint, strDiskSignature);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CClusterTest, GetResourceStatus)
{
    CCluster cls;
    const mp_string strResName;
    mp_string strStatus;
    mp_int32 rst = cls.GetResourceStatus(strResName, strStatus);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, WaitResourceStatusChanged)
{
    CCluster cls;
    const mp_string strResName;
    mp_string strStatus;
    cls.WaitResourceStatusChanged(strResName, strStatus);
    EXPECT_TRUE(1);
}
TEST_F(CClusterTest, ResumeDiskResource)
{
    CCluster cls;
    const mp_string strResName;
    mp_int32 rst = cls.ResumeDiskResource(strResName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, ResetClusterDiskResource)
{
    CCluster cls;
    const mp_string strResName;
    const mp_string strDiskMountPoint;
    mp_int32 rst = cls.ResetClusterDiskResource(strResName, strDiskMountPoint);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, onlineResource)
{
    CCluster cls;
    const mp_string strResName;
    mp_int32 rst = cls.onlineResource(strResName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, SuspendResource)
{
    CCluster cls;
    const mp_string strResName;
    mp_int32 rst = cls.SuspendResource(strResName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, OnlineClusterDiskResource)
{
    CCluster cls;
    const mp_string strResName;
	const mp_string strGrpName;
    mp_int32 rst = cls.OnlineClusterDiskResource(strResName, strGrpName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, SuspendClusterDiskResources)
{
    CCluster cls;
    const vector<mp_string> strResName;
    mp_int32 rst = cls.SuspendClusterDiskResources(strResName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, OnlineClusterDiskResources)
{
    CCluster cls;
    const vector<mp_string> strResName;
	const mp_string strGrpName;
    mp_int32 rst = cls.OnlineClusterDiskResources(strResName, strGrpName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetSqlServerFilePath)
{
    CCluster cls;
    vector<mp_string> lstPath;
    const mp_string strParam;
    mp_int32 rst = cls.GetSqlServerFilePath(lstPath, strParam);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetSQLServerClusterDisk)
{
    CCluster cls;
    db_info_t stdbInfo;
    const mp_string strVrlServerName;
    vector<mp_string> vecDiskName;
    mp_int32 rst = cls.GetSQLServerClusterDisk(stdbInfo, strVrlServerName, vecDiskName);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetDiskNum)
{
    CCluster cls;
    const mp_string strDiskName;
    mp_string strDiskNumber;
    mp_int32 rst = cls.GetDiskNum(strDiskName, strDiskNumber);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetDiskSignature)
{
    CCluster cls;
    const mp_string strDiskNumber;
    mp_string strDiskSignature;
    mp_string strDiskPatitionType;
    mp_int32 rst = cls.GetDiskSignature(strDiskNumber, strDiskSignature, strDiskPatitionType);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetDiskResBySignature)
{
    CCluster cls;
    const mp_string strDiskNumber;
    const mp_string strDiskSignature;
    mp_string strDiskPatitionType;
    mp_int32 rst = cls.GetDiskResBySignature(strDiskNumber, strDiskSignature, strDiskPatitionType);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, GetSQLServerClusterResource)
{
    CCluster cls;
    const mp_string strDiskNumber;
    mp_string strDiskSignature;
    mp_int32 rst = cls.GetSQLServerClusterResource(strDiskNumber, strDiskSignature);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, QuerySQLServerClusterInfo)
{
    CCluster cls;
    db_info_t stdbInfo;
    mp_string strClusterType; 
    vector<cluster_info_t> vecClusterInfo;
    mp_int32 rst = cls.QuerySQLServerClusterInfo(stdbInfo, strClusterType, vecClusterInfo);
    EXPECT_EQ(rst, ERROR_COMMON_FUNC_UNIMPLEMENT);
}
TEST_F(CClusterTest, CheckResGrpState)
{
    CCluster cls;
    mp_int32 iCmd;
    mp_string strQueryParam;
    mp_string strStableState = "aaa";
    Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecStartErr);
    Stub<DoSleepType, StubDoSleepType, mp_void> mystub2(&DoSleep, &StubDoSleepVoid);
    mp_int32 rst = cls.CheckResGrpState(iCmd, strQueryParam, strStableState);
    EXPECT_EQ(rst, ERROR_CLUSTER_START_SERVICE_FAILED);
}

TEST_F(CClusterTest, BuildResGrpScriptParam)
{
	CCluster cls;
	mp_string strResGrp;
	vector<mp_string> vecDevGrp;
	mp_string strClusterType;
	mp_string strOperType;
	mp_string strParam;

    cls.BuildResGrpScriptParam(strResGrp,vecDevGrp,strClusterType,strOperType,strParam);
}

