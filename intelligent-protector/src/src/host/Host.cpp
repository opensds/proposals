/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "host/Host.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Uuid.h"
#include "common/SystemExec.h"
#include "common/Defines.h"
#include "common/RootCaller.h"
#include "common/Path.h"
#include "array/Array.h"
#include "common/Ip.h"
#include "common/Sign.h"
#include "common/Utils.h"
#include "common/UniqueId.h"
#include "securec.h"
#include "common/CryptAlg.h"
#include <sstream>

CHost::CHost()
{
    CMpThread::InitLock(&m_pMutex);
}

CHost::~CHost()
{
    CMpThread::DestroyLock(&m_pMutex);
}

/*------------------------------------------------------------ 
Description  : 保存主机SN号
Input        : vecInput -- SN号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::SetHostSN(vector<mp_string> &vecInput)
{
    mp_int32 iRet = MP_SUCCESS;
    iRet = CIPCFile::WriteFile(m_hostsnFile, vecInput);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetHostSN: Write hostsn into hostsn file failed [%d].", iRet);
        return iRet;
    }

    // 修改权限
#ifdef WIN32
    mp_string strCommand = "cmd.exe /c echo Y | cacls.exe \"" + m_hostsnFile + "\" /E /R Users";

    iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "cacls hostsn file failed %d.", iRet);

        return iRet;
    }
#else
    if (chmod(m_hostsnFile.c_str(), S_IRUSR | S_IWUSR) == -1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "chmod hostsn %s failed, errno[%d]:%s.", m_hostsnFile.c_str(),
            errno, strerror(errno));
        return MP_FAILED;
    }
#endif

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Write file succ.");

    return MP_SUCCESS;

}

/*------------------------------------------------------------ 
Description  : 读取主机SN号
Input        : 
Output       : vecMacs -- SN号
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::ReadHostSNInfo(vector<mp_string>& vecMacs)
{
    m_hostsnFile = CPath::GetInstance().GetConfFilePath(HOSTSN_FILE);

    mp_int32 iRet = CMpFile::ReadFile(m_hostsnFile, vecMacs);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "GetHostSN: Read host sn file failed [%d].", iRet);
        return iRet;
    }

    if (0 == vecMacs.size())
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetHostSN: HostSN file is NULL.");
         return MP_FAILED;
    }

    return iRet;
}

/*------------------------------------------------------------ 
Description  :查询主机SN号
Input        : 
Output       : strSN -- SN号
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::GetHostSN(mp_string& strSN)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strUuid;
    vector<mp_string> vecMacs;
    vector<mp_string> vecMacsTmp;

    CThreadAutoLock cLock(&m_pMutex);

    iRet = CHost::ReadHostSNInfo(vecMacs);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Get host uuid from conf file failed, begin to get uuid.");
#ifndef AIX53
        iRet = CUuidNum::GetUuidNumber(strSN);
#else
        //AIX 5.3没有UUID接口使用，直接使用随机数加时间代替，但是只取16位，
        //如果不满16位不补充
        mp_uint64 iRandom = 0;
        ostringstream buff; 
        iRet = GetRandom(iRandom);
        buff <<iRandom << CMpTime::GetTimeSec();
        strSN = buff.str().substr(0, 16);
#endif
        if (MP_SUCCESS != iRet || "" == strSN)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get uuid failed, iRet %d or strUuid is empty.", iRet);
            
            return iRet;
        }

        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get uuid of this host succ, uuid %s.", strSN.c_str());
        
        // 写入配置文件
        vecMacsTmp.push_back(strSN);
        iRet = CHost::SetHostSN(vecMacsTmp);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write hostsn into hostsn file failed [%d].", iRet);
            
            return iRet;
        }

    }
    else
    {
        if (vecMacs.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get uuid of this host failed, HostSN file is empty.");
            
            return MP_FAILED;
        }
        
        strSN = vecMacs.front();
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get uuid of this host succ, uuid %s.", strSN.c_str());
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询Agent 版本信息
Input        : 
Output       : strAgentVersion -- Agent 版本信息
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::GetAgentVersion(mp_string& strAgentVersion, mp_string& strBuildNum)
{
    LOGGUARD("");
    strAgentVersion = AGENT_VERSION;
    strBuildNum     = AGENT_BUILD_NUM;
    if ("" != strAgentVersion && "" != strBuildNum)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Agent version is %s, build number is %s.", 
            strAgentVersion.c_str(), strBuildNum.c_str());
        
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Agent version or build number is empty.");
    
    return MP_FAILED;
}


/*------------------------------------------------------------ 
Description  : 查询主机类型
Input        : 
Output       : iOSType -- 主机类型
               strOSVersion-- 系统版本
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::GetHostOS(mp_int32& iOSType, mp_string& strOSVersion)
{ 
    LOGGUARD("");
#ifdef WIN32
    mp_char cVerTmp[MAX_SYS_VERSION_LENGTH] = {0};
    mp_int32 dwVersion = GetVersion();

    mp_int32 dwMajorVer = (mp_int32)(LOBYTE(LOWORD(dwVersion)));
    mp_int32 dwMinorVer = (mp_int32)(HIBYTE(LOWORD(dwVersion)));

    CHECK_FAIL(sprintf_s(cVerTmp, sizeof(cVerTmp), "%d.%d", dwMajorVer, dwMinorVer));
    
    strOSVersion = cVerTmp;
    iOSType      = HOST_TYPE_WINDOWS;
    
#else
    mp_int32 iRet = MP_SUCCESS;
    //Linux 非SuSE下返回1个结果,SuSE返回2个结果
    mp_int32 iRltCount = 1;
    mp_string strCmd;
    vector<mp_string> vecRlt;
    
    //sh执行该命令，错误的场景下任然返回0，
    //因此不能使用返回值判断，只能通过输出参数判断

    
    #ifdef REDHAT
    mp_string strFile = "/etc/oracle-release";
    
    if (CMpFile::FileExist(strFile.c_str()))
    {
        iOSType = HOST_TYPE_OEL;
        strCmd = "cat /etc/oracle-release 2>/dev/null | awk '{print $NF}'";
    }
    else
    {
        iOSType = HOST_TYPE_REDHAT;
        strCmd  = "cat /etc/redhat-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'";
    }
    #elif defined(ISOFT)                   
    iOSType = HOST_TYPE_ISOFT;
    strCmd  = "cat /etc/isoft-release | awk '{print$5}'";
    
    #elif defined(SUSE)
    iRltCount = 2;
    iOSType   = HOST_TYPE_SUSE;
    strCmd    = "cat /etc/SuSE-release 2>/dev/null | awk  '$2 == \"=\" {print $3}'";
    
    #elif defined(AIX)
    iOSType   = HOST_TYPE_AIX;
    strCmd    = "uname -vr | awk '{print $2\".\"$1}'";
    
    #elif defined(HP_UX_IA)
    iOSType   = HOST_TYPE_HP_UX_IA;
    strCmd    = "uname -r | awk -F 'B.' '{print $NF}'";
    
    #elif defined(SOLARIS)
    iOSType   = HOST_TYPE_SOLARIS;
    strCmd    = "uname -a | nawk '{print $3}'";
    
    #else
    iOSType = HOST_TYPE_ROCKY;
    //Rocky4
    strCmd  = "cat /etc/linx-release 2>/dev/null | awk '{print $NF}' | awk -F '.' '{print $1\".\"$2}'";
    iRet    = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    
    if (MP_SUCCESS == iRet && iRltCount == vecRlt.size())
    {
        strOSVersion = vecRlt.front();

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Rocky version[%s] and ostype[%d].", strOSVersion.c_str(), iOSType);
        return MP_SUCCESS;
    }

    //Rocky6
    strCmd  = "cat /etc/debian_version 2>/dev/null | awk '{print $NF}' | awk -F '.' '{print $1\".\"$2}'";
    
    #endif

    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (MP_SUCCESS != iRet || iRltCount != vecRlt.size())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host version failed, iRet is %d or vecRlt size must be %d, but %d.",
            iRet, iRltCount, vecRlt.size());
        return MP_FAILED;
    }

    strOSVersion = vecRlt.front();

    #ifdef SUSE
    strOSVersion += ".";
    strOSVersion += vecRlt.back();
    
    #endif    
#endif

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get host version[%s] and ostype[%d].", strOSVersion.c_str(), iOSType);

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询主机信息
Input        : 
Output       : hostInfo -- 主机信息结构体
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::GetInfo(host_info_t& hostInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strHostName;
    mp_string strSN;
    mp_string strOSVersion;
    mp_int32 iOsType;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get host info.");
    iRet = GetHostName(strHostName);
    if (MP_SUCCESS != iRet)
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host name failed.");
         return ERROR_HOST_GETINFO_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get host name %s.", strHostName.c_str());

    iRet = GetHostSN(strSN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host sn failed.");
        return ERROR_HOST_GETINFO_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get host sn %s.", strSN.c_str());

    iRet = GetHostOS(iOsType, strOSVersion);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host OsType and version failed.");
        return ERROR_HOST_GETINFO_FAILED;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get host os %d and version %s.", iOsType, strOSVersion.c_str());

    hostInfo.name    = strHostName;
    hostInfo.sn      = strSN;
    hostInfo.os      = iOsType;
    hostInfo.version = strOSVersion;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get host succ.");
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询主机磁盘信息(HP一个LUN返回2条设备信息，/dev/dsk/和/dev/disk/)
Input        : 
Output       : vecLunInfo -- 主机磁盘信息结构体
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::GetDiskInfo(vector<host_lun_info_t> & vecLunInfo)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get disk info.");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDevName;
    mp_string strVendor;
    mp_string strProduct;
    mp_string strSN;
    mp_string strLunWWN;
    mp_string  strLunID;
    vector<mp_string> vecDiskName;
    vector<mp_string>::iterator iter;
    mp_bool bRet = MP_FALSE;

#ifdef HP_UX_IA
    // HP-UX需要清理无用dsf,防止出现无法发现设备情况，具体请看删除实现说明
    iRet = CDisk::ClearInvalidDisk();
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_GET_DISK_INFO_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ClearInvalidDisk failed.");

        return iRet;
    }

    // 清理磁盘后，扫描磁盘
    iRet = ScanDisk();
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_SCAN_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Execute script(ioscan -fnC disk) failed.");
        return iRet;
    }
#endif

    iRet = CDisk::GetAllDiskName(vecDiskName);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_GET_DISK_INFO_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get bolck devs failed, iRet %d.", iRet);
        return iRet;
    }

    if (vecDiskName.empty())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The disk list is empty.");
        return MP_SUCCESS;
    }

    for (iter = vecDiskName.begin(); iter != vecDiskName.end(); ++iter)
    {
#if defined LINUX||AIX
        // 拼装全路径
        strDevName = mp_string("/dev/") + *iter;
#elif defined WIN32
        strDevName = *iter;
#elif defined HP_UX_IA
        iRet = CDisk::GetHPRawDiskName(*iter, strDevName);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get full disk name of disk(%s) failed, ret %d.",
                iter->c_str(), iRet);

            return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
        }
#elif defined SOLARIS
        strDevName = mp_string("/dev/rdsk/") + *iter + mp_string("s0");
#endif
        //厂商和型号
        iRet = CArray::GetArrayVendorAndProduct(strDevName, strVendor, strProduct);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) get array vendor and product failed.", strDevName.c_str());
            continue;
        }

        (void)CMpString::Trim((mp_char*)strVendor.c_str());
        (void)CMpString::Trim((mp_char*)strProduct.c_str());
        //排除掉非华为的产品
        bRet = (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY));
        if (bRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) is not huawei LUN, Vendor:%s.", strDevName.c_str(), strVendor.c_str());

            continue;
        }

        //获取SN
        iRet = CArray::GetArraySN(strDevName, strSN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) get array SN failed.", strDevName.c_str());

            continue;
        }

        iRet = CArray::GetLunInfo(strDevName, strLunWWN, strLunID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) get lun wwn and lun id failed.", strDevName.c_str());

            continue;
        }

#ifdef WIN32
        //获取disknumber
        mp_int32  iDiskNum;
        iRet = CDisk::GetDiskNum(strDevName, iDiskNum);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) get disk number failed.", strDevName.c_str());

            continue;
        }
#endif

        host_lun_info_t structLunInfo;
        structLunInfo.wwn = strLunWWN;
        structLunInfo.lunId = strLunID;
#ifdef HP_UX_IA
        structLunInfo.deviceName = *iter;
#elif defined SOLARIS
        structLunInfo.deviceName = "/dev/dsk/" + *iter;
#else
        structLunInfo.deviceName = strDevName;
#endif
        structLunInfo.arraySn = strSN;
        structLunInfo.arrayVendor = strVendor;
        structLunInfo.arrayModel = strProduct;

#ifdef WIN32
        mp_char cDiskNum[NAME_PATH_LEN] = {0};
        ITOA(iDiskNum, cDiskNum, NAME_PATH_LEN, DECIMAL);
        structLunInfo.diskNumber = cDiskNum;
#endif

        vecLunInfo.push_back(structLunInfo);
#ifdef HP_UX_IA
        mp_string strSecDevName;
        iRet = CDisk::GetPersistentDSFByLegacyDSF(*iter, strSecDevName);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get persistent DSF failed, device name %s",
                strDevName.c_str());

            return iRet;
        }
        structLunInfo.deviceName = strSecDevName;
        vecLunInfo.push_back(structLunInfo);
#endif
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get disk info succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 查询主机时区信息
Input        : 
Output       : vecLunInfo -- 主机磁盘信息结构体
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::GetTimeZone(timezone_info_t& sttimezone)
{
    mp_char ctzBias[HOST_TIMEZONE_LENGTH] = {0};
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get host time zone info.");

    sttimezone.iIsDST = HOST_ISNOTDST;
    
#ifdef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_long ltotalBias = 0;
    mp_long lBiasHour  = 0;
    mp_long lBiasMin   = 0;
    TIME_ZONE_INFORMATION tzi;

    iRet = GetTimeZoneInformation(&tzi);
    //处理夏令时偏移量对时区的影响
    if (TIME_ZONE_ID_DAYLIGHT == iRet)
    {
        sttimezone.iIsDST = HOST_ISDST;
        ltotalBias = tzi.Bias + tzi.DaylightBias;
    }
    else
    {
        ltotalBias = tzi.Bias;
    }

    lBiasHour = ltotalBias / (HOST_TIMEZONE_CONVERSION_UNIT);
    lBiasMin  = abs(ltotalBias) % (HOST_TIMEZONE_CONVERSION_UNIT);

    CHECK_FAIL(sprintf_s(ctzBias, sizeof(ctzBias), "%+03d%02d", lBiasHour, lBiasMin));
        
#else
    
    time_t tTime;
    tm tLoctime;

    (mp_void)time(&tTime);
    (mp_void)localtime_r(&tTime, &tLoctime);

    if (0 < tLoctime.tm_isdst)
    {
        sttimezone.iIsDST = HOST_ISDST;
    }
    
    #ifdef AIX
    tm tUtctime;
    time_t tLocal_t, tUtc_t;
    mp_double dDifftime_sec = 0;
    mp_int32 iDifftime_hour = 0;
    mp_int32 iDifftime_min  = 0;
    (mp_void)gmtime_r(&tTime, &tUtctime);

    //tm_isdst置于0,防止夏令时偏移量有效时,
    //影响Agent接口偏移量的准确性
    tLoctime.tm_isdst = 0;
    //CodeDex误报，Missing Check against Null
    tLocal_t = mktime(&tLoctime);
    tUtc_t   = mktime(&tUtctime);

    dDifftime_sec = difftime(tLocal_t, tUtc_t);

    iDifftime_hour = (mp_int32)(dDifftime_sec / (HOST_TIMEZONE_CONVERSION_UNIT * HOST_TIMEZONE_CONVERSION_UNIT));
    iDifftime_min  = (mp_int32)(dDifftime_sec / HOST_TIMEZONE_CONVERSION_UNIT) % (HOST_TIMEZONE_CONVERSION_UNIT);
    
    
    CHECK_FAIL(sprintf_s(ctzBias, sizeof(ctzBias), "%+03d%02d", iDifftime_hour, abs(iDifftime_min)));
    
    #else
    //返回的是%z格式化后的字符串长度,如+0800长度为5
    mp_size slen = strftime(ctzBias, sizeof(ctzBias), "%z", &tLoctime);
    if (HOST_TIMEZONE_LENGTH - 1 != slen 
        && HOST_TIMEZONE_LENGTH - 2 != slen)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get UTC Bias failed, return len is not 5 or 4, len is %d.", slen);
        
        return MP_FAILED;
    }

    //HP-UX 下Africa/Casablanca 为0000
    if (HOST_TIMEZONE_LENGTH - 2 == slen)
    {
        sttimezone.strTzBias = "+";
    }
    
    #endif
#endif

    sttimezone.strTzBias += ctzBias;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get time zone succ, tzBias is %s, isDst is %d", 
        sttimezone.strTzBias.c_str(), sttimezone.iIsDST);
    
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 查询主机启动器信息
Input        : 
Output       : initInfo -- 主机启动器信息结构体
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::GetInitiators(initiator_info_t& initInfo)
{
    LOGGUARD("");
    mp_int32 iRetIscsi = MP_SUCCESS;
    mp_int32 iRetFc = MP_SUCCESS;
    mp_string strResultFile;
    mp_string strScriptFileName = SCRIPT_INITIATOR_LINUX;
    mp_string strParam = SCRIPT_INITIATOR_PARAM_ISCSI;
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get initiator infos.");
#ifdef WIN32
    mp_int32 iRettmp = MP_SUCCESS;
    CErrorCodeMap errorCode;
    strScriptFileName = SCRIPT_INITIATOR_WIN;
    iRetIscsi = CSystemExec::ExecScript(strScriptFileName, strParam, &vecResult);
#else
    //Linux下查询iscsci信息时需要读取initiatorname.iscsi文件信息，该文件在Suse10/Suse11下只有root有读权限
    iRetIscsi = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_INIT, strParam, &vecResult);
#endif
    TRANSFORM_RETURN_CODE(iRetIscsi, ERROR_HOST_GET_INIATOR_FAILED);
    if (MP_SUCCESS != iRetIscsi)
    {
   // WIN32 系统下错误码转换
#ifdef WIN32
        iRettmp = errorCode.GetErrorCode(iRetIscsi);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "Exec script failed, initial return code is %d, tranformed return code is %d", iRetIscsi, iRettmp);
        iRetIscsi = iRettmp;
#endif
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Rootcaller exec failed[iscsi], iRet %d.", iRetIscsi);
    }
    else
    {
        for (iter = vecResult.begin(); iter != vecResult.end(); iter++)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get iscsi iqn %s.", iter->c_str());
            (mp_void)CMpString::Trim((mp_char*) iter->c_str());
            initInfo.iscsis.push_back(*iter);
        }
    }

    vecResult.clear();
    strParam = SCRIPT_INITIATOR_PARAM_FC;
#ifdef WIN32
    //iRetFc = CSystemExec::ExecScript(strScriptFileName, strParam, &vecResult);
	iRetFc = m_CHBA.GetHBAInfo(vecResult);
#else
    iRetFc = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_INIT, strParam, &vecResult);
#endif
    TRANSFORM_RETURN_CODE(iRetFc, ERROR_HOST_GET_INIATOR_FAILED);
    if (MP_SUCCESS != iRetFc)
    {
        //WIN32 系统下错误码转换
#ifdef WIN32
        iRettmp = errorCode.GetErrorCode(iRetFc);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "Exec script failed, initiator return code is %d, tranformed return code is %d", iRetFc, iRettmp);
        iRetFc = iRettmp;
#endif
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Rootcaller exec failed[fc], iRet %d.", iRetFc);
    }
    else
    {
        for (iter = vecResult.begin(); iter != vecResult.end(); iter++)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get fc wwn %s.", iter->c_str());
            (mp_void)CMpString::Trim((mp_char*) iter->c_str());
            initInfo.fcs.push_back(*iter);
        }
    }
    
    //获取启动器需要满足的条件
    mp_bool bBothFailed = ( (MP_SUCCESS != iRetIscsi) && (MP_SUCCESS != iRetFc) );
    mp_bool bBothFileExist = (ERROR_COMMON_SCRIPT_FILE_NOT_EXIST != iRetIscsi && ERROR_COMMON_SCRIPT_FILE_NOT_EXIST != iRetFc);
    mp_bool bBothExecSuccess = (ERROR_COMMON_SCRIPT_EXEC_FAILED != iRetIscsi && ERROR_COMMON_SCRIPT_EXEC_FAILED != iRetFc);
    if (bBothFailed)
    {
        if (bBothFileExist)
        {
            if (bBothExecSuccess)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Iscsi and fc initiators are not exist.");
                return ERROR_HOST_GET_INIATOR_FAILED;
            }
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s exec failed.", strScriptFileName.c_str());
            return ERROR_COMMON_SCRIPT_EXEC_FAILED;
        }
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s is not exist.", strScriptFileName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get initiator infos succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 扫盘
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::ScanDisk()
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin scan disk.");
    mp_int32 iRet = MP_SUCCESS;
#ifdef WIN32
    DEVINST devInst;
    CONFIGRET status;

     //得到设备树根节点
    status = CM_Locate_DevNode(&devInst, NULL, CM_LOCATE_DEVNODE_NORMAL);
    if (MP_SUCCESS != status)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CM_Locate_DevNode failed: %x ", status);
        
        return ERROR_DISK_SCAN_FAILED;
    }

    //强制刷新设备树
    status = CM_Reenumerate_DevNode(devInst, 0);
    if (MP_SUCCESS != status)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "CM_Reenumerate_DevNode when scan device tree failed on windows: %x ", status);
        
        return ERROR_DISK_SCAN_FAILED;
    }
#elif defined(LINUX)
    //hot_add
    //获取agent路径
    mp_string strAgentPath = CPath::GetInstance().GetRootPath();
    strAgentPath = CMpString::BlankComma(strAgentPath);
    //获取唯一ID，用于生成临时文件
    mp_string strUniqueID = CUniqueID::GetInstance().GetString();
    
    mp_string strScanDiskParam = strAgentPath + " " + strUniqueID;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SCANDISK, strScanDiskParam, NULL);
#elif defined(HP_UX_IA)
    //ioscan -fnC disk
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCANFNC, "", NULL);
#elif defined(AIX)
    //cfgmgr -v
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CFGMGR, "", NULL);
#elif defined(SOLARIS)
    //cfgadm -al;devfsadm
    mp_int32 iRet1 = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CFGADM, "", NULL);
    mp_int32 iRet2 = CRootCaller::Exec((mp_int32)ROOT_COMMAND_DEVFSADM, "", NULL);
    if((MP_SUCCESS == iRet1) && (MP_SUCCESS == iRet2))
    {
        iRet = MP_SUCCESS;
    }
    else
    {
        iRet = MP_FAILED;
    }
#else
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupport scan disk.");

    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Scan disk failed in this system, iRet %d.", iRet);
        
        return ERROR_DISK_SCAN_FAILED;
    }

#ifdef LINUX
    //hot_add执行以后到块设备文件和/proc/partitions中的信息生成不是同步的，需要等待
    //一般时间较短，SUSE工程师建议等待30秒较为可靠
    DoSleep(WAIT_AFTER_HOT_ADD);
#endif

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Scan disk end.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 注册Trap
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CHost::RegTrapServer(trap_server& stTrapServer)
{
    LOGGUARD("");
    //参数合法性判断
    //检查ip地址
    if (!CIPCheck::IsIPV4(stTrapServer.strServerIP))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SNMP server IP \"%s\" is invalid", stTrapServer.strServerIP.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    //检查端口
    if (stTrapServer.iPort > MAX_PORT_NUM || stTrapServer.iPort < 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Port number %d is invalid", stTrapServer.iPort);
        return ERROR_COMMON_INVALID_PARAM;
    }

    //检查协议
    if (stTrapServer.iVersion > SNMP_V3 || stTrapServer.iVersion < SNMP_V1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Protocol version %d is invalid", stTrapServer.iVersion);
        return ERROR_COMMON_INVALID_PARAM;
    }

    MP_RETURN(CAlarmDB::InsertTrapServer(stTrapServer), ERROR_HOST_REG_TRAPSERVER_FAILED);
}
/*------------------------------------------------------------ 
Description  : 解除Trap
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CHost::UnRegTrapServer(trap_server& stTrapServer)
{
    LOGGUARD("");
    //参数合法性判断
    //检查ip地址
    if (!CIPCheck::IsIPV4(stTrapServer.strServerIP))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SNMP server IP \"%s\" is invalid", stTrapServer.strServerIP.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    //检查端口
    if (stTrapServer.iPort > MAX_PORT_NUM || stTrapServer.iPort < 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Port number %d is invalid", stTrapServer.iPort);
        return ERROR_COMMON_INVALID_PARAM;
    }

    //检查协议
    if (stTrapServer.iVersion > SNMP_V3 || stTrapServer.iVersion < SNMP_V1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Protocol version %d is invalid", stTrapServer.iVersion);
        return ERROR_COMMON_INVALID_PARAM;
    }

    MP_RETURN(CAlarmDB::DeleteTrapServer(stTrapServer), ERROR_HOST_UNREG_TRAPSERVER_FAILED);
}
/*------------------------------------------------------------ 
Description  : 校验SNMP参数
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CHost::VerifySnmp(snmp_v3_param& stParam)
{
    LOGGUARD("");
    //从配置文件中读取相关信息
    snmp_v3_param stLocalParam;
    CAlarmConfig::GetSnmpV3Param(stLocalParam);
    if ((stParam.strAuthPassword != stLocalParam.strAuthPassword)||
        (stParam.strPrivPassword != stLocalParam.strPrivPassword)||
        (stParam.strSecurityName != stLocalParam.strSecurityName)||
        (stParam.iAuthProtocol != stLocalParam.iAuthProtocol)||
        (stParam.iPrivProtocol != stLocalParam.iPrivProtocol))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "SNMP V3 paramters does not match.");
        return ERROR_HOST_VERIFY_SNMP_FAILED;
    }
    stLocalParam.strAuthPassword.replace(0, stLocalParam.strAuthPassword.length(), "");
    stLocalParam.strPrivPassword.replace(0, stLocalParam.strPrivPassword.length(), "");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询第三方脚本
Input        : 
Output       : vectFileList -- 三方脚本
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::QueryThirdPartyScripts(vector<mp_string> & vectFileList)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string scriptPath;
    scriptPath = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + AGENT_THIRDPARTY_DIR;
    iRet = CMpFile::GetFolderFile(scriptPath, vectFileList);
    TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_THIRDPARTY_GETFILE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query thridparty script failed, iRet %d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 执行第三方脚本
Input        : fileName -- 脚本名称
                paramValues -- 脚本参数
Output       : vecResult -- 结果
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::ExecThirdPartyScript(mp_string fileName, mp_string paramValues, vector<mp_string>& vecResult)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strInput;

    (mp_void)CMpString::Trim((mp_char *)fileName.c_str());
    (mp_void)CMpString::Trim((mp_char *)paramValues.c_str());

    if ("" == fileName)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Input Parameter file name is null.");
        return ERROR_COMMON_INVALID_PARAM;
    }

#ifndef WIN32
    strInput = fileName.append(":").append(paramValues);
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The full script is \"%s\".", strInput.c_str());
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_THIRDPARTY, strInput, &vecResult);
#else
    mp_int32 iRettmp = MP_SUCCESS;
    CErrorCodeMap errorCode;
    //CSystemExec::ExecScript只获取到bin目录下，需组装至thridparty目录
    strInput = mp_string(AGENT_THIRDPARTY_DIR) + PATH_SEPARATOR + fileName;

    //调用时，默认参数设为FALSE，不验证脚本签名
    iRet = CSystemExec::ExecScript(strInput, paramValues, &vecResult, MP_FALSE);
    if (MP_SUCCESS != iRet)
    {
        iRettmp = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iRettmp);
        iRet = iRettmp;
    }
#endif
    TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_THIRDPARTY_EXEC_FAILED);
    if (ERROR_COMMON_SCRIPT_FILE_NOT_EXIST == iRet)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "%s is not exist.", fileName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    if (MP_SUCCESS != iRet)
    {
        //对脚本不存在错误码做特殊处理
        iRet == INTER_ERROR_SRCIPT_FILE_NOT_EXIST ? ERROR_COMMON_SCRIPT_FILE_NOT_EXIST : iRet;
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Rootcaller thirdparty script exec failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Exec thirdparty script succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : CollectLog
Input        : strLogName -- 收集的日志名称
                
Output       : 无
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::CollectLog()
{
    mp_int32 iRet = m_logCollector.CollectLog();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "CollectLog failed, iRet = %d", iRet);
    }
    return iRet;
}

#ifdef WIN32
//windows 2003平台上对应磁盘没有offline选项
/*------------------------------------------------------------ 
Description  : 磁盘上线
Input        : strDiskNum --磁盘编号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::DeviceOnline(mp_string& strDiskNum)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd;

    //校验脚本签名
    iRet = CheckScriptSign(SCRIPT_ONLINE_WIN);
    if (iRet != MP_SUCCESS)
    {
        return iRet;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin device online, disk num %s.", strDiskNum.c_str());
    strCmd = CPath::GetInstance().GetBinFilePath(SCRIPT_ONLINE_WIN);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd += " ";
    strCmd += strDiskNum;
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_ONLINE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec online cmd failed, cmd %s.", strCmd.c_str());
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device online succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询磁盘分区
Input        : 
Output       : partisioninfos -- 磁盘分区信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CHost::GetPartisions(vector<partitisions_info_t>& partisioninfos)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get disk Partisions on windows.");
    vector < sub_area_Info_t > rvecSubareaInfo;
    vector < sub_area_Info_t >::iterator iter;

    iRet = CDisk::GetSubareaInfoList(rvecSubareaInfo);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_GET_PARTITION_INFO_FAILED);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get disk subares on windows failed.");

        return iRet;
    }

    for (iter = rvecSubareaInfo.begin(); iter != rvecSubareaInfo.end(); ++iter)
    {
        partitisions_info_t partisioninfo;

        mp_char lbaTmp[FILESYS_NAME_LEN] = {0};
        mp_char numTmp[FILESYS_NAME_LEN] = {0};
        partisioninfo.lCapacity = iter->ullTotalCapacity;
        partisioninfo.strVolName = iter->acVolName;
        partisioninfo.strDiskNumber = iter->iDiskNum;
        partisioninfo.strPartitionName = iter->acDeviceName;
        I64ITOA(iter->llOffset, lbaTmp, FILESYS_NAME_LEN, DECIMAL);
        ITOA(iter->iDiskNum, numTmp, FILESYS_NAME_LEN, DECIMAL);
        partisioninfo.strLba = lbaTmp;
        partisioninfo.strDiskNumber = numTmp;

        partisioninfos.push_back(partisioninfo);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Partitisons info: capacity %lld, volname %s, partision name %s, lba %s, disk num %d.",
            partisioninfo.lCapacity, partisioninfo.strVolName.c_str(), partisioninfo.strPartitionName.c_str(),
            partisioninfo.strLba.c_str(), partisioninfo.strDiskNumber.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End Get disk Partisions on windows.");

    return iRet;
}

#endif

/*---------------------------------------------------------------------------
Function Name: GetHBAInfo
Description  : 获取win下HB卡上的WWN号
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#ifdef WIN32
CHBA::CHBA()
{
    m_hbadll = NULL;
}

CHBA::~CHBA()
{
    FreeHBAdllModule();
}

mp_bool CHBA::LoadHBAdllModule()
{
    if (m_hbadll != NULL)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "%s", "hbaapidll handle is exists.");
        return MP_TRUE;
    }

    mp_char strSystemPath[MAX_PATH_LEN] = { 0 };
    mp_int32 lenPath = GetSystemDirectory(strSystemPath, MAX_PATH_LEN);
    if (lenPath == 0)
    {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get system directory failed, errno [%d]: %s.", iErr, 
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FALSE;
    }

    mp_string ntdllPath = mp_string(strSystemPath) + "\\hbaapi.dll";
    m_hbadll = LoadLibrary(TEXT(ntdllPath.c_str()));
    if ( NULL == m_hbadll )
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "LoadHbaApidllModule failed [%ld]", GetLastError());
        return MP_FALSE;
    }


    Hbafun_GetNumber = (Hba_GetNumberOfAdapters)GetProcAddress( m_hbadll, "HBA_GetNumberOfAdapters");
    if (NULL == Hbafun_GetNumber)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Hbafun_GetNumber null, errorcode(%d).",  GetLastError());
    }

    Hbafun_GetAdapterName = (Hba_GetAdapterName)GetProcAddress( m_hbadll, "HBA_GetAdapterName");
    if (NULL == Hbafun_GetAdapterName)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Hbafun_GetAdapterName null, errorcode(%d).",  GetLastError());
    }

    Hbafun_OpenAdapter = (Hba_OpenAdapter)GetProcAddress( m_hbadll, "HBA_OpenAdapter");
    if (NULL == Hbafun_OpenAdapter)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Hbafun_OpenAdapter null, errorcode(%d).",  GetLastError());
    }

    Hbafun_CloseAdapter = (Hba_CloseAdapter)GetProcAddress( m_hbadll, "HBA_CloseAdapter");
    if (NULL == Hbafun_CloseAdapter)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Hbafun_CloseAdapter null, errorcode(%d).",  GetLastError());
    }

    Hbafun_GetAdapterAttributes = (Hba_GetAdapterAttributes)GetProcAddress( m_hbadll, "HBA_GetAdapterAttributes");
    if (NULL == Hbafun_GetAdapterAttributes)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Hbafun_GetAdapterAttributes null, errorcode(%d).",  GetLastError());
    }

    Hbafun_GetAdapterPortAttributes = (Hba_GetAdapterPortAttributes)GetProcAddress( m_hbadll, "HBA_GetAdapterPortAttributes");
    if (NULL == Hbafun_GetAdapterPortAttributes)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Hbafun_GetAdapterPortAttributes null, errorcode(%d).",  GetLastError());
    }

    mp_bool bCheck = (NULL == Hbafun_GetAdapterName)
                || (NULL == Hbafun_OpenAdapter)
                || (NULL == Hbafun_CloseAdapter)
                || (NULL == Hbafun_GetAdapterAttributes)
                || (NULL == Hbafun_GetAdapterPortAttributes)
                || (NULL == Hbafun_GetNumber);
    if (bCheck)
    {
        FreeHBAdllModule();
        return MP_FALSE;
    }

    return MP_TRUE;
}

mp_void CHBA::FreeHBAdllModule()
{
    if (m_hbadll)
    {
        FreeLibrary(m_hbadll);
        m_hbadll = NULL;
    }
}

mp_int32 CHBA::GetHBAInfo(vector<mp_string>& pvecResult)
{
    VARIANT vtMfgDomain; 
    VARIANT vtModel; 
    VARIANT vtPortWWN; 
    VariantInit(&vtMfgDomain);
    VariantInit(&vtModel);
    VariantInit(&vtPortWWN);
    mp_int32 startSize = pvecResult.size();

    if (!LoadHBAdllModule())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "LoadHbaApidllModule failed.");
        return MP_FAILED;
    }

    mp_int32 numberOfAdapters = Hbafun_GetNumber();
    mp_int32 hbaCount;
    mp_string stradaptername;

    for (hbaCount = 0; hbaCount < numberOfAdapters; hbaCount++) 
    {
        if (Hbafun_GetAdapterName(hbaCount, (char*)stradaptername.c_str()) != 0) 
        {
            continue;
        }
        HANDLE hba_handle = Hbafun_OpenAdapter((char*)stradaptername.c_str());
        if (hba_handle == 0) 
        {
            continue;
        }
        
        HBA_ADAPTERATTRIBUTES strHbaAdapterAttributes;
        if (Hbafun_GetAdapterAttributes(hba_handle, &strHbaAdapterAttributes) != 0)
        {
            Hbafun_CloseAdapter(hba_handle);
            continue;
        }

        mp_int32 hbaPort;
        mp_string PortWWN;
        mp_char buf[32];
        mp_bool bIsUnit = MP_FALSE;

        for (hbaPort = 0; hbaPort < strHbaAdapterAttributes.NumberOfPorts; hbaPort++)
        {
            HBA_PORTATTRIBUTES hbaPortAttrs;
            if((Hbafun_GetAdapterPortAttributes(hba_handle, hbaPort, &hbaPortAttrs)) != 0) 
            {
                continue;
            }
            for( mp_int32 i = 0; i < 8; i++ )
            {
                bIsUnit = hbaPortAttrs.PortWWN.wwn[i] >= 0x00 && hbaPortAttrs.PortWWN.wwn[i] <= 0x0F;
                if (bIsUnit)
                {
                    PortWWN += "0";
                    ITOA(hbaPortAttrs.PortWWN.wwn[i], buf, sizeof(buf), 16);
                    PortWWN += buf;
                }
                else
                {
                    ITOA(hbaPortAttrs.PortWWN.wwn[i], buf, sizeof(buf), 16);
                    PortWWN += buf;
                }
            }
            pvecResult.push_back(PortWWN.c_str());
        }

        Hbafun_CloseAdapter(hba_handle);
    }

    FreeHBAdllModule();

    if (pvecResult.size() <= startSize)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "HbaAdapter is not exists.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
#endif


/*---------------------------------------------------------------------------
Function Name: CollectLog
Description  : 日志收集处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CLogCollector::CollectLog()
{
    LOGGUARD("");
    //判断状态
    if (GetStatus() != LOG_INIT)
    {
        COMMLOG(OS_LOG_INFO, OS_LOG_INFO, "Logcollector is processing, status is %d.", m_status);
        //日志收集失败，返回特性错误码
        return ERROR_HOST_LOG_IS_BEENING_COLLECTED;
    }

    //判断日志个数是否超过10个，如果超过，删除时间最早的
    vector<mp_string> vecFileList;
    mp_string strTmpFileFolder = CPath::GetInstance().GetTmpPath();
    CHECK_FAIL_EX(CMpFile::GetFolderFile(strTmpFileFolder ,vecFileList));
    mp_string strOldestFile = "z";
    for (vector<mp_string>::iterator it = vecFileList.begin(); it != vecFileList.end();)
    {
        if (it->find(AGENT_LOG_ZIP_NAME) == mp_string::npos)
        {
            it = vecFileList.erase(it);
        }
        else
        {
            if (strcmp(strOldestFile.c_str(), it->c_str()) > 0)
            {
                strOldestFile = *it;
            }
            it++;
        }
    }
    if (vecFileList.size() >= MAX_ZIP_LOG_NUM)
    {
        //删除时间最早的那个文件
        if (CMpFile::DelFile(CPath::GetInstance().GetTmpFilePath(strOldestFile).c_str()) != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete file \"%s\" failed.", strOldestFile.c_str());
        }
    }

    SetStatus(LOG_COLLECTING);
    //根据时间戳生成日志名称
    mp_time time;
    CMpTime::Now(&time);
    mp_string strNowTime = CMpTime::GetTimeString(&time);
    mp_string strTime;
    for (mp_uint32 i = 0; i < strNowTime.length(); i++)
    {
        if (strNowTime[i] >= '0' && strNowTime[i] <= '9')
        {
            strTime.push_back(strNowTime[i]);
        }
    }
    mp_string strLogName = mp_string(AGENT_LOG_ZIP_NAME) + strTime + ZIP_SUFFIX;
    //创建独立处理线程
    SetLogName(strLogName);
    //创建处理线程
    thread_id_t handleThread;    //线程id
    (mp_void)memset_s(&handleThread, sizeof(handleThread), 0, sizeof(handleThread));
    return CMpThread::Create(&handleThread, LogCollectThread, this);
}

/*---------------------------------------------------------------------------
Function Name: LogCollectThread
Description  : 日志收集线程处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#ifdef WIN32
DWORD WINAPI CLogCollector::LogCollectThread(LPVOID param)
#else
mp_void* CLogCollector::LogCollectThread(mp_void* param)
#endif
{
    LOGGUARD("");
    CLogCollector* pLogCollector = (CLogCollector*)param;
    mp_int32 iRet = PackageLog(pLogCollector->GetLogName());
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PackageLog failed, iRet = %d.", iRet);
    }
    pLogCollector->SetStatus(LOG_INIT);

#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}


