/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifdef WIN32
#include "apps/sqlserver/SqlServer.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/SystemExec.h"
#include "common/CryptAlg.h"
#include "array/Array.h"
#include "securec.h"

// SQLServer 插件中使用到的脚本;
#define SQLSERVER_SCRIPT_SQLSERVER              "sqlserverinfo.bat"
#define SQLSERVER_SCRIPT_SQLSERVERADAPTIVE      "sqlserverluninfo.bat"
#define SQLSERVER_SCRIPT_SQLSERVERRECOVER       "sqlserverrecover.bat"
#define SQLSERVER_SCRIPT_SQLSERVERSAMPLE        "sqlserversample.bat"


// 返回映射的错误码;
#define RETURN_MAP_ERRCODE(iRet, strOptInfo)                                                \
    CErrorCodeMap errorCode;                                                                \
    mp_int32 iNewRet = errorCode.GetErrorCode(iRet);                                        \
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,                                                 \
        "Exec %s failed, original code: %d, map code: %d", strOptInfo, iRet, iNewRet);      \
    return iNewRet


CSqlServer::CSqlServer()
{
    m_pVssRequester = NULL;
}

CSqlServer::~CSqlServer()
{
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::ReplaceStr
Description   : 替换字符串;
Input         : const mp_string &oldStr, 输入串;
                mp_string &newStr, 输出串;
                const mp_string &old_value, 原始替换值;
                const mp_string &new_value, 替换值;
Output        : mp_string &newStr
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_void CSqlServer::ReplaceStr(const mp_string &oldStr, mp_string &newStr, const mp_string &old_value, const mp_string &new_value)
{
    size_t offset = 0;
    size_t strLen = old_value.size();
    newStr.clear();

    size_t idx = oldStr.find(old_value, offset);
    while (idx != string::npos)
    {
        newStr = newStr + oldStr.substr(offset, idx - offset);
        newStr = newStr + new_value;

        offset = idx + strLen;
        idx = oldStr.find(old_value, offset);
    }

    newStr = newStr + oldStr.substr(offset);
}

/*------------------------------------------------------------ 
Description  : 检查Sql Server是否安装
               检查注册表是否存在 HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SQL Server\Instance Names\SQL
Input        : 
Output       : bIsInstalled  -- MP_TRUE - 已安装；MP_FALSE - 未安装；
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CSqlServer::IsInstalled(mp_bool& bIsInstalled)
{
    mp_int32 iRet = MP_SUCCESS;
    HKEY hKEY;
	mp_long lRet;
	mp_char szErr[256] = {0};

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin check whether sql server installed or not.");
	lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, SQL_SERVER_REG_KEY, NULL, KEY_READ, &hKEY);
	if (lRet != ERROR_SUCCESS)
	{
        GetOSStrErr(lRet, szErr, sizeof(szErr));
        RegCloseKey(hKEY);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open reg key HKEY_LOCAL_MACHINE%s failed, errno[%d]: %s.", 
            SQL_SERVER_REG_KEY, (mp_int32)lRet, szErr);
        
        if (SQL_SERVER_QUERY_REG_ERR_NOT_EXIST != lRet)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open reg key HKEY_LOCAL_MACHINE%s failed, errno[%d]: %s.",
                SQL_SERVER_REG_KEY, (mp_int32)lRet, szErr);
            return MP_FAILED;
        }

        bIsInstalled = MP_FALSE;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Sql server isn't installed.");
        return MP_SUCCESS;
	}

    bIsInstalled = MP_TRUE;
    RegCloseKey(hKEY);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Sql server is already installed.");
    
    return MP_SUCCESS;
}

/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::GetInfo
Description   : 获取数据库信息;
Input         : vector<sqlserver_info_t> &vecdbInstInfo; 返回SQLServer 数据库信息列表;
Output        : vector<sqlserver_info_t> &vecdbInstInfo
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::GetInfo(vector<sqlserver_info_t> &vecdbInstInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strScriptName = SQLSERVER_SCRIPT_SQLSERVER;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query sql server info.");
    iRet = CSystemExec::ExecScript(strScriptName, "", &vecResult);
    if (MP_SUCCESS != iRet)
    {
        RETURN_MAP_ERRCODE(iRet, "get sqlserver info");
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "sqlserver app info is empty");
        // 以前代码中为空返回失败;
        return ERROR_SQLSERVER_DB_LIST_IS_NULL;
    }
    
    iRet = AnalyseDBQueryResult(vecdbInstInfo, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse sql server info result failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query sql server info succ.");
    return iRet;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::AnalyseDBQueryResult
Description   : 分析数据查询结构;
Input         : vector<sqlserver_info_t>& vecDbInfo, 返回信息列表;
                vector<mp_string>& vecResult, 数据库查询信息列表;
Output        : vector<sqlserver_info_t>& vecDbInfo
Return        : mp_int32
Create By     :
               2015/11/08；重写sqlserverinfo.bat，清理原有逻辑，增加返回recovery model。
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::AnalyseDBQueryResult(vector<sqlserver_info_t>& vecDbInfo, const vector<mp_string>& vecResult)
{
    mp_string strVersion;
    mp_char acTmp[1024] = {0};
    vector<mp_string> vecTmp;
    sqlserver_info_t stSqlInfo;
    mp_bool bRet;
	
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin analyse result succ.");
    for (vector<mp_string>::const_iterator iterDBInfo = vecResult.begin(); iterDBInfo != vecResult.end(); ++iterDBInfo)
    {
        vecTmp.clear();

        stSqlInfo.strInstName   = "";
        stSqlInfo.strDBName     = "";
        stSqlInfo.strVersion    = "";
        stSqlInfo.strState      = "";
        stSqlInfo.strIsCluster  = "";
        stSqlInfo.iRecoveryModel = 0;
        
        CHECK_NOT_OK(memset_s(acTmp, sizeof(acTmp), 0, sizeof(acTmp)));
        CHECK_FAIL(SNPRINTF_S(acTmp, sizeof(acTmp), sizeof(acTmp) - 1, "%s", (*iterDBInfo).c_str()));

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get result line, current line %s.", acTmp);
        (void)CMpString::Trim(acTmp);
        if ('\0' == acTmp[0])
        {
            continue;
        }

        //结果文件格式为，instance name;db name;version;online status;is cluster flag;recovery model;
        CMpString::StrSplit(vecTmp, acTmp, ';');
        if (vecTmp.size() != SQL_SERVER_INFO_TOTAL_COUNT)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid result, current line %s.", acTmp);
            continue;
        }

        bRet = MASTER == vecTmp[1] || TEMPDB == vecTmp[1] || MODEL == vecTmp[1] || MSDB == vecTmp[1];
        if (bRet == MP_TRUE)
        {
            continue;	
        }
        
        stSqlInfo.strInstName   = vecTmp[0];
        stSqlInfo.strDBName     = vecTmp[1];
        stSqlInfo.strVersion    = vecTmp[2];
        stSqlInfo.strState      = vecTmp[3];
        stSqlInfo.strIsCluster  = vecTmp[4];
        stSqlInfo.iRecoveryModel = atoi(vecTmp[5].c_str());
        vecDbInfo.push_back(stSqlInfo);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get sql server info, instacne name %s, db name %s, version %s, "
            "online status %s, is cluster flag %s, recovery model %d.", stSqlInfo.strInstName.c_str(), 
            stSqlInfo.strDBName.c_str(), stSqlInfo.strVersion.c_str(), stSqlInfo.strState.c_str(), 
            stSqlInfo.strIsCluster.c_str(), stSqlInfo.iRecoveryModel);
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End analyse result.");
    return MP_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::GetLunInfo
Description   : 获取SQLServer数据库LUN信息;
Input         : sqlserver_info_t& stdbinfo, 请求消息<主要包含实例和数据库名称>;
                vector<sqlserver_lun_info_t>& vecLunInfos, LUN信息集合;
Output        : vector<sqlserver_lun_info_t>& vecLunInfoscv
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::GetLunInfo(sqlserver_info_t& stdbinfo, vector<sqlserver_lun_info_t>& vecLunInfos)
{
    mp_int32 iRet = MP_SUCCESS;

    LOGGUARD("");
    iRet = QueryDBTableSpaceLUNInfo(stdbinfo, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "Query sqlserver table space lun info failed, database[%s]", stdbinfo.strDBName.c_str());

        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
        "Query sqlserver table space lun info succ, database[%s]", stdbinfo.strDBName.c_str());

    return MP_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::QueryDBTableSpaceLUNInfo
Description   : 查询SQLServer数据库LUN信息;
Input         : sqlserver_info_t& stdbinfo, 请求消息<主要包含实例和数据库名称>;
                vector<sqlserver_lun_info_t>& vecLunInfos, LUN信息集合;
Output        : vector<sqlserver_lun_info_t>& vecLunInfos
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::QueryDBTableSpaceLUNInfo(sqlserver_info_t& stdbinfo, vector<sqlserver_lun_info_t>& vecLunInfos)
{
    LOGGUARD("");
    mp_string strParam;
    vector<mp_string> vecPath;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,
        "Begin query sqlserver database[%s] table space", stdbinfo.strDBName.c_str());

    BuildScriptParams(strParam, stdbinfo);
    iRet = GetDBFilePath(vecPath, strParam);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "query database[%s] table space lun failed", stdbinfo.strDBName.c_str());
        return iRet;
    }

    for (vector<mp_string>::iterator iterPath = vecPath.begin();
        iterPath != vecPath.end();
        ++iterPath)
    {
        iRet = GetDBLUNInfoByPath(vecLunInfos, *iterPath);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get db lun info by path failed.");
            return iRet;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End query sqlserver database[%s] table space", stdbinfo.strDBName.c_str());
    return iRet;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::GetDBLUNInfoByPath
Description   : 查询SQLServer数据库LUN信息;
Input         : vector<sqlserver_lun_info_t>& vecLunInfos, LUN信息集合;
                const mp_string &path;路径;<例如:C>
Output        : vector<sqlserver_lun_info_t>& vecLunInfos
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::GetDBLUNInfoByPath(vector<sqlserver_lun_info_t> &vecLunInfos, const mp_string &path)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iFlag = MP_FAILED;
    vector<storage_basic_t> vecDiskInfos;
    vector<sub_area_Info_t> vecSubareaInfos;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get sqlserver database lun info by path.");

    iRet = GetDiskInfoList(vecDiskInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, OS_LOG_ERROR, "Get disk infomation on windows failed.");
        return iRet;
    }

    iRet = CDisk::GetSubareaInfoList(vecSubareaInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_DEBUG, "Get disk subarea failed.");
        return iRet;
    }

    // 展示分区信息;
    for(vector<sub_area_Info_t>::const_iterator iterSubArea = vecSubareaInfos.begin();
        iterSubArea != vecSubareaInfos.end();
        ++iterSubArea)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
            "Get Subarea Info. diskNum:%d, LBA:%ld, Capacity:%ld, drive letter:%s, volname:%s, volLable:%s, fileSystem:%s, deviceName:%s",
            iterSubArea->iDiskNum,      iterSubArea->llOffset,      iterSubArea->ullTotalCapacity,
            iterSubArea->acDriveLetter, iterSubArea->acVolName,     iterSubArea->acVolLabel,
            iterSubArea->acFileSystem,  iterSubArea->acDeviceName);
    }

    // 根据vecDiskInfos, vecSubareaInfos 以及 path 查询lun信息;
    for(vector<sub_area_Info_t>::iterator itSubareaInfo = vecSubareaInfos.begin(); itSubareaInfo != vecSubareaInfos.end(); ++itSubareaInfo)
    {
        if (path == mp_string(itSubareaInfo->acDriveLetter))
        {
            sqlserver_lun_info_t stLUNInfo;
            stLUNInfo.iDeviceType = 0;

            for(vector<storage_basic_t>::iterator itDiskInfo = vecDiskInfos.begin(); itDiskInfo != vecDiskInfos.end();  ++itDiskInfo)
            {
                if (itSubareaInfo->iDiskNum == itDiskInfo->iDiskNum)
                {
                    mp_string strFormatLUNID;
                    mp_char LBA[FILESYS_NAME_LEN] = {0};
                    stLUNInfo.strArraySN    = itDiskInfo->strArraySN;
                    stLUNInfo.strWWN        = itDiskInfo->strWWN;
                    stLUNInfo.strDeviceName = path;
                    stLUNInfo.strVOLName    = itSubareaInfo->acVolName;

                    strFormatLUNID = itDiskInfo->strLunID;
                    CMpString::FormatLUNID(strFormatLUNID, stLUNInfo.strLunID);

                    I64ITOA(itSubareaInfo->llOffset, LBA, FILESYS_NAME_LEN, DECIMAL);
                    stLUNInfo.strLBA = LBA;

                    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
                        "SQLServer lun info, path:%s, LUNID:%s, WWN:%s, SN:%s, volume:%s, offset:%s.",
                        path.c_str(), stLUNInfo.strLunID.c_str(), stLUNInfo.strWWN.c_str(),
                        stLUNInfo.strArraySN.c_str(), stLUNInfo.strVOLName.c_str(),stLUNInfo.strLBA.c_str());

                    vecLunInfos.push_back(stLUNInfo);

                    iFlag = MP_SUCCESS;
                    break;
                }
            }

            break;
        }
    }

    if (MP_SUCCESS != iFlag)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The disk(%s) is not huawei LUN, iRet %d.",
            path.c_str(), ERROR_COMMON_NOT_HUAWEI_LUN);
        
        return ERROR_COMMON_NOT_HUAWEI_LUN;
    }
    
    if (vecLunInfos.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Cann't find any database lun info by label(%s), iRet %d.",
            path.c_str(), ERROR_COMMON_QUERY_APP_LUN_FAILED);
        
        return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End get sqlserver database lun info by path.");
    return MP_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::GetDiskInfoList
Description   : 获取磁盘阵列的基本信息;
Input         : vector<storage_basic_t> &rlstDiskInfoWin, 磁盘整列基本信息;
Output        : vector<storage_basic_t> &rlstDiskInfoWin
Return        : mp_int32
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::GetDiskInfoList(vector<storage_basic_t> &rlstDiskInfoWin)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecDiskName;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get lun infos in windows.");

    // 获取主机上所有的硬盘名称;
    iRet = CDisk::GetAllDiskName(vecDiskName);
    if (0 == vecDiskName.size())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disk name list is empty when get disk name.");
        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }

    // 获取所有的磁盘名称;
    for (vector<mp_string>::iterator iterDiskName = vecDiskName.begin(); iterDiskName != vecDiskName.end();
        ++iterDiskName)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get diskName: %s", iterDiskName->c_str());
    }

    // 获取LUN的ID、WWN、阵列SN以及对应的disk number;
    for (vector<mp_string>::iterator iterDiskName = vecDiskName.begin();
        iterDiskName != vecDiskName.end();
        ++iterDiskName)
    {
        mp_string strVendor;
        mp_string strProduct;
        storage_basic_t storageInfo;
        storageInfo.iDiskNum = 0;

        iRet = CArray::GetArrayVendorAndProduct(*iterDiskName, strVendor, strProduct);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN,
                "Get disk[%s] vendor and product error. vendor:%s product:%s",
                iterDiskName->c_str(), strVendor.c_str(), strProduct.c_str());

            continue;
        }

        (mp_void)CMpString::Trim((mp_char*)strVendor.c_str());
        (mp_void)CMpString::Trim((mp_char*)strProduct.c_str());

        // 目前只支持华为存储产品;
        if (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY))
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Cann't support not Huawei's storage, vendor: %s", strVendor.c_str());
            continue;
        }

        iRet = CArray::GetArraySN(*iterDiskName, storageInfo.strArraySN);
        if (MP_SUCCESS != iRet)
        {
            continue;
        }

        iRet = CArray::GetLunInfo(*iterDiskName, storageInfo.strWWN, storageInfo.strLunID);
        if (MP_SUCCESS != iRet)
        {
            continue;
        }

        iRet = CDisk::GetDiskNum(*iterDiskName, storageInfo.iDiskNum);
        if (MP_SUCCESS != iRet)
        {
            continue;
        }

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
            "insert data. LunID:%s, ArraySN:%s, WWN:%s, diskNum:%d",
            storageInfo.strLunID.c_str(),  storageInfo.strArraySN.c_str(),
            storageInfo.strWWN.c_str(), storageInfo.iDiskNum);

        rlstDiskInfoWin.push_back(storageInfo);
    }

    // 展示基本的磁盘列表;
    for(vector<storage_basic_t>::const_iterator iterStorage = rlstDiskInfoWin.begin();
        iterStorage != rlstDiskInfoWin.end();
        ++iterStorage)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Lun Info. LunID:%s, ArraySN:%s, WWN:%s, diskNum:%d",
            iterStorage->strLunID.c_str(),  iterStorage->strArraySN.c_str(),
            iterStorage->strWWN.c_str(), iterStorage->iDiskNum);
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End get lun infos in windows.");
    return MP_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::BuildScriptParams
Description   : 构造脚本参数;
Input         : mp_string &rstrParam, 构造的查询结果;
                sqlserver_info_t& stdbinfo, 请求消息<主要包含实例和数据库名称>;
Output        : mp_string &rstrParam
Return        : mp_void
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_void CSqlServer::BuildScriptParams(mp_string &rstrParam, const sqlserver_info_t& stdbinfo)
{
    rstrParam.clear();

    rstrParam = mp_string(SCRIPTPARAM_DBNAME) + stdbinfo.strDBName + mp_string(NODE_SEMICOLON)
        + mp_string(SCRIPTPARAM_INSTNAME) + stdbinfo.strInstName  + mp_string(NODE_SEMICOLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbinfo.strUser + mp_string(NODE_SEMICOLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbinfo.strPasswd + mp_string(NODE_SEMICOLON)
        + mp_string(SQLSERVER_SCRIPTPARAM_CHECKTYPE) + stdbinfo.strCheckType + mp_string(NODE_SEMICOLON)
        + mp_string(SQLSERVER_SCRIPTPARAM_TABNAME) + mp_string("must") + mp_string(NODE_SEMICOLON)
        + mp_string(SQLSERVER_SCRIPTPARAM_CLUSTERFLAG) + stdbinfo.strIsCluster;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::GetDBFilePath
Description   : 构造SQLServer数据库LUN信息参数;
Input         : vector<mp_string> &lstPath, 查询到的路径列表;
                const mp_string &strParam, 查询参数;
Output        : vector<mp_string> lstPath
Return        : mp_void
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::GetDBFilePath(vector<mp_string> &lstPath, const mp_string &strParam)
{
    mp_string strParamTmp;
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strScriptName = SQLSERVER_SCRIPT_SQLSERVERADAPTIVE;

    LOGGUARD("");
    ReplaceStr(strParam, strParamTmp, "%", "%%");
    iRet = CSystemExec::ExecScript(strScriptName, strParamTmp, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        RETURN_MAP_ERRCODE(iRet, "get sqlserver lun path");
    }
    
    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Cann't find any disk.");
        return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    }
    
    // 解析到路径列表中;
    for (vector<string>::iterator iter = vecResult.begin();
        iter != vecResult.end();
        ++iter)
    {
        vector<mp_string> vecPathTmp;
        (void)CMpString::StrSplit(vecPathTmp, *iter, ';');
        for (vector<mp_string>::const_iterator iterPath = vecPathTmp.begin();
            iterPath != vecPathTmp.end();
            ++iterPath)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Find sqlserver path:%s", iterPath->c_str());
            lstPath.push_back(*iterPath);
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query sqlserver app info succ.");

    return MP_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::Start
Description   : 构造SQLServer数据库LUN信息参数;
Input         : sqlserver_info_t& stdbInfo, rest接口参数;
Output        : mp_int32
Return        : mp_void
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::Start(sqlserver_info_t& stdbInfo)
{
    mp_string strParam;
    mp_string strParamTmp;
    mp_int32 iRet = MP_SUCCESS;
    mp_string strScriptName = SQLSERVER_SCRIPT_SQLSERVERRECOVER;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin start database[%s]", stdbInfo.strDBName.c_str());

    BuildScriptParams(strParam, stdbInfo);
    ReplaceStr(strParam, strParamTmp, "%", "%%");
    iRet = CSystemExec::ExecScript(strScriptName, strParamTmp, NULL);

    if (ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR == iRet)
    {
        //如果返回的是通用错误码，则上报启动数据库失败的错误信息
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start SqlServer database(%s) failed, iRet %d.", 
            stdbInfo.strDBName.c_str(), ERROR_SQLSERVER_START_DB_FAILED);
        return ERROR_SQLSERVER_START_DB_FAILED;
    }
    if (MP_SUCCESS != iRet)
    {
        RETURN_MAP_ERRCODE(iRet, "start sqlserver");
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End start sqlserver database[%s]", stdbInfo.strDBName.c_str());

    return MP_SUCCESS;
}


/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::Stop
Description   : 停止SQLServer数据库;
Input         : sqlserver_info_t& stdbInfo, rest接口参数;
Output        : mp_int32
Return        : mp_void
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::Stop(sqlserver_info_t& stdbInfo)
{
    mp_string strParam;
    mp_string strParamTmp;
    mp_int32 iRet = MP_SUCCESS;
    mp_string strScriptName = SQLSERVER_SCRIPT_SQLSERVERRECOVER;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin stop database[%s]", stdbInfo.strDBName.c_str());

    BuildScriptParams(strParam, stdbInfo);
    ReplaceStr(strParam, strParamTmp, "%", "%%");
    iRet = CSystemExec::ExecScript(strScriptName, strParamTmp, NULL);
    if (ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR == iRet)
    {
        //如果返回的是通用错误码，则上报停止数据库失败的错误信息
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop SqlServer database(%s) failed, iRet %d.", 
            stdbInfo.strDBName.c_str(), ERROR_SQLSERVER_STOP_DB_FAILED);
        return ERROR_SQLSERVER_STOP_DB_FAILED;
    }

    if (MP_SUCCESS != iRet)
    {
        RETURN_MAP_ERRCODE(iRet, "stop sqlserver");
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End stop sqlserver database[%s]", stdbInfo.strDBName.c_str());

    return MP_SUCCESS;
}

/*--------------------------------------------------------------------------------------------------------
Function Name : CSqlServer::Test
Description   : 测试SQLServer数据库;
Input         : sqlserver_info_t& stdbInfo, rest接口参数;
Output        : mp_int32
Return        : mp_void
Create By    :
---------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::Test(sqlserver_info_t& stdbInfo)
{
    mp_string strParam;
    mp_string strParamTmp;
    mp_int32 iRet = MP_SUCCESS;
    mp_string strScriptName = SQLSERVER_SCRIPT_SQLSERVERSAMPLE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin test database[%s]", stdbInfo.strDBName.c_str());

    BuildScriptParams(strParam, stdbInfo);
    ReplaceStr(strParam, strParamTmp, "%", "%%");
    iRet = CSystemExec::ExecScript(strScriptName, strParamTmp, NULL);
    if (MP_SUCCESS != iRet)
    {
        RETURN_MAP_ERRCODE(iRet, "test sqlserver");
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End test sqlserver database [%s]", stdbInfo.strDBName.c_str());

    return MP_SUCCESS;
}

mp_int32 CSqlServer::TruncateTransLog(sqlserver_info_t& dbInfo)
{
    mp_string strParam;
    mp_string strParamTmp;
    mp_int32 iRet = MP_SUCCESS;
    mp_string strScriptName = SQLSERVER_SCRIPT_SQLSERVERSAMPLE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin truncate trans logs.");
    BuildScriptParams(strParam, dbInfo);
    ReplaceStr(strParam, strParamTmp, "%", "%%");
    iRet = CSystemExec::ExecScript(strScriptName, strParamTmp, NULL);
    if (MP_SUCCESS != iRet)
    {
        RETURN_MAP_ERRCODE(iRet, "truncate transaction log");
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Truncate trans logs succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CSqlServer::Freeze
Description  : 冻结数据库
Input        : vector<sqlserver_freeze_info_t> vecFreezeInfos , 冻结信息列表
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::Freeze(vector<sqlserver_freeze_info_t> vecFreezeInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<vss_db_oper_info_t> vssDbInfos;
    vss_db_oper_info_t vssDbInfo;
    vector<sqlserver_freeze_info_t>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin freeze sqlserver db.");
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
    iRet = m_pVssRequester->Freeze(vssDbInfos, vss_freeze_sqlserver);
    if (MP_SUCCESS != iRet)
    {
        delete m_pVssRequester;
        m_pVssRequester = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze sqlserver failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Freeze sqlserver db succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CSqlServer::UnFreeze
Description  : 解冻数据库
Input        : vector<sqlserver_unfreeze_info_t> vecUnFreezeInfos , 解冻信息列表
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::UnFreeze(vector<sqlserver_unfreeze_info_t> vecUnFreezeInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<vss_db_oper_info_t> vssDbInfos;
    vss_db_oper_info_t vssDbInfo;
    vector<sqlserver_unfreeze_info_t>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin unfreeze sqlserver db.");
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
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze sqlserver failed, iRet %d.", iRet);
        return iRet;
    }

    delete m_pVssRequester;
    m_pVssRequester = NULL;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Unfreeze sqlserver db succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------------------------------------------------------
Function Name: CSqlServer::GetFreezeStat
Description  : 获取冻结状态
Input        : 
Output       : mp_int32;
Return       : mp_int32
Create By    :
Modification :
Others       :
------------------------------------------------------------------------------------------------------------*/
mp_int32 CSqlServer::GetFreezeStat()
{
    if (NULL == m_pVssRequester)
    {
        return FREEZE_STAT_UNFREEZE;
    }

    return FREEZE_STAT_FREEZED;
}

#endif

