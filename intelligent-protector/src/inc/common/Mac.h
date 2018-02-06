/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_MAC_H__
#define __AGENT_MAC_H__

#include <vector>
#include <iostream>
#include <algorithm>
#include "common/Types.h"
#include "common/Defines.h"

#define BUF_LEN   256
#define MAXINTERFACES 16

class AGENT_API CMacAddr
{
public:
    //获取所有的Mac地址
    static mp_int32 GetAllLocalMacAddr(vector<mp_string>& mac);

private:
    CMacAddr();
    CMacAddr& operator=(const CMacAddr& rhs);
    CMacAddr(const CMacAddr& rhs);

    static mp_void SortMac(vector<mp_string>& mac);
};

#endif //__AGENT_MAC_H__

