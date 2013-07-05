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

#include "plat_common.h"
#include "sdl_qos_cmn.h"
#include "aal_flow.h"
#include "aal_l2.h"
#include "aal_bm.h"
#include "sdl_vlan_util.h"
#include "sdl.h"
#ifdef HAVE_MPORTS
#include "rtk_api_ext.h"
#endif

#define DSCP_MAX        64
#define TOS_MAX         8

static cs_sdl_port_drop_cfg_t __drop_pon = SDL_PORT_NO_DROP;
static cs_sdl_port_drop_cfg_t __drop_ge = SDL_PORT_NO_DROP;

/**************work around for dscp->8021p->cos mapping*********************/

static cs_status __sdl_qos_change_dscp_cos_mapping(cs_pri_cos_map_t *queue_map)
{
    cs_aal_dscp_map_t dscp_msk;
    cs_aal_dscp_map_t dscp_cfg[DSCP_MAX];
    cs_uint8 i, j;
    cs_status ret = CS_E_OK;

    dscp_msk.u = 0;
    dscp_msk.s.cos_map = 1;

    memset((void *)dscp_cfg, 0, DSCP_MAX*sizeof(cs_aal_dscp_map_t));

    ret  = aal_flow_dscp_mapping_get(AAL_PORT_ID_GE, dscp_cfg);
    if (ret) {
        goto end;
    }
    /*seach dscp->8021p mapping, the known 8021p correspond dscp will be changed cos mapping*/
    for (i = 0; i < TOS_MAX; i++) {
        for (j = 0; j < DSCP_MAX; j++) {
            if (dscp_cfg[j].s.dot1p_map == i) {
                dscp_cfg[j].s.cos_map = queue_map->cos[i];
            }
        }
    }
    /**config GE port dscp to cos mapping***/
    ret = aal_flow_dscp_mapping_set(AAL_PORT_ID_GE, &dscp_msk, dscp_cfg);
    if (ret) {
        goto end;
    }

    /**config pon port dscp to cos mapping***/
    ret = aal_flow_dscp_mapping_set(AAL_PORT_ID_PON, &dscp_msk, dscp_cfg);

end:
    return ret;
}

cs_status epon_request_onu_traffic_block(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_direction_t            dirction,
    CS_IN cs_boolean                is_block
)
{
    cs_status rc = CS_E_OK;

    cs_aal_spec_pkt_ctrl_msk_t pkt_ctrl_msk;
    cs_aal_spec_pkt_ctrl_t pkt_ctrl;
    cs_aal_port_id_t port;
    memset(&pkt_ctrl, 0 , sizeof(pkt_ctrl));


    if (dirction == CS_DOWN_STREAM) {
        if (is_block == __drop_pon) {
            goto end;
        }
        __drop_pon = is_block;
        port = AAL_PORT_ID_PON;
    } else if(dirction == CS_UP_STREAM){
        if (is_block == __drop_ge) {
            goto end;
        }
        __drop_ge = is_block;
        port = AAL_PORT_ID_GE;
    }else{
        rc = CS_E_PARAM;
        goto end;
    }

    if (is_block == FALSE) {

        pkt_ctrl_msk.u32 = 0;
        pkt_ctrl_msk.s.dpid = 1;
        pkt_ctrl_msk.s.pri = 1;
        pkt_ctrl.pri = 1;
        pkt_ctrl.dpid.dst_op = AAL_SPEC_DST_FE;
        rc = aal_special_pkt_behavior_set(port, AAL_PKT_NORMAL, pkt_ctrl_msk, &pkt_ctrl);
        if (rc) {
            goto end;
        }

    } else {
        pkt_ctrl_msk.u32 = 0;
        pkt_ctrl_msk.s.dpid = 1;
        pkt_ctrl_msk.s.pri = 1;
        pkt_ctrl.pri = 30;
        pkt_ctrl.dpid.dst_op = AAL_SPEC_DST_DROP;
        rc = aal_special_pkt_behavior_set(port, AAL_PKT_NORMAL, pkt_ctrl_msk, &pkt_ctrl);
        if (rc) {
            goto end;
        }
    }


end:
    return rc;
}

cs_status epon_request_onu_port_traffic_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_direction_t            dirction,
    CS_OUT cs_boolean               *is_drop
)
{

    if (NULL == is_drop) {
        SDL_MIN_LOG("In %s, NULL pointer!\n", __FUNCTION__);
        return CS_E_PARAM;
    }
    if (dirction == CS_DOWN_STREAM) {
        *is_drop = __drop_pon;
    } else if (dirction == CS_UP_STREAM) {
        *is_drop = __drop_ge;
    } else {
        return CS_E_PARAM;
    }
    return CS_E_OK;
}


/****************************************************************/
/* $rtn_hdr_start  epon_request_onu_mib_que_info_set        */
/* CATEGORY   : Device                                          */
/* ACCESS     : Public                                          */
/* BLOCK      : SAL                                             */
/* CHIP       : 8030                                            */
cs_status epon_request_onu_dot1p_map_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_pri_cos_map_t          *queue_map
)

{
    cs_aal_flow_mapping_msk_t qos_msk;
    cs_aal_flow_mapping_t qos_cfg;

    cs_status ret = CS_E_OK;

    if (!queue_map) {
        SDL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d", __FILE__, __LINE__);
        ret = CS_E_PARAM;
        goto end;
    }
    /*******add work around because we don't have dscp->8021p->cos mapping********/

    __sdl_qos_change_dscp_cos_mapping(queue_map);
    memset(&qos_cfg, 0 , sizeof(qos_cfg));

    qos_msk.u = 0;
    qos_msk.s.cos = 1;
    qos_cfg.cos.s.map0 = queue_map->cos[0];
    qos_cfg.cos.s.map1 = queue_map->cos[1];
    qos_cfg.cos.s.map2 = queue_map->cos[2];
    qos_cfg.cos.s.map3 = queue_map->cos[3];
    qos_cfg.cos.s.map4 = queue_map->cos[4];
    qos_cfg.cos.s.map5 = queue_map->cos[5];
    qos_cfg.cos.s.map6 = queue_map->cos[6];
    qos_cfg.cos.s.map7 = queue_map->cos[7];
    ret = aal_flow_dot1p_mapping_set(AAL_PORT_ID_GE, &qos_msk, &qos_cfg);
    if (ret) {
        goto end;
    }

    ret = aal_flow_dot1p_mapping_set(AAL_PORT_ID_PON, &qos_msk, &qos_cfg);
    if (ret) {
        goto end;
    }

    ret = cosmap_vlan_untag_ctrl_update(CS_UNI_PORT_ID1); // workaround for TX-dot1p map to cos

end:
    return ret;
}


/****************************************************************/
/* $rtn_hdr_start  epon_request_onu_dot1p_map_get        */
/* CATEGORY   : Device                                          */
/* ACCESS     : Public                                          */
/* BLOCK      : SAL                                             */
/* CHIP       : 8030                                            */
cs_status epon_request_onu_dot1p_map_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                device_id,
    CS_IN cs_int32                llidport,
    CS_OUT cs_pri_cos_map_t         *queue_map
)

{
    cs_aal_flow_mapping_t qos_cfg;
    cs_status ret = CS_E_OK;

    if (!queue_map) {
        SDL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d", __FILE__, __LINE__);
        ret = CS_E_PARAM;
        goto end;
    }
    ret = aal_flow_dot1p_mapping_get(AAL_PORT_ID_GE, &qos_cfg);
    if (ret) {
        goto end;
    }
    queue_map->cos[0] = qos_cfg.cos.s.map0;
    queue_map->cos[1] = qos_cfg.cos.s.map1;
    queue_map->cos[2] = qos_cfg.cos.s.map2;
    queue_map->cos[3] = qos_cfg.cos.s.map3;
    queue_map->cos[4] = qos_cfg.cos.s.map4;
    queue_map->cos[5] = qos_cfg.cos.s.map5;
    queue_map->cos[6] = qos_cfg.cos.s.map6;
    queue_map->cos[7] = qos_cfg.cos.s.map7;
end:
    return ret;
}


/********************************************************************************/
/* $rtn_hdr_start  epon_request_onu__tos_map_set                                */
/* CATEGORY   : Device                                                          */
/* ACCESS     : Public                                                          */
/* BLOCK      : SAL                                                             */
/* CHIP       : LynxD                                                           */
cs_status epon_request_onu_tos_map_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                device_id,
    CS_IN cs_int32                llidport,
    CS_IN cs_tos_pri_map_t          *tos_map
)

{
    cs_uint8 i;
    cs_aal_dscp_map_t qos_cfg[DSCP_MAX];
    cs_aal_dscp_map_t qos_msk;
    cs_pri_cos_map_t  queue_map;
    cs_status ret = CS_E_OK;

    if (!tos_map) {
        SDL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d", __FILE__, __LINE__);
        ret = CS_E_PARAM;
        goto end;
    }

    memset(&queue_map, 0, sizeof(cs_pri_cos_map_t));
    memset((cs_uint8 *)qos_cfg, 0, sizeof(cs_aal_dscp_map_t)*DSCP_MAX);

    ret  = epon_request_onu_dot1p_map_get(context, 0, 0, &queue_map);
    if (ret) {
        goto end;
    }

    qos_msk.u = 0;
    qos_msk.s.dot1p_map = 1;
    qos_msk.s.cos_map   = 1;

    /***********workaround for dscp->cos mapping, which should match 8021p to cos mapping***************/
    for (i = 0; i < DSCP_MAX; i++) {
        qos_cfg[i].s.cos_map = queue_map.cos[tos_map->pri[i/(DSCP_MAX/TOS_MAX)]];
    }

#if 1
    for (i = 0; i < DSCP_MAX; i++) {
        qos_cfg[i].s.dot1p_map = tos_map->pri[i/(DSCP_MAX/TOS_MAX)];
    }
#endif

    ret = aal_flow_dscp_mapping_set(AAL_PORT_ID_GE, &qos_msk, qos_cfg);
    if (ret) {
        goto end;
    }

    ret = aal_flow_dscp_mapping_set(AAL_PORT_ID_PON, &qos_msk, qos_cfg);
    if (ret) {
        goto end;
    }

end:
    return ret;
}


/*******************************************************************************/
/* $rtn_hdr_start  epon_request_onu_tos_map_get                                 */
/* CATEGORY   : Device                                                          */
/* ACCESS     : Public                                                          */
/* BLOCK      : SAL                                                             */
/* CHIP       : LynxD                                                           */
cs_status epon_request_onu_tos_map_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                device_id,
    CS_IN cs_int32                llidport,
    CS_OUT cs_tos_pri_map_t         *tos_map
)

{
    cs_uint8 i;
    cs_aal_dscp_map_t qos_cfg[DSCP_MAX];
    cs_status ret = CS_E_OK;

    if (!tos_map) {
        SDL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d", __FILE__, __LINE__);
        ret = CS_E_PARAM;
        goto end;
    }

    ret = aal_flow_dscp_mapping_get(AAL_PORT_ID_GE, qos_cfg);
    if (ret) {
        goto end;
    }

    for (i = 0; i < TOS_MAX; i++) {
        tos_map->pri[i] = qos_cfg[i*(DSCP_MAX/TOS_MAX)].s.dot1p_map;
    }
end:
    return ret;
}

/****************************************************************/
/* $rtn_hdr_start  epon_request_onu_schedule_mode_set        */
/* CATEGORY   : Device                                          */
/* ACCESS     : Public                                          */
/* BLOCK      : SAL                                             */
/* CHIP       : 8030                                            */
cs_status epon_request_onu_schedule_mode_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_sdl_sched_type_t       sche_mode
)

{
    cs_aal_bm_weight_t sche;
    cs_uint32 index;
#ifdef HAVE_MPORTS
    rtk_qos_queue_weights_t rtk_que_wei;
#endif

    cs_status ret = CS_E_OK;

    if (sche_mode == SDL_SCHD_SP_MODE) {
        sche.mode = ONU_BM_SCHED_SP;

        for (index = 0; index < MAX_PRI_NUM; index++) {
            sche.weight[index] = 0;
        }
    } else if (sche_mode == SDL_SCHD_WRR_MODE) {
        sche.mode = ONU_BM_SCHED_WRR;
        sche.weight[0] = 1;
        for (index = 1; index < MAX_PRI_NUM; index++) {
            sche.weight[index] = (sche.weight[index-1]) * 2;
        }
    } else {
        ret = CS_E_PARAM;
        goto end;
    }

    ret = aal_bm_share_scheduler_set(AAL_PORT_ID_PON, &sche);
    if (ret) {
        goto end;
    }
    ret = aal_bm_share_scheduler_set(AAL_PORT_ID_GE, &sche);
    if (ret) {
        goto end;
    }
#ifdef HAVE_MPORTS
    for (index = 0; index < MAX_PRI_NUM -1; index++) {
        rtk_que_wei.weights[index] = sche.weight[index];
    }
    if (sche_mode == SDL_SCHD_SP_MODE) {
        rtk_que_wei.weights[MAX_PRI_NUM -1] = 0;
    } else {
        rtk_que_wei.weights[MAX_PRI_NUM -1] = 127;
    }
    for (index = 0; index < UNI_PORT_MAX; index++) {
        ret = rtk_qos_schedulingQueue_set(index, &rtk_que_wei);
        if (ret) {
            goto end;
        }
    }
#endif
end:
    return ret;
}

/****************************************************************/
/* $rtn_hdr_start  epon_request_onu_schedule_mode_get        */
/* CATEGORY   : Device                                          */
/* ACCESS     : Public                                          */
/* BLOCK      : SAL                                             */
/* CHIP       : 8030                                            */
cs_status epon_request_onu_schedule_mode_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_OUT cs_sdl_sched_type_t      *sche_mode
)

{
    cs_aal_bm_weight_t sche;

    cs_status ret = CS_E_OK;
    
    if(sche_mode == NULL){
        ret = CS_E_PARAM;
        goto end;
    }

    ret = aal_bm_share_scheduler_get(AAL_PORT_ID_PON, &sche);
    if(ret){
        goto end;
    }
    if(sche.mode == ONU_BM_SCHED_SP){
        *sche_mode = SDL_SCHD_SP_MODE;
    }else if(sche.mode == ONU_BM_SCHED_WRR){
        *sche_mode = SDL_SCHD_WRR_MODE;
    }else{
        ret = CS_E_PARAM;
    }
    
end:
    return ret;
}


