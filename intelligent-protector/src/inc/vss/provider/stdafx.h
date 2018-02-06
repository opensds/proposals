/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
#ifdef WIN32

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "resource.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的
#define _ATL_ALL_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
using namespace ATL;

#include <new>
#include <string>
#include <vector>
#include <map>

#include "vss.h"
#include "vsprov.h"
#include "vscoordint.h"

// version
#include <ntverp.h>

#endif
