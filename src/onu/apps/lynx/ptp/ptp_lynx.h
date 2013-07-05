#ifndef _PTP_LYNX_H
#define _PTP_LYNX_H

#include "ptp_pkt.h"

#define  CRC_LEN    4
#define  LYNX_PTP_INFO_LEN   6

typedef struct {
    cs_uint16 rsvd : 4,
        cf_update_int : 1,
        cf_update : 1,
        cf_offset : 6,
        msg_type : 4;
    cs_uint32 ing_time;
} __attribute__((packed)) cs_ptp_info_blk_t;

extern cs_uint8 g_ptp_ge_port_id[PTP_SRC_PORT_ID_SIZE];

cs_status ptp_get_ing_time(cs_pkt_t *pkt, cs_uint64 *ing_time);
cs_status ptp_get_eg_time(cs_port_id_t port, cs_uint64 *eg_time);
void ptp_move_info_tailer(cs_pkt_t *pkt);
void ptp_remove_info_tailer(cs_pkt_t *pkt);
cs_status ptp_send_pkt(cs_port_id_t port, cs_uint8 *frame, cs_uint16 len);


#endif /* #ifndef _PTP_LYNX_H */

