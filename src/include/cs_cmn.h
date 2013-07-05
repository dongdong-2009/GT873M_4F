#ifndef _CS_CMN_H_
#define _CS_CMN_H_

#include "cs_types.h"

#define CS_PKT_OFFSET   48

typedef enum {
    CS_PKT_BPDU = 0,
    CS_PKT_8021X = 1,
    CS_PKT_GMP = 2,   
    CS_PKT_ARP = 3,
    CS_PKT_OAM = 4,
    CS_PKT_MPCP = 5,
    CS_PKT_DHCP = 6,
    CS_PKT_IROS = 7,
    CS_PKT_PPPOE = 8, /* PPPOE Disc/session */
    CS_PKT_IP = 9,
    CS_PKT_NDP = 10,
    CS_PKT_LOOPDETECT = 11,
    CS_PKT_MYMAC = 12,
    CS_PKT_PTP = 13,
    CS_PKT_RCP = 14,
    CS_PKT_GWD_LOOPDETECT = 15,
    CS_PKT_TYPE_NUM 
} cs_pkt_type_t;

typedef struct _cs_pkt_t
{
    cs_uint8    msg_type;
    cs_uint8    version;
    cs_uint16   len;
    cs_uint16   port;
    cs_uint16   offset;
    cs_uint16   svlan;
    cs_uint16   cvlan;
    cs_uint16   stpid;
    cs_uint16   ctpid;
    cs_uint8     frame_type; 
    cs_uint8     pkt_type;
    cs_uint16   eth_type;
    cs_uint16   tag_num;
    cs_uint16   u2Reserved;
    cs_uint8    data[1];
} cs_pkt_t;

typedef struct {
    /* invocation identifier */
    cs_uint32           invoke_id;
    /* the opaque application data */
    void *                  apps_data;
    void*                   apps_handler;
} cs_callback_context_t;


#endif

