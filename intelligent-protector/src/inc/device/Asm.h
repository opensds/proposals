/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_ASM_H__
#define __AGENT_ASM_H__

#ifndef WIN32
#include "common/Types.h"

class CAsm
{
public:
    CAsm();
    ~CAsm();

    mp_int32 AsmLibScan();
    
private:
    
};

#endif //WIN32
#endif //__AGENT_ASM_H__

