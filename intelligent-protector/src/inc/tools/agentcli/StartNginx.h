/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENTCLI_STARTNGINX_H_
#define _AGENTCLI_STARTNGINX_H_

#include <vector>
#include "common/Types.h"

class CStartNginx
{
public:
    static mp_int32 Handle();

private:
    static mp_int32 ExecNginxStart();
    static mp_int32 GetPassword(mp_string & CipherStr);
};

#endif
