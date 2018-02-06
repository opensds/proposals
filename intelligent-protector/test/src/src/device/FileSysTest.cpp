/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/FileSysTest.h"

TEST_F(CFileSysTest,QueryFileSysInfo){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    vector<file_sys_info_t> vecFileInfos;
    
    {
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho);
        ret = mp_CFileSys.QueryFileSysInfo(vecFileInfos);
        EXPECT_EQ(ret,ERROR_DEVICE_FILESYS_QUERY_INFO_FAILED);
    }
    
    {
        Stub<Exec0Type, StubExec0Type, mp_void> mystub2(&CRootCaller::Exec, &StubExec0);
        ret = mp_CFileSys.QueryFileSysInfo(vecFileInfos);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(CFileSysTest,Mount){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mount_info_t mountInfo;
    
    {
        Stub<CheckMountedStatusType, StubCheckMountedStatusType, mp_void> mystub1(&CFileSys::CheckMountedStatus, &StubCheckMountedStatus);
        ret = mp_CFileSys.Mount(mountInfo);
        EXPECT_EQ(ret,ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    }
    
    {
        Stub<CheckMountedStatusType, StubCheckMountedStatusType, mp_void> mystub2(&CFileSys::CheckMountedStatus, &StubCheckMountedStatus0);
        ret = mp_CFileSys.Mount(mountInfo);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<CheckMountedStatusType, StubCheckMountedStatusType, mp_void> mystub3(&CFileSys::CheckMountedStatus, &StubCheckMountedStatus1);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub4(&CRootCaller::Exec, &StubCFileSysExec0);
        ret = mp_CFileSys.Mount(mountInfo);
        EXPECT_EQ(ret,MP_SUCCESS);
    }

    {
		
        Stub<CheckMountedStatusType, StubCheckMountedStatusType, mp_void> mystub3(&CFileSys::CheckMountedStatus, &StubCheckMountedStatus1);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub5(&CRootCaller::Exec, &StubCFileSysExec);
        ret = mp_CFileSys.Mount(mountInfo);
        EXPECT_EQ(ret,ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    }
}

TEST_F(CFileSysTest,BatchMount){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mount_info_t MountInfos;
    vector<mount_info_t> vecMountInfos;
    vector<mount_failed_info_t> vecFailedInfos;
    
    vecMountInfos.push_back(MountInfos);
    ret = mp_CFileSys.Mount(vecMountInfos,vecFailedInfos);
    EXPECT_EQ(ret,MP_SUCCESS);
}

TEST_F(CFileSysTest,UMount){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    umount_info_t umountInfo;
    
    {
        Stub<CheckMountedStatusType, StubCheckMountedStatusType, mp_void> mystub1(&CFileSys::CheckMountedStatus, &StubCheckMountedStatus);
        ret = mp_CFileSys.UMount(umountInfo);
        EXPECT_EQ(ret,ERROR_DEVICE_FILESYS_UNMOUNT_FAILED);
    }
    
    {
        Stub<CheckMountedStatusType, StubCheckMountedStatusType, mp_void> mystub2(&CFileSys::CheckMountedStatus, &StubCheckMountedStatus1);
        ret = mp_CFileSys.UMount(umountInfo);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<CheckMountedStatusType, StubCheckMountedStatusType, mp_void> mystub3(&CFileSys::CheckMountedStatus, &StubCheckMountedStatus0);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub4(&CRootCaller::Exec, &StubCFileSysExec0);
        ret = mp_CFileSys.UMount(umountInfo);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub5(&CRootCaller::Exec, &StubCFileSysExec);
        ret = mp_CFileSys.UMount(umountInfo);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(CFileSysTest,BatchUMount){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    umount_info_t UMountInfos;
    vector<umount_info_t> vecUMountInfos;
    vector<umount_failed_info_t> vecFailedInfos;
    
    vecUMountInfos.push_back(UMountInfos);
    Stub<CheckMountedStatusType, StubCheckMountedStatusType, mp_void> mystub1(&CFileSys::CheckMountedStatus, &StubCheckMountedStatus);
    ret = mp_CFileSys.UMount(vecUMountInfos,vecFailedInfos);
    EXPECT_EQ(ret,MP_SUCCESS);
}

TEST_F(CFileSysTest,Freeze){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    vector<mp_string> vecDriveLetters;
    
    ret = mp_CFileSys.Freeze(vecDriveLetters);
    EXPECT_EQ(ret,ERROR_COMMON_INVALID_PARAM);
    
    vecDriveLetters.push_back("test");
    ret = mp_CFileSys.Freeze(vecDriveLetters);
    EXPECT_EQ(ret,ERROR_COMMON_FUNC_UNIMPLEMENT);
}

TEST_F(CFileSysTest,UnFreeze){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    vector<mp_string> vecDriveLetters;
    
    ret = mp_CFileSys.UnFreeze(vecDriveLetters);
    EXPECT_EQ(ret,ERROR_COMMON_INVALID_PARAM);
    
    vecDriveLetters.push_back("test");
    ret = mp_CFileSys.UnFreeze(vecDriveLetters);
    EXPECT_EQ(ret,ERROR_COMMON_FUNC_UNIMPLEMENT);
}

TEST_F(CFileSysTest,FreezeAll){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    
    ret = mp_CFileSys.FreezeAll();
    EXPECT_EQ(ret,ERROR_COMMON_FUNC_UNIMPLEMENT);
}

TEST_F(CFileSysTest,UnFreezeAll){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    
    ret = mp_CFileSys.UnFreezeAll();
    EXPECT_EQ(ret,ERROR_COMMON_FUNC_UNIMPLEMENT);
}

TEST_F(CFileSysTest,QueryFreezeState){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    vector<mp_string> vecDriveLetters;
    
    ret = mp_CFileSys.QueryFreezeState(vecDriveLetters);
    EXPECT_EQ(ret,ERROR_COMMON_FUNC_UNIMPLEMENT);
}

TEST_F(CFileSysTest,EndBackup){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mp_int32 iBackupSucc;
    
    ret = mp_CFileSys.EndBackup(iBackupSucc);
    EXPECT_EQ(ret,ERROR_COMMON_FUNC_UNIMPLEMENT);
}

TEST_F(CFileSysTest,CheckUdevDevStatus){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mp_string strDev;
    mp_string strMountPoint;
    mp_bool bIsMounted;
    {
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub1(&CMpFile::FileExist, &StubCFileExistExec0);
        ret = mp_CFileSys.CheckUdevDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_SUCCESS);
    }

    {    
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub2(&CMpFile::FileExist, &StubCFileExistExec);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub3(&CRootCaller::Exec, &StubCFileSysExec);
        ret = mp_CFileSys.CheckUdevDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,-1);
    }
    
    {
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub5(&CMpFile::FileExist, &StubCFileExistExec);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub4(&CRootCaller::Exec, &StubCFileSysExec0);
        ret = mp_CFileSys.CheckUdevDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub6(&CMpFile::FileExist, &StubCFileExistExec);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub7(&CRootCaller::Exec, &StubCFileSysExecl);
        ret = mp_CFileSys.CheckUdevDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub6(&CMpFile::FileExist, &StubCFileExistExec);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub7(&CRootCaller::Exec, &StubCFileSysExec);
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub8(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho);
        ret = mp_CFileSys.CheckUdevDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub6(&CMpFile::FileExist, &StubCFileExistExec);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub7(&CRootCaller::Exec, &StubCFileSysExecl);
	  	Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub8(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEchol);
		ret = mp_CFileSys.CheckUdevDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub6(&CMpFile::FileExist, &StubCFileExistExec);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub7(&CRootCaller::Exec, &StubCFileSysExecl);
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub8(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho0);
        strMountPoint = "test";
        ret = mp_CFileSys.CheckUdevDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(CFileSysTest,CheckDevStatus){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mp_string strDev;
    mp_string strMountPoint;
    mp_bool bIsMounted;


	{
        Stub<GetAllMountInfoType,StubGetAllMountInfoType,mp_void>mystub1(&CFileSys::GetAllMountInfo,&StubGetAllMountInfo);
        ret = mp_CFileSys.CheckDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_FAILED);
    }
	
	
    
    {
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho);
        ret = mp_CFileSys.CheckDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub2(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho0);
        strMountPoint = "test";
        ret = mp_CFileSys.CheckDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_SUCCESS);
    }

	{
		Stub<GetAllMountInfoType,StubGetAllMountInfoType,mp_void> mystub1(&CFileSys::GetAllMountInfo,&StubGetAllMountInfo0);
        strMountPoint = "ltest";
		Stub<CheckUdevDevStatusType, StubCheckUdevDevStatusType, mp_void> mystub2(&CFileSys::CheckUdevDevStatus, &StubCheckUdevDevStatus);
        ret = mp_CFileSys.CheckDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_FAILED);
    }

	{
		Stub<GetAllMountInfoType,StubGetAllMountInfoType,mp_void> mystub1(&CFileSys::GetAllMountInfo,&StubGetAllMountInfo0);
        strMountPoint = "ltest";
		Stub<CheckUdevDevStatusType, StubCheckUdevDevStatusType, mp_void> mystub2(&CFileSys::CheckUdevDevStatus, &StubCheckUdevDevStatus0);
        ret = mp_CFileSys.CheckDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_FAILED);
    }


	
    {
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub3(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEchol);
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub1(&CMpFile::FileExist, &StubCFileExistExec0);
        ret = mp_CFileSys.CheckDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub4(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEchol);
        Stub<CFileFileExistType, StubCFileExistExecType, mp_void> mystub5(&CMpFile::FileExist, &StubCFileExistExec);
        Stub<CFileSysExecType, StubCFileSysExecType, mp_void> mystub6(&CRootCaller::Exec, &StubCFileSysExec);
        ret = mp_CFileSys.CheckDevStatus(strDev,strMountPoint,bIsMounted);
        EXPECT_EQ(ret,MP_FAILED);
    }
}

TEST_F(CFileSysTest,CheckMountPointStatus){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mp_string strMountPoint;  
 	{
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub4(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEchol);
        ret = mp_CFileSys.CheckMountPointStatus(strMountPoint);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
	
    {
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub4(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEchol);
        ret = mp_CFileSys.CheckMountPointStatus(strMountPoint);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {	
		
		Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub4(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEchol);
        ret = mp_CFileSys.CheckMountPointStatus(strMountPoint);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(CFileSysTest,CheckMountedStatus){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mp_string strDevPath;
    mp_string strMountPoint;
    mp_int32 iVolType;
    mp_bool bIsMounted;
    
    {
        
		Stub<CheckDevStatusType, StubCheckDevStatusType, mp_void> mystub1(&CFileSys::CheckDevStatus, &StubCheckDevStatus);
		ret = mp_CFileSys.CheckMountedStatus(strDevPath,strMountPoint,iVolType,bIsMounted);
        EXPECT_EQ(ret,MP_FAILED);
    }

	{
        
		Stub<CheckDevStatusType, StubCheckDevStatusType, mp_void> mystub1(&CFileSys::CheckDevStatus, &StubCheckDevStatus0);
		ret = mp_CFileSys.CheckMountedStatus(strDevPath,strMountPoint,iVolType,bIsMounted);
        EXPECT_EQ(ret,ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST);
    }


}

TEST_F(CFileSysTest,GetVolumeType){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mp_string strDevname;
    mp_int32 iVolType;
    
    {
        strDevname = "/dev/sd";
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho);
        ret = mp_CFileSys.GetVolumeType(strDevname,iVolType);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        strDevname = "/dev/sd";
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho0);
        ret = mp_CFileSys.GetVolumeType(strDevname,iVolType);
        EXPECT_EQ(-1,MP_FAILED);
    }
    
    {
        strDevname = "/dev/mapper/";
        ret = mp_CFileSys.GetVolumeType(strDevname,iVolType);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    
    {
        strDevname = "/dev/vx/dsk/";
        ret = mp_CFileSys.GetVolumeType(strDevname,iVolType);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(CFileSysTest,GetFileSysInfos){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mp_string strDevname;
    file_sys_info_t fileInfo;
    
    {
        strDevname = "/dev/sd";
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho);
        ret = mp_CFileSys.GetFileSysInfos(strDevname,fileInfo);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        strDevname = "/dev/mapper/";
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho0);
        ret = mp_CFileSys.GetFileSysInfos(strDevname,fileInfo);
        EXPECT_EQ(0,MP_SUCCESS);
    }
    
    {
        strDevname = "/dev/mapper/";
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho0);
        ret = mp_CFileSys.GetFileSysInfos(strDevname,fileInfo);
        EXPECT_EQ(ret,MP_FAILED);
    }
}

TEST_F(CFileSysTest,GetDevNameByPartition){
    CFileSys mp_CFileSys;
    mp_string strDevname;
    mp_string strPartition;
    
    {
        strPartition = "1234567890";
        mp_CFileSys.GetDevNameByPartition(strPartition,strDevname);
    }
    
    {
        strPartition = "/dev/mapper/";
        mp_CFileSys.GetDevNameByPartition(strPartition,strDevname);
    }
}

TEST_F(CFileSysTest,GetDevNameByMountPoint){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    mp_string strMountPoint;
    list<mp_string> lstDevNames;
    
    {
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho);
        ret = mp_CFileSys.GetDevNameByMountPoint(strMountPoint,lstDevNames);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        Stub<ExecSystemWithEchoType, StubExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubExecSystemWithEcho);
        ret = mp_CFileSys.GetDevNameByMountPoint(strMountPoint,lstDevNames);
        EXPECT_EQ(ret,MP_FAILED);
    }
}

TEST_F(CFileSysTest,CheckLocalLun){
    mp_int32 ret;
    CFileSys mp_CFileSys;
    list<mp_string> lstDevNames;
    lstDevNames.push_back("test");
    
    {
        Stub<GetArrayVendorAndProductType, StubGetArrayVendorAndProductType, mp_void> mystub1(&CArray::GetArrayVendorAndProduct, &StubGetArrayVendorAndProduct);
        ret = mp_CFileSys.CheckLocalLun(lstDevNames);
        EXPECT_EQ(ret,MP_FALSE);
    }
    
    {
        Stub<GetArrayVendorAndProductType, StubGetArrayVendorAndProductType, mp_void> mystub1(&CArray::GetArrayVendorAndProduct, &StubGetArrayVendorAndProductl);
        ret = mp_CFileSys.CheckLocalLun(lstDevNames);
        EXPECT_EQ(ret,MP_FALSE);
    }
    
    {
        Stub<GetArrayVendorAndProductType, StubGetArrayVendorAndProductType, mp_void> mystub1(&CArray::GetArrayVendorAndProduct, &StubGetArrayVendorAndProduct0);
        ret = mp_CFileSys.CheckLocalLun(lstDevNames);
        EXPECT_EQ(ret,MP_FALSE);
    }
}
