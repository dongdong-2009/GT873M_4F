#ifndef __GW_IGMP_TVM_H__
#define	__GW_IGMP_TVM_H__

#if 0
typedef unsigned char	cs_uint8;
typedef unsigned short	cs_uint16;
typedef unsigned int	cs_uint32;

typedef struct gwtt_oam_message_list_node
{
	struct gwtt_oam_message_list_node *next;
	unsigned char GwOpcode;
	unsigned int SendSerNo;
	unsigned short WholePktLen;
	unsigned short RevPktLen;
	unsigned char SessionID[8];
	cs_uint32 TimerID;
	unsigned char *pPayLoad;
} GWTT_OAM_MESSAGE_NODE;
#endif

typedef enum
{
	GW_TVM_ENABLE = 1,
	GW_TVM_DISABLE = 2,
}GW_TVM_ENABLE_T;

#if 0
typedef enum
{
	GW_VLAN_ADD = 0,
	GW_VLAN_DEL = 1,
	GW_VLAN_CHANGE = 2,
	GW_VLAN_ORIGINAL =3,
}GW_VLAN_OPER_T;
#endif

//对外提供的接口
extern int gw_igmp_tvm_pkt_proc( char *igmp_pkt, int *len, int port_id, int *vlan_oper, int *vlan_id_cur);
extern long GwOamTvmRequestRecv( void *pRequest_input );
extern int igmp_tvm_enable_status_get(GW_TVM_ENABLE_T *enable);
extern int igmp_control_table_init(void);
extern void igmp_control_table_dump(void);
#endif
