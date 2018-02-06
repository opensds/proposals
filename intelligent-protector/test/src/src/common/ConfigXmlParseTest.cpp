/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/ConfigXmlParseTest.h"

TEST_F(CConfigXmlParserTest, Init){
    mp_int32 ret = 0;
    mp_string path = "test";
    
    Stub<CConfigXmlParserFileExistType, StubCConfigXmlParserFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCConfigXmlParserFileExist);
    ret = CConfigXmlParser::GetInstance().Init(path);
}

TEST_F(CConfigXmlParserTest, GetChildElement){
    mp_string path = "test";
    TiXmlElement* ptr = NULL;

    CConfigXmlParser::GetInstance().GetChildElement(ptr,path);
}

TEST_F(CConfigXmlParserTest, ParseNodeValue){
    mp_int32 ret = 0;
    TiXmlElement* ptr = NULL;
    mp_string test = "test";
    TiXmlElement* strSection = new TiXmlElement(test.c_str());
    NodeValue nodeValue;
    
    Stub<FirstChildElementType, StubFirstChildElementType, mp_void> mystub2(&TiXmlElement::FirstChildElement, &StubFirstChildElement);
    CConfigXmlParser::GetInstance().ParseNodeValue(strSection,nodeValue);
    CConfigXmlParser::GetInstance().ParseNodeValue(ptr,nodeValue);
}

TEST_F(CConfigXmlParserTest,IsModified){
    mp_int32 ret = 0;
    
    CConfigXmlParser::GetInstance().IsModified();
    
    Stub<GetlLastModifyTimeType, StubGetlLastModifyTimeType, mp_void> mystub2(&CMpFile::GetlLastModifyTime, &StubGetlLastModifyTime);
    CConfigXmlParser::GetInstance().IsModified();
    
    CConfigXmlParser::GetInstance().m_strCfgFilePath = "";
    ret = CConfigXmlParser::GetInstance().IsModified();
}

TEST_F(CConfigXmlParserTest, GetValueString){
    mp_int32 ret = 0;
    time_t timett;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_string strValue = "test";
    
    Stub<RootElementType, StubRootElementType, mp_void> mystub1(&TiXmlDocument::RootElement, &StubRootElement);
    ret = CConfigXmlParser::GetInstance().GetValueString(strSection,strKey,strValue);
    ret = CConfigXmlParser::GetInstance().GetValueString(strSection,strSection1,strKey,strValue);
    
    Stub<GetChildElementType, StubGetChildElementType, mp_void> mystub2(&CConfigXmlParser::GetChildElement, &StubGetChildElement);
    ret = CConfigXmlParser::GetInstance().GetValueString(strSection,strKey,strValue);
    ret = CConfigXmlParser::GetInstance().GetValueString(strSection,strSection1,strKey,strValue);
    
    CConfigXmlParser::GetInstance().m_lastTime = timett;
    CConfigXmlParser::GetInstance().GetValueString(strSection,strKey,strValue);
}

TEST_F(CConfigXmlParserTest, GetValueBool){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_bool strValue = 0;
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubGetValueString);
        Stub<GetValueString1Type, StubGetValueString1Type, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubGetValueString1);
        ret = CConfigXmlParser::GetInstance().GetValueBool(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().GetValueBool(strSection,strSection1,strKey,strValue);
    }
}

TEST_F(CConfigXmlParserTest, GetValueInt32){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_int32 strValue = 12;
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubGetValueString);
        Stub<GetValueString1Type, StubGetValueString1Type, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubGetValueString1);
        ret = CConfigXmlParser::GetInstance().GetValueInt32(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().GetValueInt32(strSection,strSection1,strKey,strValue);
    }
}

TEST_F(CConfigXmlParserTest, GetValueInt64){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_int64 strValue = 12;
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubGetValueString);
        Stub<GetValueString1Type, StubGetValueString1Type, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubGetValueString1);
        ret = CConfigXmlParser::GetInstance().GetValueInt64(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().GetValueInt64(strSection,strSection1,strKey,strValue);
    }
}

TEST_F(CConfigXmlParserTest, GetValueFloat){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_float strValue = 12.1;
    
    {
        Stub<GetValueStringType, StubGetValueStringType, mp_void> mystub1(&CConfigXmlParser::GetValueString, &StubGetValueString);
        Stub<GetValueString1Type, StubGetValueString1Type, mp_void> mystub2(&CConfigXmlParser::GetValueString, &StubGetValueString1);
        ret = CConfigXmlParser::GetInstance().GetValueFloat(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().GetValueFloat(strSection,strSection1,strKey,strValue);
    }
}

TEST_F(CConfigXmlParserTest, SetValue){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_string strValue = "test";
    {
        ret = CConfigXmlParser::GetInstance().SetValue(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().SetValue(strSection,strSection1,strKey,strValue);
    }

    {
        Stub<CConfigXmlParserFileExistType, StubCConfigXmlParserFileExistType, mp_void> mystub1(&CMpFile::FileExist, &StubCConfigXmlParserFileExist);
        Stub<RootElementType, StubRootElementType, mp_void> mystub2(&TiXmlDocument::RootElement, &StubRootElement);
        Stub<GetChildElementType, StubGetChildElementType, mp_void> mystub3(&CConfigXmlParser::GetChildElement, &StubGetChildElement);
        Stub<FirstChildElementType, StubFirstChildElementType, mp_void> mystub4(&TiXmlElement::FirstChildElement, &StubFirstChildElement);
        ret = CConfigXmlParser::GetInstance().SetValue(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().SetValue(strSection,strSection1,strKey,strValue);
    }
}
