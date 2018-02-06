/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/monitor/AbnormalHandlerTest.h"
#include "alarm/Trap.h"

#include <sys/types.h>
#include <unistd.h>
#include <sstream>

static mp_int32 StubGetFolderFile(mp_string& strFolder, vector<mp_string>& vecFileList)
{
    vecFileList.push_back("123");
    return MP_SUCCESS;
}

static mp_int32 StubExecSystemWithEcho(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    static mp_int32 icounter_inner = 0;
    if (icounter_inner++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        strEcho.push_back("123");
        return MP_SUCCESS;
    }
}

mp_int32 StubGetAgentMonitorData(mp_void *ptr, monitor_data_t& stMonitorData)
{
    static mp_int32 icounter_inner = 0;
    if (icounter_inner++ == 0)
    {
        stMonitorData.bExist = MP_FAILED;
        return MP_FAILED;
    }
    else
    {
        stMonitorData.bExist = MP_TRUE;
        stMonitorData.ulPmSize = 1;
        stMonitorData.fCpuUsage = 1;
        return MP_SUCCESS;
    }
}


static mp_int32 StubFileSize(const mp_char* pszFilePath, mp_uint32& uiSize)
{
    uiSize = 1;
    return MP_SUCCESS;
}


static void * timeoutHandle(mp_void *ptr)
{
    int nCnt = 0;
    CAbnormalHandler *pAbnormalHandler = (CAbnormalHandler *)ptr;
    while ( nCnt++ < 3)
    {
        DoSleep(1000);
    }
    
    pAbnormalHandler->m_bNeedExit = MP_TRUE;
    return (void *)0;
}

 static  mp_string stubGetLogFilePath(mp_void)
{
    return mp_string("./");    
}

static mp_int32 stubReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static mp_int32 icounter_inner = 0;
    if (icounter_inner++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        vecOutput.push_back("123");
        return MP_SUCCESS;
    }
    
}

static monitor_data_t stubAddMonitorData(mp_void)
{
    return tag_monitor_data();
}

static mp_int32 stubReadMonitorPidFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    if (iCounter++ == 1)
    {
        pid_t monitorPid = getpid();
        ostringstream oss;
        oss<<(mp_int32)monitorPid;
        vecOutput.push_back(oss.str().c_str());
    }
    else
    {
        vecOutput.push_back("123");
    }
    return MP_SUCCESS;
}
static mp_int32 stubWriteMonitorPidFile(mp_string& strFilePath, vector<mp_string>& vecInput)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0)
    {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

TEST_F(CAbnormalHandlerTest, Handle)
{
    mp_int32 iRet = MP_SUCCESS;
    monitor_data_t stMonitorData;
    CAbnormalHandler AbnormalHandler;

    AbnormalHandler.m_stAgentMointorCfg.bMonitored = MP_TRUE;
    AbnormalHandler.m_stNginxMointorCfg.bMonitored = MP_TRUE;

    typedef mp_int32 (*pOrgReadFile)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    typedef mp_int32 (*pStubReadFile)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    Stub<pOrgReadFile, pStubReadFile, mp_void> stubCMpFile(&CMpFile::ReadFile, &stubReadMonitorPidFile);

    typedef mp_int32 (*pOrgWriteFile)(mp_string& strFilePath, vector<mp_string>& vecInput);
    typedef mp_int32 (*pStubWriteFile)(mp_string& strFilePath, vector<mp_string>& vecInput);
    Stub<pOrgWriteFile, pStubWriteFile, mp_void> stubCIPCFile(&CIPCFile::WriteFile, &stubWriteMonitorPidFile);
    
    typedef mp_int32 (CAbnormalHandler::*pOrgMonitorAgent)(mp_void);
    Stub<pOrgMonitorAgent, pStubIntType, mp_void> StubCSystemExec1(&CAbnormalHandler::MonitorAgent, &stub_return_ret);
    Stub<pOrgMonitorAgent, pStubIntType, mp_void> StubCSystemExec2(&CAbnormalHandler::MonitorNginx, &stub_return_ret);

    iRet = AbnormalHandler.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.Handle();
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = AbnormalHandler.Handle();
    EXPECT_EQ(MP_SUCCESS, iRet);
}



TEST_F(CAbnormalHandlerTest, GetAgentMonitorData)
{
    mp_int32 iRet = MP_SUCCESS;
    monitor_data_t stMonitorData;
    CAbnormalHandler AbnormalHandler;
    
    typedef mp_string (CPath::*pOrgGetLogFilePath)(mp_string strFileName);
    typedef mp_string (*pStubGetLogFilePath)(mp_void);
    Stub<pOrgGetLogFilePath, pStubGetLogFilePath, mp_void> stubCPath(&CPath::GetLogFilePath, &stubGetLogFilePath);

    typedef mp_int32 (*pOrgReadFile)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    typedef mp_int32 (*pStubReadFile)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    Stub<pOrgReadFile, pStubReadFile, mp_void> stubCMpFile(&CMpFile::ReadFile, &stubReadFile);

    iRet = AbnormalHandler.GetAgentMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    /*ExecSystemWithEcho; */
    iRet = AbnormalHandler.GetAgentMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    typedef mp_int32 (*pOrgGetTmpFileTotalSize)(mp_uint64& ulSize);
    Stub<pOrgGetTmpFileTotalSize, pStubVoidType, mp_void> StubCAbnormalHandler_0(&CAbnormalHandler::GetTmpFileTotalSize, &stub_return_nothing);

    typedef mp_int32 (CAbnormalHandler::*pOrgGetMonitorData)(mp_string strProcessID, monitor_data_t& stMonitorData);
    Stub<pOrgGetMonitorData, pStubIntType, mp_void> StubCAbnormalHandler_1(&CAbnormalHandler::GetMonitorData, &stub_return_ret);

    iRet = AbnormalHandler.GetAgentMonitorData(stMonitorData);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.GetAgentMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CAbnormalHandlerTest, GetNginxMonitorData)
{
    mp_int32 iRet = MP_SUCCESS;
    monitor_data_t stMonitorData;
    CAbnormalHandler AbnormalHandler;

    typedef mp_string (CPath::*pOrgGetLogFilePath)(mp_string strFileName);
    typedef mp_string (*pStubGetLogFilePath)(mp_void);
    Stub<pOrgGetLogFilePath, pStubGetLogFilePath, mp_void> stubCPath(&CPath::GetLogFilePath, &stubGetLogFilePath);

    typedef mp_int32 (*pOrgReadFile)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    typedef mp_int32 (*pStubReadFile)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    Stub<pOrgReadFile, pStubReadFile, mp_void> stubCMpFile(&CMpFile::ReadFile, &stubReadFile);

    iRet = AbnormalHandler.GetNginxMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    /* ExecSystemWithEcho; */
    iRet = AbnormalHandler.GetNginxMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    typedef mp_string (CPath::*pOrgGetNginxLogsFilePath)(mp_string strFileName);
    Stub<pOrgGetNginxLogsFilePath, pStubStringType, mp_void> StubCPath1(&CPath::GetNginxLogsFilePath, &stub_return_string);
    
    typedef mp_int32 (*pOrgFileSize)(const mp_char* pszFilePath, mp_uint32& uiSize);
    Stub<pOrgFileSize, pStubIntType, mp_void> StubCMpFile_1(&CMpFile::FileSize, &stub_return_ret);    

    typedef mp_int32 (CAbnormalHandler::*pOrgGetMonitorData)(mp_string strProcessID, monitor_data_t& stMonitorData);
    Stub<pOrgGetMonitorData, pStubIntType, mp_void> StubCAbnormalHandler_1(&CAbnormalHandler::GetMonitorData, &stub_return_ret);

    typedef monitor_data_t (*pOrgAddMonitorData)(monitor_data_t& stMonitorData1, monitor_data_t& stMonitorData2);
    typedef monitor_data_t (*pStubAddMonitorData)(mp_void);
    Stub<pOrgAddMonitorData, pStubAddMonitorData, mp_void> StubCAbnormalHandler_2(&CAbnormalHandler::AddMonitorData, &stubAddMonitorData);

    iRet = AbnormalHandler.GetNginxMonitorData(stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CAbnormalHandlerTest, GetMonitorData)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strProcessID;
    monitor_data_t stMonitorData;
    CAbnormalHandler AbnormalHandler;

    iRet = AbnormalHandler.GetMonitorData(strProcessID, stMonitorData);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.GetMonitorData(strProcessID, stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, GetHPMonitorData)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strProcessID;
    monitor_data_t stMonitorData;
    CAbnormalHandler AbnormalHandler;
    
    iRet = AbnormalHandler.GetHPMonitorData(strProcessID, stMonitorData);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.GetHPMonitorData(strProcessID, stMonitorData);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.GetHPMonitorData(strProcessID, stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = AbnormalHandler.GetHPMonitorData(strProcessID, stMonitorData);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, StartAgent)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;

    iRet = AbnormalHandler.StartAgent();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.StartAgent();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.StartAgent();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, StartNginx)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;

    iRet = AbnormalHandler.StartNginx();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.StartNginx();
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = AbnormalHandler.StartNginx();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, RestartAgent)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;
    
    iRet = AbnormalHandler.RestartAgent();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.RestartAgent();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.RestartAgent();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, RestartNginx)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;

    iRet = AbnormalHandler.RestartNginx();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.RestartNginx();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.RestartNginx();
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CAbnormalHandlerTest, SendCPUAlarm)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;

    typedef mp_int32 (*pOrgSendAlarm)(alarm_param_t &alarmParam);
    Stub<pOrgSendAlarm, pStubIntType, mp_void> stubCTrapSender(&CTrapSender::SendAlarm, &stub_return_ret);
    
    iRet = AbnormalHandler.SendCPUAlarm(NGINX_PROCESS);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.SendCPUAlarm(AGENT_PROCESS);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, ResumeCPUAlarm)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;

    typedef mp_int32 (*pOrgResumeAlarm)(alarm_param_t &alarmParam);
    Stub<pOrgResumeAlarm, pStubIntType, mp_void> stubCTrapSender(&CTrapSender::ResumeAlarm, &stub_return_ret);

    iRet = AbnormalHandler.ResumeCPUAlarm();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = AbnormalHandler.ResumeCPUAlarm();
    EXPECT_EQ(MP_SUCCESS, iRet);
}


TEST_F(CAbnormalHandlerTest, MonitorAgent)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;

    typedef mp_int32 (CAbnormalHandler::*pOrgGetAgentMonitorData)(monitor_data_t& stMonitorData);
    typedef mp_int32 (*pStubGetAgentMonitorData)(mp_void *ptr, monitor_data_t& stMonitorData);
    Stub<pOrgGetAgentMonitorData, pStubGetAgentMonitorData, mp_void> StubCAbnormalHandler(&CAbnormalHandler::GetAgentMonitorData, &StubGetAgentMonitorData);

    typedef mp_int32 (CAbnormalHandler::*pOrgRestartAgent)(mp_void);
    Stub<pOrgRestartAgent, pStubIntType, mp_void> StubCAbnormalHandler2(&CAbnormalHandler::RestartAgent, &stub_return_ret);
    
    iRet = AbnormalHandler.MonitorAgent();
    EXPECT_EQ(MP_FAILED, iRet);

    AbnormalHandler.m_stAgentAbnormal.iPmSizeOverTimes = 2;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 1;
    iRet = AbnormalHandler.MonitorAgent();
    EXPECT_EQ(MP_FAILED, iRet);

    AbnormalHandler.m_stAgentAbnormal.iPmSizeOverTimes = 0;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 0;
    AbnormalHandler.m_stAgentAbnormal.iCpuUsageOverTimes = 2;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 1;
    AbnormalHandler.m_iAgentAlarmSendFailedTimes = ALARM_SEND_FAILED_TIME + 1;
    AbnormalHandler.m_bAgentCpuAlarmSend = MP_FALSE;
    AbnormalHandler.m_stAgentAbnormal.iTmpFileSizeOverTimes = ALARM_SEND_FAILED_TIME;
    iRet = AbnormalHandler.MonitorAgent();
    EXPECT_EQ(MP_SUCCESS, iRet);
    
}


TEST_F(CAbnormalHandlerTest, MonitorNginx)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;

    typedef mp_int32 (CAbnormalHandler::*pOrgGetAgentMonitorData)(monitor_data_t& stMonitorData);
    typedef mp_int32 (*pStubGetAgentMonitorData)(mp_void *ptr, monitor_data_t& stMonitorData);
    Stub<pOrgGetAgentMonitorData, pStubGetAgentMonitorData, mp_void> StubCAbnormalHandler(&CAbnormalHandler::GetNginxMonitorData, &StubGetAgentMonitorData);

    typedef mp_int32 (CAbnormalHandler::*pOrgRestartAgent)(mp_void);
    Stub<pOrgRestartAgent, pStubIntType, mp_void> StubCAbnormalHandler2(&CAbnormalHandler::RestartAgent, &stub_return_ret);
    
    iRet = AbnormalHandler.MonitorNginx();
    EXPECT_EQ(MP_SUCCESS, iRet);

    AbnormalHandler.m_stNginxAbnormal.iPmSizeOverTimes = 2;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 1;
    iRet = AbnormalHandler.MonitorNginx();
    EXPECT_EQ(MP_FAILED, iRet);

    AbnormalHandler.m_stNginxAbnormal.iPmSizeOverTimes = 0;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 0;
    
    AbnormalHandler.m_stNginxAbnormal.iCpuUsageOverTimes = 2;
    AbnormalHandler.m_stCommonMonitorCfg.iRetryTime = 1;
    AbnormalHandler.m_bNginxCpuAlarmSend = MP_FALSE;
    
    AbnormalHandler.m_iNginxAlarmSendFailedTimes = ALARM_SEND_FAILED_TIME + 1;
    iRet = AbnormalHandler.MonitorNginx();
    EXPECT_EQ(MP_SUCCESS, iRet);
    

}


TEST_F(CAbnormalHandlerTest, DeleteTmpFile)
{
    mp_int32 iRet = MP_SUCCESS;
    CAbnormalHandler AbnormalHandler;
    typedef mp_int32 (*pGetFolderFile)(mp_string& strFolder, vector<mp_string>& vecFileList);
    typedef mp_int32 (*pStubGetFolderFile)(mp_string& strFolder, vector<mp_string>& vecFileList);
    Stub<pGetFolderFile, pStubGetFolderFile, mp_void> StubCFile(CMpFile::GetFolderFile, &StubGetFolderFile);

    iRet = AbnormalHandler.DeleteTmpFile();
    EXPECT_EQ(MP_FAILED, iRet);

}


TEST_F(CAbnormalHandlerTest, GetTmpFileTotalSize)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_uint64 ulSize = 0;
    mp_string strFolder;
    vector<mp_string> vecFileList;
    CAbnormalHandler AbnormalHandler;
    
    typedef mp_int32 (*pGetFolderFile)(mp_string& strFolder, vector<mp_string>& vecFileList);
    typedef mp_int32 (*pStubGetFolderFile)(mp_string& strFolder, vector<mp_string>& vecFileList);
    Stub<pGetFolderFile, pStubGetFolderFile, mp_void> StubCFile(CMpFile::GetFolderFile, &StubGetFolderFile);

    typedef mp_int32 (pOrgFileSize)(const mp_char* pszFilePath, mp_uint32& uiSize);
    typedef mp_int32 (*pStubFileSize)(const mp_char* pszFilePath, mp_uint32& uiSize);
    Stub<pOrgFileSize, pStubFileSize, mp_void> StubCFile1(CMpFile::FileSize, &StubFileSize);

    iRet = AbnormalHandler.GetTmpFileTotalSize(ulSize);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CAbnormalHandlerTest, GetKSize)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strSize;
    mp_uint64 usize;
    CAbnormalHandler AbnormalHandler;

    AbnormalHandler.GetKSize(strSize);
}


TEST_F(CAbnormalHandlerTest, NeedExit)
{
    mp_int32 bRet = MP_TRUE;
    CAbnormalHandler AbnormalHandler;

    bRet = AbnormalHandler.NeedExit();
    EXPECT_EQ(MP_FALSE, bRet);
}


TEST_F(CAbnormalHandlerTest, HandleFunc)
{
    CAbnormalHandler AbnormalHandler;
    mp_void* pThis = &AbnormalHandler;

    /* start thread; */
    pthread_t tid;
    pthread_create(&tid, NULL, timeoutHandle, &AbnormalHandler);
    pthread_detach(tid);
    
    EXPECT_EQ(NULL, AbnormalHandler.HandleFunc(pThis));
}


TEST_F(CAbnormalHandlerTest, xfunc)
{
    monitor_data_t stMonitorData1;
    monitor_data_t stMonitorData2;
    mp_int32 iThreadStatus;
    abnormal_occur_times_t stAbnormalOccurTimes;
    CAbnormalHandler AbnormalHandler;

    AbnormalHandler.GetAgentMonitorCfg();
    AbnormalHandler.GetNginxMonitorCfg();
    AbnormalHandler.GetCommonMonitorCfg();

    AbnormalHandler.AddMonitorData(stMonitorData1, stMonitorData2);
    AbnormalHandler.ClearMonitorData(stMonitorData1);
    AbnormalHandler.ClearAbnormalOccurTimes(stAbnormalOccurTimes);
    AbnormalHandler.SetThreadStatus(iThreadStatus);
}



