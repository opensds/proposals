/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_LOG_TEST_H__
#define __AGENT_LOG_TEST_H__

#define private public

#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

class CLoggerTest: public testing::Test{
protected:
};

//*******************************************************************************
typedef mp_void (CLogger::*ReadLevelAndCountType)();
typedef mp_void (*StubReadLevelAndCountType)();
//*******************************************************************************
mp_void StubReadLevelAndCount(){
    return;
}

#endif
