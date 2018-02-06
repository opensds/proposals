/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "rest/HttpCGI.h"
#include "common/Log.h"
#include "rest/Interfaces.h"

CHttpRequest::CHttpRequest(FCGX_Request *pFcgiReq):m_pFcgRequest(pFcgiReq)
{
    //获取URL和查询参数
    const mp_char *uri = FCGX_GetParam(REQUEST_URI, m_pFcgRequest->envp);
    if (NULL == uri)
    {
        m_strURL = UNKNOWN;
    }
    else
    {
        mp_string strCompleteURL = uri;
        mp_string::size_type pos = strCompleteURL.find("?");
        if (pos != mp_string::npos)
        {
            m_strURL = strCompleteURL.substr(0, pos);
            m_strQueryParam = strCompleteURL.substr(pos + 1);
        }
        else
        {
            m_strURL = strCompleteURL;
            m_strQueryParam = "";
        }
    }

    //获取method
    const mp_char* method = FCGX_GetParam(REQUEST_METHOD, m_pFcgRequest->envp);
    m_strMethod = (NULL == method ? UNKNOWN : method);
}

/*------------------------------------------------------------
Function Name: getHead
Description  : 根据HTTP消息头名称获取其内容
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
const mp_char* CHttpRequest::GetHead(const mp_char* name)
{
    const mp_char* head = FCGX_GetParam(name, m_pFcgRequest->envp);
    return NULL == head ? UNKNOWN : head;
}

/*------------------------------------------------------------
Function Name: getHead
Description  : 根据HTTP消息头名称获取其内容
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
const mp_char* CHttpRequest::GetHeadNoCheck(const mp_char* name)
{
    return FCGX_GetParam(name, m_pFcgRequest->envp);
}

/*------------------------------------------------------------
Function Name: getAllHead
Description  : 根据所有HTTP消息头
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_char** CHttpRequest::GetAllHead()
{
    return m_pFcgRequest->envp;
}

/*------------------------------------------------------------
Function Name: getRemoteIP
Description  : 获取远端IP地址
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
const mp_char* CHttpRequest::GetRemoteIP()
{
    const mp_char *remoteIP = FCGX_GetParam(REMOTE_ADDR, m_pFcgRequest->envp);
    return NULL == remoteIP ? UNKNOWN : remoteIP;
}

/*------------------------------------------------------------
Function Name: getContentLen
Description  : 获取http请求中的内容的长度
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_uint32 CHttpRequest::GetContentLen()
{
    return (mp_uint32)atoi(GetHead(CONTENT_LENGTH));
}

/*------------------------------------------------------------
Function Name: getContentType
Description  : 获取http请求中的类型
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CHttpRequest::GetContentType(mp_string &type)
{
    const mp_char *ct = GetHead(CONTENT_TYPE);
    if (ct == NULL)
    {
        return MP_FALSE;
    }

    type = ct;
    return MP_TRUE;
}

/*------------------------------------------------------------
Function Name: getFcgxReq
Description  : 获取FCGX_Request的成员
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
FCGX_Request* CHttpRequest::GetFcgxReq()
{
    return m_pFcgRequest;
}

/*
 *----------------------------------------------------------------------
 *
 * ReadChar
 *
 *      Writes a byte to the output stream.
 *
 * Results:
       The byte, or EOF (-1) if an error occurred.
 *
 *----------------------------------------------------------------------
 */
mp_int32 CHttpRequest::ReadChar()
{
    return FCGX_GetChar(m_pFcgRequest->in);
}

/*
 *----------------------------------------------------------------------
 * ReadStr
 *
 *      Reads up to n consecutive bytes from the input stream
 *      into the character array str.  Performs no interpretation
 *      of the input bytes.
 *
 * Results:
        Number of bytes read.  If result is smaller than n,
 *      the end of input has been reached..
 *
 *----------------------------------------------------------------------
 */
mp_int32 CHttpRequest::ReadStr(mp_char* b, mp_int32 l)
{
    return FCGX_GetStr(b, l, m_pFcgRequest->in);
}

/*
 *----------------------------------------------------------------------
 *
 * readline
 *
 *      Reads up to n-1 consecutive bytes from the input stream
 *      into the character array str.  Stops before n-1 bytes
 *      have been read if '\n' or EOF is read.  The terminating '\n'
 *      is copied to str.  After copying the last byte into str,
 *      stores a '\0' terminator.
 *
 * Results:
 *      NULL if EOF is the first thing read from the input stream,
 *      str otherwise.
 *
 *----------------------------------------------------------------------
 */
char* CHttpRequest::Readline(mp_char* b, mp_int32 l)
{
    return FCGX_GetLine(b, l, m_pFcgRequest->in);
}

CHttpResponse::CHttpResponse(FCGX_Request* pRequest):m_pFcgRequest(pRequest)
{
    SetHead(CACHE_CONTROL, "no-cache");
}

/*------------------------------------------------------------
Function Name: setContentType
Description  : 设置http请求中的Content-Type字段值
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
void CHttpResponse::SetContentType(const mp_char* type)
{
    m_Heads[CONTENT_TYPE] = type;
}

/*------------------------------------------------------------
Function Name: getHead
Description  : 根据名称获取相应消息头信息的值
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
const mp_char* CHttpResponse::GetHead(const mp_char* name)
{
    map<mp_string, mp_string>::iterator it = m_Heads.find(name);
    if(it != m_Heads.end())
    {
        return it->second.c_str();
    }
    return NULL;
}

/*------------------------------------------------------------
Function Name: setHead
Description  : 设置响应消息头字段值
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
void CHttpResponse::SetHead(const mp_char* name, const mp_char* value)
{
    m_Heads[name] = value;
}

/*------------------------------------------------------------
Function Name: removeHead
Description  : 根据名称移除响应消息头中的对应字段
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
void CHttpResponse::RemoveHead(const mp_char* name)
{
    m_Heads.erase(name);
}

/*
 *----------------------------------------------------------------------
 *
 * writeChar
 *
 *      Writes a byte to the output stream.
 *
 * Results:
 *	    The byte, or EOF (-1) if an error occurred.
 *
 *----------------------------------------------------------------------
 */
mp_int32 CHttpResponse::WriteChar(mp_int32 c)
{
    SendHeads();
    return FCGX_PutChar(c, m_pFcgRequest->out);
}

/*
 *----------------------------------------------------------------------
 *
 * writeStr
 *
 *      Writes n consecutive bytes from the character array str
 *      into the output stream.  Performs no interpretation
 *      of the output bytes.
 *
 * Results:
 *      Number of bytes written (n) for normal return,
 *      EOF (-1) if an error occurred.
 *
 *----------------------------------------------------------------------
 */
mp_int32 CHttpResponse::WriteStr(const mp_char* str, mp_int32 n)
{
    SendHeads();
    return FCGX_PutStr(str, n, m_pFcgRequest->out);
}

/*
 *----------------------------------------------------------------------
 *
 * writeS
 *
 *      Writes a null-terminated character string to the output stream.
 *
 * Results:
 *      number of bytes written for normal return,
 *      EOF (-1) if an error occurred.
 *
 *----------------------------------------------------------------------
 */
mp_int32 CHttpResponse::WriteS(const mp_char* str)
{
    SendHeads();
    return FCGX_PutS(str, m_pFcgRequest->out);
}

/*
 *----------------------------------------------------------------------
 *
 * writeF
 *
 *      Performs printf-style output formatting and writes the results
 *      to the output stream.
 *
 * Results:
 *      number of bytes written for normal return,
 *      EOF (-1) if an error occurred.
 *
 *----------------------------------------------------------------------
 */
mp_int32 CHttpResponse::WriteF(const mp_char* format, ...)
{
    mp_int32 result;
    va_list ap;
    va_start(ap, format);
    result = FCGX_VFPrintF(m_pFcgRequest->out, format, ap);
    va_end(ap);
    return result;
}

/*------------------------------------------------------------
Function Name: sendHeads
Description  : 将消息头写入FCGI响应流中。
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CHttpResponse::SendHeads()
{
    if (!m_Heads.empty())
    {
        for (map<mp_string, mp_string>::iterator it = m_Heads.begin(); it != m_Heads.end(); ++it)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "[Http Head Sent]%s=%s.", it->first.c_str(), it->second.c_str());
            if (it->first != CONTENT_TYPE)
            {
                FCGX_FPrintF(m_pFcgRequest->out, "%s: %s\r\n", it->first.c_str(),
                    it->second.c_str());
            }
        }

        m_Heads.clear();
        FCGX_FPrintF(m_pFcgRequest->out, "\r\n");
    }
}

/*------------------------------------------------------------
Function Name: complete
Description  : 设置fcgi结束标志。
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CHttpResponse::Complete()
{
    FCGX_Finish_r(m_pFcgRequest);
}

