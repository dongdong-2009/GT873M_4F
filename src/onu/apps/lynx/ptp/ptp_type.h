#ifndef _PTP_TYPE_H
#define _PTP_TYPE_H

#include "cs_types.h"
#include "cs_cmn.h"

#define PTP_ETHER_TYPE  0x88f7

typedef enum {
    PTP_DISABLE,
    PTP_HALF_TC_MODE,
    PTP_TC_MODE,
    PTP_BC_MODE
} ptp_mode_t;

typedef struct {
    cs_uint64 sec;
    cs_uint32 nsec;
} ptp_time_t;

typedef enum {
    PTP_SYNC = 0,
    PTP_DELAY_REQ = 1,
    PTP_PDELAY_REQ = 2,
    PTP_PDELAY_RESP = 3,
    PTP_FOLLOW_UP = 8,
    PTP_DELAY_RESP = 9,
    PTP_PDELAY_RESP_FOLLOW_UP = 0x0a,
    PTP_ANNOUNCE = 0x0b,
    PTP_SIGNALING = 0x0c, 
    PTP_MANAGEMENT = 0x0d,
    PTP_TIME_SYNC = 0x0e,
    PTP_ILLEGAL_MSG = 0xffff
} ptp_msg_type_t;

typedef enum {
    PTP_DELAY_REQ_RESP_MECH,
    PTP_PEER_DELAY_MECH
} ptp_delay_mech_t;

typedef struct {
    cs_uint8 clk_id[8];
    cs_uint8 num_ports;
    cs_uint8 delay_mech;    
    cs_uint8 primary_domain; 
} ptp_tc_def_ds_t;

typedef struct {
    /* static member */     
    cs_uint8 port_id[10];    

    /* dynamic members */    
    cs_uint8 log_min_pdelay_req_intv;
    cs_uint8 faulty_flag;
    cs_uint64 peer_mean_path_delay;
} ptp_tc_port_ds_t;

#define PTP_LOG(level,String...)      cs_log_msg(level, IROS_MID_PTP, String)

#endif /* #ifndef _PTP_TYPE_H */

