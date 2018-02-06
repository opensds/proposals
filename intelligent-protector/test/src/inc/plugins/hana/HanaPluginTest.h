/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_HANA_PLUGIN_TEST_H_
#define _AGENT_HANA_PLUGIN_TEST_H_

#include "cunitpub/publicInc.h"
#include "plugins/hana/HanaPlugin.h"

class CHanaPluginTest: public testing::Test
{
public:
    static void SetUpTestCase(void)
    {
        init_cunit_data();
    }

    static void TearDownTestCase(void)
    { 
        destroy_cunit_data();
    }
protected:
    virtual void SetUp()
    { 
        reset_cunit_counter();
    }
    virtual void TearDown()
    {
        ;
    }  
};


#endif /* _AGENT_HANA_PLUGIN_TEST_H_; */

