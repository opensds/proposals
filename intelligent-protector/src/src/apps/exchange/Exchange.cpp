/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifdef WIN32
#include "apps/exchange/Exchange.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/SystemExec.h"
#include "array/Array.h"

CExchange::CExchange()
{
    m_pVssRequester = NULL;
}

CExchange::~CExchange()
{
}

/*------------------------------------------------------------ 
Description  : 查询邮箱数据库信息
Input        : 
Output       : vecExchangeDbInfo--查询的邮箱数据库信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::GetInfo(vector<exchange_db_info_t>& vecExchangeDbInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRettmp = MP_SUCCESS;
    mp_string strScriptName = EXCHANGE_OPER_SCRIPT_NAME;
    mp_string strScriptParam = mp_string(PARAM_OPER_TYPE) + OPER_TYPE_QUERY;
    vector<mp_string> vecResult;

    CErrorCodeMap errorCode;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query exchange app info.");
    
    iRet = CSystemExec::ExecScript(strScriptName, strScriptParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        iRettmp = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,  "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iRettmp);

        iRet = iRettmp;
        
        return iRet;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result info is empty, script exchange.bat.");
        
        return iRet;
    }

    iRet = AnalyseResultInfo(vecResult, vecExchangeDbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse result string info failed.");

        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query exchange app info succ.");
    
    return iRet;
}

/*------------------------------------------------------------ 
Description  :保存查询到的所有邮箱数据库应用信息
Input        : vecResult-- 所有数据库应用信息
Output       : vecExchangeDbInfo-- 数据库应用信息结构体
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::AnalyseResultInfo(vector<mp_string>& vecResult, vector<exchange_db_info_t>& vecExchangeDbInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iVersion = 0;
    mp_string strTmp;
    exchange_db_info_t stExchangeInfo;
    vector<mp_string>::iterator iter = vecResult.begin();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin analyse result info.");
    if (iter == vecResult.end())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid result info.");

        return MP_FAILED;
    }

    //get exchange version
    iRet = ConvertVersion(*iter, iVersion);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Convert exchange version failed, str version %s.",  iter->c_str());

        return iRet;
    }

    //get other exhange app info
    iter++;
    while (vecResult.end() != iter)
    {
        strTmp = *iter;
        (mp_void)CMpString::Trim((mp_char*)strTmp.c_str());
        if ("" == strTmp)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Empty line, skip it.");

            iter++;
            continue;
        }

        iRet = AnalyseExchangeAppInfo(strTmp, stExchangeInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse exchange app info failed, info string %s.", strTmp.c_str());

            iter++;
            continue;
        }

        stExchangeInfo.iVersion = iVersion;
        vecExchangeDbInfo.push_back(stExchangeInfo);
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Add exchange app info to list, version %d, storage gruop name %s, "
            "mailbox db name %s, list count %d.", iVersion, stExchangeInfo.strStorageGroup.c_str(),
            stExchangeInfo.strDbName.c_str(), vecExchangeDbInfo.size());

        iter++;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Analyse result info succ.");

    return iRet;
}

/*------------------------------------------------------------ 
Description  :转换数据库版本号
Input        : strVersion-- 数据库的版本号string类型
Output       : iVersion-- 数据库的版本号int类型
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::ConvertVersion(mp_string& strVersion, mp_int32& iVersion)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strTmp = strVersion;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin convert version, str version %s.", strTmp.c_str());

    (mp_void)CMpString::Trim((mp_char*)strTmp.c_str());
    if ("" == strTmp)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid version string.");

        return MP_FAILED;
    }

    iVersion = atoi(strVersion.c_str());
    switch (iVersion)
    {
        case EXCHANGE_2007_MAJOR_VERSION:
            iVersion = EXCHANGE_VER_2007;
            break;
        case EXCHANGE_2010_MAJOR_VERSION:
            iVersion = EXCHANGE_VER_2010;
            break;
        case EXCHANGE_2013_MAJOR_VERSION:
            iVersion = EXCHANGE_VER_2013;
            break;
        default:
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid exchange major version, major version %d.",
                iVersion);
            iRet = MP_FAILED;
            break;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Convert version succ, version %d.", iVersion);

    return iRet;
}

/*------------------------------------------------------------ 
Description  :保存查询到的邮箱数据库应用信息
Input        : strAppInfo-- 数据库应用信息string类型
Output       : stExhangeInfo-- 数据库应用信息结构体
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::AnalyseExchangeAppInfo(mp_string& strAppInfo, exchange_db_info_t& stExhangeInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iMountStatus = EXC_STATUS_UNKNOWN;
    mp_string strTmp = strAppInfo;
    vector<mp_string> elems;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin analyse exchange app info, info string %s.",
        strAppInfo.c_str());

    (mp_void)CMpString::Trim((mp_char*)strTmp.c_str());
    CMpString::StrSplit(elems, strTmp, CHAR_SLASH);
    if (elems.size() != EXCHANGE_INFO_COUNT)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid format of exchange info, count %d.", elems.size());

        return MP_FAILED;
    }

    stExhangeInfo.strStorageGroup = elems[POS_STORAGE_GROUP_NAME];
    stExhangeInfo.strDbName = elems[POS_MAILBOX_NAME];
    ConvertMountStatus(elems[POS_MOUNT_STATUS], iMountStatus);
    stExhangeInfo.iMountState = iMountStatus;
    stExhangeInfo.strEdbPath = elems[POS_EDB_FILE_PATH];
    stExhangeInfo.strSystemPath = elems[POS_SYS_DIR_PATH];
    stExhangeInfo.strLogPath = elems[POS_LOG_DIR_PATH];
    stExhangeInfo.iIsCommon = atoi(elems[POS_PUBLIC_FOLDER_FLAG].c_str());

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Analyse exchange app info succ, storage group name %s, mailbox name %s,"
        "mounted status %d, edb file path %s, system dir path %s, log dir path %s, exist public db %d.", stExhangeInfo.strStorageGroup.c_str(),
        stExhangeInfo.strDbName.c_str(), stExhangeInfo.iMountState, stExhangeInfo.strEdbPath.c_str(),
        stExhangeInfo.strSystemPath.c_str(), stExhangeInfo.strLogPath.c_str(),
        stExhangeInfo.iIsCommon);

    return iRet;
}

/*------------------------------------------------------------ 
Description  :转换数据库状态类型
Input        : strStatus-- 数据库的状态string类型
Output       : iStatus-- 数据库的状态int类型
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CExchange::ConvertMountStatus(mp_string& strStatus, mp_int32& iStatus)
{
    mp_string strTmp = strStatus;
    (mp_void)CMpString::Trim((mp_char*)strTmp.c_str());

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin convert status, str status %s.",  strTmp.c_str());

    if (EXCHANGE_MOUTNED_STATUS == strTmp)
    {
        iStatus = EXC_STATUS_MOUNTED;
    }
    else if (EXCHANGE_UNMOUNTED_STATUS == strTmp)
    {
        iStatus = EXC_STATUS_DISMOUNTED;
    }
    else
    {
        iStatus = EXC_STATUS_UNKNOWN;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Convert status succ, status %d.", iStatus);
}

/*------------------------------------------------------------ 
Description  :查询数据库lun信息
Input        : exchangeInputInfo-- 邮箱数据库信息
Output       : vecLunInfos-- 数据库所在的LUN的信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::GetLunInfo(ex_querlun_input_info_t exchangeInputInfo, vector<exchange_lun_info_t>& vecLunInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<exchange_db_info_t> vecExchangeDbInfo;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query exchange lun infos.");

    iRet = GetInfo(vecExchangeDbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query app infos failed, iRet %d.", iRet);

        return iRet;
    }

    iRet = AnalyseLunInfos(exchangeInputInfo, vecExchangeDbInfo, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse exchange lun infos failed, iRet %d.", iRet);

        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End query exchange lun infos.");

    return iRet;
}

/*------------------------------------------------------------ 
Description  :分析数据库的Lun信息
Input        : exchangeInputInfo-- 邮箱数据库信息
                 vecExchangeDbInfo-- 数据库应用信息
Output       : vecLunInfos-- 数据库所在的LUN的信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::AnalyseLunInfos(ex_querlun_input_info_t exchangeInputInfo,
    vector<exchange_db_info_t> vecExchangeDbInfo, vector<exchange_lun_info_t>& vecLunInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    list<mp_string> lstVolumePaths;
    mp_int32 iSpecifiedVersion = atoi(exchangeInputInfo.strVersion.c_str());
    mp_string strStorageGroupName = exchangeInputInfo.strStorageGroup;
    mp_string strMailBoxDBName = exchangeInputInfo.strDbNames;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin analyse exchange lun infos, strMailBoxDBName %s.",
        strMailBoxDBName.c_str());

    if (EXCHANGE_VER_2007 == iSpecifiedVersion)
    {
        iRet = Get2007VolumePathList(iSpecifiedVersion, strStorageGroupName, vecExchangeDbInfo, lstVolumePaths);
    }
    else
    {
        iRet = Get201xVolumePathList(iSpecifiedVersion, strMailBoxDBName, vecExchangeDbInfo, lstVolumePaths);
    }

    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get volume path list failed, version %d, storage group %s, mailbox db %s.",
            iSpecifiedVersion, strStorageGroupName.c_str(), strMailBoxDBName.c_str());

        return iRet;
    }

    iRet = GetExchangeLunInfos(lstVolumePaths, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get exchange lun infos failed.");

        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Analyse exchange lun infos succ.");

    return iRet;
}

/*------------------------------------------------------------ 
Description  :查询exchange207存储组所在的卷路径
Input        : iSpecifiedVersion-- 数据库版本号1表示2007
                 strStorageGroupName--存储组名称
                 vecExchangeDbInfo--数据库应用相关信息
Output       : lstVolumePaths-- 保存卷路径的链表
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::Get2007VolumePathList(mp_int32 iSpecifiedVersion, mp_string& strStorageGroupName,
    vector<exchange_db_info_t>& vecExchangeDbInfo, list<mp_string>& lstVolumePaths)
{
    mp_string strLogDirVolumePath;
    mp_string strSysDirVolumePath;
    mp_string strEdbFileVolumePath;
    vector<exchange_db_info_t>::iterator iter;
    char drive[_MAX_DRIVE] = {0};
    char dir[_MAX_DIR] = {0};
    char fname[_MAX_FNAME] = {0};
    char ext[_MAX_EXT] = {0};

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get volume path for exchange 2007, version %d,storage group name %s.",
        iSpecifiedVersion, strStorageGroupName.c_str());

    for (iter = vecExchangeDbInfo.begin(); iter != vecExchangeDbInfo.end(); iter++)
    {
        if (iter->iVersion != iSpecifiedVersion)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The exchange version is not match, version %d, expected version %d.",
                iter->iVersion, iSpecifiedVersion);

            return MP_FAILED;
        }


        if (iter->strStorageGroup != strStorageGroupName)
        {
             COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The storage group name is not match, skip it. storage group %s, "
               "expected storage group %s", iter->strStorageGroup.c_str(), strStorageGroupName.c_str());

             continue;
        }

        _splitpath_s(iter->strLogPath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive %s from log dir path %s.", drive, iter->strLogPath.c_str());
        strLogDirVolumePath = drive;
        AddVolumePathToList(lstVolumePaths, strLogDirVolumePath);

        _splitpath_s(iter->strSystemPath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive %s from system dir path %s.", drive, iter->strSystemPath.c_str());
        strSysDirVolumePath = drive;
        AddVolumePathToList(lstVolumePaths, strSysDirVolumePath);

        _splitpath_s(iter->strEdbPath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive %s from edb file path %s.", drive, iter->strEdbPath.c_str());
        strEdbFileVolumePath = drive;
        AddVolumePathToList(lstVolumePaths, strEdbFileVolumePath);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get volume path for exchange 2007 succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :查询exchange2010和2013数据库所在的卷路径
Input        : iSpecifiedVersion-- 数据库版本号2表示2010,3表示2013
                 strMailboxDbName--邮箱数据库名称
                 vecExchangeDbInfo--数据库应用相关信息
Output       : lstVolumePaths-- 保存卷路径的链表
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::Get201xVolumePathList(mp_int32 iSpecifiedVersion,  mp_string& strMailboxDbName,
    vector<exchange_db_info_t>& vecExchangeDbInfo, list<mp_string>& lstVolumePaths)
{
    mp_string strLogDirVolumePath;
    mp_string strEdbFileVolumePath;
    vector<exchange_db_info_t>::iterator iter;
    char drive[_MAX_DRIVE] = {0};
    char dir[_MAX_DIR] = {0};
    char fname[_MAX_FNAME] = {0};
    char ext[_MAX_EXT] = {0};

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get volume path for exchange 2010 or 2013, version %d, mailbox db name %s.",
        iSpecifiedVersion, strMailboxDbName.c_str());

    for (iter = vecExchangeDbInfo.begin(); iter != vecExchangeDbInfo.end(); iter++)
    {
        if (iter->iVersion != iSpecifiedVersion)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The exchange version is not match, version %d, expected version %d.",
                iter->iVersion, iSpecifiedVersion);

            return MP_FAILED;
        }

        if (iter->strDbName != strMailboxDbName)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The mailbox db name is not match, skip it. mailbox db %s, expected mailbox db %s",
                iter->strDbName.c_str(), strMailboxDbName.c_str());

            continue;
        }

        _splitpath_s(iter->strLogPath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive %s from log dir path %s.", drive, iter->strLogPath.c_str());
        strLogDirVolumePath = drive;
        AddVolumePathToList(lstVolumePaths, strLogDirVolumePath);

        _splitpath_s(iter->strEdbPath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive %s from edb file path %s.", drive, iter->strEdbPath.c_str());
        strEdbFileVolumePath = drive;
        AddVolumePathToList(lstVolumePaths, strEdbFileVolumePath);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get volume path for exchange 2010 or 2013 succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :将卷路径添加到链表中(去掉重复路径)
Input        : strVolumePath-- 卷路径
Output       : lstVolumePaths-- 保存卷路径的链表
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CExchange::AddVolumePathToList(list<mp_string>& lstVolumePaths, mp_string& strVolumePath)
{
    list<mp_string>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin add volume path to path list, volume path %s.", strVolumePath.c_str());

    for (iter = lstVolumePaths.begin(); iter != lstVolumePaths.end(); iter++)
    {
        if (*iter == strVolumePath)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Volume path is exist in path list, no nedd to add, volume path %s.",
                strVolumePath.c_str());

            return;
        }
    }

    lstVolumePaths.push_back(strVolumePath);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Add volume path to path list, volume path %s.", strVolumePath.c_str());
}

/*------------------------------------------------------------ 
Description  :查询数据库所在的Lun信息(所有)
Input        : strVolumePath-- 卷路径
Output       : stExchangeLunInfo-- 数据库所在的Lun信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::GetExchangeLunInfos(list<mp_string>& lstVolumePath, vector<exchange_lun_info_t>& lstExchangeLunInfos)
{
    mp_int32 iRet =MP_SUCCESS;
    list<mp_string>::iterator iter;
    exchange_lun_info_t stExchangeLunInfo;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get exchange lun infos.");

    for (iter = lstVolumePath.begin(); iter != lstVolumePath.end(); iter++)
    {
        iRet = GetExhcangeLunInfo(*iter, stExchangeLunInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get exchange lun info failed, path %s.", iter->c_str());

            return iRet;
        }

        lstExchangeLunInfos.push_back(stExchangeLunInfo);
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get exchange lun info succ and add it to list, volume path %s.",
            iter->c_str());
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get exchange lun infos succ.");

    return iRet;
}

/*------------------------------------------------------------ 
Description  :查询数据库所在的Lun信息(单个)
Input        : strVolumePath-- 卷路径
Output       : stExchangeLunInfo-- 数据库所在的Lun信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::GetExhcangeLunInfo(mp_string& strVolumePath, exchange_lun_info_t& stExchangeLunInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strTmp;
    mp_string strTmpLunId;
    vector <exchange_strorage_info_t> exstrorageinfos;
    vector<sub_area_Info_t> lstSubareaInfo;
    vector<sub_area_Info_t>::iterator iterDisk;
    vector<exchange_strorage_info_t>::iterator iterLun;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get exchange lun info, path %s.", strVolumePath.c_str());

    //去掉盘符后的冒号(C: --> C)
    strTmp = strVolumePath;
    mp_string::size_type iIndex = strTmp.find(STR_COLON, 0);
    if (iIndex == mp_string::npos)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Volume path %s is not contain :.", strTmp.c_str());
        
        return MP_FAILED;
    }
    strTmp = strTmp.substr(0, iIndex);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Remove colon in volume path, path %s.", strTmp.c_str());

    //获取磁盘信息，包括阵列SN,LUN ID,LUN WWN,磁盘编号
    iRet = GetDiskInfoList(exstrorageinfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get disk infomation on windows failed.");

        return iRet;
    }

    //获取磁盘序号和盘符的对应信息
    iRet = CDisk::GetSubareaInfoList(lstSubareaInfo);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get disk subares on windows failed.");

        return iRet;
    }

    for (iterDisk = lstSubareaInfo.begin(); iterDisk != lstSubareaInfo.end(); iterDisk++)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get volume info, volume path %s, hard-disk drive lettter %s.",
            iterDisk->acVolName, iterDisk->acDriveLetter);
        if (0 == _stricmp(strTmp.c_str(),iterDisk->acDriveLetter))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get matched drive letter %s, disk num %d.", iterDisk->acDriveLetter,
                iterDisk->iDiskNum);
            for (iterLun = exstrorageinfos.begin(); iterLun != exstrorageinfos.end(); iterLun++)
            {
                COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get lun info in loop, disk num %d, lun wwn %s, array sn %s.",
                    iterLun->iDiskNumber, iterLun->strWwn.c_str(), iterLun->strArraySn.c_str());

                if (iterDisk->iDiskNum == iterLun->iDiskNumber)
                {
                    mp_char lbaTmp[FILESYS_NAME_LEN] = {0};
                    stExchangeLunInfo.strLunId = iterLun->strLunId;
                    strTmpLunId = stExchangeLunInfo.strLunId;
                    CMpString::FormatLUNID(strTmpLunId, stExchangeLunInfo.strLunId);

                    stExchangeLunInfo.strWwn = iterLun->strWwn;
                    stExchangeLunInfo.strArraySn = iterLun->strArraySn;
                    stExchangeLunInfo.strDevName = iterDisk->acDriveLetter;
                    stExchangeLunInfo.strVolName = iterDisk->acVolName;

                    I64ITOA(iterDisk->llOffset, lbaTmp, FILESYS_NAME_LEN, DECIMAL);
                    stExchangeLunInfo.strLba = lbaTmp;

                    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get expected exchange lun info, volume path %s, LUNID %s, "
                        "LUN WWN %s, array SN %s, volume name %s, offset %s.", strTmp.c_str(),
                        stExchangeLunInfo.strLunId.c_str(), stExchangeLunInfo.strWwn.c_str(),
                        stExchangeLunInfo.strArraySn.c_str(), stExchangeLunInfo.strVolName.c_str(),
                        stExchangeLunInfo.strLba.c_str());

                    break;
                }
            }

            if (iterLun == exstrorageinfos.end())
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Drive %s(%s) is not the huawei lun.",
                    iterDisk->acDriveLetter, iterDisk->acVolName);

                return ERROR_COMMON_NOT_HUAWEI_LUN;
            }

            break;
        }
    }

    //如果在系统中没有找到相应的盘符
    if (iterDisk == lstSubareaInfo.end())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get exchange lun info by volume path failed, volume path %s",
            strVolumePath.c_str());

        return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get exchange lun info succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 获取磁盘的LUN信息
Input        : 
Output       : exstrorageinfos--磁盘号，LunID, Sn, Wwn
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::GetDiskInfoList(vector<exchange_strorage_info_t>& exstrorageinfos)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32  iDiskNum;
    mp_string strVendor;
    mp_string strProduct;
    mp_string strSN;
    mp_string strLunWWN;
    mp_string  strLunID;
    vector < mp_string > vecDiskName;
    vector<mp_string>::iterator iter;
    exchange_strorage_info_t exstrorageinfo;

    iRet = CDisk::GetAllDiskName(vecDiskName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get bolck devs failed, iRet %d.", iRet);
        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }

    if (vecDiskName.empty())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The disk list is empty.");
        return MP_SUCCESS;
    }

    for (iter = vecDiskName.begin(); iter != vecDiskName.end(); ++iter)
    {
        //厂商和型号
        iRet = CArray::GetArrayVendorAndProduct(*iter, strVendor, strProduct);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) is not huawei LUN, Vendor:%s.",
                (*iter).c_str(), strVendor.c_str());
            
            continue;
        }

        (mp_void)CMpString::Trim((mp_char*)strVendor.c_str());
        (mp_void)CMpString::Trim((mp_char*)strProduct.c_str());

        //排除非华为产品
        if (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY))
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) is not huawei LUN, Vendor:%s.",
                (*iter).c_str(), strVendor.c_str());
            
            continue;
        }

        iRet = CArray::GetArraySN(*iter, strSN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) get array SN failed, iRet %d.",  (*iter).c_str(), iRet);
            
            continue;
        }

        iRet = CArray::GetLunInfo(*iter, strLunWWN, strLunID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) get WWN and LunID failed, iRet %d.",  (*iter).c_str(), iRet);
            
            continue;
        }

        iRet = CDisk::GetDiskNum(*iter, iDiskNum);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) get disk number failed, iRet %d.",  (*iter).c_str(), iRet);
            
            continue;
        }

        exstrorageinfo.iDiskNumber = iDiskNum;
        exstrorageinfo.strArraySn = strSN;
        exstrorageinfo.strLunId = strLunID;
        exstrorageinfo.strWwn = strLunWWN;

        exstrorageinfos.push_back(exstrorageinfo);


    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get disk info succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 停止邮箱数据库
Input        : exstopparam -- 停止数据库时所需要的信息
               strStopType -- 停止、清理数据库标识，"1"表示清理"3"表示去挂载
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::Stop(exchange_param_t& exstopparam, mp_string strStopType)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strScriptName = EXCHANGE_OPER_SCRIPT_NAME;
    mp_string strScrParam;
    mp_string strVersion;
    vector<mp_string> pvecResult;
    mp_char cVersion[EXCHANGE_STR_LEN] = {0};

    ITOA(exstopparam.iVersion, cVersion, EXCHANGE_STR_LEN, DECIMAL);
    strVersion = cVersion;

    //组装脚本输入参数,不已":"分割，防止路径带冒号
    strScrParam = mp_string(PARAM_OPER_TYPE) + strStopType + STR_SEMICOLON
        + PARAM_VERSION + strVersion + STR_SEMICOLON
        + PARAM_NEW_STORGRPNAME + exstopparam.strStrGrpName + STR_SEMICOLON
        + PARAM_NEW_MAILBOXDBNAME + exstopparam.strDbName + STR_SEMICOLON
        + PARAM_HOSTNAME + exstopparam.strServName;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin stop exchange mailbox db, script parameter[%s].", strScrParam.c_str());

    iRet = CSystemExec::ExecScript(strScriptName, strScrParam, &pvecResult);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                 "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);

        return iNewRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End stop and clear exchange database %s.", exstopparam.strDbName.c_str());

    return iRet;
}

/*------------------------------------------------------------ 
Description  :启动邮箱数据库
Input        : exstartparam--启动数据库所需的信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::Start(exchange_param_t& exstartparam)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strScriptName = EXCHANGE_OPER_SCRIPT_NAME;
    mp_string strScrParam;
    mp_string strVersion, strStarType;
    mp_char cVersion[EXCHANGE_STR_LEN] = {0};
    mp_char cStarType[EXCHANGE_STR_LEN] = {0};

    ITOA(exstartparam.iVersion, cVersion, EXCHANGE_STR_LEN,  DECIMAL);
    ITOA(exstartparam.iStarType, cStarType, EXCHANGE_STR_LEN, DECIMAL);
    strVersion = cVersion;
    strStarType = cStarType;

     //组装脚本输入参数，不已":"分割，防止路径带冒号
     strScrParam = mp_string(PARAM_OPER_TYPE) + OPER_TYPE_RECOVERY + STR_SEMICOLON
        + PARAM_VERSION + strVersion + STR_SEMICOLON
        + PARAM_RECOVERY_TYPE + strStarType + STR_SEMICOLON
        + PARAM_OLD_STORGRPNAME + exstartparam.strStrGrpName+ STR_SEMICOLON
        + PARAM_OLD_MAILBOXDBNAME + exstartparam.strDbName + STR_SEMICOLON
        + PARAM_NEW_STORGRPNAME + exstartparam.strSlaveStrGrpName + STR_SEMICOLON
        + PARAM_NEW_MAILBOXDBNAME + exstartparam.strSlaveDbName + STR_SEMICOLON
        + PARAM_HOSTNAME + exstartparam.strServName + STR_SEMICOLON
        + PARAM_EDBPATH + exstartparam.strEdbPath + STR_SEMICOLON
        + PARAM_LOGPATH + exstartparam.strLogPath + STR_SEMICOLON
        + PARAM_SYSPATH +exstartparam.strSysPath;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin start exchange mailbox db, script parameter[%s].", strScrParam.c_str());

    iRet = CSystemExec::ExecScript(strScriptName, strScrParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                 "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);

        return iNewRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End start exchange database %s.", exstartparam.strDbName.c_str());

    return iRet;
}

/*------------------------------------------------------------ 
Description  : 停止启动exchange2013 的hostcontroller服务
Input        : OperTyp--启停服务的标识，0表示启动1表示停止
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::OperHostContorllerService(mp_int32 OperTyp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strCmd;
    mp_string stroperType;
    if (OPER_STOP_SERVICE != OperTyp)
    {
        strCmd = "cmd.exe /c powershell \"Start-Service HostControllerService -Confirm:$false\"";
        stroperType="start";
    }
    else
    {
        strCmd = "cmd.exe /c powershell \"Stop-Service HostControllerService -Confirm:$false\"";
        stroperType="stop";
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin %s service[HostControllerService] in Exchange2013,cmd %s.", 
        stroperType.c_str(), strCmd.c_str());

    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "%s service[HostControllerService] in Exchange2013 failed, iRet %d", stroperType.c_str(), iRet);

        return ERROR_COMMON_SYSTEM_CALL_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "%s service[HostControllerService] in Exchange2013 succ.", stroperType.c_str());

    return iRet;
}

/*------------------------------------------------------------ 
Description  : 冻结数据库
Input        : vecFreezeInfos -- 数据库名称和磁盘驱动号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::Freeze(vector<exchange_freeze_info_t> vecFreezeInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<vss_db_oper_info_t> vssDbInfos;
    vss_db_oper_info_t vssDbInfo;
    vector<exchange_freeze_info_t>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin freeze exchage db.");
    if (NULL != m_pVssRequester)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Other freeze opertion is running.");
        
        return ERROR_VSS_OTHER_FREEZE_RUNNING;
    }

    for (iter = vecFreezeInfos.begin(); iter != vecFreezeInfos.end(); iter++)
    {
        vssDbInfo.strDbName = iter->strDBName;
        vssDbInfo.vecDriveLetters = iter->vecDriveLetters;
        vssDbInfos.push_back(vssDbInfo);
    }

    //m_pVssRequester = new VSSRequester();
    NEW_CATCH_RETURN_FAILED(m_pVssRequester, VSSRequester);
    iRet = m_pVssRequester->Freeze(vssDbInfos, vss_freeze_exchange);
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze exchange failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Freeze exchange db succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 解冻数据库
Input        : vecFreezeInfos -- 数据库名称和磁盘驱动号
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::UnFreeze(vector<exchange_unfreeze_info_t> vecUnFreezeInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<vss_db_oper_info_t> vssDbInfos;
    vss_db_oper_info_t vssDbInfo;
    vector<exchange_unfreeze_info_t>::iterator iter;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin unfreeze exchange db.");
    if (NULL == m_pVssRequester)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "No need to thaw.");
        return ERROR_COMMON_APP_THAW_FAILED;
    }

    for (iter = vecUnFreezeInfos.begin(); iter != vecUnFreezeInfos.end(); iter++)
    {
        vssDbInfo.strDbName = iter->strDBName;
        vssDbInfo.vecDriveLetters = iter->vecDriveLetters;
        vssDbInfos.push_back(vssDbInfo);
    }

    iRet = m_pVssRequester->UnFreeze(vssDbInfos);
    if (MP_SUCCESS != iRet)
    {
        if (ERROR_INNER_THAW_NOT_MATCH != iRet)
        {
            delete m_pVssRequester;
            m_pVssRequester = NULL;
        }
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze exchange failed, iRet %d.", iRet);
        return iRet;
    }

    delete m_pVssRequester;
    m_pVssRequester = NULL;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unfreeze exchange db succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询数据库冻结状态
Input        : 
Output       : 
Return       : FREEZE_STAT_UNFREEZE -- 解冻状态 
               FREEZE_STAT_FREEZED -- 冻结状态
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CExchange::GetFreezeStat()
{
    if (NULL == m_pVssRequester)
    {
        return FREEZE_STAT_UNFREEZE;
    }

    return FREEZE_STAT_FREEZED;  
}
#endif

