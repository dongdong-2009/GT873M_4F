#ifndef _RCP_GWD_W_
#define _RCP_GWD_W_
#include "cs_types.h"
#include "rtk_api.h"
#include "cs_cmn.h"

#define NUM_PORTS_PER_SYSTEM 5
#define IROS_TICK_PER_SECOND 100

#define FLASH_GWD_RCG_SWITCH_CFG_MAX_SIZE	(11*1024)
#define FLASH_USER_DATA_MAX_SIZE     	(1024*20)
#define FLASH_GWD_RCG_SWITCH_CFG_OFFSET		(8*1024) 
/* port operational status */
typedef enum {
        EPON_PORT_OPER_DOWN = 0,
        EPON_PORT_OPER_UP,
} epon_port_oper_status_t;

typedef struct enet_format_s
{
	/* edge0 */
	cs_ulong32  en_mac_dhost_hi;
	/* edge1 */
	cs_uint16  en_mac_dhost_lo;
	cs_uint16  en_mac_shost_hi;
	/* edge2 */
	cs_ulong32  en_mac_shost_lo;
	/* edge3 */
	cs_uint16  _tpid;						/* Tag Prot ID (0x8100) */
	cs_uint16  en_tag_ctrl;				/* Tag control */
	/* edge4 */
	cs_uint16  en_pro_II;
	cs_uint16  en_ver;
	/* edge5 */
	cs_uint16  en_tot_len;
	cs_uint16  en_frag_id;
	/* edge6 */
	cs_uint16  en_flag;
	cs_uint8   en_ttl;
	cs_uint8   en_pro;						/* TCP/UDP */
	/* edge7 */
	cs_uint16  en_chk_sum;
	cs_uint16  en_ip_shost_hi;
	cs_uint16  en_ip_shost_lo;
	cs_uint16  en_ip_dhost_hi;
	cs_uint16  en_ip_dhost_lo;
	cs_uint16  en_l4_sport;
	cs_uint16  en_l4_dport;
} enet_format_t;
enum e_onu_frame_handle_result
{
    EPON_ONU_FRAME_NOT_HANDLED,
    EPON_ONU_FRAME_HANDLED
} ;

typedef enum e_onu_frame_handle_result epon_onu_frame_handle_result_t;
typedef epon_onu_frame_handle_result_t (*epon_special_frame_handler_t) (cs_uint32, cs_uint32, cs_uint8*);

//extern rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);
extern cs_status ctc_oam_eth_port_link_get_adapt(
        cs_port_id_t port,
        cs_uint8*  linkState);

extern cs_status epon_request_onu_mpcp_reg_status_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_OUT cs_boolean               *enable
);
extern cs_status onu_frame_send(cs_uint8 *frame,
                          cs_uint32 frame_len,
                          cs_port_id_t port_id,
                          cs_uint8  priority,
                          cs_uint8  mode);
extern cs_status epon_request_onu_frame_send (
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_uint8                  *frame_buf,
    CS_IN cs_uint16                 frame_len,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_uint8                  priority);
extern cs_status startup_config_read(cs_uint32 type, cs_uint32 len, cs_uint8 *buf);


#endif

