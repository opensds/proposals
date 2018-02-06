/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_ARRAY_H__
#define __AGENT_ARRAY_H__

#include "common/Types.h"
#include <vector>
#include <map>

#ifdef LINUX
#include <sys/vfs.h>
#include <sys/sysinfo.h>
#include <sys/syslog.h>
#include <iconv.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/scsi.h>
#include <ctype.h>
#include <linux/raw.h>
#include <linux/major.h>
#include <fcntl.h>
#endif
#ifdef AIX
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/scsi.h>
#include <sys/scdisk.h>
#endif
#ifdef HP_UX_IA
#include <iconv.h>
#include <sys/pstat.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stropts.h>
#include <sys/scsi.h>
#endif
#ifdef SOLARIS
#include <sys/ioctl.h>
#include <stropts.h>
#include <iconv.h>
#include <syslog.h>
#include <sys/scsi/scsi.h>
#include <sys/scsi/impl/uscsi.h>
#endif
#ifdef WIN32
#include <ntddscsi.h>
#include <setupapi.h>
#endif

#ifdef WIN32
typedef HANDLE       FILE_HANDLE;
#else
typedef int          FILE_HANDLE;
#endif

#define COUNT_LUNWWN                    0
#define COUNT_LUNID                     1
#define C8_COUNT_LUNID                  0

#define COUNT_VENDOR                    0
#define COUNT_PRODUCT                   1
#define COUNT_VERSION                   2

#define PAGE_83                         0x83
#define PAGE_80                         0x80
#define PAGE_00                         0x00
#define PAGE_C8                         0xC8
#define PAGE_CDB_0                      0x00
#define PAGE_CDB_1                      0x01

//scsi
#define SCSI_MAX_SENSE_LEN              64        //sense的最大长度
#define SCSI_DISK_DATA_LEN              512       //磁盘数据长度
#define DISK_MAX_PAGE_NUM               255       //磁盘支持的最大VPD Page数
#define DISK_NORECORD_ERROR             4         //磁盘 无记录错误码
#define SC_INQ_PAGE_HEAD_LEN            4
#define VDS_HS_PAGE_BUF_LEN(buf) (((buf)[SC_INQ_PAGE_HEAD_LEN - 2] << 8) + (buf)[SC_INQ_PAGE_HEAD_LEN - 1] + SC_INQ_PAGE_HEAD_LEN)
#define C8_PAGE_WITH_LUN_ID_LEN         196


//CDB长度
#define CDB6GENERIC_LENGTH              6         //6 字节
#define CDB10GENERIC_LENGTH             10        //10 字节
#define CDB16GENERIC_LENGTH             16        //16 字节

#define SCSIOP_READ_CAPACITY            0x25      //READ_CAPACITY命令操作码
#define SCSIOP_INQUIRY                  0x12      //CBD查询码

#define NAME_PATH_LEN                   512       //磁盘名称长度
#define DISK_PATH_MAX                   260       //磁盘路径最大值
#define MAX_VENDOR_LEN                  64        //厂商长度
#define MAX_PRODUCT_LEN                 256       //型号长度
#define MAX_VVERSION_LEN                4         //阵列V版本信息
#define MAX_SN_LEN                      30        //序列号长度
#define MAX_WWN_LEN                     64        //WWN
#define MAX_LUNID_LEN                   11        //LUN ID
#define MAX_SN_HW                       21        //序列号长度
#define MAX_PAGETCODE_LEN               3

#define ARRAY_VENDER_HUAWEI             "HUAWEI"  //华 为 阵 列 VENDERID
#define VENDOR_ULTRAPATH_HUAWEI         "up"      //华为多路径
#define ARRAY_VENDER_HUASY              "HUASY"   //华赛 阵 列 VENDERID

#define HOST_PATH_NAME                  100
#define FILESYS_NAME_LEN                256      //文件系统名称长度
#define FILESYS_TYPE_LEN                20       //文件系统类型长度
#define FILESYS_FATHER_LEN              64       //文件系统所属磁盘名长度
#define FILESYS_MOUNT_LEN               256      //文件系统挂载点
#define CAP_DATA_CONVERSION             1024     //容量单位转换
#define MAX_DISK_EXTENS                 10

#define VALUE_LEN                       64

//linux
#define DISK_BYTE_OF_SECTOR             512           //io命令返回的缓存大小
#define DATA_LEN_256                    256
#define SCSI_CMD_TIMEOUT_LINUX          (60 * 1000)
#define EXE_CMD_SCR_LEN                 256           //LINUX/AIX命令长度
#define LINE_DATA_LEN                   256           //临时文件一行的长度
#define FILE_ROW_COUNT                  255
#define MAX_NAME_LEN                    256
#define LINUX_BLOCK                     1024          //LINUX文件块大小

//AIX
#define BUFFER_LEN_36                   36
#define SCSI_CMD_TIMEOUT_AIX            60
#define MAX_STATUS_LEN                  20            //磁盘状态长度
#define DISK_STATE_ACTIVE               "0"           // 激活的
#define DISK_STATE_CLOSE                "1"           // 未激活的
#define AIX_DISK_ACTIVE                 "Available"
#define BUFFER_LEN_255                  255
#define AIX_BLOCK_SIZE                  1024

//HP
//宏定义
#define HP_BLOCK_SIZE                   1024                             //HP文件块大小
#ifdef HP_UX_IA
#define SIOC_IO                         _IOWR('S', 22, struct sctl_io)   //IO控制码
#define B_READ                          0x00000001                       // 读I/O标志
#endif
#define SCSI_CMD_TIMEOUT_HP             (60 * 1000)                      //IO命令超时时间
#define DISK_EXECMD_SCR_LEN             256
#define DISK_CMD_DATA_LEN               256

//solaris
#define SCSI_CMD_TIMEOUT_SOLARIS        60                               //IO命令超时时间
#define BLOCK_SIZE_SOLARIS              1024

#define CMD_DEV_BLOCKS                  1

#define MAX_DISK_NUM                    128      //主机侧的硬盘名称
#define MAX_LUN_ID                      4        //LUN ID最大长度
#define MAX_ARRAY_SN_LEN                20       //阵列SN最大长度
#define MAX_LUN_WWN_LEN                 16       //LUN WWN最大长度
#define FILE_SYSTEM_TYPE                20

#define DEF_RLUNS_BUFF_LEN 				(1024 * 32)
#define MAX_RLUNS_BUFF_LEN 				(1024 * 64)
#define DEF_COM_TIMEOUT 				30000

#ifdef WIN32
// 宽字节字符串结构定义
typedef struct _UNICODE_STRING {  
    USHORT  Length;  
    USHORT  MaximumLength;  
    PWSTR  Buffer;  
} UNICODE_STRING, *PUNICODE_STRING;  

// 对象属性定义
typedef struct _OBJECT_ATTRIBUTES {  
    ULONG Length;  
    HANDLE RootDirectory;  
    UNICODE_STRING *ObjectName;  
    ULONG Attributes;  
    PSECURITY_DESCRIPTOR SecurityDescriptor;  
    PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;  
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;  

// 返回值或状态类型定义
#define OBJ_CASE_INSENSITIVE      0x00000040L  
#define DIRECTORY_QUERY           (0x0001)  
#define STATUS_SUCCESS            ((ULONG)0x00000000L) // ntsubauth  

#define InitObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
}  

#define NT_SUCCESS(Status) ((ULONG)(Status) >= 0)  
#define STATUS_INSUFFICIENT_RESOURCES    ((ULONG)0xC000009AL)     // ntsubauth

// 字符串初始化
typedef VOID (WINAPI *RTLINITUNICODESTRING)(PUNICODE_STRING, PCWSTR);  
// 打开对象
typedef ULONG (WINAPI *ZWOPENDIRECTORYOBJECT)(OUT PHANDLE DirectoryHandle, 
                                                 IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);  
// 打开符号链接对象 
typedef ULONG (WINAPI *ZWOPENSYMBOLICKLINKOBJECT)(OUT PHANDLE SymbolicLinkHandle, 
                                                     IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes); 
// 查询符号链接对象 
typedef ULONG (WINAPI *ZWQUERYSYMBOLICKLINKOBJECT)(IN HANDLE SymbolicLinkHandle, 
                                                      IN OUT PUNICODE_STRING TargetName, OUT PULONG ReturnLength OPTIONAL);  
// 关闭已经打开的对象
typedef ULONG (WINAPI *ZWCLOSE)(IN HANDLE Handle);  

typedef struct tag_scsi_pass_through_with_buff
{
    SCSI_PASS_THROUGH pt;
    mp_uint64 ulPadding;
    mp_uchar aucSense[SCSI_MAX_SENSE_LEN];
    mp_uchar aucData[SCSI_DISK_DATA_LEN];
}scsi_pass_through_with_buff_t;

//硬盘设备信息-win
typedef struct tag_win_dev_info
{
    mp_int32 iDiskNum;                            //磁盘序号
    mp_int32 iLunId;                              //LUNID
    mp_int32 iScsiId;                             //SCSIID
    mp_int32 iPortId;                             //端口ID
    mp_int32 iPathId;                             //路径ID
    mp_uchar aucPageCode[DISK_MAX_PAGE_NUM];      //supported VPD page code

}win_dev_info_t;

typedef struct tag_disk_info
{        
    mp_string strLUNID;                 // LUN ID
    mp_string strArraySN;               // LUN所在阵列的序列号
    mp_string strLUNWWN;                // LUN的WWN
    mp_int32 iDiskNum;                  // LUN在主机上的硬盘名称
} disk_info;

typedef struct tag_sub_area_Info
{
    mp_int32 iDiskNum;                        //磁盘序号
    mp_uint64 llOffset;                       //分区偏移量
    mp_uint64 ullTotalCapacity;               //分区长度
    mp_char acDriveLetter[HOST_PATH_NAME];    //驱动器号（盘符）
    mp_char acVolName[DISK_PATH_MAX];         //卷名
    mp_char acVolLabel[DISK_PATH_MAX];        //卷标
    mp_char acFileSystem[FILESYS_TYPE_LEN];   //文件系统类型
    mp_char acDeviceName[DISK_PATH_MAX];
}sub_area_Info_t;

#else

typedef struct tag_luninfo
{
	mp_string strDeviceName;			// Device name
	mp_string strVendor;				// vendor
	mp_string strProduct;				// product
    mp_string strArraySN;               // LUN所在阵列的序列号
    mp_string strLUNID;                 // LUN ID
    mp_string strLUNWWN;                // LUN的WWN
} luninfo_t;

#endif

class CArray
{
public:
    CArray();
    ~CArray();

    static mp_int32 GetLunInfo(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);
    static mp_int32 GetArraySN(mp_string& strDev, mp_string& strSN);
    static mp_int32 GetArrayVendorAndProduct(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);
    static mp_int32 GetDisk80Page(mp_string& strDevice, mp_string& strSN);
    static mp_int32 GetDisk83Page(mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID);
    static mp_int32 GetDisk00Page(mp_string& strDevice, vector<mp_string>& vecResult);
    static mp_int32 GetDiskC8Page(mp_string& strDevice, mp_string& strLunID);
    static mp_int32 GetDiskArrayInfo(mp_string& strDevice, mp_string& strVendor, mp_string& strProduct);
	static mp_int32 GetHostLunIDList(mp_string& strDevice, vector<mp_int32>& vecHostLunID);
	static mp_bool CheckHuaweiLUN(mp_string strVendor);

private:
#ifdef WIN32
    static mp_int32 OpenDev(mp_string& strDev, HANDLE& handle);
    static mp_int32 GetDevHandle(const mp_string& pszDeviceName, FILE_HANDLE& pHandle);
    static mp_int32 GetDevSCSIAddress(FILE_HANDLE fHandle, win_dev_info_t& pstDevInfo);
    static mp_void SetScsiBufValues(scsi_pass_through_with_buff_t& stSCSIPass, mp_ulong& ulLength, mp_uchar ucCdb, 
        mp_uchar ucCmd);
#else
    static mp_int32 OpenDev(mp_string& strDev, mp_int32& iDevFd);
    static mp_int32 GetDiskPage(mp_int32 iFd, mp_uchar ucCmd, mp_uchar *aucBuffer);
    static mp_int32 GetVendorAndProduct(mp_int32 iFd, mp_char *aucBuffer);
#endif

    static mp_int32 BinaryToAscii(mp_char* pszHexBuffer, mp_int32 iBufferLen, mp_uchar* pucBuffer,
        mp_int32 iStartBuf, mp_int32 iLength);
    static mp_int32 ConvertLUNIDtoAscii(mp_uchar* puszAsciiLunID, mp_int32 iBufferLen, mp_uchar* puszLunLunID,
        mp_int32 iLen);
    static mp_int32 HextoDec(mp_char* pDecStr, mp_char* pHexStr, mp_int32 iMaxLen);
    static mp_int32 HexEncode(mp_char cHex, mp_int32 iStep, mp_int32& iDecNum);
    static mp_int32 CalStep(mp_int32 iStep);
    static mp_bool IsSupportXXPage(string page, vector<mp_string>& vecResult);
};

#pragma pack(1)
typedef struct tagISSP_SCSI_SENSE_HDR_S   //See SPC-3 section 4.5
{
    mp_uchar ucResponseCode;  //permit: 0x0, 0x70, 0x71, 0x72, 0x73
    mp_uchar ucSenseKey;
    mp_uchar ucAsc;
    mp_uchar ucAscq;
    mp_uchar ucByte4;
    mp_uchar ucByte5;
    mp_uchar ucByte6;
    mp_uchar ucAdditionalLength;  //always 0 for fixed sense format
} ISSP_SCSI_SENSE_HDR_S;
#pragma pack()


#define CHECK_CLOSE_FD( Call )                                                                    \
{                                                                                                 \
    mp_int32 iCheckNotOkRet = Call;                                                               \
    if (EOK != iCheckNotOkRet)                                                                    \
    {                                                                                             \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);\
        close(iFd);                                                                               \
        return MP_FAILED;                                                                         \
    }                                                                                             \
}

class CDisk
{
public:
    CDisk();
    ~CDisk();

    static mp_int32 GetDevNameByWWN(mp_string& strDevName, mp_string& strWWN);
    static mp_int32 OpenDev(mp_string& strDevice, mp_int32& iDevFd);
    static mp_int32 GetAllDiskName(vector<mp_string>& vecDiskName); 
    static mp_int32 GetDiskCapacity(mp_string& strDevice, mp_string& strBuf);
    static mp_bool IsSdisk(mp_string& strDevice);
    static mp_bool IsHdisk(mp_string& strDevice);
    static mp_bool IsDeviceExist(mp_string& strDiskName);
    static mp_bool IsDskdisk(mp_string& strName);
    static mp_int32 GetHPRawDiskName(mp_string strDiskName, mp_string& strRawDiskName);
	static mp_int32 GetSolarisRawDiskName(mp_string strDiskName, mp_string& strRawDiskName);
	static mp_int32 GetPersistentDSFByLegacyDSF(mp_string& rstrLegacyDisk, mp_string &rstrPersistentDisk);
    static mp_int32 ClearInvalidDisk();
    static mp_int32 GetSecPvName(mp_string& strPriPvName, mp_string& strLegacyDisk, mp_string& strSecPvName); 
#ifdef WIN32
    static mp_int32 GetVolPaths(map<mp_string, mp_string> &mapPath);
    static mp_bool IsDriveExist(mp_string& strDriveLetter);
    static mp_int32 GetVolumeDiskInfo(mp_char* pcVolumeName, sub_area_Info_t& pstrPartInfo);
    static mp_int32 GetNextVolumeInformation(FILE_HANDLE FindHandle, mp_char* pcVolumeName, mp_int32 iSize);
    static mp_int32 GetDiskNum(mp_string& strDeviceName, mp_int32& DiskNum);
    static mp_int32 GetSubareaInfoList(vector<sub_area_Info_t>& rvecSubareaInfo);
    static mp_int32 GetDiskInfoList(vector<disk_info> &vecDiskInfoWin);
    mp_bool InitSymboLinkRes();
    mp_void FreeSymboLinkRes();
    mp_bool QuerySymboLinkInfo(const mp_string &strSymboLink, mp_string &strTargetDevice, 
        mp_bool isVolumn = MP_FALSE);
#endif

private:
#ifdef WIN32
    HMODULE m_hNtdll;
    RTLINITUNICODESTRING RtlInitUnicodeString;  
    ZWOPENDIRECTORYOBJECT ZwOpenDirectoryObject;
    ZWOPENSYMBOLICKLINKOBJECT ZwOpenSymbolicLinkObject;
    ZWQUERYSYMBOLICKLINKOBJECT ZwQuerySymbolicLinkObject;
    ZWCLOSE ZwClose;
#endif

private:
    static mp_int32 GetDiskStatus(mp_string& strDiskName, mp_string& strStatus);
    static mp_bool IsCmdDevice(mp_string& strDiskName);
    static mp_int32 ScsiNormalizeSense(mp_string& strBuf, ISSP_SCSI_SENSE_HDR_S& stSSHdr);
    static mp_int32 ClearInvalidLUNPath(vector<mp_string>& vecLUNPaths);
    static mp_int32 ClearInvalidLegacyDSFs(vector<mp_string>& vecDiskNumbers);
    static mp_int32 ClearInvalidPersistentDSFs(vector<mp_string>& vecDiskNumbers);
    static mp_int32 DeletePersistentDSF(mp_string& strDeviceName);
    static mp_int32 GetPersistentDSFInfo(mp_string& rstrDiskName, mp_string &strPDHWPath, mp_string &strPDHWType);
    static mp_int32 ExecuteDiskCmdEx(mp_string strCmd, vector<mp_string> &vecDiskName, mp_string strFileName);
#ifdef WIN32
    static mp_int32 GetDiskPath(HDEVINFO &hIntDevInfo, mp_int64 iIndex, mp_char *pszPath, mp_int32 pszPathLen);
    static mp_int32 GetAllDiskWin(vector<mp_string>& vecDiskName);
    static mp_void GetVolPathByVolName(map<mp_string, mp_string> &mapPath);
    static mp_int32 GetVolumeDiskInfo(mp_char *pcVolumeName, sub_area_Info_t *pstrPartInfo);
    mp_bool LoadNtdllModule(void);
    mp_void FreeNtdllModule(void);
    mp_ulong QuerySymbolicLink(IN PUNICODE_STRING SymbolicLinkName, OUT PUNICODE_STRING LinkTarget, 
        mp_bool isVolumn);
#endif
};

#endif //__AGENT_ARRAY_H__

