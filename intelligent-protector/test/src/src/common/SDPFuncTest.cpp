/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/SDPFuncTest.h"
TEST_F(SDPFuncTest,Initialize_KMC){
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    
    Initialize_KMC();
    Reset_KMC();
    Timer_KMC();

    Encrypt_Str_KMC_With_Reset(inStr,outStr);
    Decrypt_Str_KMC_With_Reset(inStr,outStr);
    Gen_File_HMAC_KMC_With_Reset(filePath,fileHMAC);
    Verify_File_HMAC_KMC_With_Reset(filePath,fileHMAC);
}

TEST_F(SDPFuncTest,Finalize_KMC){
    mp_int32 ret;
    
    {
        ret = Finalize_KMC();
        EXPECT_EQ(ret,MP_SUCCESS);
    }
/*    
    {
        Initialize_KMC();
        ret = Finalize_KMC();
        EXPECT_EQ(ret,MP_FAILED);
    }*/
}

TEST_F(SDPFuncTest,Encrypt_Str_KMC){
    mp_int32 ret;
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    
    {
        ret = Encrypt_Str_KMC(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        inStr = "test";
        ret = Encrypt_Str_KMC(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    { 
        inStr = "test";
        Stub<SDP_GetCipherDataLenType, StubSDP_GetCipherDataLenType, mp_void> mystub1(&SDP_GetCipherDataLen, &StubSDP_GetCipherDataLen);
        ret = Encrypt_Str_KMC(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }   
        
    {        
        inStr = "test";
        Stub<SDP_GetCipherDataLenType, StubSDP_GetCipherDataLenType, mp_void> mystub1(&SDP_GetCipherDataLen, &StubSDP_GetCipherDataLen1);
        ret = Encrypt_Str_KMC(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }

    { 
        inStr = "test";
        Stub<SDP_EncryptType, StubSDP_EncryptType, mp_void> mystub2(&SDP_Encrypt, &StubSDP_Encrypt);
        ret = Encrypt_Str_KMC(inStr,outStr);
        EXPECT_EQ(ret,MP_SUCCESS);
    }   
}

TEST_F(SDPFuncTest,Decrypt_Str_KMC){
    mp_int32 ret;
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    
    {
        ret = Decrypt_Str_KMC(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        inStr = "test1";
        ret = Decrypt_Str_KMC(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
}

TEST_F(SDPFuncTest,Gen_File_HMAC_KMC){
    mp_int32 ret;
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    
    WSEC_UINT32 len;
    WSEC_PROGRESS_RPT_STRU* i;
    SDP_HMAC_ALG_ATTR *stHmacAlgAttr;
    WSEC_VOID* pvHmacData;
    WSEC_UINT32 *ulHDLen;
    {
        ret = Gen_File_HMAC_KMC(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        filePath = "test1";
        ret = Gen_File_HMAC_KMC(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }

    //
    {
        filePath = "test";
        Stub<SDP_GetHmacAlgAttrType, StubSDP_GetHmacAlgAttrType, mp_void> mystub1(&SDP_GetHmacAlgAttr, &StubSDP_GetHmacAlgAttr);
        ret = Gen_File_HMAC_KMC(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }

    {
        filePath = "test";
        Stub<SDP_GetHmacAlgAttrType, StubSDP_GetHmacAlgAttrType, mp_void> mystub1(&SDP_GetHmacAlgAttr, &StubSDP_GetHmacAlgAttr);
        Stub<SDP_FileHmacType,StubSDP_FileHmacType, mp_void> mystub2(&SDP_FileHmac, &StubSDP_FileHmac);
        ret = Gen_File_HMAC_KMC(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(SDPFuncTest,Verify_File_HMAC_KMC){
    mp_int32 ret;
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    
    {
        ret = Verify_File_HMAC_KMC(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        filePath = "test1";
        fileHMAC = "DDDdkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkktestfdfdfdfdfdf12311555151ffffffffff";
        ret = Verify_File_HMAC_KMC(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }
}

