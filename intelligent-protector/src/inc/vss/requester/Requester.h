/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

  Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : Requester.h
  Version       : Initial Draft
  Author        : yangwenun 00275736
  Created       : 2014/04/30
  Last Modified :
  Description   : VSS Requester，触发创建快照流程
                  与VSS Provider配合，对外提供冻结Freeze和解冻UnFreeze接口
  History       :
  1.Date        :
    Author      :
    Modification:
******************************************************************************/

#ifndef _RD_VSS_REQUESTER_
#define _RD_VSS_REQUESTER_

#include "vss/requester/VSSCommon.h"
#include "vss/requester/VSSWriterInfo.h"
#include "common/ErrorCode.h"
#include "common/Defines.h"
#ifdef WIN32

#define VSS_SQLSERVER_WRITER_NAME        "SqlServerWriter"
#define VSS_SQLSERVER_WRITER_NAME_W      L"SqlServerWriter"
#define VSS_EXCHANGE_WRITER_NAME         "Microsoft Exchange Writer"
#define VSS_EXCHANGE_WRITER_NAME_W       L"Microsoft Exchange Writer"
#define VSS_FILESYSTEM_WRITER_NAME       "File System Writer"
#define VSS_FILESYSTEM_WRITER_NAME_W     L"File System Writer"

//Enums undefined in VSS SDK 7.2 but defined in newer Windows SDK
#define VSS_VOLSNAP_ATTR_NO_AUTORECOVERY       0x00000002
#define VSS_VOLSNAP_ATTR_TXF_RECOVERY          0x02000000

enum vss_freeze_type
{ 
    vss_freeze_sqlserver=1,
    vss_freeze_exchange,
    vss_freeze_filesys
};

typedef struct tag_vss_db_oper_info
{
    string strDbName;
    vector<string> vecDriveLetters;
}vss_db_oper_info_t;

class COMInitializer
{
public:
    COMInitializer() {CoInitialize(NULL);}
    ~COMInitializer() {CoUninitialize();}
};

class VSSRequester
{
private:
    vector<vss_db_oper_info_t> m_vecDbInfos;  //保存冻结时指定的信息
    wstring m_wstrWriterName;
    vss_freeze_type m_eFreezeType;
    vector<wstring> m_vecDBNames;
    CComPtr<IVssBackupComponents> m_pVssObject;
    CComPtr<IVssAsync> m_pAsyncSnapshot;
    HANDLE m_hEventFrozen;
    HANDLE m_hEventThaw;
    HANDLE m_hEventTimeOut;
    HANDLE m_hEventEndBackup;
    vector<VssWriter> m_vecWriters;

public:
    VSSRequester();
    ~VSSRequester();

    int Init();
    //Sqlserver/Exchange和文件系统的冻结解冻的区别在于前者需要指定特定的writer，后者不需要。
    //SqlServer/Exchange冻结解冻
    int Freeze(vector<vss_db_oper_info_t>& vecVssDBInfos,    vss_freeze_type eFreezeType);
    int UnFreeze(vector<vss_db_oper_info_t>& vecVssDBInfos);
    //文件系统冻结解冻
    int Freeze(vector<string>& vecDriveLetters, vss_freeze_type eFreezeType);
    int UnFreeze(vector<string>& vecDriveLetters);
    //冻结所有卷(HCP
    int FreezeAll();
    int UnFreezeAll();
    int EndBackup(int iBackupSucc);
    
private:
    int FreezeProc(vector<wstring> vecVolumePaths, vss_freeze_type eFreezeType);
    int UnFreezeProc();
    void FreezeFailedProcess();
    void CleanUp();
    void CleanUpEx();
    void WaitForAsync(IVssAsync* pAsync);
    int GetWriterName(vss_freeze_type eFreezeType, wstring& wstrWriterName);
    HRESULT WaitForAsyncEx(IVssAsync* pAsync);
    bool IsMatch(vector<vss_db_oper_info_t>& vecDbInfoSrc, vector<vss_db_oper_info_t>& vecDbInfoDes);
    void GetDriveLetterList(vector<vss_db_oper_info_t>& vecVssDBInfos, vector<string>& vecDriveLetters);
    void GetDbNameList(vector<vss_db_oper_info_t>& vecVssDBInfos, list<string>& lstDbNames);
    void GetComponents(wstring& wstrWriterName, vector<wstring>& vecDBNames, vector<wstring>& vecIncComponents);
    int GetVolumes(vector<string>& vecDriveLetters, vector<wstring>& vecVolumes);
    int CreateEvents();
    int CreateEventsEx();
    void InitializeWriterMetadata(wstring* pwstrWriterName);
    void GatherWriterMetadata(wstring* pwstrWriterName);
    void DiscoverNonShadowedExcludedComponents(vector<wstring> shadowSourceVolumes);
    void DiscoverAllExcludedComponents();
    void DiscoverExcludedWriters();
    bool IsDescendent(wstring& wstrDescendentFullPath, vector<wstring> vecAcestorFullPaths);
    bool IsAncestor(wstring& wstrAcestorFullPath, wstring& wstrDescendentFullPath);
    void DiscoverExplicitelyIncludedComponents();
    void SelectExplicitelyIncludedComponents();
    bool VerifyOneComponent(VssComponent& component, VssWriter& writer, wstring& wstrIncludedComponent);
    bool VerifyCompsInOneWirter(wstring& wstrIncludedComponent, VssWriter& writer);
    void VerifyIncludedComponent(wstring& wstrIncludedComponent, vector<VssWriter>& vecWriterList);
    void CheckIncludedList(vector<wstring>& vecIncludedList);
    void GetIncludeComponentFullPaths(vector<VssWriter>& vecWriterList,
        vector<wstring>& vecIncludedComponentList, vector<wstring>& vecComponentFullPaths);
    void DiscoverDirectlyIncludedComponents(vector<wstring>& vecIncludedComponentList,
        vector<VssWriter>& vecWriterList);
    void SelectComponentsForBackup(vector<wstring>& shadowSourceVolumes, 
        vector<wstring>& includedComponentList);
    void AddToSnapshotSet(vector<wstring> volumeList);
    void PrepareForBackup();
    void DoSnapshotSet();
    void CreateSnapShot(vector<wstring>& vecVolumes, vector<wstring>& vecIncComponents);
    void GatherWriterStatus();
    bool IsWriterSelected(GUID guidInstanceId);
    wstring GetStringFromWriterStatus(VSS_WRITER_STATE eWriterStatus);
    void CheckSelectedWriterStatus();
    void SetBackupSucceeded(bool succeeded);
    void BackupComplete(bool succeeded);
    void WaitThawReturn(bool bAutoEndBackup);
};

#endif
#endif

