/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifdef WIN32
#include "common/ServiceHandler.h"
#include "common/Log.h"

#include "Psapi.h"
#include <Windows.h>

/*------------------------------------------------------------
Function Name:IsServiceExist
Description  :根据服务名称判断windows服务是否已经注册
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CWinServiceHanlder::IsServiceExist(mp_string strServcieName)
{
    mp_bool bResult = MP_FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, // local machine
        NULL,                            // ServicesActive database
        SC_MANAGER_ALL_ACCESS);          // full access
    if (hSCM)
    {
        // Try to open the service
        SC_HANDLE hService = OpenService(hSCM,
            strServcieName.c_str(),
            SERVICE_QUERY_CONFIG);
        if (hService)
        {
            bResult = MP_TRUE;
            CloseServiceHandle(hService);
        }

        CloseServiceHandle(hSCM);
    }

    return bResult;
}

/*------------------------------------------------------------
Function Name:InstallService
Description  :安装windows服务
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CWinServiceHanlder::InstallService(mp_string strBinPath, mp_string strServcieName, mp_string strWorkingUser, 
    mp_string strWorkingUserPwd)
{
    LOGGUARD("");
    SC_HANDLE hService ;

    if (IsServiceExist(strServcieName))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Service %s is aleady exist", strServcieName.c_str());
        return MP_TRUE;
    }

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "OpenSCManager failed, errorno[%d].", GetLastError());
        return MP_FALSE;
    }

    // Create the service
    if ("" != strWorkingUser)
    {
        hService = CreateService(hSCM, strServcieName.c_str(), strServcieName.c_str(), SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, strBinPath.c_str(), NULL, NULL, NULL, strWorkingUser.c_str(), 
            strWorkingUserPwd.c_str()); 
    }
    else
    {
        hService = CreateService(hSCM, strServcieName.c_str(), strServcieName.c_str(), SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, strBinPath.c_str(), NULL, NULL, NULL, NULL, NULL);
    }

    if (!hService)
    {
        if (GetLastError() != ERROR_SERVICE_EXISTS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create Service failed, errorno[%d].", GetLastError());
            CloseServiceHandle(hSCM);
            return MP_FALSE;
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Service \"%s\" is already exist.", strServcieName.c_str());
        }
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    return MP_TRUE;
}

/*------------------------------------------------------------
Function Name:UninstallService
Description  :卸载windows服务
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CWinServiceHanlder::UninstallService(mp_string strServcieName)
{
    LOGGUARD("");
    mp_bool bResult = MP_FALSE;
    SC_HANDLE hService;

    if (!IsServiceExist(strServcieName))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Service %s is not exist", strServcieName.c_str());
        return MP_TRUE;
    }

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, // local machine
        NULL,                            // ServicesActive database
        SC_MANAGER_ALL_ACCESS);          // full access
    if (!hSCM)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "OpenSCManager failed, errorno[%d].", GetLastError());
        return MP_FALSE;
    }

    hService = OpenService(hSCM, strServcieName.c_str(), DELETE);
    if (hService)
    {
        if (DeleteService(hService))
        {
            bResult = MP_TRUE;
        }

        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCM);

    return bResult;
}

/*------------------------------------------------------------
Function Name:UpdateServiceStatus
Description  :更新windows服务状态
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CWinServiceHanlder::UpdateServiceStatus(SERVICE_STATUS_HANDLE hServiceStatus, DWORD dwState, DWORD dwTimeOut)
{
    SERVICE_STATUS stServiceStatus;
    stServiceStatus.dwCheckPoint = 0;
    stServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    stServiceStatus.dwWin32ExitCode = 0;
    stServiceStatus.dwCurrentState  = dwState;
    stServiceStatus.dwWaitHint = dwTimeOut;

    if(dwState == SERVICE_START_PENDING)
    {
        stServiceStatus.dwControlsAccepted   =   0;
    }
    else
    {
        stServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    }

    SetServiceStatus(hServiceStatus, &stServiceStatus);
}
#endif
