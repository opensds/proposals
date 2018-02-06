/******************************************************************************
  文件名          : stub.h
  版本号          : 1.0
  作者            : 郑万刚
  生成日期        : 2015-09-14
  文件描述        : 动态打桩工具接口声明文件，支持SUSE10-x64平台
  其它            : 
                    本文件封装了stubInner.h中的函数（C++中请不要
				      使用stubInner.h）
					本stub工具不能完成对构造函数和析构函数的打桩
  功能说明 :
                    使用Stub类实现动态打桩
					在整个测试开始之前调用Stub::Init();
					在整个测试完成之后调用Stub::Destroy();				
******************************************************************************/
#ifndef __STUB_H__
#define __STUB_H__

#include "stubInner.h"
#include <stdio.h>

template <typename FunctionPointerTypeOld, typename FunctionPointerTypeNew, typename ObjectType>
class Stub
{
private:
    int idx;
public:
    Stub(FunctionPointerTypeOld pOldFunc, FunctionPointerTypeNew pNewFunc, ObjectType* pobj)
    {
        idx = setStub(pOldFunc, pNewFunc, pobj);

        if(idx < 0)
        {
            printf("Fail to set stub C!!!\n");
            throw "Fail to set stub C!!!";
        }
    }
    Stub(FunctionPointerTypeOld pOldFunc, FunctionPointerTypeNew pNewFunc){
       idx = setStub(pOldFunc, pNewFunc);

        if(idx < 0)
        {
            printf("Fail to set stub C!!!\n");
            throw "Fail to set stub C!!!";
        }
    }
    virtual ~Stub()
    {
        clearStub(idx);
    }
    static void Init(){
      if(::stubInit() !=0 )
        {
            printf("Fail to init stub!!!\n");
            throw "Fail to init stub!!!\n";
        }
    }
    static void Destroy(){
      stubFinal();
    }
};

#endif
