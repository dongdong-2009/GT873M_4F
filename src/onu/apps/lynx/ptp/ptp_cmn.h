#ifndef _PTP_CMN_H
#define _PTP_CMN_H

extern cs_uint8 g_sync_eg_status;
extern cs_uint16 g_sync_seq_id;

typedef struct {
    cs_uint8 mode;
    cs_uint8 two_step; 
    
    cs_uint8 clk_id[8];
    cs_uint8 num_ports;
    cs_uint8 delay_mech;    
    cs_uint8 primary_domain;    /* used for clock syntonization */   

    cs_uint32 pdelay_req_tmr;
} ptp_tc_node_t;

extern ptp_tc_node_t g_ptp_tc_node;

extern cs_status ptp_enable(cs_uint8 enable);

#endif /* #ifndef _PTP_CMN_H */

