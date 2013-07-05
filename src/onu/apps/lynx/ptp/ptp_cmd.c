#include "plat_common.h"
#include "cs_cmd.h"
#include "ptp_api.h"
#include "802_1as.h"
#include "sdl_ptp.h"
#include "registers.h"

extern ptp_pon_tod_t g_sys_time;
extern cs_uint32 g_rx_time_cr;
extern cs_uint32 g_tx_time_cr;

extern cyg_uint32 reg_read(cyg_uint32 addr) ;

void ptp_cmd_help()
{
    cs_printf("ptp mode [mode] - set ptp mode\n");
    cs_printf("ptp state - dump ptp state\n");
    cs_printf("ptp rc - set/get rx time correction\n");
    cs_printf("ptp tc - set/get tx time correction\n");
    cs_printf("ptp pw [width] - set pps width\n");
    cs_printf("ptp pps - 1pps pulse test\n");
}

sal_cmd_result_t ptp_cmd_proc(int argc , char **argv)
{
    if(argc == 2)
    {
        ptp_cmd_help();
    }
    else {
        argc--;
        argv++;

        if(!strcmp(argv[1], "mode")) { /* set mode */
            ptp_mode_t mode = atoi(argv[2]);
            onu_ptp_set_mode(mode);
        }
        else if(!strcmp(argv[1], "state")) {
            ptp_mode_t mode = onu_ptp_get_mode();
            cs_printf("ptp mode: %d\n", mode);
            cs_printf("pps pulse width: %d us\n", onu_8021as_get_pps_width());
            cs_printf("sys time: %llu when ptp cnt = 0x%08x\n", g_sys_time.time, g_sys_time.cnt);
        }
        else if(!strcmp(argv[1], "rc")) {
            if(argc == 2) {
                cs_printf("rx time correction: %dns\n", g_rx_time_cr);
            }
            else {
                cs_uint32 cr = atoi(argv[2]);
                g_rx_time_cr = cr;
            }
        }
        else if(!strcmp(argv[1], "tc")) {
            if(argc == 2) {
                cs_printf("rx time correction: %dns\n", g_tx_time_cr);
            }
            else {
                cs_uint32 cr = atoi(argv[2]);
                g_tx_time_cr = cr;
            }
        }
        else if(!strcmp(argv[1], "pw")) {
            cs_uint16 width = atoi(argv[2]);
            onu_8021as_set_pps_width(width);
        }
        else if(!strcmp(argv[1], "pps")) { 
            extern cs_uint16  g_pps_width;
            cs_callback_context_t     context;
            cs_uint32   s_pps;
            cs_uint32   mpcp_timer  = reg_read(ONU_MAC_LOCAL_TIMER);
            s_pps = mpcp_timer + DEF_PPS_OFFSET;
            cs_printf("cur time: %08x, trigger at %08x\n", mpcp_timer, s_pps);
            epon_request_onu_tod_trigger_pps_signal(context, 0, 0, s_pps, g_pps_width);
        }

    }

    return SAL_CMD_OK;
}

