/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/CollectLog.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/SystemExec.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Password.h"

/*------------------------------------------------------------ 
Description  : 各进程的运行状态
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CCollectLog::Handle()
{
    printf("%s\n", COLLECTLOG_HINT);
    mp_string strTmpChoice;
    mp_uint32 uiCount = 0;
    mp_uint32 uiRetryCount = 3;

    while (uiCount < uiRetryCount)
    {
        printf("%s", CONTINUE);
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice == "n")
        {
            return MP_FAILED;
        }
        else if (strTmpChoice == "y")
        {
            break;
        }
        else
        {
            uiCount += 1;
        }
    }

    if (uiCount >= uiRetryCount)
    {
        printf("Input invalid value over 3 times.\n");
        return MP_FAILED;
    }

    mp_time time;
    CMpTime::Now(&time);
    mp_string strNowTime = CMpTime::GetTimeString(&time);
    //去除字符串中非数字
    mp_string strTime;
    for (mp_uint32 i = 0; i < strNowTime.length(); i++)
    {
        if (strNowTime[i] >= '0' && strNowTime[i] <= '9')
        {
            strTime.push_back(strNowTime[i]);
        }
    }
    mp_string strLogName = mp_string(AGENT_LOG_ZIP_NAME) + strTime + ZIP_SUFFIX;
    mp_int32 iRet = PackageLog(strLogName);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Package log failed, iRet = %d.", iRet);
        printf("Collect log failed.\n");
        return iRet;
    }

    //打印提示信息
    printf("Logs are collected in %s\n", strLogName.c_str());
    return MP_SUCCESS;
}

