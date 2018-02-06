/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/String.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/Sign.h"
#include "common/ErrorCode.h"
#include "securec.h"

#ifdef WIN32
    static const mp_string SCRIPT_LIST = "agent_func.bat;agent_func.ps1;agent_start.bat;agent_stop.bat;exchange.bat;"
        "initiator.bat;install-provider.cmd;online.bat;operexchangedb.ps1;oraasmaction.bat;oraclecheckarchive.bat;"
        "oracleconsistent.bat;oraclefunc.bat;oracleinfo.bat;oracleluninfo.bat;oracletest.bat;oraclecheckcdb.bat;"
        "oradbaction.bat;process_start.bat;process_stop.bat;procmonitor.bat;queryexchangeinfo.ps1;register_app.vbs;"
        "sqlserverinfo.bat;sqlserverluninfo.bat;sqlserverrecover.bat;sqlserversample.bat;uninstall-provider.cmd;"
        "packlog.bat;rotatenginxlog.bat";
#else
    static const mp_string SCRIPT_LIST = "agent_func.sh;agent_start.sh;agent_stop.sh;db2clusterinfo.sh;"
        "db2info.sh;db2luninfo.sh;db2recover.sh;db2resourcegroup.sh;db2sample.sh;initiator.sh;oraclefunc.sh;oraasmaction.sh;"
        "oraclecheckarchive.sh;oracleclusterinfo.sh;oracleconsistent.sh;oracleinfo.sh;oracleluninfo.sh;oraclepdbinfo.sh;oraclecheckcdb.sh;oraclepdbstart.sh;"
        "oracleresourcegroup.sh;oracletest.sh;oradbaction.sh;procmonitor.sh;lvmfunc.sh;packlog.sh;rotatenginxlog.sh;"
        "cachefunc.sh;cacheclusterinfo.sh;cacheinfo.sh;cacheluninfo.sh;cachesample.sh;scandisk.sh;"
        "hanaconsistent.sh;hanarecover.sh;hanatest.sh;hanafunc.sh;sybaseconsistent.sh;sybaserecover.sh;sybasetest.sh;sybasefunc.sh";   
#endif

/*------------------------------------------------------------ 
Description  : 根据脚本名称，生成脚本签名
Input        : strFileName -- 脚本名称
Output       : strFileSign -- 脚本签名
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 GenScriptSign(const mp_string strFileName, mp_string& strFileSign)
{
    strFileSign.clear();
    mp_string strFilePath = CPath::GetInstance().GetBinFilePath(strFileName);
    
    mp_int32 iRet = ComputeHMAC(strFilePath, strFileSign);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ComputeHMAC failed, iRet = %d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 生成脚本签名文件
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 GenSignFile()
{
    vector<mp_string> vecScriptFiles;
    CMpString::StrSplit(vecScriptFiles, SCRIPT_LIST, ';');

    mp_int32 iRet = MP_SUCCESS;
    mp_string strScriptSignFile = CPath::GetInstance().GetConfFilePath(AGENT_SCRIPT_SIGN);
    if (CMpFile::FileExist(strScriptSignFile.c_str()))
    {
        iRet = CMpFile::DelFile(strScriptSignFile.c_str());
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Script sign file already exist, failed to delete, path is \"%s\", iRet = %d.", BaseFileName(strScriptSignFile.c_str()));
            return iRet;
        }
    }
    
    vector<mp_string> vecScriptSigns;
    mp_string strFileName, strFileSign, strTmp;
    for (mp_uint32 i = 0; i < vecScriptFiles.size(); i++)
    {
        strFileName = vecScriptFiles[i];
        iRet = GenScriptSign(strFileName, strFileSign);
        if (iRet == MP_SUCCESS)
        {
            strTmp = strFileName + " " + SIGN_FORMAT_STR + " " + strFileSign;
            vecScriptSigns.push_back(strTmp);
        }
    }

    iRet = CIPCFile::WriteFile(strScriptSignFile, vecScriptSigns);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write script sign file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecScriptSigns.size());
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Generate script sign file succeeded.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : main函数，无输入
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    //初始化路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (MP_SUCCESS != iRet)
    {
        printf("Init path %s failed.\n", argv[0]);
        return iRet;
    }

    //初始化xml配置
    mp_string strXMLConfFilePath = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    iRet = CConfigXmlParser::GetInstance().Init(strXMLConfFilePath);
    if (MP_SUCCESS != iRet)
    {
        printf("Init xml conf file %s failed.\n", AGENT_XML_CONF);
        return iRet;
    }

    //初始化日志文件
    mp_string strLogFilePath = CPath::GetInstance().GetLogPath();
    CLogger::GetInstance().Init(mp_string(SCRIPT_SIGN_LOG_NAME).c_str(), strLogFilePath);

    //初始化KMC
    iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin generating script sign file.");

    iRet = GenSignFile();
    //程序即将退出，此处不判断返回值
    (mp_void)FinalizeCrypt();
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End generating script sign file.");
    return iRet;
}

