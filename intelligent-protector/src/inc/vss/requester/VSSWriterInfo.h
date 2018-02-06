/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

  Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : VSSWriterInfo.h
  Version       : Initial Draft
  Author        : yangwenun 00275736
  Created       : 2014/04/30
  Last Modified :
  Description   : VSS Component/Writer/FS·â×°Àà
  History       :
  1.Date        :
    Author      :
    Modification:
******************************************************************************/

#ifndef _RD_VSS_WRITER_INFO_
#define _RD_VSS_WRITER_INFO_

#include "vss/requester/VSSCommon.h"
#include "common/ErrorCode.h"
#ifdef WIN32

typedef enum
{
    VSS_FDT_UNDEFINED = 0,
    VSS_FDT_EXCLUDE_FILES,
    VSS_FDT_FILELIST,
    VSS_FDT_DATABASE,
    VSS_FDT_DATABASE_LOG,
} VSS_DESCRIPTOR_TYPE;

struct VssFileDescriptor
{
    VssFileDescriptor() : m_bIsRecursive(false), m_enType(VSS_FDT_UNDEFINED){};

    wstring GetVolumeName(wstring wstrPath);

    // Initialize from a IVssWMFiledesc
    mp_bool Initialize(IVssWMFiledesc* pFileDesc, VSS_DESCRIPTOR_TYPE typeParam);

    // Get the string representation of the type
    wstring GetStringFromFileDescriptorType(VSS_DESCRIPTOR_TYPE eType);

    wstring m_wstrPath;
    wstring m_wstrFilespec;
    wstring m_wstrAlternatePath;
    bool m_bIsRecursive;
    VSS_DESCRIPTOR_TYPE m_enType;
    wstring m_wstrExpandedPath;
    wstring m_wstrAffectedVolume;
};

struct VssComponent
{
    VssComponent() : m_enType(VSS_CT_UNDEFINED), m_bIsSelectable(false), m_bIsTopLevel(false), m_bIsExcluded(false),
        m_bIsExplicitlyIncluded(false) {};

    bool NeedInit(PVSSCOMPONENTINFO pInfo, wstring writerName, vector<wstring>& vecDBNames);

    // Initialize from a IVssWMComponent
    void Initialize(wstring writerNameParam, IVssWMComponent* pComponent);

    // Initialize from a IVssComponent
    void Initialize(wstring writerNameParam, IVssComponent* pComponent);

    // Convert a component type into a string
    wstring GetStringFromComponentType(VSS_COMPONENT_TYPE eComponentType);

    // Return TRUE if the current component is ancestor of the given component
    bool IsAncestorOf(VssComponent& child);

    // return TRUEif it can be explicitly included
    bool CanBeExplicitlyIncluded();

    VSS_COMPONENT_TYPE m_enType;
    wstring m_wstrName;
    wstring m_wstrWriterName;
    wstring m_wstrLogicalPath;
    wstring m_wstrCaption;
    wstring m_wstrFullPath;
    bool m_bIsSelectable;
    bool m_bNotifyOnBackupComplete;
    bool m_bIsTopLevel;
    bool m_bIsExcluded;
    bool m_bIsExplicitlyIncluded;
    vector<wstring> m_vecAffectedPaths;
    vector<wstring> m_vecAffectedVolumes;
    vector<VssFileDescriptor> m_vecDescriptors;
};

struct VssWriter
{
    VssWriter() : m_bIsExcluded(false){};

    //bool Initialize(IVssExamineWriterMetadata* pMetadata, wstring& wstrWriterName, vector<wstring>& vecDBNames);
    bool Initialize(IVssExamineWriterMetadata* pMetadata, wstring* pwstrWriterName);

    wstring m_wstrName;
    wstring m_wstrId;
    wstring m_wstrInstanceId;
    bool m_bIsExcluded;
    vector<VssComponent> m_vecComponents;
    vector<VssFileDescriptor> m_vecExcludedFiles;
};

#endif
#endif

