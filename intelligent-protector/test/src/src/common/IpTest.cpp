/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/IpTest.h"
#include "common/ErrorCode.h"
#include "common/Ip.h"
/*------------------------------------------------------------ 
 Description :测试CIPCheck::IsIPV4(mp_string&)
 Input         :   
 Output       :    
 Return       : 
 Create By    :
Modification : 
-------------------------------------------------------------*/

TEST_F(IPTest, IsIPV4TEST){
  mp_string strIpAddr = "";
  mp_bool ret = true;

  //空字符串
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);

  //全零字符串
  strIpAddr = "0.0.0.0";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);

  //某个段超过255
  strIpAddr = "127.0.3.256";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);

  //存在其它字符
  strIpAddr = "1o9.25.31.4l";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);

  //非ipv4类型格式
  strIpAddr = "10:2:3:5";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);
  
  //正常字符串
  strIpAddr = "10.2.3.5";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_TRUE, ret);
}
