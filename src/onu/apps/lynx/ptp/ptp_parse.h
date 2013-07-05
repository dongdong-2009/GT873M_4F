#ifndef _PTP_PARSE_H
#define _PTP_PARSE_H

#include "ptp_type.h"

ptp_msg_type_t ptp_pkt_get_msg_type(cs_pkt_t *pkt);
cs_uint16 ptp_pkt_get_seq_id(cs_pkt_t *pkt);
void ptp_pkt_get_correctionField(cs_pkt_t *pkt, cs_uint64 *cf);
void ptp_pkt_set_correctionField(cs_pkt_t *pkt, cs_uint64 *cf);
cs_uint8 *ptp_gen_pdelay_resp_pdu(cs_pkt_t *pdelay_req, cs_uint8 two_step);
cs_uint8 *ptp_gen_pdelay_resp_follow_up_pdu(cs_pkt_t *pdelay_req, cs_uint64 time);

#endif /* #ifndef _PTP_PARSE_H */

