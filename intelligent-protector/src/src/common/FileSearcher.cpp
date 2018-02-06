/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/FileSearcher.h"
#include "common/Defines.h"
#include "common/File.h"
#include "common/String.h"
#include "common/Log.h"
#include "securec.h"
/*------------------------------------------------------------ 
Description  :设置路径
Input        :     pszPath---路径
Output       :   
Return       :    
                 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::SetPath(const mp_char* pszPath)
{
    m_paths.clear();
    const mp_char* ptr = pszPath;
    const mp_char* p = NULL;
    mp_size sz = 0;
    while(*ptr)
    {
        while(' ' == *ptr) 
        {
            ptr++;
        }

        p = ptr;
        sz = 0;
        while(*ptr && *ptr != PATH_SEPCH)
        {
            ptr++;
            sz++;
        }

        if(*ptr)
        {
            ptr++;
        }

        AddPath(p, sz);
    }
    RebuildPathString();
}
/*------------------------------------------------------------ 
Description  :获取路径
Input        :      
Output       :   
Return       :   m_strPath ---获得的路径
                 
Create By    :
Modification : 
-------------------------------------------------------------*/
const mp_string& CFileSearcher::GetPath()
{
    return m_strPath;
}
/*------------------------------------------------------------ 
Description  :清楚路径信息
Input        :      
Output       :   
Return       :    
                 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::Clear()
{
    m_paths.clear();
    m_strPath = "";
}
/*------------------------------------------------------------ 
Description  :根据路径查找文件是否存在
Input        :      pszFile---文件名，strPath---路径
Output       :   
Return       :    MP_TRUE---查找成功
                 MP_FALSE---查找失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CFileSearcher::Search(const mp_char* pszFile, mp_string& strPath)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin search file, name %s.", BaseFileName(pszFile));

    for(mp_size i = 0; i < m_paths.size(); i++)
    {
        if(IsExists(pszFile, m_paths[i], strPath))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Search file succ.");
            return MP_TRUE;
        }
    }

    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search file failed.");
    
    return MP_FALSE;
}
/*------------------------------------------------------------ 
Description  :根据路径查找多个文件是否存在
Input        :      pszFile---文件名，flist---文件信息列表
Output       :   
Return       :  true---查找成功
                  false---查找失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CFileSearcher::Search(const mp_char* pszFile, vector<mp_string>& flist)
{
    mp_size sz = 0;
    mp_string strPath;
    
    for(mp_size i = 0; i< m_paths.size(); i++)
    {
        if(IsExists(pszFile, m_paths[i], strPath))
        {
            flist.push_back(strPath);
            sz++;
        }
    }

    return (sz > 0);
}
/*------------------------------------------------------------ 
Description  :添加路径 
Input        :      pszPath---路径
Output       :   
Return       :   
                  
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::AddPath(const mp_char* pszPath)
{
    if(NULL == pszPath)
    {
        return;
    }

    AddPath(pszPath, strlen(pszPath));
    RebuildPathString();
}
/*------------------------------------------------------------ 
Description  :添加 路径信息
Input        :      pszDir---路径，sz---路径字符长度
Output       :   
Return       :   
                  
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::AddPath(const mp_char* pszDir, mp_size sz)
{
    if('\0' == pszDir[0] || 0 == sz)
    {
        return;
    }

    const mp_char* p = pszDir;
    while(sz > 0 && ' ' == *p)
    {
        p++;
        sz--;
    }

    while(sz > 0 && ' ' == p[sz-1])
    {
        sz--;
    }

    if(0 == sz)
    {
        return;
    }
    
    while(sz > 0 && IS_DIR_SEP_CHAR(p[sz-1]))
    {
        if(sz == 1)
        {
            break;
        }
        
        sz--;
    }
    
    m_paths.push_back(mp_string(p, sz));
}
/*------------------------------------------------------------ 
Description  :判断文件是否存在
Input        :      pszFile---文件，strDir---目录
Output       :   strPath---路径
Return       :   
                  
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CFileSearcher::IsExists(const mp_char* pszFile, const mp_string& strDir, mp_string& strPath)
{
    mp_char szFile[1024] = {0};
    mp_int32 iRet = MP_SUCCESS;
    
    iRet = SNPRINTF_S(szFile, sizeof(szFile), sizeof(szFile) - 1, "%s%s%s", strDir.c_str(), 
        PATH_SEPARATOR, pszFile);
    if (MP_FAILED == iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Snprintfs failed.");
        return MP_FALSE;
    }

    if (!CMpFile::FileExist(szFile))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "File not exist, file %s.", BaseFileName(szFile));
        return MP_FALSE;
    }

    strPath = szFile;
    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :重建路径为字符串格式
Input        :      
Output       :    
Return       :   
                  
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::RebuildPathString()
{
    mp_string strPath;
    for(mp_size i = 0; i < m_paths.size(); i++)
    {
        if(i > 0)
        {
            strPath.push_back(PATH_SEPCH);
        }
        strPath += m_paths[i];
    }
    
    m_strPath = strPath;
}

