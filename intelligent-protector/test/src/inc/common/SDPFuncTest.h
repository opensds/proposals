/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _SIGNTEST_H_
#define _SIGNTEST_H_

#define private public

#include <string>
#include <sstream>
#include <iomanip>
#include "common/Log.h"
#include "common/Path.h"
#include "include/wsec_type.h"
#include "src/sdp/sdp_itf.h"
#include "include/kmc_itf.h"
#include "common/SDPFunc.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubSDPFuncLogVoid(mp_void* pthis);

class SDPFuncTest: public testing::Test{
protected:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubSDPFuncLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* SDPFuncTest::m_stub;
//*******************************************************************************
typedef WSEC_ERR_T (*SDP_GetCipherDataLenType)(mp_uint32 inLen,mp_uint32 *outLen);
typedef WSEC_ERR_T (*StubSDP_GetCipherDataLenType)(mp_uint32 inLen,mp_uint32 *outLen);


//mp_int32 ret1 = memset_s(inBuf, inLen, 0, inLen);
/*Typedef  mp_int32 (*memset_sType)(WSEC_BYTE *inBuf,mp_uint32 inLen1,int inLen2,mp_uint32 inLen3);
typedef  mp_int32 (*StubSDP_memset_sType)(WSEC_BYTE *inBuf,mp_uint32 inLen1,int inLen2,mp_uint32 inLen3);*/


//qinlang
typedef WSEC_ERR_T (*SDP_GetHmacAlgAttrType)(WSEC_UINT32 len,SDP_HMAC_ALG_ATTR* stHmacAlgAttr);
typedef WSEC_ERR_T (*StubSDP_GetHmacAlgAttrType)(WSEC_UINT32 len,SDP_HMAC_ALG_ATTR* stHmacAlgAttr);

typedef WSEC_ERR_T (*SDP_EncryptType)(WSEC_UINT32 len,const WSEC_BYTE *inBuf,WSEC_UINT32 inLen,WSEC_BYTE *outBuf,WSEC_UINT32 *outLen);
typedef WSEC_ERR_T (*StubSDP_EncryptType)(WSEC_UINT32 len,const WSEC_BYTE *inBuf,WSEC_UINT32 inLen,WSEC_BYTE *outBuf,WSEC_UINT32 *outLen);

typedef WSEC_ERR_T (*SDP_FileHmacType)(WSEC_UINT32 len, const WSEC_CHAR* filePath,const WSEC_PROGRESS_RPT_STRU* i,const SDP_HMAC_ALG_ATTR *stHmacAlgAttr,WSEC_VOID* pvHmacData, WSEC_UINT32 *ulHDLen);
typedef WSEC_ERR_T (*StubSDP_FileHmacType)(WSEC_UINT32 len, const WSEC_CHAR* filePath,const WSEC_PROGRESS_RPT_STRU* i,const SDP_HMAC_ALG_ATTR *stHmacAlgAttr,WSEC_VOID* pvHmacData, WSEC_UINT32 *ulHDLen);


//*******************************************************************************
mp_void StubSDPFuncLogVoid(mp_void* pthis){
    return;
}

WSEC_ERR_T StubSDP_GetCipherDataLen(mp_uint32 inLen,mp_uint32 *outLen){
    return WSEC_SUCCESS;
}
    
WSEC_ERR_T StubSDP_GetCipherDataLen1(mp_uint32 inLen,mp_uint32 *outLen){
    return WSEC_FAILURE;
}


//QINLANG

/*mp_int32 StubSDP_memset_s(WSEC_BYTE *inBuf,mp_uint32 inLen1,int inLen2,mp_uint32 inLen3)
{
	return 1;
}*/


WSEC_ERR_T StubSDP_GetHmacAlgAttr(WSEC_UINT32 len,SDP_HMAC_ALG_ATTR* stHmacAlgAttr)
{

    return WSEC_SUCCESS;
}

WSEC_ERR_T StubSDP_Encrypt(WSEC_UINT32 len,const WSEC_BYTE *inBuf,WSEC_UINT32 inLen,WSEC_BYTE *outBuf,WSEC_UINT32 *outLen)
{
	
	return WSEC_SUCCESS;
}

WSEC_ERR_T StubSDP_FileHmac(WSEC_UINT32 len, const WSEC_CHAR* filePath,const WSEC_PROGRESS_RPT_STRU* i,const SDP_HMAC_ALG_ATTR *stHmacAlgAttr,WSEC_VOID* pvHmacData, WSEC_UINT32 *ulHDLen)

{
	return WSEC_SUCCESS;
}
#endif
