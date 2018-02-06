/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/ChgHostSN.h"
#include "common/Defines.h"
#include "common/ConfigXmlParse.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/SystemExec.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "host/Host.h"
#include "common/Password.h"
#include "common/Sign.h"

mp_string CChgHostSN::m_ChghostsnFile;

/*------------------------------------------------------------ 
Description  : 修改HostSN
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CChgHostSN::Handle()
{   
    mp_string strInput;
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to Chg HostSN.");
    
    //用户校验
    iRet = CChgHostSN::CheckUserPwd();
    if ( MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "Failed in func CChgHostSN::CheckUserPwd.");
        return iRet;
    }
 
    //获取新输入的HostSN号
    iRet = CChgHostSN::GetHostSNNum(strInput);
    if ( MP_SUCCESS != iRet )
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "Failed in func CChgHostSN::GetHostSNNum.");
        return iRet;
    }
    
    //修改该HostSN的值
    iRet = CChgHostSN::ModifyHostSN(vecResult, strInput);
    if ( MP_SUCCESS != iRet )
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "Failed in func CChgHostSN::ModifyHostSN.");
        return iRet;
    }

    //修改HostSN文件的权限和所属
    iRet = CChgHostSN::ChownHostSn(m_ChghostsnFile);
    if ( MP_SUCCESS != iRet )
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Change HostSN file auth failed.");
        return MP_FAILED;
    }

    printf("HostSN changed succ!\n");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Write HostSN file succ.");

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 从终端获取HostSN号
Input        : 
Output       : 获取到的HostSN号
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CChgHostSN::GetHostSNNum(mp_string& strInput)
{   
    mp_uint32 InputTimes = 0;

    //运行3次输入用户密码
    while (InputTimes <= MAX_FAILED_COUNT)
    {
        printf("%s", INPUT_HOSTSN_CHG);
        CPassword::GetInput(INPUT_HOSTSN_CHG, strInput,HOSTSN_LEN);
        if ( "" == strInput )
        {
            printf("The input HostSN is empty, Retry! .\n");
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input HostSN is incorrectly.");
            InputTimes++;
            continue;
        }
        else
        {
             break;
        }
    }
    
    if (InputTimes > MAX_FAILED_COUNT)
    {
        printf("%s.\n", "Input incorrect HostSN more than 3 times, exit!");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input incorrect HostSN more than 3 times.");
        return MP_FAILED;
    }
    
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 修改HostSN号
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CChgHostSN::ModifyHostSN(vector<mp_string>& vecResult, mp_string& strInput)
{   
    vecResult.clear();
    vecResult.push_back(strInput);
    m_ChghostsnFile = CPath::GetInstance().GetConfFilePath(HOSTSN_FILE);
    mp_int32 iRet = CIPCFile::WriteFile(m_ChghostsnFile, vecResult);
    if ( MP_SUCCESS != iRet )
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Write HostSN file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecResult.size());
        return iRet;
    }

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 用户校验
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CChgHostSN::CheckUserPwd()
{
    mp_string strOldPwd;
    mp_string strUsrName;
    mp_uint32 InputTimes = 0;

    //从本地配置文件中获取设置的用户名
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name failed!");
        return MP_FAILED;
    }
    
    //运行3次输入用户密码
    while (InputTimes <= MAX_FAILED_COUNT)
    {
        CPassword::InputUserPwd(strUsrName, strOldPwd, INPUT_GET_ADMIN_OLD_PWD);
        if (CPassword::CheckAdminOldPwd(strOldPwd))
        {
            break;
        }
        else
        {
            InputTimes++;
            continue;
        }
    }

    strOldPwd.replace(0, strOldPwd.length(), "");
    if (InputTimes > MAX_FAILED_COUNT)
    {
        printf("%s.\n", OPERATION_LOCKED_HINT);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, OPERATION_LOCKED_HINT);
        CPassword::LockAdmin();
        return MP_FAILED;
    }

    return MP_SUCCESS;
}


/*------------------------------------------------------------
Function Name: ChownResult
Description  : 修改HOSTSN文件的所属组权限和执行权限，改成rdadmin用户，-rw-------执行权限
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CChgHostSN::ChownHostSn(mp_string& strInput)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 uid(-1), gid(-1);
    
    const mp_char* strBaseFileName = NULL;
    strBaseFileName = BaseFileName(strInput.c_str());
    
    //获取rdadmin用户的uid和gid
    iRet = GetUidByUserName(AGENT_RUNNING_USER, uid, gid);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user(%s) uid and gid failed.", AGENT_RUNNING_USER);
        CMpFile::DelFile(strInput.c_str());
        return MP_FAILED;
    }

    // 设置rdadmin的uid和gid
    iRet = ChownFile(strInput, uid, gid);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Chown file failed, file %s.", strBaseFileName);
        CMpFile::DelFile(strInput.c_str());
        return MP_FAILED;
    }

    //修改用户执行权限
    if (chmod(strInput.c_str(), S_IRUSR | S_IWUSR) == -1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "chmod hostsn %s failed, errno[%d]:%s.", strBaseFileName,
            errno, strerror(errno));
        CMpFile::DelFile(strInput.c_str());
        return MP_FAILED;
    }
    
    return MP_SUCCESS;
#else
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCommand = "cmd.exe /c echo Y | cacls.exe \"" + strInput + "\" /E /R Users > c:\\nul";

    iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "cacls hostsn file failed %d.", iRet);
        CMpFile::DelFile(strInput.c_str());
        return iRet;
    }
    
    return MP_SUCCESS;
#endif
}