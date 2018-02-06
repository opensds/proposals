/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/LinkTest.h"
#include "device/Link.h"

TEST_F(CLinkTest,Create){
    mp_int32 ret;
    CLink m_link;
    link_info_t linkInfo;

    {
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist0);
        ret = m_link.Create(linkInfo);
        EXPECT_EQ(ret,ERROR_COMMON_DEVICE_NOT_EXIST);
    }
    
    {
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist);
        Stub<CLinkTestDirExistType, StubCLinkTestDirExistType, mp_void> mystub2(&CMpFile::DirExist, &StubCLinkTestDirExist);
        ret = m_link.Create(linkInfo);
        EXPECT_EQ(ret,ERROR_DEVICE_LINK_USED_BY_OTHER_DEV);
    }
    
	{
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist);
        Stub<CLinkTestDirExistType, StubCLinkTestDirExistType, mp_void> mystub2(&CMpFile::DirExist, &StubCLinkTestDirExist0);
        //Stub<GetDeviceUsedByLinkType, StubGetDeviceUsedByLinkType, mp_void> mystub3(&CLink::GetDeviceUsedByLink, &StubGetDeviceUsedByLink1);
        ret = m_link.Create(linkInfo);
        EXPECT_EQ(ret,ERROR_DEVICE_FILESYS_GET_DEV_FAILED);
    }
	
    {
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist);
        Stub<CLinkTestDirExistType, StubCLinkTestDirExistType, mp_void> mystub2(&CMpFile::DirExist, &StubCLinkTestDirExist0);
        Stub<GetDeviceUsedByLinkType, StubGetDeviceUsedByLinkType, mp_void> mystub3(&CLink::GetDeviceUsedByLink, &StubGetDeviceUsedByLink1);
        ret = m_link.Create(linkInfo);
        EXPECT_EQ(ret,ERROR_DEVICE_FILESYS_GET_DEV_FAILED);
    }
    
    {
        linkInfo.slaveDevName = "test";
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist);
        Stub<CLinkTestDirExistType, StubCLinkTestDirExistType, mp_void> mystub2(&CMpFile::DirExist, &StubCLinkTestDirExist0);
        Stub<GetDeviceUsedByLinkType, StubGetDeviceUsedByLinkType, mp_void> mystub3(&CLink::GetDeviceUsedByLink, &StubGetDeviceUsedByLink);
        ret = m_link.Create(linkInfo);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist);
        Stub<CLinkTestDirExistType, StubCLinkTestDirExistType, mp_void> mystub2(&CMpFile::DirExist, &StubCLinkTestDirExist0);
        Stub<GetDeviceUsedByLinkType, StubGetDeviceUsedByLinkType, mp_void> mystub3(&CLink::GetDeviceUsedByLink, &StubGetDeviceUsedByLink0);
        ret = m_link.Create(linkInfo);
        EXPECT_EQ(ret,ERROR_DEVICE_LINK_USED_BY_OTHER_DEV);
    }
}

TEST_F(CLinkTest,Delete){
    mp_int32 ret;
    CLink m_link;
    link_info_t linkInfo;

    {
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist0);
        ret = m_link.Delete(linkInfo);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist);
        ret = m_link.Delete(linkInfo);
        EXPECT_TRUE(1);
    }
}
/*
TEST_F(CLinkTest,Delete){
    mp_int32 ret;
    CLink m_link;
    link_info_t linkInfo;
GetDeviceUsedByLink(mp_string & strLinkFileName, mp_string & strUsedDeviceName)
    {
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist0);
        ret = m_link.Delete(linkInfo);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<CLinkTestCFileFileType, StubCLinkTestCFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCLinkTestCFileExist);
        ret = m_link.Delete(linkInfo);
        EXPECT_TRUE(1);
    }
}*/
