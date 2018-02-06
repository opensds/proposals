/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Time.h"
#include "common/Log.h"
/*------------------------------------------------------------ 
Description  : 获取当前时间
Input        : 
Output       : pTime -- 当前时间
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CMpTime::Now(mp_time* pTime)
{
    if (NULL == pTime)
    {
        return;
    }

    time(pTime);
}
/*------------------------------------------------------------ 
Description  :将时间秒数转化为本地时间
Input        : 
Output       :
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_tm* CMpTime::LocalTimeR(mp_time* pTime, mp_tm* pTm)
{
    if (NULL == pTime || NULL == pTm)
    {
        return NULL;
    }

#ifdef WIN32
    localtime_s(pTm, pTime);
#else
    localtime_r(pTime, pTm);
#endif

    return pTm;
}
/*------------------------------------------------------------ 
Description  :精确 获取当前时间
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CMpTime::GetTimeOfDay(timeval* tp)
{
#ifdef WIN32
    LARGE_INTEGER liQPF;     //CPU Frequency
    LARGE_INTEGER lPerformanceCount;

    QueryPerformanceFrequency(&liQPF);     //get cpu Frequency
    //retrieves the current value of the high-resolution performance counter
    QueryPerformanceCounter(&lPerformanceCount);

    //calc time (sec + usec)
    tp->tv_sec  = (mp_uint32) (lPerformanceCount.QuadPart/liQPF.QuadPart);
    tp->tv_usec = (mp_uint32)(((lPerformanceCount.QuadPart - liQPF.QuadPart * tp->tv_sec ) * SECOND_AND_MICROSECOND_TIMES ) / liQPF.QuadPart);

    return;
#else
    (mp_void)gettimeofday(tp, NULL);
    return;
#endif
}
/*------------------------------------------------------------ 
Description  : 获取当前时间(us级)
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_uint64 CMpTime::GetTimeUsec()
{
    struct timeval now;
    mp_uint64 now_usec;
    GetTimeOfDay(&now);

    now_usec = (mp_uint64)now.tv_sec * SECOND_AND_MICROSECOND_TIMES + (mp_uint64)now.tv_usec;

    return now_usec;
}

/*------------------------------------------------------------
Function Name: GetTimeSec
Description  : 获取当前时间（从系统启动开始计时），不受用户更改系统时间的影响，单位是秒
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_uint64 CMpTime::GetTimeSec()
{
#ifdef WIN32
    LARGE_INTEGER proc_freq;
    LARGE_INTEGER proc_counter;
    if (!QueryPerformanceFrequency(&proc_freq))
    {
        return 0;
    }
    if (!QueryPerformanceCounter(&proc_counter))
    {
         return 0;
    }

    return long(proc_counter.QuadPart / proc_freq.QuadPart);
#else
    struct timespec nowspec;
    clock_gettime(CLOCK_ID, &nowspec);
    return mp_uint64(nowspec.tv_sec);
#endif
}
/*------------------------------------------------------------ 
Description  : 获取当前时间并格式化输出
Input        : 
Output       : pTime -- 当前时间
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_string CMpTime::GetTimeString(mp_time* pTime)
{
    if (NULL == pTime)
    {
        return "";
    }

    mp_tm stTime;
    mp_char acTime[NOW_TIME_LENGTH] = {0};
    mp_tm *stLocalTime = LocalTimeR(pTime, &stTime);
    if (NULL == stLocalTime)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "stLocalTime is NULL.");
        return "";
    }

    strftime(acTime, sizeof(acTime), "%Y-%m-%d %X", stLocalTime);
    return mp_string(acTime);
}

/*------------------------------------------------------------
Function Name: GenSeconds
Description  : 获取从今天0时开始的时间秒
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_double CMpTime::GenSeconds()
{
    mp_time now;
    struct tm beginTime;
    struct tm* pTime = NULL;

    time(&now);
    pTime = localtime(&now);
    if (NULL == pTime)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get localtime failed.");
        return MP_FAILED;
    }

    beginTime = *pTime;
    beginTime.tm_hour = 0;
    beginTime.tm_min = 0;
    beginTime.tm_sec = 0;
    beginTime.tm_mon = 0;
    beginTime.tm_mday = 1;

    return difftime(now, mktime(&beginTime));
}


