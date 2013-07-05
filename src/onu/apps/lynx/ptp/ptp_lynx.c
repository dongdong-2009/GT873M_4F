#include "packet.h"
#include "802_1as.h"
#include "ptp_lynx.h"
#include "sdl_ptp.h"
#include "sdl_ma.h"

/* asymmetric correction: measured by ns */
cs_uint32 g_rx_time_cr = 0;
cs_uint32 g_tx_time_cr = 640;

cs_uint8 g_ptp_ge_port_id[PTP_SRC_PORT_ID_SIZE] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };

cs_uint32 ptp_get_rx_time_correction()
{
    return g_rx_time_cr;
}

void ptp_set_rx_time_correction(cs_uint32 corr)
{
    g_rx_time_cr = corr;
}

cs_uint32 ptp_get_tx_time_correction()
{
    return g_tx_time_cr;
}

void ptp_set_tx_time_correction(cs_uint32 corr)
{
    g_tx_time_cr = corr;
}

/*
 * ASIC operations
 */

/* get ing_time before move ptp info tailor */
cs_status ptp_get_ing_time(cs_pkt_t *pkt, cs_uint64 *ing_time)
{
    cs_uint32 ing_cnt = 0;
    cs_uint8   *ing_ts = pkt->data + pkt->offset + pkt->len - 4;

    ing_cnt = ((cs_uint32)ing_ts[0]<<24) | ((cs_uint32)ing_ts[1]<<16) | 
        ((cs_uint32)ing_ts[2]<<8) | ing_ts[3] ;
    PTP_LOG(IROS_LOG_LEVEL_INF, "ing_cnt = 0x%08x\n", ing_cnt);
    *ing_time = g_sys_time.time + (ing_cnt - g_sys_time.cnt) * EPON_TQ_TIME;
    *ing_time += g_rx_time_cr;

    return CS_E_OK;
}

cs_status ptp_get_eg_time(cs_port_id_t port, cs_uint64 *eg_time)
{
    cs_callback_context_t  context;

    cs_uint32 eg_cnt = 0;
    cs_status rt = CS_E_OK;

    rt = epon_request_onu_ptp_capture_eg_timestamp(context, 0, 0, port, &eg_cnt);
    if(rt) {
        PTP_LOG(IROS_LOG_LEVEL_INF, "get eg_time not ready and try again\n");
        /* delay 1ms to retry */
        hal_delay_us(10); /* delay 10us */
        rt = epon_request_onu_ptp_capture_eg_timestamp(context, 0, 0, port, &eg_cnt);
    }

    if(CS_E_OK == rt) {
        PTP_LOG(IROS_LOG_LEVEL_INF, "eg_cnt = 0x%08x\n", eg_cnt);
        *eg_time = g_sys_time.time + (eg_cnt - g_sys_time.cnt) * EPON_TQ_TIME;
        *eg_time += g_tx_time_cr;
    }    

    return rt;
}

cs_status ptp_send_pkt(cs_port_id_t port, cs_uint8 *frame, cs_uint16 len)
{   
    cs_callback_context_t context;
    cs_status ret;

    ret = epon_request_onu_ptp_frame_send(context, 0, 0, frame, len, port, 7);

    if(CS_E_OK == ret) {
        /* dump pkt */
        extern cs_pkt_profile_t g_pkt_profile[CS_PKT_TYPE_NUM+1];
        if(g_pkt_profile[CS_PKT_PTP].trace) {       
            cs_uint8 cmt[CS_LOG_COMMENT_LEN];
            sprintf(cmt, "tx pkt type: %d, port: %d, len: %d", CS_PKT_PTP, port, len);
            cs_buffer_dump(cmt, frame, len);
        }
    }
    
    return ret;
}

/*
 * cortina PTP info operations
 */
 
void ptp_move_info_tailer(cs_pkt_t *pkt)
{
    cs_uint8 *dest = pkt->data + pkt->offset - LYNX_PTP_INFO_LEN;
    cs_uint8 *src = pkt->data + pkt->offset + pkt->len - LYNX_PTP_INFO_LEN;

    memcpy(dest, src, LYNX_PTP_INFO_LEN);

    pkt->offset -= LYNX_PTP_INFO_LEN;

    /* remove crc field */
    pkt->len -= 4;
}

void ptp_remove_info_tailer(cs_pkt_t *pkt)
{
    /* remove ptp info and crc field */
    pkt->len -= (LYNX_PTP_INFO_LEN + CRC_LEN);
}

void ptp_set_update_cf(cs_pkt_t *pkt, cs_uint8 enable)
{
    cs_uint8 *info = pkt->data + pkt->offset;

    info[0] = enable ? 0x04 : 0;
}

