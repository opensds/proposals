/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

// provider.h : Cprovider µÄÉùÃ÷
#ifdef WIN32

#pragma once
#include "vss/provider/resource.h"       // Ö÷·ûºÅ
#include "common/Types.h"
#include "rdvss.h"
#include "vss.h"
#include "vsprov.h"

static long g_nComObjsInUse;

class RDVSSEnumObject : public IVssEnumObject
{
public:
    STDMETHODIMP QueryInterface(REFIID riid, void **ppObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP Next(ULONG celt, VSS_OBJECT_PROP *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IVssEnumObject **ppenum);

    RDVSSEnumObject();
    ~RDVSSEnumObject();

private:
    long m_nRefCount;
};

class ATL_NO_VTABLE Cprovider :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<Cprovider, &CLSID_provider>,
    public IVssProviderCreateSnapshotSet,
    public IVssSoftwareSnapshotProvider
{
public:
    Cprovider();
    ~Cprovider();

DECLARE_REGISTRY_RESOURCEID(IDR_PROVIDER)


BEGIN_COM_MAP(Cprovider)
    COM_INTERFACE_ENTRY(IVssProviderCreateSnapshotSet)
    COM_INTERFACE_ENTRY(IVssSoftwareSnapshotProvider)
END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

    //IVssSoftwareSnapshotProvider
public:
    STDMETHOD(BeginPrepareSnapshot)(
        VSS_ID SnapshotSetId,
        VSS_ID SnapshotId,
        VSS_PWSZ pwszVolumeName,
        LONG lNewContext
        );

    STDMETHOD(DeleteSnapshots)(
        VSS_ID SourceObjectId,
        VSS_OBJECT_TYPE eSourceObjectType,
        BOOL bForceDelete,
        LONG *plDeletedSnapshots,
        VSS_ID *pNondeletedSnapshotID
        );

    STDMETHOD(GetSnapshotProperties)(
        VSS_ID SnapshotId,
        VSS_SNAPSHOT_PROP *pProp
        );

    STDMETHOD(IsVolumeSnapshotted)(
        VSS_PWSZ pwszVolumeName,
        BOOL *pbSnapshotsPresent,
        LONG *plSnapshotCompatibility
        );

    STDMETHOD(IsVolumeSupported)(
        VSS_PWSZ pwszVolumeName,
        BOOL *pbSupportedByThisProvider
        );

    STDMETHOD(Query)(
        VSS_ID QueriedObjectId,
        VSS_OBJECT_TYPE eQueriedObjectType,
        VSS_OBJECT_TYPE eReturnedObjectsType,
        IVssEnumObject **ppEnum
        );

    STDMETHOD(QueryRevertStatus)(
        VSS_PWSZ pwszVolume,
        IVssAsync **ppAsync
        );

    STDMETHOD(RevertToSnapshot)(
        VSS_ID SnapshotId
        );

    STDMETHOD(SetContext)(
        LONG lContext
        );

    STDMETHOD(SetSnapshotProperty)(
        VSS_ID SnapshotId,
        VSS_SNAPSHOT_PROPERTY_ID eSnapshotPropertyId,
        VARIANT vProperty
        );

    // IVssProviderCreateSnapshotSet Methods
public:
    STDMETHOD(EndPrepareSnapshots)(
        VSS_ID SnapshotSetId
        );

    STDMETHOD(PreCommitSnapshots)(
        VSS_ID SnapshotSetId
        );

    STDMETHOD(CommitSnapshots)(
        VSS_ID SnapshotSetId
        );

    STDMETHOD(PostCommitSnapshots)(
        VSS_ID SnapshotSetId,
        LONG lSnapshotsCount
        );

    STDMETHOD(PreFinalCommitSnapshots)(
        VSS_ID SnapshotSetId
        );

    STDMETHOD(PostFinalCommitSnapshots)(
        VSS_ID SnapshotSetId
        );

    STDMETHOD(AbortSnapshots)(
        VSS_ID SnapshotSetId
        );

private:
    int InitConf(char* pszConfPath);
    int InitLogger(char* pszLogPath);
    void Init();
    bool GetServicePath(const wchar_t* wpszServiceName, string& strServicePath);
};

OBJECT_ENTRY_AUTO(__uuidof(provider), Cprovider)

#endif

