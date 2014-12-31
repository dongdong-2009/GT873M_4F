#include "cs_types.h"
#include "cs_cmn.h"
#include "plat_common.h"
#include "packet.h"
#include "log.h"
#include "sdl_api.h"
#include "sdl_pktctrl.h"

#define SYS_SPCL_PKT_CFG
#include "sys_cfg.h"

#include <stdio.h>

/* 
 * global variables 
 */
 
cs_pkt_profile_t g_pkt_profile[CS_PKT_TYPE_NUM+1];

cs_pkt_filter_t g_pkt_filter_set[ONU_PKT_FILTER_NUM];

cs_uint8 g_pkt_type_map[] = 
{
    CS_PKT_TYPE_NUM,   /* SDL_PKT_NORMAL*/
    CS_PKT_BPDU,          /* SDL_PKT_BPDU */
    CS_PKT_8021X,         /* SDL_PKT_8021X */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_IEEE_RSRVD1 */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_MYMAC */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_MC_A */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_MC_B */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_MC_C */
    CS_PKT_GMP,            /* SDL_PKT_IGMP/SDL_PKT_MLD */
    CS_PKT_ARP,            /* SDL_PKT_ARP */
    CS_PKT_OAM,           /* SDL_PKT_OAM */
    CS_PKT_MPCP,          /* SDL_PKT_MPCP */
    CS_PKT_DHCP,          /* SDL_PKT_DHCP */
    CS_PKT_PPPOE,         /* SDL_PKT_PPPoE_DIS */
    CS_PKT_PPPOE,        /* SDL_PKT_PPPoE_Session */
    CS_PKT_IROS,          /* SDL_PKT_HELLO */
    CS_PKT_NDP,            /* SDL_PKT_IPV6NDP */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_UDF0 */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_UDF1 */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_UDF2 */
    CS_PKT_TYPE_NUM,   /* SDL_PKT_UDF3  */
    CS_PKT_TYPE_NUM    /* SDL_PKT_SWT */
};


/*
 * internal functions
 */
 cs_uint8 app_check_vlan_tag(cs_uint16 ethtype)
{
    if(ethtype == EPON_ETHERTYPE_DOT1Q || ethtype == EPON_ETHERTYPE_TPID_SVLAN||
        ethtype == EPON_ETHERTYPE_TPID_SVLAN_OLD) {
        return 1;
    }
    else {
        return 0;
    }
}

cs_status app_pkt_l2hdr_parser(cs_pkt_t *pPkt)
{
    cs_uint8 *pBuf = NULL;
    cs_uint16 *pEthtype = NULL;
    cs_uint16 ethtype = 0;

    pBuf = pPkt->data + pPkt->offset + 12;  /* sa+da length */
    if(pPkt->len <= CS_LAYER2_HEADER_LEN)
    {
        return CS_E_ERROR;
    }

    pEthtype = (cs_uint16 *)pBuf;
    ethtype = ntohs(*pEthtype);

    if(app_check_vlan_tag(ethtype))
    {
        cs_vlan_hdr_t *vlan_hdr = NULL;

        vlan_hdr = (cs_vlan_hdr_t *)pBuf;     
        pPkt->stpid = ntohs(vlan_hdr->tpid);
        pPkt->svlan = ntohs(vlan_hdr->tag);
        pPkt->svlan &= 0xfff;
        pPkt->tag_num++;
                
        pBuf += sizeof(cs_vlan_hdr_t);
        pEthtype = (cs_uint16 *)pBuf;
        ethtype = ntohs(*pEthtype);        
        
        if(app_check_vlan_tag(ethtype))
        {
            vlan_hdr++;
            
            pPkt->ctpid = ntohs(vlan_hdr->tpid);
            pPkt->cvlan = ntohs(vlan_hdr->tag);
            pPkt->cvlan &= 0xfff;
            pPkt->tag_num++;

            pBuf += sizeof(cs_vlan_hdr_t);
            pEthtype = (cs_uint16 *)pBuf;
            ethtype = ntohs(*pEthtype);
        }
    }
    
    pPkt->eth_type = ethtype;

    return CS_E_OK;
}
#if (PRODUCT_CLASS == PRODUCTS_GT812C)
int ARP_counter = 0;
int GMP_counter = 0;
#endif
cs_status app_pkt_parser(cs_pkt_t *pPkt)
{
    cs_status ret = CS_E_ERROR;
    
    pPkt->pkt_type = g_pkt_type_map[pPkt->frame_type];    

    g_pkt_profile[CS_PKT_TYPE_NUM].counter++;

    if(pPkt->pkt_type == CS_PKT_TYPE_NUM) {
        cs_uint8 type;
        
        for(type = 0; type < CS_PKT_TYPE_NUM; type++) {
            if(g_pkt_profile[type].parser &&
                !g_pkt_profile[type].parser(pPkt)) {                 
                ret = CS_E_OK;
                break;
            }
        }

    }
    else {
        ret = CS_E_OK;
    }    

    if(pPkt->pkt_type != CS_PKT_TYPE_NUM) {
      g_pkt_profile[pPkt->pkt_type].counter++;
#if (PRODUCT_CLASS == PRODUCTS_GT812C)
      cs_callback_context_t     context;

      if(CS_PKT_ARP == pPkt->pkt_type)
      {
		//    	  cs_printf("pPkt->port is %d\n",pPkt->port);
		  if(CS_PON_PORT_ID < pPkt->port)
			  ARP_counter++;
		  if(ARP_counter > 20)
		  {

//			  cs_printf("ARP into CPU overflow \n");
			  epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_UP_STREAM, CS_PKT_ARP, DST_FE);
		  }
      }
      if(CS_PKT_GMP == pPkt->pkt_type)
      {
		  GMP_counter++;
		  if(GMP_counter > 20)
		 {
//			  cs_printf("GMP into CPU overflow \n");
			  epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_UP_STREAM, CS_PKT_GMP, DST_FE);
			  epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_DOWN_STREAM, CS_PKT_GMP, DST_FE);
		 }
    }
#endif
    }

    /* dump pkt */
    if(g_pkt_profile[pPkt->pkt_type].trace) {       
        cs_uint8 cmt[CS_LOG_COMMENT_LEN];
        sprintf(cmt, "rx pkt type: %d, port: %d, len: %d", pPkt->pkt_type, pPkt->port, pPkt->len);
        cs_buffer_dump(cmt, pPkt->data + pPkt->offset, pPkt->len);
    }

    return ret;
}

cs_status app_pkt_filter(cs_pkt_t *pPkt)
{    
    cs_uint8 pri;

    for(pri = 0; pri < ONU_PKT_FILTER_NUM; pri++) {
        if(g_pkt_filter_set[pri].entry) {
            
            if(g_pkt_filter_set[pri].port == CS_ALL_UNI_PORT_ID) {
                if(!pPkt->port) {   /* pon port */
                    continue;
                }
            }
            else { 
                if(pPkt->port != g_pkt_filter_set[pri].port ) {
                    continue;
                }
            }
            
            if(g_pkt_filter_set[pri].entry(pPkt)) {
                IROS_LOG_MSG(IROS_LOG_LEVEL_DBG3, IROS_MID_APP, 
                    "app_pkt_filter hit %d: pkt_type %d port %d\n", pri, pPkt->pkt_type, pPkt->port);
                return CS_E_ERROR;
            }
        }
    }

    return CS_E_OK;
}

cs_int32 pkt_priority = APP_QUEUE_PRI_2;
cs_status app_pkt_rx_handler(cs_pkt_t *pPkt)
{
    cs_uint32 result = 0;

    IROS_LOG_MSG(IROS_LOG_LEVEL_DBG3, IROS_MID_APP, 
        "app_pkt_rx len %d type %d port %d\n", pPkt->len, pPkt->frame_type, pPkt->port);

   pPkt->len -= 4;  /* remove crc field */
   pkt_priority = APP_QUEUE_PRI_2;
    if(app_pkt_l2hdr_parser(pPkt) ||
        app_pkt_parser(pPkt) ||
        app_pkt_filter(pPkt)) {
        result = CS_E_ERROR;
    }     
    else {        
        pPkt->msg_type = IROS_MSG_PKT;
        result = cs_pri_queue_put(app_msg_q , (void *)&pPkt, 
            sizeof(cs_uint32), CS_OSAL_NO_WAIT, pkt_priority);
    }
    
    if (result){
        IROS_LOG_MSG(IROS_LOG_LEVEL_INF,IROS_MID_APP, 
            "app_pkt_rx_handler fail type: %d, port: %d, len: %d\n", 
            pPkt->pkt_type, pPkt->port, pPkt->len);
        iros_free(pPkt);
    }
    
    return result;
}

/*
 * external APIs 
 */

cs_uint32 app_pkt_get_counter(cs_pkt_type_t pkt_type) 
{
    if(pkt_type > CS_PKT_TYPE_NUM) {
        return 0;
    }
    
    return g_pkt_profile[pkt_type].counter;
}

void app_pkt_clr_counter(cs_pkt_type_t pkt_type)
{
    g_pkt_profile[pkt_type].counter = 0;
}
 
cs_status app_pkt_add_filter_rule(cs_port_id_t port, cs_pkt_filter_entry_t entry)
{
    cs_uint8 id;
    cs_uint8 port_num;

    startup_config_read(CFG_ID_SWITCH_PORT_NUM, 1, &port_num);
    if(port != CS_ALL_UNI_PORT_ID && port > port_num) {
        return CS_E_PARAM;
    }

    for(id = 0; id < ONU_PKT_FILTER_NUM; id++) {
        if(!g_pkt_filter_set[id].entry) {
            g_pkt_filter_set[id].port = port;
            g_pkt_filter_set[id].entry = entry;

            return CS_E_OK;
        }
    }

    return CS_E_ERROR;
}

cs_status app_pkt_reg_handler(cs_uint8 pkt_type, cs_pkt_handler_t hdl)
{
    if(pkt_type < CS_PKT_TYPE_NUM) {
        g_pkt_profile[pkt_type].handler = hdl;
        return CS_E_OK;
    }
    else {
        return CS_E_PARAM;
    }
    
}

cs_status app_pkt_reg_parser(cs_uint8 pkt_type, cs_pkt_parser_t parser)
{
    if(pkt_type < CS_PKT_TYPE_NUM) {
        g_pkt_profile[pkt_type].parser= parser;
        return CS_E_OK;
    }
    else {
        return CS_E_PARAM;
    }
}

void app_pkt_toggle_trace(cs_pkt_type_t type)
{
    g_pkt_profile[type].trace = 1 - g_pkt_profile[type].trace;
    cs_printf("pkt type %d trace %d\n", type, g_pkt_profile[type].trace);
}

void app_pkt_init()
{
    cs_callback_context_t   context;
    cs_pkt_type_t  type;

    memset(g_pkt_profile, 0, sizeof(g_pkt_profile));
    memset(g_pkt_filter_set, 0, sizeof(g_pkt_filter_set));
    
    epon_request_onu_pkt_rcv_handler_reg(context, ONU_DEVICEID_FOR_API, 
        ONU_LLIDPORT_FOR_API, app_pkt_rx_handler);

    /* init slow path */
    for(type = CS_PKT_BPDU; type < CS_PKT_TYPE_NUM; type++) {
        if(INVALID_PKT_BEHAVIOR != g_spcl_pkt_def_state[type][UPSTREAM_PKT]) {
            epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_UP_STREAM, type, 
                g_spcl_pkt_def_state[type][UPSTREAM_PKT]);
        }

        if(INVALID_PKT_BEHAVIOR != g_spcl_pkt_def_state[type][DOWNSTREAM_PKT]) {
            epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_DOWN_STREAM, type, 
                g_spcl_pkt_def_state[type][DOWNSTREAM_PKT]);
        }

    }
}

cs_uint8 app_pkt_msg_proc(cs_pkt_t *pPkt)
{
	extern unsigned char pkt_stuf[1500];
	memset(pkt_stuf,0,sizeof(pkt_stuf));
	memcpy(pkt_stuf,pPkt->data+pPkt->offset,pPkt->len);
    if(g_pkt_profile[pPkt->pkt_type].handler) {
        return g_pkt_profile[pPkt->pkt_type].handler(pPkt);
    }

    return 1;
}

cs_status app_pkt_tx(cs_port_id_t port_id, cs_pkt_type_t type, cs_uint8 *frame, cs_uint16 len)
{
    cs_callback_context_t context;
    cs_status ret;
    ret = epon_request_onu_frame_send(context, 0, 0, frame, len, port_id, 7);
    if(CS_E_OK == ret) {
        /* dump pkt */
        if(g_pkt_profile[type].trace) {       
            cs_uint8 cmt[CS_LOG_COMMENT_LEN];
            sprintf(cmt, "tx pkt type: %d, port: %d, len: %d", type, port_id, len);
            cs_buffer_dump(cmt, frame, len);
        }
    }

    return ret;
}

