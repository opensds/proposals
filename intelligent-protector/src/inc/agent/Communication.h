/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

******************************************************************************
File Name     : Communication.h
Version       : Initial Draft
Author        : 
Created       : 2015/01/19
Last Modified :
Description   : 通信类定义
History       :
1.Date        :
Author      :
Modification:
******************************************************************************/
#ifndef _AGENT_COMMUNICATION_H_
#define _AGENT_COMMUNICATION_H_

#include "common/Types.h"
#include "rest/HttpCGI.h"
#include "rest/MessageProcess.h"
#include "common/Thread.h"

#define MAX_REQUEST_HANDLER 100
#define RCV_SLEEP_TIME 20

//定义预分配的FCGX_Request对象的使用状态
typedef struct tag_request_handler_info
{
    FCGX_Request* pFcgxReq;
    mp_bool isUsed;
}request_handler_info_t;

//消息对，消息队列存储的数据类型
typedef struct tag_message_pair_t
{
    CRequestMsg *pReqMsg;
    CResponseMsg *pRspMsg;
    tag_message_pair_t()
    {
        pReqMsg = NULL;
        pRspMsg = NULL;
    }
    tag_message_pair_t(CRequestMsg * pReq,  CResponseMsg * pRsp)
    {
        pReqMsg = pReq;
        pRspMsg = pRsp;
    }
}message_pair_t;

class CCommunication
{
public:
    static CCommunication &GetInstance()
    {
        return m_instance;
    }

    ~CCommunication();
    mp_int32 Init();
    mp_bool IsQueueEmpty()
    {
        return m_reqMsgQueue.empty() ? true : false;
    }

    mp_void ReleaseRequest(FCGX_Request* pReq);
    mp_int32 PopReqMsgQueue(message_pair_t &msgPair);
    mp_void PushReqMsgQueue(message_pair_t msgPair);
    mp_int32 PopRspMsgQueue(message_pair_t &msgPair);
    mp_void PushRspMsgQueue(message_pair_t msgPair);
    mp_int32 PopRspInternalMsgQueue(message_pair_t &msgPair);

private:
    CCommunication()
    {     
        (mp_void)memset_s(&m_hReceiveThread, sizeof(m_hReceiveThread), 0, sizeof(m_hReceiveThread));
        (mp_void)memset_s(&m_hSendThread, sizeof(m_hSendThread), 0, sizeof(m_hSendThread));
        //Coverity&Fortify误报:UNINIT_CTOR
        //Coveirty&Fortify不认识公司安全函数memset_s，提示m_dispatchTid.os_id未初始化
        CMpThread::InitLock(&m_reqTableMutex);
        CMpThread::InitLock(&m_reqMsgQueueMutex);
        CMpThread::InitLock(&m_rspMsgQueueMutex);
        m_iRecvThreadStatus = THREAD_STATUS_IDLE;
        m_iSendThreadStatus = THREAD_STATUS_IDLE;
        m_bNeedExit = MP_FALSE;
    }
    mp_int32 InitRequest(mp_int32 handler);
    mp_void DeleteRequest();
    mp_bool NeedExit();
    mp_void SetRecvThreadStatus(mp_int32 iThreadStatus);
    mp_void SetSendThreadStatus(mp_int32 iThreadStatus);
    FCGX_Request* GetFcgxReq();
#ifdef WIN32
    static DWORD WINAPI ReceiveThreadFunc(mp_void* pThis);
    static DWORD WINAPI SendThreadFunc(mp_void* pThis);
#else
    static mp_void* ReceiveThreadFunc(mp_void* pThis);
    static mp_void* SendThreadFunc(mp_void* pThis);
#endif
    static mp_void SendFailedMsg(CCommunication *pInstance, FCGX_Request *pFcgiReq, mp_int32 iHttpStatus, mp_int32 iRetCode);
    static mp_void HandleReceiveMsg(CCommunication *pInstance, FCGX_Request *pFcgiReq);

private:
    static CCommunication m_instance;                  //单例对象
    vector<request_handler_info_t> m_ReqHandlerTable;  //存储预分配的FCGX_Request对象
    thread_lock_t m_reqTableMutex;                     //m_ReqHandlerTable访问互斥锁
    vector<message_pair_t> m_reqMsgQueue;              //接收消息队列
    thread_lock_t m_reqMsgQueueMutex;                  //m_reqMsgQueue访问互斥锁
    vector<message_pair_t> m_rspMsgQueue;              //发送消息队列
    thread_lock_t m_rspMsgQueueMutex;                  //m_reqMsgQueue访问互斥锁
    thread_id_t m_hReceiveThread;                      //创建的接收线程句柄
    thread_id_t m_hSendThread;                         //创建的发送线程句柄
    volatile mp_bool m_bNeedExit;                      //线程退出标识
    volatile mp_int32 m_iRecvThreadStatus;             //接收线程状态
    volatile mp_int32 m_iSendThreadStatus;             //发送线程状态
};


#endif //_AGENT_COMMUNICATION_H_

