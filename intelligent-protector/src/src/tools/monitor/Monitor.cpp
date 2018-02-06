/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/monitor/AbnormalHandler.h"
#include "common/Path.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"

#ifdef WIN32
#include "common/ServiceHandler.h"
#endif

mp_bool g_bExitFlag = MP_FALSE;

#ifndef WIN32
//
/*------------------------------------------------------------
Function Name: MakeIndependent
Description  : 后台执行函数，AIX平台下还不能在后台执行，需要定位原因
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void MakeIndependent()
{
    printf("make indepentdent\n");
    pid_t pid;
    if (0 == (pid = fork()))
    {
        if (-1 == setpgid(getpid(), 0))
        {
            printf("Set process group id failed, errno[%d]: %s.",
                errno, strerror(errno));

            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set process group id failed, errno[%d]: %s.",
                errno, strerror(errno));

            return;
        }

        printf("go on, pid %d\n", getpid());
        return;
    }
    else if (pid < 0)
    {
        printf("Fork failed, errno[%d]: %s.", errno, strerror(errno));
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Fork failed, errno[%d]: %s.", errno, strerror(errno));

        return;
    }
    else
    {
    	//CodeDex误报， System Information Leak:Internal
        printf("exit 0, pid %d\n", getpid());
        exit(0);
    }
}
#else
SERVICE_STATUS_HANDLE g_hServiceStatus;
/*------------------------------------------------------------
Function Name: MonitorServiceHandler
Description  : monitor windows服务处理函数，对停止服务操作做出响应
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
DWORD WINAPI MonitorServiceHandler(DWORD request, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    LOGGUARD("");
    switch (request)
    {
    case SERVICE_CONTROL_STOP:
        CWinServiceHanlder::UpdateServiceStatus(g_hServiceStatus, SERVICE_STOPPED, START_SERVICE_TIMEOUT);
        g_bExitFlag = MP_TRUE;
        break;
    default:
        break;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: MonitorServiceMain
Description  : monitor windows服务入口函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void WINAPI MonitorServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    LOGGUARD("");
    //注册服务处理函数
    g_hServiceStatus = RegisterServiceCtrlHandlerExW((LPWSTR)MONITOR_SERVICE_NAME, MonitorServiceHandler, NULL);
    if (!g_hServiceStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Registe monitor service failed.");
        return;
    }

    //设置运行状态
    CWinServiceHanlder::UpdateServiceStatus(g_hServiceStatus, SERVICE_RUNNING, START_SERVICE_TIMEOUT);
}

/*------------------------------------------------------------
Function Name: MonitorServiceMain
Description  : 注册monitor windows服务入口函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 RunMonitorService()
{
    LOGGUARD("");
    //注册服务响应函数
    SERVICE_TABLE_ENTRY st[] =
    {
        {(LPSTR)MONITOR_SERVICE_NAME, MonitorServiceMain},
        {NULL, NULL}
    };

    mp_bool bRet = StartServiceCtrlDispatcher(st);
    if (!bRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "StartServiceCtrlDispatcher failed, errorno[%d].", GetLastError());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

#endif

/*------------------------------------------------------------
Function Name: main
Description  : monitor进程主函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    //后台运行
    //MakeIndependent();

    //初始化Monitor路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (MP_SUCCESS != iRet)
    {
        printf("Init monitor path failed.\n");
        return iRet;
    }

    //初始化配置文件模块
    iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (MP_SUCCESS != iRet)
    {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF);
        return iRet;
    }

    //初始化日志模块
    CLogger::GetInstance().Init(MONITOR_LOG_NAME, CPath::GetInstance().GetLogPath());

    //初始化kmc
    iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init crypt failed, iRet = %d", iRet);
        return iRet;
    }

    //初始化异常处理模块
    iRet = CAbnormalHandler::GetInstance().Init();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init AbnormalHandler failed, iRet = %d", iRet);
        return iRet;
    }

#ifdef WIN32
    if (argc == 2 && 0 == strcmp(argv[1], "-s"))
    {
        iRet = RunMonitorService();
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Run monitor service failed, iRet = %d", iRet);
            return iRet;
        }
    }
#endif

    for (;;)
    {
        if (g_bExitFlag)
        {
            exit(0);
        }
        DoSleep(MONITOR_SLEEP_TIME);
    }

}

