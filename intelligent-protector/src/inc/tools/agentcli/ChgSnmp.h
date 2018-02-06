/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENTCLI_CHGPWD_H_
#define _AGENTCLI_CHGPWD_H_

#include "common/Types.h"

#ifdef WIN32
#define GETCHAR _getch()
#else
#define GETCHAR CMpString::GetCh()
#endif

#define CHOOSE_OPERATION_HINT "Please choose operation"
#define SET_AUTH_PASSWD_HINT "Change authentication password"
#define SET_PRI_PASSWD_HINT  "Change private password"
#define SET_AUTH_PROTOCOL_HINT "Change authentication protocol"
#define SET_PRI_PROTOCOL_HINT "Change private protocol"
#define SET_SECURITY_NAME_HINT "Change security name"
#define SET_SECURITY_LEVEL_HINT "Change security Level"
#define SET_SECURITY_MODEL_HINT "Change security model"
#define SET_CONTEXT_ENGID_HINT "Change context engine ID"
#define SET_CONTEXT_NAME_HINT "Change context name"
#define QUIT_HINT "Other: Quit"
#define CHOOSE_HINT "Please choose:"
#define SECURITY_NAME_HINT "Enter security name:"
#define CONTEXT_EN_ID_HINT  "Enter context engine ID:"
#define CONTEXT_NAME_HINT "Enter context name:"

#define NONE "NONE"
#define MD5 "MD5"
#define SHA1 "SHA1"
#define SHA2 "SHA2"
#define DES "DES"
#define AES128 "AES128"
#define ANY "ANY"
#define V1 "V1"
#define V2 "V2"
#define USM "USM"
#define NOAUTH_NOPRIV "No authentication and no private"
#define AUTH_NOPRIV "Authentication and no private"
#define AUTH_PRIV "Authentication and private"

#define PRIVATE_PROTOCOL_NOT_SUPPORTED "The private protocol you entered is not supported."
#define PRIVATE_PROTOCOL_NOT_SAFE "The private protocol you entered is not safe, AES128 is recommended."

#define AUTH_PROTOCOL_NOT_SUPPORTED "The authentication protocol you entered is not supported."
#define AUTH_PROTOCOL_NOT_SAFE "The authentication protocol you entered is not safe, SHA2 is recommended."

#define SECURITY_LEVEL_NOT_SUPPORTED "The security level you entered is not supported."
#define SECURITY_LEVEL_NOT_SAFE "The security level you entered is not safe, authentication and private is recommended."

#define SECURITY_MODEL_NOT_SUPPORTED "The security model you entered is not supported."
#define SECURITY_MODEL_NOT_SAFE "The security model you entered is not safe, USM is recommended."

#define INPUT_TOO_LONG_HINT "The value you entered is too long, it must be less than 64 characters."

#define MAX_SNMP_PARAM_LEN 64

typedef enum
{
    SNMP_CHOOSE_SET_AUTH_PASSWD = 1,
    SNMP_CHOOSE_SET_PRI_PASSWD,
    SNMP_CHOOSE_SET_AUTH_PROTOCOL,
    SNMP_CHOOSE_SET_PRI_PROTOCOL,
    SNMP_CHOOSE_SET_SECURITY_NAME,
    SNMP_CHOOSE_SET_SECURITY_LEVEL,
    SNMP_CHOOSE_SET_SECURITY_MODEL,
    SNMP_CHOOSE_SET_CONTEXT_ENGID,
    SNMP_CHOOSE_SET_CONTEXT_NAME,
    SNMP_CHOOSE_SET_BUTT
}SNMP_CHOOSE_TYPE;

typedef enum
{
    AUTH_PROTOCOL_NONE = 1,
    AUTH_PROTOCOL_MD5,
    AUTH_PROTOCOL_SHA1,
    AUTH_PROTOCOL_SHA2 = 5
}AUTH_PROTOCOL_TYPE;

typedef enum
{
    PRIVATE_PROTOCOL_NONE = 1,
    PRIVATE_PROTOCOL_DES,
    PRIVATE_PROTOCOL_AES128 = 4
}PRIVATE_PROTOCOL_TYPE;

typedef enum
{
    SECURITY_LEVEL_NOAUTH_NOPRIV = 1,
    SECURITY_LEVEL_NOPRI,
    SECURITY_LEVEL_AUTH_PRIV    
}SECURITY_LEVEL;

typedef enum
{
    SECURITY_MODEL_ANY = 0,
    SECURITY_MODEL_V1,
    SECURITY_MODEL_V2,
    SECURITY_MODEL_USM
}SECURITY_MODEL;

class CChgSnmp
{
public:
    static mp_int32 Handle();

private:
    static SNMP_CHOOSE_TYPE GetChoice();
    static mp_int32 ChgAuthProtocol();
    static mp_int32 ChgPrivateProtocol();
    static mp_int32 ChgSecurityName();
    static mp_int32 ChgSecurityLevel();
    static mp_int32 ChgSecurityModel();
    static mp_int32 ChgContextEngineID();
    static mp_int32 ChgContextName();  
    static mp_bool CheckPasswordOverlap(const mp_string& strPasswd);
    static mp_int32 HandleInner();
    static mp_int32 HandleInner2(SNMP_CHOOSE_TYPE eChooseType);
    static mp_bool CheckSNMPPwd(PASSWOD_TYPE eType);
};



#endif
