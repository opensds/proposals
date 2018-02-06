/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/UuidTest.h"

TEST_F(CUuidNumTest,GetUuidNumber){
    mp_string strUuid;
    
    CUuidNum::GetUuidNumber(strUuid);
}
