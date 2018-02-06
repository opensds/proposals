/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/PermissionTest.h"

TEST_F(CPermissionTest,set){
    CPermission m_CPermission;
    permission_info_t permissionInfo = {"test","test","test"};
    
    {
        m_CPermission.Set(permissionInfo);
    }
    
    {
        Stub<ChmodType, StubChmodType, mp_void> mystub1(&CPermission::Chmod, &StubChmod);
        m_CPermission.Set(permissionInfo);
    }
}
