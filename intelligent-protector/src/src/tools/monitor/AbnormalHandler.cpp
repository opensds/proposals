/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/monitor/AbnormalHandler.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "common/Thread.h"
#include "common/Path.h"
#include "common/Time.h"
#include "common/File.h"
#include "common/SystemExec.h"
#include "common/Sign.h"
#include "common/CryptAlg.h"
#include "common/Defines.h"
//#include "common/ServiceHandler.h"
#include "alarm/Trap.h"

#include <sstream>

#ifdef WIN32
#include "tlhelp32.h"
#include "Psapi.h"
#include <Windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

CAbnormalHandler CAbnormalHandler::m_instance;

/*------------------------------------------------------------
Function Name: Init
Description  : 初始函数，创建监控处理线程
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::Init()
{
    return CMpThread::Create(&m_hHandleThread, HandleFunc, this);
}

/*------------------------------------------------------------
Function Name: HandleFunc
Description  : 监控处理线程处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
#ifdef WIN32
DWORD WINAPI CAbnormalHandler::HandleFunc(mp_void* pThis)
#else
mp_void* CAbnormalHandler::HandleFunc(mp_void* pThis)
#endif
{
    LOGGUARD("");
    CAbnormalHandler* pHandle = (CAbnormalHandler*)pThis;

    pHandle->SetThreadStatus(THREAD_STATUS_RUNNING);
    mp_uint32 iCount = 0;
    while (!pHandle->NeedExit())
    {
        //从配置文件中读取配置
        pHandle->GetAgentMonitorCfg();
        pHandle->GetNginxMonitorCfg();
        monitor_common_config_t commonCfg = pHandle->GetCommonMonitorCfg();
        pHandle->Handle();
        if (iCount == 0)
        {
            CallCryptTimer();
        }
        DoSleep(commonCfg.iMonitorInterval * 1000);//从配置文件读取的值，不会溢出
        iCount++;
        iCount %= 10;
    }
    pHandle->SetThreadStatus(THREAD_STATUS_EXITED);
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

/*------------------------------------------------------------
Function Name: HandleFunc
Description  : 对所有监控进程逐条处理
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::Handle()
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
#ifndef WIN32
    mp_int32 iReadMonitorPid;
    pid_t iRealPid;
    vector<mp_string> vecReadPid;
    vector<mp_string> vecRealPid;
    mp_string strPidFilePath = CPath::GetInstance().GetLogFilePath(MONITOR_PID);
    iRet = CMpFile::ReadFile(strPidFilePath, vecReadPid);   //从文件中读取monitor的pid
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read monitor pid from %s failed, iRet = %d", MONITOR_PID, iRet);
    }
    else
    {
        iRealPid = getpid();                               //获取真实的PID
        iReadMonitorPid = atoi(vecReadPid.front().c_str());
        if ((mp_int32)iRealPid != iReadMonitorPid)         //真实PID与文件中的不一样
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Read monitor pid(%d) isn't the real pid(%d).", iReadMonitorPid, iRealPid);
            ostringstream oss;
            oss << (mp_int32)iRealPid;
            vecRealPid.push_back(oss.str().c_str());
            iRet = CIPCFile::WriteFile(strPidFilePath, vecRealPid);  //将真实的PID写入文件。
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write real monitor pid(%d) failed, iRet: %d. ", (mp_int32)iRealPid, iRet);
            }
            else{
                COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Write real monitor pid(%d) to file sucess.", (mp_int32)iRealPid);
            }
        }
    }
#endif
    if (m_stAgentMointorCfg.bMonitored)
    {
        iRet = MonitorAgent();
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Monitor agent failed, iRet = %d", iRet);
        }
    }

    if (m_stNginxMointorCfg.bMonitored)
    {
        mp_int32 iRet1 = MonitorNginx();
        if (MP_SUCCESS != iRet1)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Monitor nginx failed, iRet = %d", iRet1);
            return iRet1;
        }
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: MonitorAgent
Description  : agent进程监控处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::MonitorAgent()
{
    LOGGUARD("");
    monitor_data_t monitorAgentData;
    mp_int32 iRet = GetAgentMonitorData(monitorAgentData);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get agent monitor data failed, iRet = %d", iRet);
        return iRet;
    }

    //打印agent monitor数据
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Agent monitor data: bExist=%d, iHandlerNum=%d, iThreadNum=%d, ulPmSize=%dK",
        monitorAgentData.bExist, monitorAgentData.iHandlerNum, monitorAgentData.iThreadNum, monitorAgentData.ulPmSize);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Agent monitor data: ulVmSize=%dK, fCpuUsage=%f, ulTmpFileTotalSize=%dByte.",
        monitorAgentData.ulVmSize, monitorAgentData.fCpuUsage, monitorAgentData.ulTmpFileTotalSize);

    if (!monitorAgentData.bExist)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Agent process is not exist and will be started");
        iRet = StartAgent();
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start agent failed, iRet = %d", iRet);
        }
        return iRet;
    }

    //检测agent监控数据
    CheckAgentMonitorValue(monitorAgentData);

    //监测物理内存占用率，虚拟内存占用率，句柄个数，线程个数
    if (m_stAgentAbnormal.iPmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
        m_stAgentAbnormal.iVmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
        m_stAgentAbnormal.iHandlerOverTimes > m_stCommonMonitorCfg.iRetryTime ||
        m_stAgentAbnormal.iThreadOverTimes > m_stCommonMonitorCfg.iRetryTime)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Agent process is abnormal and will be restarted, iPmSizeOverTimes is %d,\
iVmSizeOverTimes is %d, iHandlerOverTimes is %d, iThreadOverTimes is %d", m_stAgentAbnormal.iPmSizeOverTimes,
            m_stAgentAbnormal.iVmSizeOverTimes, m_stAgentAbnormal.iHandlerOverTimes, m_stAgentAbnormal.iThreadOverTimes);
        iRet = RestartAgent();
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Restart agent failed, iRet = %d", iRet);
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Agent has been restarted successfully.");
        }
        return iRet;
    }

    //检测cpu利用率是否需要发送告警
    CheckAgentCpuUsage(monitorAgentData.fCpuUsage);
    
    //检测临时文件总大小
    if (m_stAgentAbnormal.iTmpFileSizeOverTimes > m_stCommonMonitorCfg.iRetryTime)
    {
        mp_int32 iRet1 = DeleteTmpFile();
        if (iRet1 != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Temp file size exceed, but delete failed, iRet = %d", iRet1);
            return iRet1;
        }
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: MonitorNginx
Description  : nginx进程监控处理函数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::MonitorNginx()
{
    LOGGUARD("");
    monitor_data_t monitorNginxData;
    mp_int32 iRet = GetNginxMonitorData(monitorNginxData);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get nginx monitor data failed, iRet = %d", iRet);
        return iRet;
    }

    if (!monitorNginxData.bExist)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Nginx process is not exist and will be started");
        iRet = StartNginx();
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start nginx failed, iRet = %d", iRet);
        }
        return iRet;
    }

     //打印nginx monitor数据
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Nginx monitor data: bExist=%d, iHandlerNum=%d, iThreadNum=%d, ulPmSize=%dK.", 
        monitorNginxData.bExist, monitorNginxData.iHandlerNum, monitorNginxData.iThreadNum, monitorNginxData.ulPmSize);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Nginx monitor data: ulVmSize=%dK, fCpuUsage=%f, NginxLogSize=%dK", 
        monitorNginxData.ulVmSize, monitorNginxData.fCpuUsage, monitorNginxData.uiNginxLogSize);

    //检测nginx监控数据
    CheckNginxMonitorValue(monitorNginxData);
    
    //监测物理内存占用率，虚拟内存占用率，句柄个数，线程个数
    if (m_stNginxAbnormal.iPmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
        m_stNginxAbnormal.iVmSizeOverTimes > m_stCommonMonitorCfg.iRetryTime ||
        m_stNginxAbnormal.iHandlerOverTimes > m_stCommonMonitorCfg.iRetryTime ||
        m_stNginxAbnormal.iThreadOverTimes > m_stCommonMonitorCfg.iRetryTime)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Nginx process is abnormal and will be restarted, iPmSizeOverTimes is %d, \
iVmSizeOverTimes is %d, iHandlerOverTimes is %d, iThreadOverTimes is %d", m_stNginxAbnormal.iPmSizeOverTimes,
            m_stNginxAbnormal.iVmSizeOverTimes, m_stNginxAbnormal.iHandlerOverTimes, m_stNginxAbnormal.iThreadOverTimes);
        iRet = RestartNginx();
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Restart nginx failed, iRet = %d", iRet);
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Nginx has been restarted successfully.");
        }
        return iRet;
    }

    //检测cpu利用率
    CheckNginxCpuUsage(monitorNginxData.fCpuUsage);

    //检测nginx日志大小
    if (monitorNginxData.uiNginxLogSize >= m_stNginxMointorCfg.iNginxLogSizeCfg*1024)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Nginx log size[%d] is overceed[%d]", monitorNginxData.uiNginxLogSize, m_stNginxMointorCfg.iNginxLogSizeCfg*1024);
        RotateNginxLog();
    }
    return iRet;
}

/*------------------------------------------------------------
Function Name: GetAgentMonitorData
Description  : 获取agent进程所有监控数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::GetAgentMonitorData(monitor_data_t& stMonitorData)
{
#ifndef WIN32
    //获取agent进程id
    //获取nginx进程id
    mp_string strPidFilePath = CPath::GetInstance().GetLogFilePath(AGENT_PID);
    mp_string strPid;
    vector<mp_string> vecRlt;
    mp_string strRdAgentLogFilePath;
    mp_int32 iRet = CMpFile::ReadFile(strPidFilePath, vecRlt);
    if (MP_SUCCESS != iRet || vecRlt.empty())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "read rdagent.pid failed");
        strPid = " ";
    }
    else
    {
        strPid = vecRlt.front();
    }

#ifdef SOLARIS
    mp_string strCmd = "ps -aef | grep '" + mp_string(AGENT_EXEC_NAME) + "' |grep '" + strPid
            + "' | grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | nawk '{print $2}'";
#else
    mp_string strCmd = "ps -aef | grep '" + mp_string(AGENT_EXEC_NAME) + "' |grep '" + strPid
            + "' | grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | awk '{print $2}'";
#endif
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get agent process ID failed, iRet = %d, size of vecRlt is %d",
              iRet, vecRlt.size());
        stMonitorData.bExist = MP_FALSE;
        //进程不存在按成功处理
        return MP_SUCCESS;
    }
    strRdAgentLogFilePath = CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME);
    strRdAgentLogFilePath = CMpString::BlankComma(strRdAgentLogFilePath);
    (void)chmod(strRdAgentLogFilePath.c_str(), S_IRUSR | S_IWUSR);      //rdagent.log没有创建的情况下，以上system执行之后新生成的rdagent.log
                                                                //权限为644或者664，这种情况需要这里改成660
    stMonitorData.bExist = MP_TRUE;
    //获取临时文件总容量
    (mp_void)GetTmpFileTotalSize(stMonitorData.ulTmpFileTotalSize);
#ifdef HP_UX_IA
    iRet = GetHPMonitorData(vecRlt.front(), stMonitorData);
#else
    iRet = GetMonitorData(vecRlt.front(), stMonitorData);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get monitor data failed, iRet = %d", iRet);
    }

    return iRet;
#else
    mp_string strWinAgent = mp_string(AGENT_EXEC_NAME) + ".exe";
    return GetWinMonitorData(strWinAgent, stMonitorData);
#endif
}

/*------------------------------------------------------------
Function Name: GetNginxMonitorData
Description  : 获取nginx进程所有监控数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::GetNginxMonitorData(monitor_data_t& stMonitorData)
{
    //获取nginx日志大小，单位为k
    mp_string strNginxLogPath = CPath::GetInstance().GetNginxLogsFilePath(NGINX_LOG_FILE);
    (mp_void)CMpFile::FileSize(strNginxLogPath.c_str(), stMonitorData.uiNginxLogSize);
    stMonitorData.uiNginxLogSize = stMonitorData.uiNginxLogSize / 1024;

#ifndef WIN32
    //获取nginx进程id
    mp_string strPidFilePath = CPath::GetInstance().GetNginxLogsFilePath(NGINX_PID);
    mp_string strPid;
    vector<mp_string> vecRlt;
    mp_string strRdAgentLogFilePath;
    mp_int32 iRet = CMpFile::ReadFile(strPidFilePath, vecRlt);
    if (MP_SUCCESS != iRet || vecRlt.empty())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "read ngnix.pid failed");
        strPid = " ";
    }
    else
    {
        strPid = vecRlt.front();
    }

#ifdef SOLARIS
    mp_string strCmd = "ps -aef | grep '" + mp_string(NGINX_EXEC_NAME) + "' |grep '" + strPid
             + "' |grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | nawk '{print $2}'";
#else
    mp_string strCmd = "ps -aef | grep '" + mp_string(NGINX_EXEC_NAME) + "' |grep '" + strPid
             + "' |grep -v grep | grep -v gdb | grep -v monitor | grep -v vi | grep -v tail | awk '{print $2}'";
#endif

    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get nginx process ID failed, iRet = %d, size of vecRlt is %d",
              iRet, vecRlt.size());
        stMonitorData.bExist = MP_FALSE;
        //进程不存在按成功处理
        return MP_SUCCESS;
    }
    strRdAgentLogFilePath = CPath::GetInstance().GetLogFilePath(AGENT_LOG_NAME);
    strRdAgentLogFilePath = CMpString::BlankComma(strRdAgentLogFilePath);
    (void)chmod(strRdAgentLogFilePath.c_str(), S_IRUSR | S_IWUSR);      //rdagent.log没有创建的情况下，以上system执行之后新生成的rdagent.log
                                                                //权限为644或者664，这种情况需要这里改成660
    stMonitorData.bExist = MP_TRUE;

    for (vector<mp_string>::iterator it = vecRlt.begin(); it != vecRlt.end(); it++)
    {
        monitor_data_t monitorData;
#ifdef HP_UX_IA
        iRet = GetHPMonitorData(*it, monitorData);
#else
        iRet = GetMonitorData(*it, monitorData);
#endif
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get monitor data failed, iRet = %d", iRet);
            return iRet;
        }
        stMonitorData = AddMonitorData(stMonitorData, monitorData);
    }

    return MP_SUCCESS;
#else
    mp_string strWinNginx = mp_string(NGINX_EXEC_NAME) + ".exe";
    return GetWinMonitorData(strWinNginx, stMonitorData);
#endif
}

/*------------------------------------------------------------
Function Name: GetHPMonitorData
Description  : 获取hp环境agent进程所有监控数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::GetHPMonitorData(mp_string strProcessID, monitor_data_t& stMonitorData)
{
    //hp下无法获取线程数量和句柄数量
    stMonitorData.iHandlerNum = 0;
    stMonitorData.iThreadNum = 0;

    //将top命令转存为文件
    mp_string strTopFilePath = CPath::GetInstance().GetTmpFilePath(TOP_TMP_FILE);
    mp_string strCmd = mp_string(HP_TOP_CMD) + " \"" + strTopFilePath + "\"";
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));

    //获取物理内存
    vector<mp_string> vecRlt;
    strCmd = mp_string("cat \"") + strTopFilePath + "\" | grep rdagent | awk '{print $8}'";
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get pysical memory failed, iRet = %d, size of vecRlt is %d",
               iRet, vecRlt.size());
        return iRet;
    }
    //只对返回数据的第一个元素进行处理
    stMonitorData.ulPmSize = GetKSize(vecRlt.front());

    //获取虚拟内存
    vecRlt.clear();
    strCmd = mp_string("cat \"") + strTopFilePath + "\" | grep rdagent | awk '{print $7}'";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get virtual memory failed, iRet = %d, size of vecRlt is %d",
               iRet, vecRlt.size());
        return iRet;
    }
    //Agent只会有一个进程，只对返回数据的第一个元素进行处理
    stMonitorData.ulVmSize = GetKSize(vecRlt.front());

    //获取cpu利用率
    vecRlt.clear();
    strCmd = mp_string("cat \"") + strTopFilePath + "\" | grep rdagent | awk '{print $11}'";
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get cpu usage failed, iRet = %d, size of vecRlt is %d",
               iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.fCpuUsage = (mp_float)atof(vecRlt.front().c_str());

    //删除临时文件
    iRet = CMpFile::DelFile(strTopFilePath.c_str());
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete tmp file failed, ret is %d", iRet);
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: GetMonitorData
Description  : 根据进程id获取非hp环境各种监控数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::GetMonitorData(mp_string strProcessID, monitor_data_t& stMonitorData)
{
    //获取线程数
#ifdef AIX
    mp_string strCmd = "ps -p " + strProcessID + " -o thcount | grep -v THCNT";
#else
    mp_string strCmd = "ps -p " + strProcessID + " -o nlwp | grep -v NLWP";
#endif
    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get thread num failed, iRet = %d, size of vecRlt is %d",
               iRet, vecRlt.size());
        return iRet;
    }
    CMpString::TrimLeft((mp_char*)vecRlt.front().c_str());
    CMpString::TrimRight((mp_char*)vecRlt.front().c_str());
    stMonitorData.iThreadNum = atoi(vecRlt.front().c_str());

    //获取句柄数
    strCmd = "ls /proc/" + strProcessID + "/fd | wc -l";
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get handler num failed, iRet = %d, size of vecRlt is %d",
               iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.iHandlerNum = atoi(vecRlt.front().c_str());

    //获取物理内存(进程使用的总物理内存数, Kbytes字节)
#ifdef SOLARIS
    strCmd = "ps -efo rss,pid,comm | grep -w '" + strProcessID + "' | grep -v 'grep' | nawk '{print $1}'";
#else
    strCmd = "ps auxw | grep -w '" + strProcessID + "' | grep -v 'grep' | awk '{print $6}'";
#endif
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get pysical memory failed, iRet = %d, size of vecRlt is %d",
               iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.ulPmSize = atol(vecRlt.front().c_str());
    //获取虚拟内存(读取的数值单位为 bytes字节(包含code+data+stack))
#ifdef SOLARIS
    strCmd = "ps -efo vsz,pid,comm | grep -w '" + strProcessID + "' | grep -v 'grep' | nawk '{print $1}'";
#else
    strCmd = "ps -p " + strProcessID + " -o vsz | grep -v VSZ";
#endif
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get virtual memory failed, iRet = %d, size of vecRlt is %d",
               iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.ulVmSize = atol(vecRlt.front().c_str());
    //获取CPU利用率
#ifdef SOLARIS
    strCmd = "ps -efo pcpu,pid,comm | grep -w '" + strProcessID + "' | grep -v 'grep' | nawk '{print $1}'";
#else
    strCmd = "ps auxw | grep -w '" + strProcessID + "' | grep -v 'grep' | awk '{print $3}'";
#endif
    vecRlt.clear();
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if ((MP_SUCCESS != iRet) || (vecRlt.empty()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get cpu usage failed, iRet = %d, size of vecRlt is %d",
               iRet, vecRlt.size());
        return iRet;
    }
    stMonitorData.fCpuUsage = (mp_float)atof(vecRlt.front().c_str());

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: AddMonitorData
Description  : 对多条监控数据进行数据叠加
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
monitor_data_t CAbnormalHandler::AddMonitorData(monitor_data_t& stMonitorData1, monitor_data_t& stMonitorData2)
{
    monitor_data_t stSumMointorData;
    stSumMointorData.bExist = stMonitorData1.bExist || stMonitorData2.bExist;
    stSumMointorData.fCpuUsage = stMonitorData1.fCpuUsage + stMonitorData2.fCpuUsage;
    stSumMointorData.iHandlerNum = stMonitorData1.iHandlerNum + stMonitorData2.iHandlerNum;
    stSumMointorData.ulPmSize = stMonitorData1.ulPmSize + stMonitorData2.ulPmSize;
    stSumMointorData.iThreadNum = stMonitorData1.iThreadNum + stMonitorData2.iThreadNum;
    stSumMointorData.ulVmSize = stMonitorData1.ulVmSize + stMonitorData2.ulVmSize;
    (stMonitorData1.ulTmpFileTotalSize == 0) ?
        (stSumMointorData.ulTmpFileTotalSize = stMonitorData2.ulTmpFileTotalSize) :
        (stSumMointorData.ulTmpFileTotalSize = stMonitorData1.ulTmpFileTotalSize);
    (stMonitorData1.uiNginxLogSize == 0) ?
        (stSumMointorData.uiNginxLogSize = stMonitorData2.uiNginxLogSize) :
        (stSumMointorData.uiNginxLogSize = stMonitorData1.uiNginxLogSize);
    return stSumMointorData;
}

/*------------------------------------------------------------
Function Name: ClearMonitorData
Description  : 清除监控数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CAbnormalHandler::ClearMonitorData(monitor_data_t & stMonitorData)
{
    stMonitorData.bExist = MP_FALSE;
    stMonitorData.ulPmSize = 0;
    stMonitorData.ulVmSize = 0;
    stMonitorData.iHandlerNum = 0;
    stMonitorData.iThreadNum = 0;
    stMonitorData.fCpuUsage = 0.0;
    stMonitorData.ulTmpFileTotalSize = 0;
    stMonitorData.uiNginxLogSize = 0;
}

/*------------------------------------------------------------
Function Name: ClearAbnormalOccurTimes
Description  : 清除异常发生次数
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CAbnormalHandler::ClearAbnormalOccurTimes(abnormal_occur_times_t & stAbnormalOccurTimes)
{
    stAbnormalOccurTimes.iPmSizeOverTimes = 0;
    stAbnormalOccurTimes.iVmSizeOverTimes = 0;
    stAbnormalOccurTimes.iHandlerOverTimes = 0;
    stAbnormalOccurTimes.iThreadOverTimes = 0;
    stAbnormalOccurTimes.iCpuUsageOverTimes = 0;
    stAbnormalOccurTimes.iTmpFileSizeOverTimes = 0;
}

/*------------------------------------------------------------
Function Name: StartAgent
Description  : 启动agent进程
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::StartAgent()
{
    LOGGUARD("");
#ifdef WIN32
    mp_string strCmd = mp_string("sc start ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    //校验脚本签名
    mp_int32 iRet = CheckScriptSign(START_SCRIPT);
    if (iRet != MP_SUCCESS)
    {
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + AGENT_EXEC_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: RestartAgent
Description  : 重启agent进程
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::RestartAgent()
{
    LOGGUARD("");
    
#ifdef WIN32
    mp_string strCmd = mp_string("sc stop ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = mp_string("sc start ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    //校验脚本签名
    mp_int32 iRet = CheckScriptSign(STOP_SCRIPT);
    if (iRet != MP_SUCCESS)
    {
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + AGENT_EXEC_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    //校验脚本签名
    iRet = CheckScriptSign(START_SCRIPT);
    if (iRet != MP_SUCCESS)
    {
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + AGENT_EXEC_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif

    //重启成功，清空异常监控数据
    ClearAbnormalOccurTimes(m_stAgentAbnormal);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: StartNginx
Description  : 启动nginx进程
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::StartNginx()
{
    LOGGUARD("");
#ifdef WIN32
    //windows下nginx服务需要先停止再启动
    mp_string strCmd = mp_string("sc stop ") + NGINX_SERVICE_NAME;
    CSystemExec::ExecSystemWithoutEcho(strCmd);
    
    strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_EXE);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_START;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start nginx failed, iRet = %d.", iRet);
        return iRet;
    }
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_UNIX);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd+ " " + NGINX_START;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: RestartNginx
Description  : 重新启动nginx进程
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::RestartNginx()
{
    LOGGUARD("");
#ifdef WIN32
    mp_string strCmd = mp_string("sc stop ") + NGINX_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_EXE);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_START;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start nginx failed, iRet = %d.", iRet);
        return iRet;
    }
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    //校验脚本签名
    mp_int32 iRet = CheckScriptSign(STOP_SCRIPT);
    if (iRet != MP_SUCCESS)
    {
        return iRet;
    }
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    
    strCmd = CPath::GetInstance().GetBinFilePath(AGENTCLI_UNIX);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd+ " " + NGINX_START;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    //重启成功，清空异常监控数据
    ClearAbnormalOccurTimes(m_stNginxAbnormal);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: SendCPUAlarm
Description  : 发送agent cpu超出阈值告警
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::SendCPUAlarm(mp_int32 iProcessType)
{
    LOGGUARD("");
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = ALARM_ID_CPUUSAGE;
    //从配置文件读取门限值
    ostringstream oss;
    if (iProcessType == AGENT_PROCESS)  // Agent CPU
    {
        oss << m_stAgentMointorCfg.fCpuUsageCfg;
    } 
    else   // NGINX CPU
    {
        oss << m_stNginxMointorCfg.fCpuUsageCfg;
    }   
    alarmParam.strAlarmParam = oss.str();

    CHECK_FAIL_EX(CTrapSender::SendAlarm(alarmParam));
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: SendAgentCPUAlarm
Description  : 发送agent cpu超出阈值恢复告警
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::ResumeCPUAlarm()
{
    LOGGUARD("");
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = ALARM_ID_CPUUSAGE;
    //从配置文件读取门限值
    ostringstream oss;
    oss << m_stAgentMointorCfg.fCpuUsageCfg;
    alarmParam.strAlarmParam = oss.str();

    CHECK_FAIL_EX(CTrapSender::ResumeAlarm(alarmParam));
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: DeleteTmpFile
Description  : 产出tmp目录下时间超长的临时文件
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::DeleteTmpFile()
{
    LOGGUARD("");
    mp_string strTmpFileFolder = CPath::GetInstance().GetTmpPath();
    vector<mp_string> vecFileList;
    mp_time timeNow;
    CMpTime::Now(&timeNow);
    CHECK_FAIL_EX(CMpFile::GetFolderFile(strTmpFileFolder ,vecFileList));
    mp_int32 iRet = MP_SUCCESS;
    for (vector<mp_string>::iterator it = vecFileList.begin(); it != vecFileList.end(); it++)
    {
        mp_time timeLastModified = 0;
        mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(*it);
        iRet = CMpFile::GetlLastModifyTime(strFilePath.c_str(), timeLastModified);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get last modify time of %s failed.", BaseFileName(strFilePath.c_str()));
            continue;
        }
        //存留时间超过最大时间，删除
        if (timeNow < timeLastModified || timeNow - timeLastModified > MAX_TMP_EXIST_TIME)
        {
            mp_int32 iRet1 = CMpFile::DelFile(strFilePath.c_str());
            if (MP_SUCCESS != iRet1)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete file \"%s\" failed.", BaseFileName(strFilePath.c_str()));
                iRet = iRet1;
            }
        }
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: GetTmpFileTotalSize
Description  : 获取tmp目录总大小，单位字节
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::GetTmpFileTotalSize(mp_uint64& ulSize)
{
    ulSize = 0;
    mp_string strTmpFileFolder = CPath::GetInstance().GetTmpPath();
    vector<mp_string> vecFileList;
    CHECK_FAIL_EX(CMpFile::GetFolderFile(strTmpFileFolder ,vecFileList));
    for (vector<mp_string>::iterator it = vecFileList.begin(); it != vecFileList.end(); it++)
    {
        mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(*it);
        mp_uint32 uiSize = 0;
        CHECK_FAIL_EX(CMpFile::FileSize(strFilePath.c_str(), uiSize));
        ulSize += uiSize;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetKSize
Description  : 从获取到的容量字符串中对单位为k和m进行计算，统一转换成k
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_uint64 CAbnormalHandler::GetKSize(mp_string strSize)
{
	//CodeDex误报，Type Mismatch:Signed to Unsigned
    if (mp_string::npos != strSize.find("M"))
    {
        return mp_uint64(atol(strSize.c_str())) * 1024;
    }
    else if (mp_string::npos != strSize.find("K"))
    {
        return mp_uint64(atol(strSize.c_str()));
    }
    else
    {
        return mp_uint64(atol(strSize.c_str()) / 1024);
    }
}

mp_bool CAbnormalHandler::NeedExit()
{
    return m_bNeedExit;
}

mp_void CAbnormalHandler::SetThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}


mp_void CAbnormalHandler::RotateNginxLog()
{
   mp_int32 iRet = CSystemExec::ExecScript(ROATE_NGINX_LOG_SCRIPT, "", NULL);
   if (iRet != MP_SUCCESS)
   {
       COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Roate nginx log failed, iRet = %d.", iRet);
   }
}


/*------------------------------------------------------------
Function Name: GetAgentMonitorCfg
Description  : 从配置文件中获取agent监控配置
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
monitor_process_config_t& CAbnormalHandler::GetAgentMonitorCfg()
{
    CConfigXmlParser::GetInstance().GetValueBool(MONITOR_SECTION, AGENT_EXEC_NAME, m_stAgentMointorCfg.bMonitored);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, AGENT_EXEC_NAME, THRD_CNT, m_stAgentMointorCfg.iThreadNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, AGENT_EXEC_NAME, HANDLER_CNT, m_stAgentMointorCfg.iHandlerNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, AGENT_EXEC_NAME, PM_SIZE, m_stAgentMointorCfg.iPmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, AGENT_EXEC_NAME, VM_SIZE, m_stAgentMointorCfg.iVmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueFloat(MONITOR_SECTION, AGENT_EXEC_NAME, CPU_USAGE, m_stAgentMointorCfg.fCpuUsageCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, AGENT_EXEC_NAME, TMPFILE_SIZE, m_stAgentMointorCfg.iTmpFileTotalSizeCfg);
    return m_stAgentMointorCfg;
}

/*------------------------------------------------------------
Function Name: GetNginxMonitorCfg
Description  : 从配置文件中获取nginx监控配置
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
monitor_process_config_t& CAbnormalHandler::GetNginxMonitorCfg()
{
    CConfigXmlParser::GetInstance().GetValueBool(MONITOR_SECTION, CFG_NGINX_SECTION, m_stNginxMointorCfg.bMonitored);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, CFG_NGINX_SECTION, THRD_CNT, m_stNginxMointorCfg.iThreadNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, CFG_NGINX_SECTION, HANDLER_CNT, m_stNginxMointorCfg.iHandlerNumCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, CFG_NGINX_SECTION, PM_SIZE, m_stNginxMointorCfg.iPmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, CFG_NGINX_SECTION, VM_SIZE, m_stNginxMointorCfg.iVmSizeCfg);
    CConfigXmlParser::GetInstance().GetValueFloat(MONITOR_SECTION, CFG_NGINX_SECTION, CPU_USAGE, m_stNginxMointorCfg.fCpuUsageCfg);
    CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, CFG_NGINX_SECTION, NGINX_LOG_SIZE, m_stNginxMointorCfg.iNginxLogSizeCfg);
    return m_stNginxMointorCfg;
}

/*------------------------------------------------------------
Function Name: GetCommonMonitorCfg
Description  : 从配置文件中获取通用监控配置
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
monitor_common_config_t& CAbnormalHandler::GetCommonMonitorCfg()
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, RETRY_TIME, m_stCommonMonitorCfg.iRetryTime);
    if (iRet != MP_SUCCESS)
    {
        m_stCommonMonitorCfg.iRetryTime = DEFAULT_XML_RETRY_TIME_VALUE;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueInt32(MONITOR_SECTION, MONITOR_INTERVAL, m_stCommonMonitorCfg.iMonitorInterval);
    if (iRet != MP_SUCCESS)
    {
        m_stCommonMonitorCfg.iRetryTime = DEFAULT_XML_INTERVAL_VALUE;
    }
    return m_stCommonMonitorCfg;
}

/*------------------------------------------------------------
Function Name: CheckNginxMonitorValue
Description  : 检查nginx监控数据
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CAbnormalHandler::CheckNginxMonitorValue(monitor_data_t& monitorNginxData)
{
    //单位转换
    //配置文件中涉及容量的单位均为M，而此处PmSize容量为Kbyte，VmSize单位为Kbyte
    mp_uint64 ulTransPmSizeCfg = (mp_uint64)m_stNginxMointorCfg.iPmSizeCfg*1024;
    mp_uint64 ulTransVmSizeCfg = (mp_uint64)m_stNginxMointorCfg.iVmSizeCfg*1024;
    CHECK_VALUE(monitorNginxData.ulPmSize, ulTransPmSizeCfg, m_stNginxAbnormal.iPmSizeOverTimes);
    CHECK_VALUE(monitorNginxData.ulVmSize, ulTransVmSizeCfg, m_stNginxAbnormal.iVmSizeOverTimes);
    CHECK_VALUE(monitorNginxData.iHandlerNum, m_stNginxMointorCfg.iHandlerNumCfg, m_stNginxAbnormal.iHandlerOverTimes);
    CHECK_VALUE(monitorNginxData.iThreadNum, m_stNginxMointorCfg.iThreadNumCfg, m_stNginxAbnormal.iThreadOverTimes);
    CHECK_VALUE(monitorNginxData.fCpuUsage, m_stNginxMointorCfg.fCpuUsageCfg, m_stNginxAbnormal.iCpuUsageOverTimes);
}

/*------------------------------------------------------------
Function Name: CheckAgentMonitorValue
Description  : 检查agent监控数据
Return       :
call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CAbnormalHandler::CheckAgentMonitorValue(monitor_data_t& monitorAgentData)
{
    //单位转换
    //配置文件中涉及容量的单位均为M，而此处PmSize容量为Kbyte，VmSize单位为Kbyte，iTmpFile总容量的单位为byte
    mp_uint64 ulTransPmSizeCfg = (mp_uint64)m_stAgentMointorCfg.iPmSizeCfg*1024;
    mp_uint64 ulTransVmSizeCfg = (mp_uint64)m_stAgentMointorCfg.iVmSizeCfg*1024;
    mp_uint64 ulTmpFileSizeCfg = (mp_uint64)m_stAgentMointorCfg.iTmpFileTotalSizeCfg*1024*1024;
    CHECK_VALUE(monitorAgentData.ulPmSize, ulTransPmSizeCfg, m_stAgentAbnormal.iPmSizeOverTimes);
    CHECK_VALUE(monitorAgentData.ulVmSize, ulTransVmSizeCfg, m_stAgentAbnormal.iVmSizeOverTimes);
    CHECK_VALUE(monitorAgentData.iHandlerNum, m_stAgentMointorCfg.iHandlerNumCfg, m_stAgentAbnormal.iHandlerOverTimes);
    CHECK_VALUE(monitorAgentData.iThreadNum, m_stAgentMointorCfg.iThreadNumCfg, m_stAgentAbnormal.iThreadOverTimes);
    CHECK_VALUE(monitorAgentData.fCpuUsage, m_stAgentMointorCfg.fCpuUsageCfg, m_stAgentAbnormal.iCpuUsageOverTimes);
    CHECK_VALUE(monitorAgentData.ulTmpFileTotalSize, ulTmpFileSizeCfg, m_stAgentAbnormal.iTmpFileSizeOverTimes);
}

mp_void CAbnormalHandler::CheckAgentCpuUsage(mp_float fCpuUsage)
{
    //检测cpu利用率
    if ((m_stAgentAbnormal.iCpuUsageOverTimes > m_stCommonMonitorCfg.iRetryTime) && !m_bAgentCpuAlarmSend)
    {
        //发送cpu利用率过高告警，只要进程超高都要发送
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Agent process cpu usage overceed %f over %d times.",
            m_stAgentMointorCfg.fCpuUsageCfg, m_stAgentAbnormal.iCpuUsageOverTimes);
        mp_int32 iRet = SendCPUAlarm(AGENT_PROCESS);
        if (MP_SUCCESS != iRet)
        {
            m_iAgentAlarmSendFailedTimes++;
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Send agent cpu alarm failed, iRet = %d", iRet);
        }
        else
        {
            m_stAgentAbnormal.iCpuUsageOverTimes = 0;
            m_iAgentAlarmSendFailedTimes = 0;
            m_bAgentCpuAlarmSend = MP_TRUE;
            m_bAgentCpuAlarmResumed = MP_FALSE;
            m_bNeedResumed = MP_TRUE;
        }

        //失败超过指定次数后，不再发送恢复告警
        if (m_iAgentAlarmSendFailedTimes > ALARM_SEND_FAILED_TIME)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Alarm has been sent failed over %d times and will not send again.", ALARM_SEND_FAILED_TIME);
            m_stAgentAbnormal.iCpuUsageOverTimes = 0;
            m_bAgentCpuAlarmSend = MP_TRUE;
            m_bAgentCpuAlarmResumed = MP_FALSE;
        }
    }

    //agent和nginx进程cpu利用率都低于阈值才发送恢复告警
    if (fCpuUsage <= m_stAgentMointorCfg.fCpuUsageCfg)
    {
        m_stAgentAbnormal.iCpuUsageOverTimes = 0;
        m_bAgentCpuAlarmResumed = MP_TRUE;
        m_bNginxCpuAlarmResumed = MP_FALSE;
    }

}

mp_void CAbnormalHandler::CheckNginxCpuUsage(mp_float fCpuUsage)
{
    mp_bool bCheckFirst = ( (m_stNginxAbnormal.iCpuUsageOverTimes > m_stCommonMonitorCfg.iRetryTime) && !m_bNginxCpuAlarmSend );
    if ( bCheckFirst )
    {
        //发送cpu利用率过高告警
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Nginx process cpu usage overceed %f over %d times.",
            m_stNginxMointorCfg.fCpuUsageCfg, m_stNginxAbnormal.iCpuUsageOverTimes);
        mp_int32 iRet = SendCPUAlarm(NGINX_PROCESS);
        if (MP_SUCCESS != iRet)
        {
            m_iNginxAlarmSendFailedTimes++;
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Send nginx cpu alarm failed, iRet = %d", iRet);
        }
        else
        {
            m_stNginxAbnormal.iCpuUsageOverTimes = 0;
            m_iNginxAlarmSendFailedTimes = 0;
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Send alarm successfully.");
            m_bNginxCpuAlarmSend = MP_TRUE;
            m_bNginxCpuAlarmResumed = MP_FALSE;
            m_bNeedResumed = MP_TRUE;
        }

        //超过指定次数后，不再发送恢复告警
        if (m_iNginxAlarmSendFailedTimes > ALARM_SEND_FAILED_TIME)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Alarm has been sent failed over %d times and will not send again.", ALARM_SEND_FAILED_TIME);
            m_stNginxAbnormal.iCpuUsageOverTimes = 0;
            m_bNginxCpuAlarmSend = MP_TRUE;
            m_bNginxCpuAlarmResumed = MP_FALSE;
        }
    }

    //agent和nginx进程cpu利用率都低于阈值才发送恢复告警
    if (fCpuUsage <= m_stNginxMointorCfg.fCpuUsageCfg)
    {
        m_stNginxAbnormal.iCpuUsageOverTimes = 0;
        mp_bool bCheckSecond = ( !m_bNginxCpuAlarmResumed && m_bAgentCpuAlarmResumed && m_bNeedResumed );
        if ( bCheckSecond )
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Nginx process cpu usage overceed alarm is resumed.");
            mp_int32 iRet = ResumeCPUAlarm();
            if (MP_SUCCESS != iRet)
            {
                m_iNginxAlarmSendFailedTimes++;
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Resume agent cpu alarm failed, iRet = %d", iRet);
            }
            else
            {
                m_iNginxAlarmSendFailedTimes = 0;
                m_bNginxCpuAlarmSend = MP_FALSE;
                m_bAgentCpuAlarmSend = MP_FALSE;
                m_bNginxCpuAlarmResumed = MP_TRUE;
                m_bNeedResumed = MP_FALSE;
            }

            //超过指定次数后，不再发送恢复告警
            if (m_iNginxAlarmSendFailedTimes > ALARM_SEND_FAILED_TIME)
            {
                m_bNginxCpuAlarmSend = MP_FALSE;
                m_bNginxCpuAlarmResumed = MP_TRUE;
                m_bNeedResumed = MP_FALSE;
            }
        }
    }    
}

#ifdef WIN32
/*------------------------------------------------------------
Function Name: GetWinMonitorData
Description  : 获取windows监控配置
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAbnormalHandler::GetWinMonitorData(mp_string strPorcessName, monitor_data_t& stMonitorData)
{
    //获取进程句柄
    HANDLE hToolHelp = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (NULL == hToolHelp)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "CreateToolhelp32Snapshot failed!");
        CloseHandle(hToolHelp);
        return MP_FAILED;
    }

    PROCESSENTRY32W process;
    process.dwSize = sizeof(process);
    BOOL bMore = ::Process32FirstW(hToolHelp, &process);
    while (bMore)
    {
        mp_char buf[MAX_BUF_SIZE] = {0};
        ::WideCharToMultiByte( CP_ACP, 0, process.szExeFile, -1, buf, MAX_BUF_SIZE, NULL, NULL );
        if (0 == strcmp(buf, strPorcessName.c_str()))
        {
            monitor_data_t stMonitorDataTmp;
            stMonitorDataTmp.bExist = MP_TRUE;
            stMonitorDataTmp.iThreadNum = process.cntThreads;
            //尝试提权（64位机器必须提权才能对另外一个进程的参数进行操作）
            HANDLE hToken;
            if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken))
            {
                COMMLOG(OS_LOG_WARN,LOG_COMMON_WARN,"%s","Get Access Token failed!");
                CloseHandle(hToken);
                CloseHandle(hToolHelp);
                return MP_FAILED;
            }
            SetWinPrivilege(hToken,SE_DEBUG_NAME,TRUE);

            HANDLE hPro = ::OpenProcess(PROCESS_ALL_ACCESS,FALSE, process.th32ProcessID);
            if (NULL == hPro)
            {
                COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR,"Get handle faild,system error: %d", GetLastError());
                CloseHandle(hPro);
                CloseHandle(hToken);
                CloseHandle(hToolHelp);
                return false;
            }

            //获取进程句柄
            DWORD dwHandleCount = 0;
            ::GetProcessHandleCount(hPro, &dwHandleCount);
            stMonitorDataTmp.iHandlerNum = dwHandleCount;

            //获取物理内存和虚拟内存使用量
            PROCESS_MEMORY_COUNTERS pmc;
            ::GetProcessMemoryInfo(hPro, &pmc, sizeof(pmc));
            stMonitorDataTmp.ulVmSize = pmc.PagefileUsage/1024; // 单位kbyte
            stMonitorDataTmp.ulPmSize = pmc.WorkingSetSize/1024; //单位kbyte

            //获取cpu利用率
            stMonitorDataTmp.fCpuUsage = GetWinCPUUsage(hPro);
            CloseHandle(hToken);
            CloseHandle(hPro);
            stMonitorData = AddMonitorData(stMonitorData, stMonitorDataTmp);
        }
        bMore = ::Process32NextW(hToolHelp, &process);
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: SetWinPrivilege
Description  : 设置win权限
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CAbnormalHandler::SetWinPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if ( !LookupPrivilegeValue(
        NULL,
        lpszPrivilege,
        &luid ) )
    {
        return MP_TRUE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;


    if ( !AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES) NULL,
        (PDWORD) NULL) )
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR,"Call AdjustTokenPrivileges failed");
        return MP_FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR,"GetLastError = %d", ERROR_NOT_ALL_ASSIGNED);
        return MP_FALSE;
    }
    return MP_TRUE;
}

/*------------------------------------------------------------
Function Name: GetWinCPUUsage
Description  : 获取cpu利用率
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_float CAbnormalHandler::GetWinCPUUsage(const HANDLE hAgent)
{
    static mp_int32 processor_count_ = FAULT_NUM;

    static mp_int64 last_time_ = 0;
    static mp_int64 last_system_time_ = 0;

    FILETIME now;
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    mp_int64 system_time;
    mp_int64 now_time;

    if(processor_count_ == FAULT_NUM)
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        processor_count_ = info.dwNumberOfProcessors;
    }

    GetSystemTimeAsFileTime(&now);

    if (!GetProcessTimes(hAgent, &creation_time, &exit_time, &kernel_time, &user_time))
    {
        return FAULT_NUM;
    }

    system_time = (mp_int64)((mp_double)FileTimeTOUTC(kernel_time) / processor_count_ + (mp_double)FileTimeTOUTC(user_time) / processor_count_);
    now_time = FileTimeTOUTC(now);

    last_system_time_ = system_time;
    last_time_ = now_time;

    Sleep(2000);

    if (!GetProcessTimes(hAgent, &creation_time, &exit_time, &kernel_time, &user_time))
    {
        return FAULT_NUM;
    }
    GetSystemTimeAsFileTime(&now);
    system_time = (mp_int64)((mp_double)FileTimeTOUTC(kernel_time) / processor_count_ + (mp_double)FileTimeTOUTC(user_time) / processor_count_);
    now_time = FileTimeTOUTC(now);

    mp_double dDifSysTime = (mp_double)(system_time - last_system_time_);
    mp_double dDifNowTime = (mp_double)(now_time - last_time_);
    return (mp_float)((dDifSysTime/dDifNowTime)*100);
}

/*------------------------------------------------------------
Function Name: FileTimeTOUTC
Description  : filetime转换成utc时间
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int64 CAbnormalHandler::FileTimeTOUTC(const FILETIME& inputTime)
{
    ULARGE_INTEGER tmpTime;
    tmpTime.LowPart = inputTime.dwLowDateTime;
    tmpTime.HighPart = inputTime.dwHighDateTime;
    return (mp_int64)tmpTime.QuadPart;
}
#endif
