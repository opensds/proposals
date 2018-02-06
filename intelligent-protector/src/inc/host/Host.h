/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_HOST_H__
#define __AGENT_HOST_H__

#include <vector>
#include "common/Types.h"
#include "common/Thread.h"
#include "common/Defines.h"
#include "alarm/Alarm.h"
#include "array/Array.h"
#ifdef WIN32
#include "cfgmgr32.h"
#include "setupapi.h"
#endif
 
#define WAIT_AFTER_HOT_ADD            30 * 1000
#define MAX_SYS_VERSION_LENGTH        10              //系统版本最大长度

#define HOST_ISDST                    1
#define HOST_ISNOTDST                 0
#define HOST_TIMEZONE_CONVERSION_UNIT -60            //时区换算单位
#define HOST_TIMEZONE_LENGTH          6
#define MAX_ZIP_LOG_NUM               10

#define SCRIPT_INITIATOR_LINUX        "initiator.sh"
#define SCRIPT_INITIATOR_WIN          "initiator.bat"
#define SCRIPT_ONLINE_WIN             "online.bat"
#define SCRIPT_INITIATOR_PARAM_FC     "fcs"
#define SCRIPT_INITIATOR_PARAM_ISCSI  "iscsis"

typedef struct tag_host_info
{
    mp_string name;
    mp_string sn;
    mp_string version;
    mp_int32 os;
}host_info_t;

typedef struct tag_initiator_info
{
    vector<mp_string> iscsis;
    vector<mp_string> fcs;
}initiator_info_t;

typedef struct tag_host_lun_info
{
    mp_string arrayVendor;
    mp_string lunId;
    mp_string wwn;
    mp_string arraySn;
    mp_string arrayVersion;
    mp_string arrayModel;
    mp_string deviceName;
    mp_string diskNumber;
}host_lun_info_t;

typedef struct tag_partitisions_info
{
    mp_int32 iType;
    mp_uint64 lCapacity;
    mp_string strPartitionName;
    mp_string strDiskNumber;
    mp_string strVolName;
    mp_string strLba;
}partitisions_info_t;

typedef struct tag_timezone_info
{
    mp_int32  iIsDST;
    mp_string strTzBias;
}timezone_info_t;


//## sturct for class CHBA
#ifdef WIN32
typedef struct HBA_wwn {
  unsigned char wwn[8];
} HBA_WWN;

typedef struct HBA_fc4types {
  char bits[32];
} HBA_FC4TYPES;

typedef struct HBA_AdapterAttributes {
  char       Manufacturer[64];
  char       SerialNumber[64];
  char       Model[256];
  char       ModelDescription[256];
  HBA_WWN       NodeWWN;
  char       NodeSymbolicName[256];
  char       HardwareVersion[256];
  char       DriverVersion[256];
  char       OptionROMVersion[256];
  char       FirmwareVersion[256];
  unsigned int VendorSpecificID;
  unsigned int NumberOfPorts;
  char       DriverName[256];
} HBA_ADAPTERATTRIBUTES;

typedef struct HBA_PortAttributes {
  HBA_WWN       NodeWWN;
  HBA_WWN       PortWWN;
  unsigned int    PortFcId;
  unsigned int  PortType;
  unsigned int PortState;
  unsigned int       PortSupportedClassofService;
  HBA_FC4TYPES  PortSupportedFc4Types;
  HBA_FC4TYPES  PortActiveFc4Types;
  char          PortSymbolicName[256];
  char          OSDeviceName[256];
  unsigned int PortSupportedSpeed;
  unsigned int PortSpeed;
  unsigned int    PortMaxFrameSize;
  HBA_WWN       FabricName;
  unsigned int    NumberofDiscoveredPorts;
} HBA_PORTATTRIBUTES;

typedef struct _MSFC_HBAPortAttributesResults {
  UCHAR NodeWWN[8];
  UCHAR PortWWN[8];
  ULONG PortFcId;
  ULONG PortType;
  ULONG PortState;
  ULONG PortSupportedClassofService;
  UCHAR PortSupportedFc4Types[32];
  UCHAR PortActiveFc4Types[32];
  ULONG PortSupportedSpeed;
  ULONG PortSpeed;
  ULONG PortMaxFrameSize;
  UCHAR FabricName[8];
  ULONG NumberofDiscoveredPorts;
} MSFC_HBAPortAttributesResults;

typedef int(*Hba_GetNumberOfAdapters)();
typedef int(*Hba_GetAdapterName)(int, char*);
typedef HANDLE(*Hba_OpenAdapter)(char*);
typedef void(*Hba_CloseAdapter)(HANDLE);
typedef void(*GetDiscoveredPortAttributes)( unsigned int PortIndex,  unsigned int DiscoveredPortIndex, ULONG HBAStatus, MSFC_HBAPortAttributesResults PortAttributes);
typedef int(*Hba_GetAdapterAttributes)(HANDLE handle,HBA_ADAPTERATTRIBUTES *hbaattributes);
typedef int(*Hba_GetAdapterPortAttributes)(HANDLE handle, unsigned int portindex, HBA_PORTATTRIBUTES *portattributes);
#endif
//##

enum 
{
    LOG_INIT = 0,
    LOG_COLLECTING   
};

class CLogCollector
{
public:
#ifdef WIN32
    static DWORD WINAPI LogCollectThread(LPVOID param);
#else
    static mp_void* LogCollectThread(mp_void* param);
#endif
    CLogCollector()
    {
        m_status = LOG_INIT;
        m_strLogName = "";
    }
    mp_int32 CollectLog();
    mp_uint32 GetStatus()
    {
        return m_status;
    }
    mp_void SetStatus(mp_uint32 uiStatus)
    {
        m_status = uiStatus;
    }
    mp_string GetLogName()
    {
        return m_strLogName;
    }
    mp_void SetLogName(mp_string strLogName)
    {
        m_strLogName = strLogName;
    }

private:
    mp_uint32 m_status;
    mp_string m_strLogName;
};

//## class to get wwn
#ifdef WIN32
class CHBA
{
public :
    CHBA();
    ~CHBA();
    mp_int32 GetHBAInfo(vector<mp_string>&);
private :
    HMODULE m_hbadll;
    Hba_GetNumberOfAdapters Hbafun_GetNumber;
    Hba_GetAdapterName Hbafun_GetAdapterName;
    Hba_OpenAdapter Hbafun_OpenAdapter;
    Hba_CloseAdapter Hbafun_CloseAdapter;
    Hba_GetAdapterAttributes Hbafun_GetAdapterAttributes;
    Hba_GetAdapterPortAttributes Hbafun_GetAdapterPortAttributes;

private :
    mp_bool LoadHBAdllModule();
    mp_void FreeHBAdllModule();
};
#endif
//## 

class CHost
{
private:
    mp_string m_hostsnFile;
    thread_lock_t m_pMutex;
    CLogCollector m_logCollector;	
    #ifdef WIN32
    CHBA m_CHBA;
    #endif
public:
     CHost();
     ~CHost();

    mp_int32 GetInfo(host_info_t& hostInfo);
    mp_int32 GetDiskInfo(vector<host_lun_info_t> & vecLunInfo);
    mp_int32 GetTimeZone(timezone_info_t& sttimezone);
    mp_int32 GetAgentVersion(mp_string& strAgentVersion, mp_string& strBuildNum);
    mp_int32 GetInitiators(initiator_info_t& initInfo);
    mp_int32 GetPartisions();
    mp_int32 ScanDisk();
    mp_int32 RegTrapServer(trap_server& stTrapServer);
    mp_int32 UnRegTrapServer(trap_server& stTrapServer);
    mp_int32 VerifySnmp(snmp_v3_param& stParam);
    mp_int32 QueryThirdPartyScripts(vector<mp_string> & vecFileInfo);
    mp_int32 ExecThirdPartyScript(mp_string fileName, mp_string paraValues, vector<mp_string>& vecResult);
    mp_int32 CollectLog();
    mp_string GetLogName()
    {
        return m_logCollector.GetLogName();
    }
    mp_bool CanDownloadLog()
    {
        return m_logCollector.GetStatus() == LOG_INIT ? MP_TRUE : MP_FALSE;
    }
#ifdef WIN32
    mp_int32 DeviceOnline(mp_string& strDiskNum);
    mp_int32 GetPartisions(vector<partitisions_info_t>& partisioninfos);
#endif

private:
    mp_int32 GetHostSN(mp_string& strSN);
    mp_int32 GetHostOS(mp_int32& iOSType, mp_string& strOSVersion);
    mp_int32 ReadHostSNInfo(vector<mp_string>& vecMacs);
    mp_int32 SetHostSN(vector<mp_string> &vecMacs);
};

#endif //__AGENT_HOST_H__

