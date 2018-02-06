/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef ___AGENT_PATH_TEST_H__
#define ___AGENT_PATH_TEST_H__
#include "common/Path.h"
#include "gtest/gtest.h"
#include <iostream>
class PathTest: public testing::Test {
protected:
  PathTest(){}
  ~PathTest(){}
  static void SetUpTestCase(){
    std::cout << "SetUpTestCase()" << std::endl;
  }
  static void TearDownTestCase(){
    std::cout << "TearDownTestCase()" << std::endl;
  }
  void SetUp(){
    std::cout << "SetUp()" << std::endl;
  }
  void TearDown(){
    std::cout << "TearDown()" << std::endl;
  }
};
#endif
