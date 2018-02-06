/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _CONFIGXMLPARSETEST_H_
#define _CONFIGXMLPARSETEST_H_

#define private public

#include "common/ConfigXmlParse.h"
#include "common/Types.h"
#include "common/File.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CConfigXmlParserTest: public testing::Test{
protected:
    static mp_void SetUpTestCase(){
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};

Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CConfigXmlParserTest::m_stub;

//******************************************************************************
typedef TiXmlElement* (CConfigXmlParser::*GetChildElementType)(TiXmlElement* pParentElement, mp_string strSection);
typedef TiXmlElement* (*StubGetChildElementType)(TiXmlElement* pParentElement, mp_string strSection);
//
typedef TiXmlElement* (TiXmlElement::*FirstChildElementType)();
typedef TiXmlElement* (*StubFirstChildElementType)();
//
typedef TiXmlAttribute* (TiXmlElement::*FirstAttributeType)();
typedef TiXmlAttribute* (*StubFirstAttributeType)();

typedef mp_int32 (CConfigXmlParser::*GetValueStringType)(mp_string strSection, mp_string strKey, mp_string& strValue);
typedef mp_int32 (*StubGetValueStringType)(mp_string strSection, mp_string strKey, mp_string& strValue);

typedef mp_int32 (CConfigXmlParser::*GetValueString1Type)(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_string& strValue);
typedef mp_int32 (*StubGetValueString1Type)(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_string& strValue);

typedef mp_int32 (*GetlLastModifyTimeType)(const mp_char* pszFilePath, mp_time& tLastModifyTime);
typedef mp_int32 (*StubGetlLastModifyTimeType)(const mp_char* pszFilePath, mp_time& tLastModifyTime);

typedef TiXmlElement* (TiXmlDocument::*RootElementType)();
typedef TiXmlElement* (*StubRootElementType)(mp_void* pthis);

typedef mp_bool (*CConfigXmlParserFileExistType)(const mp_char* pszFilePath);
typedef mp_bool (*StubCConfigXmlParserFileExistType)(const mp_char* pszFilePath);
//*******************************************************************************
mp_bool StubCConfigXmlParserFileExist(const mp_char* pszFilePath){
    return -1;
}

TiXmlElement* StubRootElement(mp_void* pthis){
    mp_string test = "test";
    return new TiXmlElement(test.c_str());
}

TiXmlElement* StubGetChildElement(TiXmlElement* pParentElement, mp_string strSection){
    mp_string test = "test";
    return new TiXmlElement(test.c_str());
}

TiXmlElement* StubFirstChildElement(){
    mp_string test = "test";
    return new TiXmlElement(test.c_str());
}

TiXmlAttribute* StubFirstAttribute(){
    TiXmlAttribute* test;
    return test;
}

mp_int32 StubGetlLastModifyTime(const mp_char* pszFilePath, mp_time& tLastModifyTime){
    return 1;
}

mp_int32 StubGetValueString(mp_string strSection, mp_string strKey, mp_string& strValue){
    return 0;
}

mp_int32 StubGetValueString1(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_string& strValue){
    return 0;
}

mp_void StubCLoggerLogVoid(mp_void* pthis){
    return;
}

#endif
