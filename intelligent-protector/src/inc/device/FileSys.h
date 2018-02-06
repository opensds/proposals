/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_FILE_SYS_H__
#define __AGENT_FILE_SYS_H__

#include <vector>
#include <list>

#include "common/Types.h"
#include "vss/requester/Requester.h"

#define SIMPLE_MARK   "/dev/sd"          //简单卷标识
#define LVM_MARK      "/dev/mapper/"     //Linux Lvm 标识
#define VXVM_MARK     "/dev/vx/dsk/"     //Linux VxVM标识

#define DEV_LENGTH    5                  // "/dev/"长度

typedef struct tag_mount_info
{
    mp_string deviceName;  //this is the volume path in windows
    mp_string mountPoint;  //this is hard-disk partition letter
    mp_int32 volumeType;   //defined in Defines.h
}mount_info_t;

typedef struct tag_umount_info
{
    mp_string deviceName;  //this is the volume path in windows
    mp_string mountPoint;  //this is hard-disk partition letter
    mp_int32 volumeType;   //defined in Defines.h
}umount_info_t;

typedef struct tag_mount_failed_info
{
    mp_string deviceName;
    mp_int32 errCode;
}mount_failed_info_t;

typedef struct tag_umount_failed_info
{
    mp_string deviceName;
    mp_int32 errCode;
}umount_failed_info_t;

typedef struct tag_file_sys_info
{
    mp_string deviceName;
    mp_string mountpoint;
    mp_string fileSysType;
    mp_int64 capacity;
    mp_int32 volType;
#ifdef WIN32
    mp_int64 offSet;
    mp_int32 diskNumber;
#endif
}file_sys_info_t;

#ifdef LINUX
typedef struct tag_linux_mount_info   //使用Linux API获取的文件系统mount信息只包含这三项
{
    mp_string deviceName;
    mp_string mountpoint;
    mp_string fileSysType;
}linux_mount_info_t;
#endif

//文件系统业务类
class CFileSys
{
private:
#ifdef WIN32    
    VSSRequester* m_pVssRequester;
#endif

public:
    CFileSys();
    ~CFileSys();

    mp_int32 QueryFileSysInfo(vector<file_sys_info_t>& vecFileInfos);
    mp_int32 Mount(mount_info_t& mountInfo);
    mp_int32 Mount(vector<mount_info_t>& vecMountInfos, vector<mount_failed_info_t>& vecFailedInfos);
    mp_int32 UMount(umount_info_t& umountInfo);
    mp_int32 UMount(vector<umount_info_t>& vecUMountInfos, vector<umount_failed_info_t>& vecFailedInfos);  
    mp_int32 Freeze(vector<mp_string>& vecDriveLetters);  
    mp_int32 UnFreeze(vector<mp_string>& vecDriveLetters);
    mp_int32 FreezeAll();
    mp_int32 UnFreezeAll();
    mp_int32 EndBackup(mp_int32 iBackupSucc);
    mp_int32 QueryFreezeState(vector<mp_string>& vecDriveLetters);

#ifdef WIN32
    mp_int32 DeleteDriveLetter(umount_info_t& driveletterinfo);
#endif
  

private:    
#ifdef WIN32
    mp_int32 CheckMountPointStatus(mp_string& strMountPoint);
    mp_int32 CheckMountedStatus(mp_string& strVolumePath, mp_string& strMountPoint, 
        mp_bool& bIsMounted);
    mp_int32 OfflineVolume(mp_string& strVolume);
    mp_int32 DeleteDriveLetter(mp_string& strDriveLetter);

#else
    mp_int32 CheckUdevDevStatus(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted);

    mp_int32 CheckDevStatus(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted);
    mp_int32 CheckMountPointStatus(mp_string& strMountPoint);
    mp_int32 CheckMountedStatus(mp_string& strDevPath, mp_string& strMountPoint, 
    mp_int32 iVolType, mp_bool& bIsMounted);

    mp_int32 GetVolumeType(mp_string strDevname, mp_int32& iVolType);
    mp_int32 GetFileSysInfos(mp_string strDevname, file_sys_info_t& fileInfo);
    mp_void GetDevNameByPartition(mp_string strPartition, mp_string& strDevName);
    mp_int32 GetDevNameByMountPoint(mp_string strMountPoint, list<mp_string>& lstDevNames);
    mp_bool CheckLocalLun(list<mp_string>& lstDevNames);
#ifdef LINUX
	mp_int32 GetAllMountInfo(vector<linux_mount_info_t> &vecMountInfo);
#endif
#endif
#if (defined HP_UX_IA) || (defined SOLARIS)
    mp_int32 QueryFileSysType(mp_string strDevFullPath, mp_string& strFileSysType);
#endif
};

#endif //__AGENT_FILE_SYS_H__

