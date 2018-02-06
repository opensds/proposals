#ifndef _KMC_H_
#define _KMC_H_
#endif

#include <iostream>
#include <string>
#include <string.h>
#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#endif
#include <sys/stat.h>
#include <sstream>
#include <iomanip>

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>

using namespace std;

#define OS_LOG_DEBUG        0
#define OS_LOG_INFO         1
#define OS_LOG_WARN         2
#define OS_LOG_ERROR        3

#define MASTERKEY_LEN   1024
#define SIGN_DATA_LEN_MAX   1024
#define FILE_MAC_LEN    32
#define FILE_SIGN_DATA_MAX (FILE_MAC_LEN + 32)

#define KMC_ALGORITHM_ID    EVP_aes_256_cbc()

#define KMC_NULL_PTR       ((void *)0)

#define KMC_SALT_LEN            128u
#define KMC_IV_MAX_LEN          16u
#define KMC_KEY_MAX_LEN        128u

#define KMC_HMAC_HEAD_LEN       (44)
#define KMC_SYM_MAX_BLOCK_SIZE  16u  

#define CIPHER_MAX_LEN 1024
#define WORKKEY_MAX_LEN 1024

#define KMC_FALSE_FOREVER  (!__LINE__)
#define KMC_SAFE_ASSIGN(ptr, val)      do {if (KMC_NULL_PTR != (ptr)){*(ptr) = (val);}else{;}} while(KMC_FALSE_FOREVER)

#define KMC_STORE_FILE "KMC_Crt.conf"
#define KMC_ERROR_LOG_FILE "KMC_error.log"

typedef struct
{   
    unsigned int ulIterCount; 
    unsigned char aucSalt[KMC_SALT_LEN];
    unsigned char aucIV[KMC_IV_MAX_LEN];    
}KMC_CIPHER_HEAD_STRU;

#define KMC_CIPHER_HEAD_LEN     (sizeof(KMC_CIPHER_HEAD_STRU))

typedef struct
{
    void*  pBuff;
    unsigned int nLen;
}KMC_BUFF;

/*------------------------------------------------------------ 
Description  : 将数字数组转换成十六进制，然后将十六进制直接翻译成string
Input        : strIn：数组 inLen：数组长度
Output       : hexOut：转换结果
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/ 
int strToHex(unsigned char *strIn, string &hexOut, unsigned int inLen);

/*------------------------------------------------------------ 
Description  : 将string翻译成十六进制数组
Input        : hexIn：等待转换的string
Output       : strOut：结果数组 outLen：数组长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int hexToStr(const string &hexIn, unsigned char *strOut, unsigned int outLen);

/*------------------------------------------------------------ 
Description  : 创建KMC关键的Master Key, 该Key生成之后，加密和解密必须使用同一个Key，否则解密不成功
Input        : iLen：长度
Output       : pMasterKey：MK存放数组
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_CreateMK(char* pMasterKey, int iLen);

/*------------------------------------------------------------ 
Description  : 判断文件是否存在
Input        : pszFilePath：文件路径
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_FileExist(const char* pszFilePath);

/*------------------------------------------------------------ 
Description  : 检查文件是否正常
Input        : pszFileName：文件路径，iFileLen：文件本来长度
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_CheckFileDetails(const char* pszFileName, int iFileLen);

/*------------------------------------------------------------ 
Description  : 将MK写入文件
Input        : pszInMaterKey：MK，pszFileName：文件路径，iLen：KM长度
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_WriteMasterKey(char *pszInMaterKey, int iLen, const char* pszFileName);

/*------------------------------------------------------------ 
Description  : 读MK
Input        : pszFileName：文件路径，iLen：KM长度
Output       : pszInMaterKey：MK
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_ReadMasterKey(char *pszOutMaterKey, int iLen, const char* pszFileName);

/*------------------------------------------------------------ 
Description  : 初始化KMC组件
Input        : pszKstoreFile：KMC组件的MK文件，pszKmcLogFile：KMC日志文件
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_init(char *pszKstoreFile, char *pszKmcLogFile);

/*------------------------------------------------------------ 
Description  : 获取MK，当文件记录与内存不一致，以内存为准，再次刷内存到文件
Input        : pszMK
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_GetMasterKey(char **pszMK);

/*------------------------------------------------------------ 
Description  : KMC的散列函数，主要将MK加盐值散列
Input        : pvPassword：MK，ulPwdLen：MK长度，pvSalt：盐值，ulSaltLen：盐值长度，iIter：散列迭代次数，
Output       : pvDerivedKey：散列后生成的work KEY，ulDKLen：work KEY的长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Pbkdf2(const char* pvPassword, int ulPwdLen, const unsigned char* pvSalt, int ulSaltLen, int iIter, int ulDKLen, unsigned char *pvDerivedKey);

/*------------------------------------------------------------ 
Description  : 加密时获取work key
Input        : pulIterCount：散列迭代次数，pucSalt：盐值，ulSaltLen：盐值长度
Output       : pucIV：IV向量，ulIVLen: IV向量长度，pucKey：散列后生成的root KEY，ulKeyLen：root KEY的长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_EncryptGetWorkKey(unsigned int *pulIterCount, unsigned char *pucSalt, unsigned int ulSaltLen, unsigned char *pucIV,
    unsigned int ulIVLen, unsigned char *pucKey, unsigned int ulKeyLen);

/*------------------------------------------------------------ 
Description  : 获取IV向量和root Key长度
Input        : 
Output       : pulKeyLen：rootKey长度，pulIVLen：IV向量长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_GetAlgProperty(unsigned int *pulKeyLen, unsigned int *pulIVLen);

/*------------------------------------------------------------ 
Description  : 生成加密头
Input        : pstCipherHead：加密头，pucKey：root key，pulKeyLen：root key长度，pulIVLen：IV向量长度
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_FillCipherTextHeader(KMC_CIPHER_HEAD_STRU *pstCipherHead, unsigned char *pucKey, unsigned int *pulKeyLen, unsigned int *pulIVLen);

/*------------------------------------------------------------ 
Description  : 调用openssl的加密接口，对密码进行正式加密
Input        : pvKey：root key，ulKeyLen：root key长度，pvIV：IV向量，ulIVLen：IV向量长度，
                pvPlainText：明文，ulPlainLen：明文长度，pvCipherText：密文
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_SSL_Encrypy(const void* pvKey, unsigned int ulKeyLen, const void* pvIV, unsigned int ulIVLen, 
    const void* pvPlainText, unsigned int ulPlainLen, void* pvCipherText);

/*------------------------------------------------------------ 
Description  : KMC加密
Input        : pucPlainText：明文，ulPTLen：长度，pucCipherText：密文
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Encrypy(const unsigned char *pucPlainText, unsigned int ulPTLen, unsigned char *pucCipherText);

/*------------------------------------------------------------ 
Description  : KMC对外呈现加密接口
Input        : inStr：明文密码
Output       : outStr：密文
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Encrypt_Str(const string &inStr, string &outStr);

/*------------------------------------------------------------ 
Description  : KMC解密获取root key
Input        : ulIterCount：加密迭代数量，pucSalt：盐值，ulSaltLen：盐值长度，pucKey：rootkey，ulKeyLen：rootkey长度
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/              
int KMC_DecryptGetWorkKey(unsigned int ulIterCount, const unsigned char *pucSalt, unsigned int ulSaltLen, unsigned char *pucKey, unsigned int ulKeyLen);

/*------------------------------------------------------------ 
Description  : KMC获取加密头
Input        : pucCipherText：加密密文，ulCTLen：长度
Output       : pstCipherHead：头
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/ 
int KMC_GetCipherHead(const unsigned char *pucCipherText, unsigned int ulCTLen, KMC_CIPHER_HEAD_STRU *pstCipherHead);

/*------------------------------------------------------------ 
Description  : KMC获取加密真正密文
Input        : pucCipherText：加密密文，ulCTLen：长度, iLen：长度
Output       : pszCipherBody：密文
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/ 
int KMC_GetCipherBody(const unsigned char *pucCipherText, unsigned int ulCTLen, unsigned char *pszCipherBody, int iLen);

/*------------------------------------------------------------ 
Description  : openssl解密
Input        : pucCipherBody：密文，iCipherBodyLen：长度, iLen：长度，uKey：rootKey, uIv: IV向量
Output       : pucPlainText：明文，piPTLen：明文长度，
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/ 
int KMC_SSL_Decrypt(unsigned char* pucCipherBody, int iCipherBodyLen, unsigned char *pucPlainText, int *piPTLen, unsigned char *uKey, unsigned char* uIv);

/*------------------------------------------------------------ 
Description  : KMC解密
Input        : pucCipherText：密文，ulCTLen：长度
Output       : pucPlainText：明文，piPTLen：明文长度，
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Decrypt(const unsigned char *pucCipherText, unsigned int ulCTLen, unsigned char *pucPlainText, unsigned int *pulPTLen);

/*------------------------------------------------------------ 
Description  : KMC解密对外接口
Input        : inStr：密文
Output       : outStr：明文
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Decrypt_Str(const string &inStr, string &outStr);

const char hn[] = "SHA256";

/*------------------------------------------------------------ 
Description  : KMC签名获取HMAC和签名的Key
Input        : 
Output       : pKeySign：Key，pOutHmac：HMAC
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_SignGetHMACKey(EVP_PKEY** pKeySign, unsigned char *pOutHmac);

/*------------------------------------------------------------ 
Description  : KMC对文件签名
Input        : pFileName：文件，pSignKey：签名Key
Output       : pSignData：密文，iLen：密文长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_SignFile(const char *pFileName, unsigned char *pSignData, int* iLen, EVP_PKEY* pSignKey);

/*------------------------------------------------------------ 
Description  : KMC获取签名key
Input        : pInHmac：HMAC
Output       : pKeySign：校验签名Key
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_SignGetVerifyKey(EVP_PKEY** pKeySign, unsigned char *pInHmac);

/*------------------------------------------------------------ 
Description  : KMC校验文件签名
Input        : pFileName：文件，pSignData：密文，iLen：密文长度，pSignKey：Key
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_VerifyFile(const char *pFileName, unsigned char *pSignData, int iLen, EVP_PKEY* pSignKey);

/*------------------------------------------------------------ 
Description  : KMC签名对外接口
Input        : filePath：文件
Output       : fileHMAC：签名密文
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Gen_File_HMAC(const string &filePath, string &fileHMAC);

/*------------------------------------------------------------ 
Description  : KMC校验签名对外接口
Input        : filePath：文件，fileHMAC：签名密文
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Verify_File_HMAC(const string &filePath, const string &fileHMAC);
