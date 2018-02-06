/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENTCLI_COLLECTLOG_H_
#define _AGENTCLI_COLLECTLOG_H_

#include "common/Types.h"
#define COLLECTLOG_HINT "This operation allows you to collect debugging logs of Agent running on this host. \
The collected logs can be used to analyze the operating status of Agent."

class CCollectLog
{
public:
    static mp_int32 Handle();
};

#endif
