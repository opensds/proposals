/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/


#include "apps/cache/Cache.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/Defines.h"
#include "common/RootCaller.h"
#include "array/Array.h"

CCache::CCache()
{
}

CCache::~CCache()
{
}

/*------------------------------------------------------------ 
Description  : 分析获取数据库信息脚本返回结果中的数据库信息
Input        : vecResult -- 数据库信息字符串
Output       : vecdbInstInfo -- 数据库信息
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CCache::AnalyseInstInfoScriptRst(vector<mp_string> vecResult, vector<cache_inst_info_t> &vecdbInstInfo)
{
    size_t idxSep, idxSepSec;
    mp_string strinstName;
    mp_string strdbName;
    mp_string strversion;
    mp_string strState;
    vector<mp_string>::iterator iter;

    cache_inst_info_t stdbInstInfo;
    const mp_string SEPARATOR = STR_COLON;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin analyse instance info script result.");
    
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get inst info failed when find 1nd separator, inst info is [%s].", (*iter).c_str());;
            continue;
        }
        strinstName = iter->substr(0, idxSep);

        //find 2rd separator(;)    
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
         if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get inst info failed when find 2nd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }    
        strversion = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);
        strState = iter->substr(idxSepSec + 1);

        stdbInstInfo.strinstName = strinstName;
        stdbInstInfo.strversion= strversion;
        stdbInstInfo.istate= atoi(strState.c_str());
        vecdbInstInfo.push_back(stdbInstInfo);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "The analyse result of script is:instname(%s), version(%s), state(%d).",
            stdbInstInfo.strinstName.c_str(), 
            stdbInstInfo.strversion.c_str(), stdbInstInfo.istate, vecdbInstInfo.size());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to analyse instance info script result.");
}

/*------------------------------------------------------------ 
Description  : 分析获取数据库LUN信息脚本返回结果中的数据库存储信息
Input        : vecResult -- 数据库存储信息字符串
Output       : vecdbStorInfo -- 数据库存储信息
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CCache::AnalyseLunInfoScriptRST(vector<mp_string> vecResult, vector<cache_storage_info_t> &vecdbStorInfo)
{  
    size_t idxSep, idxSepSec, idxSepTrd, idxSepFor, idxSepFiv;
    mp_string strvolName;
    mp_string strvgName;
    mp_string strdeviceName;
    mp_string strdiskName;
    mp_string strvolType;
    mp_string strstorageType;
    vector<mp_string>::iterator iter;

    cache_storage_info_t stdbStorInfo;
    const mp_string SEPARATOR = STR_COLON;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get instance storage info.");
    
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get instance storage info failed when find 1nd separator, storage info is [%s].", (*iter).c_str());;
            continue;
        }
        strvolName = iter->substr(0, idxSep);

        //find 2nd separator(;)            
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get instance storage info failed when find 2nd separator, storage info is [%s].", (*iter).c_str());
            continue;
        }      
        strvgName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);  

        //find 3rd separator(;)挂载点
        idxSepTrd = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepTrd)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get instance storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
            continue;
        }    
        strdeviceName = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);

        //find 4rd separator(;)    
        idxSepFor = iter->find(SEPARATOR, idxSepTrd + 1);
        if (mp_string::npos == idxSepFor)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get instance storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
            continue;
        } 
        strdiskName = iter->substr(idxSepTrd + 1, (idxSepFor - idxSepTrd) - 1);

        //find 5rd separator(;)    
        idxSepFiv = iter->find(SEPARATOR, idxSepFor + 1);
        if (mp_string::npos == idxSepFiv)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get instance storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
            continue;
        } 
        strvolType = iter->substr(idxSepFor + 1, (idxSepFiv - idxSepFor) - 1);
        strstorageType = iter->substr(idxSepFiv + 1);
    
        stdbStorInfo.strvolName = strvolName;
        stdbStorInfo.strvgName= strvgName;
        stdbStorInfo.strdeviceName= strdeviceName;
        stdbStorInfo.strdiskName = strdiskName;
        stdbStorInfo.ivolType = atoi(strvolType.c_str());
        stdbStorInfo.istorageType= atoi(strstorageType.c_str());
        vecdbStorInfo.push_back(stdbStorInfo);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "Get instance storage info structure:volName(%s), vgName(%s), deviceName(%s), diskName(%s), volType(%d), storageType(%d).", 
            stdbStorInfo.strvolName.c_str(), stdbStorInfo.strvgName.c_str(), stdbStorInfo.strdeviceName.c_str(), 
            stdbStorInfo.strdiskName.c_str(), stdbStorInfo.ivolType, stdbStorInfo.istorageType);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to get instance storage info.");
}


/*------------------------------------------------------------ 
Description  : 构建获取LUN信息脚本输入参数，虽然现在只有一个参数，
但是后来可能会有其它的参数，所以保留。
Input        : stdbinfo -- 数据库信息
Output       : strParam -- 输入参数字符串
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
void CCache::BuildLunInfoScriptParam(cache_db_info_t stdbinfo, mp_string& strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbinfo.strinstName;
}


/*------------------------------------------------------------ 
Description  : 构建执行保护策略和启动停止脚本输入参数
Input        : stdbinfo -- 数据库信息
                  strOperType --操作类型
Output       : strParam -- 输入参数字符串
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
void CCache::BuildScriptParam(cache_db_info_t stdbinfo, mp_string strOperType, mp_string& strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbinfo.strinstName;
}

/*------------------------------------------------------------ 
Description  : 获取磁盘LUN信息
Input        : vecdbStorInfo -- 数据库存储信息
Output       : vecdbLunInfo -- 数据库的LUN信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCache::GetDiskLunInfo(vector<cache_storage_info_t> vecdbStorInfo, vector<cache_lun_info_t> &vecdbLunInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    vector<cache_storage_info_t>::iterator iter;
    cache_lun_info_t stdbLunInfo;
    mp_string strLunWWN;
    mp_string strLunID;
    mp_string strSN;
    mp_string strVendor;
    mp_string strProduct;
    mp_string strDev;


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get lun info of disk.");
    
    for (iter = vecdbStorInfo.begin(); iter != vecdbStorInfo.end(); ++iter)
    {
        strDev = iter->strdiskName;
        //厂商和型号
        iRet = CArray::GetArrayVendorAndProduct(strDev, strVendor, strProduct);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get array info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        }

        (void)CMpString::Trim((mp_char*)strVendor.c_str());
        (void)CMpString::Trim((mp_char*)strProduct.c_str());
        
        //排除掉非华为的产品
        if (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY))
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "The disk(%s) is not huawei LUN, Vendor:%s.", 
                strDev.c_str(), strVendor.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        iRet = CArray::GetArraySN(strDev, strSN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SN info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        }

        iRet = CArray::GetLunInfo(strDev, strLunWWN, strLunID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        }

        stdbLunInfo.strpvName = iter->strdiskName;
        stdbLunInfo.strvolName = iter->strvolName;
        stdbLunInfo.strvgName=iter->strvgName;
        stdbLunInfo.strdeviceName = iter->strdeviceName;
        stdbLunInfo.ivolType = iter->ivolType;
        stdbLunInfo.istorageType = iter->istorageType;
        stdbLunInfo.strarraySn = strSN;
        stdbLunInfo.strlunId = strLunID;
        stdbLunInfo.strwwn = strLunWWN;
        vecdbLunInfo.push_back(stdbLunInfo);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "Get lun info of disk:arraySN(%s), lunId(%s), wwn(%s).", 
            stdbLunInfo.strarraySn.c_str(), stdbLunInfo.strlunId.c_str(), stdbLunInfo.strwwn.c_str());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to get lun info of disk.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 获取数据库信息
Input        :
Output       : vecdbInstInfo -- 数据库信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCache::GetInfo(vector<cache_inst_info_t> &vecdbInstInfo)
{
    mp_string strParam;
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get instance info.");

    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYCACHEINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute get instance info script failed, iRet %d.", iRet);
        return iRet;
    }
    AnalyseInstInfoScriptRst(vecResult, vecdbInstInfo);
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get instance info succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 获取数据库LUN信息
Input        :vecdbInstInfo -- 数据库信息
Output       : vecLunInfos -- 数据库LUN信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCache::GetLunInfo(cache_db_info_t stdbinfo, vector<cache_lun_info_t>& vecLunInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;
    vector<cache_storage_info_t> vecdbStorInfo;
        
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get instance(%s) lun info.", stdbinfo.strinstName.c_str());
    
    BuildLunInfoScriptParam(stdbinfo, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYCACHELUNINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute get instance(%s) lun info script failed, iRet %d.", stdbinfo.strinstName.c_str(), iRet);
        return iRet;
    }
    
    AnalyseLunInfoScriptRST(vecResult, vecdbStorInfo);

    iRet = GetDiskLunInfo(vecdbStorInfo, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info of disk failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get instance lun(%s) info succ.", stdbinfo.strinstName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 测试连接数据库
Input        :stdbInfo -- 数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CCache::Test(cache_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test instance(%s).", stdbInfo.strinstName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_TESTDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLECACHE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute test instance(%s) script failed, iRet %d.", stdbInfo.strinstName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test instance(%s) succ.", stdbInfo.strinstName.c_str());
    return MP_SUCCESS;
}

