/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_LVM_H__
#define __AGENT_LVM_H__

#ifndef WIN32
#include <vector>
#include "common/Types.h"

#define ALONE_TO_ALONE          0
#define ALONE_TO_CLUSTER      1
#define CLUSTER_TO_CLUSTER      3
#define CLUSTER_TO_ALONE          2

typedef struct tag_lv_info
{
    mp_uint64 ullCapcity;
    mp_int32 iVolType;
    mp_string strLvName;
    mp_string strPath;
}lv_info_t;

typedef struct tag_vg_info
{
    mp_string strVolType;
    mp_int32 iState;
    mp_string strVgName;
    mp_string strMapInfo;
    vector<mp_string> vecPvs;
}vg_info_t;

typedef struct tag_pv_info
{
    mp_string strPvName;
    mp_string strWWN;
}pv_info_t;

class CLvm
{
public:
    CLvm()
    {
    }
    ~CLvm()
    {
    }

    mp_int32 QueryVgInfo(vg_info_t& struVgInfo);
    mp_int32 QueryLvInfo(mp_string& strVgName, vector<lv_info_t>& vecLvs);
    mp_int32 ExportVg(mp_string& strVgName, mp_int32 iVolType);
    mp_int32 ImportVg(vector<mp_string>& vecPriPvName, mp_string& strVgName, mp_int32 iVolType, mp_string& strMapInfo, vector<mp_string>& vecWWN);
    mp_int32 ActivateVg(mp_string& strVgName, mp_int32 iVolType, mp_string strVgActiveMode, mp_int32 iRecoverType);
    mp_int32 DeActivateVg(mp_string& strVgName, mp_int32 iVolType);
    mp_int32 ScanDisks_VXVM();

private:
    //Linux
    mp_int32 QueryVgInfo_LLVM(vg_info_t& struVgInfo);
    mp_int32 ExportVg_LLVM(mp_string& strVgName);
    mp_int32 ImportVg_LLVM(mp_string& strVgName, vector<mp_string>& vecWWN);
    mp_int32 CheckVgStatus_LLVM(mp_string& strVgName, vector<mp_string>& vecWWN, mp_int32 &iSuccessTimes);
    mp_int32 ActivateVg_LLVM(mp_string& strVgName);
    mp_int32 DeActivateVg_LLVM(mp_string& strVgName);
    //AIX
    mp_int32 ExportVg_ALVM(mp_string& strVgName);
    mp_int32 ImportVg_ALVM(mp_string& strVgName, vector<mp_string>& vecWWN);
    mp_int32 ActivateVg_ALVM(mp_string& strVgName);
    mp_int32 DeActivateVg_ALVM(mp_string& strVgName);
    //HP
    mp_int32 QueryVgInfo_HLVM(vg_info_t& struVgInfo);

    mp_int32 ExportVg_HLVM(mp_string& strVgName);
    
    mp_int32 ImportVg_HLVM(vector<mp_string>& vecPriPvName, mp_string& strVgName, mp_string& strMapInfo, vector<mp_string>& vecWWN);

    mp_int32 ActivateVg_HLVM(mp_string& strVgName, mp_string strVgActiveMode, mp_int32 iRecoverType);

    mp_int32 DeActivateVg_HLVM(mp_string& strVgName);

    //VXVM
    mp_int32 ExportVg_VXVM(mp_string& strVgName );
    
    mp_int32 ImportVg_VXVM(mp_string& strVgName, mp_string& strMapInfo);
    
    mp_int32 ActivateVg_VXVM(mp_string& strVgName);
    
    mp_int32 DeActivateVg_VXVM(mp_string& strVgName);
    
    mp_bool WriteVgMapInfo(mp_string& strMapInfo, mp_string& strMapInfoFile);

    mp_int32 IsVgExported(mp_string& strVgName, mp_bool& bIsExported);

    mp_int32 IsVgExported_VXVM(mp_string& strVgName, mp_bool& bIsExported);
    
    mp_bool IsVgExist(mp_string& strVgName);

	mp_int32 GetVgName_LLVM(mp_string& strDevice, vector<mp_string> &vecVgName);

	mp_int32 GetVgName_ALVM(mp_string& strDevice, mp_string& strVgName);

	mp_int32 GetVgName_HLVM(mp_string& strDevice, mp_string& strVgName);
};

#endif //WIN32
#endif //__AGENT_LVM_H__

