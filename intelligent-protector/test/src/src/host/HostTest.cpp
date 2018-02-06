/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "host/HostTest.h"

TEST_F(CHostTest,SetHostSN){
    vector<mp_string> vecMacs;
	
    CHost m_CHost;
    {
        Stub<WriteFileType, StubWriteFileType, mp_void> mystub1(&CIPCFile::WriteFile, &StubWriteFile);
        m_CHost.SetHostSN(vecMacs);
    }
}

TEST_F(CHostTest,ReadHostSNInfo){
    vector<mp_string> vecMacs;
    CHost m_CHost;
    {
        Stub<CHostTestReadFileType, StubCHostTestReadFileType, mp_void> mystub1(&CMpFile::ReadFile, &StubCHostTestReadFile);
        m_CHost.ReadHostSNInfo(vecMacs);
    }
    
    {
        Stub<CHostTestReadFileType, StubCHostTestReadFileType, mp_void> mystub1(&CMpFile::ReadFile, &StubCHostTestReadFilet);
        m_CHost.ReadHostSNInfo(vecMacs);
    }
}

TEST_F(CHostTest,GetHostSN){
    mp_string strSN;
    CHost m_CHost;
    {
        Stub<CHostTestReadFileType, StubCHostTestReadFileType, mp_void> mystub1(&CMpFile::ReadFile, &StubCHostTestReadFile);
        m_CHost.GetHostSN(strSN);
    }
    
    {
        Stub<CHostTestReadFileType, StubCHostTestReadFileType, mp_void> mystub1(&CMpFile::ReadFile, &StubCHostTestReadFilet);
        m_CHost.GetHostSN(strSN);
    }
}

TEST_F(CHostTest,GetAgentVersion){
    mp_string strAgentVersion;
    mp_string strBuildNum;
    CHost m_CHost;
    
    {
        strAgentVersion = "";
        strBuildNum = "";
        m_CHost.GetAgentVersion(strAgentVersion,strBuildNum);
    }
    
    {
        strAgentVersion = "test";
        strBuildNum = "test";
        m_CHost.GetAgentVersion(strAgentVersion,strBuildNum);
    }
    
}

TEST_F(CHostTest,GetInfo){
    host_info_t hostInfo;
    CHost m_CHost;
    
    {
        m_CHost.GetInfo(hostInfo);
    }
    
    {
        Stub<CHostTestReadFileType, StubCHostTestReadFileType, mp_void> mystub1(&CMpFile::ReadFile, &StubCHostTestReadFilet);
        m_CHost.GetInfo(hostInfo);
    }
    
    {
        Stub<CHostTestReadFileType, StubCHostTestReadFileType, mp_void> mystub1(&CMpFile::ReadFile, &StubCHostTestReadFilet);
        Stub<CHostExecSystemWithEchoType, StubCHostExecSystemWithEchoType, mp_void> mystub2(&CSystemExec::ExecSystemWithEcho, &StubCHostExecSystemWithEcho);
        m_CHost.GetInfo(hostInfo);
    }
    
    
}

TEST_F(CHostTest,GetDiskInfo){
    vector<host_lun_info_t> vecLunInfo;
    CHost m_CHost;
    
    {
        m_CHost.GetDiskInfo(vecLunInfo);
    }
    
    {
        Stub<CHostGetAllDiskNameType, StubCHostGetAllDiskNameType, mp_void> mystub1(&CDisk::GetAllDiskName, &StubCHostGetAllDiskName);
        m_CHost.GetDiskInfo(vecLunInfo);
    }
    
    {
        Stub<CHostGetAllDiskNameType, StubCHostGetAllDiskNameType, mp_void> mystub1(&CDisk::GetAllDiskName, &StubCHostGetAllDiskNamet);
        m_CHost.GetDiskInfo(vecLunInfo);
    }
    
    {
        Stub<CHostGetAllDiskNameType, StubCHostGetAllDiskNameType, mp_void> mystub1(&CDisk::GetAllDiskName, &StubCHostGetAllDiskNamet);
        Stub<CHostGetArrayVendorAndProductType, StubCHostGetArrayVendorAndProductType, mp_void> mystub2(&CArray::GetArrayVendorAndProduct, &StubCHostGetArrayVendorAndProduct);
        m_CHost.GetDiskInfo(vecLunInfo);
    }
    
    {
        Stub<CHostGetAllDiskNameType, StubCHostGetAllDiskNameType, mp_void> mystub1(&CDisk::GetAllDiskName, &StubCHostGetAllDiskNamet);
        Stub<CHostGetArrayVendorAndProductType, StubCHostGetArrayVendorAndProductType, mp_void> mystub2(&CArray::GetArrayVendorAndProduct, &StubCHostGetArrayVendorAndProductt);
        Stub<CHostGetArraySNType, StubCHostGetArraySNType, mp_void> mystub3(&CArray::GetArraySN, &StubCHostGetArraySN);
        m_CHost.GetDiskInfo(vecLunInfo);
    }
    
    {
        Stub<CHostGetAllDiskNameType, StubCHostGetAllDiskNameType, mp_void> mystub1(&CDisk::GetAllDiskName, &StubCHostGetAllDiskNamet);
        Stub<CHostGetArrayVendorAndProductType, StubCHostGetArrayVendorAndProductType, mp_void> mystub2(&CArray::GetArrayVendorAndProduct, &StubCHostGetArrayVendorAndProductt);
        Stub<CHostGetArraySNType, StubCHostGetArraySNType, mp_void> mystub3(&CArray::GetArraySN, &StubCHostGetArraySN);
        Stub<CHostGetLunInfoType, StubCHostGetLunInfoType, mp_void> mystub4(&CArray::GetLunInfo, &StubCHostGetLunInfo);
        m_CHost.GetDiskInfo(vecLunInfo);
    }
}

TEST_F(CHostTest,GetHostOS){
    mp_int32 iOSType;
    mp_string strOSVersion;
    CHost m_CHost;
    
    Stub<CHostExecSystemWithEchoType, StubCHostExecSystemWithEchoType, mp_void> mystub1(&CSystemExec::ExecSystemWithEcho, &StubCHostExecSystemWithEcho);
    m_CHost.GetHostOS(iOSType,strOSVersion);
}

TEST_F(CHostTest,GetTimeZone){
    timezone_info_t sttimezone;
    CHost m_CHost;
    
    m_CHost.GetTimeZone(sttimezone);
}

TEST_F(CHostTest,GetInitiators){
    initiator_info_t initInfo;
    CHost m_CHost;
    
    {
        m_CHost.GetInitiators(initInfo);
    }
    
    {
        Stub<CHostTestExecType, StubCHostTestExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCHostTestExecl);
        m_CHost.GetInitiators(initInfo);
    }
}

TEST_F(CHostTest,ScanDisk){
    CHost m_CHost;
    
    {
        m_CHost.ScanDisk();
    }
    
    {
        Stub<CHostTestExecType, StubCHostTestExecType, mp_void> mystub1(&CRootCaller::Exec, &StubCHostTestExec0);
        m_CHost.ScanDisk();
    }
}

TEST_F(CHostTest,RegTrapServer){
    trap_server stTrapServer;
    CHost m_CHost;
    
    {
        m_CHost.RegTrapServer(stTrapServer);
    }
}

TEST_F(CHostTest,UnRegTrapServer){
    trap_server stTrapServer;
    CHost m_CHost;
    
    {
        m_CHost.UnRegTrapServer(stTrapServer);
    }
}

TEST_F(CHostTest,VerifySnmp){
    snmp_v3_param stParam;
    CHost m_CHost;
    
    stParam.strAuthPassword = "test";
    stParam.strPrivPassword = "test";
    stParam.strSecurityName = "test";
    stParam.iAuthProtocol = 1;
    stParam.iPrivProtocol = 1;
    {
        Stub<GetSnmpV3ParamType, StubGetSnmpV3ParamType, mp_void> mystub1(&CAlarmConfig::GetSnmpV3Param, &StubGetSnmpV3Param0);
        m_CHost.VerifySnmp(stParam);
    }
    
    {
        Stub<GetSnmpV3ParamType, StubGetSnmpV3ParamType, mp_void> mystub1(&CAlarmConfig::GetSnmpV3Param, &StubGetSnmpV3Param);
        m_CHost.VerifySnmp(stParam);
    }
}

TEST_F(CHostTest,QueryThirdPartyScripts){
    vector<mp_string> vectFileList;
    CHost m_CHost;
    
    {
        m_CHost.QueryThirdPartyScripts(vectFileList);
    }
}

TEST_F(CHostTest,ExecThirdPartyScript){
    mp_string fileName;
    mp_string paramValues;
    vector<mp_string> vecResult;
    CHost m_CHost;
    
    {
        m_CHost.ExecThirdPartyScript(fileName,paramValues,vecResult);
    }
    
    {
        fileName = "test";
        m_CHost.ExecThirdPartyScript(fileName,paramValues,vecResult);
    }
}

TEST_F(CHostTest,LogCollectThread){
    CLogCollector m_CLogCollector;
    CLogCollector param;
    
    {
        Stub<GetLogNameType, StubGetLogNameType, mp_void> mystub2(&CLogCollector::GetLogName, &StubGetLogName);
        Stub<PackageLogType, StubPackageLogType, mp_void> mystub1(&PackageLog, &StubPackageLog);
        m_CLogCollector.LogCollectThread(&param);
    }
}

TEST_F(CHostTest,CollectLog){
	CHost m_CHost;
	
    //m_CHost.CollectLog();
}
