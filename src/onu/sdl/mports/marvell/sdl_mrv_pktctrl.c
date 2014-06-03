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
#include "plat_common.h" 
#include "aal_l2.h"
#include "aal.h"
#include "aal_flow.h"
#include "sdl.h"
#include "aal_cls.h"
#include "sdl_ma.h"
#include "sdl_pktctrl.h"

#include "MARVELL_BSP_expo.h"
#include "switch_expo.h"
#include "gtDrvSwRegs.h"
#include "switch_drv.h"
#include "msApiDefs.h"

extern cs_status aal_pon_mac_addr_get( CS_IN cs_mac_t *mac);
#define LOOPDELETADD 
#define __LOOPDETECT_ETHTYPE 0xfffe
#define __IROS_OAM_ETHTYPE   0xffff
#define __PPPOE_DIS_ETHTYPE  0x8863

#define __DEFAULT_MSTI       0

/********************************************
*     RTL ACL resource       0--highest pri *
*     special packets        10 (0-9)       *
*     CTC classfication      24 (10-33)     *
*    / QOS remarking         24 (10-33)     *
*     MAC filter/bind        24 (34-57)     *
*     egress queue ratelimit 4  (58-61)     *
*     vlan                   1  (63)        *
*     reserved               1  (62)        *
*********************************************/
#define __PKT_STP_TRAP_RULE         0
#define __PKT_STP_BLOCK_RULE        1
#define __PKT_ARP_RULE              2
#define __PKT_DHCP_RULE             3
#define __PKT_PPPOE_RULE            4
#define __PKT_LOOPDETECT_RULE       5
#define __PKT_IROS_RULE             6
#define __PKT_IGMP_RULE             7
#define __PKT_MYMAC_RULE            8
#ifdef LOOPDELETADD
#define __PKT_LOOP_RULE				9
#endif

#define __PKT_RATE_MAX     2000

#define __FLOW_ID_PRI(queue) (AAL_RATE_LIMIT_FLOW_28 +(queue))


static cs_sdl_queue_t   g_ds_pkt_queue[CS_PKT_TYPE_NUM];

static cs_uint32        g_queue_pkt_rate[4];

static cs_sdl_pkt_dst_t g_pkt_dst[2][CS_PKT_TYPE_NUM];

static cs_uint32        g_stp_blockport_status;


static cs_uint8 g_sdl_pkt_map[] = 
{
    AAL_PKT_BPDU,           /* CS_PKT_BPDU*/
    AAL_PKT_8021X,          /* CS_PKT_8021X */
    AAL_PKT_IGMP,           /* CS_PKT_GMP */
    AAL_PKT_ARP,            /* CS_PKT_ARP */
    AAL_PKT_OAM,            /* CS_PKT_OAM */
    AAL_PKT_MPCP,           /* CS_PKT_MPCP */
    AAL_PKT_DHCP,           /* CS_PKT_DHCP */
    AAL_PKT_SWT,            /* CS_PKT_IROS */
    AAL_PKT_SWT,            /* CS_PKT_PPPOE */
    AAL_PKT_SWT,            /* CS_PKT_IP */
    AAL_PKT_IPV6NDP,        /* CS_PKT_NDP */
    AAL_PKT_SWT,            /* CS_PKT_LOOPDETECT */
    AAL_PKT_MYMAC,          /* CS_PKT_MYMAC */
    AAL_PKT_SWT,            /* CS_PKT_TYPE_NUM */
#ifdef LOOPDELETADD
	AAL_PKT_SWT,           
	AAL_PKT_SWT,
	AAL_PKT_SWT,
#endif
};



static cs_status __l2_pon_iros_state_set( CS_IN cs_sdl_pkt_dst_t state )
{
    cs_aal_pkt_spec_udf_msk_t spe_udf_msk;
    cs_aal_pkt_spec_udf_t     spe_udf_cfg;
    cs_aal_spec_pkt_ctrl_msk_t spe_pkt_msk;
    cs_aal_spec_pkt_ctrl_t     spe_pkt_cfg;

    /* set udf2 ethtype 0xffff */
    spe_udf_cfg.udf2_etype = __IROS_OAM_ETHTYPE;
    spe_udf_msk.u32 = 0;
    spe_udf_msk.s.udf2_etype = 1;
    (void)aal_pkt_special_udf_set(AAL_PORT_ID_PON, spe_udf_msk, &spe_udf_cfg);

    memset(&spe_pkt_cfg, 0, sizeof(cs_aal_spec_pkt_ctrl_t));
    
    spe_pkt_msk.u32 = 0;
    spe_pkt_msk.s.dpid = 1;
    spe_pkt_cfg.dpid.dst_op = (state==DST_FE)?AAL_SPEC_DST_FE:((state==DST_CPU)?AAL_SPEC_DST_PORT: AAL_SPEC_DST_DROP);
    spe_pkt_cfg.dpid.dpid = AAL_PORT_ID_CPU;
    
    return aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_UDF2, spe_pkt_msk, &spe_pkt_cfg);
}
#ifdef LOOPDELETADD
static cs_status __l2_pon_lpd_state_set( CS_IN cs_sdl_pkt_dst_t state )
{
    cs_aal_pkt_spec_udf_msk_t spe_udf_msk;
    cs_aal_pkt_spec_udf_t     spe_udf_cfg;
    cs_aal_spec_pkt_ctrl_msk_t spe_pkt_msk;
    cs_aal_spec_pkt_ctrl_t     spe_pkt_cfg;

    /* set udf2 ethtype 0xffff */
    spe_udf_cfg.udf0_etype = 0x0800;
	spe_udf_cfg.udf0_da.addr[0]=0x00;
	spe_udf_cfg.udf0_da.addr[1]=0x0f;
	spe_udf_cfg.udf0_da.addr[2]=0xe9;
	spe_udf_cfg.udf0_da.addr[3]=0x04;
	spe_udf_cfg.udf0_da.addr[4]=0x8e;
	spe_udf_cfg.udf0_da.addr[5]=0xdf;
	spe_udf_cfg.udf0_chk_da = 1;
    spe_udf_msk.u32 = 0;
    spe_udf_msk.s.udf0_etype = 1;
	spe_udf_msk.s.udf0_da = 1;
	spe_udf_msk.s.udf0_chk_da = 1;
    (void)aal_pkt_special_udf_set(AAL_PORT_ID_GE, spe_udf_msk, &spe_udf_cfg);

  //  memset(&spe_pkt_cfg, 0, sizeof(cs_aal_spec_pkt_ctrl_t));
    
    spe_pkt_msk.u32 = 0;
    spe_pkt_msk.s.dpid = 1;
    spe_pkt_cfg.dpid.dst_op = AAL_SPEC_DST_PORT;
    spe_pkt_cfg.dpid.dpid = AAL_PORT_ID_CPU;
    
    return aal_special_pkt_behavior_set(AAL_PORT_ID_GE, AAL_PKT_UDF0, spe_pkt_msk, &spe_pkt_cfg);
}
#endif

static cs_status __l2_pon_spec_pkt_state_set(
     CS_IN cs_pkt_type_t             pkt_type,
     CS_IN cs_sdl_pkt_dst_t          state
)
{  
    cs_aal_spec_pkt_ctrl_msk_t  pkt_msk;
    cs_aal_spec_pkt_ctrl_t      pkt_cfg;
    cs_aal_pkt_type_t           pkt;
    cs_status                   ret;

         
    if(pkt_type >= CS_PKT_TYPE_NUM)
    {
        SDL_MIN_LOG("pkt type is not be supported.(%d) FILE: %s, LINE: %d", pkt_type, __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    memset(&pkt_msk, 0, sizeof(cs_aal_spec_pkt_ctrl_msk_t));
    memset(&pkt_cfg, 0, sizeof(cs_aal_spec_pkt_ctrl_t));

    pkt_msk.u32 = 0;
    pkt_msk.s.dpid = 1;
   
    pkt_cfg.dpid.dst_op = (state==DST_FE)?AAL_SPEC_DST_FE:((state==DST_CPU)?AAL_SPEC_DST_PORT: AAL_SPEC_DST_DROP);
    pkt_cfg.dpid.dpid = AAL_PORT_ID_CPU;
        
    pkt = g_sdl_pkt_map[pkt_type];

    if(pkt == AAL_PKT_SWT)
    {
        if(pkt_type == CS_PKT_PPPOE)
        {
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_PPPoE_DIS, pkt_msk, &pkt_cfg);
            ret += aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_PPPoE_Session, pkt_msk, &pkt_cfg);
        }
        else if (pkt_type == CS_PKT_IROS)
        {
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_HELLO, pkt_msk, &pkt_cfg);
            ret += __l2_pon_iros_state_set(state );
        }
#ifdef LOOPDELETADD
		else if(pkt_type == CS_PKT_GWD_LOOPDETECT)
		{
			  ret = aal_special_pkt_behavior_set(AAL_PORT_ID_GE, AAL_PKT_UDF0, pkt_msk, &pkt_cfg);
          	  ret += __l2_pon_lpd_state_set(state );
		}
#endif
        else 
        {
            SDL_MIN_LOG("pkt type is not be supported.(%d) FILE: %s, LINE: %d", pkt_type, __FILE__, __LINE__);
            ret = CS_E_ERROR;
        }
    }
    else
    {   
        ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, pkt, pkt_msk, &pkt_cfg);          
    }
        
    if(ret)
    {
        SDL_MIN_LOG("pkt type set failed.(%d) FILE: %s, LINE: %d", pkt_type, __FILE__, __LINE__);
        return CS_E_ERROR;
    }     
     
    return CS_E_OK;
}

#if 0
static cs_status __l2_sw_mac_trap_set(
     CS_IN cs_pkt_type_t             pkt_type,
     CS_IN cs_sdl_pkt_dst_t          state
)
{   
    GT_ETHERADDR mac;
    GT_STATUS rtk_ret;

    if(pkt_type == CS_PKT_BPDU)
    {  
        mac.arEther[0] = 0x01;
        mac.arEther[1] = 0x80;
        mac.arEther[2] = 0xc2;
        mac.arEther[3] = 0x00;
        mac.arEther[4] = 0x00;
        mac.arEther[5] = 0x00;
    }
    else if(pkt_type == CS_PKT_OAM)
    {  
        mac.arEther[0] = 0x01;
        mac.arEther[1] = 0x80;
        mac.arEther[2] = 0xc2;
        mac.arEther[3] = 0x00;
        mac.arEther[4] = 0x00;
        mac.arEther[5] = 0x02;
    }
    else if(pkt_type == CS_PKT_8021X)
    {  
        mac.arEther[0] = 0x01;
        mac.arEther[1] = 0x80;
        mac.arEther[2] = 0xc2;
        mac.arEther[3] = 0x00;
        mac.arEther[4] = 0x00;
        mac.arEther[5] = 0x03;
    }
    else
    {
        SDL_MIN_LOG("pkt type is not be supported.(%d) FILE: %s, LINE: %d", pkt_type, __FILE__, __LINE__);
        return CS_E_PARAM;
    }  

//    rtk_ret = rtk_trap_rmaAction_set(&mac, rma_action);
//    mtodo: mrv api 针对不同转发规则的相应MAC地址的ATU操作
    if(rtk_ret!=GT_OK)
    {
        SDL_MIN_LOG("rtk_trap_rmaAction_set return %d. FILE: %s, LINE: %d", rtk_ret, __FILE__, __LINE__);
        return CS_E_ERROR;
    }

    return CS_E_OK;
}

static cs_status __l2_sw_acl_trap_set(
     CS_IN cs_pkt_type_t             pkt_type,
     CS_IN cs_sdl_pkt_dst_t          state
)
{  
    GT_STATUS rt = 0;
    cs_uint32 rule_idx;

    if(pkt_type == CS_PKT_ARP)
    {  
        rule_idx = __PKT_ARP_RULE;
    }
    else if(pkt_type == CS_PKT_DHCP)
    {  
        rule_idx = __PKT_DHCP_RULE;
    }
    else if(pkt_type == CS_PKT_GMP)
    {  
        rule_idx = __PKT_IGMP_RULE;
    }
    else if(pkt_type == CS_PKT_PPPOE)
    {  
        rule_idx = __PKT_PPPOE_RULE;
    }
    else if(pkt_type == CS_PKT_MYMAC)
    {  
        rule_idx = __PKT_MYMAC_RULE;
    }
    else if(pkt_type == CS_PKT_LOOPDETECT)
    {  
        rule_idx = __PKT_LOOPDETECT_RULE;
    }
    else if(pkt_type == CS_PKT_IROS)
    {  
        rule_idx = __PKT_IROS_RULE;
    }
	#ifdef LOOPDELETADD
	else if(pkt_type == CS_PKT_GWD_LOOPDETECT)
	{
		rule_idx = __PKT_LOOP_RULE;
	}
	#endif
    else
    {
        SDL_MIN_LOG("pkt type is not be supported.(%d) FILE: %s, LINE: %d", pkt_type, __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(state == DST_FE )
    {
        if(pkt_type == CS_PKT_PPPOE)
        {
//        	mtodo: pppoe
        }
    }
    else
    {            
        if(pkt_type == CS_PKT_LOOPDETECT)
        {
//        	mtodo: loopdetect
        }
        else if(pkt_type == CS_PKT_IROS)
        {
//        	mtodo: CS_PKT_IROS
        }
		#ifdef LOOPDELETADD
	    else if(pkt_type == CS_PKT_GWD_LOOPDETECT)
        {
/*
            field.fieldType = FILTER_FIELD_DMAC;
            field.filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;

            field.filter_pattern_union.dmac.value.octet[0] = 0x00;
            field.filter_pattern_union.dmac.value.octet[1] = 0x0f;
            field.filter_pattern_union.dmac.value.octet[2] = 0xe9;
            field.filter_pattern_union.dmac.value.octet[3] = 0x04;
            field.filter_pattern_union.dmac.value.octet[4] = 0x8e;
            field.filter_pattern_union.dmac.value.octet[5] = 0xdf;            
            field.filter_pattern_union.dmac.mask.octet[0]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[1]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[2]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[3]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[4]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[5]  = 0xFF;
            */
//	    	mtodo: CS_PKT_GWD_LOOPDETECT
        }

		#endif
        else if(pkt_type == CS_PKT_PPPOE)
        {
//        	mtodo: pppoe
        }
        else if(pkt_type == CS_PKT_MYMAC)
        {   
             cs_mac_t     mac;

            //MYmac is configured in Startcofig,so it is fixed.
            //But must pay attention to the order of initialization

            memset(&mac, 0, sizeof(cs_mac_t));

            (void)aal_pon_mac_addr_get(&mac);

            /*

            field.fieldType = FILTER_FIELD_DMAC;
            field.filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;

            field.filter_pattern_union.dmac.value.octet[0] = mac.addr[0];
            field.filter_pattern_union.dmac.value.octet[1] = mac.addr[1];
            field.filter_pattern_union.dmac.value.octet[2] = mac.addr[2];
            field.filter_pattern_union.dmac.value.octet[3] = mac.addr[3];
            field.filter_pattern_union.dmac.value.octet[4] = mac.addr[4];
            field.filter_pattern_union.dmac.value.octet[5] = mac.addr[5];            
            field.filter_pattern_union.dmac.mask.octet[0]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[1]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[2]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[3]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[4]  = 0xFF;
            field.filter_pattern_union.dmac.mask.octet[5]  = 0xFF;
            */

//            mtodo: mymac process
            
        }
        else
        {
        }
    }

    if(rt)
    {
        SDL_MIN_LOG("RTK ACL rule set failed.(%d) FILE: %s, LINE: %d", rule_idx,  __FILE__, __LINE__);
        return CS_E_ERROR;
    }
     
    return CS_E_OK;
}

static cs_status __l2_sw_uni_set(
     CS_IN cs_pkt_type_t             pkt_type,
     CS_IN cs_sdl_pkt_dst_t          state
)
{   
    cs_status    ret;
    
    if((pkt_type == CS_PKT_BPDU)||
        (pkt_type == CS_PKT_OAM)||
        (pkt_type == CS_PKT_8021X))
    {  
        ret = __l2_sw_mac_trap_set(pkt_type, state);
    }
    else 
    {  
        ret = __l2_sw_acl_trap_set(pkt_type, state);
    }
    
    return ret;
}
#endif

static cs_status __l2_sw_uni_stp_block(
     CS_IN cs_port_id_t              portid,
     CS_IN cs_boolean                block
)
{
    GT_STATUS rt;
    cs_status ret = CS_E_OK;

    rt = GT_OK;

#if 0
    rtk_filter_field_t  field[2];
    rtk_filter_cfg_t     cfg;
    rtk_filter_action_t  act; 
    rtk_filter_number_t  ruleNum;

    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));
    memset(&field, 0, 2*sizeof(rtk_filter_field_t));

    field[0].fieldType = FILTER_FIELD_DMAC;       
    field[0].filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
    field[0].filter_pattern_union.dmac.value.octet[0] = 0x01;
    field[0].filter_pattern_union.dmac.value.octet[1] = 0x80;
    field[0].filter_pattern_union.dmac.value.octet[2] = 0xc2;
    field[0].filter_pattern_union.dmac.value.octet[3] = 0x00;
    field[0].filter_pattern_union.dmac.value.octet[4] = 0x00;
    field[0].filter_pattern_union.dmac.value.octet[5] = 0x00;
    field[0].filter_pattern_union.dmac.mask.octet[0]  = 0xFF;
    field[0].filter_pattern_union.dmac.mask.octet[1]  = 0xFF;
    field[0].filter_pattern_union.dmac.mask.octet[2]  = 0xFF;
    field[0].filter_pattern_union.dmac.mask.octet[3]  = 0xFF;
    field[0].filter_pattern_union.dmac.mask.octet[4]  = 0xFF;
    field[0].filter_pattern_union.dmac.mask.octet[5]  = 0xFF;
    field[0].next = NULL;

    field[1].fieldType = FILTER_FIELD_DMAC;       
    field[1].filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
    field[1].filter_pattern_union.dmac.value.octet[0] = 0x00;
    field[1].filter_pattern_union.dmac.value.octet[1] = 0x00;
    field[1].filter_pattern_union.dmac.value.octet[2] = 0x00;
    field[1].filter_pattern_union.dmac.value.octet[3] = 0x00;
    field[1].filter_pattern_union.dmac.value.octet[4] = 0x00;
    field[1].filter_pattern_union.dmac.value.octet[5] = 0x00;
    field[1].filter_pattern_union.dmac.mask.octet[0]  = 0x00;
    field[1].filter_pattern_union.dmac.mask.octet[1]  = 0x00;
    field[1].filter_pattern_union.dmac.mask.octet[2]  = 0x00;
    field[1].filter_pattern_union.dmac.mask.octet[3]  = 0x00;
    field[1].filter_pattern_union.dmac.mask.octet[4]  = 0x00;
    field[1].filter_pattern_union.dmac.mask.octet[5]  = 0x00;
    field[1].next = NULL;
            
    if(block)    
    {
        if(g_stp_blockport_status &(1 << (portid-1)))
            goto end;

        g_stp_blockport_status |= (1 << (portid-1));
            
        rt = rtk_filter_igrAcl_field_add(&cfg, &field[1]);       
        if(rt)
        {
            ret = CS_E_ERROR; 
            goto end;
        }
        act.actEnable[FILTER_ENACT_DROP] = TRUE;
        
        cfg.invert = FALSE;    
        cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
        cfg.activeport.value = g_stp_blockport_status;
        cfg.activeport.mask = 0xFF;  
        
        rt = rtk_filter_igrAcl_cfg_del(__PKT_STP_BLOCK_RULE);
        rt += rtk_filter_igrAcl_cfg_add(__PKT_STP_BLOCK_RULE, &cfg, &act, &ruleNum); 
        if(rt)
        {
            ret = CS_E_ERROR; 
            goto end;
        }
        memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
        memset(&act, 0, sizeof(rtk_filter_action_t));
        
        rt = rtk_filter_igrAcl_field_add(&cfg, &field[0]);       
        if(rt)
        {
            ret = CS_E_ERROR; 
            goto end;
        }
        cfg.invert = FALSE;    
        cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
        cfg.activeport.value = g_stp_blockport_status;
        cfg.activeport.mask = 0xFF;  

        act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;       
        act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;           
        act.filterPriority = 7;
        
        rt = rtk_filter_igrAcl_cfg_del(__PKT_STP_TRAP_RULE);
        rt += rtk_filter_igrAcl_cfg_add(__PKT_STP_TRAP_RULE, &cfg, &act, &ruleNum); 
        if(rt)
        {
            ret = CS_E_ERROR; 
            goto end;
        }           
    }
    else
    {
        if(0==(g_stp_blockport_status &(1 << (portid-1))))
            goto end;

        g_stp_blockport_status &= ~(1<<(portid-1));

        if(0 == g_stp_blockport_status)
        {    
            rt = rtk_filter_igrAcl_cfg_del(__PKT_STP_BLOCK_RULE);
            rt += rtk_filter_igrAcl_cfg_del(__PKT_STP_TRAP_RULE);
            if(rt)
            {
                ret = CS_E_ERROR; 
                goto end;
            }            
        }
        else
        {
            rt = rtk_filter_igrAcl_field_add(&cfg, &field[1]);       
            if(rt)
            {
                ret = CS_E_ERROR; 
                goto end;
            }
            act.actEnable[FILTER_ENACT_DROP] = TRUE;
            
            cfg.invert = FALSE;    
            cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
            cfg.activeport.value = g_stp_blockport_status;
            cfg.activeport.mask = 0xFF;  
            
            rt = rtk_filter_igrAcl_cfg_del(__PKT_STP_BLOCK_RULE);
            rt += rtk_filter_igrAcl_cfg_add(__PKT_STP_BLOCK_RULE, &cfg, &act, &ruleNum); 
            if(rt)
            {
                ret = CS_E_ERROR; 
                goto end;
            }
            memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
            memset(&act, 0, sizeof(rtk_filter_action_t));
            
            rt = rtk_filter_igrAcl_field_add(&cfg, &field[0]);       
            if(rt)
            {
                ret = CS_E_ERROR; 
                goto end;
            }
            cfg.invert = FALSE;    
            cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
            cfg.activeport.value = g_stp_blockport_status;
            cfg.activeport.mask = 0xFF;  

            act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;       
            act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;           
            act.filterPriority = 7;
            
            rt = rtk_filter_igrAcl_cfg_del(__PKT_STP_TRAP_RULE);
            rt += rtk_filter_igrAcl_cfg_add(__PKT_STP_TRAP_RULE, &cfg, &act, &ruleNum); 
            if(rt)
            {
                ret = CS_E_ERROR; 
                goto end;
            }           
        }
    }
#else
//    mtodo: 添加目的mac 01-80-c2-00-00-00及全0的丢弃规则。
#endif
   
    return ret;
}

#if 0
static cs_status __l2_mii_port_set(
     CS_IN cs_pkt_type_t             pkt_type,
     CS_IN cs_sdl_pkt_dst_t          state
)
{   
    //Not support now
    return CS_E_OK;
}
#endif

/******Special packet MA status ********************/ 
static cs_status __l2_pkt_rate_set(    
    CS_IN cs_sdl_queue_t            queue,
    CS_IN cs_uint32                 pkt_s
)
{   
    cs_status               ret = CS_E_OK;
    cs_aal_rate_limit_msk_t rl_msk;
    cs_aal_rate_limit_t     rate_limit;
    
    memset(&rate_limit, 0, sizeof(cs_aal_rate_limit_t));

    rl_msk.u = 0;
     
    rl_msk.s.enable = 1;
    rl_msk.s.rate   = 1;
    rl_msk.s.cbs    = 1;
    rl_msk.s.rlmode = 1;
    rate_limit.rate = pkt_s;
    rate_limit.ctrl.s.enable = (pkt_s == 0)? 0: 1;
    rate_limit.ctrl.s.rlmode = AAL_RATE_LIMIT_MODE_PPS;
    rate_limit.cbs = 100;
    
    ret = aal_flow_rate_limit_set(__FLOW_ID_PRI(queue), &rl_msk, &rate_limit);
      
    return ret;
}

static cs_status __l2_ds_pkt_pri_set(    
    CS_IN cs_pkt_type_t             pkt_type,
    CS_IN cs_sdl_queue_t            queue
)
{
    cs_aal_spec_pkt_ctrl_msk_t  pkt_msk;
    cs_aal_spec_pkt_ctrl_t      pkt_cfg;
    cs_aal_pkt_type_t           pkt;
    cs_status                   ret = CS_E_OK;


    memset(&pkt_cfg, 0, sizeof(cs_aal_spec_pkt_ctrl_t));

    pkt_msk.u32          = 0;
    pkt_msk.s.cos        = 1;
    pkt_msk.s.rate_limit = 1;
    pkt_msk.s.bypass_plc = 1;

    /* cos: 0,1~que 0, */
    /*      2,3~que 1, */
    /*      4,5~que 2, */
    /*      6,7~que 3, */
    pkt_cfg.cos        = 2*queue;      
    pkt_cfg.flow_en    = TRUE; 
    pkt_cfg.flow_id    = (__FLOW_ID_PRI(queue)-AAL_RATE_LIMIT_FLOW_0);
    pkt_cfg.bypass_plc = FALSE;
    
    pkt = g_sdl_pkt_map[pkt_type];
      
    if(pkt == AAL_PKT_SWT)
    {
        if(pkt_type == CS_PKT_PPPOE)
        {
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_PPPoE_DIS, pkt_msk, &pkt_cfg);
            if(ret)
                return ret;
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_PPPoE_Session, pkt_msk, &pkt_cfg);
        }
        else if (pkt_type == CS_PKT_IROS)
        {
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_HELLO, pkt_msk, &pkt_cfg);
            if(ret)
                return ret;
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_UDF2, pkt_msk, &pkt_cfg);
        }
    }
    else
    {   
        ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, pkt, pkt_msk, &pkt_cfg);          
    }   
    
    return ret;
}

static cs_status __l2_ds_pkt_def_set(    
    CS_IN cs_pkt_type_t             pkt_type
)
{
    cs_aal_spec_pkt_ctrl_msk_t  pkt_msk;
    cs_aal_spec_pkt_ctrl_t      pkt_cfg;
    cs_aal_pkt_type_t           pkt;
    cs_status                   ret = CS_E_OK;

    
    memset(&pkt_cfg, 0, sizeof(cs_aal_spec_pkt_ctrl_t));

    pkt_msk.u32          = 0;
    pkt_msk.s.cos        = 1;
    pkt_msk.s.rate_limit = 1;
    pkt_msk.s.bypass_plc = 1;

    pkt_cfg.cos          = 0;
    pkt_cfg.flow_en      = FALSE; 
    pkt_cfg.flow_id      = 0;
    pkt_cfg.bypass_plc   = FALSE;
    
    pkt = g_sdl_pkt_map[pkt_type];
      
    if(pkt == AAL_PKT_SWT)
    {
        if(pkt_type == CS_PKT_PPPOE)
        {
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_PPPoE_DIS, pkt_msk, &pkt_cfg);
            if(ret)
                return ret;
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_PPPoE_Session, pkt_msk, &pkt_cfg);
        }
        else if (pkt_type == CS_PKT_IROS)
        {
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_HELLO, pkt_msk, &pkt_cfg);
            if(ret)
                return ret;
            ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, AAL_PKT_UDF2, pkt_msk, &pkt_cfg);
        }
    }
    else
    {   
        ret = aal_special_pkt_behavior_set(AAL_PORT_ID_PON, pkt, pkt_msk, &pkt_cfg);          
    }   
    
    return ret;
}

cs_status epon_request_onu_port_stp_state_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              portid,
    CS_IN cs_sdl_stp_state_t        state
)
{
    GT_LPORT rtk_port;
    GT_PORT_STP_STATE stp_state;
    GT_STATUS rtk_ret;
    cs_status ret = CS_E_OK;

    GT_32 unit, port;

    if((portid < CS_UNI_PORT_ID1) || (portid > UNI_PORT_MAX))
    {
        SDL_MIN_LOG("port id invalid.(%d) FILE: %s, LINE: %d", portid, __FILE__, __LINE__);

        ret = CS_E_PARAM; 
        goto end;
    }

    if(state > STP_S_FORWARDING)
    {
        SDL_MIN_LOG("stp state invalid.(%d) FILE: %s, LINE: %d", state, __FILE__, __LINE__);
        ret = CS_E_PARAM; 
        goto end;
    }

    rtk_port = L2P_PORT(portid);
    stp_state = state;

    if(GT_OK != gt_getswitchunitbylport(rtk_port, &unit, &port))
    	return CS_E_PARAM;

    rtk_ret = gstpSetPortState(QD_DEV_PTR, port, stp_state);

    if(rtk_ret!=GT_OK)
    {
        SDL_MIN_LOG("gstpSetPortState return %d. FILE: %s, LINE: %d", rtk_ret, __FILE__, __LINE__);
        ret = CS_E_ERROR; 
        goto end;
    }

    if((stp_state == STP_S_BLOCKING)||
        (stp_state == STP_S_LEARNING))
    {
        // blocking port
        ret = __l2_sw_uni_stp_block( portid, TRUE);
    }
    else
    {
        // Free port
        ret=  __l2_sw_uni_stp_block( portid, FALSE);
    } 
        
end:
    return ret;
}

cs_status epon_request_onu_port_stp_state_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              portid,
    CS_OUT cs_sdl_stp_state_t        *state
)
{
    GT_LPORT rtk_port;
    GT_PORT_STP_STATE stp_state;
    GT_STATUS rtk_ret;

    GT_32 unit, port;

    if((portid < CS_UNI_PORT_ID1) || (portid > UNI_PORT_MAX))
    {
        SDL_MIN_LOG("port id invalid.(%d) FILE: %s, LINE: %d", portid, __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(NULL == state)
    {
        SDL_MIN_LOG("null pointer. FILE: %s, LINE: %d", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    rtk_port = L2P_PORT(portid);
    if(GT_OK != gt_getswitchunitbylport(rtk_port, &unit, &port))
    	return CS_E_PARAM;

    rtk_ret = gstpGetPortState(QD_DEV_PTR, port, &stp_state);
    if(rtk_ret!=GT_OK)
    {
        SDL_MIN_LOG("gstpGetPortState return %d. FILE: %s, LINE: %d", rtk_ret, __FILE__, __LINE__);
        return CS_E_ERROR;
    }

    *state = stp_state;

    return CS_E_OK;
}

cs_status epon_request_onu_spec_pkt_dst_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_direction_t            direction,
    CS_IN cs_pkt_type_t             pkt_type,
    CS_IN cs_sdl_pkt_dst_t          state
)
{
    cs_status                   ret;

   cs_printf("pkt_type is %d, state is %d\r\n",pkt_type,state);
    if(direction > CS_UP_STREAM)
    {
        SDL_MIN_LOG("Direction invalid.(%d) FILE: %s, LINE: %d", direction, __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(state > DST_CPU)
    {
        SDL_MIN_LOG("pkt state invalid.(%d) FILE: %s, LINE: %d", state, __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(pkt_type >= CS_PKT_TYPE_NUM)
    {
        SDL_MIN_LOG("pkt type invalid.(%d) FILE: %s, LINE: %d", pkt_type, __FILE__, __LINE__);
        return CS_E_PARAM;
    }
    
    if(CS_DOWN_STREAM==direction)
    {
        /* set PON(lynxd) chip */              
        ret = __l2_pon_spec_pkt_state_set(pkt_type, state);
    }
    else 
    {
#if 0
        /* set Realtek switch chip */
        ret = __l2_sw_uni_set(pkt_type, state);
        // set management port
        ret +=__l2_mii_port_set(pkt_type, state);

        if(((pkt_type == CS_PKT_ARP) && (state == DST_CPU)) || ((pkt_type == CS_PKT_MYMAC) && (state == DST_CPU)))
        {
			cs_aal_port_id_t port;
			cs_aal_pkt_type_t pkt;
			cs_aal_spec_pkt_ctrl_msk_t  pkt_msk;
			cs_aal_spec_pkt_ctrl_t      pkt_cfg;

			memset(&pkt_msk, 0, sizeof(cs_aal_spec_pkt_ctrl_msk_t));
			memset(&pkt_cfg, 0, sizeof(cs_aal_spec_pkt_ctrl_t));

			pkt_msk.u32 = 0;
			pkt_msk.s.dpid = 1;

			pkt_cfg.dpid.dst_op = (state==DST_FE)?AAL_SPEC_DST_FE:((state==DST_CPU)?AAL_SPEC_DST_PORT: AAL_SPEC_DST_DROP);
			pkt_cfg.dpid.dpid   = AAL_PORT_ID_CPU;

			pkt = g_sdl_pkt_map[pkt_type];

			if(CS_DOWN_STREAM==direction)
			   port = AAL_PORT_ID_PON;
			else
			   port = AAL_PORT_ID_GE;
			ret = aal_special_pkt_behavior_set(port, pkt, pkt_msk, &pkt_cfg);
			if((pkt_type == CS_PKT_MYMAC) && (state == DST_CPU))
			{
				pkt_msk.u32 = 0;
				pkt_msk.s.dpid = 1;

				pkt_cfg.dpid.dst_op = AAL_SPEC_DST_PORT;
				pkt_cfg.dpid.dpid   = AAL_PORT_ID_PON;

				pkt = g_sdl_pkt_map[pkt_type];

				port = AAL_PORT_ID_CPU;
				ret = aal_special_pkt_behavior_set(port, pkt, pkt_msk, &pkt_cfg);
			}
        }
//#else
        extern GT_STATUS MinimizeCPUTraffic2(GT_QD_DEV *dev, GT_U8* macAddr);
        cs_mac_t     mac;
        (void)aal_pon_mac_addr_get(&mac);
        FOR_UNIT_START(GT_32, unit)
        MinimizeCPUTraffic2(QD_DEV_PTR, mac.addr);
        FOR_UNIT_END
#endif
        if(CS_PKT_GMP == pkt_type)
        {
        	cs_uint8 port_num = 0, per_port = 0;
        	cs_long32 unit = 0, port = 0;
            startup_config_read(CFG_ID_SWITCH_PORT_NUM, 1, &port_num);
            for(per_port = 1; per_port < port_num+1; per_port++) {
            	if(gt_getswitchunitbylport(L2P_PORT(per_port), &unit, &port) == GT_OK)
            	{
            		if(gprtSetIGMPSnoop(QD_DEV_PTR, port, (state == DST_CPU)?TRUE:FALSE) == GT_OK)
            			ret = CS_E_OK;
            	}
            }
        }
        ret = CS_E_OK;
    }
    
    if(ret != CS_E_OK)
    {
        SDL_MIN_LOG("pkt type(%d) set error. FILE: %s, LINE: %d", pkt_type, __FILE__, __LINE__);
        return CS_E_ERROR;
    }

    // record
    g_pkt_dst[direction][pkt_type] = state; 
    cs_printf("set record ret = %d ....\r\n",ret);
    if(CS_DOWN_STREAM == direction)
    {
        if(state == DST_CPU)
        {
            ret = __l2_ds_pkt_pri_set( pkt_type, g_ds_pkt_queue[pkt_type]);    
        }
        else
        {
            ret = __l2_ds_pkt_def_set( pkt_type);
        }
    }
              
    return ret;
}

cs_status epon_request_onu_spec_pkt_dst_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_direction_t            direction,
    CS_IN  cs_pkt_type_t             pkt_type,
    CS_OUT cs_sdl_pkt_dst_t          *state
)
{
    if(direction > CS_UP_STREAM)
    {
        SDL_MIN_LOG("Direction invalid.(%d) FILE: %s, LINE: %d", direction, __FILE__, __LINE__);
        return CS_E_PARAM;
    }
    
    if(pkt_type >= CS_PKT_TYPE_NUM)
    {
        SDL_MIN_LOG("pkt type invalid.(%d) FILE: %s, LINE: %d", pkt_type, __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    *state = g_pkt_dst[direction][pkt_type];
   
    return CS_E_OK;
}

cs_status epon_request_onu_pkt_cpu_queue_rate_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_sdl_queue_t            queue,
    CS_IN cs_uint32                 rate
)
{   
    cs_status     ret     = CS_E_OK;
  
    if(queue > QUEUE_3)
        return CS_E_PARAM;

    if(rate > __PKT_RATE_MAX)
        return CS_E_PARAM;

    ret =  __l2_pkt_rate_set( queue, rate);
    if(ret)
        return ret;
    
    g_queue_pkt_rate [queue] = rate;
        
    return CS_E_OK;
}

cs_status epon_request_onu_pkt_cpu_queue_rate_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_sdl_queue_t            queue,
    CS_OUT cs_uint32                 *rate    
)
{   
    
    if(queue > QUEUE_3)
        return CS_E_PARAM;
        
    if(rate == NULL)
        return CS_E_PARAM;

    *rate = g_queue_pkt_rate [queue];

    return CS_E_OK;
}

cs_status epon_request_onu_pkt_cpu_queue_map_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_direction_t            direction,
    CS_IN cs_pkt_type_t             pkt_type,
    CS_IN cs_sdl_queue_t            queue
)
{       
    cs_status     ret     = CS_E_OK;
   
     /* only support downstream */
    if(direction != CS_DOWN_STREAM)
        return CS_E_NOT_SUPPORT;

    if(pkt_type >= CS_PKT_TYPE_NUM)
        return CS_E_PARAM;
      
    if(queue > QUEUE_3)
        return CS_E_PARAM;

    if((pkt_type == CS_PKT_IP)||
        (pkt_type == CS_PKT_LOOPDETECT))
        return CS_E_NOT_SUPPORT;
        
    if(queue == g_ds_pkt_queue[pkt_type])
        return CS_E_OK;

    if(g_pkt_dst[direction][pkt_type] == DST_CPU)
    {
        ret = __l2_ds_pkt_pri_set( pkt_type, queue);
        if(ret)
            return ret;
    }

    g_ds_pkt_queue[pkt_type] = queue;
   
    return CS_E_OK;
}

cs_status epon_request_onu_pkt_cpu_queue_map_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_direction_t            direction,
    CS_IN  cs_pkt_type_t             pkt_type,
    CS_OUT cs_sdl_queue_t            *queue
)
{   
    if (NULL == queue)    
        return CS_E_PARAM;

     /* only support downstream */
    if(direction != CS_DOWN_STREAM)
        return CS_E_NOT_SUPPORT;

    if(pkt_type >= CS_PKT_TYPE_NUM)
        return CS_E_PARAM;
    
    *queue = g_ds_pkt_queue[pkt_type];
 
    return CS_E_OK;
}

static cs_int32 mrv_cpu_rx_parse(cs_uint8 *pkt, cs_uint16 in_len, cs_uint16 *out_len, cs_uint8 *s_port)
{
	cs_status ret = CS_E_ERROR;

	if(pkt && out_len && s_port)
	{
		GT_ATU_ENTRY entry;
		GT_U16 vid = 0, etype = 0;
		GT_BOOL found = GT_FALSE;

		memset(&entry, 0, sizeof(entry));

		memcpy(entry.macAddr.arEther, pkt+ETHERNET_HEADER_SIZE, GT_ETHERNET_HEADER_SIZE);

		etype = ntohs(*(GT_U16*)(pkt+12));
		vid = ntohs(*(GT_U16*)(pkt+14));

		if(etype == 0x8100)
			vid &= 0xfff;
		else
			vid = 1;

		entry.DBNum = 1;

		FOR_UNIT_START(GT_32, unit)

			if(gfdbFindAtuMacEntry(QD_DEV_PTR, &entry, &found) == GT_OK && found)
			{
				GT_U8 i = 0;
				for(i=0; i<QD_DEV_PTR->maxPorts; i++)
				{
					if(entry.portVec&(1<<i))
						break;
				}

				if(i<QD_DEV_PTR->maxPorts)
				{
					*s_port = i;
					ret = CS_E_OK;
					break;
				}
			}

		FOR_UNIT_END

		*out_len = in_len;
	}

	return ret;
}

static cs_int32 mrv_cpu_tx_parse(cs_uint8 *pkt_in, cs_uint16 in_len, cs_uint8 *pkt_out, cs_uint16 *out_len, cs_uint8 d_port)
{
	cs_status ret = CS_E_ERROR;

	if(pkt_in && pkt_out && out_len && in_len > 8)
	{
#if 0
		*out_len = in_len-8;
#else
		*out_len = in_len;
#endif
		memcpy(pkt_out, pkt_in, *out_len);
		ret = CS_E_OK;
	}

	return ret;
}

// fo fwd packet with ether type 0x8899 from reateltek switch
//#define SDL_VLAN_SW_RTK                    10 // total 12 entries

cs_status sdl_switch_init(void)
{
    cs_aal_cl_rule_cfg_t ce_entry;
    cs_aal_cl_fib_data_t fibdata;
    cs_uint16 fib_index = 0;
    cs_uint16 rule_offset = 0; //DOMAIN_ID_SWITCH has onlu one rule
    int i;
    cs_uint8    tmp_val=0;
    cs_callback_context_t  context;

    diag_printf("sdl swich chip is being initialized   ... ");

    if (startup_config_read(CFG_ID_SWITCH_CHIP_ID, 1, &tmp_val) == CS_E_OK) {
//	mtodo: mrv switch chip id set
    }

     /* attach cpu tx and rx parser */
    __ma_rx_parser_hook_reg(mrv_cpu_rx_parse);
    __ma_tx_parser_hook_reg(mrv_cpu_tx_parse);

    /*
    ** create a rule for capturing packet with ether type is 0x8899 to CPU
    ** this kind of packet comes from reateltek switch
    */
    memset(&ce_entry, 0, sizeof(cs_aal_cl_rule_cfg_t));  
    memset(&fibdata, 0, sizeof(cs_aal_cl_fib_data_t)); 

    ce_entry.valid = 1;
    ce_entry.key_type = AAL_CL_PROTO_KEY;  

    ce_entry.cl_key.proco_key[0].mask.s.ev2pt  = 1;
    ce_entry.cl_key.proco_key[0].ev2pt         = 0x8899;
     
    fibdata.mask.u32 = 0;
    fibdata.mask.s.dpid       = 1;
    fibdata.mask.s.cos        = 1;
    fibdata.mask.s.rate_limit = 1;
    fibdata.valid             = 1;
    fibdata.permit            = 1;
    fibdata.permit_filter_dis = 1;
    fibdata.permit_pri        = 1;
    fibdata.dpid.dst_op       = AAL_CL_DST_PORT;
    fibdata.dpid.dpid         = AAL_PORT_ID_CPU;
    fibdata.cos_sel           = AAL_CL_COS_SEL_CL;
    fibdata.cos               = 4; /*4,5 mapp to que 2 of CPU*/
    fibdata.flowId_sel        = AAL_CL_FLOWID_SEL_CL_RSLT; 
    fibdata.flowid            = (__FLOW_ID_PRI(2)-AAL_RATE_LIMIT_FLOW_0); 
    fibdata.cl_rate_limiter_bypass = 0; 
  
    if (aal_cl_rule_set(AAL_PORT_ID_GE, DOMAIN_ID_SWITCH, rule_offset, &ce_entry) != CS_E_OK)
        goto end;

    //fib_index = SDL_VLAN_KEY_NUM_ENTRY * rule_offset + 0;
    fib_index = 8 * rule_offset + 0;
    if (aal_cl_fib_set(AAL_PORT_ID_GE, DOMAIN_ID_SWITCH, fib_index, &fibdata) != CS_E_OK)
        goto end;
    
    /*for the throughput and scheduling issue, suggested by realtek engineers*/
    for(i = 0; i<8; i++)
    {
        cs_plat_ssp_switch_write(context,0,0,0x200+i, 0x24); 
    }
    
    cs_plat_ssp_switch_write(context,0,0,0x219, 0x18); 

   /*for TCP throughput work around, default value is 0xed*/ 
    cs_plat_ssp_switch_write(context,0,0,0x1203, 0x12); 

    diag_printf("[done]\n");
    return CS_E_OK;

end:
    diag_printf("[fail]\n");
    return CS_E_ERROR;      
}

cs_status sdl_pktctrl_init(void)
{
    cs_aal_fdb_port_cfg_t port_cfg;
    cs_aal_fdb_port_msk_t port_msk;
    cs_callback_context_t context;
    cs_aal_port_id_t      port;
    cs_aal_spec_pkt_ctrl_msk_t  sp_msk;
    cs_aal_spec_pkt_ctrl_t      sp_ctrl;
    cs_int32              i   = 0;
    cs_pkt_type_t         pkt_type;
    cs_status             ret = CS_E_OK;
    cs_uint32 queue_rate[4] = {
                            300,   /* queue 0 */
                            500,   /* queue 1 */
                            600,   /* queue 2 */
                            100    /* queue 3 */
    };
    /******Downstream OAM,802.1X packet will be mapped to queue3 ****/
    cs_sdl_queue_t downstream_pkt[CS_PKT_TYPE_NUM] = {
                            QUEUE_0,   /** CS_PKT_BPDU       */     
                            QUEUE_3,   /** CS_PKT_8021X      */    
                            QUEUE_2,   /** CS_PKT_GMP        */    
                            QUEUE_0,   /** CS_PKT_ARP        */    
                            QUEUE_3,   /** CS_PKT_OAM        */    
                            QUEUE_2,   /** CS_PKT_MPCP       */    
                            QUEUE_1,   /** CS_PKT_DHCP       */    
                            QUEUE_0,   /** CS_PKT_IROS       */    
                            QUEUE_1,   /** CS_PKT_PPPOE      */    
                            QUEUE_0,   /** CS_PKT_IP         */    
                            QUEUE_0,   /** CS_PKT_NDP        */    
                            QUEUE_0,   /** CS_PKT_LOOPDETECT */    
                            QUEUE_0    /** CS_PKT_MYMAC      */    
    };
    
    diag_printf("sdl l2 is being initialized   ... ");

    /* Reduce the priority of the MYMAC packet*/
    /*   27 to 2 */
    memset(&sp_ctrl, 0, sizeof(cs_aal_spec_pkt_ctrl_t));
    sp_msk.u32     = 0;
    sp_msk.s.pri   = 1;
    sp_ctrl.pri    = 2;
    
    for (port = AAL_PORT_ID_GE; port <= AAL_PORT_ID_MII1; ++port) {
        ret = aal_special_pkt_behavior_set(port, AAL_PKT_MYMAC, sp_msk, &sp_ctrl);
        if(ret)
            return ret;
    }
    
    memset(&g_ds_pkt_queue, 0, sizeof(g_ds_pkt_queue));
    memset(&g_queue_pkt_rate, 0, sizeof(g_queue_pkt_rate));

    for (i = 0; i <= QUEUE_3; i++) 
    {
        ret = epon_request_onu_pkt_cpu_queue_rate_set( context, 0, 0, i, queue_rate[i]);
        if(ret)
            return ret;
    }

    for (pkt_type = CS_PKT_BPDU; pkt_type < CS_PKT_TYPE_NUM; pkt_type++) 
    {     
        if((pkt_type == CS_PKT_IP)||(pkt_type == CS_PKT_LOOPDETECT))
            continue;
        
        ret = epon_request_onu_pkt_cpu_queue_map_set( context, 0, 0, CS_DOWN_STREAM, 
                                                      pkt_type, 
                                                      downstream_pkt[pkt_type]);
        if(ret)
            return ret;   
    }
    
    /* enable learning function for frames destinated to CPU */
    memset(&port_cfg, 0, sizeof(port_cfg));
    memset(&port_msk, 0, sizeof(port_msk));

    port_msk.s.learn_dpid_cpu_dis = 1;
    port_cfg.learn_dpid_cpu_dis   = 0;
    (void)aal_fdb_port_cfg_set(AAL_PORT_ID_GE, port_msk, &port_cfg);
    
    g_stp_blockport_status = 0;
     
    diag_printf("[done]\n");
    return CS_E_OK;
}

