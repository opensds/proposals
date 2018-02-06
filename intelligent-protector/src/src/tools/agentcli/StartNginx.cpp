/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/StartNginx.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/SystemExec.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "host/Host.h"
#include "common/Ip.h"
#include "common/Password.h"
#include "common/Sign.h"
#include "common/CryptAlg.h"
#include "common/ConfigXmlParse.h"

mp_int32 CStartNginx::GetPassword(mp_string &CipherStr)
{
    if ( MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, CipherStr))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get GetValueString of ssl_key_password failed");
        return MP_FAILED;
    }
    
    return MP_SUCCESS;
}

mp_int32 CStartNginx::ExecNginxStart()
{
#ifdef WIN32
    //启动Nginx
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    //校验脚本签名
    mp_int32 iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }
    iRet = CheckScriptSign(START_SCRIPT);
    //程序即将退出，此处不判断返回值
    (mp_void)FinalizeCrypt();
    
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Script sign check failed, script name is \"%s\", iRet = %d.", 
            START_SCRIPT, iRet);
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    //校验脚本签名
    mp_int32 iRet = CheckScriptSign(START_SCRIPT);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CheckScriptSign failed, iRet = %d", iRet);
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd+ " " + NGINX_AS_PARAM_NAME;
    if(0 == getuid())
    {
         strCmd = "su - rdadmin -c \" " + strCmd + " \" ";   
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "execute :%s", strCmd.c_str());
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    return MP_SUCCESS;    
}

mp_int32 CStartNginx::Handle()
{
    mp_int32 iRet;
    mp_string inStr;
    mp_string outStr;
    vector<mp_string> vecResult;
    
    mp_string strNginxPWDFile = CPath::GetInstance().GetNginxConfFilePath(SSL_PASSWORD_TEMP_FILE);
    if ( MP_SUCCESS != GetPassword(inStr))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get encryptStr failed");
        printf("Process nginx of OceanStor BCManager Agent was started failed.\n");
        return MP_FAILED;
    }
    
    DecryptStr(inStr, outStr);
    if (outStr.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "DecryptStr nginx password failed.");
        return MP_FAILED;
    }
    
    vecResult.push_back(outStr.c_str());
    if ( MP_FAILED == CIPCFile::WriteFile(strNginxPWDFile, vecResult))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "write password to File failed");
        printf("Process nginx of OceanStor BCManager Agent was started failed.\n");
        return MP_FAILED;
    }
    
    if ( MP_SUCCESS != ExecNginxStart())
    {
        iRet = CMpFile::DelFile(strNginxPWDFile.c_str());
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete tmp file failed, ret is %d", iRet);
        }
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "start nginx failed");
        printf("Process nginx of OceanStor BCManager Agent was started failed.\n");
        return MP_FAILED;
    }

    iRet = CMpFile::DelFile(strNginxPWDFile.c_str());
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete tmp file failed, ret is %d", iRet);
        printf("Process nginx of OceanStor BCManager Agent was started failed.\n");
        return MP_FAILED;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "start nginx success");
    printf("Process nginx of OceanStor BCManager Agent was started successfully.\n");
    return MP_SUCCESS;
}



    
