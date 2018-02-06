/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

******************************************************************************
File Name     : CHttpCGI.h
Version       : Initial Draft
Author        :
Created       : 2015/01/19
Last Modified :
Description   : 处理CGI接口消息
History       :
1.Date        :
Author      :
Modification:
******************************************************************************/
#ifndef _AGENT_HTTP_CGI_H_
#define _AGENT_HTTP_CGI_H_

#include <map>
#include <stdlib.h>

#include "common/Types.h"
#include "common/Defines.h"    //macro DLLAPI is defined in Defines.h
#include "fcgi/include/fcgiapp.h"

//#ifdef WIN32
//#define SHARE_API  __declspec(dllexport)
//#else
//#define SHARE_API
//#endif

//#pragma warning( disable: 4251 )
class CHttpRequest
{
public:
    CHttpRequest(FCGX_Request *request);
    CHttpRequest()
    {
        m_pFcgRequest = NULL;
    }

    CHttpRequest(CHttpRequest& httpReq)
    {
        m_strURL = httpReq.m_strURL;
        m_strQueryParam = httpReq.m_strQueryParam;
        m_strMethod = httpReq.m_strMethod;
        m_pFcgRequest = httpReq.m_pFcgRequest;
    }

    CHttpRequest& operator=(CHttpRequest& httpReq)
    {
        m_strURL = httpReq.m_strURL;
        m_strQueryParam = httpReq.m_strQueryParam;
        m_strMethod = httpReq.m_strMethod;
        m_pFcgRequest = httpReq.m_pFcgRequest;
        return *this;
    }

    const mp_char* GetURL()
    {
        return m_strURL.c_str();
    }
    const mp_char* GetQueryParamStr()
    {
        return m_strQueryParam.c_str();
    }
    const mp_char* GetHead(const char* name);
    const mp_char* GetHeadNoCheck(const mp_char* name);
    mp_char** GetAllHead();
    FCGX_Request* GetFcgxReq();
    const mp_char* GetRemoteIP();
    const mp_char* GetMethod()
    {
        return m_strMethod.c_str();
    }
    void SetMethod(std::string strMethod)
    {
        m_strMethod = strMethod;
    }
    void SetAllHead(char** envp)
    {
        m_pFcgRequest->envp = envp;
    }
    void SetFcgxReq(FCGX_Request* pFcgxReq)
    {
        m_pFcgRequest = pFcgxReq;
    }
    mp_uint32 GetContentLen();
    mp_bool GetContentType(std::string &type);
    mp_int32 ReadChar();
    mp_int32 ReadStr(char* b, mp_int32 l);
    mp_char* Readline(char* b, mp_int32 l);

private:
    mp_string m_strURL;  //URL字符串
    mp_string m_strQueryParam; //查询字符串
    mp_string m_strMethod;
    FCGX_Request *m_pFcgRequest;  //分配req对象，解决fastcgi
};

class CHttpResponse
{
public:
    CHttpResponse(FCGX_Request *request);
    CHttpResponse()
    {
        m_pFcgRequest = NULL;
    }
    void SetContentType(const char* type);
    const mp_char* GetHead(const char* name);
    void SetHead(const char* name, const char* value);
    void RemoveHead(const char* name);
    mp_int32 WriteChar(mp_int32 c);
    mp_int32 WriteStr(const char* str, mp_int32 n);
    mp_int32 WriteS(const char* str);
    mp_int32 WriteF(const char* format, ...);
    void SendError(mp_int32);
    void Complete();
private:
    FCGX_Request *m_pFcgRequest; //fastcgi请求对象
    map<mp_string, mp_string> m_Heads; //消息头对象
    void SendHeads();
};

#endif //_AGENT_HTTP_CGI_H_

