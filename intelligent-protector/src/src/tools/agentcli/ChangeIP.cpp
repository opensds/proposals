/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/ChangeIP.h"
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

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef LINUX
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#endif
/*------------------------------------------------------------ 
Description  : 修改Ngnix绑定IP地址
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CChangeIP::Handle()
{
#ifndef WIN32
    if(0 == getuid())
    {
        printf("%s", "Root can not change ip, please su to rdadmin.\n");
        return MP_FAILED;
    }
#endif
    mp_string strInput;   
    mp_uint32 iInputFailedTimes = 0;
    mp_int32 iRet = MP_FAILED;
    mp_bool bRet = MP_FALSE;
    while (iInputFailedTimes <= MAX_FAILED_COUNT)
    {
        printf("%s", INPUT_IP_HINT);
        CPassword::GetInput(INPUT_IP_HINT, strInput);
        //判断ip地址合法性
        bRet = (!CIPCheck::IsIPV4(strInput) && strInput != ANY_IP);
        if (bRet)
        {
            printf("%s\n", INVALID_IP_ADDRESS);
            iInputFailedTimes++;
            continue;
        } 
        //判断是否是本地ip地址
        if (!IsLocalIP(strInput))
        {
            printf("%s\n", NOT_LOCAL_IP_ADDRESS);
            iInputFailedTimes++;
            continue;
        }
        //判断ip地址是否是当前ip地址
        mp_string strCurrentIP;
        iRet = GetIPAddress(strCurrentIP);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetIPAddress failed, iRet = %d.", iRet);
            printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
            return iRet;
        }
        if (0 == strcmp(strCurrentIP.c_str(), strInput.c_str()))
        {
            printf("%s\n", SAME_IP_ADDRESS);
            iInputFailedTimes++;
            continue;
        }
        iRet = SetIPAddress(strInput);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SetIPAddress failed, iRet = %d.", iRet);
            printf("%s\n", SAME_IP_ADDRESS);
            return iRet;
        }
        else
        {
            printf("%s", CHANGE_IP_SUCCESS);
            printf("%s ", RESTART_WEB_SERVICE_HINT);
            printf("%s", CONTINUE);
            CPassword::GetInput(CONTINUE, strInput);
            mp_bool bRet2 = (strInput == "y" || strInput == "Y");
            if (bRet2)
            {
                //重启nginx
                iRet = RestartNginx();
                if (MP_SUCCESS != iRet)
                {
                    printf("Restart nginx failed.\n");
                    return iRet;
                }
                printf("Restart nginx successfully.\n");
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Change IP address binded by web service successfully.");
            }
            else
            {
                printf("Ip will take effect after nginx restart.\n");
               COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Change IP address successfully, but not binded Agent.");
            }
            return MP_SUCCESS;
        }
    }

    printf("Input wrong IP address over 3 times.\n");
    return MP_FAILED;
   
}

/*------------------------------------------------------------ 
Description  : 从配置文件中读取当前ip地址
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CChangeIP::GetIPAddress(mp_string &strIP)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile.c_str()))
    {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Nginx config file does not exist, path is \"%s\"", AGENT_NGINX_CONF_FILE);
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

    for (mp_uint32 i = 0; i < vecResult.size(); i++)
    {
        mp_string strTmp = vecResult[i];
        mp_string::size_type pos = strTmp.find(BIND_IP_TAG, 0);
        if (pos != mp_string::npos)
        {
            pos += strlen(BIND_IP_TAG);
            mp_string::size_type iColPos = strTmp.find(CHAR_COLON, pos);
            if (iColPos != mp_string::npos)
            {
                strIP = strTmp.substr(pos, iColPos - pos);
                (mp_void)CMpString::Trim((mp_char *)strIP.c_str());
            }
            else
            {
                strIP = ANY_IP;
            }
            break;
        }
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 将ip地址在配置文件对应位置进行修改
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CChangeIP::SetIPAddress(mp_string strIP)
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

    for (mp_uint32 i = 0; i < vecResult.size(); i++)
    {
        mp_string strTmp = vecResult[i];
        mp_string::size_type pos = strTmp.find(BIND_IP_TAG, 0);
        if (pos != mp_string::npos)
        {
            mp_string strLast;
            //先找:
            mp_string::size_type iColPos = strTmp.find(CHAR_COLON, pos + strlen(BIND_IP_TAG));
            if (iColPos != mp_string::npos)
            {
                strLast = strTmp.substr(iColPos + 1);
            }
            else
            {
                //再找数字
                mp_string::size_type iNumPos = strTmp.find_first_of(NUM_STRING);
                if (iNumPos == mp_string::npos || iNumPos <= strlen(BIND_IP_TAG))
                {
                    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Nginx config file is corrupt, strTmp = %s.", strTmp.c_str());
                    return MP_FAILED;
                }
                strLast = strTmp.substr(iNumPos);
            }
            vecResult[i] = mp_string(EIGHT_BALNK) + mp_string(BIND_IP_TAG) + " " + strIP + CHAR_COLON + strLast;
            break;
        }
    }

    iRet = CIPCFile::WriteFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write nginx config file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecResult.size());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 获取当前主机所有ip地址
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CChangeIP::GetLocalIPs(vector<mp_string>& vecIPs)
{
    mp_char szErr[MAX_ERROR_MSG_LEN] = {0};
    mp_int32 iErr;
	//CodeDex误报，UNINIT
#ifdef WIN32
    mp_char cHostName[MAX_HOSTNAME_LEN];
    WORD v = MAKEWORD(2, 0);
    WSADATA wsaData;
    if (WSAStartup(v, &wsaData) != MP_SUCCESS)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "WSAStartup failed, errno[%d]: %s", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    PHOSTENT hent;
    mp_int32 iRet = gethostname(cHostName, sizeof(cHostName));
    if (iRet != MP_SUCCESS)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "gethostname failed, errno[%d]: %s", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        WSACleanup();
        return MP_FAILED;
    }

    hent = gethostbyname(cHostName);
    for (mp_uint32 i = 0; hent && hent->h_addr_list[i]; i++)
    {
        mp_string strIP = inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i]));
        if (strIP != LOCAL_IP_ADDRESS)
        {
            vecIPs.push_back(strIP);
        }
    }
    WSACleanup();
#elif defined LINUX
    struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifa = NULL;
    mp_int32 family = AF_INET;
    mp_int32 iRet = MP_FAILED;
    mp_char host[NI_MAXHOST];
	    
    if(getifaddrs(&ifaddr) == -1)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "getifaddrs failed, errno[%d]: %s", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "ifa->ifa_addr == NULL");
            continue;
        }
        
        family = ifa->ifa_addr->sa_family;
        if(family != AF_INET) 
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "family != AF_INET");
            continue;
        }
        iRet = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if(iRet != 0)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "getnameinfo failed, errno %s", gai_strerror(iRet));
            continue;
        }
        if(strcmp(host, LOCAL_IP_ADDRESS) != 0)
        {
            vecIPs.push_back(host);
        }
    }
    freeifaddrs(ifaddr);
#else
#ifdef AIX
    mp_string strCMD = "ifconfig -a|grep \"inet \"|awk '{print $2}'";
#elif defined HP_UX_IA
	mp_string strCMD = "netstat -ni|grep -v 'Address'|awk '{print $4}'";
#elif defined SOLARIS
	mp_string strCMD = "netstat -ni | grep -v 'Address' | sed '$d' | nawk '{print $4}'";
#endif
    vector<mp_string> tmpIPs;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCMD, tmpIPs);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get all ip failed, iRet = %d and ipcount = %d", iRet, tmpIPs.size());
        return MP_FAILED;
    }
    for(vector<string>::iterator iter_tmp = tmpIPs.begin(); iter_tmp != tmpIPs.end(); iter_tmp++)
    {
        if(*iter_tmp != LOCAL_IP_ADDRESS)
        {
            vecIPs.push_back(*iter_tmp);
        }
    }
#endif
    vecIPs.push_back(ANY_IP);
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 判断当前ip是否是本地ip
Input        : 
Output       : 
Return       : MP_SUCCESS -- 成功 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_bool CChangeIP::IsLocalIP(mp_string strIP)
{
    vector<mp_string> vecIPs;
    mp_int32 iRet = GetLocalIPs(vecIPs);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetLocalIPs failed, iRet = %d", iRet);
        return MP_FALSE;
    }
    for (vector<mp_string>::iterator it = vecIPs.begin(); it != vecIPs.end(); it++)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "%s", it->c_str());
    }
    for (vector<mp_string>::iterator it = vecIPs.begin(); it != vecIPs.end(); it++)
    {
        if (strIP == *it)
        {
            return MP_TRUE;
        }
    }

    return MP_FALSE;
}

mp_int32 CChangeIP::RestartNginx()
{
#ifdef WIN32
    mp_string strCmd = mp_string("sc stop ") + NGINX_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = mp_string("sc start ") + NGINX_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    //校验脚本签名
    mp_int32 iRet = CheckScriptSign(STOP_SCRIPT);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CheckScriptSign failed, iRet = %d", iRet);
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    if(0 == getuid())
    {
         strCmd = "su - rdadmin -c \" " + strCmd + " \" ";
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "execute :%s", strCmd.c_str());
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    //校验脚本签名
    iRet = CheckScriptSign(START_SCRIPT);
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
