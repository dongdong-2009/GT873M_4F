#include "iros_config.h"
#include "plat_common.h"
#include "packet.h"
#include "cs_msg.h"
#include "cs_event.h"
#include "event.h"
#include "cs_hello.h"
#include "onu_hello.h"
#include "sdl_api.h"
#include "app_oam_cs.h"
#include "app_gpio.h"
#ifdef HAVE_NTT_OAM
#include "app_oam_ntt.h"
#endif

onu_events_t onu_events[EPON_EVENT_TYPE_MAX];

cs_int32 onu_evt_reg(cs_int32 evt_type , void *func , void *cookie)
{
    cs_int32 i;
    if (func == NULL)
        return CS_E_ERROR;

    for(i = 0 ; i < MAX_HANDLER_NUM ; i++)
    {
        if(onu_events[evt_type].handler[i] == func)
            return CS_E_OK;

        if(onu_events[evt_type].handler[i] == NULL)
            break;
    }

    if(i >= MAX_HANDLER_NUM)
        return CS_E_ERROR;
    
    onu_events[evt_type].handler[i] = func;

    return CS_E_OK;
}

void event_detect(cs_int32 type, cs_uint16 msg_len, void* msg)
{
    onu_event_msg_t *pMsg = NULL;
    cs_int32 ret;

    pMsg = (onu_event_msg_t *)iros_malloc(IROS_MID_EVENT, sizeof(onu_event_msg_t) - 1 + msg_len);
    if (!pMsg) {
        diag_printf("event_detect iros_malloc failed.\n");
        return;
    }

    pMsg->type = IROS_MSG_EVENT;
    pMsg->evt.type = type;
    pMsg->evt.relay = 0;
    pMsg->evt.msg_len = msg_len;
    memcpy(pMsg->evt.data , msg , msg_len);
    IROS_LOG_MSG(IROS_LOG_LEVEL_DBG3 , IROS_MID_EVENT , "detect %s event\n",
            (type == EPON_EVENT_PORT_LINK_CHANGE)?"link":
            (type == EPON_EVENT_REG_CHANGE)?"reg_change":
            (type == EPON_EVENT_BM_CRC)?"BM crc error":
            (type == EPON_EVENT_ENC_ENCRYPTED)?"DS encrypted-frame-recv":"unkonwn");

    if(type == EPON_EVENT_BM_CRC)
    {
        IROS_LOG_CRI(IROS_MID_EVENT, "detect BM CRC error\n");
    }
    ret = cs_pri_queue_put(app_msg_q , (void *) & pMsg, sizeof(cs_uint32), CS_OSAL_NO_WAIT, APP_QUEUE_PRI_3);
    if (ret) {
        IROS_LOG_MSG(IROS_LOG_LEVEL_DBG3, IROS_MID_EVENT, "event_detect cs_queue_put failed %d\n", ret);
        iros_free(pMsg);
        return;
    }

    return;
}

void event_send_sc(cs_int32 type, cs_uint16 msg_len, void* msg)
{
    onu_event_msg_t *pMsg = NULL;
    cs_int32 ret;

    pMsg = (onu_event_msg_t *)iros_malloc(IROS_MID_EVENT, sizeof(onu_event_msg_t) - 1 + msg_len);
    if (!pMsg) {
        diag_printf("event_detect iros_malloc failed.\n");
        return;
    }

    pMsg->type = IROS_MSG_EVENT;
    pMsg->evt.type = type;
    pMsg->evt.relay = 1;
    pMsg->evt.msg_len = msg_len;
    memcpy(pMsg->evt.data , msg , msg_len);
    IROS_LOG_MSG(IROS_LOG_LEVEL_DBG3 , IROS_MID_EVENT , "detect %s event\n",
            (type == EPON_EVENT_PORT_LINK_CHANGE)?"link":
            (type == EPON_EVENT_REG_CHANGE)?"reg_change":
            (type == EPON_EVENT_BM_CRC)?"BM crc error":
            (type == EPON_EVENT_ENC_ENCRYPTED)?"DS encrypted-frame-recv":"unkonwn");

    if(type == EPON_EVENT_BM_CRC)
    {
        IROS_LOG_CRI(IROS_MID_EVENT, "detect BM CRC error\n");
    }
    ret = cs_pri_queue_put(app_msg_q , (void *) & pMsg, sizeof(cs_uint32), CS_OSAL_NO_WAIT, APP_QUEUE_PRI_3);
    if (ret) {
        IROS_LOG_MSG(IROS_LOG_LEVEL_DBG3, IROS_MID_EVENT, "event_detect cs_queue_put failed %d\n", ret);
        iros_free(pMsg);
        return;
    }

    return;
}


void event_test()
{
    int i;

    for(i = 0; i < 20 ; i++)
    {
        event_send_sc(i, 4, &i);
    }
}

#ifdef HAVE_KT_OAM
extern void onu_start_detect_pon_act();
extern void onu_stop_detect_pon_act();
#endif
void onu_evt_changed_led_proc(cs_uint16 dev, cs_int32 evt, void *msg)
{
    sdl_event_reg_t *pMsg = (sdl_event_reg_t *)msg;

    if(msg == NULL)
        return;

    if(pMsg->reg)
    {
    #ifdef HAVE_NTT_OAM
        ntt_dl_info_type_t  stats;
        ntt_dl_result_type_t result;
        onu_ntt_upgrade_info_get(&stats, &result);
        if(stats != NTT_DL_RST_WAIT) {
        cs_led_proc(LED_PON, LED_GREEN,LED_ON);
        }
    #elif HAVE_KT_OAM    
        cs_led_proc(LED_PON_LNK, LED_GREEN,LED_ON);
        /*onu_start_detect_pon_act();*/
    #else
        cs_led_proc(LED_PON_LNK, LED_GREEN,LED_ON);
    #endif
    }
    else
    {
    #ifdef HAVE_NTT_OAM
        ntt_dl_info_type_t  stats;
        ntt_dl_result_type_t result;
        onu_ntt_upgrade_info_get(&stats, &result);
        if(stats != NTT_DL_RST_WAIT) {
            cs_led_proc(LED_PON, LED_GREEN,LED_OFF);
        }

        cs_led_proc(LED_AUTH, LED_GREEN,LED_OFF);
    #elif HAVE_KT_OAM    
        cs_led_proc(LED_PON, LED_GREEN,LED_OFF);
        /*onu_stop_detect_pon_act();*/
    #else
        cs_led_proc(LED_PON_LNK, LED_GREEN,LED_OFF);
    #endif    
    }

    return;
}

void onu_link_changed_led_proc(cs_uint16 dev, cs_int32 evt, void *msg)
{
    sdl_event_port_link_t *pMsg = (sdl_event_port_link_t *)msg;

    if(pMsg == NULL)
        return;

    if(pMsg->port == CS_PON_PORT_ID)
    {
        if(pMsg->link)
        {
            IROS_LOG_CRI(IROS_MID_EVENT, "PON port link up\n");
            cs_printf("PON port link up\n");
            #ifdef HAVE_CTC_OAM
            cs_led_proc(LED_PON, LED_GREEN,LED_FAST_TWINKLE);
            cs_led_proc(LED_LOS, LED_RED,LED_OFF);
            #endif
        }
        else
        {
            IROS_LOG_CRI(IROS_MID_EVENT, "PON port link down\n");
            cs_printf("PON port link down\n");
            #ifdef HAVE_CTC_OAM
            cs_led_proc(LED_PON, LED_GREEN,LED_OFF);
            cs_led_proc(LED_LOS, LED_RED,LED_TWINKLE);
            #endif
        }
    }

    if(pMsg->port >= CS_UNI_PORT_ID1 && pMsg->port <= UNI_PORT_MAX)
    {
        if(pMsg->link)
        {
            IROS_LOG_CRI(IROS_MID_EVENT, "UNI port-%d link up\n",pMsg->port);
            cs_printf("UNI port-%d link up\n",pMsg->port);
        }
        else
        {
            IROS_LOG_CRI(IROS_MID_EVENT, "UNI port-%d link down\n",pMsg->port);
            cs_printf("UNI port-%d link down\n",pMsg->port);
        }
    }
}

#if 1
void onu_reset(void)
{
	iros_system_reset(RESET_TYPE_FORCE);
}
void onu_pon_link_down_proc(cs_uint16 dev, cs_int32 evt, void *msg)
{
    sdl_event_reg_t *pMsg = (sdl_event_reg_t *)msg;

    if(msg == NULL)
        return;

    if(pMsg->reg)
    {
    	//do nothing
    }
	else
	{
		onu_reset();
	}

	
}
#endif

void event_init()
{
    cs_callback_context_t     context;
    IROS_LOG_MSG(IROS_LOG_LEVEL_INF, IROS_MID_EVENT, "EVENT: init\n");
#ifdef HAVE_ONU_EVENT    
    cs_event_init();
#endif
    memset(onu_events, 0, EPON_EVENT_TYPE_MAX*sizeof(onu_events_t));
    epon_request_onu_event_callback_reg(context, ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, 
        (sdl_event_cb_f)event_detect);
    onu_evt_reg(EPON_EVENT_REG_CHANGE, (void *)onu_evt_changed_led_proc, NULL);
    onu_evt_reg(EPON_EVENT_PORT_LINK_CHANGE,(void *)onu_link_changed_led_proc, NULL);
	onu_evt_reg(EPON_EVENT_REG_CHANGE,(void *)onu_pon_link_down_proc, NULL);
}

void event_main_proc(cs_int32 type, cs_uint32 msg_len, void* msg, cs_boolean relay)
{
#ifdef HAVE_ONU_EVENT
    evt_msg_t *frame = NULL;
    cs_sdk_event_msg_t *evt;
    cs_int32 pkt_len =0;
    cs_uint32 size;
#endif    
    onu_event_handler_t handler = NULL;
    cs_int32 i;
    /*onu_event_hton_t hton_func = onu_events[type].hton_func;*/

    if (!msg || msg_len == 0) {
        return;
    }

    if (!relay) {
        for(i = 0 ; i < MAX_HANDLER_NUM ; i++)
        {
            handler = onu_events[type].handler[i];
            if(handler == NULL)
                continue;

            handler(0, type , msg);
        }
        return;
    }

#ifdef HAVE_ONU_EVENT
    size = sizeof(cs_oam_pdu_hdr_t) + sizeof(cs_sdk_event_msg_t) - 1 + msg_len;
    frame = (evt_msg_t *)evt_msg_alloc(size);
    
    if (!frame) {
        return;
    }

    pkt_len = cs_oam_hdr_build((cs_uint8 *)frame->evt_frame, &onu_node.sc.macaddr, CS_OAM_OPCODE_EVENT, CS_OUI);
    evt = (cs_sdk_event_msg_t *)((cs_uint8 *)frame->evt_frame+pkt_len);
    evt->msg_type = htonl(CS_ESS_SDK_EVENT);
    memcpy(evt->device.addr , onu_node.device.addr , 6);
    evt->event = htonl(type);
    evt->len = htonl(msg_len);

    /*hton_func(0, type , msg);*/
    memcpy(evt->data, msg, msg_len);
    cs_event_add(frame, size);
#endif
    return;
}


cs_uint8 onu_evt_msg_proc(iros_msg_t *msg)
{
    onu_event_msg_t *pEvtMsg = (onu_event_msg_t *)msg;
    onu_event_hdr_t *evt = (onu_event_hdr_t *)&pEvtMsg->evt;

    event_main_proc(evt->type, evt->msg_len, evt->data, evt->relay);

    return 1;
}

cs_int32 event_hton_reg(cs_int32 evt_type , void *func , void *cookie)
{

    if (func == NULL)
        return CS_E_ERROR;

    onu_events[evt_type].hton_func = func;

    return CS_E_OK;
}
