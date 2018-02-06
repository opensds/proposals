/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_TIME_H__
#define __AGENT_TIME_H__

#include "common/Types.h"
#include "common/Defines.h"

#define SECOND_AND_MICROSECOND_TIMES    (1000*1000)
#define NOW_TIME_LENGTH 200

#ifndef CLOCK_MONOTONIC
#define CLOCK_ID CLOCK_REALTIME
#else
#define CLOCK_ID CLOCK_MONOTONIC
#endif

class AGENT_API CMpTime
{
public:
    static mp_void Now(mp_time* pTime);
    static mp_tm* LocalTimeR(mp_time* pTime, mp_tm* pTm); //线程安全
    static mp_void GetTimeOfDay(timeval* tp);
    static mp_uint64 GetTimeUsec();
    static mp_uint64 GetTimeSec();
    static mp_string GetTimeString(mp_time* pTime);
    static mp_double GenSeconds();
};

#endif //__AGENT_TIME_H__

