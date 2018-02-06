/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/LvmTest.h"

TEST_F(CLvmTest,QueryVgInfo){
    CLvm m_CLvm;
    vg_info_t struVgInfo;
    
    m_CLvm.QueryVgInfo(struVgInfo);
}

TEST_F(CLvmTest,QueryVgInfo_LLVM){
    CLvm m_CLvm;
    vg_info_t struVgInfo;
    
    {
        m_CLvm.QueryVgInfo_LLVM(struVgInfo);
    }
    
    {
        Stub<CLvmSysExecType, StubCLvmSysExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCLvmSysExect);
        m_CLvm.QueryVgInfo_LLVM(struVgInfo);
    }
}

TEST_F(CLvmTest,QueryVgInfo_HLVM){
    CLvm m_CLvm;
    vg_info_t struVgInfo;
    
    m_CLvm.QueryVgInfo_HLVM(struVgInfo);
}

TEST_F(CLvmTest,QueryLvInfo){
    CLvm m_CLvm;
    mp_string strVgName;
    vector<lv_info_t> vecLvs;
    
    m_CLvm.QueryLvInfo(strVgName,vecLvs);
}

TEST_F(CLvmTest,ExportVg){
    CLvm m_CLvm;
    mp_string strVgName;
    mp_int32 iVolType;
    
    {
        iVolType = VOLUME_TYPE_LINUX_LVM;
        m_CLvm.ExportVg(strVgName,iVolType);
    }
    
    {
        iVolType = VOLUME_TYPE_AIX_LVM;
        m_CLvm.ExportVg(strVgName,iVolType);
    }
    
    {
        iVolType = VOLUME_TYPE_HP_LVM;
        m_CLvm.ExportVg(strVgName,iVolType);
    }
    
    {
        iVolType = VOLUME_TYPE_LINUX_VXVM;
        m_CLvm.ExportVg(strVgName,iVolType);
    }
    
    {
        iVolType = VOLUME_TYPE_SIMPLE;
        m_CLvm.ExportVg(strVgName,iVolType);
    }
}

TEST_F(CLvmTest,ImportVg){
    CLvm m_CLvm;
    vector<mp_string> vecPriPvName;
    mp_string strVgName;
    mp_int32 iVolType;
    mp_string strMapInfo;
    vector<mp_string> vecWWN;
    
    {
        iVolType = VOLUME_TYPE_LINUX_LVM;
        m_CLvm.ImportVg(vecPriPvName,strVgName,iVolType,strMapInfo,vecWWN);
    }
    
    {
        iVolType = VOLUME_TYPE_AIX_LVM;
        m_CLvm.ImportVg(vecPriPvName,strVgName,iVolType,strMapInfo,vecWWN);
    }
    
    {
        iVolType = VOLUME_TYPE_HP_LVM;
        m_CLvm.ImportVg(vecPriPvName,strVgName,iVolType,strMapInfo,vecWWN);
    }
    
    {
        iVolType = VOLUME_TYPE_LINUX_VXVM;
        m_CLvm.ImportVg(vecPriPvName,strVgName,iVolType,strMapInfo,vecWWN);
    }
    
    {
        iVolType = VOLUME_TYPE_SIMPLE;
        m_CLvm.ImportVg(vecPriPvName,strVgName,iVolType,strMapInfo,vecWWN);
    }
}

TEST_F(CLvmTest,ActivateVg){
    CLvm m_CLvm;
    mp_string strVgName;
    mp_int32 iVolType;
    mp_string strVgActiveMode;
    mp_int32 iRecoverType;
    
    {
        iVolType = VOLUME_TYPE_LINUX_LVM;
        m_CLvm.ActivateVg(strVgName,iVolType,strVgActiveMode,iRecoverType);
    }
    
    {
        iVolType = VOLUME_TYPE_AIX_LVM;
        m_CLvm.ActivateVg(strVgName,iVolType,strVgActiveMode,iRecoverType);
    }
    
    {
        iVolType = VOLUME_TYPE_HP_LVM;
        m_CLvm.ActivateVg(strVgName,iVolType,strVgActiveMode,iRecoverType);
    }
    
    {
        iVolType = VOLUME_TYPE_LINUX_VXVM;
        m_CLvm.ActivateVg(strVgName,iVolType,strVgActiveMode,iRecoverType);
    }
    
    {
        iVolType = VOLUME_TYPE_SIMPLE;
        m_CLvm.ActivateVg(strVgName,iVolType,strVgActiveMode,iRecoverType);
    }
}

TEST_F(CLvmTest,DeActivateVg){
    CLvm m_CLvm;
    mp_string strVgName;
    mp_int32 iVolType;
    
    {
        iVolType = VOLUME_TYPE_LINUX_LVM;
        m_CLvm.DeActivateVg(strVgName,iVolType);
    }
    
    {
        iVolType = VOLUME_TYPE_AIX_LVM;
        m_CLvm.DeActivateVg(strVgName,iVolType);
    }
    
    {
        iVolType = VOLUME_TYPE_HP_LVM;
        m_CLvm.DeActivateVg(strVgName,iVolType);
    }
    
    {
        iVolType = VOLUME_TYPE_LINUX_VXVM;
        m_CLvm.DeActivateVg(strVgName,iVolType);
    }
    
    {
        iVolType = VOLUME_TYPE_SIMPLE;
        m_CLvm.DeActivateVg(strVgName,iVolType);
    }
}

TEST_F(CLvmTest,ImportVg_LLVM){
    CLvm m_CLvm;
    mp_string strVgName;
    vector<mp_string> vecWWN;
    
    {
        vecWWN.push_back("test");
        m_CLvm.ImportVg_LLVM(strVgName,vecWWN);
    }
    
    {
        vecWWN.push_back("test");
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub1(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        m_CLvm.ImportVg_LLVM(strVgName,vecWWN);
    }
    
    {
        vecWWN.push_back("test");
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub1(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        Stub<GetVgName_LLVMType, StubGetVgName_LLVMType, mp_void> mystub2(&CLvm::GetVgName_LLVM, &StubGetVgName_LLVM);
        m_CLvm.ImportVg_LLVM(strVgName,vecWWN);
    }
    
    {
        vecWWN.push_back("test");
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub1(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        Stub<GetVgName_LLVMType, StubGetVgName_LLVMType, mp_void> mystub2(&CLvm::GetVgName_LLVM, &StubGetVgName_LLVM);
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub3(&CLvm::IsVgExported, &StubIsVgExported);
        m_CLvm.ImportVg_LLVM(strVgName,vecWWN);
    }
    
    {
        vecWWN.push_back("test");
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub1(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        Stub<GetVgName_LLVMType, StubGetVgName_LLVMType, mp_void> mystub2(&CLvm::GetVgName_LLVM, &StubGetVgName_LLVM);
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub3(&CLvm::IsVgExported, &StubIsVgExported0);
        m_CLvm.ImportVg_LLVM(strVgName,vecWWN);
    }
}

TEST_F(CLvmTest,ImportVg_HLVM){
    CLvm m_CLvm;
    mp_string strVgName = "test";
    vector<mp_string> vecPriPvName;
    mp_string strMapInfo = "test";
    vector<mp_string> vecWWN;
    
    vecWWN.push_back("test");
    
    {
        m_CLvm.ImportVg_HLVM(vecPriPvName,strVgName,strMapInfo,vecWWN);
    }
    
    {
        vecPriPvName.push_back("test");
        Stub<WriteVgMapInfoType, StubWriteVgMapInfoType, mp_void> mystub1(&CLvm::WriteVgMapInfo, &StubWriteVgMapInfo);
        m_CLvm.ImportVg_HLVM(vecPriPvName,strVgName,strMapInfo,vecWWN);
    }
    
    {
        vecPriPvName.push_back("test");
        Stub<WriteVgMapInfoType, StubWriteVgMapInfoType, mp_void> mystub1(&CLvm::WriteVgMapInfo, &StubWriteVgMapInfo);
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub2(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        m_CLvm.ImportVg_HLVM(vecPriPvName,strVgName,strMapInfo,vecWWN);
    }
    
    {
        vecPriPvName.push_back("test");
        Stub<WriteVgMapInfoType, StubWriteVgMapInfoType, mp_void> mystub1(&CLvm::WriteVgMapInfo, &StubWriteVgMapInfo);
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub2(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        Stub<GetSecPvNameType, StubGetSecPvNameType, mp_void> mystub3(&CDisk::GetSecPvName, &StubGetSecPvName);
        m_CLvm.ImportVg_HLVM(vecPriPvName,strVgName,strMapInfo,vecWWN);
    }
    
    {
        vecPriPvName.push_back("test");
        Stub<WriteVgMapInfoType, StubWriteVgMapInfoType, mp_void> mystub1(&CLvm::WriteVgMapInfo, &StubWriteVgMapInfo);
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub2(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        Stub<GetSecPvNameType, StubGetSecPvNameType, mp_void> mystub3(&CDisk::GetSecPvName, &StubGetSecPvName);
        Stub<GetVgName_HLVMType, StubGetVgName_HLVMType, mp_void> mystub4(&CLvm::GetVgName_HLVM, &StubGetVgName_HLVM);
        m_CLvm.ImportVg_HLVM(vecPriPvName,strVgName,strMapInfo,vecWWN);
    }
    
    {
        vecPriPvName.push_back("test");
        Stub<WriteVgMapInfoType, StubWriteVgMapInfoType, mp_void> mystub1(&CLvm::WriteVgMapInfo, &StubWriteVgMapInfo);
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub2(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        Stub<GetSecPvNameType, StubGetSecPvNameType, mp_void> mystub3(&CDisk::GetSecPvName, &StubGetSecPvName);
        Stub<GetVgName_HLVMType, StubGetVgName_HLVMType, mp_void> mystub4(&CLvm::GetVgName_HLVM, &StubGetVgName_HLVMt);
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub5(&CLvm::IsVgExported, &StubIsVgExported);
        m_CLvm.ImportVg_HLVM(vecPriPvName,strVgName,strMapInfo,vecWWN);
    }
    
    {
        vecPriPvName.push_back("test");
        Stub<WriteVgMapInfoType, StubWriteVgMapInfoType, mp_void> mystub1(&CLvm::WriteVgMapInfo, &StubWriteVgMapInfo);
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub2(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        Stub<GetSecPvNameType, StubGetSecPvNameType, mp_void> mystub3(&CDisk::GetSecPvName, &StubGetSecPvName);
        Stub<GetVgName_HLVMType, StubGetVgName_HLVMType, mp_void> mystub4(&CLvm::GetVgName_HLVM, &StubGetVgName_HLVMt);
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub5(&CLvm::IsVgExported, &StubIsVgExported0);
        m_CLvm.ImportVg_HLVM(vecPriPvName,strVgName,strMapInfo,vecWWN);
    }
}

TEST_F(CLvmTest,ExportVg_LLVM){
    CLvm m_CLvm;
    mp_string strVgName;
    
    {
        m_CLvm.ExportVg_LLVM(strVgName);
    }
    
    {
        Stub<CLvmSysExecType, StubCLvmSysExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCLvmSysExec);
        m_CLvm.ExportVg_LLVM(strVgName);
    }
    
    {
        Stub<CLvmSysExecType, StubCLvmSysExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCLvmSysExec);
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub2(&CLvm::IsVgExported, &StubIsVgExported);
        m_CLvm.ExportVg_LLVM(strVgName);
    }
    
    {
        Stub<CLvmSysExecType, StubCLvmSysExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCLvmSysExec);
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub2(&CLvm::IsVgExported, &StubIsVgExported0);
        m_CLvm.ExportVg_LLVM(strVgName);
    }
}

TEST_F(CLvmTest,ExportVg_HLVM){
    CLvm m_CLvm;
    mp_string strVgName;
    
    {
        m_CLvm.ExportVg_HLVM(strVgName);
    }
    
    {
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub2(&CLvm::IsVgExported, &StubIsVgExported);
        m_CLvm.ExportVg_HLVM(strVgName);
    }
    
    {
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub2(&CLvm::IsVgExported, &StubIsVgExported0);
        m_CLvm.ExportVg_HLVM(strVgName);
    }
}

TEST_F(CLvmTest,DeActivateVg_LLVM){
    CLvm m_CLvm;
    mp_string strVgName;
    
    {
        m_CLvm.DeActivateVg_LLVM(strVgName);
    }
    
    {
        Stub<CLvmSysExecType, StubCLvmSysExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCLvmSysExec);
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub2(&CLvm::IsVgExported, &StubIsVgExported);
        m_CLvm.DeActivateVg_LLVM(strVgName);
    }
    
    {
        Stub<CLvmSysExecType, StubCLvmSysExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCLvmSysExec);
        Stub<IsVgExportedType, StubIsVgExportedType, mp_void> mystub2(&CLvm::IsVgExported, &StubIsVgExported0);
        m_CLvm.DeActivateVg_LLVM(strVgName);
    }
}

TEST_F(CLvmTest,ImportVg_ALVM){
    CLvm m_CLvm;
    mp_string strVgName;
    vector<mp_string> vecWWN;
    
    vecWWN.push_back("test");
    
    {
        m_CLvm.ImportVg_ALVM(strVgName,vecWWN);
    }
    
    {
        Stub<CLvmGetDevNameByWWNType, StubCLvmGetDevNameByWWNType, mp_void> mystub2(&CDisk::GetDevNameByWWN, &StubCLvmGetDevNameByWWN);
        m_CLvm.ImportVg_ALVM(strVgName,vecWWN);
    }
}

TEST_F(CLvmTest,ActivateVg_HLVM){
    CLvm m_CLvm;
    mp_string strVgName;
    mp_string strVgActiveMode;
    mp_int32 iRecoverType;
    
    {
        iRecoverType = ALONE_TO_ALONE;
        m_CLvm.ActivateVg_HLVM(strVgName,strVgActiveMode,iRecoverType);
    }
    
    {
        iRecoverType = ALONE_TO_CLUSTER;
        m_CLvm.ActivateVg_HLVM(strVgName,strVgActiveMode,iRecoverType);
    }
    
    {
        iRecoverType = CLUSTER_TO_CLUSTER;
        m_CLvm.ActivateVg_HLVM(strVgName,strVgActiveMode,iRecoverType);
    }
    
    {
        iRecoverType = CLUSTER_TO_ALONE;
        m_CLvm.ActivateVg_HLVM(strVgName,strVgActiveMode,iRecoverType);
    }
}
