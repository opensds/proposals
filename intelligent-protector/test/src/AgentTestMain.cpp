/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "gtest/gtest.h"
#include "stub.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

GTEST_API_ int main(int argc, char *argv[]) {
  cout << "Running main() from agent_unit_test.cpp\n";
  try{
    Stub<void*,void*,void>::Init();
  }catch(...){
      printf("Stub::Init() failed on %s file in %d line.\n", __FILE__, __LINE__);
      return EXIT_FAILURE;
  }
  testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  Stub<void*,void*,void>::Destroy();
  return ret;
}
