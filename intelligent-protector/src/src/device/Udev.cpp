/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/Udev.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/String.h"
#include "common/RootCaller.h"

#ifndef WIN32
/*------------------------------------------------------------ 
Description  : 获取udev规则文件名
Input        : 
Output       : strUdevRulesFileName--返回获取到的udev规则文件名
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CUdev::GetUdevRulesFileName(mp_string& strUdevRulesFileName)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsSpecial = MP_TRUE;
    vector<mp_string> vecResult;
    mp_string strParam;
    mp_string::iterator iter;
    mp_string strUdevRulesFileDir;

    vecResult.clear();

    strParam = " /etc/udev/udev.conf|grep udev_rules|grep -v '#'|awk -F \"=\" '{print $2}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CAT, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "Get udev rule files dir failed.");
        return iRet;
    }

    if(vecResult.empty())
    {
        strUdevRulesFileName = "/etc/udev/rules.d/99-oracle-asmdevices.rules";
    }
    else
    {
        strUdevRulesFileDir = vecResult.front();
        for (iter = strUdevRulesFileDir.begin();iter != strUdevRulesFileDir.end();)
        {
            bIsSpecial = ( *iter == '\"' ||*iter == ' ');
            if (bIsSpecial)
            {
                iter = strUdevRulesFileDir.erase(iter);
            }
            else
            {
                iter++;
            }
        }
        strUdevRulesFileName = strUdevRulesFileDir + FILENAME_UDEVRULES;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The udev rules file full name is:%s.",strUdevRulesFileName.c_str());
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 创建udev规则
Input        : strUdevRule -- udev规则
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CUdev::Create(mp_string& strUdevRule, mp_string& strWWN)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strLowerWWN = "";
    const mp_string  SeparatorFir = "RESULT==\"3";
    const mp_string  SeparatorSec = "\"";
    const mp_string  SpecialChar = "\"$";
    const mp_string  SpecialCharGrep="*";
    mp_string strParam;
    mp_string strTmp;
    mp_string strUdevRulesFileName;
    mp_string strRealUdevRule = strUdevRule;
    vector<mp_string> vecResult;
    size_t idxSep,idxSepSec;
    size_t idx = 0;
    size_t idxPos = 0;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,"Begin to write udev rule info file.");

    iRet = GetUdevRulesFileName(strUdevRulesFileName);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_UDEV_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"Get udev rules file name failed.");
        return iRet;
    }

    idxSep = strRealUdevRule.find(SeparatorFir);
    if (mp_string::npos == idxSep)
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"Udev rule record:%s is not invaild.", strRealUdevRule.c_str());
         return ERROR_DEVICE_UDEV_CREATE_FAILED;
    }

    idxSep = idxSep + SeparatorFir.length();
    idxSepSec = strRealUdevRule.find(SeparatorSec,idxSep);
    if (mp_string::npos == idxSepSec)
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"Udev rule record[%s] is not invaild.", strRealUdevRule.c_str());
         return ERROR_DEVICE_UDEV_CREATE_FAILED;
    }

    strLowerWWN = CMpString::ToLower((mp_char*)(strWWN.c_str()));
    strRealUdevRule.replace(idxSep,idxSepSec - idxSep, strLowerWWN);
    strTmp = strRealUdevRule;
    for(idx = 0;idx < strTmp.length();idx ++)
    {
        idxPos = SpecialCharGrep.find(strTmp.at(idx));
        if(mp_string::npos == idxPos)
        {
              continue;
        }
        strTmp.insert(idx,"\\");
        idx ++;
    }

    strParam = " " + strUdevRulesFileName + "| grep '^" + strTmp + "$'";

    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CAT, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_UDEV_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"Inquire udev rule record[%s] failed.", strTmp.c_str());
        return iRet;
    }

    if(!vecResult.empty())
    {
          COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,"Udev rule record[%s] is already exist.", strTmp.c_str());
          return MP_SUCCESS;
    }

    for(idx = 0;idx < strRealUdevRule.length(); idx++)
    {
        idxPos = SpecialChar.find(strRealUdevRule.at(idx));
        if(mp_string::npos == idxPos)
        {
              continue;
        }
        strRealUdevRule.insert(idx,"\\");
        idx ++;
    }

    strParam = " " + strRealUdevRule +">> \"" + strUdevRulesFileName + "\"";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_ECHO, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_UDEV_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
      COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"Write udev rule record failed.");
      return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,"End to write udev rule info file.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :删除udev规则
Input        : strUdevRule -- udev规则
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CUdev::Delete(mp_string& strUdevRule, mp_string& strWWN)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strLowerWWN = "";
    mp_string strParam;
    mp_string strUdevRulesFileName;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,"Begin to delete udev rule of file.");

    iRet = GetUdevRulesFileName(strUdevRulesFileName);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_UDEV_DELETE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"Get udev rules file name failed.");
        return iRet;
    }

    strLowerWWN = CMpString::ToLower((mp_char*)(strWWN.c_str()));
    strParam = " -i /" + strLowerWWN + "/d " + strUdevRulesFileName;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SED, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_UDEV_DELETE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,"Delete udev rule record failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,"End to delete udev rule of file.");
    return MP_SUCCESS;
}

mp_int32 CUdev::ReloadRules()
{
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to reload udev rules.");
    mp_string strParam;
    mp_int32 iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_UDEV_RELOAD_RULES, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Reload udev rule record with root permission failed.");
        return MP_FAILED;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,"Reload udev rules succ.");
    return MP_SUCCESS;
}

#endif //WIN32

