/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/ThreadTest.h"

TEST_F(CMpThreadTest,Create){
    thread_id_t* id;
    thread_proc_t proc;
    mp_void* arg;
    mp_uint32 uiStackSize;
    
    CMpThread::Create(id,proc,arg,uiStackSize);
    
    Stub<pthread_attr_setstacksizeType, Stubpthread_attr_setstacksizeType, mp_void> mystub1(&pthread_attr_setstacksize, &Stubpthread_attr_setstacksize);
    Stub<pthread_createType, Stubpthread_createType, mp_void> mystub2(&pthread_create, &Stubpthread_create);
    CMpThread::Create(id,proc,arg,uiStackSize);
    
    Stub<pthread_createType, Stubpthread_createType, mp_void> mystub3(&pthread_create, &Stubpthread_create0);
    CMpThread::Create(id,proc,arg,uiStackSize);
}

TEST_F(CMpThreadTest,WaitForEnd){
    thread_id_t* id;
    mp_void** retValue;
    
    Stub<pthread_joinType, Stubpthread_joinType, mp_void> mystub1(&pthread_join, &Stubpthread_join);
    CMpThread::WaitForEnd(id,retValue);
    
    Stub<pthread_joinType, Stubpthread_joinType, mp_void> mystub2(&pthread_join, &Stubpthread_join0);
    CMpThread::WaitForEnd(id,retValue);
}

TEST_F(CMpThreadTest,InitCond){
    thread_cond_t pcond;
    
    CMpThread::InitCond(&pcond);
    
    thread_cond_t* pcond1 = NULL;
    CMpThread::InitCond(pcond1);
}

TEST_F(CMpThreadTest,DestroyCond){
    thread_cond_t pcond;
    
    Stub<pthread_cond_destroyType, Stubpthread_cond_destroyType, mp_void> mystub2(&pthread_cond_destroy, &Stubpthread_cond_destroy);
    CMpThread::DestroyCond(&pcond);
    
    thread_cond_t* pcond1 = NULL;
    CMpThread::DestroyCond(pcond1);
}

TEST_F(CMpThreadTest,TryLock){
    thread_lock_t plock;
    
    CMpThread::TryLock(&plock);
}

TEST_F(CMpThreadTest,CalcRetryInterval){
    mp_uint32 uiTimeoutInMillisecs;
    
    CMpThread::CalcRetryInterval(uiTimeoutInMillisecs);
}

TEST_F(CMpThreadTest,IsTimeout){
    mp_uint64 ulStartTime;
    mp_uint32 uiTimeoutInMillisecs;
    
    CMpThread::IsTimeout(ulStartTime,uiTimeoutInMillisecs);
}

TEST_F(CMpThreadTest,TimedLock){
    thread_lock_t plock;
    mp_uint32 uiTimeoutInMillisecs;
    
    CMpThread::TimedLock(&plock,uiTimeoutInMillisecs);
}

TEST_F(CMpThreadTest,GetThreadId){
    CMpThread::GetThreadId();
}
