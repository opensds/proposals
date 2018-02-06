/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/db2/Db2.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/Defines.h"
#include "common/RootCaller.h"
#include "array/Array.h"

CDB2::CDB2()
{
}

CDB2::~CDB2()
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
mp_void CDB2::AnalyseInstInfoScriptRst(vector<mp_string> vecResult, vector<db2_inst_info_t> &vecdbInstInfo)
{
    size_t idxSep, idxSepSec, idxSepTrd;
    mp_string strinstName;
    mp_string strdbName;
    mp_string strversion;
    mp_string strState;
    vector<mp_string>::iterator iter;

    db2_inst_info_t stdbInstInfo;
    const mp_string SEPARATOR = STR_COLON;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin analyse db info script result.");
    
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 1nd separator, inst info is [%s].", (*iter).c_str());;
            continue;
        }
        strinstName = iter->substr(0, idxSep);

        //find 2nd separator(;)            
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 2nd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }      
        strdbName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);  

        //find 3rd separator(;)    
        idxSepTrd = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepTrd)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 3rd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }    
        strversion = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);
        strState = iter->substr(idxSepTrd + 1);

        stdbInstInfo.strinstName = strinstName;
        stdbInstInfo.strdbName = strdbName;
        stdbInstInfo.strversion= strversion;
        stdbInstInfo.istate= atoi(strState.c_str());
        vecdbInstInfo.push_back(stdbInstInfo);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "The analyse result of script is:instname(%s), dbname(%s), version(%s), state(%d).",
            stdbInstInfo.strinstName.c_str(), stdbInstInfo.strdbName.c_str(),
            stdbInstInfo.strversion.c_str(), stdbInstInfo.istate, vecdbInstInfo.size());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to analyse db info script result.");
}

/*------------------------------------------------------------ 
Description  : 分析获取数据库LUN信息脚本返回结果中的数据库存储信息
Input        : vecResult -- 数据库存储信息字符串
Output       : vecdbStorInfo -- 数据库存储信息
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CDB2::AnalyseLunInfoScriptRST(vector<mp_string> vecResult, vector<db2_storage_info_t> &vecdbStorInfo)
{  
    size_t idxSep, idxSepSec, idxSepTrd, idxSepFor, idxSepFiv;
    mp_string strvolName;
    mp_string strvgName;
    mp_string strdeviceName;
    mp_string strdiskName;
    mp_string strvolType;
    mp_string strstorageType;
    vector<mp_string>::iterator iter;

    db2_storage_info_t stdbStorInfo;
    const mp_string SEPARATOR = STR_COLON;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get db storage info.");
    
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 1nd separator, storage info is [%s].", (*iter).c_str());;
            continue;
        }
        strvolName = iter->substr(0, idxSep);

        //find 2nd separator(;)            
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 2nd separator, storage info is [%s].", (*iter).c_str());
            continue;
        }      
        strvgName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);  

        //find 3rd separator(;)挂载点
        idxSepTrd = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepTrd)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
            continue;
        }    
        strdeviceName = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);

        //find 4rd separator(;)    
        idxSepFor = iter->find(SEPARATOR, idxSepTrd + 1);
        if (mp_string::npos == idxSepFor)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
            continue;
        } 
        strdiskName = iter->substr(idxSepTrd + 1, (idxSepFor - idxSepTrd) - 1);

        //find 5rd separator(;)    
        idxSepFiv = iter->find(SEPARATOR, idxSepFor + 1);
        if (mp_string::npos == idxSepFiv)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
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
            "Get db storage info structure:volName(%s), vgName(%s), deviceName(%s), diskName(%s), volType(%d), storageType(%d).", 
            stdbStorInfo.strvolName.c_str(), stdbStorInfo.strvgName.c_str(), stdbStorInfo.strdeviceName.c_str(), 
            stdbStorInfo.strdiskName.c_str(), stdbStorInfo.ivolType, stdbStorInfo.istorageType);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to get db storage info.");
}


/*------------------------------------------------------------ 
Description  : 构建获取LUN信息脚本输入参数
Input        : stdbinfo -- 数据库信息
Output       : strParam -- 输入参数字符串
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
void CDB2::BuildLunInfoScriptParam(db2_db_info_t stdbinfo, mp_string& strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbinfo.strinstName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbinfo.strdbName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbinfo.strdbUsername+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbinfo.strdbPassword;
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
void CDB2::BuildScriptParam(db2_db_info_t stdbinfo, mp_string strOperType, mp_string& strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbinfo.strinstName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbinfo.strdbName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbinfo.strdbUsername+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbinfo.strdbPassword+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_OPERTYPE) + strOperType;
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
mp_int32 CDB2::GetDiskLunInfo(vector<db2_storage_info_t> vecdbStorInfo, vector<db2_lun_info_t> &vecdbLunInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    vector<db2_storage_info_t>::iterator iter;
    db2_lun_info_t stdbLunInfo;
    mp_string strLunWWN;
    mp_string strLunID;
    mp_string strSN;
    mp_string strVendor;
    mp_string strProduct;
    mp_string strDev;


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get lun info of disk.");
    
    for (iter = vecdbStorInfo.begin(); iter != vecdbStorInfo.end(); ++iter)
    {
#ifdef HP_UX_IA

        iRet = CDisk::GetHPRawDiskName(iter->strdiskName, strDev);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get raw device info of disk(%s) failed, iRet %d.",
                iter->strdiskName.c_str(), iRet);
            return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
        }
#else
        strDev = iter->strdiskName;
#endif
        //厂商和型号
        iRet = CArray::GetArrayVendorAndProduct(strDev, strVendor, strProduct);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get array info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        (void)CMpString::Trim((mp_char*)strVendor.c_str());
        (void)CMpString::Trim((mp_char*)strProduct.c_str());
        
        //排除掉非华为的产品
        if (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The disk(%s) is not huawei LUN, Vendor:%s.", 
                strDev.c_str(), strVendor.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        iRet = CArray::GetArraySN(strDev, strSN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SN info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        iRet = CArray::GetLunInfo(strDev, strLunWWN, strLunID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
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
mp_int32 CDB2::GetInfo(vector<db2_inst_info_t> &vecdbInstInfo)
{
    mp_string strParam;
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get db info.");

    //db2下获取实例状态命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYDB2INFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute get db info script failed, iRet %d.", iRet);
        return iRet;
    }
    AnalyseInstInfoScriptRst(vecResult, vecdbInstInfo);
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get db info succ.");
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
mp_int32 CDB2::GetLunInfo(db2_db_info_t stdbinfo, vector<db2_lun_info_t>& vecLunInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;
    vector<db2_storage_info_t> vecdbStorInfo;
        
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get db(%s) lun info.", stdbinfo.strdbName.c_str());
    
    BuildLunInfoScriptParam(stdbinfo, strParam);
    
    //db2下获取实例状态命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYDB2LUNINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute get db(%s) lun info script failed, iRet %d.", stdbinfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    AnalyseLunInfoScriptRST(vecResult, vecdbStorInfo);

    iRet = GetDiskLunInfo(vecdbStorInfo, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info of disk failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get db lun(%s) info succ.", stdbinfo.strdbName.c_str());
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 启动数据库
Input        :stdbInfo -- 数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::Start(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_STARTDB, strParam);
    
    //db2下启动数据库命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_RECOVERDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute start db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 停止数据库
Input        :stdbInfo -- 数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::Stop(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin stop db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_STOPDB, strParam);
    
    //db2下停止数据库命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_RECOVERDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute stop db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop db(%s) succ.", stdbInfo.strdbName.c_str());
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
mp_int32 CDB2::Test(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_TESTDB, strParam);
    
    //db2下测试数据库连接命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLEDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute test db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 冻结数据库
Input        :stdbInfo -- 数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::Freeze(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to freeze db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_FREEZEDB, strParam);
    
    //db2下冻结数据库命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLEDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute freeze db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 解冻数据库
Input        :stdbInfo -- 数据库信息
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::UnFreeze(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to thaw db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_THAWDB, strParam);
    
    //db2下解冻数据库命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLEDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute thaw db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 查询数据库冻结状态
Input        :stdbInfo -- 数据库信息
Output       : iFreezeState -- 数据库冻结状态
Return       :  MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::QueryFreezeState(db2_db_info_t stdbInfo, mp_int32& iFreezeState)
{
    mp_string strParam;
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query db(%s) freeze state.", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_FREEZESTATE, strParam);

	//codedex误报CHECK_CONTAINER_EMPTY,容器vecResult在之前的代码能保证不为空，此处可以不判断
    //db2下查询数据库冻结状态命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLEDB2, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute query db(%s) freeze state script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    iFreezeState = atoi(vecResult.front().c_str());
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, 
        "Query db(%s) freeze state(%d) succ.", stdbInfo.strdbName.c_str(), iFreezeState);
    return MP_SUCCESS;
}

