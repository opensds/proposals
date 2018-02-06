/******************************************************************************
版权所有 (C), 2006-2020, 华为技术有限公司

  文件名          : stub.cpp
  版本号          : 2.0
  作者            : 王永峰(63774)
  生成日期        : 2009-04-01
  文件描述        : Linux版动态打桩工具的实现文件
  其它            : 功能说明：
                    主要用于单元测试打桩，实现用全局函数对全局函数和类的成员函数（包括虚函数和静态函数）打桩
******************************************************************************/

#include <unistd.h>
#include <errno.h>
#include <map>
#include <string.h>
#include "stubInner.h"
#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>

const int WORD_SIZE   = sizeof(void *);
const int ADDR_SIZE   = sizeof(void *);
const int DADDR_SIZE  = sizeof(void *) + sizeof(void *);
const int RESULT_SIZE = sizeof(int);
const int IDX_SIZE    = sizeof(int);
const int CTL_SIZE    = sizeof(char);

#if (defined _SUSE9)
const int MAC_SIZE         = 8;
char  g_pMac[MAC_SIZE + 1] = "\xc7\xc0\x0\x0\x0\x0\xff\xe0";
const int N_OFFSET         = 2; 

#elif (defined _SUSE10_64)
const int MAC_SIZE         = 12;
char  g_pMac[MAC_SIZE + 1] = "\x48\xb8\x0\x0\x0\x0\x0\x0\x0\x0\xff\xe0";
const int N_OFFSET         = 2; 

#elif (defined _SUSE10_32)
const int MAC_SIZE         = 7;
char  g_pMac[MAC_SIZE + 1] = "\xb8\x0\x0\x0\x0\xff\xe0";
const int N_OFFSET         = 1; 

#else
#error "This platform is not supported"

#endif

static bool g_bIsValid = false;
FILE *g_pf = NULL;

struct tagStubInfo
{
    tagStubInfo(){}
    tagStubInfo(void** pMac, void* pOldFunc)
    {
        m_lMac[0]= pMac[0];
        m_lMac[1]= pMac[1];
        m_pOldFunc = pOldFunc;
    }
    void* m_pOldFunc;
    void* m_lMac[2];
};

static std::map<int, tagStubInfo> g_mStub;

static void write_file()
{
    char p[] = "secret";
    fwrite(p, 6, 1, g_pf);
    fflush(g_pf);    
}

static int doStub(void* pOldFunc, void* pNewFunc)
{
    write_file();
    void* lMac[2];

    memcpy(&lMac[0], pOldFunc, WORD_SIZE);
    memcpy(&lMac[1], (void*)((long)pOldFunc + WORD_SIZE), WORD_SIZE);
    
    tagStubInfo oriInfo(lMac, pOldFunc);
    
    memcpy(lMac, g_pMac, MAC_SIZE);
    memcpy(((char *)lMac) + N_OFFSET, &pNewFunc, ADDR_SIZE);
    
    memcpy(pOldFunc, &lMac[0], WORD_SIZE);
    memcpy((void*)((long)pOldFunc + WORD_SIZE), &lMac[1], WORD_SIZE);
    
    static int iIdx = 0;
    if(++iIdx == INT_MAX)
    {
        iIdx = 1;
    }
    g_mStub.insert(std::pair<int, tagStubInfo>(iIdx, oriInfo));
    return iIdx;
}

static int doClearStub(int iIdx)
{
    if(0 == g_mStub.count(iIdx))
    {
        return -1;
    }
    tagStubInfo info = g_mStub[iIdx];
    write_file();
    memcpy(info.m_pOldFunc, &info.m_lMac[0], WORD_SIZE);
    memcpy((void*)((long)(info.m_pOldFunc) + WORD_SIZE), &info.m_lMac[1], WORD_SIZE);
    return 0;
}

static int doClearAllStub()
{
    int c = 0;
    for(std::map<int, tagStubInfo>::iterator it = g_mStub.begin();
    it != g_mStub.end(); ++it)
    {
        if(0 == doClearStub(it->first))
        {
            c++;
        }
    }
    g_mStub.clear();
    return c;
}

static const char *get_name()
{
    static char name[20];
    int c = 0;
    name[c++] = '/';
    name[c++] = 'd';
    name[c++] = 'e';
    name[c++] = 'v';
    name[c++] = '/';
    name[c++] = 'd';
    name[c++] = 'm';
    name[c++] = 'e';
    name[c++] = 'm';
    name[c++] = 0;
    return name;
}

int stubInit()
{
    if(g_bIsValid)
    {
        return 0;
    }
    
    g_pf = fopen(get_name(), "w");
    if(!g_pf)
    {
        return -1;
    }
    g_bIsValid = true;
    return 0;
}

void stubFinal()
{
    if(!g_bIsValid)
    {
        return;
    }
    fclose(g_pf);
    return;
}

int setStubC(void *pOldFunc, void *pNewFunc)
{
	return _setStub(pOldFunc, pNewFunc, NULL);
}

static void* findVirtualFuncAddr(long pOldFunc, const char *pObject)
{
	//虚表的地址就存放在对象的前sizeof(unsigned long)字节中
    const unsigned long *pVirtualTableAddr = (const unsigned long*)*((const unsigned long*)pObject);
	//pOldFunc实际上是一个序号，在SUSE10_X64系统中，
	//第一个虚函数的序号为1，第二个虚函数的序号为9，第三个虚函数的序号为17
	long vtabIndex = (pOldFunc-1)/sizeof(char*);//当前要获取的函数在virtual table中的索引号
    if (pVirtualTableAddr != NULL)
    {
        void *funcAddr = (void *)pVirtualTableAddr[vtabIndex];
        return funcAddr;
    }
    printf("meet some error on %s file %d line\n", __FILE__, __LINE__);
    return NULL;
}

int _setStub(void *pOldFunc, void *pNewFunc, void *pObject)
{
    if(!g_bIsValid)
    {
        return -2;
    }
    void *pRealNewFunc = pNewFunc;
    void *pRealOldFunc = NULL;
    if(NULL == pObject)
    {
        pRealOldFunc = pOldFunc;
    }
    else
    {
        pRealOldFunc = findVirtualFuncAddr((long)pOldFunc, (const char*)pObject);
        if (pRealOldFunc == NULL)
        {
            return -1;
        }
    }
    return doStub(pRealOldFunc, pRealNewFunc);
}

int clearStub(int iIdx)
{
    if(!g_bIsValid)
    {
        return -2;
    }
    return doClearStub(iIdx);
}

int clearAllStub()
{
    if(!g_bIsValid)
    {
        return -2;
    }
    return doClearAllStub();
}
