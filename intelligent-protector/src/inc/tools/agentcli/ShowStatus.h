/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENTCLI_SHOWSTATUS_H_
#define _AGENTCLI_SHOWSTATUS_H_

#include "common/Types.h"

#define RUNNING_TAG "RUNNING"
#define SVN_CONF "svn"

typedef enum
{
    PROCESS_RDAGENT = 0,
    PROCESS_NGINX,
    PROCESS_MONITOR,
    PROCESS_BUTT
}PROCESS_TYPE;

class CShowStatus
{
public:
    static mp_int32 Handle();

private:
    static mp_bool IsStartted(PROCESS_TYPE eType);
	static mp_void  ShowSvn();
};

#endif
