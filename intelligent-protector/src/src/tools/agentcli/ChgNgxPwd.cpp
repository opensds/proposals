/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/Password.h"
#include "common/Path.h"
#include "tools/agentcli/ChgNgxPwd.h"

/*------------------------------------------------------------ 
Description  : 处理函数，无输入
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CChgNgxPwd::Handle()
{
    //校验当前管理员旧密码
    //从配置文件读取用户名
    mp_string strUsrName, strCrtFile, strCrtKeyFile, strKeyPwd;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }
    mp_string strOldPwd;
    mp_uint32 iInputFailedTimes = 0;
    while (iInputFailedTimes <= MAX_FAILED_COUNT)
    {
        CPassword::InputUserPwd(strUsrName, strOldPwd, INPUT_GET_ADMIN_OLD_PWD);
        if (CPassword::CheckAdminOldPwd(strOldPwd))
        {
            break;
        }
        else
        {
            iInputFailedTimes++;
            continue;
        }
    }

    strOldPwd.replace(0, strOldPwd.length(), "");
    if (iInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("%s.\n", OPERATION_LOCKED_HINT);
        CPassword::LockAdmin();
        return MP_FAILED;
    }

    // 输入证书名称和证书key名称
    iRet = InputNginxInfo(strCrtFile, strCrtKeyFile, strKeyPwd);
    if (MP_SUCCESS != iRet)
    {
        return iRet;
    }

    //修改证书名称和证书key名称
    iRet = ChgNginxInfo(strCrtFile, strCrtKeyFile, strKeyPwd);
    if (MP_SUCCESS != iRet)
    {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
    }
    else
    {
        printf("%s\n", OPERATION_PROCESS_SUCCESS_HINT);
        printf("Please restart agent to enable the new password.\n");
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Nginx information of certificate is modified successfully.");
    }

    return iRet;
}

mp_int32 CChgNgxPwd::InputNginxInfo(mp_string &strCertificate, mp_string &strKeyFile, mp_string &strNewPwd)
{
    mp_uint32 iInputFailedTimes = 0;
    mp_string strEncrpytPwd, strInputPwd, strNginxFileFullPath;
    while (iInputFailedTimes <= MAX_FAILED_COUNT)
    {
        printf("%s\n", SET_NGINX_SSL_CRT_HINT);
        //Nginx证书名称长度不做限制
        CPassword::GetInput(SET_NGINX_SSL_CRT_HINT, strCertificate, MAX_PATH_LEN);
        strNginxFileFullPath = CPath::GetInstance().GetNginxConfFilePath(strCertificate);
        //判断证书文件是否存在
        if (!CMpFile::FileExist(strNginxFileFullPath.c_str()))
        {
            printf("%s\n", OPERATION_INPUT_CRT_FAIL_HINT);
            ++iInputFailedTimes;
            continue;
        }
        else
        {
            break;
        }
    }
    
    if (iInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
        return MP_FAILED;
    }

    iInputFailedTimes = 0;
    while (iInputFailedTimes <= MAX_FAILED_COUNT)
    {
        printf("%s\n", SET_NGINX_SSL_CRT_KEY_HINT);
        //Nginx证书名称长度不做限制
        CPassword::GetInput(SET_NGINX_SSL_CRT_KEY_HINT, strKeyFile, MAX_PATH_LEN);
        strNginxFileFullPath = CPath::GetInstance().GetNginxConfFilePath(strKeyFile);
        //判断证书文件是否存在
        if (!CMpFile::FileExist(strNginxFileFullPath.c_str()))
        {
            printf("%s\n", OPERATION_INPUT_CRT_KEY_FAIL_HINT);
            ++iInputFailedTimes;
            continue;
        }
        else
        {
            break;
        }
    }
    
    if (iInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
        return MP_FAILED;
    }

    //输入新密码
    mp_int32 iRet = CPassword::ChgPwdNoCheck(strInputPwd);
    if (MP_SUCCESS != iRet)
    {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
        return iRet;
    }
    
    EncryptStr(strInputPwd, strNewPwd);
    return MP_SUCCESS;
}

mp_int32 CChgNgxPwd::ChgNginxInfo(mp_string strCertificate, mp_string strKeyFile, mp_string strNewPwd)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile.c_str()))
    {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        return MP_FAILED;
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    iRet = CMpFile::ReadFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet || vecResult.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read nginx config file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecResult.size());
        return MP_FAILED;
    }

    mp_string strTmp;
    mp_uchar iFlgReplace = 0;
    mp_string::size_type iPosCrtFile, iPosCrtKeyFile;
    for (mp_uint32 i = 0; i < vecResult.size(); i++)
    {
        strTmp = vecResult[i];
        iPosCrtFile = strTmp.find(NGINX_SSL_CRT_FILE, 0);
        if (iPosCrtFile != mp_string::npos)
        {
            // NGINX_SSL_CRT_FILE最后空格，去掉1
            iPosCrtFile += strlen(NGINX_SSL_CRT_FILE) - 1;
            mp_string strInsert = "   " + strCertificate + STR_SEMICOLON;
            vecResult[i].replace(iPosCrtFile, strTmp.length() - iPosCrtFile, strInsert);
            iFlgReplace |= 0x01;
        }

        iPosCrtKeyFile = strTmp.find(NGINX_SSL_CRT_KEY_FILE, 0);
        if (iPosCrtKeyFile != mp_string::npos)
        {
            // NGINX_SSL_CRT_FILE最后空格，去掉1
            iPosCrtKeyFile += strlen(NGINX_SSL_CRT_KEY_FILE) - 1;
            mp_string strInsert = "   " + strKeyFile + STR_SEMICOLON;
            vecResult[i].replace(iPosCrtKeyFile, strTmp.length() - iPosCrtKeyFile, strInsert);
            iFlgReplace |= 0x02;
        }

    }

    // 如果能找到iFlgReplace为7，否则找不到需要替换的全部数组则不能替换
    if (0x03 != iFlgReplace)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Replace nginx config file failed, iFlgReplace=%x.", iFlgReplace);
        return MP_FAILED;
    }

    iRet = CIPCFile::WriteFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write nginx config file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecResult.size());
        return MP_FAILED;
    }
    
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD,strNewPwd);
    if ( MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "set value of ssl_key_password failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

