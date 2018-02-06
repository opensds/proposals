/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/app/AppTest.h"

TEST_F(CAppTest, QueryInfo)
{
	CApp app;
	vector<app_info_t> vecAppInfos;
	
    app.QueryInfo(vecAppInfos);
}

TEST_F(CAppTest, Freeze)
{
	CApp app;
	vector<app_failed_info_t> vecAppFailedList;
	app_auth_info_t appAuthInfo;
	mp_time tFreezeTime;
	
    app.Freeze(appAuthInfo,tFreezeTime,vecAppFailedList);
}

TEST_F(CAppTest, UnFreeze)
{
	CApp app;
	vector<app_failed_info_t> vecAppFailedList;
	app_auth_info_t appAuthInfo;
	
    app.UnFreeze(appAuthInfo,vecAppFailedList);
}

TEST_F(CAppTest, EndBackup)
{
	CApp app;
	vector<app_failed_info_t> vecAppFailedList;
	app_auth_info_t appAuthInfo;
	mp_int32 iBackupSucc;
	
    app.EndBackup(appAuthInfo,iBackupSucc,vecAppFailedList);
}

TEST_F(CAppTest, TruncateLog)
{
	CApp app;
	vector<app_failed_info_t> vecAppFailedList;
	app_auth_info_t appAuthInfo;
	mp_time tTruncateTime;
	
    app.TruncateLog(appAuthInfo,tTruncateTime,vecAppFailedList);
}

TEST_F(CAppTest, QuerySqlInfo)
{
	CApp app;
	vector<app_info_t> vecAppInfos;
	
    app.QuerySqlInfo(vecAppInfos);
}

TEST_F(CAppTest, QueryOracleFreezeState)
{
	CApp app;
	app_auth_info_t appAuthInfo;
	mp_int32 iState;
	
    app.QueryOracleFreezeState(appAuthInfo,iState);
	
	Stub<IsInstalledType, StubIsInstalledType, mp_void> mystub1(&COracle::IsInstalled, &StubIsInstalled);
	app.QueryOracleFreezeState(appAuthInfo,iState);
}

TEST_F(CAppTest, QueryOracleInfo)
{
	CApp app;
	vector<app_info_t> vecAppInfos;
	
    app.QueryOracleInfo(vecAppInfos);
	
	Stub<IsInstalledType, StubIsInstalledType, mp_void> mystub1(&COracle::IsInstalled, &StubIsInstalled);
	app.QueryOracleInfo(vecAppInfos);
}

TEST_F(CAppTest, FreezeOracle)
{
	CApp app;
	vector<app_failed_info_t> vecAppFailedList;
	app_auth_info_t appAuthInfo;
	
    app.FreezeOracle(appAuthInfo,vecAppFailedList);
	
	Stub<IsInstalledType, StubIsInstalledType, mp_void> mystub1(&COracle::IsInstalled, &StubIsInstalled);
	app.FreezeOracle(appAuthInfo,vecAppFailedList);
}

TEST_F(CAppTest, UnFreezeOracle)
{
	CApp app;
	vector<app_failed_info_t> vecAppFailedList;
	app_auth_info_t appAuthInfo;
	
    app.UnFreezeOracle(appAuthInfo,vecAppFailedList);
	
	Stub<IsInstalledType, StubIsInstalledType, mp_void> mystub1(&COracle::IsInstalled, &StubIsInstalled);
	app.UnFreezeOracle(appAuthInfo,vecAppFailedList);
}

TEST_F(CAppTest, UnFreezeExOracle)
{
	CApp app;
	app_auth_info_t appAuthInfo;
	
    app.UnFreezeExOracle(appAuthInfo);
	
	Stub<IsInstalledType, StubIsInstalledType, mp_void> mystub1(&COracle::IsInstalled, &StubIsInstalled);
	app.UnFreezeExOracle(appAuthInfo);
}

TEST_F(CAppTest, TruncateOracleLog)
{
	CApp app;
	vector<app_failed_info_t> vecAppFailedList;
	app_auth_info_t appAuthInfo;
	mp_time tTruncateTime;
	
    app.TruncateOracleLog(appAuthInfo,tTruncateTime,vecAppFailedList);
	
	Stub<IsInstalledType, StubIsInstalledType, mp_void> mystub1(&COracle::IsInstalled, &StubIsInstalled);
	app.TruncateOracleLog(appAuthInfo,tTruncateTime,vecAppFailedList);
}

TEST_F(CAppTest, QueryFreezeState)
{
	CApp app;
	app_auth_info_t appAuthInfo;
	mp_int32 iState;
	
    app.QueryFreezeState(appAuthInfo,iState);
}

TEST_F(CAppTest, UnFreezeEx)
{
	CApp app;
	app_auth_info_t appAuthInfo;
	
    app.UnFreezeEx(appAuthInfo);
}

/*
TEST_F(CArrayTest, OpenDev)
{
    mp_string str = "test";
    mp_int32 iDevFd;
    mp_int32 rst;
    //open = 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, StubopenEq0);
        rst = CArray::OpenDev(str, iDevFd);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    //open < 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, StubopenLt0);
        rst = CArray::OpenDev(str, iDevFd);
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
}*/


