/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/TimeTest.h"

TEST_F(CMpTimeTest,Now){
    mp_time* pTime = NULL;
    CMpTime::Now(pTime);
}

TEST_F(CMpTimeTest,LocalTimeR){
    mp_time* pTime = NULL;
    mp_tm* pTm = NULL;
    
    CMpTime::LocalTimeR(pTime,pTm);
}

TEST_F(CMpTimeTest,GetTimeOfDay){
    timeval tp;
    
    CMpTime::GetTimeOfDay(&tp);
}

TEST_F(CMpTimeTest,GetTimeUsec){
    
    CMpTime::GetTimeUsec();
}

TEST_F(CMpTimeTest,GetTimeSec){
    
    CMpTime::GetTimeSec();
}

TEST_F(CMpTimeTest,GetTimeString){
    mp_time pTime;
    
    CMpTime::GetTimeString(NULL);
    
    Stub<LocalTimeRType, StubLocalTimeRType, mp_void> mystub1(&CMpTime::LocalTimeR, &StubCMpTimeLocalTimeR);
    CMpTime::GetTimeString(&pTime);
}

TEST_F(CMpTimeTest,GenSeconds){
    
    CMpTime::GenSeconds();
}
