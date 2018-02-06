/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef WIN32
#include <signal.h>
#include <libgen.h>
#include <sys/wait.h>
#endif
#include <sstream>
#include "securec.h"
#include "common/SystemExec.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/CryptAlg.h"
#include "common/UniqueId.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/Sign.h"

/*------------------------------------------------------------
Function Name: ExecSystemWithoutEcho
Description  : 执行系统调用，不获取回显信息
               bNeedRedirect，默认为true，表示需要将系统执行结果重定向到日志文件
               bNeedRedirect，可以显示设为false，向echo等命令无需将执行结果重定向到日志文件
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemExec::ExecSystemWithoutEcho(mp_string& strCommand, mp_bool bNeedRedirect)
{
    mp_string strLogCmd;
    RemoveFullPathForLog(strCommand, strLogCmd);

    //LOGGUARD("");
    //检查命令合法性
    if (CheckCmdDelimiter(strCommand) == MP_FALSE)
    {
        //打印日志
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "strCommand %s contain invalid character.", strLogCmd.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_int32 iRet = MP_FAILED;

    //在命令末尾添加重定向命令
    //信息全部重定向到agent日志文件
    mp_string strLogFilePath;

#ifdef WIN32
    //strLogFilePath = strLogFilePath = CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME);
    //mp_string strNewCommand;
    //strNewCommand = bNeedRedirect ? strCommand + " 1>>" + strLogFilePath + " 2>&1" : strCommand;
    //windows下多个进程会调用此函数，不能将执行结果重定向到某一个日志文件下
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command is %s", strLogCmd.c_str());

    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
    DWORD dwCode = 0;

    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    stStartupInfo.wShowWindow = SW_HIDE;

    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));
    if (!CreateProcess(NULL,   // No module name (use command line).
                       TEXT((LPTSTR)strCommand.c_str()), // Command line.
                       NULL,            // Process handle not inheritable.
                       NULL,            // Thread handle not inheritable.
                       MP_FALSE,           // Set handle inheritance to ISSP_FALSE.
                       0,               // No creation flags.
                       NULL,            // Use parent's environment block.
                       NULL,            // Use parent's starting directory.
                       &stStartupInfo,             // Pointer to STARTUPINFO structure.
                       &stProcessInfo)            // Pointer to PROCESS_INFORMATION structure.
    )
    {
        mp_int32 iErr = GetLastError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateProcess failed, errono[%d]: %s", 
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));

        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin WaitForSingleObject");

    if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess, 1000 * 3600))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "WaitForSingleObject timeout");
        iRet = MP_FAILED;
    }
    else
    {
        GetExitCodeProcess(stProcessInfo.hProcess, &dwCode);
        iRet = dwCode;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "GetExitCodeProcess is %d", iRet);
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "end WaitForSingleObject");
	//CodeDex误报，SECURE_CODING
    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Leave ExecSystemWithoutEcho, command is %s", strLogCmd.c_str());
    return iRet;
#else
    uid_t uid = getuid();
    if (0 == uid)
    {
        strLogFilePath = CPath::GetInstance().GetLogFilePath(ROOT_EXEC_LOG_NAME);
    }
    else
    {
        strLogFilePath = CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME);
    }

    strLogFilePath = CMpString::BlankComma(strLogFilePath);
    mp_string strNewCommand, strLogNewCmd;
    strNewCommand = bNeedRedirect ? strCommand + " 1>>" + strLogFilePath + " 2>&1" : strCommand;
    strLogNewCmd = bNeedRedirect ? strLogCmd + " 1>>" + strLogFilePath + " 2>&1" : strLogCmd;

    //COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "begin system call");
    //Coverity&Fortify误报:FORTIFY.Command_Injection 
    //已经按修复建议增加命令分隔符判断CheckCmdDelimiter    
    FILE *pStream = popen(strNewCommand.c_str(), "r");
    if (NULL == pStream)
    {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "popen call failed, errno[%d]:%s.", iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }
    
    while (!feof(pStream))
    {
        mp_char tmpBuf[1000] = {0};
        mp_char* cRet = fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (NULL == cRet)
        {
            //COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "fgets is null.");
        }
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Leave ExecSystemWithoutEcho, command is %s", strLogNewCmd.c_str());
    iRet = pclose(pStream);
    if (-1 == iRet)
    {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "pclose excute failed, errno[%d]:%s.", iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }
    else
    {
        //pclose返回的是shell最终状态，需要获取子进程的返回值
        //还需要考虑WIFSIGNALED和WIFSTOPPED的影响
        if (WIFEXITED(iRet))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Sub process return %d.", WEXITSTATUS(iRet));
            return WEXITSTATUS(iRet);
        }
        else
        {
            //子进程异常返回
            mp_int32 iErr = GetOSError();
            mp_char szErr[256] = {0};
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Sub process exit abnormally, errno[%d]:%s.", iErr,
                GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return ERROR_COMMON_SYSTEM_CALL_FAILED;
        }
    }
#endif
}

/*------------------------------------------------------------
Function Name: ExecSystemWithEcho
Description  : 执行系统调用，获取回显信息
               bNeedRedirect，默认为true，表示需要将系统执行结果重定向到日志文件
               bNeedRedirect，可以显示设为false，向echo等命令无需将执行结果重定向到日志文件
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemExec::ExecSystemWithEcho(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    mp_string strLogCmd;
    RemoveFullPathForLog(strCommand, strLogCmd);
    //LOGGUARD("");
    //检查命令合法性
    if (CheckCmdDelimiter(strCommand) == MP_FALSE)
    {
        //打印日志
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "strCommand %s contain invalid character.", strLogCmd.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

#ifdef WIN32
    mp_int32 iRet = MP_FAILED;
    SECURITY_ATTRIBUTES sa;
    HANDLE hRead,hWrite;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if(!CreatePipe(&hRead, &hWrite, &sa, 0)) //创建匿名管道
    {
        iRet = GetLastError();
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "CreatePipe failed, iRet = %d", iRet);
        return iRet;
    }

    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
    DWORD dwCode = 0;

    ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
    stStartupInfo.cb = sizeof(stStartupInfo);
    GetStartupInfo(&stStartupInfo);  
    stStartupInfo.hStdError = hWrite;      //把创建进程的标准错误输出重定向到管道输入     
    stStartupInfo.hStdOutput = hWrite;   //把创建进程的标准输出重定向到管道输入 
    stStartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    stStartupInfo.wShowWindow = SW_HIDE;

    ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));
    if (!CreateProcess(NULL,   // No module name (use command line).
                       TEXT((LPTSTR)strCommand.c_str()), // Command line.
                       NULL,            // Process handle not inheritable.
                       NULL,            // Thread handle not inheritable.
                       MP_TRUE,           // Set handle inheritance to ISSP_FALSE.
                       0,               // No creation flags.
                       NULL,            // Use parent's environment block.
                       NULL,            // Use parent's starting directory.
                       &stStartupInfo,             // Pointer to STARTUPINFO structure.
                       &stProcessInfo)            // Pointer to PROCESS_INFORMATION structure.
    )
    {
        iRet = GetLastError();
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "CreateProcess failed, iRet = %d", iRet);
        CloseHandle(hWrite);
        CloseHandle(hRead);
        return iRet;
    }
    //CodeDex误报，SECURE_CODING
    CloseHandle(hWrite);

    while (MP_TRUE)
    {
        mp_char tmpBuf[1000] = {0};
        DWORD bytesRead = 0;
        if(ReadFile(hRead, tmpBuf, 1000, &bytesRead, NULL) == NULL)
        {
            break;
        }
        strEcho.push_back(tmpBuf);
    }

    CloseHandle(stProcessInfo.hProcess);
    CloseHandle(stProcessInfo.hThread);
    CloseHandle(hRead);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Leave ExecSystemWithEcho, command is %s", strLogCmd.c_str());
    return MP_SUCCESS;
#else
    uid_t uid = getuid();
    //root用户，将错误重定向至rootexec日志，否则，将错误重定向至rdagent日志
    mp_string strLogFilePath;
    if (0 == uid)
    {
        strLogFilePath = CPath::GetInstance().GetLogFilePath(ROOT_EXEC_LOG_NAME);
    }
    else
    {
        strLogFilePath = CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME);
    }

    strLogFilePath = CMpString::BlankComma(strLogFilePath);
    mp_string strNewCommand, strLogNewCmd;
    strNewCommand= bNeedRedirect ? strCommand + " 2>>" + strLogFilePath : strCommand;
    strLogNewCmd = bNeedRedirect ? strLogCmd + " 1>>" + strLogFilePath + " 2>&1" : strLogCmd;
    
    //COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "begin popen");
    //Coverity&Fortify误报:FORTIFY.Command_Injection 
    //已经按修复建议增加命令分隔符判断CheckCmdDelimiter
    FILE *pStream = popen(strNewCommand.c_str(), "r");
    if (NULL == pStream)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "popen failed.");
        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }
    //COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "end popen");
    while (!feof(pStream))
    {
        mp_char tmpBuf[1000] = {0};
        mp_char* cRet = fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (NULL == cRet)
        {
            //COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "fgets is null.");
        }
        if (strlen(tmpBuf) > 0)
        {
            tmpBuf[strlen(tmpBuf) - 1] = 0; //去掉获取出来的字符串末尾的'\n'
        }
        if ((0 == tmpBuf[0])
            || ('\n' == tmpBuf[0]))
        {
            continue;
        }

        strEcho.push_back(mp_string(tmpBuf));
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Leave ExecSystemWithEcho, command is %s", strLogNewCmd.c_str());
    pclose(pStream);
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ExecScript
Description  : 在当前agent用户下执行脚本，执行结果通过函数返回值返回
Input        : strScriptFileName 脚本文件名称
               strParam 脚本输入参数
Return       : 直接返回脚本执行返回值，如需转换，请在外层转换
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemExec::ExecScript(mp_string strScriptFileName, mp_string strParam, vector<mp_string>* pvecResult, mp_bool bNeedCheckSign)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;
    //获取agent路径
    mp_string strAgentPath = CPath::GetInstance().GetRootPath();
    //获取脚本全路径
    mp_string strScriptFilePath = CPath::GetInstance().GetBinFilePath(strScriptFileName);
    //获取唯一ID，用于生成临时文件
    mp_string strUniqueID = CUniqueID::GetInstance().GetString();

    iRet = CheckExecParam(strScriptFileName, strParam, bNeedCheckSign, strAgentPath, strScriptFilePath, strUniqueID);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check exec param failed, ret %d.", iRet);
        return iRet;
    }
    //组装命令
    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    //为保证和R3C10兼容，保留以前的参数顺序UID,PATH
    mp_bool bIsThirdParty = MP_FALSE;
    if (strScriptFileName.find(AGENT_THIRDPARTY_DIR) != mp_string::npos) 
    {
        bIsThirdParty = MP_TRUE;
        CHECK_FAIL(SNPRINTF_S(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "%s %s %s",
                 strScriptFilePath.c_str(), strUniqueID.c_str(), strAgentPath.c_str()));
    }
    //其他脚本的参数顺序是PATH,UID
    else
    {
        CHECK_FAIL(SNPRINTF_S(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "%s %s %s",
                 strScriptFilePath.c_str(), strAgentPath.c_str(), strUniqueID.c_str()));
    }
    mp_string strCmd = acCmd;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strScriptFileName.c_str());
    //执行脚本均不获取回显
    iRet = ExecSystemWithoutEcho(strCmd);
    //执行第三方脚本无论成功失败都需要读取结果文件，此处不能直接返回。
    mp_bool bRet = (!bIsThirdParty && MP_SUCCESS != iRet);
    if (bRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ExecSystemWithoutEcho failed.");
        return iRet;
    }

    //读取结果文件
    mp_int32 iRet1 = MP_SUCCESS;
    if (0 != pvecResult)
    {
        bIsThirdParty ? (iRet1 = CIPCFile::ReadOldResult(strUniqueID, *pvecResult)) : (iRet1 = CIPCFile::ReadResult(strUniqueID, *pvecResult));
        if (MP_SUCCESS != iRet1)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read result file failed.");
        }
    }

    return iRet == MP_SUCCESS ? iRet1 : iRet;
}

/*------------------------------------------------------------
Function Name: CheckExecParam
Description  : 
Input        :        
Return       : 
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemExec::CheckExecParam(mp_string &strScriptFileName, mp_string &strParam, mp_bool &bNeedCheckSign, 
    mp_string &strAgentPath, mp_string &strScriptFilePath, mp_string &strUniqueID)
{
    LOGGUARD("");

    mp_int32 iRet = MP_FAILED;

    //判断脚本是否存在
    if (!CMpFile::FileExist(strScriptFilePath.c_str()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Script is not exist, path is %s.", strScriptFileName.c_str());
        //统一在外部进行错误码转换
        return INTER_ERROR_SRCIPT_FILE_NOT_EXIST;
    }
    //校验脚本签名
    if (bNeedCheckSign)
    {
        iRet = CheckScriptSign(strScriptFileName);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CheckScriptSign failed");
            return iRet;
        }
    }
    
    strScriptFilePath = CMpString::BlankComma(strScriptFilePath);
    strAgentPath = CMpString::BlankComma(strAgentPath);

    if (!strParam.empty())
    {
        //将参数写入ipc文件
        iRet = CIPCFile::WriteInput(strUniqueID, strParam);
        if (iRet != MP_SUCCESS)
        {
            //打印日志
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "WriteInput failed, ret %d.", iRet);
            return iRet;
        }
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Check excute param success.");
    return MP_SUCCESS;
}

