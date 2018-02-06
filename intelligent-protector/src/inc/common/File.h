/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_FILE_H__
#define __AGENT_FILE_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include "common/Types.h"
#include "common/Defines.h"

#ifdef WIN32
#define S_ISDIR(m) ((m)&_S_IFDIR)
#endif

#define GETOSERRORCODE(a) \
    do \
    { \
        if(a) \
        { \
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call fclose failed, errno[%d]:%s.", errno, strerror(errno)); \
            return MP_FAILED; \
        } \
    } \
    while(0)

class AGENT_API CMpFile
{
public:
    static mp_int32 OpenFile();
    static mp_bool FileExist(const mp_char* pszFilePath);
    static mp_bool DirExist(const mp_char* pszDirPath);
    static mp_int32 CreateDir(const mp_char* pszDirPath);
    static mp_int32 FileSize(const mp_char* pszFilePath, mp_uint32& uiSize);
    static mp_int32 GetlLastModifyTime(const mp_char* pszFilePath, mp_time& tLastModifyTime);
    static mp_int32 ReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput);
    static mp_int32 GetFolderFile(mp_string& strFolder, vector<mp_string>& vecFileList);
    static mp_bool WaitForFile(const mp_char* pszFilePath, mp_int32 iSleepInterval, mp_int32 iSleepCount);
    static mp_int32 DelFile(const mp_char* pszFilePath);
};

//ICP文件，用于进程间通信
#define RESULT_TMP_FILE   "result_tmp"
#define INPUT_TMP_FILE    "input_tmp"
#define TOP_TMP_FILE      "top_tmp"
#define EN_TMP_FILE       "en_tmp"

class AGENT_API CIPCFile
{
public:
    static mp_int32 ReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput);
    static mp_int32 WriteFile(mp_string& strFilePath, vector<mp_string>& vecInput);
    static mp_int32 WriteInput(mp_string& strUniqueID, mp_string& strInput);
    static mp_int32 ReadInput(mp_string& strUniqueID, mp_string& strInput);
    static mp_int32 WriteResult(mp_string& strUniqueID, vector<mp_string>& vecRlt);
    static mp_int32 ReadResult(mp_string& strUniqueID, vector<mp_string>& vecRlt);
    static mp_int32 ReadOldResult(mp_string& strUniqueID, vector<mp_string>& vecRlt);
    static mp_int32 ChownResult(mp_string& strUniqueID);

};

#endif //__AGENT_FILE_H__

