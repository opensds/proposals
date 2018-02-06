/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/ConfigXmlParse.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/CryptAlg.h"
#include "alarm/Db.h"
#include <sstream>

#define CMD_ARG_COUNT 2
#define CREATE_TABLE "table"
#define REENCRYPT_RECORD "record"
#define REENCRYPT_PASS "pass"
#define DECRYPT_SALT "salt"
#define NGINX_PASSWORD "ssl_certificate_key_password"
#define CREATE_APP_STATUS_TABLE "CREATE TABLE IF NOT EXISTS [AppStatusTable] ([Key] VARCHAR(100));"
#define CRETAE_FREEZE_OBJ_TABLE "CREATE TABLE IF NOT EXISTS [FreezeObjTable] ([InstanceName] VARCHAR(1024), \
    [DBName] VARCHAR(64), [BeginStatus] INTEGER(4), [LoopTime] INTEGER(4), [User] VARCHAR(1024), \
    [MP] VARCHAR(1024), [JsonData] VARCHAR(1024), [AppType] INTEGER(4), [BeginTime] INT64(8));"

typedef struct stFreezeObjRecord
{
    mp_string strInstanceName;
    mp_string strDBName;
    mp_uint32 uiStatus;
    mp_uint32 uiLoopTime;
    mp_string strUser;
    mp_string strPass;
    mp_string strJsonData;
    mp_int32 iAppType;
    mp_uint64 ulBeginTime;
}FreezeObjRecord;

/*------------------------------------------------------------ 
Description  : 显示帮助
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
void PrintHelp()
{
    printf("usage:\n");
    printf("[path]datamigration %s\n", CREATE_TABLE);
    printf("[path]datamigration %s\n", REENCRYPT_RECORD);
    printf("[path]datamigration %s\n", REENCRYPT_PASS);
    printf("[path]datamigration %s\n", DECRYPT_SALT);
}

/*------------------------------------------------------------ 
Description  : 创建表
Input        : strSql -- 建表语句
Output       : 
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败,返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CreateTable(mp_string strSql)
{
    LOGGUARD("");
    CDB &db = CDB::GetInstance();
    DbParamStream dps;
    mp_int32 iRet = db.ExecSql(strSql, dps);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "Create table failed, iRet = %d.", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Create table succeeded.");
    }
    return iRet;
}

/*------------------------------------------------------------ 
Description  : 对数据库记录重新加密
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 ReEncryptRecords()
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    ostringstream buff;
    CDB &db = CDB::GetInstance();

    buff << "select " << InstanceName << "," << DBName << "," << BeginStatus << "," << LoopTime "," 
         << User << "," << MP << "," << JsonData << "," << AppType << "," << BeginTime << " from " << FreezeObjTable;

    DbParamStream dps;
    iRet = db.QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query from table %s failed, iRet = %d.", FreezeObjTable, iRet);
        return iRet;
    }

    list<FreezeObjRecord> lstRecord;
    FreezeObjRecord freezeObj;
    for (mp_int32 iRow = 0; iRow < iRowCount; iRow++)
    {
        mp_string strInstanceName, strDBName, strStatus, strLoopTime;
        mp_string strUser, strPass, strJsonData, strAppType, strBeginTime;
        readBuff >> strInstanceName;
        readBuff >> strDBName;
        readBuff >> strStatus;
        readBuff >> strLoopTime;
        readBuff >> strUser;
        readBuff >> strPass;
        readBuff >> strJsonData;
        readBuff >> strAppType;
        readBuff >> strBeginTime;

        mp_string strUserTmp, strPassTmp;
        mp_string strUserEnc, strPassEnc;
        DecryptStr(strUser, strUserTmp);
        DecryptStr(strPass, strPassTmp);
        if (strUserTmp.empty() || strPassTmp.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Decrypt failed.");
            return MP_FAILED;
        }
        
        EncryptStr(strUserTmp, strUserEnc);
        EncryptStr(strPassTmp, strPassEnc);
        strPassTmp.replace(0, strPassTmp.length(), "");
        if (strUserEnc.empty() || strPassEnc.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Encrypt failed.");
            return MP_FAILED;
        }
        
        freezeObj.strInstanceName = strInstanceName;
        freezeObj.strDBName = strDBName;
        freezeObj.uiStatus = atoi(strStatus.c_str());
        freezeObj.uiLoopTime = atoi(strLoopTime.c_str());
        freezeObj.strUser = strUserEnc;
        freezeObj.strPass = strPassEnc;
        freezeObj.strJsonData = strJsonData;
        freezeObj.iAppType = atoi(strAppType.c_str());
        freezeObj.ulBeginTime = atol(strBeginTime.c_str());

        lstRecord.push_back(freezeObj);
    }

    buff.str("");
    dps.Clear();
    buff << "delete from " << FreezeObjTable;
    iRet = db.ExecSql(buff.str(), dps);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete records from %s failed, iRet = %d.", FreezeObjTable, iRet);
        return iRet;
    }

    buff.str("");
    buff << "insert into " << FreezeObjTable << "(" << InstanceName << "," << DBName << "," << BeginStatus 
        << "," << LoopTime << "," << User << "," << MP << "," << JsonData << "," << AppType << "," 
        << BeginTime << ") values(?, ?, ?, ?, ?, ?, ?, ?, ?);";
    for (list<FreezeObjRecord>::iterator iter = lstRecord.begin(); iter != lstRecord.end(); ++iter)
    {
        dps.Clear();
        DbParam dp = iter->strInstanceName;
        dps << dp;
        dp = iter->strDBName;
        dps << dp;
        dp = iter->uiStatus;
        dps << dp;
        dp = iter->uiLoopTime;
        dps << dp;
        dp = iter->strUser;
        dps << dp;
        dp = iter->strPass;
        dps << dp;
        dp = iter->strJsonData;
        dps << dp;
        dp = iter->iAppType;
        dps << dp;
        dp = iter->ulBeginTime;
        dps << dp;

        iRet = db.ExecSql(buff.str(), dps);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Insert records into %s failed, iRet = %d.", FreezeObjTable, iRet);
            return iRet;
        }
    }

    lstRecord.clear();

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Re-encrypting sql records succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 重新加密nginx配置文件中密码
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 ReEncryptNginxPassword()
{
    mp_string strKey;
    mp_string strTmp;
    mp_int32 iRet;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, strKey);
    if ( MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get GetValueString of ssl_key_password failed");
        return MP_FAILED;
    }
    //CodeDex误报,KLOCWORK.NPD.FUNC.MUST
    DecryptStr(strKey, strTmp);
    if (strTmp.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Decrypt nginx password failed.");
        return MP_FAILED;
    }
    
    EncryptStr(strTmp, strKey);
    if (strKey.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Encrypt nginx password failed.");
        return MP_FAILED;
    }
    
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD,strKey);
    if ( MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "set value of ssl_key_password failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 对配置文件中密码重新加密
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 ReEncryptPasswords()
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strSnmpAuth, strSnmpPrivate;
    mp_string strSnmpAuthTmp, strSnmpPrivateTmp;

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_AUTH_PASSWORD, strSnmpAuth);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get %s from xml config file failed, ret = %d.", CFG_AUTH_PASSWORD, iRet);
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_PRIVATE_PASSWOD, strSnmpPrivate);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get %s from xml config file failed, ret = %d.", CFG_PRIVATE_PASSWOD, iRet);
        return iRet;
    }

    if(strSnmpAuth.empty() || strSnmpPrivate.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Password in xml config file is null.");
        return MP_FAILED;
    }
        
    DecryptStr(strSnmpAuth, strSnmpAuthTmp);
    DecryptStr(strSnmpPrivate, strSnmpPrivateTmp);
    if (strSnmpAuthTmp.empty() || strSnmpPrivateTmp.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Decrypt snmp password failed.");
        return MP_FAILED;
    }
    
    EncryptStr(strSnmpAuthTmp, strSnmpAuth);
    EncryptStr(strSnmpPrivateTmp, strSnmpPrivate);
    strSnmpAuthTmp.replace(0, strSnmpAuthTmp.length(), "");
    strSnmpPrivateTmp.replace(0, strSnmpPrivateTmp.length(), "");
    if (strSnmpAuth.empty() || strSnmpPrivate.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Encrypt snmp password failed.");
        return MP_FAILED;
    }
    
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_AUTH_PASSWORD, strSnmpAuth);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set %s value into xml config failed.", CFG_AUTH_PASSWORD);
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_PRIVATE_PASSWOD, strSnmpPrivate);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set %s value in xml config file failed.", CFG_PRIVATE_PASSWOD);
        return MP_FAILED;
    }

    iRet = ReEncryptNginxPassword();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Re-encrypt password in nginx config file failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Re-encrypt passwords succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 对配置文件中盐值解密
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 DecryptSalt()
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strSalt, strSaltPlain;

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get salt value from xml config file failed, ret = %d.", iRet);
        return iRet;
    }

    if(strSalt.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Salt value in xml config file is null.");
        return MP_FAILED;
    }

    DecryptStr(strSalt, strSaltPlain);
    if (strSaltPlain.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Decrypt salt value failed.");
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSaltPlain);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set salt value in xml config file failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Decrypt salt value succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 为了减低HandleCmd() 圈复杂度的一系列函数
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 HandleCmdCreateTable()
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin creating sql tables.");

    iRet = CreateTable(CREATE_APP_STATUS_TABLE);
    if(iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create sql table %s failed.", AppStatusTable);
        return iRet;
    }
    iRet = CreateTable(CRETAE_FREEZE_OBJ_TABLE);
    if(iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create sql table %s failed.", FreezeObjTable);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Finish creating sql tables.");
    return MP_SUCCESS;
}

mp_int32 HandleCmdReEncryptRecords()
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin re-encrypting sql records.");
    iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init crypt failed, iRet = %d", iRet);
        return iRet;
    }
    iRet = ReEncryptRecords();
    //程序即将退出，此处不判断返回值
    (mp_void)FinalizeCrypt();
    if(iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Re-encrypt sql records failed.");
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Finish re-encrypting sql records.");
    return MP_SUCCESS;
}

mp_int32 HandleCmdReEncryptPasswords()
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin re-encrypting passwords.");
    iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init crypt failed, iRet = %d", iRet);
        return iRet;
    }
    iRet = ReEncryptPasswords();
    //程序即将退出，此处不判断返回值
    (mp_void)FinalizeCrypt();
    if(iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Re-encrypt passwords failed.");
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Finish re-encrypting passwords.");
    return MP_SUCCESS;
}

mp_int32 HandleCmdDecryptSalt()
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin decrypting salt value.");
    iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init crypt failed, iRet = %d", iRet);
        return iRet;
    }
    iRet = DecryptSalt();
    //程序即将退出，此处不判断返回值
    (mp_void)FinalizeCrypt();
    if(iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Decrypt salt value failed.");
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Finish decrypting salt value.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : 根据输入参数分别进行处理
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 HandleCmd(mp_string strCmd)
{
    if (strcmp(strCmd.c_str(), CREATE_TABLE) == 0)
    {
        return HandleCmdCreateTable();
    }
    else if  (strcmp(strCmd.c_str(), REENCRYPT_RECORD) == 0)
    {
        return HandleCmdReEncryptRecords();
    }
    else if (strcmp(strCmd.c_str(), REENCRYPT_PASS) == 0)
    {
        return HandleCmdReEncryptPasswords();
    }
    else if (strcmp(strCmd.c_str(), DECRYPT_SALT) == 0)
    {
        return HandleCmdDecryptSalt();
    }
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
    if (argc != CMD_ARG_COUNT || strcmp(argv[1], CREATE_TABLE) != 0 && strcmp(argv[1], REENCRYPT_RECORD) != 0 \
             && strcmp(argv[1], REENCRYPT_PASS) != 0 && strcmp(argv[1], DECRYPT_SALT) != 0)
    {
        PrintHelp();
        return MP_FAILED;
    }
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
    CLogger::GetInstance().Init(DATA_MIGRA_LOG_NAME, strLogFilePath);

    iRet = HandleCmd(argv[1]);
    return iRet;
}

