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
 
DEFINITIONS:  "DEVICE" means the Cortina Systems? Daytona SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems? SDK software.
 
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
#include "cs_types.h"
#include "plat_common.h"

#include "oam_std_pdu.h"
#include "oam_ctc_pdu.h"
#include "oam_ctc_client.h"
#include "app_oam_ctc_adapt.h"
#include "app_ip_init.h"
#include "app_ip_mgnt.h"
#ifdef HAVE_LOOP_DETECT
#include "loop_detect.h"
#endif
#include "sdl_api.h"
#include "app_stats_ctc.h"
#include "app_alarm_ctc.h"
#ifdef HAVE_MC_CTRL
#include "mc_api.h"
#include "mc_ctcctrl.h"
#endif
#ifdef HAVE_ZTE_OAM
#include "db_instance.h"
#endif

#include "sdl_vlan.h"
#include "port_stats.h"
#include "oam_std_pdu.h"
#include "cli_common.h"

#define TAG2VID(tag)    ((tag)&0x0fff)
#define TAG2PRI(tag)    (((tag)&0xe000)>>13)
cs_uint32 PTY_ENABLE = 0;
extern cs_int32 onu_evt_reg(cs_int32 evt_type , void *func , void *cookie);

static cs_uint32 g_laser_tx_power_timer = 0;
extern ipintf_info_t ipintf_info;
static void __laser_tx_power_on(void *data)
{
    cs_callback_context_t   context;
    memset(&context, 0, sizeof(context));
    epon_request_onu_pon_trans_tx_pwr_set(context, 0, 0, 1);
    g_laser_tx_power_timer = 0;
}

cs_status oam_port_event_register_adapt (
         void                        *call_back)
{
    onu_evt_reg(EPON_EVENT_PORT_LINK_CHANGE, (void*)call_back, NULL);
    return CS_E_OK;
}

#ifdef HAVE_MC_CTRL
extern mc_fwd_action_t mc_snoop_ingress_filter(cs_pkt_t *pkt, cs_uint8 type, cs_uint8 *chg);
extern mc_fwd_action_t mc_ctrl_ingress_filter(cs_pkt_t *pkt, cs_uint8 type, cs_uint8 *chg);
extern cs_status mc_egress_filter(cs_port_id_t port, cs_uint8 type, cs_uint8 *frame, cs_uint16 *len);
#endif

/***************** SDL ADAPT START ****************************/
cs_status ctc_oam_eth_admin_state_get_adapt(
        cs_port_id_t port,
        cs_uint8*  admin)
{
    //TODO:not completed
    /* admin
#define OAM_INTF_PHY_ADMIN_DIS                  1
#define OAM_INTF_PHY_ADMIN_EN                   2
     */
    cs_status ret;
    cs_sdl_port_admin_t port_admin = 0;
    cs_callback_context_t context;

    ret = epon_request_onu_port_admin_get(context, 
        0,0, port, &port_admin);
    if (ret) {
        return ret;
    }

    if (port_admin == SDL_PORT_ADMIN_DOWN) {
        *admin = OAM_INTF_PHY_ADMIN_DIS;
    } else {
        *admin = OAM_INTF_PHY_ADMIN_EN;
    }

    return CS_E_OK;
}


//duchen: OAM_INTF_PHY_ADMIN_DIS?????
cs_status ctc_oam_eth_auto_admin_get_adapt(
        cs_port_id_t port,
        cs_uint8*  admin)
{
    //TODO:not completed
    /* admin
#define OAM_INTF_PHY_ADMIN_DIS                  1
#define OAM_INTF_PHY_ADMIN_EN                   2
     */
    cs_status ret;
    cs_sdl_port_autoneg_status_t auto_neg = 0;
    cs_callback_context_t context;

    ret = epon_request_onu_port_auto_neg_get(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, &auto_neg);
    if (ret) {
        return ret;
    }

    if (auto_neg == SDL_PORT_ADMIN_DOWN) {
        *admin = OAM_INTF_PHY_ADMIN_DIS;
    } else {
        *admin = OAM_INTF_PHY_ADMIN_EN;
    }

    return CS_E_OK;
}

cs_status ctc_oam_eth_auto_local_cap_get_adapt(
        cs_port_id_t port,
        cs_uint32 * value,
        cs_uint8 * getNum)
{
    //TODO:not completed
    /* max value num is 9 */
    /*
       global(0),--reserved for future use.
       other(1),--undefined
       unknown(2),--initializing, true ability not yet known.
       10BASE-T(14),--10BASE-T as defined in Clause 14
       10BASE-TFD(142), --Full duplex 10BASE-T as defined in Clauses 14 and 31
       100BASE-T4(23),--100BASE-T4 as defined in Clause 23
       100BASE-TX(25),--100BASE-TX as defined in Clause 25
       100BASE-TXFD(252), --Full duplex 100BASE-TX as defined in Clauses 25 and 31
       10GBASE-KX4(483), --10GBASE-KX4 PHY as defined in Clause 71
       10GBASE-KR(495), --10GBASE-KR PHY as defined in Clause 72
       10GBASE-T(55),--10GBASE-T PHY as defined in Clause 55
       FDX PAUSE(312), --PAUSE operation for full duplex links as defined in
       Annex 31BFDX APAUSE(313), --Asymmetric PAUSE operation for full duplex links as defined
       in Clause 37 and Annex 31B
       FDX SPAUSE(314), --Symmetric PAUSE operation for full duplex links as defined
       in Clause 37 and Annex 31B
       FDX BPAUSE(315), --Asymmetric and Symmetric PAUSE operation for full duplex
       links as defined in Clause 37 and Annex 31B
       100BASE-T2(32),--100BASE-T2 as defined in Clause 32
       100BASE-T2FD(322), --Full duplex 100BASE-T2 as defined in Clauses 31 and 32
       1000BASE-X(36),--1000BASE-X as defined in Clause 36
       1000BASE-XFD(362), --Full duplex 1000BASE-X as defined in Clause 36
       1000BASE-KX(393), --1000BASE-KX PHY as defined in Clause 70
       1000BASE-T(40),--1000BASE-T UTP PHY as defined in Clause 40
       1000BASE-TFD(402), --Full duplex 1000BASE-T UTP PHY to be defined in Clause 40
       Rem Fault1(37),--Remote fault bit 1 (RF1) as specified in Clause 37
       Rem Fault2(372), --Remote fault bit 2 (RF1) as specified in Clause 37
       isoethernet(8029) --802.9 ISLAN-16T
       */

    //TOPO: code from LynxC, not sure the Lynxd has change the cap
#ifndef HAVE_MPORTS
    value[0] = 0x28;
    value[1] = 0x192;
    value[2] = 0x142;
    *getNum = 3;
#else
    value[0] = 14;
    value[1] = 142;
    value[2] = 23;
    value[3] = 25;
    value[4] = 252;
    value[5] = 312;
    value[6] = 314;
    value[7] = 32;
    value[8] = 322;
    *getNum = 9;
#endif
    
    return CS_E_OK;
}



cs_status ctc_oam_eth_auto_advts_cap_get_adapt(
        cs_port_id_t port,
        cs_uint32 * value,
        cs_uint8 * getNum)
{
    //TODO:not completed
#ifndef HAVE_MPORTS
    value[0] = 0x28;
    value[1] = 0x192;
    value[2] = 0x142;
    *getNum = 3;
#else
    value[0] = 14;
    value[1] = 142;
    value[2] = 23;
    value[3] = 25;
    value[4] = 252;
    value[5] = 312;
    value[6] = 314;
    value[7] = 32;
    value[8] = 322;
    *getNum = 9;
#endif
    
    return CS_E_OK;
}

cs_status ctc_oam_eth_fec_ability_get_adapt(
        cs_uint8 *ability)
{
    /*
    #define OAM_INTF_FEC_ABILITY_UNKOWN             1
    #define OAM_INTF_FEC_SUPPORDTED                 2
    #define OAM_INTF_FEC_NOT_SUPPORTED              3
    */
    
    cs_callback_context_t     context;
    cs_status                 retCode;
    cs_boolean                enable;

    retCode =  epon_request_onu_fec_ability_get(context, 
                                        ONU_DEVICEID_FOR_API,
                                        ONU_LLIDPORT_FOR_API, 
                                        &enable);   
                                        
    if(enable == 0)
        *ability = OAM_INTF_FEC_NOT_SUPPORTED;
    else if(enable == 1)
        *ability = OAM_INTF_FEC_SUPPORDTED;
    else
        *ability = OAM_INTF_FEC_ABILITY_UNKOWN;
        
    return retCode;

}

cs_status ctc_oam_eth_fec_mode_set_adapt(
        cs_uint32 fecMode)
{
     /*
     * fec Mode: 1. unknow
     *           2. enable 
     *           3. Disable 
     #define OAM_INTF_FEC_MODE_UNKOWN                1
     #define OAM_INTF_FEC_MODE_EN                    2
     #define OAM_INTF_FEC_MODE_DIS                   3
     */
     
    cs_callback_context_t     context;
    cs_boolean                mode;
    cs_status                 retCode;
    
    if(fecMode==OAM_INTF_FEC_MODE_EN)
       mode = 1;
    else
       mode = 0;    

    retCode =  epon_request_onu_fec_set(context, 
                                        ONU_DEVICEID_FOR_API, 
                                        ONU_LLIDPORT_FOR_API, 
                                        mode);
 
    return retCode;
    
}

cs_status ctc_oam_eth_fec_mode_get_adapt(
        cs_uint32* fecMode)
{
    //TODO:not completed
    /*
    *fecMode = OAM_INTF_FEC_MODE_EN;
    return CS_E_OK;
    */
    cs_callback_context_t     context;
    cs_boolean                mode;
    cs_status                 retCode;
    
    retCode =  epon_request_onu_fec_get(context, 
                                             ONU_DEVICEID_FOR_API, 
                                             ONU_LLIDPORT_FOR_API, 
                                             &mode);
                                             
                                             
     if(mode == 1)
        *fecMode = OAM_INTF_FEC_MODE_EN;
     else if(mode == 0)
        *fecMode = OAM_INTF_FEC_MODE_DIS;
     else
        *fecMode = OAM_INTF_FEC_MODE_UNKOWN;    
    
    return retCode;
}




cs_status ctc_oam_onu_active_pon_get_adapt(
        cs_uint8* pon)
{
    *pon = 1; 
    return CS_E_OK;
}

cs_status ctc_oam_onu_active_pon_set_adapt(
        cs_uint8 pon)
{
    return CS_E_NOT_SUPPORT;
}

cs_status ctc_oam_eth_port_link_get_adapt(
        cs_port_id_t port,
        cs_uint8*  linkState)
{
    //TODO:not completed
    /* linkState  
     #define OAM_CTC_ONU_ETH_DOWN                0x0
     #define OAM_CTC_ONU_ETH_UP                  0x1
     */
    cs_status ret;
    cs_sdl_port_link_status_t    link_status = 0;
    cs_callback_context_t context;

    ret = epon_request_onu_port_link_status_get(context, 
        0, 0, port, &link_status);
	//epon_request_onu_port_link_status_get(context, 0, 0, port, &link_status);

    if (ret) {
        return ret;
    }

    if (link_status == SDL_PORT_LINK_STATUS_DOWN) {
        *linkState = OAM_CTC_ONU_ETH_DOWN;
    } else {
        *linkState = OAM_CTC_ONU_ETH_UP;
    }

    return CS_E_OK;

}


cs_status ctc_oam_eth_port_pause_get_adapt(
        cs_port_id_t port,
        cs_uint8*  flowControl)
{
    cs_status ret;
    cs_callback_context_t context;
    cs_boolean enable;

    ret = epon_request_onu_port_flow_ctrl_get(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, &enable);
    
    *flowControl = enable;
        
    return ret;

}

cs_status ctc_oam_eth_port_pause_set_adapt(
        cs_port_id_t port,
        cs_uint8 flowControl)
{   
    cs_status ret;
    cs_callback_context_t context;
    cs_boolean enable;
#ifdef HAVE_ZTE_OAM 
    db_zte_port_t port_info;
#endif    
    enable = flowControl>0 ? 1 : 0;
    ret = epon_request_onu_port_flow_ctrl_set(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, enable);
#ifdef HAVE_ZTE_OAM         
    memset(&port_info,0x00,sizeof(port_info));
    db_read(DB_ZTE_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
    port_info.port[port - 1].pause = enable;
    db_write(DB_ZTE_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
#endif
    return ret;

}



cs_status ctc_oam_eth_admin_state_set_adapt(
        cs_port_id_t port,
        cs_uint32 phyAdmin)
{
    cs_status ret;
    cs_sdl_port_admin_t port_admin = 0;
    cs_callback_context_t context;

    if (phyAdmin == OAM_INTF_PHY_ADMIN_DIS) {
        port_admin = SDL_PORT_ADMIN_DOWN;
    } else {
        port_admin = SDL_PORT_ADMIN_UP;
    }

    ret = epon_request_onu_port_admin_set(context, 
        0,0, port, port_admin);

    return ret;
}


cs_status ctc_oam_auto_neg_restart_set_adapt(
        cs_port_id_t port)
{
    cs_status ret;
    cs_callback_context_t context;

    ret = epon_request_onu_port_autoneg_restart(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port);

    return ret;
}

cs_status ctc_oam_auto_neg_ctrl_set_adapt(
        cs_port_id_t port,
        cs_uint32 autoNegAdmin)
{
    cs_status ret;
    cs_sdl_port_speed_cfg_t cfg = 0;
    cs_callback_context_t context;
    cs_uint8  auto_neg = SDL_PORT_AUTONEG_DOWN;
#ifdef HAVE_ZTE_OAM 
    db_zte_port_t   port_info;
#endif
    ret = epon_request_onu_port_status_get(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, &cfg);
    if (ret) {
        return ret;
    }
    if (autoNegAdmin == OAM_INTF_PHY_ADMIN_DIS) {
        if (cfg == SDL_PORT_AUTO_10_100_1000 || 
            cfg == SDL_PORT_AUTO_10_100 || 
            cfg == SDL_PORT_1000_FULL) {
            cfg = SDL_PORT_100_FULL;
            ret = epon_request_onu_port_status_set(context, 
                ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, cfg);
        }
        auto_neg = SDL_PORT_AUTONEG_DOWN;
    } else {
        if (cfg != SDL_PORT_AUTO_10_100_1000 && 
            cfg != SDL_PORT_AUTO_10_100 &&
            cfg != SDL_PORT_1000_FULL) {
            cfg = SDL_PORT_AUTO_10_100_1000;
            ret = epon_request_onu_port_status_set(context, 
                ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, cfg);
        }
        auto_neg = SDL_PORT_AUTONEG_UP;
    }

#ifdef HAVE_ZTE_OAM 
    if(ret == CS_E_OK){
        if(port <= DB_ZTE_MAX_PORT && port > 0){
            memset(&port_info,0x00,sizeof(port_info));
            db_read(DB_ZTE_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
            port_info.port[port - 1 ].auto_neg = auto_neg;
            db_write(DB_ZTE_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
        }
    }
#endif
    return ret;
}

cs_status ctc_oam_eth_port_policing_get_adapt(
        cs_port_id_t port,
        cs_uint8  *oper,
        cs_uint32 *cir,
        cs_uint32 *cbs,
        cs_uint32 *ebs)
{
    cs_status ret;
    cs_callback_context_t context;
    cs_sdl_policy_t       policy;

    ret = epon_request_onu_port_policy_get(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, &policy);
    if (ret) {
        return ret;
    }

    *oper = policy.enable;
    *cir = policy.rate;
    *cbs = policy.cbs;
    *ebs = policy.ebs;

    return CS_E_OK;

}

cs_status ctc_oam_eth_port_policing_set_adapt(
        cs_port_id_t port,
        cs_uint8   oper,
        cs_uint32  cir,
        cs_uint32  cbs,
        cs_uint32  ebs)
{
    cs_status ret;
    cs_callback_context_t context;
    cs_sdl_policy_t       policy;
#ifdef HAVE_ZTE_OAM    
    db_zte_port_t   port_info;
#endif

    policy.rate = cir;
    policy.cbs = cbs;
    policy.ebs = ebs;
    if (oper == CTC_PORT_POLICY_OPER_DISABLE) {
        policy.enable = 0;
    } else {
        policy.enable = 1;
    }

    ret = epon_request_onu_port_policy_set(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, &policy);
#ifdef HAVE_ZTE_OAM 
    memset(&port_info,0x00,sizeof(port_info));
    db_read(DB_ZTE_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
    port_info.port[port - 1 ].policy_rate = cir;
    port_info.port[port - 1 ].policy_cbs = cbs;
    port_info.port[port - 1 ].policy_ebs = cbs;
    port_info.port[port - 1 ].policy_enable = policy.enable;
    db_write(DB_ZTE_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
#endif

    return ret;

}


cs_status ctc_oam_eth_ds_rate_limit_get_adapt(
        cs_port_id_t port,
        cs_uint8 * oper,
        cs_uint32 * cir,
        cs_uint32 * pir)
{
    cs_status ret;
    cs_callback_context_t context;
    cs_sdl_policy_t       rate;

    ret = epon_request_onu_port_ds_rate_limit_get(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, &rate);
    if (ret) {
        return ret;
    }

    *oper = rate.enable;
    *cir = rate.rate;
    *pir = rate.cbs;

    return CS_E_OK;

}

cs_status ctc_oam_eth_ds_rate_limit_set_adapt(
        cs_port_id_t port,
        cs_uint8   oper,
        cs_uint32  cir,
        cs_uint32  pir)
{
    cs_status ret;
    cs_callback_context_t context;
    cs_sdl_policy_t       rate;

    rate.rate = cir;
    rate.cbs = pir;
    rate.ebs = 0;
    if (oper == CTC_PORT_POLICY_OPER_DISABLE) {
        rate.enable = 0;
    } else {
        rate.enable = 1;
    }

    ret = epon_request_onu_port_ds_rate_limit_set(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, &rate);

    return ret;


}

cs_status ctc_oam_onu_mac_limit_set_adapt(
    cs_port_id_t port,
    cs_uint16  num)
{
    cs_callback_context_t context;
    cs_status rt = CS_E_OK;
    

    rt = epon_request_onu_fdb_mac_limit_set(context, 0, 0, port, num);
    if(rt){
        return rt;
    }



    return rt;
}

cs_status ctc_oam_onu_mac_limit_get_adapt(
    cs_port_id_t port,
    cs_uint16*  num)
{

    cs_callback_context_t context;
    cs_uint32 number;
    cs_status rt = CS_E_OK;

    rt = epon_request_onu_fdb_mac_limit_get(context, 0, 0, port, &number);

    *num = number & 0xffff;

    return rt;

}


cs_status ctc_oam_onu_port_mode_set_adapt(
    cs_port_id_t port,
    cs_uint8  mode)
{
    cs_status ret;
    cs_sdl_port_speed_cfg_t cfg = 0;
    cs_callback_context_t context;

    switch (mode) {
        case CTC_OAM_PORT_MODE_10M_HALF:
            cfg = SDL_PORT_10_HALF;
            break;
        case CTC_OAM_PORT_MODE_10M_FULL:
            cfg = SDL_PORT_10_FULL;
            break;
        case CTC_OAM_PORT_MODE_100M_HALF:
            cfg = SDL_PORT_100_HALF;
            break;
        case CTC_OAM_PORT_MODE_100M_FULL:
            cfg = SDL_PORT_100_FULL;
            break;
        case CTC_OAM_PORT_MODE_1000M_FULL:
            cfg = SDL_PORT_1000_FULL;
            break;
        default:
            return CS_E_PARAM;
    }            

    ret = epon_request_onu_port_status_set(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, cfg);

    return ret;

}


cs_status ctc_oam_onu_port_stats_get_adapt(
        cs_port_id_t port,
        oam_ctc_eth_port_stats_t*stats)
{
    oam_port_uni_stats_t uni_stats;
    if(!stats)
        return CS_E_PARAM;

    memset(&uni_stats, 0, sizeof(uni_stats));
    if(CS_E_OK != app_onu_port_stats_get(port, &uni_stats))
    {
        return CS_E_ERROR;
    }

    stats->in_octets = uni_stats.rxbyte_cnt; 
    stats->in_ucast_pkt = uni_stats.rxucfrm_cnt;
    stats->in_nucast_pkt = uni_stats.rxmcfrm_cnt + uni_stats.rxbcfrm_cnt;
    stats->in_discard = 0;
    stats->in_error = uni_stats.rxcrcerrfrm_cnt + uni_stats.rxundersizefrm_cnt + uni_stats.rxoversizefrm_cnt ;
    stats->out_octets = uni_stats.txbyte_cnt;
    stats->out_ucast_pkt = uni_stats.txucfrm_cnt;
    stats->out_nucast_pkt = uni_stats.txmcfrm_cnt + uni_stats.txbcfrm_cnt;
    stats->out_discard = 0;
    stats->out_error = uni_stats.txcrcerrfrm_cnt + uni_stats.txoversizefrm_cnt;

    return CS_E_OK;
}


cs_status ctc_oam_pon_port_stat_get_adapt(
        oam_ctc_onu_llid_stats1 *stats1,
        oam_ctc_onu_llid_stats2 *stats2)
{
    oam_pon_stats_t pon_stats;
    
    if(!stats1 && !stats2)
    {
        return CS_E_PARAM;
    }

    memset(&pon_stats, 0, sizeof(pon_stats));

    app_onu_pon_stats_get(&pon_stats);

    if(stats1)
    {
        stats1->out_pkts = pon_stats.pon_txframe_cnt + pon_stats.pon_txmcframe_cnt + pon_stats.pon_txbcframe_cnt;
        stats1->out_octets = pon_stats.pon_tx_byte_cnt;
        stats1->out_mc_pkts = pon_stats.pon_txmcframe_cnt;
        stats1->out_bc_pkts = pon_stats.pon_txbcframe_cnt;
        stats1->in_pkts = pon_stats.pon_frame_cnt + pon_stats.pon_mcframe_cnt + pon_stats.pon_bcframe_cnt;
        stats1->in_octets = pon_stats.pon_byte_cnt;
        stats1->in_mc_pkts = pon_stats.pon_mcframe_cnt;
        stats1->in_bc_pkts = pon_stats.pon_bcframe_cnt;
    }

    if(stats2)
    {
        cs_callback_context_t     context;
        cs_sdl_fec_cnt_t   fec_cnt;
        epon_request_onu_fec_cnt_get(context, ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, &fec_cnt);
        stats2->in_crc8_err_pkts = pon_stats.pon_crc8_err;
        stats2->in_fec_crct_blks = fec_cnt.rx_correct_blks;
        stats2->in_fec_uncrct_blks = fec_cnt.rx_uncorrect_blks;
        
        stats2->out_mpcp_reg_pkts = 0;
        stats2->out_mpcp_rreq_pkts = pon_stats.pon_mpcp_tx_req_reg;
        stats2->out_mpcp_rpt_pkts = pon_stats.pon_mpcp_tx_rpt;
        stats2->in_mpcp_gate_pkts = pon_stats.pon_mpcp_uc_gat_norm+pon_stats.pon_mpcp_uc_gat_frpt+pon_stats.pon_mpcp_bc_gat_bdis;
        stats2->in_mpcp_reg_pkts = pon_stats.pon_mpcp_bc_reg_ack;
        
        stats2->out_mpcp_pkts = stats2->out_mpcp_rreq_pkts+stats2->out_mpcp_rpt_pkts+pon_stats.pon_mpcp_tx_ack_ack;
        stats2->in_mpcp_pkts = stats2->in_mpcp_gate_pkts+stats2->in_mpcp_reg_pkts;
    }

    return CS_E_OK;
}
cs_status ctc_oam_onu_port_mode_get_adapt(
    cs_port_id_t port,
    cs_uint8*  mode)
{

    cs_status ret;
    cs_sdl_port_speed_cfg_t cfg = 0;
    cs_callback_context_t context;

    ret = epon_request_onu_port_status_get(context, 
            ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, port, &cfg);
    if (ret) {
        return ret;
    }

    switch (cfg) {
        case SDL_PORT_AUTO_10_100_1000:
        case SDL_PORT_AUTO_10_100:
            *mode = CTC_OAM_PORT_MODE_AUTO;
            break;
        case SDL_PORT_10_HALF:
            *mode = CTC_OAM_PORT_MODE_10M_HALF;
            break;
        case SDL_PORT_10_FULL:
            *mode = CTC_OAM_PORT_MODE_10M_FULL;
            break;
        case SDL_PORT_100_HALF:
            *mode = CTC_OAM_PORT_MODE_100M_HALF;
            break;
        case SDL_PORT_100_FULL:
            *mode = CTC_OAM_PORT_MODE_100M_FULL;
            break;
        case SDL_PORT_1000_FULL:
            *mode = CTC_OAM_PORT_MODE_1000M_FULL;
            break;
        default:
            break;
    }            

    return ret;
}


cs_status ctc_oam_onu_port_work_mode_get_adapt(
    cs_port_id_t portId,
    cs_uint8*  mode)
{

    cs_sdl_port_link_status_t link_statu;
    cs_sdl_port_ether_speed_t port_speed;
    cs_sdl_port_ether_duplex_t port_duplex;
    cs_status rc = CS_E_OK;
    cs_callback_context_t context;

    rc = epon_request_onu_port_link_status_get(context, 
        0, 0, portId, &link_statu); 
    if(rc){
        goto end;
    }

    if (link_statu == SDL_PORT_LINK_STATUS_DOWN) {
        *mode = CTC_OAM_PORT_MODE_10M_HALF;
    } else {
        rc = epon_request_onu_port_speed_get(context, 
            ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, portId, &port_speed); 
        if(rc){
            goto end;
        }
        rc = epon_request_onu_port_duplex_get(context, 
            ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, portId, &port_duplex); 
        if(rc){
            goto end;
        }

        switch (port_speed) {
            case SDL_PORT_SPEED_10:
                if (port_duplex == SDL_PORT_DUPLEX_FULL) {
                    *mode = CTC_OAM_PORT_MODE_10M_FULL;
                } else {
                    *mode = CTC_OAM_PORT_MODE_10M_HALF;
                }
                break;
            case SDL_PORT_SPEED_100:
                if (port_duplex == SDL_PORT_DUPLEX_FULL) {
                    *mode = CTC_OAM_PORT_MODE_100M_FULL;
                } else {
                    *mode = CTC_OAM_PORT_MODE_100M_HALF;
                }
                break;
            case SDL_PORT_SPEED_1000:
                *mode = CTC_OAM_PORT_MODE_1000M_FULL;
                break;
        }
                
    }

end:
    return rc;
}

cs_status ctc_oam_eth_mac_aging_time_get_adapt(
        cs_uint32 *time) 
{
    
    cs_callback_context_t context;
    cs_status ret = CS_E_OK;

    ret = epon_request_onu_fdb_age_get(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, time);
    if (ret) {
        return ret;
    }

    return CS_E_OK;

}

cs_status ctc_oam_eth_mac_aging_time_set_adapt(
        cs_uint32 time) 
{
    cs_callback_context_t context;
    cs_status ret = CS_E_OK;

    ret = epon_request_onu_fdb_age_set(context, 
        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API,  time);
    if (ret) {
        return ret;
    }

    return CS_E_OK;

}



cs_status ctc_oam_onu_fast_leave_ability_get_adapt(
        cs_uint32 * num,
        cs_uint32 * mode)
{
    *num = 6;
    mode[0] = 0;    /* non_fast_leave at snoop mode */
    mode[1] = 0x01; /* fast_leave at snoop mode */
    mode[2] = 0x10; /* non_fast_leave at ctrl mode */
    mode[3] = 0x11; /* fast_leave at ctrl mode */
    mode[4] = 0x2;  /* non_fast_leave at mld snoop mode */
    mode[5] = 0x3; /* fast_leave at mld snoop mode */
    return CS_E_OK;
}

cs_status ctc_oam_onu_fast_leave_state_get_adapt(
        cs_uint32 * state)
{
#ifdef HAVE_MC_CTRL
    *state = mc_get_fast_leave_state();
#endif
    return CS_E_OK;
}

cs_status ctc_oam_fast_leave_ctrl_set_adapt(
        cs_uint32 fastLeaveAdmin)
{
#ifdef HAVE_MC_CTRL
    mc_set_fast_leave_state(fastLeaveAdmin);
#endif
    return CS_E_OK;
}



cs_status ctc_oam_onucap_get_adapt(
        cs_uint8 * numUSQueues,
        cs_uint8 * maxQueueUSPort,
        cs_uint8 * numDSQueues,
        cs_uint8 * maxQueueDSPort)
{
    *numUSQueues = 8;
    *maxQueueUSPort = 8;
    *numDSQueues = 4;
    *maxQueueDSPort = 4;
    return CS_E_OK;
}

cs_status ctc_oam_onu_llid_queue_config_get_adapt (
        cs_llid_t llid,
        cs_uint8 * num,
        oam_ctc_onu_llid_config_t * config)
{
    *num = 3; 
    config[0].qid = 0x100;
    config[0].wrr = 0x200;
    config[1].qid = 0x100;
    config[1].wrr = 0x200;
    config[2].qid = 0x100;
    config[2].wrr = 0x200;
    return CS_E_NOT_SUPPORT;
}

cs_status ctc_oam_onu_llid_queue_config_set_adapt (
        cs_llid_t llid,
        cs_uint8  num,
        oam_ctc_onu_llid_config_t * config)
{

    return CS_E_NOT_SUPPORT;
}



/********************* POWER SAVING ADAPT START *********************/


cs_status ctc_oam_onu_sleep_control_set_adapt(
        cs_uint32 sleep_duration,
        cs_uint32 wait_duration,
        cs_uint8 sleep_flag,
        cs_uint8 sleep_mode)
{
    return CS_E_NOT_SUPPORT;
}


cs_status ctc_oam_onu_power_saving_cap_get_adapt(
        cs_uint8 * sleep_mode,
        cs_uint8 * early_wakeup)
{
/*
 * #define  CTC_OAM_SLEEP_MODE_NOT_SUPP      0 
#define  CTC_OAM_SLEEP_MODE_TX_ONLY       1 
#define  CTC_OAM_SLEEP_MODE_RX_ONLY       2 
#define  CTC_OAM_SLEEP_MODE_TX_RX         3


#define CTC_OAM_EARLY_WAKEUP_SUPP       0
#define CTC_OAM_EARLY_WAKEUP_NOT_SUPP   1
*/
    *sleep_mode = CTC_OAM_SLEEP_MODE_NOT_SUPP;
    *early_wakeup = CTC_OAM_EARLY_WAKEUP_NOT_SUPP;
    return CS_E_NOT_SUPPORT;
}


cs_status ctc_oam_onu_power_saving_config_get_adapt(
        cs_uint8 * early_wakeup,
        cs_uint32 *max_sleep_duration)
{
    /*
     * #define CTC_OAM_EARLY_WAKEUP_ENABLE   0 
#define CTC_OAM_EARLY_WAKEUP_DISABLE  1 */
    *early_wakeup = CTC_OAM_EARLY_WAKEUP_DISABLE;
    *max_sleep_duration = 0x10232;
    return CS_E_NOT_SUPPORT;

}

cs_status ctc_oam_onu_power_saving_config_set_adapt(
        cs_uint8 early_wakeup,
        cs_uint32 max_sleep_duration)
{
    return CS_E_NOT_SUPPORT;
}


/****************** PON protect ADAPT **************************/ 

cs_status ctc_oam_onu_pon_protect_parm_get_adapt(
        cs_uint16 * opt_time,
        cs_uint16 * mpcp_time)
{
    *opt_time = 0x1234;
    *mpcp_time = 0x5678;
    return CS_E_NOT_SUPPORT;
}

cs_status ctc_oam_onu_pon_protect_parm_set_adapt(
        cs_uint16 opt_time,
        cs_uint16 mpcp_time)
{

    return CS_E_NOT_SUPPORT;
}

static cs_uint32 g_ctc_oam_cfg_counter = 0 ;

cs_status ctc_oam_onu_cfg_counter_get_adapt(cs_uint32 *counter)
{
#ifdef HAVE_ZTE_OAM
    db_zte_onu_svc_t   svc;
    
    db_read(DB_ZTE_ONU_SVC_INFO_ID,(cs_uint8 *) &svc, sizeof(svc));
    g_ctc_oam_cfg_counter = svc.cfg_counter;
#endif
    *counter = g_ctc_oam_cfg_counter;
    return CS_E_OK;
}
cs_status ctc_oam_onu_cfg_counter_set_adapt(cs_uint32 counter)
{
#ifdef HAVE_ZTE_OAM
    db_zte_onu_svc_t   svc;
    
    db_read(DB_ZTE_ONU_SVC_INFO_ID,(cs_uint8 *) &svc, sizeof(svc));
    svc.cfg_counter = counter; 
    db_write(DB_ZTE_ONU_SVC_INFO_ID,(cs_uint8 *) &svc, sizeof(svc));
    db_save_to_flash();
#endif
    g_ctc_oam_cfg_counter = counter;

    return CS_E_OK;

}

cs_status ctc_oam_onu_cfg_mgmt_set_adapt(cs_uint8 action)
{
#ifdef HAVE_ZTE_OAM
    switch(action){
        case 1:
            db_save_to_flash();
            break;
        case 2:
            db_zte_set_default();
            db_save_to_flash();
            db_restore();
        case 3:
            db_zte_set_default();
            db_save_to_flash();
            break;
        default:
            return CS_E_ERROR;
    }
#endif
    return CS_E_OK;
}



static cs_uint8 g_holdover_state = OAM_CTC_ONU_HOLDOVER_DEACTIVED;
static cs_uint32 g_holdover_time = 0;

cs_status ctc_oam_holdover_get_adapt(
        cs_uint32 *holdover_state,
        cs_uint32 *holdover_time)
{
    cs_status ret = CS_E_OK;
    cs_uint8 gate_timeout = 0;

    
    if(holdover_time == NULL 
            || holdover_state == NULL){
        return CS_E_PARAM;
    }

    /* not initialization, get it from start config */
    if(g_holdover_time == 0){
         startup_config_read(CFG_ID_MPCP_TIMEOUT, 1, &gate_timeout);
         g_holdover_time = gate_timeout*10;
    }

    *holdover_state = g_holdover_state;
    *holdover_time = g_holdover_time;

    return ret;
}

cs_status ctc_oam_holdover_set_adapt(
        cs_uint32 holdover_state,
        cs_uint32 holdover_time)
{
    cs_callback_context_t context;
    cs_uint8 gate_timeout = 0;
    cs_status ret = CS_E_OK;

    if(holdover_state == OAM_CTC_ONU_HOLDOVER_DEACTIVED ){
        startup_config_read(CFG_ID_MPCP_TIMEOUT, 1, &gate_timeout);
        ret = epon_request_onu_mpcp_timer_set(context, 0, 0,1, gate_timeout*10);
    }else if(holdover_state ==OAM_CTC_ONU_HOLDOVER_ACTIVED  
            && holdover_time >= OAM_CTC_ONU_HOLDOVER_TIME_MIN
            && holdover_time <= OAM_CTC_ONU_HOLDOVER_TIME_MAX ){
        ret = epon_request_onu_mpcp_timer_set(context, 0, 0, 0,holdover_time);
    }else{
        return CS_E_PARAM;
    }

    g_holdover_state = holdover_state;
    g_holdover_time = holdover_time;
    return ret;
}




/************************VLAN ADAPT START ***************************/

#define CTC_DEFAULT_TPID 0x8100
#define CTC_MCVLAN_OPER_DEL 0
#define CTC_MCVLAN_OPER_ADD 1
#define CTC_MCVLAN_OPER_CLR 2
#define CTC_MCVLAN_OPER_GET 3
#define CTC_MCVLAN_TAG_OPER_SWAP 2


cs_status ctc_oam_onu_default_vlan_tag_get_adapt(
        cs_port_id_t port,
        cs_uint16 * tpid,
        cs_uint16 * tag)
{
    cs_status rt;
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode;
    cs_sdl_vlan_cfg_t    vlan_cfg[CTC_ONU_AGGR_VLAN_NUM];
    cs_uint16 nums;

    rt = epon_request_onu_vlan_get(context, 0, 0, port, &def_vlan, &vlan_mode, vlan_cfg, &nums);

    if(rt!=CS_E_OK)
        return rt;    

    *tpid = CTC_DEFAULT_TPID;
    *tag = def_vlan.vid | def_vlan.pri<<13;
    return CS_E_OK;
}


cs_status ctc_oam_onu_vlan_mode_get_adapt(
        cs_port_id_t port,
        cs_uint8 * mode)
{
    cs_status rt;
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode;
    cs_sdl_vlan_cfg_t    vlan_cfg[CTC_ONU_AGGR_VLAN_NUM];
    cs_uint16 nums;
    
    rt = epon_request_onu_vlan_get(context, 0, 0, port, &def_vlan, &vlan_mode, vlan_cfg, &nums);
    
    if(rt!=CS_E_OK)
        return rt;    

    *mode = (cs_uint8)vlan_mode;
    return CS_E_OK;
}

cs_status ctc_oam_onu_translate_vlan_get_adapt(
        cs_port_id_t port,
        cs_uint16 * translate_num,
        oam_ctc_onu_vlan_xlate_entry * vlan)
{
    cs_status rt;
    int i = 0;
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode;
    cs_sdl_vlan_cfg_t    vlan_cfg[CTC_ONU_AGGR_VLAN_NUM];
    cs_uint16 nums;
    
    rt = epon_request_onu_vlan_get(context, 0, 0, port, &def_vlan, &vlan_mode, vlan_cfg, &nums);
    
    if(rt!=CS_E_OK)
        return rt;    

    while(i<nums)
    {
        vlan[i].tag2.tpid = CTC_DEFAULT_TPID; 
        vlan[i].tag1.tpid = CTC_DEFAULT_TPID; 
        vlan[i].tag2.tag = vlan_cfg[i].s_vlan.vid;
        vlan[i].tag1.tag = vlan_cfg[i].c_vlan.vid;
        ++i;
    }

    *translate_num = nums;

    return CS_E_OK;
}

cs_status ctc_oam_onu_trunk_vlan_get_adapt(
        cs_port_id_t port,
        cs_uint16 * trunk_num,
        oam_ctc_onu_vlan_tag_t * vlan)
{
    cs_status rt;
    int i = 0;
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode;
    cs_sdl_vlan_cfg_t    vlan_cfg[CTC_ONU_AGGR_VLAN_NUM];
    cs_uint16 nums;
    
    rt = epon_request_onu_vlan_get(context, 0, 0, port, &def_vlan, &vlan_mode, vlan_cfg, &nums);
    
    if(rt!=CS_E_OK)
        return rt;    

    while(i<nums)
    {
        vlan[i].tpid = CTC_DEFAULT_TPID; 
        vlan[i].tag = vlan_cfg[i].c_vlan.vid;
        ++i;
    }

    *trunk_num = nums;

    return CS_E_OK;
}

cs_status ctc_oam_onu_agg_vlan_get_adapt(
        cs_port_id_t port,
        cs_uint16 * agg_num,
        ctc_oam_agg_vlan_t * vlan)
{
    cs_status rt;
    int i = 0;
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode;
    cs_sdl_vlan_cfg_t    vlan_cfg[CTC_ONU_AGGR_VLAN_NUM];
    cs_uint16 nums;
    
    rt = epon_request_onu_vlan_get(context, 0, 0, port, &def_vlan, &vlan_mode, vlan_cfg, &nums);
    
    if(rt!=CS_E_OK)
        return rt;    

    while(i<nums)
    {
        vlan[0].agg_vlan[i].tpid = CTC_DEFAULT_TPID; 
        vlan[0].agg_vlan[i].tag = vlan_cfg[i].c_vlan.vid;
        ++i;
    }

    vlan[0].agg_dst_vlan.tpid = CTC_DEFAULT_TPID;
    vlan[0].agg_dst_vlan.tag = vlan_cfg[0].s_vlan.vid;
    vlan[0].agg_vlan_num = nums;

    *agg_num = 1;

    return CS_E_OK;
}

cs_status ctc_oam_onu_transparent_vlan_set_adapt(
        cs_port_id_t port)
{
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode = SDL_VLAN_MODE_TRANSPARENT;
    cs_status ret = CS_E_OK;
#ifdef HAVE_ZTE_OAM
    db_zte_vlan_t vlan;
#endif

    def_vlan.vid = 0;
    def_vlan.pri = 0;

    ret = epon_request_onu_vlan_set(context,0,0, port, def_vlan, vlan_mode, NULL, 0);

#ifdef HAVE_ZTE_OAM
    if(ret == CS_E_OK
            && port <=DB_ZTE_MAX_PORT 
            && port > 0){
        memset(&vlan,0x00,sizeof(vlan));
        db_read(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan, sizeof(vlan));
        vlan.vlan[port - 1].vlan_mode = vlan_mode;
        db_write(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan, sizeof(vlan));
    }
#endif
    return ret;
}

cs_status ctc_oam_onu_tag_vlan_set_adapt(
        cs_port_id_t port,
        oam_ctc_onu_vlan_tag_t vlan)
{
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode = SDL_VLAN_MODE_TAG;
    cs_status ret = CS_E_OK;
#ifdef HAVE_ZTE_OAM
    db_zte_vlan_t vlan_cfg;
#endif
    def_vlan.vid = TAG2VID(vlan.tag);
    def_vlan.pri = TAG2PRI(vlan.tag);

    ret = epon_request_onu_vlan_set(context,0,0, port, def_vlan, vlan_mode, NULL, 0);

#ifdef HAVE_ZTE_OAM
    if(ret == CS_E_OK
            && port <=DB_ZTE_MAX_PORT 
            && port > 0){
        memset(&vlan_cfg,0x00,sizeof(vlan_cfg));
        db_read(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan_cfg, sizeof(vlan_cfg));
        vlan_cfg.vlan[port - 1].vlan_mode = vlan_mode;
        vlan_cfg.vlan[port - 1].def_vlan.tpid = vlan.tpid;
        vlan_cfg.vlan[port - 1].def_vlan.tag = vlan.tag;
        db_write(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan_cfg, sizeof(vlan_cfg));
    }
#endif
    return ret ;
}

cs_status ctc_oam_onu_translate_vlan_set_adapt(
        cs_port_id_t port,
        oam_ctc_onu_vlan_tag_t defvlan,
        cs_uint16 num,
        oam_ctc_onu_vlan_xlate_entry * vlan)
{
    int i;
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode = SDL_VLAN_MODE_TRANSLATION;
    cs_sdl_vlan_cfg_t   sdl_vlan[CTC_ONU_AGGR_VLAN_NUM];
    cs_status ret = CS_E_OK;
#ifdef HAVE_ZTE_OAM
    db_zte_vlan_t vlan_cfg;
#endif

    def_vlan.vid = TAG2VID(defvlan.tag);
    def_vlan.pri = TAG2PRI(defvlan.tag);

    for(i = 0; i<num; ++i)
    {
        sdl_vlan[i].s_vlan.vid = vlan[i].tag2.tag;
        sdl_vlan[i].c_vlan.vid = vlan[i].tag1.tag;
    }

    ret =  epon_request_onu_vlan_set(context,0,0, port, def_vlan, vlan_mode, sdl_vlan, num);
#ifdef HAVE_ZTE_OAM
    if(ret == CS_E_OK
            && port <=DB_ZTE_MAX_PORT 
            && port > 0){
        memset(&vlan_cfg,0x00,sizeof(vlan_cfg));
        db_read(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan_cfg, sizeof(vlan_cfg));
        vlan_cfg.vlan[port - 1].vlan_mode = vlan_mode;
        vlan_cfg.vlan[port - 1].def_vlan.tpid = defvlan.tpid;
        vlan_cfg.vlan[port - 1].def_vlan.tag = defvlan.tag;
        memset(vlan_cfg.vlan[port -1 ].vlan,0x00,sizeof(vlan_cfg.vlan[port -1 ].vlan));
        vlan_cfg.vlan[port -1 ].vlan_num = num * 2;
        for(i = 0 ; i < num; i ++){
            vlan_cfg.vlan[port -1 ].vlan[i*2].tpid = vlan[i].tag1.tpid;
            vlan_cfg.vlan[port -1 ].vlan[i*2].tag = vlan[i].tag1.tag;
            vlan_cfg.vlan[port -1 ].vlan[i*2 +1].tpid = vlan[i].tag2.tpid;
            vlan_cfg.vlan[port -1 ].vlan[i*2 +1].tag = vlan[i].tag2.tag;
        }

        db_write(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan_cfg, sizeof(vlan_cfg));
    }
#endif
    return ret;
}


cs_status ctc_oam_onu_trunk_vlan_set_adapt(
        cs_port_id_t port,
        oam_ctc_onu_vlan_tag_t defvlan,
        cs_uint16 num,
        oam_ctc_onu_vlan_tag_t * vlan)
{
    int i;
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode = SDL_VLAN_MODE_TRUNK;
    cs_sdl_vlan_cfg_t   sdl_vlan[CTC_ONU_AGGR_VLAN_NUM];
    cs_status ret = CS_E_OK;
#ifdef HAVE_ZTE_OAM
    db_zte_vlan_t vlan_cfg;
#endif
    def_vlan.vid = TAG2VID(defvlan.tag);
    def_vlan.pri = TAG2PRI(defvlan.tag);
    for(i = 0; i<num; ++i)
    {
        sdl_vlan[i].s_vlan.vid = vlan[i].tag;
        sdl_vlan[i].c_vlan.vid = vlan[i].tag;
    }
    ret =  epon_request_onu_vlan_set(context,0,0, port, def_vlan, vlan_mode, sdl_vlan, num);
#ifdef HAVE_ZTE_OAM
    if(ret == CS_E_OK
            && port <=DB_ZTE_MAX_PORT 
            && port > 0){
        memset(&vlan_cfg,0x00,sizeof(vlan_cfg));
        db_read(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan_cfg, sizeof(vlan_cfg));
        vlan_cfg.vlan[port - 1].vlan_mode = vlan_mode;
        vlan_cfg.vlan[port - 1].def_vlan.tpid = defvlan.tpid;
        vlan_cfg.vlan[port - 1].def_vlan.tag = defvlan.tag;
        memset(vlan_cfg.vlan[port -1 ].vlan,0x00,sizeof(vlan_cfg.vlan[port -1 ].vlan));
        vlan_cfg.vlan[port -1 ].vlan_num = num;
        for(i = 0 ; i < num; i ++){
            vlan_cfg.vlan[port -1 ].vlan[i].tpid = vlan[i].tpid;
            vlan_cfg.vlan[port -1 ].vlan[i].tag = vlan[i].tag;
        }

        db_write(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan_cfg, sizeof(vlan_cfg));
    }
#endif
    return ret;
}

cs_status ctc_oam_onu_agg_vlan_set_adapt(
        cs_port_id_t port,
        oam_ctc_onu_vlan_tag_t defvlan,
        cs_uint16 num,
        ctc_oam_agg_vlan_t * vlan)
{
    int i;
    cs_callback_context_t context;
    cs_sdl_vlan_tag_t    def_vlan;
    cs_sdl_vlan_mode_t  vlan_mode = SDL_VLAN_MODE_AGGREGATION;
    cs_sdl_vlan_cfg_t   sdl_vlan[CTC_ONU_AGGR_VLAN_NUM];
    cs_status ret = CS_E_OK;
#ifdef HAVE_ZTE_OAM
    db_zte_vlan_t vlan_cfg;
#endif

    if(num > 1)
    {
        return CS_E_NOT_SUPPORT;
    }

    def_vlan.vid = TAG2VID(defvlan.tag);
    def_vlan.pri = TAG2PRI(defvlan.tag);

    for(i = 0; i<vlan[0].agg_vlan_num; ++i)
    {
        sdl_vlan[i].c_vlan.vid = vlan[0].agg_vlan[i].tag;
        sdl_vlan[i].s_vlan.vid = vlan[0].agg_dst_vlan.tag;
    }

    ret =  epon_request_onu_vlan_set(context,0,0, port, def_vlan, vlan_mode, sdl_vlan, vlan[0].agg_vlan_num);

#ifdef HAVE_ZTE_OAM
    if(ret == CS_E_OK
            && port <=DB_ZTE_MAX_PORT 
            && port > 0){
        memset(&vlan_cfg,0x00,sizeof(vlan_cfg));
        db_read(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan_cfg, sizeof(vlan_cfg));
        vlan_cfg.vlan[port - 1].vlan_mode = vlan_mode;
        vlan_cfg.vlan[port - 1].def_vlan.tpid = defvlan.tpid;
        vlan_cfg.vlan[port - 1].def_vlan.tag = defvlan.tag;
        memset(vlan_cfg.vlan[port -1 ].vlan,0x00,sizeof(vlan_cfg.vlan[port -1 ].vlan));
        vlan_cfg.vlan[port -1 ].vlan_num = 1;
        vlan_cfg.vlan[port -1 ].agg_vlan_num = vlan[0].agg_vlan_num;
        
        vlan_cfg.vlan[port -1 ].vlan[0].tpid = vlan[0].agg_dst_vlan.tpid;
        vlan_cfg.vlan[port -1 ].vlan[0].tag = vlan[0].agg_dst_vlan.tag;
        for(i = 0 ; i < vlan[0].agg_vlan_num && i < CTC_ONU_AGGR_VLAN_NUM; i ++){
            vlan_cfg.vlan[port -1 ].vlan[i+1].tpid = vlan[0].agg_vlan[i].tpid;
            vlan_cfg.vlan[port -1 ].vlan[i+1].tag = vlan[0].agg_vlan[i].tag;
        }
        db_write(DB_ZTE_VLAN_ID,(cs_uint8 *) &vlan_cfg, sizeof(vlan_cfg));
    }
#endif

    return ret;
}








/**************** PLAT ADAPT START *********************************/

cs_status ctc_oam_onu_opmdiag_get_adapt(
        cs_int16 * temp,
        cs_uint16 * volt,
        cs_uint16 * tx_bias,
        cs_uint16 * tx_power,
        cs_uint16 * rx_power)
{
    cs_status ret = CS_E_OK;
    cs_callback_context_t   context;

    ret = cs_plat_i2c_opm_status_read(context,0,0,temp,
            volt, tx_bias, tx_power,rx_power);
    
    return ret;
}


cs_status ctc_oam_onu_mgmt_config_get_adapt(
        ctc_oam_mgmt_parm_t * parm)
{
   if(parm == NULL)
		return CS_E_ERROR;
	else
	{
		cs_uint8 en;
		cs_uint32 rm_subnet, rm_rmmask;
		if((epon_request_onu_ip_config_get(&parm->mgmt_ip.addr.ipv4, &parm->mask, &parm->mgmt_gateway.addr.ipv4) == CS_OK) &&
			(epon_request_onu_inband_ip_config_get(&en, &parm->mgmt_data_pri, &parm->mgmt_cvlan, &rm_subnet, &rm_rmmask) == CS_OK))
		{
			#if 1
			parm->mgmt_ip.addr.ipv4 = ipintf_info.ptyip;
			parm->mask = ipintf_info.ptymask;
			parm->mgmt_gateway.addr.ipv4 = ipintf_info.ptygtw;
			#endif
			return CS_OK;
		}
		else
		{
			cs_printf("get mgmt ip fail!\r\n");
			return CS_ERROR;
		}
	}
    
     return CS_E_OK;
}
#define GWD_PRODUCT_CFG_OFFSET_INBAND  (1024*20)
extern onu_slow_path_cfg_cfg_t   g_slow_path_ip_cfg;
extern cs_status ip_mode_set(int mode);

cs_status ctc_oam_onu_mgmt_config_set_adapt(
        ctc_oam_mgmt_parm_t * parm)
{
	unsigned char *buff=NULL;
    int size=0;
	int ret;
    
	if(parm == NULL)
		return CS_E_ERROR;
	else
	{
		ret = get_userdata_from_flash((unsigned char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_INBAND, sizeof(g_slow_path_ip_cfg));
		if(ret)
			{
				cs_printf("get ipdate fail..\n");
			}
		g_slow_path_ip_cfg.pty_ip=parm->mgmt_ip.addr.ipv4;
        g_slow_path_ip_cfg.pty_mask=parm->mask;
        g_slow_path_ip_cfg.pty_gateway=parm->mgmt_gateway.addr.ipv4;
        g_slow_path_ip_cfg.pty_vlanid=parm->mgmt_cvlan;
		buff=(unsigned char *)&g_slow_path_ip_cfg;
		size=sizeof(g_slow_path_ip_cfg);
		PTY_ENABLE = 1;
        ret = save_userdata_to_flash(buff, GWD_PRODUCT_CFG_OFFSET_INBAND, size);
		if(ret)
		{
           cs_printf( "%% Save fail .");
        }
		#if 1
		ip_mode_set(2);
		#endif
		if((epon_request_onu_ip_config_set(parm->mgmt_ip.addr.ipv4, parm->mask, parm->mgmt_gateway.addr.ipv4) == CS_OK) &&
			(epon_request_onu_inband_ip_config_set(APP_IPINTF_INBNAD_MGNT, parm->mgmt_data_pri, parm->mgmt_cvlan , 0, 0) == CS_OK))
			return CS_OK;
		else
		{
			cs_printf("set mgmt ip fail!\r\n");
			return CS_ERROR;
		}
	}
}

typedef enum
{
    TRANSID_MAJOR   = 0,
    TRANSID_STANDBY = 1,
    TRANSID_BOTH    = 2
} cs_transid;

cs_status ctc_oam_onu_laser_control_set_adapt(
        cs_uint16 action,
        cs_uint8 * onuId,
        cs_transid transId)
{
    cs_mac_t onu_mac;
    cs_callback_context_t     context;

    if(NULL == onuId)
    {
        return CS_E_PARAM;
    }
    else if((transId != TRANSID_MAJOR) && (transId != TRANSID_BOTH))
    {
        return CS_E_NOT_SUPPORT;
    }

    memset(&context, 0, sizeof(context));

    epon_request_onu_pon_mac_addr_get(context,0,0, &onu_mac);

    if(((onuId[0]==onu_mac.addr[0])
    && (onuId[1]==onu_mac.addr[1])
    && (onuId[2]==onu_mac.addr[2])
    && (onuId[3]==onu_mac.addr[3])
    && (onuId[4]==onu_mac.addr[4])
    && (onuId[5]==onu_mac.addr[5])) 
    || 
    ((0xff==onuId[0])&&(0xff==onuId[1])&&(0xff==onuId[2])&&(0xff==onuId[3])&&(0xff==onuId[4])&&(0xff==onuId[5])))
    {
        if(g_laser_tx_power_timer!=0) /*stop previous timer */
        {
            cs_timer_del(g_laser_tx_power_timer);
            g_laser_tx_power_timer = 0;
        }

        if(0==action) /* tx laser on */
        {
            epon_request_onu_pon_trans_tx_pwr_set(context,0,0,1);
        }
        else if(65535==action)
        {
            epon_request_onu_pon_trans_tx_pwr_set(context,0,0,0);
        }
        else
        {
            epon_request_onu_pon_trans_tx_pwr_set(context,0,0,0);
            g_laser_tx_power_timer = cs_timer_add(action*1000, __laser_tx_power_on, NULL);
        }
    }else{
        return CS_E_PARAM;
    }

    return CS_E_OK;
}



cs_status ctc_oam_onu_reset_set_adapt()
{
    iros_system_reset(RESET_TYPE_FORCE);
    return CS_E_OK;
}




/************** APP ADAPT START ****************************/

cs_status ctc_oam_sla_get_adapt(
        cs_uint8  * oper,
        cs_uint8  * scheduling_scheme,
        cs_uint8  * high_pri_boundary,
        cs_uint32 * cycle_len,
        cs_uint8  * num_of_service,
        oam_sla_queue_t * sla)
{
    return CS_E_NOT_SUPPORT;
}

cs_status ctc_oam_sla_set_adapt(
        cs_uint8   oper,
        cs_uint8   scheduling_scheme,
        cs_uint8   high_pri_boundary,
        cs_uint32  cycle_len,
        cs_uint8   num_of_service,
        oam_sla_queue_t * sla)
{
    return CS_E_NOT_SUPPORT;

}

cs_status ctc_oam_loop_detect_status_get_adapt(
        cs_port_id_t port,
        cs_uint32 *status) 
{
#ifdef HAVE_LOOP_DETECT
    cs_boolean enable = TRUE;

    if(CS_E_OK != loop_detect_oper_get(port, &enable))
    {
        return CS_E_ERROR;
    }

    if(enable)
    {
        *status = 2;
    }
    else
    {
        *status = 1;
    }

    return CS_E_OK;
    
#else
    return CS_E_NOT_SUPPORT;
#endif

}



cs_status ctc_oam_loop_detect_status_set_adapt(
        cs_port_id_t port,
        cs_uint32 status) 
{
#ifdef HAVE_LOOP_DETECT
#ifdef HAVE_ZTE_OAM
     db_zte_port_t   port_info;
#endif
    if(status == 1)
    {
        loop_detect_disable(port);
        /*also need unblock the port*/
        loop_detect_unblock(port);
    }
    else if(status == 2)
    {
        loop_detect_enable(port);
    }
    else
    {
        return CS_E_PARAM;
    }
#ifdef HAVE_ZTE_OAM
    if(port <= DB_ZTE_MAX_PORT && port > 0){
        memset(&port_info,0x00,sizeof(port_info));
        db_read(DB_ZTE_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
        port_info.port[port - 1 ].loop_detect = status == 2? 1:0;
        db_write(DB_ZTE_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
    }

#endif

    return CS_E_OK;

#else
    return CS_E_NOT_SUPPORT;
#endif

}

cs_status ctc_oam_disable_looped_status_set_adapt(
        cs_port_id_t port,
        cs_uint32 status) 
{
#ifdef HAVE_LOOP_DETECT
    cs_boolean enable = TRUE;

    if(status)
    {
        /*need block*/
        enable = TRUE;
    }
    else
    {
        /*no need block*/
        enable = FALSE;
    }

    loop_detect_port_block_enable(port, enable);
    
#endif
    return CS_E_OK;

}


cs_status ctc_oam_onu_classification_get_adapt(
        cs_port_id_t port,
        cs_uint32 *num,
        cs_uint8 * data)
{
    oam_ctc_onu_classification_struct * pData = 
        (oam_ctc_onu_classification_struct*) data;
    oam_ctc_onu_class_fselect_t * pEntry;
    oam_ctc_onu_class_fselect_v6_t * pEntry6;

    cs_sdl_classification_t  cls[CLASS_RULES_MAX];
    cs_callback_context_t     context;
    cs_status ret = CS_E_OK;
    cs_uint8 cls_num = 0;
    cs_uint32 i = 0,j = 0, len = 0, total_len = 0;

    memset(cls,0x00,sizeof(cls));

    ret = epon_request_onu_port_classification_get(
            context,0,0,port,&cls_num,cls);
    if(ret != CS_E_OK){
        return ret;
    }
    
    *num = cls_num;

    for(i = 0; i < cls_num; i++){
        pData = (oam_ctc_onu_classification_struct*) (data + total_len);
        len = 3;
        total_len += sizeof(oam_ctc_onu_classification_struct) - 1;
        pData->precedence = cls[i].precedence;
        pData->queueMapped = cls[i].queueMapped;
        pData->priMark = cls[i].priMark;
        pData->entries = cls[i].entries;
        for(j = 0; j < pData->entries; j++){
            if(cls[i].fselect[j].fieldSelect >= CLASS_RULES_FSELECT_IPVER){
                len += sizeof(oam_ctc_onu_class_fselect_v6_t);
                pEntry6 = (oam_ctc_onu_class_fselect_v6_t* ) (data + total_len);
                total_len += sizeof(oam_ctc_onu_class_fselect_v6_t);
                pEntry6->fieldSelect = cls[i].fselect[j].fieldSelect;
                pEntry6->validationOper = cls[i].fselect[j].validationOper;
                memcpy(pEntry6->matchValue,cls[i].fselect[j].matchValue,CLASS_MATCH_VAL_LEN);
            }else{
                len += sizeof(oam_ctc_onu_class_fselect_t);
                pEntry = (oam_ctc_onu_class_fselect_t* ) (data + total_len);
                total_len += sizeof(oam_ctc_onu_class_fselect_t);
                pEntry->fieldSelect = cls[i].fselect[j].fieldSelect;
                pEntry->validationOper = cls[i].fselect[j].validationOper;
                memcpy(pEntry->matchValue,cls[i].fselect[j].matchValue + (CLASS_MATCH_VAL_LEN - CTC_CLASS_MATCH_VAL_LEN),
                        CTC_CLASS_MATCH_VAL_LEN);
            }
        }

        pData->len = len;

    }

    return CS_E_OK;
}

cs_status ctc_oam_onu_classification_set_adapt(
        cs_uint8 action,
        cs_port_id_t port,
        cs_uint32 num,
        cs_uint8 * data)
{
    cs_status ret = CS_E_OK;
    cs_sdl_classification_t  cls[CLASS_RULES_MAX];
    cs_callback_context_t     context;
    cs_uint8 precedence[CLASS_RULES_MAX];

    oam_ctc_onu_classification_struct * pData;
    oam_ctc_onu_class_fselect_t * pEntry;
    oam_ctc_onu_class_fselect_v6_t * pEntry6;
    cs_uint32 i = 0,j = 0, len = 0, total_len = 0;

    memset(cls,0x00,sizeof(cls));

    switch(action){
        case CTC_CLASS_RULES_ACTION_CLR:
            ret = epon_request_onu_port_classification_clr(
                    context,0,0,port);
            break;
        case CTC_CLASS_RULES_ACTION_DEL:
            for(i = 0; i < num; i++){
                pData = (oam_ctc_onu_classification_struct*) (data + total_len);
                total_len += (pData->len + 2);
                precedence[i] = pData->precedence; 
            }
            ret = epon_request_onu_port_classification_del(
                    context,0,0,port,num,precedence);
            break;
        case CTC_CLASS_RULES_ACTION_ADD:
            for(i = 0; i < num && i < CLASS_RULES_MAX; i ++){
                pData = (oam_ctc_onu_classification_struct*)(data + total_len);
                total_len += (pData->len + 2);
                cls[i].precedence = pData->precedence;
                cls[i].queueMapped = pData->queueMapped;
                cls[i].priMark = pData->priMark;
                cls[i].entries = pData->entries;
                len = 0;
                for(j = 0; j < pData->entries && j < CLASS_FIELD_SELECT_MAX; j++){
                    pEntry = (oam_ctc_onu_class_fselect_t *)(pData->data + len);
                    if(pEntry->fieldSelect >= CLASS_RULES_FSELECT_IPVER){
                        pEntry6 = (oam_ctc_onu_class_fselect_v6_t *)(pData->data + len);
                        len += sizeof(oam_ctc_onu_class_fselect_v6_t);
                        cls[i].fselect[j].fieldSelect = pEntry6->fieldSelect;
                        cls[i].fselect[j].validationOper = pEntry6->validationOper;
                        memcpy(cls[i].fselect[j].matchValue,pEntry6->matchValue,CLASS_MATCH_VAL_LEN);
                    }else{
                        pEntry = (oam_ctc_onu_class_fselect_t *)(pData->data + len);
                        len += sizeof(oam_ctc_onu_class_fselect_t);
                        cls[i].fselect[j].fieldSelect = pEntry->fieldSelect;
                        cls[i].fselect[j].validationOper = pEntry->validationOper;
                        memcpy(cls[i].fselect[j].matchValue + (CLASS_MATCH_VAL_LEN - CTC_CLASS_MATCH_VAL_LEN),
                                pEntry->matchValue,CTC_CLASS_MATCH_VAL_LEN);
                    }
                }
            }
            ret = epon_request_onu_port_classification_add(
                    context,0,0,port,num,cls);
            break;
    }
    
    return ret;

}



/******************************* MC ADAPT **************************/

cs_status ctc_oam_onu_mcvlan_get_adapt(
        cs_port_id_t port,
        cs_uint16 *num,
        cs_uint16 * vlanIds)
{
    cs_callback_context_t         context;
    cs_uint8 mc_mode = CTC_MC_SWITCH_START_IMGP_SNOOPING;

    if( (port<CS_UNI_PORT_ID1)||(port>UNI_PORT_MAX) )
    {
        return CS_E_PARAM;
    }

    if( (NULL==num)||(NULL==vlanIds) )
    {
        return CS_E_PARAM;
    }
    
    ctc_oam_onu_mc_switch_get_adapt(&mc_mode);
    if(CTC_MC_SWITCH_START_IMGP_SNOOPING != mc_mode) {
        return CS_E_NOT_SUPPORT;
    }

    return epon_request_onu_mc_vlan_get(context,0,0, port, vlanIds, num);
}

cs_status ctc_oam_onu_mcvlan_set_adapt(
        cs_port_id_t port,
        cs_uint8 oper,
        cs_uint16 num,
        cs_uint16 * vlanIds)
{
    cs_callback_context_t         context;
    cs_uint8 mc_mode = CTC_MC_SWITCH_START_IMGP_SNOOPING;

    if( (port<CS_UNI_PORT_ID1)||(port>UNI_PORT_MAX) )
    {
        return CS_E_PARAM;
    }

    if(oper > CTC_MCVLAN_OPER_CLR)
    {
        return CS_E_PARAM;
    }

    if( NULL==vlanIds )
    {
        return CS_E_PARAM;
    }

    ctc_oam_onu_mc_switch_get_adapt(&mc_mode);
    if(CTC_MC_SWITCH_START_IMGP_SNOOPING != mc_mode) {
        return CS_E_NOT_SUPPORT;
    }

    if(CTC_MCVLAN_OPER_DEL == oper)
    {
        return epon_request_onu_mc_vlan_del(context,0,0, port, vlanIds, num);
    }
    else if(CTC_MCVLAN_OPER_ADD == oper)
    {
        return epon_request_onu_mc_vlan_add(context,0,0, port, vlanIds, num);
    }
    else if(CTC_MCVLAN_OPER_CLR == oper)
    {
        return epon_request_onu_mc_vlan_clr(context,0,0, port);
    }
    else {
        return CS_E_PARAM;
    }
}


cs_status ctc_oam_onu_mctagstrip_get_adapt(
        cs_port_id_t port,
        cs_uint8 * tagStriped,
        cs_uint8 * num,
        oam_ctc_iptv_vid_t * vlans)
{
    cs_callback_context_t         context;
    cs_sdl_mc_vlan_act_t          vlan_act;
    cs_sdl_vlan_cfg_t             swap_rule[CTC_ONU_AGGR_VLAN_NUM];
    cs_uint16                     rule_nums;
    int i;

    if( (port<CS_UNI_PORT_ID1)||(port>UNI_PORT_MAX) )
    {
        return CS_E_PARAM;
    }

    if( (NULL==tagStriped) || (NULL==num) || (NULL==vlans) )
    {
        return CS_E_PARAM;
    }

    epon_request_onu_mc_vlan_action_get(context,0,0, port, &vlan_act,swap_rule,&rule_nums);

    if(SDL_MC_VLAN_TAG_SWAP == vlan_act)
    {
        for(i=0; i<rule_nums; ++i)
        {
            vlans[i].vid = swap_rule[i].s_vlan.vid;
            vlans[i].iptv_vid = swap_rule[i].c_vlan.vid;
        }
        *num = rule_nums;
    }

    *tagStriped = (cs_uint8)vlan_act;

    return CS_E_OK;
}

cs_status ctc_oam_onu_mctagstrip_set_adapt(
        cs_port_id_t port,
        cs_uint8  tagStriped,
        cs_uint8  num,
        oam_ctc_iptv_vid_t * vlans)
{
    cs_callback_context_t         context;
    cs_sdl_mc_vlan_act_t          vlan_act;
    cs_sdl_vlan_cfg_t             swap_rule[CTC_ONU_AGGR_VLAN_NUM];
    int i;

    if( (port<CS_UNI_PORT_ID1)||(port>UNI_PORT_MAX) )
    {
        return CS_E_PARAM;
    }

    if( (CTC_MCVLAN_TAG_OPER_SWAP==tagStriped) && (NULL==vlans) )
    {
        return CS_E_PARAM;
    }

    if(CTC_MCVLAN_TAG_OPER_SWAP==tagStriped)
    {
        for(i = 0; i<num; ++i)
        {   
            swap_rule[i].s_vlan.vid = vlans[i].vid;
            swap_rule[i].c_vlan.vid = vlans[i].iptv_vid;
        }
    }

    vlan_act = (cs_sdl_mc_vlan_act_t)tagStriped;

    return epon_request_onu_mc_vlan_action_set(context,0,0, port, vlan_act, swap_rule, num);
}


cs_status ctc_oam_onu_mc_switch_get_adapt(
        cs_uint8 *mode)
{
#ifdef HAVE_MC_CTRL    
    mc_mode_t mc_mode;
    mc_mode_get(0, &mc_mode);

    switch(mc_mode) {
        case MC_SNOOPING:
            *mode = CTC_MC_SWITCH_START_IMGP_SNOOPING;
            break;

        case MC_MANUAL:
            *mode = CTC_MC_SWITCH_START_CTC_IGMP;
            break;

        default:
            *mode = ZTE_MC_SWITCH_TRANSPARENT;
            break;
    }

#endif    
    return CS_E_OK;

}

cs_status ctc_oam_onu_mc_switch_set_adapt(
        cs_uint8 mode)
{
#ifdef HAVE_MC_CTRL
#if 0
	cs_printf("in ctc_oam_onu_mc_switch_set_adapt, mode :0x%x\n", mode);
#endif
    mc_mode_t mc_mode;
    cs_uint8 port_num;
    cs_uint8 port;
    cs_uint8 old_mode;

    ctc_oam_onu_mc_switch_get_adapt(&old_mode);
    if(old_mode == mode) {
        return CS_E_OK;
    }

    if(mode == CTC_MC_SWITCH_START_IMGP_SNOOPING) {
        mc_mode = MC_SNOOPING;
		#if 0
		cs_printf(" mc_mode = MC_SNOOPING;\n");
		#endif
        mc_vlan_lrn_set(0, MC_SVL);
        mc_set_port_ingress_filter(0, mc_snoop_ingress_filter);
        mc_set_port_egress_filter(0, mc_egress_filter);
    }
    else if(mode == CTC_MC_SWITCH_START_CTC_IGMP){
        mc_mode = MC_MANUAL;
		#if 0
		cs_printf(" mc_mode = MC_MANUAL;\n");
		#endif
        mc_ctrl_init();
        mc_set_port_ingress_filter(0, mc_ctrl_ingress_filter);
        mc_set_port_egress_filter(0, mc_egress_filter);
    }
    else {
        mc_mode = MC_DISABLE;
		#if 0
		cs_printf(" mc_mode = MC_DISABLE;\n");
		#endif
    }    

    /* clear mc vlan */
    startup_config_read(CFG_ID_SWITCH_PORT_NUM, 1, &port_num);
    for(port = 1; port < port_num+1; port++) {
        cs_callback_context_t  context;
        epon_request_onu_mc_vlan_clr(context, 0, 0, port);
    }
	
    #if 0
	cs_printf(" mc_mode :0x%x\n", mc_mode);
	#endif
    return mc_mode_set(0, mc_mode);
#else
    return CS_E_NOT_SUPPORT;
#endif
}

cs_status ctc_oam_onu_mc_control_type_get_adapt(
        cs_uint8 * type)
{
#ifdef HAVE_MC_CTRL   
    *type = mc_get_mc_fwd_mode();
#endif
    return CS_E_OK;
}

cs_status ctc_oam_onu_mc_control_type_set_adapt(
        cs_uint8  type)
{
#ifdef HAVE_MC_CTRL   
    return mc_set_mc_fwd_mode(type);
#else
    return CS_E_NOT_SUPPORT;
#endif
}

cs_status ctc_oam_onu_mc_control_vlan_mac_get_adapt(
        cs_uint16 *num,
        oam_ctc_onu_mc_control_vlan_mac_t * vlanMac)
{
#ifdef HAVE_MC_CTRL   
    mc_ctc_ctrl_get_entries((cs_uint8 *)vlanMac, num);
#endif
    return CS_E_OK;

}

cs_status ctc_oam_onu_mc_control_mac_ip_get_adapt(
        cs_uint16 *num,
        oam_ctc_onu_mc_control_mac_ip_t * macIp)
{
#ifdef HAVE_MC_CTRL   
    mc_ctc_ctrl_get_entries((cs_uint8 *)macIp, num);
#endif
    return CS_E_OK;

}

cs_status ctc_oam_onu_mc_control_vlan_ip_get_adapt(
        cs_uint16 *num,
        oam_ctc_onu_mc_control_vlan_ip_t* vlanIp)
{
#ifdef HAVE_MC_CTRL   
    mc_ctc_ctrl_get_entries((cs_uint8 *)vlanIp, num);
#endif
    return CS_E_OK;

}

cs_status ctc_oam_onu_mc_control_clr_adapt()
{
#ifdef HAVE_MC_CTRL    
    mc_ctc_ctrl_clr_mfdb();
#endif
    return CS_E_OK;
}

cs_status ctc_oam_onu_mc_control_vlan_mac_set_adapt(
        cs_uint8 action,
        cs_uint16 num,
        oam_ctc_onu_mc_control_vlan_mac_t * vlanMac)
{
#ifdef HAVE_MC_CTRL    
    cs_uint16 i;
    
    switch(action) {
        case CTC_MC_CONTROL_ACTION_DEL:
            for(i = 0; i < num; i++) {
                mc_ctc_ctrl_del_mac_vlan_entry(vlanMac[i].userId, vlanMac[i].mc_da, vlanMac[i].vlanId);
            }
            break;

        case CTC_MC_CONTROL_ACTION_ADD:
            for(i = 0; i < num; i++) {
                mc_ctc_ctrl_add_mac_vlan_entry(vlanMac[i].userId, vlanMac[i].mc_da, vlanMac[i].vlanId);
            }
            break;

        default:
            break;
    }
#endif
    
    return CS_E_OK;
}

cs_status ctc_oam_onu_mc_control_mac_ip_set_adapt(
        cs_uint8 action,
        cs_uint16 num,
        oam_ctc_onu_mc_control_mac_ip_t * macIp)
{
#ifdef HAVE_MC_CTRL
    cs_uint16 i;

    mc_ip_type_t ip_type;
    
    switch(action) {
        case CTC_MC_CONTROL_ACTION_DEL:
            for(i = 0; i < num; i++) {
                ip_type = (macIp[i].ip_addr[0]) ? MC_IPV6 : MC_IPV4;
                mc_ctc_ctrl_del_da_sip_entry(macIp[i].userId, macIp[i].mc_da, ip_type, macIp[i].ip_addr);
            }
            break;

        case CTC_MC_CONTROL_ACTION_ADD:
            for(i = 0; i < num; i++) {
                ip_type = (macIp[i].ip_addr[0]) ? MC_IPV6 : MC_IPV4;
                mc_ctc_ctrl_add_da_sip_entry(macIp[i].userId, macIp[i].mc_da, ip_type, macIp[i].ip_addr);
            }
            break;

        default:
            break;
    }
#endif

    return CS_E_OK;
}

cs_status ctc_oam_onu_mc_control_vlan_ip_set_adapt(
        cs_uint8 action,
        cs_uint16 num,
        oam_ctc_onu_mc_control_vlan_ip_t* vlanIp)
{
#ifdef HAVE_MC_CTRL
    cs_uint16 i;

    mc_ip_type_t ip_type;
    cs_uint8 *ip_addr;
    
    switch(action) {
        case CTC_MC_CONTROL_ACTION_DEL:
            for(i = 0; i < num; i++) {
                if(vlanIp[i].ip_addr[0]) {
                    ip_type = MC_IPV6;
                    ip_addr = vlanIp[i].ip_addr;
                }
                else {                    
                    ip_type = MC_IPV4;
                    ip_addr = &vlanIp[i].ip_addr[2];
                }
                
                mc_ctc_ctrl_del_ip_vlan_entry(vlanIp[i].userId, vlanIp[i].ip_addr, ip_type, vlanIp[i].vlanId);
            }
            break;

        case CTC_MC_CONTROL_ACTION_ADD:
            for(i = 0; i < num; i++) {
                if(vlanIp[i].ip_addr[0]) {
                    ip_type = MC_IPV6;
                    ip_addr = vlanIp[i].ip_addr;
                }
                else {                    
                    ip_type = MC_IPV4;
                    ip_addr = &vlanIp[i].ip_addr[2];
                }
                
                mc_ctc_ctrl_add_ip_vlan_entry(vlanIp[i].userId, vlanIp[i].ip_addr, ip_type, vlanIp[i].vlanId);
            }
            break;

        default:
            break;
    }
#endif

    return CS_E_OK;
}

cs_status ctc_oam_onu_grp_nmb_max_get_adapt(
        cs_port_id_t port,
        cs_uint8 *  maxGrp)
{
#ifdef HAVE_MC_CTRL
    cs_uint16 max_grp;

    cs_uint8 mc_mode;

    ctc_oam_onu_mc_switch_get_adapt(&mc_mode);
    if(CTC_MC_SWITCH_START_IMGP_SNOOPING != mc_mode) {
        return CS_E_RESOURCE;
    }
    
    if(CS_E_OK == mc_port_max_grp_get(0, port, &max_grp)) {
        *maxGrp = max_grp;
        return CS_E_OK;
    }
#endif

    return CS_E_ERROR;
}

cs_status ctc_oam_onu_grp_nmb_max_set_adapt(
        cs_port_id_t port,
        cs_uint8   maxGrp)
{
#ifdef HAVE_MC_CTRL
    cs_uint8 mc_mode;

    ctc_oam_onu_mc_switch_get_adapt(&mc_mode);
    if(CTC_MC_SWITCH_START_IMGP_SNOOPING != mc_mode) {
        return CS_E_NOT_SUPPORT;
    }
    
    return mc_port_max_grp_set(0, port, maxGrp);
#else
    return CS_E_NOT_SUPPORT;
#endif
}

/********************* Event ****************/
cs_status ctc_oam_onu_event_type_get(
        oam_ctc_event_entry_t entry,
        ctc_oam_alarm_class_e *type,
        cs_port_id_t *port)
{
    if(entry.object_type == OAM_CTC_ALARM_TYPE_ONU)
    {
        *type = CTC_ONU_ALARM;
    }
    else if(entry.object_type == OAM_CTC_ALARM_TYPE_PON)
    {
        *type = CTC_PON_ALARM;
        *port = CS_PON_PORT_ID;
    }
    else if(entry.object_type == OAM_CTC_ALARM_TYPE_UNI)
    {
        *type = CTC_PORT_ALARM;
        *port = (entry.object_inst & 0x000000ff);
    }
    else
    {
        return CS_E_PARAM;
    }

    return CS_E_OK;
}
cs_status ctc_oam_event_status_get_adapt(
        oam_ctc_event_entry_t entry,
        cs_uint32 *status)
{
    ctc_oam_alarm_class_e type = CTC_ONU_ALARM;
    cs_port_id_t port= CS_PON_PORT_ID;
    cs_status ret = CS_E_OK;
    cs_uint32 alarm_config = 0;

    ret = ctc_oam_onu_event_type_get(entry, &type, &port);
    if(ret != CS_E_OK)
    {
        return ret;
    }

    ret = ctc_oam_get_alarm_config(type, port, entry.id, &alarm_config);
    if(ret != CS_E_OK)
    {
        return ret;
    }
    if(alarm_config == CTC_ALARM_ON)
    {
        *status = OAM_CTC_EVENT_STATUS_ENABLE;
    }
    else
    {
        *status = OAM_CTC_EVENT_STATUS_DISABLE;
    }

    return ret;
}

cs_status ctc_oam_event_thresh_get_adapt(
        oam_ctc_event_entry_t entry,
        cs_uint32 *raise_thresh,
        cs_uint32 *clear_thresh)
{
    ctc_oam_alarm_class_e type = CTC_ONU_ALARM;
    cs_port_id_t port= CS_PON_PORT_ID;
    cs_status ret = CS_E_OK;

    ret = ctc_oam_onu_event_type_get(entry, &type, &port);
    if(ret != CS_E_OK)
    {
        return ret;
    }

    ret = ctc_oam_get_alarm_threshold(type, port, entry.id, raise_thresh, clear_thresh);
    
    return ret;
}

cs_status ctc_oam_event_status_set_adapt(
        oam_ctc_event_status_t config)
{
    ctc_oam_alarm_class_e type = CTC_ONU_ALARM;
    cs_port_id_t port= CS_PON_PORT_ID;
    cs_status ret = CS_E_OK;
    cs_uint8 enable = 0;

    ret = ctc_oam_onu_event_type_get(config.entry, &type, &port);
    if(ret != CS_E_OK)
    {
        return ret;
    }

    if(config.status == OAM_CTC_EVENT_STATUS_ENABLE)
    {
        enable = CTC_ALARM_ON;
    }
    else
    {
        enable = CTC_ALARM_OFF;
    }

    ret = ctc_oam_set_alarm_config(type, port, config.entry.id, enable);
    
    return ret;
}

cs_status ctc_oam_event_thresh_set_adapt(
        oam_ctc_event_thresh_t config)
{
    ctc_oam_alarm_class_e type = CTC_ONU_ALARM;
    cs_port_id_t port= CS_PON_PORT_ID;
    cs_status ret = CS_E_OK;

    ret = ctc_oam_onu_event_type_get(config.entry, &type, &port);
    if(ret != CS_E_OK)
    {
        return ret;
    }

    ret = ctc_oam_set_alarm_threshold(type, port, config.entry.id, config.raise_thresh, config.clear_thresh);
    
    return ret;
}


/********************* ALARM ADAPT ****************/
cs_status ctc_oam_onu_alarm_class_port_get(
        ctc_oam_mgmt_obj_idx_t index,
        ctc_oam_alarm_class_e *type,
        cs_port_id_t *port)
{
    if(!index.valid)
    {
        *type = CTC_ONU_ALARM;
    }
    else
    {
        if(index.idxLeaf == OAM_CTC_LEAF_CODE_ONU_PON)
        {
            *type = CTC_PON_ALARM;
            *port = CS_PON_PORT_ID;
        }
        else if(index.idxLeaf == OAM_CTC_LEAF_CODE_ONU_PORT)
        {
            if(index.idxBranch == OAM_CTC_INDEX_BRANCH && index.idxValue == 0)
            {
                *type = CTC_PON_ALARM;
                *port = CS_PON_PORT_ID;
            }
            else
            {
                *type = CTC_PORT_ALARM;
                *port = index.idxValue;
            }
        }
        else
            return CS_E_PARAM;
    }

    return CS_E_OK;
}

cs_status ctc_oam_onu_alarm_get_adapt(
        ctc_oam_mgmt_obj_idx_t index,
        cs_uint16 alarm_id,
        cs_uint32 * alarm_config)
{
    ctc_oam_alarm_class_e type = CTC_ONU_ALARM;
    cs_port_id_t port= CS_PON_PORT_ID;
    cs_status ret = CS_E_OK;

    ret = ctc_oam_onu_alarm_class_port_get(index, &type, &port);
    if(ret != CS_E_OK)
        return ret;

    return ctc_oam_get_alarm_config(type, port, alarm_id, alarm_config);

}

cs_status ctc_oam_onu_alarm_set_adapt(
        ctc_oam_mgmt_obj_idx_t index,
        cs_uint16 alarm_id,
        cs_uint32  alarm_config)
{
    ctc_oam_alarm_class_e type = CTC_ONU_ALARM;
    cs_port_id_t port= 0;
    cs_status ret = CS_E_OK;

    ret = ctc_oam_onu_alarm_class_port_get(index, &type, &port);
    if(ret != CS_E_OK)
        return ret;

    return ctc_oam_set_alarm_config(type, port, alarm_id, alarm_config);

}

cs_status ctc_oam_onu_alarm_threshold_get_adapt(
        ctc_oam_mgmt_obj_idx_t index,
        cs_uint16 alarm_id,
        cs_uint32  *raise_threshold,
        cs_uint32  *clear_threshold)
{
    ctc_oam_alarm_class_e type = CTC_ONU_ALARM;
    cs_port_id_t port= 0;
    cs_status ret = CS_E_OK;

    ret = ctc_oam_onu_alarm_class_port_get(index, &type, &port);
    if(ret != CS_E_OK)
        return ret;

    return ctc_oam_get_alarm_threshold(type, port, alarm_id, 
                                                    raise_threshold, clear_threshold);

}

cs_status ctc_oam_onu_alarm_threshold_set_adapt(
        ctc_oam_mgmt_obj_idx_t index,
        cs_uint16 alarm_id,
        cs_uint32  raise_threshold,
        cs_uint32  clear_threshold)
{
    ctc_oam_alarm_class_e type = CTC_ONU_ALARM;
    cs_port_id_t port= 0;
    cs_status ret = CS_E_OK;

    ret = ctc_oam_onu_alarm_class_port_get(index, &type, &port);
    if(ret != CS_E_OK)
        return ret;

    return ctc_oam_set_alarm_threshold(type, port, alarm_id, 
                                                  raise_threshold, clear_threshold);

}



/***************** Statics Adapt START ***********************/
cs_status ctc_oam_onu_stats_monitor_status_get_adapt(
        cs_port_id_t port,
        cs_uint16 *monitor_status,
        cs_uint32 * monitor_period)
{
    return ctc_onu_stats_monitor_status_get(port, monitor_status, monitor_period);
}

cs_status ctc_oam_onu_stats_monitor_status_set_adapt(
        cs_port_id_t port,
        cs_uint16 monitor_status,
        cs_uint32  monitor_period)
{   
    return ctc_onu_stats_monitor_status_set(port, monitor_status, monitor_period);
}

cs_status ctc_oam_onu_curr_stats_get_adapt(
        cs_port_id_t port,
        oam_ctc_onu_port_stats_t * stats)
{
    return ctc_onu_monitor_stats_current_get(port, stats);
}

cs_status ctc_oam_onu_curr_stats_set_adapt(
        cs_port_id_t port,
        oam_ctc_onu_port_stats_t * stats)
{
    return ctc_onu_stats_monitor_restart(port);
}

cs_status ctc_oam_onu_histroy_stats_get_adapt(
        cs_port_id_t port,
        oam_ctc_onu_port_stats_t * stats)
{
    return ctc_onu_monitor_stats_histroy_get(port, stats);
}

cs_status ctc_oam_auth_keep_silence_set_adapt(cs_boolean enable)
{
    cs_callback_context_t     context;
    
    return epon_request_onu_mpcp_offline_set(context, 0, 0, enable);
}

cs_status  ctc_oam_fw_get_adapt(oam_image_version_t * img_ver_info)
{
    sys_chip_info_t info;
    cs_callback_context_t context; 

    if(img_ver_info == NULL){
        return CS_E_PARAM;
    }

    memset(&info,0x00,sizeof(info));

    cs_plat_chipset_info_get(context,0,0,&info);

    img_ver_info->major  = info.fw_ver.major;
    img_ver_info->minor  = info.fw_ver.minor;
    img_ver_info->revision  = info.fw_ver.revision;
    img_ver_info->build_id  = info.fw_ver.build_id ;

    return CS_E_OK;

}

cs_status ctc_oam_chipid_get_adapt(sys_chip_info_t * info)
{
    cs_callback_context_t context; 

    if(info == NULL){
        return CS_E_PARAM;
    }

    cs_plat_chipset_info_get(context,0,0,info);

    return CS_E_OK;
}

cs_status ctc_oam_sn_get_adapt(sys_onu_info_t * info)
{
    cs_callback_context_t context; 

    if(info == NULL){
        return CS_E_PARAM;
    }

    cs_plat_onu_info_get(context,0,0,info);

    return CS_E_OK;
}
