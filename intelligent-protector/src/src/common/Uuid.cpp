/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Uuid.h"

#ifndef AIX53
/*------------------------------------------------------------ 
Description  :获取主机Uuid
Input        :       
Output       :     strUuid---Uuid
Return       :     MP_SUCCESS---成功
                   MP_FAILED---失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CUuidNum::GetUuidNumber(mp_string &strUuid)
{
    mp_uint32 iRet = MP_SUCCESS;
    mp_uuid uuid;
#ifdef WIN32
    HRESULT hRet = CoCreateGuid(&uuid);
    if (S_OK != hRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create uuid failed.");
        
        return MP_FAILED;
    }
#elif (defined SOLARIS) || (defined LINUX)
    uuid_generate(uuid);
    
#else
    //AIX and HP-UX
    uuid_create(&uuid, &iRet);
    if (uuid_s_ok != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create uuid failed.");
        
        return MP_FAILED;
    }
#endif

    iRet = FormatUuid(uuid, strUuid);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Formate uuid failed, iRet is %d", iRet);
        
        return MP_FAILED;
    }
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :格式化Uuid
Input        :     uuid --uuid  
Output       :     strUuid---Uuid
Return       :     MP_SUCCESS---成功
                   MP_FAILED---失败
Create By    :
Modification : 
-------------------------------------------------------------*/

mp_int32 CUuidNum::FormatUuid(mp_uuid uuid, mp_string &strUuid)
{
#ifdef LINUX
    unsigned char* ucUuid = uuid;
#else
    unsigned char* ucUuid = (unsigned char *)&uuid;
#endif
    mp_size uuidlen = sizeof(mp_uuid);
    mp_char cTmp[3] = {0};
    
    for (mp_size i = 0; i < uuidlen; i++, ucUuid++)
    {
        CHECK_FAIL(sprintf_s(cTmp, sizeof(cTmp), "%02X", *ucUuid));

        strUuid += cTmp;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "FormatUuid succ, uuid is %s", strUuid.c_str());
    return MP_SUCCESS;
}
#endif

