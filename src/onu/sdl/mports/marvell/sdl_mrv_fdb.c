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
 
Copyright (c) 2010 by Cortina Systems Incorporated
****************************************************************************/

#include "sdl_fdb.h"
#include "sdl_mac_filterbind_util.h"
#include "aal_l2.h"
#include "sdl_init.h"
#include "sdl_init.h"
#include "sdl.h"

#include "MARVELL_BSP_expo.h"
#include "switch_expo.h"

/*Private Macro Definition*/
#define __FDB_ENTRY_MAX                                           (sdl_int_cfg.max_fdb_entry)

#define __FDB_STATIC_ENTRY_MAC_MAX      (16)
#define __FDB_ENTRY_HW_INDEX_MAX        (2112)
#define __FDB_AGING_TIME_MAX            (458)
#define __FDB_AGING_TIME_DEFAULT        (300)
#define __FDB_MAC_LIMIT_MAX             (255)


extern sdl_init_cfg_t sdl_int_cfg;

sdl_fdb_port_cfg_t g_fdb_port_cfg[UNI_PORT_MAX];
cs_uint32   g_fdb_aging_time = 0;

typedef struct{
    cs_sdl_fdb_entry_t    entry[__FDB_STATIC_ENTRY_MAC_MAX];
    cs_uint16             valid_map;
} __fdb_static_entry_t ;

__fdb_static_entry_t  __fdb_static_entry_table[UNI_PORT_MAX];      

#if 0
static GT_U32 getDbnum(GT_QD_DEV * dev)
{
	GT_U32 maxdb = 0;

	if(dev)
	{

		if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_FULL))
			maxdb = 16;
		else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
			maxdb = 64;
		else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
			maxdb = 256;
		else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_4096))
			maxdb = 4096;
		else
			maxdb = 1;
	}

	return maxdb;
}
#endif

static GT_U8 getlportfromucportvec(GT_QD_DEV *dev, GT_U16 portvec)
{
	GT_U8 port;

	for(port=0; port<dev->maxPorts; port++)
	{
		if(portvec & (1<<port))
			break;
	}

	return port;
}

/* Check if it is a valid Unicast mac addr 
 * CS_E_OK  - valid
 * CS_E_ERROR - invalid
 */
static cs_status __fdb_check_mac(cs_mac_t *mac)
{
    cs_uint8 zero_mac[6] = {0, 0, 0, 0, 0, 0};

    /* if all '0', invalid */
    if(!memcmp(mac, zero_mac, sizeof(cs_mac_t))) {
        SDL_MIN_LOG("The MAC address is empty!\n");
        return CS_E_ERROR;
    }

    /* if multi-cast, invalid */
    if(mac->addr[0] & 0x1) {
        SDL_MIN_LOG("The MAC address is multi-cast address!\n");
        return CS_E_ERROR;
    }
    
    return CS_E_OK;
}

/* workaround for BUG 29184,make 802.1x packet to be known MC packet*/
static cs_status __fdb_8021x_static_mac(void)
{
    GT_STATUS gt_ret = GT_OK;
    GT_ATU_ENTRY entry;
    GT_LPORT port;

    entry.macAddr.arEther[0]= 0x01;
    entry.macAddr.arEther[1]= 0x80;
    entry.macAddr.arEther[2]= 0xc2;
    entry.macAddr.arEther[3]= 0x00;
    entry.macAddr.arEther[4]= 0x00;
    entry.macAddr.arEther[5]= 0x03;

    entry.entryState.mcEntryState = GT_MC_STATIC;

    entry.portVec = 0x100;
  
           /* set to HW */
    FOR_UNIT_START(GT_U32, unit)
		if(get_switch_wan_port_info(unit, &port) == GT_OK)
			entry.portVec = 1<<port;
		else
			continue;
//		gt_ret = gfdbAddMacEntry(QD_DEV_PTR, &entry);
		if(GT_OK != gt_ret)
		{
			SDL_MIN_LOG("gfdbAddMacEntry return %d. FILE: %s, LINE: %d", gt_ret, __FILE__, __LINE__);
			return CS_E_ERROR;
		}
    FOR_UNIT_END

    return CS_E_OK;
}

static void __onu_fdb_entry_clr_uplink_port (
    CS_IN   cs_callback_context_t   context,
    CS_IN   cs_int32                device_id,
    CS_IN   cs_int32                llidport,
    CS_IN   cs_sdl_fdb_clr_mode_t   clr_mode
)
{
	GT_UINT flag = 0;
	GT_STATUS ret = GT_OK;
	GT_ATU_ENTRY entry, e1;

    /* Only flush dynamic entries on the Realtek service port */
    
    if(clr_mode == SDL_FDB_CLR_STATIC){
        SDL_MIN_LOG("Do not support service port static MAC adrress flush \n");
        return;
    }
    
    FOR_UNIT_START(GT_32, unit)
    ret = gfdbGetAtuEntryFirst(QD_DEV_PTR, &entry);
    while(ret == GT_OK){

    	if(entry.entryState.ucEntryState == GT_UC_DYNAMIC &&
    			(entry.portVec & (1<<(L2P_PORT(CS_UPLINK_PORT)))))
    	{
    		flag = 1;
    		e1 = entry;
    		ret = gfdbGetAtuEntryNext(QD_DEV_PTR,&e1);

    		if(ret == GT_OK)
    		{
    			ret = gfdbDelAtuEntry(QD_DEV_PTR, &entry);
    			if(ret != GT_OK)
    			SDL_MIN_LOG("gfdbDelAtuEntry return %d.\n", ret);
    		}
    		else
    			continue;
    	}

    	if(flag)
    		entry = e1;
    	else
    	{
    		ret = gfdbGetAtuEntryNext(QD_DEV_PTR, &entry);
    		if(ret != GT_OK)
    			SDL_MIN_LOG("%s call gfdbGetAtuEntryNext fail\r\n", __func__);
    	}
    }
    FOR_UNIT_END
    
    return;    
}

cs_status fdb_static_entry_show(void)
{
    cs_uint8 i, j;

    cs_printf("\n\n-----------SW Static FDB Table---------------\n\n");
    for(i=0; i<UNI_PORT_MAX; i++){
        cs_uint8 index = 0;
        cs_printf("PortID: %d---\n", i+1);
        for(j=0; j<__FDB_STATIC_ENTRY_MAC_MAX; j++)
        {
            if(__fdb_static_entry_table[i].valid_map & (1<<j)){
                cs_printf("index : %d\n", ++index);
                cs_printf("vlan  : %d\n", __fdb_static_entry_table[i].entry[j].vlan_id);
                cs_printf("mac   : %02x:%02x:%02x:%02x:%02x:%02x\n",
                        __fdb_static_entry_table[i].entry[j].mac.addr[0],
                        __fdb_static_entry_table[i].entry[j].mac.addr[1],
                        __fdb_static_entry_table[i].entry[j].mac.addr[2],
                        __fdb_static_entry_table[i].entry[j].mac.addr[3],
                        __fdb_static_entry_table[i].entry[j].mac.addr[4],
                        __fdb_static_entry_table[i].entry[j].mac.addr[5]);
            }
        }
        cs_printf("\n");
    }
    
    return CS_E_OK;
}

cs_status fdb_static_entry_get(cs_uint32 port,cs_uint32 vlanid)
{
    cs_uint8  j;
	cs_uint8 index = 0;
    cs_printf("\n\n-----------SW Static FDB Table---------------\n\n");
 
        cs_printf("PortID: %d---\n",port);
        for(j=0; j<__FDB_STATIC_ENTRY_MAC_MAX; j++)
        {
            if(__fdb_static_entry_table[port -1].valid_map & (1<<j))
			{
				if(__fdb_static_entry_table[port-1].entry[j].vlan_id == vlanid)
					{
						cs_printf("index : %d\n", ++index);
               			cs_printf("vlan  : %d\n", __fdb_static_entry_table[port-1].entry[j].vlan_id);
                	    cs_printf("mac   : %02x:%02x:%02x:%02x:%02x:%02x\n",
                        __fdb_static_entry_table[port-1].entry[j].mac.addr[0],
                        __fdb_static_entry_table[port-1].entry[j].mac.addr[1],
                        __fdb_static_entry_table[port-1].entry[j].mac.addr[2],
                        __fdb_static_entry_table[port-1].entry[j].mac.addr[3],
                        __fdb_static_entry_table[port-1].entry[j].mac.addr[4],
                        __fdb_static_entry_table[port-1].entry[j].mac.addr[5]);
					}

            }
        }
        cs_printf("\n");
    
    return CS_E_OK;
}

/*check entry is added per ports*/
static cs_boolean fdb_static_entry_find(cs_port_id_t port_id, cs_mac_t *addr, cs_uint8 *index)
{
    cs_uint8 i;
    
    *index = __FDB_STATIC_ENTRY_MAC_MAX;
    
    for(i=0; i<__FDB_STATIC_ENTRY_MAC_MAX; i++){
        if((__fdb_static_entry_table[port_id].valid_map & (1<<i))
            && (!memcmp(__fdb_static_entry_table[port_id].entry[i].mac.addr, addr, 6))){
            *index = i;
            return EPON_TRUE;
        }
    }
    
    return EPON_FALSE;
}

/*check entry is added globaly*/
#if 1
static cs_boolean fdb_static_entry_check(cs_mac_t *addr,GT_LPORT port_id)
{
	cs_uint8 index;
    if(fdb_static_entry_find(port_id, addr, &index))
    	{
			return EPON_TRUE;
    	}
    return EPON_FALSE;
}

static cs_status fdb_static_entry_idx_get(cs_port_id_t port_id, cs_uint8 *index)
{
    cs_uint8 i;
    
    *index = __FDB_STATIC_ENTRY_MAC_MAX;
    
    for(i=0; i<__FDB_STATIC_ENTRY_MAC_MAX; i++){
        if(!(__fdb_static_entry_table[port_id].valid_map & (1<<i))){
            *index = i;
            return CS_E_OK;
        }
    }
    
    return CS_E_NOT_FOUND;
}
#endif
cs_status epon_request_onu_fdb_full_behavior_set (
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_IN cs_sdl_fdb_full_behavior_t    full_behavior
)
{
    return CS_E_NOT_SUPPORT;
}

cs_status epon_request_onu_fdb_full_behavior_get (
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_OUT cs_sdl_fdb_full_behavior_t   *full_behavior
)
{
    return CS_E_NOT_SUPPORT;
}

cs_status epon_request_onu_fdb_learn_mode_set (
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_IN cs_sdl_fdb_learn_mode_t       lrn_mode
)
{
    return CS_E_NOT_SUPPORT;
}


cs_status epon_request_onu_fdb_learn_mode_get (
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_OUT cs_sdl_fdb_learn_mode_t      *lrn_mode
)
{
    *lrn_mode = SDL_FDB_SHARED_VLAN_LEARNING;
    
    return CS_E_OK;
}

cs_status epon_request_onu_fdb_mac_limit_set (
    CS_IN cs_callback_context_t         context,
    CS_IN cs_int32                      device_id,
    CS_IN cs_int32                      llidport,
    CS_IN cs_port_id_t                  port_id,
    CS_IN cs_uint32                     mac_num
)
{
    cs_uint32               static_mac_num;
    cs_uint32               dynamic_limit = 0;
    GT_STATUS           	gt_ret = 0;
    cs_status               rt = CS_E_OK;
    GT_32 					unit, port;

    UNI_PORT_CHECK(port_id);
    
    if((mac_num>__FDB_MAC_LIMIT_MAX) && (mac_num != 0xffff)){
        diag_printf("mac_num %d is invalid\n", mac_num);
        rt = CS_E_PARAM;
        goto END;
    }

    static_mac_num = g_fdb_port_cfg[port_id-1].static_mac_num;
    
    /*mac_num = 0xffff: Learning and pass all*/
    if(0xffff==mac_num){
        /*Set limit number > lookup table number(eg,2113),
          learn over action is meanless, ie,action is forward.                                                               作用的, 意即action是forward.   
        */
    	if(GT_OK == gt_getswitchunitbylport(port_id-1, &unit, &port))
    	{
			gt_ret = gfdbSetPortAtuLearnLimit(QD_DEV_PTR, port, 0xff);
			if(GT_OK != gt_ret){
				diag_printf("gfdbSetPortAtuLearnLimit return %d\n", gt_ret);
				rt = CS_E_ERROR;
				goto END;
			}
    	}
    }
    else{ /*mac_num = (0-0xff)*/
        if (sdl_int_cfg.fdb_limit_include_static) {
            if (static_mac_num > mac_num) {
                diag_printf("static entry number %d is larger than dynamic limit %d\n", static_mac_num, mac_num);
                rt = CS_E_PARAM;
                goto END;
            } else {
                dynamic_limit = mac_num - static_mac_num; /* '0' is valid */
            }
        } 
        else {
            dynamic_limit = mac_num;
        }

        if((dynamic_limit + static_mac_num) > sdl_int_cfg.max_fdb_entry) {
            diag_printf("mac_num exceed the max FDB entry!\n");
            rt = CS_E_PARAM;
            goto END;
        }
    
        gt_ret = gfdbSetPortAtuLearnLimit(QD_DEV_PTR, port_id-1, dynamic_limit);
        if(GT_OK != gt_ret) {
            diag_printf("gfdbSetPortAtuLearnLimit return %d\n", gt_ret);
            rt = CS_E_ERROR;
            goto END;
        }
    }
    
    /* then, clear all dynamic entires for this port */
    rt = epon_request_onu_fdb_entry_clr_per_port(context, device_id, llidport, port_id, SDL_FDB_CLR_DYNAMIC);
    if (rt) {
        SDL_MIN_LOG("epon_request_onu_fdb_entry_clr_per_port return %d!\n", rt);
        goto END;
    }
        
    g_fdb_port_cfg[port_id-1].mac_limit = mac_num;

END:
    return rt;
}



cs_status epon_request_onu_fdb_mac_limit_get (
    CS_IN  cs_callback_context_t         context,
    CS_IN  cs_int32                      device_id,
    CS_IN  cs_int32                      llidport,
    CS_IN  cs_port_id_t                  port_id,
    CS_OUT cs_uint32                     *mac_num
)
{
    if(NULL==mac_num) {
        SDL_MIN_LOG("mac_num is NULL pointer\n");
        return CS_E_PARAM;
    }

    UNI_PORT_CHECK(port_id);
        
    *mac_num = g_fdb_port_cfg[port_id-1].mac_limit;
    
    return CS_E_OK;
}

cs_status epon_request_onu_fdb_age_set (
    CS_IN cs_callback_context_t          context,
    CS_IN cs_int32                       device_id,
    CS_IN cs_int32                       llidport,
    CS_IN cs_uint32                      aging_time
)
{
    GT_STATUS  ret = 0;
    cs_status       rt = CS_E_OK;

    if (aging_time > sdl_int_cfg.fdb_aging_max_time) {
        SDL_MIN_LOG("aging_time %d is invalid\n", aging_time);
        rt = CS_E_PARAM;
        goto END;
    }
    
    if(g_fdb_aging_time == aging_time){
       SDL_MIN_LOG("aging_time %d is same as before\n", aging_time);
       rt = CS_E_OK;
       goto END;
    }

    /*set aging time*/
    FOR_UNIT_START(GT_32, unit)
    ret = gfdbSetAgingTimeout(QD_DEV_PTR, aging_time);
    if (GT_OK != ret) {
        SDL_MIN_LOG("gfdbSetAgingTimeout return %d!\n", ret);
        rt = CS_E_ERROR;
        goto END;
    }
    
    FOR_UNIT_END

    g_fdb_aging_time = aging_time;
     
END:
    return rt;
    
}

cs_status epon_request_onu_fdb_age_get (
    CS_IN  cs_callback_context_t                   context,
    CS_IN  cs_int32                                device_id,
    CS_IN  cs_int32                                llidport,
    CS_OUT cs_uint32                               *aging_time
)
{    
    if(NULL==aging_time) {
        SDL_MIN_LOG("aging_time is NULL pointer\n");
        return CS_E_PARAM;
    }
   
    *aging_time = g_fdb_aging_time;
    
    return CS_E_OK;
    
}

cs_status epon_request_onu_mac_learn_set(
    CS_IN cs_callback_context_t   context,
    CS_IN cs_int32                device_id,
    CS_IN cs_int32                llidport,
    CS_IN cs_port_id_t            port_id,
    CS_IN cs_sdl_fdb_mac_learn_t  status
)
{
#if 1
    GT_STATUS   gt_ret  = 0;
    cs_status rt = CS_E_OK;
    
    UNI_PORT_CHECK(port_id);


    if (status > SDL_FDB_MAC_LEARN_ENABLE) {
        SDL_MIN_LOG("In %s, status %d is not supported\n", __FUNCTION__, status);
        rt = CS_E_PARAM;
        goto END;
    }
    
    if (status == SDL_FDB_MAC_LEARN_DISABLE) {
        /*MAC limit counter is set to 0*/
    	GT_32 unit, port;
    	if(GT_OK == gt_getswitchunitbylport(port_id-1, &unit, &port))
    	{
    		gt_ret = gprtSetLearnDisable(QD_DEV_PTR, port_id-1, GT_TRUE);
    		if(GT_OK != gt_ret){
    			SDL_MIN_LOG("In function:%s,line:%d invoke gprtSetLearnDisable fail!\n",__FUNCTION__, __LINE__);
    			rt = CS_E_ERROR;
    			goto END;
    		}
    	}
        
        /*when disable mac-learn,should clear all dynamic entires */
        rt = epon_request_onu_fdb_entry_clr_per_port(context, device_id, llidport, port_id,SDL_FDB_CLR_DYNAMIC);
        if(rt){
            SDL_MIN_LOG("In function:%s,line:%d invoke epon_request_onu_uc_l2_entry_clr fail!\n",__FUNCTION__, __LINE__);
            goto END;
        }
        
        g_fdb_port_cfg[port_id-1].lrn_en = SDL_FDB_MAC_LEARN_DISABLE;
        
    }
    else{
        /*learn over action is droping*/
    	GT_32 unit, port;

    	if(GT_OK == gt_getswitchunitbylport(port_id-1, &unit, &port))
    	{
			gt_ret = gprtSetLearnDisable(QD_DEV_PTR, port, GT_FALSE);
			if(GT_OK != gt_ret){
				SDL_MIN_LOG("In function:%s,line:%d invoke gprtSetLearnDisable fail!\n",__FUNCTION__, __LINE__);
				rt = CS_E_ERROR;
				goto END;
			}
    	}

        g_fdb_port_cfg[port_id-1].lrn_en = SDL_FDB_MAC_LEARN_ENABLE;
        
#if 0
        /*when enable mac learn, mac limit should take effect*/
        rt = epon_request_onu_fdb_mac_limit_set(context, device_id, llidport, port_id, g_fdb_port_cfg[port_id-1].mac_limit);
        if(rt){
            SDL_MIN_LOG("In function:%s,line:%d invoke epon_request_onu_uc_l2_entry_clr fail!\n",__FUNCTION__, __LINE__);
            goto END;

        }
#endif
    }
    
END:
    return rt;
#endif
    return CS_E_NOT_SUPPORT;
    
}

cs_status epon_request_onu_mac_learn_get(
    CS_IN  cs_callback_context_t   context,
    CS_IN  cs_int32                device_id,
    CS_IN  cs_int32                llidport,
    CS_IN  cs_port_id_t            port_id,
    CS_OUT cs_sdl_fdb_mac_learn_t  *status
)
{

    *status = SDL_FDB_MAC_LEARN_ENABLE;
    
    return CS_E_OK;


}


cs_status epon_request_onu_fdb_entry_clr (
    CS_IN   cs_callback_context_t   context,
    CS_IN   cs_int32                device_id,
    CS_IN   cs_int32                llidport,
    CS_IN   cs_sdl_fdb_clr_mode_t   clr_mode
)
{
    cs_port_id_t portid;
    cs_status rt = CS_E_OK;

    /* Uni port MAC flush */
    for(portid=CS_UNI_PORT_ID1; portid<=UNI_PORT_MAX; portid++){
        rt =  epon_request_onu_fdb_entry_clr_per_port(context, device_id, llidport, portid, clr_mode);
        if(rt){
            SDL_MIN_LOG("epon_request_onu_fdb_entry_clr_per_port return %d\n", rt);
        }
    }

    /* Service port MAC flush */
    __onu_fdb_entry_clr_uplink_port(context, device_id, llidport, clr_mode);
    
    return rt;
    
}

cs_status epon_request_onu_fdb_entry_clr_per_port (
    CS_IN   cs_callback_context_t   context,
    CS_IN   cs_int32                device_id,
    CS_IN   cs_int32                llidport,
    CS_IN   cs_port_id_t            portid,
    CS_IN   cs_sdl_fdb_clr_mode_t   clr_mode
)
{
#if 0
    GT_ATU_ENTRY l2_data, l2_t;
    GT_STATUS rtn  = GT_OK;
    cs_boolean rt = FALSE;
    cs_uint8 index, fdel = 0;
#endif
    GT_32 	unit,port;
#if 0
    GT_U32  maxdb = 0, i;
#endif

    UNI_PORT_CHECK(portid);

    if(clr_mode > SDL_FDB_CLR_STATIC){
        SDL_MIN_LOG("invalid clr_mode (%d).\n", clr_mode);
        return CS_E_ERROR;
    }
    
    if(GT_OK != gt_getswitchunitbylport(portid-1, &unit, &port))
    	return CS_ERROR;

#if 0

    maxdb = getDbnum(QD_DEV_PTR);

    if(maxdb == 0)
    	return CS_ERROR;

    for(i = 0; i<maxdb; i++)
    {
    	cs_printf("clr dbnum %d\n", i);

    	memset(&l2_data, 0, sizeof(l2_data));

    	l2_data.DBNum = i;
		rtn = gfdbGetAtuEntryFirst(QD_DEV_PTR, &l2_data);

		if(rtn == GT_OK)
			cs_printf("found first for dbnum %d\n", i);

		while(rtn == GT_OK){

			switch(clr_mode)
			{
				case SDL_FDB_CLR_DYNAMIC:
					 if( l2_data.entryState.ucEntryState == GT_UC_DYNAMIC &&
							 (l2_data.portVec & (1<<port)))
					 {
						 fdel = 1;
					 }
					 break;

				 case SDL_FDB_CLR_STATIC:
					if(l2_data.entryState.ucEntryState == GT_UC_STATIC &&
							(l2_data.portVec & (1<<port)) )
					{
						/* do not delete the MAC address that is not in software static MAC list */
						rt = fdb_static_entry_find(portid-1, (cs_mac_t*)&l2_data.macAddr.arEther[0], &index);
						if(rt){
							/* Clear the corresponding static mac entry table */
							memset(&__fdb_static_entry_table[portid-1].entry[index], 0, sizeof(cs_sdl_fdb_entry_t));
							__fdb_static_entry_table[portid - 1].valid_map &= ~((0x1)<<index);
							g_fdb_port_cfg[portid-1].static_mac_num--;

							fdel = 1;
						}
					}
					break;

					case SDL_FDB_CLR_BOTH:
						if(l2_data.entryState.ucEntryState == GT_UC_STATIC &&
								(l2_data.portVec & (1<<port)))
						{
							/* do not delete the MAC address that is not in software static MAC list */
							rt = fdb_static_entry_find(portid -1, (cs_mac_t*)&l2_data.macAddr.arEther[0], &index);
							if(rt){
							   /* Clear the corresponding static mac entry table */
								memset(&__fdb_static_entry_table[portid-1].entry[index], 0, sizeof(cs_sdl_fdb_entry_t));
								__fdb_static_entry_table[portid - 1].valid_map &= ~((0x1)<<index);
								g_fdb_port_cfg[portid-1].static_mac_num--;

								fdel = 1;
							}
						}
						else if( l2_data.entryState.ucEntryState == GT_UC_DYNAMIC &&
							 (l2_data.portVec &(1<<port)))
						{
							fdel = 1;
						}
						break;

					default:
						/* will not come here logically */
						break;
			  }

			l2_t = l2_data;
			rtn = gfdbGetAtuEntryNext(QD_DEV_PTR, &l2_t);

			cs_printf("gfdbGetAtuEntryNext ret (%d)\n", rtn);

			if(fdel)
			{
				fdel = 0;
				gfdbDelAtuEntry(QD_DEV_PTR, &l2_data);
				cs_printf("del %02x:%02x:%02x:%02x:%02x:%02x\n",
						l2_data.macAddr.arEther[0],
						l2_data.macAddr.arEther[1],
						l2_data.macAddr.arEther[2],
						l2_data.macAddr.arEther[3],
						l2_data.macAddr.arEther[4],
						l2_data.macAddr.arEther[5]);
			}

			l2_data = l2_t;
		}
    }

#else
    if(clr_mode == SDL_FDB_CLR_DYNAMIC)
    {
    	if(gfdbRemovePort(QD_DEV_PTR, GT_MOVE_ALL_UNLOCKED, port) != GT_OK)
    		return CS_E_ERROR;
    }
    else
    {
    	if(gfdbRemovePort(QD_DEV_PTR, GT_MOVE_ALL, port) != GT_OK)
    		return CS_E_ERROR;
    }
#endif

    /*For security reason, double check the SW Static table*/
    if((SDL_FDB_CLR_STATIC == clr_mode) || (SDL_FDB_CLR_BOTH == clr_mode)){
    
        if(__fdb_static_entry_table[portid-1].valid_map)
        {
            memset(&__fdb_static_entry_table[portid-1].entry[0], 0, __FDB_STATIC_ENTRY_MAC_MAX*sizeof(cs_sdl_fdb_entry_t));
            __fdb_static_entry_table[portid-1].valid_map = 0;

            SDL_MIN_LOG("Software static MAC address table not synchronized with hardware\n");
        }

        /* Clear the static MAC address number */
        g_fdb_port_cfg[portid-1].static_mac_num = 0;

#if 0
//        mtodo: added mac limit set call
        /*After setting, re-config the mac limit,Dynamic entries will be cleared there */
        
        if(sdl_int_cfg.fdb_limit_include_static){
            rt = epon_request_onu_fdb_mac_limit_set(context, 
                                                    device_id,
                                                    llidport, 
                                                     portid, 
                                                 g_fdb_port_cfg[portid -1].mac_limit);
            if(rt){
                SDL_MIN_LOG("epon_request_onu_fdb_mac_limit_set return %d!\n", rt);
                return rt;
            }
        }
#endif
    }
    
    return CS_E_OK;    
}

cs_status epon_request_onu_fdb_entry_add(
    CS_IN  cs_callback_context_t         context,
    CS_IN  cs_int32                      device_id,
    CS_IN  cs_int32                      llidport,
    CS_IN  cs_sdl_fdb_entry_t            *entry
)
{
    GT_LPORT         portid;
    GT_ATU_ENTRY l2_data;
    cs_uint8           index;
    GT_STATUS      gt_ret = 0;
    cs_status          rt = CS_E_OK;

    GT_32	unit, port;

    if(NULL==entry){
        diag_printf("param is NULL pointer\n");
        return CS_E_PARAM;
    }
    
    UNI_PORT_CHECK(entry->port);
    VID_CHECK(entry->vlan_id);
    
    if(entry->type != SDL_FDB_ENTRY_STATIC){
        diag_printf("Entry type is wrong!\n");
        return CS_E_NOT_SUPPORT;
    }
    
    /* Check if it is a valid Unicast mac addr */
    rt = __fdb_check_mac(&entry->mac);
    if(rt){
       diag_printf("In %s(%d) error MAC!\n", __FUNCTION__, __LINE__);
        return CS_E_PARAM;
    }
    
    /*Check whether it is already added*/
    portid = entry->port-1;
    rt = fdb_static_entry_check(&entry->mac,portid);
    if(rt){
        diag_printf("ERROR: Static mac entry has already been added!\n");
        return CS_E_PARAM;
    }
    
    rt = fdb_static_entry_idx_get(portid, &index);
    if(rt){
        diag_printf("ERROR: SW Static entry table is full!\n");
        return CS_E_RESOURCE;
    }
    
    if(g_fdb_port_cfg[portid].static_mac_num >= sdl_int_cfg.max_static_mac_entry){
        diag_printf("ERROR: Static mac entry exceed! max num %d\n", sdl_int_cfg.max_static_mac_entry);
        return CS_E_PARAM;
    }
        
    /* if static MAC number is larger than MAC limit number, 
     * ONU just accept or keep the front static entries, 
     * number is equal to MAC limit number. 
     * In this case, there will be no dynamic entry available.
     */   
    if(sdl_int_cfg.fdb_limit_include_static && 
        (g_fdb_port_cfg[portid].static_mac_num >= g_fdb_port_cfg[portid].mac_limit)){
        diag_printf("WARNING: Static table exceeds mac limit %d now\n"
                    "Remaining entries will be ignored\n",
                    g_fdb_port_cfg[portid].mac_limit);
        return CS_E_PARAM;
    }
    
    if(GT_OK != gt_getswitchunitbylport(portid, &unit, &port ))
    	return CS_E_ERROR;

    memset(&l2_data, 0, sizeof(GT_ATU_ENTRY));
    memcpy(&l2_data.macAddr.arEther[0], &entry->mac, sizeof(cs_mac_t));
    l2_data.entryState.ucEntryState = (entry->type == SDL_FDB_ENTRY_STATIC)?GT_UC_STATIC:GT_UC_DYNAMIC;
    l2_data.DBNum = entry->vlan_id;
    l2_data.portVec |= 1<<(L2P_PORT(port));
    
    gt_ret = gfdbAddMacEntry(QD_DEV_PTR, &l2_data);
    if(GT_OK != gt_ret){
        diag_printf("gfdbAddMacEntry return %d!\n", gt_ret);
        rt = CS_E_ERROR;
        goto END;
    }
    
    memcpy(&__fdb_static_entry_table[portid].entry[index], entry, sizeof(cs_sdl_fdb_entry_t));
    __fdb_static_entry_table[portid].valid_map |= (0x1)<<index;
    
    g_fdb_port_cfg[portid].static_mac_num = g_fdb_port_cfg[portid].static_mac_num + 1;
    
    /*After setting, re-config the mac limit,Dynamic entries will be cleared there */
    
    if(sdl_int_cfg.fdb_limit_include_static){
        rt = epon_request_onu_fdb_mac_limit_set(context, 
                                                device_id,
                                                llidport, 
                                                entry->port, 
                                                g_fdb_port_cfg[portid].mac_limit);
        if(rt){
            diag_printf("epon_request_onu_fdb_mac_limit_set return %d\n", rt);
            goto END;
        }
    }
    
END:
    return rt;
}
cs_status epon_request_onu_fdb_entry_del_dynamic (
    CS_IN   cs_callback_context_t   context,
    CS_IN   cs_int32                device_id,
    CS_IN   cs_int32                llidport,
    CS_IN  cs_mac_t                      *mac,
    CS_IN  cs_uint16                     vlan
)
{
    cs_aal_fdb_entry_t fdb;
    cs_uint8           index = 0;
    cs_status          rt = CS_E_OK;

    for (index = 0; index < __FDB_ENTRY_MAX; index++) {
        memset(&fdb, 0, sizeof(cs_aal_fdb_entry_t));
        if (aal_fdb_entry_get(index, &fdb)) {
            return CS_E_ERROR;
        }

        if(fdb.static_flag == 1)  {
            continue;
        }

        if(fdb.vid != vlan)
        {
        	continue;
        }

        if(memcmp(&fdb.mac, mac, sizeof(cs_mac_t)))
        		continue;

        memset(&fdb, 0, sizeof(fdb));
        fdb.vld = 1;

        rt = aal_fdb_entry_set(index, &fdb);
        if (rt) {
        	SDL_MIN_LOG("return value: %d, invoke aal_fdb_entry_set fail,!\n", rt);
            return rt;
        }
    }

    if(rt != CS_E_OK)
    	SDL_MIN_LOG("clear dynamic fdb entry fail!\r\n");
    return CS_E_OK;

}

cs_status epon_request_onu_fdb_entry_del(
    CS_IN  cs_callback_context_t         context,
    CS_IN  cs_int32                      device_id,
    CS_IN  cs_int32                      llidport,
    CS_IN  cs_mac_t                      *mac,
    CS_IN  cs_uint16                     vlan
)
{
    GT_ATU_ENTRY l2_data;
    cs_port_id_t       portid;
    cs_uint8           index;
    GT_STATUS      gt_ret = 0;
    GT_BOOL           found = GT_FALSE;
    cs_status          rt = CS_E_OK;
    GT_LPORT        port = 0;

    if((NULL==mac)) {
        SDL_MIN_LOG("In %s(%d) error params!\n");
        return CS_E_PARAM;
    }
    
    VID_CHECK(vlan);

    /* Check if it is a valid Unicast mac addr */
    if(__fdb_check_mac(mac)){
        SDL_MIN_LOG("error MAC!\n");
        return CS_E_PARAM;
    }
    

    memset(&l2_data, 0, sizeof(GT_ATU_ENTRY));
    memcpy(&l2_data.macAddr.arEther[0], mac, sizeof(cs_mac_t));
    l2_data.DBNum = vlan;

    FOR_UNIT_START(GT_U32, unit)

    /*Firstly get the portid*/
    gt_ret = gfdbFindAtuMacEntry(QD_DEV_PTR, &l2_data, &found);
    if(GT_OK != gt_ret || found == GT_FALSE){
        SDL_MIN_LOG("gfdbFindAtuMacEntry return %d\n", gt_ret);
        continue;
    }
    
    for(port = 0; port < UNI_PORT_MAX; port++)
    {
    	GT_U32 lunit = 0, lport=0;
    	gt_getswitchunitbylport(port, &lunit, &lport );

    	if(lunit != unit)
    		continue;

    	portid = lport2port(l2_data.portVec, lport);

    	if(portid == GT_INVALID_PORT)
    		continue;

		if(!fdb_static_entry_find(port, mac, &index)){
			SDL_MIN_LOG("ERROR: Static mac entry is not found!\n");
			return CS_E_OK;
		}

		memset(&__fdb_static_entry_table[port].entry[index], 0, sizeof(cs_sdl_fdb_entry_t));
		__fdb_static_entry_table[port].valid_map &= ~((0x1)<<index);

		g_fdb_port_cfg[port].static_mac_num = g_fdb_port_cfg[port].static_mac_num - 1;

		/* after setting, re-config the mac limit Dynamic entries will be cleared there */
		if(sdl_int_cfg.fdb_limit_include_static){
			rt = epon_request_onu_fdb_mac_limit_set(context,
				device_id, llidport, port+1, g_fdb_port_cfg[port].mac_limit);
			if(rt){
				SDL_MIN_LOG("In function:%s,line:%d invoke epon_request_onu_fdb_mac_limit_set fail!\n",__FUNCTION__, __LINE__);
				goto END;
			}
		}
    
    }

	gt_ret = gfdbDelAtuEntry(QD_DEV_PTR, &l2_data);
	if(GT_OK != gt_ret){
		SDL_MIN_LOG("gfdbDelAtuEntry return %d\n", gt_ret);
		rt = CS_E_ERROR;
		goto END;
	}

	FOR_UNIT_END

END:
    return rt;
}

cs_status epon_request_onu_fdb_entry_get(
    CS_IN  cs_callback_context_t         context,
    CS_IN  cs_int32                      device_id,
    CS_IN  cs_int32                      llidport,
    CS_IN  cs_mac_t                      *mac,                  
    CS_IN  cs_uint16                     vlan,
    CS_OUT cs_sdl_fdb_entry_t            *entry
)
{
    GT_ATU_ENTRY l2_data;
    GT_STATUS      gt_ret = 0;
    cs_status          rt = CS_E_ERROR;
    GT_BOOL        found = GT_FALSE;

    if((NULL==mac) || (NULL==entry) || (vlan>4095)) {
        SDL_MIN_LOG("In %s(%d) error params!\n",__FUNCTION__, __LINE__);
        return CS_E_PARAM;
    }

    /* Check if it is a valid Unicast mac addr */
    if(__fdb_check_mac(mac)){
        SDL_MIN_LOG("In %s(%d) error MAC!\n", __FUNCTION__, __LINE__);
        return CS_E_PARAM;
    }
    

    memset(&l2_data, 0, sizeof(GT_ATU_ENTRY));
    memcpy(&l2_data.macAddr.arEther[0], mac, sizeof(cs_mac_t));
    l2_data.DBNum = vlan;

    FOR_UNIT_START(GT_U32, unit)

    gt_ret = gfdbFindAtuMacEntry(QD_DEV_PTR, &l2_data, &found);
    if(GT_OK != gt_ret ){
       SDL_MIN_LOG("gfdbFindAtuMacEntry return %d\n", gt_ret);
       continue;
    }
    
    if(found == GT_FALSE)
    {
        SDL_MIN_LOG("gfdbFindAtuMacEntry not found entry");
        continue;
    }

    memcpy(&entry->mac, &l2_data.macAddr.arEther[0], sizeof(cs_mac_t));
    entry->vlan_id = l2_data.DBNum;
    entry->port = P2L_PORT(getlportfromucportvec(QD_DEV_PTR, l2_data.portVec));
    entry->type = l2_data.entryState.ucEntryState == GT_UC_DYNAMIC ? SDL_FDB_ENTRY_DYNAMIC : SDL_FDB_ENTRY_STATIC;
    
    rt = CS_E_OK;
    break;

    FOR_UNIT_END

    return rt;
}

cs_status epon_request_onu_fdb_entry_get_byindex( 
    CS_IN  cs_callback_context_t         context,
    CS_IN  cs_int32                      device_id,
    CS_IN  cs_int32                      llidport,
    CS_IN  cs_sdl_fdb_entry_get_mode_t   mode,
    CS_IN  cs_uint16                     offset,
    CS_OUT cs_sdl_fdb_entry_t            *entry,
    CS_OUT cs_uint16                     *next
)
{
    GT_ATU_ENTRY l2_data;
    GT_U32 count = 0, i, db = 0, loffset = offset;
    GT_STATUS gt_ret = GT_OK;
    cs_status ret = CS_E_OK;
    GT_BOOL found = GT_FALSE, getnext = 0;

    GT_U8 maczero[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if((NULL==next) || (NULL==entry)) {
        SDL_MIN_LOG("null pointer\n");
        return CS_E_PARAM;
    }
    
    if(mode > SDL_FDB_ENTRY_GET_MODE_ALL){
        SDL_MIN_LOG("Error Get mode.\n");
        return CS_E_PARAM;
    }

    if(memcmp(entry->mac.addr, maczero, 6))
    	getnext = 1;

    i = 0;
    count = 0;

    FOR_UNIT_START(GT_U32, unit)

    GT_VTU_ENTRY ve;
    GT_32 vnum = 0;

    if( gvtuGetEntryCount(QD_DEV_PTR, &vnum) != GT_OK)
    	continue;

	if(getnext)
	{
		db = entry->vlan_id;
		ve.DBNum = entry->vlan_id;
		ve.vid = entry->vlan_id;
		memcpy(l2_data.macAddr.arEther, entry->mac.addr, 6);
	}
	else
	{
		memset(&l2_data, 0, sizeof(l2_data));
	    if(vnum > 0)
	    {
	    	ve.DBNum = 1;
	    	ve.vid = 1;
	    }
	    else
	    {
	    	if( gvtuGetEntryFirst(QD_DEV_PTR, &ve) != GT_OK )
	    		continue;
	    }
	}

	do
    {
		while(1)
		{
			l2_data.DBNum = ve.DBNum;
			if( db != ve.DBNum )
			{
				gt_ret = gfdbGetAtuEntryFirst(QD_DEV_PTR, &l2_data);
				db = ve.DBNum;
			}
			else
				gt_ret = gfdbGetAtuEntryNext(QD_DEV_PTR, &l2_data);

			if(gt_ret != GT_OK)
			{
				SDL_MIN_LOG("gfdbGetAtuEntryNext fail(%d)\r\n", gt_ret);
				break;
			}

			if((!getnext) && (i < loffset))
			{
				i++;
				continue;
			}

			if((l2_data.entryState.ucEntryState != GT_UC_DYNAMIC && mode == SDL_FDB_ENTRY_DYNAMIC) ||
					(l2_data.entryState.ucEntryState != GT_UC_STATIC && mode == SDL_FDB_ENTRY_STATIC) )
				continue;

			memset(entry, 0, sizeof(cs_sdl_fdb_entry_t));
			memcpy(&entry->mac, &l2_data.macAddr.arEther[0], sizeof(cs_mac_t));
			entry->vlan_id = l2_data.DBNum;
			entry->port = getlportfromucportvec(QD_DEV_PTR, l2_data.portVec)+1;
			entry->type = l2_data.entryState.ucEntryState == GT_UC_DYNAMIC ? SDL_FDB_ENTRY_DYNAMIC : SDL_FDB_ENTRY_STATIC;
			found = GT_TRUE;
			break;
		}

		if(found == GT_TRUE)
			break;
    }while(gvtuGetEntryNext(QD_DEV_PTR, &ve) == GT_OK);
    
	if(found == GT_TRUE)
		break;

    FOR_UNIT_END

    if(found == GT_TRUE)
    	*next = offset+1;
    else
    	ret = CS_E_ERROR;

    return ret;
    
}
cs_status epon_request_onu_fdb_entry_get_byindex_atu(
    CS_IN  cs_callback_context_t         context,
    CS_IN  cs_int32                      device_id,
    CS_IN  cs_int32                      llidport,
    CS_IN  cs_sdl_fdb_entry_get_mode_t   mode,
    CS_IN  cs_uint16                     offset,
    CS_OUT cs_sdl_fdb_entry_t            *entry,
    CS_OUT cs_uint16                     *next
)
{
    cs_aal_fdb_entry_t fdb;
    cs_uint8           index = 0;
    cs_uint8           zero_mac[6] = {0, 0, 0, 0, 0, 0};

    if(NULL==next || NULL==entry ||(offset>__FDB_ENTRY_MAX)) {
        SDL_MIN_LOG("error params\n");
        return CS_E_PARAM;
    }

    /* Look for the static entry */
    for(index=offset; index<__FDB_ENTRY_MAX; index++) {
        memset(&fdb, 0, sizeof(cs_aal_fdb_entry_t));
        if(aal_fdb_entry_get(index, &fdb)){
            return CS_E_ERROR;
        }
        
        if ((fdb.aging_status == 0) && (fdb.static_flag == 0)) {
            continue;
        }

        if((fdb.vld == 0) 
        || ((fdb.static_flag != 0) && (mode == SDL_FDB_ENTRY_GET_MODE_DYNAMIC))
        || ((fdb.static_flag != 1) && (mode == SDL_FDB_ENTRY_GET_MODE_STATIC)) 
        || (memcmp(fdb.mac.addr, zero_mac, 6) == 0))
            continue;
                                
         memcpy(&entry->mac, &fdb.mac, sizeof(cs_mac_t));
         
         if(fdb.vid_vld)
             entry->vlan_id = fdb.vid;
         else
             entry->vlan_id = 0;
         
         entry->port = fdb.port_id + 1;
         entry->type = fdb.static_flag;

         *next = index + 1;
         
         return CS_E_OK;
        
    }
    
    return CS_E_NOT_FOUND;

}


cs_status epon_request_onu_fdb_entry_get_byindex_per_port( 
    CS_IN  cs_callback_context_t         context,
    CS_IN  cs_int32                      device_id,
    CS_IN  cs_int32                      llidport,
    CS_IN  cs_port_id_t                  port_id,
    CS_IN  cs_sdl_fdb_entry_get_mode_t   mode,
    CS_IN  cs_uint16                     offset,
    CS_OUT cs_sdl_fdb_entry_t            *entry,
    CS_OUT cs_uint16                     *next
)
{
	GT_ATU_ENTRY l2_data;
    GT_U32 count = 0, i, hwport = 0,db = 0, lunit, lport, loffset = offset;
    GT_STATUS gt_ret = GT_OK;
    cs_status ret =  CS_E_OK;
    GT_BOOL found = GT_FALSE, getnext = 0;

    GT_U8 maczero[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if((NULL==next) || (NULL==entry)) {
        SDL_MIN_LOG("null pointer\n");
        return CS_E_PARAM;
    }

    if(mode>SDL_FDB_ENTRY_GET_MODE_ALL) {
        SDL_MIN_LOG("Mode is not supported\n");
        return CS_E_PARAM;
    }  
    if(offset>__FDB_ENTRY_HW_INDEX_MAX) {
        SDL_MIN_LOG("Offset is beyond max value\n");
        return CS_E_PARAM;
    }     
    UNI_PORT_CHECK(port_id);
    
    if(gt_getswitchunitbylport(port_id, &lunit, &lport) != GT_OK)
    	return CS_E_PARAM;

    FOR_UNIT_START(GT_U32, unit)

    GT_QD_DEV *dev = QD_DEV_PTR;
    gfdbGetAtuAllCount(QD_DEV_PTR, &count);
    hwport = GT_LPORT_2_PORT(lport);
    if(mode == SDL_FDB_ENTRY_GET_MODE_STATIC){
		cs_printf("into statuc\n");
        for(i=offset; i<__FDB_STATIC_ENTRY_MAC_MAX; i++){
            if(!(__fdb_static_entry_table[port_id-1].valid_map & (1<<i)))
                continue;
                
            memset(entry, 0, sizeof(cs_sdl_fdb_entry_t)); 
            memcpy(&entry->mac, &__fdb_static_entry_table[port_id-1].entry[i].mac.addr[0], sizeof(cs_mac_t));
            entry->vlan_id = __fdb_static_entry_table[port_id-1].entry[i].vlan_id;
            entry->port = port_id;                                      
            entry->type = SDL_FDB_ENTRY_STATIC;
            
            *next = i + 1;
            
            return CS_E_OK;
        }  
        
        if(i == __FDB_STATIC_ENTRY_MAC_MAX){
            SDL_MIN_LOG("port id: %d, no static entry is found\n", port_id);
            
            *next = __FDB_STATIC_ENTRY_MAC_MAX;
            
            return CS_E_NOT_FOUND;
        }
        
    }
    
#if 0
    while(1){
        gt_ret = rtk_l2_addr_next_get(READMETHOD_NEXT_L2UCSPA, port_id-1, &address, &l2_data);
        if(GT_OK != gt_ret){
            SDL_MIN_LOG("rtk_l2_addr_next_get return %d\n", gt_ret);
            return CS_E_NOT_FOUND;
        }
        
        if(((SDL_FDB_ENTRY_GET_MODE_DYNAMIC == mode) && (1 == l2_data.is_static)) ||
            ((SDL_FDB_ENTRY_GET_MODE_STATIC == mode) && (0 == l2_data.is_static))){
            cs_printf("@");
            address++;
            continue;
        }
            
        memset(entry, 0, sizeof(cs_sdl_fdb_entry_t)); 
        memcpy(&entry->mac, &l2_data.mac, sizeof(cs_mac_t));
        entry->vlan_id = l2_data.cvid;
        entry->port = port_id;                                      
        entry->type = l2_data.is_static ? SDL_FDB_ENTRY_STATIC : SDL_FDB_ENTRY_DYNAMIC;
        
        break;
    }
    
    *next = address + 1;
#endif
    if(memcmp(entry->mac.addr, maczero, 6))
    	getnext = 1;

    i = 0;
    count = 0;

    GT_VTU_ENTRY ve;
    GT_32 vnum = 0;

    if( gvtuGetEntryCount(QD_DEV_PTR, &vnum) != GT_OK)
    	continue;

	if(getnext)
	{
		db = entry->vlan_id;
		ve.DBNum = entry->vlan_id;
		ve.vid = entry->vlan_id;
		memcpy(l2_data.macAddr.arEther, entry->mac.addr, 6);
	}
	else
	{
		memset(&l2_data, 0, sizeof(l2_data));
	    if(vnum > 0)
	    {
	    	ve.DBNum = 1;
	    	ve.vid = 1;
	    }
	    else
	    {
	    	if( gvtuGetEntryFirst(QD_DEV_PTR, &ve) != GT_OK )
	    		continue;
	    }
	}
#if 0
    for(i=0; i<count; i++)
    {
    	if(i == 0)
    		gt_ret = gfdbGetAtuEntryFirst(QD_DEV_PTR, &l2_data);
    	else
    		gt_ret = gfdbGetAtuEntryNext(QD_DEV_PTR, &l2_data);
    	cs_printf("into statuc gt_ret is %d\n",gt_ret);
    	if(gt_ret != GT_OK)
    	{
    		ret = CS_E_ERROR;
    		break;
    	}
    	cs_printf("into statuc11\n");
    	if(i < offset)
    		continue;
    	cs_printf("into statuc22\n");
    	if((l2_data.entryState.ucEntryState != GT_UC_DYNAMIC && mode == SDL_FDB_ENTRY_DYNAMIC) ||
    			(l2_data.entryState.ucEntryState != GT_UC_STATIC && mode == SDL_FDB_ENTRY_STATIC) ||
    			(!(l2_data.portVec & (1<<hwport))))
    		continue;
    	cs_printf("into statuc33\n");
    	memset(entry, 0, sizeof(cs_sdl_fdb_entry_t));
    	memcpy(&entry->mac, &l2_data.macAddr.arEther[0], sizeof(cs_mac_t));
    	entry->vlan_id = l2_data.DBNum;
    	entry->port = P2L_PORT(getlportfromucportvec(QD_DEV_PTR, l2_data.portVec));
    	entry->type = l2_data.entryState.ucEntryState == GT_UC_DYNAMIC ? SDL_FDB_ENTRY_DYNAMIC : SDL_FDB_ENTRY_STATIC;
    	*next = i+1;
    	break;
    }
#else
	do
    {
		while(1)
		{
			l2_data.DBNum = ve.DBNum;
			if( db != ve.DBNum )
			{
				gt_ret = gfdbGetAtuEntryFirst(QD_DEV_PTR, &l2_data);
				db = ve.DBNum;
			}
			else
				gt_ret = gfdbGetAtuEntryNext(QD_DEV_PTR, &l2_data);

			if(gt_ret != GT_OK)
			{
				SDL_MIN_LOG("gfdbGetAtuEntryNext fail(%d)\r\n", gt_ret);
				break;
			}

			if((!getnext) && (i < loffset))
			{
				i++;
				continue;
			}

			if((l2_data.entryState.ucEntryState != GT_UC_DYNAMIC && mode == SDL_FDB_ENTRY_DYNAMIC) ||
					(l2_data.entryState.ucEntryState != GT_UC_STATIC && mode == SDL_FDB_ENTRY_STATIC) )
				continue;
			if(port_id !=getlportfromucportvec(QD_DEV_PTR, l2_data.portVec)+1)
				continue;
			memset(entry, 0, sizeof(cs_sdl_fdb_entry_t));
			memcpy(&entry->mac, &l2_data.macAddr.arEther[0], sizeof(cs_mac_t));
			entry->vlan_id = l2_data.DBNum;
			entry->port = getlportfromucportvec(QD_DEV_PTR, l2_data.portVec)+1;
			entry->type = l2_data.entryState.ucEntryState == GT_UC_DYNAMIC ? SDL_FDB_ENTRY_DYNAMIC : SDL_FDB_ENTRY_STATIC;
			found = GT_TRUE;
			break;
		}

		if(found == GT_TRUE)
			break;
    }while(gvtuGetEntryNext(QD_DEV_PTR, &ve) == GT_OK);
	if(found == GT_TRUE)
		break;
#endif
    FOR_UNIT_END
#if 1
    if(found == GT_TRUE)
    	*next = offset+1;
    else
    	ret = CS_E_ERROR;

#endif
	//cs_printf("addess end:%d\n",address);
 	//cs_printf("next:%d",*next);
    return ret;
}


//REVIEW: get full-behavior from startup-cfg for all customer.
//default value: enable learning; shared-vlan mode; limit 64; aging 300sec.
cs_status sdl_fdb_init(
    CS_IN const sdl_init_cfg_t *cfg
)
{
    cs_callback_context_t context;
    cs_port_id_t          portid;
    cs_aal_fdb_port_cfg_t port_cfg;
    cs_aal_fdb_port_msk_t mask;
    cs_status rt;
    memset(__fdb_static_entry_table, 0, UNI_PORT_MAX*sizeof(__fdb_static_entry_t));

    for(portid=CS_UNI_PORT_ID1; portid<=UNI_PORT_MAX; portid++){
    
#if 1
        g_fdb_port_cfg[portid-1].lrn_en = SDL_FDB_MAC_LEARN_DISABLE;
        rt = epon_request_onu_mac_learn_set(context, 0, 0, portid, SDL_FDB_MAC_LEARN_ENABLE);
        if(rt){
            SDL_MIN_LOG("epon_request_onu_mac_learn_set return %d. FILE: %s, LINE: %d\n", rt, __FILE__, __LINE__);

        }

   
        g_fdb_port_cfg[portid-1].lrn_mod = SDL_FDB_INDEPENDENT_VLAN_LEARNING;
        rt = epon_request_onu_fdb_learn_mode_set(context, 0, 0, portid, SDL_FDB_INDEPENDENT_VLAN_LEARNING);
        if(rt){
            SDL_MIN_LOG("epon_request_onu_fdb_learn_mode_set return %d. FILE: %s, LINE: %d\n", rt, __FILE__, __LINE__);

        }
#endif
                
        g_fdb_port_cfg[portid-1].mac_limit = 0;
//        epon_request_onu_fdb_mac_limit_set(context, 0, 0, portid, 0xffff);
        
        g_fdb_port_cfg[portid-1].lrn_en = SDL_FDB_MAC_LEARN_DISABLE;
        g_fdb_port_cfg[portid-1].full_mod = 0;
        g_fdb_port_cfg[portid-1].lrn_mod = SDL_FDB_INDEPENDENT_VLAN_LEARNING;
        g_fdb_port_cfg[portid-1].static_mac_num = 0;
                
    }
    
    epon_request_onu_fdb_age_set(context, 0, 0, __FDB_AGING_TIME_DEFAULT);

    
    /*learn over action is drop at init. if want to forward, set limit num > max entry num*/
//    rtk_l2_limitLearningCntAction_set(RTK_WHOLE_SYSTEM, LIMIT_LEARN_CNT_ACTION_FORWARD);

    
    /* workaround for bug 24604.
     * When set “l2_learn_lru_en” of reg “FE_TOP_MII0_PORT_CTRL_0” 
     * (even when MII0l2 learn is disabled), 
     * LRU works fine when mac limit is set to 64.
     */
    mask.u32 = 0;
    mask.s.learn_dis = 1;
    mask.s.learn_lmt_ovrwrt = 1;
	mask.s.learn_vlan_vld = 1;
    port_cfg.learn_dis = 1;
    port_cfg.learn_lmt_ovrwrt = 1;
	port_cfg.learn_vlan_vld =1;
    
    aal_fdb_port_cfg_set(AAL_PORT_ID_MII0, mask, &port_cfg);

    
    aal_fdb_port_cfg_set(AAL_PORT_ID_GE, mask, &port_cfg);

    
    /* workaround for BUG 29184,make 802.1x packet to be known MC packet*/
    __fdb_8021x_static_mac();
 
    
    return CS_E_OK;
}


