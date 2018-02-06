/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/device/DevicePluginTest.h"


static int s_icounter_inner = 0;
static mp_int32 stubQueryFileSysInfo(void *ptr, std::vector<file_sys_info_t>& vecFileInfos)
{
    static int iCounter_inner = 0;
    if (iCounter_inner++)
    {
        return MP_FAILED;
    }
    else
    {
        file_sys_info_t sysInfo;
        vecFileInfos.push_back(sysInfo);

        return MP_SUCCESS;
    }
}


static  mp_int32 stubQueryVgInfo(void *ptr, vg_info_t& struVgInfo)
{
    static int iCounter_inner = 0;
    if (iCounter_inner++)
    {
        return MP_FAILED;
    }
    else
    {
        struVgInfo.vecPvs.push_back("abc");
        return MP_SUCCESS;
    }
 }

static mp_int32 stubReturnRet(mp_void)
{
    static int s_icounter_inner = 0;
    if (s_icounter_inner++)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 StubGetArrayJson(const Json::Value& jsValue, vector<Json::Value>& vecValue)
{
    vecValue.push_back(Json::Value("Value_value"));

    return MP_SUCCESS;
}

mp_int32 stub_return_mount_failed(mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType, mp_bool& bIsMounted)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0)
    {   
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

mp_bool stub_return_file_not_exist(const mp_char* pszFilePath)
{
    return MP_FALSE;
}

TEST_F(CDevicePluginTest, DoAction)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.DoAction(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);        
}


TEST_F(CDevicePluginTest, FileSysQueryInfo)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    typedef mp_int32 (CFileSys::*pOrgQueryFileSysInfo)(vector<file_sys_info_t>& vecFileInfos);
    typedef mp_int32 (*pStubQueryFileSysInfo)(void *ptr, vector<file_sys_info_t>& vecFileInfos);
    Stub<pOrgQueryFileSysInfo, pStubQueryFileSysInfo, mp_void> stubCFileSys(&CFileSys::QueryFileSysInfo, &stubQueryFileSysInfo);

    iRet = plugObj.FileSysQueryInfo(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);    

    iRet = plugObj.FileSysQueryInfo(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);    
}

TEST_F(CDevicePluginTest, FileSysMount)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj; 
    
    iRet = plugObj.FileSysMount(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
    
    typedef mp_int32 (CFileSys::*pOrgCheckMountedStatus)(mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType, mp_bool& bIsMounted);
    typedef mp_int32 (*pStubCheckMountedStatus)(mp_string& strDevPath, mp_string& strMountPoint, mp_int32 iVolType, mp_bool& bIsMounted);
    Stub<pOrgCheckMountedStatus, pStubCheckMountedStatus, mp_void> stubUnitlex(&CFileSys::CheckMountedStatus, &stub_return_mount_failed);
    iRet = plugObj.FileSysMount(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);  

    typedef mp_int32 (*pOrgCheckFileSysMountParam)(mp_string strDeviceName, mp_int32 volumeType, mp_string strMountPoint);
    Stub<pOrgCheckFileSysMountParam, pStubIntType, mp_void> stubUnitle(CheckFileSysMountParam, &stub_return_ret);
    plugObj.FileSysMount(&req, &rsp);
    plugObj.FileSysMount(&req, &rsp);

    typedef mp_int32 (CFileSys::*pOrgMount)(mount_info_t& mountInfo);
    Stub<pOrgMount, pStubIntType, mp_void> stubCFileSys(&CFileSys::Mount, &stub_return_ret);
    plugObj.FileSysMount(&req, &rsp);
    
}

TEST_F(CDevicePluginTest, FileSysBatchMount)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    req.m_msgBody.m_msgBodyType = BODY_DECODE_JSON;
    req.m_msgBody.m_msgLen = 500;
    req.m_msgBody.m_raw_msg  = std::auto_ptr<CMessage_Block>(new CMessage_Block(req.m_msgBody.m_msgLen));

    typedef mp_int32 (CFileSys::*pOrgMount)(mount_info_t& mountInfo);
    Stub<pOrgMount, pStubIntType, mp_void> stubCFileSys(&CFileSys::Mount, &stub_return_ret);

    iRet = plugObj.FileSysBatchMount(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = plugObj.FileSysBatchMount(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CDevicePluginTest, FileSysUmount)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.FileSysUmount(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);       

    iRet = plugObj.FileSysUmount(&req, &rsp);
    EXPECT_TRUE(1);      

    typedef mp_int32 (*pOrgCheckFileSysMountParam)(mp_string strDeviceName, mp_int32 volumeType, mp_string strMountPoint);
    Stub<pOrgCheckFileSysMountParam, pStubIntType, mp_void> stubUnitle(CheckFileSysMountParam, &stub_return_ret);
    plugObj.FileSysUmount(&req, &rsp);
    plugObj.FileSysUmount(&req, &rsp);
    
    typedef mp_int32 (CFileSys::*pOrgUMount)(umount_info_t& umountInfo);
    Stub<pOrgUMount, pStubIntType, mp_void> stubCFileSys(&CFileSys::UMount, &stub_return_ret);
    
    iRet = plugObj.FileSysUmount(&req, &rsp);
    EXPECT_TRUE(1);       
}


TEST_F(CDevicePluginTest, FileSysBatchUmount)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.FileSysBatchUmount(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}

TEST_F(CDevicePluginTest, DriveLetterBatchDelete)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.DriveLetterBatchDelete(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);        
}

TEST_F(CDevicePluginTest, QueryFreezeState)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    typedef mp_int32 (CFileSys::*pOrgQueryFreezeState)(vector<mp_string>& vecDriveLetters);
    Stub<pOrgQueryFreezeState, pStubIntType, mp_void> stubCFileSys(&CFileSys::QueryFreezeState, &stub_return_ret);

    iRet = plugObj.QueryFreezeState(&req, &rsp);
    EXPECT_TRUE(1);        
}

TEST_F(CDevicePluginTest, Freeze)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);   

    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet); 

    typedef mp_int32 (CFileSys::*pOrgFreeze)(vector<mp_string>& vecDriveLetters);
    Stub<pOrgFreeze, pStubIntType, mp_void> stubCFileSys(&CFileSys::Freeze, &stub_return_ret);

    iRet = plugObj.Freeze(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet); 
}

TEST_F(CDevicePluginTest, UnFreeze)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.UnFreeze(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);        

    iRet = plugObj.UnFreeze(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);        

    typedef mp_int32 (CFileSys::*pOrgUnFreeze)(vector<mp_string>& vecDriveLetters);
    Stub<pOrgUnFreeze, pStubIntType, mp_void> stubCFileSys(&CFileSys::UnFreeze, &stub_return_ret);

    iRet = plugObj.UnFreeze(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}


TEST_F(CDevicePluginTest, ScanASMLib)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    typedef mp_int32 (CAsm::*pOrgAsmLibScan)(mp_void);
    Stub<pOrgAsmLibScan, pStubIntType, mp_void> stubCAsm(&CAsm::AsmLibScan, &stub_return_ret);

    iRet = plugObj.ScanASMLib(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);        

    iRet = plugObj.ScanASMLib(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);       
}

TEST_F(CDevicePluginTest, LinkCreate)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.LinkCreate(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet); 

    typedef mp_bool (*pOrgFileExist)(const mp_char* pszFilePath);
    typedef mp_bool (*pStubFileExist)(const mp_char* pszFilePath);
    Stub<pOrgFileExist, pStubFileExist, mp_void> stubUnitlex(&CMpFile::FileExist, &stub_return_file_not_exist);
    iRet = plugObj.LinkCreate(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet); 
    //EXPECT_EQ(ERROR_COMMON_DEVICE_NOT_EXIST, iRet);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub00(CheckPathString, &stub_return_ret);
    plugObj.LinkCreate(&req, &rsp);
    
    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    /* LinkCreate ß∞‹; */
    plugObj.LinkCreate(&req, &rsp);
    
    typedef mp_int32 (CLink::*pOrgCreate)(link_info_t& linkInfo);
    Stub<pOrgCreate, pStubIntType, mp_void> stubCLink(&CLink::Create, &stub_return_ret);
    iRet = plugObj.LinkCreate(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);  
}

TEST_F(CDevicePluginTest, LinkDelete)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;
    s_icounter_inner = 0;

    iRet = plugObj.LinkDelete(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);    

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub00(CheckPathString, &stub_return_ret);
    plugObj.LinkDelete(&req, &rsp);

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.LinkDelete(&req, &rsp);

    typedef mp_int32 (CLink::*pOrgDelete)(link_info_t& linkInfo);
    typedef mp_int32 (*pStubReturnRet)(mp_void);
    Stub<pOrgDelete, pStubReturnRet, mp_void> stubCLink(&CLink::Delete, &stubReturnRet);

    iRet = plugObj.LinkDelete(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CDevicePluginTest, LinkBatchCreate)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.LinkBatchCreate(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}

TEST_F(CDevicePluginTest, LinkBatchDelete)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.LinkBatchDelete(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}

TEST_F(CDevicePluginTest, LVMQueryVgs)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.LVMQueryVgs(&req, &rsp);

    typedef mp_int32 (CLvm::*pOrgQueryVgInfo)(vg_info_t& struVgInfo);
    typedef mp_int32 (*pStubQueryVgInfo)(void *ptr, vg_info_t& struVgInfo);
    Stub<pOrgQueryVgInfo, pStubQueryVgInfo, mp_void> stubCLvm(&CLvm::QueryVgInfo, &stubQueryVgInfo);
    
    iRet = plugObj.LVMQueryVgs(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);  

    iRet = plugObj.LVMQueryVgs(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);  
}

TEST_F(CDevicePluginTest, LVMExportVgs)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;
    s_icounter_inner = 0;

    iRet = plugObj.LVMExportVgs(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);    

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.LVMExportVgs(&req, &rsp);

    typedef mp_int32 (*pOrgCheckParamInteger32)(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude);
    Stub<pOrgCheckParamInteger32, pStubIntType, mp_void> stub00(CheckParamInteger32, &stub_return_ret);
    plugObj.LVMExportVgs(&req, &rsp);
    
    typedef mp_int32 (CLvm::*pOrgExportVg)(mp_string& strVgName, mp_int32 iVolType);
    typedef mp_int32 (*pStubReturnRet)(mp_void);
    Stub<pOrgExportVg, pStubReturnRet, mp_void> stubCLvm(&CLvm::ExportVg, &stubReturnRet);
    
    iRet = plugObj.LVMExportVgs(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);        
}

TEST_F(CDevicePluginTest, LVMImportVgs)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.LVMImportVgs(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.LVMImportVgs(&req, &rsp);

    typedef mp_int32 (*pOrgCheckParamInteger32)(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude);
    Stub<pOrgCheckParamInteger32, pStubIntType, mp_void> stub00(CheckParamInteger32, &stub_return_ret);
    plugObj.LVMImportVgs(&req, &rsp);

    typedef mp_int32 (CLvm::*pOrgImportVg)(vector<mp_string>& vecPriPvName, mp_string& strVgName, mp_int32 iVolType, mp_string& strMapInfo, vector<mp_string>& vecWWN);
    Stub<pOrgImportVg, pStubIntType, mp_void> stubCLvm(&CLvm::ImportVg, &stub_return_ret);

    iRet = plugObj.LVMImportVgs(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}

TEST_F(CDevicePluginTest, LVMActivateVgs)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.LVMActivateVgs(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);       

    iRet = plugObj.LVMActivateVgs(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}

TEST_F(CDevicePluginTest, LVMDeactivateVgs)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.LVMDeactivateVgs(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);             

    typedef mp_int32 (*pOrgCheckParamInteger32)(mp_int32 paramValue, mp_int32 begValue, mp_int32 endValue, vector<mp_int32> &vecExclude);
    Stub<pOrgCheckParamInteger32, pStubIntType, mp_void> stub00(CheckParamInteger32, &stub_return_ret);
    iRet = plugObj.LVMDeactivateVgs(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);

    typedef mp_int32 (CLvm::*pOrgDeActivateVg_LLVM)(mp_string& strVgName);
    Stub<pOrgDeActivateVg_LLVM, pStubIntType, mp_void> stub01(&CLvm::DeActivateVg_LLVM, &stub_return_ret);
    iRet = plugObj.LVMDeactivateVgs(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet); 
}

TEST_F(CDevicePluginTest, LVMQueryLVs)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.LVMQueryLVs(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}

TEST_F(CDevicePluginTest, LVMScanDisks)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    typedef mp_int32 (CLvm::*pOrgScanDisks_VXVM)(mp_void);
    Stub<pOrgScanDisks_VXVM, pStubIntType, mp_void> stubCLvm(&CLvm::ScanDisks_VXVM, &stub_return_ret);

    iRet = plugObj.LVMScanDisks(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);      

    iRet = plugObj.LVMScanDisks(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}

TEST_F(CDevicePluginTest, UDEVCreateRules)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.UDEVCreateRules(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);  

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    iRet = plugObj.UDEVCreateRules(&req, &rsp);
    EXPECT_EQ(ERROR_DEVICE_UDEV_CREATE_FAILED, iRet);

    typedef mp_int32 (CUdev::*pOrgCreate)(mp_string& strUdevRule, mp_string& strWWN);
    Stub<pOrgCreate, pStubIntType, mp_void> stubCUdev(&CUdev::Create, &stub_return_ret);
    iRet = plugObj.UDEVCreateRules(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);  
}

TEST_F(CDevicePluginTest, UDEVDeleteRules)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.UDEVDeleteRules(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);          

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    iRet = plugObj.UDEVDeleteRules(&req, &rsp);
    EXPECT_EQ(ERROR_DEVICE_UDEV_DELETE_FAILED, iRet); 

    typedef mp_int32 (CUdev::*pOrgDelete)(mp_string& strUdevRule, mp_string& strWWN);
    Stub<pOrgDelete, pStubIntType, mp_void> stubCUdev(&CUdev::Delete, &stub_return_ret);
    iRet = plugObj.UDEVDeleteRules(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CDevicePluginTest, UDEVBatchCreateRules)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.UDEVBatchCreateRules(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);       

    iRet = plugObj.UDEVBatchCreateRules(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);     
}

TEST_F(CDevicePluginTest, UDEVBatchDeleteRules)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.UDEVBatchDeleteRules(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);    
    
    iRet = plugObj.UDEVBatchDeleteRules(&req, &rsp);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
}

TEST_F(CDevicePluginTest, RawDeviceCreate)
{
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    iRet = plugObj.RawDeviceCreate(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);      

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    iRet = plugObj.RawDeviceCreate(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);  

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub00(CheckPathString, &stub_return_ret);
    plugObj.RawDeviceCreate(&req, &rsp);
    
    typedef mp_int32 (CRaw::*pOrgCreate)(raw_info_t& rawInfo);
    Stub<pOrgCreate, pStubIntType, mp_void> stubCRaw(&CRaw::Create, &stub_return_ret);

    iRet = plugObj.RawDeviceCreate(&req, &rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);        
}

TEST_F(CDevicePluginTest, RawDeviceDelete)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;
    s_icounter_inner = 0;

    iRet = plugObj.RawDeviceDelete(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);    

    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.RawDeviceDelete(&req, &rsp);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub00(CheckPathString, &stub_return_ret);
   plugObj.RawDeviceDelete(&req, &rsp);

    typedef mp_int32 (CRaw::*pOrgDelete)(raw_info_t& rawInfo);
    typedef mp_int32 (*pStubReturnRet)(mp_void);
    Stub<pOrgDelete, pStubReturnRet, mp_void> stubCRaw(&CRaw::Delete, &stubReturnRet);

    iRet = plugObj.RawDeviceDelete(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet);
}


static mp_int32 StubCreate(mp_void)
{
    static int i = 0;
    return  (i++ == 0) ? MP_FAILED: MP_SUCCESS;
}
TEST_F(CDevicePluginTest, RawDeviceBatchCreate)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    typedef  mp_int32 (*pOrgGetArrayJson)(const Json::Value& jsValue, vector<Json::Value>& vecValue);
    typedef  mp_int32 (*pStubGetArrayJson)(const Json::Value& jsValue, vector<Json::Value>& vecValue);
    Stub<pOrgGetArrayJson, pStubGetArrayJson, mp_void> stub(&CJsonUtils::GetArrayJson, StubGetArrayJson);
    plugObj.RawDeviceBatchCreate(&req, &rsp);
    
    plugObj.RawDeviceBatchCreate(&req, &rsp);
    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.RawDeviceBatchCreate(&req, &rsp);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub00(CheckPathString, &stub_return_ret);
    iRet = plugObj.RawDeviceBatchCreate(&req, &rsp);      

    typedef mp_int32 (CRaw::*pOrgCreate)(raw_info_t& rawInfo);
    typedef mp_int32 (*pStubCreate)(mp_void);
    Stub<pOrgCreate, pStubCreate, mp_void> stubCRaw(&CRaw::Create, &StubCreate);
    plugObj.RawDeviceBatchCreate(&req, &rsp);
    
    plugObj.RawDeviceBatchCreate(&req, &rsp);   
}


static mp_int32 StubDelete(mp_void)
{
    static int i = 0;
    return  (i++ == 0) ? MP_FAILED: MP_SUCCESS;
}
TEST_F(CDevicePluginTest, RawDeviceBatchDelete)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    typedef  mp_int32 (*pOrgGetArrayJson)(const Json::Value& jsValue, vector<Json::Value>& vecValue);
    typedef  mp_int32 (*pStubGetArrayJson)(const Json::Value& jsValue, vector<Json::Value>& vecValue);
    Stub<pOrgGetArrayJson, pStubGetArrayJson, mp_void> stub(&CJsonUtils::GetArrayJson, StubGetArrayJson);
    plugObj.RawDeviceBatchDelete(&req, &rsp);

    plugObj.RawDeviceBatchDelete(&req, &rsp);
    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    plugObj.RawDeviceBatchDelete(&req, &rsp);

    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub00(CheckPathString, &stub_return_ret);
    iRet = plugObj.RawDeviceBatchDelete(&req, &rsp);

    typedef mp_int32 (CRaw::*pOrgDelete)(raw_info_t& rawInfo);
    typedef mp_int32 (*pStubDelete)(mp_void);
    Stub<pOrgDelete, pStubDelete, mp_void> stubCRaw(&CRaw::Delete, &StubDelete);
    plugObj.RawDeviceBatchDelete(&req, &rsp);
    
    plugObj.RawDeviceBatchDelete(&req, &rsp);      
}


static mp_int32 StubSet(mp_void)
{
    static int i = 0;
    return  (i++ == 0) ? MP_FAILED: MP_SUCCESS;
}

TEST_F(CDevicePluginTest, Permission)
 {
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    CDevicePlugin plugObj;

    typedef  mp_int32 (*pOrgGetArrayJson)(const Json::Value& jsValue, vector<Json::Value>& vecValue);
    typedef  mp_int32 (*pStubGetArrayJson)(const Json::Value& jsValue, vector<Json::Value>& vecValue);
    Stub<pOrgGetArrayJson, pStubGetArrayJson, mp_void> stub(&CJsonUtils::GetArrayJson, StubGetArrayJson);
    iRet = plugObj.Permission(&req, &rsp);
    EXPECT_EQ(MP_FAILED, iRet); 
 
    typedef bool (Json::Value::*pOrgisMember)( const char *key ) const;
    Stub<pOrgisMember, pStubBoolType, mp_void> stubValue(&Json::Value::isMember, &stub_return_bool);
    iRet = plugObj.Permission(&req, &rsp);
	EXPECT_EQ(MP_FAILED, iRet);
	
    typedef mp_int32 (*pOrgCheckParamString)(mp_string &paramValue, mp_int32 lenBeg, mp_int32 lenEnd, mp_string &strInclude, mp_string &strExclude);
    Stub<pOrgCheckParamString, pStubIntType, mp_void> stub01(CheckParamString, &stub_return_ret);
    iRet = plugObj.Permission(&req, &rsp);
	EXPECT_EQ(MP_FAILED, iRet);
		
    typedef mp_int32 (*pOrgCheckPathString)(mp_string &pathValue, mp_string strPre);
    Stub<pOrgCheckPathString, pStubIntType, mp_void> stub00(CheckPathString, &stub_return_ret);
    plugObj.Permission(&req, &rsp);

    typedef mp_int32 (CPermission::*pOrgSet)(permission_info_t& permissionInfo);
    typedef mp_int32 (*pStubSet)(mp_void);
    Stub<pOrgSet, pStubSet, mp_void> stubCPermission(&CPermission::Set, &StubSet);
    plugObj.Permission(&req, &rsp);

    plugObj.Permission(&req, &rsp);
}


