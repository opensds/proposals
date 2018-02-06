/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef WIN32
#include "device/Raw.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "array/Array.h"

#include <sys/stat.h>
#include <sstream>

CRaw::CRaw()
{
}

CRaw::~CRaw()
{
}

#if defined LINUX
/*------------------------------------------------------------
Function Name: Create
Description  : 创建裸设备
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Create(raw_info_t& rawInfo)
{
    LOGGUARD("");
    //先判断raw服务是否启动
    mp_int32 iRet = StartRawService();
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_START_FAILED);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check raw service failed.");
        return iRet;
    }

    //判断裸设备是否存在
    if(CMpFile::FileExist(rawInfo.strRawDevPath.c_str()))
    {
        mp_string strUsedDevByRaw;
        iRet = GetDeviceUsedByRaw(rawInfo.strRawDevPath, strUsedDevByRaw);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_GET_DEV_FAILED);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get block device \"%s\" for raw failed.",
                     rawInfo.strRawDevPath.c_str());
            return iRet;
        }
        if (rawInfo.strDevName == strUsedDevByRaw)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The raw dev \"%s\" is bounded to the right block dev.",
                rawInfo.strDevName.c_str());
            return MP_SUCCESS;
        }
        else if (!strUsedDevByRaw.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The raw dev is not bounded to the expected block dev, expected dev %s, curr dev %s.",
                rawInfo.strDevName.c_str(), strUsedDevByRaw.c_str());
            return ERROR_DEVICE_RAW_USED_BY_OTHER_DEV;
        }
        //当strUsedDevByRaw为空字符串是存在裸设备文件，但是对应的块设备文件已经不存在
        //未删除裸设备时去映射可能存在这样的场景
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The bounded dev is not exist, bound raw %s to dev %s.",
                rawInfo.strRawDevPath.c_str(), rawInfo.strDevName.c_str());

            //删除该裸设备以后再创建
            iRet = Delete(rawInfo);
            TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_DELETE_FAILED);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete raw device before recreate it again failed, raw device %s.",
                    rawInfo.strRawDevPath.c_str());
                return iRet;
            }
        }
    }

    //判断主端设备是否是分区
    mp_string::size_type pos = rawInfo.strDevName.find_first_of(SECTION_NUM_TAG);
    mp_bool bSection = MP_FALSE;
    mp_string strSectionNum;
    if (mp_string::npos != pos)
    {
        bSection = MP_TRUE;
        strSectionNum = rawInfo.strDevName.substr(pos, pos + 1);
    }

    //根据LUN WWN获取其所在的设备名称
    mp_string strDevName;
    iRet = CDisk::GetDevNameByWWN(strDevName, rawInfo.strLunWWN);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_NOT_HUAWEI_LUN);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device name by wwn faild, wwn is \"%s\".",
                    rawInfo.strLunWWN.c_str());
        return iRet;
    }
    if (bSection)
    {
        strDevName = strDevName + strSectionNum;
    }

    //server能保证裸设备参数正确，上述操作只是做检查
    mp_string strParam = rawInfo.strRawDevPath + " " + rawInfo.strDevName;
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_RAW, strParam, NULL, ERROR_DEVICE_RAW_CREATE_FAILED);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device, raw device name \"%s\", block device name \"%s\".",
               rawInfo.strRawDevPath.c_str(), rawInfo.strDevName.c_str());

    //创建裸设备以后对应的/dev/raw/rawx文件不是立即生成，一般会很快生成，原理是内核发消息给udevd，然后生成raw设备文件
    //创建完裸设备文件以后调用sync系统调用刷新文件系统缓存到磁盘
    sync();

    mp_bool bIsRawExist = CMpFile::WaitForFile(rawInfo.strRawDevPath.c_str(), CHECK_RAW_DEVICE_INTERVAL, CHECK_RAW_DEVICE_COUNT);
    if (!bIsRawExist)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Raw device file \"%s\" dose not exist.", rawInfo.strRawDevPath.c_str());
        return ERROR_COMMON_DEVICE_NOT_EXIST;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device successful.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Delete
Description  : 删除裸设备
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Delete(raw_info_t& rawInfo)
{
    LOGGUARD("");
#if defined LINUX
    //判断裸设备是否存在
    if(!CMpFile::FileExist(rawInfo.strRawDevPath.c_str()))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The raw device \"%s\" dose not exist.",
            rawInfo.strRawDevPath.c_str());
        return MP_SUCCESS;

    }

    mp_string strParam = rawInfo.strRawDevPath + " 0 0";
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_RAW, strParam, NULL, ERROR_DEVICE_RAW_DELETE_FAILED);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Delete raw device successful.");
    return MP_SUCCESS;
#endif
#if defined AIX
    mp_string strDevName;
    mp_int32 iRet = CDisk::GetDevNameByWWN(strDevName, rawInfo.strLunWWN);
    //如果通过wwn查询不到设备名，直接返回成功
    if (ERROR_COMMON_DEVICE_NOT_EXIST == iRet)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Dev of WWN(%s) is not exist.", rawInfo.strLunWWN.c_str());
        return MP_SUCCESS;
    }

    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_NOT_HUAWEI_LUN);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute GetDevNameByWWN failed, iRet %d.", iRet);
        return iRet;
    }

    if (strDevName != rawInfo.strDevName.substr(strlen("/dev/")))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Dev name(%s) of WWN(%s) is not equal to %s.",
            strDevName.c_str(), rawInfo.strLunWWN.c_str(), rawInfo.strDevName.c_str());
        
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_string strParam = "-dl " + strDevName;
    return CRootCaller::Exec(ROOT_COMMAND_RMDEV, strParam, NULL);
#endif
}

/*------------------------------------------------------------
Function Name: StartRawService
Description  : 启动裸设备服务
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::StartRawService()
{
#ifdef SUSE
    //判断raw service是否已经启动
    mp_string strParam = "|grep '^raw'";
    mp_int32 iRet = CRootCaller::Exec(ROOT_COMMAND_LSMOD, strParam, NULL);
    if (iRet == MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Raw service is already started.");
        return MP_SUCCESS;
    }

    strParam = "raw start";
    ROOT_EXEC(ROOT_COMMAND_SERVICE, strParam, NULL);
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetDeviceUsedByRaw
Description  : 获取裸设备所使用的设备
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::GetDeviceUsedByRaw(mp_string& strRawDevPath, mp_string& strUsedDevName)
{
    mp_int32 iBoundMajor, iBoundMinor;
    mp_int32 iRet = GetBoundedDevVersions(strRawDevPath, iBoundMajor, iBoundMinor);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device version for raw device \"%s\" failed.",
              strRawDevPath.c_str());
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get device version succ, raw device \"%s\", bound major %d, bound minor %d.",
            strRawDevPath.c_str(), iBoundMajor, iBoundMinor);

    //获取/dev目录下所有设备文件列表
    vector<mp_string> vecFileNames;
    mp_string strDir = "/dev";
    iRet = CMpFile::GetFolderFile(strDir, vecFileNames);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get file list in dir \"%s\" failed.", strDir.c_str());
        return iRet;
    }

    if (0 == vecFileNames.size())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Dir \"%s\" is empty.", strDir.c_str());
        return MP_SUCCESS;
    }

    mp_string strFilePath;
    mp_int32 iMajorTmp, iMinorTmp;
    for (vector<mp_string>::iterator iter = vecFileNames.begin(); iter != vecFileNames.end(); iter++)
    {
        strFilePath = strDir + PATH_SEPARATOR + *iter;
        iRet = GetDeviceNumber(strFilePath, iMajorTmp, iMinorTmp);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get major and minor version for \"%s\" failed",
                strFilePath.c_str());
            return iRet;
        }
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get dev version succ, dev name \"%s\", major %d, minor %d.",
            strFilePath.c_str(), iMajorTmp, iMinorTmp);

        if (iMajorTmp == iBoundMajor && iMinorTmp == iBoundMinor)
        {
            strUsedDevName = strFilePath;
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device \"%s\" was already bounded to the raw device %s.",
                strUsedDevName.c_str(), strRawDevPath.c_str());
            break;
        }
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetBoundedDevVersions
Description  : 获取bounded版本
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::GetBoundedDevVersions(mp_string& strRawDevPath, mp_int32& iBoundMajor, mp_int32& iBoundMinor)
{
    struct stat sb;
    mp_int32 iMajor = 0;
    mp_int32 iMinor = 0;

    mp_int32 iRet = stat(strRawDevPath.c_str(), &sb);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stat raw device failed, raw device \"%s\", errno %d.",
            strRawDevPath.c_str(), errno);
        return MP_FAILED;
    }

    if (!S_ISCHR(sb.st_mode))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "\"%s\" is not a character device.",
            strRawDevPath.c_str());
        return MP_FAILED;
    }

    //major和minor在头文件<sys/sysmacros.h>中定义
    iMajor = major(sb.st_rdev); //lint !e747
    iMinor = minor(sb.st_rdev); //lint !e747
    if (iMajor != RAW_MAJOR)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "\"%s\" is not a raw device.",
            strRawDevPath.c_str());
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get raw device version succ, raw device \"%s\", major %d, minor %d.",
        strRawDevPath.c_str(), iMajor, iMinor);

    //将iMinor作为参数传给rootcaller
    ostringstream oss;
    oss << iMinor;
    mp_string strParam = oss.str();
    vector<mp_string> vecRlt;
    ROOT_EXEC(ROOT_COMMAND_RAW_MINOR_MAJOR, strParam, &vecRlt);

    if (vecRlt.size() != 2)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Size of vecRlt is not 2.");
        return MP_FAILED;
    }
    iBoundMajor = atoi(vecRlt[0].c_str());
    iBoundMinor = atoi(vecRlt[1].c_str());
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get raw device bounded info succ, raw device \"%s\", bound major %d, bound minor %d.",
           strRawDevPath.c_str(), iBoundMajor, iBoundMinor);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetDeviceNumber
Description  : 获取裸设备序列号
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::GetDeviceNumber(mp_string& rstrDeviceName, mp_int32& iMajor, mp_int32& iMinor)
{
    struct stat sb;
    mp_int32 iRet = stat(rstrDeviceName.c_str(), &sb);
    if (0 != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "stat raw dev \"%s\" failed.", rstrDeviceName.c_str());
        return MP_FAILED;
    }

    iMajor = major(sb.st_rdev); //lint !e747
    iMinor = minor(sb.st_rdev); //lint !e747

    return MP_SUCCESS;
}
#endif

#if defined AIX
/*------------------------------------------------------------
Function Name: Create
Description  : 创建裸设备
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Create(raw_info_t& rawInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDev;
    mp_string strRawDevPath;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_DEBUG, 
        "Begin to create raw device, device name is %s, rawdevice name is %s.", 
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
    strDev = rawInfo.strDevName.substr(strlen("/dev/"));

    // 判断裸设备下存储附加设备是否和设备名称相同
    strRawDevPath = rawInfo.strRawDevPath.substr(0,5) + rawInfo.strRawDevPath.substr(6);
    //不相同则说明是mknod创建的设备，需要创建一次
    if (rawInfo.strDevName != strRawDevPath)
    {
        // 通过mknod创建设备文件
        iRet = CreateRAWDevice(rawInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateDeviceByMknod failed.");
            return iRet;
        }
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device succ.");
        return MP_SUCCESS;
    }
    
    //在AIX下，首先半段上面下发的dev名称是否为hdisk,应该考虑这样的场景:
    //将hdisk2改名为oraasm2,但是去映射之后重新映射上来,名字又变成hdiskxx，所以需要根据www找到hdisk然后重命名.
    mp_bool bIsHdisk = CDisk::IsHdisk(strDev);
    if (!bIsHdisk)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Raw device name(%s) isn't hdisk, need to rename.", strDev.c_str());
        mp_string strHdiskName;
        mp_bool bIsDevExist = MP_FALSE;
        mp_bool bRet = MP_FALSE;
        iRet = CDisk::GetDevNameByWWN(strHdiskName, rawInfo.strLunWWN);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device name failed, WWN: %s.", rawInfo.strLunWWN.c_str());
            return iRet;
        }
    
        mp_string strParam = "\"" + strHdiskName + "\" -n \"" + strDev + "\"";
        vector<mp_string> vecResult;
        
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RENDEV, strParam, &vecResult);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Change dev name failed, param: rendev -l %s, iRet: %d.", strParam.c_str(), iRet);
            return iRet;
        }
    
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Raw device rename success, before rename: %s, after rename: %s.", strHdiskName.c_str(), strDev.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Delete
Description  : 删除裸设备
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Delete(raw_info_t& rawInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDev;
    mp_string strRawDevPath;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_DEBUG, 
        "Begin to delete raw device, device name is %s, rawdevice name is %s.", 
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
    strDev = rawInfo.strDevName.substr(strlen("/dev/"));
    //判断裸设备是否存在
    if(CMpFile::FileExist(rawInfo.strRawDevPath.c_str()))
    {

        // 判断裸设备下存储附加设备是否和设备名称相同
        strRawDevPath = rawInfo.strRawDevPath.substr(0,5) + rawInfo.strRawDevPath.substr(6);
        //不相同则说明是mknod创建的设备，需要删除
        if (rawInfo.strDevName != strRawDevPath)
        {
            // 删除通过mknod创建的设备文件
            ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_RM, rawInfo.strRawDevPath, NULL, ERROR_DEVICE_RAW_DELETE_FAILED);
        }
    }
    
    mp_string strParam = "-dl " + strDev;
    return CRootCaller::Exec(ROOT_COMMAND_RMDEV, strParam, NULL);
}

/*------------------------------------------------------------
Description  : 通过mknod创建设备
Input        : rstMountDiskInfo -- 挂载信息
Output       :
Return       : MP_SUCCESS -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::CreateRAWDevice(raw_info_t& rawInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string majorNumDevice = "", minorNumDevice = "", majorNumMount = "", minorNumMount = "";
    mp_bool isExists = MP_FALSE;
    mp_string strDeviceName = "";

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to create Device handle by mknod, device name %s, to create %s.",
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());

    strDeviceName = rawInfo.strDevName.insert(5,"r");
    // 获取原始设备设备号
    iRet = GetDeviceNumber(strDeviceName, majorNumDevice, minorNumDevice);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get AIX rhisk device number failed, device name %s",
            strDeviceName.c_str());

        return iRet;
    }

    // 判断需要创建设备是否存在
    iRet = IsDeviceExists(rawInfo.strRawDevPath, isExists);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check raw device failed, raw device name %s",
            rawInfo.strRawDevPath.c_str());

        return iRet;
    }

    // 如果当前设备已经存在需要判断是否是准备创建的设备
    if (MP_TRUE == isExists)
    {
        iRet = CheckDeviceByDeviceNumber(rawInfo, strDeviceName, majorNumMount, minorNumMount, majorNumDevice, minorNumDevice);
        if(MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Compare raw device(%s) and mknod device(%s) by device number failed .", 
            rawInfo.strRawDevPath.c_str(), strDeviceName.c_str());
            return iRet;
        }
    }
     // 需要创建的设备不存在，直接创建
    else
    {
        iRet = CreateDeviceByMknod(rawInfo.strRawDevPath, majorNumDevice, minorNumDevice);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create device(%s) by mknod(%s) failed.", 
                rawInfo.strRawDevPath.c_str(), strDeviceName.c_str());
            return iRet;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to create Device handle by mknod, device name %s, to create %s.",
        strDeviceName.c_str(), rawInfo.strRawDevPath.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :  根据DFS更新设备信息
Input        :
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CRaw::CheckDeviceByDeviceNumber(raw_info_t& rawInfo, mp_string strDeviceName, mp_string &majorNumMount, mp_string &minorNumMount, 
    mp_string majorNumDevice, mp_string minorNumDevice)
{
        mp_int32 iRet = MP_SUCCESS;
        iRet = GetDeviceNumber(rawInfo.strRawDevPath, majorNumMount, minorNumMount);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get AIX device number of raw device failed, device name %s.",
                rawInfo.strRawDevPath.c_str());
            return iRet;
        }

        // 判断通过mknod的设备是否是准备创建的设备
        mp_bool bEqual = majorNumDevice != majorNumMount || minorNumDevice != minorNumMount;
        if (bEqual)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The device (%s) is created by device (major %s, minor %s), do nothing do mknod.",
                rawInfo.strRawDevPath.c_str(), majorNumMount.c_str(), minorNumMount.c_str());
            return ERROR_DEVICE_RAW_USED_BY_OTHER_DEV;
        }
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The device (%s) is created by device (%s), do nothing do mknod.",
                rawInfo.strRawDevPath.c_str(), strDeviceName.c_str());
        }

        return MP_SUCCESS;
}
#endif

#ifdef HP_UX_IA
/*------------------------------------------------------------
Function Name: Create
Description  : 创建裸设备(之前设备名和裸设备名均为生产端名称，
               现均优化为灾备端名称)
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Create(raw_info_t& rawInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strRawDevice;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, 
		"Begin to create raw device, device name is (%s), raw device name is (%s).", 
		rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
    //如果是裸设备则取备端hdisk进行权限赋值，如果lv则主备端名字不会变化
    if (CDisk::IsDskdisk(rawInfo.strDevName))
    {
        // 判断裸设备下存储附加设备是否和设备名称相同
        // 如果相同的设备为/dev/dsk/cxxdxxtxx和/dev/rdsk/cxxdxxtxx
        mp_string strRawDevPath;
        strRawDevPath = rawInfo.strRawDevPath.substr(0,5) + rawInfo.strRawDevPath.substr(6);

        //不相同则说明是mknod创建的设备，需要创建一次
        if (rawInfo.strDevName != strRawDevPath)
        {
            // 通过mknod创建设备文件
            iRet = CreateRAWDevice(rawInfo);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateDeviceByMknod failed.");
                return iRet;
            }
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Delete
Description  : 删除裸设备
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Delete(raw_info_t& rawInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strPersistentDSF;
    mp_string strRawDevPath;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to delete raw device, raw device %s.",
            rawInfo.strRawDevPath.c_str());

    //判断裸设备是否存在
    if(!CMpFile::FileExist(rawInfo.strRawDevPath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The raw device \"%s\" dose not exist.",
            rawInfo.strRawDevPath.c_str());
        return MP_SUCCESS;

    }

    // 判断裸设备下存储附加设备是否和设备名称相同
    // 如果相同的设备为/dev/dsk/cxxdxxtxx和/dev/rdsk/cxxdxxtxx
    strRawDevPath = rawInfo.strRawDevPath.substr(0,5) + rawInfo.strRawDevPath.substr(6);
    //不相同则说明是mknod创建的设备，需要删除
    if (rawInfo.strDevName!= strRawDevPath)
    {
        iRet = DeleteDevice(rawInfo.strRawDevPath);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_DELETE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete mount path (%s) failed.",
                rawInfo.strRawDevPath.c_str());
            return iRet;
        }
    }

    // 2014-11-20:
    //不能删除persistent disk，如果删除后也会自动生成，导致有IO无法去映射
    //
    // 2014-11-19:
    //删除 persistent disk，如果使用disk mknod的设备，在上面deleteDevice后就会直接
    // 删除对应disk设备，此处会删除失败，不判断操作结果
    //iRet = DeletePersistentDSF(strPersistentDSF);
    //if (ISSP_RTN_OK != iRet)
    //{
    //    COMMLOG(OS_LOG_ERROR, LOG_AGENT_INTERNAL_ERROR, "Delete persistent DSF (%s) failed.",
    //        strPersistentDSF.c_str());
    //    //return ISSP_RTN_FAIL;
    //}
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete raw device %s succ.", rawInfo.strRawDevPath.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 通过mknod创建设备
Input        : rstMountDiskInfo -- 挂载信息
Output       :
Return       : MP_SUCCESS -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::CreateRAWDevice(raw_info_t& rawInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string majorNumDevice = "", minorNumDevice = "", majorNumMount = "", minorNumMount = "";
    mp_bool isExists = MP_FALSE;
    mp_bool isPDMk = MP_TRUE;
    mp_string strDeviceName = "";

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to create Device handle by mknod, device name %s, to create %s.",
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
	
    iRet = CDisk::GetHPRawDiskName(rawInfo.strDevName, strDeviceName);
	if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get raw disk name of disk(%s) failed, ret %d.",
                rawInfo.strDevName.c_str(), iRet);

        return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
    }

    // 获取原始设备设备号
    iRet = GetDeviceNumber(strDeviceName, majorNumDevice, minorNumDevice);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get HP Disk SN failed, device name %s",
            rawInfo.strDevName.c_str());

        return iRet;
    }

    // 判断需要创建设备是否存在?
    iRet = IsDeviceExists(rawInfo.strRawDevPath, isExists);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check device failed, device name %s",
            rawInfo.strRawDevPath.c_str());

        return iRet;
    }

    // 如果当前设备已经存在需要判断是否是准备创建的设备
    if (MP_TRUE == isExists)
    {
        iRet = UpdateDeviceByDsf(rawInfo, majorNumMount, minorNumMount, majorNumDevice, minorNumDevice);
        if(MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "UpdateDeviceByDsf failed.");
            return iRet;
        }
    }
     // 需要创建的设备不存在，直接创建
    else
    {
        iRet = CreateDeviceByMknod(rawInfo.strRawDevPath, majorNumDevice, minorNumDevice);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateDeviceByMknod failed");
            return iRet;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to create Device handle by mknod, device name %s, to create %s.",
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 获取Device info
Input        : strMajor -- Device 的major
                 strMinor -- Device 的minor
Output       : deviceStatus -- 设备状态 1不存在 2NO_HW状态 3正常状态
Return       : MP_SUCCESS -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::GetDeviceStatus(mp_string &strMajor, mp_string &strMinor, mp_char &deviceStatus)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult1;
    vector<mp_string> vecResult2;
    mp_bool bRet;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to get device status(m %s, n %s).",
        strMajor.c_str(), strMinor.c_str());

    //ls -l /dev/rdisk/* | awk '{if ($6=="0x550600") print $NF}'; ls -l /dev/rdsk/* | awk '{if ($6=="0x550600") print $NF}'

    strParam = " /dev/rdisk/* | awk '{if ($5== \"" + strMajor + "\") print $NF\" \"$6}' | awk '{if ($2== \""
        + strMinor + "\") print $1}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device failed m %s, n %s.", strMajor.c_str(), strMinor.c_str());
        return iRet;
    }

    strParam = " /dev/rdsk/* | awk '{if ($5== \"" + strMajor + "\") print $NF\" \"$6}' | awk '{if ($2== \""
        + strMinor + "\") print $1}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult2);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device failed m %s, n %s.", strMajor.c_str(), strMinor.c_str());
        return iRet;
    }

    // 设备不存在
    bRet = (vecResult1.empty()&&vecResult2.empty());
    if (bRet)
    {
        deviceStatus = HP_DEVICE_STATUS_NOEXISTS;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status(m %s, n %s), Device is not exists.",
            strMajor.c_str(), strMinor.c_str());
        return MP_SUCCESS;
    }

    // 获取设备状态
    mp_string strDevice = vecResult1.empty()?vecResult2.front():vecResult1.front();
    vecResult1.clear();
    vecResult2.clear();

    // 存在disk设备存在，但是通过ioscan无法查询出来的情况
    //ioscan -funC disk -N | grep /dev/rdisk/disk421; ioscan -funC disk | grep /dev/rdisk/disk421

    strParam = " -funC disk -N | grep " + strDevice;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device by ioscan failed, device %s.", strDevice.c_str());
        return iRet;
    }

    strParam = " -funC disk | grep " + strDevice;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult2);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device by ioscan failed, device %s.", strDevice.c_str());
        return iRet;
    }

    // 设备不存在
    bRet = (vecResult1.empty()&&vecResult2.empty());
    if (bRet)
    {
        deviceStatus = HP_DEVICE_STATUS_NOEXISTS;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status by ioscan(device %s), device is not valid.",
            strDevice.c_str());
        return MP_SUCCESS;
    }
    vecResult1.clear();
    vecResult2.clear();

    // 判断该设备是否是NO_HW状?
    //ioscan -funC disk -N /dev/rdisk/disk421 | grep NO_HW; ioscan -funC disk /dev/rdisk/disk421 | grep NO_HW

    strParam = " -funC disk -N " + strDevice + " | grep NO_HW";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device status by ioscan failed, device %s.", strDevice.c_str());
        return iRet;
    }

    strParam = " -funC disk " + strDevice + " | grep NO_HW";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult2);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device status by ioscan failed, device %s.", strDevice.c_str());
        return iRet;
    }

    // 设备时候NO_HW状态
    bRet = (!vecResult1.empty()||!vecResult2.empty());
    if (bRet)
    {
        deviceStatus = HP_DEVICE_STATUS_NOHW;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status by ioscan(device %s), device S/W status is NO_HW.",
            strDevice.c_str());
    }
    else
    {
        deviceStatus = HP_DEVICE_STATUS_NORMAL;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status by ioscan(device %s), device S/W status is normal.",
            strDevice.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status(m %s, n %s).",
        strMajor.c_str(), strMinor.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 删除设备
Input        : strDeviceName -- 需要删除设备的设备名称
Output       :
Return       : MP_SUCCESS -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::DeleteDevice(mp_string& strDeviceName)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    mp_bool bDeviceExists = MP_FALSE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to delete device %s.", strDeviceName.c_str());

    iRet = IsDeviceExists(strDeviceName, bDeviceExists);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check device %s exists failed.", strDeviceName.c_str());
        return MP_FAILED;
    }

    if (MP_FALSE == bDeviceExists)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device %s is not exists.", strDeviceName.c_str());
        return MP_SUCCESS;
    }

    strParam = " -a " + strDeviceName;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RMSF, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete device %s failed.", strDeviceName.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to delete device %s.", strDeviceName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :  根据DFS更新设备信息
Input        :
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CRaw::UpdateDeviceByDsf(raw_info_t& rawInfo, mp_string &majorNumMount, mp_string &minorNumMount, 
    mp_string majorNumDevice, mp_string minorNumDevice)
{
        mp_int32 iRet = MP_SUCCESS;
        iRet = GetDeviceNumber(rawInfo.strRawDevPath, majorNumMount, minorNumMount);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get HP Device SN failed, device name %s",
                rawInfo.strRawDevPath.c_str());

            return iRet;
        }

        // 判断通过mknod的设备是否是准备创建的设备
        // 如果不是需要判断设备状态
        //1.创建mk的设备已经不存在
        //2.创建mk的设备已经是NO_HW状态
        //3.创建mk的设备时正常状态
        mp_bool bEqual = majorNumDevice != majorNumMount || minorNumDevice != minorNumMount;
        if (bEqual)
        {
            mp_char deviceStatus = 0;
            iRet = GetDeviceStatus(majorNumMount, minorNumMount, deviceStatus);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDeviceStatus failed");
                return iRet;
            }

            // 如果创建的设备状态异常，则删除mountpath
            mp_bool needDel = (HP_DEVICE_STATUS_NOEXISTS == deviceStatus || HP_DEVICE_STATUS_NOHW == deviceStatus);
            if (needDel)
            {
                iRet = DeleteDevice(rawInfo.strRawDevPath);
                if (MP_SUCCESS != iRet)
                {
                    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateDeviceByMknod failed");
                    return iRet;
                }
            }
            // 设备名称已经被其他占用，返回错误
            else
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The device (%s) is created by device (major %s, minor %s), do nothing do mknod.",
                    rawInfo.strRawDevPath.c_str(), majorNumMount.c_str(), minorNumMount.c_str());
                return ERROR_DEVICE_RAW_USED_BY_OTHER_DEV;
            }
        }
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The device (%s) is created by device (%s), do nothing do mknod.",
                rawInfo.strRawDevPath.c_str(), rawInfo.strDevName.c_str());
        }

        return MP_SUCCESS;
}
#endif

#if defined SOLARIS
/*------------------------------------------------------------
Function Name: Create
Description  : 创建裸设备
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Create(raw_info_t& rawInfo)
{
    //solaris下裸设备无需创建直接返回成功
    LOGGUARD("");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Delete
Description  : 删除裸设备
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Delete(raw_info_t& rawInfo)
{
    LOGGUARD("");
    return MP_SUCCESS;
}
#endif

#if (defined HP_UX_IA) || (defined AIX)
/*------------------------------------------------------------
Function Name: GetDeviceNumber
Description  : 获取裸设备主次设备号
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::GetDeviceNumber(mp_string& rstrDeviceName, mp_string& majorNum, mp_string& minorNum)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strRes;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to get Device number, device name %s.",
        rstrDeviceName.c_str());

    // ll /dev/rdsk/c84t0d3  | awk '{print $5,$6}'"
#ifdef HP_UX_IA
    strParam = " " + rstrDeviceName + "| awk '{print $5,$6}'";
#elif AIX
    strParam = " " + rstrDeviceName + "| awk '{print $5,$6}' | tr -d ','";
#endif
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Device number failed, device name %s.",
            rstrDeviceName.c_str());

        return MP_FAILED;
    }

    if(!vecResult.empty())
    {
        strRes = vecResult.front();
    }
    vecResult.clear();

    // 正常返回结果188 0x540300
    size_t blackIndex = strRes.find(" ");
    if (mp_string::npos == blackIndex)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Device number failed, device name %s, find %s.",
            rstrDeviceName.c_str(), strRes.c_str());
        return MP_FAILED;
    }
    majorNum = strRes.substr(0, blackIndex);
    minorNum = strRes.substr(blackIndex + 1);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Device number succ, device name %s, major %s, minor %s.",
        rstrDeviceName.c_str(), majorNum.c_str(), minorNum.c_str());
    return MP_SUCCESS;
}
#endif

/*------------------------------------------------------------
Description  : 判断disk是否存在
Input        : rstrDeviceName -- 设备名称
Output       : isDeviceExist -- ISSP_TRUE 设备存在、ISSP_FALSE 设备不存在
Return       : MP_SUCCESS -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::IsDeviceExists(mp_string& rstrDeviceName, mp_bool& isDeviceExist)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strRes;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to check Device exists, device name %s.",
        rstrDeviceName.c_str());

    strParam = " " + rstrDeviceName + " | wc -l";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check Device exists failed, device name %s.",
            rstrDeviceName.c_str());

        return MP_FAILED;
    }

    if(!vecResult.empty())
    {
        strRes = vecResult.front();
        (void)CMpString::Trim((mp_char*)strRes.c_str());
    }
    vecResult.clear();

    if (0 == strcmp(strRes.c_str(), "0"))
    {
        isDeviceExist = MP_FALSE;
    }
    else
    {
        isDeviceExist = MP_TRUE;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Check Device exists succ, device name %s.",
        rstrDeviceName.c_str());

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :  通过mknod创建设备
Input        : strDeviceName -- 要创建的设备
                  strMajorNum--原设备的主设备号
                  strMinorNum--原设备的次设备号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CRaw::CreateDeviceByMknod(mp_string &strDeviceName, mp_string strMajorNum,
    mp_string strMinorNum)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to create device %s by mknod with major %s, minor %s.",
            strDeviceName.c_str(), strMajorNum.c_str(), strMinorNum.c_str());

    // like this "mknod ora_data_05 c 188 0x540300"

    strParam = strDeviceName + " c " + strMajorNum + " " +strMinorNum;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_MKNOD, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "mknod device failed, major %s, minor %s, to create %s.",
            strMajorNum.c_str(), strMinorNum.c_str(), strDeviceName.c_str());

        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to create device %s by mknod with major %s, minor %s.",
            strDeviceName.c_str(), strMajorNum.c_str(), strMinorNum.c_str());
    return MP_SUCCESS;
}

#endif //WIn32

