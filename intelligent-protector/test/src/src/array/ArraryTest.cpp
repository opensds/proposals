/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "array/ArrayTest.h"

//Begin CArrayTest
TEST_F(CArrayTest, Con_Des)
{
    CArray arr;
    EXPECT_TRUE(1);
}
TEST_F(CArrayTest, OpenDev)
{
    mp_string str = "test";
    mp_int32 iDevFd;
    mp_int32 rst;
    //open = 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, StubopenEq0);
        rst = CArray::OpenDev(str, iDevFd);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    //open < 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, StubopenLt0);
        rst = CArray::OpenDev(str, iDevFd);
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
}
TEST_F(CArrayTest, GetDiskArrayInfo)
{
    mp_string strDev = "test";
    mp_string strVendor = "test";
    mp_string strProduct = "test";
    mp_int32 rst = 0;
    //GetVendorAndProduct < 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub2(&ioctl, &StubioctlLt0);
        rst = CArray::GetDiskArrayInfo(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, -1);
    }
    //open < 0
    {
        Stub<openType, StubopenType, mp_void> mystub3(&open, &StubopenLt0);
        rst = CArray::GetDiskArrayInfo(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, MP_FAILED);
    }
    // open = 0 and GetVendorAndProduct = 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub2(&ioctl, &StubioctlEq0);
        rst = CArray::GetDiskArrayInfo(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CArrayTest, GetArrayVendorAndProduct)
{
    mp_string strDev = "test";
    mp_string strVendor = "test";
    mp_string strProduct = "test";
    mp_int32 rst = 0;
    //Exec < 0
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecLt0);
        rst = CArray::GetArrayVendorAndProduct(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, -1);
    }
    //Exec 返回空
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub2(&CRootCaller::Exec, &StubCRootCallerExecEq0);
        rst = CArray::GetArrayVendorAndProduct(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //Exec 返回OK
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub3(&CRootCaller::Exec, &StubCRootCallerExecOk);
        rst = CArray::GetArrayVendorAndProduct(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CArrayTest, GetDisk83Page)
{
    mp_string strDevice = "test";
    mp_string strLunWWN = "test";
    mp_string strLunID = "test";
    mp_int32 rst = 0;
    Stub<closeType, StubcloseType, mp_void> mystub1(&close, &StubcloseEq0);
    //open < 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, &StubopenLt0);
        rst = CArray::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //GetDiskPage < 0
    {
        Stub<openType, StubopenType, mp_void> mystub2(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub3(&ioctl, &StubioctlLt0);
        rst = CArray::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, -1);
    }
    //buff > 32
    {
        Stub<openType, StubopenType, mp_void> mystub4(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub5(&ioctl, &StubioctlEq0GetDisk83Page);
        rst = CArray::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //normal
    {
        Stub<openType, StubopenType, mp_void> mystub4(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub5(&ioctl, &StubioctlEq0Buf30);
        rst = CArray::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CArrayTest, GetDisk80Page)
{
    mp_string strDevice = "test";
    mp_string strSN = "test";
    mp_int32 rst = 0;
    Stub<closeType, StubcloseType, mp_void> mystub(&close, &StubcloseEq0);
    //open < 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, &StubopenLt0);
        rst = CArray::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //GetDiskPage < 0
    {
        Stub<openType, StubopenType, mp_void> mystub2(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub3(&ioctl, &StubioctlLt0);
        rst = CArray::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, -1);
    }
    //buff > 64
    {
        Stub<openType, StubopenType, mp_void> mystub4(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub5(&ioctl, &StubioctlEq0GetDisk80Page);
        rst = CArray::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //noraml
    {
        Stub<openType, StubopenType, mp_void> mystub4(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub5(&ioctl, &StubioctlEq0Buf30);
        rst = CArray::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CArrayTest, GetLunInfo)
{
    mp_string strDevice = "test";
    mp_string strLunWWN = "test";
    mp_string strLunID = "test";
    mp_int32 rst = 0;
    //Exec < 0
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecLt0);
        rst = CArray::GetLunInfo(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, -1);
    }
    //Exec outempty
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub2(&CRootCaller::Exec, &StubCRootCallerExecEq0);
        rst = CArray::GetLunInfo(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //Exec OK
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub3(&CRootCaller::Exec, &StubCRootCallerExecOk);
		Stub<IsSupportXXPageType, StubIsSupportXXPageType, mp_void> mystub1(&CArray::IsSupportXXPage, &StubIsSupportXXPage);
        rst = CArray::GetLunInfo(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CArrayTest, GetArraySN)
{
    mp_string strDevice = "test";
    mp_string strSN = "test";
    mp_int32 rst = 0;
    //Exec < 0
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecLt0);
        rst = CArray::GetArraySN(strDevice, strSN);
        EXPECT_EQ(rst, -1);
    }
    //Exec outempty
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub2(&CRootCaller::Exec, &StubCRootCallerExecEq0);
        rst = CArray::GetArraySN(strDevice, strSN);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //Exec OK
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub3(&CRootCaller::Exec, &StubCRootCallerExecOk);
        rst = CArray::GetArraySN(strDevice, strSN);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CArrayTest, HextoDec)
{
    mp_int32 rst = CArray::HextoDec(NULL, NULL, 1);
    EXPECT_EQ(rst, MP_FAILED);
    mp_char str[] = "test";
    mp_char tmp[] = "1a";
    CArray::HextoDec(str, tmp, 2);
    mp_char str1[] = "test";
    mp_char tmp1[] = "jk";
    rst = CArray::HextoDec(str1, tmp1, 2);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CArrayTest, HexEncode)
{
    mp_int32 des;
    mp_int32 rst = CArray::HexEncode('a', 1, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CArray::HexEncode('b', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CArray::HexEncode('c', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CArray::HexEncode('d', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CArray::HexEncode('e', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CArray::HexEncode('f', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = CArray::HexEncode('g', 0, des);
    EXPECT_EQ(rst, MP_FAILED);
}
//End CArrayTest
//Begin CDiskTest
TEST_F(CDiskTest, GetAllDiskName)
{
    vector<mp_string> vecDiskName;
    mp_int32 rst = 0;
    //ExecSystemWithEcho < 0
    {
        Stub<CSystemExecExecSystemWithEchoType, StubCSystemExecExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubCSystemExecExecSystemWithEchoLt0);
        rst = CDisk::GetAllDiskName(vecDiskName);
        EXPECT_EQ(rst, -1);
    }
    //IsCmdDevice != 0
    {
        Stub<CSystemExecExecSystemWithEchoType, StubCSystemExecExecSystemWithEchoType, mp_void> mystub2(&CSystemExec::ExecSystemWithEcho, &StubCSystemExecExecSystemWithEchoEq0IsCmdDevice);
        rst = CDisk::GetAllDiskName(vecDiskName);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}

TEST_F(CDiskTest, ClearInvalidLUNPath)
{
    vector<mp_string> vecLUNPaths;
    mp_int32 rst = CDisk::ClearInvalidLUNPath(vecLUNPaths);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, ClearInvalidLegacyDSFs)
{
    vector<mp_string> vecLUNPaths;
    mp_int32 rst = CDisk::ClearInvalidLegacyDSFs(vecLUNPaths);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, ClearInvalidPersistentDSFs)
{
    vector<mp_string> vecLUNPaths;
    mp_int32 rst = CDisk::ClearInvalidPersistentDSFs(vecLUNPaths);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, GetPersistentDSFInfo)
{
    mp_string name, path, type;
    mp_int32 rst = CDisk::GetPersistentDSFInfo(name, path, type);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, DeletePersistentDSF)
{
    mp_string name;
    mp_int32 rst = CDisk::DeletePersistentDSF(name);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, ExecuteDiskCmdEx)
{
    mp_string cmd, fname;
    vector<mp_string> dName;
    mp_int32 rst = CDisk::ExecuteDiskCmdEx(cmd, dName, fname);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, IsDskdisk)
{
    mp_string test = "test";
    mp_bool rst = CDisk::IsDskdisk(test);
    EXPECT_TRUE(rst); 
}
TEST_F(CDiskTest, GetHPRawDiskName)
{
    mp_string test = "test";
    mp_int32 rst = CDisk::GetHPRawDiskName(test, test);
    EXPECT_EQ(rst, MP_SUCCESS); 
}
TEST_F(CDiskTest, GetSecPvName)
{
    mp_string test = "test";
    mp_int32 rst = CDisk::GetSecPvName(test, test, test);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, ClearInvalidDisk)
{
    mp_int32 rst = CDisk::ClearInvalidDisk();
    EXPECT_EQ(rst, MP_SUCCESS); 
}
TEST_F(CDiskTest, GetPersistentDSFByLegacyDSF)
{
    mp_string test = "test";
    mp_int32 rst = CDisk::GetPersistentDSFByLegacyDSF(test, test);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, IsDeviceExist)
{
    mp_string test = "test";
    mp_bool rst = MP_FALSE;
    //Exec < 0
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRootCallerExecLt0);
        rst = CDisk::IsDeviceExist(test);
        EXPECT_FALSE(rst);
    }
    //Exec = 0
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub2(&CRootCaller::Exec, &StubCRootCallerExecEq0);
        rst = CDisk::IsDeviceExist(test);
        EXPECT_FALSE(rst);
    }
    //Exec ok
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub3(&CRootCaller::Exec, &StubCRootCallerExecEq0IsDeviceExist);
        rst = CDisk::IsDeviceExist(test);
        EXPECT_TRUE(rst);
    }
    //ScsiNormalizeSense ucSenseKey = 1
    {
        Stub<CRootCallerExecType, StubCRootCallerExecType, mp_void> mystub4(&CRootCaller::Exec, &StubCRootCallerExecEq0IsDeviceExist_1);
        rst = CDisk::IsDeviceExist(test);
        EXPECT_FALSE(rst);
    }
}
TEST_F(CDiskTest, GetDiskCapacity)
{
    mp_string strDevice = "test";
    mp_string strBuf = "test";
    mp_int32 rst = 0;
    //open < 0
    {
        Stub<openType, StubopenType, mp_void> mystub1(&open, &StubopenLt0);
        rst = CDisk::GetDiskCapacity(strDevice, strBuf);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //open = 0 ioctl < 0
    {
        Stub<openType, StubopenType, mp_void> mystub2(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub3(&ioctl, &StubioctlLt0);
        rst = CDisk::GetDiskCapacity(strDevice, strBuf);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //ioctl = 0
    {
        Stub<openType, StubopenType, mp_void> mystub4(&open, &StubopenEq0);
        Stub<ioctlType, StubioctlType, mp_void> mystub5(&ioctl, &StubioctlOkGetDiskCapacity);
        rst = CDisk::GetDiskCapacity(strDevice, strBuf);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CDiskTest, IsSdisk)
{
    mp_string strDevice = "test";
    mp_bool rst = CDisk::IsSdisk(strDevice);
    EXPECT_FALSE(rst);
}
TEST_F(CDiskTest, GetDiskStatus)
{
    Stub<CSystemExecExecSystemWithEchoType, StubCSystemExecExecSystemWithEchoType, mp_void> mystub(&CSystemExec::ExecSystemWithEcho, &StubCSystemExecExecSystemWithEchoOk);
    mp_string name, status;
    mp_int32 rst = CDisk::GetDiskStatus(name, status);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, IsHdisk)
{
    mp_string strDevice = "test";
    mp_bool rst = CDisk::IsHdisk(strDevice);
    EXPECT_FALSE(rst);
}
TEST_F(CDiskTest, GetDevNameByWWN)
{
    mp_string strDevice;
    mp_string strWWN = "test";
    mp_int32 rst = 0;
    //GetAllDiskName < 0
    {
        Stub<CDiskGetAllDiskNameType, StubCDiskGetAllDiskNameType, mp_void> mystub1(&CDisk::GetAllDiskName, &StubCDiskGetAllDiskNameLt0);
        rst = CDisk::GetDevNameByWWN(strDevice, strWWN);
        EXPECT_EQ(rst, -1);
    }
    //GetAllDiskName = 0
    {
        Stub<CDiskGetAllDiskNameType, StubCDiskGetAllDiskNameType, mp_void> mystub2(&CDisk::GetAllDiskName, &StubCDiskGetAllDiskNameEq0);
        rst = CDisk::GetDevNameByWWN(strDevice, strWWN);
        EXPECT_EQ(rst, ERROR_COMMON_DEVICE_NOT_EXIST);
    }
    //GetAllDiskName ok GetLunInfo < 0
    {
        Stub<CDiskGetAllDiskNameType, StubCDiskGetAllDiskNameType, mp_void> mystub3(&CDisk::GetAllDiskName, &StubCDiskGetAllDiskNameOk);
        Stub<CArrayGetArrayVendorAndProductType, StubCArrayGetArrayVendorAndProductType, mp_void> mystub4(&CArray::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        Stub<CArrayGetLunInfoType, StubCArrayGetLunInfoType, mp_void> mystub5(&CArray::GetLunInfo, &StubCArrayGetLunInfoLt0);
        rst = CDisk::GetDevNameByWWN(strDevice, strWWN);
        EXPECT_EQ(rst, -1);
    }
    //GetLunInfo ok
    {
        Stub<CDiskGetAllDiskNameType, StubCDiskGetAllDiskNameType, mp_void> mystub6(&CDisk::GetAllDiskName, &StubCDiskGetAllDiskNameOk);
        Stub<CArrayGetArrayVendorAndProductType, StubCArrayGetArrayVendorAndProductType, mp_void> mystub7(&CArray::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        Stub<CArrayGetLunInfoType, StubCArrayGetLunInfoType, mp_void> mystub8(&CArray::GetLunInfo, &StubCArrayGetLunInfoOk);
        rst = CDisk::GetDevNameByWWN(strDevice, strWWN);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
//End CDiskTest

TEST_F(CDiskTest, GetHostLunIDList)
{
    mp_string strDevice = "test";
    vector<mp_int32> vecHostLunID;
        
    mp_bool rst = CArray::GetHostLunIDList(strDevice,vecHostLunID);
    EXPECT_EQ(MP_FAILED, rst);

    Stub<openType, StubopenType, mp_void> mystub1(&open, StubopenEq0);
    CArray::GetHostLunIDList(strDevice,vecHostLunID);
    EXPECT_EQ(MP_FAILED, rst);
}

TEST_F(CDiskTest, GetDisk00Page)
{
    mp_string strDevice = "test";
    vector<mp_string> vecResult;

    mp_bool rst = CArray::GetDisk00Page(strDevice,vecResult);
    EXPECT_EQ(MP_FAILED, rst);
    
    Stub<openType, StubopenType, mp_void> mystub1(&open, StubopenEq0);
    rst = CArray::GetDisk00Page(strDevice,vecResult);
    EXPECT_EQ(MP_FAILED, rst);
}

TEST_F(CDiskTest, GetDiskC8Page)
{
    mp_string strDevice = "test";
    mp_string strLunID;

    mp_bool rst = CArray::GetDiskC8Page(strDevice,strLunID);
    EXPECT_EQ(MP_FAILED, rst);
    
    Stub<openType, StubopenType, mp_void> mystub1(&open, StubopenEq0);
    rst = CArray::GetDiskC8Page(strDevice,strLunID);
    EXPECT_EQ(MP_FAILED, rst);
}

TEST_F(CDiskTest, CheckHuaweiLUN)
{
    mp_string strVendor;
    
    mp_bool rst = CArray::CheckHuaweiLUN(strVendor);
    EXPECT_FALSE(rst);
}

#ifdef SOLARIS
TEST_F(CDiskTest, GetSolarisRawDiskName)
{
    mp_string strDiskName;
    mp_string strRawDiskName;

    mp_bool rst = CDisk::GetSolarisRawDiskName(strDiskName,strRawDiskName);
    EXPECT_FALSE(rst);
}
#endif

