/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "array/Array.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/RootCaller.h"
#include "common/Defines.h"
#include "common/String.h"
#include "common/UniqueId.h"
#include "common/Path.h"
#include "common/SystemExec.h"

CArray::CArray()
{
}

CArray::~CArray()
{
}

//windows平台
#ifdef WIN32
/*------------------------------------------------------------ 
Description  : 打开设备
Input        : 
Output       : 
Return       :  MP_SUCCESS---打开成功
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CArray::OpenDev(mp_string& strDev, FILE_HANDLE& handle)
{
    /*
    DWORD dwShareMode  = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD dwAccessMode = GENERIC_READ | GENERIC_WRITE;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin open dev, dev %s.", strDev.c_str());
    HANDLE handletemp = CreateFile(pszDeviceName, dwAccessMode, dwShareMode, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == handletemp)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open dev failed, dev %s, errno[%d]: %s.", strDev.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_ARRAY_OPEN_DEV_FAILED;
    }

    handle = handletemp;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open dev succ.");
    */
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 得到设备handle
Input        : 
Output       : 
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CArray::GetDevHandle(const mp_string& pszDeviceName, FILE_HANDLE& pHandle)
{
    DWORD dwShareMode  = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD dwAccessMode = GENERIC_READ | GENERIC_WRITE ;

    //调用CreateFile得到设备handle
    FILE_HANDLE handletemp = CreateFile(pszDeviceName.c_str(),
                                   dwAccessMode,
                                   dwShareMode,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL);
    if (INVALID_HANDLE_VALUE == handletemp)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateFile handle is invalid,device(%s)", pszDeviceName.c_str());

        return MP_FAILED;
    }
    pHandle = handletemp;

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 得到设备SCSI地址信息
Input        : fHandle---SCSI文件handle
Output       : pstDevInfo---硬盘设备信息
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CArray::GetDevSCSIAddress(FILE_HANDLE fHandle, win_dev_info_t& pstDevInfo)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin GetDiskSCSIAddress.");

    mp_int32 iStatus = 0;
    mp_ulong ulReturn = 0;
    SCSI_ADDRESS stScsiAddress;

    stScsiAddress.Length = sizeof(SCSI_ADDRESS);

    iStatus = DeviceIoControl(fHandle,
                              IOCTL_SCSI_GET_ADDRESS,
                              &stScsiAddress,
                              sizeof(stScsiAddress),
                              &stScsiAddress,
                              sizeof(stScsiAddress),
                              &ulReturn,
                              NULL);
    if (!iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDiskSCSIAddress io error(%d)", iStatus);

        return MP_FAILED;
    }

    pstDevInfo.iPathId = stScsiAddress.PathId;
    pstDevInfo.iPortId = stScsiAddress.PortNumber;
    pstDevInfo.iScsiId = stScsiAddress.TargetId;
    pstDevInfo.iLunId  = stScsiAddress.Lun;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "GetDiskSCSIAddress io end");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :设置设备SCSI缓存值
Input        :  
Output       :  
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CArray::SetScsiBufValues(scsi_pass_through_with_buff_t& stSCSIPass, mp_ulong& ulLength, mp_uchar ucCdb, mp_uchar ucCmd)
{
    stSCSIPass.pt.Length = sizeof(SCSI_PASS_THROUGH);
    stSCSIPass.pt.CdbLength = 6;
    stSCSIPass.pt.SenseInfoLength = 32;
    stSCSIPass.pt.DataIn = SCSI_IOCTL_DATA_IN;
    stSCSIPass.pt.DataTransferLength = 0xff;
    stSCSIPass.pt.TimeOutValue = 60;
    stSCSIPass.pt.DataBufferOffset = offsetof(scsi_pass_through_with_buff_t, aucData);
    stSCSIPass.pt.SenseInfoOffset = offsetof(scsi_pass_through_with_buff_t, aucSense);
    stSCSIPass.pt.Cdb[0] = SCSIOP_INQUIRY;
    stSCSIPass.pt.Cdb[1] = ucCdb;
    stSCSIPass.pt.Cdb[2] = ucCmd;
    stSCSIPass.pt.Cdb[3] = 0x00;
    stSCSIPass.pt.Cdb[4] = 0xff;
    stSCSIPass.pt.Cdb[5] = 0x00;

    ulLength = offsetof(scsi_pass_through_with_buff_t, aucData) + stSCSIPass.pt.DataTransferLength;
}
//非windows平台
#else

/*------------------------------------------------------------ 
Description  : 打开设备
Input        :strDev -- 设备名
Output       : iDevFd -- 设备描述符
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::OpenDev(mp_string& strDev, mp_int32& iDevFd)
{
    mp_int32 fd = 0;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin open dev, dev %s.", strDev.c_str());
#ifdef LINUX
    //scsi设备
    fd = open(strDev.c_str(), O_RDONLY | O_NONBLOCK);
    if (MP_SUCCESS > fd)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi dev failed, dev %s, errno[%d]: %s.", strDev.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return ERROR_COMMON_OPER_FAILED;
    }

    iDevFd = fd;
#else //HP/AIX
    //scsi设备
    fd = open(strDev.c_str(), O_RDWR | O_NONBLOCK);
    if(MP_SUCCESS > fd)
    {
        //COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open scsi dev failed, dev %s, errno[%d]: %s.", strDev.c_str(),
        //    errno, strerror(errno));
        iErr = GetOSError();
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open scsi dev failed, dev %s, errno[%d]: %s.", strDev.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        //其他设备
        fd = open(strDev.c_str(), O_RDONLY | O_NONBLOCK);
        if (MP_SUCCESS > fd)
        {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open dev failed, dev %s, errno[%d]: %s.", strDev.c_str(),
                iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return ERROR_COMMON_OPER_FAILED;
        }
    }
#endif
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open dev succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :获取磁盘所属阵列信息
Input        :strDevice -- 设备名
Output       : strVendor -- 阵列型号
                   strVendor -- 阵列厂商
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetDiskArrayInfo(mp_string& strDevice, mp_string& strVendor, mp_string& strProduct)
{
#ifdef LINUX
    mp_char acBuffer[DISK_BYTE_OF_SECTOR] = {0};
#elif defined HP_UX_IA
    mp_char  acBuffer[DATA_LEN_256] = {0};
#elif defined AIX
    mp_char acBuffer[BUFFER_LEN_36] = {0};
#elif defined SOLARIS
    mp_char  acBuffer[DATA_LEN_256] = {0};
#endif
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};
    mp_char pszProduct[MAX_PRODUCT_LEN]={0};
    mp_char pszVendor[MAX_VENDOR_LEN]={0};
    //打开scsi设备
    mp_int32 iFd = open(strDevice.c_str(), O_RDWR | O_NONBLOCK);
    if (0 > iFd)
    {
        iFd = open(strDevice.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (0 > iFd)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi device(%s) failed, errno[%d]: %s.",
            strDevice.c_str(), iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    mp_int32 iRet = GetVendorAndProduct(iFd, acBuffer);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk array info failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }
    //CodeDex误报，FORTIFY.Path_Manipulation
    CHECK_CLOSE_FD(memcpy_s(pszVendor, MAX_VENDOR_LEN, acBuffer + 8, 8));
    CHECK_CLOSE_FD(memcpy_s(pszProduct, MAX_PRODUCT_LEN, acBuffer + 16, 16));
    close(iFd);
    strVendor = pszVendor;
    strProduct = pszProduct;

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取80和83页信息
Input        :iFd -- 设备描述符
Output       : ucCmd -- 80,83页标识
                   aucBuffer -- 80,83页信息
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetDiskPage(mp_int32 iFd, mp_uchar ucCmd, mp_uchar *aucBuffer)
{
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    //初始化CDB
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x01;
    aucCdb[2] = ucCmd;
    aucCdb[3] = 0;
    aucCdb[4] = 255;

#if defined LINUX
    sg_io_hdr_t io_hdr;
    mp_uchar aucSense[SCSI_MAX_SENSE_LEN] = {0};

    //初始化sg_io_hdr_t
    CHECK_NOT_OK(memset_s(&io_hdr, sizeof(sg_io_hdr_t), 0, sizeof(sg_io_hdr_t)));
    io_hdr.interface_id = 'S';
    io_hdr.cmdp = aucCdb;
    io_hdr.cmd_len = CDB6GENERIC_LENGTH;

    io_hdr.sbp = aucSense;
    io_hdr.mx_sb_len = SCSI_MAX_SENSE_LEN;

    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;

    io_hdr.dxferp = aucBuffer;
    io_hdr.dxfer_len = DATA_LEN_256;

    io_hdr.timeout = SCSI_CMD_TIMEOUT_LINUX;

    mp_int32 iRet = ioctl(iFd, SG_IO, &io_hdr);
    if(MP_SUCCESS != iRet || 0 != io_hdr.status)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, ret %d, io_status is %d.", iRet, io_hdr.status);
        return MP_FAILED;
    }

#elif defined AIX
    struct sc_passthru ioCmd;
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(ioCmd), 0, sizeof(ioCmd)));

    //设置CDB
    CHECK_NOT_OK(memcpy_s(ioCmd.scsi_cdb, sizeof(ioCmd.scsi_cdb), aucCdb, sizeof(aucCdb)));

    //设置CDB的长度
    ioCmd.command_length = sizeof(aucCdb);

    //设置超时时间
    ioCmd.timeout_value = SCSI_CMD_TIMEOUT_AIX;

    //传输数据的缓冲区
    ioCmd.buffer = (mp_char*)aucBuffer;
    //传输数据的长度
    ioCmd.data_length = BUFFER_LEN_255;

    //传输数据的方向，读还是写
    ioCmd.flags = B_READ;
    ioCmd.flags |= SC_ASYNC;

    //ioctl发送SCSI数据
    mp_int32 iRet = ioctl(iFd, DK_PASSTHRU, &ioCmd);
    if (0 != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }

#elif defined HP_UX_IA
    struct sctl_io ioCmd;

    //初始化sctl_io
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(struct sctl_io), 0, sizeof(struct sctl_io)));
    ioCmd.flags = B_READ;
    ioCmd.cdb_length = CDB6GENERIC_LENGTH;
    CHECK_NOT_OK(memcpy_s(ioCmd.cdb, sizeof(ioCmd.cdb), aucCdb, ioCmd.cdb_length));
    ioCmd.data= aucBuffer;
    ioCmd.data_length = DATA_LEN_256;
    ioCmd.max_msecs = SCSI_CMD_TIMEOUT_HP;

    mp_int32 iRet = ioctl(iFd, SIOC_IO, &ioCmd);
    if(MP_SUCCESS > iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }
#elif defined SOLARIS
    aucCdb[5] = 0x00;
    struct uscsi_cmd ioCmd;
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(struct uscsi_cmd), 0, sizeof(struct uscsi_cmd)));
    ioCmd.uscsi_flags = USCSI_READ;

    //CDB
    ioCmd.uscsi_cdblen = 6;
    ioCmd.uscsi_cdb = (mp_char*)aucCdb;

    ioCmd.uscsi_bufaddr = (mp_char*)aucBuffer;
    ioCmd.uscsi_buflen = 256;

    ioCmd.uscsi_timeout = SCSI_CMD_TIMEOUT_SOLARIS;
    mp_int32 iRet = ioctl(iFd, USCSICMD, &ioCmd);
    if (MP_SUCCESS > iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取阵列信息
Input        :iFd -- 设备描述符
Output       : aucBuffer -- 阵列厂商和型号信息字符串
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetVendorAndProduct(mp_int32 iFd, mp_char *acBuffer)
{
    mp_uchar aucCdb[CDB6GENERIC_LENGTH] = {0};
    mp_int32 iRet = MP_SUCCESS;
#if defined LINUX
    struct sg_io_hdr stScsiCmd;

    CHECK_NOT_OK(memset_s(&stScsiCmd, sizeof(struct sg_io_hdr), 0, sizeof(struct sg_io_hdr)));
    stScsiCmd.interface_id = 'S';
    stScsiCmd.dxfer_direction = SG_DXFER_FROM_DEV;

    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0;
    aucCdb[2] = 0;
    aucCdb[3] = 0;
    aucCdb[4] = 0xff;
    aucCdb[5] = 0;

    //下发给设备的命令
    stScsiCmd.cmdp = aucCdb;
    stScsiCmd.cmd_len = CDB6GENERIC_LENGTH;

    CHECK_NOT_OK(memset_s(acBuffer, DISK_BYTE_OF_SECTOR, 0, DISK_BYTE_OF_SECTOR));
    stScsiCmd.dxferp = acBuffer;
    stScsiCmd.dxfer_len = DISK_BYTE_OF_SECTOR;

    //下发ioctl的超时时间
    stScsiCmd.timeout = SCSI_CMD_TIMEOUT_LINUX;

    //把构造好的命令通过ioctl下发
    iRet = ioctl(iFd, SG_IO, &stScsiCmd);
    if (0 > iRet || 0 != stScsiCmd.status)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, ret %d.", iRet);
        return MP_FAILED;
    }

#elif defined AIX
    mp_uint32 uiRetryCount = 3;
    mp_uint32 uiRetryIndex = 0;

    struct sc_passthru ioCmd;
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(ioCmd), 0, sizeof(ioCmd)));

    // CDB
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x0;
    aucCdb[2] = 0x0;
    aucCdb[4] = (mp_uchar)BUFFER_LEN_36;

    //设置CDB
    CHECK_NOT_OK(memcpy_s(ioCmd.scsi_cdb, sizeof(ioCmd.scsi_cdb), aucCdb, sizeof(aucCdb)));

    //设置CDB的长度
    ioCmd.command_length = sizeof(aucCdb);
    //设置超时时间
    ioCmd.timeout_value = SCSI_CMD_TIMEOUT_AIX;
    //传输数据的缓冲区
    ioCmd.buffer = acBuffer;
    //传输数据的长度
    ioCmd.data_length = BUFFER_LEN_36;
    //传输数据的方向，读还是写
    ioCmd.flags = B_READ;
    ioCmd.flags |= SC_ASYNC;

    //ioctl发送SCSI数据,若发送失败，则重试
    for (uiRetryIndex = 0; uiRetryIndex < uiRetryCount; uiRetryIndex++)
    {
        iRet = ioctl(iFd, DK_PASSTHRU, &ioCmd);
        if (0 != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, ret %d.", iRet);
        }
        else
        {
            break;
        }

        DoSleep(500);
    }

    if (0 != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, ret = %d.", iRet);
        return MP_FAILED;
    }

#elif defined HP_UX_IA
    struct sctl_io ioCmd;

    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(struct sctl_io), 0, sizeof(struct sctl_io)));
    ioCmd.flags = B_READ;

    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0;
    aucCdb[2] = 0;
    aucCdb[3] = 0;
    aucCdb[4] = 0xff;
    aucCdb[5] = 0;

    //CDB
    ioCmd.cdb_length = CDB6GENERIC_LENGTH;
    CHECK_NOT_OK(memcpy_s(ioCmd.cdb, sizeof(ioCmd.cdb), aucCdb, ioCmd.cdb_length));

    ioCmd.data= acBuffer;
    ioCmd.data_length = DATA_LEN_256;

    ioCmd.max_msecs = SCSI_CMD_TIMEOUT_HP;

    //把构造好的命令通过ioctl下发
    iRet = ioctl(iFd, SIOC_IO, &ioCmd);
    if (0 > iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }
#elif defined SOLARIS
    struct uscsi_cmd ioCmd;
    aucCdb[0] = SCSIOP_INQUIRY;
    aucCdb[1] = 0x00;
    aucCdb[2] = 0x00;
    aucCdb[3] = 0x00;
    aucCdb[4] = 0xff;
    aucCdb[5] = 0x00;
    CHECK_NOT_OK(memset_s(&ioCmd, sizeof(struct uscsi_cmd), 0, sizeof(struct uscsi_cmd)));
    ioCmd.uscsi_flags = USCSI_READ;

    //CDB
    ioCmd.uscsi_cdblen = 6;
    ioCmd.uscsi_cdb = (mp_char*)aucCdb;

    ioCmd.uscsi_bufaddr = (mp_char*)acBuffer;
    ioCmd.uscsi_buflen = 256;

    ioCmd.uscsi_timeout = SCSI_CMD_TIMEOUT_SOLARIS;
    iRet = ioctl(iFd, USCSICMD, &ioCmd);
    if(0 > iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed, iRet = %d.", iRet);
        return MP_FAILED;
    }
#endif
    return MP_SUCCESS;
}
#endif

/*------------------------------------------------------------ 
Description  :获取 通道的设备列表
Input        :strDevice -- 设备名称
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetHostLunIDList(mp_string& strDevice, vector<mp_int32>& vecHostLunID)
{
#ifdef LINUX 
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};
    mp_int32 iFd = -1;
    mp_uchar sense_b[32] = {0,};
    static mp_uchar gaucResponseBuff[MAX_RLUNS_BUFF_LEN] = {0,};
    mp_char rlCmdBlk[12] = {0xa0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    struct sg_io_hdr io_hdr;
    mp_uint32 mx_resp_len = DEF_RLUNS_BUFF_LEN;
    mp_uint32 list_len = 0, lunID = 0;
    mp_int32 i = 0, off = 8, lunCount = 0;
    
    //打开scsi设备
    mp_int32 iRet = OpenDev(strDevice, iFd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    //构造CDB
    rlCmdBlk[6] = (mp_uchar)((mx_resp_len >> 24) & 0xff);
    rlCmdBlk[7] = (mp_uchar)((mx_resp_len >> 16) & 0xff);
    rlCmdBlk[8] = (mp_uchar)((mx_resp_len >> 8) & 0xff);
    rlCmdBlk[9] = (mp_uchar)(mx_resp_len & 0xff);

    iRet = memset_s(&io_hdr, sizeof(struct sg_io_hdr), 0, sizeof(struct sg_io_hdr));
    if (EOK != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "memset io_hdr failed, ret %d.", iRet);
        close(iFd);
        return MP_FAILED;
    }
    io_hdr.interface_id = 'S';
    io_hdr.cmdp = (mp_uchar *)rlCmdBlk;
    io_hdr.cmd_len = sizeof(rlCmdBlk);

    io_hdr.sbp = sense_b;
    io_hdr.mx_sb_len = sizeof(sense_b);

    iRet = memset_s(gaucResponseBuff, sizeof(gaucResponseBuff), 0, sizeof(gaucResponseBuff));
    if (EOK != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "memset gaucResponseBuff, ret %d.", iRet);
        close(iFd);
        return MP_FAILED;
    }
    io_hdr.dxferp = (char *)gaucResponseBuff;
    io_hdr.dxfer_len = (int)mx_resp_len;
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;

    io_hdr.timeout = DEF_COM_TIMEOUT;

    iRet = ioctl(iFd, SG_IO, &io_hdr);
    if (0 != iRet)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi dev failed, dev %s, errno[%d]: %s.", strDevice.c_str(),
            iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        close(iFd);
        return MP_FAILED;
    }
    close(iFd);
    
    if (io_hdr.status || io_hdr.host_status || io_hdr.driver_status )
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "report lun failed. status:%d, host_status:%d, driver_status:%d\n",
            io_hdr.status, io_hdr.host_status, io_hdr.driver_status);
        return MP_FAILED;
    }

    //lint -e701
    list_len  = (mp_uint32)(gaucResponseBuff[0] << 24);
    list_len += (mp_uint32)(gaucResponseBuff[1] << 16);
    list_len += (mp_uint32)(gaucResponseBuff[2] << 8);
    list_len += (mp_uint32)(gaucResponseBuff[3]);
    //lint +e701
    lunCount = list_len / 8;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Lun list length = %d.", lunCount);
    if ((list_len + 8) > mx_resp_len)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "dev %s, too many luns.", strDevice.c_str());
        return MP_FAILED;
    }

    //lint -e701
    for (i = 0, off = 8; i < lunCount; i++)
    {
        // 参考多路径代码，off进行调整
        //CodeDex误报，CSEC_LOOP_ARRAY_CHECKING，数组下标不会越界
        off += 8;
        lunID  = (mp_uint32)(gaucResponseBuff[off - 8] << 8);
        lunID += (mp_uint32)(gaucResponseBuff[off - 7]);
        lunID += (mp_uint32)(gaucResponseBuff[off - 6] << 24);
        lunID += (mp_uint32)(gaucResponseBuff[off - 5] << 16);
        vecHostLunID.push_back((mp_int32)lunID);
    }
    //lint +e701
    return MP_SUCCESS;
#else  
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Not linux is not supported reporting lun list.");
    return MP_FAILED;
#endif    
}

/*------------------------------------------------------------ 
Description  :获取阵列信息
Input        :strDev -- 设备名
Output       : strproduct -- 阵列厂商
                   strvendor --型号
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetArrayVendorAndProduct(mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get array info of disk(%s).", strDev.c_str());
#ifdef WIN32
    mp_char acVendor[MAX_VENDOR_LEN] = {0};
    mp_char acProduct[MAX_PRODUCT_LEN] = {0};
    mp_char  acVersion[MAX_VVERSION_LEN] = {0};
    mp_ulong ulReturn = 0;
    mp_ulong ulLength = 0;
    mp_int32 iCheckRet = EOK;
    FILE_HANDLE hOpenDeviceRet;
    scsi_pass_through_with_buff_t stSCSIPass;

    iRet = GetDevHandle(strDev, hOpenDeviceRet);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevHandle open device failed, device(%s)", strDev.c_str());

        return MP_FAILED;
    }

    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_0, PAGE_CDB_0);
    iRet = DeviceIoControl(hOpenDeviceRet,
                             IOCTL_SCSI_PASS_THROUGH,
                             &stSCSIPass,
                             sizeof(SCSI_PASS_THROUGH),
                             &stSCSIPass,
                             ulLength,
                             &ulReturn,
                             FALSE);
    if (!iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetArrayVendorAndProduct io error, device(%s)", strDev.c_str());
        (mp_void)CloseHandle(hOpenDeviceRet);

        return MP_FAILED;
    }

    //厂商
    iCheckRet = memcpy_s(acVendor, sizeof(acVendor), stSCSIPass.aucData + 8, 8);
    if (EOK != iCheckRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memcpy_s failed, ret %d.", iCheckRet);
        (mp_void)CloseHandle(hOpenDeviceRet);

        return MP_FAILED;
    }
    strvendor = acVendor;
    //型号
    iCheckRet = memcpy_s(acProduct, sizeof(acProduct), stSCSIPass.aucData + 16, 16);
    if (EOK != iCheckRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memcpy_s failed, ret %d.", iCheckRet);
        (mp_void)CloseHandle(hOpenDeviceRet);

        return MP_FAILED;
    }
    strproduct = acProduct;
    //阵列V版本信息
    //iCheckRet = memcpy_s(acVersion, sizeof(acVersion), stSCSIPass.aucData + 33, 1);
    //if (EOK != iCheckRet)
    //{
    //    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memcpy_s failed, ret %d.", iCheckRet);
    //    (void)CloseHandle(hOpenDeviceRet);

     //   return MP_FAILED;
    //}
    //strVersion = acVersion;

    (mp_void)CloseHandle(hOpenDeviceRet);
#else //非WIN系统
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;
    mp_int32 iCount = 0;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_VENDORANDPRODUCT, strDev, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get vendor and product info of device failed.");
        return iRet;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get vendor and product info of device failed.");
        return MP_FAILED;
    }

    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        if(COUNT_VENDOR == iCount)
        {
            strvendor = *iter;
        }
        if(COUNT_PRODUCT == iCount)
        {
            strproduct = *iter;
        }
        iCount ++;
    }
#endif
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get array info of disk(%s) succ, vendor %s, product %s.",
        strDev.c_str(), strvendor.c_str(), strproduct.c_str());

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取00页信息
Input        :strDev -- 设备名
Output       : vecResul--VPD pages code
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetDisk00Page(mp_string& strDevice, vector<mp_string>& vecResult)
{
#ifdef WIN32
    scsi_pass_through_with_buff_t stSCSIPass;
    win_dev_info_t pstDevInfo;
    FILE_HANDLE fHandle;
    mp_ulong ulLength = 0;
    mp_ulong ulReturn = 0;
    mp_int32 iStatus = MP_SUCCESS;
    //CodeDex误报，FORTIFY.Path_Manipulation
    iStatus = memset_s(&pstDevInfo, sizeof(win_dev_info_t), 0, sizeof(win_dev_info_t));
    if (EOK != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iStatus);
        return MP_FAILED;
    }

    iStatus = GetDevHandle(strDevice, fHandle);
    if (MP_SUCCESS != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevHandle open device failed, device(%s)", strDevice.c_str());

        return MP_FAILED;
    }

    iStatus = GetDevSCSIAddress(fHandle, pstDevInfo);
    if (MP_SUCCESS != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevSCSIAddress failed, device(%s)", strDevice.c_str());
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_1, PAGE_00);
    stSCSIPass.pt.PathId = (mp_uchar)pstDevInfo.iPathId;
    stSCSIPass.pt.TargetId = (mp_uchar)pstDevInfo.iScsiId;
    stSCSIPass.pt.Lun = (mp_uchar)pstDevInfo.iLunId;

    iStatus = DeviceIoControl(fHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &stSCSIPass,
                             sizeof(SCSI_PASS_THROUGH),
                             &stSCSIPass,
                             ulLength,
                             &ulReturn,
                             FALSE);
    if (!iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDisk0x00Page io error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    mp_int32 validLen = stSCSIPass.aucData[3];
    if (0 == validLen)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDisk0x00Page Page Length error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    mp_char pageCode[MAX_PAGETCODE_LEN] = {0};
    mp_string tmp = "";
    for (int i = 1; i <= validLen; i++)
    {
        if (MP_SUCCESS != BinaryToAscii(pageCode, MAX_PAGETCODE_LEN, stSCSIPass.aucData, 3 + i, 1))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Binary to ascii failed.");
            (mp_void)CloseHandle(fHandle);
            return MP_FAILED;
        }
        tmp = pageCode;
        vecResult.push_back(tmp);
    }
    (mp_void)CloseHandle(fHandle);
#else //非WIN系统
#if (defined HP_UX_IA) || (defined LINUX)
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
#elif defined AIX || (defined SOLARIS) 
    mp_uchar aucBuffer[BUFFER_LEN_255] = {0};
#endif
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get 00 page of disk(%s).", strDevice.c_str());

    //打开scsi设备
    mp_int32 iFd = open(strDevice.c_str(), O_RDWR | O_NONBLOCK);
    if (0 > iFd)
    {
        iFd = open(strDevice.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (0 > iFd)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    mp_int32 iRet = GetDiskPage(iFd, PAGE_00, aucBuffer);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk 00Page failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }
    mp_int32 validLen = aucBuffer[3];
    if (0 == validLen)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Page length error, page len %u.", validLen);
        close(iFd);
        return MP_FAILED;
    }
    mp_char pageCode[MAX_PAGETCODE_LEN] = {0};
    mp_string tmp = "";
    for (int i = 1; i <= validLen; i++)
    {
        iRet = BinaryToAscii(pageCode, MAX_PAGETCODE_LEN, aucBuffer, 3 + i, 1);
        if (MP_SUCCESS!= iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Binary to ascii failed, ret %d.", iRet);
            close(iFd);
            return iRet;
        }
        tmp = pageCode;
        vecResult.push_back(tmp);
    }
    close(iFd);
#endif  //非WIN系统
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get 00 page of disk(%s) succ.",
        strDevice.c_str());

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取C8页信息
Input        :strDev -- 设备名
Output       : strLunID --LUN的ID
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetDiskC8Page(mp_string& strDevice, mp_string& strLunID)
{
#ifdef WIN32
    scsi_pass_through_with_buff_t stSCSIPass;
    win_dev_info_t pstDevInfo;
    FILE_HANDLE fHandle;
    mp_ulong ulLength = 0;
    mp_ulong ulReturn = 0;
    mp_int32 iStatus = MP_SUCCESS;
    //CodeDex误报，FORTIFY.Path_Manipulation
    iStatus = memset_s(&pstDevInfo, sizeof(win_dev_info_t), 0, sizeof(win_dev_info_t));
    if (EOK != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iStatus);
        return MP_FAILED;
    }

    iStatus = GetDevHandle(strDevice, fHandle);
    if (MP_SUCCESS != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevHandle open device failed, device(%s)", strDevice.c_str());

        return MP_FAILED;
    }

    iStatus = GetDevSCSIAddress(fHandle, pstDevInfo);
    if (MP_SUCCESS != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevSCSIAddress failed, device(%s)", strDevice.c_str());
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_1, PAGE_C8);
    stSCSIPass.pt.PathId = (mp_uchar)pstDevInfo.iPathId;
    stSCSIPass.pt.TargetId = (mp_uchar)pstDevInfo.iScsiId;
    stSCSIPass.pt.Lun = (mp_uchar)pstDevInfo.iLunId;

    iStatus = DeviceIoControl(fHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &stSCSIPass,
                             sizeof(SCSI_PASS_THROUGH),
                             &stSCSIPass,
                             ulLength,
                             &ulReturn,
                             FALSE);
    if (!iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDisk0xC8Page io error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    mp_uint32 validLen = VDS_HS_PAGE_BUF_LEN(stSCSIPass.aucData);
    if (validLen < C8_PAGE_WITH_LUN_ID_LEN)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Disk 0xC8 Page not contain lun id");
        (mp_void)CloseHandle(fHandle);
        return MP_SUCCESS;
    }
    mp_uint32 lunid = stSCSIPass.aucData[192];
    lunid = (lunid << 8) | (stSCSIPass.aucData[193]);
    lunid = (lunid << 8) | (stSCSIPass.aucData[194]);
    lunid = (lunid << 8) | (stSCSIPass.aucData[195]);
    mp_char acDevLUNID[MAX_LUNID_LEN] = {0};

    mp_int32 iCheckFailRet = SNPRINTF_S(acDevLUNID, MAX_LUNID_LEN, MAX_LUNID_LEN - 1, "%u", lunid);                                                             
    if (MP_FAILED == iCheckFailRet)                                                              
    {                                                                                             
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call SNPRINTF_S failed, ret %d.", iCheckFailRet); 
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;                                                                         
    }  

    strLunID = acDevLUNID;
    (mp_void)CloseHandle(fHandle);
#else //非WIN系统
#if (defined HP_UX_IA) || (defined LINUX)
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
#elif defined AIX || (defined SOLARIS)
    mp_uchar aucBuffer[BUFFER_LEN_255] = {0};
#endif
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get C8 page of disk(%s).", strDevice.c_str());

    //打开scsi设备
    mp_int32 iFd = open(strDevice.c_str(), O_RDWR | O_NONBLOCK);
    if (0 > iFd)
    {
        iFd = open(strDevice.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (0 > iFd)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    mp_int32 iRet = GetDiskPage(iFd, PAGE_C8, aucBuffer);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk C8Page failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }

    mp_uint32 validLen = VDS_HS_PAGE_BUF_LEN(aucBuffer);
    if (validLen < C8_PAGE_WITH_LUN_ID_LEN)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Disk 0xC8 Page not contain lun id");
        close(iFd);
        return MP_SUCCESS;
    }
    mp_uint32 lunid = aucBuffer[192];
    lunid = (lunid << 8) | (aucBuffer[193]);
    lunid = (lunid << 8) | (aucBuffer[194]);
    lunid = (lunid << 8) | (aucBuffer[195]);
    mp_char acDevLUNID[MAX_LUNID_LEN] = {0};
    iRet = SNPRINTF_S(acDevLUNID, MAX_LUNID_LEN, MAX_LUNID_LEN - 1, "%u", lunid);
    if (MP_FAILED == iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call SNPRINTF_S failed, ret %d.", iRet);
        close(iFd);
        return MP_FAILED;
    }
    
    strLunID = acDevLUNID;
    close(iFd);
#endif  //非WIN系统
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get C8 page of disk(%s) succ, LunID %s.",
        strDevice.c_str(), strLunID.c_str());

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :获取83页信息
Input        :strDev -- 设备名
Output       : strLunWWN -- LUN的WWN
                   strLunID --LUN的ID
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetDisk83Page(mp_string& strDevice, mp_string& strLunWWN, mp_string& strLunID)
{
#ifdef WIN32
    scsi_pass_through_with_buff_t stSCSIPass;
    win_dev_info_t pstDevInfo;
    FILE_HANDLE fHandle;
    mp_ulong ulLength = 0;
    mp_ulong ulReturn = 0;
    mp_int32 iStatus = MP_SUCCESS;
    mp_char acLUNWWN[MAX_WWN_LEN] = {0};
    //CodeDex误报，FORTIFY.Path_Manipulation
    iStatus = memset_s(&pstDevInfo, sizeof(win_dev_info_t), 0, sizeof(win_dev_info_t));
    if (EOK != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iStatus);
        return MP_FAILED;
    }

    iStatus = GetDevHandle(strDevice, fHandle);
    if (MP_SUCCESS != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevHandle open device failed, device(%s)", strDevice.c_str());

        return MP_FAILED;
    }

    iStatus = GetDevSCSIAddress(fHandle, pstDevInfo);
    if (MP_SUCCESS != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevSCSIAddress failed, device(%s)", strDevice.c_str());
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_1, PAGE_83);
    stSCSIPass.pt.PathId = (mp_uchar)pstDevInfo.iPathId;
    stSCSIPass.pt.TargetId = (mp_uchar)pstDevInfo.iScsiId;
    stSCSIPass.pt.Lun = (mp_uchar)pstDevInfo.iLunId;

    iStatus = DeviceIoControl(fHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &stSCSIPass,
                             sizeof(SCSI_PASS_THROUGH),
                             &stSCSIPass,
                             ulLength,
                             &ulReturn,
                             FALSE);
    if (!iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDisk0x83Page io error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    if (stSCSIPass.aucData[7] > MAX_SN_LEN)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDisk0x80Page Page Length error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    //WWN
    mp_uchar pucSnLength = stSCSIPass.aucData[7];

    if (MAX_WWN_LEN <= pucSnLength * 2)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDisk0x80Page Page Length error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    if (MP_SUCCESS != BinaryToAscii(acLUNWWN, MAX_WWN_LEN, stSCSIPass.aucData, 8, pucSnLength))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Binary to ascii failed.");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    strLunWWN = acLUNWWN;

    //LUN ID
    mp_uchar acHexData[MAX_LUNID_LEN] = {0};
    mp_char acDevLUNID[MAX_LUNID_LEN] = {0};

    if (MP_SUCCESS != ConvertLUNIDtoAscii(acHexData, MAX_LUNID_LEN, stSCSIPass.aucData + 20, 9))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Convert lun id to ascii failed.");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }
    (mp_void)HextoDec(acDevLUNID, (mp_char *)acHexData, MAX_LUNID_LEN);
    strLunID = acDevLUNID;
    (mp_void)CloseHandle(fHandle);
#else //非WIN系统
#if (defined HP_UX_IA) || (defined LINUX)
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
#elif defined AIX || (defined SOLARIS)
    mp_uchar aucBuffer[BUFFER_LEN_255] = {0};
#endif
    mp_char  acLUNWWN[MAX_WWN_LEN] = {0};
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get 83 page of disk(%s).", strDevice.c_str());

    //打开scsi设备
    mp_int32 iFd = open(strDevice.c_str(), O_RDWR | O_NONBLOCK);
    if (0 > iFd)
    {
        iFd = open(strDevice.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (0 > iFd)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    mp_int32 iRet = GetDiskPage(iFd, PAGE_83, aucBuffer);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk 83Page failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }

    mp_int32 iPageLen = aucBuffer[7];
    if (iPageLen * 2 > MAX_WWN_LEN - 1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Page length error, page len %d.", iPageLen);
        close(iFd);
        return MP_FAILED;
    }

    //华为阵列WWN
    iRet = BinaryToAscii(acLUNWWN, MAX_WWN_LEN, aucBuffer, 8, iPageLen);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Binary to ascii failed, ret %d.", iRet);
        close(iFd);
        return iRet;
    }
    strLunWWN = acLUNWWN;

    //LUN ID
    mp_uchar acHexData[MAX_LUNID_LEN] = {0};
    mp_char acDevLUNID[MAX_LUNID_LEN] = {0};

    iRet = ConvertLUNIDtoAscii(acHexData, MAX_LUNID_LEN, aucBuffer + 20, 9);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Convert lun id to ascii failed, ret %d.", iRet);
        close(iFd);
        return iRet;
    }

    close(iFd);
    (mp_void)HextoDec(acDevLUNID, (mp_char *)acHexData, MAX_LUNID_LEN);
    strLunID = acDevLUNID;
#endif  //非WIN系统
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get 83 page of disk(%s) succ, wwn %s, LunID %s.",
        strDevice.c_str(), strLunWWN.c_str(), strLunID.c_str());

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :获取80页信息
Input        :strDev -- 设备名
Output       : strSN --阵列SN
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetDisk80Page(mp_string& strDevice, mp_string& strSN)
{
#ifdef WIN32
    scsi_pass_through_with_buff_t stSCSIPass;
    win_dev_info_t pstDevInfo;
    FILE_HANDLE fHandle;
    mp_ulong ulLength = 0;
    mp_ulong ulReturn = 0;
    mp_int32 iStatus = MP_SUCCESS;
    mp_char acArraySN[MAX_ARRAY_SN_LEN + 1] = {0};
    //CodeDex误报，FORTIFY.Path_Manipulation
    iStatus = memset_s(&pstDevInfo, sizeof(win_dev_info_t), 0, sizeof(win_dev_info_t));
    if (EOK != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iStatus);
        return MP_FAILED;
    }

    iStatus = GetDevHandle(strDevice, fHandle);
    if (MP_SUCCESS != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevHandle open device failed, device(%s)", strDevice.c_str());

        return MP_FAILED;
    }

    iStatus = GetDevSCSIAddress(fHandle, pstDevInfo);
    if (MP_SUCCESS != iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDevSCSIAddress failed, device(%s)", strDevice.c_str());
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    SetScsiBufValues(stSCSIPass, ulLength, PAGE_CDB_1, PAGE_80);
    stSCSIPass.pt.PathId = (mp_uchar)pstDevInfo.iPathId;
    stSCSIPass.pt.TargetId = (mp_uchar)pstDevInfo.iScsiId;
    stSCSIPass.pt.Lun = (mp_uchar)pstDevInfo.iLunId;

    iStatus = DeviceIoControl(fHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &stSCSIPass,
                             sizeof(SCSI_PASS_THROUGH),
                             &stSCSIPass,
                             ulLength,
                             &ulReturn,
                             FALSE);
    if (!iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDisk0x80Page io error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    if (stSCSIPass.aucData[3] > VALUE_LEN)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDisk0x80Page Page Length error");
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    (mp_void)CloseHandle(fHandle);
    CHECK_NOT_OK(memcpy_s(acArraySN, sizeof(acArraySN), stSCSIPass.aucData + 4, MAX_ARRAY_SN_LEN));
    strSN = acArraySN;
#else   //非WIN系统
#if (defined HP_UX_IA) || (defined LINUX)
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
#elif defined AIX || (defined SOLARIS)
    mp_uchar aucBuffer[BUFFER_LEN_255] = {0};
#endif
    mp_char acDiskSn[VALUE_LEN] = {0};
    mp_char acSN[MAX_SN_LEN] = {0};

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to Get 80 page of disk(%s).", strDevice.c_str());
    //打开scsi设备
    mp_int32 iFd = open(strDevice.c_str(), O_RDWR | O_NONBLOCK);
    if (0 > iFd)
    {
        iFd = open(strDevice.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (0 > iFd)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    mp_int32 iRet = GetDiskPage(iFd, PAGE_80, aucBuffer);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Disk 80Page failed, iRet = %d.", iRet);
        close(iFd);
        return iRet;
    }

    //页长度
    mp_int32 iPageLen = aucBuffer[3];
    if (iPageLen > VALUE_LEN-1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Page length error, page len %d.", iPageLen);
        close(iFd);
        return MP_FAILED;
    }

    //序列号
    CHECK_CLOSE_FD(memcpy_s(acDiskSn, sizeof(acDiskSn), aucBuffer + 4, iPageLen));
    (mp_void)SNPRINTF_S(acSN, MAX_SN_LEN, MAX_SN_HW - 1, "%s", acDiskSn);
    close(iFd);
    strSN = acSN;
#endif //非WIN系统
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get 83 page of disk(%s) succ.", strDevice.c_str());

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :获取LUN信息
Input        :strDev -- 设备名
Output       : strLunWWN -- LUN的WWN
                   strLunID --LUN的ID
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetLunInfo(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get lun info of disk(%s).", strDev.c_str());
#ifdef WIN32
    iRet = GetDisk83Page(strDev, strLunWWN, strLunID);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 83 page of device failed.");
        return iRet;
    }
    //if support C8,get lun id from C8
    vector<mp_string> vecResult;
    iRet = GetDisk00Page(strDev, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 00 page of device failed.");
        return iRet;
    }
    mp_bool isSupport = IsSupportXXPage("c8", vecResult);
    if (MP_TRUE == isSupport)
    {
        iRet = GetDiskC8Page(strDev, strLunID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get C8 page of device failed.");
            return iRet;
        }
    }
#else
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;
    mp_int32 iCount = 0;

    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_83PAGE, strDev, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 83 page of device failed.");
        return iRet;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 83 page of device failed.");
        return MP_FAILED;
    }

    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        if(COUNT_LUNWWN == iCount)
        {
            strLunWWN = *iter;
        }
        if(COUNT_LUNID == iCount)
        {
            strLunID = *iter;
        }
        iCount ++;
    }
    //if support C8,get lun id from C8
    vector<mp_string> vecResult1;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_00PAGE, strDev, &vecResult1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 00 page of device failed.");
        return iRet;
    }

    if (vecResult1.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 00 page of device failed.");
        return MP_FAILED;
    }

    mp_bool isSupport = IsSupportXXPage("c8", vecResult1);
    if (MP_TRUE == isSupport)
    {
        vector<mp_string> vecResult2;
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_C8PAGE, strDev, &vecResult2);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get C8 page of device failed.");
            return iRet;
        }
        mp_int32 iCount1 = 0;
        for (iter = vecResult2.begin(); iter != vecResult2.end(); ++iter)
        {
            if(C8_COUNT_LUNID == iCount1)
            {
                strLunID = *iter;
            }
            iCount1 ++;
        }
    }
#endif
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get lun info of disk(%s) succ.", strDev.c_str());
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :获取阵列SN信息
Input        :strDev -- 设备名
Output       : strSN --阵列SN
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::GetArraySN(mp_string& strDev, mp_string& strSN)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get SN info of disk(%s).", strDev.c_str());
#ifdef WIN32
    iRet = GetDisk80Page(strDev, strSN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 80 page of device failed.");
        return iRet;
    }
#else
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;

    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_80PAGE, strDev, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 80 page of device failed.");
        return iRet;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 80 page of device failed.");
        return MP_FAILED;
    }

    strSN = vecResult.front();
#endif
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get SN info of disk(%s) succ, sn %s.", strDev.c_str(), strSN.c_str());

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :二进制转为ASCII
Input        :
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CArray::BinaryToAscii(mp_char* pszHexBuffer, mp_int32 iBufferLen, mp_uchar* pucBuffer,
    mp_int32 iStartBuf, mp_int32 iLength)
{
    if (NULL == pucBuffer)
    {
        return MP_FAILED;
    }

    if (iBufferLen - 1 < iLength * 2)
    {
        return MP_FAILED;
    }

    for (mp_int32 iBuffLen = 0; iBuffLen < iLength; iBuffLen++)
    {
        CHECK_FAIL(sprintf_s(&pszHexBuffer[iBuffLen * 2], iBufferLen - iBuffLen * 2, "%02x",
            pucBuffer[iBuffLen + iStartBuf]));
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :LUN ID转为ASCII
Input        :
Output       : 
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CArray::ConvertLUNIDtoAscii(mp_uchar* puszAsciiLunID, mp_int32 iBufferLen, mp_uchar* puszLunLunID,
    mp_int32 iLen)
{
    if ((NULL == puszAsciiLunID) || (NULL == puszLunLunID))
    {
        return MP_FAILED;
    }

    if (iBufferLen - 1 < iLen)
    {
        return MP_FAILED;
    }

    for (mp_int32 iCount = 0; iCount < iLen / 2; iCount++)
    {
        CHECK_FAIL(sprintf_s((char *)&puszAsciiLunID[iCount * 2], iBufferLen - iCount * 2, "%02x",
            puszLunLunID[iCount]));
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 计算16进制指数值
Input        : iStep---16进制指数值
Output       :
Return       :  iResult---返回计算结果
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CArray::CalStep(mp_int32 iStep)
{
    mp_int32 iResult = 1;

    while (iStep > 0)
    {
        iResult = iResult * 16;
        iStep--;
    }

    return iResult;
}

/*------------------------------------------------------------ 
Description  : 判断vecResult是否有page这个字符串
Input        : vecResult---VPD page code
Output       : 
Return       :  MP_TRUE--支持
               MP_FALSE---不支持
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CArray::IsSupportXXPage(string page, vector<mp_string>& vecResult)
{
    vector<mp_string>::iterator iter = vecResult.begin();
    for (; iter != vecResult.end(); iter++)
    {
        if (page == *iter)
        {
            return MP_TRUE;
        }
    }
    return MP_FALSE;
}


/*------------------------------------------------------------ 
Description  : 十六进制转换十进制
Input        : pHexStr---十六进制 
Output       : pDecStr---十进制
Return       :  MP_SUCCESS---转换成功
               MP_FAILED---转换失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CArray::HextoDec(mp_char* pDecStr, mp_char* pHexStr, mp_int32 iMaxLen)
{
    mp_int32 iStrLen = 0;
    mp_int32 iStep = 0;
    mp_int32 iDecNum = 0;

    if ((NULL == pDecStr) || (NULL == pHexStr))
    {
        return MP_FAILED;
    }

    iStrLen = (mp_int32)strlen(pHexStr);
    for (mp_int32 iTemp = 0; iTemp < iStrLen; iTemp++)
    {
        iStep = (iStrLen - iTemp) - 1;
        if ('0' == pHexStr[iTemp])
        {
            continue;
        }
        else if ((pHexStr[iTemp] > '0') && (pHexStr[iTemp] <= '9'))
        {
            iDecNum = iDecNum + (pHexStr[iTemp] - '0') * CalStep(iStep);
        }
        else if (((pHexStr[iTemp] >= 'a') && (pHexStr[iTemp] <= 'f'))
            || ((pHexStr[iTemp] >= 'A') && (pHexStr[iTemp] <= 'F')))
        {
            (void)HexEncode(pHexStr[iTemp], iStep, iDecNum);
        }
        else
        {
            return MP_FAILED;
        }
    }

    CHECK_FAIL(SNPRINTF_S(pDecStr, (mp_uint32)iMaxLen, (mp_uint32)iMaxLen - 1, "%d", iDecNum));

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 将十六进制编码转换成十进制
               iDecNum = iDecNum + (cHex - 55) * CalStep(iStep); A->10, B->11.
Input        : cHex---十六进制，iStep---16进制指数值
Output       : iDecNum---十进制数
Return       :  MP_SUCCESS---编码成功
               MP_FAILED---编码失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CArray::HexEncode(mp_char cHex, mp_int32 iStep, mp_int32& iDecNum)
{
    mp_bool bRet = ((cHex >= 'A' && cHex <= 'F') || (cHex >= 'a' && cHex <= 'f'));
    if (!bRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Hex encode failed, cHex: %c.", cHex);
        return MP_FAILED;
    }
    
    if (cHex >= 'a' && cHex <= 'z')  
    {
        cHex -= 'a' - 'A';  // 'a' --> 'A' 小写转大写
    }

    iDecNum = iDecNum + (cHex - 55) * CalStep(iStep); //A:10, B:11...F:15   (A: 65 - 55 = 10)
    return MP_SUCCESS;
}


mp_bool CArray::CheckHuaweiLUN(mp_string strVendor)
{
    //排除掉非华为的产品
    if (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
        && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
        && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY))
    {
        return MP_FALSE;
    }
    return MP_TRUE;
}

/*------------------------------------------------------------ 
Description  :获取主机上所有磁盘名称
Input        :
Output       : vecDiskName --磁盘名称
Return       : MP_SUCCESS -- 成功 
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CDisk::GetAllDiskName(vector<mp_string>& vecDiskName)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get all disk names.");
#ifdef WIN32
    //降低圈复杂度，对函数进行拆分
    return GetAllDiskWin(vecDiskName);
#endif
#ifdef LINUX
    mp_string strCmd = "cat /proc/partitions  | grep -e 'sd[a-z]' -e 'hd[a-z]' | awk '{print $4}' | grep -v -e '[1-9]' ";

    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDiskName);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);

        return iRet;
    }

    //查找CCISS卡设备

    //移除命令设备和移除测试失败的设备
    for (vector<mp_string>::iterator it = vecDiskName.begin(); it != vecDiskName.end();)
    {
        if (IsCmdDevice(*it))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "\"%s\" is a cmd device, skip it.", it->c_str());
            it = vecDiskName.erase(it);
            continue;
        }

        if (!IsDeviceExist(*it))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device \"%s\" dosen't exit, skip it.", it->c_str());
            it = vecDiskName.erase(it);
            continue;
        }

        it++;
    }
#endif
#ifdef AIX
    mp_string strCmd = "lsdev -Cc disk  | awk '{print $1}'";

    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDiskName);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return iRet;
    }

    //去除没有激活的
    for (vector<mp_string>::iterator it = vecDiskName.begin(); it != vecDiskName.end();)
    {
        mp_string strStatus;
        iRet = GetDiskStatus(*it, strStatus);
        if ((MP_SUCCESS == iRet) && (strStatus != "Available"))
        {
            it = vecDiskName.erase(it);
        }
        else
        {
            it++;
        }
    }
#endif
#ifdef HP_UX_IA

        /*mp_string strParam = " -funC disk 2>>/dev/null | grep /dev/dsk | awk '{print $1}'";
        mp_int32 iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecDiskName);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec system cmd failed, cmd is ioscan%s, iRet is %d.", strParam.c_str(), iRet);
            return iRet;
        }

        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get disk name succ.");
        if (vecDiskName.empty())
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The vector is empty.");
        }*/
        mp_string strUniqueID = CUniqueID::GetInstance().GetString();
        mp_string strTempFile = "App_" + strUniqueID + ".temp";
        mp_string strExectCmd = "/usr/sbin/ioscan -funC disk 2>/dev/null | grep /dev/dsk | awk '{print $1}'>" + strTempFile;
        mp_string strDiskName;
        mp_string strFullDiskName;
        vector<mp_string> vecSrcDiskName;

        mp_int32 iRet = ExecuteDiskCmdEx(strExectCmd, vecDiskName, strTempFile);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Execute disk cmd failed, cmd %s.", strExectCmd.c_str());
            return iRet;
        }
#endif
#ifdef SOLARIS
    //solaris获取磁盘信息
    mp_string strCmd = "iostat -En | nawk '{if(NR%5==1) {print $1}}'";

    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDiskName);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return iRet;
    }
  
#endif

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End get all disk names.");

    return MP_SUCCESS;
}

//如下sdd1是扩展分区、sdl是阵列命令设备
//major minor  #blocks  name
//   8    48   10485760 sdd
//   8    49          1 sdd1
//   8    53      98272 sdd5
//   8    54      98288 sdd6
//   8    55      98288 sdd7
//   8    56      98288 sdd8
//   8    57     977904 sdd9
//   8   176          1 sdl
/*------------------------------------------------------------ 
Description  : 判断是否是cmd设备
Input        : strDiskName---磁盘名称
Output       :
Return       :  MP_TRUE---是cmd设备
               MP_FALSE---非cmd设备
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CDisk::IsCmdDevice(mp_string& strDiskName)
{
    LOGGUARD("");
    vector<mp_string> vecRlt;
    mp_int32 iBlocks = 0;

    mp_string strCmd = "cat /proc/partitions | grep -e '" + strDiskName + "'  | awk '{print $3}'";
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return MP_FALSE;
    }

    if (vecRlt.size() <= 1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "vecRlt.size() is smaller than 1, the value is %d.",vecRlt.size());
        return MP_FALSE;
    }

    //第一行显示的是设备，其它是分区
    iBlocks = atoi(vecRlt[0].c_str());
    if (CMD_DEV_BLOCKS == iBlocks)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "%s is a cmd device.", strDiskName.c_str());
        return MP_TRUE;
    }

    return MP_FALSE;
}


/*------------------------------------------------------------
Description  : 判断设备是否是HP下disk
Input        : strName -- disk名称
Output       :
Return       : MP_TRUE 
                   MP_FALSE
Create By    :
Modification :
-------------------------------------------------------------*/
mp_bool CDisk::IsDskdisk(mp_string& strName)
{
    mp_bool bIsDskdisk = MP_TRUE;
#ifdef HP_UX_IA
    mp_string::size_type iIndex;

    iIndex = 0;
    mp_string::size_type iIndexDsk = strName.find("/dev/dsk");
    mp_string::size_type iIndexDisk = strName.find("/dev/disk");

    iIndex = (iIndexDsk == mp_string::npos) ? iIndexDisk : iIndexDsk;
    if (iIndex != mp_string::npos)
    {
        bIsDskdisk = MP_TRUE;
    }
    else
    {
        bIsDskdisk = MP_FALSE;
    }
#endif
    return bIsDskdisk;
}

/*------------------------------------------------------------
Function Name: GetHPRawDiskName
Description  : 由磁盘设备名称得到对应的裸盘设备名-hp
               Data Accessed: None.
               Data Updated : None.
Input        : pszDiskName-磁盘设备名
Output       : pszRawDiskName-对应的裸盘设备名
Return       : 成功或失败.
Call         :
Called by    :
Created By   : 
Modification :
Others       :
-------------------------------------------------------------*/
mp_int32 CDisk::GetHPRawDiskName(mp_string strDiskName, mp_string& strRawDiskName)
{
#ifdef HP_UX_IA
    mp_int32 iPos = 0;
    mp_int32 iPosDsk = strDiskName.find("dsk", 0);
    mp_int32 iPosDisk = strDiskName.find("disk", 0);
    iPos = iPosDsk == mp_string::npos ? iPosDisk : iPosDsk;
    if (mp_string::npos == iPos)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disk name is invalid, disk name %s.", strDiskName.c_str());

        return MP_FAILED;
    }

    mp_string strHead = strDiskName.substr(0, iPos);
    mp_string strTail = strDiskName.substr(iPos, strDiskName.length()-iPos);
    strRawDiskName = strHead + "r" + strTail;
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetSolarisRawDiskName
Description  : 由磁盘设备名称得到对应的裸盘设备名-solaris
               Data Accessed: None.
               Data Updated : None.
Input        : pszDiskName-磁盘设备名
Output       : pszRawDiskName-对应的裸盘设备名
Return       : 成功或失败.
Call         :
Called by    :
Created By   : 
Modification :
Others       :
-------------------------------------------------------------*/
mp_int32 CDisk::GetSolarisRawDiskName(mp_string strDiskName, mp_string& strRawDiskName)
{
#ifdef SOLARIS
    mp_int32 iPos = strDiskName.find("dsk", 0);
    if (mp_string::npos == iPos)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disk name is invalid, disk name %s.", strDiskName.c_str());

        return MP_FAILED;
    }

    mp_string strHead = strDiskName.substr(0, iPos);
    mp_string strTail = strDiskName.substr(iPos, strDiskName.length()-iPos);
    strRawDiskName = strHead + "r" + strTail;
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetSecPvName
Description  : 根据生产端PV名称获取灾备端相应PV名称
                     如生产端为/dev/disk/diskxx,则灾备端为/dev/disk/diskxx
                     生产端为/dev/dsk/xx,则灾备端为/dev/dsk/xx-hp
               Data Accessed: None.
               Data Updated : None.
Input        : strPriPvName-生产端PV名称
                  strLegacyDisk-灾备端LegacyDisk名称
Output       : strSecPvName-灾备端PV名称
Return       : 成功或失败.
Call         :
Called by    :
Created By   : 
Modification :
Others       :
-------------------------------------------------------------*/
mp_int32 CDisk::GetSecPvName(mp_string& strPriPvName, mp_string& strLegacyDisk, mp_string& strSecPvName)
{
#ifdef HP_UX_IA
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strPersistentDisk;
    mp_string::size_type iIndexDsk, iIndexDisk;
        
    iIndexDsk = strPriPvName.find("/dev/dsk");
    if (iIndexDsk != mp_string::npos)
    {
        strSecPvName = strLegacyDisk;
    }
    else
    {
        iIndexDsk = strPriPvName.find("/dev/disk");
        if (iIndexDsk != mp_string::npos)
        {
            iRet = CDisk::GetPersistentDSFByLegacyDSF(strLegacyDisk, strPersistentDisk);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get persistent DSF failed, device name %s",
                    strLegacyDisk.c_str());

                return iRet;
            }
            strSecPvName = strPersistentDisk;
        }
        else
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The pripv is not dsk or disk.");
            return MP_FAILED;
        }
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 清理无用设备-HP-UX
Input        :
Output       :
Return       : MP_SUCCESS -- 成功
               MP_FAILED -- 失败
Create By    :
Modification :
当前只适用于HP-UX 11i V3
HP-UX 11i V3版本存在legacy DSFs and persistent DSFs两种设备
之前LUN映射后WWN和LUNPATH建立了通道，一个LUN 通道由设备SN+host LUN ID决定
如果在一个LUNPATH上映射不同WWN的LUN，此时是无法扫描到LUN的
在进行设置存储前需要进行清理无效的设备

1.ioscan -kfNC lunpath 查询出无用的lunpath:解除WWN映射并去掉
2.ioscan -kfNC disk 查询无用的disk
3.根据2步骤中的disk，通过ioscan -m dsf查询无用的dsk删除
4.删除步骤2中的disk
-------------------------------------------------------------*/
mp_int32 CDisk::ClearInvalidDisk()
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "ClearInvalidDisk beginning.");

    // 获取无用lunpath
    strParam= " -kfNC lunpath 2>>/dev/null | awk '{if ($5==\"NO_HW\") print $3}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get invalid lunpath failed.");
        return MP_FAILED;
    }

    // 解除WWN映射，并删除无用的lunpath
    iRet = ClearInvalidLUNPath(vecResult);
    vecResult.clear();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Clean invalid lunpath failed.");
        return MP_FAILED;
    }

    // 获取无用磁盘列表
    strParam= " -kfNC disk 2>>/dev/null | awk '{if ($5==\"NO_HW\") print $2}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get invalid disk failed.");
        return MP_FAILED;
    }

    // 删除无用legacy DSFs
    iRet = ClearInvalidLegacyDSFs(vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Clean invalid legacy DSFs failed.");
        vecResult.clear();
        return MP_FAILED;
    }

    // 删除无用的
    iRet = ClearInvalidPersistentDSFs(vecResult);
    vecResult.clear();
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Clean invalid persistent DSFs failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "ClearInvalidDisk finished.");
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 清理无用LUNPATH-HP-UX
Input        : vecLUNPaths -- 对应无用的lunpath
Output       :
Return       : MP_SUCCESS -- 成功
                  MP_FAILED -- 失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CDisk::ClearInvalidLUNPath(vector<mp_string>& vecLUNPaths)
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to clear invalid lunpath.");

    for (vector<mp_string>::iterator iter = vecLUNPaths.begin(); iter != vecLUNPaths.end(); ++iter)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to clear invalid lunpath %s.", iter->c_str());
        strParam = " -f replace_wwid -H " + *iter;

        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCSIMGR, strParam, NULL);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "replace wwn of lunpath failed, lunpath %s.",
                iter->c_str());

            continue;
        }

        strParam = " -H " + *iter;
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RMSF, strParam, NULL);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "rmsf lunpath failed, lunpath %s.",
                iter->c_str());

            continue;
        }

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to clear invalid lunpath %s.", iter->c_str());
    }


    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Clear invalid lunpath succ.");
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 清理无用legacy DSFs-HP-UX
Input        : vecDiskNumbers -- 对应无用的disk number
Output       :
Return       : MP_SUCCESS -- 成功
                  MP_FAILED -- 失败
Create By    :
Modification :
删除下面类似的legacy DSFs
ioscan -m dsf /dev/rdisk/disk414
Persistent DSF           Legacy DSF(s)
========================================
/dev/rdisk/disk414       /dev/rdsk/c85t0d4
                         /dev/rdsk/c85t1d4
-------------------------------------------------------------*/
mp_int32 CDisk::ClearInvalidLegacyDSFs(vector<mp_string>& vecDiskNumbers)
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to clear invalid legacy DSFs.");

    for (vector<mp_string>::iterator iter = vecDiskNumbers.begin(); iter != vecDiskNumbers.end(); ++iter)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to clear Legacy DSFs of disk%s.", iter->c_str());

        strParam = " -m dsf /dev/rdisk/disk" + *iter + " 2>>/dev/null | grep /dev | awk '{print $NF}' | awk -F \"/\" '{print $NF}'";

        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Legacy DSFs of disk%s failed.",
                iter->c_str());
            vecResult.clear();
            continue;
        }

        for (vector<mp_string>::iterator legacy_iter = vecResult.begin(); legacy_iter != vecResult.end(); ++legacy_iter)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to clear invalid legacy DSF /dev/dsk/%s.", legacy_iter->c_str());

            strParam = " -a /dev/dsk/" + *legacy_iter;
            iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RMSF, strParam, NULL);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "rmsf legacy DSFs (%s) failed.",
                    legacy_iter->c_str());

                continue;
            }

            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to clear invalid legacy DSF /dev/dsk/%s.", legacy_iter->c_str());
        }

        vecResult.clear();
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to clear Legacy DSFs of disk%s.", iter->c_str());
    }


    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Clear invalid legacy DSFs succ.");
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 清理无用persistent DSFs-HP-UX
Input        : vecDiskNumbers -- 对应无用的disk number
Output       :
Return       : MP_SUCCESS -- 成功
                  MP_FAILED -- 失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CDisk::ClearInvalidPersistentDSFs(vector<mp_string>& vecDiskNumbers)
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDeviceName;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to clear invalid persistent DSFs.");

    for (vector<mp_string>::iterator iter = vecDiskNumbers.begin(); iter != vecDiskNumbers.end(); ++iter)
    {
        // 存在不是当前使用的磁盘，如果失败不用直接返回
        strDeviceName = "/dev/disk/disk" + *iter;
        (mp_void)DeletePersistentDSF(strDeviceName);
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Clear invalid persistent DSFs succ.");
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 获取disk设备的HW Path和HW Type
Input        : rstrDiskNumber -- persistent DSF instance number
Output       : strPDHWPath/strPDHWType对应设备HWPATH和HWType
Return       : MP_SUCCESS -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CDisk::GetPersistentDSFInfo(mp_string& rstrDiskName, mp_string &strPDHWPath,
    mp_string &strPDHWType)
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strRes;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to get persistentDSF info, device name %s.",
        rstrDiskName.c_str());

    //  ioscan -kfNC disk /dev/disk/disk418 | grep disk | awk '{print $3,$5}'
    strParam = " -kfNC disk " + rstrDiskName + "2>>/dev/null | grep disk | awk '{print $3,$5}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get persistentDSF info failed, device name %s.",
            rstrDiskName.c_str());

        return MP_FAILED;
    }

    if (!vecResult.empty())
    {
        strRes = vecResult.front();
    }
    vecResult.clear();

    // 正常返回结果64000/0xfa00/0x88 NO_HW
    size_t blackIndex = strRes.find(" ");
    if (mp_string::npos == blackIndex)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get persistentDSF info failed, device name %s, find %s.",
            rstrDiskName.c_str(), strRes.c_str());
        return MP_FAILED;
    }
    strPDHWPath = strRes.substr(0, blackIndex);
    strPDHWType = strRes.substr(blackIndex + 1);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get persistentDSF info succ, device name %s, h/w path %s, h/w type %s.",
        rstrDiskName.c_str(), strPDHWPath.c_str(), strPDHWType.c_str());
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 获取disk设备的HW Path和HW Type
Input        : rstrLegancyDisk -- Legacy DSF name
Output       : rstrPersistentDisk对应legacy DSF的persistent DSF
Return       : MP_SUCCESS -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CDisk::GetPersistentDSFByLegacyDSF(mp_string& rstrLegacyDisk, mp_string &rstrPersistentDisk)
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to get persistent DSF by legacy DSF, device name %s.",
        rstrLegacyDisk.c_str());

    //  ioscan -m dsf /dev/dsk/c85t0d1 | grep /dev | awk '{print $1}'

    strParam = " -m dsf " + rstrLegacyDisk + " 2>>/dev/null | grep /dev | awk '{print $1}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult);
    if (MP_SUCCESS != iRet )
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get persistent DSF failed, device name %s.",
            rstrLegacyDisk.c_str());

        return MP_FAILED;
    }

    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get persistent DSF failed, device name %s, find nothing.",
            rstrLegacyDisk.c_str());
        return MP_FAILED;

    }
    rstrPersistentDisk = vecResult.front();
    vecResult.clear();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get persistent DSF by legacy DSF, device name %s.",
        rstrLegacyDisk.c_str());
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : 删除Persistent DSF
Input        : strDeviceName -- 需要Persistent DSF的设备名称
Output       :
Return       : ISSP_RTN_OK -- 操作成功
               非ISSP_RTN_OK -- 操作失败
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CDisk::DeletePersistentDSF(mp_string& strDeviceName)
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    mp_string strHWPath;
    mp_string strHWType;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to clear persistent DSF (%s).", strDeviceName.c_str());

    if (mp_string::npos == strDeviceName.find("/dev/disk") && string::npos == strDeviceName.find("/dev/rdisk"))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The device is not persistent DSF (%s).", strDeviceName.c_str());
        return MP_FAILED;
    }

    strParam = " -a " + strDeviceName;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RMSF, strParam, NULL);
    if (MP_SUCCESS == iRet)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to rmsf persistent DSF (%s) by rmsf -a.", strDeviceName.c_str());
        return MP_SUCCESS;
    }

    // 通过rmsf删除设备失败，尝试通过HW Path删除
    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "rmsf persistent DSF (%s) failed, begin to delete by rmsf -H.",
        strDeviceName.c_str());
    iRet = GetPersistentDSFInfo(strDeviceName, strHWPath, strHWType);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get persistent DSF (%s) info failed.", strDeviceName.c_str());
        return MP_FAILED;
    }

    strParam = " -H " + strHWPath;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RMSF, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "rmsf persistent DSF failed by rmsf -H, disk %s hwpath %s failed.",
            strDeviceName.c_str(), strHWPath.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to rmsf persistent DSF (%s) by rmsf -H.", strDeviceName.c_str());
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ExecuteDiskCmdEx
Description  : 执行命令-hp
Data Accessed: None.
Data Updated : None.
Input        : pszCmd-命令;strFileName-临时文件名
Output       : listValue-值链表
Return       : 成功或失败.
Call         :
Called by    :
Created By   : 
Modification :
Others       :
-------------------------------------------------------------*/
mp_int32 CDisk::ExecuteDiskCmdEx(mp_string strCmd, vector<mp_string> &vecDiskName, mp_string strFileName)
{
#ifdef HP_UX_IA
    FILE *pFile = NULL;
    mp_char acBuf[DISK_CMD_DATA_LEN] = {0};
    
    const mp_char* strBaseFileName = NULL;
    strBaseFileName = BaseFileName(strFileName.c_str());

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Exec cmd, cmd %s.", strCmd.c_str());
    if (0 != system(strCmd.c_str()))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec cmd failed, cmd %s.", strCmd.c_str());
        (void)unlink(strFileName.c_str());

        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open file %s.", strBaseFileName);
    pFile = fopen(strFileName.c_str(), "r");
    if (NULL == pFile)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open file failed, file %s.", strBaseFileName);
        (void)unlink(strFileName.c_str());

        return MP_FAILED;
    }

    for (mp_uint32 uiIndex = 0; uiIndex < FILE_ROW_COUNT; uiIndex++)
    {
        CHECK_NOT_OK(memset_s(acBuf, DISK_CMD_DATA_LEN, 0, DISK_CMD_DATA_LEN));
        if (NULL == fgets(acBuf, sizeof(acBuf), pFile))
        {
            break;
        }

        //去掉首尾无用的字符
        (void)CMpString::TotallyTrimRight(acBuf);
        vecDiskName.push_back(mp_string(acBuf));
    }

    (void)fclose(pFile);
    (void)unlink(strFileName.c_str());
#endif
    return MP_SUCCESS;
}

#ifdef WIN32
/*------------------------------------------------------------ 
Description  : 获取磁盘路径
Input        :  
Output       :
Return       :   
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDisk::GetDiskPath(HDEVINFO &hIntDevInfo, mp_int64 iIndex, mp_char *pszPath, mp_int32 pszPathLen)
{
    SP_DEVICE_INTERFACE_DATA stInterFaceData;
    mp_int32 iStatus = 0;
    mp_ulong ulReqSize = 0;
    mp_ulong ulInterfaceDetailDataSize = 0;
    mp_ulong ulErrorCode = 0;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pstInterfaceDetailData = NULL;
    mp_int32 iRet = MP_SUCCESS;

    if ((0 > iIndex) || (NULL == pszPath))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Index less than zero or pszPath is null pointer.");

        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get disk path.");
    iRet = memset_s(&stInterFaceData, sizeof(SP_DEVICE_INTERFACE_DATA), 0, sizeof(SP_DEVICE_INTERFACE_DATA));
    if (EOK != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);

        return MP_FAILED;
    }
    stInterFaceData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

    iStatus = SetupDiEnumDeviceInterfaces (hIntDevInfo,              // Interface Device Info handle
                                           0,                       // Device Info data
                                           &DiskClassGuid,          // Interface registered by driver
                                           (mp_ulong)iIndex,      // Member
                                           &stInterFaceData);      // Device Interface Data
    if (!iStatus)
    {
        ulErrorCode = GetLastError();
        if (ulErrorCode == ERROR_NO_MORE_ITEMS)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "SetupDiEnumDeviceInterfaces error, errorcode %d", DISK_NORECORD_ERROR);

            return DISK_NORECORD_ERROR;
        }

        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SetupDiEnumDeviceInterfaces error, errorcode %d", ulErrorCode);

        return MP_FAILED;
    }

    iStatus = SetupDiGetDeviceInterfaceDetail (hIntDevInfo,          // Interface Device info handle
                                               &stInterFaceData,    // Interface data for the event class
                                               NULL,                // Checking for buffer size
                                               0,                   // Checking for buffer size
                                               &ulReqSize,          // Buffer size required to get the detail data
                                               NULL);              // Checking for buffer size
    if (!iStatus || ulReqSize <= 0)
    {
        ulErrorCode = GetLastError();
        if (ulErrorCode != ERROR_INSUFFICIENT_BUFFER)
        {
           COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SetupDiGetDeviceInterfaceDetail error(%d)", ulErrorCode);

            return MP_FAILED;
        }
    }

    ulInterfaceDetailDataSize = ulReqSize;
    pstInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(ulInterfaceDetailDataSize, sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA));

    if (NULL == pstInterfaceDetailData)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call calloc failed.");
        
        return MP_FAILED;
    }

    pstInterfaceDetailData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

    iStatus = SetupDiGetDeviceInterfaceDetail(hIntDevInfo,                 // Interface Device info handle
                                              &stInterFaceData,           // Interface data for the event class
                                              pstInterfaceDetailData,     // Interface detail data
                                              ulInterfaceDetailDataSize,  // Interface detail data size
                                              &ulReqSize,                 // Buffer size required to get the detail data
                                              NULL);                     // Interface device info

    if (!iStatus)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SetupDiGetDeviceInterfaceDetail error");
        free(pstInterfaceDetailData);

        return MP_FAILED;
    }

    iRet = strncpy_s(pszPath, pszPathLen, pstInterfaceDetailData->DevicePath, DISK_PATH_MAX + 1);
    if (EOK != iRet)
    {
        free(pstInterfaceDetailData);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call strncpy_s failed, ret %d.", iRet);

        return MP_FAILED;
    }

    free(pstInterfaceDetailData);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End get disk path.");
    return MP_SUCCESS;
}


//获取所有驱动器号和卷路径保存在map中
mp_int32 CDisk::GetVolPaths(map<mp_string, mp_string> &mapPath)
{
    LOGGUARD("");
    mp_ulong ulDrive = 0;
    mp_int32 iErr = 0;
    mp_char cDriveNum = 'A'; //盘符从A开始
    mp_char acVolName[DISK_PATH_MAX] = {0};
    mp_char szErr[256] = {0};
    mp_int32 iRet = 0;
    mp_string strDriverLetter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get volume paths.");

    ulDrive = GetLogicalDrives();
    if (0 == ulDrive)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get logical drives failed, errno[%d]:%s.", iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    for (cDriveNum = 'A'; cDriveNum <= 'Z'; cDriveNum++)
    {
        if (ulDrive & 0x01)
        {
            strDriverLetter = cDriveNum;
            strDriverLetter += ":\\";

            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get volume path for mount point %s.", strDriverLetter.c_str());
            iRet = memset_s(acVolName, sizeof(acVolName), 0, sizeof(acVolName));
            if (EOK != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
                return iRet;
            }

            iRet = GetVolumeNameForVolumeMountPoint(strDriverLetter.c_str(), acVolName, sizeof(acVolName));
            if (0 == iRet)
            {
                iErr = GetOSError();
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get volume path for mount point failed, mount point %s, errno[%d]:%s.",
                    strDriverLetter.c_str(), iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
                return MP_FAILED;
            }

            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Add volume path and mount point info, volume path %s, mount point %s.",
                acVolName, strDriverLetter.c_str());
            (mp_void)mapPath.insert(make_pair(acVolName, strDriverLetter));
        }

        ulDrive = ulDrive >> 1;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get volume paths succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 判断驱动是否存在
Input        : strDrive---驱动名称
Output       :
Return       :  MP_TRUE---驱动存在
               MP_FALSE---驱动不存在
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CDisk::IsDriveExist(mp_string& strDrive)
{
    //LOGGUARD("");
    mp_ulong ulDrive = 0;
    mp_int32 iErr = 0;
    mp_char cDriveNum = 'A'; //盘符从A开始
    mp_char szErr[256] = {0};
    mp_string strDriverLetter;

    ulDrive = GetLogicalDrives();
    if (0 == ulDrive)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get logical drives failed, errno[%d]:%s.", iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FALSE;
    }

    for (cDriveNum = 'A'; cDriveNum <= 'Z'; cDriveNum++)
    {
        if (ulDrive & 0x01)
        {
            strDriverLetter = cDriveNum;
            strDriverLetter += ":\\";
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get drive letter %s, expected drive letter %s.", strDriverLetter.c_str(),
                strDrive.c_str());
            if (strDrive == strDriverLetter)
            {
                COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Drive %s exist.", strDrive.c_str());
                return MP_TRUE;
            }
        }

        ulDrive = ulDrive >> 1;
    }

    return MP_FALSE;
}
/*------------------------------------------------------------ 
Description  : 获取磁盘卷信息
Input        : pcVolumeName---PC卷名称
Output       :pstrPartInfo---磁盘部分信息
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDisk::GetVolumeDiskInfo(mp_char* pcVolumeName, sub_area_Info_t& pstrPartInfo)
{
    LOGGUARD("");
    FILE_HANDLE hDevice;
    mp_int32 iRet = 0;
    mp_ulong ulReCount = 0;
    mp_ulong ulCount = 0;
    mp_ulong ulBufferSize = sizeof(VOLUME_DISK_EXTENTS) + (MAX_DISK_EXTENS - 1) * sizeof(DISK_EXTENT);
    mp_char *pcOutBuffer = NULL;
    PVOLUME_DISK_EXTENTS pstrVolumeDiskExtents;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin GetVolumeDiskInfo.");

    pcOutBuffer = (mp_char *)calloc(ulBufferSize, sizeof(mp_char));
    if (NULL == pcOutBuffer)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call calloc failed.");
        
        return MP_FAILED;
    }

    hDevice = CreateFile(pcVolumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hDevice)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "GetVolumeDiskInfo hDevice is INVALID_HANDLE_VALUE when get volume disk info on windows.");
        free(pcOutBuffer);
        return MP_FAILED;
    }

    iRet = DeviceIoControl(hDevice, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, pcOutBuffer, ulBufferSize,
        &ulReCount, (LPOVERLAPPED) NULL);
    if (MP_FALSE == iRet)
    {
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "DeviceIoControl error! errorcode[%d] volname=%s when get volume disk info on windows",
                GetLastError(), pcVolumeName);
        }

        (mp_void)CloseHandle(hDevice);

        free(pcOutBuffer);

        return MP_FAILED;
    }

    pstrVolumeDiskExtents = (PVOLUME_DISK_EXTENTS)pcOutBuffer;
    if (pstrVolumeDiskExtents->NumberOfDiskExtents > 1)
    {
        (mp_void)CloseHandle(hDevice);
        free(pcOutBuffer);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
            "Get partion pstrPartInfo error disk when get volume disk info on windows");

        return MP_FAILED;
    }

    //卷分区可以跨越多块磁盘，目前不考虑这种情况，只对应最后一块磁盘序号
    for (ulCount = 0; ulCount < pstrVolumeDiskExtents->NumberOfDiskExtents; ulCount++)
    {
        pstrPartInfo.iDiskNum = (mp_int32)pstrVolumeDiskExtents->Extents[ulCount].DiskNumber;
        pstrPartInfo.llOffset = pstrVolumeDiskExtents->Extents[ulCount].StartingOffset.QuadPart;
        pstrPartInfo.ullTotalCapacity = pstrVolumeDiskExtents->Extents[ulCount].ExtentLength.QuadPart;
    }

    (mp_void)CloseHandle(hDevice);

    free(pcOutBuffer);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End GetVolumeDiskInfo.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 获取next卷信息
Input        : pcVolumeName---PC卷名称
Output       :  
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDisk::GetNextVolumeInformation(FILE_HANDLE FindHandle, mp_char* pcVolumeName, mp_int32 iSize)
{
    LOGGUARD("");
    mp_int32 iret = MP_SUCCESS;
    if ((NULL == FindHandle) || (NULL == pcVolumeName))
    {
        return MP_FAILED;
    }

    mp_ulong ulError = 0;
    mp_int32 iSuccess = FindNextVolume(FindHandle, pcVolumeName, iSize);

    if (!iSuccess)
    {
        ulError = GetLastError();
        if (ulError != ERROR_NO_MORE_FILES)
        {
            COMMLOG(OS_LOG_ERROR ,LOG_COMMON_ERROR,
                "FindNextVolume failed with error code(%lu) when get next volume info on windows", ulError);
            return MP_FAILED;
        }

        return MP_FAILED;
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 获取磁盘号
Input        : strDeviceName---磁盘名称
Output       :  iDiskNum---磁盘号
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDisk::GetDiskNum(mp_string& strDeviceName, mp_int32& iDiskNum)
{
    mp_int32 iRet =MP_SUCCESS;
    FILE_HANDLE fHandle;
    mp_ulong ulReturn = 0;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};
    STORAGE_DEVICE_NUMBER strNum = {0};

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get disk num.");
    fHandle = CreateFile(strDeviceName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == fHandle)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open device failed, errno[%d]:%s.", iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    iRet = DeviceIoControl(fHandle, IOCTL_STORAGE_GET_DEVICE_NUMBER, &strNum, sizeof(STORAGE_DEVICE_NUMBER), &strNum,
        sizeof(STORAGE_DEVICE_NUMBER), &ulReturn, NULL);
    if (!iRet)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device number failed, errno[%d]:%s.", iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        (mp_void)CloseHandle(fHandle);
        return MP_FAILED;
    }

    iDiskNum = (mp_int32)strNum.DeviceNumber;
    (mp_void)CloseHandle(fHandle);
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get disk num succ, disk num %d.", iDiskNum);
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 获取分区信息列表
Input        :  
Output       :  rvecSubareaInfo---分区信息
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDisk::GetSubareaInfoList(vector<sub_area_Info_t>& rvecSubareaInfo)
{
    LOGGUARD("");
    mp_ulong ulError = 0;
    mp_ulong ulCharCount = 0;
    mp_char acVolumeName[DISK_PATH_MAX] = {0};
    sub_area_Info_t strPartInfo;
    FILE_HANDLE FindHandle;
    map<mp_string, mp_string> mapVol;
    map<mp_string, mp_string>::iterator itMap;
    DWORD dwFlg = 0;
    mp_int32 iRet = MP_SUCCESS;

    iRet = CDisk::GetVolPaths(mapVol);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get volume paths failed, iRet %d.", iRet);
        return iRet;
    }

    FindHandle = FindFirstVolume(acVolumeName, ARRAYSIZE(acVolumeName));
    if (INVALID_HANDLE_VALUE == FindHandle)
    {
        ulError = GetLastError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "FindNextVolume failed with error code(%lu) when get subares info "
            "on windows", ulError);
        return MP_FAILED;
    }

    do
    {
        errno_t erRes = EOK;
        erRes = memset_s(&strPartInfo, sizeof(sub_area_Info_t), 0, sizeof(sub_area_Info_t));
        if (EOK != erRes)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", erRes);
            (mp_void)FindVolumeClose(FindHandle);
            return MP_FAILED;
        }

        CHECK_FAIL(SNPRINTF_S(strPartInfo.acVolName, sizeof(strPartInfo.acVolName),
            sizeof(strPartInfo.acVolName) - 1, "%s", acVolumeName));

        //去掉'\\?\'和最后的'\'
        mp_uint32 uiIndex = strlen(acVolumeName) - 1;

        //QueryDosDeviceW doesn't allow a trailing backslash
        acVolumeName[uiIndex] = '\0';

        //去掉前面的\\?\，QueryDosDevice函数需要去掉，故从第四个字符开始
        ulCharCount = QueryDosDevice(&acVolumeName[4], strPartInfo.acDeviceName, ARRAYSIZE(strPartInfo.acDeviceName));
        acVolumeName[uiIndex] = '\\';
        if (0 == ulCharCount)
        {
            ulError = GetLastError();
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "QueryDosDevice failed with error code(%lu) when get subares info on windows", ulError);
            continue;
        }

        if (DRIVE_FIXED == GetDriveType(acVolumeName))
        {
            itMap = mapVol.find(acVolumeName);
            if (itMap != mapVol.end())
            {
                CHECK_FAIL(SNPRINTF_S(strPartInfo.acDriveLetter, sizeof(strPartInfo.acDriveLetter),
                    sizeof(strPartInfo.acDriveLetter) - 1, (mp_char*)(*itMap).second.c_str()));

                strPartInfo.acDriveLetter[strlen(strPartInfo.acDriveLetter) - 2] = '\0';//去掉末尾的":\"
            }

            acVolumeName[uiIndex] = '\0';
            if (MP_SUCCESS != CDisk::GetVolumeDiskInfo(acVolumeName, strPartInfo))
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                    "GetVolumeDiskInfo error,vol(%s) get subares info on windows", acVolumeName);
                continue;
            }

            acVolumeName[uiIndex] = '\\';

            //获取文件系统类型
            (mp_void)GetVolumeInformation(acVolumeName, strPartInfo.acVolLabel, sizeof(strPartInfo.acVolLabel),
                NULL, 0, &dwFlg, strPartInfo.acFileSystem, sizeof(strPartInfo.acFileSystem));

            //将分区信息加入到list中
            rvecSubareaInfo.push_back(strPartInfo);
        }
    }while(MP_SUCCESS == CDisk::GetNextVolumeInformation(FindHandle, acVolumeName, ARRAYSIZE(acVolumeName)));

    (mp_void)FindVolumeClose(FindHandle);

    if (0 == rvecSubareaInfo.size())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get subarea info list failed get subares info on windows.");
    }
    else
    {
        for (vector<sub_area_Info_t>::iterator itSubareaInfoWin = rvecSubareaInfo.begin();
            itSubareaInfoWin != rvecSubareaInfo.end(); itSubareaInfoWin++)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "device=%s, fs=%s, path=%s, lable=%s, vol=%s, disk=%d, lba=%lld, cp=%lld",
                itSubareaInfoWin->acDeviceName, itSubareaInfoWin->acFileSystem, itSubareaInfoWin->acDriveLetter,
                itSubareaInfoWin->acVolLabel, itSubareaInfoWin->acVolName, itSubareaInfoWin->iDiskNum,
                itSubareaInfoWin->llOffset, itSubareaInfoWin->ullTotalCapacity);
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End GetSubareaInfoList.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : 获取磁盘信息列表
Input        :  
Output       :  vecDiskInfoWin---磁盘信息
Return       :  MP_SUCCESS---获取成功
                
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDisk::GetDiskInfoList(vector<disk_info> &vecDiskInfoWin)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecDiskName;
    vector<mp_string>::iterator itDiskNameLst;
    disk_info stDiskInfo;
    mp_string strVendor;
    mp_string strProduct;
    mp_bool bRet = MP_FALSE;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get lun infos in windows.");

    //获取主机上有硬盘名称
    (void)GetAllDiskWin(vecDiskName);
    if (0 == vecDiskName.size())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disk name list is empty when get disk name on windows.");

        return MP_SUCCESS;
    }

    //获取LUN的ID、WWN、阵列SN以及对应的disk number
    for (itDiskNameLst = vecDiskName.begin(); itDiskNameLst != vecDiskName.end(); ++itDiskNameLst)
    {
        stDiskInfo.iDiskNum = 0;
        stDiskInfo.strArraySN = "";
        stDiskInfo.strLUNID = "";
        stDiskInfo.strLUNWWN = "";
        
        //阵列的厂商和型号
        iRet = CArray::GetArrayVendorAndProduct(*itDiskNameLst, strVendor, strProduct);
        if (MP_FAILED == iRet)
        {
            continue;
        }

        strVendor = CMpString::Trim((mp_char *)strVendor.c_str());
        strProduct = CMpString::Trim((mp_char *)strProduct.c_str());
        //排除掉非华为的产品
        bRet = (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY));
        if (bRet)
        {
            continue;
        }

        //获取SN
        iRet = CArray::GetArraySN(*itDiskNameLst, stDiskInfo.strArraySN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Failed to get array SN on windows.");
            continue;
        }

        //获取LUN info
        iRet = CArray::GetLunInfo(*itDiskNameLst, stDiskInfo.strLUNWWN, stDiskInfo.strLUNID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Failed to get LUN info on windows.");
            continue;
        }

        //获取disknumber
        iRet = CDisk::GetDiskNum(*itDiskNameLst, stDiskInfo.iDiskNum);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Failed to get disk number on windows.");
            continue;
        }

        vecDiskInfoWin.push_back(stDiskInfo);
    }

    if (0 == vecDiskInfoWin.size())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disk info list is empty when get disk info on windows.");
    }
    else
    {
        for (vector<disk_info>::iterator itDiskInfoWin = vecDiskInfoWin.begin();
            itDiskInfoWin != vecDiskInfoWin.end(); itDiskInfoWin++)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "sn=%s, id=%s, wwn=%s, disk=%d",
                itDiskInfoWin->strArraySN.c_str(), itDiskInfoWin->strLUNID.c_str(),
                itDiskInfoWin->strLUNWWN.c_str(), itDiskInfoWin->iDiskNum);
        }
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get lun infos in windows succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : 获取所有磁盘信息列表
Input        :  
Output       :  vecDiskName---磁盘名称组
Return       :  MP_SUCCESS---获取成功
                
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDisk::GetAllDiskWin(vector<mp_string>& vecDiskName)
{
    mp_int64 iIndex = 0;
    mp_int32 iRet = MP_SUCCESS;
    mp_char acDosLinkName[NAME_PATH_LEN] = {0};
    HDEVINFO hIntDevInfo;

    vecDiskName.clear();
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin get all lun infos in windows.");
    // 创建枚举设备句柄
    hIntDevInfo = SetupDiGetClassDevs(&DiskClassGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE));
    for ( ; ;)
    {
        iRet = memset_s(acDosLinkName, sizeof(acDosLinkName), 0, sizeof(acDosLinkName));
        if (EOK != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
            (mp_void)SetupDiDestroyDeviceInfoList(hIntDevInfo);
            return MP_SUCCESS;
        }

        //查询磁盘路径
        iRet = GetDiskPath(hIntDevInfo, iIndex, acDosLinkName, NAME_PATH_LEN);
        if (DISK_NORECORD_ERROR == iRet)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Disk is not recorded, then return succ.");

            (mp_void)SetupDiDestroyDeviceInfoList(hIntDevInfo);
            return MP_SUCCESS;
        }
        else if (MP_FAILED == iRet)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "GetDiskPath is failed, then continue.");
            iIndex++;
            continue;
        }

        vecDiskName.push_back(mp_string(acDosLinkName));
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Disk name %s.", acDosLinkName);
        iIndex++;
    }

    (mp_void)SetupDiDestroyDeviceInfoList(hIntDevInfo);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get all lun infos in windows end.");
}
/*------------------------------------------------------------ 
Description  : 通过卷名称获取卷路径
Input        :  
Output       :  mapPath---映射路径
Return       :  
                
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CDisk::GetVolPathByVolName(map<mp_string, mp_string> &mapPath)
{
    mp_ulong ulDrive;
    mp_char cDriveNum = 'A'; //盘符从A开始
    mp_char acVolName[MAX_PATH_LEN];
    mp_int32 iRet = 0;
    mp_string strTmp = "";

    ulDrive = GetLogicalDrives();

    for (cDriveNum = 'A'; cDriveNum <= 'Z'; cDriveNum++)
    {
        if (ulDrive & 0x01)
        {
            strTmp = cDriveNum;
            strTmp += ":\\";

            iRet = memset_s(acVolName, sizeof(acVolName), 0, sizeof(acVolName));
            if (EOK != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
                return;
            }

            iRet = GetVolumeNameForVolumeMountPoint((mp_char *)strTmp.c_str(), acVolName, sizeof(acVolName));
            if ((0 != iRet) && (0 != strlen(acVolName)))
            {
                (mp_void)mapPath.insert(make_pair(acVolName, strTmp));
            }
        }

        ulDrive = ulDrive >> 1;
    }
}
/*------------------------------------------------------------ 
Description  : 获取磁盘卷信息
Input        : pcVolumeName---PC卷名称
Output       :pstrPartInfo---磁盘部分信息
Return       :  MP_SUCCESS---获取成功
               MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDisk::GetVolumeDiskInfo(mp_char *pcVolumeName, sub_area_Info_t *pstrPartInfo)
{
    HANDLE hDevice;
    mp_int32 iResult;
    mp_ulong ulReCount;
    mp_ulong ulCount;
    mp_ulong ulBufferSize = sizeof(VOLUME_DISK_EXTENTS) + (MAX_DISK_EXTENS - 1) * sizeof(DISK_EXTENT);
    mp_char *pcOutBuffer = NULL;
    PVOLUME_DISK_EXTENTS pstrVolumeDiskExtents;

    if ((NULL == pcVolumeName) || (NULL == pstrPartInfo))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s",
            "Invalid parameters for null pointer when get volume disk info on windows.");
        return MP_FAILED;
    }

    pcOutBuffer = (mp_char *)calloc(ulBufferSize, sizeof(mp_char));
    if (NULL == pcOutBuffer)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call calloc failed.");
        
        return MP_FAILED;
    }

    hDevice = CreateFile(pcVolumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hDevice)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s",
            "GetVolumeDiskInfo hDevice is INVALID_HANDLE_VALUE when get volume disk info on windows.");
        free(pcOutBuffer);
        return MP_FAILED;
    }

    iResult = DeviceIoControl(hDevice, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, pcOutBuffer, ulBufferSize,
        &ulReCount, (LPOVERLAPPED) NULL);
    if (0 == iResult)
    {
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "DeviceIoControl error! errorcode[%d] volname=%s when get volume disk info on windows",
                GetLastError(), pcVolumeName);
        }

        (mp_void)CloseHandle(hDevice);

        free(pcOutBuffer);

        return MP_FAILED;
    }

    pstrVolumeDiskExtents = (PVOLUME_DISK_EXTENTS)pcOutBuffer;
    if (pstrVolumeDiskExtents->NumberOfDiskExtents > 1)
    {
        (mp_void)CloseHandle(hDevice);
        free(pcOutBuffer);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s",
            "Get partion pstrPartInfo error disk when get volume disk info on windows");

        return MP_FAILED;
    }

    //卷分区可以跨越多块磁盘，目前不考虑这种情况，只对应最后一块磁盘序号
    for (ulCount = 0; ulCount < pstrVolumeDiskExtents->NumberOfDiskExtents; ulCount++)
    {
        pstrPartInfo->iDiskNum = (mp_int32)pstrVolumeDiskExtents->Extents[ulCount].DiskNumber;
        pstrPartInfo->llOffset = pstrVolumeDiskExtents->Extents[ulCount].StartingOffset.QuadPart;
        pstrPartInfo->ullTotalCapacity = pstrVolumeDiskExtents->Extents[ulCount].ExtentLength.QuadPart;
    }

    (mp_void)CloseHandle(hDevice);

    free(pcOutBuffer);

    return (iResult);
}

CDisk::CDisk()
{
    m_hNtdll = NULL;
}

CDisk::~CDisk()
{
    FreeNtdllModule();
}
/*------------------------------------------------------------ 
Description  : 关闭dll
Input        :  
Output       : 
Return       :  
                
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CDisk::FreeNtdllModule()
{
    if (m_hNtdll)
    {
        FreeLibrary(m_hNtdll);
        m_hNtdll = NULL;
    }
}
/*------------------------------------------------------------ 
Description  : 加载dll
Input        :  
Output       : 
Return       :  MP_TRUE---加载成功
                MP_FALSE---加载失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CDisk::LoadNtdllModule()
{
    LOGGUARD("");
    mp_bool bRet = MP_FALSE;
    if (m_hNtdll != NULL)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s", "ntdll handle is exists.");
        return MP_FALSE;
    }

    mp_char strSystemPath[MAX_PATH_LEN] = { 0 };
    int lenPath = GetSystemDirectory(strSystemPath, MAX_PATH_LEN);
    if (lenPath == 0)
    {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get system directory failed, errno [%d]: %s.", iErr, 
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FALSE;
    }

    mp_string ntdllPath = mp_string(strSystemPath) + "\\ntdll.dll";
    m_hNtdll = LoadLibrary(TEXT(ntdllPath.c_str()));
    if ( NULL == m_hNtdll )
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "LoadNtdllModule failed [%ld]", GetLastError());
        return MP_FALSE;
    }

    RtlInitUnicodeString = (RTLINITUNICODESTRING)GetProcAddress( m_hNtdll, "RtlInitUnicodeString");
    if (NULL == RtlInitUnicodeString)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "RtlInitUnicodeString null, errorcode(%d).",  GetLastError());
    }

    ZwOpenDirectoryObject = (ZWOPENDIRECTORYOBJECT)GetProcAddress( m_hNtdll, "ZwOpenDirectoryObject");
    if (NULL == ZwOpenDirectoryObject)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ZwOpenDirectoryObject null, errorcode(%d).",  GetLastError());
    }

    ZwOpenSymbolicLinkObject = (ZWOPENSYMBOLICKLINKOBJECT)GetProcAddress( m_hNtdll, "ZwOpenSymbolicLinkObject");
    if (NULL == ZwOpenSymbolicLinkObject)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ZwOpenSymbolicLinkObject null, errorcode(%d).",  GetLastError());
    }

    ZwQuerySymbolicLinkObject = (ZWQUERYSYMBOLICKLINKOBJECT)GetProcAddress( m_hNtdll, "ZwQuerySymbolicLinkObject");
    if (NULL == ZwQuerySymbolicLinkObject)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ZwQuerySymbolicLinkObject null, errorcode(%d).",  GetLastError());
    }

    ZwClose = (ZWCLOSE)GetProcAddress( m_hNtdll, "ZwClose");
    if (NULL == ZwClose)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ZwClose null, errorcode(%d).",  GetLastError());
    }

    bRet = ((NULL == RtlInitUnicodeString)
        || (NULL == ZwOpenDirectoryObject)
        || (NULL == ZwOpenSymbolicLinkObject)
        || (NULL == ZwQuerySymbolicLinkObject)
        || (NULL == ZwClose));
    if (bRet)
    {
        return MP_FALSE;
    }
    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  : 查询符号链接
Input        :  SymbolicLinkName---符号链接名字，isVolumn---卷判断
Output       : LinkTarget---链接目标
Return       :  MP_TRUE---加载成功
                MP_FALSE---加载失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_ulong CDisk::QuerySymbolicLink(IN PUNICODE_STRING SymbolicLinkName, OUT PUNICODE_STRING LinkTarget,
    mp_bool isVolumn)
{
    OBJECT_ATTRIBUTES oa;
    mp_ulong status;
    HANDLE handle;

    UNICODE_STRING     strDirName;
    OBJECT_ATTRIBUTES  oba;
    mp_ulong           ntStatus;
    HANDLE             hDirectory = NULL;

    LOGGUARD("");
    if (isVolumn == MP_TRUE)
    {
        RtlInitUnicodeString(&strDirName, L"\\global\?\?");
        InitObjectAttributes(&oba, &strDirName, OBJ_CASE_INSENSITIVE, NULL, NULL);
        ntStatus = ZwOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY, &oba);
        if ( ntStatus != STATUS_SUCCESS )
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open directory object failed (0x%X), [%ld].", ntStatus, GetLastError());
            if (hDirectory != NULL)
            {
                ZwClose(hDirectory);
            }
            return ntStatus;
        }
    }

    InitObjectAttributes(&oa, SymbolicLinkName, OBJ_CASE_INSENSITIVE, hDirectory, 0);
    //Return value
    //ZwOpenSymbolicLinkObject returns STATUS_SUCCESS on success or the appropriate error status.
    status = ZwOpenSymbolicLinkObject(&handle, GENERIC_READ, &oa);
    if (STATUS_SUCCESS != status)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ZwOpenSymbolicLinkObject (%s) faild (0x%X), [%ld].", 
            SymbolicLinkName->Buffer, status, GetLastError());
        return status;
    }

    LinkTarget->MaximumLength = MAX_PATH_LEN * sizeof(WCHAR);
    LinkTarget->Length = 0;
    LinkTarget->Buffer = (PWSTR)GlobalAlloc(GPTR, LinkTarget->MaximumLength);
    if (!LinkTarget->Buffer)
    {
        ZwClose(handle);
        if ( hDirectory != NULL )
        {
            ZwClose(hDirectory);
        }
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GlobalAlloc faild (0x%X)", STATUS_INSUFFICIENT_RESOURCES);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(LinkTarget->Buffer, LinkTarget->MaximumLength);

    //Return value
    //ZwQuerySymbolicLinkObject returns either STATUS_SUCCESS to indicate the routine completed without error or 
    // STATUS_BUFFER_TOO_SMALL if the Unicode string provided at LinkTarget is too small to hold the returned string.
    status = ZwQuerySymbolicLinkObject(handle, LinkTarget, NULL);
    ZwClose(handle);
    if ( hDirectory != NULL )
    {
        ZwClose(hDirectory);
    }

    if (STATUS_SUCCESS != status)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ZwQuerySymbolicLinkObject faild (0x%X), [%ld].", status, GetLastError());
        GlobalFree(LinkTarget->Buffer);
        return status;
    }

    return status;
}
/*------------------------------------------------------------ 
Description  : 初始化符号链接资源
Input        :   
Output       :  
Return       :  MP_TRUE---初始化成功
                MP_FALSE---初始化失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CDisk::InitSymboLinkRes()
{
    if (!LoadNtdllModule())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "LoadNtdllModule failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  : 关闭符号链接资源
Input        :   
Output       :  
Return       :   
                 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CDisk::FreeSymboLinkRes()
{
    FreeNtdllModule();
}
/*------------------------------------------------------------ 
Description  : 查询符号链接信息
Input        :   strSymboLink---符号链接名字，isVolumn---判断卷
Output       :  strTargetDevice---目标设备
Return       :   MP_TRUE---查询成功
                 MP_FALSE---查询失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CDisk::QuerySymboLinkInfo(const mp_string &strSymboLink, mp_string &strTargetDevice, mp_bool isVolumn)
{
    mp_bool initFlag = (NULL == RtlInitUnicodeString)
                        || (NULL == ZwOpenDirectoryObject)
                        || (NULL == ZwOpenSymbolicLinkObject)
                        || (NULL == ZwQuerySymbolicLinkObject)
                        || (NULL == ZwClose);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin query symbolink info (%s).", strSymboLink.c_str());

    if (initFlag == MP_TRUE)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "querySymboLinkInfo faild, not init resource.");
        return MP_FALSE;
    }

    mp_ulong status;
    UNICODE_STRING szSymbolicLink;
    UNICODE_STRING szDeviceName;
    WCHAR *lpszSymbolicLink;
    lpszSymbolicLink = (PWSTR)GlobalAlloc(GPTR, MAX_PATH_LEN * sizeof(WCHAR));
    if (!lpszSymbolicLink){
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GlobalAlloc faild.");
        return MP_FALSE;
    }

    mp_int32 iRet = swprintf_s(lpszSymbolicLink, MAX_PATH_LEN,  L"%S", strSymboLink.c_str());
    if (iRet == MP_FAILED)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ISSP_SNPRINTF_S Faild.");
        GlobalFree(lpszSymbolicLink);
        return MP_FALSE;
    }

    RtlInitUnicodeString(&szSymbolicLink, lpszSymbolicLink);

    status = QuerySymbolicLink(&szSymbolicLink, &szDeviceName, isVolumn);
    if (STATUS_SUCCESS != status)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "QuerySymbolicLink faild [0x%X].", status);
        if (!szDeviceName.Buffer)
        {
            GlobalFree(szDeviceName.Buffer);
        }
        if (!lpszSymbolicLink)
        {
            GlobalFree(lpszSymbolicLink);
        }
        return MP_FALSE;
    }

    wstring wStrTmp = szDeviceName.Buffer;
    strTargetDevice = CMpString::UnicodeToANSI(wStrTmp);
    GlobalFree(szDeviceName.Buffer);
    GlobalFree(lpszSymbolicLink);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query symbolink info succ.");
    return MP_TRUE;
}

#else
/*------------------------------------------------------------ 
Description  :判断设备是否存在
Input        :   strDiskName---磁盘名字
Output       :   
Return       :   MP_TRUE---存在
                 MP_FALSE---不存在
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CDisk::IsDeviceExist(mp_string& strDiskName)
{
    LOGGUARD("");
    //组装设备全名
    mp_string strFullDiskName = "/dev/" + strDiskName;
    mp_string strBuf;

    //打开设备需要root权限
    vector<mp_string> vecRlt;
    mp_int32 iRet = CRootCaller::Exec(ROOT_COMMAND_CAPACITY, strFullDiskName, &vecRlt);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CRootCaller::Exec failed, iRet is %d.", iRet);
        return MP_FALSE;
    }
    if (vecRlt.size() != 1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "vecRlt.size() is not equal to 1, the value is %d.", vecRlt.size());
        return MP_FALSE;
    }
    else
    {
        strBuf = vecRlt[0];
    }

    ISSP_SCSI_SENSE_HDR_S stSshdr = {0};
    iRet = ScsiNormalizeSense(strBuf, stSshdr);
    if (MP_SUCCESS == iRet && stSshdr.ucSenseKey)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "ScsiNormalizeSense: iRet is %d, stSshdr.ucSenseKey is %d.",
            iRet, stSshdr.ucSenseKey);
        return MP_FALSE;
    }

    return MP_TRUE;
}
#endif //WIN32
/*------------------------------------------------------------ 
Description  :获取磁盘容量
Input        :   strDevice---磁盘名字
Output       :   strBuf---容量
Return       :   MP_TRUE---存在
                 MP_FALSE---不存在
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDisk::GetDiskCapacity(mp_string& strDevice, mp_string& strBuf)
{
    LOGGUARD("");
#ifdef LINUX
    //打开scsi设备
    mp_int32 iFd = open(strDevice.c_str(), O_RDWR | O_NONBLOCK);
    if (0 > iFd)
    {
        iFd = open(strDevice.c_str(), O_RDONLY | O_NONBLOCK);
    }
    if (0 > iFd)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open scsi device failed, iFd = %d.", iFd);
        return MP_FAILED;
    }

    mp_int32 iRet, iRetryNum = 0;
    mp_bool bIsBigDisk = MP_FALSE;
    mp_bool bIsBreakOut = MP_TRUE;
    mp_bool bCheckRst = MP_FALSE;
    mp_bool bRet = MP_FALSE;
    mp_uchar aucBuffer[DISK_BYTE_OF_SECTOR] = {0};
    mp_uchar ucSenseBuf[SCSI_MAX_SENSE_LEN] = {0};

    do
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Query capactiy info %s, time %d.", strDevice.c_str(), iRetryNum);
        sg_io_hdr stScsiCmd;
        CHECK_CLOSE_FD(memset_s(&stScsiCmd, sizeof(struct sg_io_hdr), 0, sizeof(struct sg_io_hdr)));
        stScsiCmd.interface_id = 'S';
        stScsiCmd.dxfer_direction = SG_DXFER_FROM_DEV;
        mp_uchar auCdb[CDB16GENERIC_LENGTH] = {0};
        if (!bIsBigDisk)
        {
            auCdb[0] = SCSIOP_READ_CAPACITY;
            //下发给设备的命令
            stScsiCmd.cmdp = auCdb;
            stScsiCmd.cmd_len = CDB10GENERIC_LENGTH;
        }
        else
        {
            auCdb[0] = 0x9e;
            auCdb[1] = 0x10;
            auCdb[13] = 12;

            //下发给设备的命令
            stScsiCmd.cmdp = auCdb;
            stScsiCmd.cmd_len = CDB16GENERIC_LENGTH;
        }
        //CodeDex误报，FORTIFY.Path_Manipulation
        CHECK_CLOSE_FD(memset_s(aucBuffer, DISK_BYTE_OF_SECTOR, 0, DISK_BYTE_OF_SECTOR));
        stScsiCmd.dxferp = aucBuffer;
        stScsiCmd.dxfer_len = DISK_BYTE_OF_SECTOR;

        //下发ioctl的超时时间
        stScsiCmd.timeout = SCSI_CMD_TIMEOUT_LINUX;

        //sense init
        CHECK_CLOSE_FD(memset_s(ucSenseBuf, SCSI_MAX_SENSE_LEN, 0, SCSI_MAX_SENSE_LEN));
        stScsiCmd.sbp = ucSenseBuf;
        stScsiCmd.mx_sb_len = SCSI_MAX_SENSE_LEN;

        //把构造好的命令通过ioctl下发
        iRet = ioctl(iFd, SG_IO, &stScsiCmd);
        //执行成功，但是sg_io_hdr的status错误码为CHECK_CONDITION
        //(具体定义见/usr/include/scsi/scsi.h，获取具体值需要移位后再进行判断)，
        //sense code和sense key在sbp中，需要具体进行解析，解析方式参考内核代码
        //CHECK_CONDITION说明阵列侧发生了状态变化，需要主机端进行检查，
        //当前简单处理在主机侧进行重试，如果重试三次还是存在问题，
        //则还是和之前的处理策略相同，直接退出执行
        //参考http://www.tldp.org/HOWTO/SCSI-Generic-HOWTO/sg_io_hdr_t.html
        //https://www.ibm.com/developerworks/cn/linux/l-scsi-api/
        //http://sg.danny.cz/sg/p/sg_v3_ho.html
        bCheckRst = (iRetryNum < 3 && 0 == iRet && ((stScsiCmd.status >> 1) & 0x7F) == 1);
        if (bCheckRst)
        {
            COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Ioctl failed %s, sg_io_hdr.status %d(CHECK_CONDITION), retry to do it %d.", 
                strDevice.c_str(), stScsiCmd.status, iRetryNum);
            bIsBreakOut = MP_FALSE;
            ++iRetryNum;
            DoSleep(100);
            continue;
        }

        bCheckRst = (iRet < 0 || 0 != stScsiCmd.status);
        if (bCheckRst)
        {

            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Ioctl failed %s, ret %d, cmdStatus %d, len %d.", 
                strDevice.c_str(), iRet, stScsiCmd.status, stScsiCmd.mx_sb_len);
            close(iFd);
            return MP_FAILED;
        }

        if (!bIsBigDisk)
        {
            bRet = ((0xff == (mp_uchar)aucBuffer[0]) && (0xff == (mp_uchar)aucBuffer[1])
                && (0xff == (mp_uchar)aucBuffer[2]) && (0xff == (mp_uchar)aucBuffer[3]));
            if (bRet)
            {
                bIsBreakOut = MP_FALSE;
                bIsBigDisk = MP_TRUE;
                continue;
            }
            bIsBreakOut = MP_TRUE;
        }
        else
        {
            bIsBreakOut = MP_TRUE;
        }
    } while (!bIsBreakOut); //lint !e944

    strBuf = (mp_char*)ucSenseBuf;
    close(iFd);
#endif
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :获取Scsi NormalizeSense 的功能
Input        :    
Output       :  
Return       :   MP_SUCCESS---获取成功
                 MP_FAILED---获取失败
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDisk::ScsiNormalizeSense(mp_string& strBuf, ISSP_SCSI_SENSE_HDR_S& stSSHdr)
{
    LOGGUARD("");
    CHECK_NOT_OK(memset_s(&stSSHdr, sizeof(ISSP_SCSI_SENSE_HDR_S), 0, sizeof(ISSP_SCSI_SENSE_HDR_S)));
    stSSHdr.ucResponseCode = ((mp_uchar)strBuf[0] & 0x7f);

    if ((stSSHdr.ucResponseCode & 0x70) == 0x70)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "stSSHdr.ucResponseCode is %d.", stSSHdr.ucResponseCode);
        return MP_FAILED;
    }

    mp_int32 iSBLen = strBuf.length();
    if (stSSHdr.ucResponseCode >= 0x72)
    {
        //descriptor format
        if (iSBLen > 1)
        {
            stSSHdr.ucSenseKey = ((mp_uchar)strBuf[1] & 0xf);
        }

        if (iSBLen > 2)
        {
            stSSHdr.ucAsc = strBuf[2];
        }

        if (iSBLen > 3)
        {
            stSSHdr.ucAscq = strBuf[3];
        }

        if (iSBLen > 7)
        {
            stSSHdr.ucAdditionalLength = strBuf[7];
        }
        return MP_SUCCESS;
    }
    //else if stSSHdr.ucResponseCode < 0x72
    //fixed format
    if (iSBLen > 2)
    {
        stSSHdr.ucSenseKey = ((mp_uchar)strBuf[2] & 0xf);
    }
    if (iSBLen > 7)
    {
        iSBLen = (iSBLen < (strBuf[7] + 8)) ? iSBLen : (strBuf[7] + 8);

        if (iSBLen > 12)
        {
            stSSHdr.ucAsc = strBuf[12];
        }

        if (iSBLen > 13)
        {
            stSSHdr.ucAscq = strBuf[13];
        }
    }

    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :判断是否sd型磁盘(LINUX)
Input        :strDevice -- 设备名
Output       : 
Return       : MP_TRUE -- 是
                   MP_FALSE -- 否
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_bool CDisk::IsSdisk(mp_string& strDevice)
{
    mp_string::size_type pos = strDevice.find("sd");
    return (pos == 0) ? MP_TRUE : MP_FALSE;
}
/*------------------------------------------------------------ 
Description  :获取磁盘状态
Input        :    strDiskName---磁盘名
Output       :  strStatus---磁盘状态值
Return       :   MP_SUCCESS---获取成功
                 MP_FAILED---获取失败，iRet---对应错误码
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CDisk::GetDiskStatus(mp_string& strDiskName, mp_string& strStatus)
{
    LOGGUARD("");
    mp_string strCmd = "lsdev -Cc disk | awk '$1==\"" + strDiskName + "\" {print $2}'";
    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Exec system cmd failed, cmd is %s, iRet is %d.", strCmd.c_str(), iRet);
        return iRet;
    }

    if (vecRlt.size() < 1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "vecRlt.size() is smaller than 1, the value is %d.",vecRlt.size());
        return MP_FAILED;
    }
    else
    {
        strStatus = vecRlt[0];
    }
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :判断是否hdisk型磁盘(AIX)
Input        :strDevice -- 设备名
Output       : 
Return       : MP_TRUE -- 是
                   MP_FALSE -- 否
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_bool CDisk::IsHdisk(mp_string& strDevice)
{
    mp_string::size_type pos = strDevice.find("hdisk");
    return (pos == 0) ? MP_TRUE : MP_FALSE;
}

/*---------------------------------------------------------------------
Function Name: GetDevNameByWWN
Description  : 根据lun wwn获取对应的设备名称，不带"/dev/"前缀
Input        : strLunWWN LUN WWN；如果是多个LUN组成一个vg，server端有可能下发以逗号分隔的WWN串，
               这时返回的是其中最前面lun所在的dev名称；
               如果需要精确匹配，请拆分成准确的lun wwn，分别调用
Output       :
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CDisk::GetDevNameByWWN(mp_string& strDevName, mp_string& strWWN)
{
#ifndef WIN32
    vector<mp_string> vecDiskName;
    mp_string strLunWWN;
    mp_string strNewDiskName;

    mp_int32 iRet = CDisk::GetAllDiskName(vecDiskName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get all disk name failed, iRet = %d.", iRet);
        return iRet;
    }

    mp_string strVendor, strProduct, strLunID;
    for (vector<mp_string>::iterator it = vecDiskName.begin(); it != vecDiskName.end(); it++)
    {
        //过滤掉非华为厂商设备
        //前面添加"/dev"
#if defined LINUX||AIX
        // 拼装全路径
        strNewDiskName = mp_string("/dev/") + *it;
#elif defined HP_UX_IA
        iRet = CDisk::GetHPRawDiskName(*it, strNewDiskName);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get full disk name of disk(%s) failed, ret %d.",
                it->c_str(), iRet);

            return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
        }
#endif
        iRet = CArray::GetArrayVendorAndProduct(strNewDiskName, strVendor, strProduct);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get Array Vendor And Product failed.", it->c_str());
            //本地磁盘会返回失败，直接跳过;
            continue;
        }
        (void)CMpString::Trim((mp_char*)strVendor.c_str());
        (void)CMpString::Trim((mp_char*)strProduct.c_str());
        if ((0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)) &&
            (0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)) &&
            (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY)))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "device vendor %s is not huawei", strVendor.c_str());
            continue;
        }

        iRet = CArray::GetLunInfo(strNewDiskName, strLunWWN, strLunID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetLunInfo failed, iRet=%d.", iRet);
            return iRet;
        }
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "disk %s LUN WWN: %s, LUN ID: %s",
               strNewDiskName.c_str(), strLunWWN.c_str(), strLunID.c_str());
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Input WWN is %s", strWWN.c_str());
        //存在多个lun组成一个vg的场景，server下发的wwn以分号分隔
        mp_string::size_type pos = strWWN.find(strLunWWN);
        if (mp_string::npos != pos)
        {
            //返回参数不加/dev前缀
            strDevName = *it;
            break;
        }
    }

    if(strDevName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Can not find device by WWN \"%s\".", strWWN.c_str());
        return ERROR_COMMON_DEVICE_NOT_EXIST;
    }
#endif
    return MP_SUCCESS;
}


