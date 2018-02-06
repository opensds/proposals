/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENTCLI_CHAGNEIP_H_
#define _AGENTCLI_CHAGNEIP_H_

#include "common/Types.h"

#include <vector>


#define RESTART_WEB_SERVICE_HINT "Restart the web service to reload the configuration file."
#define INPUT_IP_HINT "Input new IP address:"
#define INVALID_IP_ADDRESS "Invalid IP address."
#define NOT_LOCAL_IP_ADDRESS "Input IP address is not local."
#define SAME_IP_ADDRESS "Input IP address is as same as current IP address."
#define CHANGE_IP_SUCCESS "Ip address of Nginx changed successfully, and it will be taken effect after Agent was started."

#define BIND_IP_TAG "listen"
#define ANY_IP      "0.0.0.0"
#define LOCAL_IP_ADDRESS "127.0.0.1"
#define NUM_STRING "0123456789"
#define EIGHT_BALNK "        "
#define MAX_IP_COUNT 200

class CChangeIP
{
public:
    static mp_int32 Handle();

private:
    static mp_int32 GetIPAddress(mp_string &strIP);
    static mp_int32 SetIPAddress(mp_string strIP);
    static mp_int32 GetLocalIPs(vector<mp_string>& vecIPs);
    static mp_bool IsLocalIP(mp_string strIP);
    static mp_int32 RestartNginx();
};

#endif

