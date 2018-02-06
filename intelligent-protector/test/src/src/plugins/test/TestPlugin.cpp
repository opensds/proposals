/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/test/TestPlugin.h"

//REGISTER_PLUGIN(CTestPlugin); //lint !e19
extern "C" AGENT_EXPORT IPlugin* QueryInterface() 
{ 
    CTestPlugin *testP = new CTestPlugin();
    return testP;
}

CTestPlugin::CTestPlugin()
{
}

CTestPlugin::~CTestPlugin()
{
}

mp_int32 CTestPlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    return MP_SUCCESS;
}

