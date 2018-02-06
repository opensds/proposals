/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#ifdef WIN32
#pragma comment(lib, "Netapi32.lib")

//#include <windows.h>
#else
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef AIX
#include <arpa/inet.h>
#elif defined SOLARIS
#include <sys/sockio.h>
#endif
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <netdb.h>
//#include <errno.h>
#endif
#include "common/Mac.h"
#include "common/Log.h"
#include "common/LogCode.h"
#include "common/Defines.h"
#include "securec.h"

#ifdef WIN32
/*------------------------------------------------------------ 
Description  :获得所有本地Mac地址
Input        :       
Output       :     mac---mac地址
Return       :     MP_SUCCESS---成功
                     
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMacAddr::GetAllLocalMacAddr(vector<mp_string>& mac)
{
    mp_char buf[BUF_LEN] = {0};
    NCB ncb;
    LANA_ENUM AdapterList;
    CHECK_NOT_OK(memset_s(&ncb, sizeof(ncb), 0, sizeof(ncb)));
    ncb.ncb_command = NCBENUM;
    ncb.ncb_buffer = (mp_uchar*)&AdapterList;
    ncb.ncb_length = sizeof(AdapterList);
    (void)Netbios(&ncb);
    mac.clear();

    for (mp_uint32 i = 0; i < AdapterList.length ; ++i)
    {
        struct ASTAT
        {
            ADAPTER_STATUS adapt;
            NAME_BUFFER    psz_name[30];
        } Adapter;

        // Reset the LAN adapter so that we can begin querying it
        NCB Ncb;
        CHECK_NOT_OK(memset_s(&Ncb, sizeof(Ncb), 0, sizeof(Ncb)));
        Ncb.ncb_command  = NCBRESET;
        Ncb.ncb_lana_num = AdapterList.lana[i];
        if (Netbios(&Ncb) != NRC_GOODRET)
        {
            continue;
        }

        // Prepare to get the adapter status block
        CHECK_NOT_OK(memset_s(&Ncb, sizeof(Ncb), 0, sizeof(Ncb)));
        Ncb.ncb_command = NCBASTAT;
        Ncb.ncb_lana_num = AdapterList.lana[i];
        CHECK_NOT_OK(strcpy_s((mp_char*)Ncb.ncb_callname, sizeof(Ncb.ncb_callname), "*"));
        CHECK_NOT_OK(memset_s(&Adapter, sizeof(Adapter), 0, sizeof(Adapter)));
        Ncb.ncb_buffer = (mp_uchar*)&Adapter;
        Ncb.ncb_length = sizeof (Adapter);

        // Get the adapter's info and, if this works, return it in standard,
        // colon-delimited form.
        if (Netbios(&Ncb) == 0)
        {
            CHECK_FAIL(sprintf_s(buf, sizeof(buf), "%02X-%02X-%02X-%02X-%02X-%02X",
                Adapter.adapt.adapter_address[0],
                Adapter.adapt.adapter_address[1],
                Adapter.adapt.adapter_address[2],
                Adapter.adapt.adapter_address[3],
                Adapter.adapt.adapter_address[4],
                Adapter.adapt.adapter_address[5]));

            COMMLOG(OS_LOG_INFO, LOG_COMMON_ERROR, "Get mac address of this host, mac address %s.",
                buf);

            mac.push_back(buf);
        }
    }

    SortMac(mac);

    return MP_SUCCESS;
}
#else
/*------------------------------------------------------------ 
Description  :获得所有本地Mac地址
Input        :       
Output       :     mac---mac地址
Return       :     MP_SUCCESS---成功
                     MP_FAILED---失败
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMacAddr::GetAllLocalMacAddr(vector<mp_string>& mac)
{
    mp_int32 fd;
    struct ifreq maclist[MAXINTERFACES];
    struct arpreq arp;
    struct ifconf ifc;
    struct sockaddr_dl *sdl=0;
    caddr_t addrchar;
    mp_uchar myhaddr[6] = {0};
    mp_char buf[BUF_LEN] = {0};
    mp_uint64 num = 0;
    mp_uint64 intrface = 0;
    mp_int32 iRet = MP_SUCCESS;

    CHECK_NOT_OK(memset_s(&arp, sizeof(struct arpreq), 0x00, sizeof(struct arpreq)));
    CHECK_NOT_OK(memset_s(maclist, sizeof(maclist), 0x00, sizeof(maclist)));
    CHECK_NOT_OK(memset_s(&ifc, sizeof(struct ifconf), 0x00, sizeof(struct ifconf)));
    CHECK_NOT_OK(memset_s(&addrchar, sizeof(caddr_t), 0x00, sizeof(caddr_t)));

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof(maclist);
        ifc.ifc_buf = (caddr_t) maclist;
        if (!ioctl (fd, SIOCGIFCONF, (mp_char*) &ifc))
        {
            //get interface num
            num = ifc.ifc_len / sizeof (struct ifreq);
            while (intrface < num )
            {

#if defined(AIX)
                if (AF_LINK == maclist[intrface].ifr_addr.sa_family)
                {
                    sdl = (struct sockaddr_dl *) (&maclist[intrface].ifr_addr);
                    if(sdl->sdl_alen > 0)
                    {
                        addrchar = LLADDR(sdl);
                        for (mp_uint32 j = 0; j < 6; j++)
                        {
                            myhaddr[j] = (mp_uchar)(addrchar[j]);
                        }
                    }
                }
#else
                if(!ioctl(fd,SIOCGIFFLAGS,(mp_char*)&maclist[intrface]))
                {
                    //filter loop interface
                    if (maclist[intrface].ifr_flags&IFF_LOOPBACK)
                    {
                        intrface++;
                        continue;
                    }

#if defined(__SUN_SOLARIS)||defined(HP_UX_IA)

                    arp.arp_pa.sa_family = AF_INET;
                    arp.arp_ha.sa_family = AF_INET;
                    (reinterpret_cast<struct sockaddr_in*>(&arp.arp_pa))->sin_addr.s_addr=(
                        reinterpret_cast<struct sockaddr_in*>(&maclist[intrface].ifr_addr))->sin_addr.s_addr;

                    //Get HW ADDRESS of the net card
                    if (!(ioctl (fd, SIOCGARP, (mp_char*) &arp)))
                    {
                        for (mp_uint32 j = 0; j < 6; j++)
                        {
                            myhaddr[j] = (mp_uchar)(arp.arp_ha.sa_data[j]);
                        }
                    }
                    else
                    {
                        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ioctl excute SIOCGARP failed, code %d, detail %s.",
                            errno, strerror(errno));
                        intrface++;
                        continue;
                    }

#elif defined(LINUX)
                    //Get HW ADDRESS of the net card
                    if (!(ioctl (fd, SIOCGIFHWADDR, (mp_char*) &maclist[intrface])))
                    {
                        for (mp_uint32 j = 0; j < 6; j++)
                        {
                            myhaddr[j] = (mp_uchar)(maclist[intrface].ifr_hwaddr.sa_data[j]);
                        }
                    }
                    else
                    {
                        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ioctl excute SIOCGIFHWADDR failed, code %d, detail %s.",
                            errno, strerror(errno));
                        intrface++;
                        continue;

                    }
#endif
                }
                else
                {
                    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ioctl excute SIOCGIFFLAGS failed, code %d, detail %s.",
                        errno, strerror(errno));
                    intrface++;
                    continue;

                }
#endif
                iRet = sprintf_s(buf, BUF_LEN,"%02X-%02X-%02X-%02X-%02X-%02X",
                        myhaddr[0],
                        myhaddr[1],
                        myhaddr[2],
                        myhaddr[3],
                        myhaddr[4],
                        myhaddr[5]);
                if (MP_FAILED== iRet)
                {
                    close (fd);
                    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call sprintf_s failed, ret %d.", iRet);

                    return MP_FAILED;
                }

                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get mac address of this host, mac address %s.", buf);

                mac.push_back(buf);
                intrface++;
            }
        }
        else
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ioctl excute SIOCGIFCONF failed, code %d, detail %s.",
                errno, strerror(errno));
        }

        close (fd);
    }

    SortMac(mac);

    return MP_SUCCESS;
}//lint !e529

#endif
/*------------------------------------------------------------ 
Description  :整理mac
Input        :      mac---Mac地址
Output       :      
Return       :      
                     
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CMacAddr::SortMac(vector<mp_string>& mac)
{
    if (mac.size() > 1)
    {
        std::sort(mac.begin(), mac.end());
    }
}

