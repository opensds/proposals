/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/LogTest.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"

TEST_F(CLoggerTest, CLoggerSetLogLevelTest){
  mp_int32 level = -1;
  CLogger& log = CLogger::GetInstance();
  
  //LogLevel < 0
  mp_int32 iRet = log.SetLogLevel(level);
  EXPECT_EQ(iRet, MP_FAILED);
  //LogLevel > 4
  level = 5;
  iRet = log.SetLogLevel(level);
  EXPECT_EQ(iRet, MP_FAILED);
  //LogLevel = 2
  level = 2;
  iRet = log.SetLogLevel(level);
  EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CLoggerTest, CLoggerSetLogCountTest){
  mp_int32 logCount = 0;
  CLogger& log = CLogger::GetInstance();
  
  //logCount <= 0
  mp_int32 iRet = log.SetLogCount(logCount);
  EXPECT_EQ(iRet, MP_FAILED);
  
  //logCount > 0
  logCount = 2;
  iRet = log.SetLogCount(logCount);
  EXPECT_EQ(iRet, MP_SUCCESS);
}

typedef mp_int32 (CConfigXmlParser::*TypeGetValueInt32)(mp_string, mp_string, mp_int32&);
typedef mp_int32 (*TypeStubGetValueInt32)(void*This, mp_string strSection, mp_string strKey, mp_int32& iValue);
static mp_int32 StubGetValueInt32_1(void*This, mp_string strSection, mp_string strKey, mp_int32& iValue){
  static int i = -1;
  i++;
  if(i == 0){//第一次调用返回失败
    return ERROR_COMMON_READ_CONFIG_FAILED;
  }else if(i == 1){//第二次返回成功
    iValue = 2;
    return MP_SUCCESS;
  }else if(i == 2){//第三次返回失败
    return ERROR_COMMON_READ_CONFIG_FAILED;
  }else{
    iValue = 2;
    return MP_SUCCESS;
  }
  return ERROR_COMMON_READ_CONFIG_FAILED;
}

TEST_F(CLoggerTest, CLoggerReadLevelAndCountTest){
  Stub<TypeGetValueInt32, TypeStubGetValueInt32, void> stub(&CConfigXmlParser::GetValueInt32, StubGetValueInt32_1);
  CLogger& log = CLogger::GetInstance();
  log.ReadLevelAndCount();
  log.ReadLevelAndCount();
  log.ReadLevelAndCount();
}

TEST_F(CLoggerTest, MkHead){
    mp_int32 ret;
    mp_int32 iLevel = 0;
    mp_char pszHeadBuf[10] = {0};
    mp_int32 iBufLen = 10;
    ret = CLogger::GetInstance().MkHead(iLevel,pszHeadBuf,iBufLen);
    
    iLevel = 1;
    ret = CLogger::GetInstance().MkHead(iLevel,pszHeadBuf,iBufLen);
    
    iLevel = 2;
    ret = CLogger::GetInstance().MkHead(iLevel,pszHeadBuf,iBufLen);
    
    iLevel = 3;
    ret = CLogger::GetInstance().MkHead(iLevel,pszHeadBuf,iBufLen);
    
    iLevel = 4;
    ret = CLogger::GetInstance().MkHead(iLevel,pszHeadBuf,iBufLen);
    
    iLevel = 5;
    ret = CLogger::GetInstance().MkHead(iLevel,pszHeadBuf,iBufLen);
}

TEST_F(CLoggerTest, SwitchLogFile){
    mp_int32 ret;
    mp_char* pszLogPath = "test";
    mp_char* pszLogName = "test";
    mp_int32 iLogCount = 10;
    
    ret = CLogger::GetInstance().SwitchLogFile(pszLogPath,pszLogName,iLogCount);
}
/* this function have been removed by DTS2016053009096
TEST_F(CLoggerTest, BaseFileName){
    mp_int32 ret;
    mp_char* pszFileName = "test";
    
    CLogger::GetInstance().BaseFileName(pszFileName);
}
*/

TEST_F(CLoggerTest, OpenLogCache){
    CLogger::GetInstance().OpenLogCache();
}

TEST_F(CLoggerTest, CloseLogCache){
    CLogger::GetInstance().CloseLogCache();
}

TEST_F(CLoggerTest, WriteLog2Cache){
	ostringstream strMsg;

    CLogger::GetInstance().WriteLog2Cache(strMsg);
}

TEST_F(CLoggerTest, OpenLogFile){
    mp_int32 ret;
    
    CLogger::GetInstance().OpenLogFile();
}

TEST_F(CLoggerTest, Log){
    mp_int32 ret;
    mp_int32 iLevel = 1;
    mp_int32 iFileLine = 1;
    mp_uint64 ulCode = 1;
    mp_char pszFileName = 't';
    mp_char pszFormat = 't';
    
    Stub<ReadLevelAndCountType, StubReadLevelAndCountType, mp_void> mystub1(&CLogger::ReadLevelAndCount, &StubReadLevelAndCount);
    Stub<ReadLevelAndCountType, StubReadLevelAndCountType, mp_void> mystub2(&CLogger::ReadLogCacheThreshold, &StubReadLevelAndCount);
    CLogger::GetInstance().Log(iLevel,iFileLine,ulCode,&pszFileName,&pszFormat);
}
