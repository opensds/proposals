/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Password.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Path.h"
#include <sstream>
#include <algorithm> 
/*------------------------------------------------------------ 
Description  :修改密码
Input        :      eType---密码类型
Output       :      
Return       : MP_SUCCESS---修改成功
                  MP_FAILED---修改失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ChgPwd(PASSWOD_TYPE eType)
{
    mp_uint32 uiInputFailedTimes = 0;
    //从配置文件读取用户名
    mp_string strUsrName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    //输入新密码
    uiInputFailedTimes = 0;
    mp_string strNewPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(eType, strNewPwd))
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    //重复输入新密码
    uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (strConfirmedPwd == strNewPwd)
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            if(uiInputFailedTimes <= MAX_FAILED_COUNT) 
            {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    //保存密码
    if (!SaveOtherPwd(eType, strNewPwd))
    {
        printf("Save password failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Save password failed.");
        return MP_FAILED;
    }

    strNewPwd.replace(0, strNewPwd.length(), "");
    strConfirmedPwd.replace(0, strConfirmedPwd.length(), "");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :修改密码
Input        :      eType---密码类型
Output       :     strPwd---密码
Return       : MP_SUCCESS---修改成功
                  MP_FAILED---修改失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ChgPwd(PASSWOD_TYPE eType, mp_string& strPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    //从配置文件读取用户名
    mp_string strUsrName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    //输入新密码
    uiInputFailedTimes = 0;
    mp_string strNewPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(eType, strNewPwd))
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    //重复输入新密码
    uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (strConfirmedPwd == strNewPwd)
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            if(uiInputFailedTimes <= MAX_FAILED_COUNT)
            {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    strPwd = strNewPwd;
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :修改密码不做长度和密码复杂度验证
Output       :     strPwd---密码
Return       : MP_SUCCESS---修改成功
                  MP_FAILED---修改失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ChgPwdNoCheck(mp_string& strPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    mp_string strUsrName;
    mp_string strNewPwd;

    //输入新密码
    InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT, -1);

    //重复输入新密码
    uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD, -1);
        if (strConfirmedPwd == strNewPwd)
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            if(uiInputFailedTimes <= MAX_FAILED_COUNT)
            {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    strPwd = strNewPwd;
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :修改admin密码
Input        :       
Output       :      
Return       : MP_SUCCESS---修改成功
                  MP_FAILED---修改失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ChgAdminPwd()
{
    mp_int32 iRet = MP_SUCCESS;
    //从配置文件读取用户名
    mp_string strUsrName;
    mp_string strNewPwd;
    
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    //校验当前管理员旧密码
    iRet = VerifyOldUserPwd(strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("%s\n", OPERATION_LOCKED_HINT);
        CPassword::LockAdmin();
        return MP_FAILED;
    }

    //输入新密码
    iRet = InputNewUserPwd(strUsrName, strNewPwd);
    if (MP_SUCCESS != iRet)
    {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }
    
    //校验新密码
    iRet = ConfirmNewUserPwd(strUsrName, strNewPwd);
    if (MP_SUCCESS != iRet)
    {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }
    
    //保存密码
    if (!SaveAdminPwd(strNewPwd))
    {
        printf("Save password failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Save password failed.");
        return MP_FAILED;
    }

    strNewPwd.replace(0, strNewPwd.length(), "");
    printf("Password of %s is modified successfully.\n", strUsrName.c_str());
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Password of %s is modified successfully.", strUsrName.c_str());
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :判定旧密码
Input        :       strUserName---用户名
Output       :      
Return       : MP_SUCCESS---密码匹配成功
                  MP_FAILED---密码不匹配
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::VerifyOldUserPwd(mp_string& strUserName)
{
    mp_string strOldPwd;
    mp_uint32 uiInputFailedTimes = 0;
    
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUserName, strOldPwd, INPUT_GET_ADMIN_OLD_PWD);
        if (CheckAdminOldPwd(strOldPwd))
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            continue;
        }
    }

    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        return MP_FAILED;
    }

    strOldPwd.replace(0, strOldPwd.length(), "");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :输入 用户新密码
Input        :       strUserName---用户名，strNewPwd---新密码
Output       :      
Return       : MP_SUCCESS---输入成功
                  MP_FAILED---输入失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::InputNewUserPwd(mp_string& strUserName, mp_string& strNewPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUserName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(PASSWORD_ADMIN, strNewPwd))
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            continue;
        }
    }
    
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
         return MP_FAILED;
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :确认 用户新密码
Input        : strUserName---用户名，strNewPwd---新密码
Output       :      
Return       : MP_SUCCESS---输入成功
                  MP_FAILED---输入失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ConfirmNewUserPwd(mp_string& strUserName, mp_string& strNewPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUserName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (strConfirmedPwd == strNewPwd)
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            if(uiInputFailedTimes <= MAX_FAILED_COUNT)
            {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        return MP_FAILED;
    }

    strConfirmedPwd.replace(0, strConfirmedPwd.length(), "");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :输入用户 密码
Input        : strUserName---用户名，strNewPwd--- 密码，eType---密码类型
Output       :      
Return       :  
                   
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CPassword::InputUserPwd(mp_string strUserName, mp_string &strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen)
{
    mp_uint32 uiIndex;
    char chTmpPwd[PWD_LENGTH] = {0};
    switch (eType)
    {
    case INPUT_GET_ADMIN_OLD_PWD:
        printf("%s of %s:", INPUT_OLD_PASSWORD, strUserName.c_str());
        uiIndex = mp_string(INPUT_OLD_PASSWORD + strUserName).length() + mp_string(" of ").length();
        break;
    case INPUT_CONFIRM_NEW_PWD:
        printf(CONFIRM_PASSWORD);
        uiIndex = mp_string(CONFIRM_PASSWORD).length();
        break;
    case INPUT_SNMP_OLD_PWD:
        printf(INPUT_SNMP_OLD_PASSWORD);
        uiIndex = mp_string(INPUT_SNMP_OLD_PASSWORD).length();
        break;
    default:
        printf(INPUT_NEW_PASSWORD);
        uiIndex = mp_string(INPUT_NEW_PASSWORD).length();
        break;
    }

    mp_uchar chPwd = (mp_uchar)GETCHAR;
    mp_uint32 uiLen = uiIndex;
    mp_bool bIsBackSpace=MP_TRUE;
    mp_bool bOutofLen=MP_TRUE;
    while ((chPwd & 0xff) != ENTER_SPACE && (chPwd & 0xff) != NEWLINE_SPACE)
    {
        bIsBackSpace = ( ((chPwd & 0xff) == BACK_SPACE) && (uiIndex == uiLen) );
        if ( bIsBackSpace )
        {
            chPwd = (mp_uchar)GETCHAR;
            continue;
        }

        if ((chPwd & 0xff) == BACK_SPACE)
        {
            printf("\b \b");
            uiIndex--;
        }
        else
        {
            printf(" ");
			//CodeDex误报CSEC_LOOP_ARRAY_CHECKING，数组下标不会越界
            chTmpPwd[uiIndex - uiLen] = chPwd;
            uiIndex++;
            
            bOutofLen = ( (iPwdLen > 0) && ((uiIndex - uiLen) > iPwdLen) ) ;
            if ( bOutofLen )
            {
                break;
            }
        }
        chPwd = (mp_uchar)GETCHAR;
    }
    chTmpPwd[uiIndex - uiLen] = '\0';
    printf("\n");

    strUserPwd = chTmpPwd;
    (mp_void)memset_s(chTmpPwd, PWD_LENGTH, 0, PWD_LENGTH);
}
/*------------------------------------------------------------ 
Description  :检查admin旧密码
Input        : strOldPwd---旧密码
Output       :      
Return       :  MP_TRUE---旧密码匹配
                MP_FALSE ---旧密码不匹配
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckAdminOldPwd(const mp_string& strOldPwd)
{
    //获取配置文件中保存的盐值
    mp_string strSalt;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if(MP_SUCCESS != iRet)
    {
        printf("Get salt value from xml configuration file failed.\n");
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get salt value from xml configuration file failed.");
        return MP_FALSE;
    }

    //使用sha256获取hash值，保持与老版本兼容
    mp_char outHashHex[SHA256_BLOCK_SIZE + 1] = {0};
    mp_string strInput = strOldPwd + strSalt;
    iRet = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, sizeof(outHashHex));
    strInput.replace(0, strInput.length(), "");
    if (iRet != MP_SUCCESS)
    {
        printf("Get sha256 hash value failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sha256 hash value failed.");
        return MP_FAILED;
    }

    //新版本均采用PBKDF2进行散列
    mp_string strOut;
    iRet = PBKDF2Hash(strOldPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
        return iRet;
    }

    //从配置文件中获取老密码的hash值
    mp_string strOldHash;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, strOldHash);
    if (MP_SUCCESS != iRet)
    {
        printf("Parse xml config failed, key is hash.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Parse xml config failed, key is hash.");
        return MP_FAILED;
    }

    return (strOldHash == mp_string(outHashHex) || strOldHash == strOut) ? MP_TRUE : MP_FALSE;
}

/*------------------------------------------------------------ 
Description  : 获取nginx的key
Input        : vecResult -- nginx配置文件内容
Output       : strKey -- key
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CPassword::GetNginxKey(mp_string & strKey, const vector<mp_string> & vecResult)
{
    mp_string strTmp;
    mp_string::size_type iPosSSLPwd = mp_string::npos;
    for (mp_uint32 i = 0; i < vecResult.size(); i++)
    {
        strTmp = vecResult[i];
        iPosSSLPwd = strTmp.find(NGINX_SSL_PWD, 0);
        if (iPosSSLPwd != mp_string::npos)
        {
            iPosSSLPwd += strlen(NGINX_SSL_PWD);
            mp_string::size_type iPosSemicolon = strTmp.find(CHAR_SEMICOLON, iPosSSLPwd);
            if (iPosSemicolon != mp_string::npos)
            {
                strKey = strTmp.substr(iPosSSLPwd, iPosSemicolon-iPosSSLPwd);
                (mp_void)CMpString::Trim((mp_char *)strKey.c_str());
            }
            break;
        }
    }

}

/*------------------------------------------------------------ 
Description  : 检查新输入的nginx的key是否和老的一样
Input        : strNewPwd -- 新输入的nginx的key
Output       : 
Return       : MP_TRUE -- 一样
               MP_FALSE -- 不一样
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckNginxOldPwd(const mp_string& strNewPwd)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile.c_str()))
    {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Nginx config file does not exist, path is \"%s\"", AGENT_NGINX_CONF_FILE);
        return MP_FALSE;
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    iRet = CMpFile::ReadFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet || vecResult.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read nginx config file failed, iRet = %d, size of vecResult is %d.",
                        iRet, vecResult.size());
        return MP_FALSE;
    }

    mp_string strOldPwd = "";
    GetNginxKey(strOldPwd, vecResult);
    mp_string strDecryptPwd = "";
    DecryptStr(strOldPwd, strDecryptPwd);
    iRet = (strDecryptPwd == strNewPwd ? MP_TRUE : MP_FALSE);
    strDecryptPwd.replace(0, strDecryptPwd.length(), "");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :检查admin旧密码
Input        : strOldPwd---旧密码
Output       :      
Return       :  MP_TRUE---旧密码匹配
                MP_FALSE ---旧密码不匹配
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckOtherOldPwd(PASSWOD_TYPE eType, const mp_string& strOldPwd)
{
    //如果是第一次初始化密码，不校验旧密码，返回成功
    if (eType == PASSWORD_INPUT)
    {
        return MP_SUCCESS;
    }

    if (eType == PASSWORD_NGINX_SSL)
    {
        return CheckNginxOldPwd(strOldPwd);
    }

    mp_string strCfgValue, strKeyName;
    mp_int32 iRet;
    if(eType == PASSWORD_SNMP_AUTH)
    {
        strKeyName = CFG_AUTH_PASSWORD;
    }
    else if(eType == PASSWORD_SNMP_PRIVATE)
    {
        strKeyName = CFG_PRIVATE_PASSWOD;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, strKeyName, strCfgValue);
    if (MP_SUCCESS != iRet)
    {
        printf("Get password value from xml config failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get password value from xml config failed.");
        return MP_FALSE;
    }

    mp_string strDecryptPwd = "";
    DecryptStr(strCfgValue, strDecryptPwd);
    iRet = (strDecryptPwd == strOldPwd ? MP_TRUE : MP_FALSE);
    strDecryptPwd.replace(0, strDecryptPwd.length(), "");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :检查新密码
Input        :  strNewPwd--- 密码，eType---密码类型
Output       :      
Return       :  bRet---新密码和旧密码相同
                   MP_FALSE---新密码和旧密码不同
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckNewPwd(PASSWOD_TYPE eType, const mp_string& strNewPwd)
{
    //检查是否是用户名反转
    mp_string strUserName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUserName);
    if(MP_SUCCESS != iRet)
    {
        printf("Get name value from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get name value from xml configuration file failed.");
        return MP_FALSE;
    }
    mp_string strReverseUserName = strUserName;
    std::reverse(strReverseUserName.begin(), strReverseUserName.end());
    if (strUserName == strNewPwd || strReverseUserName == strNewPwd)
    {
        printf("Can't use username or reversed username as password.\n");
        return MP_FALSE;
    }

    //检查是否和老密码相同
    mp_bool bRet = MP_FALSE;
    if (eType == PASSWORD_ADMIN)
    {
        bRet = CheckAdminOldPwd(strNewPwd);
    }
    else
    {
        bRet = CheckOtherOldPwd(eType, strNewPwd);
    }
    if (bRet)
    {
        printf("New password is equal to old password.\n");
        return MP_FALSE;
    }

    bRet = CheckCommon(strNewPwd);

    //检查是否循环校验，snmp漏洞
    if (bRet && ((eType == PASSWORD_SNMP_AUTH) || (eType == PASSWORD_SNMP_PRIVATE)))
    {
        if (CheckPasswordOverlap(strNewPwd))
        {
            return MP_FALSE;
        }
    }

    return bRet;
}

/*------------------------------------------------------------ 
Description  :保存admin密码
Input        :  strPwd--- 密码 
Output       :      
Return       :  MP_TRUE---保存成功
                   MP_FALSE---保存失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::SaveAdminPwd(const mp_string& strPwd)
{
    //获取盐值
    mp_string strSalt;
    mp_int32 iRet;
    mp_uint64 randNum;

    iRet = GetRandom(randNum);
    if (iRet != MP_SUCCESS)
    {
        printf("Get Random number failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Random number failed.");
        return MP_FALSE;
    }
    ostringstream oss;
    oss<<randNum;
    strSalt = oss.str();

    //获取hash值
    //mp_char outHashHex[SHA256_BLOCK_SIZE + 1] = {0};
    //mp_string strInput(strPwd);
    //strInput = strInput + strSalt;
    //iRet = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, sizeof(outHashHex));
    //strInput.clear();
    //if (iRet != MP_SUCCESS)
    //{
    //    printf("Get sha256 hash value failed.\n");
    //    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sha256 hash value failed.");
    //    return MP_FALSE;
    //}

    //获取PBKDF2散列值
    mp_string strOut;
    iRet = PBKDF2Hash(strPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
        return iRet;
    }

    //将盐值存入配置文件
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if(iRet != MP_SUCCESS)
    {
        printf("Set salt value failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set salt value failed.");
        return MP_FALSE;
    }
    
    //将重新计算的hash值存入配置文件
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, strOut);
    if (iRet != MP_SUCCESS)
    {
        printf("Set PBKDF2 hash value failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set SHA256 hash value failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :保存其他密码
Input        :  strPwd--- 密码 
Output       :      
Return       :  MP_TRUE---保存成功
                   MP_FALSE---保存失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::SaveOtherPwd(PASSWOD_TYPE eType, const mp_string& strPwd)
{
    mp_string strEncrpytPwd;
    EncryptStr(strPwd, strEncrpytPwd);
    
    mp_string strKeyName;
    if(eType == PASSWORD_NGINX_SSL)
    {
    	//CodeDex误报，Dead Code
        return SaveNginxPwd(strEncrpytPwd);
    }
    else if(eType == PASSWORD_SNMP_AUTH)
    {
        strKeyName = CFG_AUTH_PASSWORD;
    }
    else if(eType == PASSWORD_SNMP_PRIVATE)
    {
        strKeyName = CFG_PRIVATE_PASSWOD;
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, strKeyName, strEncrpytPwd);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml config failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml config failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}

/*------------------------------------------------------------ 
Description  : 保存新输入的nginx的key
Input        : strPwd -- 新输入的nginx的key
Output       : 
Return       : MP_TRUE -- 保存成功
               MP_FALSE -- 保存失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::SaveNginxPwd(const mp_string& strPwd)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile.c_str()))
    {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        return MP_FALSE;
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    iRet = CMpFile::ReadFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet || vecResult.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read nginx config file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecResult.size());
        return MP_FALSE;
    }

    mp_string strTmp;
    mp_string::size_type iPosSSLPwd;
    for (mp_uint32 i = 0; i < vecResult.size(); i++)
    {
        strTmp = vecResult[i];
        iPosSSLPwd = strTmp.find(NGINX_SSL_PWD, 0);
        if (iPosSSLPwd != mp_string::npos)
        {
            iPosSSLPwd += strlen(NGINX_SSL_PWD);
            mp_string strInsert = " " + strPwd + STR_SEMICOLON;
            vecResult[i].replace(iPosSSLPwd, strTmp.length()-iPosSSLPwd, strInsert);
            break;
        }
    }

    iRet = CIPCFile::WriteFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write nginx config file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecResult.size());
        return MP_FALSE;
    }

    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :计算密码复杂度
Input        :  strPwd--- 密码 
Output       : iNum---数字，iUppercase---大写字母，iLowcase---输入的小写字母，iSpecial---特殊字符
Return       :  MP_TRUE---保存成功
                   MP_FALSE---保存失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::CalComplexity(const mp_string& strPwd, mp_int32& iNum, mp_int32& iUppercase, mp_int32& iLowcase, mp_int32& iSpecial)
{
    for (mp_uint32 uindex = 0; uindex < strPwd.length(); uindex++)
    {
        if (string::npos == mp_string(PWD_REX).find(strPwd[uindex]))
        {
            printf("%s", WRONGPWD_HINT);
            return MP_FAILED;
        }
        if (strPwd[uindex] >= '0' && strPwd[uindex] <= '9')
            iNum++;
        if (strPwd[uindex] >= 'A' && strPwd[uindex] <= 'Z')
            iUppercase++;
        if (strPwd[uindex] >= 'a' && strPwd[uindex] <= 'z')
            iLowcase++;
        if(string::npos != mp_string(SPECIAL_REX).find(strPwd[uindex]))
        {
            iSpecial++;
        }
    }

    if (iSpecial == 0)
    {
        printf("%s", WRONGPWD_HINT);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :CheckCommon功能
Input        :  strPwd--- 密码 
Output       :  
Return       :  MP_TRUE---密码符合要求
                   MP_FALSE---密码简单
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckCommon(const mp_string& strPwd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iNum = 0;
    mp_int32 iUppercase = 0;
    mp_int32 iLowcase = 0;
    mp_int32 iSpecial = 0;
    
    //长度检查
    if (strPwd.length() < PWD_MIN_LEN || strPwd.length() > PWD_MAX_LEN)
    {
        printf("%s", WRONGPWD_HINT);
        return MP_FALSE;
    }

    iRet = CalComplexity(strPwd, iNum, iUppercase, iLowcase, iSpecial);
    if (MP_SUCCESS != iRet)
    {
        return MP_FALSE;
    }

    mp_int32 iComplex = CalculateComplexity(iNum, iUppercase, iLowcase);
    if (iComplex < 2)
    {
        printf("%s", WRONGPWD_HINT);
        return MP_FALSE;
    }

    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :计算复杂度
Input        :  strPwd--- 密码 
Output       :  iNum---数字，iUppercase---大写字母，iLowcase---输入的小写字母 
Return       :   iComplex---复杂度值
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::CalculateComplexity(mp_int32 iNumber, mp_int32 iUppercase, mp_int32 iLowcase)
{
    mp_int32 iComplex = 0;
    if (iNumber > 0)
        iComplex++;
    if (iUppercase > 0)
        iComplex++;
    if (iLowcase > 0)
        iComplex++;
    return iComplex;
}
/*------------------------------------------------------------ 
Description  :检查密码循环重叠
Input        :  strPasswd--- 密码 
Output       :      
Return       :  MP_TRUE--- 不设置密码
                   MP_FALSE--- 继续操作
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckPasswordOverlap(const mp_string& strPasswd)
{
    mp_uint32 uiIndex = 1;
    mp_uint32 uiMaxLen = strPasswd.length()/2;
    if (0 == uiMaxLen)
    {
        return MP_FALSE;
    }
    for (; uiIndex <= uiMaxLen; uiIndex++)
    {
        //如果此时长度不能被总长度整除，说明不会有循环模式，直接返回false
        mp_int32 iRemainNum = strPasswd.length()%uiIndex;
        if (0 != iRemainNum)
        {
            continue;
        }

        mp_string strMeta = strPasswd.substr(0, uiIndex);
        mp_uint32 subIndex = uiIndex; 
        mp_bool bFlag = MP_TRUE; // 全部循环标识符号
        for (; subIndex <= (strPasswd.length() - uiIndex); subIndex += uiIndex)
        {
            mp_string strTmp = strPasswd.substr(subIndex, uiIndex);
            if (strTmp != strMeta)
            {
                bFlag = MP_FALSE;
                break;
            }
        }

        if (bFlag)
        {
            //如果全部匹配，则说明是循环字符串
            //如果是循环的密码，则不符合要求(SNMP漏洞)
            printf("%s", PASSWORD_NOT_SAFE);
            printf("%s", CONTINUE);
            mp_int32 iChoice = getchar();
            if (iChoice != 'y' && iChoice != 'Y')
            {
                //如果不选择是，则用户认为这个密码是不想设置的，故需要返回是循环的
                return MP_TRUE;
            }
            //如果用户选择继续操作，则认为用户接受了该循环，故代码后续判断该密码符合要求
            return MP_FALSE;
        }
    }

    return MP_FALSE;
}

/*------------------------------------------------------------ 
Description  : 读入用户操作
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CPassword::GetInput(mp_string strHint, mp_string& strInput, mp_int32 iInputLen)
{
    mp_char chTmpInput[PWD_LENGTH] = {0};
    mp_uchar ch = (mp_uchar)GETCHAR;
    mp_uint32 iIndex = strHint.length();
    mp_uint32 iLen = iIndex;
    while ((ch & 0xff) != ENTER_SPACE && (ch & 0xff) != NEWLINE_SPACE)
    {
        if ( ((ch & 0xff) == BACK_SPACE) && (iIndex == iLen))
        {
            ch = (mp_uchar)GETCHAR;
            continue;
        }

        if ((ch & 0xff) == BACK_SPACE)
        {
            printf("\b \b");
            iIndex--;
        }
        else
        {
            // 支持不限制长度输入的情况
            if ((iInputLen > 0) && ((iIndex - iLen) == iInputLen))
            {
                ch = (mp_uchar)GETCHAR;
                continue;
            }
            //CodeDex误报，CSEC_LOOP_ARRAY_CHECKING，数组下标不会越界
            chTmpInput[iIndex-iLen] = ch;
            printf("%c", ch);
            iIndex++;
        }
        ch = (mp_uchar)GETCHAR;
    }
    chTmpInput[iIndex-iLen] = '\0';
    strInput = chTmpInput;
    printf("\n");
}

/*------------------------------------------------------------ 
Description  :  锁定用户
Input        :  
Output       :      
Return       :  
                
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CPassword::LockAdmin()
{
    mp_uint64 time = CMpTime::GetTimeSec();
    ostringstream oss;
    oss<<time;
    mp_string strTime = oss.str();
    vector<mp_string> vecInput;
    vecInput.push_back(strTime);
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    mp_int32 iRet = CIPCFile::WriteFile(strPath, vecInput);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "WriteFile failed, iRet = %d", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "agentcli is locked.");
    }
    return;
}

/*------------------------------------------------------------ 
Description  :  获取操作锁定时间
Input        :  无
Output       :      
Return       :  
                
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_uint64 CPassword::GetLockTime()
{
    //CodeDex误报，Type Mismatch:Signed to Unsigned
    vector<mp_string> vecOutput;
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    mp_int32 iRet = CMpFile::ReadFile(strPath, vecOutput);
    if (iRet != MP_SUCCESS || vecOutput.size() == 0)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "iRet = %d, vecOutput.size = %d", iRet, vecOutput.size());
        return 0;
    }

    return atol(vecOutput.front().c_str());
}

/*------------------------------------------------------------ 
Description  :  清除锁定信息
Input        :  
Output       :      
Return       :  
                
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CPassword::ClearLock()
{
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    if(CMpFile::FileExist(strPath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "agentcli is unlocked.");
    }
    mp_int32 iRet = CMpFile::DelFile(strPath.c_str());
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "DelFile failed, iRet = %d", iRet);
    }
    return;
}

