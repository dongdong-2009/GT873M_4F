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
#include <network.h>
#include "mempool.h"
#include "osal_api_core.h"
#include "startup_cfg.h"
#include "cs_module.h"
#include "plat_common.h"
#include "packet.h"
//#include "iros_cmn_msg.h"
#include "event.h"
#include "app_timer.h"
#include "sdl_api.h"
#include "sdl_pktctrl.h"

#include "app_ip_types.h"
#include "app_ip_mgnt.h"
#include "app_ip_init.h"
#include "app_ip_util.h"
#include "cli_common.h"

static  app_timer_msg_t age_timer;
ipintf_info_t ipintf_info;
extern cs_status ipintf_cmd_reg();

/*
*   PROTOTYPE    cs_uint32 app_ipintf_get_max_port()
*   INPUT            None.
*   OUTPUT         None
*   Description     Get max port number from startup configure.
*   Pre-condition  
*   Post-condition
*   Note             
*/
cs_uint32 app_ipintf_get_max_port()
{
    cs_uint8 port_num;
    
    startup_config_read(CFG_ID_SWITCH_PORT_NUM, 1, &port_num);
    port_num++; /* add pon port */
    
    return port_num;
}

/*
*   PROTOTYPE    cs_status app_ipintf_arp_pkt_parser(cs_pkt_t *pPkt)
*   INPUT            pPkt
*   OUTPUT         None
*   Description     
*   Pre-condition  
*   Post-condition
*   Note  
*/
cs_status app_ipintf_arp_pkt_parser(cs_pkt_t *pPkt)
{
    if(pPkt->eth_type == EPON_ETHERTYPE_ARP) {
        pPkt->pkt_type = CS_PKT_ARP;
        return CS_E_OK;
    }
    else {
        return CS_E_NOT_SUPPORT;
    }
}

/*
*   PROTOTYPE    cs_status app_ipintf_ip_pkt_parser(cs_pkt_t *pPkt)
*   INPUT            pPkt
*   OUTPUT         None
*   Description     
*   Pre-condition  
*   Post-condition
*   Note  
*/
cs_status app_ipintf_ip_pkt_parser(cs_pkt_t *pPkt)
{
    if(EPON_ETHERTYPE_IP == pPkt->eth_type) {
        cs_callback_context_t context;
        cs_mac_t mac;

        epon_request_onu_pon_mac_addr_get(context, 0, 0, &mac);
        if(!memcmp(pPkt->data + pPkt->offset, mac.addr, 6)) {
            pPkt->pkt_type = CS_PKT_IP;
            return CS_E_OK;
        }
    }

    return CS_E_NOT_SUPPORT;
}

/*
*   PROTOTYPE    void app_ipintf_link_change_cb(cs_uint16 evt_dev, cs_uint32 evt_type, void *data)
*   INPUT            
*   OUTPUT         None
*   Description     
*   Pre-condition  
*   Post-condition
*   Note  
*/
void app_ipintf_link_change_cb(cs_uint16 evt_dev, cs_uint32 evt_type, void *data)
{
    sdl_event_port_link_t *msg = (sdl_event_port_link_t*)data;
    
    if(msg == NULL)
        return;
    
    app_ipintf_port_mac_flush(msg->port);

    return;
}

/*
*   PROTOTYPE    void app_ipintf_mac_aging_init()
*   INPUT            void
*   OUTPUT         None
*   Description     
*   Pre-condition  
*   Post-condition
*   Note  
*/
void app_ipintf_mac_aging_init(void)
{
    cs_uint32 ageHandle = 0;
    static cs_uint32 init = 0;
    if(!init)
    {
        age_timer.msg_type = IROS_MSG_TIME_OUT;
        age_timer.timer_type = APP_TIMER_ARPAGING;
        ageHandle = cs_msg_circle_timer_add(app_msg_q,IPINTF_TM_TICKS*1000, 
                                                                (void *)&age_timer);
        if(ageHandle == CS_INVALID_TIMER) {
            IROS_LOG_MIN(IROS_MID_ETHDRV, "arp aging timer initialization, add timer failed.\n");
        }
        init = 1;
    }
}



/*
*   PROTOTYPE    void app_ipintf_device_init(eth_info_t *eth_info)
*   INPUT            eth_info
*   OUTPUT         None
*   Description     
*   Pre-condition  
*   Post-condition
*   Note  
*/
void app_ipintf_device_init(ipintf_info_t *eth_info)
{
    cs_uint8 *mac = eth_info->mac;
    memset(eth_info, 0, sizeof(ipintf_info_t));
    eth_info->localip = IPINTF_DFT_IP_ADDR;/*192.168.0.1*/
    eth_info->ipmask = IPINTF_DFT_IP_MASK; /*255.255.255.0*/
    eth_info->inband = 0;
    eth_info->pri = 0;
    eth_info->vlanid = 0;
    eth_info->remoteip = 0;
    eth_info->remotemask = 0;
    eth_info->gateway = 0;
    eth_info->mtu = IPINTF_DFT_MTU;/*max MTU 1500*/
    eth_info->ageinterval = IPINTF_MAC_AGING_INTERVAL;
    eth_info->keepalive = IPINTF_ARP_KEEPALIVE_DFT;
    startup_config_read(CFG_ID_MAC_ADDRESS , ETHER_ADDR_LEN,eth_info->mac);
    cs_printf("my mac = %02X:%02X:%02X:%02X:%02X:%02X\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

}


/*
*   PROTOTYPE    ipintf_protocol_config(void)
*   INPUT            void
*   OUTPUT         None
*   Description     
*   Pre-condition  
*   Post-condition
*   Note  
*/
cs_status ipintf_protocol_config(void)
{

    app_pkt_reg_parser(CS_PKT_ARP, app_ipintf_arp_pkt_parser);
    app_pkt_reg_handler(CS_PKT_ARP, app_ipintf_pkt_recv);
    app_pkt_reg_parser(CS_PKT_IP, app_ipintf_ip_pkt_parser);
    app_pkt_reg_handler(CS_PKT_IP, app_ipintf_pkt_recv);
    app_ipintf_pkt_dir_default();
    
    return CS_E_OK;
}

#if 1
//引用其他模块的函数
extern onu_slow_path_cfg_cfg_t   g_slow_path_ip_cfg;
extern cs_status ip_mode_set(int mode);
extern cs_status ip_info_save_to_global(cs_uint32 ip, cs_uint32 mask, cs_uint32	gateway, cs_uint16 vlan);
extern cs_status ip_check(cs_uint32 ip);
extern cs_status mask_check(cs_uint32 mask);
extern cs_status gateway_check(cs_uint32	gateway);
extern cs_status vlan_check(cs_uint16 vlan);

//本模块的函数
static cs_status device_ip_set_default(cs_uint32 *ip, cs_uint32 *mask, cs_uint32 *gateway, cs_uint16 *vlan);
extern cs_status ip_info_check(cs_uint32 ip, cs_uint32 mask, cs_uint32 gateway, cs_uint16 vlan);
extern cs_status device_ip_init(void);


/**********************************************************************************************************************************************
*函数名:device_ip_init
*函数功能描述：设备ip 的初始化(flash 读取失败后，采用默认配置)
*函数参数：
*函数返回值：cs_status
*作者：朱晓辉
*函数创建日期：2013-5-24
*函数修改日期：尚未修改
*修改人：尚未修改
*修改原因：尚未修改
*版本：1.0
*历史版本：无
**********************************************************************************************************************************************/
extern cs_status device_ip_init(void)
{
	cs_uint32 device_ip = 0;
	cs_uint32 device_mask = 0;
	cs_uint32 device_gateway = 0;
	cs_uint16 device_vlan = 0;

#if 0
	cs_printf("in device_ip_init\n");
#endif
	cs_status ret = CS_E_OK;

	int status = 0;
	status = get_userdata_from_flash((unsigned char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(g_slow_path_ip_cfg));
	
	if(0 != status)
	{
		cs_printf("get ipdate fail..\n");

		#if 0
		memset(&g_slow_path_ip_cfg, 0, sizeof(g_slow_path_ip_cfg));
		char ip[] = "192.168.0.1";
		char mask[] = "255.255.255.0";
		char gateway[] = "192.168.0.1";
		g_slow_path_ip_cfg.device_ip = ntohl(inet_addr(ip));
		g_slow_path_ip_cfg.device_mask = ntohl(inet_addr(mask));
		g_slow_path_ip_cfg.device_gateway = ntohl(inet_addr(gateway));
		#endif

		device_ip_set_default(&device_ip, &device_mask, &device_gateway, &device_vlan);
		
		g_slow_path_ip_cfg.device_ip = device_ip;
		g_slow_path_ip_cfg.device_mask = device_mask;
		g_slow_path_ip_cfg.device_gateway = device_gateway;
		g_slow_path_ip_cfg.device_vlan = device_vlan;
		int status_save = 0;
		status_save = save_userdata_to_flash((unsigned char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(g_slow_path_ip_cfg));
		if(0 != status_save)
		{
			cs_printf("save_userdata_to_flash failed\n");
		}
		else
		{
			//do nothing
		#if 1
			cs_printf("save_userdata_to_flash success\n");
		#endif
		}
	}
	else
	{
		//do nothing
		device_ip = g_slow_path_ip_cfg.device_ip;
		device_mask = g_slow_path_ip_cfg.device_mask;
		device_gateway = g_slow_path_ip_cfg.device_gateway;
		device_vlan = g_slow_path_ip_cfg.device_vlan;
		
		ret = ip_info_check(device_ip, device_mask, device_gateway, device_vlan);
		if(CS_E_OK == ret)
		{
			//do nothing
		}
		else
		{
			device_ip_set_default(&device_ip, &device_mask, &device_gateway, &device_vlan);
			
			g_slow_path_ip_cfg.device_ip = device_ip;
			g_slow_path_ip_cfg.device_mask = device_mask;
			g_slow_path_ip_cfg.device_gateway = device_gateway;
			g_slow_path_ip_cfg.device_vlan = device_vlan;
			int status_save = 0;
			status_save = save_userdata_to_flash((unsigned char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(g_slow_path_ip_cfg));
			if(0 != status_save)
			{
				cs_printf("save_userdata_to_flash failed\n");
			}
			else
			{
				//do nothing
			#if 1
				cs_printf("save_userdata_to_flash success\n");
			#endif
			}
		}
		
	}

#if 1
	device_ip = g_slow_path_ip_cfg.device_ip;
	device_mask = g_slow_path_ip_cfg.device_mask;
	device_gateway = g_slow_path_ip_cfg.device_gateway;
	
	ip_info_save_to_global(device_ip, device_mask, device_gateway, device_vlan);
	
	ip_mode_set(1);
	app_ipintf_add_ip_address(device_ip, device_gateway, device_mask);
	ip_mode_set(1);
#endif

#if 0
	cs_printf("device_ip :0x%x, device_gateway :0x%x, device_mask :0x%x\n", device_ip, device_gateway, device_mask);
#endif
	#if 1
	g_slow_path_ip_cfg.inband_ip = device_ip;
	g_slow_path_ip_cfg.inband_ip_mask = device_mask;
	g_slow_path_ip_cfg.inband_gateway = device_gateway;
	g_slow_path_ip_cfg.outband_ip = device_ip;
	g_slow_path_ip_cfg.outband_ip_mask = device_mask;
	g_slow_path_ip_cfg.outband_gateway = device_gateway;
	#endif
	return ret;

}



/**********************************************************************************************************************************************
*函数名:		device_ip_set_default
*函数功能描述：将设备 ip 相关信息设置成默认配置
*函数参数：
*函数返回值：cs_status
*作者：朱晓辉
*函数创建日期：2013-5-24
*函数修改日期：尚未修改
*修改人：尚未修改
*修改原因：尚未修改
*版本：1.0
*历史版本：无
**********************************************************************************************************************************************/
static cs_status device_ip_set_default(cs_uint32 *ip, cs_uint32 *mask, cs_uint32 *gateway, cs_uint16 *vlan)
{
	cs_status ret = CS_E_OK;
	char ip_default[] = "192.168.0.1";
	char mask_default[] = "255.255.255.0";
	char gateway_default[] = "192.168.0.1";
	*ip =  ntohl(inet_addr(ip_default));
	*mask = ntohl(inet_addr(mask_default));
	*gateway = ntohl(inet_addr(gateway_default));
	
	return ret;
}



/**********************************************************************************************************************************************
*函数名:		ip_info_check
*函数功能描述：ip 相关信息的有效性检查
*函数参数：
*函数返回值：cs_status
*作者：朱晓辉
*函数创建日期：2013-5-24
*函数修改日期：尚未修改
*修改人：尚未修改
*修改原因：尚未修改
*版本：1.0
*历史版本：无
**********************************************************************************************************************************************/
extern cs_status ip_info_check(cs_uint32 ip, cs_uint32 mask, cs_uint32 gateway, cs_uint16 vlan)
{
	cs_status ret = CS_E_OK;
	
	ret = ip_check(ip);
	if(CS_E_OK != ret)
	{
		cs_printf("ip_check failed\n");
		goto end;
	}
	else
	{
		//do nothing
	}

	ret = mask_check(mask);
	if(CS_E_OK != ret)
	{
		cs_printf("mask_check failed\n");
		goto end;
	}
	else
	{
		//do nothing
	}
	
	ret = gateway_check(gateway);
	if(CS_E_OK != ret)
	{
		cs_printf("gateway failed\n");
		goto end;
	}
	else
	{
		//do nothing
	}
	
	ret = vlan_check(ip);
	if(CS_E_OK != ret)
	{
		cs_printf("ip_check failed\n");
		goto end;
	}
	else
	{
		//do nothing
	}


	
	
end:
	return ret;
}





#endif

extern cs_status device_ip_init(void);
extern cs_status uart_ip_init(void);

/*
*   PROTOTYPE    cs_status app_ipintf_init()
*   INPUT            void
*   OUTPUT         None
*   Description     
*   Pre-condition  
*   Post-condition
*   Note  
*/
cs_status app_ipintf_init(void)
{
    cs_uint32 i = 0;
    cs_uint32 len = 0;

    app_ipintf_device_init(&ipintf_info);
    ipintf_info.maxport = app_ipintf_get_max_port();
    
    /* init port */
    len = sizeof(ipintf_port_info_t) *ipintf_info.maxport;
    ipintf_info.port = (ipintf_port_info_t*)iros_malloc(IROS_MID_ETHDRV, len);
    if(!ipintf_info.port)
    {
        cs_printf("%s failed, no memory \n", __func__);
        return -1;
    }
    memset(ipintf_info.port, 0, len);

    for(i=0;i<ipintf_info.maxport;i++)
    {
        ipintf_info.port[i].port = i;
    }
    /* init mac table */
    len = sizeof(ipintf_mac_entry_t) * IPINTF_MAC_ENTRY_MAX;
    ipintf_info.mactbl= (ipintf_mac_entry_t*)iros_malloc(IROS_MID_ETHDRV,len);
    if(!ipintf_info.mactbl)
    {
        cs_printf("%s failed, no memory \n", __func__);
        return -1;
    }
    memset(ipintf_info.mactbl, 0, len);	
        
    /* register callback handle for link state change */
    onu_evt_reg(EPON_EVENT_PORT_LINK_CHANGE, app_ipintf_link_change_cb, NULL);
    onu_evt_reg(EPON_EVENT_REG_CHANGE, app_ipintf_reg_event_cb ,NULL);

    app_ipintf_pkt_special_process_reg();
    app_ipintf_mac_aging_init();

	device_ip_init();
	
	#ifdef HAVE_TERMINAL_SERVER
	uart_ip_init();
	#endif
	
    app_ipintf_set_mtu(ipintf_info.mtu);
    ipintf_protocol_config();
    ipintf_cmd_reg();
    cs_printf("%s port num %d\n", __func__, ipintf_info.maxport);
    
    return CS_E_OK;
    
}


/*
*   PROTOTYPE    cs_status init_ip_service(void)
*   INPUT            void
*   OUTPUT         None
*   Description     
*   Pre-condition  
*   Post-condition
*   Note  
*/
#ifdef CYGPKG_LWIP_IPV4_SUPPORT
extern int lwip_init(void);
#endif
cs_status init_ip_service(void)
{
    cs_status ret = 0;
    #ifdef CYGPKG_LWIP_IPV4_SUPPORT
    /* init ip stack */
    lwip_init();
    #endif
    ret = app_ipintf_init();
    if(ret != 0) {
        cs_printf("%s - app_ipintf_init, failed %d\n",__func__, ret);
        return CS_E_ERROR;
    }

    return ret;
    
}

#endif


