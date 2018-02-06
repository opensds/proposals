/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/ServicePluginTest.h"

class abc
{
};

class CPlugx: public CServicePlugin
{
    public:
    virtual mp_int32 DoAction(CRequestMsg* req, CResponseMsg* rsp) { return MP_SUCCESS; }
};

TEST_F(CRestActionMapTest, Add)
{
    CRestActionMap<abc> restObj;
    restObj.Add("abc", "123", NULL);
}


TEST_F(CRestActionMapTest, GetAction)
{
    CRestActionMap<abc> restObj;
    CRestActionMap<abc>::rest_action_t act();
    //restObj.GetAction(mp_string("abc"), mp_string("123"), &act);
}


TEST_F(CRestActionMapTest, PrintMap)
{
    CRestActionMap<abc> restObj;
    restObj.PrintMap();
}


TEST_F(CServicePluginTest, xfunc)
{
    CRequestMsg req;
    CResponseMsg rsp;
    CPlugx plugxObj;
    
    plugxObj.GetTypeId();
    plugxObj.Initialize(NULL);
    plugxObj.Destroy();
    plugxObj.SetOption(NULL, NULL);
    plugxObj.GetOption(NULL, NULL, 1);
    plugxObj.CreateObject(NULL);
    plugxObj.GetClasses(NULL, 1);
    plugxObj.GetName();
    plugxObj.GetVersion();
    plugxObj.GetSCN();
    
    plugxObj.Invoke(&req, &rsp);
}

