#include "KMC.h"
//#include "common/Log.h"
//#include "common/Path.h"
#include "securec.h"

char g_FileKeyStore[1024] = {0};
char *g_pszMaterKey = NULL;

FILE* g_LogFile = NULL;

#ifndef WIN32
#define KMC_LOG(pszFormat, args...) \
do{ \
    if (g_LogFile != NULL) \
    { \
        fprintf(g_LogFile, "", pszFormat, ##args);\
    }\
} \
while(0);
#else
#define KMC_LOG(pszFormat, ...) \
do{ \
    if (g_LogFile != NULL) \
    { \
        fprintf(g_LogFile, pszFormat, __VA_ARGS__);\
    }\
} \
while(0);
#endif
/*------------------------------------------------------------ 
Description  : 将数字数组转换成十六进制，然后将十六进制直接翻译成string
Input        : strIn：数组 inLen：数组长度
Output       : hexOut：转换结果
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/ 
int strToHex(unsigned char *strIn, string &hexOut, unsigned int inLen)
{
    if (strIn == NULL || inLen == 0)
    {
        return 1;
    }
    ostringstream oss;
    oss << hex;
    for (unsigned int i = 0; i < inLen; i++)
    {
        oss << setw(2) << setfill('0') << (int)strIn[i];
    }
    hexOut = oss.str();
    if (hexOut.size() != inLen * 2)
    {
        hexOut = "";
        return 1;
    }

    return 0;
}

/*------------------------------------------------------------ 
Description  : 将string翻译成十六进制数组
Input        : hexIn：等待转换的string
Output       : strOut：结果数组 outLen：数组长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int hexToStr(const string &hexIn, unsigned char *strOut, unsigned int outLen)
{
    unsigned int inLen = hexIn.length();
    if (inLen == 0 || inLen % 2 == 1)
    {
        return 1;
    }
    if (strOut == NULL)
    {
        return 1;
    }
    inLen /= 2;
    if (outLen < inLen)
    {
        return 1;
    }
    int iRet = memset_s(strOut, outLen, 0, outLen);
    if (iRet != 0)
    {
       KMC_LOG("Call memset_s failed, ret = %d.", iRet);
        return 1;
    }

    stringstream ss;
    int tmp;
    ss << hex;
    for (unsigned int i = 0; i < inLen; i++)
    {
        ss << hexIn.substr(i*2, 2);
        tmp = -1;
        ss >> tmp;
        if (tmp == -1)
        {
            return 1;
        }
        strOut[i] = (unsigned char)tmp;
        ss.str("");
        ss.clear();
    }

    return 0;
}

/*------------------------------------------------------------ 
Description  : 创建KMC关键的Master Key, 该Key生成之后，加密和解密必须使用同一个Key，否则解密不成功
Input        : iLen：长度
Output       : pMasterKey：MK存放数组
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_CreateMK(char* pMasterKey, int iLen)
{
    if (NULL == pMasterKey || 0 == iLen)
    {
        KMC_LOG("The Param of pMasterKey(%p) or iLen(%d) is error!\n", pMasterKey, iLen);
        return 1;
    }
    
    int iRet = RAND_bytes((unsigned char*)pMasterKey, iLen);
    if(1 != iRet)
    {
        KMC_LOG("Call CRYPT_random failed : %d\n", iRet);
        return 1;
    }
    return 0;
}

/*------------------------------------------------------------ 
Description  : 判断文件是否存在
Input        : pszFilePath：文件路径
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_FileExist(const char* pszFilePath)
{
#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszFilePath, &fileStat))
#else
    struct stat fileStat;
    if (0 != stat(pszFilePath, &fileStat))
#endif
    {
        KMC_LOG("stat file(%s) failed, errno: %d, %s!\n", pszFilePath, errno, strerror(errno));
        return 1;
    }
#ifndef WIN32
    if (S_ISDIR(fileStat.st_mode))
    {
        KMC_LOG("File(%s) is a dir.!\n");
        return 1;
    }
#endif
    return 0;
}

/*------------------------------------------------------------ 
Description  : 检查文件是否正常
Input        : pszFileName：文件路径，iFileLen：文件本来长度
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_CheckFileDetails(const char* pszFileName, int iFileLen)
{
    FILE *pFile;
    int iRet = 0;
    int iReadLen = 0;
#ifndef WIN32
    char pszFilePath[PATH_MAX + 1] = {0};   
    if (strlen(pszFileName) > PATH_MAX || NULL == realpath(pszFileName, pszFilePath))
    {
        KMC_LOG("realpath file(%s) failed..., errno: %d, %s\n", pszFileName, errno, strerror(errno));
        return 1;
    }
#else
    const char *pszFilePath = pszFileName;
#endif
    pFile = fopen(pszFilePath, "rb");
    if (!pFile)
    {
        KMC_LOG("Open file(%s) failed, errno: %d, %s!\n", pszFilePath, errno, strerror(errno));
        return 1;
    }
    iRet = fseek(pFile, 0L, SEEK_END);
    if (iRet != 0)
    {
        KMC_LOG("fseek file(%s) failed, errno: %d, %s!\n", pszFileName, errno, strerror(errno));
        fclose(pFile);
        return 1;
    }
    iReadLen = (int)ftell(pFile);
    if (iReadLen != iFileLen)
    {
        KMC_LOG("File was broken, iReadLen: %d, iFileLen: %d\n", iReadLen, iFileLen);
        fclose(pFile);
        return 1;
    }
    //printf("File is OK, iReadLen: %d, iFileLen: %d\n", iReadLen, iFileLen);
    fclose(pFile);
    return 0;
}

/*------------------------------------------------------------ 
Description  : 将MK写入文件
Input        : pszInMaterKey：MK，pszFileName：文件路径，iLen：KM长度
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_WriteMasterKey(char *pszInMaterKey, int iLen, const char* pszFileName)
{
    FILE *pFile;
    int iRet = 0;
    //string strHexString;
    if (!pszInMaterKey)
    {
        KMC_LOG("pszInMaterKey is NULL, write failed!\n");
        return 1;
    }

    pFile = fopen(pszFileName, "w+");
    if (!pFile)
    {
        KMC_LOG("Open file(%s) failed, errno: %d, %s!\n", pszFileName, errno, strerror(errno));
        return 1;
    }
    
    /*iRet = strToHex((unsigned char*)pszInMaterKey, strHexString, iLen);
    if (iRet != 0)
    {
        KMC_LOG("strToHex failed!\n");
        fclose(pFile);
        return 1;
    }*/

    iRet = fwrite(pszInMaterKey, iLen, 1, pFile);
    if (iRet != 1)
    {
        KMC_LOG("Write file(%s) failed, iRet: %d, errno: %d, %s!\n", pszFileName, iRet, errno, strerror(errno));
        fclose(pFile);
        return 1;
    }
    fclose(pFile);
    return 0;
}

/*------------------------------------------------------------ 
Description  : 读MK
Input        : pszFileName：文件路径，iLen：KM长度
Output       : pszInMaterKey：MK
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_ReadMasterKey(char *pszOutMaterKey, int iLen, const char* pszFileName)
{
    FILE *pFile;
    int iRet = 0;

    if (!pszOutMaterKey)
    {
        KMC_LOG("pszOutMaterKey is NULL, read failed!\n");
        return 1;
    }
#ifndef WIN32
    char pszFilePath[PATH_MAX + 1] = {0};   
    if (strlen(pszFileName) > PATH_MAX || NULL == realpath(pszFileName, pszFilePath))
    {
        KMC_LOG("realpath file(%s) failed..., errno: %d, %s\n", pszFileName, errno, strerror(errno));
        return 1;
    }
#else
    const char *pszFilePath = pszFileName;
#endif
    pFile = fopen(pszFilePath, "rb");
    if (!pFile)
    {
        KMC_LOG("Open file(%s) failed, errno: %d, %s!\n", pszFilePath, errno, strerror(errno));
        return 1;
    }
    iRet = fseek(pFile, 0L, SEEK_SET);
    if (iRet != 0)
    {
        KMC_LOG("fseek file(%s) failed, errno: %d, %s!\n", pszFilePath, errno, strerror(errno));
        fclose(pFile);
        return 1;
    }
    iRet = fread(pszOutMaterKey, iLen, 1, pFile);
    if (iRet != 1)
    {
        KMC_LOG("Read file(%s) failed, iRet: %d!\n", pszFileName, iRet);
        fclose(pFile);
        return 1;
    }

    fclose(pFile);
    return 0;
}

/*------------------------------------------------------------ 
Description  : 初始化KMC组件MK
Input        : 
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_init_MK()
{
    int iRet = KMC_FileExist(g_FileKeyStore);
    if (iRet != 0)
    {
        iRet = KMC_CreateMK(g_pszMaterKey, MASTERKEY_LEN);
        if (iRet != 0)
        {
           KMC_LOG("Create KMC Master Key failed!\n");
            return 1;
        }
        iRet = KMC_WriteMasterKey(g_pszMaterKey, MASTERKEY_LEN, g_FileKeyStore);
        if (iRet != 0)
        {
           KMC_LOG("Write KMC keystore file failed\n");
            return 1;
        }
        KMC_LOG("Write KMC keystore file sucess.\n");
        return 0;
    }
    
    //iRet = KMC_CheckFileDetails(g_FileKeyStore, MASTERKEY_LEN);
#ifndef WIN32
    if (access(g_FileKeyStore, R_OK))
    {
        KMC_LOG("File(%s) is not accessable.\n", g_FileKeyStore);
        return 1;
    }
#endif
    iRet = KMC_ReadMasterKey(g_pszMaterKey, MASTERKEY_LEN, g_FileKeyStore);
    if (iRet != 0)
    {
        KMC_LOG("Read KMC keystore file failed\n");
        KMC_LOG("KMC keystore file has excption, rebuild it.\n");
        iRet = KMC_CreateMK(g_pszMaterKey, MASTERKEY_LEN);
        if (iRet != 0)
        {
            KMC_LOG("Create KMC Master Key failed!\n");
            return 1;
        }
        iRet = KMC_WriteMasterKey(g_pszMaterKey, MASTERKEY_LEN, g_FileKeyStore);
        if (iRet != 0)
        {
            KMC_LOG("Rebuild KMC keystore file failed\n");
            return 1;
        }
        KMC_LOG("Rebuild KMC keystore file sucess.\n");
        return 0;
    }
    return 0;
}
/*------------------------------------------------------------ 
Description  : 初始化KMC组件
Input        : pszKstoreFile：KMC组件的MK文件，pszKmcLogFile：KMC日志文件
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_init(char *pszKstoreFile, char *pszKmcLogFile)
{
    int iRet = 0;
    if (NULL == pszKstoreFile)
    {
        return 1;
    }
    
    if (NULL == pszKmcLogFile || strlen(pszKmcLogFile) <= 0 || strlen(pszKmcLogFile) >= MASTERKEY_LEN)
    {
        g_LogFile = NULL;
    }
    else
    {
        g_LogFile = fopen(pszKmcLogFile, "w+");
        if (g_LogFile == NULL)
        {
            KMC_LOG("Open file(%s) failed, errno: %d, %s!\n", pszKstoreFile, errno, strerror(errno));
            return 1;
        }
    }  
    
    int iFileLen = strlen(pszKstoreFile);
    if (iFileLen >= MASTERKEY_LEN || iFileLen <= 0)
    {
        KMC_LOG("The lenth(%d) of pszKstoreFile is error.\n", iFileLen);
        return 1;
    }
    
    iRet = memcpy_s(g_FileKeyStore, MASTERKEY_LEN, pszKstoreFile, iFileLen);
    if (iRet != 0)
    {
        KMC_LOG("Init KMC failed!\n");
        return 1;
    }
    
    g_FileKeyStore[iFileLen] = '\0';

    g_pszMaterKey = (char*)malloc(sizeof(char) * MASTERKEY_LEN);
    if (!g_pszMaterKey)
    {
        KMC_LOG("Init KMC failed!\n");
        return 1;
    }
    iRet = memset_s(g_pszMaterKey, MASTERKEY_LEN, 0, MASTERKEY_LEN);
    if (iRet != 0)
    {
        KMC_LOG("Call memset_s failed, ret = %d.\n", iRet);
        return 1;
    }
    
    iRet = KMC_init_MK();
    if (iRet != 0)
    {
        KMC_LOG("KMC init MK failed.\n");
        return 1;
    }
    
    KMC_LOG("Kmc init success!\n");
    return 0;
}

/*------------------------------------------------------------ 
Description  : 获取MK，当文件记录与内存不一致，以内存为准，再次刷内存到文件
Input        : pszMK
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_GetMasterKey(char **pszMK)
{
    char pszMasterKey[MASTERKEY_LEN] = {0};
    if (NULL == g_pszMaterKey)
    {
       KMC_LOG("g_pszMaterKey is null, Get MK failed!\n");
        return 1;
    }
    
    int iRet = KMC_ReadMasterKey(pszMasterKey, MASTERKEY_LEN, g_FileKeyStore);
    if (iRet != 0)
    {
       KMC_LOG("Read KMC keystore file failed\n");
        return 1;
    }
    
    iRet = CRYPTO_memcmp(pszMasterKey, g_pszMaterKey, MASTERKEY_LEN);
    if (iRet == 0)   //文件与内存中记录的相同，返回
    {
        //printf("Get MK success!\n");
        *pszMK = g_pszMaterKey;
        return 0;
    }
    //文件记录与内存不一致，以内存为准，再次刷内存到文件
    iRet = KMC_WriteMasterKey(g_pszMaterKey, MASTERKEY_LEN, g_FileKeyStore);
    if (iRet != 0)
    {
       KMC_LOG("Rebuild KMC keystore file failed\n");
        return 1;
    }
    
    *pszMK = g_pszMaterKey;
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC的散列函数，主要将MK加盐值散列
Input        : pvPassword：MK，ulPwdLen：MK长度，pvSalt：盐值，ulSaltLen：盐值长度，iIter：散列迭代次数，
Output       : pvDerivedKey：散列后生成的work KEY，ulDKLen：work KEY的长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Pbkdf2(const char* pvPassword, int ulPwdLen, const unsigned char* pvSalt, int ulSaltLen, int iIter, int ulDKLen, unsigned char *pvDerivedKey)
{
    const EVP_MD *ctx = NULL;
    ctx = EVP_sha256();
    if (NULL == ctx)
    {
       KMC_LOG("ctx: is NULL, call KMC_Pbkdf2 failed!\n");
        return 1;
    }
    if(1 == PKCS5_PBKDF2_HMAC(pvPassword, ulPwdLen, pvSalt, ulSaltLen, iIter, ctx, ulDKLen, pvDerivedKey))
	{
        //printf("Call PKCS5_PBKDF2_HMAC sunccess, call KMC_Pbkdf2 success!\n");
		return 0;
	}
   KMC_LOG("Call PKCS5_PBKDF2_HMAC failed, call KMC_Pbkdf2 failed!\n");
	return 1;
}

/*------------------------------------------------------------ 
Description  : 加密时获取work key
Input        : pulIterCount：散列迭代次数，pucSalt：盐值，ulSaltLen：盐值长度
Output       : pucIV：IV向量，ulIVLen: IV向量长度，pucKey：散列后生成的root KEY，ulKeyLen：root KEY的长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_EncryptGetWorkKey(unsigned int *pulIterCount, unsigned char *pucSalt, unsigned int ulSaltLen, unsigned char *pucIV,
    unsigned int ulIVLen, unsigned char *pucKey, unsigned int ulKeyLen)
{
    char *pszMK = NULL;
    unsigned int ulTempIterCount = 1;
    if (NULL == pucSalt || 0 == ulSaltLen || NULL == pucIV || 0 == ulIVLen || 0 == ulKeyLen)
    {
        KMC_LOG("Param is error, Encrypt get work key failed.\n");
        return 1;
    }
    int iRet = RAND_bytes((unsigned char*)pucSalt, ulSaltLen);
    if(1 != iRet)
    {
        KMC_LOG("Call CRYPT_random failed : %d, Get pucSalt failed.\n", iRet);
        return 1;
    }
    iRet = KMC_GetMasterKey(&pszMK);
    if (iRet != 0)
    {
        KMC_LOG("Encrypt Get MK failed!\n");
        return 1;
    }
    iRet = RAND_bytes((unsigned char*)pucIV, ulIVLen);
    if(1 != iRet)
    {
        KMC_LOG("Call CRYPT_random failed : %d, Encrypt Get IV failed.\n", iRet);
        return 1;
    }

    iRet = KMC_Pbkdf2(pszMK, MASTERKEY_LEN, pucSalt, (int)ulSaltLen, (int)ulTempIterCount, (int)ulKeyLen, pucKey);
    if(0 != iRet)
    {
        KMC_LOG("Generate encrypt work key failed : %d.\n", iRet);
        return 1;
    }
    *pulIterCount = ulTempIterCount;
    pszMK = NULL;

    //printf("Generate encrypt work key sunccess.\n");
    //memset_s(pszMK, 0, sizeof(pszMK));
    return 0;
}

/*------------------------------------------------------------ 
Description  : 获取IV向量和root Key长度
Input        : 
Output       : pulKeyLen：rootKey长度，pulIVLen：IV向量长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_GetAlgProperty(unsigned int *pulKeyLen, unsigned int *pulIVLen)
{
    unsigned int ulTempKeyLen   = 0;
    unsigned int ulTempIVLen    = 0;

    if (KMC_NULL_PTR != pulKeyLen)
    {
        ulTempKeyLen = EVP_CIPHER_key_length(KMC_ALGORITHM_ID);
    }

    if (KMC_NULL_PTR != pulIVLen)
    {
        ulTempIVLen = EVP_CIPHER_iv_length(KMC_ALGORITHM_ID);
    }
    /* output param */

    if (pulKeyLen != NULL)
    {
        *pulKeyLen = ulTempKeyLen;
    }
    if (pulIVLen != NULL)
    {
        *pulIVLen = ulTempIVLen;
    }
    return 0;
}

/*------------------------------------------------------------ 
Description  : 生成加密头
Input        : pstCipherHead：加密头，pucKey：root key，pulKeyLen：root key长度，pulIVLen：IV向量长度
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_FillCipherTextHeader(KMC_CIPHER_HEAD_STRU *pstCipherHead, unsigned char *pucKey, unsigned int *pulKeyLen, unsigned int *pulIVLen)
{
    int  iRet       = 0;
    unsigned int ulIVLen     = 0;
    unsigned int ulKeyLen    = 0;
    unsigned int ulIterCount = 0;

    /* Get key length, IV length */
    iRet = KMC_GetAlgProperty(&ulKeyLen, &ulIVLen);
    if(iRet != 0) 
    {
       KMC_LOG("KMC_GetAlgProperty failed. \n");
        return 1;
    }
    /* Get WK (work key) interface */
    iRet = KMC_EncryptGetWorkKey(&ulIterCount, pstCipherHead->aucSalt, KMC_SALT_LEN, pstCipherHead->aucIV, ulIVLen, pucKey, ulKeyLen);
    if(iRet != 0) 
    {
       KMC_LOG("KMC_EncryptGetWorkKey failed. \n");
        return 1;
    }
    /* fill header except pstCipherHead->ulCDLen */
    pstCipherHead->ulIterCount  = ulIterCount;
    *pulKeyLen = ulKeyLen;
    *pulIVLen  = ulIVLen;

    return 0;
}

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
    const void* pvPlainText, unsigned int ulPlainLen, void* pvCipherText)
{
    KMC_BUFF stCipher = {0};
    const EVP_CIPHER *cipher_type;
    int bytes_written = 0;
    int iPlainLe = (int)(ulPlainLen + 32);
    if (iPlainLe <= 0 || iPlainLe > CIPHER_MAX_LEN)
    {
        KMC_LOG("iPlainLe(%d) is error. \n", iPlainLe);
        return 1;
    }
    
    stCipher.pBuff = (void*)malloc(iPlainLe);
    if (stCipher.pBuff == NULL)
    {
        KMC_LOG("Malloc stCipher.pBuff error. \n");
        return 1;
    }
    
    EVP_CIPHER_CTX *ctx;
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        KMC_LOG("CIPHER CTX creation failed. \n");
        free(stCipher.pBuff);
        return 1;
    }
    
    cipher_type = KMC_ALGORITHM_ID;
    
    if(!EVP_EncryptInit_ex(ctx, cipher_type, NULL, (unsigned char *)pvKey, (unsigned char *)pvIV)){
        KMC_LOG("ERROR in EVP_EncryptInit_ex \n");
        EVP_CIPHER_CTX_free(ctx);
        free(stCipher.pBuff);
        return 1;
    }
    
    if(!EVP_EncryptUpdate(ctx, (unsigned char *)stCipher.pBuff, &bytes_written, (unsigned char *)pvPlainText, (int)ulPlainLen ) ) {
        EVP_CIPHER_CTX_free(ctx);
        free(stCipher.pBuff);
        return 1;
    }
    stCipher.nLen += bytes_written;
    
    if(!EVP_EncryptFinal_ex(ctx, ((unsigned char *)stCipher.pBuff) + bytes_written, &bytes_written)){
        EVP_CIPHER_CTX_free(ctx);
        KMC_LOG("ERROR in EVP_EncryptFinal_ex \n");
        free(stCipher.pBuff);
        return 1;
    }
    stCipher.nLen += bytes_written;
    
    EVP_CIPHER_CTX_free(ctx);
    if (stCipher.nLen <= 0 || stCipher.nLen > CIPHER_MAX_LEN)
    {
        KMC_LOG("stCipher.nLen(%s) is error.\n", stCipher.nLen);
        free(stCipher.pBuff);
        return 1;
    }
    int iRet = memcpy_s(pvCipherText, stCipher.nLen, stCipher.pBuff, stCipher.nLen);
    if (iRet != 0)
    {
        KMC_LOG("Call memcpy_s failed, ret = %d.\n", iRet);
        free(stCipher.pBuff);
        return 1;
    }
    free(stCipher.pBuff);
    
    //int i = 0;
    //printf("encrypt string: ");
    //for( i =0;i < stCipher.nLen; i++)
    //   KMC_LOG("%s", pvCipherText);
    
    //printf("\n");
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC加密
Input        : pucPlainText：明文，ulPTLen：长度，pucCipherText：密文
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Encrypy(const unsigned char *pucPlainText, unsigned int ulPTLen, unsigned char *pucCipherText)
{
    KMC_CIPHER_HEAD_STRU *pstCipherHead = NULL;
    unsigned char aucKey[KMC_KEY_MAX_LEN] = {0};
    unsigned int ulKeyLen        = 0;
    unsigned int ulIVLen         = 0;
    
    int iRet = memset_s(pucCipherText, sizeof(KMC_CIPHER_HEAD_STRU), 0, sizeof(KMC_CIPHER_HEAD_STRU));
    if (iRet != 0)
    {
        KMC_LOG("Call memset_s failed, ret = %d.", iRet);
        return 1;
    }
    
    pstCipherHead = (KMC_CIPHER_HEAD_STRU *)pucCipherText;
    
    iRet = KMC_FillCipherTextHeader(pstCipherHead, aucKey, &ulKeyLen, &ulIVLen);
    if (iRet != 0)
    {
        KMC_LOG("KMC_Encrypy failed, ret = %d.", iRet);
        return 1;
    }
    
    iRet = KMC_SSL_Encrypy(aucKey, ulKeyLen, pstCipherHead->aucIV, ulIVLen, pucPlainText, ulPTLen, (pucCipherText + KMC_CIPHER_HEAD_LEN));
    if (iRet != 0)
    {
        KMC_LOG("KMC_Encrypy, ret = %d.", iRet);
        return 1;
    }
    
    iRet = memset_s(aucKey, ulKeyLen, 0, ulKeyLen);
    if (iRet != 0)
    {
        KMC_LOG("Call memset_s failed, ret = %d.", iRet);
        return 1;
    }
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC对外呈现加密接口
Input        : inStr：明文密码
Output       : outStr：密文
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Encrypt_Str(const string &inStr, string &outStr)
{ 
    
    int inLen = inStr.length();
    int outLen = KMC_CIPHER_HEAD_LEN + (((inLen / KMC_SYM_MAX_BLOCK_SIZE) + 1) * KMC_SYM_MAX_BLOCK_SIZE);
    int iRet = 0;
    
    if (inLen <= 0 || inLen > CIPHER_MAX_LEN)
    {
        KMC_LOG("inStr len(%d) is error.", inLen);
        return 1;
    }
    if (outLen <= 0 || outLen > CIPHER_MAX_LEN)
    {
        KMC_LOG("outStr len(%d) is error.", outLen);
        return 1;
    }
    
    unsigned char * inBuf = new unsigned char [inLen];
    unsigned char * outBuf = new unsigned char [outLen];
    
    iRet = memset_s(inBuf, inLen, 0, inLen);
    if (iRet != 0)
    {
        KMC_LOG("Call memset_s failed, ret = %d.", iRet);
        delete [] inBuf;
        delete [] outBuf;
        return 1;
    }
    
    iRet = memset_s(outBuf, outLen, 0, outLen);
    if (iRet != 0)
    {
        KMC_LOG("Call memset_s failed, ret = %d.", iRet);
        delete [] inBuf;
        delete [] outBuf;
        return 1;
    }

    iRet = memcpy_s(inBuf, inLen, inStr.c_str(), inLen);
    if (iRet != 0)
    {
        KMC_LOG("Call memcpy_s failed, ret = %d.", iRet);
        delete [] inBuf;
        delete [] outBuf;
        return 1;
    }
    
    iRet = KMC_Encrypy(inBuf, inLen, outBuf);
    if (iRet != 0)
    {
        KMC_LOG("Call KMC_Encrypy failed, ret = %d.", iRet);
        delete [] inBuf;
        delete [] outBuf;
        return 1;
    }

    iRet = strToHex(outBuf, outStr, outLen);
    if(0 != iRet){
        KMC_LOG("Convert str to Hex str failed.");
        outStr="";
        delete [] inBuf;
        delete [] outBuf;
        return 1;
    }
    
    delete [] inBuf;
    delete [] outBuf;
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC解密获取root key
Input        : ulIterCount：加密迭代数量，pucSalt：盐值，ulSaltLen：盐值长度，pucKey：rootkey，ulKeyLen：rootkey长度
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/              
int KMC_DecryptGetWorkKey(unsigned int ulIterCount, const unsigned char *pucSalt, unsigned int ulSaltLen, unsigned char *pucKey, unsigned int ulKeyLen)
{
    char *pszMK = NULL;
    if (NULL == pucSalt || 0 == ulSaltLen || 0 == ulKeyLen)
    {
        KMC_LOG("Param is error, Decrypt get work key failed.\n");
        return 1;
    }
    int iRet = KMC_GetMasterKey(&pszMK);
    if (iRet != 0)
    {
        KMC_LOG("Encrypt Get MK failed!\n");
        return 1;
    }
    iRet = KMC_Pbkdf2(pszMK, MASTERKEY_LEN, pucSalt, (int)ulSaltLen, (int)ulIterCount, (int)ulKeyLen, pucKey);
    if(0 != iRet)
    {
        KMC_LOG("Generate encrypt work key failed : %d.\n", iRet);
        return 1;
    }
    pszMK = NULL;

    //printf("Generate decrypt work key success.\n");
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC获取加密头
Input        : pucCipherText：加密密文，ulCTLen：长度
Output       : pstCipherHead：头
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/ 
int KMC_GetCipherHead(const unsigned char *pucCipherText, unsigned int ulCTLen, KMC_CIPHER_HEAD_STRU *pstCipherHead)
{
    if (NULL == pstCipherHead)
    {
        KMC_LOG("Pram pstCipherHead is NULL, Get cipher head failed!\n");
        return 1;
    }
    if (sizeof(KMC_CIPHER_HEAD_STRU) >= ulCTLen)
    {
        KMC_LOG("The cipher txt is error, actual lenth: %d, need atleast: %d!\n", ulCTLen, sizeof(KMC_CIPHER_HEAD_STRU));
        return 1;
    }
    
    int iRet = memcpy_s(pstCipherHead, sizeof(KMC_CIPHER_HEAD_STRU), pucCipherText, sizeof(KMC_CIPHER_HEAD_STRU));
    if (iRet != 0)
    {
        KMC_LOG("Call memcpy_s failed, ret = %d.", iRet);
        return 1;
    }
    
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC获取加密真正密文
Input        : pucCipherText：加密密文，ulCTLen：长度, iLen：长度
Output       : pszCipherBody：密文
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/ 
int KMC_GetCipherBody(const unsigned char *pucCipherText, unsigned int ulCTLen, unsigned char *pszCipherBody, int iLen)
{
    if (NULL == pszCipherBody || NULL == pucCipherText || ulCTLen <= iLen || 0 == iLen)
    {
        KMC_LOG("Pram is error, Get cipher body failed!\n");
        return 1;
    }
    
    int iStartPos = ulCTLen - iLen;
    if (iLen <= 0 || iLen > CIPHER_MAX_LEN)
    {
        KMC_LOG("The param ilen(%d) is error\n", iLen);
        return 1;
    }
    if (iStartPos <= 0 || iStartPos > ulCTLen)
    {
        KMC_LOG("The param iStartPos(%d) is error\n", iStartPos);
        return 1;
    }
    int iRet = memcpy_s(pszCipherBody, iLen, pucCipherText + (ulCTLen - iLen), iLen);
    if (iRet != 0)
    {
        KMC_LOG("Call memset_s failed, ret = %d.\n", iRet);
        return 1;
    }
    return 0;
}

/*------------------------------------------------------------ 
Description  : openssl解密
Input        : pucCipherBody：密文，iCipherBodyLen：长度, iLen：长度，uKey：rootKey, uIv: IV向量
Output       : pucPlainText：明文，piPTLen：明文长度，
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/ 
int KMC_SSL_Decrypt(unsigned char* pucCipherBody, int iCipherBodyLen, unsigned char *pucPlainText, int *piPTLen, unsigned char *uKey, unsigned char* uIv)
{
    EVP_CIPHER_CTX *ctx;
    int len = 0;
	int totallen = 0;
    
    len = iCipherBodyLen + EVP_CIPHER_block_size(KMC_ALGORITHM_ID);
    
    if(!(ctx = EVP_CIPHER_CTX_new())) 
    {
        KMC_LOG("CIPHER CTX creation failed. \n");
        return 1;
    }
     
    if(!EVP_DecryptInit_ex(ctx, KMC_ALGORITHM_ID, NULL, uKey, uIv))
    {
        KMC_LOG("ERROR in EVP_DecryptInit_ex \n");
        EVP_CIPHER_CTX_free(ctx);
        return 1;
    }
    
    if(!EVP_DecryptUpdate(ctx, pucPlainText, &len, pucCipherBody, iCipherBodyLen))
    {
        KMC_LOG("ERROR in EVP_DecryptUpdate\n");
        EVP_CIPHER_CTX_free(ctx);
        return 1;
    }
    
    totallen = len;
    
    if(!EVP_DecryptFinal_ex(ctx, pucPlainText + totallen, &len))
    {
        KMC_LOG("ERROR in EVP_DecryptFinal_ex\n");
        EVP_CIPHER_CTX_free(ctx);
        return 1;
    }
    *piPTLen = totallen + len;
    
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC解密
Input        : pucCipherText：密文，ulCTLen：长度
Output       : pucPlainText：明文，piPTLen：明文长度，
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Decrypt(const unsigned char *pucCipherText, unsigned int ulCTLen, unsigned char *pucPlainText, unsigned int *pulPTLen)
{
    unsigned char pucCipherBody[CIPHER_MAX_LEN] = {0};
    unsigned char pucKey[WORKKEY_MAX_LEN] = {0};
    unsigned int iCipherBodyLen = 0;
    unsigned int ulKeyLen = 0;
    unsigned int ulIVLen = 0;
    
    KMC_CIPHER_HEAD_STRU *pstCipherHead = (KMC_CIPHER_HEAD_STRU*)malloc(sizeof(KMC_CIPHER_HEAD_STRU));
    if (NULL == pstCipherHead)
    {
        KMC_LOG("Malloc pstCipherHead failed, decrypt failed!\n");
        return 1;
    }
    int iRet = KMC_GetCipherHead(pucCipherText, ulCTLen, pstCipherHead);
    if (iRet != 0)
    {
        KMC_LOG("Get cipher head failed!\n");
        free(pstCipherHead);
        return 1;
    }
    
    iCipherBodyLen = ulCTLen - KMC_CIPHER_HEAD_LEN;
    iRet = KMC_GetCipherBody(pucCipherText, ulCTLen, pucCipherBody, iCipherBodyLen);
    if (iRet != 0)
    {
        KMC_LOG("Get cipher body failed!\n");
        free(pstCipherHead);
        return 1;
    }
    
    iRet = KMC_GetAlgProperty(&ulKeyLen, &ulIVLen);
    if (iRet != 0)
    {
        KMC_LOG("Get alg property failed!\n");
        free(pstCipherHead);
        return 1;
    }

    iRet = KMC_DecryptGetWorkKey(pstCipherHead->ulIterCount, pstCipherHead->aucSalt, KMC_SALT_LEN, pucKey, ulKeyLen);
    if (iRet != 0)
    {
        KMC_LOG("Generate decrypt key failed!\n");
        free(pstCipherHead);
        return 1;
    }
    
    iRet = KMC_SSL_Decrypt(pucCipherBody, iCipherBodyLen, pucPlainText, (int*)pulPTLen, pucKey, pstCipherHead->aucIV);
    if (iRet != 0)
    {
        KMC_LOG("Openssl decrypt cipher body failed!\n");
        free(pstCipherHead);
        return 1;
    }
    if (*pulPTLen <= 0 || *pulPTLen > CIPHER_MAX_LEN)
    {
        KMC_LOG("pulPTLen(%d) is error, Openssl decrypt cipher body failed!\n", *pulPTLen);
        free(pstCipherHead);
        return 1;
    }
    pucPlainText[*pulPTLen] = 0;
    free(pstCipherHead);
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC解密对外接口
Input        : inStr：密文
Output       : outStr：明文
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Decrypt_Str(const string &inStr, string &outStr)
{
    outStr = "";
    unsigned int inLen = inStr.length();
    if (inLen <= 0 || inLen % 2 == 1 || inLen > CIPHER_MAX_LEN)
    {
        KMC_LOG("In str len(%d) is error.\n", inLen);
        return 1;
    }
    inLen /= 2;
    unsigned char *inBuf = new unsigned char[inLen];
    if (inBuf == NULL)
    {
        KMC_LOG("New buf failed.\n");
        return 1;
    }
    //printf("inStr: %s.\n", inStr.c_str());
    int iRet = hexToStr(inStr, inBuf, inLen);
    if (iRet != 0)
    {
        KMC_LOG("Convert Hex str to str failed.\n");
        delete [] inBuf;
        return 1;
    }
    unsigned int outLen = inLen;
    unsigned int outBufLen = inLen+1;
    if (outBufLen <= 0 || outBufLen > CIPHER_MAX_LEN)
    {
        KMC_LOG("Out str len(%d) is error.\n", outBufLen);
        delete [] inBuf;
        return 1;
    }
    unsigned char *outBuf = new unsigned char[outBufLen];
    if (outBuf == NULL)
    {
        KMC_LOG("New buf failed.\n");
        delete [] inBuf;
        return 1;
    }
    iRet = memset_s(outBuf, outBufLen, 0, outBufLen);
    if (iRet != 0)
    {
        KMC_LOG("Call memset_s failed, ret = %d.", iRet);
        delete [] inBuf;
        delete [] outBuf;
        return 1;
    }
    iRet = KMC_Decrypt(inBuf, inLen, outBuf, &outLen);
    if(iRet != 0)
    {
        KMC_LOG("KMC decryption failed, ret = %d.\n", iRet);
        delete [] inBuf;
        delete [] outBuf;
        return 1;
    }

    outStr = (char *)outBuf;
    iRet = memset_s(outBuf, outBufLen, 0, outBufLen);
    if (iRet != 0)
    {
        KMC_LOG("Call memset_s failed, ret = %d.", iRet);
        delete [] inBuf;
        delete [] outBuf;
        return 1;
    }
    delete [] inBuf;
    delete [] outBuf;
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC签名获取HMAC和签名的Key
Input        : 
Output       : pKeySign：Key，pOutHmac：HMAC
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_SignGetHMACKey(EVP_PKEY** pKeySign, unsigned char *pOutHmac)
{
    unsigned char pHmac[EVP_MAX_MD_SIZE] = {0};
    int iLen = EVP_MAX_MD_SIZE;
    if(pKeySign == NULL)
    {   
        return 1;
    }
    
    if(*pKeySign != NULL) 
    {
        EVP_PKEY_free(*pKeySign);
        *pKeySign = NULL;
    }

    const EVP_MD* md = EVP_get_digestbyname(hn);
    if(md == NULL) 
    {
        KMC_LOG("EVP_get_digestbyname failed, error 0x%lx\n", ERR_get_error());
        OPENSSL_cleanse(pHmac, iLen);
        return 1; /* failed */
    }
    
    int iSize = EVP_MD_size(md);
    int iRet = RAND_bytes(pHmac, iSize);
    if(iRet != 1) 
    {
        KMC_LOG("RAND_bytes failed, error 0x%lx\n", ERR_get_error());
        OPENSSL_cleanse(pHmac, iLen);
        return 1; /* failed */
    }
    
    int i = 0;
    for(i=0; i < iSize; ++i)
    {
        pOutHmac[i] = pHmac[i];
    }
    
    *pKeySign = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, pHmac, iSize);
    if(*pKeySign == NULL) 
    {
        KMC_LOG("EVP_PKEY_new_mac_key failed, error 0x%lx\n", ERR_get_error());
        OPENSSL_cleanse(pHmac, iLen);
        return 1; /* failed */
    }
    
    OPENSSL_cleanse(pHmac, iLen);
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC对文件签名
Input        : pFileName：文件，pSignKey：签名Key
Output       : pSignData：密文，iLen：密文长度
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_SignFile(const char *pFileName, unsigned char *pSignData, int* iLen, EVP_PKEY* pSignKey)
{
    if (NULL == pSignData || NULL == pSignKey)
    {
        return 1;
    }
    
    char pBuff[SIGN_DATA_LEN_MAX] = {0};
    *iLen = 0;
    EVP_MD_CTX* ctx = NULL;
#ifndef WIN32
    char pszFilePath[PATH_MAX + 1] = {0};   
    if (strlen(pFileName) > PATH_MAX || NULL == realpath(pFileName, pszFilePath))
    {
        KMC_LOG("realpath file(%s) failed..., errno: %d, %s\n", pFileName, errno, strerror(errno));
        return 1;
    }
#else
    const char *pszFilePath = pFileName;
#endif
    FILE* fp = fopen(pszFilePath, "rb");
    if (!fp)
    {
        KMC_LOG("Open file(%s) failed..., errno: %d, %s\n", pszFilePath, errno, strerror(errno));
        return 1;
    }
    
    ctx = EVP_MD_CTX_create();
    if(ctx == NULL) 
    {
        KMC_LOG("EVP_MD_CTX_create failed, error 0x%lx\n", ERR_get_error());
        fclose(fp);
        return 1;
    }
    
    const EVP_MD* md = EVP_get_digestbyname(hn);
    if(md == NULL) 
    {
        KMC_LOG("EVP_get_digestbyname failed, error 0x%lx\n", ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    int iRet = EVP_DigestInit_ex(ctx, md, NULL);
    if(iRet != 1) 
    {
        KMC_LOG("EVP_DigestInit_ex failed, error 0x%lx\n", ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    iRet = EVP_DigestSignInit(ctx, NULL, md, NULL, pSignKey);
    if(iRet != 1) 
    {
        KMC_LOG("EVP_DigestSignInit failed, error 0x%lx\n", ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    while (*iLen = fread (pBuff, 1, SIGN_DATA_LEN_MAX, fp))
    {
        iRet = EVP_DigestSignUpdate(ctx, pBuff, *iLen);
        if(iRet != 1) 
        {
            KMC_LOG("EVP_DigestSignUpdate failed, error 0x%lx\n", ERR_get_error());
            EVP_MD_CTX_destroy(ctx);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);

    iRet = EVP_DigestSignFinal(ctx, pSignData, (size_t*)iLen);
    if(iRet != 1) 
    {
        KMC_LOG("EVP_DigestSignFinal failed (3), return code %d, error 0x%lx\n", iRet, ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        return 1;
    }
    EVP_MD_CTX_destroy(ctx);
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC获取签名key
Input        : pInHmac：HMAC
Output       : pKeySign：校验签名Key
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_SignGetVerifyKey(EVP_PKEY** pKeySign, unsigned char *pInHmac)
{
    if(NULL == pKeySign || NULL == pInHmac)
    {   
        return 1;
    }
    
    if(*pKeySign != NULL) 
    {
        EVP_PKEY_free(*pKeySign);
        *pKeySign = NULL;
    }

    *pKeySign = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, pInHmac, FILE_MAC_LEN);
    if(*pKeySign == NULL) 
    {
        KMC_LOG("EVP_PKEY_new_mac_key failed, error 0x%lx\n", ERR_get_error());
        return 1; /* failed */
    }
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC校验文件签名
Input        : pFileName：文件，pSignData：密文，iLen：密文长度，pSignKey：Key
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_VerifyFile(const char *pFileName, unsigned char *pSignData, int iLen, EVP_PKEY* pSignKey)
{
    if(!pSignData || !iLen || !pSignKey) 
    {
        return -1;
    }
    char pBuff[SIGN_DATA_LEN_MAX] = {0};
    int iReadSize = SIGN_DATA_LEN_MAX;
    EVP_MD_CTX* ctx = NULL;
#ifndef WIN32
    char pszFilePath[PATH_MAX + 1] = {0};   
    if (strlen(pFileName) > PATH_MAX || NULL == realpath(pFileName, pszFilePath))
    {
        KMC_LOG("realpath file(%s) failed..., errno: %d, %s\n", pFileName, errno, strerror(errno));
        return 1;
    }
#else
    const char *pszFilePath = pFileName;
#endif
    FILE* fp = fopen(pszFilePath, "rb");
    if (!fp)
    {
        KMC_LOG("Open file failed..., errno: %d, %s\n", errno, strerror(errno));
        exit(1);
    }
    
    ctx = EVP_MD_CTX_create();
    if(ctx == NULL) 
    {
       KMC_LOG("EVP_MD_CTX_create failed, error 0x%lx\n", ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    const EVP_MD* md = EVP_get_digestbyname(hn);
    if(md == NULL) 
    {
        KMC_LOG("EVP_get_digestbyname failed, error 0x%lx\n", ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    int iRet = EVP_DigestInit_ex(ctx, md, NULL);
    if(iRet != 1) 
    {
        KMC_LOG("EVP_DigestInit_ex failed, error 0x%lx\n", ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    iRet = EVP_DigestSignInit(ctx, NULL, md, NULL, pSignKey);
    if(iRet != 1) 
    {
        KMC_LOG("EVP_DigestSignInit failed, error 0x%lx\n", ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    while (iReadSize = fread (pBuff, 1, SIGN_DATA_LEN_MAX, fp))
    {
        iRet = EVP_DigestSignUpdate(ctx, pBuff, iReadSize);
        if(iRet != 1) 
        {
            KMC_LOG("EVP_DigestSignUpdate failed, error 0x%lx\n", ERR_get_error());
            EVP_MD_CTX_destroy(ctx);
            fclose(fp);
            return 1;
        }
    }
    unsigned char pSignBuff[EVP_MAX_MD_SIZE];
    size_t size = sizeof(pSignBuff);
    
    iRet = EVP_DigestSignFinal(ctx, pSignBuff, &size);
    if(iRet != 1) 
    {
        KMC_LOG("EVP_DigestVerifyFinal failed, error 0x%lx\n", ERR_get_error());
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    if(size <= 0) 
    {
        KMC_LOG("EVP_DigestSignFinal failed (2)\n");
        EVP_MD_CTX_destroy(ctx);
        fclose(fp);
        return 1;
    }
    
    const size_t m = (iLen < size ? iLen : size);
    iRet = CRYPTO_memcmp(pSignData, pSignBuff, m);
    OPENSSL_cleanse(pSignBuff, sizeof(pSignBuff));

    fclose(fp);
    return iRet;
}

/*------------------------------------------------------------ 
Description  : KMC签名对外接口
Input        : filePath：文件
Output       : fileHMAC：签名密文
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Gen_File_HMAC(const string &filePath, string &fileHMAC)
{
    fileHMAC = "";
    if (filePath.length() == 0)
    {
        return 1;
    }
    
    OpenSSL_add_all_algorithms();
    EVP_PKEY *pKeySign = NULL;
    int iLen = FILE_MAC_LEN;
    unsigned char pOutHmac[EVP_MAX_MD_SIZE] = {0};
    unsigned char pSignData[100] = {0};
    
    int iRet = KMC_SignGetHMACKey(&pKeySign, pOutHmac);
    if (iRet != 0)
    {
        KMC_LOG("Get the Key failed!\n");
        return 1;
    }
    string strHMAC;
    iRet = strToHex(pOutHmac, strHMAC, iLen);
    if (iRet != 0)
    {
        KMC_LOG("Get the MAC failed!\n");
        return 1;
    }
    iLen = 0;
    iRet = KMC_SignFile(filePath.c_str(), (unsigned char*)pSignData, &iLen, pKeySign);
    if (iRet != 0)
    {
         KMC_LOG("Sign file failed!\n");
        return 1;
    }
    
    string strSignData;
    iRet = strToHex(pSignData, strSignData, iLen);
    if (iRet != 0)
    {
        KMC_LOG("Get the MAC failed!\n");
        return 1;
    }
    
    fileHMAC = strHMAC + strSignData;
    return 0;
}

/*------------------------------------------------------------ 
Description  : KMC校验签名对外接口
Input        : filePath：文件，fileHMAC：签名密文
Output       : 
Return       : 1：失败 0：成功
Create By    :
Modification : 
-------------------------------------------------------------*/
int KMC_Verify_File_HMAC(const string &filePath, const string &fileHMAC)
{
    if (filePath.length() == 0 || fileHMAC.length() == 0)
    {
        return 1;
    }
    int hmacLen = fileHMAC.length();
    if (hmacLen % 2 == 1)
    {
        return 1;
    }
    hmacLen /= 2;
    if (hmacLen != FILE_SIGN_DATA_MAX)
    {
        KMC_LOG("hmacLen(%d) is error.", hmacLen);
        return 1;
    }
    //CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    unsigned char *hmacBuf = new unsigned char[hmacLen];
    if (hmacBuf == NULL)
    {
        KMC_LOG("New buf failed.");
        return 1;
    }

    int iRet = hexToStr(fileHMAC, hmacBuf, hmacLen);
    if (iRet != 0)
    {
        KMC_LOG("Convert Hex str to str failed.");
        delete [] hmacBuf;
        return 1;
    }

    unsigned char *pvHmacData = new unsigned char[FILE_MAC_LEN];
    if (pvHmacData == NULL)
    { 
        KMC_LOG("New buf failed.");
        delete [] hmacBuf;
        return 1;
    }
    iRet = memcpy_s(pvHmacData, FILE_MAC_LEN, hmacBuf, FILE_MAC_LEN);
    if (iRet != 0)
    {
        KMC_LOG("Call memcpy_s failed, ret = %d.", iRet);
        delete [] hmacBuf;
        delete [] pvHmacData;
        return 1;
    }
    int iSignLen = hmacLen - FILE_MAC_LEN;
    if (iSignLen != (FILE_SIGN_DATA_MAX - FILE_MAC_LEN))
    {
        KMC_LOG("Sign data len(%d) is error.", iSignLen);
        delete [] hmacBuf;
        delete [] pvHmacData;
        return 1;
    }
    unsigned char* pszSignData = new unsigned char[iSignLen];
    if (pszSignData == NULL)
    { 
        KMC_LOG("New buf failed.");
        delete [] hmacBuf;
        delete [] pvHmacData;
        return 1;
    }
    iRet = memcpy_s(pszSignData, iSignLen, hmacBuf + FILE_MAC_LEN, iSignLen); //lint !e662
    if (iRet != 0)
    {
        KMC_LOG("Call memcpy_s failed, ret = %d.", iRet);
        delete [] hmacBuf;
        delete [] pvHmacData;
        delete [] pszSignData;
        return 1;
    }
    
    OpenSSL_add_all_algorithms();
    EVP_PKEY *pKeySign = NULL;
    iRet = KMC_SignGetVerifyKey(&pKeySign, pvHmacData);
    if(iRet != 0) 
    {   
        delete [] hmacBuf;
        delete [] pvHmacData;
        delete [] pszSignData;
        KMC_LOG("KMC_SignGetVerifyKey failed, ret = %d.", iRet);
        return 1;
    }
    iRet = KMC_VerifyFile(filePath.c_str(), pszSignData, iSignLen, pKeySign);
    delete [] hmacBuf;
    delete [] pvHmacData;
    delete [] pszSignData;
    if(iRet != 0) 
    {
        KMC_LOG("Verify file HMAC failed, ret = %d.", iRet);
        return 1;
    }
    
    return 0;
}


