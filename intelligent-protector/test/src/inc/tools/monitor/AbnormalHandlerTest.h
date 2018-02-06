/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_ABNORMALHANDLER_TEST_H_
#define _AGENT_ABNORMALHANDLER_TEST_H_

#include "cunitpub/publicInc.h"
#include "tools/monitor/AbnormalHandler.h"

class CAbnormalHandlerTest: public testing::Test
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


#endif /* _AGENT_ABNORMALHANDLER_TEST_H_; */

