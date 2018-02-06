/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_SYSTEM_EXEC_H__
#define __AGENT_SYSTEM_EXEC_H__

#include "common/Types.h"
#include "common/Defines.h"
#include "common/Thread.h"
#include <vector>

class CSystemExec
{
public:
    //系统调用
    AGENT_API static mp_int32 ExecSystemWithoutEcho(mp_string& strCommand, mp_bool bNeedRedirect = MP_TRUE);
    AGENT_API static mp_int32 ExecSystemWithEcho(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect = MP_TRUE);
    //非root权限执行脚本
    AGENT_API static mp_int32 ExecScript(mp_string strScriptFileName, mp_string strParam, vector<mp_string>* pvecResult, mp_bool bNeedCheckSign = MP_TRUE);
private:
    AGENT_API static mp_int32 CheckExecParam(mp_string &strScriptFileName, mp_string &strParam, mp_bool &bNeedCheckSign, 
        mp_string &strAgentPath, mp_string &strScriptFilePath, mp_string &strUniqueID);
};

#endif //__AGENT_SYSTEM_EXEC_H__

