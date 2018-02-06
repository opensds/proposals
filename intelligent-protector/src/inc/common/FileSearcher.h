/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_FILE_SEARCHER_H__
#define __AGENT_FILE_SEARCHER_H__

#include <vector>
#include "common/Types.h"
#include "common/Defines.h"

class AGENT_API CFileSearcher
{
private:
    typedef vector<mp_string> PATHS;
    mp_string m_strPath;
    PATHS m_paths;

    //typedef mtl_vector<mtl_string> PLUGINCFG_LIST;
    //PLUGINCFG_LIST m_listCfgFile;

public:
    CFileSearcher(){};

    mp_void SetPath(const mp_char* pszPath);
    const mp_string& GetPath();
    mp_void AddPath(const mp_char* pszPath);
    mp_void Clear();
    mp_bool Search(const mp_char* pszFile, mp_string& strPath);
    mp_bool Search(const mp_char* pszFile, vector<mp_string>& flist);

private:
    mp_void RebuildPathString();
    mp_void AddPath(const mp_char* pszDir, mp_size sz);
    mp_bool IsExists(const mp_char* pszFile, const mp_string& strDir, mp_string& strPath);
};

#endif //__AGENT_FILE_SEARCHER_H__

