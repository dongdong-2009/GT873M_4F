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
#include "sdl.h"
#include "sdl_int.h"
#include "sdl_ptp.h"
#include "aal_ptp.h"
#include "aal_util.h"
#include "aal_mpcp.h"
#include "uart.h"
#include "aal_l2.h"

#ifdef HAVE_MPORTS

#include "rtl8367b_asicdrv_eav.h"
#include "rtk_api.h"
#include "rtk_api_ext.h"
#include "sdl_peri_util.h"
#include "aal_cls.h"
#endif


#define __SDL_TOD_CHAR_NUM_MAX        (64)
#define __SDL_PTP_TIMER_SYNC_ADJUST   (4)
#define __SDL_PTP_REF_TIMER_CNT       (0x17D784)    /*25ms increment BEBC20*/

#define __PTP_ETHTYPE       0x88f7
#define __PKT_PTP_UP_RULE            9
#define __PKT_PTP_DN_RULE            10

cs_aal_ptp_pon_cfg_t  g_pon_cfg;
cs_aal_ptp_ge_cfg_t   g_ge_cfg;
cs_sdl_ptp_clk_src_t  g_ptp_timer_src = SDL_PTP_CLK_SRC_PTP;
static cs_uint8 g_count = 0;

static void __ptp_get_ts_handler(void)
{
    cs_aal_ptp_glb_status_t  glb_stats;
    cs_aal_ptp_glb_msk_t glb_msk;
    cs_aal_ptp_glb_cfg_t glb_cfg;
    cs_aal_ptp_int_msk_t int_mask;
    cs_uint32 timer_diff;
    cs_uint32 set_timer;
    cs_uint32 ref_timer;

    /*If get_timestamp_cmd != 1, return;*/
    aal_ptp_glb_cfg_get(&glb_cfg);
    if(glb_cfg.get_timestamp_cmd != 1)
        return;

    /*Get current timer: ptp time and mpcp timer*/
    aal_ptp_glb_status_get(&glb_stats);

    /*After read, Set get_timestamp_cmd =0, so that can get ptp timer and mpcp timer next time*/
    glb_msk.u32 = 0;
    glb_msk.s.get_timestamp_cmd = 1;
    glb_cfg.get_timestamp_cmd = 0;
    aal_ptp_glb_cfg_set(glb_msk, &glb_cfg);

    /*Get timer difference*/
    timer_diff = glb_stats.get_mpcp_timestamp - glb_stats.get_timestamp;
    SDL_MIN_LOG("Iter%0d: GotMPCPTimer=0x%08x GotPTPTimer=0x%08x Diff=%0d\n", g_count, glb_stats.get_mpcp_timestamp, glb_stats.get_timestamp, timer_diff);

    if(timer_diff == 0){
        /*disable interrupt*/
        int_mask.u32 = 0;
        int_mask.s.get_ts_intI = 1;
        int_mask.s.set_ts_intI = 1;
        aal_ptp_int_disable(int_mask);

        /*Clear interrupt src*/
        aal_ptp_int_status_clr(int_mask);

        /*Set set_timestamp_cmd =0*/
        glb_msk.u32 = 0;
        glb_msk.s.set_timestamp_cmd = 1;
        glb_msk.s.get_timestamp_cmd = 1;
        glb_cfg.set_timestamp_cmd = 0;
        glb_cfg.get_timestamp_cmd = 0;
        aal_ptp_glb_cfg_set(glb_msk, &glb_cfg);

        g_count = 0;
    }
    else{
        /*Set the new ptp reference time*/
        ref_timer = glb_stats.get_mpcp_timestamp + __SDL_PTP_REF_TIMER_CNT; /*200ms increment*/
        glb_msk.u32 = 0;
        glb_msk.s.ref_timer = 1;
        glb_cfg.ref_timer = ref_timer;

        set_timer = (g_count < 4) ? ref_timer : (ref_timer + timer_diff);


        /*Adjust the PTP timer to sync-up with MPCP timer*/
        glb_msk.s.set_timestamp_cmd = 1;
        glb_msk.s.set_timestamp = 1;
        glb_cfg.set_timestamp_cmd = 1;
        glb_cfg.set_timestamp = set_timer;
        aal_ptp_glb_cfg_set(glb_msk, &glb_cfg);
        SDL_MIN_LOG("Iter%0d: NewRefTimer=0x%08x SetPTPTimer=0x%08x\n", g_count, ref_timer, set_timer);

        g_count++;
    }

    return;
}

static void __ptp_set_ts_handler(void)
{
    cs_aal_ptp_glb_msk_t glb_msk;
    cs_aal_ptp_glb_cfg_t glb_cfg;
    cs_aal_ptp_int_msk_t int_mask;

    /*clear set timestamp status*/
    int_mask.u32 = 0;
    int_mask.s.set_ts_intI = 1;
    aal_ptp_int_status_clr(int_mask);

    /*clear set timestamp cmd*/
    glb_msk.u32 = 0;
    glb_msk.s.set_timestamp_cmd = 1;
    glb_cfg.set_timestamp_cmd = 0;
    aal_ptp_glb_cfg_set(glb_msk, &glb_cfg);

    return;
}

static void __ptp_timer_sync(void)
{
    cs_aal_ptp_glb_msk_t msk;
    cs_aal_ptp_glb_cfg_t cfg;
    cs_aal_ptp_int_msk_t int_mask;
    cs_uint32 mpcp_timer;
    cs_uint32 ref_timer;
    cs_boolean reg;

    if(g_ptp_timer_src == SDL_PTP_CLK_SRC_PTP){
        return;
    }

    aal_mpcp_reg_status_get(&reg);
    if(reg){
        SDL_MIN_LOG("ptp timer sync with mpcp for register!!!\n");

        /*enable interrupt*/
        int_mask.u32 = 0;
        int_mask.s.get_ts_intI = 1;
        int_mask.s.set_ts_intI = 1;
        /*clear interrupt*/
        aal_ptp_int_status_clr(int_mask);

        /*enable interrupt*/
        aal_ptp_int_enable(int_mask);

        /*get local mpcp timer*/
        REG_READ(ONU_MAC_LOCAL_TIMER, mpcp_timer);

        /*Set ptp reference timer */
        ref_timer = mpcp_timer + __SDL_PTP_REF_TIMER_CNT;
        msk.u32 =0;
        msk.s.get_timestamp_cmd = 1;
        msk.s.set_timestamp_cmd = 1;
        cfg.get_timestamp_cmd = 0;
        cfg.set_timestamp_cmd = 0;

        msk.s.ref_timer = 1;
        cfg.ref_timer = ref_timer;
        aal_ptp_glb_cfg_set(msk, &cfg);
    }
    else{
        SDL_MIN_LOG("ptp timer sync for de-register!!!\n");

        /*disable interrupt*/
        int_mask.u32 = 0;
        int_mask.s.get_ts_intI = 1;
        int_mask.s.set_ts_intI = 1;
        aal_ptp_int_disable(int_mask);

        /*Clear interrupt src*/
        aal_ptp_int_status_clr(int_mask);

        msk.u32 = 0;
        msk.s.set_timestamp_cmd = 1;
        msk.s.get_timestamp_cmd = 1;
        cfg.set_timestamp_cmd = 0;
        cfg.get_timestamp_cmd = 0;
        aal_ptp_glb_cfg_set(msk, &cfg);

        /*clear counter*/
        g_count = 0;
    }

    return;
}

cs_status epon_request_onu_ptp_cfg_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_IN  cs_uint16                 msg_type,
    CS_IN  cs_boolean                enable
)
{
    cs_aal_ptp_pon_msk_t  pon_msk;
    cs_aal_ptp_pon_cfg_t  pon_cfg;
    cs_aal_ptp_ge_msk_t   ge_msk;
    cs_aal_ptp_ge_cfg_t   ge_cfg;
    cs_status             rt = CS_E_OK;

    if(enable>1){
        rt = CS_E_PARAM;
        SDL_MIN_LOG("ERROR PARAM\n");
        goto END;
    }

    if(port_id > CS_UNI_PORT_ID4){
        rt = CS_E_PARAM;
        SDL_MIN_LOG("ERROR Port ID\n");
        goto END;
    }

    if(port_id == CS_PON_PORT_ID){
        memset(&pon_cfg, 0, sizeof(cs_aal_ptp_pon_cfg_t));
        memset(&pon_msk, 0, sizeof(cs_aal_ptp_pon_msk_t));

        pon_msk.u32 = 0;
        pon_msk.s.tx_en = 1;
        pon_msk.s.rx_en = 1;
        //pon_msk.s.egress_bswap_en = 1;
        //pon_msk.s.ingress_bswap_en = 1;
        pon_msk.s.packet_cos = 1;
        pon_msk.s.process_en = 1;
        pon_msk.s.tx_crc_recal_en = 1;
        pon_msk.s.packet_dpid = 1;

        if(enable){
            pon_cfg.tx_en = 1;
            pon_cfg.rx_en = 1;
            //pon_cfg.egress_bswap_en = 0;
            //pon_cfg.ingress_bswap_en = 0;
            pon_cfg.packet_cos = 2;    /*configured according to test scripts*/
            pon_cfg.tx_crc_recal_en = 0;
            pon_cfg.packet_dpid = 4; /*configured according to test scripts*/
        }
        else{
            memcpy(&pon_cfg, &g_pon_cfg, sizeof(cs_aal_ptp_pon_cfg_t));
        }
        pon_cfg.process_en = msg_type;
        rt = aal_ptp_pon_cfg_set(pon_msk, &pon_cfg);
        if(rt){
            SDL_MIN_LOG("aal_ptp_pon_cfg_set fail!\n");
            goto END;
        }
    }
    else{
        memset(&ge_cfg, 0, sizeof(cs_aal_ptp_ge_cfg_t));
        memset(&ge_msk, 0, sizeof(cs_aal_ptp_ge_msk_t));

        ge_msk.u32 = 0;
        ge_msk.s.tx_en = 1;
        ge_msk.s.rx_en = 1;
        //ge_msk.s.egress_bswap_en = 1;
        //ge_msk.s.ingress_bswap_en = 1;
        ge_msk.s.packet_cos = 1;
        ge_msk.s.process_en = 1;
        ge_msk.s.tx_update_crc = 1;
        ge_msk.s.pass_thru_hdr = 1;
        //ge_msk.s.txfifo_thrshld = 1;
        //ge_msk.s.ipg_sel = 1;

        if(enable){
            ge_cfg.tx_en = 1;
            ge_cfg.rx_en = 1;
            //ge_cfg.egress_bswap_en = 0;
            //ge_cfg.ingress_bswap_en = 0;
            ge_cfg.packet_cos = 1;  /*configured according to test scripts*/
            ge_cfg.tx_update_crc = 1; /*configured according to test scripts*/
            ge_cfg.pass_thru_hdr = 1;  /*configured according to test scripts*/
            //ge_cfg.txfifo_thrshld = 0x32; /*configured according to test scripts, sdl will overwrite this value*/
            //ge_cfg.ipg_sel = 0x1;         /*configured according to test scripts*/
        }
        else{
            memcpy(&ge_cfg, &g_ge_cfg, sizeof(cs_aal_ptp_ge_cfg_t));
        }
        ge_cfg.process_en = msg_type;
        rt = aal_ptp_ge_cfg_set(ge_msk, &ge_cfg);
        if(rt){
            SDL_MIN_LOG("aal_ptp_ge_cfg_set fail!\n");
            goto END;
        }
    }

END:
    return rt;

}

cs_status epon_request_onu_ptp_cfg_get(
    CS_IN   cs_callback_context_t     context,
    CS_IN   cs_int32                  device_id,
    CS_IN   cs_int32                  llidport,
    CS_IN   cs_port_id_t              port_id,
    CS_OUT  cs_uint16                 *msg_type,
    CS_OUT  cs_boolean                *enable
)
{
    cs_aal_ptp_pon_cfg_t  pon_cfg;
    cs_aal_ptp_ge_cfg_t   ge_cfg;
    cs_status             rt = CS_E_OK;

    if((NULL==enable)||(NULL==msg_type)){
        SDL_MIN_LOG("ERROR PARAM\n",);
        rt = CS_E_PARAM;
        goto END;
    }

    if(port_id > CS_UNI_PORT_ID4){
        rt = CS_E_PARAM;
        SDL_MIN_LOG("ERROR Port ID\n");
        goto END;
    }

    if(port_id == CS_PON_PORT_ID){
        rt = aal_ptp_pon_cfg_get(&pon_cfg);
        if(rt){
            SDL_MIN_LOG("aal_ptp_pon_cfg_get fail!\n");
            goto END;
        }

        *enable = pon_cfg.tx_en;
        *msg_type = pon_cfg.process_en;
    }
    else{
        rt = aal_ptp_ge_cfg_get(&ge_cfg);
        if(rt){
            SDL_MIN_LOG("aal_ptp_ge_cfg_get fail!\n");
            goto END;
        }

        *enable = ge_cfg.tx_en;
        *msg_type = ge_cfg.process_en;
    }


END:
    return rt;


}


/*Description,
port_id : PON port or GE port
correction: asymmetry correction value
sign: the sign of asymmetry correction
*/
cs_status epon_request_onu_ptp_asym_correction_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_IN  cs_sdl_ptp_correct_dir_t  direction,
    CS_IN  cs_uint16                 value
)
{
    cs_aal_ptp_pon_msk_t  pon_msk;
    cs_aal_ptp_pon_cfg_t  pon_cfg;
    cs_aal_ptp_ge_msk_t   ge_msk;
    cs_aal_ptp_ge_cfg_t   ge_cfg;
    cs_status             rt = CS_E_OK;

    if(direction>SDL_PTP_CORRECT_DIR_ADD){
        SDL_MIN_LOG("ERROR PARAM\n",);
        rt = CS_E_PARAM;
        goto END;
    }

    if(port_id > CS_UNI_PORT_ID4){
        rt = CS_E_PARAM;
        SDL_MIN_LOG("ERROR Port ID\n");
        goto END;
    }

    if(port_id == CS_PON_PORT_ID){
        pon_msk.u32 = 0;
        pon_msk.s.correct_dir = 1;
        pon_msk.s.correct_delta = 1;

        pon_cfg.correct_dir = direction;
        pon_cfg.correct_delta = value;

        rt = aal_ptp_pon_cfg_set(pon_msk, &pon_cfg);
        if(rt){
            SDL_MIN_LOG("aal_ptp_pon_cfg_set fail!\n");
            goto END;
        }
    }
    else{
        ge_msk.u32 = 0;
        ge_msk.s.correct_dir = 1;
        ge_msk.s.correct_delta = 1;

        ge_cfg.correct_dir = direction;
        ge_cfg.correct_delta = value;

        rt = aal_ptp_ge_cfg_set(ge_msk, &ge_cfg);
        if(rt){
            SDL_MIN_LOG(" aal_ptp_ge_cfg_set fail!\n");
            goto END;
        }
    }

END:
    return rt;

}

cs_status epon_request_onu_ptp_asym_correction_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_sdl_ptp_correct_dir_t  *direction,
    CS_OUT cs_uint16                 *value
)
{
    cs_aal_ptp_pon_cfg_t  pon_cfg;
    cs_aal_ptp_ge_cfg_t   ge_cfg;
    cs_status             rt = CS_E_OK;

    if((NULL==direction)||(NULL==value)){
        SDL_MIN_LOG(" ERROR PARAM\n",);
        rt = CS_E_PARAM;
        goto END;
    }

    if(port_id > CS_UNI_PORT_ID4){
        rt = CS_E_PARAM;
        SDL_MIN_LOG("ERROR Port ID\n");
        goto END;
    }

    if(port_id == CS_PON_PORT_ID){
        rt = aal_ptp_pon_cfg_get(&pon_cfg);
        if(rt){
            SDL_MIN_LOG("aal_ptp_pon_cfg_get fail!\n");
            goto END;
        }

        *direction = pon_cfg.correct_dir;
        *value = pon_cfg.correct_delta;
    }
    else{
        rt = aal_ptp_ge_cfg_get(&ge_cfg);
        if(rt){
            SDL_MIN_LOG("aal_ptp_ge_cfg_set fail!\n");
            goto END;
        }

        *direction = ge_cfg.correct_dir;
        *value = ge_cfg.correct_delta;
    }

END:
    return rt;
}

cs_status epon_request_onu_ptp_capture_eg_timestamp(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_uint32                 *timestamp
)
{
    cs_aal_ptp_status_t pon_status;
    cs_status           rt = CS_E_OK;

    if(NULL==timestamp){
        SDL_MIN_LOG("ERROR PARAM\n",);
        rt = CS_E_PARAM;
        goto END;
    }

    if(port_id > CS_UNI_PORT_ID4){
        rt = CS_E_PARAM;
        SDL_MIN_LOG("ERROR Port ID\n");
        goto END;
    }

    if(port_id == CS_PON_PORT_ID){
        cs_uint32 i;
        cs_uint32 ts;

        rt = aal_ptp_pon_status_get(&pon_status);
        if(rt){
            SDL_MIN_LOG(" aal_ptp_pon_status_get fail!\n");
            goto END;
        }

        if(!pon_status.valid) {
            rt = CS_E_ERROR;
            SDL_MIN_LOG("Timestamp is invalid\n");
            goto END;
        }

        for(i=0; i<100; i++) {
            REG_READ(ONU_MAC_PTP_EG_LATCH, ts);
        }

        if(ts != pon_status.egress_timer_latch) {
            SDL_MIN_LOG("the first ts is wrong(0x%08x)\n", pon_status.egress_timer_latch);
        }

        /* clear eg sts */
        REG_WRITE(ONU_MAC_PTP_EG_STS, 0x1);

        *timestamp = ts;
    }
    else {
        cs_uint32 i = 0;
        cs_uint32 ts0, ts1;

        cs_boolean ge_eg_valid = 0;

        do {
            REG_FIELD_READ(GE_PTP_EG_STS, lt_valid, ge_eg_valid);
            i++;
            if(i > 1000) {
                SDL_MAJ_LOG("ge eg is not ready!\n");
                return CS_E_ERROR;
            }
        } while(!ge_eg_valid);

        REG_READ(GE_PTP_EG_LATCH, ts0);
        hal_delay_us(10);
        REG_READ(GE_PTP_EG_LATCH, ts1);
        if(ts0 != ts1) {
            SDL_MAJ_LOG("ge eg latch is changed!\n");
        }
#if 0
        for(i=1; i<100; i++) {
            ts_tmp = ts;
            REG_READ(GE_PTP_EG_LATCH, ts);
            if(ts != ts_tmp) {
                SDL_MIN_LOG("%d: 0x%08x - 0x%08x\n", i, ts_tmp, ts);
            }
        }

        if(ts != ts_tmp) {
            hal_delay_us(1);
            SDL_MIN_LOG("read last time for ge eg latch reg\n");
            REG_READ(GE_PTP_EG_LATCH, ts);
            ts_tmp = ts;
            REG_READ(GE_PTP_EG_LATCH, ts);
            if(ts != ts_tmp) {
                cs_printf("get eg time fail!\n");
                rt = CS_E_ERROR;
            }
        }
#endif

        /* clear eg sts */
        REG_WRITE(GE_PTP_EG_STS, 0x1);

        for(i=0; i<1000; i++) {
            REG_FIELD_READ(GE_PTP_EG_STS, lt_valid, ge_eg_valid);
            if(ge_eg_valid) {
                SDL_MAJ_LOG("ge_eg_valid is set\n");
                return CS_E_ERROR;
            }
        }

        *timestamp = ts1;
    }

END:
    return rt;


}

cs_status epon_request_onu_ptp_latch_ing_timestamp(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_uint32                 *timestamp
)
{
    cs_aal_ptp_status_t pon_status;
    cs_aal_ptp_status_t ge_status;
    cs_status           rt = CS_E_OK;

    if(NULL==timestamp){
        SDL_MIN_LOG(" ERROR PARAM\n",);
        rt = CS_E_PARAM;
        goto END;
    }

    if(port_id > CS_UNI_PORT_ID4){
        rt = CS_E_PARAM;
        SDL_MIN_LOG("ERROR Port ID\n");
        goto END;
    }

    if(port_id == CS_PON_PORT_ID){
        rt = aal_ptp_pon_status_get(&pon_status);
        if(rt){
            SDL_MIN_LOG("aal_ptp_pon_status_get fail!\n");
            goto END;
        }

        *timestamp = pon_status.ingress_timer_latch;

    }
    else{
        rt = aal_ptp_ge_status_get(&ge_status);
        if(rt){
            SDL_MIN_LOG("aal_ptp_ge_status_get fail!\n");
            goto END;
        }

        *timestamp = ge_status.ingress_timer_latch;
    }

END:
    return rt;

}

/*Parameter description,
enable -- 1pps+ToD signal TX enable or not
pps_width - 1pps signal width*/
cs_status epon_request_onu_tod_cfg_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_boolean                enable
)
{
    cs_status rt = CS_E_OK;
    cs_aal_tod_msk_t msk;
    cs_aal_tod_cfg_t cfg;

    if(enable>1){
        SDL_MIN_LOG("enable %d is not support!\n", enable);
        rt = CS_E_PARAM;
        goto end;
    }

    msk.u32 = 0;
    msk.s.en = 1;
    cfg.en = enable;

    rt = aal_tod_cfg_set(msk, &cfg);
    if(rt){
        SDL_MIN_LOG("aal_tod_cfg_set fail!\n");
        goto end;
    }

end:
    return rt;

}

cs_status epon_request_onu_tod_cfg_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_OUT cs_boolean                *enable
)
{
    cs_status rt = CS_E_OK;
    cs_aal_tod_cfg_t cfg;

    if(NULL==enable){
        SDL_MIN_LOG("NULL poniter!\n");
        rt = CS_E_PARAM;
        goto end;
    }

    rt = aal_tod_cfg_get(&cfg);
    if(rt){
        SDL_MIN_LOG("aal_tod_cfg_get fail!\n");
        goto end;
    }

    *enable = cfg.en;

end:
    return rt;

}


cs_status epon_request_onu_tod_info_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_uint8                  len,
    CS_IN  cs_uint8                  *tod
)
{
    if(NULL==tod){
        SDL_MIN_LOG("NULL pointer\n");
        return CS_E_PARAM;
    }

    if(len>__SDL_TOD_CHAR_NUM_MAX){
        SDL_MIN_LOG("Error Param\n");
        return CS_E_PARAM;
    }

    return(aal_tod_info_set(len, tod));
}


cs_status epon_request_onu_tod_info_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_OUT cs_uint8                  *len,
    CS_OUT cs_uint8                  *tod
)
{
    if((NULL==len) ||(NULL==tod)){
        SDL_MIN_LOG("NULL pointer\n");
        return CS_E_PARAM;
    }

    return(aal_tod_info_get(len, tod));
}


/*s_pps - the value of local timer to trigger 1pps signal
Trigger to send out ToD signal
*/
cs_status epon_request_onu_tod_trigger_pps_signal(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_uint32                 s_pps,
    CS_IN  cs_uint16                 pps_width
)
{
    cs_status rt = CS_E_OK;
    cs_aal_tod_msk_t msk;
    cs_aal_tod_cfg_t cfg;

    msk.u32 =0;
    msk.s.s_pps = 1;
    msk.s.pps_width = 1;

    cfg.s_pps = s_pps;
    cfg.pps_width = pps_width;

    rt = aal_tod_cfg_set(msk, &cfg);
    if(rt){
        SDL_MIN_LOG("aal_tod_cfg_set fail!\n");
        goto end;
    }

end:
    return rt;


}


/*
cs_status epon_request_onu_tod_trigger_tod_signal(cs_uint32 s_tod);
s_tod - the value of local timer to trigger ToD signal
*/
cs_status epon_request_onu_tod_trigger_tod_signal(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_uint32                 s_tod
)
{
    cs_status rt = CS_E_OK;
    cs_aal_tod_msk_t msk;
    cs_aal_tod_cfg_t cfg;

    msk.u32 =0;
    msk.s.s_tod = 1;
    cfg.s_tod = s_tod;

    rt = aal_tod_cfg_set(msk, &cfg);
    if(rt){
        SDL_MIN_LOG("aal_tod_cfg_set fail!\n");
        goto end;
    }

end:
    return rt;

}

/*
baud_rate - Baud rate refer to ctc 2.1 standard, the baud rate should be 4800,9600,19200,38400, defaut as 4800
*/
cs_status epon_request_onu_tod_uart_cfg_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_uint32                 baud_rate
)
{
    uart_cfg_mask_t mask;
    uart_cfg_t cfg = {0};
    cs_status ret = CS_E_OK;

    if((UART_BAUD_RATE_4800 > baud_rate)||(UART_BAUD_RATE_38400 < baud_rate))
    {
        SDL_MIN_LOG("baud_rate %d is out of range!\n", baud_rate);
        ret = CS_E_PARAM;
        goto end;
    }

    mask.u32 = 0;
    mask.s.data_bits = 1;
    mask.s.stop_bits = 1;
    mask.s.parity = 1;
    mask.s.baud_rate = 1;
    mask.s.enable = 1;

    cfg.data_bits = 8;
    cfg.stop_bits = 1;
    cfg.parity = 0;
    cfg.baud_rate = baud_rate;
    cfg.enable = 1;

    ret = aal_tod_uart_set(mask, &cfg);
    if(ret)
    {
        SDL_MIN_LOG("aal_tod_uart_set baud rate fail! baud_rate[%d]\n", baud_rate);
        goto end;
    }

end:
    return ret;
}

cs_status epon_request_onu_tod_uart_cfg_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_OUT cs_uint32                 *baud_rate
)
{
    uart_cfg_t cfg = {0};
    cs_status ret = CS_E_OK;

    if(NULL == baud_rate)
    {
        SDL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        ret = CS_E_PARAM;
        goto end;
    }

    ret = aal_tod_uart_get(&cfg);
    if(ret)
    {
        SDL_MIN_LOG("aal_tod_uart_set baud rate fail! ret[%d] \n", ret);
        goto end;
    }

    *baud_rate = cfg.baud_rate;
end:
    return ret;
}

cs_status epon_request_onu_ptp_clk_src_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_sdl_ptp_clk_src_t      src
)
{
    cs_aal_ptp_int_msk_t int_mask;
    cs_aal_ptp_glb_msk_t glb_msk;
    cs_aal_ptp_glb_cfg_t glb_cfg;


    if(src >SDL_PTP_CLK_SRC_MPCP){
        return CS_E_PARAM;
    }

    g_ptp_timer_src = src;

    SDL_MIN_LOG("ptp timer src select from : %d\n", g_ptp_timer_src);

    if(src == SDL_PTP_CLK_SRC_MPCP){
        __ptp_timer_sync();
    }
    else{
        /*disable interrupt*/
        int_mask.u32 = 0;
        int_mask.s.get_ts_intI = 1;
        int_mask.s.set_ts_intI = 1;
        aal_ptp_int_disable(int_mask);

        /*Clear interrupt src*/
        aal_ptp_int_status_clr(int_mask);

        glb_msk.u32 = 0;
        glb_msk.s.set_timestamp_cmd = 1;
        glb_cfg.set_timestamp_cmd = 0;
        glb_msk.s.get_timestamp_cmd = 1;
        glb_cfg.get_timestamp_cmd = 0;
        aal_ptp_glb_cfg_set(glb_msk, &glb_cfg);

        /*clear counter*/
        g_count = 0;
    }

    return CS_E_OK;

}

// filter all PTP message from MII0 port to CPU
cs_status epon_request_onu_mgmt_port_ptp_state_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_uint8                            enable
)
{
    cs_aal_pkt_spec_udf_msk_t udf_msk;
    cs_aal_pkt_spec_udf_t     udf_cfg;

    cs_aal_spec_pkt_ctrl_msk_t   pkt_msk;
    cs_aal_spec_pkt_ctrl_t       pkt_cfg;

    cs_status ret = CS_E_OK;
    
    udf_msk.u32 = 0;
    udf_msk.s.udf2_etype = 1; 

    memset(&udf_cfg, 0, sizeof(cs_aal_pkt_spec_udf_t));
    udf_cfg.udf2_etype = __PTP_ETHTYPE;

    ret = aal_pkt_special_udf_set(AAL_PORT_ID_MII0, udf_msk, &udf_cfg);
    if(ret != CS_E_OK)
        return ret;
        
    pkt_msk.u32 = 0;
    pkt_msk.s.dpid = 1;
    pkt_msk.s.cos        = 1;
    //pkt_msk.s.rate_limit = 1;
    //pkt_msk.s.bypass_plc = 1;
    
    memset(&pkt_cfg, 0, sizeof(cs_aal_spec_pkt_ctrl_t));
    
    if(enable == 0)
    {
        pkt_cfg.dpid.dst_op = AAL_SPEC_DST_FE;
        pkt_cfg.dpid.dpid = AAL_PORT_ID_CPU;
        pkt_cfg.cos        = 0;      
        //pkt_cfg.flow_en    = FALSE; 
        //pkt_cfg.flow_id    = 0;
        //pkt_cfg.bypass_plc = FALSE;
    }
    else
    {
        pkt_cfg.dpid.dst_op = AAL_SPEC_DST_PORT;
        pkt_cfg.dpid.dpid = AAL_PORT_ID_CPU;
        pkt_cfg.cos        = 1;      
        //pkt_cfg.flow_en    = TRUE; 
        //pkt_cfg.flow_id    = (AAL_RATE_LIMIT_FLOW_28-AAL_RATE_LIMIT_FLOW_0);
        //pkt_cfg.bypass_plc = FALSE;
    }
    
    ret = aal_special_pkt_behavior_set(AAL_PORT_ID_MII0, AAL_PKT_UDF2, pkt_msk, &pkt_cfg);
    return ret;
}


#ifdef HAVE_MPORTS
cs_status epon_request_onu_ptp_sw_port_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_IN  cs_boolean                enable)
{
    rtk_uint32 port;
    rtk_api_ret_t rtk_ret;

    if((port_id<CS_UNI_PORT_ID1) || ((port_id>CS_UNI_PORT_ID4)&&(port_id!=CS_UPLINK_PORT))){
        SDL_MIN_LOG("ERROR PARAM\n");
        return CS_E_PARAM;
    }

    if(enable>1){
        SDL_MIN_LOG("ERROR PARAM\n");
        return CS_E_PARAM;
    }

    port = (port_id == CS_UPLINK_PORT) ? SWITCH_UPLINK_PORT : (port_id - 1);

    rtk_ret = rtl8367b_setAsicEavEnable(port, enable);
    if(rtk_ret != RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_setAsicEavEnable return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

    rtk_ret = rtl8367b_setAsicEavTimeSyncEn(port, enable);
    if(rtk_ret != RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_setAsicEavTimeSyncEn return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

    return CS_E_OK;

}

cs_status epon_request_onu_ptp_capture_sw_eg_timestamp(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_sdl_ptp_timestamp_t    *timestamp)
{
    rtk_uint32 eg_second = 0;
    rtk_uint32 eg_ns = 0;
    rtk_uint32 valid = 0;
    rtk_uint32 port;
    rtk_api_ret_t rtk_ret = RT_ERR_OK;

    if((port_id<CS_UNI_PORT_ID1) || ((port_id>CS_UNI_PORT_ID4)&&(port_id!=CS_UPLINK_PORT))){
        SDL_MIN_LOG("ERROR PARAM\n");
        return CS_E_PARAM;
    }

    port = (port_id == CS_UPLINK_PORT) ? SWITCH_UPLINK_PORT : (port_id - 1);

    rtk_ret = rtl8367b_getAsicEavTimeSyncValid(port, &valid);
    if(rtk_ret != RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_getAsicEavTimeSyncValid return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

    if(!valid){
        SDL_MIN_LOG("Timestamp is invalid\n");
        return CS_E_ERROR;
    }

    rtk_ret = rtl8367b_getAsicEavEgressTimestamp512ns(port, &eg_ns);
    if(rtk_ret != RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_getAsicEavEgressTimestamp512ns return %d\n", rtk_ret);
        return CS_E_ERROR;
    }
    timestamp->nano_second = eg_ns;

    rtk_ret = rtl8367b_getAsicEavEgressTimestampSeccond(port, &eg_second);
    if(rtk_ret!=RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_getAsicEavEgressTimestampSeccond return %d. FILE: %s, LINE: %d", rtk_ret, __FILE__, __LINE__);
        return CS_E_ERROR;
    }

    timestamp->second = eg_second;

    return CS_E_OK;

}

cs_status epon_request_onu_ptp_update_sw_time(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_sdl_ptp_time_dir_t     direction,
    CS_IN  cs_sdl_ptp_timestamp_t    timestamp)
{

    rtk_api_ret_t rtk_ret = RT_ERR_OK;
    rtk_uint32 second = 0;
    rtk_uint32 nano_second = 0;

    if(direction>SDL_PTP_TIME_ADJUST_DIR_ADD){
        SDL_MIN_LOG("ERROR PARAM\n");
        return CS_E_PARAM;
    }

    if(direction == SDL_PTP_TIME_ADJUST_DIR_ADD){
        second = timestamp.second;
        nano_second = timestamp.nano_second;
    }
    else{
        second = 0xffffffff - timestamp.second + 1;
        nano_second = 0x1fffff - timestamp.nano_second + 1;
    }

    rtk_ret = rtl8367b_setAsicEavTimeOffsetSeccond(second);
    if(rtk_ret != RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_setAsicEavTimeOffsetSeccond return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

    rtk_ret = rtl8367b_setAsicEavTimeOffset512ns(nano_second);
    if(rtk_ret != RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_setAsicEavTimeOffset512ns return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

    rtk_ret = rtl8367b_setAsicEavOffsetTune(EPON_TRUE);
    if(rtk_ret!=RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_setAsicEavOffsetTune return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

    return CS_E_OK;

}

/*
    verify if the time jitter can meet requirement if RTL8305 doesn't stamp on the PTP messages, leave everything to LynxD.
a.	Solution: Test only one port(For example P1)
a)	Configure an ACL in RTL8305, all PTP message received by the uplink port should be forwarded to P1.
b)	Configure RTL8305, all PTP message should be treated as the highest priority flow.
c)	Configure LynxD, all PTP message should be treated as the highest priority flow.

*/
cs_uint8 g_ptp_jitter_test_enable = 0;

cs_status epon_request_onu_ptp_jitter_test (
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_uint8                  enable
)
{
    cs_status rt = CS_E_OK;

#ifdef HAVE_MPORTS
    rtk_filter_cfg_t   cfg;
    rtk_filter_action_t act;
    cs_uint32 ruleNum;
    rtk_filter_field_t field;

    cs_aal_cl_rule_cfg_t ce_entry;
    cs_aal_cl_fib_data_t fibdata;

    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));
    memset(&field, 0, sizeof(rtk_filter_field_t));

    memset(&ce_entry, 0, sizeof(cs_aal_cl_rule_cfg_t));
    memset(&fibdata, 0, sizeof(cs_aal_cl_fib_data_t));

    if(g_ptp_jitter_test_enable == enable)
        return CS_E_OK;

    if(enable == 1)
    {
        // PTP message: downstream traffic, redirect to port 0, with highest priority
        field.fieldType = FILTER_FIELD_ETHERTYPE;
        field.filter_pattern_union.etherType.dataType = FILTER_FIELD_DATA_MASK;
        field.filter_pattern_union.etherType.value = __PTP_ETHTYPE;
        field.filter_pattern_union.etherType.mask = 0xFFFF;
        rt = rtk_filter_igrAcl_field_add(&cfg, &field);
        if(rt){
            cs_printf("rtk_filter_igrAcl_field_add failed, rt %d\r\n", rt);
            return rt;
        }

        cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
        cfg.activeport.value = 0x40;
        cfg.activeport.mask = 0xFF;
        cfg.invert = FALSE;
        act.actEnable[FILTER_ENACT_REDIRECT] = TRUE;
        act.filterRedirectPortmask = 0x01;
        act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;
        act.filterPriority = 7;
        rt = rtk_filter_igrAcl_cfg_add(__PKT_PTP_DN_RULE, &cfg, &act, &ruleNum);
        if(rt){
            cs_printf("rtk_filter_igrAcl_cfg_add downstream rule failed, rt %d\r\n", rt);
            return rt;
        }

        // PTP message: upstream traffic, highest priority
        memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
        memset(&act, 0, sizeof(rtk_filter_action_t));

        field.fieldType = FILTER_FIELD_ETHERTYPE;
        field.filter_pattern_union.etherType.dataType = FILTER_FIELD_DATA_MASK;
        field.filter_pattern_union.etherType.value = __PTP_ETHTYPE;
        field.filter_pattern_union.etherType.mask = 0xFFFF;
        rt = rtk_filter_igrAcl_field_add(&cfg, &field);
        if(rt){
            cs_printf("rtk_filter_igrAcl_field_add failed, rt %d\r\n", rt);
            return rt;
        }

        cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
        cfg.activeport.value = 0x01;
        cfg.activeport.mask = 0xFF;
        cfg.invert = FALSE;
        act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;
        act.filterPriority = 7;
        rt = rtk_filter_igrAcl_cfg_add(__PKT_PTP_UP_RULE, &cfg, &act, &ruleNum);
        if(rt){
            cs_printf("rtk_filter_igrAcl_cfg_add upstream rule failed, rt %d\r\n", rt);
            return rt;
        }

        // PTP message, highest priority for both upstream and downstream traffic
        ce_entry.valid = 1;
        ce_entry.key_type = AAL_CL_MAC_ETYPE_KEY;
        ce_entry.cl_key.mac_etype_key[0].mask.s.ev2pt  = 1;
        ce_entry.cl_key.mac_etype_key[0].ev2pt         = 0x88f7;

        fibdata.valid             = 1;
        fibdata.permit            = 1;
        fibdata.mask.u32 = 0;
        fibdata.mask.s.cos        = 1;
        fibdata.cos_sel           = AAL_CL_COS_SEL_CL;
        fibdata.cos               = 7;

        rt = aal_cl_rule_set(AAL_PORT_ID_GE, DOMAIN_ID_MAC_ETYPE, 0, &ce_entry) ;
        if(rt){
            cs_printf("aal_cl_rule_set upstream rule failed, rt %d\r\n", rt);
            return rt;
        }

        rt = aal_cl_fib_set(AAL_PORT_ID_GE, DOMAIN_ID_MAC_ETYPE, 0, &fibdata) ;
        if(rt){
            cs_printf("aal_cl_fib_set upstream fib failed, rt %d\r\n", rt);
            return rt;
        }

        rt = aal_cl_rule_set(AAL_PORT_ID_PON, DOMAIN_ID_MAC_ETYPE, 0, &ce_entry) ;
        if(rt){
            cs_printf("aal_cl_rule_set downstream rule failed, rt %d\r\n", rt);
            return rt;
        }

        rt = aal_cl_fib_set(AAL_PORT_ID_PON, DOMAIN_ID_MAC_ETYPE, 0, &fibdata) ;
        if(rt){
            cs_printf("aal_cl_fib_set downstream fib failed, rt %d\r\n", rt);
            return rt;
        }


    }
     else
     {
        rt = rtk_filter_igrAcl_cfg_del(__PKT_PTP_UP_RULE);
        if(rt){
            cs_printf("rtk_filter_igrAcl_cfg_del upstream rule failed, rt %d\r\n", rt);
            return rt;
        }

        rt = rtk_filter_igrAcl_cfg_del(__PKT_PTP_DN_RULE);
        if(rt){
            cs_printf("rtk_filter_igrAcl_cfg_del downstream rule failed, rt %d\r\n", rt);
            return rt;
        }

        memset(&ce_entry, 0, sizeof(cs_aal_cl_rule_cfg_t));
        ce_entry.key_type = AAL_CL_MAC_ETYPE_KEY;
        rt = aal_cl_rule_set(AAL_PORT_ID_GE, DOMAIN_ID_MAC_ETYPE, 0, &ce_entry) ;
        if(rt){
            cs_printf("aal_cl_rule_set upstream rule failed, rt %d\r\n", rt);
            return rt;
        }

        memset(&fibdata, 0, sizeof(cs_aal_cl_fib_data_t));
        rt = aal_cl_fib_set(AAL_PORT_ID_GE, DOMAIN_ID_MAC_ETYPE, 0, &fibdata) ;
        if(rt){
            cs_printf("aal_cl_fib_set upstream fib failed, rt %d\r\n", rt);
            return rt;
        }

        memset(&ce_entry, 0, sizeof(cs_aal_cl_rule_cfg_t));
        ce_entry.key_type = AAL_CL_MAC_ETYPE_KEY;
        rt = aal_cl_rule_set(AAL_PORT_ID_PON, DOMAIN_ID_MAC_ETYPE, 0, &ce_entry) ;
        if(rt){
            cs_printf("aal_cl_rule_set downstream rule failed, rt %d\r\n", rt);
            return rt;
        }

        memset(&fibdata, 0, sizeof(cs_aal_cl_fib_data_t));
        rt = aal_cl_fib_set(AAL_PORT_ID_PON, DOMAIN_ID_MAC_ETYPE, 0, &fibdata) ;
        if(rt){
            cs_printf("aal_cl_fib_set downstream fib failed, rt %d\r\n", rt);
            return rt;
        }
     }
#endif
    if(rt == CS_E_OK)
        g_ptp_jitter_test_enable = enable;

    return rt;

}

#endif

cs_status sdl_ptp_init(void)
{
    cs_status rt = CS_E_OK;

    memset(&g_pon_cfg, 0, sizeof(cs_aal_ptp_pon_cfg_t));
    memset(&g_ge_cfg, 0, sizeof(cs_aal_ptp_ge_cfg_t));

    rt = aal_ptp_pon_cfg_get(&g_pon_cfg);
    if(rt){
        SDL_MIN_LOG("aal_ptp_pon_cfg_get fail!\n");
    }

    rt = aal_ptp_ge_cfg_get(&g_ge_cfg);
    if(rt){
        SDL_MIN_LOG("aal_ptp_ge_cfg_get fail!\n");
    }

    sdl_int_handler_reg(SDL_INT_GLB_PTP_TS_GET, __ptp_get_ts_handler);
    sdl_int_handler_reg(SDL_INT_GLB_PTP_TS_SET, __ptp_set_ts_handler);
    sdl_int_handler_reg(SDL_INT_REG_CHANGE, __ptp_timer_sync);

#ifdef HAVE_MPORTS

    /*workaroud for second in ptp flag is not updated.
    config PORT 5 to loopback */
    cs_uint32 data;
    rtk_mac_t mac;
    cs_callback_context_t     context;
    rtk_api_ret_t rtk_ret;

    cs_plat_ssp_switch_read(context, 0, 0, 0x1305, &data);
    data |= 0x1;
    cs_plat_ssp_switch_write(context, 0, 0, 0x1305, data);

    cs_plat_ssp_switch_write(context, 0, 0, 0x1310, 0x1076);
    cs_plat_ssp_switch_write(context, 0, 0, 0xae, 0x48f0);

    /*make sure this address is not used.
    this address should aligned with the packet send to port5*/
    mac.octet[0] = 0x01;
    mac.octet[1] = 0x80;
    mac.octet[2] = 0xc2;
    mac.octet[3] = 0x00;
    mac.octet[4] = 0x00;
    mac.octet[5] = 0x20;
    rtk_ret = rtk_trap_rmaAction_set(&mac, RMA_ACTION_TRAP2CPU);
    if(rtk_ret != RT_ERR_OK){
        SDL_MIN_LOG("rtk_trap_rmaAction_set return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

    rtk_ret =  rtl8367b_setAsicEavTimeFreq(1280);
    if(rtk_ret != RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_getAsicEavTimeFreq return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

    rtk_ret = rtl8367b_setAsicEavOffsetTune(EPON_TRUE);
    if(rtk_ret!=RT_ERR_OK){
        SDL_MIN_LOG("rtl8367b_setAsicEavOffsetTune return %d\n", rtk_ret);
        return CS_E_ERROR;
    }

#endif

    return rt;
}



