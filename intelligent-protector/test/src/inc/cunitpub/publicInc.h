/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_INIT_GLOBAL_
#define _AGENT_INIT_GLOBAL_

#include <sstream>

#include <unistd.h>
#include "gtest/gtest.h"

#define private public

#include "stub.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/Sign.h"
#include "common/ErrorCode.h"
#include "common/Password.h"
#include "common/SystemExec.h"
#include "common/ConfigXmlParse.h"

#include "rest/Interfaces.h"

typedef mp_void (*pOrgGetInput)(mp_string strHint, mp_string& strInput, mp_int32 iInputLen);
typedef mp_void (*StubGetInputType)(mp_string strHint, mp_string &strInput);
typedef mp_int32 (*pStubIntType)(mp_void);
typedef mp_bool (*pStubBoolType)(mp_void);
typedef mp_string (*pStubStringType)(mp_void);
typedef const mp_char * (*pStubCstringType)(mp_void);
typedef mp_void (*pStubVoidType)(mp_void);
typedef mp_int32 (*pStubRetType)(mp_void);


template<typename inputType>
inline mp_string toString(inputType val)
{
    mp_string strval; 
    std::ostringstream oss;
    
    oss << val;
    strval = oss.str();
    
    return strval;
}

mp_void stub_set_cpasswdString(mp_string strHint, mp_string& strInput);
mp_void stub_set_cpasswdLongString(mp_string strHint, mp_string& strInput);
mp_void stub_set_string(mp_string &strInput);
mp_void stub_set_numberStr(mp_string &strInput);
mp_bool stub_return_bool(mp_void);
mp_int32 stub_return_ret(mp_void);
mp_int32 stub_return_number(mp_void);
mp_string stub_return_string(mp_void);
const mp_char * stub_return_cstring(mp_void);
mp_void stub_return_nothing(mp_void);

mp_void reset_cunit_counter(mp_void);
mp_void init_cunit_data();
mp_void destroy_cunit_data();

#endif /* _AGENT_INIT_GLOBAL_; */



