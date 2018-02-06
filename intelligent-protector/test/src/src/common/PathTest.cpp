/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/PathTest.h"
#include "stub.h"
#include "common/ErrorCode.h"
#include "securec.h"

#include <libgen.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

static char *StubTestDirname(char * path){
  return (char*)"/home/test/Agent/bin";
}

/*------------------------------------------------------------
Function Name: 
Description  : 这个测试用例只是一个stub的测试示例
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
TEST_F(PathTest, JustAnExampleForStub){
  try{
    typedef char* (*DirType)(char *);
    Stub<DirType, DirType, void> stub(dirname, StubTestDirname);
    CPath &path = CPath::GetInstance();
    mp_char str[] = "/home/test/Agent/bin/rdagent";
    path.Init(str);
    mp_string expect = "/home/test/Agent";
    EXPECT_EQ(expect, path.GetRootPath());
  }catch(...){
    printf("Error on %s file %d line.\n", __FILE__, __LINE__);
    exit(0);
  }
}

/*------------------------------------------------------------
Function Name: 
Description  : 对CPath::Init(mp_char *)函数进行测试
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
TEST_F(PathTest, InitHomeRdadminAgent){
  CPath &path = CPath::GetInstance();

  mp_char str[] = "/home/rdadmin/Agent/bin/rdagent";
  int ret = 0;
  ret = path.Init(str);
  mp_string expect = "/home/rdadmin/Agent";
  EXPECT_EQ(ret, MP_SUCCESS);
  EXPECT_EQ(expect, path.GetRootPath());
  expect = "/home/rdadmin/Agent/bin";
  EXPECT_EQ(expect, path.GetBinPath());
}

TEST_F(PathTest, InitException){
  CPath &path = CPath::GetInstance(); 
  int ret = 0;

  ret = path.Init(NULL);
  EXPECT_EQ(ret, ERROR_COMMON_OPER_FAILED);

  mp_char str[] = "";
  ret = path.Init(str);
  EXPECT_EQ(ret, ERROR_COMMON_OPER_FAILED);
}

