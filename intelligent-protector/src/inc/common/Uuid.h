/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_UUID_H__
#define __AGENT_UUID_H__
#ifndef AIX53

#include <vector>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Objbase.h>
#include <Guiddef.h>
#elif (defined SOLARIS) || (defined LINUX)
#include <uuid/uuid.h>
#elif defined(HP_UX_IA)
#include <dce/uuid.h>
#else
//AIX
#include <uuid.h>
#endif

#include "common/Types.h"
#include "common/Defines.h"
#include "common/Log.h"


#ifdef WIN32
typedef GUID           mp_uuid;
#else
typedef uuid_t         mp_uuid;
#endif

class AGENT_API CUuidNum
{
public:
    static mp_int32 GetUuidNumber(mp_string &strUuid);

private:
    static mp_int32 FormatUuid(mp_uuid uuid, mp_string &strUuid);
};
#endif

#endif //__AGENT_UUID_H__
