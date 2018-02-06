/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_PASSWORD_H__
#define __AGENT_PASSWORD_H__

#ifdef WIN32
#include <conio.h>
#endif

#include "common/Types.h"
#include "common/Defines.h"
#include <vector>

//特殊asii字符
#define BACK_SPACE           8   //退格
#define NEWLINE_SPACE        10  //换行
#define ENTER_SPACE          13  //回车
#define MAX_FAILED_COUNT     2
#define PWD_LENGTH           512
#define FAILED_SLEEP_TIME    5000
#define PWD_MAX_LEN          16
#define PWD_MIN_LEN          8

#define LOCK_ADMIN_FILE      "rdadmin"  
#define LOCK_MAX_TIME        15*60


#ifdef WIN32
#define GETCHAR _getch()
#else
#define GETCHAR CMpString::GetCh()
#endif

#define NGINX_SSL_PWD         "ssl_certificate_key_password"
#define INPUT_NEW_PASSWORD    "Enter new password:"
#define CONFIRM_PASSWORD      "Enter the new password again:"
#define INPUT_OLD_PASSWORD    "Enter password"
#define INPUT_SNMP_OLD_PASSWORD "Enter old password:"
#define WRONGPWD_HINT         "The password must contain 8 to 16 characters, at least two kinds of uppercase letters(A-Z),\
lowercase letters(a-z), digits(0-9), and special characters (`~!@#$%^&*()-_=+\\|[{}];:'\",<.>/?).\n"

//密码复杂度
#define USER_REX               "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"
#define PWD_REX                "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz`~!@#$%^&*()-_=+\\|[{}];:'\",<.>/?"
#define SPECIAL_REX            "`~!@#$%^&*()-_=+\\|[{}];:'\",<.>/?"
#define PASSWORD_NOT_SAFE      "The Password you entered is not safe."
#define CONTINUE               "Continue? (y/n):"

#define OPERATION_PROCESS_SUCCESS_HINT "Operation processed successfully."
#define OPERATION_PROCESS_FAIL_HINT "Operation processed failed."
#define OPERATION_LOCKED_HINT "Input invalid password over 3 times and agentcli will be locked, please try again after 15 minutes."
#define CHANGE_PASSWORD_NOT_MATCH "Password does not match, please input again."

typedef enum
{
    PASSWORD_ADMIN = 0,     //Agent admin
    PASSWORD_SNMP_AUTH,     //SNMP Auth
    PASSWORD_SNMP_PRIVATE,  //SNMP Private
    PASSWORD_NGINX_SSL,     //Nginx SSL Cert Key
    PASSWORD_INPUT          //用户第一次初始化密码
}PASSWOD_TYPE;

typedef enum
{
    INPUT_GET_ADMIN_OLD_PWD = 0,
    INPUT_CONFIRM_NEW_PWD,
    INPUT_SNMP_OLD_PWD,
    INPUT_DEFAULT
}INPUT_TYPE;

class AGENT_API CPassword
{
public:
    static mp_int32 ChgPwd(PASSWOD_TYPE eType);
    static mp_int32 ChgPwd(PASSWOD_TYPE eType, mp_string& strPwd);
	static mp_int32 ChgPwdNoCheck(mp_string& strPwd);
    static mp_int32 ChgAdminPwd();
    static mp_bool CheckAdminOldPwd(const mp_string& strOldPwd);
    static mp_bool CheckOtherOldPwd(PASSWOD_TYPE eType, const mp_string& strOldPwd);
    static mp_bool CheckNginxOldPwd(const mp_string& strOldPwd);
    static mp_void GetNginxKey(mp_string & strKey, const vector<mp_string>& vecResult);
    static mp_bool CheckNewPwd(PASSWOD_TYPE eType, const mp_string& strNewPwd);
    static mp_bool SaveAdminPwd(const mp_string& strPwd);
    static mp_bool SaveOtherPwd(PASSWOD_TYPE eType, const mp_string& strPwd);
    static mp_bool SaveNginxPwd(const mp_string& strPwd);
    static mp_void InputUserPwd(mp_string strUserName, mp_string &strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen = PWD_MAX_LEN);
    static mp_int32 VerifyOldUserPwd(mp_string& strUserName);
    static mp_int32 InputNewUserPwd(mp_string& strUserName, mp_string& strNewPwd);
    static mp_int32 ConfirmNewUserPwd(mp_string& strUserName, mp_string& strNewPwd);
    static mp_int32 CalComplexity(const mp_string& strPwd, mp_int32& iNum, mp_int32& iUppercase, mp_int32& iLowcase,
        mp_int32& iSpecial);
    static mp_bool CheckCommon(const mp_string& strPwd);
    static mp_int32 CalculateComplexity(mp_int32 iNumber, mp_int32 iUppercase, mp_int32 iLowcase);
    static mp_bool CheckPasswordOverlap(const mp_string& strPasswd);
    static mp_void GetInput(mp_string strHint, mp_string& strInput, mp_int32 iInputLen = PWD_MAX_LEN);
    static mp_void LockAdmin();
    static mp_uint64 GetLockTime();
    static mp_void ClearLock();
};


#endif

