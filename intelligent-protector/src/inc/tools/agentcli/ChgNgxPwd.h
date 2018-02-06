/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/*
 * ChgNgxPwd.h
 *
 *  Created on: 2015年6月27日
 *      Author: l00349472
 */

#ifndef _AGENTCLI_CHGNGXPWD_H_
#define _AGENTCLI_CHGNGXPWD_H_

#include "common/Types.h"

#define SET_NGINX_SSL_PASSWD_HINT "Change key password:"//"Change nginx ssl certificate key password:"
#define SET_NGINX_SSL_CRT_HINT "Change certificate file name:"
#define SET_NGINX_SSL_CRT_KEY_HINT "Change certificate key file name:"

// 最后增加个空格，防止查询错误
#define NGINX_SSL_CRT_FILE         "ssl_certificate "
#define NGINX_SSL_CRT_KEY_FILE     "ssl_certificate_key "

#ifndef WIN32
#define OPERATION_INPUT_CRT_FAIL_HINT "Change certificate file failed, certificate file must be location at "\
									  "${AGENT_ROOT}/bin/nginx/conf directory, please check certificate file exists."
#define OPERATION_INPUT_CRT_KEY_FAIL_HINT "Change certificate key file failed, certificate key file must be location at "\
									  "${AGENT_ROOT}/bin/nginx/conf directory, please check certificate file exists."
#else
#define OPERATION_INPUT_CRT_FAIL_HINT "Change certificate file failed, certificate file must be location at "\
									  "%AGENT_INSTALL_PATH%\\bin\\nginx\\conf directory, please check certificate file exists."
#define OPERATION_INPUT_CRT_KEY_FAIL_HINT "Change certificate key file failed, certificate key file must be location at "\
									  "%AGENT_INSTALL_PATH%\\bin\\nginx\\conf directory, please check certificate file exists."
#endif

class CChgNgxPwd
{
public:
    static mp_int32 Handle();
private:
	//2016-03 新增支持修改证书名称和证书key文件
	static mp_int32 InputNginxInfo(mp_string &strCertificate, mp_string &strKeyFile, mp_string &strNewPwd);
	static mp_int32 ChgNginxInfo(mp_string strCertificate, mp_string strKeyFile, mp_string strNewPwd);
};

#endif /* _AGENTCLI_CHGNGXPWD_H_ */
