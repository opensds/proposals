/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_UDEV_H__
#define __AGENT_UDEV_H__

#ifndef WIN32
#include "common/Types.h"
#include <vector>

#define FILENAME_UDEVRULES    "99-oracle-asmdevices.rules"

class CUdev
{
public:
    CUdev()
    {
    }

    ~CUdev()
    {
    }

    mp_int32 GetUdevRulesFileName(mp_string& strUdevRulesFileName);

    mp_int32 Create(mp_string& strUdevRule, mp_string& strWWN);

    mp_int32 Delete(mp_string& strUdevRule, mp_string& strWWN);

	mp_int32 ReloadRules();

private:

};

#endif //WIN32
#endif //__AGENT_UDEV_H__

