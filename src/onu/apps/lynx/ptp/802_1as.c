#include "plat_common.h"
#include "802_1as.h"
#include "registers.h"
#include "sdl_ptp.h"
#include "app_timer.h"
#include "uart.h"

extern cs_uint32 reg_read(cs_uint32 addr);
extern cs_uint32 reg_write(cs_uint32 addr, cs_uint32 wdata);

void onu_8021as_proc(cs_uint8 *frame);
void tod_fill_info(cs_uint64 tod, tod_info_t *info);

cs_boolean g_802_1as_en;
cs_uint16  g_pps_width;

tod_info_t g_tod_info;

ptp_pon_tod_t g_sys_time;
cs_uint32 g_1pps_trigger_cnt = 0;
cs_uint32 g_tod_trigger_cnt = 0;

void onu_sys_time_update()
{
    cs_callback_context_t     context;

    cs_uint64 left_ns = 0;
    cs_uint32 offset = 0;
    cs_uint32 tod_trigger_offset = 62500 /*1ms*/;
    cs_uint32 cur_mpcp_cnt = reg_read(ONU_MAC_LOCAL_TIMER);

    /* update sys time */
    offset = cur_mpcp_cnt- g_sys_time.cnt;
    g_sys_time.cnt = cur_mpcp_cnt;
    g_sys_time.time += offset * EPON_TQ_TIME;

    /* update tod info, this should be in the onu_8021as_proc() function*/
    tod_fill_info(g_sys_time.time/1000000000, &g_tod_info);

    PTP_LOG(IROS_LOG_LEVEL_DBG0, "update sys time %llu\n", g_sys_time.time);

    /* update 1pps and tod trigger */
    left_ns = g_sys_time.time % NS_NUM_PER_SEC;
    left_ns = NS_NUM_PER_SEC - left_ns;
    offset = left_ns / EPON_TQ_TIME;
    if(offset + g_sys_time.cnt != g_1pps_trigger_cnt) {
        cs_uint64 time;
        g_1pps_trigger_cnt = offset + g_sys_time.cnt;
        g_tod_trigger_cnt = g_1pps_trigger_cnt + tod_trigger_offset; /* 1ms after the 1pps driven high */
        time = g_sys_time.time + (cs_uint64)offset * EPON_TQ_TIME;
        PTP_LOG(IROS_LOG_LEVEL_INF, "trigger 1pps signal at time %llu(0x%08x)\n",
            time, g_1pps_trigger_cnt);

        /*set the 1pps signal trigger counter*/
        epon_request_onu_tod_trigger_pps_signal(context, 0, 0, g_1pps_trigger_cnt, g_pps_width);

        /*set the tod information*/
        epon_request_onu_tod_info_set(context, 0, 0, sizeof(g_tod_info), (cs_uint8 *)(&g_tod_info));
        PTP_LOG(IROS_LOG_LEVEL_INF, "trigger tod info at time %llu(0x%08x)\n",
                    time + (cs_uint64)tod_trigger_offset*EPON_TQ_TIME, g_tod_trigger_cnt);

        /*set the tod trigger counter*/
        epon_request_onu_tod_trigger_tod_signal(context, 0, 0, g_tod_trigger_cnt);
    }
    return;
}

void onu_8021as_init()
{
    static app_timer_msg_t  g_ptp_tmr_msg;

    memset(&g_sys_time, 0, sizeof(g_sys_time));

    g_pps_width = DEF_PPS_WIDTH;

    memset(&g_tod_info, 0, sizeof(g_tod_info));
    g_tod_info.start = '$';
    memcpy(g_tod_info.msg_id, "GPZDA", 5);
    g_tod_info.end = '*';
    g_tod_info.cr = '\r';
    g_tod_info.lf = '\n';

    /* timer init */
    g_ptp_tmr_msg.msg_type = IROS_MSG_TIME_OUT;
    g_ptp_tmr_msg.timer_type = APP_TIMER_PTP;
    cs_msg_circle_timer_add(app_msg_q, 700, (void *)&g_ptp_tmr_msg);

    onu_8021as_enable(1);
}

cs_uint8 onu_check_8021as_pdu(oam_pdu_hdr_t *pdu)
{
    if(g_802_1as_en && TIME_SYNC_SUBTYPE == pdu->sub_type) {
        onu_8021as_proc((cs_uint8 *)pdu);

        return 1;
    }

    return 0;
}

void tod_calc_utc_time(cs_uint64 tod, cs_uint32 *day, cs_uint8 *hour, cs_uint8 *min, cs_uint8 *sec)
{
    cs_uint32 sec_of_day = 60 * 60 * 24;
    cs_uint32 sec_of_hour = 60 * 60;

    *day = tod/sec_of_day;
    tod -= *day * sec_of_day;

    *hour = tod/sec_of_hour;
    tod -= *hour * sec_of_hour;

    *sec = tod;
}

void tod_calc_date(cs_uint32 days, cs_uint8 *day, cs_uint8 *month, cs_uint16 *year)
{
    cs_uint16 years;
    cs_uint8  leap = 0;
    cs_uint32 leap_num = 0;

    cs_uint8 mid;

    cs_uint8 days_of_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    years = days / 365;
    *year = years + 1970;

    leap_num = (years - 1) / 4;

    if( (years - 2) % 4 == 0) {
        leap = 1;
    }

    days -= (366 * leap_num + 365 * (years - leap_num));

    if(leap) {
        days_of_month[1] = 29;
    }

    *month = 1;

    for(mid = 0; mid < 12; mid++) {
        if(days > days_of_month[mid]) {
            days -= days_of_month[mid];
            *month++;
        }
    }
    *day = days;

}

void tod_fill_info(cs_uint64 tod, tod_info_t *info)
{
    cs_uint32 days;
    cs_uint8 hour, min, sec;
    cs_uint8 day, month;
    cs_uint16 year;

    cs_uint16 byte;
    cs_uint8 cs = 0;
    cs_uint8 *data = (cs_uint8 *)info;
    cs_uint8 tod_buffer[48] = {0};

    if(NULL == info)
    {
        PTP_LOG(IROS_LOG_LEVEL_DBG1, "The g_tod_info is NULL point. tod = %llu\n", tod);
        return;
    }

    tod_calc_utc_time(tod, &days, &hour, &min, &sec);

    sprintf(info->utc_time, "%02d%02d%02d", hour, min, sec);

    tod_calc_date(days, &day, &month, &year);
    sprintf(info->day, "%02d", day);
    sprintf(info->month, "%02d", month);
    sprintf(info->year, "%04d", year);

    sprintf(info->local_zone_hr, "%02d", hour);
    sprintf(info->local_zone_min, "%02d", min);

    /* calc check sum */
    for(byte = 1; byte < sizeof(tod_info_t) - 5; byte++) {
        cs ^= data[byte];
    }
    sprintf(info->cs, "%02x", cs);
    memcpy(tod_buffer, data, sizeof(tod_info_t));
    tod_buffer[47] = '\0';
    PTP_LOG(IROS_LOG_LEVEL_DBG1, "The tod_info is: %s.\n",tod_buffer);
    return;
}


void onu_8021as_proc(cs_uint8 *frame)
{
    time_sync_pdu_t *pdu = (time_sync_pdu_t *)frame;

    cs_uint32 tod_ns;
    cs_uint64 tod_s;

    PTP_LOG(IROS_LOG_LEVEL_INF, "recv TIMESYNC: epoch_num = %04x, sec = %08x, nsec = %08x\n",
        ntohs(pdu->tod_epoch_num), ntohl(pdu->tod_s), ntohl(pdu->tod_ns));

    tod_s = (cs_uint64)(ntohs(pdu->tod_epoch_num)) << 32;

    tod_s += ntohl(pdu->tod_s);
    tod_ns = ntohl(pdu->tod_ns);

    /* update sys time */
    g_sys_time.time = tod_ns + tod_s * NS_NUM_PER_SEC;
    g_sys_time.cnt = ntohl(pdu->X);

    /* update tod info, this should be 8 times per second.*/
    tod_fill_info(tod_s, &g_tod_info);

    PTP_LOG(IROS_LOG_LEVEL_INF, "sys time update: nsec %llu when mpcp cnt = 0x%08x\n",
        g_sys_time.time, g_sys_time.cnt);

}

cs_status onu_8021as_enable(cs_boolean enable)
{
    cs_callback_context_t context;

    g_802_1as_en = enable;

    epon_request_onu_tod_cfg_set(context, 0, 0, enable);

    /*configure the Tod uart4 to send out the tod information*/
    epon_request_onu_tod_uart_cfg_set(context, 0, 0, UART_BAUD_RATE_4800);

    return CS_E_OK;
}

void onu_8021as_set_pps_width(cs_uint16 width)
{
    g_pps_width = width;
}

cs_uint16 onu_8021as_get_pps_width()
{
    return g_pps_width;
}

