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

#include "aal_ptp.h"
#include "aal_util.h"
#include "aal.h"
#include "uart.h"

#define __PTP_COS_MAX                          (0x7)
#define __PTP_DPID_MAX                         (0x7)
#define __PTP_IPG_MAX                          (0x7)
#define __PTP_TX_FIFO_THRSHLD                  (0x1ff)
#define __PTP_CORRECT_DELTA_MAX                (0x7fff)
#define __TOD_CHAR_NUM_MAX                     (0x40)


cs_status aal_ptp_glb_cfg_set(cs_aal_ptp_glb_msk_t msk, cs_aal_ptp_glb_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(msk.u32 == 0){
        return CS_E_OK;
    }

    if(msk.s.ref_timer)
        REG_FIELD_WRITE(GLOBAL_PTP_LOCAL_TIMER_REF, val, cfg->ref_timer);

    if(msk.s.set_timestamp)
        REG_FIELD_WRITE(GLOBAL_PTP_SET_TIMESTAMP, val, cfg->set_timestamp);

    if(msk.s.set_timestamp_cmd)
        REG_FIELD_WRITE(GLOBAL_PTP_SET_TIMESTAMP_CMD, set, (cfg->set_timestamp_cmd>0));

    if(msk.s.get_timestamp_cmd)
        REG_FIELD_WRITE(GLOBAL_PTP_GET_TIMESTAMP_CMD, done, (cfg->get_timestamp_cmd>0));

    return CS_E_OK;

}

cs_status aal_ptp_glb_cfg_get(cs_aal_ptp_glb_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_FIELD_READ(GLOBAL_PTP_LOCAL_TIMER_REF, val, cfg->ref_timer);
    REG_FIELD_READ(GLOBAL_PTP_SET_TIMESTAMP, val, cfg->set_timestamp);
    REG_FIELD_READ(GLOBAL_PTP_GET_TIMESTAMP_CMD, done, cfg->get_timestamp_cmd);
    REG_FIELD_READ(GLOBAL_PTP_SET_TIMESTAMP_CMD, set, cfg->set_timestamp_cmd);

    return CS_E_OK;

}

cs_status aal_ptp_glb_status_get(cs_aal_ptp_glb_status_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_FIELD_READ(GLOBAL_PTP_GET_TIMESTAMP, val, cfg->get_timestamp);
    REG_FIELD_READ(GLOBAL_PTP_GET_MPCP_TIMESTAMP, val, cfg->get_mpcp_timestamp);

    return CS_E_OK;
}

cs_status aal_ptp_ge_cfg_set(cs_aal_ptp_ge_msk_t msk, cs_aal_ptp_ge_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(msk.u32 == 0){
        return CS_E_OK;
    }

    if(msk.s.tx_en)
        REG_FIELD_WRITE(GE_PTP_CONFIG, tx_bypass, ((cfg->tx_en>0)?0:1));

    if(msk.s.rx_en)
        REG_FIELD_WRITE(GE_PTP_CONFIG, rx_bypass, ((cfg->rx_en>0)?0:1));

    if(msk.s.egress_bswap_en)
        REG_FIELD_WRITE(GE_PTP_CONFIG, eg_lts_bswap, (cfg->egress_bswap_en>0));

    if(msk.s.ingress_bswap_en)
        REG_FIELD_WRITE(GE_PTP_CONFIG, ig_lts_bswap, (cfg->ingress_bswap_en>0));

    if(msk.s.packet_cos){
        if(cfg->packet_cos > __PTP_COS_MAX){
            AAL_MIN_LOG("packet_cos invalid.(%d) FILE: %s, LINE: %d\n", cfg->packet_cos, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(GE_PTP_CONFIG, packet_cos, (cfg->packet_cos));
    }

    if(msk.s.process_en)
        REG_FIELD_WRITE(GE_PTP_CONFIG, process_en, cfg->process_en);

    if(msk.s.correct_dir){
        if(cfg->correct_dir > AAL_PTP_CORRECT_DIR_ADD){
            AAL_MIN_LOG("correct_dir invalid.(%d) FILE: %s, LINE: %d\n", cfg->correct_dir, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(GE_PTP_ASYM_DELAY, sign, cfg->correct_dir);
    }

    if(msk.s.correct_delta){
        if(cfg->correct_delta > __PTP_CORRECT_DELTA_MAX){
            AAL_MIN_LOG("correct_delta invalid.(%d) FILE: %s, LINE: %d\n", cfg->correct_delta, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(GE_PTP_ASYM_DELAY, correction, cfg->correct_delta);
    }

    if(msk.s.tx_update_crc)
        REG_FIELD_WRITE(GE_ETH_MAC_CONFIG0, tx_update_crc, (cfg->tx_update_crc>0));

    if(msk.s.pass_thru_hdr)
        REG_FIELD_WRITE(GE_ETH_MAC_CONFIG0, pass_thru_hdre, (cfg->pass_thru_hdr>0));

    if(msk.s.txfifo_thrshld){
        if(cfg->txfifo_thrshld > __PTP_TX_FIFO_THRSHLD){
            AAL_MIN_LOG("txfifo_thrshld invalid.(%d) FILE: %s, LINE: %d\n", cfg->txfifo_thrshld, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(GE_ETH_MAC_CONFIG0, txfifo_thrshld, cfg->txfifo_thrshld);
    }

    if(msk.s.ipg_sel){
        if(cfg->ipg_sel > __PTP_IPG_MAX){
            AAL_MIN_LOG("ipg_sel invalid.(%d) FILE: %s, LINE: %d\n", cfg->ipg_sel, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(GE_ETH_MAC_CONFIG0, tx_ipg_sel, cfg->ipg_sel);
    }

    return CS_E_OK;

}



cs_status aal_ptp_ge_cfg_get(cs_aal_ptp_ge_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_FIELD_READ(GE_PTP_CONFIG, tx_bypass, cfg->tx_en);
    cfg->tx_en =(cfg->tx_en>0)? 0: 1;
    REG_FIELD_READ(GE_PTP_CONFIG, rx_bypass, cfg->rx_en);
    cfg->rx_en =(cfg->rx_en>0)? 0: 1;
    REG_FIELD_READ(GE_PTP_CONFIG, eg_lts_bswap, cfg->egress_bswap_en);
    REG_FIELD_READ(GE_PTP_CONFIG, ig_lts_bswap, cfg->ingress_bswap_en);
    REG_FIELD_READ(GE_PTP_CONFIG, packet_cos, cfg->packet_cos);
    REG_FIELD_READ(GE_PTP_CONFIG, process_en, cfg->process_en);
    REG_FIELD_READ(GE_PTP_ASYM_DELAY, sign, cfg->correct_dir);
    REG_FIELD_READ(GE_PTP_ASYM_DELAY, correction, cfg->correct_delta);

    REG_FIELD_READ(GE_ETH_MAC_CONFIG0, tx_update_crc, cfg->tx_update_crc);
    REG_FIELD_READ(GE_ETH_MAC_CONFIG0, pass_thru_hdre, cfg->pass_thru_hdr);
    REG_FIELD_READ(GE_ETH_MAC_CONFIG0, txfifo_thrshld, cfg->txfifo_thrshld);
    REG_FIELD_READ(GE_ETH_MAC_CONFIG0, tx_ipg_sel, cfg->ipg_sel);


    return CS_E_OK;
}

cs_status aal_ptp_ge_status_get(cs_aal_ptp_status_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_FIELD_READ(GE_PTP_ING_LATCH, local_time, cfg->ingress_timer_latch);
    REG_FIELD_READ(ONU_MAC_PTP_ING_FWD, local_time, cfg->ingress_timer_fwd);
    REG_FIELD_READ(GE_PTP_EG_LATCH, local_time, cfg->egress_timer_latch);
    REG_FIELD_READ(GE_PTP_PROCESS, cf_offset, cfg->cf_offset);
    REG_FIELD_READ(GE_PTP_PROCESS, message_type, cfg->msg_type);
    REG_FIELD_READ(GE_PTP_EG_STS, lt_valid, cfg->valid);

    return CS_E_OK;
}

cs_status aal_ptp_pon_cfg_set(cs_aal_ptp_pon_msk_t msk, cs_aal_ptp_pon_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(msk.u32 == 0){
        return CS_E_OK;
    }

    if(msk.s.tx_en)
        REG_FIELD_WRITE(ONU_MAC_PTP_CONFIG, ptp_tx_en, (cfg->tx_en>0));

    if(msk.s.rx_en)
        REG_FIELD_WRITE(ONU_MAC_PTP_CONFIG, ptp_rx_en, (cfg->rx_en>0));

    if(msk.s.egress_bswap_en)
        REG_FIELD_WRITE(ONU_MAC_PTP_CONFIG, eg_lts_bswap, (cfg->egress_bswap_en>0));

    if(msk.s.ingress_bswap_en)
        REG_FIELD_WRITE(ONU_MAC_PTP_CONFIG, ig_lts_bswap, (cfg->ingress_bswap_en>0));

    if(msk.s.packet_cos){
        if(cfg->packet_cos > __PTP_COS_MAX){
            AAL_MIN_LOG("packet_cos invalid.(%d) FILE: %s, LINE: %d\n", cfg->packet_cos, __FILE__, __LINE__);
            return CS_E_PARAM;
        }
        REG_FIELD_WRITE(ONU_MAC_PTP_CONFIG, packet_cos, (cfg->packet_cos));
    }

    if(msk.s.tx_crc_recal_en)
        REG_FIELD_WRITE(ONU_MAC_PTP_CONFIG, ptp_tx_crc_recal_en, (cfg->tx_crc_recal_en>0));

    if(msk.s.process_en)
        REG_FIELD_WRITE(ONU_MAC_PTP_CONFIG, process_en, cfg->process_en);

    if(msk.s.correct_dir){
        if(cfg->correct_dir > AAL_PTP_CORRECT_DIR_ADD){
            AAL_MIN_LOG("correct_dir invalid.(%d) FILE: %s, LINE: %d\n", cfg->correct_dir, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(ONU_MAC_PTP_ASYM_DELAY, sign, cfg->correct_dir);
    }

    if(msk.s.correct_delta){
        if(cfg->correct_delta > __PTP_CORRECT_DELTA_MAX){
            AAL_MIN_LOG("correct_delta invalid.(%d) FILE: %s, LINE: %d\n", cfg->correct_delta, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(ONU_MAC_PTP_ASYM_DELAY, correction, cfg->correct_delta);
    }

    if(msk.s.packet_dpid){
        if(cfg->packet_dpid > __PTP_DPID_MAX){
            AAL_MIN_LOG("packet_dpid invalid.(%d) FILE: %s, LINE: %d\n", cfg->packet_dpid, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(ONU_MAC_PTP_CONFIG, packet_dpid, cfg->packet_dpid);
    }

    return CS_E_OK;

}

cs_status aal_ptp_pon_cfg_get(cs_aal_ptp_pon_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_FIELD_READ(ONU_MAC_PTP_CONFIG, ptp_tx_en, cfg->tx_en);
    REG_FIELD_READ(ONU_MAC_PTP_CONFIG, ptp_rx_en, cfg->rx_en);
    REG_FIELD_READ(ONU_MAC_PTP_CONFIG, eg_lts_bswap, cfg->egress_bswap_en);
    REG_FIELD_READ(ONU_MAC_PTP_CONFIG, ig_lts_bswap, cfg->ingress_bswap_en);
    REG_FIELD_READ(ONU_MAC_PTP_CONFIG, packet_cos, cfg->packet_cos);
    REG_FIELD_READ(ONU_MAC_PTP_CONFIG, ptp_tx_crc_recal_en, cfg->tx_crc_recal_en);
    REG_FIELD_READ(ONU_MAC_PTP_CONFIG, process_en, cfg->process_en);
    REG_FIELD_READ(ONU_MAC_PTP_ASYM_DELAY, sign, cfg->correct_dir);
    REG_FIELD_READ(ONU_MAC_PTP_ASYM_DELAY, correction, cfg->correct_delta);
    REG_FIELD_READ(ONU_MAC_PTP_CONFIG, packet_dpid, cfg->packet_dpid);

    return CS_E_OK;
}

cs_status aal_ptp_pon_status_get(cs_aal_ptp_status_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_FIELD_READ(ONU_MAC_PTP_ING_LATCH, local_time, cfg->ingress_timer_latch);
    REG_FIELD_READ(GE_PTP_ING_FWD, local_time, cfg->ingress_timer_fwd);
    REG_FIELD_READ(ONU_MAC_PTP_EG_LATCH, local_time, cfg->egress_timer_latch);
    REG_FIELD_READ(ONU_MAC_PTP_PROCESS, cf_offset, cfg->cf_offset);
    REG_FIELD_READ(ONU_MAC_PTP_PROCESS, message_type, cfg->msg_type);
    REG_FIELD_READ(ONU_MAC_PTP_EG_STS, lt_valid, cfg->valid);

    return CS_E_OK;
}

cs_status aal_ptp_ma_cfg_set(cs_aal_ptp_ma_msk_t msk, cs_aal_ptp_ma_cfg_t *cfg)
{
    cs_uint8 data;

    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(msk.u32 == 0){
        return CS_E_OK;
    }

    if(msk.s.en){
        REG_FIELD_WRITE(MA_PTP_CTRL0, ptp_en, (cfg->en>0));
        REG_FIELD_WRITE(MA_CPU_CTRL, ptp_enable, (cfg->en>0));
    }

    if(msk.s.egress_bswap_en)
        REG_FIELD_WRITE(MA_CPU_CTRL, tx_byte_swap_en, (cfg->egress_bswap_en>0));

    if(msk.s.ingress_bswap_en)
        REG_FIELD_WRITE(MA_CPU_CTRL, rx_byte_swap_en, (cfg->ingress_bswap_en>0));

    if(msk.s.es_en)
        REG_FIELD_WRITE(MA_CPU_CTRL, es, (cfg->es_en>0));

    if(msk.s.lookup_bypass_en){
        REG_FIELD_READ(MA_CPU_CTRL, bypass, data);
        data &= 0xf;

        if(cfg->lookup_bypass_en){
            data |= (1<<2);
            REG_FIELD_WRITE(MA_CPU_CTRL, bypass, data);
        }
        else{
            data &= ~(1<<2);
            REG_FIELD_WRITE(MA_CPU_CTRL, bypass, data);
        }
    }

    if(msk.s.mir_cos){
        if(cfg->mir_cos>__PTP_COS_MAX){
            AAL_MIN_LOG("mir_cos invalid.(%d) FILE: %s, LINE: %d\n", cfg->mir_cos, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(MA_CPU_CTRL, mir_cos, cfg->mir_cos);
    }

    if(msk.s.mir_dpid){
        if(cfg->mir_dpid>__PTP_DPID_MAX){
            AAL_MIN_LOG("mir_dpid invalid.(%d) FILE: %s, LINE: %d\n", cfg->mir_dpid, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(MA_CPU_CTRL, mir_dpid, cfg->mir_dpid);
    }

    if(msk.s.cpu_cos){
        if(cfg->cpu_cos>__PTP_COS_MAX){
            AAL_MIN_LOG("cpu_cos invalid.(%d) FILE: %s, LINE: %d\n", cfg->cpu_cos, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(MA_CPU_CTRL, cpu_cos, cfg->cpu_cos);
    }

    if(msk.s.cpu_dpid){
        if(cfg->cpu_dpid>__PTP_DPID_MAX){
            AAL_MIN_LOG("cpu_dpid invalid.(%d) FILE: %s, LINE: %d\n", cfg->cpu_dpid, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(MA_CPU_CTRL, cpu_dpid, cfg->cpu_dpid);
    }

    if(msk.s.dma_cos){
        if(cfg->dma_cos>__PTP_COS_MAX){
            AAL_MIN_LOG("dma_cos invalid.(%d) FILE: %s, LINE: %d\n", cfg->dma_cos, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(MA_CPU_CTRL, dma_cos, cfg->dma_cos);
    }

    if(msk.s.dma_dpid){
        if(cfg->dma_dpid>__PTP_DPID_MAX){
            AAL_MIN_LOG("dma_dpid invalid.(%d) FILE: %s, LINE: %d\n", cfg->dma_dpid, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(MA_CPU_CTRL, dma_dpid, cfg->dma_dpid);
    }

    if(msk.s.mii0_cos){
        if(cfg->mii0_cos>__PTP_COS_MAX){
            AAL_MIN_LOG("mii0_cos invalid.(%d) FILE: %s, LINE: %d\n", cfg->mii0_cos, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(MA_CPU_CTRL, mii0_cos, cfg->mii0_cos);
    }

    if(msk.s.mii0_dpid){
        if(cfg->mii0_dpid>__PTP_DPID_MAX){
            AAL_MIN_LOG("mii0_dpid invalid.(%d) FILE: %s, LINE: %d\n", cfg->mii0_dpid, __FILE__, __LINE__);
            return CS_E_PARAM;
        }

        REG_FIELD_WRITE(MA_CPU_CTRL, mii0_dpid, cfg->mii0_dpid);
    }

    return CS_E_OK;

}

cs_status aal_ptp_ma_cfg_get(cs_aal_ptp_ma_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    //REG_FIELD_READ(MA_PTP_CTRL0, ptp_en, cfg->en);
    REG_FIELD_READ(MA_CPU_CTRL, ptp_enable, cfg->en);
    REG_FIELD_READ(MA_CPU_CTRL, tx_byte_swap_en, cfg->egress_bswap_en);
    REG_FIELD_READ(MA_CPU_CTRL, rx_byte_swap_en, cfg->ingress_bswap_en);
    REG_FIELD_READ(MA_CPU_CTRL, es, cfg->es_en);
    REG_FIELD_READ(MA_CPU_CTRL, bypass, cfg->lookup_bypass_en);
    REG_FIELD_READ(MA_CPU_CTRL, mir_cos, cfg->mir_cos);
    REG_FIELD_READ(MA_CPU_CTRL, mir_dpid, cfg->mir_dpid);
    REG_FIELD_READ(MA_CPU_CTRL, cpu_cos, cfg->cpu_cos);
    REG_FIELD_READ(MA_CPU_CTRL, cpu_dpid, cfg->cpu_dpid);
    REG_FIELD_READ(MA_CPU_CTRL, dma_cos, cfg->dma_cos);
    REG_FIELD_READ(MA_CPU_CTRL, dma_dpid, cfg->dma_dpid);
    REG_FIELD_READ(MA_CPU_CTRL, mii0_cos, cfg->mii0_cos);
    REG_FIELD_READ(MA_CPU_CTRL, mii0_dpid, cfg->mii0_dpid);

    return CS_E_OK;
}


cs_status aal_tod_cfg_set(cs_aal_tod_msk_t msk, cs_aal_tod_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(msk.u32 == 0){
        return CS_E_OK;
    }

    if(msk.s.en){
        REG_FIELD_WRITE(GLOBAL_PIN_CTRL, glb_ptp_1pps_seln, ((cfg->en?0:1)));

       /*to be verified*/
       /* // Disable PPS
          glb_ctrl = reg_read( LYNXD_GLOBAL_PIN_CTRL);
          glb_ctrl &= 0xffbfffff;
          glb_ctrl |= 0x400000;
          reg_write(LYNXD_GLOBAL_PIN_CTRL, glb_ctrl);


          // Enable PPS
            glb_ctrl = reg_read( LYNXD_GLOBAL_PIN_CTRL);
            glb_ctrl ^= 0x400000;
            reg_write(LYNXD_GLOBAL_PIN_CTRL, glb_ctrl);
       */
    }

    if(msk.s.s_pps)
        REG_FIELD_WRITE(ONU_MAC_TOD_S_PPS, s_pps, cfg->s_pps);

    if(msk.s.pps_width)
        REG_FIELD_WRITE(ONU_MAC_TOD_PPS_WID, pps_width, cfg->pps_width);

    if(msk.s.s_tod)
        REG_FIELD_WRITE(ONU_MAC_TOD_S_TOD, s_tod, cfg->s_tod);

    return CS_E_OK;

}

cs_status aal_tod_cfg_get(cs_aal_tod_cfg_t *cfg)
{
    if(NULL == cfg){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_FIELD_READ(GLOBAL_PIN_CTRL, glb_ptp_1pps_seln, cfg->en);
    cfg->en = (cfg->en?0:1);
    REG_FIELD_READ(ONU_MAC_TOD_S_PPS, s_pps, cfg->s_pps);
    REG_FIELD_READ(ONU_MAC_TOD_PPS_WID, pps_width, cfg->pps_width);
    REG_FIELD_READ(ONU_MAC_TOD_S_TOD, s_tod, cfg->s_tod);

    return CS_E_OK;

}

cs_status aal_tod_info_set(cs_uint8 len, cs_uint8 *tod)
{
    cs_uint32 tod_dword;
    cs_uint32 dword_num;
    cs_uint32 reg_addr;
    cs_uint8  idx;

    if(NULL==tod){
        AAL_MIN_LOG("NULL Pointer.\n");
        return CS_E_PARAM;
    }

    if(len>__TOD_CHAR_NUM_MAX){
        AAL_MIN_LOG("counter invalid.(%d)\n", len);
        return CS_E_PARAM;
    }

    if(0==len){
        return CS_E_OK;
    }

    /*RTL programmed tod_length=0 represents 1 byte*/
    REG_FIELD_WRITE(PER_TOD_CTRL, tod_counter, len-1);

    if(len%4)
        dword_num = len/4 + 1;
    else
        dword_num = len/4;

    for(idx=0; idx<dword_num; idx++){
        tod_dword = 0;
        if(len>=4){
            tod_dword = tod[idx*4] + (tod[idx*4+1] << 8) + (tod[idx*4+2] << 16)+ (tod[idx*4+3] << 24);
            len -= 4;
        }
        else{
            if(len==3){
                tod_dword = tod[idx*4] + (tod[idx*4+1] << 8) + (tod[idx*4+2] << 16);
            }
            else if (len==2) {
                tod_dword = tod[idx*4] + (tod[idx*4+1] << 8);
            }
            else if (len==1) {
                tod_dword = tod[idx*4];
            }
            else if (len==0) {
                AAL_MIN_LOG("TOD_info: Should not come here\n");
            }
        }
        /*Upper 64 Bytes of memory is assigned to TOD*/
        reg_addr = PER_BIW_MEM + 16 + idx;
        REG_WRITE(reg_addr, tod_dword);
    }

    return CS_E_OK;
}

cs_status aal_tod_info_get(cs_uint8 *len, cs_uint8 *tod)
{
    cs_uint32 tod_dword;
    cs_uint32 dword_num;
    cs_uint8  tod_cnt;
    cs_uint32 reg_addr;
    cs_uint8  idx;

    if((NULL==len) || (NULL==tod)){
        AAL_MIN_LOG("NULL Pointer.\n");
        return CS_E_PARAM;
    }

    REG_FIELD_READ(PER_TOD_CTRL, tod_counter, tod_cnt);
    /*RTL programmed tod_length=0 represents 1 byte*/
    *len = ++tod_cnt;

    if(tod_cnt%4)
        dword_num = tod_cnt/4 + 1;
    else
        dword_num = tod_cnt/4;

    for(idx=0; idx<dword_num; idx++){
        tod_dword = 0;
        /*Upper 64 Bytes of memory is assigned to TOD*/
        reg_addr = PER_BIW_MEM + 16 + idx;
        REG_READ(reg_addr, tod_dword);

        if(tod_cnt>=4){
            tod[idx*4+3] = (tod_dword >> 24) & 0xff;
            tod[idx*4+2] = (tod_dword >> 16) & 0xff;
            tod[idx*4+1] = (tod_dword >> 8) & 0xff;
            tod[idx*4] = tod_dword & 0xff;
            tod_cnt -= 4;
        }
        else{
            if(tod_cnt==3){
                tod[idx*4+2] = (tod_dword >> 16) & 0xff;
                tod[idx*4+1] = (tod_dword >> 8) & 0xff;
                tod[idx*4] = tod_dword & 0xff;
            }
            else if (tod_cnt==2) {
                tod[idx*4+1] = (tod_dword >> 8) & 0xff;
                tod[idx*4] = tod_dword & 0xff;
            }
            else if (tod_cnt==1) {
                tod[idx*4] = tod_dword & 0xff;
            }
            else if (tod_cnt==0) {
                AAL_MIN_LOG("TOD_info: Should not come here\n");
            }
        }
    }

    return CS_E_OK;
}

/*
This interface is only for configure the Tod Uart,
if used for other feature, need to confirm the implemetation.
*/
cs_status aal_tod_uart_set(uart_cfg_mask_t msk, uart_cfg_t *cfg)
{
    PER_UART4_CFG_t uart4;
    cs_uint16 baud_count = 0;

    if(NULL == cfg)
    {
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    if(msk.u32 == 0){
        return CS_E_OK;
    }

    REG_READ(PER_UART4_CFG, uart4.wrd);

    if(msk.s.data_bits)
    {
        if(7==cfg->data_bits)
        {
            uart4.bf.char_cnt = 2;
        }
        else if(8 == cfg->data_bits)
        {
            uart4.bf.char_cnt = 3;
        }
    }

    if (msk.s.stop_bits)
    {
        if(1==cfg->stop_bits)
        {
            uart4.bf.uart_stop_2bit = 0;
        }
        else if (2==cfg->stop_bits)
        {
            uart4.bf.uart_stop_2bit = 1;
        }
    }

    if(msk.s.parity)
    {
        if(UART_NONE_PARITY==cfg->parity)
        {
            uart4.bf.uart_parity_en = 0;
        }
        else if(UART_ODD_PARITY==cfg->parity)
        {
            uart4.bf.uart_parity_en = 1;
            uart4.bf.even_parity_sel = 0;
        }
        else if(UART_EVEN_PARITY==cfg->parity)
        {
            uart4.bf.uart_parity_en = 1;
            uart4.bf.even_parity_sel = 1;
        }
    }

    if(msk.s.baud_rate)
    {
        switch ( cfg->baud_rate )
        {
            case UART_BAUD_RATE_4800:
            case UART_BAUD_RATE_9600 :
            case UART_BAUD_RATE_19200 :
            case UART_BAUD_RATE_38400 :
            case UART_BAUD_RATE_115200 :
                baud_count = (62500000/cfg->baud_rate) - 1; /*Baud rate = (1 / ({baud_count_hi, baud_count} + 1)) * PER_CLK 62.5MHz*/
                uart4.bf.baud_count = baud_count;
                break;
            default:
                uart4.bf.baud_count = BAUD_COUNT_DEFAULT_4800;
        }
    }

    if(msk.s.enable)
    {
        uart4.bf.uart_en = 1;
        uart4.bf.tx_sm_enable = 1;
    }

    REG_WRITE(PER_UART4_CFG, uart4.wrd);
    return CS_E_OK;

}

cs_status aal_tod_uart_get(uart_cfg_t *cfg)
{
    PER_UART4_CFG_t uart_cfg;
    cs_uint32 baud_rate = 0;

    if(NULL == cfg)
    {
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    uart_cfg.wrd = 0;
    REG_READ(PER_UART4_CFG, uart_cfg.wrd);

    if(0 != uart_cfg.bf.baud_count)
    {
        baud_rate = 62500000/(uart_cfg.bf.baud_count+1);
    }
    cfg->baud_rate = baud_rate;

    return CS_E_OK;
}


cs_status aal_ptp_int_enable(cs_aal_ptp_int_msk_t mask)
{
    cs_uint32 int_en;

    if(mask.u32 == 0){
        return CS_E_OK;
    }

    REG_READ(GLOBAL_PTP_INTENABLE, int_en);
    int_en |= mask.u32;
    REG_WRITE(GLOBAL_PTP_INTENABLE, int_en);

    return CS_E_OK;
}

cs_status aal_ptp_int_disable(cs_aal_ptp_int_msk_t mask)
{
    cs_uint32 int_en;

    if(mask.u32 == 0){
        return CS_E_OK;
    }

    REG_READ(GLOBAL_PTP_INTENABLE, int_en);
    int_en &= (~mask.u32);
    REG_WRITE(GLOBAL_PTP_INTENABLE, int_en);

    return CS_E_OK;
}

cs_status aal_ptp_int_en_get(cs_aal_ptp_int_msk_t *mask)
{
    cs_uint32 int_en;

    if(NULL == mask){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_READ(GLOBAL_PTP_INTENABLE, int_en);
    mask->u32 = int_en;

    return CS_E_OK;
}

cs_status aal_ptp_int_status_get(cs_aal_ptp_int_msk_t* mask)
{

    cs_uint32 int_src;

    if(NULL == mask){
        AAL_MIN_LOG("NULL pointer. FILE: %s, LINE: %d\n", __FILE__, __LINE__);
        return CS_E_PARAM;
    }

    REG_READ(GLOBAL_PTP_INTERRUPT, int_src);
    mask->u32 = int_src;
    return CS_E_OK;
}

cs_status aal_ptp_int_status_clr(cs_aal_ptp_int_msk_t src)
{
    if(src.u32 == 0){
        return CS_E_OK;
    }

    REG_WRITE(GLOBAL_PTP_INTERRUPT, src.u32); /*write 1 to clear*/

    return CS_E_OK;
}
cs_status ptp_init(void)
{
    cs_uint32 glb_ctrl;

     /*enable PTP*/
    REG_READ(GLOBAL_CTRL, glb_ctrl);
    glb_ctrl |= 0x180; // enable ptp
    REG_WRITE(GLOBAL_CTRL, glb_ctrl);

    return CS_E_OK;
}


