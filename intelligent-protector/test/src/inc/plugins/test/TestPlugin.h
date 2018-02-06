/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_TEST_PLUGIN_H__
#define __AGENT_TEST_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"

class CTestPlugin : public CServicePlugin
{
    
public:
    CTestPlugin();
    ~CTestPlugin();

    mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp);
};

#endif

