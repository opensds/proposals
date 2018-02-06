/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Path.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/ConfigXmlParse.h"

#define WRITE_CMD  "write"
#define READ_CMD   "read"

mp_int32 HandleCmd(mp_int32 argc, mp_char** argv)
{
    mp_string strCmd = argv[1];
    mp_string strSectionName = argv[2];
    mp_int32 iRet = MP_FAILED;
    if (0 == strcmp(strCmd.c_str(), WRITE_CMD))
    {
        if (argc == 5)
        {
            mp_string strKey = argv[3];
            mp_string strValue = argv[4];
            iRet = CConfigXmlParser::GetInstance().SetValue(strSectionName, strKey, strValue);
        }
        //参数个数6
        else
        {
            mp_string strChildSectionName = argv[3];
            mp_string strKey = argv[4];
            mp_string strValue = argv[5];
            iRet = CConfigXmlParser::GetInstance().SetValue(strSectionName, strChildSectionName, strKey, strValue);
        }
    }
    //读命令
    else
    {
        mp_string strValue;
        if (argc == 4)
        {
            mp_string strKey = argv[3];
            iRet = CConfigXmlParser::GetInstance().GetValueString(strSectionName, strKey, strValue);
        }
        //参数个数5
        else
        {
            mp_string strChildSectionName = argv[3];
            mp_string strKey = argv[4];
            iRet = CConfigXmlParser::GetInstance().GetValueString(strSectionName, strChildSectionName, strKey, strValue);
        }

        if (iRet == MP_SUCCESS)
        {
            printf("%s\n", strValue.c_str());
        }
    }

    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "XmlCfg failed, iRet = %d.", iRet);
    }
    return iRet;
}

/*------------------------------------------------------------ 
Description  : xmlcfg主函数
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    if ((argc !=  4 && argc !=  5 && argc !=  6) ||
        (0 != strcmp(argv[1], READ_CMD) && 0 != strcmp(argv[1], WRITE_CMD)))
    {
        printf("Usage: [path]xmlcfg write|read section [child_section] key [value]\n");
        return MP_FAILED;
    }

    if ((0 == strcmp(argv[1], READ_CMD) && argc ==  6) ||
        (0 == strcmp(argv[1], WRITE_CMD) && argc ==  4))
    {
        printf("Usage: [path]xmlcfg write|read section [child_section] key [value]\n");
        return MP_FAILED;
    }

    //初始化XmlCfg路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (MP_SUCCESS != iRet)
    {
        printf("Init xmlcfg path failed.\n");
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
    CLogger::GetInstance().Init(XML_CFG_LOG_NAME, CPath::GetInstance().GetLogPath());

    return HandleCmd(argc, argv);
}

