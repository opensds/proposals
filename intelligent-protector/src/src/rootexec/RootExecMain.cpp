/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include <sys/types.h>
#include <pwd.h>
#include "common/Types.h"
#include "common/RootCaller.h"
#include "rootexec/SystemCall.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/UniqueId.h"
#include "common/ErrorCode.h"
#include "common/CryptAlg.h"

/*------------------------------------------------------------
Function Name: IsRunManually
Description  : 判断是否是自己启动

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool IsRunManually()
{
    pid_t my_pid, my_gid;

    my_pid = getpid();
    my_gid = getpgrp();
    if (my_pid == my_gid)
    {
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_int32 SUPPProcessCommand(mp_int32 iCommandID, mp_string strUniqueID,mp_int32 &oFlag)
{
    oFlag = 0;

#ifdef LIN_FRE_SUPP
    if (iCommandID == ROOT_COMMAND_FILESYS_FREEZE)
    {
         return CSystemCall::FreezeFileSys(strUniqueID);
     }      
    else if (iCommandID == ROOT_COMMAND_FILESYS_THAW)
    {
        return CSystemCall::ThawFileSys(strUniqueID);
    }
#endif
    oFlag = 1;
    return MP_TRUE;

}

mp_int32 COMProcessCommand(mp_int32 iCommandID, mp_string strUniqueID,mp_int32 &oFlag)
{
    oFlag = 0;
    mp_bool bRet1 = (iCommandID > ROOT_COMMAND_SCRIPT_BEGIN && iCommandID < ROOT_COMMAND_SCRIPT_END);
    mp_bool bRet2 = (iCommandID > ROOT_COMMAND_SYSTEM_BEGIN && iCommandID < ROOT_COMMAND_SYSTEM_END);
    if (bRet1)
    {
        if ((iCommandID != ROOT_COMMAND_THIRDPARTY))
        {
            return CSystemCall::ExecScript(strUniqueID, iCommandID);
        }

        return CSystemCall::ExecThirdPartyScript(strUniqueID);
    }
    else if (bRet2)
    {
        return CSystemCall::ExecSysCmd(strUniqueID, iCommandID);
    }
    else if (iCommandID == ROOT_COMMAND_80PAGE)
    {
        return CSystemCall::GetDisk80Page(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_83PAGE)
    {
        return CSystemCall::GetDisk83Page(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_00PAGE)
    {
        return CSystemCall::GetDisk00Page(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_C8PAGE)
    {
        return CSystemCall::GetDiskC8Page(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_CAPACITY)
    {
        return CSystemCall::GetDiskCapacity(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_VENDORANDPRODUCT)
    {
        return CSystemCall::GetVendorAndProduct(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_RAW_MINOR_MAJOR)
    {
        return CSystemCall::GetRawMajorMinor(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_BATCH_GETLUN_INFO)
    {
        return CSystemCall::BatchGetLUNInfo(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_UDEV_RELOAD_RULES)
    {
        return CSystemCall::ReloadUDEVRules(strUniqueID);
    }
    else if (iCommandID == ROOT_COMMAND_SYNC_DATA_FILE)
    {
        return CSystemCall::SyncDataFile(strUniqueID);
    }

    oFlag = 1;
    return MP_TRUE;
}
/*------------------------------------------------------------
Function Name: ProcessCommand
Description  : 根据输入参数执行具体操作，是执行脚本，还是执行系统调用，或其他

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 ProcessCommand(mp_int32 iCommandID, mp_string strUniqueID)
{
    LOGGUARD("CommandID is %d, Unique ID is %s", iCommandID, strUniqueID.c_str());
    mp_int32 iRet = 0;
    mp_int32 oFlag = 0;

    iRet = COMProcessCommand(iCommandID,strUniqueID,oFlag);
    if(0 == oFlag)
    {
        return iRet;
    }

    iRet = SUPPProcessCommand(iCommandID, strUniqueID,oFlag);
    if(0 == oFlag)
    {
        return iRet;
    }

    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unknown Command, ID is %d.", iCommandID);
    return ERROR_COMMON_INVALID_PARAM;
}

/*------------------------------------------------------------
Function Name: Exec
Description  : 根据输入参数执行具体操作

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/

mp_int32 Exec(mp_int32 iCommandID, mp_string strUniqueID)
{
    LOGGUARD("Command ID is %d, UniqueID = %s", iCommandID, strUniqueID.c_str());
    mp_int32 iRet = MP_FAILED;
    struct passwd* pPwd = NULL;

    iRet = setuid(0);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set effective user id to root failed, errno[%d]:%s.",
            errno, strerror(errno));

        return iRet;
    }
	//CodeDex误报，Often Misused:Privilege Management
    pPwd = getpwuid(0);
    if (NULL == pPwd)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get password database info of root failed, errno[%d]:%s.",
            errno, strerror(errno));

        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    iRet = setgid(pPwd->pw_gid);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set effective group id to %d failed, errno[%d]:%s.",
            pPwd->pw_gid, errno, strerror(errno));
        return iRet;
    }

    //初始化KMC
    iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }
    
    iRet = ProcessCommand(iCommandID, strUniqueID);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Process Command(ID = %d) failed, ret %d.", iCommandID, iRet);
        return iRet;
    }

    //程序即将退出，此处不判断返回值
    (mp_void)FinalizeCrypt();
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: main函数
Description  : 用户输入规则如下:
               Usage: rootexec -r <AGENT_ROOT> -c <命令id> -u <用户id>]);
               -a <命令id>    脚本/系统命令内部命令字，定义参见ROOT_COMMAND，必选参数
               -i <全局不重复id>  当前执行用户不重复的id，用于生成临时文件，必选参数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 main(mp_int32 argc, mp_char** argv)
{
	//CodeDex误报，Portability Flaw
    mp_int32 iRet = MP_FAILED;
    mp_int32 iOpt = 0;
    mp_bool bIsManual = IsRunManually();
    if (bIsManual)
    {
        printf("This program can't be started manually.\n");
        return MP_FAILED;
    }

    mp_int32 iCommandID = MP_FAILED;
    mp_string strUniqueID;

    const mp_char* pszOptString = "c:u:";
    iOpt = getopt(argc, argv, pszOptString);
    while (-1 != iOpt)
    {
        switch (iOpt)
        {
            case 'c':
                iCommandID = atoi(optarg);
                break;
            case 'u':
                strUniqueID = optarg;
                break;
            default:
                return ERROR_COMMON_INVALID_PARAM;
        }

        iOpt = getopt(argc, argv, pszOptString);
    }

    if (iCommandID == MP_FAILED || strUniqueID.empty())
    {
        printf("Usage: rootexec -c <Command_ID> -u <Unique_ID>\n");
        return ERROR_COMMON_INVALID_PARAM;
    }

    //初始化路径
    iRet = CPath::GetInstance().Init(argv[0]);
    if (MP_SUCCESS != iRet)
    {
        printf("Init path %s failed.\n", argv[0]);
        return iRet;
    }

    //初始化xml配置
    mp_string strXMLConfFilePath = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    iRet = CConfigXmlParser::GetInstance().Init(strXMLConfFilePath);
    if (MP_SUCCESS != iRet)
    {
        printf("Init xml conf file %s failed.\n", AGENT_XML_CONF);
        return iRet;
    }

    //初始化日志路径
    mp_string strFilePath = CPath::GetInstance().GetLogPath();
    CLogger::GetInstance().Init(mp_string(ROOT_EXEC_LOG_NAME).c_str(), strFilePath);

    return Exec(iCommandID, strUniqueID);
}

