/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_RAW_H__
#define __AGENT_RAW_H__

#ifndef WIN32
#include "common/Types.h"
#include "common/Thread.h"

#ifdef LINUX
#include <linux/raw.h>
#include <linux/major.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
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


#define SECTION_NUM_TAG        "123456789"

#define CHECK_RAW_DEVICE_COUNT       1000
#define CHECK_RAW_DEVICE_INTERVAL    10  //ms

#define HP_DEVICE_STATUS_NOEXISTS   1
#define HP_DEVICE_STATUS_NOHW       2
#define HP_DEVICE_STATUS_NORMAL     3

typedef struct tag_raw_info
{
    mp_string strLunWWN;    //LUN WWN
    mp_string strDevName;  //块设备名称
    mp_string strRawDevPath;  //裸设备名称
}raw_info_t;

class CRaw
{
public:
    CRaw();
    ~CRaw();
    mp_int32 Create(raw_info_t& rawInfo);
    mp_int32 Delete(raw_info_t& rawInfo);

private:
    mp_int32 StartRawService();
    
    mp_int32 GetDeviceUsedByRaw(mp_string& strRawDevPath, mp_string& strUsedDevName);
    
    mp_int32 GetBoundedDevVersions(mp_string& strRawDevPath, mp_int32& iBoundMajor, mp_int32& iBoundMinor);

    mp_int32 CreateRAWDevice(raw_info_t& rawInfo);
#ifdef LINUX
    mp_int32 GetDeviceNumber(mp_string& strRawDevPath, mp_int32& iMajor, mp_int32& iMinor);
#elif AIX|HP_UX_IA
    mp_int32 GetDeviceNumber(mp_string& rstrDeviceName, mp_string &majorNum, mp_string &minorNum);
#endif
    mp_int32 IsDeviceExists(mp_string& rstrDeviceName, mp_bool& isDeviceExist);

    mp_int32 GetDeviceStatus(mp_string &strMajor, mp_string &strMinor, mp_char &deviceStatus);

    mp_int32 DeleteDevice(mp_string& strDeviceName);

    mp_int32 CheckDeviceByDeviceNumber(raw_info_t& rawInfo, mp_string strDeviceName, mp_string &majorNumMount, mp_string &minorNumMount, 
    mp_string majorNumDevice, mp_string minorNumDevice);

    mp_int32 UpdateDeviceByDsf(raw_info_t& rawInfo, mp_string &majorNumMount, mp_string &minorNumMount, 
    mp_string majorNumDevice, mp_string minorNumDevice);

    mp_int32 CreateDeviceByMknod(mp_string &strDeviceName, mp_string strMajorNum, mp_string strMinorNum);
};


#endif //WIN32
#endif //__AGENT_LVM_H__

