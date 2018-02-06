/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/MacTest.h"
#include "common/Mac.h"
#include "common/Log.h"

#include "stub.h"

typedef mp_void (*StubLogType)(void);
typedef mp_void (CLogger::*LogType)(mp_int32, const mp_int32, 
        mp_uint64, const mp_char*, const mp_char*, ...);

static mp_void StubCLoggerLog(void){
  return;
}

TEST_F(CMacAddrTest, GetAllLocalMacAddrTest){
  Stub<LogType, StubLogType, void> stubLog(&CLogger::Log, StubCLoggerLog);
  vector<mp_string> mac;
  int iRet = CMacAddr::GetAllLocalMacAddr(mac);
  
  EXPECT_EQ(iRet, MP_SUCCESS);
  
  //ASSERT_EQ(mac.size(), 1); //测试主机只有一个网卡
  //EXPECT_EQ(mac[0], "00-50-56-B1-0A-23");
}
