/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_IP_H__
#define __AGENT_IP_H__

#include "common/Types.h"
#include "common/Defines.h"

#include <vector>

const char* const DELIM = ".";
#define MAX_PORT_NUM 65535

class AGENT_API CIPCheck
{
public:
    static mp_bool IsIPV4(mp_string& strIpAddr);

private:
    static mp_void SplitIPV4(mp_string& strIpAddr, vector<mp_string>& vecOutput);
    static mp_bool IsNumber(mp_string str);
};

#endif //__AGENT_UNIQUEID_H__

