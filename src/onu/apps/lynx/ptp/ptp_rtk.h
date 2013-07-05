#ifndef _PTP_RTK_H
#define _PTP_RTK_H

#define RTK_PTP_TS_LEN  8

#define RTK_PTP_TIME_SYNC_INTERVAL 10


cs_status sw_ptp_get_ing_time(cs_pkt_t *pkt, cs_uint64 *ing_time);
cs_status sw_ptp_get_eg_time(cs_port_id_t port, cs_uint64 *eg_time);
void ptp_remove_ts_info(cs_pkt_t *pkt);
void sw_ptp_time_sync();
void ptp_update_sw_time(cs_uint64 ing_time);

#endif /* #ifndef _PTP_RTK_H */

