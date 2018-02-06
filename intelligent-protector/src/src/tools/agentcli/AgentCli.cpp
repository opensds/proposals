/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common//CryptAlg.h"
#include "common/Password.h"
#include "tools/agentcli/ShowStatus.h"
#include "tools/agentcli/ChgSnmp.h"
#include "tools/agentcli/ChgNgxPwd.h"
#include "tools/agentcli/CollectLog.h"
#include "tools/agentcli/ChangeIP.h"
#include "tools/agentcli/ChgHostSN.h"
#include "tools/agentcli/StartNginx.h"
#ifndef WIN32
#include "array/Array.h"
#include <pwd.h>
#endif

#define CMD_PARAM_NUM 2
//#define CMD_PARAM_NUM_DEVICE 3
#define CHG_PWD "chgpwd"
#define SHOW_STATUS "show"
#define CHG_SNMP "chgsnmp"
#define CHG_NGX_PWD "chgkey"
#define GEN_SECONDS "genseconds"
#define COLLECT_LOG  "collectlog"
#define CHANGE_IP "chgbindip"
#define CHG_HOST_SN "chghostsn"
#define START_NGINX "startnginx"
//#define REPORT_LUN  "reportlun"     // get map lun id list by hostnum channelnum targetnum
//#define GETWWN      "getwwn"        // get wwn  by device name
/*------------------------------------------------------------ 
Description  : 显示agentcli帮助
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
void PrintHelp()
{
    printf("usage:\n");
    printf("[path]agentcli %s\n", SHOW_STATUS);
    printf("[path]agentcli %s\n", CHG_PWD);
    printf("[path]agentcli %s\n", CHG_SNMP);
    printf("[path]agentcli %s\n", CHG_NGX_PWD);
    printf("[path]agentcli %s\n", COLLECT_LOG);
    printf("[path]agentcli %s\n", CHANGE_IP);
    printf("[path]agentcli %s\n", GEN_SECONDS);
    printf("[path]agentcli %s\n", CHG_HOST_SN);
    printf("[path]agentcli %s\n", START_NGINX);
    
/*
#ifndef WIN32
    printf("[path]agentcli %s devicename\n", REPORT_LUN);
    printf("[path]agentcli %s devicename\n", GETWWN);
#endif
*/
}

mp_int32 HandleCmd(mp_string strCmd, mp_string strParam)
{
    //根据输入参数分别进行处理
    if (0 == strcmp(strCmd.c_str(), CHG_PWD))
    {
        return CPassword::ChgAdminPwd();
    }
    else if  (0 == strcmp(strCmd.c_str(), SHOW_STATUS))
    {
        return CShowStatus::Handle();
    }
    else if (0 == strcmp(strCmd.c_str(), CHG_SNMP))
    {
        return CChgSnmp::Handle();
    }
    else if (0 == strcmp(strCmd.c_str(), CHG_NGX_PWD))
    {
        return CChgNgxPwd::Handle();
    }
    else if (0 == strcmp(strCmd.c_str(), COLLECT_LOG))
    {
        return CCollectLog::Handle();
    }
    else if (0 == strcmp(strCmd.c_str(), CHANGE_IP))
    {
        return CChangeIP::Handle();
    }
    else if (0 == strcmp(strCmd.c_str(), GEN_SECONDS))
    {
        mp_double seconds = CMpTime::GenSeconds();
        printf("%.f\n", seconds);
        return MP_SUCCESS;
    }
    else if (0 == strcmp(strCmd.c_str(), CHG_HOST_SN))
    {
        return CChgHostSN::Handle();
    }
    else if (0 == strcmp(strCmd.c_str(), START_NGINX))
    {
        return CStartNginx::Handle();
    }
    
/*
#ifndef WIN32
    else if (0 == strcmp(strCmd.c_str(), REPORT_LUN))
    {
        vector<mp_int32> vecHostLunID;
        mp_int32 iRet = CArray::GetHostLunIDList(strParam, vecHostLunID);
        if (iRet != MP_SUCCESS)
        {
            printf("report lun failed.\n");
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host lun list(%s) failed.", strParam.c_str());
            return MP_FAILED;
        }
        else
        {
            printf("Lun list length = %d which imples %d lun entr%s\n", vecHostLunID.size() * 8, vecHostLunID.size(), 
                ((1 == vecHostLunID.size())?"y":"ies"));

            for (vector<mp_int32>::iterator iter = vecHostLunID.begin(); iter != vecHostLunID.end(); ++iter)
            {
                printf("Peripheral device addressing: lun=%d\n", *iter);
            }
            return MP_SUCCESS;
        }
    }
    else if (0 == strcmp(strCmd.c_str(), GETWWN))
    {
        mp_string strWWN, strLUNID;
        mp_int32 iRet = CArray::GetDisk83Page(strParam, strWWN, strLUNID);
        if (iRet != MP_SUCCESS)
        {
            printf("get wwn failed.\n");
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get wwn(%s) failed.", strParam.c_str());
            return MP_FAILED;
        }
        else
        {
            printf("Designation descriptor number\n");
            printf("[0x");
            printf("%s", strWWN.c_str());
            printf("]\n");
            return MP_SUCCESS;
        }
    }
#endif
*/
    else
    {
        PrintHelp();
        return MP_FAILED;
    }
}

/*------------------------------------------------------------
Function Name: main
Description  : agentcli进程主函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_bool bCheck = (argc == CMD_PARAM_NUM) && (0 == strcmp(argv[1], CHG_PWD) || 0 == strcmp(argv[1], SHOW_STATUS)
             || 0 == strcmp(argv[1], CHG_SNMP) || 0 == strcmp(argv[1], CHG_NGX_PWD) || 0 == strcmp(argv[1], COLLECT_LOG) 
             || 0 == strcmp(argv[1], CHANGE_IP)|| 0 == strcmp(argv[1], GEN_SECONDS) || 0 == strcmp(argv[1], CHG_HOST_SN)
             || 0 == strcmp(argv[1], START_NGINX));
/*
#ifndef WIN32
    bCheck = bCheck || ((argc == CMD_PARAM_NUM_DEVICE) && (0 == strcmp(argv[1], REPORT_LUN) || 0 == strcmp(argv[1], GETWWN)));
#endif
*/
    if (!bCheck)
    {
        PrintHelp();
        return MP_FAILED;
    }

#ifndef WIN32
    struct passwd *currentUser;
	//CodeDex误报，Missing Check against Null
    currentUser = getpwuid(getuid());
    if( !strncmp(currentUser->pw_name, "root", strlen("root")))
    {
       printf("You can not execute this command as user \"root\".\n");
       return MP_FAILED;
    }
#endif

    //初始化agentcli路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (MP_SUCCESS != iRet)
    {
        printf("Init agentcli path failed.\n");
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
    CLogger::GetInstance().Init(AGENT_CLI_LOG_NAME, CPath::GetInstance().GetLogPath());

    mp_uint64 currentTime = CMpTime::GetTimeSec();
    mp_uint64 lockTime = CPassword::GetLockTime();
    if (lockTime != 0)
    {
        if (currentTime - lockTime < LOCK_MAX_TIME)
        {
            printf("agentcli is locked, please try again later.\n");
            return MP_FAILED;
        }
        else
        {
            CPassword::ClearLock();
        }
    }
    
	//CodeDex误报，NULL_RETURNS，iRet不会为空
    //初始化KMC
    iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        printf("Init crypt failed.\n");
        return iRet;
    }

    //根据输入参数分别进行处理
    mp_string strParam;
    iRet = HandleCmd(argv[1], strParam);
    //程序即将退出，此处不判断返回值
    (mp_void)FinalizeCrypt();
    return iRet;
}


