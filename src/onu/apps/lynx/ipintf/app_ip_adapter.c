/****************************************************************************
            Software License for Customer Use of Cortina Software
                          Grant Terms and Conditions

IMPORTANT NOTICE - READ CAREFULLY: This Software License for Customer Use
of Cortina Software ("LICENSE") is the agreement which governs use of
software of Cortina Systems, Inc. and its subsidiaries ("CORTINA"),
including computer software (source code and object code) and associated
printed materials ("SOFTWARE").  The SOFTWARE is protected by copyright laws
and international copyright treaties, as well as other intellectual property
laws and treaties.  The SOFTWARE is not sold, and instead is only licensed
for use, strictly in accordance with this document.  Any hardware sold by
CORTINA is protected by various patents, and is sold but this LICENSE does
not cover that sale, since it may not necessarily be sold as a package with
the SOFTWARE.  This LICENSE sets forth the terms and conditions of the
SOFTWARE LICENSE only.  By downloading, installing, copying, or otherwise
using the SOFTWARE, you agree to be bound by the terms of this LICENSE.
If you do not agree to the terms of this LICENSE, then do not download the
SOFTWARE.

DEFINITIONS:  "DEVICE" means the Cortina Systems?LynxD SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems?SDK software.

GRANT OF LICENSE:  Subject to the restrictions below, CORTINA hereby grants
CUSTOMER a non-exclusive, non-assignable, non-transferable, royalty-free,
perpetual copyright license to (1) install and use the SOFTWARE for
reference only with the DEVICE; and (2) copy the SOFTWARE for your internal
use only for use with the DEVICE.

RESTRICTIONS:  The SOFTWARE must be used solely in conjunction with the
DEVICE and solely with Your own products that incorporate the DEVICE.  You
may not distribute the SOFTWARE to any third party.  You may not modify
the SOFTWARE or make derivatives of the SOFTWARE without assigning any and
all rights in such modifications and derivatives to CORTINA.  You shall not
through incorporation, modification or distribution of the SOFTWARE cause
it to become subject to any open source licenses.  You may not
reverse-assemble, reverse-compile, or otherwise reverse-engineer any
SOFTWARE provided in binary or machine readable form.  You may not
distribute the SOFTWARE to your customers without written permission
from CORTINA.

OWNERSHIP OF SOFTWARE AND COPYRIGHTS. All title and copyrights in and the
SOFTWARE and any accompanying printed materials, and copies of the SOFTWARE,
are owned by CORTINA. The SOFTWARE protected by the copyright laws of the
United States and other countries, and international treaty provisions.
You may not remove any copyright notices from the SOFTWARE.  Except as
otherwise expressly provided, CORTINA grants no express or implied right
under CORTINA patents, copyrights, trademarks, or other intellectual
property rights.

DISCLAIMER OF WARRANTIES. THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY
EXPRESS OR IMPLIED WARRANTY OF ANY KIND, INCLUDING ANY IMPLIED WARRANTIES
OF MERCHANTABILITY, NONINFRINGEMENT, OR FITNESS FOR A PARTICULAR PURPOSE,
TITLE, AND NON-INFRINGEMENT.  CORTINA does not warrant or assume
responsibility for the accuracy or completeness of any information, text,
graphics, links or other items contained within the SOFTWARE.  Without
limiting the foregoing, you are solely responsible for determining and
verifying that the SOFTWARE that you obtain and install is the appropriate
version for your purpose.

LIMITATION OF LIABILITY. IN NO EVENT SHALL CORTINA OR ITS SUPPLIERS BE
LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, LOST
PROFITS, BUSINESS INTERRUPTION, OR LOST INFORMATION) OR ANY LOSS ARISING OUT
OF THE USE OF OR INABILITY TO USE OF OR INABILITY TO USE THE SOFTWARE, EVEN
IF CORTINA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
TERMINATION OF THIS LICENSE. This LICENSE will automatically terminate if
You fail to comply with any of the terms and conditions hereof. Upon
termination, You will immediately cease use of the SOFTWARE and destroy all
copies of the SOFTWARE or return all copies of the SOFTWARE in your control
to CORTINA.  IF you commence or participate in any legal proceeding against
CORTINA, then CORTINA may, in its sole discretion, suspend or terminate all
license grants and any other rights provided under this LICENSE during the
pendency of such legal proceedings.
APPLICABLE LAWS. Claims arising under this LICENSE shall be governed by the
laws of the State of California, excluding its principles of conflict of
laws.  The United Nations Convention on Contracts for the International Sale
of Goods is specifically disclaimed.  You shall not export the SOFTWARE
without first obtaining any required export license or other approval from
the applicable governmental entity, if required.  This is the entire
agreement and understanding between You and CORTINA relating to this subject
matter.
GOVERNMENT RESTRICTED RIGHTS. The SOFTWARE is provided with "RESTRICTED
RIGHTS." Use, duplication, or disclosure by the Government is subject to
restrictions as set forth in FAR52.227-14 and DFAR252.227-7013 et seq. or
its successor. Use of the SOFTWARE by the Government constitutes
acknowledgment of CORTINA's proprietary rights therein. Contractor or
Manufacturer is CORTINA.

Copyright (c) 2009 by Cortina Systems Incorporated
****************************************************************************/
#ifdef HAVE_IP_STACK
#include <pkgconf/system.h>
#include <network.h>
#ifndef CYGPKG_LWIP_IPV4_SUPPORT
//#include <network.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/route.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include "plat_common.h"
#include "cs_cmd.h"
#include "app_ip_types.h"
#include "app_ip_init.h"

#else
#include <errno.h>
#include <lwip/netif.h>
#include <lwip/err.h>
#include <lwip/tcpip.h>
#include <lwip/icmp.h>
#include <lwip/sockets.h>

#include "plat_common.h"
#include "app_ip_types.h"
#include "app_ip_init.h"
#include "app_ip_util.h"
#endif
extern cs_uint32 PTY_ENABLE;
#ifndef CYGPKG_LWIP_IPV4_SUPPORT
typedef struct {
    struct  rt_msghdr m_rtm;
    char    m_space[128];
} m_rtmsg_t;
m_rtmsg_t m_rtmsg;

cs_status app_ipintf_get_rt_tbl(size_t *len, cs_uint8 **buf)
{
    cs_uint32 mib[6];
    size_t max_len;
    
    *len = 0;
    *buf = NULL;
    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE; /* AF_ROUTE */
    mib[2] = 0;
    mib[3] = AF_INET;
    mib[4] = NET_RT_FLAGS;
    mib[5] = RTF_LLINFO;
    if(sysctl(mib, 6, NULL, &max_len, NULL, 0) < 0) {
        cs_printf("%s, route-sysctl-estimate, %s\n", __func__, strerror(errno));
        return (1);
    }
    
    if(max_len == 0) {
        cs_printf("No arp entry\n");
        return CS_E_OK;
    }
    
    if(max_len > APP_IPINTF_MAX_ARP_ENTRY_LEN) {
        cs_printf("%s, too many arp entry\n", max_len);
        return CS_E_ERROR;
    }

    if((*buf = iros_malloc(IROS_MID_ETHDRV, max_len)) == NULL) {
        cs_printf("%s, no memory(len %d)\n", __func__, max_len);
        return (1);
    }
    if(sysctl(mib, 6, *buf, &max_len, NULL, 0) < 0) {
        cs_printf("%s, actual retrieval of routing table\n", __func__);
        iros_free(*buf);
        *buf = NULL;
        return (1);
    }
    
    *len = max_len;
    return (0);
    
}


cs_status app_ipintf_dump_rt_entry(struct rt_msghdr *rtm)
{
    struct sockaddr_inarp *sin;
    struct sockaddr_dl *sdl;
 //   struct hostent *hp;
    sin = (struct sockaddr_inarp *)(rtm + 1);
    sdl = (struct sockaddr_dl *)(sin + 1);
    
    cs_printf("%s at ", inet_ntoa(sin->sin_addr));
    if(sdl->sdl_alen) {
        cs_uint8 *mac;
        mac = LLADDR(sdl);
        cs_printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }        
    else
        cs_printf("(incomplete)");

    if (rtm->rtm_rmx.rmx_expire == 0) {
        cs_printf(" permanent");
    }
    if (sin->sin_other & SIN_PROXY) {
        cs_printf(" published (proxy only)");
    }
    if (rtm->rtm_addrs & RTA_NETMASK) {
        sin = (struct sockaddr_inarp *)(sdl->sdl_len + (char *)sdl);
        if (sin->sin_addr.s_addr == 0xffffffff) {
            cs_printf(" published");
        }
        if (sin->sin_len != 8) {
            cs_printf("(wierd)");
        }
    }
    cs_printf("\n");
    
    return CS_E_OK;
    
}

/* Dump the entire arp table */
cs_status app_ipintf_arp_dump(cs_uint32 addr)
{
    cs_status ret = CS_E_OK;
    size_t max_len = 0;
    cs_uint8 *buf = NULL;
    cs_uint8 *next;
    struct rt_msghdr *rtm;
    struct sockaddr_inarp *sin;
    
    ret = app_ipintf_get_rt_tbl(&max_len, &buf);
    if(ret != CS_E_OK) {
        return ret;
    }
    if(max_len == 0) {
        return CS_E_OK;
    }
    /* rtm_msglen = sizeof(rt_msghdr) + sizeof(sockaddr_inarp) */
    for(next = buf; next < buf + max_len; next += rtm->rtm_msglen) {
        rtm = (struct rt_msghdr *)next;
        sin = (struct sockaddr_inarp *)(rtm + 1);
        if(addr) {
            if (addr != sin->sin_addr.s_addr) {
                continue;
            }
            else {
                app_ipintf_dump_rt_entry(rtm);
                break;
            }
        }
        app_ipintf_dump_rt_entry(rtm);
    }

    iros_free(buf);
    return (ret);
}

// Compute INET checksum
cs_status app_ipintf_inet_cksum(cs_uint16 *addr, int len)
{
    cs_uint16 odd_byte = 0;
    cs_uint32 nleft = len;
    cs_uint16 *w = addr;
    cs_uint16 answer;
    cs_uint32 sum = 0;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if( nleft == 1 ) {
        *(cs_uint8 *)(&odd_byte) = *(cs_uint8 *)w;
        sum += odd_byte;
    }

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0x0000ffff); /* add hi 16 to low 16 */
    sum += (sum >> 16); /* add carry */
    answer = ~sum; /* truncate to 16 bits */
    
    return (answer);
    
}

cs_status app_ipintf_icmp_show(
                                                    cs_uint8 *pkt,
                                                    cs_uint32 len,
                                                    struct sockaddr_in *from,
                                                    struct sockaddr_in *to)
{
    cyg_tick_count_t *tp, tv;
    struct ip *ip;
    struct icmp *icmp;
    cs_uint32 interval = 0;
    
    tv = cyg_current_time();
    ip = (struct ip *)pkt;
    if ((len < sizeof(*ip)) || ip->ip_v != IPVERSION) {
        cs_printf("%s: Short packet or not IP! - Len: %d, Version: %d\n",
                                                                                      inet_ntoa(from->sin_addr), len, ip->ip_v);
        return 0;
    }
    icmp = (struct icmp *)(pkt + sizeof(*ip));
    len -= (sizeof(*ip) + 8);
    tp = (cyg_tick_count_t *)&icmp->icmp_data;
    if (icmp->icmp_type != ICMP_ECHOREPLY) {
        cs_printf("%s: Invalid ICMP - type: %d\n", inet_ntoa(from->sin_addr), icmp->icmp_type);
        return 0;
    }
    
    if(ntohs(icmp->icmp_id) != UNIQUEID) {
        cs_printf("%s: ICMP received for wrong id - sent: %x, recvd: %x\n", 
                        inet_ntoa(from->sin_addr), UNIQUEID, ntohs(icmp->icmp_id));
    }
    interval = (tv - *tp) * 10 ; /* ms */
    cs_printf("%d bytes from %s: icmp_seq=%d, time=%dms\n", len, inet_ntoa(from->sin_addr),
                                          ntohs(icmp->icmp_seq), interval==0?IPINTF_PING_INTERVAL_MIN:interval);

    return (from->sin_addr.s_addr == to->sin_addr.s_addr);
}

void app_ipintf_ping_host(cs_int32 s, struct sockaddr_in *host)
{
    cs_uint32 i;
    cs_int32 len;
    cs_uint32 seq;
    cs_uint32 ok_recv = 0;
    cs_uint32 bogus_recv = 0;
    cs_uint32 *dp;
    socklen_t fromlen;
    cyg_tick_count_t *tp;
    cs_uint8 request[MAX_PACKET];
    cs_uint8 response[MAX_PACKET];
    struct sockaddr_in from;
    struct icmp *icmp = NULL;
    
    icmp = (struct icmp *)request;
    for (seq = 0;  seq < NUM_PINGS;  seq++) {
        // Build ICMP packet
        icmp->icmp_type = ICMP_ECHO;
        icmp->icmp_code = 0;
        icmp->icmp_cksum = 0;
        icmp->icmp_seq = htons(seq);
        icmp->icmp_id = htons(0x1234);
        // Set up ping data
        tp = (cyg_tick_count_t *)&icmp->icmp_data;
        *tp++ = cyg_current_time();
        dp = (cs_uint32 *)tp;
        for (i = sizeof(*tp);  i < MIN_PACKET;  i += sizeof(*dp)) {
            *dp++ = i;
        }
        // Add checksum
        icmp->icmp_cksum = app_ipintf_inet_cksum((cs_uint16 *)icmp, MIN_PACKET+8);
        // Send it off
        if(sendto(s, icmp, MIN_PACKET+8, 0, (struct sockaddr *)host, sizeof(*host)) < 0) {
            cs_printf("%s\n", strerror(errno));
            continue;
        }
        // Wait for a response
        fromlen = sizeof(from);
        len = recvfrom(s, response, sizeof(response), 0, (struct sockaddr *)&from, &fromlen);
        if(len < 0) {
            cs_printf("%s\n", strerror(errno));
        }
        else
        {
            if(app_ipintf_icmp_show(response, len, &from, host)) {
                ok_recv++;
            }
            else {
                bogus_recv++;
            }
        }
    }
    cs_printf("Sent %d packets, received %d OK, %d bad\n", NUM_PINGS, ok_recv, bogus_recv);
    
    return;
    
}


cs_status ipintf_ping(char *host)
{
    struct protoent *p;
    struct timeval tv;
    struct sockaddr_in host_addr;
    cs_int32 sock_fd;
    
    if(INADDR_NONE == inet_addr(host)) {
        cs_printf("invalid ip format %s\n", host);
        return CS_E_ERROR;
    }
    
    if ((p = getprotobyname("icmp")) == (struct protoent *)0) {
        cs_printf("%s, getprotobyname failed , errno%d\n", __func__, errno);
        return CS_E_ERROR;
    }

    sock_fd = socket(AF_INET, SOCK_RAW, p->p_proto);
    if(sock_fd < 0) {
        cs_printf("%s, socket failed, fd %d, errno %d\n", __func__, sock_fd, errno);
        return CS_E_ERROR;
    }
    
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // Set up host address
    host_addr.sin_family = AF_INET;
    host_addr.sin_len = sizeof(host_addr);
    host_addr.sin_port = 0;
    host_addr.sin_addr.s_addr = inet_addr(host);
    cs_printf("ping host %s\n", host);
    app_ipintf_ping_host(sock_fd, &host_addr);
    close(sock_fd);

    return CS_E_OK;
}

cs_status ipintf_arp_del(cs_uint8 * host)
{
    cs_printf("Not implement\n");
    return 0;
}
cs_status ipintf_arp_show(cs_uint8 *host)
{
    cs_uint32 ipaddr = 0;
    
    if(host != NULL) {
        ipaddr = inet_addr(host);
        if(INADDR_NONE == ipaddr) {
            cs_printf("invalid ip format %s\n", host);
            return -1;
        }
    }
    
    return app_ipintf_arp_dump(ipaddr);
    
}



/*
*   PROTOTYPE    cs_status app_ipintf_delete_ip_address(cs_uint32 ip_addr, cs_uint32 mask)
*   INPUT            ip_addr
*                       mask  
*   OUTPUT         None.
*   Description     
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status ipintf_delete_ip_address(cs_uint32 ip_addr, cs_uint32 mask)
{
	cs_printf("..........................................1_ip_delete\n");
    struct sockaddr_in *addrp;
    struct ifreq ifr;
    cs_uint32 sock_fd;
    cs_uint32 one = 1;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, fd %d, %s\n", __func__, sock_fd, strerror(errno));
        return CS_E_ERROR;
    }

    if(setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, SO_BROADCAST, %s\n", __func__, strerror(errno));
        close(sock_fd);
        return CS_E_ERROR;
    }

    /* remove ip address */
    addrp = (struct sockaddr_in *) &ifr.ifr_addr;
    memset(addrp, 0, sizeof(*addrp));
    addrp->sin_family = AF_INET;
    addrp->sin_len = sizeof(*addrp);
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = htonl(ip_addr);
    strcpy(ifr.ifr_name, IPINTF_NAME_ETH0);
    if(ioctl(sock_fd, SIOCDIFADDR, &ifr))
    {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, SIOCDIFADDR , %s \n", __func__, strerror(errno));
        close(sock_fd);
        return CS_E_ERROR;
    }
    close(sock_fd);
    APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"Remove ONU IP - %s\n", inet_ntoa(addrp->sin_addr));

    ipintf_info.localip = 0;
    ipintf_info.ipmask = 0;
    
    return CS_E_OK;
    
}

/*
*   PROTOTYPE    cs_status app_ipintf_add_ip_address(cs_uint32 localIp, cs_uint32 gwIp, cs_uint32 mask)
*   INPUT            localIp
*                       gwIp    gateway ip address
*                       mask  
*   OUTPUT         None.
*   Description     
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status ipintf_add_ip_address(cs_uint32 localIp, cs_uint32 gwIp, cs_uint32 mask)
{
    struct sockaddr_in *addrp;
    struct ifreq ifr;
    cs_int32 sock_fd;
    cs_int32 one = 1;
    struct ecos_rtentry route;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, fd %d, %s\n", __func__, sock_fd, strerror(errno));
        return CS_E_ERROR;
    }

    if(setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, SO_BROADCAST, %s\n", __func__, strerror(errno));
        goto out;
    }

    /* set local ip address */
    addrp = (struct sockaddr_in *) &ifr.ifr_addr;
    memset(addrp, 0, sizeof(*addrp));
    addrp->sin_family = AF_INET;
    addrp->sin_len = sizeof(*addrp);
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = htonl(localIp);
    strcpy(ifr.ifr_name, IPINTF_NAME_ETH0);
    if(ioctl(sock_fd, SIOCSIFADDR, &ifr)) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, SIOCSIFADDR, %s\n", __func__, strerror(errno));
        goto out;
    }
    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "Add ONU IP - %s\n", inet_ntoa(addrp->sin_addr));
    ipintf_info.localip = localIp;

    addrp->sin_addr.s_addr = htonl(mask);
    if(ioctl(sock_fd, SIOCSIFNETMASK, &ifr)) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, SIOCSIFNETMASK, %s\n", __func__, strerror(errno));
        goto out;
    }
    APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"Add NETMASK - %s\n", inet_ntoa(addrp->sin_addr));
    ipintf_info.ipmask = mask;

    /*
    * Must do this again so that [sub]netmask (and so default route)
    * is taken notice of.
    */
    addrp->sin_addr.s_addr = htonl(localIp);
    if(ioctl(sock_fd, SIOCSIFADDR, &ifr)) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, SIOCSIFADDR 2, %s\n", __func__, strerror(errno));
        goto out;
    }

    ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if(ioctl(sock_fd, SIOCSIFFLAGS, &ifr)) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, SIOCSIFFLAGS, %s\n", __func__, strerror(errno));
        goto out;
    }

    // Set up routing
    if(gwIp != 0)
    {
        memset(&route, 0, sizeof(route));
        addrp->sin_family = AF_INET;
        addrp->sin_len = sizeof(*addrp);
        addrp->sin_port = 0;
        addrp->sin_addr.s_addr = 0; // 0,0 means GATEWAY for the default route
        memcpy(&route.rt_dst, addrp, sizeof(*addrp));
        addrp->sin_addr.s_addr = 0;
        memcpy(&route.rt_genmask, addrp, sizeof(*addrp));
        addrp->sin_addr.s_addr = htonl(gwIp);
        memcpy(&route.rt_gateway, addrp, sizeof(*addrp));

        route.rt_dev = ifr.ifr_name;
        route.rt_flags = RTF_UP|RTF_GATEWAY;
        route.rt_metric = 0;

        if(ioctl(sock_fd, SIOCADDRT, &route))
        {
            APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"Route - dst: %s, mask: %s, gateway: %s\n", 
                                                    inet_ntoa(((struct sockaddr_in *)&route.rt_dst)->sin_addr),
                                                    inet_ntoa(((struct sockaddr_in *)&route.rt_genmask)->sin_addr),
                                                    inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
            if(errno != EEXIST) {
                APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, SIOCADDRT, %s\n", __func__, strerror(errno));
                goto out;
            }
        }
        else
        {
            APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"Add Gateway - %s\n", 
                                                inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
        }
    }
    ipintf_info.gateway = gwIp;
    close(sock_fd);
    return CS_E_OK;
    
out:
    close(sock_fd);
    return CS_E_ERROR;
}

/*
*   PROTOTYPE    cs_status app_ipintf_set_mtu(cs_uint32 mtu)
*   INPUT            mtu
*   OUTPUT         None.
*   Description     
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status ipintf_set_mtu(cs_uint32 mtu)
{
    struct ifreq ifr;
    cs_int32 sock_fd;
    
    if(mtu < IPINTF_MIN_MTU || mtu > IPINTF_MAX_MTU) {
        return CS_E_ERROR;
    }
    
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, fd %d, %s\n", __func__, sock_fd, strerror(errno));
        return CS_E_ERROR;
    }

    strcpy(ifr.ifr_name, IPINTF_NAME_ETH0);
    ifr.ifr_mtu = mtu;
    /* bsd stack bug, should not judge return code(-1) here */
    ioctl(sock_fd, SIOCSIFMTU, &ifr);
    close(sock_fd);
    
    APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, set MTU from %d to %d\n", __func__, ipintf_info.mtu, mtu);
    ipintf_info.mtu = mtu;

    return CS_E_OK;
    
}    
#else // defined(CYGPKG_NET_LWIP)

extern struct netif * ipintf_net_if_get();
extern struct netif * ipintf_net_if_get_1();
cs_status ipintf_arp_del(cs_uint8 * host)
{
    cs_uint32 ipaddr = 0;
    int ret = 0;
    extern int etharp_arp_del(int ipv4);
            
    
    if(host != NULL) {
        ipaddr = inet_addr(host);
        if(INADDR_NONE == ipaddr) {
            cs_printf("invalid ip format %s\n", host);
            return -1;
        }
    }

    
    ret = etharp_arp_del(ipaddr);
    if(ret == 0) {
        cs_printf("delete ok\n");
    }
    else {
        cs_printf("delete failed");
    }
    return 0;
}

cs_status ipintf_arp_show(cs_uint8 * host)
{
    cs_uint32 ipaddr = 0;
    int ret = 0;
    int state;
    char mac[6];
    extern int etharp_arp_show(int ipv4, int *state, char *mac);

    if(host != NULL) {
        ipaddr = inet_addr(host);
        if(INADDR_NONE == ipaddr) {
            cs_printf("invalid ip format %s\n", host);
            return -1;
        }
        ret = etharp_arp_show(ipaddr, &state, mac);
    }
    else {
        ret = etharp_arp_show(0, &state, mac);
    }

    return ret;
}

#if 1
extern cs_status ip_info_save_to_global(cs_uint32 ip, cs_uint32 mask, cs_uint32	gateway, cs_uint16 vlan);
extern cs_status ip_info_get_from_global(cs_uint32 *ip, cs_uint32 *mask, cs_uint32 *gateway, cs_uint16 *vlan);
extern cs_status uart_ip_info_save_to_global(cs_uint32 uart_ip, cs_uint32 uart_mask, cs_uint32 uart_gateway, cs_uint16 uart_vlan);

extern cs_status uart_ip_info_get_from_global(cs_uint32 *uart_ip, cs_uint32 *uart_mask, cs_uint32 *uart_gateway, cs_uint16 *uart_vlan);

extern cs_status ip_mode_get(int *mode);

static cs_uint32 eth0_ip = 0;
extern cs_status eth0_ip_set(cs_uint32 ip)
{
	cs_status ret = CS_E_OK;
	eth0_ip = ip;
	return ret;
}

extern cs_status eth0_ip_get(cs_uint32 *ip)
{
	cs_status ret = CS_E_OK;
	*ip = eth0_ip;
	return ret;
}


static cs_uint32 eth1_ip = 0;
extern cs_status eth1_ip_set(cs_uint32 ip)
{
	cs_status ret = CS_E_OK;
	eth1_ip = ip;
	return ret;
}

extern cs_status eth1_ip_get(cs_uint32 *ip)
{
	cs_status ret = CS_E_OK;
	*ip = eth1_ip;
	return ret;
}
#endif

#if (LOCAL_IP_NETWORK_CARD == MODULE_YES)
extern cs_status local_ip_info_save_to_global(cs_uint32 ip, cs_uint32 mask, cs_uint32	gateway, cs_uint16 vlan);
extern cs_status local_ip_info_get_from_global(cs_uint32 *ip, cs_uint32 *mask, cs_uint32 *gateway, cs_uint16 *vlan);
#endif

cs_status ipintf_add_ip_address(cs_uint32 localIp, cs_uint32 gwIp, cs_uint32 mask)
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    struct netif *net_if;


#ifdef HAVE_TERMINAL_SERVER
	int mode = 0;
	ip_mode_get(&mode);

	if(0 == mode)
	{
		net_if = ipintf_net_if_get_1();
		if(net_if == NULL) 
		{
		    return CS_E_ERROR;
		}
		else
		{
			//do noting
		}

		cs_uint32	uart_ip;
		cs_uint32	uart_mask;
		cs_uint32	uart_gateway;
		cs_uint16	uart_vlan;
		uart_ip_info_get_from_global(&uart_ip, &uart_mask, &uart_gateway, &uart_vlan);
		uart_ip = (localIp);
		uart_mask = (mask);
		uart_gateway = (gwIp);
		uart_ip_info_save_to_global(uart_ip, uart_mask, uart_gateway, uart_vlan);

		
		cs_uint32 eth1_ip = 0;
		eth1_ip = localIp;
		eth1_ip_set(eth1_ip);
	}
	else
	{
		net_if = ipintf_net_if_get();
		if(net_if == NULL) 
		{
		    return CS_E_ERROR;
		}
		else
		{
			//do noting
		}
		
		cs_uint32	device_ip;
		cs_uint32	device_mask;
		cs_uint32	device_gateway;
		cs_uint16	device_vlan;
		ip_info_get_from_global(&device_ip, &device_mask, &device_gateway, &device_vlan);
		device_ip = (localIp);
		device_mask = (mask);
		device_gateway = (gwIp);
		ip_info_save_to_global(device_ip, device_mask, device_gateway, device_vlan);


		cs_uint32 eth0_ip = 0;
		eth0_ip = localIp;
		eth0_ip_set(eth0_ip);
		
	}

#else

	#if (LOCAL_IP_NETWORK_CARD == MODULE_YES)
	int mode = 0;
	ip_mode_get(&mode);
	//cs_printf("mode :%d\n", mode);
	if(3 == mode)
	{
		net_if = ipintf_net_if_get_1();
		if(net_if == NULL) 
		{
		    return CS_E_ERROR;
		}
		else
		{
			//do noting
		}
		
		cs_uint32	local_ip;
		cs_uint32	local_mask;
		cs_uint32	local_gateway;
		cs_uint16	local_vlan;

		local_ip_info_get_from_global(&local_ip, &local_mask, &local_gateway, &local_vlan);
		local_ip = (localIp);
		local_mask = (mask);
		local_gateway = (gwIp);
		local_ip_info_save_to_global(local_ip, local_mask, local_gateway, local_vlan);

		
		cs_uint32 eth1_ip = 0;
		eth1_ip = localIp;
		eth1_ip_set(eth1_ip);
	}
	else
	{
		net_if = ipintf_net_if_get();
		if(net_if == NULL) 
		{
			return CS_E_ERROR;
		}
		else
		{
			//do noting
		}

		cs_uint32	device_ip;
		cs_uint32	device_mask;
		cs_uint32	device_gateway;
		cs_uint16	device_vlan;
		ip_info_get_from_global(&device_ip, &device_mask, &device_gateway, &device_vlan);
		device_ip = (localIp);
		device_mask = (mask);
		device_gateway = (gwIp);
		ip_info_save_to_global(device_ip, device_mask, device_gateway, device_vlan);

		cs_uint32 eth0_ip = 0;
		eth0_ip = localIp;
		eth0_ip_set(eth0_ip);
	}
	#endif
	


#endif

	#if 0
	cs_printf("in ipintf_add_ip_address, localIp :0x%x\n", localIp);
	#endif
    ipaddr.addr = htonl(localIp);
    netmask.addr = htonl(mask);
    gw.addr = htonl(gwIp);
    
    netif_set_addr(net_if, &ipaddr, &netmask, &gw);
    APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, set ip 0x%x, mask 0x%x, gateway 0x%x\n", 
        __func__, localIp, mask, mask);
	if(PTY_ENABLE)
		{
		    ipintf_info.ptyip= localIp;
		    ipintf_info.ptymask= mask;
		    ipintf_info.ptygtw= gwIp;
		}
	else
		{
		    ipintf_info.localip = localIp;
		    ipintf_info.ipmask = mask;
		    ipintf_info.gateway= gwIp;
		}

	
    return CS_E_OK;
    
}
cs_status ipintf_delete_ip_address(cs_uint32 ip_addr, cs_uint32 mask)
{
    return ipintf_add_ip_address(0, 0, 0);
}
cs_status ipintf_set_mtu(cs_uint32 mtu)
{
    struct netif *net_if;
    
    if(mtu > IPINTF_MAX_MTU || mtu < IPINTF_MIN_MTU) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, %d invalid parameter(%d~%d)\n", 
            __func__, mtu, IPINTF_MIN_MTU, IPINTF_MAX_MTU);
        return CS_E_ERROR;
    }
    net_if = ipintf_net_if_get();
    if(net_if == NULL) {
        return CS_E_ERROR;
    }
    APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, set MTU from %d to %d\n", __func__, net_if->mtu, mtu);
    net_if->mtu = mtu;
    ipintf_info.mtu = mtu;
    return CS_E_OK;
    
}


// Compute INET checksum
cs_status app_ipintf_inet_cksum(cs_uint16 *addr, int len)
{
    cs_uint16 odd_byte = 0;
    cs_uint32 nleft = len;
    cs_uint16 *w = addr;
    cs_uint16 answer;
    cs_uint32 sum = 0;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if( nleft == 1 ) {
        *(cs_uint8 *)(&odd_byte) = *(cs_uint8 *)w;
        sum += odd_byte;
    }

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0x0000ffff); /* add hi 16 to low 16 */
    sum += (sum >> 16); /* add carry */
    answer = ~sum; /* truncate to 16 bits */
    
    return (answer);
    
}

cs_status app_ipintf_icmp_show(
                                                    cs_uint8 *pkt,
                                                    cs_uint32 len,
                                                    struct sockaddr_in *from,
                                                    struct sockaddr_in *to)
{
    cyg_tick_count_t *tp, tv;
    struct ip_hdr *ip;
    struct icmp_echo_hdr *icmp;
    cs_uint32 interval = 0;
    
    tv = cyg_current_time();
    ip = (struct ip *)pkt;
    if ((len < sizeof(*ip)) || IPH_V(ip) != 4) {
        cs_printf("%s: Short packet or not IP! - Len: %d, Version: %d\n",
                                                                              inet_ntoa(from->sin_addr), len, IPH_V(ip));
        return 0;
    }
    icmp = (struct icmp_echo_hdr *)(pkt + sizeof(*ip));
    len -= (sizeof(*ip) + 8);
    tp = (cyg_tick_count_t *)(pkt + sizeof(*ip) + sizeof(*icmp));
    if (ICMPH_TYPE(icmp) != ICMP_ER) {
        cs_printf("%s: Invalid ICMP - type: %d\n", inet_ntoa(from->sin_addr), ICMPH_TYPE(icmp));
        return 0;
    }
    if(ntohs(icmp->id) != UNIQUEID) {
        cs_printf("%s: ICMP received for wrong id - sent: %x, recvd: %x\n", 
                        inet_ntoa(from->sin_addr), UNIQUEID, ntohs(icmp->id));
    }
    interval = (tv - *tp) * 10 ; /* ms */
    cs_printf("%d bytes from %s: icmp_seq=%d, time=%dms\n", len, inet_ntoa(from->sin_addr),
                                          ntohs(icmp->seqno), interval==0?1:interval);

    return (from->sin_addr.s_addr == to->sin_addr.s_addr);

}

cs_status ipintf_icmp_process(
    cs_int32 fd, 
    struct sockaddr *from, 
    cs_int32 *fromlen, 
    cs_uint8 *resp,
    cs_int32 *resp_len)
{
    cs_int32 rc = 0;
    cs_int32 len;
    fd_set rfds;
    struct timeval tv = {1, 0};

    memset(&rfds, 0, sizeof(rfds));
    FD_SET(fd, &rfds);

    rc = select(fd + 1, &rfds, NULL, NULL, &tv);
    if(rc <= 0) {
        cs_printf("No response\n");
        return CS_E_ERROR;
    }
    if(FD_ISSET(fd, &rfds)) {
        len = recvfrom(fd, resp, *resp_len, 0, from, fromlen);
        if(len > 0) {
            *resp_len = len;
            return CS_E_OK;
        }
    }
    
    cs_printf("No response, %d\n", __LINE__);
    return CS_E_ERROR;
}

void app_ipintf_ping_host(cs_int32 s, struct sockaddr_in *host)
{
    cs_int32 len = 0;
    cs_uint32 seq;
    cs_uint32 ok_recv = 0;
    cs_uint32 bogus_recv = 0;
    cs_uint32 *dp;
    int fromlen;
    cs_status rc = CS_E_OK;
    cyg_tick_count_t *tp;
    cs_uint8 request[48];
    cs_uint8 response[48];
    struct sockaddr_in from;
    struct icmp_echo_hdr *icmp;

    icmp = (struct icmp_echo_hdr *)request;
    for (seq = 0;  seq < NUM_PINGS;  seq++) {
        // Build ICMP packet
        memset(request, 0 , 48);
        ICMPH_TYPE_SET(icmp, ICMP_ECHO);
        ICMPH_CODE_SET(icmp, 0);
        icmp->chksum = 0;
        icmp->seqno = htons(seq);
        icmp->id = htons(0x1234);
        // Set up ping data
        tp = (cyg_tick_count_t *)&(request[sizeof(struct icmp_echo_hdr)]);
        *tp++ = cyg_current_time();
        dp = (cs_uint32 *)tp;

        // Add checksum
        icmp->chksum = app_ipintf_inet_cksum((cs_uint16 *)icmp,  sizeof(request));
        // Send it off
        if(sendto(s, icmp, sizeof(request), 0, (struct sockaddr *)host, sizeof(*host)) < 0) {
            cs_printf("%s\n", strerror(errno));
            continue;
        }
        // Wait for a response 
        fromlen = sizeof(from);
        len = sizeof(response);
        rc = ipintf_icmp_process(s, (struct sockaddr *)&from, &fromlen, response, &len);
        if(rc == CS_E_OK) {
            if(app_ipintf_icmp_show(response, len, &from, host)) { 
                ok_recv++;
            }
            else {
                bogus_recv++;
            }
        }

    }
    cs_printf("Sent %d packets, received %d OK, %d bad\n", NUM_PINGS, ok_recv, bogus_recv);

    return;

}

cs_status ipintf_ping(char *host)
{
    cs_int32 sock_fd;
    struct sockaddr_in addr;
    struct sockaddr_in host_addr;

    cs_uint32 local_ip;
    cs_uint32 mask;
    cs_uint32 gw_ip;
    cs_uint32 host_ip;

    host_ip = inet_addr(host);
    if(INADDR_NONE == host_ip) {
        cs_printf("invalid ip format %s\n", host);
        return CS_E_ERROR;
    }

    /* check local ip */
    app_ipintf_get_ip_address(&local_ip, &mask, &gw_ip);
    local_ip = ntohl(local_ip);
    if(local_ip == host_ip || host_ip == 0x0100007f) {   /* ping local ip */
        cs_uint32 i;

        cs_printf("ping host %s\n", host);
        for(i=0; i<4; i++) {
            cs_printf("20 bytes from %s: icmp_seq=%d, time=%dms\n", host, i, 1);
        }
        cs_printf("Sent 4 packets, received 4 OK, 0 bad\n");

        return CS_E_OK;
    }

    sock_fd = socket(AF_INET, SOCK_RAW, 1);
    if(sock_fd < 0) {
        cs_printf("%s, socket failed, fd %d, errno %d\n", __func__, sock_fd, errno);
        return CS_E_ERROR;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock_fd,(struct sockaddr *)&addr,sizeof(addr));
 
    // TODO : if socket supports SO_RCVTIMEO, it could reduce some code.
    // Set up host address
    host_addr.sin_family = AF_INET;
    host_addr.sin_len = sizeof(host_addr);
    host_addr.sin_port = 0;
    host_addr.sin_addr.s_addr = inet_addr(host);
    
    cs_printf("ping host %s\n", host);
    app_ipintf_ping_host(sock_fd, &host_addr);
    close(sock_fd);

    return CS_E_OK;

    
}
#endif
#endif

