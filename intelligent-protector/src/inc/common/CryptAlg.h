/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _CRYPT_ALG_H_
#define _CRYPT_ALG_H_

#include "common/Types.h"
#include "common/Defines.h"
#ifdef WIN32
#include <Wincrypt.h>
#endif


// 密钥分开存放，增加破解难度
static const int AES_KEY_LEN = 16;
static const int AES_IV_LEN = 32;

static const char rKeyStr1[AES_KEY_LEN + 1] = "3FDA1BA5B3E64F1F";
static const char rIVStr1[AES_IV_LEN + 1] = "C124AC7EA0083A1C841411826D8A72DC";
static const char rKeyStr2[AES_KEY_LEN + 1] = "8056BE33A5BC12E4";
static const char rIVStr2[AES_IV_LEN + 1] = "031B171D3E2C638C9993C7155E3CA914";

#define PBKDF_KEY_LEN 64
#define PBKDF_ITER_TIMES 50000
#define CIPHER_BUFF_LEN 128
#define PBKDF_SALT_MAX_LEN 40

#define NOT_KMC_CIPHER_TEXT 353
#define INPUT_BUFF_LEN_NOT_ENOUGH 153

typedef enum
{
    CRYPT_ENCYP_AES = 0,  //AES加密
    CRYPT_DECYP_AES,      //AES解密
    CRYPT_SHA,            //sha 256
    CRYPT_SALT,           //计算盐值
    CRYPT_SECOND,         //计算秒数
    CRYPT_PBKDF2          //PBKDF2算法
}CRYPT_ALG;


#define CHECK_AES_NOT_OK( Call ) \
{\
    mp_int32 iCheckNotOkRet = Call;\
    if (MP_SUCCESS != iCheckNotOkRet)\
    {\
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, \
            "Call %s failed, ret %d.", #Call, iCheckNotOkRet); \
        return ;\
    }\
}\

#define CHECK_AES_DELETE_NOT_OK( Call , var1, var2, var3)\
{\
    mp_int32 iCheckNotOkRet = Call;          \
    if (MP_SUCCESS != iCheckNotOkRet)           \
    {\
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, \
             "Call %s failed, ret %d.", #Call, iCheckNotOkRet); \
        delete[] var1; \
        delete[] var2; \
        delete[] var3; \
        return; \
    } \
}

#define SHA256_BLOCK_SIZE 64

// AES encrypt
mp_void AGENT_API AES_encrypt(const mp_char *pBuffer, const mp_int32 bufSize, mp_string &strOut);

// AES decrypt
mp_void AGENT_API AES_decrypt(const mp_char *pBuffer, const mp_int32 bufSize, mp_string &strOut);

// SHA256
mp_int32 AGENT_API GetSha256Hash(const mp_char* buff, const mp_int32 len, mp_char* hashHex, mp_int32 hexLen);

//PBKDF2
mp_int32 AGENT_API PBKDF2Hash(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText);

//RANDOM
mp_int32 AGENT_API GetRandom(mp_uint64 &num);

// Init crypt
mp_int32 AGENT_API InitCrypt();

// Finalize crypt
mp_int32 AGENT_API FinalizeCrypt();

// Call crypt timer
mp_void AGENT_API CallCryptTimer();

// Encrypt string
mp_void AGENT_API EncryptStr(const mp_string &inStr, mp_string &outStr);

// Decrypt string
mp_void AGENT_API DecryptStr(const mp_string &inStr, mp_string &outStr);

// Compute HMAC
mp_int32 AGENT_API ComputeHMAC(const mp_string &filePath, mp_string &fileHMAC);

// Verify HMAC
mp_int32 AGENT_API VerifyHMAC(const mp_string &filePath, const mp_string & fileHMAC); 


#endif

