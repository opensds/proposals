/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT__COMMON_SIGN_H__
#define _AGENT__COMMON_SIGN_H__

#include "common/Types.h"
#include "common/Defines.h"
#include <vector>
#include <map>

#define SIGN_FORMAT_STR "="

mp_int32 AGENT_API CheckScriptSign(const mp_string strFileName);
mp_int32 AGENT_API CheckFileSign(const mp_string strFileName, const vector<mp_string>& vecSigns);
mp_void AGENT_API GetScriptNames(const mp_string strFileName, map<mp_string,mp_int32>& mapScriptNames);
mp_int32 AGENT_API CheckSign(mp_string strFileName, mp_string strSignEncrypt);

#endif /* _AGENT__COMMON_SIGN_H__ */
