/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_LINK_H__
#define __AGENT_LINK_H__

#ifndef WIN32
#include "common/Types.h"

typedef struct tag_link_info
{
    mp_string wwn;
    mp_string slaveDevName;
    mp_string softLinkName;
}link_info_t;

class CLink
{
public:
    CLink();
    ~CLink();

    mp_int32 Create(link_info_t& linkInfo);
    mp_int32 Delete(link_info_t& linkInfo);
private:
    mp_int32 GetDeviceUsedByLink(mp_string & strLinkFileName, mp_string & strUsedDeviceName);
};
#endif //WIN32
#endif //__AGENT_LINK_H__

