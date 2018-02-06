/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_TYPES_H_
#define _AGENT_TYPES_H_

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <limits.h>
#include <netinet/in.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

//布尔值mp_bool取值
#define MP_TRUE       1
#define MP_FALSE      0
//函数返回值，如果需要其他特定返回值，各函数方法自己定义
#define MP_SUCCESS    0
#define MP_FAILED     -1

typedef void                  mp_void;
typedef int                   mp_bool;
typedef float                 mp_float;
typedef double                mp_double;
typedef char                  mp_char;
typedef unsigned char         mp_uchar;
typedef int                   mp_int32;
typedef unsigned int          mp_uint32;
typedef short                 mp_int16;
typedef unsigned short        mp_uint16;
typedef long                  mp_long;
typedef unsigned long         mp_ulong;
typedef string                mp_string;
typedef wstring               mp_wstring;
typedef size_t                mp_size;
typedef time_t                mp_time;
typedef tm                    mp_tm;

#ifdef WIN32
typedef HMODULE               mp_handle_t;
typedef __int64               mp_int64;
typedef unsigned __int64      mp_uint64;
typedef WCHAR                 mp_wchar;
#else
typedef long long             mp_int64;
typedef unsigned long long    mp_uint64;
typedef wchar_t               mp_wchar;
typedef void *                mp_handle_t;
#endif

#endif   //_AGENT_TYPES_H_

