/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/CryptAlgTest.h"
#include "stub.h"
#include "common/Log.h"
#include "common/Types.h"

typedef mp_void (*StubFuncType)(void);
typedef mp_void (CLogger::*LogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);

static mp_void StubCLoggerLog(void){
    return;
}

//加密解密测试
TEST_F(CryptAlgTest, AESEncryptAndDescryptTest){
  try{
    Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);
    //空字符串
    mp_string encryptOut;
    mp_string descryptOut;
    const int encryptOriginalSize = 280;
    char encryptOriginal[encryptOriginalSize] = "";
    AES_encrypt(encryptOriginal, sizeof(encryptOriginal), encryptOut);
    
    AES_decrypt(encryptOut.c_str(), encryptOut.size(), descryptOut);

    EXPECT_STREQ(encryptOriginal, descryptOut.c_str());
    encryptOut.clear();
    descryptOut.clear();
    
    //"awqhdioq%^%wqjdhIODHOI"
    int iret = strcpy_s(encryptOriginal, encryptOriginalSize, "awqhdioq%^%wqjdhIODHOI");
    if(iret != EOK){
      printf("in %s %d line strcpy_s error!\n", __FILE__, __LINE__);
      exit(0);
    }
    AES_encrypt(encryptOriginal, sizeof(encryptOriginal), encryptOut);
    AES_decrypt(encryptOut.c_str(), encryptOut.size(), descryptOut);
    EXPECT_STREQ(encryptOriginal, descryptOut.c_str());

    //第一个参数为NULL
    AES_encrypt(NULL, 0, encryptOut);
    EXPECT_EQ(0, encryptOut.size());

    //第二个参数为0
    AES_encrypt(encryptOriginal, 0, encryptOut);
    EXPECT_EQ(0, encryptOut.size());

    descryptOut.clear();
    AES_decrypt(NULL, 45, descryptOut);
    EXPECT_EQ(0, descryptOut.size());
    
  }catch(...){
    printf("in %s file %d line catch some error!\n", __FILE__, __LINE__);
    exit(0);
  }
}

TEST_F(CryptAlgTest,  GetMinLenTest){
  int len;
  int ret;

  size_t getMinLen(size_t len);
  
  len = 1;
  ret = getMinLen(len);
  EXPECT_EQ(48, ret);

  len = 49;
  ret = getMinLen(len);
  EXPECT_EQ(64, ret);
}

TEST_F(CryptAlgTest, CheckDecryptLenTest){
  int len;
  mp_bool ret;

  mp_bool checkDecryptLen(size_t len);
  
  len = 32;
  ret = checkDecryptLen(len);
  EXPECT_EQ(ret, false);

  len = 64;
  ret = checkDecryptLen(len);
  EXPECT_EQ(ret, true);

  len = 65;
  ret = checkDecryptLen(len);
  EXPECT_EQ(ret, false);
}

TEST_F(CryptAlgTest, HexStr2ASCIITest){
  try{
    Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);

    mp_int32 ret;
    mp_uchar ASCIIStr[200];

    mp_int32 HexStr2ASCII(const mp_uchar *hexStr, mp_uchar *ASCIIStr, size_t hexLen, size_t ASCIILen);

    mp_uchar hexStr[10]="381";

    ret = HexStr2ASCII(hexStr, ASCIIStr, 3, 200);
    EXPECT_EQ(ret, MP_FAILED);

    ret = HexStr2ASCII(NULL, ASCIIStr, 0, 200);
    EXPECT_EQ(ret, MP_FAILED);

    
    ret = HexStr2ASCII(ASCIIStr, NULL, 0, 200);
    EXPECT_EQ(ret, MP_FAILED);
  }catch(...){
     printf("in %s file %d line catch some error!\n", __FILE__, __LINE__);
     exit(0);
  }
}

TEST_F(CryptAlgTest, ASCII2HexStrTest){
  try{

    mp_int32 ASCII2HexStr(const mp_uchar *ASCIIStr, mp_uchar* hexStr, size_t len, size_t hexLen);
    
    Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);

    mp_uchar ASCIIStr[50], hexStr[50];

    mp_int32 ret = ASCII2HexStr(NULL, hexStr, 0, 50);
    EXPECT_EQ(ret, MP_FAILED);

    memcpy_s(ASCIIStr, 50, "hello!", 6);
    ret = ASCII2HexStr(ASCIIStr, NULL, 6, 0);
    EXPECT_EQ(ret, MP_FAILED);

  }catch(...){
     printf("in %s file %d line catch some error!\n", __FILE__, __LINE__);
     exit(0);
  }
}

int stubOpenError(void){
    return -1;
}

ssize_t stubReadError(void){
    return -1;
}

TEST_F(CryptAlgTest, GetRandomTest){
  mp_uint64 number;
  mp_int32 ret;

  Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);
  
  ret = GetRandom(number);
  EXPECT_EQ(ret, MP_SUCCESS);

  do{
    typedef int (*orignalType)(const char *, int, ...);
    typedef int (*stubType)(void);
    Stub<orignalType, stubType, void> stub(open, stubOpenError);
    ret = GetRandom(number);
    EXPECT_EQ(ret, MP_FAILED);
  }while(0);

  do{
    typedef ssize_t (*orignalType)(int fd, void *buf, size_t count);
    typedef ssize_t (*stubType)(void);
    Stub<orignalType, stubType, void> stub(read, stubReadError);
    ret = GetRandom(number);
    EXPECT_EQ(ret, MP_FAILED);
  }while(0);
}

TEST_F(CryptAlgTest, PBKDF2HashTest){
  mp_string strPlainText = "12345678901234567890123456789012345678901234567890";
  mp_string strSalt = "12345678901234567890123456789012345678901234567890";
  mp_string strCipherText;

  Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);
  
  int ret = PBKDF2Hash(strPlainText, strSalt, strCipherText);
  EXPECT_EQ(ret, MP_FAILED);

  strSalt = "123456789012345678901234567890123456789";
  ret = PBKDF2Hash(strPlainText, strSalt, strCipherText);
  EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(CryptAlgTest, GetSha256HashTest){
  mp_char outHashHex[SHA256_BLOCK_SIZE + 1] = {0};
  mp_string strInput = "qgswhoqwhsowhcqhwqjsdwiohsshw";
  
  mp_int32 ret = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, sizeof(outHashHex));

  EXPECT_EQ(MP_SUCCESS, ret);
}

