#include "plat_common.h"
#include "packet.h"
#include "ptp_halftc.h"
#include "ptp_cmn.h"
#include "ptp_api.h"
#include "sdl_ptp.h"

ptp_tc_node_t g_ptp_tc_node;

static cs_status ptp_pkt_parser(cs_pkt_t *pkt);

ptp_mode_t onu_ptp_get_mode()
{
    return g_ptp_tc_node.mode;
}

cs_status onu_ptp_set_mode(ptp_mode_t mode)
{
    if(g_ptp_tc_node.mode == mode) {
        return CS_E_OK;
    }

    switch(mode) {
        case PTP_DISABLE:
            ptp_enable(0);
            break;

        case PTP_HALF_TC_MODE:
            halftc_enable();
            break;

        default:
            return CS_E_NOT_SUPPORT;
    }
    
    g_ptp_tc_node.mode = mode;
    
    return CS_E_OK;
}

static cs_status ptp_pkt_parser(cs_pkt_t *pkt)
{   
    if(pkt->eth_type == PTP_ETHER_TYPE) {
        pkt->pkt_type = CS_PKT_PTP;
        pkt->len += 4; /* add crc field */
        return CS_E_OK;
    }
    
    return CS_E_NOT_SUPPORT;
        
}

cs_status ptp_enable(cs_uint8 enable)
{
    cs_callback_context_t context;

    if(enable) {
        app_pkt_reg_parser(CS_PKT_PTP, ptp_pkt_parser);
    }
    else {
        app_pkt_reg_parser(CS_PKT_PTP, NULL);
        app_pkt_reg_handler(CS_PKT_PTP, NULL);
    }
    
    epon_request_onu_ptp_clk_src_set(context, 0, 0, 
        enable ? SDL_PTP_CLK_SRC_MPCP : SDL_PTP_CLK_SRC_PTP);

    epon_request_onu_ptp_cfg_set(context, 0, 0, CS_PON_PORT_ID, 0xffff, enable);
    epon_request_onu_ptp_cfg_set(context, 0, 0, CS_UNI_PORT_ID1, 0xffff, enable);
#ifdef HAVE_FE
    epon_request_onu_mgmt_port_ptp_state_set(context, 0, 0, enable);
#endif    
    return CS_E_OK;
}

void ptpv2_init()
{
    memset(&g_ptp_tc_node, 0, sizeof(g_ptp_tc_node));

    g_ptp_tc_node.mode = PTP_DISABLE;
    g_ptp_tc_node.num_ports = 2;
    g_ptp_tc_node.delay_mech = PTP_DELAY_REQ_RESP_MECH;
    g_ptp_tc_node.two_step = 0;
}

