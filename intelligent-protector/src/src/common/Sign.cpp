/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Sign.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Path.h"
#include "common/Log.h"
#include "common/UniqueId.h"
#include "common/ErrorCode.h"
#include "securec.h"


#ifdef WIN32
    #define SCRIPT_AGENT_FUNC       "agent_func.bat"
    #define SCRIPT_AGENT_FUNC_PS    "agent_func.ps1"
    #define SCRIPT_ORACLE_FUNC      "oraclefunc.bat"
    #define ORACLE_SCRIPT_PREFIX    "ora"
#else
    #define SCRIPT_AGENT_FUNC       "agent_func.sh"
#endif

/*------------------------------------------------------------ 
Description  : 根据脚本名称，获取所有需要校验的脚本名称
Input        : strFileName -- 脚本名称
Output       : mapScriptNames -- 所有需要校验的脚本名称
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void GetScriptNames(const mp_string strFileName, map<mp_string,mp_int32>& mapScriptNames)
{
    mapScriptNames.insert(map<mp_string,mp_int32>::value_type(strFileName, 0));
    mp_string strNameTmp = mp_string(SCRIPT_AGENT_FUNC);
    mapScriptNames.insert(map<mp_string,mp_int32>::value_type(strNameTmp, 0));

#ifdef WIN32
    strNameTmp = mp_string(SCRIPT_AGENT_FUNC_PS);
    mapScriptNames.insert(map<mp_string,mp_int32>::value_type(strNameTmp, 0));
    
    if(strFileName.find(ORACLE_SCRIPT_PREFIX) == 0)
    {
        strNameTmp = mp_string(SCRIPT_ORACLE_FUNC);
        mapScriptNames.insert(map<mp_string,mp_int32>::value_type(strNameTmp, 0));
    }
#endif
}

/*------------------------------------------------------------ 
Description  : 校验脚本签名
Input        : strFileName -- 脚本名称
Output       : strSignEncrypt -- 加密后的脚本签名
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CheckSign(mp_string strFileName, mp_string strSignEncrypt)
{
    if (strSignEncrypt.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Script sign is not found, script name is \"%s\".", strFileName.c_str());
        return ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED;
    }
    
    mp_string strFilePath = CPath::GetInstance().GetBinFilePath(strFileName);
    mp_int32 iRet = VerifyHMAC(strFilePath, strSignEncrypt);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check script sign failed, sign is not equal, script name is \"%s\".", 
            strFileName.c_str());
        return ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 校验脚本签名
Input        : strFileName -- 脚本名称
               vecSigns -- 签名文件内容
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CheckFileSign(const mp_string strFileName, const vector<mp_string>& vecSigns)
{
    mp_bool bScriptSignFound = MP_FALSE;
    mp_string strTmp;
    mp_string::size_type iPosTmp = mp_string::npos;
    for (mp_uint32 i = 0; i < vecSigns.size(); i++)
    {
        strTmp = vecSigns[i];
        iPosTmp = strTmp.find(strFileName, 0);
        if (iPosTmp != mp_string::npos)
        {
            bScriptSignFound = MP_TRUE;
            break;
        }
    }
    if (bScriptSignFound == MP_FALSE)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Script sign is not found, script name is \"%s\".", strFileName.c_str());
        return ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED;
    }

    iPosTmp += strFileName.length();
    mp_string::size_type iPosBegin = strTmp.find(SIGN_FORMAT_STR, iPosTmp);
    if (iPosBegin == mp_string::npos)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Script sign file is in wrong format, \"%s\" is not found.", SIGN_FORMAT_STR);
        return ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED;
    }

    mp_string strSignEncrypt = "";
    iPosBegin += strlen(SIGN_FORMAT_STR);
    mp_string::size_type iPosEnd = strTmp.length()- 1;
    for (; iPosBegin < iPosEnd; iPosBegin++)
    {
        if (strTmp[iPosBegin] != ' ' && strTmp[iPosBegin] != '\t')
        {
            break;
        }
    }
    
    if (iPosBegin < iPosEnd)
    {
        for (; iPosEnd > iPosBegin; iPosEnd--)
        {
            if (strTmp[iPosEnd] != ' ' && strTmp[iPosEnd] != '\t')
            {
                break;
            }
        }
        strSignEncrypt = strTmp.substr(iPosBegin, (iPosEnd-iPosBegin)+1);
    }

    return CheckSign(strFileName, strSignEncrypt);
}

#ifdef WIN32
    static const mp_string SIGN_SCRIPT_LIST = "agent_func.bat;agent_func.ps1;agent_start.bat;agent_stop.bat;"
        "exchange.bat;initiator.bat;install-provider.cmd;online.bat;operexchangedb.ps1;oraasmaction.bat;"
        "oraclecheckarchive.bat;oracleconsistent.bat;oraclefunc.bat;oracleinfo.bat;oracleinst.bat;oracleluninfo.bat;oraclecheckcdb.bat;"
        "oracletest.bat;oradbaction.bat;process_start.bat;process_stop.bat;procmonitor.bat;queryexchangeinfo.ps1;"
        "register_app.vbs;sqlserverinfo.bat;sqlserverluninfo.bat;sqlserverrecover.bat;sqlserversample.bat;"
        "uninstall-provider.cmd;packlog.bat;rotatenginxlog.bat";
#else
    static const mp_string SIGN_SCRIPT_LIST = "agent_func.sh;agent_start.sh;agent_stop.sh;db2clusterinfo.sh;"
        "db2info.sh;db2luninfo.sh;db2recover.sh;db2resourcegroup.sh;db2sample.sh;initiator.sh;oraasmaction.sh;"
        "oraclecheckarchive.sh;oracleclusterinfo.sh;oracleconsistent.sh;oracleinfo.sh;oracleinst.sh;oracleluninfo.sh;oraclepdbinfo.sh;oraclecheckcdb.sh;oraclepdbstart.sh;"
        "oracleresourcegroup.sh;oracletest.sh;oradbaction.sh;procmonitor.sh;lvmfunc.sh;packlog.sh;rotatenginxlog.sh;"
        "cachefunc.sh;cacheclusterinfo.sh;cacheinfo.sh;cacheluninfo.sh;cachesample.sh;scandisk.sh;"
        "hanaconsistent.sh;hanarecover.sh;hanatest.sh;hanafunc.sh;sybaseconsistent.sh;sybaserecover.sh;sybasetest.sh;sybasefunc.sh"; 
#endif


/*------------------------------------------------------------ 
Description  : 校验脚本签名
Input        : strFileName -- 脚本文件名
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CheckScriptSign(const mp_string strFileName)
{
    if (SIGN_SCRIPT_LIST.find(strFileName) == mp_string::npos)
    {
        return MP_SUCCESS;
    }

    mp_string strScriptSignFile = CPath::GetInstance().GetConfFilePath(AGENT_SCRIPT_SIGN);
    if (!CMpFile::FileExist(strScriptSignFile.c_str()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Script sign file does not exist, path is \"%s\".", AGENT_SCRIPT_SIGN);
        return ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED;
    }

    vector<mp_string> vecSigns;
    mp_int32 iRet = CMpFile::ReadFile(strScriptSignFile, vecSigns);
    if (MP_SUCCESS != iRet || vecSigns.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read script sign file failed, path is \"%s\", iRet = %d, size of vecResult is %d.",
            AGENT_SCRIPT_SIGN, iRet, vecSigns.size());
        return iRet;
    }

    map<mp_string,mp_int32> mapScriptNames;
    GetScriptNames(strFileName, mapScriptNames);

    for (map<mp_string,mp_int32>::iterator it=mapScriptNames.begin(); it!=mapScriptNames.end(); ++it)
    {
        iRet = CheckFileSign(it->first, vecSigns);
        if (iRet != MP_SUCCESS)
        {
            return iRet;
        }
    }
    
    return MP_SUCCESS;
}
