/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __RESTTEST_H__
#define __RESTTEST_H__

#define private public
#include "rest/MessageProcess.h"
#include "rest/HttpCGI.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CHttpRequestTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
    
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CHttpRequestTest::m_stub;


class CHttpResponseTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CHttpResponseTest::m_stub;

class CMessage_BlockTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CMessage_BlockTest::m_stub;

class CRequestURLTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CRequestURLTest::m_stub;

class CRequestMsgBodyTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CRequestMsgBodyTest::m_stub;

class CRequestMsgTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CRequestMsgTest::m_stub;

class CResponseMsgTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CResponseMsgTest::m_stub;

class CUrlUtilsTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CUrlUtilsTest::m_stub;

class CJsonUtilsTest: public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }    
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CJsonUtilsTest::m_stub;

//定义函数类型
/*类成员函数的类型命名：Class名+函数名+Type后缀
 *类成员函数的Stub函数的类型命名：Stub前缀+Class名+函数名+Type后缀
 *类成员函数的Stub函数参数：是void* pthis + 原函数参数，这么写是因为，可能会改原函数传入的输出参数。静态函数与原函数参数一致。
 *静态函数、全局函数、库函数的类型命名：函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的类型命名：Stub前缀+函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的参数：与原函数参数一致，这么写是因为，可能会改原函数传入的输出参数。
*/
typedef mp_char* (*FCGX_GetParamType)(const mp_char *name, FCGX_ParamArray envp);
typedef mp_char* (*StubFCGX_GetParamType)(const mp_char *name, FCGX_ParamArray envp);

typedef mp_int32 (*FCGX_GetCharType)(FCGX_Stream *stream);
typedef mp_int32 (*StubFCGX_GetCharType)(FCGX_Stream *stream);

typedef mp_int32 (*FCGX_GetStrType)(mp_char *str, mp_int32 n, FCGX_Stream *stream);
typedef mp_int32 (*StubFCGX_GetStrType)(mp_char *str, mp_int32 n, FCGX_Stream *stream);

typedef mp_char* (*FCGX_GetLineType)(mp_char *str, mp_int32 n, FCGX_Stream *stream);
typedef mp_char* (*StubFCGX_GetLineType)(mp_char *str, mp_int32 n, FCGX_Stream *stream);

typedef mp_int32 (*FCGX_PutCharType)(mp_int32 c, FCGX_Stream *stream);
typedef mp_int32 (*StubFCGX_PutCharType)(mp_int32 c, FCGX_Stream *stream);

typedef mp_int32 (*FCGX_PutStrType)(const mp_char *str, mp_int32 n, FCGX_Stream *stream);
typedef mp_int32 (*StubFCGX_PutStrType)(const mp_char *str, mp_int32 n, FCGX_Stream *stream);

typedef mp_int32 (*FCGX_VFPrintFType)(FCGX_Stream *stream, const mp_char *format, va_list arg);
typedef mp_int32 (*StubFCGX_VFPrintFType)(FCGX_Stream *stream, const mp_char *format, va_list arg);

typedef mp_void (*FCGX_Finish_rType)(FCGX_Request *request);
typedef mp_void (*StubFCGX_Finish_rType)(FCGX_Request *request);

/* Stub 函数的取名规则：Stub+(Class名+)原函数名+需要改的结果说明(+特殊用处)
 * 比如：StubopenEq0。是用来取代open函数的，返回值为0。
 * Lt：小于    Eq：等于  Ok：有返回值和输出
 * 参数参照类型命名处说明
*/
mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}
mp_char* StubFCGX_GetParamOk(const mp_char *name, FCGX_ParamArray envp)
{
    static mp_char rst[] = "2";
    return rst;
}
mp_int32 StubFCGX_GetCharTypeEq0(FCGX_Stream *stream)
{
    return 0;
}
mp_int32 StubFCGX_GetStrEq0(mp_char *str, mp_int32 n, FCGX_Stream *stream)
{
    return 0;
}
mp_char* StubFCGX_GetLineOk(mp_char *str, mp_int32 n, FCGX_Stream *stream)
{
    static mp_char rst[] = "2";
    return rst;
}
mp_int32 StubFCGX_PutCharEq0(mp_int32 c, FCGX_Stream *stream)
{
    return 0;
}
mp_int32 StubFCGX_PutStrEq0(const mp_char *str, mp_int32 n, FCGX_Stream *stream)
{
    return 0;
}
mp_int32 StubFCGX_VFPrintFEq0(FCGX_Stream *stream, const mp_char *format, va_list arg)
{
    return 0;
}
mp_void StubFCGX_Finish_rVoid(FCGX_Request *request)
{
    return;
}
#endif
