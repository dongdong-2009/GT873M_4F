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
#include <errno.h>
#include "plat_common.h"
#include "oam_api.h"
#include "plat_common.h"

#include "packet.h"
#include "sdl_api.h"
#include "cs_cmn.h"
#include "osal_api_core.h"

//#include "mempool.h"

#include "app_vlan.h"
#include "app_ip_types.h"
#include "app_ip_init.h"
#include "app_ip_mgnt.h"
#include "app_ip_api.h"
#include "app_ip_adapter.h"
#include "app_ip_util.h"
/*
* if return code of these routines > 0, just return from the calling place.
* otherwise, contine the code from the calling place
*/
extern cs_uint32 PTY_ENABLE;
app_ipintf_ip_rx_filter_routine_t  app_ipintf_ip_rx_filter = NULL;
app_ipintf_arp_rx_filter_routine_t app_ipintf_arp_rx_filter = NULL;
app_ipintf_ip_tx_filter_routine_t app_ipintf_ip_tx_filter = NULL;
app_ipintf_arp_tx_filter_routine_t app_ipintf_arp_tx_filter = NULL;
app_ipintf_enable_inband_routine_t app_ipintf_enable_inband = NULL;
app_ipintf_enable_inband_routine_t app_ipintf_enable_outband = NULL;


extern void netdev_pkt_recv(cs_pkt_t * event_buf);

/*
*   PROTOTYPE    cs_uint8 *app_ipintf_get_my_macaddr(void)
*   INPUT            None
*   OUTPUT         MAC address of current ONU.
*   Description     Get my mac address.
*   Pre-condition  The ip stack interface module should be already initialized.
*   Post-condition
*   Note  
*/
const cs_uint8 *app_ipintf_get_my_macaddr(void)
{
    return &(ipintf_info.mac[0]);
}

/*
*   PROTOTYPE    void app_ipintf_init_pkt(cs_uint32 len,cs_pkt_t *pkt)
*   INPUT            len  total length of packet include header and payload.
*                       pkt  
*   OUTPUT         None.
*   Description     Set default value to the fields of pkt. 
*   Pre-condition  
*   Post-condition
*   Note             
*/
void app_ipintf_init_pkt(cs_uint32 len,cs_pkt_t *pkt)
{
    memset(pkt,0,len);
    pkt->offset = CS_PKT_OFFSET;
    pkt->len = len-CS_PKT_OFFSET-(sizeof(cs_pkt_t)-1);
}

/*
*   PROTOTYPE    cs_boolean app_ipintf_macaddr_equal(const cs_uint8 *,const cs_uint8 *,cs_uint8 )
*   INPUT            
*   OUTPUT         None.
*   Description     If two mac address are equal, return TRUE, otherwise, return FALSE.
*   Pre-condition  
*   Post-condition
*   Note  
*/
cs_boolean app_ipintf_macaddr_equal(const cs_uint8 *a, const cs_uint8 *b, cs_uint8 len)
{
    cs_uint8 i;

    if (!a || !b) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF, "%s, null pointer\n", __func__);
        return FALSE;
    }

    if (len > 6)
        len = 6;

    for (i=0; i<len; i++) {
        if (a[i] != b[i])
            return FALSE;
    }
    
    return TRUE;
    
}


extern cs_status eth0_ip_get(cs_uint32 *ip);
extern cs_status eth1_ip_get(cs_uint32 *ip);


/*
*   PROTOTYPE    cs_boolean app_ipintf_equal_my_ipaddr(cs_uint32 target_ip)
*   INPUT            target_ip
*   OUTPUT         None.
*   Description     If target ip equal to local ip, return TRUE, otherwise, return FALSE.
*   Pre-condition  
*   Post-condition
*   Note  
*/
cs_boolean app_ipintf_my_ipaddr_equal(cs_uint32 target_ip)
{
#if 0
	//cs_printf("app_ipintf_my_ipaddr_equal---target_ip:0x%08x\n",target_ip);
    if(LOCAL_IPADDR == target_ip ||PTY_IPADDR == target_ip)
    	{
    //		cs_printf("...............................ip equal\n");
        return TRUE;
    	}
    else
    	{
    //	cs_printf("...................................op not equal\n");
        return FALSE;
    	}
#else
	cs_uint32 eth0_ip = 0;
	cs_uint32 eth1_ip = 0;

	eth0_ip_get(&eth0_ip);
	eth1_ip_get(&eth1_ip);
//	cs_printf("eth0ip is %x eht1_ip is %x targetip is %x\r\n",eth0_ip,eth1_ip,target_ip);

#if 0
	cs_printf("target_ip :0x%x, eth0_ip: 0x%x, eth1_ip :0x%x\n", target_ip, eth0_ip, eth1_ip);
#endif


	if(eth0_ip == target_ip || eth1_ip == target_ip)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#endif
return TRUE;
}

cs_int32 app_ipintf_mac2port_get(cs_uint8 *mac)
{
    cs_uint32 i = 0;

    for(i = 0; i < IPINTF_MAC_ENTRY_MAX; i++) {
        if(MACTBL[i].used) {
            /* Have already had this mac, just update the entry. */
            if(app_ipintf_macaddr_equal(MACTBL[i].mac, mac, 6)) {
                return MACTBL[i].dpid;
            }
        }
    }
    
    return -1;
    
}

/*
*   PROTOTYPE    void app_ipintf_port_mac_flush(cs_port_id_t port)
*   INPUT            port
*   OUTPUT         None.
*   Description     Flush all mac address entries on this port.
*   Pre-condition  
*   Post-condition
*   Note  
*/
void app_ipintf_port_mac_flush(cs_port_id_t port)
{
    cs_uint32 i = 0;
    if(port >= ipintf_info.maxport) {
         APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, invalid port %d\n", __func__, port);
         return;
    }
    
    for(i = 0; i < IPINTF_MAC_ENTRY_MAX; i++) {
        if(MACTBL[i].dpid == port) {
            MACTBL[i].used = 0;
        }
    }

    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s, port %d\n", __func__, port);
    
    return;
    
}

/*
*   PROTOTYPE    void app_ipintf_mac_aging(void)
*   INPUT            None
*   OUTPUT         None.
*   Description     Age mac table.
*   Pre-condition  
*   Post-condition
*   Note             This functions is called periodically.
*/
void app_ipintf_mac_aging(void)
{
    cs_uint32 i = 0;

    for(i = 0; i < IPINTF_MAC_ENTRY_MAX; i++) {
        if(MACTBL[i].hit == 0) {
            MACTBL[i].used = 0;
        }
        MACTBL[i].hit = 0;
    }
    
    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s.\n", __func__);
    
    return;
    
}

/*
*   PROTOTYPE    void app_ipintf_mac_learning(cs_pkt_t *pkt)
*   INPUT            pkt
*   OUTPUT         None.
*   Description     Learn source mac address.
*                      If mac address has already been in the table, update the "hit" flag.
*                      otherwise, add source mac to the table if the table is not full.
*   Pre-condition  
*   Post-condition
*   Note             
*/
void app_ipintf_mac_learning(cs_pkt_t *pkt)
{
    cs_uint32 i = 0;
    epon_macaddr_header_t *macHdr = NULL;

    /* sanity check */
    if(pkt->port >= ipintf_info.maxport) {
         APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s, invalid port %d\n", __func__, pkt->port );
         return;
    }
    
    macHdr =(epon_macaddr_header_t*)(pkt->data+pkt->offset);
    for(i = 0; i < IPINTF_MAC_ENTRY_MAX; i++) {
        if(MACTBL[i].used) {
            /* Have already had this mac, just update the entry. */
            if(app_ipintf_macaddr_equal(MACTBL[i].mac, macHdr->src, 6)) {
                MACTBL[i].vlanid = (pkt->svlan&0xfff);
                MACTBL[i].dpid = pkt->port;
                MACTBL[i].hit = 1;
                return;
            }
        }
    }

    /* mac learning */
    for(i = 0; i < IPINTF_MAC_ENTRY_MAX; i++) {
        if(MACTBL[i].used == 0) {
            MACTBL[i].used = 1;
            MACTBL[i].hit = 1;
            MACTBL[i].vlanid = pkt->svlan & 0xfff;
            MACTBL[i].dpid =  pkt->port;
            memcpy(MACTBL[i].mac, macHdr->src, 6*sizeof(cs_uint8));
            return;
        }
    }

    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s, mac table is full\n", __func__);
    
}

/*
*   PROTOTYPE    cs_status app_ipintf_link_status_chk(cs_port_id_t dport)
*   INPUT            dport
*   OUTPUT         None.
*   Description    Check the port link status.
*                      If it is active, return 0, otherwise, return -1.
*   Pre-condition  
*   Post-condition
*   Note             For PON port, if OAM discovery is not finished, return -1.
*/
cs_status app_ipintf_link_status_chk(cs_port_id_t dport)
{
    cs_callback_context_t     context;
    
    if(dport == CS_PON_PORT_ID) {
        if(!(oam_sm_discovery_completed(0))) {
            APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s, PON port is not active\n", __func__);
            return CS_E_ERROR;
        }
    }
    else {
        cs_sdl_port_link_status_t portStatus = SDL_PORT_LINK_STATUS_DOWN;
        epon_request_onu_port_link_status_get(context, 0,0,dport,&portStatus);
        if(!portStatus) {
            APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s, port %d is not active\n", __func__, dport);
            return CS_E_ERROR;
        }
    }
    
    return CS_E_OK;
    
}

/*
*   PROTOTYPE    cs_status send_pkt_with_vlan_check(cs_port_id_t dport, cs_pkt_t *pkt)
*   INPUT            dport
*                       pkt
*   OUTPUT         None.
*   Description     Send packet out.
*                       Before sending it out, apply vlan egress checking.
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status app_ipintf_send_pkt_with_vlan_check(cs_port_id_t dport, cs_pkt_t *pkt, cs_uint8 *buf)
{
    cs_status ret = CS_E_OK;
    cs_uint8 priority = 7;
    cs_uint8 *data = NULL;
    //cs_uint8 *buf = NULL;
    cs_uint32 len = 0;
    cs_callback_context_t     context;

    if(app_ipintf_link_status_chk(dport)) {
        return CS_E_ERROR;
    }
    len = pkt->len;    
    data = pkt->data + pkt->offset;
    ret = vlan_egress_handle(pkt->port, dport, data, buf, &len);
    if(ret == CS_E_ERROR) {
        return CS_E_ERROR;
    }
    
    if(ret == CS_E_OK) {
        ret = epon_request_onu_frame_send(context, ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API,
		                                                  data, len<60?60:len, dport, priority);
    }
    else {
        ret = epon_request_onu_frame_send(context, ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API,
		                                                  buf, len<60?60:len, dport, priority);

    }

    IPINTF_STATIS_INC(SEND, dport);
    return ret;
    
}

/*
*   PROTOTYPE    cs_status send_pkt(cs_port_id_t dport, cs_pkt_t *pkt)
*   INPUT            dport
*                       pkt
*   OUTPUT         None.
*   Description     Send packet out without vlan egress checking.
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status app_ipintf_send_pkt(cs_port_id_t dport, cs_pkt_t *pkt)
{
    cs_status rc = CS_E_OK;
    cs_uint8 priority = 7;
    cs_uint8 *hdr = NULL;
    cs_uint32 len = 0;
    cs_callback_context_t     context;

    if(app_ipintf_link_status_chk(dport)) {
        IPINTF_STATIS_INC(DROP, dport);
        return CS_E_OK;
    }
    hdr = ETH_GET_FRAME_BUFFER(pkt);
    len = pkt->len;
    
    rc = epon_request_onu_frame_send(context, ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API,
		                                                  hdr,len<60?60:len,dport,priority);
    IPINTF_STATIS_INC(SEND, dport);
    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s, dport %d,len %d\n", __func__, dport, len);
    return rc;
    
}

/*
*   PROTOTYPE    cs_status app_ipintf_pkt_send(cs_pkt_t* pkt, cs_uint32 len)
*   INPUT            pkt
*   OUTPUT         None.
*   Description    
*   Pre-condition  
*   Post-condition
*   Note : when app_ipintf_arp_egress_filter return code != 0, it means the whole 
*            process is finished or error, we should return from here.
*            If return code == 0, we should not return and continue the following code. 
*/
cs_status app_ipintf_packet_tx(cs_pkt_t* pkt)
{
    cs_uint16 i = 0;
    cs_uint32 len = pkt->len;
    cs_uint8 *hdr = NULL;
    app_vlan_double_tag_header_t eth_header;
        
    hdr = pkt->data + pkt->offset;
    if(BITON(hdr[0],0)) {
        vlan_header_parse(hdr, &eth_header);
        if(eth_header.ethertype == EPON_ETHERTYPE_ARP) {
            if(app_ipintf_arp_tx_filter) {
                if(app_ipintf_arp_tx_filter(pkt) == IPINTF_DROP) {
                    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s drop, arp tx filter\n", __func__);
                    return CS_E_ERROR;
                }
            }
         }
        /* flooding packets to all ports */
        APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "BC flooding, len %d\n", len);
        for(i=0;i<ipintf_info.maxport;i++) {
            app_ipintf_send_pkt(PTABLE[i].port,pkt);
        }
    }
    else
    {
        for(i=0; i<IPINTF_MAC_ENTRY_MAX; i++) {
            if(MACTBL[i].used && app_ipintf_macaddr_equal(hdr, MACTBL[i].mac, 6)) {
                /* For known unicast packet, push vlan and pri */
                if(MACTBL[i].vlanid != 0) {
                    push_vlan_ingress(MACTBL[i].vlanid, 0, pkt);
                }
                APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "UC send to %d, len %d\n", MACTBL[i].dpid, pkt->len);
                return app_ipintf_send_pkt(MACTBL[i].dpid, pkt);
            }
        }
        /* unknown unicast : flooding */
        APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "DLF UC flooding , len %d\n", pkt->len);
        for(i=0;i<ipintf_info.maxport;i++) {
            app_ipintf_send_pkt(PTABLE[i].port,pkt);
        }
    }

    return CS_E_OK;
    
}

/*
*   PROTOTYPE    cs_status app_ipintf_ip_pre_process(cs_pkt_t* pkt)
*   INPUT            pkt
*   OUTPUT         None.
*   Description     For different customers, there may be different process for IP 
*                       packet before sending to IP stack.
*                       This function calls the special process rountine.
*   Pre-condition  
*   Post-condition
*   Note             e.g. for NTT, all IP packets should be discarded except http packets.
*/
cs_status app_ipintf_ip_pre_process(cs_pkt_t* pkt)
{        
    IPINTF_STATIS_INC(IP,pkt->port);
    if(app_ipintf_ip_rx_filter == NULL ) {
        return CS_E_OK;
    }
    
    if(app_ipintf_ip_rx_filter(pkt) == IPINTF_DROP) {
        return CS_E_ERROR;
    }
    
    return CS_E_OK;

}

/*
*   PROTOTYPE    cs_uint8 app_ipintf_pkt_recv(cs_pkt_t* pkt)
*   INPUT            pkt
*   OUTPUT         None.
*   Description     receive packet from HW.
*   Pre-condition  
*   Post-condition
*   Note             special: only packets from PON need vlan id check
*                      general: allow ARP from GE/FE, without checking vlanid  
*/
cs_uint8 app_ipintf_pkt_recv(cs_pkt_t* pkt)
{
    cs_status rc = CS_E_OK;
    cs_uint32 len = 0;
    cs_uint8 *pbuf = NULL;

    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG0,"%s, sport %d, len %d, vlan %d, type %d\n", 
        __func__, pkt->port, pkt->len, pkt->svlan, pkt->eth_type);

    IPINTF_STATIS_INC(RECV, pkt->port);
   // app_ipintf_mac_learning(pkt);
    
    if(pkt->eth_type == EPON_ETHERTYPE_ARP) {
        rc = app_ipintf_arp_pre_process(pkt);
    }
    else { 
        rc = app_ipintf_ip_pre_process(pkt);
    }
//cs_printf("rc is %d\n",rc);
    if(rc != 0) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3,"%s, does not send to IP stack, 0x%x\n", __func__, (int)pkt);
        IPINTF_STATIS_INC(DROP, pkt->port);
        return rc;
    }
    
    app_ipintf_mac_learning(pkt);
    if(pkt->svlan) {
        len = (cs_uint32)pkt->len;
        pbuf = ETH_GET_FRAME_BUFFER(pkt);
        pop_vlan(pbuf,pbuf,&len);
        //pkt->stpid = 0;
        //pkt->svlan = 0;
        pkt->len = len;
        if(pkt->tag_num >= 1)
            pkt->tag_num -= 1;
    }
    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3,"%s, forward pkt to IP stack, len %d\n", __func__, pkt->len);
    netdev_pkt_recv(pkt);

    return CS_E_OK;
 
}

/*
*   PROTOTYPE    void ethdrv_tm_handle(void)
*   INPUT            None
*   OUTPUT         None.
*   Description     timer routine.
*                       every ETH_TM_TICKS, this function will be called.
*   Pre-condition  
*   Post-condition
*   Note             
*/
void ethdrv_tm_handle(void)
{
#if 0
	cs_printf("in ethdrv_tm_handle, ipintf_info.keepalive :0x%x\n", ipintf_info.keepalive);
#endif
    static cs_uint32 ticks = 0;
    
    ticks += IPINTF_TM_TICKS;

    /* mac address aging */
    if(ticks % ipintf_info.ageinterval == 0) {
        app_ipintf_mac_aging();
    }
    
    /* arp keep alive */
    if(ticks % ipintf_info.keepalive == 0) {
        app_ipintf_arp_keep_alive();
    }
    
  //  APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "%s, %d seconds\n", __func__, ticks);
    
}

/*
*   PROTOTYPE    cs_status app_ipintf_set_wan_cfg(cs_uint8,cs_uint8,cs_uint16,cs_uint32,cs_uint32)
*   INPUT            enable
*                       pri
*                       vlanid  
*                       remote_subnet  
*                       subnet_mask  
*   OUTPUT         None.
*   Description     
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status app_ipintf_set_wan_cfg(
                                                    cs_uint8 enable, 
                                                    cs_uint8 pri,
                                                    cs_uint16 vlanid,
                                                    cs_uint32 remote_subnet, 
                                                    cs_uint32 subnet_mask)
{
    cs_callback_context_t     context;

    APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s inband, pri %d, vlan %d - 0x%08x, 0x%08x\n", 
        enable==APP_IPINTF_INBNAD_MGNT?"enable":"disable", pri,vlanid,remote_subnet, subnet_mask);
    ipintf_info.inband = enable==APP_IPINTF_INBNAD_MGNT ? 1:0;
    ipintf_info.keepalive = ipintf_info.inband?(IPINTF_ARP_KEEPALIVE_ZTE):(IPINTF_ARP_KEEPALIVE_DFT);
    
	if(PTY_ENABLE)
		{
			ipintf_info.pri = pri;
			ipintf_info.vlanid = vlanid;
		}
    else
    	{
			ipintf_info.uartpri = pri;
			ipintf_info.uartvlan= vlanid;
    	}
    ipintf_info.remoteip = remote_subnet;
    ipintf_info.remotemask = subnet_mask;
    if(ipintf_info.inband) {
        epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_DOWN_STREAM, CS_PKT_ARP, DST_CPU);
		epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_UP_STREAM, CS_PKT_ARP, DST_FE);
        epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_DOWN_STREAM, CS_PKT_MYMAC, DST_CPU);
        epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_UP_STREAM, CS_PKT_MYMAC,  DST_CPU);
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, down MYMAC/ARP : CPU, up MYMAC/ARP : FE, \n", __func__);

        app_ipintf_arp_keep_alive();

    }
    else {
        epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_DOWN_STREAM, CS_PKT_ARP, DST_FE);
        epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_UP_STREAM, CS_PKT_ARP, DST_CPU);
        epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_DOWN_STREAM, CS_PKT_MYMAC, DST_FE);
        epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_UP_STREAM, CS_PKT_MYMAC, DST_CPU);
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"%s, down MYMAC/ARP : FE, up MYMAC/ARP : CPU, \n", __func__);

    }

    return CS_E_OK;
    
}

extern cs_uint32 PTY_ENABLE;

/*
*   PROTOTYPE    cs_status app_ipintf_get_ip_address(cs_uint32 *, cs_uint32 *, cs_uint32 *)
*   INPUT            None
*   OUTPUT         *local_ip
*                       *mask
*                       *gw_ip
*   Description     
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status app_ipintf_get_ip_address(cs_uint32 *local_ip, cs_uint32 *mask, cs_uint32 *gw_ip)
{
    /*
    *local_ip = ipintf_info.localip;
    *mask = ipintf_info.ipmask;
    *gw_ip = ipintf_info.gateway;
    */
	
    memcpy(local_ip, &ipintf_info.localip, sizeof(cs_uint32));
    memcpy(mask, &ipintf_info.ipmask, sizeof(cs_uint32));
    memcpy(gw_ip, &ipintf_info.gateway, sizeof(cs_uint32));

    return CS_E_OK;
    
}

/*
*   PROTOTYPE    cs_status app_ipintf_get_wan_cfg(cs_uint8*,cs_uint8*,cs_uint16*,cs_uint32*,cs_uint32*)
*   INPUT           None.
*   OUTPUT         *enable
*                       *pri
*                       *vlanid  
*                       *remote_subnet  
*                       *subnet_mask  
*   Description     
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status app_ipintf_get_wan_cfg(
                                                    cs_uint8 *enable, 
                                                    cs_uint8 *pri,
                                                    cs_uint16 *vlanid,
                                                    cs_uint32 *remote_subnet, 
                                                    cs_uint32 *remote_mask)
{
    cs_uint8 inband = ipintf_info.inband?APP_IPINTF_INBNAD_MGNT:APP_IPINTF_OUTBNAD_MGNT;
    memcpy(enable, &inband, sizeof(cs_uint8));
    memcpy(pri, &ipintf_info.pri, sizeof(cs_uint8));
    memcpy(vlanid, &ipintf_info.vlanid, sizeof(cs_uint16));
    memcpy(remote_subnet, &ipintf_info.remoteip, sizeof(cs_uint32));
    memcpy(remote_mask, &ipintf_info.remotemask, sizeof(cs_uint32));
    
    return CS_E_OK;
    
}

/*
*   PROTOTYPE    cs_status app_ipintf_get_mtu(cs_uint32 *mtu)
*   INPUT           None.
*   OUTPUT         *mtu
*   Description     
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_status app_ipintf_get_mtu(cs_uint32 *mtu)
{
    memcpy(mtu, &ipintf_info.mtu, sizeof(cs_uint32));
    return CS_E_OK;
}

cs_status app_ipintf_check_ip_address(cs_uint32 ip, cs_uint32 mask, cs_uint32 gw)
{
    cs_int32 lan_len = 0;
    cs_int32 non_zero_num = 0;
    cs_int32 i;
    cs_int32 mask_1 = 0;

    /* check ip address validity */
    if( ip == 0 || ip == (cs_uint32)-1) {
        return 1;
    }

    /* check multicast address */
    if((ip >> 28) == 0xe) {
        return 1;
    }

    /* check loopback address(127.0.0.1) */
    if(ip == 0x7f000001) {
        return 1;
    }

    /* check netmask validity */
    if( mask == 0 || mask == 0xffffffff ) {
        return 1;
    }
    for(i=0; i<32; i++) {
        if(mask & (1<<i)) {
            mask_1 = i;
            break;
        }
    }
    for(i=mask_1; i<32; i++) {
        if(mask & (1<<i)) {
            continue;
        }
        return 1;
    }

    /* check subnet address */
    if(0 == (ip & (~mask)) ) {
        return 1;
    }

    /* check broadcast address */
    for(i=0; i<32; i++) {
        if(mask & (1<<i)) {
            continue;
        }

        lan_len++;
        if(ip & (1<<i)) {
            non_zero_num++;
        }
    }

    if(lan_len == non_zero_num) {
        return 1;
    }

    /* all 0 is subnet address */
    for(i=0; i<lan_len; i++) {
        if( (ip>>i) & 1) {
            non_zero_num++;
        }
    }

    if(non_zero_num == 0) {
        return 1;
    }

    /* input gateway, need to check gateway validity */
    if(gw) {
        if( (gw&mask) != (ip&mask) ) {
            return 1;
        }

        non_zero_num = 0;
        for(i=0; i<lan_len; i++) {
            if( (gw>>i) & 1) {
                non_zero_num++;
            }
        }

        // gw ip should not be brocast or all zero
        if(non_zero_num == lan_len || non_zero_num == 0) {
            return 1;
        }
    }

    return 0;
    
}    


cs_status app_ipintf_arp_keepalive_interval_set(cs_uint32 interval)
{
    if(interval == 0)
        ipintf_info.keepalive = IPINTF_ARP_KEEPALIVE_DFT;
    else
        ipintf_info.keepalive = interval;
    
    return CS_E_OK;
    
}

cs_status app_ipintf_arp_del(cs_uint8 *host)
{
    return ipintf_arp_del(host);
}

cs_status app_ipintf_arp_show(cs_uint8 *host)
{
    return ipintf_arp_show(host);
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
cs_status app_ipintf_delete_ip_address(cs_uint32 ip_addr, cs_uint32 mask)
{
    return ipintf_delete_ip_address(ip_addr, mask);
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
cs_status app_ipintf_add_ip_address(cs_uint32 localIp, cs_uint32 gwIp, cs_uint32 mask)
{
    return ipintf_add_ip_address(localIp, gwIp, mask);
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
cs_status app_ipintf_set_mtu(cs_uint32 mtu)
{
    return ipintf_set_mtu(mtu);
}

cs_status app_ipintf_ip_config_set(cs_uint32 local_ip, cs_uint32 ip_mask, cs_uint32 gw_ip)
{
    cs_status ret = CS_E_OK;

    APP_IPINTF_LOG(IROS_LOG_LEVEL_DBG3, "Config ONU IP - 0x%08x, 0x%08x, 0x%08x\n", 
                                                        local_ip, ip_mask, gw_ip);
    /* check parameter sanity 
    ret = app_ipintf_check_ip_address(local_ip, ip_mask, gw_ip);
    if(ret != 0) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF, "%s, invalid IP parameter\n", __func__);
        return CS_E_ERROR;
    }*/
    if((local_ip == ipintf_info.localip) &&
        (ip_mask == ipintf_info.ipmask)&&
        (gw_ip == ipintf_info.gateway)) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF, "IP does change - ip 0x%08x, mask 0x%08x, gw 0x%08x\n", 
                                                        ipintf_info.localip, ipintf_info.ipmask, ipintf_info.gateway);
        return CS_E_OK;
    }
    #if 1
    /* delete current ip address */
    ret = app_ipintf_delete_ip_address(ipintf_info.localip, ipintf_info.ipmask);
    if(ret != 0) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF, "Failed to delete IP - 0x%08x, 0x%08x\n", 
                                                        ipintf_info.localip, ipintf_info.ipmask);
    }
    #endif
    /* add new ip addr */
    ret = app_ipintf_add_ip_address(local_ip, gw_ip, ip_mask);
    if(ret != 0) {
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"Failed to config ONU IP - 0x%08x, 0x%08x, 0x%08x\n", 
                                                        local_ip, ip_mask, gw_ip);
        return CS_E_ERROR;
    }
    return CS_E_OK;
}
cs_status app_ipintf_ping(char *host)
{
    return ipintf_ping(host);
}

void app_ipintf_mac_table_dump(cs_uint32 port)
{
    cs_uint32 i = 0;
    cs_printf("ARP table:\n");
    for(i=0; i<IPINTF_MAC_ENTRY_MAX; i++){
        if(MACTBL[i].used) {
            cs_printf("index = %2d: used %d,hit %d, vlanid %4d, dpid %d,"
                "ESA = %02X %02X %02X %02X %02X %02X\n",i,
                MACTBL[i].used,MACTBL[i].hit, MACTBL[i].vlanid, MACTBL[i].dpid,
                MACTBL[i].mac[0],MACTBL[i].mac[1],MACTBL[i].mac[2],
                MACTBL[i].mac[3],MACTBL[i].mac[4],MACTBL[i].mac[5]);
        }
    }
}

void app_ipintf_reg_event_cb(cs_uint16 dev, cs_int32 type, void* msg)
{
    sdl_event_reg_t *reg;
    if(NULL == msg)
        return;
    
    app_ipintf_port_mac_flush(CS_PON_PORT_ID);
    reg = (sdl_event_reg_t*)msg;
#if 0
	cs_printf("in app_ipintf_reg_event_cb, reg->reg: 0x%x\n", reg->reg);
#endif
    if(reg->reg) {
		
        if(app_ipintf_enable_inband)
            app_ipintf_enable_inband(); 
            
        APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"No need to set in/out band mode \n");

    }
    else {
        if(app_ipintf_enable_outband)
            app_ipintf_enable_outband();
    }
    
    APP_IPINTF_LOG(IROS_LOG_LEVEL_INF,"pon %s \n", reg->reg?"register":"deregister");
    
}

#endif
