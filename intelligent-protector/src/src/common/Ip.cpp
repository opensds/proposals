/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Ip.h"

/*------------------------------------------------------------ 
Description  :判断IPV4地址是否正确
Input        :      strIpAddr---IP地址
Output       :    
Return       :   MP_TRUE---IPV4正确
                  MP_FALSE---IPV4错误
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CIPCheck::IsIPV4(mp_string& strIpAddr)
{
    vector<mp_string> vecOut;
    SplitIPV4(strIpAddr, vecOut);
    if (vecOut.size() != 4)
    {
        return MP_FALSE;
    }

    mp_int32 i255 = 0;
    mp_int32 i0 = 0;
    for (vector<mp_string>::iterator it = vecOut.begin(); it != vecOut.end(); it++)
    {
        if(!IsNumber(it->c_str()))
        {
            return MP_FALSE;
        }
        mp_int32 iValue = atoi(it->c_str());
        if (iValue <0 || iValue > 255)
        {
            return MP_FALSE;
        }
        if (iValue == 0)
        {
            i0++;
        }
        if (iValue == 255)
        {
            i255++;
        }
    }

    //全0或全255返回错误
    if (i0 == 4 || i255 == 4)
    {
        return MP_FALSE;
    }

    //其他判断
    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :分割IPV4
Input        :      strIpAddr---IP地址
Output       :    vecOutput---输出分割后的值
Return       :    
                   
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CIPCheck::SplitIPV4(mp_string& strIpAddr, vector<mp_string>& vecOutput)
{
    mp_string strIp = strIpAddr;
    mp_string::size_type pos = strIp.find(DELIM);
    while (pos != mp_string::npos)
    {
        mp_string strOut = strIp.substr(0, pos);
        strIp = strIp.substr(pos + 1);
        pos = strIp.find(DELIM);
        vecOutput.push_back(strOut);
    }
    vecOutput.push_back(strIp);
}

/*------------------------------------------------------------ 
Description  :判断传入的字符串是否是10进制的
                  只包含'0'-'9'的数字
Input        :      str为传入的字符串
Output       :    
Return       :    如果是只包含'0'-'9'的十进制则返回MP_TRUE
                   
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CIPCheck::IsNumber(mp_string str){

    if(str.size() == 0){
        return MP_FALSE;
    }

    if(str.size() > 1 && str[0] == '0'){
        return MP_FALSE;
    }
    
    for(mp_string::iterator iter = str.begin(); iter != str.end(); iter++)
   {
        if(*iter < '0' || *iter > '9')
        {
            return MP_FALSE;
        }
    }
    
    return MP_TRUE;
}


