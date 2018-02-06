/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifdef WIN32

#include "common/Path.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/ConfigXmlParse.h"
#include "common/SystemExec.h"
#include "common/ServiceHandler.h"
#include "common/Sign.h"
#include "common/CryptAlg.h"

SERVICE_STATUS_HANDLE g_hServiceStatus;
/*------------------------------------------------------------
Function Name: CheckBinName
Description  : 检查进程名字
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CheckBinName(mp_string strBinName)
{
    if (0 == strcmp(strBinName.c_str(), AGENT_EXEC_NAME) || 0 == strcmp(strBinName.c_str(), MONITOR_EXEC_NAME)
        || 0 == strcmp(strBinName.c_str(), NGINX_AS_PARAM_NAME))
    {
        return MP_TRUE;
    }
    else
    {
        return MP_FALSE;
    }
}

/*------------------------------------------------------------
Function Name: CheckOperate
Description  : 检查操作类型
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CheckOperate(mp_string strOperate)
{
    if (0 == strcmp(strOperate.c_str(), INSTALL_OPERATOR) || 0 == strcmp(strOperate.c_str(), UNINSTALL_OPERATOR)
        || 0 == strcmp(strOperate.c_str(), RUN_OPERATOR))
    {
        return MP_TRUE;
    }
    else
    {
        return MP_FALSE;
    }
}

/*------------------------------------------------------------
Function Name: NginxServiceHandler
Description  : nginx服务处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
DWORD WINAPI NginxServiceHandler(DWORD request, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    LOGGUARD("");
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    //校验脚本签名
    mp_int32 iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }
    iRet = CheckScriptSign(STOP_SCRIPT);
    //程序即将退出，此处不判断返回值
    (mp_void)FinalizeCrypt();
    if (iRet != MP_SUCCESS)
    {
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    
    switch (request)
    {
    case SERVICE_CONTROL_STOP:
        iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop nginx failed.");
            CWinServiceHanlder::UpdateServiceStatus(g_hServiceStatus, SERVICE_RUNNING, START_SERVICE_TIMEOUT);
            break;
        }
        
        CWinServiceHanlder::UpdateServiceStatus(g_hServiceStatus, SERVICE_STOPPED, START_SERVICE_TIMEOUT);
        break;
    default:
        break;
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: NginxServiceMain
Description  : nginx服务处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void WINAPI NginxServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    LOGGUARD("");
    //注册服务处理函数
    g_hServiceStatus = RegisterServiceCtrlHandlerExW((LPWSTR)NGINX_SERVICE_NAME, NginxServiceHandler, NULL);
    if (!g_hServiceStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Register nginx service failed.");
        return;
    }

    //启动Nginx
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_EXE);
    
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_START;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start nginx failed, iRet = %d.", iRet);
        CWinServiceHanlder::UpdateServiceStatus(g_hServiceStatus, SERVICE_STOPPED, START_SERVICE_TIMEOUT);
        return;
    }

    //设置运行状态
    CWinServiceHanlder::UpdateServiceStatus(g_hServiceStatus, SERVICE_RUNNING, START_SERVICE_TIMEOUT);
}

/*------------------------------------------------------------
Function Name: RunNginxService
Description  : nginx服务处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 RunNginxService()
{
    LOGGUARD("");
    //注册服务响应函数
    SERVICE_TABLE_ENTRY st[] =
    {
        {(LPSTR)NGINX_SERVICE_NAME, NginxServiceMain},
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

/*------------------------------------------------------------
Function Name: ProcessAgentService
Description  : 通过操作码响应不同agent服务处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 ProcessAgentService(mp_string strOperator, mp_string strWorkingUser, mp_string strWorkingUserPwd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bRet = MP_FALSE;
    //安装agent服务
    if (0 == strcmp(strOperator.c_str(), INSTALL_OPERATOR))
    {
        mp_string strExecName = mp_string(AGENT_EXEC_NAME) + ".exe";
        mp_string strBinPath = CPath::GetInstance().GetBinFilePath(AGENT_EXEC_NAME);
        strBinPath = CMpString::BlankComma(strBinPath);
        strBinPath = strBinPath + " -s";
        bRet = CWinServiceHanlder::InstallService(strBinPath, AGENT_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Install service \"%s\" failed.", AGENT_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    }
    //卸载Agent服务
    else if (0 == strcmp(strOperator.c_str(), UNINSTALL_OPERATOR))
    {
        bRet = CWinServiceHanlder::UninstallService(AGENT_SERVICE_NAME);
        if (!bRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Uninstall service \"%s\" failed.", AGENT_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupported param %s", strOperator.c_str());
        return MP_FAILED;
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: ProcessMonitorService
Description  : 通过操作码响应不同monitor服务处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/

mp_int32 ProcessMonitorService(mp_string strOperator, mp_string strWorkingUser, mp_string strWorkingUserPwd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bRet = MP_FALSE;
    //安装monitor服务
   if (0 == strcmp(strOperator.c_str(), INSTALL_OPERATOR))
    {
        mp_string strExecName = mp_string(MONITOR_EXEC_NAME) + ".exe";
        mp_string strBinPath = CPath::GetInstance().GetBinFilePath(strExecName);
        strBinPath = CMpString::BlankComma(strBinPath);
        strBinPath = strBinPath + " -s";
        bRet = CWinServiceHanlder::InstallService(strBinPath, MONITOR_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Install service \"%s\" failed.", MONITOR_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    }
    //卸载monitor服务
    else if(0 == strcmp(strOperator.c_str(), UNINSTALL_OPERATOR))
    {
        bRet = CWinServiceHanlder::UninstallService(MONITOR_SERVICE_NAME);
        if (!bRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Uninstall service \"%s\" failed.", MONITOR_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupported param %s", strOperator.c_str());
        return MP_FAILED;
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: ProcessNginxService
Description  : 通过操作码响应不同nginx服务处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 ProcessNginxService(mp_string strOperator, mp_string strWorkingUser, mp_string strWorkingUserPwd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bRet = MP_FALSE;
    //启动nginx服务
    if (0 == strcmp(strOperator.c_str(), RUN_OPERATOR))
    {
        iRet = RunNginxService();
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Run Nginx service failed, Ret = %d.", iRet);
        }
    }
    //安装nginx服务
    else if (0 == strcmp(strOperator.c_str(), INSTALL_OPERATOR))
    {
        mp_string strExecName = mp_string(WIN_SERVICE_EXEC_NAME) + ".exe";
        mp_string strBinPath = CPath::GetInstance().GetBinFilePath(strExecName);
        strBinPath = CMpString::BlankComma(strBinPath);
        strBinPath = strBinPath + " " + NGINX_AS_PARAM_NAME + " " + RUN_OPERATOR;
        bRet = CWinServiceHanlder::InstallService(strBinPath, NGINX_SERVICE_NAME, strWorkingUser, strWorkingUserPwd);
        if (!bRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Install service \"%s\" failed.", NGINX_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    }
    //卸载nginx服务
    else
    {
        bRet = CWinServiceHanlder::UninstallService(NGINX_SERVICE_NAME);
        if (!bRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Uninstall service \"%s\" failed.", NGINX_SERVICE_NAME);
            iRet = MP_FAILED;
        }
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: main
Description  : windows服务主函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_string strWorkingUser;
    mp_string strWorkingUserPwd;
    
    if ((argc != WIN_SERVICE_PRARM_NUM && argc != WIN_SERVICE_PRARM_NUM - 2) || !CheckBinName(argv[1]) || !CheckOperate(argv[2]))
    {
        printf("Usage: [path]winservice.exe rdagent|nginx|monitor run|install|uninstall [work_user password]");
        return MP_FAILED;
    }

    if (argc == WIN_SERVICE_PRARM_NUM)
    {
        strWorkingUser = argv[3];
        strWorkingUserPwd = argv[4];
    }

    //初始化WinService路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (MP_SUCCESS != iRet)
    {
        printf("Init winservice path failed.\n");
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
    CLogger::GetInstance().Init(WIN_SERVICE_LOG_NAME, CPath::GetInstance().GetLogPath());

    if (0 == strcmp(argv[1], AGENT_EXEC_NAME))
    {
       return ProcessAgentService(argv[2], strWorkingUser, strWorkingUserPwd);
    }
    else if  (0 == strcmp(argv[1], MONITOR_EXEC_NAME))
    {
        return ProcessMonitorService(argv[2], strWorkingUser, strWorkingUserPwd);
    }
    else
    {
        return ProcessNginxService(argv[2], strWorkingUser, strWorkingUserPwd);
    }
}

#endif
