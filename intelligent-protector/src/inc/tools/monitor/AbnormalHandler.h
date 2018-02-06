/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _MONITOR_HANDLER_H_
#define _MONITOR_HANDLER_H_

#include "common/Types.h"
#include "common/Thread.h"
#include <sstream>

#define MONITOR_SECTION "Monitor"

#define THRD_CNT "thread_count"
#define HANDLER_CNT "handle_count"
#define PM_SIZE "pm_size"
#define VM_SIZE "vm_size"
#define CPU_USAGE "cpu_usage"
#define TMPFILE_SIZE "tmpfile_size"
#define RETRY_TIME "retry_time"
#define MONITOR_INTERVAL "monitor_interval"
#define NGINX_LOG_SIZE "nginx_log_size"
#define NGINX_LOG_FILE "error.log"

#ifdef WIN32
#define ROATE_NGINX_LOG_SCRIPT "rotatenginxlog.bat"
#else
#define ROATE_NGINX_LOG_SCRIPT "rotatenginxlog.sh"
#endif

#define MAX_TMP_EXIST_TIME 3600*24  //临时文件存在的最长时间，单位秒
#define MONITOR_SLEEP_TIME 1000
#define ALARM_SEND_FAILED_TIME 3
#define MAX_BUF_SIZE 200
#define FAULT_NUM -1

#define DEFAULT_XML_RETRY_TIME_VALUE 3
#define DEFAULT_XML_INTERVAL_VALUE 30

#define HP_TOP_CMD "top -n 1000 -f"

#define AGENT_PROCESS        0
#define NGINX_PROCESS        1

typedef struct tag_abnormal_occur_times
{
    tag_abnormal_occur_times()
    {
        iPmSizeOverTimes = 0;
        iVmSizeOverTimes = 0;
        iHandlerOverTimes = 0;
        iThreadOverTimes = 0;
        iCpuUsageOverTimes = 0;
        iTmpFileSizeOverTimes = 0;
    }
    mp_int32 iPmSizeOverTimes;  //物理内存连续超过配置文件次数
    mp_int32 iVmSizeOverTimes;  //虚拟内存连续超过配置文件次数
    mp_int32 iHandlerOverTimes; //文件描述符个数连续超过配置文件次数
    mp_int32 iThreadOverTimes;  //线程个数连续超过配置文件次数
    mp_int32 iCpuUsageOverTimes; //CPU利用率连续超过配置文件次数
    mp_int32 iTmpFileSizeOverTimes;
}abnormal_occur_times_t;

typedef struct tag_monitor_data
{
    tag_monitor_data()
    {
        bExist = MP_FALSE;
        iHandlerNum = 0;
        iThreadNum = 0;
        ulPmSize = 0;
        ulVmSize = 0;
        fCpuUsage = 0.0;
        ulTmpFileTotalSize = 0;
        uiNginxLogSize = 0;
    }
    mp_bool bExist;
    mp_int32 iHandlerNum;
    mp_int32 iThreadNum;
    mp_uint64 ulPmSize;  //单位Kbyte
    mp_uint64 ulVmSize;  //单位Kbyte
    mp_float fCpuUsage; //单位%
    mp_uint32 uiNginxLogSize;     //单位Kbyte
    mp_uint64 ulTmpFileTotalSize; //单位Kbyte
}monitor_data_t;

typedef struct tag_monitor_process_config
{
    tag_monitor_process_config()
    {
        bMonitored = MP_FALSE;
        iPmSizeCfg = 0;
        iVmSizeCfg = 0;
        iHandlerNumCfg = 0;
        iThreadNumCfg = 0;
        fCpuUsageCfg = 0;
        iTmpFileTotalSizeCfg = 0;
        iNginxLogSizeCfg = 0;
    }
    mp_bool bMonitored;
    mp_int32 iPmSizeCfg;
    mp_int32 iVmSizeCfg;
    mp_int32 iHandlerNumCfg;
    mp_int32 iThreadNumCfg;
    mp_float fCpuUsageCfg;
    mp_int32 iTmpFileTotalSizeCfg;  //单位K,agent进程特有参数
    mp_int32 iNginxLogSizeCfg;      //单位K,nginx进程特有参数
}monitor_process_config_t;

typedef struct tag_monitor_common_config
{
    tag_monitor_common_config()
    {
        iRetryTime = 3;
        iMonitorInterval = 30;
    }
    mp_int32 iRetryTime;
    mp_int32 iMonitorInterval;

}monitor_common_config_t;

class CAbnormalHandler
{
public:
    static CAbnormalHandler& GetInstance()
    {
        return m_instance;
    }

    ~CAbnormalHandler()
    {
    }

    mp_int32 Init();
    mp_int32 Handle();
private:
    CAbnormalHandler()
    {
        m_bAgentCpuAlarmSend = MP_FALSE;
        m_bAgentCpuAlarmResumed = MP_FALSE;
        m_bNginxCpuAlarmSend = MP_FALSE;
        m_bNginxCpuAlarmResumed = MP_FALSE;
        m_bNeedResumed = MP_TRUE;
        m_iAgentAlarmSendFailedTimes = 0;
        m_iNginxAlarmSendFailedTimes = 0;
        (mp_void)memset_s(&m_hHandleThread, sizeof(m_hHandleThread), 0, sizeof(m_hHandleThread));
        //Coverity&Fortify误报:UNINIT_CTOR
        //Coveirty&Fortify不认识公司安全函数memset_s，提示m_dispatchTid.os_id未初始化
        m_iThreadStatus = THREAD_STATUS_IDLE;
        m_bNeedExit = MP_FALSE;
    }

    mp_int32 GetAgentMonitorData(monitor_data_t& stMonitorData);
    mp_int32 GetNginxMonitorData(monitor_data_t& stMonitorData);
    mp_int32 GetMonitorData(mp_string strProcessID, monitor_data_t& stMonitorData);
    mp_int32 GetHPMonitorData(mp_string strProcessID, monitor_data_t& stMonitorData);
    mp_int32 StartAgent();
    mp_int32 StartNginx();
    mp_int32 RestartAgent();
    mp_int32 RestartNginx();
    mp_int32 SendCPUAlarm(mp_int32 iProcessType);
    mp_int32 ResumeCPUAlarm();
    monitor_process_config_t& GetAgentMonitorCfg();
    monitor_process_config_t& GetNginxMonitorCfg();
    monitor_common_config_t& GetCommonMonitorCfg();
    mp_int32 MonitorAgent();
    mp_int32 MonitorNginx();
    static monitor_data_t AddMonitorData(monitor_data_t& stMonitorData1, monitor_data_t& stMonitorData2);
    static mp_void ClearMonitorData(monitor_data_t& stMonitorData);
    static mp_void ClearAbnormalOccurTimes(abnormal_occur_times_t& stAbnormalOccurTimes);
    static mp_int32 DeleteTmpFile();
    static mp_int32 GetTmpFileTotalSize(mp_uint64& ulSize);
    static mp_uint64 GetKSize(mp_string strSize);
    mp_bool NeedExit();
    mp_void SetThreadStatus(mp_int32 iThreadStatus);
    mp_void RotateNginxLog();
    mp_void CheckNginxMonitorValue(monitor_data_t& monitorNginxData);
    mp_void CheckAgentMonitorValue(monitor_data_t& monitorAgentData);
    mp_void CheckAgentCpuUsage(mp_float fCpuUsage);
    mp_void CheckNginxCpuUsage(mp_float fCpuUsage);

#ifdef WIN32
    static DWORD WINAPI HandleFunc(mp_void* pThis);
    mp_int32 GetWinMonitorData(mp_string strPorcessName, monitor_data_t& stMonitorData);
    mp_float GetWinCPUUsage(const HANDLE hAgent);
    static mp_bool SetWinPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
    static mp_int64 FileTimeTOUTC(const FILETIME& inputTime);
#else
    static mp_void* HandleFunc(mp_void* pThis);
#endif

private:
    static CAbnormalHandler m_instance;         //单例对象
    thread_id_t m_hHandleThread;                //线程句柄
    abnormal_occur_times_t m_stAgentAbnormal;   //Agent监控进程异常处理数据
    abnormal_occur_times_t m_stNginxAbnormal;   //Nginx监控进程异常处理数据
    monitor_process_config_t m_stAgentMointorCfg;       //Agent进程监控配置数据
    monitor_process_config_t m_stNginxMointorCfg;       //Nginx进程监控配置数据
    monitor_common_config_t m_stCommonMonitorCfg; //进程监控公共配置
    mp_bool m_bAgentCpuAlarmSend;  //是否已经发送agent cpu利用率高告警标识
    mp_bool m_bAgentCpuAlarmResumed;  //是否已经发送agent cpu利用率高恢复告警标识
    mp_bool m_bNginxCpuAlarmSend;  //是否已经发送nginx cpu利用率高告警标识
    mp_bool m_bNginxCpuAlarmResumed;  //是否已经发送nginx cpu利用率高恢复告警标识
    mp_bool m_bNeedResumed; //是否需要发送恢复告警
    mp_int64 m_iAgentAlarmSendFailedTimes;  //Agent cpu告警发送失败次数
    mp_int64 m_iNginxAlarmSendFailedTimes;  //Nginx cpu告警发送失败次数
    volatile mp_int32 m_iThreadStatus;
    volatile mp_bool m_bNeedExit;
};

#define CHECK_VALUE(iValue, iConfigValue, iOverTimes)  (iValue > iConfigValue) ? iOverTimes++ : (iOverTimes = 0); \
    if (iValue > iConfigValue)  \
    {\
        ostringstream oss;\
        oss << #iValue << " overceed config value(" << iConfigValue << ") " << iOverTimes << " times.";\
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "%s", oss.str().c_str());\
    }\

#endif

