/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/oracle/Oracle.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "common/Path.h"
#include "array/Array.h"
#include <sstream>
COracle::COracle()
{
}

COracle::~COracle()
{
}

/*------------------------------------------------------------ 
Description  : check oracle is installed
Input        : NULL
Output       : 
Return       : MP_TRUE -- install
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/   
mp_int32 COracle::IsInstalled(mp_bool &bIsInstalled)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    oracle_db_info_t stDBInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin check oracle database install.");
    BuildConsistentScriptParam(stDBInfo, strParam, ORACLE_SCRIPTPARAM_CHECK_ORACLE);
    
#ifdef WIN32
    HKEY hKEY;
    mp_char szErr[256] = {0};

    mp_long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ORACLE_REG_KEY, NULL, KEY_READ, &hKEY);
    if (lRet != ERROR_SUCCESS)
    {
        GetOSStrErr(lRet, szErr, sizeof(szErr));
        RegCloseKey(hKEY);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open reg key HKEY_LOCAL_MACHINE\\%s failed, errno[%d]: %s.", 
            ORACLE_REG_KEY, (mp_int32)lRet, szErr);
        
        if (WIN_ERROR_FILE_NOT_FOUND != lRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open reg key HKEY_LOCAL_MACHINE\\%s failed, errno[%d]: %s.",
                ORACLE_REG_KEY, (mp_int32)lRet, szErr);
            return MP_FAILED;
        }

        bIsInstalled = MP_FALSE;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Oracle isn't installed.");
        return MP_SUCCESS;
    }

    bIsInstalled = MP_TRUE;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Oracle is already installed.");
#else
    //Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_FREEZEORACLE, strParam, NULL);
#endif

    if (ERROR_SCRIPT_ORACLE_NOT_INSTALLED == iRet)
    {
        bIsInstalled = MP_FALSE;
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Oracle not installed.");
    }
    else if (MP_SUCCESS == iRet)
    {
        bIsInstalled = MP_TRUE;
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Oracle installed.");
    }
    else
    {
#ifdef WIN32
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
#else
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check oracle databas failed, iRet %d", iRet);
        return iRet;
#endif
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Check oracle database succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : get oracle pdb list
Input        : lstPDBInfo -- oracle PDB list to be returned
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetPDBInfo(oracle_pdb_req_info_t &stPdbReqInfo, vector<oracle_pdb_rsp_info_t> &vecOraclePdbInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get oracle PDB info.");

    BuildPDBInfoScriptParam(stPdbReqInfo, strParam);
#ifdef WIN32
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get oracle PDB info failed, this function is unimplement");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYORACLEPDBINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute oracle PDB info script failed, iRet %d", iRet);
        return iRet;
    }

    iRet = AnalysePDBInfoScriptRst(vecResult, vecOraclePdbInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse oracle PDB info failed, iRet %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get oracle PDB info succ.");
    return MP_SUCCESS;
#endif
}
/*------------------------------------------------------------ 
Description  : start oracle pdb
Input        : stPdbReqInfo -- oracle PDB info
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::StartPluginDB(oracle_pdb_req_info_t &stPdbReqInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start oracle PDB.");

    BuildPDBInfoScriptParam(stPdbReqInfo, strParam);

#ifdef WIN32
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start PDB failed, this function is unimplement");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#else
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STARTORACLEPDB, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute starting oracle PDB script failed, iRet %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start PDB succ.");
    return MP_SUCCESS;
#endif
}

/*------------------------------------------------------------ 
Description  : build the parameter for the script to be excuted
Input        : stPDBInfo -- the oracle database information
Output       : strParam -- the paramter to be returned
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void COracle::BuildPDBInfoScriptParam(oracle_pdb_req_info_t &stPdbInfo, mp_string &strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stPdbInfo.strInstName+ mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stPdbInfo.strOracleHome+ mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stPdbInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stPdbInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCIPRTPARAM_PDBNAME) + stPdbInfo.strPdbName;
}

/*------------------------------------------------------------ 
Description  : analyse database pdb info from the result of script
Input        : vecResult -- the results returned by the script
Output       : vecOraclePdbInfo -- the oracle PDB vector to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/   
mp_int32 COracle::AnalysePDBInfoScriptRst(vector<mp_string> &vecResult, vector<oracle_pdb_rsp_info_t> &vecOraclePdbInfo)
{
    size_t idxSep, idxSepSec;
    mp_string strConID;
    mp_string strPdbName;
    mp_string strStatus;
    oracle_pdb_rsp_info_t stPDBInfo;
    vector<mp_string>::iterator iter;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin analyse oracle PDB info script result");

    const mp_string SEPARATOR = STR_SEMICOLON;

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get PDB info failed, result file is null.");
        return ERROR_SQLSERVER_DB_NOT_EXIST;
    }
    
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Result file: %s.", (*iter).c_str());
        //find 1st separator(;) version
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get PDB inst info failed when find 1st separator, PDB info is [%s].", (*iter).c_str());
            continue;
        }
        strConID = iter->substr(0, idxSep);

        //find 2nd separator(;)  instance name  
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get PDB inst info failed when find 2nd separator, PDB info is [%s].", (*iter).c_str());
            continue;
        }      
        strPdbName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);  
   
        strStatus = iter->substr(idxSepSec + 1);
  
        stPDBInfo.iConID = atoi(strConID.c_str());
        if (stPDBInfo.iConID < 0)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PDB cond_id not defined, con_id: %d.", stPDBInfo.iConID);
            return ERROR_SQLSERVER_DB_NOT_EXIST;  
        }

        stPDBInfo.strPdbName = strPdbName;
        if (stPDBInfo.strPdbName.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PDB name is null");
            return ERROR_SQLSERVER_DB_NOT_EXIST;  
        }

        iRet = TranslatePDBStatus(strStatus, stPDBInfo.iStatus);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PDB status not defined, status: %s.", strStatus.c_str());
            return ERROR_SQLSERVER_DB_NOT_EXIST;  
        }

        vecOraclePdbInfo.push_back(stPDBInfo);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The analyse result of script is:conID(%d), pdbName(%s), status(%d).", \
            stPDBInfo.iConID, stPDBInfo.strPdbName.c_str(), stPDBInfo.iStatus);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Analyse oracle PDB info script result succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : Translate the string of PDB status into int32
Input        : strStatus -- status in string 
Output       : iStatus -- status in int32
Return       : MP_SUCCESS -- success
               MP_FAILED -- failed
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::TranslatePDBStatus(mp_string &strStatus, mp_int32 &iStatus)
{
    mp_string strPdbMounted(INIT_PDB_STATUS_MOUNTED);
    mp_string strPdbReadOnly(INIT_PDB_STATUS_READ_ONLY);
    mp_string strPdbReadWrite(INIT_PDB_STATUS_READ_WRITE);
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Translate PDB status.");
    if (strStatus == strPdbMounted)
    {
        iStatus = PDB_MOUNTED;
    }
    else if(strStatus == strPdbReadOnly)
    {
        iStatus = PDB_READ_ONLY;
    }
    else if(strStatus == strPdbReadWrite)
    {
        iStatus = PDB_READ_WRITE;
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Translate PDB status fialed, status: %s.", strStatus.c_str());
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Translate PDB status succ.");
    return MP_SUCCESS;
}

/*************************begin 获取Oracle实例列表***********************************/
/*------------------------------------------------------------ 
Description  : get oracle list
Input        : lstOracleInstInfo -- oracle database list to be returned
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/   
mp_int32 COracle::GetDBInfo(list<oracle_inst_info_t> &lstOracleInstInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get oracle info.");
#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_INFO), strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //oracle下切换到Oracle用户下执行执行SQLPLUS必须在root下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYORACLEINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute oracle info script failed, iRet %d", iRet);
        return iRet;
    }

    //find oracle database info by script
    (mp_void)AnalyseInstInfoScriptRst(vecResult, lstOracleInstInfo);

    //anlyse config file, use config database info first when the result of script has this instance info still.
    iRet = AnalyseDatabaseNameByConfFile(lstOracleInstInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse oracle instance mapping info failed, iRet %d", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get oracle info succ.");
    return MP_SUCCESS;
}

mp_int32 COracle::AnalyseDatabaseNameByConfFile(list<oracle_inst_info_t> &lstOracleInstInfo)
{
    mp_int32 iRet = MP_FAILED;
    mp_string strOraCfgFile = CPath::GetInstance().GetConfPath() 
        + PATH_SEPARATOR + ORA_INSTANCE_DATABASE_CONFIGFILE;
        
    // check config exists
    if (MP_FALSE == CMpFile::FileExist(strOraCfgFile.c_str()))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Oracle config file not exist, no need to analyse mapping info.");
        return MP_SUCCESS;
    }

    vector<mp_string> cfgContent;
    iRet = CMpFile::ReadFile(strOraCfgFile, cfgContent);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read config file failed, iRet %d", iRet);
        return iRet;
    }

    size_t idxSep;
    mp_string strIntance, strDB;
    // anlayse database and instance config information
    for (vector<mp_string>::iterator iter = cfgContent.begin(); 
        iter != cfgContent.end(); ++iter)
    {
        //find 1st separator( )
        idxSep = iter->find(" ");
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst mapping info failed when find 1nd separator, inst info is [%s].", (*iter).c_str());;
            continue;
        }
        strIntance = iter->substr(0, idxSep);
        strDB = iter->substr(idxSep + 1);

        // check mapping info
        for (list<oracle_inst_info_t>::iterator iterScript = lstOracleInstInfo.begin(); 
                iterScript != lstOracleInstInfo.end(); ++iterScript)
        {
            if (iterScript->strInstName == strIntance)
            {
                iterScript->strDBName = strDB;
            }
        }
    }

    return MP_SUCCESS;
}

mp_int32 COracle::GetArchiveLogMode(oracle_db_info_t& stDBInfo, mp_int32& iArvhiceLogMode)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    mp_int32 iArchiveModeTmp = 0;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get oracle archive log mode.");
    BuildConsistentScriptParam(stDBInfo, strParam, ORACLE_SCRIPTPARAM_GET_ARCHIVE_LOG_MODE);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_CONSISTENT), strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Get archive log mode failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_ARCHIVEORACLE, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle arvhice log mode failed, iRet %d",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of get oracle archive log mode is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }

    iArchiveModeTmp = atoi(vecResult.front().c_str());
    iArvhiceLogMode = (0 == iArchiveModeTmp ? NON_ARCHIVE_LOG_MODE : ARCHIVE_LOG_MODE);
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get oracle archive log mode succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : analyse database instance info from the result of script
Input        : vecResult -- the results returned by the script
Output       : lstOracleInstInfo -- the oracle instance list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/   
mp_int32 COracle::AnalyseInstInfoScriptRst(vector<mp_string> &vecResult, list<oracle_inst_info_t> &lstOracleInstInfo)
{
    size_t idxSep, idxSepSec, idxSepTrd, idxSepFor, idxSepFiv;
    mp_string strVersion;
    mp_string strInstName;
    mp_string strDBName;
    mp_string strState;
    mp_string strIsASMDB;
    mp_string strVSSStatus;
    mp_string strOracleHome;
    oracle_inst_info_t struDBInstInfo;
    vector<mp_string>::iterator iter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin analyse oracle info script result.");

    const mp_string SEPARATOR = STR_SEMICOLON;
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        //find 1st separator(;) version
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 1nd separator, inst info is [%s].", (*iter).c_str());;
            continue;
        }
        strVersion = iter->substr(0, idxSep);

        //find 2nd separator(;)  instance name  
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 2nd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }      
        strInstName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);  

        //find 3rd separator(;)  database name
        idxSepTrd = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepTrd)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 3rd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }    
        strDBName = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);

        //find 4rd separator(;)  state
        idxSepFor = iter->find(SEPARATOR, idxSepTrd + 1);
        if (mp_string::npos == idxSepFor)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 4rd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }    
        strState = iter->substr(idxSepTrd + 1, (idxSepFor - idxSepTrd) - 1);

        //find 5rd separator(;)  ASM flag
        idxSepFiv = iter->find(SEPARATOR, idxSepFor + 1);
        if (mp_string::npos == idxSepFiv)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 5rd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }    
        strIsASMDB = iter->substr(idxSepFor + 1, (idxSepFiv - idxSepFor) - 1);

#ifdef WIN32
        //find 6rd separator(;) : VSS status
        size_t idxSepSix = iter->find(SEPARATOR, idxSepFiv + 1);
        if (mp_string::npos == idxSepSix)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 6rd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }
        strVSSStatus = iter->substr(idxSepFiv + 1, (idxSepSix - idxSepFiv) - 1);
        strOracleHome = iter->substr(idxSepSix + 1);
#else
        strOracleHome = iter->substr(idxSepFiv + 1);
#endif
        
        struDBInstInfo.strVersion = strVersion;
        struDBInstInfo.strInstName = strInstName;
        struDBInstInfo.strDBName = strDBName;
        struDBInstInfo.iState = atoi(strState.c_str());
        struDBInstInfo.iIsASMDB = atoi(strIsASMDB.c_str());
        struDBInstInfo.strOracleHome = strOracleHome;
#ifdef WIN32
        struDBInstInfo.iVssWriterStatus = atoi(strVSSStatus.c_str());
#endif

        lstOracleInstInfo.push_back(struDBInstInfo);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "The analyse result of script is:instname(%s), dbname(%s), version(%s), state(%d), oraclehome(%s).",
            struDBInstInfo.strInstName.c_str(), struDBInstInfo.strDBName.c_str(),
            struDBInstInfo.strVersion.c_str(), struDBInstInfo.iState, struDBInstInfo.strOracleHome.c_str());
    }


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Analyse oracle info script result succ.");
    return MP_SUCCESS;
  
}

/*************************end 获取Oracle实例列表***********************************/


/*************************begin 获取Oracle实例名列表***********************************/
/*------------------------------------------------------------ 
Description  : 获取实例名
Input        : 
Output       : lstOracleInstName -- 实例名列表
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetInstances(oracle_db_info_t& stDBInfo, list<oracle_inst_info_t>& lstOracleInsts)
{
    mp_int32 iRet = MP_SUCCESS;
    list<oracle_inst_info_t>::iterator iter;

    iRet = GetDBInfo(lstOracleInsts);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get instance list failed, iRet %d.", iRet);
        return iRet;
    }

    for (iter = lstOracleInsts.begin(); iter != lstOracleInsts.end(); iter++)
    {
        stDBInfo.strInstName = iter->strInstName;
        iRet = GetArchiveLogMode(stDBInfo, iter->iArchiveLogMode);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get oracle conf info failed, iRet %d.", iRet);
            return iRet;
        }
    }

    return iRet;
}

/*************************end 获取Oracle实例名列表***********************************/

/*************************begin 获取Oracle数据库使用LUN列表**********************/
/*------------------------------------------------------------ 
Description  : get the storage information of the oracle database
Input        : stDBInfo -- the oracle database information
Output       : vecLUNInfos -- the result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 COracle::GetDBLUNInfo(oracle_db_info_t &stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin excute oracle lun info script.");

    if (ORACLE_QUERY_ARCHIVE_LOGS != stDBInfo.iGetArchiveLUN)
    {
        // 获取数据文件、控制文件、日志文件
        iRet = GetLunInfoByStorageType(stDBInfo, vecLUNInfos, mp_string(DBADAPTIVE_PRAMA_MUST));
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_QUERY_APP_LUN_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetLunInfoByStorageType failed, type(must), instanceName(%s), iRet %d.",
                stDBInfo.strInstName.c_str(), iRet);
            return iRet;
        }

        //获取临时文件、spfile文件
        iRet = GetLunInfoByStorageType(stDBInfo, vecLUNInfos, mp_string(DBADAPTIVE_PRAMA_OPTION));
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_QUERY_APP_LUN_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetLunInfoByStorageType failed, type(operation), instanceName(%s), iRet %d.",
                stDBInfo.strInstName.c_str(), iRet);
            return iRet;
        }
    }
    else
    {
        iRet = GetLunInfoByStorageType(stDBInfo, vecLUNInfos, mp_string(DBADAPTIVE_PRAMA_ARCHIVE));
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_QUERY_APP_LUN_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetLunInfoByStorageType failed, type(archive), instanceName(%s), iRet %d.",
                stDBInfo.strInstName.c_str(), iRet);
            return iRet;
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Excute oracle lun info script succ.");
    return MP_SUCCESS;
}

// 获取oracle数据库的存储内容
// strStorageType:
// must         -- 获取控制、数据、日志表空间，这三种必须在LUN上
// option       -- 获取临时、spfile的表空间，这两种可以不在LUN上
// archive      -- 获取归档日志存储内容
/*------------------------------------------------------------ 
Description  : get the storage information of the oracle database by storage type {must|option|archive}
Input        : stDBInfo -- the oracle database information
Output       : vecLUNInfos -- the result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 COracle::GetLunInfoByStorageType(oracle_db_info_t stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos,
    mp_string strStorageType)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;
    vector<oracle_storage_script_info> vecDBStorageScriptInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin excute GetLunInfoByStorageType.");

    stDBInfo.strTableSpaceName = strStorageType;
    BuildLunInfoScriptParam(stDBInfo, strParam);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_LUN_INFO), strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYORACLELUNINFO, strParam, &vecResult);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute oracle lun info script failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "vecResult:%d;", vecResult.size());

    //分析脚本返回结果
    (mp_void)AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "vecDBStorageScriptInfo:%d;", vecDBStorageScriptInfo.size());

    //根据脚本返回结果获取LUN信息
    iRet = AnalyseLunInfoByScriptRST(vecDBStorageScriptInfo, vecLUNInfos, strStorageType);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse oracle lun info from script result failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "vecLUNInfos:%d;", vecLUNInfos.size());


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Excute GetLunInfoByStorageType succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : build the parameter for the script to be excuted
Input        : stDBInfo -- the oracle database information
Output       : strParam -- the paramter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void COracle::BuildLunInfoScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_TABLESPACENAME) + stDBInfo.strTableSpaceName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
}

//分析脚本返回结果
/*------------------------------------------------------------ 
Description  : analyse database storage info from the result of script
Input        : vecResult -- the string results returned by the script
Output       : vecDBStorageScriptInfo -- the structure result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/   
mp_int32 COracle::AnalyseLunInfoScriptRST(vector<mp_string> &vecResult,
    vector<oracle_storage_script_info> &vecDBStorageScriptInfo)
{
//CodeDex误报，UNINIT
#ifdef WIN32
    size_t idxSep, idxSepSec, idxSepTrd;
#else
    size_t idxSep, idxSepSec, idxSepTrd, idxSepFor, idxSepFive, idxSepSix, idxSepSeven, idxEight;
#endif
    mp_string strStorMainType;
    mp_string strStorSubType;
    mp_string strSystemDevice;
    mp_string strDeviceName;
    mp_string strDevicePath;
    mp_string strVgName;
    mp_string strASMDiskGroup;
    mp_string strUDEVRes;
    mp_string strUDEVDevice;
    vector<mp_string>::iterator iter;

    const mp_string SEPARATOR = NODE_SEMICOLON;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get oracle storage info.");

    //storMainType;storSubType;systemDevice;DeviceName;DevicePath;VGName;ASMDGName;UDEVResult
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
#ifdef WIN32
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "%s", iter->c_str());
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 1nd separator, storage info=%s.", (*iter).c_str());;
            continue;
        }
        strDeviceName = iter->substr(0, idxSep);

        //find 2nd separator(;)
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 2nd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strDevicePath = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);

        //find 3rd separator(;)
        idxSepTrd = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepTrd)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 3rd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strStorMainType = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);
        strASMDiskGroup = iter->substr(idxSepTrd + 1);
#else
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "%s", iter->c_str());
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 1nd separator, storage info=%s.", (*iter).c_str());;
            continue;
        }
        strStorMainType = iter->substr(0, idxSep);

        //find 2nd separator(;)
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 2nd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strStorSubType = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);

        //find 3rd separator(;)
        idxSepTrd = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepTrd)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 3rd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strSystemDevice = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);

        //find 4rd separator(;)
        idxSepFor = iter->find(SEPARATOR, idxSepTrd + 1);
        if (mp_string::npos == idxSepFor)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 4rd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strDeviceName = iter->substr(idxSepTrd + 1, (idxSepFor - idxSepTrd) - 1);

        //find 5rd separator(;)
        idxSepFive = iter->find(SEPARATOR, idxSepFor + 1);
        if (mp_string::npos == idxSepFive)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 5rd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strDevicePath = iter->substr(idxSepFor + 1, (idxSepFive - idxSepFor) - 1);

        //find 6rd separator(;)
        idxSepSix = iter->find(SEPARATOR, idxSepFive + 1);
        if (mp_string::npos == idxSepSix)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 6rd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strVgName = iter->substr(idxSepFive + 1, (idxSepSix - idxSepFive) - 1);

        //find 7rd separator(;)
        idxSepSeven= iter->find(SEPARATOR, idxSepSix + 1);
        if (mp_string::npos == idxSepSeven)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 7rd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strASMDiskGroup = iter->substr(idxSepSix + 1, (idxSepSeven - idxSepSix) - 1);

        //find 8rd separator(;)
        idxEight = iter->find(SEPARATOR, idxSepSeven + 1);
        if (mp_string::npos == idxEight)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 8rd separator, storage info=%s.", (*iter).c_str());
            continue;
        }
        strUDEVRes = iter->substr(idxSepSeven + 1, (idxEight - idxSepSeven) - 1);
        strUDEVDevice = iter->substr(idxEight + 1);
#endif

        oracle_storage_script_info oracle_stor_info;
        oracle_stor_info.strStorMainType = strStorMainType;
        oracle_stor_info.strStorSubType = strStorSubType;
        oracle_stor_info.strSystemDevice = strSystemDevice;
        oracle_stor_info.strDeviceName = strDeviceName;
        oracle_stor_info.strDevicePath = strDevicePath;
        oracle_stor_info.strVgName = strVgName;
        oracle_stor_info.strASMDiskGroup = strASMDiskGroup;
        oracle_stor_info.strUDEVRes = strUDEVRes;
        oracle_stor_info.strUDEVName = strUDEVDevice;
        vecDBStorageScriptInfo.push_back(oracle_stor_info);

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
            "Get oracle storage info structure:storMainType(%s), storSubType(%s), systemDevice(%s), deviceName(%s), "
            "devicePath(%s), vgName(%s), ASMDGName(%s), UDEVResult(%s), UDEVDevice(%s).",
            oracle_stor_info.strStorMainType.c_str(), oracle_stor_info.strStorSubType.c_str(), oracle_stor_info.strSystemDevice.c_str(),
            oracle_stor_info.strDeviceName.c_str(), oracle_stor_info.strDevicePath.c_str(), oracle_stor_info.strVgName.c_str(),
            oracle_stor_info.strASMDiskGroup.c_str(), oracle_stor_info.strUDEVRes.c_str(), oracle_stor_info.strUDEVName.c_str());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get oracle storage info succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : check whether to repeat the lun information
Input        : vecLUNInfos -- the string results returned by the script
Output       :  oracle_lun_info -- the structure result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/   
mp_bool COracle::CheckLUNInfoExists(vector<oracle_lun_info_t> &vecLUNInfos, oracle_lun_info_t &oracle_lun_info)
{
    vector<oracle_lun_info_t>::iterator lunIter;
    for (lunIter = vecLUNInfos.begin(); lunIter != vecLUNInfos.end(); ++lunIter)
    {
        if (lunIter->strWWN == oracle_lun_info.strWWN && lunIter->strDeviceName == oracle_lun_info.strDeviceName)
        {

            break;
        }
    }

    return (lunIter == vecLUNInfos.end()) ? MP_FALSE : MP_TRUE;
}

#ifdef WIN32

/*------------------------------------------------------------ 
Description  : get lun information by sub area list,  disk list and script result in windows platform
Input        : stSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : stDBLUNInfo -- the script result to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetLUNInfoWin(mp_string &strPath, sub_area_Info_t &stSubareaInfoWin, 
    vector<disk_info> &vecDiskInfoWinRes, oracle_lun_info_t &stDBLUNInfo)
{
    mp_char acOffset[MAX_PATH_LEN] = {0};
    vector<disk_info>::iterator itDiskInfoWin;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get oracle lun info with disk and subarea in windows.");

    for (itDiskInfoWin = vecDiskInfoWinRes.begin(); itDiskInfoWin != vecDiskInfoWinRes.end(); itDiskInfoWin++)
    {
        if (stSubareaInfoWin.iDiskNum == itDiskInfoWin->iDiskNum)
        {
            stDBLUNInfo.strArraySn = itDiskInfoWin->strArraySN;
            stDBLUNInfo.strLUNId = itDiskInfoWin->strLUNID;
            stDBLUNInfo.strWWN = itDiskInfoWin->strLUNWWN;
            stDBLUNInfo.strDeviceName = stSubareaInfoWin.acVolName;
            stDBLUNInfo.strDevicePath = strPath;
            CHECK_FAIL(SNPRINTF_S(acOffset, sizeof(acOffset), sizeof(acOffset) - 1, "%lld", stSubareaInfoWin.llOffset));
            stDBLUNInfo.strLBA = mp_string(acOffset);
            break;
        }
    }

    if (itDiskInfoWin == vecDiskInfoWinRes.end())
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN,
            "Get lun info(%s) with disk and subarea failed.", strPath.c_str());
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get oracle lun info with disk and subarea in windows.");
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : get lun information by path in windows
Input        : strPath -- the disk label
                vecSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetLUNInfoByPathWin(mp_string &strPath, vector<sub_area_Info_t> &vecSubareaInfoWin,
    vector<disk_info> &vecDiskInfoWinRes, vector<oracle_lun_info_t> &vecLUNInfos)
{
    vector<sub_area_Info_t>::iterator itSubareaInfoWin;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get lun info by path in windows.");

    mp_string iRetUpper = CMpString::ToUpper((mp_char *)strPath.c_str());
    for (itSubareaInfoWin = vecSubareaInfoWin.begin(); itSubareaInfoWin != vecSubareaInfoWin.end(); itSubareaInfoWin++)
    {
    	mp_string tmpstrPath = CMpString::ToUpper((mp_char *)(itSubareaInfoWin->acDriveLetter));		
        if (iRetUpper == tmpstrPath)
        {
            oracle_lun_info_t stDBLUNInfo;
            stDBLUNInfo.strArraySn = "";
            stDBLUNInfo.iStorMainType = atoi(STORAGE_TYPE_FS);
            stDBLUNInfo.iStorSubType= VOLTYPE_NOVOL;
            if (MP_SUCCESS != GetLUNInfoWin(strPath, *itSubareaInfoWin, vecDiskInfoWinRes, stDBLUNInfo))
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info failed.");
                return MP_FAILED;
            }

            vecLUNInfos.push_back(stDBLUNInfo);

            break;
        }
    }

    if (itSubareaInfoWin == vecSubareaInfoWin.end())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "The disk label(%s) is not exist.", strPath.c_str());

        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get lun info by path in windows succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : get filesystem lun information in windows
Input        : vecDiskPath -- the disk label list
                vecSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetDBLUNFSInfoWin(vector<mp_string> &vecDiskPath, 
    vector<disk_info> &vecDiskInfoWinRes, vector<sub_area_Info_t> &vecSubareaInfoWin, 
    vector<oracle_lun_info_t> &vecLUNInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    LOGGUARD("");
    for (vector<mp_string>::iterator itPath = vecDiskPath.begin(); itPath != vecDiskPath.end(); itPath++)
    {
        iRet = GetLUNInfoByPathWin(*itPath, vecSubareaInfoWin, vecDiskInfoWinRes, vecLUNInfos);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun file system info(%s) by path in windows failed.",
                itPath->c_str());
            return iRet;
        }
    }

    return iRet;
}

/*------------------------------------------------------------ 
Description  : get asm lun information in windows
Input        : vecAdaptiveLUNInfo -- script result list
                vecSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetDBLUNASMInfoWin(vector<oracle_storage_script_info> &vecAdaptiveLUNInfo, 
    vector<disk_info> &vecDiskInfoWinRes, vector<sub_area_Info_t> &vecSubareaInfoWin, 
    vector<oracle_lun_info_t> &vecLUNInfos)
{
    CDisk diskManager;
    vector<sub_area_Info_t>::iterator itSubareaInfoWin;
    LOGGUARD("");
    if (MP_TRUE != diskManager.InitSymboLinkRes())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "init Symbolink failed.");
        return MP_FAILED;
    }

    // 获取ASM标记分区的设备信息
    for (vector<oracle_storage_script_info>::iterator iter = vecAdaptiveLUNInfo.begin();
            iter != vecAdaptiveLUNInfo.end(); ++iter)
    {
        if (iter->strStorMainType != STORAGE_TYPE_ASMWIN)
        {
            continue;
        }

        if (MP_TRUE != diskManager.QuerySymboLinkInfo(iter->strDeviceName, iter->strSystemDevice))
        {
            continue;
        }

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "querySymboLinkInfo: [%s]=>[%s]",
            iter->strDeviceName.c_str(), iter->strSystemDevice.c_str());
        itSubareaInfoWin = vecSubareaInfoWin.begin();
        for ( ; itSubareaInfoWin != vecSubareaInfoWin.end(); ++itSubareaInfoWin)
        {
            if (0 == mp_string(itSubareaInfoWin->acDeviceName).compare(iter->strSystemDevice))
            {
                oracle_lun_info_t stDBLUNInfo;
                stDBLUNInfo.strArraySn = "";
                stDBLUNInfo.strASMDiskGroup = iter->strASMDiskGroup;
                stDBLUNInfo.iStorMainType = atoi(STORAGE_TYPE_ASMWIN);
                stDBLUNInfo.iStorSubType = VOLTYPE_NOVOL;
                if (MP_SUCCESS != GetLUNInfoWin(iter->strDevicePath, *itSubareaInfoWin, vecDiskInfoWinRes, stDBLUNInfo))
                {
                    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info(%s) failed.", iter->strDevicePath.c_str());
                    diskManager.FreeSymboLinkRes();
                    return MP_FAILED;
                }

                // check luninfo exists.
                if (MP_FALSE == CheckLUNInfoExists(vecLUNInfos, stDBLUNInfo))
                {
                    vecLUNInfos.push_back(stDBLUNInfo);
                }
                break;
            }
        }

        if (itSubareaInfoWin == vecSubareaInfoWin.end())
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "ASM disk([%s]=>[%s]) match volumn deviceName failed.",
                iter->strDeviceName.c_str(), iter->strSystemDevice.c_str());
        }
    }
    diskManager.FreeSymboLinkRes();

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : get lun information in windows
Input        : vecDiskPath -- the disk label list
                vecSubareaInfoWin -- sub area list
                vecDiskInfoWinRes -- disk list
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetDBLUNInfoWin(vector<mp_string> &vecDiskPath,
    vector<oracle_storage_script_info> &vecAdaptiveLUNInfo, vector<oracle_lun_info_t> &vecLUNInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    CDisk diskManager;

    vector<disk_info> vecDiskInfoWinRes;
    vector<sub_area_Info_t> vecSubareaInfoWin;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get oracle lun in windows.");

    //获取磁盘信息，包括阵列SN,LUN ID,LUN WWN,磁盘编号
    iRet = CDisk::GetDiskInfoList(vecDiskInfoWinRes);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s",
            "Get disk infomation on windows failed.");

        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }

    //获取磁盘序号和盘符的对应信息
    iRet = CDisk::GetSubareaInfoList(vecSubareaInfoWin);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s",
            "Get disk subares on windows failed.");

        return ERROR_DISK_GET_PARTITION_INFO_FAILED;
    }

    //获取数据库文件所在的LUN信息
    iRet = GetDBLUNFSInfoWin(vecDiskPath, vecDiskInfoWinRes, vecSubareaInfoWin, vecLUNInfos);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun filesystem info in windows failed (%d).", iRet);
        return iRet;
    }


    iRet = GetDBLUNASMInfoWin(vecAdaptiveLUNInfo, vecDiskInfoWinRes, vecSubareaInfoWin, vecLUNInfos);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun ASM info in windows failed (%d).", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get oracle lun in windows succ.");
    return MP_SUCCESS;
}

#endif

#ifdef WIN32
/*------------------------------------------------------------ 
Description  : analyse lun info with result of script in windows
Input        : vecDBStorageScriptInfo -- the script result
                strStorageType -- the storage type {must|option|archive}
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::AnalyseLunInfoByScriptRSTWIN(vector<oracle_storage_script_info> &vecDBStorageScriptInfo,
    vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType)
{
    mp_int32 iRet = MP_SUCCESS;

    LOGGUARD("");
    // 获取的磁盘列表
    vector<mp_string> vecDiskPath;
    vector<oracle_lun_info_t> vecDBLUNInfos;
   //CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    for (vector<oracle_storage_script_info>::iterator iter = vecDBStorageScriptInfo.begin();
        iter != vecDBStorageScriptInfo.end(); ++iter)
    {
        if (iter->strStorMainType == STORAGE_TYPE_FS)
        {
            vecDiskPath.push_back(iter->strDeviceName);
        }
    }

    iRet = GetDBLUNInfoWin(vecDiskPath, vecDBStorageScriptInfo, vecDBLUNInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get windows lun info failed, iRet %d.", iRet);
        return iRet;
    }

    // 判断arraySN和重复LUN信息
    for (vector<oracle_lun_info_t>::iterator lunIter = vecDBLUNInfos.begin();
        lunIter != vecDBLUNInfos.end(); ++lunIter)
    {
        // 如果是获取数据文件、控制文件、日志文件存储，必须所有的文件都在存储上
        if (lunIter->strArraySn.empty())
        {
            if (strStorageType == DBADAPTIVE_PRAMA_MUST)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Part of DB file is not stored on huawei/huasai array.");
                return ERROR_COMMON_NOT_HUAWEI_LUN;
            }
            else
            {
                COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "ArraySN is empty, but not must info, continue.");
                continue;
            }
        }

        // check luninfo exists.
        if (MP_FALSE == CheckLUNInfoExists(vecLUNInfos, *lunIter))
        {
            vecLUNInfos.push_back(*lunIter);
        }
    }

    return MP_SUCCESS;
}
#else

/*------------------------------------------------------------ 
Description  : check array sn of device used by oracle database
Input        : strDev -- device name
                strStorageType -- the storage type {must|option|archive}
Output       : strArraySN -- array to be get
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetAndCheckArraySN(mp_string strDev, mp_string &strArraySN, mp_string strStorageType)
{
    mp_int32 iRet = MP_SUCCESS;
    LOGGUARD("");
    // 获取LUN对应的信息
    iRet = CArray::GetArraySN(strDev, strArraySN);
    if (MP_SUCCESS != iRet)
    {
        // 如果是获取数据文件、控制文件、日志文件存储，必须所有的文件都在存储上
        if (strStorageType == DBADAPTIVE_PRAMA_MUST)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get array SN of lun(%s) failed, iRet %d.",
                strDev.c_str(), iRet);
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
    
        }
        //如果是可选文件，则继续执行下一个查询
        else
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Get array SN of option lun(%s) failed, continue get next lun.",
                strDev.c_str(), iRet);
            return MP_SUCCESS;
        }
    }
    
    // 判断阵列SN是否为空
    mp_bool bCheckArraySN = (strArraySN.empty() && (strStorageType == DBADAPTIVE_PRAMA_MUST));
    if (MP_TRUE == bCheckArraySN)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Part of DB file is not stored on huawei/huasai array");
        return ERROR_COMMON_NOT_HUAWEI_LUN;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : get vendor and product of device used by oracle database
Input        : strDev -- device name
                strStorageType -- the storage type {must|option|archive}
Output       : strVendor -- array sn to be get
                strProduct -- product to be get
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetVendorAndProduct(mp_string strDev, mp_string &strVendor, 
    mp_string &strProduct, mp_string strStorageType)
{
    mp_int32 iRet = MP_SUCCESS;
    LOGGUARD("");
    //阵列的厂商和型号
    iRet = CArray::GetArrayVendorAndProduct(strDev, strVendor, strProduct);
    if (MP_FAILED == iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get vendor and product info failed.");
        return iRet;
    }

    mp_char *pStrTmp = CMpString::Trim((mp_char *)strVendor.c_str());
    if (!pStrTmp)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get vendor info of Device name(%s) failed.", strDev.c_str());
        return ERROR_COMMON_NOT_HUAWEI_LUN;
    }
    strVendor = pStrTmp;

    //product内容查询出来后并没有使用，如果为空只做提示，不做退出
    pStrTmp = CMpString::Trim((mp_char *)strProduct.c_str());
    if (!pStrTmp)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Get product info of Device name(%s) failed.", strDev.c_str());
        strProduct = "";
    }
    else
    {
        strProduct = pStrTmp;
    }
    
    //排除掉非华为的产品
    if (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
        && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
        && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Device name(%s) is not in huawei/huasai array.", strDev.c_str());
        return ERROR_COMMON_NOT_HUAWEI_LUN;
    }

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : analyse lun info with result of script in no windows
Input        : vecDBStorageScriptInfo -- the script result
                strStorageType -- the storage type {must|option|archive}
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::AnalyseLunInfoByScriptRSTNoWIN(vector<oracle_storage_script_info> &vecDBStorageScriptInfo,
    vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strUDEVConfDir(""), strUDEVRoot("");
    mp_string strUdevDeviceRecord("");
    mp_string strWWN(""), strArraySN(""), strLUNID(""), strVendor(""), strProduct("");
    mp_string strDev, strDevList;
    vector<mp_string> vecResult, vecAnalyse;
    map<mp_string, luninfo_t> mapLuninfo;
    oracle_lun_info_t oracle_lun_info;

    LOGGUARD("");
    // 获取UDEV的配置
#ifdef LINUX
    iRet = GetUDEVConfig(strUDEVConfDir, strUDEVRoot);
    if (MP_SUCCESS != iRet)
    {
       COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get udev config failed, iRet %d.", iRet);
       return iRet;
    }
#endif

    // 获取设备列表字符串，通过分号隔开
    for (vector<oracle_storage_script_info>::iterator iter = vecDBStorageScriptInfo.begin();
        iter != vecDBStorageScriptInfo.end(); ++iter)
    {
        // get device name
#ifdef HP_UX_IA
        iRet = CDisk::GetHPRawDiskName(iter->strSystemDevice, strDev);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get raw device info of disk(%s) failed, iRet %d.",
                iter->strSystemDevice.c_str(), iRet);
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        }
#else
        strDev = iter->strSystemDevice;
#endif

        // check not exists
        mp_string strTmp = strDev + ";";
        if (strDevList.find(strTmp) == mp_string::npos)
        {
           strDevList = strDevList + strDev + ";";  
        }
    }

    // 去掉最后一个分号
    strDevList = strDevList.substr(0, strDevList.length() - 1);
    
    // 批量获取LUN信息,提升性能
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_BATCH_GETLUN_INFO, strDevList, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info (%s) failed.", strDevList.c_str());
        return iRet;
    }
	//codedex误报CHECK_CONTAINER_EMPTY，此处容器已判空
    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of lun info (%s) is null.", strDevList.c_str());
        return MP_FAILED;
    }

    // 分析结果文件
    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        luninfo_t luninfo;
        vecAnalyse.clear();
        CMpString::StrSplit(vecAnalyse, *iter, ';');
        if (vecAnalyse.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse result of ROOT_COMMAND_BATCH_GETLUN_INFO failed, lun info list is empty(%s).", 
                iter->c_str());
            return MP_FAILED;
        }

        if (vecAnalyse.size() != 6)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Analyse result of ROOT_COMMAND_BATCH_GETLUN_INFO failed, lun info list size is wrong(%s).", 
                iter->c_str());
            return MP_FAILED;
        }

        //devicename;vendor;product;arraysn;lunid;wwn
        vector<mp_string>::iterator iterAnalyse = vecAnalyse.begin();
        luninfo.strDeviceName = *(iterAnalyse++);
        luninfo.strVendor = *(iterAnalyse++);
        luninfo.strProduct = *(iterAnalyse++);
        luninfo.strArraySN = *(iterAnalyse++);
        luninfo.strLUNID = *(iterAnalyse++);
        luninfo.strLUNWWN = *iterAnalyse;

        mapLuninfo.insert(pair<mp_string, luninfo_t>(luninfo.strDeviceName, luninfo));
    }


    // 分析存储信息
    for (vector<oracle_storage_script_info>::iterator iter = vecDBStorageScriptInfo.begin();
        iter != vecDBStorageScriptInfo.end(); ++iter)
    {
        strWWN = "";
        strArraySN = "";
        strLUNID = "";
        strUdevDeviceRecord = "";

        oracle_lun_info.iStorMainType = atoi(iter->strStorMainType.c_str());
        oracle_lun_info.iStorSubType = atoi(iter->strStorSubType.c_str());
        oracle_lun_info.strDeviceName = iter->strDeviceName;
        oracle_lun_info.strPvName = iter->strSystemDevice;
        oracle_lun_info.strDevicePath = iter->strDevicePath;
        oracle_lun_info.strUDEVRules = strUdevDeviceRecord;
        oracle_lun_info.strVgName = iter->strVgName;
        oracle_lun_info.strASMDiskGroup = iter->strASMDiskGroup;

#ifdef LINUX
        if (!iter->strUDEVRes.empty())
        {
            iRet = GetUDEVInfo(strUDEVConfDir, iter->strUDEVName, iter->strUDEVRes, strUdevDeviceRecord);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get udev info failed, iRet %d.", iRet);
                return iRet;
            }
            if (!strUdevDeviceRecord.empty())
            {
                oracle_lun_info.strDeviceName = strUDEVRoot + iter->strUDEVName;
                oracle_lun_info.strUDEVRules = strUdevDeviceRecord;
            }
            else
            {
                COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Get (%s) udev info empty, iRet %d.", iter->strUDEVName.c_str(), iRet);
            }
        }
#endif

#ifdef HP_UX_IA
        iRet = CDisk::GetHPRawDiskName(iter->strSystemDevice, strDev);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get raw device info of disk(%s) failed, iRet %d.",
                iter->strSystemDevice.c_str(), iRet);
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        }
#else
        strDev = iter->strSystemDevice;
#endif

        map<mp_string, luninfo_t>::iterator mapIter = mapLuninfo.find(strDev);
        if (mapIter == mapLuninfo.end())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Cannot find lun info of device (%s).", strDev.c_str());
            return ERROR_COMMON_QUERY_APP_LUN_FAILED;
        }

        // 获取厂商
        luninfo_t luninfo = mapIter->second;
        strVendor = luninfo.strVendor;
        strProduct = luninfo.strProduct;

        mp_bool bHuaweiLUN = CArray::CheckHuaweiLUN(strVendor);
        if (MP_FALSE == bHuaweiLUN)
        {
            if (strStorageType != DBADAPTIVE_PRAMA_MUST)
            {
                COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Device name(%s-%s) is not in huawei/huasai array.", strStorageType.c_str(), strDev.c_str());
                continue;
            }
            else
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Device name(%s-%s) is not in huawei/huasai array.", strStorageType.c_str(), strDev.c_str());
                return ERROR_COMMON_NOT_HUAWEI_LUN;
            }           
        }

        // 获取LUN对应的ArranSN信息
        strArraySN = luninfo.strArraySN;
        if (strArraySN.empty())
        {
            // 查询must表空间，arraylun为空时返回错误码
            if (DBADAPTIVE_PRAMA_MUST == strStorageType)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get and check arraySN info of disk(%s-%s) failed, iRet %d.",
                    strStorageType.c_str(), strDev.c_str(), iRet);
                return ERROR_COMMON_NOT_HUAWEI_LUN;
            }

            // 查询option和archive，不返回arraysn为空的LUN
            continue;
        }

        // 获取LUN对应的信息
        strWWN = luninfo.strLUNWWN;
        strLUNID = luninfo.strLUNID;
        mp_bool bLuninfo = strWWN.empty() || strLUNID.empty(); 
        if (MP_TRUE == bLuninfo)
        {
            // 如果是获取数据文件、控制文件、日志文件存储，必须所有的文件都在存储上
            if (strStorageType == DBADAPTIVE_PRAMA_MUST)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun(%s) info failed, iRet %d.",
                    strDev.c_str(), iRet);
                return ERROR_COMMON_QUERY_APP_LUN_FAILED;
            }
            //如果是可选文件，则继续执行下一个查询
            else
            {
                COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Get info of option lun(%s) failed, continue get next lun.",
                    strDev.c_str(), iRet);
                continue;
            }
        }

        oracle_lun_info.strLUNId = strLUNID;
        oracle_lun_info.strArraySn = strArraySN;
        oracle_lun_info.strWWN = strWWN;

        // check luninfo exists.
        if (MP_FALSE == CheckLUNInfoExists(vecLUNInfos, oracle_lun_info))
        {
            vecLUNInfos.push_back(oracle_lun_info);
        }
    }

    return MP_SUCCESS; 
}
#endif


//通过脚本返回LUN返回信息
/*------------------------------------------------------------ 
Description  : analyse lun info with result of script
Input        : vecDBStorageScriptInfo -- the script result
                strStorageType -- the storage type {must|option|archive}
Output       : vecLUNInfos -- the lun information list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::AnalyseLunInfoByScriptRST(vector<oracle_storage_script_info> &vecDBStorageScriptInfo,
    vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType)
{
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get oracle lun info by script storage info.");

#ifdef WIN32
    return AnalyseLunInfoByScriptRSTWIN(vecDBStorageScriptInfo, vecLUNInfos, strStorageType);
#else
    return AnalyseLunInfoByScriptRSTNoWIN(vecDBStorageScriptInfo, vecLUNInfos, strStorageType);
#endif
}

#ifdef LINUX
/*------------------------------------------------------------ 
Description  : get udev config
Input        :
Output       :  strUDEVConfDir -- the udev config to be returned
                strUDEVRoot -- the udev root directory to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetUDEVConfig(mp_string &strUDEVConfDir, mp_string &strUDEVRoot)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsSpecial = MP_TRUE;
    mp_string::iterator iter;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get UDEV config.");

    mp_string strParam = "/etc/udev/udev.conf|grep udev_rules|grep -v '#'|awk -F \"=\" '{print $2}' 2>/dev/null";
    vector<mp_string> vecResult;

    //执行cat操作必须在root用户下面执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CAT, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get UDEV config dir failed, iRet %d.", iRet);
        return iRet;
    }

    if(vecResult.empty())
    {
        strUDEVConfDir = "/etc/udev/rules.d/";
    }
    else
    {
        strUDEVConfDir = vecResult.front();
        for (iter = strUDEVConfDir.begin();iter != strUDEVConfDir.end();)
        {
            bIsSpecial = ( *iter == '\"' ||*iter == ' ');
            if (bIsSpecial)
            {
                iter = strUDEVConfDir.erase(iter);
            }
            else
            {
                iter++;
            }
        }
    }
    
    vecResult.clear();
    strParam = "/etc/udev/udev.conf|grep udev_root|grep -v '#'|awk -F \"=\" '{print $2}'  2>/dev/null";
    //执行cat操作必须在root用户下面执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CAT, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get UDEV root path failed, iRet %d.", iRet);
        return iRet;
    }
    if(vecResult.empty())
    {
        strUDEVRoot = "/dev/";
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,
            "Get UDEV config succ, udev root is:%s, udev rules file dir is:%s.", strUDEVRoot.c_str(), strUDEVConfDir.c_str());
        return MP_SUCCESS;
    }

    strUDEVRoot = vecResult.front();
    for (iter = strUDEVRoot.begin();iter != strUDEVRoot.end();)
    {
        bIsSpecial = ( *iter == '\"' ||*iter == ' ');
        if (bIsSpecial)
        {
            iter = strUDEVRoot.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO,
            "Get UDEV config succ, udev root is:%s, udev rules file dir is:%s.", strUDEVRoot.c_str(), strUDEVConfDir.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : get udev information
Input        : strUdevRulesFileDir -- the directory of udev directory
                strUdevName -- the name of udev device
                strUdevResult -- the udev result(WWN) of udev device
Output       : 
                strUdevDeviceRecord -- the udev config string to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::GetUDEVInfo(mp_string strUdevRulesFileDir, mp_string strUdevName, mp_string strUdevResult,
    mp_string &strUdevDeviceRecord)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<string> lstParser;
    mp_string strCommand;
    mp_string strUdevRulesFile;

    vector<mp_string>::iterator itetemp;
    vector<mp_string> lstUdevRulesFileList;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get UDEV (%s) info.", strUdevName.c_str());

    mp_string strParam = strUdevRulesFileDir + "|awk -F \" \" '{print $NF}'|grep '.rules'  2>/dev/null";

    //执行cat操作必须在root用户下面执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &lstUdevRulesFileList);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get (%s) udev rules files list failed, iRet %d.", strUdevName.c_str(), iRet);
        return iRet;
    }

    for (itetemp = lstUdevRulesFileList.begin(); itetemp!= lstUdevRulesFileList.end(); itetemp++)
    {
        strUdevRulesFile = strUdevRulesFileDir+(*itetemp);

        strParam = strUdevRulesFile + " | grep -v '#' | grep " + strUdevName + " | grep " + strUdevResult + " 2>/dev/null";
        //执行cat操作必须在root用户下面执行
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_CAT, strParam, &lstParser);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get (%s) udev rules files information info failed, ulRetCode code %d",
                strUdevName.c_str(), iRet);

            return iRet;
        }
        else
        {
            if (!(lstParser.empty()))
            {
                strUdevDeviceRecord = lstParser.front();
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get (%s) udev rules record information successful %s",
                    strUdevName.c_str(), strUdevDeviceRecord.c_str());

                return MP_SUCCESS;
             }
        }
        lstParser.clear();
    }

    if (itetemp == lstUdevRulesFileList.end())
    {
        strUdevDeviceRecord = "";
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Udev record information of device(%s) is not exist.", strUdevName.c_str());
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get UDEV info(%s) succ.", strUdevName.c_str());
    return MP_SUCCESS;
}

#endif
/*************************end   获取Oracle数据库使用LUN列表**********************/

/*************************begin 检测归档目录使用率**********************/
/*------------------------------------------------------------ 
Description  : build the script parameter for check threshold 
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void COracle::BuildCheckThresholdScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ARCHIVETHRESHOLD) + stDBInfo.strArchThreshold + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
}

/*------------------------------------------------------------ 
Description  : execute srcipt to check threshold 
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::CheckArchiveThreshold(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin check oracle database archive threshold.");
    BuildCheckThresholdScriptParam(stDBInfo, strParam);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_CHECK_ARCHIVE), strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_CHECKARCHIVETHRESHOLD, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check oracle database(%s-%s) archive threshold failed, iRet %d",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Check oracle database archive threshold succ.");
    return MP_SUCCESS;
}

/*************************end 检测归档目录使用率?*********************/

/*************************begin 测试Oracle数据库连接**********************/
/*------------------------------------------------------------ 
Description  : build the script parameter for test database
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void COracle::BuildTestScriptParam(oracle_db_info_t stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
}

/*------------------------------------------------------------ 
Description  : execute srcipt to test oracle database
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::Test(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin test oracle database.");
    BuildTestScriptParam(stDBInfo, strParam);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_TEST), strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_TESTORACLE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Test oracle database(%s-%s) failed, iRet %d",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test oracle database succ.");
    return MP_SUCCESS;

}
/*************************end 测试Oracle数据库连接***********************/

/*************************begin冻结Oracle数据库**********************/
/*------------------------------------------------------------ 
Description  : build the script parameter for freeze or thaw database
Input        : stDBInfo -- the database structor
                strFrushType -- freeze type {freeze|thaw}
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void COracle::BuildConsistentScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam, mp_string strFrushType)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_FRUSHTYPE) + strFrushType;
}

/*------------------------------------------------------------ 
Description  : execute srcipt to freeze database
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::Freeze(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin freeze oracle database.");
    BuildConsistentScriptParam(stDBInfo, strParam, ORACLE_SCRIPTPARAM_FREEZEDB);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_CONSISTENT), strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_FREEZEORACLE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Freeze oracle database(%s-%s) failed, iRet %d",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze oracle database succ.");
    return MP_SUCCESS;
}

/*************************end冻结Oracle数据库**********************/

/*************************begin 解冻Oracle数据库**********************/
/*------------------------------------------------------------ 
Description  : execute srcipt to thaw database
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::Thaw(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin thaw oracle database.");
    BuildConsistentScriptParam(stDBInfo, strParam, ORACLE_SCRIPTPARAM_THAWDB);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_CONSISTENT), strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_THAWORACLE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Thaw oracle database(%s-%s) failed, iRet %d",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw oracle database succ.");
    return MP_SUCCESS;
}
/*************************end解冻Oracle数据库**********************/

/*************************begin 强制归档Oracle数据库**********************/
/*------------------------------------------------------------ 
Description  : execute srcipt to archive database
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::ArchiveDB(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin archive oracle database.");
    BuildConsistentScriptParam(stDBInfo, strParam, ORACLE_SCRIPTPARAM_ARCHIVEDB);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_CONSISTENT), strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //Oracle下获取数据库LUN信息需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_ARCHIVEORACLE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Archive oracle database(%s-%s) failed, iRet %d",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Archive oracle database succ.");
    return MP_SUCCESS;
}
/*************************end强制归档Oracle数据库**********************/

/***********************begin删除Oracle数据库归档日志***********************/
/*------------------------------------------------------------ 
Description  : 构造脚本参数
Input        : stDBInfo -- 数据库信息
               truncTime -- 删除归档日志的截止时间，此时间之前的归档日志都删除
Output       : strParam -- 脚本参数字符串
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::BuildTruncateLogScriptParam(oracle_db_info_t &stDBInfo, mp_time truncTime, mp_string &strParam)
{
	//CodeDex误报，UNUSED_VALUE
    mp_tm* pTime = NULL;
    mp_tm stTime;
    pTime = CMpTime::LocalTimeR(&truncTime, &stTime);
    if (NULL == pTime)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Convert truncTime(%lld) to localTime failed", truncTime);
        return ERROR_COMMON_INVALID_PARAM;
    }
    std::ostringstream strBuf;
    strBuf << ORACLE_SCRIPTPARAM_INSTNAME << stDBInfo.strInstName << STR_COLON \
            << ORACLE_SCRIPTPARAM_DBUSERNAME << stDBInfo.strDBUsername << STR_COLON \
            << ORACLE_SCRIPTPARAM_DBPASSWORD << stDBInfo.strDBPassword << STR_COLON \
            << ORACLE_SCIPRTPARAM_TRUNCATE_LOG_TIME << (pTime->tm_year + 1900) << STR_DASH \
            << (pTime->tm_mon + 1) << STR_DASH << pTime->tm_mday << STR_DASH << pTime->tm_hour << STR_DASH \
            << pTime->tm_min << STR_DASH << pTime->tm_sec << STR_COLON \
            << ORACLE_SCRIPTPARAM_FRUSHTYPE << ORACLE_SCRIPTPARAM_TRUNCATEARCHIVELOG;
    
    strParam = strBuf.str();
    pTime = NULL;
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 删除Oracle实例中的归档日志
Input        : stDBInfo -- 数据库信息
               truncTime -- 删除归档日志的截止时间，此时间之前的归档日志都删除
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::TruncateArchiveLog(oracle_db_info_t &stDBInfo, mp_time truncTime)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin truncate archivelogs of oracle database %s.", stDBInfo.strInstName.c_str());
    iRet = BuildTruncateLogScriptParam(stDBInfo, truncTime, strParam);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Build truncate archivelogs script param failed, iRet %d", iRet);
        return iRet;
    }

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_CONSISTENT), strParam, NULL);
    if (iRet != MP_SUCCESS)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        iRet = iNewRet;
    }
#else
    //Oracle下需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_TRUNCATEARCHIVELOGORACLE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Truncate archivelogs of oracle database(%s) failed, iRet %d",
            stDBInfo.strInstName.c_str(), iRet);
    }
#endif

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End truncate archivelogs of oracle database %s.", stDBInfo.strInstName.c_str());
    return iRet;
}
/*************************end删除Oracle数据库归档日志**********************/



/*************************begin 获取是否是CDB**********************/
/*------------------------------------------------------------ 
Description  : build the script parameter for check if oracle is cdb
Input        : stDBInfo --the structor of database infomation 
Output       : strParam -- string of script parameter
Return       : MP_SUCCESS -- success  
               not MP_SUCCESS -- failed，return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void COracle::BuildCheckCDBScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
}


/*------------------------------------------------------------ 
Description  : execute script to check if oracle is cdb
Input         : stDBInfo -- the structor of database infomation
Output       : iCDBType  -- the type code of oracle cdb
Return       : MP_SUCCESS -- success 
               not MP_SUCCESS -- failed ,  return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::CheckCDB(oracle_db_info_t &stDBInfo, mp_int32& iCDBType)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    mp_int32 iCDBTypeTmp = 0;
    vector<mp_string> vecResult;


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin check CDB of oracle database %s.", stDBInfo.strInstName.c_str());
    BuildCheckCDBScriptParam(stDBInfo,  strParam);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_CHECK_CDB), strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check CDB of oracle database(%s) failed, iRet %d",
            stDBInfo.strInstName.c_str(), iRet);
        return iNewRet;
    }
#else
    //Oracle下需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLECHECKCDB, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check CDB of oracle database(%s) failed, iRet %d",
            stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }
#endif
    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of get oracle archive log mode is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }

    iCDBTypeTmp = atoi(vecResult.front().c_str());
    iCDBType = (0 == iCDBTypeTmp ? ORACLE_TYPE_NON_CDB: ORACLE_TYPE_CDB);
#ifdef WIN32
    if (iCDBType == ORACLE_TYPE_CDB)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Windows does't support oracle CDB.");
        return ERROR_COMMON_FUNC_UNIMPLEMENT;
    }
#endif

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End check CDB of oracle database %s.", stDBInfo.strInstName.c_str());
    return iRet;
}
/*************************end获取是否是CDB**********************/


/*************************begin 启动ASM实例**********************/
/*------------------------------------------------------------ 
Description  : build the script parameter for start asm instance
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void COracle::BuildStartASMScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMDISKGROUPS) + stDBInfo.strASMDiskGroup + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ACTION) + "0";
}

/*------------------------------------------------------------ 
Description  : execute srcipt to start asm instance
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::StartASMInstance(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start ASM instance in script.");
    BuildStartASMScriptParam(stDBInfo, strParam);
#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_ASMACTION), strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //启动ASM实例需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STARTASMINSTANCE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "start ASM instance(%s) in script, iRet %d",
            stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start ASM instance in script succ.");
    return MP_SUCCESS;
}
/*************************end 启动ASM实例**********************/

/*************************begin停止ASM实例**********************/
/*------------------------------------------------------------ 
Description  : build the script parameter for stop asm instance
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void COracle::BuildStopASMScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ACTION) + "1";

}

/*------------------------------------------------------------ 
Description  : execute srcipt to stop asm instance
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::StopASMInstance(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin stop ASM instance in script.");
    BuildStopASMScriptParam(stDBInfo, strParam);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_ASMACTION), strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //启动ASM实例需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STOPASMINSTANCE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop ASM instance(%s) in script, iRet %d",
            stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop ASM instance in script succ.");
    return MP_SUCCESS;
}
/*************************end 停止ASM实例**********************/

/*************************begin 启动oracle实例**********************/
/*------------------------------------------------------------ 
Description  : build the script parameter for start oracle database
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void COracle::BuildStartOracleScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam)
{
    ostringstream oss;
    oss << stDBInfo.iIncludeArchLog;
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ISASM) + stDBInfo.strIsASM + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMDISKGROUPS) + stDBInfo.strASMDiskGroup + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCIPRTPARAM_IS_INCLUDE_ARCH) + oss.str() + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ACTION) + "0";
}

/*------------------------------------------------------------ 
Description  : execute srcipt to start oracle database
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::StartOracleInstance(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start oracle instance in script.");
    BuildStartOracleScriptParam(stDBInfo, strParam);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_DB_ACTION), strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //启动ASM实例需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STARTORACLEDB, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "start oracle instance(%s) in script, iRet %d",
            stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start oracle instance in script succ.");
    return MP_SUCCESS;
}

/*************************end 启动oracle实例**********************/

/*************************begin 停止oracle实例**********************/
/*------------------------------------------------------------ 
Description  : build the script parameter for stop oracle database
Input        : stDBInfo -- the database structor
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void COracle::BuildStopOracleScriptParam(oracle_db_info_t &stDBInfo, mp_string &strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ISASM) + stDBInfo.strIsASM + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ASMPASSWOD) + stDBInfo.strASMPassword + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome + mp_string(NODE_COLON)
        + mp_string(ORACLE_SCRIPTPARAM_ACTION) + "1";
}

/*------------------------------------------------------------ 
Description  : execute srcipt to stop oracle database
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::StopOracleInstance(oracle_db_info_t &stDBInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin stop oracle instance in script.");
    BuildStopOracleScriptParam(stDBInfo, strParam);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_DB_ACTION), strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //启动ASM实例需要切换到Oracle用户下，必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STOPORACLEDB, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop oracle instance(%s) in script, iRet %d",
            stDBInfo.strInstName.c_str(), iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop oracle instance in script succ.");
    return MP_SUCCESS;
}

/*************************end 停止oracle实例**********************/


/*************************begin 查询oracle冻结状态**********************/
/*------------------------------------------------------------ 
Description  : execute srcipt to query oracle freeze status
Input        : stDBInfo -- the database structor
Output       : 
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 COracle::QueryFreezeState(oracle_db_info_t &stDBInfo, mp_int32 &iFreezeState)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query oracle freeze state.");
    BuildConsistentScriptParam(stDBInfo, strParam, ORACLE_SCRIPTPARAM_FREEZESTATUS);

#ifdef WIN32
    //windows下调用脚本
    iRet = CSystemExec::ExecScript(mp_string(WIN_ORACLE_CONSISTENT), strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        CErrorCodeMap errorCode;
        mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
             "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    //oracle下获取实例状态命令必须在root或者实例用户下执行
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_FREEZEORACLE, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif

    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute get oracle freeze state script failed, iRet %d", iRet);
        return iRet;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of get oracle freeze state is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }

    iFreezeState = atoi(vecResult.front().c_str());
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query oracle freeze state(%d) succ.", iFreezeState);
    return MP_SUCCESS;
}
/*************************end 查询oracle冻结状态**********************/



