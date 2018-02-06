/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/ShowStatusTest.h"

static mp_int32 s_iCounter = 0;

static mp_bool stubIsStartted(mp_void)
{
    if (s_iCounter == 0)
    {
        return MP_FALSE;
    }
    else
    {
        return MP_TRUE;
    }
}

static mp_int32 stub_ReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static mp_int32 iCounter = 0;
    if ( iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        vecOutput.push_back("xx.xx.xx.xx");
        return MP_SUCCESS;
    }
}


TEST_F(CShowStatusTest, Handle)
{
    mp_int32 iRet = MP_FALSE;

    typedef mp_bool (*pOrgIsStartted)(PROCESS_TYPE eType);
    typedef mp_bool (*pStubIsStartted)(mp_void);
    Stub<pOrgIsStartted, pStubIsStartted, mp_void> stubCShowStatus(&CShowStatus::IsStartted, &stubIsStartted);

    CShowStatus showStatus;
    iRet = showStatus.Handle();
    EXPECT_EQ(iRet, MP_SUCCESS);

    s_iCounter = 1;
    iRet = showStatus.Handle();
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    return;
}


TEST_F(CShowStatusTest, IsStartted)
{
    CShowStatus showStatus;
    
    showStatus.IsStartted(PROCESS_RDAGENT);
    showStatus.IsStartted(PROCESS_NGINX);
    showStatus.IsStartted(PROCESS_MONITOR);
    showStatus.IsStartted(PROCESS_BUTT);
}


TEST_F(CShowStatusTest, ShowSvn)
{
    CShowStatus showStatus;
    mp_int32 iRet = MP_FALSE;

    typedef mp_int32 (*pOrgReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    typedef mp_int32 (*pStubReadFileType)(mp_string& strFilePath, vector<mp_string>& vecOutput);
    Stub<pOrgReadFileType, pStubReadFileType, mp_void> stubSign1(&CMpFile::ReadFile, &stub_ReadFile);
    
    showStatus.ShowSvn();
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    showStatus.ShowSvn();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

