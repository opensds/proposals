/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/UtilsTest.h"

TEST_F(UtilsTest,DoSleep){
    mp_uint32 ms;

    DoSleep(ms);
}

TEST_F(UtilsTest,PackageLog){
    mp_string strLogName;
    PackageLog(strLogName);
}

TEST_F(UtilsTest,SignalRegister){
    mp_int32 signo;
    signal_proc func;
    SignalRegister(signo,func);
}

TEST_F(UtilsTest,DlibOpen){
    mp_char pszLibName;
    DlibOpen(&pszLibName);
}

TEST_F(UtilsTest,DlibOpenEx){
    mp_char pszLibName;
    mp_bool bLocal;
    
    DlibOpenEx(&pszLibName,bLocal);
}

TEST_F(UtilsTest,DlibClose){
    mp_handle_t hDlib;
    DlibClose(hDlib);
}

TEST_F(UtilsTest,DlibDlsym){
    mp_char pszFname;
    mp_handle_t hDlib;
    
//    DlibDlsym(hDlib,&pszFname);
}

TEST_F(UtilsTest,DlibError){
    mp_char szMsg;
    mp_uint32 isz;
    
    DlibError(&szMsg,isz);
}

TEST_F(UtilsTest,InitCommonModules){
    mp_char pszFullBinPath;
    
    InitCommonModules(&pszFullBinPath);
    
    Stub<CPathInitType, StubCPathInitType, mp_void> mystub1(&CPath::Init, &StubCPathInit);
    InitCommonModules(&pszFullBinPath);
    
    Stub<CConfigXmlParserInitType, StubCConfigXmlParserInitType, mp_void> mystub2(&CConfigXmlParser::Init, &StubCConfigXmlParserInit);
    InitCommonModules(&pszFullBinPath);
}

TEST_F(UtilsTest,GetHostName){
    mp_string strHostName;
    GetHostName(strHostName);
}

TEST_F(UtilsTest,GetOSError){
    GetOSError();
}
/*
TEST_F(UtilsTest,GetOSStrErr){
    mp_int32 err;
	mp_char buf;
	mp_size buf_len;
	
    GetOSStrErr(err,&buf,buf_len);
}*/

TEST_F(UtilsTest,GetCurrentUserName){
     mp_string strUserName;
     mp_ulong iErrCode;

     GetCurrentUserName(strUserName,iErrCode) ;
}

TEST_F(UtilsTest,ChownFile){
     mp_string strFileName;
     mp_int32 uid;
     mp_int32 gid; 

     ChownFile(strFileName,uid,gid);
}

TEST_F(UtilsTest,CheckParamString)
{
    mp_string paramValue;
    mp_int32 lenBeg;
    mp_int32 lenEnd;
    mp_string strEnd;
	mp_string strEnd1;

    CheckParamString(paramValue,lenBeg,lenEnd,strEnd,strEnd1);
	
	{
		paramValue = "12345";
		lenBeg = 1;
		lenEnd = 100;
		strEnd = "/";
		
		CheckParamString(paramValue,lenBeg,lenEnd,strEnd,strEnd1);
	}
}

TEST_F(UtilsTest,CheckParamString1)
{
    mp_string paramValue;
    mp_int32 lenBeg;
    mp_int32 lenEnd;
    mp_string strEnd;

    CheckParamString(paramValue,lenBeg,lenEnd,strEnd);
	
	{
		paramValue = "12345";
		lenBeg = 1;
		lenEnd = 100;
		strEnd = "/";
		
		CheckParamString(paramValue,lenBeg,lenEnd,strEnd);
	}
}

TEST_F(UtilsTest,CheckParamStringEnd)
{
    mp_string paramValue;
    mp_int32 lenBeg;
    mp_int32 lenEnd;
    mp_string strEnd;

    CheckParamStringEnd(paramValue,lenBeg,lenEnd,strEnd);
	
	{
		paramValue = "12345";
		lenBeg = 1;
		lenEnd = 100;
		strEnd = "/";
		
		CheckParamStringEnd(paramValue,lenBeg,lenEnd,strEnd);
	}
}

TEST_F(UtilsTest,CheckParamInteger32)
{
    mp_int32 paramValue;
    mp_int32 begValue;
    mp_int32 endValue;
    vector<mp_int32> vecExclude;
 
    CheckParamInteger32(paramValue,begValue,endValue,vecExclude);
	
	{
	    paramValue = 5;
		begValue = 1;
		endValue = 2;
		
		CheckParamInteger32(paramValue,begValue,endValue,vecExclude);
	}
	
		{
	    paramValue = 5;
		begValue = 1;
		endValue = 10;
		
		CheckParamInteger32(paramValue,begValue,endValue,vecExclude);
	}
}

TEST_F(UtilsTest,CheckParamInteger64)
{
    mp_int64 paramValue;
    mp_int64 begValue;
    mp_int64 endValue;
    vector<mp_int64> vecExclude;
 
    CheckParamInteger64(paramValue,begValue,endValue,vecExclude);
	
	{
	    paramValue = 5;
		begValue = 1;
		endValue = 2;
		
		CheckParamInteger64(paramValue,begValue,endValue,vecExclude);
	}
	
		{
	    paramValue = 5;
		begValue = 1;
		endValue = 10;
		
		CheckParamInteger64(paramValue,begValue,endValue,vecExclude);
	}
}

TEST_F(UtilsTest,CheckParamStringIsIP)
{
    mp_string paramValue;
 
    CheckParamStringIsIP(paramValue);
}

TEST_F(UtilsTest,CheckPathString)
{
    mp_string pathValue;
 
    CheckPathString(pathValue);
}

TEST_F(UtilsTest,CheckPathString1)
{
    mp_string pathValue = "//1234";
	mp_string strPre = "/";
 
    CheckPathString(pathValue,strPre);
}

TEST_F(UtilsTest,CheckFileSysMountParam)
{
    mp_string strDeviceName = "/\\1234";
	mp_string strMountPoint = "/\\1234";
	mp_int32 volumeType;
 
    CheckFileSysMountParam(strDeviceName,volumeType,strMountPoint);
	
	{
		
	}
}

TEST_F(UtilsTest,CheckFileSysFreezeParam)
{
    mp_string strDiskNames = "//1234";
 
    CheckFileSysFreezeParam(strDiskNames);
}
