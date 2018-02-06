/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/RawTest.h"

TEST_F(CRawTest,Create){
    CRaw m_CRaw;
    mp_int32 ret;
    raw_info_t rawInfo;

    {
        ret = m_CRaw.Create(rawInfo);
        EXPECT_EQ(ret,1073949050);
    }
    
    {
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec0);
        Stub<CRawExistType, StubCRawExistType, mp_void> mystub2(&CMpFile::FileExist, &StubCRawExist);
        m_CRaw.Create(rawInfo);
        EXPECT_EQ(ret,1073949050);
    }
    
    {
        rawInfo.strDevName = "test";
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec0);
        Stub<CRawExistType, StubCRawExistType, mp_void> mystub2(&CMpFile::FileExist, &StubCRawExist);
        Stub<GetDeviceUsedByRawType, StubGetDeviceUsedByRawType, mp_void> mystub3(&CRaw::GetDeviceUsedByRaw, &StubGetDeviceUsedByRawt);
        m_CRaw.Create(rawInfo);
        EXPECT_EQ(ret,1073949050);
    }
    
    {
        rawInfo.strDevName = "test";
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec0);
        Stub<CRawExistType, StubCRawExistType, mp_void> mystub2(&CMpFile::FileExist, &StubCRawExist);
        Stub<GetDeviceUsedByRawType, StubGetDeviceUsedByRawType, mp_void> mystub3(&CRaw::GetDeviceUsedByRaw, &StubGetDeviceUsedByRaw);
        m_CRaw.Create(rawInfo);
        EXPECT_EQ(ret,1073949050);
    }
    
    {
        rawInfo.strDevName = "test";
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec0);
        Stub<CRawExistType, StubCRawExistType, mp_void> mystub2(&CMpFile::FileExist, &StubCRawExist);
        Stub<GetDeviceUsedByRawType, StubGetDeviceUsedByRawType, mp_void> mystub3(&CRaw::GetDeviceUsedByRaw, &StubGetDeviceUsedByRaw0);
        m_CRaw.Create(rawInfo);
    }
    
    {
        rawInfo.strDevName = "123456789";
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec0);
        Stub<CRawExistType, StubCRawExistType, mp_void> mystub2(&CMpFile::FileExist, &StubCRawExist0);
        m_CRaw.Create(rawInfo);
        EXPECT_EQ(ret,1073949050);
    }
    
    {
        rawInfo.strDevName = "123456789";
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec0);
        Stub<CRawExistType, StubCRawExistType, mp_void> mystub2(&CMpFile::FileExist, &StubCRawExist0);
        Stub<GetDevNameByWWNType, StubGetDevNameByWWNType, mp_void> mystub3(&CDisk::GetDevNameByWWN, &StubGetDevNameByWWN);
        m_CRaw.Create(rawInfo);
    }
}

TEST_F(CRawTest,Delete){
    CRaw m_CRaw;
    mp_int32 ret;
    raw_info_t rawInfo;

    {
        Stub<CRawExistType, StubCRawExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCRawExist);
        m_CRaw.Delete(rawInfo);
        EXPECT_EQ(0,MP_SUCCESS);
    }
    
    {
        Stub<CRawExistType, StubCRawExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCRawExist0);
        m_CRaw.Delete(rawInfo);
        EXPECT_EQ(0,MP_SUCCESS);
    }
}

TEST_F(CRawTest,StartRawService){
    CRaw m_CRaw;
    mp_int32 ret;

    {
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec);
        m_CRaw.StartRawService();
    }
    
    {
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec0);
        m_CRaw.StartRawService();
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(CRawTest,GetDeviceUsedByRaw){
    CRaw m_CRaw;
    mp_string strRawDevPath;
    mp_string strUsedDevName;

    {
        m_CRaw.GetDeviceUsedByRaw(strRawDevPath,strUsedDevName);
    }
    
    {
        Stub<GetBoundedDevVersionsType, StubGetBoundedDevVersionsType, mp_void> mystub1(&CRaw::GetBoundedDevVersions, &StubGetBoundedDevVersions);
        Stub<GetFolderFileType, StubGetFolderFileType, mp_void> mystub2(&CMpFile::GetFolderFile, &StubGetFolderFile);
        m_CRaw.GetDeviceUsedByRaw(strRawDevPath,strUsedDevName);
    }
    
    {
        Stub<GetBoundedDevVersionsType, StubGetBoundedDevVersionsType, mp_void> mystub1(&CRaw::GetBoundedDevVersions, &StubGetBoundedDevVersions);
        Stub<GetFolderFileType, StubGetFolderFileType, mp_void> mystub2(&CMpFile::GetFolderFile, &StubGetFolderFile0);
        m_CRaw.GetDeviceUsedByRaw(strRawDevPath,strUsedDevName);
    }
    
    {
        Stub<GetBoundedDevVersionsType, StubGetBoundedDevVersionsType, mp_void> mystub1(&CRaw::GetBoundedDevVersions, &StubGetBoundedDevVersions);
        Stub<GetFolderFileType, StubGetFolderFileType, mp_void> mystub2(&CMpFile::GetFolderFile, &StubGetFolderFilet);
        Stub<GetDeviceNumberType, StubGetDeviceNumberType, mp_void> mystub3(&CRaw::GetDeviceNumber, &StubGetDeviceNumber);
        m_CRaw.GetDeviceUsedByRaw(strRawDevPath,strUsedDevName);
    }
    
    {
        Stub<GetBoundedDevVersionsType, StubGetBoundedDevVersionsType, mp_void> mystub1(&CRaw::GetBoundedDevVersions, &StubGetBoundedDevVersions);
        Stub<GetFolderFileType, StubGetFolderFileType, mp_void> mystub2(&CMpFile::GetFolderFile, &StubGetFolderFilet);
        Stub<GetDeviceNumberType, StubGetDeviceNumberType, mp_void> mystub3(&CRaw::GetDeviceNumber, &StubGetDeviceNumber0);
        m_CRaw.GetDeviceUsedByRaw(strRawDevPath,strUsedDevName);
    }
}

TEST_F(CRawTest,GetBoundedDevVersions){
    CRaw m_CRaw;
    mp_string strRawDevPath;
    mp_int32 iBoundMajor;
    mp_int32 iBoundMinor;

    {
        m_CRaw.GetBoundedDevVersions(strRawDevPath,iBoundMajor,iBoundMinor);
    }
}

TEST_F(CRawTest,GetDeviceNumber){
    CRaw m_CRaw;
    mp_string rstrDeviceName;
    mp_int32 iMajor;
    mp_int32 iMinor;

    {
        m_CRaw.GetDeviceNumber(rstrDeviceName,iMajor,iMinor);
    }
}

TEST_F(CRawTest,IsDeviceExists){
    CRaw m_CRaw;
    mp_int32 ret;
    mp_string rstrDeviceName;
    mp_bool isDeviceExist;

    {
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec);
        m_CRaw.IsDeviceExists(rstrDeviceName,isDeviceExist);
        EXPECT_EQ(ret,0);
    }
    
    {
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExecl);
        m_CRaw.IsDeviceExists(rstrDeviceName,isDeviceExist);
    }
}

TEST_F(CRawTest,CreateDeviceByMknod){
    CRaw m_CRaw;
    mp_int32 ret;
    mp_string strDeviceName;
    mp_string strMajorNum;
    mp_string strMinorNum;

    {
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec);
        ret = m_CRaw.CreateDeviceByMknod(strDeviceName,strMajorNum,strMinorNum);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        Stub<CRawExecType, StubCRawExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCRawExec0);
        ret = m_CRaw.CreateDeviceByMknod(strDeviceName,strMajorNum,strMinorNum);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}
