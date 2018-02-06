/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_ERROR_CODE_H_
#define _AGENT_ERROR_CODE_H_

#include "common/Types.h"
#include <map>

//********************************************************
//BEGIN********内部错误码，不返回给Server(部分函数保留内部错误码，需要时直接返回给Server)
//********************************************************
#define ERROR_INNER_THAW_NOT_MATCH                          0x70000001  //解冻操作和冻结操作不匹配
//********************************************************
//END**********内部错误码，不返回给Server
//********************************************************

//BEGIN***********************R5版本返回给server错误码，范围0x4003291A-0x400329FF************************//
#define ERROR_COMMON_OPER_FAILED                            0xFFFFFFFF   //执行失败

//************公共错误码***************************范围0x4003291A - 0x4003294F//
#define ERROR_COMMON_INVALID_PARAM                          0x4003291A  //参数错误
#define ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED               0x4003291B  //检查脚本签名失败
#define ERROR_COMMON_RECOVER_INSTANCE_NOSTART               0x4003291C  //数据库实例未启动
#define ERROR_COMMON_DB_USERPWD_WRONG                       0x4003291D  //数据库用户名或密码错误
#define ERROR_COMMON_DB_INSUFFICIENT_PERMISSION             0x4003291E  //数据库用户权限不足
#define ERROR_COMMON_FUNC_UNIMPLEMENT                       0x4003291F  //功能未实现
#define ERROR_COMMON_READ_CONFIG_FAILED                     0x40032921  //读取配置文件失败
#define ERROR_COMMON_DLL_LOAD_FAILED                        0x40032924  //动态库加载失败
#define ERROR_COMMON_SYSTEM_CALL_FAILED                     0x40032925  //系统调用失败(替换成具体场景错误码)
#define ERROR_COMMON_CLIENT_IS_LOCKED                       0x40032926  //客户端被锁定(描述信息待讨论)
#define ERROR_COMMON_SCRIPT_FILE_NOT_EXIST                  0x40032927  //脚本文件不存在
#define ERROR_COMMON_SCRIPT_EXEC_FAILED                     0x40032928  //脚本执行失败
#define ERROR_COMMON_PLUGIN_LOAD_FAILED                     0x40032929  //插件加载失败
#define ERROR_COMMON_NOT_HUAWEI_LUN                         0x4003292B  //存储信息无法识别
#define ERROR_COMMON_USER_OR_PASSWD_IS_WRONG                0x4003292C  //用户名或密码错误
#define ERROR_COMMON_QUERY_APP_LUN_FAILED                   0x4003292F  //查询应用LUN信息失败
#define ERROR_COMMON_DEVICE_NOT_EXIST                       0x40032931  //指定设备不存在
#define ERROR_COMMON_APP_FREEZE_FAILED                      0x40032932  //冻结失败(Oracle/db2不适用冻结术语,需修改)
#define ERROR_COMMON_APP_THAW_FAILED                        0x40032933  //解冻失败(Oracle/db2不适用冻结术语,需修改)
#define ERROR_COMMON_APP_FREEZE_TIMEOUT                     0x40032934  //冻结超时(Oracle/db2不适用冻结术语,需修改)
#define ERROR_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE        0x40032935  //不支持数据库文件部署在不同类型的磁盘上
#define ERROR_COMMON_PROC_REQUEST_BUSY                      0x40032936  //Agent业务忙
#define ERROR_COMMON_DB_FILE_NOT_EXIST                      0x40032937  //数据库文件不存在

//************Array&Disk***********************************范围0x40032950 - 0x4003295F//
#define ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED               0x40032954  //获取磁盘的裸设备名称失败
#define ERROR_DISK_ONLINE_FAILED                            0x40032956  //上线磁盘失败
#define ERROR_DISK_SCAN_FAILED                              0x40032957  //扫描磁盘失败
#define ERROR_DISK_GET_PARTITION_INFO_FAILED                0x40032958  //获取磁盘分区信息失败
#define ERROR_DISK_GET_DISK_INFO_FAILED                     0x40032959  //获取磁盘信息失败
#define ERROR_DISK_GET_VOLUME_PATH_FAILED                   0x4003295A  //获取卷路径失败

//************Host***********************************范围0x40032960 - 0x4003296F//
#define ERROR_HOST_VERIFY_SNMP_FAILED                       0x40032960  //SNMP协议参数不匹配
#define ERROR_HOST_GETINFO_FAILED                           0x40032961  //查询主机信息失败
#define ERROR_HOST_UNREG_TRAPSERVER_FAILED                  0x40032962  //删除Trap IP地址失败
#define ERROR_HOST_THIRDPARTY_GETFILE_FAILED                0x40032963  //查询第三方脚本失败
#define ERROR_HOST_THIRDPARTY_EXEC_FAILED                   0x40032964  //执行第三方脚本失败
#define ERROR_HOST_REG_TRAPSERVER_FAILED                    0x40032965  //注册Trap IP地址失败
#define ERROR_HOST_GET_INIATOR_FAILED                       0x40032966  //查询启动器信息失败
#define ERROR_HOST_GET_TIMEZONE_FAILED                      0x40032967  //查询主机时区信息失败
#define ERROR_HOST_LOG_IS_BEENING_COLLECTED                 0x40032967  //日志正在收集

//************Device*********************************范围0x40032970 - 0x4003298F//
//filesys
#define ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST          0x40032970  //挂载点目录不存在
#define ERROR_DEVICE_FILESYS_MOUTN_DEV_IS_MOUNTED           0x40032971  //设备已经挂载到其他挂载点
#define ERROR_DEVICE_FILESYS_MOUNT_POINT_OCCUPIED           0x40032972  //指定的挂载点已经被占用
#define ERROR_DEVICE_FILESYS_OFFLINE_VOLUME_FAILED          0x40032973  //下线卷失败
#define ERROR_DEVICE_FILESYS_DELETE_DRIVER_LETTER_FAILED    0x40032974  //删除盘符失败
#define ERROR_DEVICE_FILESYS_UNMOUNT_FAILED                 0x40032975  //去挂载失败
#define ERROR_DEVICE_FILESYS_GET_DEV_FAILED                 0x40032976  //查询设备信息失败
#define ERROR_DEVICE_FILESYS_MOUNT_FAILED                   0x40032977  //挂载失败
#define ERROR_DEVICE_FILESYS_QUERY_INFO_FAILED              0x40032978  //查询文件系统信息失败
//raw
#define ERROR_DEVICE_RAW_USED_BY_OTHER_DEV                  0x40032979  //裸设备已被占用
#define ERROR_DEVICE_RAW_START_FAILED                       0x4003297A  //启动裸设备服务失败
#define ERROR_DEVICE_RAW_DELETE_FAILED                      0x4003297B  //删除裸设备失败
#define ERROR_DEVICE_RAW_CREATE_FAILED                      0x4003297C  //创建裸设备失败
//lvm
#define ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED             0x4003297D  //查询卷组信息失败 
#define ERROR_DEVICE_LVM_EXPORT_VG_FAILED                   0x4003297E  //导出卷组失败
#define ERROR_DEVICE_LVM_IMPORT_VG_FAILED                   0x4003297F  //导入卷组失败
#define ERROR_DEVICE_LVM_GET_PV_FAILED                      0x40032980  //查询物理卷信息失败
#define ERROR_DEVICE_LVM_ACTIVE_VG_FAILED                   0x40032981  //激活卷组失败
#define ERROR_DEVICE_LVM_DEACTIVE_VG_FAILED                 0x40032982  //去激活卷组失败
#define ERROR_DEVICE_VXVM_SCAN_DISK_FAILED                  0x4003298A  //VXVM扫描磁盘失败
//link
#define ERROR_DEVICE_LINK_USED_BY_OTHER_DEV                 0x40032983  //软连接已被占用
#define ERROR_DEVICE_LINK_CREATE_FAILED                     0x40032984  //创建软连接失败
#define ERROR_DEVICE_LINK_DELETE_FAILED                     0x40032985  //删除软连接失败
//udev
#define ERROR_DEVICE_UDEV_CREATE_FAILED                     0x40032986  //写入udev规则失败
#define ERROR_DEVICE_UDEV_DELETE_FAILED                     0x40032987  //删除udev规则失败
//asm
#define ERROR_DEVICE_ASM_SCAN_ASMLIB_FAILED                 0x40032988  //扫描ASM磁盘失败
//permission
#define ERROR_DEVICE_PERMISSION_SET_FAILED                  0x40032989  //设置权限失败

//***********Oracle错误码****************************范围0x40032990 - 0x400329AF//
#define ERROR_ORACLE_ASM_DBUSERPWD_WRONG                    0x40032990  //ASM用户或密码错误 
#define ERROR_ORACLE_ASM_RECOVER_INSTANCE_NOSTART           0x40032991  //ASM实例未启动 
#define ERROR_ORACLE_ASM_INSUFFICIENT_WRONG                 0x40032992  //ASM用户权限不足
#define ERROR_ORACLE_NOARCHIVE_MODE                         0x40032993  //数据库未开启归档模式
#define ERROR_ORACLE_OVER_ARCHIVE_USING                     0x40032994  //数据库归档目录空闲空间超过阈值
#define ERROR_ORACLE_OVER_MAX_LINK                          0x40032995  //数据库连接已超过最大连接数
#define ERROR_ORACLE_IN_BACKUP                              0x40032996  //数据库已处于热备模式
#define ERROR_ORACLE_NOT_IN_BACKUP                          0x40032997  //数据库未处于热备模式
#define ERROR_ORACLE_ARCHIVE_FAILED                         0x40032998  //数据库强制归档失败
#define ERROR_ORACLE_DB_ALREADYRUNNING                      0x40032999  //数据库已处于运行状态
#define ERROR_ORACLE_DB_ALREADYMOUNT                        0x4003299A  //数据库已处于挂载状态
#define ERROR_ORACLE_DB_ALREADYOPEN                         0x4003299B  //数据库已处于打开状态
#define ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT             0x4003299C  //ASM磁盘组已被挂载
#define ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT                 0x4003299D  //ASM磁盘组未被挂载
#define ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED                0x4003299E  //数据库开启热备模式失败
#define ERROR_ORACLE_END_HOT_BACKUP_FAILED                  0x4003299F  //数据库结束热备模式失败
#define ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT               0x400329A0  //数据库开启热备模式超时
#define ERROR_ORACLE_TRUNCATE_LOG_FAILED                    0x400329A1  //删除数据库实例的归档日志失败
#define ERROR_ORACLE_TNS_PROTOCOL_ADAPTER                   0x400329A2  //TNS适配器错误
#define ERROR_ORACLE_START_INSTANCES_FAILED                 0x400329A3  //启动数据库实例失败
#define ERROR_ORACLE_INSTANCE_NOT_CDB						0x400329A4	//查询插件数据库时下发的实例不是容器数据库实例

//***********DB2错误码*******************************范围0x400329B0 - 0x400329B9//
#define ERROR_DB2_SUSPEND_IO_FAILED                         0x400329B0 //数据库悬挂IO失败
#define ERROR_DB2_RESUME_IO_FAILED                          0x400329B1 //数据库解除悬挂IO失败
#define ERROR_DB2_SUSPEND_IO_TIMEOUT                        0x400329B2 //数据库悬挂IO超时


//**********SqlServer错误码**************************范围0x400329BA - 0x400329C9//
#define ERROR_SQLSERVER_GET_DB_STATUS_FAILED                0x400329BA  //查询数据库状态失败
#define ERROR_SQLSERVER_DB_STATUS_OFFLINE                   0x400329BB  //数据库不在线
#define ERROR_SQLSERVER_DB_NOT_EXIST                        0x400329BC  //数据库不存在
#define ERROR_SQLSERVER_INSTANCE_NOT_EXIST                  0x400329BD  //数据库实例不存在 
#define ERROR_SQLSERVER_START_INSTANCE_FAILED               0x400329BE  //启动数据库实例失败
#define ERROR_SQLSERVER_DB_LIST_IS_NULL                     0x400329BF  //数据库信息列表为空
#define ERROR_SQLSERVER_START_DB_FAILED                     0x400329C0  //启动数据库失败
#define ERROR_SQLSERVER_STOP_DB_FAILED                      0x400329C1  //停止数据库失败


//**********Exchange错误码**************************范围0x400329CA - 0x400329D5//
#define ERROR_EXCHANGE_REMOVE_FAILED                        0x400329CA  //邮箱数据库卸载清理失败
#define ERROR_EXCHANGE_MOUNT_FAILED                         0x400329CB  //邮箱数据库装载启动失败
#define ERROR_EXCHANGE_SOFTRECVERY_FAILED                   0x400329CC  //邮箱数据库软恢复失败
#define ERROR_EXCHANE_MOUNT_INMULTIAD_FAIL                  0x400329CD  //多AD域下挂载失败

//**********Cluster错误码****************************范围0x400329D6 - 0x400329EF//
#define ERROR_CLUSTER_QUERY_FAILED                          0x400329D6  //查询集群信息失败
#define ERROR_CLUSTER_QUERY_NODE_FAILED                     0x400329D7  //查询集群节点信息失败
#define ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED            0x400329D8  //查询集群服务状态失败
#define ERROR_CLUSTER_START_SERVICE_FAILED                  0x400329D9  //启动集群服务失败
#define ERROR_CLUSTER_PACKAGE_ONLINE_FAILED                 0x400329DA  //上线程序包(资源组)失败
#define ERROR_CLUSTER_PACKAGE_OFFLINE_FAILED                0x400329DD  //下线程序包(资源组)失败
#define ERROR_CLUSTER_QUERY_ACTIVE_HOST_FAILED              0x400329DE  //查询活动节点失败
#define ERROR_CLUSTER_QUERY_GROUP_INFO_FAILED               0x400329DF  //查询程序包(资源组)信息失败
#define ERROR_CLUSTER_SQLSERVER_RESOURCE_NOT_EXIST          0x400329E0  //SQL Server资源组不存在
#define ERROR_CLUSTER_GET_CLUSTER_NETWORK_NAME_FAILED       0x400329E1  //查询网络资源信息失败
#define ERROR_CLUSTER_GET_DISK_PATITION_TYPE_FAILED         0x400329E2  //查询磁盘的分区类型失败
#define ERROR_CLUSTER_GET_DISK_RESOURCE_FAILED              0x400329E3  //查询磁盘资源信息失败
#define ERROR_CLUSTER_RESUME_DISK_RESOURCE_FAILED           0x400329E4  //恢复磁盘资源失败
#define ERROR_CLUSTER_REPAIR_DISK_RESOURCE_FAILED           0x400329E5  //修复磁盘资源失败
#define ERROR_CLUSTER_ONLINE_DISK_RESOURCE_FAILED           0x400329E6  //上线磁盘资源失败
#define ERROR_CLUSTER_SUSPEND_DISK_RESOURCE_FAILED          0x400329E7  //挂起磁盘资源失败
#define ERROR_CLUSTER_SERVICE_NOSTART                       0x400329E8  //集群服务未启动
#define ERROR_CLUSTER_DB_NOT_INCLUSTER                      0x400329E9  //数据库未加入集群
#define ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL              0x400329EA  //资源状态异常
#define ERROR_CLUSTER_NOT_ACTIVE_NODE                       0x400329EB  //集群节点为非活动节点

//**********VSS错误码****************************范围0x400329F0 - 0x400329FF//
#define ERROR_VSS_INIT_FILEDES_GETVOLUME_FAILED             0x400329F0  //初始化文件信息时获取卷信息失败
#define ERROR_VSS_TIME_OUT                                  0x400329F1  //VSS操作超时
#define ERROR_VSS_FREEZE_TIMEOUT                            0x400329F2  //冻结超时
#define ERROR_VSS_OTHER_FREEZE_RUNNING                      0x400329F3  //已经有冻结操作在执行
#define ERROR_VSS_EXCHANGE_DB_NOT_EXIST                     0x400329F4  //指定的存储组或邮箱数据库不存在

// Sun Cluster
#define ERROR_DEVICE_VXVM_EXPORT_DG_FAILED                  0x400329F5  //导出磁盘组失败
#define ERROR_DEVICE_VXVM_IMPORT_DG_FAILED                  0x400329F6  //导入磁盘组失败
#define ERROR_DEVICE_VXVM_ACTIVE_DG_FAILED                  0x400329F7  //激活磁盘组失败
#define ERROR_DEVICE_VXVM_DEACTIVE_DG_FAILED                0x400329F8  //去激活磁盘组失败
#define ERROR_DEVICE_VXVM_QUERY_DG_STATUS_FAILED            0x400329F9  //查询磁盘信息失败 

//End***********************返回给server错误码**********************************//


//BEGIN***********************脚本错误码，用于脚本错误返回，范围0-255************************//
//************公共错误码***************************，范围5-19//
#define ERROR_SCRIPT_COMMON_EXEC_FAILED                         5
#define ERROR_SCRIPT_COMMON_RESULT_FILE_NOT_EXIST               6
#define ERROR_SCRIPT_COMMON_TMP_FILE_IS_NOT_EXIST               7
#define ERROR_SCRIPT_COMMON_PATH_WRONG                          8
#define ERROR_SCRIPT_COMMON_PARAM_WRONG                         9
#define ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG                    10
#define ERROR_SCRIPT_COMMON_INSTANCE_NOSTART                    11
#define ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG                  15
#define ERROR_SCRIPT_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE     16
#define ERROR_SCRIPT_COMMON_DEVICE_FILESYS_MOUNT_FAILED			17
#define ERROR_SCRIPT_COMMON_DEVICE_FILESYS_UNMOUNT_FAILED		18

//***********Oracle脚本错误码********************，范围20-69//
#define ERROR_SCRIPT_ORACLE_ASM_DBUSERPWD_WRONG                 21
#define ERROR_SCRIPT_ORACLE_ASM_INSUFFICIENT_WRONG              22
#define ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART                23
#define ERROR_SCRIPT_ORACLE_NOARCHIVE_MODE                      24
#define ERROR_SCRIPT_ORACLE_OVER_ARCHIVE_USING                  25
#define ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_ALREADYMOUNT          26
#define ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_NOTMOUNT              27
#define ERROR_SCRIPT_ORACLE_APPLICATION_OVER_MAX_LINK           28
#define ERROR_SCRIPT_ORACLE_DB_ALREADY_INBACKUP                 29
#define ERROR_SCRIPT_ORACLE_DB_INHOT_BACKUP                     30
#define ERROR_SCRIPT_ORACLE_DB_ALREADYRUNNING                   31
#define ERROR_SCRIPT_ORACLE_DB_ALREADYMOUNT                     32
#define ERROR_SCRIPT_ORACLE_DB_ALREADYOPEN                      33
#define ERROR_SCRIPT_ORACLE_DB_ARCHIVEERROR                     34
#define ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_FAILED             35
#define ERROR_SCRIPT_ORACLE_END_HOT_BACKUP_FAILED               36
#define ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT            37

#define ERROR_SCRIPT_ORACLE_TRUNCATE_ARCHIVELOG_FAILED          42
#define ERROR_SCRIPT_ORACLE_TNS_PROTOCOL_ADAPTER                43
#define ERROR_SCRIPT_ORACLE_NOT_INSTALLED                       44
#define ERROR_SCRIPT_ORACLE_INST_NOT_CDB						47
#define ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT						48
#define ERROR_SCRIPT_ORACLE_START_PDB_FAILED					49
#define ERROR_SCRIPT_DB_FILE_NOT_EXIST					        50

//**********DB2脚本错误码*********************，范围 70-99 //
#define ERROR_SCRIPT_DB2_SUSPEND_IO_FAILED                      70
#define ERROR_SCRIPT_DB2_RESUME_IO_FAILED                       71
#define ERROR_SCRIPT_DB2_SUSPEND_IO_TIMEOUT                     72
//**********Exchange脚本错误码*********************，范围 100-129 //
#define ERROR_SCRIPT_EXCHANGE_REMOVE_FAILED                     100
#define ERROR_SCRIPT_EXCHANGE_SOFTRECVERY_FAILED                101
#define ERROR_SCRIPT_EXCHANGE_MOUNT_FAILED                      102
#define ERROR_SCRIPT_EXCHANGE_MOUNT_INMULTIAD_FAIL              103
//**********SqlServer脚本错误码*********************，范围 130-159 //
#define ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR                    130
#define ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED          131
#define ERROR_SCRIPT_SQLSERVER_132                              132
#define ERROR_SCRIPT_SQLSERVER_133                              133
#define ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED           134
#define ERROR_SCRIPT_SQLSERVER_DB_STATUS_OFFLINE                135
#define ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST               136
#define ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST                     137
#define ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED             138
#define ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED            139
//**********Cluster脚本错误码*********************，范围 160-189 //
#define ERROR_SCRIPT_CLUSTER_SERVICE_NOSTART                    160
#define ERROR_SCRIPT_CLUSTER_DB_NOT_INCLUSTER                   161
#define ERROR_SCRIPT_CLUSTER_RESOURCE_STATUS_ABNORMAL           162
#define ERROR_SCRIPT_CLUSTER_RESOURCE_ONLINE_FAILED             163
#define ERROR_SCRIPT_CLUSTER_RESOURCE_OFFLINE_FAILED            164
#define ERROR_SCRIPT_CLUSTER_NOT_ACTIVE_NODE                    165

#define INTER_ERROR_SRCIPT_FILE_NOT_EXIST                       255
//END***********************脚本错误码，用于脚本错误返回，范围0-255************************//


//脚本错误碼到真实错误码转换处理
class CErrorCodeMap
{
private:
    std::map<mp_int32, mp_int32> m_mapErrorCode;

public:
    CErrorCodeMap()
    {    //初始化错误码
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_EXEC_FAILED, ERROR_COMMON_SCRIPT_EXEC_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED, ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_RESULT_FILE_NOT_EXIST, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_TMP_FILE_IS_NOT_EXIST, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_PATH_WRONG, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_PARAM_WRONG, ERROR_COMMON_INVALID_PARAM));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG, ERROR_COMMON_DB_USERPWD_WRONG));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_INSTANCE_NOSTART, ERROR_COMMON_RECOVER_INSTANCE_NOSTART));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG, ERROR_COMMON_DB_INSUFFICIENT_PERMISSION));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE, ERROR_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB_FILE_NOT_EXIST, ERROR_COMMON_DB_FILE_NOT_EXIST));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_DEVICE_FILESYS_MOUNT_FAILED, ERROR_DEVICE_FILESYS_MOUNT_FAILED));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_DEVICE_FILESYS_UNMOUNT_FAILED, ERROR_DEVICE_FILESYS_UNMOUNT_FAILED));
        //oracle错误码
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_DBUSERPWD_WRONG, ERROR_ORACLE_ASM_DBUSERPWD_WRONG));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_INSUFFICIENT_WRONG, ERROR_ORACLE_ASM_INSUFFICIENT_WRONG));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART, ERROR_ORACLE_ASM_RECOVER_INSTANCE_NOSTART));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_NOARCHIVE_MODE, ERROR_ORACLE_NOARCHIVE_MODE));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_OVER_ARCHIVE_USING, ERROR_ORACLE_OVER_ARCHIVE_USING));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_ALREADYMOUNT, ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_NOTMOUNT, ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_APPLICATION_OVER_MAX_LINK, ERROR_ORACLE_OVER_MAX_LINK));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ALREADY_INBACKUP, ERROR_ORACLE_IN_BACKUP));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_INHOT_BACKUP, ERROR_ORACLE_NOT_IN_BACKUP));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ALREADYRUNNING, ERROR_ORACLE_DB_ALREADYRUNNING));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ALREADYMOUNT, ERROR_ORACLE_DB_ALREADYMOUNT));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ALREADYOPEN, ERROR_ORACLE_DB_ALREADYOPEN));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ARCHIVEERROR, ERROR_ORACLE_ARCHIVE_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_FAILED, ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_END_HOT_BACKUP_FAILED, ERROR_ORACLE_END_HOT_BACKUP_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT, ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_TRUNCATE_ARCHIVELOG_FAILED, ERROR_ORACLE_TRUNCATE_LOG_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_TNS_PROTOCOL_ADAPTER, ERROR_ORACLE_TNS_PROTOCOL_ADAPTER));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_NOT_INSTALLED, ERROR_SCRIPT_ORACLE_NOT_INSTALLED));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_INST_NOT_CDB, ERROR_ORACLE_INSTANCE_NOT_CDB));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT, ERROR_SQLSERVER_DB_NOT_EXIST));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_START_PDB_FAILED, ERROR_SQLSERVER_START_DB_FAILED));
		
        //DB2错误码
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB2_SUSPEND_IO_FAILED, ERROR_DB2_SUSPEND_IO_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB2_RESUME_IO_FAILED, ERROR_DB2_RESUME_IO_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB2_SUSPEND_IO_TIMEOUT, ERROR_DB2_SUSPEND_IO_TIMEOUT));
       
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_EXCHANGE_REMOVE_FAILED, ERROR_EXCHANGE_REMOVE_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_EXCHANGE_SOFTRECVERY_FAILED, ERROR_EXCHANGE_SOFTRECVERY_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_EXCHANGE_MOUNT_FAILED, ERROR_EXCHANGE_MOUNT_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_EXCHANGE_MOUNT_INMULTIAD_FAIL, ERROR_EXCHANE_MOUNT_INMULTIAD_FAIL));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(INTER_ERROR_SRCIPT_FILE_NOT_EXIST, ERROR_COMMON_SCRIPT_FILE_NOT_EXIST));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED, ERROR_CLUSTER_QUERY_FAILED));
        
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_RESOURCE_ONLINE_FAILED, ERROR_CLUSTER_PACKAGE_ONLINE_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_RESOURCE_OFFLINE_FAILED, ERROR_CLUSTER_PACKAGE_OFFLINE_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_SERVICE_NOSTART, ERROR_CLUSTER_SERVICE_NOSTART));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_DB_NOT_INCLUSTER, ERROR_CLUSTER_DB_NOT_INCLUSTER));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_RESOURCE_STATUS_ABNORMAL, ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL));
          
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED, ERROR_SQLSERVER_GET_DB_STATUS_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_DB_STATUS_OFFLINE, ERROR_SQLSERVER_DB_STATUS_OFFLINE));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST, ERROR_SQLSERVER_INSTANCE_NOT_EXIST));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST, ERROR_SQLSERVER_DB_NOT_EXIST));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED, ERROR_SQLSERVER_START_INSTANCE_FAILED));
        
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_NOT_ACTIVE_NODE, ERROR_CLUSTER_NOT_ACTIVE_NODE));
    }
    mp_int32 GetErrorCode(mp_int32 iRet)
    {
        map<mp_int32, mp_int32>::iterator it = m_mapErrorCode.find(iRet);
        return it != m_mapErrorCode.end() ? it->second : MP_FAILED;
    }
};

/*------------------------------------------------------------
Function Name: TRANSFORM_RETURN_CODE
Description  : 转换错误码，对于返回的-1进行错误码替换
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#define TRANSFORM_RETURN_CODE(iRet, RETURN_CODE) \
    iRet = (iRet == MP_FAILED ? RETURN_CODE : iRet)


/*------------------------------------------------------------
Function Name: MP_RETURN
Description  : 函数返回，对于Call调用返回-1的情况进行特殊替换
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#define MP_RETURN(Call, RETURN_CODE) \
{\
    mp_int32 iFuncRet = Call;\
    return iFuncRet == MP_FAILED ? RETURN_CODE : iFuncRet;\
}

/*------------------------------------------------------------
Function Name: CHECK_MP_RETURN
Description  : 函数在失败情况下返回，对于Call调用返回-1的情况进行特殊替换
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#define CHECK_MP_RETURN(Call, RETURN_CODE) \
{\
    mp_int32 iFuncRet = Call;\
    if (iFuncRet != MP_SUCCESS)\
        return iFuncRet == MP_FAILED ? RETURN_CODE : iFuncRet;\
}

#endif //_AGENT_ERROR_CODE_H_

