/*
 * gwd_eth_loop_detect.h
 *
 *  Created on: 2012-7-18
 *      Author: tommy
 */

#ifndef GWD_ETH_LOOP_DETECT_H_
#define GWD_ETH_LOOP_DETECT_H_



#include "sdl_vlan.h"

#define EPON_ETHER_MACADDR_LEN          6
#define IROS_MSG_GWD_PORT_LOOP_EVENT	1
#define MAX_VLAN_RULE_PER_PORT 8

typedef enum{
	GWD_ETH_PORT_LOOP_ALARM=1,
	GWD_ETH_PORT_LOOP_ALARM_CLEAR,
}GWD_ETH_LOOP_STATUS_T;

typedef struct{
	unsigned long int msgtype;
#if 0
	unsigned long int portid;
	unsigned long int vid;
#endif
	unsigned long int loopstatus;
}gwd_ethloop_msg_t;

/* ethernet frame header */
typedef struct {
        epon_macaddr_t          dst;
        epon_macaddr_t          src;
        cs_uint16           ethertype;
        cs_uint32           lb_port;
        cs_uint8            reserved[60];
} __attribute__((packed)) epon_ether_header_lb_t;

/* ethernet frame header with vlan */
typedef struct {
        epon_macaddr_t          dst;
        epon_macaddr_t          src;
        cs_uint16           tpid;
        cs_uint16           vlan;
        cs_uint16           ethertype;
        cs_uint32           lb_port;
        cs_uint8            flag;
        cs_uint8            reserved[59];
} __attribute__((packed)) epon_ether_header_lb_vlan_t;



/* type of OLT/ONU ports */
typedef enum {
        /* OLT NNI physical port */
        EPON_PORT_NNI = 1,
        /* OLT management physical port */
        EPON_PORT_MGMT,
        /* ONU UNI physical port */
        EPON_PORT_UNI,
        /* OLT/ONU PON physical port */
        EPON_PORT_PON,
        /* OLT aggregator logical port */
        EPON_PORT_AGGR,
        /* OLT/ONU LLID based logical port */
        EPON_PORT_LLID,
        /* OLT protected switch group port */
        EPON_PORT_PSG,
} cs_port_type;

typedef struct 
{        
    cs_vlan_id   vlan;
    cs_uint32    tagged_portmap;
    cs_uint32    untagged_portmap;
    cs_uint32    unmodified_portmap;
    cs_uint32    private_data;        
} cs_sw_vlan_config;

typedef struct {
        epon_macaddr_t          dst;
        epon_macaddr_t          src;
        cs_uint16           ethertype;
        cs_uint32           lb_port;
        cs_uint8            reserved[60];
} __attribute__((packed)) cs_ether_header_lb;

typedef struct {
        epon_macaddr_t          dst;
        epon_macaddr_t          src;
        cs_uint16           tpid;
        cs_uint16           vlan;
        cs_uint16           ethertype;
        cs_uint32           lb_port;
        cs_uint8            flag;
        cs_uint8            reserved[59];
} __attribute__((packed)) cs_ether_header_lb_vlan;

#if 0
int gwdEthPortLoopMsgBuildAndSend(unsigned long int portid, unsigned long int vid, unsigned long int status);
#else
int gwdEthPortLoopMsgBuildAndSend(unsigned int status);
int gwdEthPortLoopLedAction();
extern cs_status aal_pon_mac_addr_get( CS_IN cs_mac_t *mac);
extern cs_status aal_mpcp_llid_get(cs_uint16 *llid);
extern cs_uint8 oam_plat_get_switch_port_num();
extern void dumpPkt(char *comment, int port, unsigned char *buffer, int len);
extern cs_status epon_request_onu_fdb_entry_del_dynamic (
    CS_IN   cs_callback_context_t   context,
    CS_IN   cs_int32                device_id,
    CS_IN   cs_int32                llidport,
    CS_IN  cs_mac_t                      *mac,
    CS_IN  cs_uint16                     vlan
);
cs_status epon_request_onu_pon_mac_addr_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                device_id,
    CS_IN cs_int32                llidport,
    CS_OUT cs_mac_t                    *mac
);
cs_status epon_request_onu_mpcp_llid_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_OUT cs_uint16                *llid
);

#endif

#endif /* GWD_ETH_LOOP_DETECT_H_ */
