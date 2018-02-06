/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/StringTest.h"
#include "common/String.h"
#include "common/Log.h"

#include "stub.h"

#include <cstdio>
#include <list>
#include <string>
using namespace std;

TEST_F(CMpStringTest, TrimTest){

  mp_char testString[200] = " h l ,l ee w x  x  ";
  mp_char *iRet = CMpString::Trim(testString);
  EXPECT_STREQ(iRet, "h l ,l ee w x  x");
  
  iRet = CMpString::Trim(NULL);
  EXPECT_EQ((mp_char*)NULL, iRet);
  
  strcpy_s(testString, 200, "ni hao!  ");
  iRet = CMpString::Trim(testString);
  EXPECT_EQ(testString, iRet);
  EXPECT_STREQ(iRet, "ni hao!");
  
  strcpy_s(testString, 200, "     ni hao!");
  iRet = CMpString::Trim(testString);
  EXPECT_EQ(testString, iRet);
  EXPECT_STREQ(iRet, "ni hao!");
}

TEST_F(CMpStringTest, TotallyTrimRightTest){
  mp_char testString[200] = " h l ,l ee w x  x  \n\r\t ";
  mp_char *iRet = CMpString::TotallyTrimRight(testString);
  EXPECT_EQ(iRet, testString);
  EXPECT_STREQ(iRet, " h l ,l ee w x  x");
  
  iRet = CMpString::TotallyTrimRight("");
  EXPECT_EQ(iRet, (mp_char*)NULL);
}

TEST_F(CMpStringTest, FormatLUNIDTest){
  mp_string LunID = "0000";
  mp_string expectOut;
  CMpString::FormatLUNID(LunID, expectOut);
  EXPECT_EQ("0", expectOut);
  
  LunID = "0";
  CMpString::FormatLUNID(LunID, expectOut);
  EXPECT_EQ("0", expectOut);
  
  LunID = "103579";
  CMpString::FormatLUNID(LunID, expectOut);
  EXPECT_EQ("103579", expectOut);
  
  LunID = "0103579";
  CMpString::FormatLUNID(LunID, expectOut);
  EXPECT_EQ("103579", expectOut);
}

TEST_F(CMpStringTest, HasSpaceTest){
  mp_char testString[200] = " h l ,l ee w x  x  \n\r\t ";
  mp_bool iRet = CMpString::HasSpace(testString);
  EXPECT_EQ(MP_TRUE, iRet);
  
  strcpy_s(testString, 200, "nihao!");
  iRet = CMpString::HasSpace(testString);
  EXPECT_EQ(MP_FALSE, iRet);
  
  iRet = CMpString::HasSpace(NULL);
  EXPECT_EQ(MP_FALSE, iRet);
}

TEST_F(CMpStringTest, ToUpperTest_ToLowerTest){
  
  mp_char testString[] = "Hello, WoRLD!  !";
  mp_char *iRetUpper = CMpString::ToUpper(testString);
  EXPECT_EQ(testString, iRetUpper);
  EXPECT_STREQ(iRetUpper, "HELLO, WORLD!  !");
  
  mp_char *iRetLower = CMpString::ToLower(testString);
  EXPECT_EQ(testString, iRetLower);
  EXPECT_STREQ(iRetLower, "hello, world!  !");
}

TEST_F(CMpStringTest, StrTokenTest){
  mp_string token = "AB:CD:EF:08:x5";
  mp_string separator = ":";
  list<mp_string> lStr;
  CMpString::StrToken(token, separator, lStr);
  ASSERT_EQ(lStr.size(), 5);
  list<mp_string>::iterator it = lStr.begin();
  EXPECT_EQ(*(it++), "AB");
  EXPECT_EQ(*(it++), "CD");
  EXPECT_EQ(*(it++), "EF");
  EXPECT_EQ(*(it++), "08");
  EXPECT_EQ(*(it++), "x5");
  EXPECT_EQ(it, lStr.end());
}

TEST_F(CMpStringTest, StrSplitTest){
  mp_string token = "AB:CD:EF:08:x5";
  mp_char separator = ':';
  vector<mp_string> lStr;
  CMpString::StrSplit(lStr, token, separator);
  ASSERT_EQ(lStr.size(), 5);
  EXPECT_EQ(lStr[0], "AB");
  EXPECT_EQ(lStr[1], "CD");
  EXPECT_EQ(lStr[2], "EF");
  EXPECT_EQ(lStr[3], "08");
  EXPECT_EQ(lStr[4], "x5");
}

TEST_F(CMpStringTest, BlankCommaTest){
  mp_string path = "test123";
  mp_string sRet = CMpString::BlankComma(path);
  EXPECT_EQ(path, sRet);
  
  path = "test 123";
  sRet = CMpString::BlankComma(path);
  EXPECT_EQ("\""+path+"\"", sRet);
}

