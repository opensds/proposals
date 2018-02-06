/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _SYSTEM_CALL_H_
#define _SYSTEM_CALL_H_
#include "common/Types.h"
#include <map>

//redhat 5.2¡¢redhat 5.5
#define RAW_CTRL_FILE          "/dev/rawctl"
//suse10¡¢suse11¡¢redhat6.3
#define RAW_CTRL_FILE_NEW      "/dev/raw/rawctl"

//file system 2.6.29up
#define OPER_TIME_OUT    30
#define THAW_ERR         22  

class CSystemCall
{
public:
    static mp_int32 ExecSysCmd(mp_string& strUniqueID, mp_int32 iCommandID);
    static mp_int32 ExecScript(mp_string& strUniqueID, mp_int32 iCommandID);
    static mp_int32 GetDisk80Page(mp_string& strUniqueID);
    static mp_int32 GetDisk83Page(mp_string& strUniqueID);
    static mp_int32 GetDisk00Page(mp_string& strUniqueID);
    static mp_int32 GetDiskC8Page(mp_string& strUniqueID);
    static mp_int32 GetDiskCapacity(mp_string& strUniqueID);
    static mp_int32 GetVendorAndProduct(mp_string& strUniqueID);
    static mp_int32 ExecThirdPartyScript(mp_string& strUniqueID);
    static mp_int32 GetRawMajorMinor(mp_string& strUniqueID);
	static mp_int32 BatchGetLUNInfo(mp_string& strUniqueID);
	static mp_int32 ReloadUDEVRules(mp_string& strUniqueID);
	static mp_int32 SyncDataFile(mp_string& strUniqueID);

#ifdef LIN_FRE_SUPP
   static mp_int32 ThawFileSys(mp_string& strUniqueID);
   static mp_int32 FreezeFileSys(mp_string& strUniqueID);
#endif

private:
    static mp_int32 GetParamFromTmpFile(mp_string& strUniqueID, mp_string& strParam);
    static mp_int32 GetLUNInfo(mp_string &strDevice, mp_string &strLUNInfo);
};

class CCommandMap
{
public:
    CCommandMap();
    ~CCommandMap()
    {
    }
    mp_string GetCommandString(mp_int32 iCommandID);
    mp_bool NeedEcho(mp_int32 iCommandID);
    mp_void InitDB2ScriptMap();
    mp_void InitOracleScriptMap();
	mp_void InitHostScriptMap();
    mp_void InitCacheScriptMap();
    mp_void InitSybaseScriptMap();
    mp_void InitHanaScriptMap();
    mp_void InitSysCmdMap1();
    mp_void InitSysCmdMap2();
    mp_void InitSysCmdMap3();
    mp_void InitSysCmdMap4();
    mp_void InitSysCmdMap5();
    mp_void InitSysCmdMap6();
    mp_void InitNeedEchoCmdMap1();
    mp_void InitNeedEchoCmdMap2();

private:
    map<mp_int32, mp_string> m_mapCommand;
    map<mp_int32, mp_bool> m_mapNeedEcho;
};

#endif

