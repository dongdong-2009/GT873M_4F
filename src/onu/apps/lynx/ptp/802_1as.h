#ifndef _802_1AS_H_
#define _802_1AS_H_

#include "oam_common.h"
#include "oam_std_pdu.h"
#include "ptp_type.h"

#define TIME_SYNC_SUBTYPE   0x0a
#define EPON_TQ_TIME    16
#define DEF_PPS_OFFSET  10000000
#define DEF_PPS_WIDTH   60000

#define NS_NUM_PER_SEC  1000000000

typedef struct {
    cs_uint32 cnt;
    cs_uint64 time;
} ptp_pon_tod_t;

extern ptp_pon_tod_t g_sys_time;

typedef struct {
    cs_uint8     dst_mac[OAM_MACADDR_LEN];
    cs_uint8     src_mac[OAM_MACADDR_LEN];
    cs_uint16   eth_type;
    cs_uint8     sub_type;
    cs_uint8     oui[3];
    cs_uint16    msg_id;
    cs_uint32   X;
    cs_uint16    tod_epoch_num;
    cs_uint32   tod_s;
    cs_uint32   tod_ns;
    cs_uint8     src_port[10];
    cs_uint8     log_msg_intv;
    double       rate_ratio;
    cs_uint16   gm_tm_base_ind;
    cs_uint8    last_gm_ph_chg[12];
    cs_uint8    scaled_last_gm_freq_chg[4];
} __attribute__((packed)) time_sync_pdu_t;

typedef struct {
    cs_uint8 start; /* $ */
    cs_uint8 msg_id[5];
    cs_uint8 utc_time[6];
    cs_uint8 day[2];
    cs_uint8 month[2];
    cs_uint8 year[4];
    cs_uint8 local_zone_hr[2];
    cs_uint8 local_zone_min[2];
    cs_uint8 end;   /* * */
    cs_uint8 cs[2];
    cs_uint8 cr;
    cs_uint8 lf;
} __attribute__((packed)) tod_info_t;

void onu_8021as_init();
cs_status onu_8021as_enable(cs_boolean enable);
cs_uint8 onu_check_8021as_pdu(oam_pdu_hdr_t *pdu);
void onu_8021as_set_pps_width(cs_uint16 width);
cs_uint16 onu_8021as_get_pps_width();


#endif  /* #ifndef _802_1AS_H_ */

