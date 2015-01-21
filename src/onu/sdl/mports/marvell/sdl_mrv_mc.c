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

DEFINITIONS:  "DEVICE" means the Cortina Systems?Daytona SDK product.
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
/*
 * $Id: sdl_mc.c,v 1.1.2.20 2011/10/10 07:40:56 fpei Exp $
 */

#include "sdl_mc.h"
#include "sdl_vlan.h"
#include "aal_l2.h"
#include "aal_cls.h"

#include "sdl_ma.h"
#include "sdl.h"
#include "aal.h"

#include "MARVELL_BSP_expo.h"
#include "switch_expo.h"
#include "gtDrvSwRegs.h"
#include "switch_drv.h"

#define __DEFAULT_FID        0
#define __SVL                0
#define __IVL                1


//workaroud for MAC+VLAN
// only consider the case that a fdb be set once in a port.
#define __MAX_MC_VLAN        8


typedef struct {
    cs_uint16     vlanid;
    cs_uint16     fdb_cnt;
    
} mc_vlan_t;

typedef struct {
    mc_vlan_t     l2_vlan[__MAX_MC_VLAN];
    cs_uint16     used_num;
    
} cs_sdl_mc_vlan_port_t;

static cs_sdl_mc_vlan_port_t  mc_vlan[UNI_PORT_MAX]; 
    
static cs_status __mc_vlan_add( CS_IN cs_port_id_t portid, CS_IN cs_uint16  vlanid)
{    
    cs_int32             i;
    cs_status             ret = CS_E_OK;
    cs_callback_context_t context;

    
    UNI_PORT_CHECK(portid);
    
    for(i=0; i<mc_vlan[portid-1].used_num; i++)
    {  
         // find, then fdb_cnt ++
        if(vlanid == mc_vlan[portid-1].l2_vlan[i].vlanid)
        {
            mc_vlan[portid-1].l2_vlan[i].fdb_cnt++;
            return CS_E_OK;    
        }              
    }
    // new vlanid
    if(mc_vlan[portid-1].used_num == __MAX_MC_VLAN)
    {
        return CS_E_RESOURCE;    
    }

    ret = epon_request_onu_mc_vlan_add(context,0,0, portid,&vlanid,1);
    if(ret)
        return ret;
    
    mc_vlan[portid-1].l2_vlan[mc_vlan[portid-1].used_num].vlanid = vlanid;
    mc_vlan[portid-1].l2_vlan[mc_vlan[portid-1].used_num].fdb_cnt++;
    mc_vlan[portid-1].used_num++;
    
    return CS_E_OK;    
    
}

static cs_status __mc_vlan_del( CS_IN cs_port_id_t portid, CS_IN cs_uint16  vlanid)
{    
    cs_int32              i;
    cs_uint8              temp =0;
    cs_boolean            find = FALSE;
    cs_status             ret = CS_E_OK;
    cs_callback_context_t context;

    
    UNI_PORT_CHECK(portid);
    
    for(i=0; i<mc_vlan[portid-1].used_num; i++)
    {  
         // find
        if(vlanid == mc_vlan[portid-1].l2_vlan[i].vlanid)
        {
            temp = mc_vlan[portid-1].l2_vlan[i].fdb_cnt-1;
            find = TRUE;      
            break;
        }
    }
    
    if(!find)
    {
        return CS_E_NOT_FOUND;
    }
    
    // find, then fdb_cnt --
    if(temp !=0)
    {   
        mc_vlan[portid-1].l2_vlan[i].fdb_cnt--;
        return CS_E_OK;  
    }
    // last member
    ret = epon_request_onu_mc_vlan_del(context,0,0, portid, &vlanid, 1);
    if(ret)
         return ret;

    mc_vlan[portid-1].used_num--;
    mc_vlan[portid-1].l2_vlan[i].vlanid   = mc_vlan[portid-1].l2_vlan[mc_vlan[portid-1].used_num].vlanid;
    mc_vlan[portid-1].l2_vlan[i].fdb_cnt  = mc_vlan[portid-1].l2_vlan[mc_vlan[portid-1].used_num].fdb_cnt;
    mc_vlan[portid-1].l2_vlan[mc_vlan[portid-1].used_num].vlanid  =0;
    mc_vlan[portid-1].l2_vlan[mc_vlan[portid-1].used_num].fdb_cnt =0;

    return CS_E_OK;    
}

static cs_status __mc_vlan_clr()
{
    cs_callback_context_t context;
    cs_port_id_t          portid;
    cs_status             ret = CS_E_OK;

    for(portid= CS_UNI_PORT_ID1; portid<=CS_UNI_PORT_ID4; portid++)
    {  
        ret = epon_request_onu_mc_vlan_clr(context,0,0, portid);
        if(ret)
            return ret;
    }

    memset(&mc_vlan[0], 0, UNI_PORT_MAX*sizeof(cs_sdl_mc_vlan_port_t));
    
    return CS_E_OK;    
}
/* if enable is 0 unknown mc forward enable
 * else enable is 1 unknown mc forward disable
*/
cs_status epon_request_onu_unknown_mc_forward_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              portid,
    CS_IN cs_boolean                enable
)
{
	cs_status ret = CS_E_ERROR;

	GT_32 unit, port;

	if(portid == CS_PON_PORT_ID)
		portid = CS_UPLINK_PORT;

	if(gt_getswitchunitbylport(L2P_PORT(portid), &unit, &port) == GT_OK)
	{
		GT_STATUS ret = GT_OK;
		GT_EGRESS_FLOOD mode;
		if(gprtGetEgressFlood(QD_DEV_PTR, port, &mode) == GT_OK)
		{
#if 0
			switch(mode)
			{
			case GT_BLOCK_EGRESS_NONE:
				if(!enable)
					mode = GT_BLOCK_EGRESS_UNKNOWN_MULTICAST;
				break;
			case GT_BLOCK_EGRESS_UNKNOWN:
				if(enable)
					mode = GT_BLOCK_EGRESS_UNKNOWN_UNICAST;
				break;
			case GT_BLOCK_EGRESS_UNKNOWN_MULTICAST:
				if(enable)
					mode = GT_BLOCK_EGRESS_NONE;
				break;
			case GT_BLOCK_EGRESS_UNKNOWN_UNICAST:
				if(!enable)
					mode = GT_BLOCK_EGRESS_UNKNOWN_MULTICAST;
				break;
#else
				switch(mode)
				{
				case GT_BLOCK_EGRESS_NONE:
					if(enable)
					{
						mode = GT_BLOCK_EGRESS_UNKNOWN_MULTICAST;
					}
					break;
				case GT_BLOCK_EGRESS_UNKNOWN:
					if(!enable)
					{
						mode = GT_BLOCK_EGRESS_UNKNOWN_UNICAST;
					}
					break;
				case GT_BLOCK_EGRESS_UNKNOWN_MULTICAST:
					if(!enable)
					{
						mode = GT_BLOCK_EGRESS_NONE;
					}
					break;
				case GT_BLOCK_EGRESS_UNKNOWN_UNICAST:
					if(enable)
					{
						mode = GT_BLOCK_EGRESS_UNKNOWN;
					}
					break;
#endif
			}

			if(gprtSetEgressFlood(QD_DEV_PTR, port, mode) == GT_OK)
				ret = CS_E_OK;
		}
	}
    return ret;
}
#if 0
cs_status epon_request_onu_igmpsnoop_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              portid,
    CS_IN cs_boolean                enable
)
{
	cs_status ret = CS_E_ERROR;

	GT_32 unit, port;

	if(portid == CS_PON_PORT_ID)
		portid = CS_UPLINK_PORT;

	if(gt_getswitchunitbylport(L2P_PORT(portid), &unit, &port) == GT_OK)
	{
		if(gprtSetIGMPSnoop(QD_DEV_PTR, port, enable) == GT_OK)
			ret = CS_E_OK;
	}

	return ret;
}
#endif
cs_status epon_request_onu_mc_l2_entry_add (
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_uint32                 device_id,
    CS_IN  cs_uint32                 llidport,
    CS_IN  cs_port_id_t              portid,
    CS_IN  cs_sdl_mc_l2_entry_t      *entry
)
{
	cs_status ret = CS_E_ERROR;

    if(NULL==entry)
        return CS_E_PARAM;

    UNI_PORT_CHECK(portid);
    VID_CHECK(entry->vlan);

    /* must be multicast address */
    if (!(entry->mac.addr[0] & 0x1))
    {
        SDL_MIN_LOG("not a multicast mac address. FILE: %s, LINE: %d", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    // vlan==0 ---SVL
    if(entry->vlan != 0)
    {
        //workaroud for MAC+VLAN
        // only consider the case that a fdb be set once in a port.
        ret = __mc_vlan_add(  portid, entry->vlan);
        if(ret)
        {
            SDL_MIN_LOG("__mc_vlan_add return %d. ", ret);
            return ret;
        }
    }

    {
    	GT_ATU_ENTRY atu;
    	GT_BOOL found = GT_FALSE;

    	GT_32 unit, port;

    	SDL_MIN_LOG("gmac: %02x:%02x:%02x:%02x:%02x:%02x vid: %d port %d\n", entry->mac.addr[0],
    			entry->mac.addr[1],
    			entry->mac.addr[2],
    			entry->mac.addr[3],
    			entry->mac.addr[4],
    			entry->mac.addr[5],
    			entry->vlan, portid);

    	memset(&atu, 0, sizeof(atu));

    	memcpy(atu.macAddr.arEther, entry->mac.addr, sizeof(atu.macAddr.arEther));
    	atu.DBNum = entry->vlan == 0 ? 1:entry->vlan;
    	atu.entryState.mcEntryState = GT_MC_STATIC;

    	gt_getswitchunitbylport(L2P_PORT(portid), &unit, &port);

    	if(gfdbFindAtuMacEntry(QD_DEV_PTR, &atu, &found) == GT_OK)
    	{
   			atu.portVec |= 1<<(L2P_PORT(portid));
    	}
    	else
    	{
    		atu.portVec = 1<<(L2P_PORT(portid));
    	}

    	if(gfdbAddMacEntry(QD_DEV_PTR, &atu) == GT_OK)
    		ret = CS_E_OK;

    	SDL_MIN_LOG("%s ret (%d)\n", __func__, ret);
    }

	return ret;
}

cs_status epon_request_onu_mc_l2_entry_del (
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_uint32                 device_id,
    CS_IN  cs_uint32                 llidport,
    CS_IN  cs_port_id_t              portid,
    CS_IN  cs_sdl_mc_l2_entry_t      *entry
)
{
	cs_status ret = CS_E_ERROR;

    if(NULL==entry)
        return CS_E_PARAM;

    UNI_PORT_CHECK(portid);
    VID_CHECK(entry->vlan);

    /* must be multicast address */
    if (!(entry->mac.addr[0] & 0x1))
    {
        SDL_MIN_LOG("not a multicast mac address. FILE: %s, LINE: %d", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    {
    	GT_ATU_ENTRY atu;
    	GT_STATUS gtret = GT_ERROR;
    	GT_BOOL found = GT_FALSE;
    	GT_32 unit, port;

    	if(gt_getswitchunitbylport(L2P_PORT(portid), &unit, &port) == GT_OK)
    	{

    		memset(&atu, 0, sizeof(atu));
    		memcpy(atu.macAddr.arEther, entry->mac.addr, sizeof(atu.macAddr.arEther));
    		atu.DBNum = entry->vlan == 0?1:entry->vlan;
    		if( gfdbFindAtuMacEntry(QD_DEV_PTR, &atu, &found) == GT_OK)
    		{
    			if(atu.portVec& ( 1<<port ))
    			{
    		        if(entry->vlan != 0)
    		        {
    		            ret = __mc_vlan_del(portid, entry->vlan);
    		            if(ret)
    		            {
    		                SDL_MIN_LOG("__mc_vlan_del return %d. ", ret);
    		                return ret;
    		            }
    		        }

    		        atu.portVec &= ~(1<<port);

    		        if(atu.portVec & ((1<<UNI_PORT_MAX)-1))
    		        	gtret = gfdbAddMacEntry(QD_DEV_PTR, &atu);
    		        else
    		        	gtret =gfdbDelAtuEntry(QD_DEV_PTR, &atu);
    			}
    		}

    		if(gtret == GT_OK)
    			ret = CS_E_OK;
    	}
    }

	return ret;
}

cs_status epon_request_onu_mc_l2_entry_clr (
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_uint32                 device_id,
    CS_IN  cs_uint32                 llidport
)
{
	return __mc_vlan_clr();
}

cs_status epon_request_onu_mc_l2_port_get (
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_uint32                 device_id,
    CS_IN  cs_uint32                 llidport,
    CS_IN  cs_sdl_mc_l2_entry_t      *entry,
    CS_OUT cs_sdl_portmask_t         *portmask
)
{
	cs_status ret = CS_E_ERROR;

	GT_ATU_ENTRY atu;
	GT_BOOL found = GT_FALSE;
	GT_STATUS gt_ret = GT_ERROR;

	if(entry && portmask)
	{
		memset(&atu, 0, sizeof(atu));

		memcpy(atu.macAddr.arEther, entry->mac.addr, sizeof(entry->mac.addr));
		atu.DBNum = entry->vlan == 0?1:entry->vlan;

		FOR_UNIT_START(GT_32, unit)

		gt_ret = gfdbFindAtuMacEntry(QD_DEV_PTR, &atu, &found);

		if(gt_ret == GT_OK && found)
		{
			int i;
			portmask->bits[0] = 0;

			for(i=0; i<QD_DEV_PTR->maxPorts; i++)
			{
				if(atu.portVec&(1<<i))
					portmask->bits[0] |= 1<<i;
			}

			ret = CS_E_OK;
			break;
		}

		FOR_UNIT_END

		if(!found)
			ret = CS_E_NOT_FOUND;
	}

	return ret;
}

cs_status epon_request_onu_mc_ip_entry_add(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_uint32                 device_id,
    CS_IN cs_uint32                 llidport,
    CS_IN cs_port_id_t              portid,
    CS_IN cs_sdl_mc_ip_entry_t      *entry

)
{

    return CS_E_NOT_SUPPORT;
}

cs_status epon_request_onu_mc_ip_entry_del(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_uint32                 device_id,
    CS_IN cs_uint32                 llidport,
    CS_IN cs_port_id_t              portid,
    CS_IN cs_ip_t                   grp,
    CS_IN cs_uint16                 vlan
)
{

    return CS_E_NOT_SUPPORT;
}

cs_status epon_request_onu_mc_ip_entry_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_uint32                 device_id,
    CS_IN  cs_uint32                 llidport,
    CS_IN  cs_port_id_t              portid,
    CS_IN  cs_ip_t                   grp,
    CS_IN  cs_uint16                 vlan,
    CS_OUT cs_sdl_mc_ip_entry_t      *entry
)
{

    return CS_E_NOT_SUPPORT;
}

cs_status epon_request_onu_mc_ip_entry_clr(
    CS_IN  cs_callback_context_t       context,
    CS_IN  cs_uint32                   device_id,
    CS_IN  cs_uint32                   llidport
)
{
    return CS_E_NOT_SUPPORT;
}

cs_status sdl_mc_l2entry_sw_show(cs_uint8 index, cs_uint8 num)
{
    return CS_E_NOT_SUPPORT;
}

cs_status sdl_mc_init(void)
{

    memset(&mc_vlan[0], 0, UNI_PORT_MAX*sizeof(cs_sdl_mc_vlan_port_t));
      
    return CS_E_OK;
}


