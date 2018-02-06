/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include <sstream>
#include "common/UniqueId.h"

CUniqueID CUniqueID::m_instance;

/*------------------------------------------------------------
Function Name: GetString
Description  : 获取全局唯一ID，格式为string
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_string CUniqueID::GetString()
{
    CThreadAutoLock lock(&m_uniqueIDMutex);
    ostringstream oss;
    oss << m_iUniqueID;
    m_iUniqueID = (m_iUniqueID + 1) % MAX_UNIQUE_ID;
    return oss.str();
}

/*------------------------------------------------------------
Function Name: GetInt
Description  : 获取全局唯一ID，格式为int
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CUniqueID::GetInt()
{
    CThreadAutoLock lock(&m_uniqueIDMutex);
    mp_int32 iID = m_iUniqueID;
    m_iUniqueID = (m_iUniqueID + 1) % MAX_UNIQUE_ID;
    return iID;
}

