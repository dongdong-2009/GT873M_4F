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

#include "plat_common.h" 
#include "sdl_classify.h"
#include "sdl_classify_util.h"
#include "sdl_util.h"
#include "sdl.h"

#include "MARVELL_BSP_expo.h"
#include "switch_expo.h"
#include "gtDrvSwRegs.h"
#include "switch_drv.h"

// port(1-4),index(0-11)
#define  __RTK_ACL_IDX(port,index)    (((port-1)*RTK_ACL_CLS_MAC_PORT_LENGTH)+RTK_ACL_CLS_MAC_START+index)

typedef struct _pri_queue_remap_ {
	cs_uint8 pri;
	cs_uint8 queue;
}pri_queue_remap_t;

sw_cls_mac_res_port_t g_sw_tbl[UNI_PORT_MAX];
static cs_boolean     g_def_rule_en;

static pri_queue_remap_t s_def_pq_map[] = {
		{0, 0},
		{1, 0},
		{2, 1},
		{3, 1},
		{4, 2},
		{5, 2},
		{6, 3},
		{7, 3}
};

static cs_status __sw_acl_8021p_sel_set(cs_port_id_t portid, cs_sdl_cls_rule_t *psel, cs_uint8 pri, cs_uint8 queue)
{
	cs_status ret = CS_E_ERROR;

	GT_32 unit, port;

	GT_STATUS rt = GT_OK;

	cs_uint32 lport = L2P_PORT(portid);

	if(gt_getswitchunitbylport(lport, &unit, &port) == GT_OK)
	{
		cs_uint8 mpri = ntohl((*(cs_uint32*)&psel->matchValue[CLASS_MATCH_VAL_LEN-4]));

/*		set QD_REG_IEEE_PRI_REMAP_3_0(tag remap)register*/

		rt = gqosSetTagRemap(QD_DEV_PTR, port, mpri, pri);

		if(rt == GT_OK) //添加优先级至队列的映射
		{
			rt = gcosSetUserPrio2Tc(QD_DEV_PTR, pri, queue);
			if(rt == GT_OK)
				ret = CS_E_OK;
			else //删除之前成功的优先级转换操作
				gqosSetTagRemap(QD_DEV_PTR, port, mpri, mpri);
		}
	}

	return ret;
}

static cs_status __sw_acl_8021p_sel_undo(cs_port_id_t portid, cs_sdl_cls_rule_t *psel)
{
	cs_status ret = CS_E_ERROR;
	GT_32 unit, port;

	GT_STATUS rt = GT_OK;

	cs_uint32 lport = L2P_PORT(portid);

	if(gt_getswitchunitbylport(lport, &unit, &port) == GT_OK)
	{
		cs_uint8 mpri = ntohl((*(cs_uint32*)&psel->matchValue[CLASS_MATCH_VAL_LEN-4]));

/*		删除优先级转换设置 */

		rt = gqosSetTagRemap(QD_DEV_PTR, port, mpri, s_def_pq_map[mpri].pri);
		if(rt == GT_OK)
		{

//删除原来设置的优先级-队列映射
			rt = gcosSetUserPrio2Tc(QD_DEV_PTR, mpri, s_def_pq_map[mpri].queue);
			if(rt == GT_OK)
				ret = CS_E_OK;
		}
	}

	return ret;
}

static cs_status __sw_acl_da_sel_set(cs_port_id_t portid, cs_sdl_cls_rule_t *psel, cs_uint8 pri, cs_uint8 queue)
{
	cs_status ret = CS_E_ERROR;

	return ret;
}

static cs_status __sw_acl_sa_sel_set(cs_port_id_t portid, cs_sdl_cls_rule_t *psel, cs_uint8 pri, cs_uint8 queue)
{
	cs_status ret = CS_E_ERROR;

	return ret;
}

static cs_status __sw_acl_vid_sel_set(cs_port_id_t portid, cs_sdl_cls_rule_t *psel, cs_uint8 pri, cs_uint8 queue)
{
	cs_status ret = CS_E_ERROR;

	GT_32 unit, port;

	GT_VTU_ENTRY entry;
	GT_BOOL	found = GT_FALSE;

	GT_STATUS rt = GT_OK;

	cs_uint32 lport = L2P_PORT(portid);

	if(gt_getswitchunitbylport(lport, &unit, &port) == GT_OK)
	{
		cs_uint16 vid = ntohl((*(cs_uint32*)&psel->matchValue[CLASS_MATCH_VAL_LEN-4]));

//		添加vtu entry的修改
		memset(&entry, 0, sizeof(GT_VTU_ENTRY));
		entry.vid = vid;
		entry.DBNum = vid;

		rt = gvtuFindVidEntry(QD_DEV_PTR, &entry, &found);
		if(rt == GT_OK && found == GT_TRUE)
		{
			gvtuDelEntry(QD_DEV_PTR, &entry);

			entry.vidPriOverride = GT_TRUE;
			entry.vidPriority = pri;

			gvtuAddEntry(QD_DEV_PTR, &entry);

//		只改变包的优先级，队列映射由优先级-队列映射寄存器设置完成 by wangxy 2014-02-17
			rt = gprtSetVTUPriOverride(QD_DEV_PTR, port, PRI_OVERRIDE_FRAME);

			if(rt == GT_OK) //添加优先级至队列的映射
			{
				rt = gcosSetUserPrio2Tc(QD_DEV_PTR, pri, queue);
				if(rt == GT_OK)
					ret = CS_E_OK;
			}
		}
	}

	return ret;
}

static cs_status __sw_acl_dscp_sel_set(cs_port_id_t portid, cs_sdl_cls_rule_t *psel, cs_uint8 pri, cs_uint8 queue)
{
	cs_status ret = CS_E_ERROR;

	return ret;
}

static cs_status __sw_acl_add (
    CS_IN cs_port_id_t                 port_id,
    CS_IN cs_uint8                     index,
    CS_IN cs_sdl_classification_t      *prule
)
{
    cs_sdl_cls_rule_t   *ptemp =NULL;
    cs_status ret = CS_E_ERROR;
    
    ptemp = &prule->fselect[0];
    
    switch(ptemp->fieldSelect)
    {
        case CLASS_RULES_FSELECT_DA_MAC:  
        {
        	ret = __sw_acl_da_sel_set(port_id, ptemp, prule->priMark, prule->queueMapped);
            break;
        }
        case CLASS_RULES_FSELECT_SA_MAC:   
        {
        	ret = __sw_acl_sa_sel_set(port_id, ptemp, prule->priMark, prule->queueMapped);
            break;
        }
        case CLASS_RULES_FSELECT_802_1P:     
        {
        	ret = __sw_acl_8021p_sel_set(port_id, ptemp, prule->priMark, prule->queueMapped);
            break;
        }
        case CLASS_RULES_FSELECT_VLAN_ID:    
        {
        	ret = __sw_acl_vid_sel_set(port_id, ptemp, prule->priMark, prule->queueMapped);
            break;
        }
        case CLASS_RULES_FSELECT_ETH_TYPE:
        {
            break;
        }
        case CLASS_RULES_FSELECT_DA_IPV4:
        {     
            break;
        }
        case CLASS_RULES_FSELECT_SA_IPV4:
        {     
            break;
        }
        case CLASS_RULES_FSELECT_IPPRTO:
        {     
            break;
        }
        case CLASS_RULES_FSELECT_TOS_DSCP:
        {     
        	ret = __sw_acl_dscp_sel_set(port_id, ptemp, prule->priMark, prule->queueMapped);
            break;
        }
        case CLASS_RULES_FSELECT_IPV6_TC:
        {     
            break;
        }
        case CLASS_RULES_FSELECT_L4_SRC_PORT:
        {     
            break;
        }
        case CLASS_RULES_FSELECT_L4_DST_PORT:  
        {     
            break;
        }   
        case CLASS_RULES_FSELECT_IPVER:  
        {   
            break;        
        }
        case CLASS_RULES_FSELECT_IPV6_FLOWLABEL:  
        {   
            break;        
        }   
        case CLASS_RULES_FSELECT_DA_IPV6:  
        {   
            break;        
        }
        case CLASS_RULES_FSELECT_SA_IPV6:  
        {   
            break;        
        }
        case CLASS_RULES_FSELECT_DA_IPV6_PREFIX:  
        {   
            break;        
        }
        case CLASS_RULES_FSELECT_SA_IPV6_PREFIX: 
        {   
            break;        
        }
        case CLASS_RULES_FSELECT_IPV6_NEXT_HEADER:  
        {     
            break;
        }    
        default:       
            return CS_E_PARAM;
    }

    
    /*Set QOS Action */
    if(prule->priMark== 0xff) /* 0xff means that don't perform priority marking */
    {
        // act.filterPriority = prule->queueMapped;
        /* 
           NO action, because the resources of classification is share with MAC filter/bind,
           Must ensure that Forward action is from MAC filter/bind,
        */
    }
    else
    {
       // act.actEnable[FILTER_ENACT_1P_REMARK] = TRUE;
    }
      
    return CS_E_OK;
}

static cs_status __sw_acl_del (
    CS_IN cs_port_id_t                 port_id,
    CS_IN cs_uint8                     index
)
{   
    GT_STATUS  retVal;
          
//    retVal = rtk_filter_igrAcl_cfg_del(__RTK_ACL_IDX(port_id,index));
    if(retVal){
        return CS_E_ERROR;
    }
    
    return CS_E_OK;
}

static cs_status __sw_acl_def ( CS_IN cs_boolean  enable)
{    
    cs_uint32      i;
    GT_STATUS rt = GT_OK;

    if(enable == g_def_rule_en)
        return CS_E_OK;
   
    if(enable)
    {
//        rt = rtk_qos_1pRemarkEnable_set(SWITCH_UPLINK_PORT, EPON_TRUE);
        if(rt)
            return CS_E_ERROR;
    }
    else
    {
        for(i=0; i<UNI_PORT_MAX; i++)
        {   
            if(g_sw_tbl[i].g_cls_cnt)
                return CS_E_OK;
        }

//        rt = rtk_qos_1pRemarkEnable_set(SWITCH_UPLINK_PORT, EPON_FALSE);
     //   rt += rtk_filter_igrAcl_cfg_del(RTK_ACL_CLS_DEF); 
        if(rt)
            return CS_E_ERROR;      
    }
   
    g_def_rule_en = enable;
      
    return CS_E_OK;
}

cs_status epon_request_onu_port_classification_add(
        CS_IN cs_callback_context_t         context,
        CS_IN cs_int32                      device_id,
        CS_IN cs_int32                      llidport,
        CS_IN cs_port_id_t                  port_id,
        CS_IN cs_uint8                      num,
        CS_IN cs_sdl_classification_t       *cfg
)
{    
    cs_int32  i;
    cs_status ret = CS_E_OK;
    cs_uint8  start_idx=0; //affect highest index,for the start index of update HW 
    cs_uint8  tbl_num =0;
    cs_uint8  tbl_size =0;
    cs_uint32 masktemp;
    cs_sdl_classification_t  *ptemp =NULL;
    cs_sdl_classification_t  tbl[RTK_ACL_CLS_MAC_PORT_LENGTH];

        
    UNI_PORT_CHECK(port_id);

    if(cfg == NULL)
    {  
        SDL_MIN_LOG(" NULL pointer.\n");
        return CS_E_PARAM;
    }
     
    //tbl_size = (g_sw_tbl[port_id-1].g_mac_cnt >4)?(RTK_ACL_CLS_MAC_PORT_LENGTH -g_sw_tbl[port_id-1].g_mac_cnt): RTK_ACL_CLS_PORT_LENGTH;
    tbl_size = RTK_ACL_CLS_MAC_PORT_LENGTH -g_sw_tbl[port_id-1].g_mac_cnt;
    if(tbl_size == 0)
    {  
        SDL_MIN_LOG("CLS resouce is 0.\n");
        return CS_E_RESOURCE;
    }
       
    if((num >tbl_size)||(num <1))
    {  
        SDL_MIN_LOG("para error. rule num: %d. [%s %d]\n", num,  __FUNCTION__, __LINE__);
        return CS_E_PARAM;
    }
    
    ret = class_rule_para_check(num, cfg);
    if (ret) 
        return ret;

    ptemp = cfg;
    
     /* only support one filter key for multi-ports onu */
    for(i=0; i<num; i++)
    {   
        if(ptemp->entries!= 1){
            return CS_E_PARAM;
        }
        /* not support 0xff*/
        if(ptemp->priMark == 0xff){
            return CS_E_NOT_SUPPORT;
        }
        masktemp = 0;
        /* rule field para check */
        ret = class_field_para_check(ptemp->entries, ptemp->fselect, &masktemp);
        if (ret) 
            return ret;
        if((masktemp&(1<<CLASS_RULES_FSELECT_DA_IPV6_PREFIX))||
            (masktemp&(1<<CLASS_RULES_FSELECT_SA_IPV6_PREFIX)))
        {
            if (ptemp->fselect[0].matchValue[CLASS_MATCH_VAL_LEN - 1] > 64)
            {
                SDL_MIN_LOG("illegal IPV6 PREFIX(1-64),%d. L%d\n",ptemp->fselect[0].matchValue[CLASS_MATCH_VAL_LEN - 1], __LINE__);
                return CS_E_PARAM;
            }
        }
            
        ptemp++;
    }
    
     /* add record */
    ret = class_rule_tbl_add_record(port_id, tbl_size, num, cfg, &start_idx);
    if (ret) 
        return ret;

    memset(tbl, 0, RTK_ACL_CLS_MAC_PORT_LENGTH*sizeof(cs_sdl_classification_t));
    /* get record to update HW */ 
    ret = class_rule_tbl_get_record(port_id, &tbl_num, tbl);
    if (ret) 
        return ret;

    for(i=tbl_num-1; i>=start_idx; i--)
    { 
        ret =__sw_acl_add(port_id, i, &tbl[i]);
        if (ret) 
            return ret;
    }

    g_sw_tbl[port_id-1].g_cls_cnt = tbl_num;

    ret =__sw_acl_def(TRUE);
       
    return ret;    
}

cs_status epon_request_onu_port_classification_del(
        CS_IN cs_callback_context_t         context,
        CS_IN cs_int32                      device_id,
        CS_IN cs_int32                      llidport,
        CS_IN cs_port_id_t                  port_id,
        CS_IN cs_uint8                      prenum,
        CS_IN cs_uint8                      *precedence
)
{   
    cs_int32  i;
    cs_status ret = CS_E_OK;
    cs_uint8  tbl_num =0;
    cs_uint8  tbl_size =0;
    cs_uint8  start_idx; //Effects highest index,for the start index of update HW 
    cs_sdl_classification_t  tbl[RTK_ACL_CLS_MAC_PORT_LENGTH];

    
    UNI_PORT_CHECK(port_id);

    // tbl_size = (g_sw_tbl[port_id-1].g_mac_cnt >4)?(RTK_ACL_CLS_MAC_PORT_LENGTH -g_sw_tbl[port_id-1].g_mac_cnt): RTK_ACL_CLS_PORT_LENGTH;
    tbl_size = RTK_ACL_CLS_MAC_PORT_LENGTH -g_sw_tbl[port_id-1].g_mac_cnt; 
    if((prenum >tbl_size)||(prenum <1))
    {  
        SDL_MIN_LOG("para error. rule num: %d. [%s %d]\n", prenum,  __FUNCTION__, __LINE__);
        return CS_E_PARAM;
    }
    
    if(precedence == NULL)
    {  
        SDL_MIN_LOG(" NULL pointer.\n");
        return CS_E_PARAM;
    }
    /* get current data number before del operation */
    ret = class_rule_tbl_get_record(port_id, &tbl_num, tbl);
    if (ret) 
        return ret;
    
    if(tbl_num==0)
         return CS_E_OK;
        
    ret = class_rule_tbl_del_record(port_id, prenum, precedence, &start_idx);
    if (ret) 
        return ret;
    /* can't deleted any entry*/
    if(start_idx == tbl_num)
        return CS_E_OK;

    memset(tbl, 0, RTK_ACL_CLS_MAC_PORT_LENGTH*sizeof(cs_sdl_classification_t));
    tbl_num =0;
     /* get finally record to update HW */ 
    ret = class_rule_tbl_get_record(port_id, &tbl_num, tbl);
    if (ret) 
        return ret;

    for(i=start_idx; i< tbl_size; i++)
    {   
        if(i>=tbl_num)
        {
            ret =__sw_acl_del(port_id, i);
        }
        else
        {
            ret =__sw_acl_add(port_id, i, &tbl[i]);
        }
      
        if (ret) 
            return ret;
    }

    g_sw_tbl[port_id-1].g_cls_cnt = tbl_num;
   
    ret =__sw_acl_def(FALSE);
       
    return ret;       
}

cs_status epon_request_onu_port_classification_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,                
    CS_OUT cs_uint8                  *rule_num,
    CS_OUT cs_sdl_classification_t   *cfg
)
{ 
   
    UNI_PORT_CHECK(port_id);
   
    if((rule_num == NULL)||(cfg == NULL))
    {  
        SDL_MIN_LOG("para error.  [%s %d]\n",  __FUNCTION__, __LINE__);
        return CS_E_PARAM;
    }

    return class_rule_tbl_get_record(port_id, rule_num, cfg);        
}

cs_status epon_request_onu_port_classification_clr(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id
)
{
    cs_int32  i;
    cs_uint8  tbl_size =0;
    cs_status ret = CS_E_OK;

  
    UNI_PORT_CHECK(port_id);

   // tbl_size = (g_sw_tbl[port_id-1].g_mac_cnt >4)?(RTK_ACL_CLS_MAC_PORT_LENGTH -g_sw_tbl[port_id-1].g_mac_cnt): RTK_ACL_CLS_PORT_LENGTH;
    tbl_size = RTK_ACL_CLS_MAC_PORT_LENGTH -g_sw_tbl[port_id-1].g_mac_cnt;
    ret = class_rule_tbl_clr_record(port_id);
    if (ret) 
        return ret;
    
    for(i=0; i< tbl_size; i++)
    {       
        ret =__sw_acl_del(port_id, i);
        if (ret) 
            return ret;
    }

    g_sw_tbl[port_id-1].g_cls_cnt = 0;

    ret =__sw_acl_def(FALSE);
       
    return ret;      
}


cs_status sdl_cls_init(void)
{

    memset(g_sw_tbl, 0, UNI_PORT_MAX*sizeof(sw_cls_mac_res_port_t));
    g_def_rule_en = 0;
    
    return CS_E_OK;    
}

