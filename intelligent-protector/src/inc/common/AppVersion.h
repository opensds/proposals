/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_APP_VERSION_
#define _AGENT_APP_VERSION_

#include "common/Types.h"

#define AGENT_PACKAGE_VERSION  "V100R001C00"
#define AGENT_VERSION          "V100R001C00"
#define RD_PROVIDER_VERSION    L"1.0.0.0"
#define COMPILE_TIME           "compile"

//变更规则"1.0.0" :
//1.每次V或者R版本号的变更，第一个数字加1.
//2.每次C版本号的变更，第二个数字加1.  
//3.每次交互版本时，第三个数字加1.
#define AGENT_BUILD_NUM        "1.0.0"

inline void AgentVersion()
{
    printf("Copyright 2017-2018 Huawei Technologies Co., Ltd.\n");
    printf("Version     : %s\n", AGENT_VERSION);
    printf("Build Number: %s\n", AGENT_BUILD_NUM);
}

#endif //_AGENT_APP_VERSION_

