#include "plat_common.h"
#include "osal_api_packet.h"
#include "ptp_type.h"
#include "ptp_pkt.h"
#include "ptp_lynx.h"

ptp_msg_type_t ptp_pkt_get_msg_type(cs_pkt_t *pkt)
{
    cs_uint8 *hdr = (pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t)); /* eth_hdr + vlan_tag */

    cs_uint8 msg_type = *hdr & 0xf;
    
    return msg_type;
}

cs_uint16 ptp_pkt_get_seq_id(cs_pkt_t *pkt)
{
    cs_uint16 seq_id = 0;
    cs_uint8 *hdr = (pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t)); /* eth_hdr + vlan_tag */

    hdr += PTP_SEQ_ID_OFFSET;

    seq_id = hdr[0];
    seq_id = (seq_id << 8);
    seq_id |= hdr[1];

    return seq_id;
}

void ptp_pkt_get_correctionField(cs_pkt_t *pkt, cs_uint64 *cf)
{
    cs_uint8 *cf_ptr = (pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t)) + PTP_CF_OFFSET;

    memcpy((cs_uint8 *)cf, cf_ptr, PTP_CF_SIZE);
}

void ptp_pkt_set_correctionField(cs_pkt_t *pkt, cs_uint64 *cf)
{
    cs_uint8 *cf_ptr = (pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t)) + PTP_CF_OFFSET;

    memcpy(cf_ptr, (cs_uint8 *)cf, PTP_CF_SIZE);
}

cs_uint8 ptp_pkt_get_2step_flag(cs_pkt_t *pkt)
{
    cs_uint8 *flag_ptr = (pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t)) + PTP_FLAG_OFFSET;

    return (*flag_ptr >> 1) & 0x1;
    
}

void ptp_pkt_set_2step_flag(cs_pkt_t *pkt)
{
    cs_uint8 *flag_ptr = (pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t)) + PTP_FLAG_OFFSET;

    *flag_ptr &= 0x2;
    
}

cs_uint8 *ptp_gen_pdelay_resp_pdu(cs_pkt_t *pdelay_req, cs_uint8 two_step)
{
    cs_uint32 eth_hdr_len = 0;     
    cs_uint8 *msg_type_ptr = NULL;
    cs_uint8 *req_port_id_ptr = NULL;
    cs_uint8 *src_port_id_ptr = NULL;
    cs_uint8 *gen_src_port_id_ptr = NULL;
    cs_uint8 *sa_ptr = NULL;
    cs_uint8 *flag0_ptr = NULL;

    cs_uint8 *frame = iros_malloc(IROS_MID_PTP, pdelay_req->len);

    eth_hdr_len = sizeof(cs_ether_header_t) + pdelay_req->tag_num*sizeof(cs_vlan_hdr_t);

    /* copy pdelay_req */
    memcpy(frame, pdelay_req->data + pdelay_req->offset, pdelay_req->len);

    /* modify sa */
    sa_ptr = frame + LYNX_PTP_INFO_LEN + 6;
    startup_config_read(CFG_ID_MAC_ADDRESS, 6, sa_ptr);

    msg_type_ptr = frame + LYNX_PTP_INFO_LEN + eth_hdr_len + PTP_MSG_TYPE_OFFSET;
    *msg_type_ptr = PTP_PDELAY_RESP;

    flag0_ptr = frame + LYNX_PTP_INFO_LEN + eth_hdr_len + PTP_FLAG_OFFSET;
    if(two_step) {
        *flag0_ptr |= 0x02;
    }
    else {
        *flag0_ptr &= 0xfd;
    }

    /* set src port id */
    gen_src_port_id_ptr = frame + eth_hdr_len + LYNX_PTP_INFO_LEN + PTP_SRC_PORT_ID_OFFSET;
    memcpy(gen_src_port_id_ptr, g_ptp_ge_port_id, PTP_SRC_PORT_ID_SIZE);

    /* copy requestingPortIdentity */
    req_port_id_ptr = frame + eth_hdr_len + LYNX_PTP_INFO_LEN + PTP_HDR_SIZE + PTP_TIMESTAMP_SIZE;
    src_port_id_ptr = (pdelay_req->data + pdelay_req->offset + eth_hdr_len)
        + LYNX_PTP_INFO_LEN + PTP_SRC_PORT_ID_OFFSET;
    memcpy(req_port_id_ptr, src_port_id_ptr, PTP_PORT_ID_SIZE);
    
    return frame;
}

cs_uint8 *ptp_gen_pdelay_resp_follow_up_pdu(cs_pkt_t *pdelay_req, cs_uint64 time)
{
    cs_uint32 eth_hdr_len = 0;    
    cs_uint8 *msg_type_ptr = NULL;
    cs_uint8 *req_port_id_ptr = NULL;
    cs_uint8 *src_port_id_ptr = NULL;
    cs_uint8 *gen_src_port_id_ptr = NULL;
    cs_uint8 *cf_ptr = NULL;
    cs_uint64 cf = 0;
    cs_uint8 *sa_ptr = NULL;

    cs_uint8 *frame = iros_malloc(IROS_MID_PTP, pdelay_req->len - LYNX_PTP_INFO_LEN);

    eth_hdr_len = sizeof(cs_ether_header_t) + pdelay_req->tag_num*sizeof(cs_vlan_hdr_t);

    /* copy pdelay_req */
    memcpy(frame, pdelay_req->data + pdelay_req->offset + LYNX_PTP_INFO_LEN, 
                    pdelay_req->len - LYNX_PTP_INFO_LEN);

    /* modify sa */
    sa_ptr = frame + 6;
    startup_config_read(CFG_ID_MAC_ADDRESS, 6, sa_ptr);

    msg_type_ptr = frame + eth_hdr_len + PTP_MSG_TYPE_OFFSET;
    *msg_type_ptr = PTP_PDELAY_RESP_FOLLOW_UP;

    /* set src port id */
    gen_src_port_id_ptr = frame + eth_hdr_len + PTP_SRC_PORT_ID_OFFSET;
    memcpy(gen_src_port_id_ptr, g_ptp_ge_port_id, PTP_SRC_PORT_ID_SIZE);

    /* copy requestingPortIdentity */
    req_port_id_ptr = frame + eth_hdr_len + PTP_HDR_SIZE + PTP_TIMESTAMP_SIZE; 
    src_port_id_ptr = (pdelay_req->data + pdelay_req->offset + eth_hdr_len)
        + LYNX_PTP_INFO_LEN + PTP_SRC_PORT_ID_OFFSET;
    memcpy(req_port_id_ptr, src_port_id_ptr, PTP_PORT_ID_SIZE);

    cf_ptr = frame + eth_hdr_len + PTP_CF_OFFSET;
    memcpy((cs_uint8 *)&cf, cf_ptr, PTP_CF_SIZE);
    cf = ntohll(cf);    

    cf += (time<<16);
    PTP_LOG(IROS_LOG_LEVEL_INF, "update pdelay_resp_follow_up cf: 0x%llx\n", cf);
    
    cf = htonll(cf);
    memcpy(cf_ptr, (cs_uint8 *)&cf, PTP_CF_SIZE);

    return frame;
}

#if 0
cs_uint8 ptp_check_2step_flag(cs_pkt_t *pkt)
{
    ptp_cmn_msg_hdr_t *hdr = (ptp_cmn_msg_hdr_t *)(pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t));

    if(hdr->flagField & 1) {
        return 1;
    }
    else {
        return 0;
    }
    
}

void ptp_set_2step_flag(cs_pkt_t *pkt)
{
    ptp_cmn_msg_hdr_t *hdr = (ptp_cmn_msg_hdr_t *)(pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t));

    hdr->flagField |= 1;
    
}

cs_uint16 ptp_get_seq_id(cs_pkt_t *pkt)
{
    ptp_cmn_msg_hdr_t *hdr = (ptp_cmn_msg_hdr_t *)(pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t));

    return hdr->sequenceId;
    
}

cs_uint64 ptp_get_cf_field(cs_pkt_t *pkt)
{
    ptp_cmn_msg_hdr_t *hdr = (ptp_cmn_msg_hdr_t *)(pkt->data + pkt->offset 
        + sizeof(cs_ether_header_t) + pkt->tag_num*sizeof(cs_vlan_hdr_t));

    return ntohll(hdr->correctionField);
}

void ptp_set_cf_field(ptp_cmn_msg_hdr_t *pdu, cs_uint32 res_time)
{
    cs_uint64 cf;
    
    cf = res_time << (16 + 4);
    pdu->correctionField = htonll(cf);
}

void ptp_add_cf_field(ptp_cmn_msg_hdr_t *pdu, cs_uint32 res_time)
{
    cs_uint64 cf;
    
    cf = ntohll(pdu->correctionField);
    cf += res_time << (16 + 4);
    
    pdu->correctionField = htonll(cf);
}

void ptp_set_cf_update(cs_pkt_t *pkt, cs_uint8 enable)
{
    cs_ptp_info_blk_t *info = (cs_ptp_info_blk_t *)(pkt->data + pkt->offset);

    info->cf_update = enable;
}

void ptp_set_cf_update_int(cs_pkt_t *pkt, cs_uint8 enable)
{
    cs_ptp_info_blk_t *info = (cs_ptp_info_blk_t *)(pkt->data + pkt->offset);

    info->cf_update_int = enable;
}

cs_uint8 *ptp_gen_follow_up_pdu(cs_pkt_t *sync, cs_uint32 res_time)
{
    cs_uint8 *frame = iros_malloc(IROS_MID_PTP, 
        sync->len - sizeof(cs_ptp_info_blk_t) - sizeof(ptp_sync_pdu_t) + sizeof(ptp_follow_up_pdu_t));

    cs_uint8 *eth_hdr = sync->data + sync->offset + sizeof(cs_ptp_info_blk_t);
    cs_uint32 offset = 0;

    ptp_follow_up_pdu_t *follow_up_pdu = NULL;

    /* copy ether hdr and vlan tag */
    offset = sizeof(cs_ether_header_t) + sync->tag_num*sizeof(cs_vlan_hdr_t);
    memcpy(frame, eth_hdr, offset + sizeof(ptp_sync_pdu_t));
    frame += offset;

    /* update correction */
    follow_up_pdu = (ptp_follow_up_pdu_t *)frame;
    ptp_set_cf_field((ptp_cmn_msg_hdr_t *)follow_up_pdu, res_time);

    return frame;
}

cs_uint8 *ptp_gen_pdelay_req_pdu(cs_pkt_t *pdelay_req)
{
    cs_uint8 *frame = iros_malloc(IROS_MID_PTP, pdelay_req->len);
    cs_ptp_info_blk_t *info = (cs_ptp_info_blk_t *)frame;
    ptp_pdelay_resp_pdu_t *pdu = (ptp_pdelay_resp_pdu_t *)(frame + sizeof(cs_ether_header_t) + 
        pdelay_req->tag_num*sizeof(cs_vlan_hdr_t) + sizeof(cs_ptp_info_blk_t));

    /* copy pdelay_req */
    memcpy(frame, pdelay_req->data + pdelay_req->offset, pdelay_req->len);

    /* copy requestingPortIdentity */
    memcpy(pdu->requestingPortIdentity, pdu->hdr.sourcePortIdentity, 10);

    info->msg_type = PTP_PDELAY_RESP;

    return frame;
}

cs_uint8 *ptp_gen_pdelay_resp_follow_up_pdu(cs_pkt_t *pdelay_req, cs_uint32 residence)
{
    cs_uint8 *frame = iros_malloc(IROS_MID_PTP, pdelay_req->len);
    cs_ptp_info_blk_t *info = (cs_ptp_info_blk_t *)frame;
    ptp_pdelay_resp_follow_up_pdu_t *pdu = (ptp_pdelay_resp_follow_up_pdu_t *)(frame + sizeof(cs_ether_header_t) + 
        pdelay_req->tag_num*sizeof(cs_vlan_hdr_t) + sizeof(cs_ptp_info_blk_t));
    cs_uint64 cf;

    /* copy pdelay_req */
    memcpy(frame, pdelay_req->data + pdelay_req->offset, pdelay_req->len);

    /* copy requestingPortIdentity */
    memcpy(pdu->requestingPortIdentity, pdu->hdr.sourcePortIdentity, 10);

    info->msg_type = PTP_PDELAY_RESP_FOLLOW_UP;

    cf = residence << (16 + 4);
    pdu->hdr.correctionField = htonll(cf);

    return frame;
}
#endif

