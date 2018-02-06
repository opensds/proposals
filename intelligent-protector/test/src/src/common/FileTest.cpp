/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/FileTest.h"
#include "common/Log.h"
#include "common/Path.h"
#include "stub.h"
#include "common/Types.h"
#include "gtest/gtest.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <cstdlib>
#include <vector>
using namespace std;

typedef mp_void (*StubLogType)(void);
typedef mp_void (CLogger::*LogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
    
static mp_void StubCLoggerLog(void){
    return;
}
static mp_bool StubCMpFileFileExist(const mp_char* pszFilePath){
  return MP_TRUE;
}

static mp_void ReadFileTest(vector<mp_string> & expect){
  try{
    mp_string filePath = "cmpfilereadfiletest.txt";
    
    FILE *fp = fopen(filePath.c_str(), "w");
    if(NULL == fp){
      fprintf(stderr, "error: %d %s", __LINE__, __FUNCTION__); 
      exit(0); 
    }
    for(vector<mp_string>::iterator it = expect.begin(); it != expect.end(); it++){
      fprintf(fp, "%s\n", it->c_str());
    }
    fclose(fp);
    fp = NULL;
    
    vector<mp_string> out;
    mp_int32 ret = CMpFile::ReadFile(filePath, out);
    EXPECT_EQ(MP_SUCCESS, ret);
    
    remove(filePath.c_str());
    
    vector<mp_string>::iterator itExpect, itRet;
    for(itExpect = expect.begin(), itRet = out.begin(); itExpect != expect.end() && itRet != out.end(); itExpect++, itRet++){
      EXPECT_TRUE(*itExpect == *itRet) << "itExpect: " <<  *itExpect << "\nitRet: " << *itRet << "\nFILE:" 
                << __FILE__ << " LINE: " << __LINE__;
    }
  }catch(...){
    printf("Error on %s file %d line.\n", __FILE__, __LINE__);
    exit(0);
  }
}

static mp_bool StubFileExistFalse(void){
  return false;
}

static mp_bool StubFileExistTrue(void){
  return true;
}

static FILE * StubFopenNull(void){
  return NULL;
}

TEST_F(CMpFileTest, CMpFileTestOpenFileTest){
  mp_int32 iRet = CMpFile::OpenFile();

  EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CMpFileTest, CMpFileFileExistTest){
  mp_string path = "/bin";
  EXPECT_EQ(MP_FALSE, CMpFile::FileExist(path.c_str()));

  mp_string lsPath = path + "/ls"; 
  EXPECT_EQ(MP_TRUE, CMpFile::FileExist(lsPath.c_str()));

  mp_string pathNotExist = path + "/askjcsaidhwsdhmxzcnsakjdhsa";
  EXPECT_EQ(MP_FALSE, CMpFile::FileExist(pathNotExist.c_str()));
}

TEST_F(CMpFileTest, CMpFileDirExistTest){
  mp_string path = "/bin";
  EXPECT_EQ(MP_TRUE, CMpFile::DirExist(path.c_str()));

  mp_string runPath = path + "/ls";
  EXPECT_EQ(MP_FALSE, CMpFile::DirExist(runPath.c_str()));

  mp_string pathNotExist = path + "/askjcsaidhwsdhmxzcnsakjdhsa";
  EXPECT_EQ(MP_FALSE, CMpFile::DirExist(pathNotExist.c_str()));
}

TEST_F(CMpFileTest, CMpFileTestCreateDirTest){
  mp_int32 iRet = CMpFile::CreateDir(NULL);
  EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CMpFileTest, CMpFileFileSizeTest){
    const char *fileName = "cpmfiletest_zwg_197412356478.txt";
    FILE *fp = fopen(fileName, "w");
    if(NULL == fp){
        fprintf(stderr, "can not open %s in %s in %d line.\n", fileName, __FILE__, __LINE__);
        exit(0);
    }
    char str[30] = "abcde";
    if(5 != fwrite(str, 1, 5, fp)){
      fprintf(stderr, "fwrite failed in %s in %d line.\n", __FILE__, __LINE__);
      fclose(fp);
      exit(0);
    }

    fclose(fp);
    fp = NULL;
    
    mp_uint32 uiSize;

    EXPECT_EQ(MP_SUCCESS, CMpFile::FileSize(fileName, uiSize));
    EXPECT_EQ(5, uiSize);
    if(0 != remove(fileName)){
        fprintf(stderr, "can not remove %s on %s in %d line!\n", fileName, __FILE__, __LINE__);
        exit(0);
    }

    EXPECT_EQ(MP_FAILED, CMpFile::FileSize(fileName, uiSize));

    uiSize = 0;
    EXPECT_EQ(MP_FAILED, CMpFile::FileSize(NULL, uiSize));
}

TEST_F(CMpFileTest, CMpFileGetLastModifyTimeTest){
  mp_time tLastModifyTime;
  mp_string path;
  mp_int32 iRet = CMpFile::GetlLastModifyTime(NULL, tLastModifyTime);
  EXPECT_EQ(iRet, MP_FAILED);

  path = "wqgdiuqwgQQWQQEDvjsegdwq123";
  iRet = CMpFile::GetlLastModifyTime(path.c_str(), tLastModifyTime);
  EXPECT_EQ(iRet, MP_FAILED);

  path = "/bin/ls";
  iRet = CMpFile::GetlLastModifyTime(path.c_str(), tLastModifyTime);
  EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CMpFileTest, CMpFileReadFileTest){
  Stub<LogType,StubLogType, void> stubLog(&CLogger::Log, &StubCLoggerLog);
  vector<mp_string> vec;
  //正常文件读写
  vec.push_back("hello!");
  vec.push_back("你好!");
  vec.push_back("This is a test file!");
  ReadFileTest(vec);
  vec.clear();

  //空行
  vec.push_back("");
  ReadFileTest(vec);

  //文件不存在
  do{
    mp_string strFilePath="dwigdiehdiewgfiewihdwqiugei";
    typedef mp_bool (*StubTypeFileExist)(void);
    typedef mp_bool (*TypeFileExist)(const mp_char*);
    Stub<TypeFileExist, StubTypeFileExist, void> stubFileExist(&CMpFile::FileExist, StubFileExistFalse);
    mp_int32 ret = CMpFile::ReadFile(strFilePath,  vec);
    EXPECT_EQ(ret, MP_FAILED);
  }while(0);

  //fopen返返回NULL
  do{
    mp_string strFilePath="dwigdiehdiewgfiewihdwqiugei";
    typedef FILE* (*StubFopen)(void);
    typedef FILE* (*TypeFopen)(const char*, const char*);
    Stub<TypeFopen, StubFopen, void> stub(fopen, StubFopenNull);

    typedef mp_bool (*StubTypeFileExist)(void);
    typedef mp_bool (*TypeFileExist)(const mp_char*);
    Stub<TypeFileExist, StubTypeFileExist, void> stubFileExist(&CMpFile::FileExist, StubFileExistFalse);
    mp_int32 ret = CMpFile::ReadFile(strFilePath,  vec);
    EXPECT_EQ(ret, MP_FAILED);
  }while(0);
}

TEST_F(CMpFileTest, CMpFileGetFolderFileTest){
  mp_string dirName = "getfolderfiletest_zwg_14652542";
  if(mkdir(dirName.c_str(), 0755) < 0){
      fprintf(stderr, "mkdir %s on %s in %d line!\n", dirName.c_str(), __FILE__, __LINE__);
      exit(0);
  }

  FILE *fp1 = NULL;
  FILE *fp2 = NULL;
  mp_string fName1 = dirName+"/test1sagdgqwi.txt";
  mp_string fName2 = dirName+"/test2sagdgqwi.txt";
  fp1 = fopen(fName1.c_str(), "w");
  if(fp1 == NULL){
    rmdir(dirName.c_str());
    fprintf(stderr, "open file error on %s file %d line!\n", __FILE__, __LINE__);
    exit(0);
  }
  fp2 = fopen(fName2.c_str(), "w");
  if(fp2 == NULL){
      fclose(fp1);
      remove(fName1.c_str());
      rmdir(dirName.c_str());
      fprintf(stderr, "open file error on %s file %d line!\n", __FILE__, __LINE__);
      exit(0);
  }
  vector<mp_string> expectList;
  expectList.push_back(fName1);
  expectList.push_back(fName2);
  
  sort(expectList.begin(), expectList.end());

  vector<mp_string> fileList;

  EXPECT_EQ(MP_SUCCESS, CMpFile::GetFolderFile(dirName, fileList));
  ASSERT_EQ(expectList.size(), fileList.size());
  sort(fileList.begin(), fileList.end());
  
  for(vector<mp_string>::iterator iter1 = fileList.begin(), iter2 = expectList.begin();
        iter1 != fileList.end(), iter2 != expectList.end();
        iter1++, iter2++){
      EXPECT_EQ(dirName +"/"+ *iter1, *iter2);
  }

   fclose(fp1);
   fclose(fp2);
   remove(fName1.c_str());
   remove(fName2.c_str());
   rmdir(dirName.c_str());
}

//iSleepCount < 0
static void WaitForFileTest1(void){
  mp_bool iRet = CMpFile::WaitForFile(NULL, -1, -1);
  EXPECT_EQ(iRet, MP_FALSE);
}

//FileExist() return false;
static void WaitForFileTest2(void){
  typedef mp_bool (*TypeFileExist)(const mp_char*);
  typedef mp_bool (*StubTypeFileExist)(void);
  Stub<TypeFileExist, StubTypeFileExist, void> stubFileExist(&CMpFile::FileExist, StubFileExistFalse);
  Stub<LogType, StubLogType, void> stubLog(&CLogger::Log, StubCLoggerLog);
  mp_bool iRet = CMpFile::WaitForFile(NULL, 1, 1);
  EXPECT_EQ(MP_FALSE, iRet);
}

//FileExist() return true
static void WaitForFileTest3(void){
  typedef mp_bool (*TypeFileExist)(const mp_char*);
  typedef mp_bool (*StubTypeFileExist)(void);
  Stub<TypeFileExist, StubTypeFileExist, void> stubFileExist(&CMpFile::FileExist, StubFileExistTrue);
  Stub<LogType, StubLogType, void> stubLog(&CLogger::Log, StubCLoggerLog);
  mp_bool iRet = CMpFile::WaitForFile(NULL, 1, 1);
  EXPECT_EQ(MP_TRUE, iRet);
}

TEST_F(CMpFileTest, WaitForFileTest){
  WaitForFileTest1();
  WaitForFileTest2();
  WaitForFileTest3();
}


TEST_F(CMpFileTest, DelFileTest){

  Stub<LogType, StubLogType, void>  stubLog(&CLogger::Log, StubCLoggerLog);
  //入参为空
  mp_int32 iRet = CMpFile::DelFile(NULL);
  EXPECT_EQ(MP_SUCCESS, iRet);

  //文件不存在
  iRet = CMpFile::DelFile("xwsxhhoichwocie");
  EXPECT_EQ(MP_SUCCESS, iRet);

  //文件存在，但是无法删除
  //iRet = CMpFile::DelFile("/bin/ls");
  //EXPECT_EQ(MP_FAILED, iRet);

  //文件存在， 能够删除
  FILE *fp = fopen("testdel.txt", "w");
  if(NULL == fp){
    fprintf(stderr, "can not open file on %s file %d line\n", __FILE__, __LINE__);
    exit(0);
  }
  fclose(fp);
  fp = NULL;
  iRet = CMpFile::DelFile("testdel.txt");
  EXPECT_EQ(MP_SUCCESS, iRet);
}


//临时文件不存在，且正常的情况下的CIPCFile::ReadFile 和WriteFile的测试
static void CIPCReadWriteFileTest1(void){
  Stub<LogType, StubLogType, void> stubLog(&CLogger::Log, StubCLoggerLog);
  
  mp_string path = "/tmp/CIPCReadWriteFileTest.txt";
  vector<mp_string> in, out;
  in.push_back("sqwvceqv");
  in.push_back("dwqgdwqigwq");
  mp_int32 iRet = CIPCFile::WriteFile(path, in);
  EXPECT_EQ(MP_SUCCESS, iRet);

  iRet = CIPCFile::ReadFile(path, out);
  EXPECT_EQ(MP_SUCCESS, iRet);

  EXPECT_EQ(in.size(), out.size());

  vector<mp_string>::iterator InIter, OutIter;
  for(InIter = in.begin(), OutIter = out.begin(); InIter != in.end() && OutIter != out.end(); InIter++, OutIter++){
    EXPECT_EQ(*InIter, *OutIter);
  }
}

//临时文件存在的情况
static void CIPCReadWriteFileTest2(void){
  mp_string path = "/tmp/CIPCReadWriteFileTest.txt";
  FILE* fp = fopen(path.c_str(), "w");
  if(NULL == fp){
    fprintf(stderr, "open file error in %s file in %d line\n", __FILE__, __LINE__);
    exit(0);
  }
  fclose(fp);
  CIPCReadWriteFileTest1();
  remove("/tmp/CIPCReadWriteFileTest.txt");
}

//临时文件不存在时读取临时文件
static void CIPCReadWriteFileTest3(void){
  Stub<LogType, StubLogType, void> stubLog(&CLogger::Log, StubCLoggerLog);
  mp_string path = "/tmp/CIPCReadWriteFileTest.txt";
  vector<mp_string> out;
  mp_int32 iRet = CIPCFile::ReadFile(path, out);
  EXPECT_EQ(iRet, MP_FAILED);
}

//要读取的文件存在，但是打开失败
static void CIPCReadWriteFileTest4(void){

  mp_string path = "/tmp/CIPCReadWriteFileTest.txt";
  FILE* fp = fopen(path.c_str(), "w");
  if(NULL == fp){
    fprintf(stderr, "open file error in %s file in %d line\n", __FILE__, __LINE__);
    exit(0);
  }
  fclose(fp);

  vector<mp_string> out;
  
  typedef FILE* (*StubFopen)(void);
  typedef FILE* (*TypeFopen)(const char*, const char*);
  Stub<TypeFopen, StubFopen, void> stub(fopen, StubFopenNull);
  Stub<LogType, StubLogType, void> stubLog(&CLogger::Log, StubCLoggerLog);
  int iRet = CIPCFile::WriteFile(path, out);
  EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CIPCFileTest, CIPCReadWriteFileTest){
  CIPCReadWriteFileTest1();
  CIPCReadWriteFileTest2();
  CIPCReadWriteFileTest3();
  CIPCReadWriteFileTest4();
}

static mp_string StubCPathGetTmpFilePath(void *This, mp_string strFileName){
  mp_string path = "/tmp";
  return path + "/" + strFileName;
}

TEST_F(CIPCFileTest, CIPCWriteReadInputTest){
  Stub<LogType, StubLogType, void> stubLog(&CLogger::Log, StubCLoggerLog);
  typedef mp_string (CPath::*TypeCPathGetTmpFilePath)(mp_string);
  typedef mp_string (*TypeStubCPathGetTmpFilePath)(void*, mp_string);
  Stub<TypeCPathGetTmpFilePath, TypeStubCPathGetTmpFilePath, void> stubPath(&CPath::GetTmpFilePath, StubCPathGetTmpFilePath);

  mp_string input = "nihao !hello,world@!";
  mp_string strUniqueID = "swqgsiuw";

  mp_int32 iRet = CIPCFile::WriteInput(strUniqueID, input);

  EXPECT_EQ(iRet, MP_SUCCESS);


  mp_string out;
  iRet = CIPCFile::ReadInput(strUniqueID, out);
  EXPECT_EQ(iRet, MP_SUCCESS);

  EXPECT_EQ(input, out);
}

TEST_F(CIPCFileTest, CIPCWriteReadResultTest){
  Stub<LogType, StubLogType, void> stubLog(&CLogger::Log, StubCLoggerLog);
  typedef mp_string (CPath::*TypeCPathGetTmpFilePath)(mp_string);
  typedef mp_string (*TypeStubCPathGetTmpFilePath)(void*, mp_string);
  Stub<TypeCPathGetTmpFilePath, TypeStubCPathGetTmpFilePath, void> stubPath(&CPath::GetTmpFilePath, StubCPathGetTmpFilePath);

  vector<mp_string> input;
  input.push_back("nihao !");
  input.push_back("hello,world@!");
  mp_string strUniqueID = "swqgsiuw";

  mp_int32 iRet = CIPCFile::WriteResult(strUniqueID, input);

  EXPECT_EQ(0, MP_SUCCESS);


  vector<mp_string> out;
  iRet = CIPCFile::ReadResult(strUniqueID, out);
  EXPECT_EQ(iRet, MP_SUCCESS);

  EXPECT_EQ(input.size(), out.size());
  EXPECT_EQ(input.size(), 2);

  EXPECT_EQ(input[1], out[1]);
  EXPECT_EQ(input[0], out[0]);
}

