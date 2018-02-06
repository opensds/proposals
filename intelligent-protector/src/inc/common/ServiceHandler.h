/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _SERVICE_HANDLER_HANDLER_H_
#define _SERVICE_HANDLER_HANDLER_H_

#ifdef WIN32
#include "common/Types.h"
#include "common/Defines.h"

#define START_SERVICE_TIMEOUT (1000 * 60)
#define STOP_SERVICE_TIMEOUT  (1000 * 60)

#define WIN_SERVICE_PRARM_NUM 5

class AGENT_API CWinServiceHanlder
{
public:
    static mp_bool InstallService(mp_string strBinPath, mp_string strServcieName, mp_string strWorkingUser, 
        mp_string strWorkingUserPwd);
    static mp_bool UninstallService(mp_string strServcieName);
    static mp_void UpdateServiceStatus(SERVICE_STATUS_HANDLE hServiceStatus, DWORD dwState, DWORD dwTimeOut);

private:
    static mp_bool IsServiceExist(mp_string strServcieName);
};

#endif
#endif

