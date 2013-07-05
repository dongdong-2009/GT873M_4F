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
#ifndef _APP_IP_UTIL_H_
#define _APP_IP_UTIL_H_
#include "app_ip_types.h"
#include "cs_cmn.h"

extern cs_status app_ipintf_arp_show(cs_uint8 *host);
typedef ipintf_filter_retcode (*app_ipintf_ip_rx_filter_routine_t) (cs_pkt_t *pkt);
typedef ipintf_filter_retcode (*app_ipintf_arp_rx_filter_routine_t) (cs_pkt_t *pkt);
typedef ipintf_filter_retcode (*app_ipintf_ip_tx_filter_routine_t) (cs_pkt_t *pkt);
typedef ipintf_filter_retcode (*app_ipintf_arp_tx_filter_routine_t) (cs_pkt_t *pkt);
typedef cs_status (*app_ipintf_enable_inband_routine_t) ();

extern app_ipintf_ip_rx_filter_routine_t  app_ipintf_ip_rx_filter ;
extern app_ipintf_arp_rx_filter_routine_t app_ipintf_arp_rx_filter;
extern app_ipintf_ip_tx_filter_routine_t app_ipintf_ip_tx_filter;
extern app_ipintf_arp_tx_filter_routine_t app_ipintf_arp_tx_filter;
extern app_ipintf_enable_inband_routine_t app_ipintf_enable_inband ;
extern app_ipintf_enable_inband_routine_t app_ipintf_enable_outband ;

#define ETH_GET_FRAME_BUFFER(p) (((cs_pkt_t*)(p))->data + ((cs_pkt_t*)(p))->offset)
#define ETH_GET_FRAME_LENGTH(p)  (((cs_pkt_t*)(p))->len)
extern void app_ipintf_port_mac_flush(cs_uint32 port);
extern cs_uint8 app_ipintf_pkt_recv(cs_pkt_t* pkt);
extern cs_boolean app_ipintf_my_ipaddr_equal(cs_uint32 target_ip);
extern cs_boolean  app_ipintf_macaddr_equal(const cs_uint8 *mac_a, const cs_uint8 *mac_b, cs_uint8 len);
extern cs_status app_ipintf_check_ip_address(cs_uint32 ip, cs_uint32 mask, cs_uint32 gw);
extern cs_status app_ipintf_send_pkt(cs_port_id_t port_id, cs_pkt_t *pkt);
extern cs_status app_ipintf_send_pkt_with_vlan_check(cs_port_id_t port_id, cs_pkt_t *pkt, cs_uint8 *);
extern cs_status app_ipintf_packet_tx(cs_pkt_t* pkt);
extern void app_ipintf_init_pkt(cs_uint32 len,cs_pkt_t *pkt);
extern void ethdrv_tm_handle(void);
extern cs_status ethdrv_arp_keepalive_interval_set(cs_uint32 interval);

extern cs_status app_ipintf_add_ip_address(cs_uint32 localIp, cs_uint32 gwIp, cs_uint32 mask);
extern cs_status app_ipintf_set_mtu(cs_uint32 mtu);
extern cs_status app_ipintf_delete_ip_address(cs_uint32 ip_addr, cs_uint32 mask);
extern cs_status app_ipintf_set_wan_cfg(
                                                    cs_uint8 enable, 
                                                    cs_uint8 pri,
                                                    cs_uint16 vlanid,
                                                    cs_uint32 remote_subnet, 
                                                    cs_uint32 subnet_mask);



extern cs_status app_ipintf_get_ip_address(cs_uint32 *local_ip, cs_uint32 *mask, cs_uint32 *gw_ip);
extern cs_status app_ipintf_get_wan_cfg(
                                                    cs_uint8 *enable, 
                                                    cs_uint8 *pri,
                                                    cs_uint16 *vlanid,
                                                    cs_uint32 *remote_subnet, 
                                                    cs_uint32 *remote_mask);
extern cs_status app_ipintf_get_mtu(cs_uint32 *mtu);
extern cs_status app_ipintf_arp_keepalive_interval_set(cs_uint32 interval);
extern cs_status app_ipintf_ip_config_set(cs_uint32 local_ip, cs_uint32 ip_mask, cs_uint32 gw_ip);
extern cs_status app_ipintf_arp_keep_alive(void);
extern cs_status app_ipintf_arp_pre_process(cs_pkt_t* pkt);
extern const cs_uint8 *app_ipintf_get_my_macaddr(void);
extern void app_ipintf_mac_table_dump(cs_uint32 port);
extern cs_status app_ipintf_arp_del(cs_uint8 *host);
extern void app_ipintf_reg_event_cb(cs_uint16 dev, cs_int32 type, void* msg);
#endif/* _APP_IP_UTIL_H_ */

