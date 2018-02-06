/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_DEFINES_H_
#define _AGENT_DEFINES_H_

#include "common/Types.h"
#include "securec.h"

#define DECIMAL                        10             //十进制转换

#define MAX_FULL_PATH_LEN              300
#define MAX_PATH_LEN                   260
#define MAX_FILE_NAME_LEN              80
#define MAX_LINE_SIZE                  2048
#define MAX_SINGED_INTEGER_VALUE       2147483647
#define MAX_HOSTNAME_LEN               260            //主机名长度
#define MAX_MAIN_CMD_LENGTH            1000           //系统命令长度
#define MAX_ARRAY_SN_LEN               20             //SN最大长度
#define MAX_ERROR_MSG_LEN              256

#define HOST_TYPE_WINDOWS              1              //Windows
#define HOST_TYPE_REDHAT               2              //RedHat
#define HOST_TYPE_HP_UX_IA             3              //HPUX IA
#define HOST_TYPE_SOLARIS              4              //SOLARIS
#define HOST_TYPE_AIX                  5              //AIX
#define HOST_TYPE_SUSE                 6              //SUSE
#define HOST_TYPE_ROCKY                7              //ROCKY
#define HOST_TYPE_OEL                  8              //OEL
#define HOST_TYPE_ISOFT                9

//device type
#define DEVICE_TYPE_FILESYS             0             //文件系统
#define DEVICE_TYPE_RAW                 1             //裸设备
#define DEVICE_TYPE_ASM_LIB             2             //ASMLib磁盘
#define DEVICE_TYPE_ASM_RAW             3             //ASM裸设备
#define DEVICE_TYPE_ASM_LINK            4             //ASM软链接
#define DEVICE_TYPE_ASM_UDEV            5             //ASMOnUdev
#define DEVICE_TYPE_ASM_DISK_ID         6             //windows ASM磁盘标识符

//volume type
#define VOLUME_TYPE_SIMPLE              0             //简单卷（无卷管理）
#define VOLUME_TYPE_LINUX_LVM           1             //Linux LVM
#define VOLUME_TYPE_LINUX_VXVM          2             //Linux VxVM
#define VOLUME_TYPE_AIX_LVM             3             //AIX卷管理
#define VOLUME_TYPE_HP_LVM              4             //HP卷管理
#define VOLUME_TYPE_UDEV                5             //UDEV设备映射

#define FREEZE_STAT_FREEZED             0             //冻结中
#define FREEZE_STAT_UNFREEZE            1             //解冻
#define FREEZE_STAT_UNKNOWN             2             //未知

#ifdef WIN32
//windows errorcode define
#define WIN_ERROR_FILE_NOT_FOUND        2        //The system cannot find the file specified.

#endif

//脚本参数
#define SCRIPTPARAM_INSTNAME                "INSTNAME="
#define SCRIPTPARAM_DBNAME                  "DBNAME="
#define SCRIPTPARAM_DBUSERNAME              "DBUSERNAME="
#define SCRIPTPARAM_DBPASSWORD              "DBPASSWORD="
#define SCRIPTPARAM_CLUSTERTYPE             "CLUSTERTYPE="
#define SCRIPTPARAM_RESGRPNAME              "RESGRPNAME="
#define SCRIPTPARAM_DEVGRPNAME              "DEVGRPNAME="
#define SQLSERVER_SCRIPTPARAM_TABNAME       "TABLESPACENAME="
#define SQLSERVER_SCRIPTPARAM_CLUSTERFLAG   "ISCLUSTER="
#define SQLSERVER_SCRIPTPARAM_CHECKTYPE     "CHECKTYPE="
#define SCRIPTPARAM_CLUSTERNAME             "CLUSTERNAME="


// *** BEGIN *** DTS2014071801749 y00275736 20014-07-24
//该超时时间由10秒增加到60秒，带IO测试时10秒超时冻结操作失败概率很高
//Max wait time for frozen event
#define VSS_TIMEOUT_FREEZE_MSEC         60000
// *** END *** DTS2014071801749 y00275736 20014-07-24
//Call QueryStatus every 10 ms while waiting for frozen event
#define VSS_TIMEOUT_EVENT_MSEC          10
#define VSS_TIMEOUT_MSEC                (60*1000)
#define VSS_EXEC_MUTEX_TIMEOUT          60000

//VSS writer name
#define VSS_SQLSERVER_WRITER_NAME        "SqlServerWriter"
#define VSS_SQLSERVER_WRITER_NAME_W      L"SqlServerWriter"
#define VSS_EXCHANGE_WRITER_NAME         "Microsoft Exchange Writer"
#define VSS_EXCHANGE_WRITER_NAME_W       L"Microsoft Exchange Writer"
#define VSS_FILESYSTEM_WRITER_NAME       "File System Writer"
#define VSS_FILESYSTEM_WRITER_NAME_W     L"File System Writer"
//Events name
#define EVENT_NAME_FROZEN                L"Global\\RDVSSEvent-frozen"
#define EVENT_NAME_THAW                  L"Global\\RDVSSEvent-thaw"
#define EVENT_NAME_TIMEOUT               L"Global\\RDVSSEvent-timeout"
#define EVENT_NAME_ENDBACKUP             L"Global\\RDVSSEvent-endbackup"
#define RD_AGENT_SERVICE_NAME_W          L"ReplicationDirector Agent"


#define LENGTH_SEC_DESC                128
#define LENGTH_DISK_DESC               1024    //磁盘集的字符串长度
#define DEFAULT_RDVSS_LOG_PATH         "C:\\"

//分隔符
#define STR_COMMA                      ","            //字符串中的逗号
#define STR_SEMICOLON                  ";"            //字符串中的分号，Windows下已此为分隔符，避免与路径中的冒号冲突
#define STR_COLON                      ":"            //字符串中的冒号
#define STR_DASH                       "-"
#define STR_PLUS                       "+"
#define STR_VERTICAL_LINE              "|"
#define STR_DOUBLE_VERTICAL_LINE       "||"
#define STR_ADDRESS                    "&"
#define STR_DOUBLE_ADDRESS             "&&"
#define STR_SPACE                     " "
#define CHAR_COMMA                     ','
#define CHAR_SEMICOLON                 ';'
#define CHAR_COLON                     ':'
#define CHAR_VERTICAL_LINE             '|'
#define CHAR_SLASH                     '/'


//日志文件名称
#define AGENT_LOG_NAME                 "rdagent.log"
#define ROOT_EXEC_LOG_NAME             "rootexec.log"
#define CRYPTO_LOG_NAME                "crypto.log"
#define RD_VSS_LOG_NAME                "rdvss.log"
#define MONITOR_LOG_NAME               "monitor.log"
#define WIN_SERVICE_LOG_NAME           "winservice.log"
#define XML_CFG_LOG_NAME               "xmlcfg.log"
#define AGENT_CLI_LOG_NAME             "agentcli.log"
#define GET_INPUT_LOG_NAME             "getinput.log"
#define SCRIPT_SIGN_LOG_NAME           "scriptsign.log"
#define DATA_MIGRA_LOG_NAME            "datamigra.log"
#define AGENT_LOG_ZIP_NAME             "sysinfo"

//可执行程序名称
#define AGENT_EXEC_NAME                "rdagent"
#define ROOT_EXEC_NAME                 "rootexec"
#define CRYPT_TOOL_NAME                "crypto"
#define MONITOR_EXEC_NAME              "monitor"
#define NGINX_EXEC_NAME                "rdnginx"
#define WIN_SERVICE_EXEC_NAME          "winservice"
#define XML_CFG_EXEC_NAME              "xmlcfg"
#define GET_INPUT_EXEC_NAME            "getinput"
//nginx作为参数名
#define NGINX_AS_PARAM_NAME            "nginx"

//配置文件
#define AGENT_XML_CONF                 "agent_cfg.xml"
#define AGENT_PLG_CONF                 "pluginmgr.xml"
#define AGENT_NGINX_CONF_FILE          "nginx.conf"
#define AGENT_SCRIPT_SIGN              "script.sig"

//nginx优化参数
#define CFG_SSL_KEY_PASSWORD            "ssl_key_password"
#define SSL_PASSWORD_TEMP_FILE          "nginxPassword.tmp"
#define NGINX_START                     "startnginx"
#define AGENTCLI_UNIX                   "agentcli"
//脚本文件
#define SCRIPT_QUERY_INITIATOR         "initialtor.sh"
#define SCRIPT_ACTION_DB2              "Db2_sample.sh"
#ifndef WIN32
#define START_SCRIPT                   "agent_start.sh"
#define STOP_SCRIPT                    "agent_stop.sh"
#define ZIP_SUFFIX                     ".tar.gz"
#else
#define START_SCRIPT                   "process_start.bat"
#define STOP_SCRIPT                    "process_stop.bat"
#define ZIP_SUFFIX                     ".zip"

//nginx优化参数
#define AGENTCLI_EXE                   "agentcli.exe"

#endif

//目录名称
#define AGENT_TMP_DIR                  "tmp"
#define AGENT_CONF_DIR                 "conf"
#define AGENT_BIN_DIR                  "bin"
#define AGENT_PLUGIN_DIR               "plugins"
#define AGENT_LOG_DIR                  "log"
#define AGENT_THIRDPARTY_DIR           "thirdparty"
#define AGENT_DB                       "db"
#define AGENT_NGINX                    "nginx"
#define AGENT_NGINX_LOGS               "logs"
#define AGENT_NGINX_CONF               "conf"

//一些公用符号定义
#define NODE_COLON                        ":"            //字符串中的冒号
#define NODE_SEMICOLON                    ";"            //字符串中的分号

//windows服务相关
#define MONITOR_SERVICE_NAME "RdMonitor"
#define AGENT_SERVICE_NAME "RdAgent"
#define NGINX_SERVICE_NAME "RdNginx"
#define INSTALL_OPERATOR "install"
#define UNINSTALL_OPERATOR "uninstall"
#define RUN_OPERATOR "run"

//进程pid文件
#define AGENT_PID "rdagent.pid"
#define NGINX_PID "nginx.pid"
#define MONITOR_PID "monitor.pid"

//host SN文件名
#define HOSTSN_FILE                   "HostSN"

//附件相关内部json对象key
#define REST_PARAM_ATTACHMENT_NAME            "attachmentName"    //附件名称
#define REST_PARAM_ATTACHMENT_PATH            "attachmentPath"    //附件完整路径，包括文件名称

//工作线程总数
//#define MAX_WORKER_NUM                 1

#ifdef WIN32
#define DLLAPI             //解决静态链接fcgi找不到符号的问题，提示找不到__imp_FXGXxxxxx符号，而__imp_符号是动态库导入库
                           //的符号前缀，是在使用DLLIMPORT的时候生成的.
#ifdef AGENT_DLL_EXPORTS   //specified in vs
#define AGENT_API                      __declspec(dllexport)
#else
#define AGENT_API                      __declspec(dllimport)
#endif
#define AGENT_EXPORT                   __declspec(dllexport)
#define AGENT_IMPORT                   __declspec(dllimport)
#define PATH_SEPARATOR                 "\\"
#define IS_DIR_SEP_CHAR(x)             ('\\'==(x) || '/'==(x))
#define LIB_SUFFIX                     ".dll"
#define PATH_SEPCH                     ';'
#define GETCH                          _getch()
#define SNPRINTF_S(dest, length, count, pszformat,...)    _snprintf_s(dest, length, count, pszformat, __VA_ARGS__)
#define SNWPRINTF_S(dest, length, count, pszformat, ...)  _snwprintf_s(dest, length, count, pszformat, __VA_ARGS__)
#define ITOA(val, buf, size, radix)                       _itoa_s(val, buf, size, radix)
#define I64ITOA(val, buf, size, radix)                     _i64toa_s(val, buf, size, radix)
#define VSNPRINTF_S(dest, length, count, format, valist)  vsnprintf_s(dest, length, count, format, valist);
#define VSNWPRINTF_S(dest, length, count, format, valist) _vsnwprintf_s(dest, length, count, format, valist);
#else //WIN32
#define AGENT_API
#define AGENT_EXPORT
#define AGENT_IMPORT
#define PATH_SEPARATOR                 "/"
#define IS_DIR_SEP_CHAR(x)             ('/'==(x))
#define LIB_SUFFIX                     ".so"
#define PATH_SEPCH                     ':'
#define GETCH                          getch()
#define SNPRINTF_S(dest, length, count, pszformat,...)   snprintf_s(dest, length, count, pszformat, __VA_ARGS__)
#define ITOA(val, buf, size, radix)                      itoa(val, buf, radix)
#define VSNPRINTF_S(dest, length, count, format, valist) vsnprintf_s(dest, length, count, format, valist);
#endif


#define IS_SPACE(x) ( x == ' ' || x == '\t')
#define strempty(str) (0 == str || 0 == (*str))

//为配合使用公司安全函数securec需要使用宏CHECK_FAIL和CHECK_NOT_OK
//安全函数snprintf_s需要用CHECK_FAIL进行判断，只有-1时表示出错
#define CHECK_FAIL( Call )                                                                        \
{                                                                                                 \
    mp_int32 iCheckFailRet = Call;                                                                \
    if (MP_FAILED == iCheckFailRet)                                                               \
    {                                                                                             \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call %s failed, ret %d.", #Call, iCheckFailRet); \
        return MP_FAILED;                                                                         \
    }                                                                                             \
}

#define CHECK_NOT_OK( Call )                                                                      \
{                                                                                                 \
    mp_int32 iCheckNotOkRet = Call;                                                               \
    if (EOK != iCheckNotOkRet)                                                                    \
    {                                                                                             \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);\
        return MP_FAILED;                                                                         \
    }                                                                                             \
}

#define CHECK_FAIL_NOLOG( Call )                                                                  \
{                                                                                                 \
    mp_int32 iCheckFailRet = Call;                                                                \
    if (MP_FAILED == iCheckFailRet)                                                               \
    {                                                                                             \
        return MP_FAILED;                                                                         \
    }                                                                                             \
}

//将调用返回码返回
#define CHECK_FAIL_EX( Call )                                                                     \
{                                                                                                 \
    mp_int32 iCheckNotOkRet = Call;                                                               \
    if (MP_SUCCESS != iCheckNotOkRet)                                                             \
    {                                                                                             \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);\
        return iCheckNotOkRet;                                                                    \
    }                                                                                             \
}



//打印宏名称
#define MACR(x) #x

//数据库冻结解冻状态，多个模块公用
enum DB_STATUS
{
    DB_FREEZE = 0,
    DB_UNFREEZE,
    DB_UNKNOWN,
    DB_BUTT
};


#endif //_AGENT_DEFINES_H_

