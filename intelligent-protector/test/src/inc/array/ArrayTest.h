/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __ARRAYTEST_H__
#define __ARRAYTEST_H__

#define private public

#include "array/Array.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/RootCaller.h"
#include "common/Defines.h"
#include "common/String.h"
#include "common/UniqueId.h"
#include "common/Path.h"
#include "common/SystemExec.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CArrayTest : public testing::Test
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
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CArrayTest::m_stub;

class CDiskTest : public testing::Test
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
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CDiskTest::m_stub;

//定义函数类型
/*类成员函数的类型命名：Class名+函数名+Type后缀
 *类成员函数的Stub函数的类型命名：Stub前缀+Class名+函数名+Type后缀
 *类成员函数的Stub函数参数：是void* pthis + 原函数参数，这么写是因为，可能会改原函数传入的输出参数。静态函数与原函数参数一致。
 *静态函数、全局函数、库函数的类型命名：函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的类型命名：Stub前缀+函数名+Type后缀。
 *静态函数、全局函数、库函数的stub函数的参数：与原函数参数一致，这么写是因为，可能会改原函数传入的输出参数。
*/
typedef mp_int32 (*openType)(const mp_char* pathname, mp_int32 flags, ...);
typedef mp_int32 (*StubopenType)(const mp_char* pathname, mp_int32 flags);

typedef mp_int32 (*closeType)(mp_int32 fd);
typedef mp_int32 (*StubcloseType)(mp_int32 fd);

typedef mp_int32 (*ioctlType)(mp_int32 fd, long unsigned int request, ...);
typedef mp_int32 (*StubioctlType)(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr);

typedef mp_int32 (*CRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);

typedef mp_int32 (*CSystemExecExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);
typedef mp_int32 (*StubCSystemExecExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);

typedef mp_bool (*CDiskIsSdiskType)(mp_string& strDevice);
typedef mp_bool (*StubCDiskIsSdiskType)(mp_string& strDevice);

typedef mp_bool (*CDiskIsHdiskType)(mp_string& strDevice);
typedef mp_bool (*StubCDiskIsHdiskType)(mp_string& strDevice);

typedef mp_int32 (*CDiskGetAllDiskNameType)(vector<mp_string>& vecDiskName);
typedef mp_int32 (*StubCDiskGetAllDiskNameType)(vector<mp_string>& vecDiskName);

typedef mp_int32 (*CArrayGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);
typedef mp_int32 (*StubCArrayGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);

typedef mp_int32 (*CArrayGetLunInfoType)(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);
typedef mp_int32 (*StubCArrayGetLunInfoType)(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);

typedef mp_bool (*IsSupportXXPageType)(string page, vector<mp_string>& vecResult);
typedef mp_bool (*StubIsSupportXXPageType)(string page, vector<mp_string>& vecResult);

/* Stub 函数的取名规则：Stub+(Class名+)原函数名+需要改的结果说明(+特殊用处)
 * 比如：StubopenEq0。是用来取代open函数的，返回值为0。
 * Lt：小于    Eq：等于  Ok：有返回值和输出
 * 参数参照类型命名处说明
*/
mp_bool StubIsSupportXXPage(string page, vector<mp_string>& vecResult)
{
    return MP_TRUE;
}

mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}
mp_int32 StubopenEq0(const mp_char* pathname, mp_int32 flags)
{
    return 0;
}
mp_int32 StubopenLt0(const mp_char* pathname, mp_int32 flags)
{
    return -1;
}
mp_int32 StubcloseEq0(mp_int32 fd)
{
    return 0;
}
mp_int32 StubioctlLt0(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    return -1;
}
mp_int32 StubioctlEq0(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    return 0;
}
mp_int32 StubioctlEq0GetDisk80Page(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    *((mp_char*)(io_hdr->dxferp) + 3) = 64;
    return 0;
}
mp_int32 StubioctlEq0GetDisk83Page(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    *((mp_char*)(io_hdr->dxferp) + 7) = 32;
    return 0;
}
mp_int32 StubioctlEq0Buf30(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    *((mp_char*)(io_hdr->dxferp) + 7) = 30;
    return 0;
}
mp_int32 StubCRootCallerExecLt0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    return -1;
}
mp_int32 StubCRootCallerExecEq0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    return 0;
}
mp_int32 StubCRootCallerExecEq0IsDeviceExist(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    mp_string tmp;
    tmp[0] = 0x72;
    pvecResult->push_back(tmp);
    return 0;
}
mp_int32 StubCRootCallerExecEq0IsDeviceExist_1(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    mp_string tmp = "123";
    tmp[0] = 0x69;
    tmp[1] = 0x12;
    tmp[2] = 0x11;
    pvecResult->push_back(tmp);
    return 0;
}
mp_int32 StubCRootCallerExecOk(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (!pvecResult)
    {
        return -1;
    }
    pvecResult->push_back("huawei");
    pvecResult->push_back("rong");
    return 0;
}
mp_int32 StubCSystemExecExecSystemWithEchoLt0(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    return -1;
}
mp_int32 StubCSystemExecExecSystemWithEchoEq0(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    return 0;
}
mp_int32 StubCSystemExecExecSystemWithEchoEq0IsCmdDevice(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    strEcho.push_back("1");
    strEcho.push_back("1");
    return 0;
}

mp_int32 StubCSystemExecExecSystemWithEchoOk(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    strEcho.push_back("test");
    return 0;
}
mp_int32 StubioctlOkGetDiskCapacity(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    if (!io_hdr->dxferp)
    {
        return -1;
    }
    *((mp_char*)(io_hdr->dxferp)) = 0xff;
    *((mp_char*)(io_hdr->dxferp) + 1) = 0xff;
    *((mp_char*)(io_hdr->dxferp) + 2) = 0xff;
    *((mp_char*)(io_hdr->dxferp) + 3) = 0xff;
    return 0;
}
mp_bool StubCDiskIsSdiskEq1(mp_string& strDevice)
{
    return 1;
}
mp_bool StubCDiskIsSdiskEq0(mp_string& strDevice)
{
    return 0;
}
mp_bool StubCDiskIsHdiskEq1(mp_string& strDevice)
{
    return 1;
}
mp_bool StubCDiskIsHdiskEq0(mp_string& strDevice)
{
    return 0;
}
mp_int32 StubCDiskGetAllDiskNameLt0(vector<mp_string>& vecDiskName)
{
    return -1;
}
mp_int32 StubCDiskGetAllDiskNameEq0(vector<mp_string>& vecDiskName)
{
    return 0;
}
mp_int32 StubCDiskGetAllDiskNameOk(vector<mp_string>& vecDiskName)
{
    vecDiskName.push_back("test");
    vecDiskName.push_back("nohuawei");
    vecDiskName.push_back(ARRAY_VENDER_HUAWEI);
    vecDiskName.push_back(ARRAY_VENDER_HUAWEI);
    return 0;
}
mp_int32 StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN(mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    if (strDev == "/dev/test")
    {
        return -1;
    }
    if (strDev == "/dev/HUAWEI")
    {
        strvendor = ARRAY_VENDER_HUAWEI;
    }
    return 0;
}
mp_int32 StubCArrayGetLunInfoLt0(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    return -1;
}
mp_int32 StubCArrayGetLunInfoOk(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    strLunWWN = "test";
    return 0;
}
#endif
