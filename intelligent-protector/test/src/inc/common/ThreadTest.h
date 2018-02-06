/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _THREADTEST_H_
#define _THREADTEST_H_

#define private public

#include "common/Thread.h"
#include "common/Time.h"
#include "common/Utils.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCMpThreadVoid(mp_void* pthis);

class CMpThreadTest: public testing::Test{
protected:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCMpThreadVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CMpThreadTest::m_stub;

//*******************************************************************************
typedef mp_int32 (*pthread_joinType)(pthread_t thread, void **value_ptr);
typedef mp_int32 (*Stubpthread_joinType)(pthread_t thread, void **value_ptr);

typedef mp_int32 (*pthread_attr_setstacksizeType)(pthread_attr_t *attr, size_t stacksize);
typedef mp_int32 (*Stubpthread_attr_setstacksizeType)(pthread_attr_t *attr, size_t stacksize);

typedef mp_int32 (*pthread_createType)(pthread_t *restrictthread,const pthread_attr_t *restrictattr,void *(*start_routine)(void*), void *restrictarg);
typedef mp_int32 (*Stubpthread_createType)(pthread_t *restrictthread,const pthread_attr_t *restrictattr,void *(*start_routine)(void*), void *restrictarg);

typedef mp_int32 (*pthread_cond_destroyType)(pthread_cond_t *cond);
typedef mp_int32 (*Stubpthread_cond_destroyType)(pthread_cond_t *cond);
//*******************************************************************************
mp_void StubCMpThreadVoid(mp_void* pthis){
    return;
}

mp_int32 Stubpthread_join(pthread_t thread, void **value_ptr){
    return 1;
}

mp_int32 Stubpthread_join0(pthread_t thread, void **value_ptr){
    return 0;
}

mp_int32 Stubpthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize){
    return 0;
}

mp_int32 Stubpthread_create(pthread_t *restrictthread,const pthread_attr_t *restrictattr,void *(*start_routine)(void*), void *restrictarg){
    return 1;
}

mp_int32 Stubpthread_create0(pthread_t *restrictthread,const pthread_attr_t *restrictattr,void *(*start_routine)(void*), void *restrictarg){
    return 0;
}

mp_int32 Stubpthread_cond_destroy(pthread_cond_t *cond){
    int i = 0;
}

#endif
