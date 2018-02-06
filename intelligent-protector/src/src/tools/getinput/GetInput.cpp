/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/Password.h"
#include "common/CryptAlg.h"
#include <sstream>

#define USER_PASSWORD_KEY  "userpwd"


/*------------------------------------------------------------
Function Name: CheckPasswdFromFile
Description  : 推送安装，从文件中读取密码,并校验
Return       : MP_SUCCESS 成功
                   MP_FAILED 失败
Call         :
Called by    :
Modification :
Others       :
------------------------------------------------------------*/
mp_int32 CheckPasswdFromFile(mp_string& strPwd, mp_string strPwdConf)
{
    vector<mp_string> vecOutput;
    vector<mp_string>::iterator iter;
    mp_string::size_type spos = 0;
    
    mp_int32 iRet = CMpFile::ReadFile(strPwdConf, vecOutput);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read info from the conf file failed, Ret is %d.", MP_FAILED);

        return MP_FAILED;
    }
	//CodeDex误报,KLOCWORK.NPD.FUNC.MUST
    for (iter = vecOutput.begin(); iter != vecOutput.end(); ++iter)
    {
        spos = (*iter).find('=');
        if (mp_string::npos != spos)
        {
            mp_string strKey = CMpString::Trim((mp_char*)((*iter).substr(0, spos)).c_str());
            if (USER_PASSWORD_KEY == strKey)
            {
                strPwd = CMpString::Trim((mp_char*)((*iter).substr(spos + 1)).c_str());
                break;
            }
            
        }
    }

    mp_bool bRet = CPassword::CheckCommon(strPwd);
    if (!bRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check passwd rules failed, Ret is %d.", MP_FAILED);

        return MP_FAILED;
    }

    return MP_SUCCESS;
}


/*------------------------------------------------------------
Function Name: main
Description  : getinput进程主函数
Return       :
Call         :
Called by    :
Modification :
Others       :
-------------------------------------------------------------*/
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_string strPwd, strPwdConf;
    //初始化agentcli路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (MP_SUCCESS != iRet)
    {
        printf("Init getinput path failed.\n");
        return iRet;
    }

    //初始化配置文件模块
    iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (MP_SUCCESS != iRet)
    {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF);
        return iRet;
    }

    //初始化日志模块
    CLogger::GetInstance().Init(GET_INPUT_LOG_NAME, CPath::GetInstance().GetLogPath());

    //校验密码
    if (2 == argc)
    {
        strPwdConf = argv[1];
        
        iRet = CheckPasswdFromFile(strPwd, strPwdConf);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check passwd from conf failed, Ret is %d.", MP_FAILED);
            
            return MP_FAILED;
        }
    }
    else if (1 == argc)
    {
        iRet = CPassword::ChgPwd(PASSWORD_INPUT, strPwd);
        if (MP_SUCCESS != iRet)
        {
            printf("Set password failed.\n");
            return iRet;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is error, Ret is %d.", MP_FAILED);

        return MP_FAILED;
    }

    //加密
    mp_string strSalt;
    mp_uint64 randNum;

    iRet = GetRandom(randNum);
    if (iRet != MP_SUCCESS)
    {
        printf("Get Random number failed.");
        return MP_FAILED;
    }
    
    ostringstream oss;
    oss<<randNum;
    strSalt = oss.str();

    //获取hash值
    //mp_char outHashHex[SHA256_BLOCK_SIZE + 1] = {0};
    //mp_string strInput = strPwd + strSalt;
    //iRet = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, sizeof(outHashHex));
    //if (iRet != MP_SUCCESS)
    //{
    //   printf("Get sha256 hash value failed.\n");
    //    return MP_FAILED;
    //}
    //strPwd.clear();
    //strInput.clear();

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
        return MP_FAILED;
    }
    
    vector<mp_string> vecInput;
    vecInput.push_back(strOut);
    mp_string strEnFilePath = CPath::GetInstance().GetTmpFilePath(EN_TMP_FILE);
    //写入临时文件
    iRet = CIPCFile::WriteFile(strEnFilePath, vecInput);
    if (iRet != MP_SUCCESS)
    {
        printf("Write file failed.\n");
    }
    return iRet;
}


