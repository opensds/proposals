/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/RootCaller.h"
#include "common/Utils.h"
#include "common/UniqueId.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/SystemExec.h"
#include "securec.h"

/*------------------------------------------------------------
Function Name: Exec
Description  : root权限执行函数，供其他模块静态调用
               iCommandID: 命令ID,定义参见ROOT_COMMAND
               strParam: root权限执行参数，非引用，如无参数，直接输入""
               pvecResult: 保存执行结果的vector，如无需结果，直接输入NULL
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRootCaller::Exec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;

    //获取当前执行唯一id
    mp_string strUniqueID = CUniqueID::GetInstance().GetString();

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

    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    mp_string strBinFilePath = CPath::GetInstance().GetBinFilePath(ROOT_EXEC_NAME);
    strBinFilePath = CMpString::BlankComma(strBinFilePath);
    CHECK_FAIL(SNPRINTF_S(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "%s -c %d -u %s",
         strBinFilePath.c_str(), iCommandID, strUniqueID.c_str()));

    mp_string strCmd = acCmd;
    mp_string strLogCmd;
    RemoveFullPathForLog(strCmd, strLogCmd);
    //检查是否包含非法字符
    if (CheckCmdDelimiter(strCmd) == MP_FALSE)
    {
        //打印日志
        return ERROR_COMMON_INVALID_PARAM;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strLogCmd.c_str());
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    //对于第三方脚本，无论成功失败，均需要读取结果文件
    if (iRet != MP_SUCCESS)
    {
        //错误码转换，脚本执行返回转换后的错误码，非脚本执行统一返回-1
        if (iCommandID >= ROOT_COMMAND_SCRIPT_BEGIN && iCommandID <= ROOT_COMMAND_SCRIPT_END)
        {
            CErrorCodeMap errorCode;
            mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                 "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
            if(ROOT_COMMAND_THIRDPARTY != iCommandID)
            {
                return iNewRet;
            }
            iRet = iNewRet;
        }
        else
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                 "Exec system failed, initial return code %d", iRet);
            return MP_FAILED;
        }
    }

    //读取结果文件
    mp_int32 iRet1 = ReadResultFile(iCommandID, strUniqueID, pvecResult);
    

    return (iRet == MP_SUCCESS) ? iRet1 : iRet;
}


/*------------------------------------------------------------
Function Name: ReadResultFile
Description  : Exec函数执行完后，如果需要读取结果文件，将调用这个函数
               iCommandID: 命令ID,定义参见ROOT_COMMAND
               strUniqueID: Exec 函数执行的唯一id
               pvecResult: 保存执行结果的vector，如无需结果，直接输入NULL
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRootCaller::ReadResultFile(mp_int32 iCommandID, mp_string& strUniqueID, vector<mp_string>* pvecResult)
{
    mp_int32 iRet = MP_SUCCESS;
    
    if (0 != pvecResult)
    {
        if ( ROOT_COMMAND_THIRDPARTY != iCommandID )
        {
            iRet = CIPCFile::ReadResult(strUniqueID, *pvecResult);
        }
        else
        {
            iRet = CIPCFile::ReadOldResult(strUniqueID, *pvecResult);
        }
        
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read result file failed.");
        }
    }

    return iRet;
}

