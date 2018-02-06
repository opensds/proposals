/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/FileSys.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/File.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "array/Array.h"
#ifdef LIN_FRE_SUPP
#include "alarm/AppFreezeStatus.h"
#endif
#ifdef LINUX
#include <mntent.h>
#endif
CFileSys::CFileSys()
{
#ifdef WIN32    
    m_pVssRequester = NULL;
#endif
}

CFileSys::~CFileSys()
{
}

/*------------------------------------------------------------ 
Description  : 查询文件系统信息
               容灾流程中查询文件系统信息只在生产端Server会调用
Input        : 
Output       : vecFileInfos -- 查询的文件信息列表
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::QueryFileSysInfo(vector<file_sys_info_t>& vecFileInfos)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    file_sys_info_t fileInfo;
#ifdef WIN32
    vector <sub_area_Info_t> vecSubAreaInfo;
    vector <sub_area_Info_t>::iterator iter;
    
    iRet = CDisk::GetSubareaInfoList(vecSubAreaInfo);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_GET_PARTITION_INFO_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get disk and volume info for file sys failed, iRet %d.", iRet);
        return iRet;
    }

    for (iter = vecSubAreaInfo.begin(); iter != vecSubAreaInfo.end(); iter++)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query file sys info succ.");
        fileInfo.deviceName = iter->acVolName;
        fileInfo.mountpoint = iter->acDriveLetter;
        fileInfo.fileSysType = iter->acFileSystem;
        fileInfo.capacity = iter->ullTotalCapacity;
        fileInfo.offSet = iter->llOffset;
        fileInfo.diskNumber = iter->iDiskNum;
        fileInfo.volType = 0;

        vecFileInfos.push_back(fileInfo);
    }
#else

    vector<mp_string> vecFileSysMount;
    vector<mp_string>::iterator vecIter;


    //查询已挂载的文件系统
    mp_string strCmd = " | awk '{print $1}' | grep -w /dev";
    
    iRet = CRootCaller::Exec(ROOT_COMMAND_MOUNT, strCmd, &vecFileSysMount);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get mounted file system failed, iRet %d.", iRet);
        return ERROR_DEVICE_FILESYS_QUERY_INFO_FAILED;
    }

    for (vecIter = vecFileSysMount.begin(); vecIter != vecFileSysMount.end(); ++vecIter)
    {
        iRet = GetFileSysInfos(*vecIter, fileInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Get disk(%s) file system infos failed.", vecIter->c_str());
            continue;
        }

        vecFileInfos.push_back(fileInfo);
    }
    
#endif

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query file sys info succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 挂载文件系统
Input        : mountInfo -- 待挂载的文件系统信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::Mount(mount_info_t& mountInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;

#ifdef WIN32
    mp_string strVolume = mountInfo.deviceName;
    mp_string strDriverLetter = mountInfo.mountPoint;
    mp_bool bIsMounted = MP_FALSE;
    mp_int32 iVolType = mountInfo.volumeType;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin mount dev, volume %s, hard-disk driver letter %s, volume type %d.",
        strVolume.c_str(), strDriverLetter.c_str(), iVolType);

    iRet = CheckMountedStatus(strVolume, strDriverLetter, bIsMounted);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check mounted status of device failed.");
        return iRet;
    }

    if (bIsMounted)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Device has already mounted to the specified mount point, volume %s, "
            "mount point %s", strVolume.c_str(), strDriverLetter.c_str());
        return MP_SUCCESS;
    }

    strDriverLetter += ":\\";
    //If the function fails, the return value is zero
    iRet = SetVolumeMountPoint(strDriverLetter.c_str(), strVolume.c_str());
    if (0 == iRet)
    {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set volume mount point failed, mount point(driver letter) %s, volume"
            " path %s iRet %d, errno[%d]:%s.", strDriverLetter.c_str(), strVolume.c_str(), iRet, iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_DEVICE_FILESYS_MOUNT_FAILED;
    }
#else
    mp_string strMountPoint = mountInfo.mountPoint;
    mp_string strDevFullPath = mountInfo.deviceName;
    mp_int32 iVolType = mountInfo.volumeType;
    mp_bool bIsMounted = MP_FALSE;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin mount dev, dev %s, mount point %s, volume type %d.",
        strDevFullPath.c_str(), strMountPoint.c_str(), iVolType);

    iRet = CheckMountedStatus(strDevFullPath, strMountPoint, iVolType, bIsMounted);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check mounted status of device failed.");
        return iRet;
    }

    if (bIsMounted)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Device has already mounted to the specified mount point, device %s, "
            "mount point %s", strDevFullPath.c_str(), strMountPoint.c_str());
        return MP_SUCCESS;
    }
#if (defined HP_UX_IA) || (defined SOLARIS)
    mp_string strFileSysType;
    mp_string strFsckParam;
    iRet = QueryFileSysType(strDevFullPath, strFileSysType);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query file sys type failed.");
        return iRet;
    }
    strFsckParam = " -y -F " + strFileSysType;
#endif

#ifdef SOLARIS
    strParam = " -F " + strFileSysType + " " + strDevFullPath + " \"" + strMountPoint + "\"";
#else
    strParam = strDevFullPath + " \"" + strMountPoint + "\"";
#endif
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Invoke root to mount file sys, param %s.", strParam.c_str());
    iRet = CRootCaller::Exec(ROOT_COMMAND_MOUNT, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Root caller exec failed, iRet %d.", iRet);
#if (defined HP_UX_IA) || (defined SOLARIS)
        strParam = strFsckParam + " " + strDevFullPath;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Invoke root to fsck vxfs or hfs file sys, param %s.", strParam.c_str());
        iRet = CRootCaller::Exec(ROOT_COMMAND_FSCK, strParam, NULL);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Root caller exec failed, iRet %d.", iRet);
            return iRet;
        }

#ifdef SOLARIS
        strParam = " -F " + strFileSysType + " " + strDevFullPath + " \"" + strMountPoint + "\"";
#else
        strParam = strDevFullPath + " \"" + strMountPoint + "\"";
#endif
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Invoke root to mount file sys, param %s.", strParam.c_str());
        iRet = CRootCaller::Exec(ROOT_COMMAND_MOUNT, strParam, NULL);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Root caller exec failed, iRet %d.", iRet);
            return iRet;
        }
#else
        return iRet;
#endif
    }
#endif

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Mount dev succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 批量挂载文件系统
Input        : vecMountInfos -- 待挂载的文件系统信息列表
               vecFailedInfos -- 挂载失败的文件信息列表
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::Mount(vector<mount_info_t>& vecMountInfos, vector<mount_failed_info_t>& vecFailedInfos)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector<mount_info_t>::iterator iter;
    mount_failed_info_t mountFailedInfo;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin mount devs.");
    for (iter = vecMountInfos.begin(); iter != vecMountInfos.end(); iter++)
    {
        iRet = Mount(*iter);
        if (MP_SUCCESS != iRet)
        {
            mountFailedInfo.deviceName = iter->deviceName;
            mountFailedInfo.errCode = iRet;
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Mount failed, iRet %d.", iRet);
            continue;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End mount devs.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 去挂载文件系统
Input        : umountInfo -- 去挂载的文件系统信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::UMount(umount_info_t& umountInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;

#ifdef WIN32
    HANDLE hDevice;
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;
    mp_string strVolume = umountInfo.deviceName;
    mp_string strVolumeTmp;
    mp_string strDriverLetter = umountInfo.mountPoint;
    mp_int32 iVolType = umountInfo.volumeType;
    mp_bool bIsMounted = MP_FALSE;
    DWORD dBytesReturned = 0;

    //关于挂载点、卷路径的专业术语定义
    //https://msdn.microsoft.com/en-us/library/windows/desktop/aa365248(v=vs.85).aspx  (Naming a Volume)
    //A volume mount point is any user-mode path that can be used to access a volume. There are three types of volume mount points:
    //A drive letter, for example, "C:\".
    //A volume GUID path, for example, "\\?\Volume{26a21bda-a627-11d7-9931-806e6f6e6963}\".
    //A mounted folder, for example, "C:\MountD\".
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin umount dev, volume %s, hard-disk driver letter %s, voltype %d.",
        strVolume.c_str(), strDriverLetter.c_str(), iVolType);

    iRet = CheckMountedStatus(strVolume, strDriverLetter, bIsMounted);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_UNMOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check mounted status of device failed.");
        return iRet;
    }

    if (!bIsMounted)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Volume and drive letter dose not exist, do nothing.");
        return MP_SUCCESS;
    }

    iRet = OfflineVolume(strVolume);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_OFFLINE_VOLUME_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Offline volume failed, volume %s.", strVolume.c_str());
        return iRet;
    }

    //Deletes a drive letter
    iRet = DeleteDriveLetter(strDriverLetter);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_DELETE_DRIVER_LETTER_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete drive letter faield, drive letter %s.", strDriverLetter.c_str());
        return iRet;
    }
#else
    mp_string strDevFullPath = umountInfo.deviceName;
    mp_string strMountPoint = umountInfo.mountPoint;
    mp_int32 iVolType = umountInfo.volumeType;
    mp_string strParam;
    mp_bool bIsMounted = MP_FALSE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin umount dev, dev %s, mount point %s, voltype %d.",
        strDevFullPath.c_str(), strMountPoint.c_str(), iVolType);

    iRet = CheckMountedStatus(strDevFullPath, strMountPoint, iVolType, bIsMounted);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_UNMOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        if (ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST == iRet)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Mount point dosen't exist, skip it.");
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check mounted status of device failed.");
        return iRet;
    }

    if (!bIsMounted)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Device dosen't mounted, skip it, device %s.",
            strDevFullPath.c_str());
        return MP_SUCCESS;
    }

    strParam = strDevFullPath;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Invoke root to umount file sys, param %s.", strParam.c_str());
    iRet = CRootCaller::Exec(ROOT_COMMAND_UMOUNT, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_UNMOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Root caller exec failed, iRet %d.", iRet);
        return iRet;
    }
#endif //WIN32

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Umount dev succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 批量去挂载文件系统
Input        : vecUMountInfos -- 去挂载的文件系统信息列表
               vecFailedInfos -- 去挂载失败的文件信息列表
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::UMount(vector<umount_info_t>& vecUMountInfos, vector<umount_failed_info_t>& vecFailedInfos)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector<umount_info_t>::iterator iter;
    umount_failed_info_t uMountFailedInfo;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin umount devs.");
    for (iter = vecUMountInfos.begin(); iter != vecUMountInfos.end(); iter++)
    {
        iRet = UMount(*iter);
        if (MP_SUCCESS != iRet)
        {
            uMountFailedInfo.deviceName = iter->deviceName;
            uMountFailedInfo.errCode = iRet;
            vecFailedInfos.push_back(uMountFailedInfo);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Umount failed, iRet %d.", iRet);
            continue;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End umount devs.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 冻结文件系统
Input        : vecDriveLetters -- 待冻结的文件系统驱动器号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/   
mp_int32 CFileSys::Freeze(vector<mp_string>& vecDriveLetters)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin freeze file sys.");

    if (vecDriveLetters.empty())
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Input Parameter file name is null.");

        return ERROR_COMMON_INVALID_PARAM;
    }
    
#ifdef WIN32 
    
    if (NULL != m_pVssRequester)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Other freeze opertion is running.");
        return ERROR_VSS_OTHER_FREEZE_RUNNING;
    }
    
    //m_pVssRequester = new VSSRequester();
    NEW_CATCH_RETURN_FAILED(m_pVssRequester, VSSRequester);
    iRet = m_pVssRequester->Freeze(vecDriveLetters, vss_freeze_filesys);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_FREEZE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze file sys failed, iRet %d.", iRet);
        return iRet;
    }
#elif defined(LIN_FRE_SUPP)

    mp_string strMountp;
    list<mp_string> lstDevNames;
    vector<mp_string>::iterator iter;
    vector<mp_string> vctDriLettersSucc;
    CAppFreezeStatus cfreezestatus;
    freeze_status stStatus;

    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); ++iter)
    {
        lstDevNames.clear();

        if (!CMpFile::DirExist(iter->c_str()))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Mount point dose not exist, mount point %s.", iter->c_str());
            
            return ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST;
        }
        
        iRet = GetDevNameByMountPoint(*iter, lstDevNames);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_GET_DEV_FAILED);
        if (MP_SUCCESS != iRet || lstDevNames.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get dev name by mount point %s failed, iRet %d.",
                iter->c_str(), iRet);

            return iRet;
        }

        if (MP_TRUE != CheckLocalLun(lstDevNames))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "At least one Lun is not Huawei product in the mount point %s.",
                iter->c_str());
            
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        stStatus.strKey = *(iter);
        cfreezestatus.Get(stStatus);

        if (stStatus.iStatus != DB_UNFREEZE)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The file system %s is freeze by other execute, so iRet %d.", 
                iter->c_str(), ERROR_COMMON_APP_FREEZE_FAILED);
            
            return ERROR_COMMON_APP_FREEZE_FAILED;
        }

        strMountp = strMountp + *(iter) + ":";
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze file system mount point(%s).", strMountp.c_str());

    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); ++iter)
    {
        stStatus.strKey = *(iter);

        iRet = cfreezestatus.Insert(stStatus);
        vctDriLettersSucc.push_back(*(iter));
        
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The file system %s insert to database failed, iRet is %d",
                iter->c_str(), iRet);

            if (!vctDriLettersSucc.empty())
            {
                iRet = UnFreeze(vctDriLettersSucc);
                if (MP_SUCCESS != iRet)
                {
                    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "File system roll back fialed, iRet %d.", iRet);
                }
            }
            
            return ERROR_COMMON_APP_FREEZE_FAILED;
        }
        
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_FILESYS_FREEZE, *(iter), NULL);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze file system %s failed, iRet %d, then rollback the freeze"
                " succ file system before.", iter->c_str(), iRet);
             if (iter != vecDriveLetters.begin())
             {
                //冻结回滚操作
                iRet = UnFreeze(vctDriLettersSucc);
                if (MP_SUCCESS != iRet)
                {
                    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "File system roll back fialed, iRet %d.", iRet);
                }
             }
            
             return ERROR_COMMON_APP_FREEZE_FAILED;
        }
    }
    
#else

    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupport Freeze file system.");

    return ERROR_COMMON_FUNC_UNIMPLEMENT;

#endif

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Freeze file sys succ.");

    return iRet;
}

/*------------------------------------------------------------ 
Description  : 解冻文件系统
Input        : vecDriveLetters -- 待解冻的文件系统驱动器号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::UnFreeze(vector<mp_string>& vecDriveLetters)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin unfreeze file sys.");

    if (vecDriveLetters.empty())
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Input Parameter file name is null.");

        return ERROR_COMMON_INVALID_PARAM;
    }
#ifdef WIN32 

    if (NULL == m_pVssRequester)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "No need to thaw.");
        return ERROR_COMMON_APP_THAW_FAILED;
    }

    iRet = m_pVssRequester->UnFreeze(vecDriveLetters);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_THAW_FAILED);
    if (MP_SUCCESS != iRet)
    {
        if (ERROR_INNER_THAW_NOT_MATCH != iRet)
        {
            delete m_pVssRequester;
            m_pVssRequester = NULL;
        }
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze file sys failed, iRet %d.", iRet);
        return iRet;
    }

    delete m_pVssRequester;
    m_pVssRequester = NULL;
    
#elif defined(LIN_FRE_SUPP)

    mp_string strMountp;
    CAppFreezeStatus cfreezestatus;
    vector<mp_string>::iterator iter;

    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); ++iter)
    {
        freeze_status stfrestat;
        
        if (!CMpFile::DirExist(iter->c_str()))
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Mount point dose not exist, mount point %s.", iter->c_str());
            
            return ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST;
        }

        stfrestat.strKey = *(iter);
        cfreezestatus.Get(stfrestat);
        if (stfrestat.iStatus != DB_FREEZE)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "No need to thaw, the database has no infos about %s.",
                stfrestat.strKey.c_str());
            
            return ERROR_COMMON_APP_THAW_FAILED;
        }

        strMountp = strMountp + *(iter) + ":";
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw file system mount point(%s).", strMountp.c_str());

    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); ++iter)
    {
        freeze_status stStatus;
        
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_FILESYS_THAW, *(iter), NULL);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Thaw file system %s failed, iRet %d.", iter->c_str(), iRet);
        
            return ERROR_COMMON_APP_THAW_FAILED;
        }

        stStatus.strKey = *(iter);
        iRet = cfreezestatus.Delete(stStatus);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The file system %s delete from database failed, iret is %d",
                iter->c_str(), iRet);
            
            return iRet;
        }
    }
    
#else

    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupport Freeze file system.");

    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unfreeze file sys succ.");

    return iRet;
}
/*------------------------------------------------------------ 
Description  : 冻结所有卷
Input        : 无
Output       : 无
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CFileSys::FreezeAll()
{
    LOGGUARD("");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin freeze all volumes.");
    
#ifdef WIN32 
    mp_int32 iRet = MP_SUCCESS;
    
    if (NULL != m_pVssRequester)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Other freeze opertion is running.");
        return ERROR_VSS_OTHER_FREEZE_RUNNING;
    }
    
    //m_pVssRequester = new VSSRequester();
    NEW_CATCH_RETURN_FAILED(m_pVssRequester, VSSRequester);
    iRet = m_pVssRequester->FreezeAll();
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze all volumes failed, iRet %d.", iRet);
        return iRet;
    }    

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Freeze all volumes succ.");
    return iRet;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unimplement function.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

}
/*------------------------------------------------------------ 
Description  : 解冻所有卷
Input        : 无
Output       : 无
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CFileSys::UnFreezeAll()
{
    LOGGUARD("");

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin unfreeze all volumes.");
#ifdef WIN32 
    mp_int32 iRet = MP_SUCCESS;
    
    if (NULL == m_pVssRequester)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "No need to unfreeze.");
        return ERROR_COMMON_APP_THAW_FAILED;
    }

    iRet = m_pVssRequester->UnFreezeAll();
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze all volumes failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unfreeze all volumes succ.");
    return iRet;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unimplement function.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

}

/*------------------------------------------------------------ 
Description  : 查询linux文件系统的状态
Input        : vecDriveLetters -- 待冻结的文件系统驱动器号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CFileSys::QueryFreezeState(vector<mp_string>& vecDriveLetters)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin Query file system status.");
#ifdef LIN_FRE_SUPP

    CAppFreezeStatus cfreezestatus;
    freeze_status stStatus;
    vector<mp_string>::iterator iter;

    for (iter = vecDriveLetters.begin(); iter != vecDriveLetters.end(); ++iter)
    {
        if ("" != *iter)
        {
            stStatus.strKey = *(iter);
            cfreezestatus.Get(stStatus);

            if (stStatus.iStatus != DB_FREEZE)
            {
                COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query file system status %d.", DB_UNFREEZE);
                return DB_UNFREEZE; 
            }
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query file system status %d.", DB_FREEZE);
    
    return DB_FREEZE;
#elif defined(WIN32)

    if (NULL == m_pVssRequester)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query file system status %d.", FREEZE_STAT_UNFREEZE);
        return FREEZE_STAT_UNFREEZE;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query file system status %d.", FREEZE_STAT_FREEZED);
    return FREEZE_STAT_FREEZED;  
#else

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unsupport query file system status.");

    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

}

/*------------------------------------------------------------ 
Description  : 结束备份
Input        : iBackupSucc -- 是否备份成功
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CFileSys::EndBackup(mp_int32 iBackupSucc)
{
    LOGGUARD("");

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin unfreeze all volumes.");
#ifdef WIN32 
    mp_int32 iRet = MP_SUCCESS;

    if (NULL == m_pVssRequester)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "No need to end backup.");
        return MP_SUCCESS;
    }

    iRet = m_pVssRequester->EndBackup(iBackupSucc);
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "End backup failed, iRet %d.", iRet);
        return iRet;
    }
    delete m_pVssRequester;
    m_pVssRequester = NULL;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unfreeze all volumes succ.");
    return iRet;
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unimplement function.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

}


#ifdef WIN32

/*------------------------------------------------------------ 
Description  : 删除驱动器号
Input        : driveletterinfo -- 删除驱动器号所需要的信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::DeleteDriveLetter(umount_info_t& driveletterinfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsMounted = MP_FALSE;
    mp_string strVolume = driveletterinfo.deviceName;
    mp_string strDriverLetter = driveletterinfo.mountPoint;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin delete letter, volume %s, hard-disk driver letter %s, voltype %d.",
        driveletterinfo.deviceName.c_str(), driveletterinfo.mountPoint.c_str(), driveletterinfo.volumeType);

    iRet = CheckMountedStatus(strVolume, strDriverLetter, bIsMounted);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_DELETE_DRIVER_LETTER_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check mounted status of device failed.");
        
        return iRet;
    }

    if (!bIsMounted)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Volume and drive letter dose not exist, do nothing.");
        
        return MP_SUCCESS;
    }

    iRet = DeleteDriveLetter(strDriverLetter);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_DELETE_DRIVER_LETTER_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete drive letter faield, drive letter %s.", strDriverLetter.c_str());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete drive letter succ, drive letter %s.", strDriverLetter.c_str());
    
    return iRet;
}


/*------------------------------------------------------------ 
Description  : 检查挂载点状态
Input        : vecDriveLetters -- 待解冻的文件系统驱动器号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::CheckMountPointStatus(mp_string& strMountPoint)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd;
    vector<mp_string> vecDevs;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin check mount point status.");

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Check mount point status succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 检查挂载点状态
Input        : strVolume -- 卷路径
               strMountPoint -- 挂载点
               bIsMounted -- 是否挂载，已挂载返回MP_TRUE，未挂载返回MP_FALSE
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::CheckMountedStatus(mp_string& strVolume, mp_string& strMountPoint, mp_bool& bIsMounted)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsExist = MP_FALSE;
    mp_string strDriveLetter = strMountPoint + ":\\";
    map<mp_string, mp_string> mapVolumePaths;
    map<mp_string, mp_string>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin check mount status.");
    iRet = CDisk::GetVolPaths(mapVolumePaths);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_GET_VOLUME_PATH_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get volume paths failed, iRet %d.", iRet);
        return iRet;
    }

    iter = mapVolumePaths.find(strVolume);
    if (iter != mapVolumePaths.end())
    {
        if (strDriveLetter == iter->second)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Volume was mounted to the expected mount point.");
            bIsMounted = MP_TRUE;
            return MP_SUCCESS;
        }

        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Volume was mounted to the other mount point, volume %s, mount point %s.",
            strVolume.c_str(), iter->second.c_str());
        return ERROR_DEVICE_FILESYS_MOUTN_DEV_IS_MOUNTED;
    }

    bIsExist = CDisk::IsDriveExist(strDriveLetter);
    if (bIsExist)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Other volume was mounted to the mount point, mount point %s.",
            strDriveLetter.c_str());

        return ERROR_DEVICE_FILESYS_MOUNT_POINT_OCCUPIED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End check mount status.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 离线卷
Input        : strVolume -- 卷路径
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::OfflineVolume(mp_string& strVolume)
{
    mp_int32 iRet = MP_SUCCESS;
    HANDLE hDevice;
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;
    mp_string strVolumeTmp;
    DWORD dBytesReturned = 0;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin offline volume, volume %s.", strVolume.c_str());
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open the volume %s.", strVolume.c_str());
    //All volume and mounted folder functions that take a volume GUID path as an input parameter require the trailing
    //backslash. All volume and mounted folder functions that return a volume GUID path provide the trailing backslash,
    //but this is not the case with the CreateFile function. You can open a volume by calling CreateFile and omit
    //the trailing backslash from the volume name you specify
    //remove the trailing backslash for CreateFile API
    strVolumeTmp = strVolume.substr(0, strlen(strVolume.c_str()) - 1);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get the volume path for CreateFile, volume path %s.", strVolumeTmp.c_str());
    hDevice = CreateFile(strVolumeTmp.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hDevice)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open the volume failed, volume %s, errno[%d]:%s.", strVolumeTmp.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_DEVICE_FILESYS_OFFLINE_VOLUME_FAILED;
    }

    //If the operation fails or is pending, DeviceIoControl returns zero
    //The system flushes all cached data to the volume before locking it. For example, any data held in a lazy-write
    //cache is written to the volume.
    //The NTFS file system treats a locked volume as a dismounted volume. The FSCTL_DISMOUNT_VOLUME control code
    //functions similarly but does not check for open files before dismounting. Note that without a successful lock
    //operation, a dismounted volume may be remounted by any process at any time. This would not be an ideal state
    //for performing a volume backup, for example.
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Lock the volume.");
    iRet = DeviceIoControl(hDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dBytesReturned, (LPOVERLAPPED)NULL);
    if (0 == iRet)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Lock volume failed, volume %s, errno[%d]:%s.", strVolume.c_str(),iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        (mp_void)CloseHandle(hDevice);
        return ERROR_DEVICE_FILESYS_OFFLINE_VOLUME_FAILED;
    }

    //Takes a volume offline
    //If the operation fails or is pending, DeviceIoControl returns zero
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Take the volume offline.");
    iRet = DeviceIoControl(hDevice, IOCTL_VOLUME_OFFLINE, NULL, 0, NULL, 0, &dBytesReturned, (LPOVERLAPPED)NULL);
    if (0 == iRet)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Task volume offline failed, volume %s, errno[%d]:%s.", strVolume.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        (mp_void)DeviceIoControl(hDevice, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dBytesReturned, (LPOVERLAPPED)NULL);
        (mp_void)CloseHandle(hDevice);
        return ERROR_DEVICE_FILESYS_OFFLINE_VOLUME_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unlock the volume and close the handle.");
    (mp_void)DeviceIoControl(hDevice, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dBytesReturned, (LPOVERLAPPED)NULL);
    (mp_void)CloseHandle(hDevice);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin offline volume, volume %s.", strVolume.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 删除驱动器号
Input        : strDriveLetter -- 驱动器号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::DeleteDriveLetter(mp_string& strDriveLetter)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsExist = MP_FALSE;
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;
    mp_string strTmpDrive;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin delete drive letter, drive letter %s.", strDriveLetter.c_str());
    strTmpDrive = strDriveLetter + ":\\";
    bIsExist = CDisk::IsDriveExist(strTmpDrive);
    if (bIsExist)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Drive letter exist, delete it.");
        //If the function fails, the return value is zero
        iRet = DeleteVolumeMountPoint(strTmpDrive.c_str());
        if (0 == iRet)
        {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Deletes the mount point failed, mount point(drive letter) %s, errno[%d]:%s.",
                strTmpDrive.c_str(), iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return ERROR_DEVICE_FILESYS_DELETE_DRIVER_LETTER_FAILED;
        }
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Delete drive letter succ.");

    return MP_SUCCESS;
}
#else
/*------------------------------------------------------------ 
Description  : 检查UDEV设备状态
Input        : strDev -- 设备名称
               strMountPoint -- 挂载点
               bIsMounted -- 是否挂载，已挂载返回MP_TRUE，未挂载返回MP_FALSE
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::CheckUdevDevStatus(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
#ifdef LINUX
    mp_string strParam;
    mp_string strCmd;
    mp_string strFileType;
    mp_string strExistMountPoint;
    vector<mp_string> vecResult;
    vector<mp_string> vecMountPoints;

    if (!CMpFile::FileExist(strDev.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The device is not exist.", strDev.c_str());
        return MP_SUCCESS;
    }
        
    strParam = " " + strDev + "| awk '{print $1}'|cut -c1";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get file type failed, iRet %d.", iRet);
        return iRet;
    }
    
    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get file type failed.");
        return MP_FAILED;
    }

    strFileType = vecResult.front();
    vecResult.clear();

    if ("l" == strFileType)
    {
        strParam = " " + strDev + "| awk '{print $NF}'|awk -F '/' '{print $NF}'";
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get used device by link file failed, iRet %d.", iRet);
            return iRet;
        }
        
        if (vecResult.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get used device by link file failed.");
            return MP_FAILED;
        }

        strDev = "/dev/" + vecResult.front();

        vector<linux_mount_info_t> vecMountInfo;
        iRet = GetAllMountInfo(vecMountInfo);
        if (MP_SUCCESS != iRet || vecMountInfo.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) mount info failed.", strDev.c_str());
            return MP_FAILED;
        }
        for(vector<linux_mount_info_t>::iterator it = vecMountInfo.begin(); it != vecMountInfo.end(); it++)
        {
            if (strDev == it->deviceName)
            {
                vecMountPoints.push_back(it->mountpoint);
                break;
            }
        }

        if (!vecMountPoints.empty())
        {
            strExistMountPoint = vecMountPoints.front();
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get exist mount point %s.", strExistMountPoint.c_str());
            if (strMountPoint == strExistMountPoint)
            {
                bIsMounted = MP_TRUE;
                COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device was mounted to the expected mount point.");
                return MP_SUCCESS;
            }

            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Device was mounted to the other mount point, device %s, mount point %s.",
                strDev.c_str(), vecMountPoints.front().c_str());
            return ERROR_DEVICE_FILESYS_MOUTN_DEV_IS_MOUNTED;
        }
    }
#endif
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 检查设备状态
Input        : strDev -- 设备名称
               strMountPoint -- 挂载点
               bIsMounted -- 是否挂载，已挂载返回MP_TRUE，未挂载返回MP_FALSE
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::CheckDevStatus(mp_string& strDev, mp_string& strMountPoint, mp_bool& bIsMounted)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd;
    mp_string strParam;
    mp_string strFileType;
    mp_string strExistMountPoint;
    vector<mp_string> vecResult;
    vector<mp_string> vecMountPoints;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin check dev status.");
#ifdef LINUX
    strCmd = " | awk '$1 == \"" + strDev + "\" {print $3}'";

#elif defined(AIX)
    strCmd = " | awk '$1 == \"" + strDev + "\" {print $2}'";

#elif defined (HP_UX_IA)
    ///usr/sbin/mount，HP下新创建用户默认/usr/sbin没有在PATH环境变量中
    strCmd = " | awk '$3 == \"" + strDev + "\" {print $1}'";
#elif defined (SOLARIS)
    ///usr/sbin/mount，HP下新创建用户默认/usr/sbin没有在PATH环境变量中
    strCmd = " | nawk '$3 == \"" + strDev + "\" {print $1}'";
#else
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unimplement function.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
    
#endif
#ifdef LINUX
    vector<linux_mount_info_t> vecMountInfo;
    iRet = GetAllMountInfo(vecMountInfo);
    if (MP_SUCCESS != iRet || vecMountInfo.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) mount info failed.", strDev.c_str());
        return MP_FAILED;
    }
    for(vector<linux_mount_info_t>::iterator it = vecMountInfo.begin(); it != vecMountInfo.end(); it++)
    {
        if (strDev == it->deviceName)
        {
            vecMountPoints.push_back(it->mountpoint);
            break;
        }
    }
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be exceuted.", strCmd.c_str());
    iRet = CRootCaller::Exec(ROOT_COMMAND_MOUNT, strCmd, &vecMountPoints);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec system command failed, iRet %d.", iRet);

        return iRet;
    }
#endif
    if (!vecMountPoints.empty())
    {
        strExistMountPoint = vecMountPoints.front();
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get exist mount point %s.", strExistMountPoint.c_str());
        if (strMountPoint == strExistMountPoint)
        {
            bIsMounted = MP_TRUE;
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device was mounted to the expected mount point.");
            return MP_SUCCESS;
        }

        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Device was mounted to the other mount point, device %s, mount point %s.",
            strDev.c_str(), vecMountPoints.front().c_str());
        return ERROR_DEVICE_FILESYS_MOUTN_DEV_IS_MOUNTED;
    }
    else
    {
        iRet = CheckUdevDevStatus(strDev, strMountPoint, bIsMounted);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check device status failed.", strDev.c_str());
            return iRet;
        }
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End check dev status.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 检查挂载点状态
Input        : strMountPoint -- 挂载点
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::CheckMountPointStatus(mp_string& strMountPoint)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd;
    vector<mp_string> vecDevs;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin check mount point status.");

    //暂不支持挂载点包括空格
#ifdef LINUX
        strCmd = " | awk '$3 == \"" + strMountPoint + "\" {print $1}'";
    
#elif defined(AIX)
        strCmd = " | awk '$2 == \"" + strMountPoint + "\" {print $1}'";
    
#elif defined (HP_UX_IA)
        strCmd = " | awk '$1 == \"" + strMountPoint + "\" {print $3}'";

#elif defined (SOLARIS)
        strCmd = " | nawk '$1 == \"" + strMountPoint + "\" {print $3}'";
#else
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unimplement function.");
        return ERROR_COMMON_FUNC_UNIMPLEMENT;
        
#endif
#ifdef LINUX   //挂载点包括空格
    vector<linux_mount_info_t> vecMountInfo;
    iRet = GetAllMountInfo(vecMountInfo);
    if (MP_SUCCESS != iRet || vecMountInfo.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the strMountPoint(%s) info failed.", strMountPoint.c_str());
        return MP_FAILED;
    }
    for(vector<linux_mount_info_t>::iterator it = vecMountInfo.begin(); it != vecMountInfo.end(); it++)
    {
        if(strMountPoint == it->mountpoint)
        {
            vecDevs.push_back(it->deviceName);
            break;
        }
    }
#else
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be exceuted.", strCmd.c_str());

    iRet = CRootCaller::Exec(ROOT_COMMAND_MOUNT, strCmd, &vecDevs);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec system command failed, iRet %d.", iRet);
        return iRet;
    }
#endif
    if (!vecDevs.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Other device was mounted to the mount point, device %s, mount point %s.",
            vecDevs[0].c_str(), strMountPoint.c_str());
        return ERROR_DEVICE_FILESYS_MOUNT_POINT_OCCUPIED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Check mount point status succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 检查设备挂载状态
Input        : strDevPath -- 设备名称
               strMountPoint -- 挂载点
               iVolType -- 卷类型
               bIsMounted -- 是否挂载，已挂载返回MP_TRUE，未挂载返回MP_FALSE
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::CheckMountedStatus(mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType,
    mp_bool& bIsMounted)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDev = strDevPath;
    mp_string strParam;
    vector<mp_string> vecMountPoints;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin check mount status, dev path %s, mount point %s, voltype %d.",
        strDevPath.c_str(), strMountPoint.c_str(), iVolType);

    iRet = CheckDevStatus(strDev, strMountPoint, bIsMounted);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check dev status failed.");
        return iRet;
    }

    if (bIsMounted)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device has mounted to the specified mount point.");
        return MP_SUCCESS;
    }

    if (!CMpFile::DirExist(strMountPoint.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Mount point dose not exist, mount point %s.", strMountPoint.c_str());
        return ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST;
    }

    iRet = CheckMountPointStatus(strMountPoint);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check mount point status failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End check mount status.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取设备卷类型
Input        : strDevname -- 设备名称,带有/dev/
Output       : iVolType -- 卷类型
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::GetVolumeType(mp_string strDevname, mp_int32& iVolType)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strNameTmp;
    mp_string strCmd;
    mp_string::size_type sLvmPos = strDevname.find(LVM_MARK);
    mp_string::size_type sLxvmPos = strDevname.find(VXVM_MARK);
    mp_string::size_type sSimplePos = strDevname.find(SIMPLE_MARK);

    vector<mp_string> vecDevNameInfos;


    //带LVM的文件系统
    if (sLvmPos != mp_string::npos)
    {
        iVolType = VOLUME_TYPE_LINUX_LVM;
    }

    //带LxVM的文件系统
    else if (sLxvmPos != mp_string::npos)
    {
        iVolType = VOLUME_TYPE_LINUX_VXVM;    
    }

    //块设备上的文件系统
    else if (sSimplePos != mp_string::npos)
    {
        iVolType = VOLUME_TYPE_SIMPLE;
        
        //Udev与块设备/dev/sd*类似的情况
        //去掉/dev/
        strNameTmp = strDevname.substr(DEV_LENGTH);
        strCmd = "cat /proc/partitions | grep -w '" + strNameTmp + "'";
        iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDevNameInfos);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check the dev %s in /proc/partitions failed, iRet %d.", 
                strDevname.c_str(), iRet);
         
            return MP_FAILED;
        }

        //其他类型，暂不支持
        if (vecDevNameInfos.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupport Dev:%s.", strDevname.c_str());

            return MP_FAILED;
        }
    }
    //其他类型，暂不支持
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupport Dev:%s.", strDevname.c_str());

        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Dev %s volume type %d.", strDevname.c_str(), iVolType);

    return MP_SUCCESS;
}


#ifdef LINUX
/*------------------------------------------------------------ 
Description  :获取设备文件系统挂载信息
Input        : 
Output       : vecMountInfo -- 单个文件系统挂载点     
Return       : MP_SUCCESS -- 返回成功
                 非MP_SUCCESS -- 返回失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::GetAllMountInfo(vector<linux_mount_info_t> &vecMountInfo)
{
    mp_string filename("/proc/mounts");
    FILE *fileMount;
    struct mntent *mountInfo;
    fileMount = setmntent(filename.c_str(), "r");
    if (!fileMount)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Translate the mount information by LINUX API failed.");
        endmntent(fileMount);
        return MP_FAILED;
    }
    while(mountInfo = getmntent(fileMount))
    {
        linux_mount_info_t tempMountInfo;
        tempMountInfo.deviceName.insert(0, mountInfo->mnt_fsname);
        tempMountInfo.mountpoint.insert(0, mountInfo->mnt_dir);
        tempMountInfo.fileSysType.insert(0, mountInfo->mnt_type);
        if(tempMountInfo.deviceName.empty() || tempMountInfo.mountpoint.empty() || tempMountInfo.fileSysType.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "MountPoint and Filesys information: devName: %s, mountpoint: %s, filesystype: %s.", tempMountInfo.deviceName.c_str(), tempMountInfo.mountpoint.c_str(), tempMountInfo.fileSysType.c_str());
            continue;
        }
        vecMountInfo.push_back(tempMountInfo);
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get all of the Mount information success.");
    endmntent(fileMount);
    return MP_SUCCESS;
}
#endif
/*------------------------------------------------------------ 
Description  :获取设备文件系统信息
Input        : strDevname -- 设备名称,带有/dev/
Output       : fileInfo -- 文件系统信息
Return       : MP_SUCCESS -- 返回成功
                 非MP_SUCCESS -- 返回失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::GetFileSysInfos(mp_string strDevname, file_sys_info_t& fileInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iVolType = 0;
    mp_string strCmd;
    vector<mp_string> vecMountPoint;
    vector<mp_string> vecFileSysType;
    vector<mp_string> vecFileCapacipty;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get disk(%s) file system infos.", strDevname.c_str());

    iRet = GetVolumeType(strDevname, iVolType);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Dev %s volume type failed.", strDevname.c_str());
        
        return iRet;
    }

    //获取块设备的挂载点
#ifdef LINUX
    vector<linux_mount_info_t> vecMountInfo;
    iRet = GetAllMountInfo(vecMountInfo);
    if (MP_SUCCESS != iRet || vecMountInfo.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) mount info failed.", strDevname.c_str());
        return MP_FAILED;
    }
    for(vector<linux_mount_info_t>::iterator it = vecMountInfo.begin(); it != vecMountInfo.end(); it++)
    {
        if(strDevname == it->deviceName)
        {
            vecMountPoint.push_back(it->mountpoint);
            vecFileSysType.push_back(it->fileSysType);
            break;
        }
    }
    if (vecMountPoint.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) file mount point failed.", strDevname.c_str());
        return MP_FAILED;
    }
    if (vecFileSysType.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) file system type failed.", strDevname.c_str());
        return MP_FAILED;
    }
#else
    strCmd = " | awk '$1 == \"" + strDevname + "\" {print $3}'";
    iRet = CRootCaller::Exec(ROOT_COMMAND_MOUNT, strCmd, &vecMountPoint);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);

    if (MP_SUCCESS != iRet || vecMountPoint.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) mount point failed, iRet %d.", strDevname.c_str(), iRet);
        return MP_FAILED;
    }

    //获取块设备的文件系统类型
    strCmd = " | awk '$1 == \"" + strDevname + "\" {print $5}'";
    iRet = CRootCaller::Exec(ROOT_COMMAND_MOUNT, strCmd, &vecFileSysType);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);

    if (MP_SUCCESS != iRet || vecFileSysType.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) file system type failed, iRet %d.", strDevname.c_str(), iRet);
 
        return MP_FAILED;
    }
#endif
    //获取块设备的总容量, kb
    strCmd = "df -k " + strDevname + " | grep -n '' | awk '{print $2}'";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecFileCapacipty);
    if (MP_SUCCESS != iRet || vecFileCapacipty.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) total capacity failed, iRet %d.", strDevname.c_str(), iRet);
 
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Disk(%s) capacity %s", strDevname.c_str(), vecFileCapacipty.back().c_str());

    mp_int64 iCapacity = atol(vecFileCapacipty.back().c_str());
    
    fileInfo.deviceName  = strDevname;
    fileInfo.fileSysType = vecFileSysType.front();
    fileInfo.mountpoint  = vecMountPoint.front();
    fileInfo.capacity    = iCapacity * 1024;
    fileInfo.volType     = iVolType;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get disk(%s) file system infos succ, file system type %s, mount point %s, capacity %lld,"
         "volType %d", strDevname.c_str(), fileInfo.fileSysType.c_str(), fileInfo.mountpoint.c_str(), fileInfo.capacity, fileInfo.volType);

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :根据分区获取设备名称
Input        : strPartition -- 分区名称
Output       : strPartition -- 设备名称
Return       : MP_SUCCESS -- 返回成功
                 非MP_SUCCESS -- 返回失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSys::GetDevNameByPartition(mp_string strPartition, mp_string& strDevName)
{
    LOGGUARD("");
    
    mp_string::size_type iIndex = 0;
    mp_string strNum("1234567890");
    iIndex = strPartition.find_first_of(strNum);

    if (iIndex != mp_string::npos)
    {
       strDevName =  strPartition.substr(0, iIndex);
    }
    else
    {
        strDevName = strPartition;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get dev name(%s) by partition(%s) succ.",
        strDevName.c_str(), strPartition.c_str());
}

/*------------------------------------------------------------ 
Description  :根据挂载点获取设备名称
Input        : strMountPoint -- 挂载点
Output       : lstDevNames -- 挂载点所包含的设备名称
Return       : MP_SUCCESS -- 返回成功
                 非MP_SUCCESS -- 返回失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::GetDevNameByMountPoint(mp_string strMountPoint, list<mp_string>& lstDevNames)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get dev name by mount point %s.", strMountPoint.c_str());
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iVolType = 0;
    mp_string strCmd, strParam, strDevName;
    mp_bool bRet = MP_FALSE;
    vector<mp_string> VecFileSystem;
    vector<mp_string> vecDevNames;
    vector<mp_string> VecVgName;
    vector<mp_string>::iterator iter;
    
#ifdef LINUX
    vector<linux_mount_info_t> vecMountInfo;
    iRet = GetAllMountInfo(vecMountInfo);
    bRet = (MP_SUCCESS != iRet || vecMountInfo.empty());
    if (bRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the disk(%s) devname failed.", strMountPoint.c_str());
        return MP_FAILED;
    }
    for(vector<linux_mount_info_t>::iterator it = vecMountInfo.begin(); it != vecMountInfo.end(); it++)
    {
        if(strMountPoint == it->mountpoint)
        {
            VecFileSystem.push_back(it->deviceName);
            break;
        }
    }
    if (VecFileSystem.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the filesystem of mount point(%s) failed, iRet %d.", 
            strMountPoint.c_str(), iRet);
 
        return MP_FAILED;
    }
#else
    strCmd = " | awk '{if ($3 ==\"" + strMountPoint + "\") print $1}'";
    iRet = CRootCaller::Exec(ROOT_COMMAND_MOUNT, strParam, &VecFileSystem);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);

    bRet = (MP_SUCCESS != iRet || VecFileSystem.empty());
    if (bRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get the filesystem of mount point(%s) failed, iRet %d.", 
            strMountPoint.c_str(), iRet);
 
        return MP_FAILED;
    }
#endif
    iRet = GetVolumeType(VecFileSystem.front(), iVolType);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Dev %s volume type failed.", VecFileSystem.front().c_str());
        
        return iRet;
    }

    switch (iVolType)
    {
    case VOLUME_TYPE_SIMPLE:
        //'/dev/sdc1'截断为'/dev/sdc', 且删除重复设备
        GetDevNameByPartition(VecFileSystem.front(), strDevName);
        lstDevNames.remove(strDevName);
        lstDevNames.push_back(strDevName);
        break;
    case VOLUME_TYPE_LINUX_LVM:
        strParam = " \"" + VecFileSystem.front() + "\" | sed -n '2p' | awk -F \" \" '{print $2}'";
        ROOT_EXEC(ROOT_COMMAND_LVS, strParam, &VecVgName);
        if (VecVgName.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg name of mount point(%s) failed, iRet %d.", 
                strMountPoint.c_str(), iRet);

            return MP_FAILED;
        }

        strParam = " | awk '$2 == \"" + VecVgName.front() + "\" {print $1}'";
        ROOT_EXEC(ROOT_COMMAND_PVS, strParam, &vecDevNames);
        if (vecDevNames.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Pv name of mount point(%s) failed, iRet %d.", 
                strMountPoint.c_str(), iRet);

            return MP_FAILED;
        }

        for (iter = vecDevNames.begin(); iter != vecDevNames.end(); ++iter)
        {
            //'/dev/sdc1'截断为'/dev/sdc', 且删除重复设备
            GetDevNameByPartition(*iter, strDevName);
            lstDevNames.remove(strDevName);
            lstDevNames.push_back(strDevName);
        }
        
        break;
        
    default:
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupport Freeze file system volume Type: %d.", iVolType);
        
        return ERROR_COMMON_FUNC_UNIMPLEMENT;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get dev name by mount point %s succ.", strMountPoint.c_str());

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :判断是否为本地Lun
Input        : lstDevNames -- 设备名称，带/dev/
Output       : 
Return       : MP_TRUE
                  MP_FALSE
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CFileSys::CheckLocalLun(list<mp_string>& lstDevNames)
{
    LOGGUARD("");

    mp_int32 iRet = MP_SUCCESS;
    mp_string strVendor;
    mp_string strProduct;
    list<mp_string>::iterator itdev;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to Check dev name is the Huawei lun or not.");

    for (itdev = lstDevNames.begin(); itdev != lstDevNames.end(); ++itdev)
    {
        //厂商和型号
        iRet = CArray::GetArrayVendorAndProduct(*itdev, strVendor, strProduct);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The device(%s) get array vendor and product failed.", itdev->c_str());

            return MP_FALSE;
        }

        (void)CMpString::Trim((mp_char*)strVendor.c_str());
        (void)CMpString::Trim((mp_char*)strProduct.c_str());
        //非华为的产品
        if (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The device(%s) is not huawei LUN, Vendor:%s.", 
                itdev->c_str(), strVendor.c_str());

            return MP_FALSE;
        }

        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The device(%s) is huawei LUN, Vendor:%s, Product is: %s.", 
                itdev->c_str(), strVendor.c_str(), strProduct.c_str());
    }


    return MP_TRUE;
    
}

#endif //WIN32

#if (defined HP_UX_IA) || (defined SOLARIS)
/*------------------------------------------------------------ 
Description  :根据设备名查询文件系统类型
Input        : strDevFullPath -- 设备名
Output       : strFileSysType -- 文件系统类型(vxfs或者hfs)
Return       : MP_SUCCESS -- 返回成功
                 非MP_SUCCESS -- 返回失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CFileSys::QueryFileSysType(mp_string strDevFullPath, mp_string& strFileSysType)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecFileSysType;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"fstyp %s\" will be exceuted.", strDevFullPath.c_str());
    
    iRet = CRootCaller::Exec(ROOT_COMMAND_FSTYP, strDevFullPath, &vecFileSysType);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_MOUNT_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query filesystem type failed, iRet %d.", iRet);
        return iRet;
    }

    strFileSysType = vecFileSysType.front();
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query file sys type by device(%s) succ.", strDevFullPath.c_str());
}
#endif

