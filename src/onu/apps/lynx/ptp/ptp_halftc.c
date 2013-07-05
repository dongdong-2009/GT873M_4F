#include "packet.h"
#include "sdl_ptp.h"
#include "ptp_type.h"
#include "ptp_parse.h"
#include "ptp_lynx.h"
#include "ptp_cmn.h"

cs_uint64 g_sync_residenc_time = 0;
cs_uint64 g_sync_eg_time = 0;

cs_uint8 g_sync_eg_status = 0;
cs_uint16 g_sync_seq_id = 0;

cs_uint8 ptp_halftc_process(cs_pkt_t *pkt);

void halftc_enable()
{
    if(!g_ptp_tc_node.mode) {
        ptp_enable(1);
    }
    
    app_pkt_reg_handler(CS_PKT_PTP, ptp_halftc_process);
}

void ptp_halftc_sync_proc(cs_pkt_t *pkt)
{         
    cs_uint64 ing_time;
    
    if(pkt->port != CS_PON_PORT_ID) {  
        return ;
    }

    ptp_get_ing_time(pkt, &ing_time);
    PTP_LOG(IROS_LOG_LEVEL_INF, "sync ing_time: 0x%llx\n", ing_time);

    g_sync_seq_id = ptp_pkt_get_seq_id(pkt);
    PTP_LOG(IROS_LOG_LEVEL_INF, "sync seq_id = %d\n", g_sync_seq_id);
    
    /* move ptp info tailor to the head */
    ptp_move_info_tailer(pkt);    
    
    /* send out sync */
    ptp_send_pkt(CS_UNI_PORT_ID1, pkt->data + pkt->offset, pkt->len);

    if(ptp_get_eg_time(CS_UNI_PORT_ID1, &g_sync_eg_time)) {
        g_sync_eg_status = 0;
    }
    else {
        if(g_sync_eg_time < ing_time + 100) {
            PTP_LOG(IROS_LOG_LEVEL_MAJ, "get wrong sync eg time: 0x%llx\n", g_sync_eg_time);
        }
        else {
            g_sync_eg_status = 1;
            PTP_LOG(IROS_LOG_LEVEL_INF, "sync eg_time: 0x%llx\n", g_sync_eg_time);
        }
    }
}

void ptp_halftc_follow_up_proc(cs_pkt_t *pkt)
{
    cs_uint16 seq_id;
    cs_uint64 cf;
    
    if(!g_sync_eg_status || pkt->port != CS_PON_PORT_ID) {
        return;
    }        

    seq_id = ptp_pkt_get_seq_id(pkt);
    PTP_LOG(IROS_LOG_LEVEL_INF, "follow_up_seq_id = %d\n", seq_id);
    if(seq_id != g_sync_seq_id) {
        PTP_LOG(IROS_LOG_LEVEL_INF, "seq_id doesn't match the sync\n");
        return ;
    }

    /* remove ptp tailer, bypass ptp engine */
    ptp_remove_info_tailer(pkt);

    ptp_pkt_get_correctionField(pkt, &cf);
    cf = ntohll(cf);
    PTP_LOG(IROS_LOG_LEVEL_INF, "follow_up cf: 0x%llx\n", cf);

    /* modify correction field */
    cf += (g_sync_eg_time<<16);
    PTP_LOG(IROS_LOG_LEVEL_INF, "update cf of follow_up: 0x%llx\n", cf);
    if((cf>>48) & 0xffff) {
        PTP_LOG(IROS_LOG_LEVEL_MAJ, "cf(0x%llx) is not reasonable, discard it\n", cf);
    }
    else {
        cf = htonll(cf);

        ptp_pkt_set_correctionField(pkt, &cf);

        app_pkt_tx(CS_UNI_PORT_ID1, CS_PKT_PTP, pkt->data + pkt->offset, pkt->len);
    }

    /* start next round */
    g_sync_eg_status = 0;
}

void  ptp_halftc_pdelay_req_proc(cs_pkt_t *pkt)
{
    cs_uint64 pdelay_req_ing_time = 0;
    cs_uint64 pdelay_resp_eg_time = 0;
    cs_uint64 turnaround_time = 0;
    
    cs_uint8 *pdelay_resp_pdu = NULL;
    cs_uint8 *pdelay_resp_follow_up_pdu = NULL;
    
    if(pkt->port == CS_PON_PORT_ID) {
        return;
    }

    ptp_get_ing_time(pkt, &pdelay_req_ing_time);
    PTP_LOG(IROS_LOG_LEVEL_INF, "pdelay_req_ing_time: 0x%llx\n", pdelay_req_ing_time);

    ptp_move_info_tailer(pkt);    

    pdelay_resp_pdu = ptp_gen_pdelay_resp_pdu(pkt, 1);

    /* send out pdelay_resp */
    ptp_send_pkt(CS_UNI_PORT_ID1, pdelay_resp_pdu, pkt->len);

    if(ptp_get_eg_time(CS_UNI_PORT_ID1, &pdelay_resp_eg_time)) {
        iros_free(pdelay_resp_pdu);
        return ;
    }
    
    PTP_LOG(IROS_LOG_LEVEL_INF, "pdelay_resp_eg_time: 0x%llx\n", pdelay_resp_eg_time);

    turnaround_time = pdelay_resp_eg_time - pdelay_req_ing_time;
    if( (turnaround_time>>32) || ((turnaround_time>>52) & 0xfff) ) {
        PTP_LOG(IROS_LOG_LEVEL_MAJ, "turnaround_time is not reasonable: 0x%llx\n", turnaround_time);
    }
    else {
        PTP_LOG(IROS_LOG_LEVEL_INF, "turnaround_time: 0x%llx\n", turnaround_time);

        pdelay_resp_follow_up_pdu = ptp_gen_pdelay_resp_follow_up_pdu(pkt, turnaround_time);
        app_pkt_tx(CS_UNI_PORT_ID1, CS_PKT_PTP, pdelay_resp_follow_up_pdu, pkt->len - LYNX_PTP_INFO_LEN);
    }

    iros_free(pdelay_resp_follow_up_pdu);
    iros_free(pdelay_resp_pdu);
}

void ptp_halftc_mgmt_pkt_proc(cs_pkt_t *pkt)
{
    ptp_remove_info_tailer(pkt);
    app_pkt_tx(1-pkt->port, CS_PKT_PTP, pkt->data + pkt->offset, pkt->len);
}

cs_uint8 ptp_halftc_process(cs_pkt_t *pkt)
{
    ptp_msg_type_t type;
    
    /* add code to parser ptp pkt */
    type = ptp_pkt_get_msg_type(pkt);

    PTP_LOG(IROS_LOG_LEVEL_INF, "recv msg %d from port %d\n", type, pkt->port);

    switch(type) {
    
        case PTP_SYNC:
            ptp_halftc_sync_proc(pkt);
            break;

        case PTP_PDELAY_REQ:
            ptp_halftc_pdelay_req_proc(pkt);
            break;

        case PTP_FOLLOW_UP:
            ptp_halftc_follow_up_proc(pkt);
            break;

        case PTP_ANNOUNCE:
        case PTP_SIGNALING:
        case PTP_MANAGEMENT:
            ptp_halftc_mgmt_pkt_proc(pkt);
            break;

        default:    /* other msg drop */
            break;
    }
    
    return 1;
}

