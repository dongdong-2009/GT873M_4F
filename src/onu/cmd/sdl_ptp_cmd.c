/****************************************************************************
            Software License for Customer Use of Cortina Software
                          Grant Terms and Conditions

IMPORTANT NOTICE - READ CAREFULLY: This Software License for Customer Use
of Cortina Software ("LICENSE") is the agreement which governs use of
software of Cortina Systems, Inc. and its subsidiaries ("CORTINA"),
including computer software (source code and object code) and associated
printed materials ("SOFTWARE").  The SOFTWARE is protected by copyright laws
and international copyright treaties, as well as other intellectual property
laws and treaties.  The SOFTWARE is not sold, and instead is only licensed
for use, strictly in accordance with this document.  Any hardware sold by
CORTINA is protected by various patents, and is sold but this LICENSE does
not cover that sale, since it may not necessarily be sold as a package with
the SOFTWARE.  This LICENSE sets forth the terms and conditions of the
SOFTWARE LICENSE only.  By downloading, installing, copying, or otherwise
using the SOFTWARE, you agree to be bound by the terms of this LICENSE.
If you do not agree to the terms of this LICENSE, then do not download the
SOFTWARE.

DEFINITIONS:  "DEVICE" means the Cortina Systems? Daytona SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems? SDK software.

GRANT OF LICENSE:  Subject to the restrictions below, CORTINA hereby grants
CUSTOMER a non-exclusive, non-assignable, non-transferable, royalty-free,
perpetual copyright license to (1) install and use the SOFTWARE for
reference only with the DEVICE; and (2) copy the SOFTWARE for your internal
use only for use with the DEVICE.

RESTRICTIONS:  The SOFTWARE must be used solely in conjunction with the
DEVICE and solely with Your own products that incorporate the DEVICE.  You
may not distribute the SOFTWARE to any third party.  You may not modify
the SOFTWARE or make derivatives of the SOFTWARE without assigning any and
all rights in such modifications and derivatives to CORTINA.  You shall not
through incorporation, modification or distribution of the SOFTWARE cause
it to become subject to any open source licenses.  You may not
reverse-assemble, reverse-compile, or otherwise reverse-engineer any
SOFTWARE provided in binary or machine readable form.  You may not
distribute the SOFTWARE to your customers without written permission
from CORTINA.

OWNERSHIP OF SOFTWARE AND COPYRIGHTS. All title and copyrights in and the
SOFTWARE and any accompanying printed materials, and copies of the SOFTWARE,
are owned by CORTINA. The SOFTWARE protected by the copyright laws of the
United States and other countries, and international treaty provisions.
You may not remove any copyright notices from the SOFTWARE.  Except as
otherwise expressly provided, CORTINA grants no express or implied right
under CORTINA patents, copyrights, trademarks, or other intellectual
property rights.

DISCLAIMER OF WARRANTIES. THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY
EXPRESS OR IMPLIED WARRANTY OF ANY KIND, INCLUDING ANY IMPLIED WARRANTIES
OF MERCHANTABILITY, NONINFRINGEMENT, OR FITNESS FOR A PARTICULAR PURPOSE,
TITLE, AND NON-INFRINGEMENT.  CORTINA does not warrant or assume
responsibility for the accuracy or completeness of any information, text,
graphics, links or other items contained within the SOFTWARE.  Without
limiting the foregoing, you are solely responsible for determining and
verifying that the SOFTWARE that you obtain and install is the appropriate
version for your purpose.

LIMITATION OF LIABILITY. IN NO EVENT SHALL CORTINA OR ITS SUPPLIERS BE
LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, LOST
PROFITS, BUSINESS INTERRUPTION, OR LOST INFORMATION) OR ANY LOSS ARISING OUT
OF THE USE OF OR INABILITY TO USE OF OR INABILITY TO USE THE SOFTWARE, EVEN
IF CORTINA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
TERMINATION OF THIS LICENSE. This LICENSE will automatically terminate if
You fail to comply with any of the terms and conditions hereof. Upon
termination, You will immediately cease use of the SOFTWARE and destroy all
copies of the SOFTWARE or return all copies of the SOFTWARE in your control
to CORTINA.  IF you commence or participate in any legal proceeding against
CORTINA, then CORTINA may, in its sole discretion, suspend or terminate all
license grants and any other rights provided under this LICENSE during the
pendency of such legal proceedings.
APPLICABLE LAWS. Claims arising under this LICENSE shall be governed by the
laws of the State of California, excluding its principles of conflict of
laws.  The United Nations Convention on Contracts for the International Sale
of Goods is specifically disclaimed.  You shall not export the SOFTWARE
without first obtaining any required export license or other approval from
the applicable governmental entity, if required.  This is the entire
agreement and understanding between You and CORTINA relating to this subject
matter.
GOVERNMENT RESTRICTED RIGHTS. The SOFTWARE is provided with "RESTRICTED
RIGHTS." Use, duplication, or disclosure by the Government is subject to
restrictions as set forth in FAR52.227-14 and DFAR252.227-7013 et seq. or
its successor. Use of the SOFTWARE by the Government constitutes
acknowledgment of CORTINA's proprietary rights therein. Contractor or
Manufacturer is CORTINA.

Copyright (c) 2010 by Cortina Systems Incorporated
****************************************************************************/

#include "cs_cmd.h"


#ifdef HAVE_SDL_CMD
#include "sdl_ptp.h"
#include "sdl_ma.h"



#ifdef HAVE_SDL_CMD_HELP
static const char *uni_cmd_usage_ptp_cfg[] = 
{
    "cfg <port> <msg_type> <en>",
    "port num:",
    "   0: Pon port",
    "   1: GE port",
    "en:",
    "   1: enable ptp function",
    "   0: disable ptp function",
};

static const char *uni_cmd_usage_ptp_correction[] = 
{
    "cor <port> <direction> <value>",
    "port num:",
    "   0: PON port",
    "   1: GE port",
    "direction:",
    "   1: add ",
    "   0: subtract",
    "value:",
    "   0-0x7fff: correction value ",
};

static const char *uni_cmd_usage_tod_cfg[] = 
{
    "tod <en>",
    "en:",
        "   1: enable ptp function",
        "   0: disable ptp function",
};

static const char *uni_cmd_usage_tod_info[] = 
{
    "tod <len> <data1> <data2>...",
    "len:",
        "1-64: tod info length",
    "info:",
        "tod data, byte wide",
};

static const char *uni_cmd_usage_tod_pps_trigger[] = 
{
    "trig_pps <s_pps> <pps_width> ",
    "s_pps:",
        "0-0xffffffff: start of pps signal rise",
    "pps_width:",
        "0-0xffff: Width of pps pulse",
};

static const char *uni_cmd_usage_tod_tod_trigger[] = 
{
    "trig_tod <s_tod>",
    "s_tod:",
        "0-0xffffffff: start of tod signal",
};

static const char *uni_cmd_usage_ptp_clk_src[] = 
{
    "src <0-1>",
    "0: ptp timer",
    "1: mpcp timer",
};

#ifdef HAVE_MPORTS
static const char *uni_cmd_usage_ptp_sync_pkt_send[] = 
{
    "sync_send",
};

static const char *uni_cmd_usage_ptp_sw_time_set[] = 
{
    "sw_time_set [direction][second] [nano-second]",
       "direction: add(1), sub(0)",
};

static const char *uni_cmd_usage_ptp_sw_eg_timestamp_get[] = 
{
    "sw_eg_timestamp_get [port] ",
       "port: uni port(1-4), uplink port(0x30)",
        
};

static const char *uni_cmd_usage_ptp_sw_port_set[] = 
{
    "sw_port_set [port] ",
       "port: uni port(1-4), uplink port(0x30)",
        
};

static const char *cmd_usage_ptp_jitter_test_set[] = 
{
    "ptp jitter_test <enable>",
       "1: enable, 0: disable",
        
};

#endif

#endif

sal_cmd_result_t uni_cmd_ptp_cfg(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_status  status = 0;
    cs_port_id_t  portid = 0;
    cs_uint16     msg_type = 0;
    cs_boolean    enable = 0;

    if(argc != 5) return SAL_CMD_INVALID_PARAM;
    
    portid = (cs_port_id_t)iros_strtol(argv[2]);
    msg_type = (cs_uint16)iros_strtol(argv[3]);
    enable = (cs_boolean)iros_strtol(argv[4]);

    status = epon_request_onu_ptp_cfg_set(context, 0, 0, portid, msg_type, enable);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_cfg_set failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    
    
    status = epon_request_onu_ptp_cfg_get(context, 0, 0, portid, &msg_type, &enable);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_cfg_get failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    cs_printf("PTP Configuration\n");    
    cs_printf("Port  ID: %d\n", portid);    
    cs_printf("Msg type: 0x%04x\n", msg_type);    
    cs_printf("Enable  : %d\n", enable);    
    
    return 0;
}

sal_cmd_result_t uni_cmd_ptp_correction_set(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_status  status = 0;
    cs_uint16     value = 0;
    cs_port_id_t  portid = 0;
    cs_sdl_ptp_correct_dir_t  direction = 0;

    if(argc != 5) return SAL_CMD_INVALID_PARAM;
    
    portid = (cs_port_id_t)iros_strtol(argv[2]);
    direction = (cs_sdl_ptp_correct_dir_t)iros_strtol(argv[3]);
    value = (cs_uint16)iros_strtol(argv[4]);

    status = epon_request_onu_ptp_asym_correction_set(context, 0, 0, portid, direction, value);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_asym_correction_set failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }

    status = epon_request_onu_ptp_asym_correction_get(context, 0, 0, portid, &direction, &value);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_asym_correction_get failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    
    cs_printf("PTP Correction Configuration\n");    
    cs_printf("Port  ID : %d\n", portid);    
    cs_printf("Direction: %d\n", direction);    
    cs_printf("value    : %d\n", value);  

    return 0;
}

sal_cmd_result_t uni_cmd_tod_cfg(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_status  status = 0;
    cs_boolean  enable = 0;

    if(argc != 3) return SAL_CMD_INVALID_PARAM;
    
    enable = (cs_boolean)iros_strtol(argv[2]);

    status = epon_request_onu_tod_cfg_set(context, 0, 0, enable);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_tod_cfg_set failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    
    status = epon_request_onu_tod_cfg_get(context, 0, 0, &enable);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_tod_cfg_set failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    cs_printf("PTP TOD Configuration\n");    
    cs_printf("enable   : %d\n", enable);    

    return 0;
}

sal_cmd_result_t uni_cmd_tod_trigger(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_uint32 s_tod = 0;
    cs_status  status = 0;

    if(argc != 3) return SAL_CMD_INVALID_PARAM;
    
    s_tod = (cs_uint32)iros_strtol(argv[2]);

    status = epon_request_onu_tod_trigger_tod_signal(context, 0, 0, s_tod);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_tod_trigger_tod_signal failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }

    return 0;
}

sal_cmd_result_t uni_cmd_pps_trigger(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_uint32 s_pps = 0;
    cs_uint16 pps_width = 0;
    cs_status  status = 0;

    if(argc != 4) return SAL_CMD_INVALID_PARAM;
    
    s_pps = (cs_uint32)iros_strtol(argv[2]);
    pps_width = (cs_uint16)iros_strtol(argv[3]);

    status = epon_request_onu_tod_trigger_pps_signal(context, 0, 0, s_pps, pps_width );
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_tod_trigger_pps_signal failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }

    return 0;
}


sal_cmd_result_t uni_cmd_tod_info(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_uint8 i = 0;
    cs_uint8 len = 0;
    cs_uint8 data[64];
    cs_status  status = 0;
    
    len = (cs_uint8)iros_strtol(argv[2]);
    if(len>64) return SAL_CMD_INVALID_PARAM;
    if(argc > len+3) return SAL_CMD_INVALID_PARAM;
    
    for(i=0; i<len; i++)
        data[i] = (cs_uint8)iros_strtol(argv[i+3]);

    status = epon_request_onu_tod_info_set(context, 0, 0, len, &data[0]);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_tod_info_set failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    
    memset(&data, 0, sizeof(data));
    status = epon_request_onu_tod_info_get(context, 0, 0, &len, &data[0]);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_tod_info_get failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    
    cs_printf("Data len : %d\n", len);  
    for(i=0; i<len; i++){
        cs_printf("Data %d:  0x%x\n", i, data[i]);  
    }

    return 0;
}


sal_cmd_result_t uni_cmd_ptp_src(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_sdl_ptp_clk_src_t src = 0;
    cs_status  status = 0;

    if(argc != 3) return SAL_CMD_INVALID_PARAM;

    src = (cs_sdl_ptp_clk_src_t)iros_strtol(argv[2]);

    status = epon_request_onu_ptp_clk_src_set(context, 0, 0, src);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_clk_src_set failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    
    return 0;
}

#ifdef HAVE_MPORTS
sal_cmd_result_t uni_cmd_ptp_sync_send(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_status  status = CS_E_OK;

    if(argc != 2) return SAL_CMD_INVALID_PARAM;

    status = epon_request_onu_ptp_sync_pkt_send(context, 0, 0);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_sync_pkt_send failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    
    return 0;
}

sal_cmd_result_t uni_cmd_ptp_sw_eg_get(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_port_id_t  portid = 0;
    cs_status  status = CS_E_OK;
    cs_sdl_ptp_timestamp_t timestamp;

    if(argc != 3) return SAL_CMD_INVALID_PARAM;
    
    portid = (cs_port_id_t)iros_strtol(argv[2]);
    
    status = epon_request_onu_ptp_capture_sw_eg_timestamp(context, 0, 0, portid, &timestamp);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_sync_pkt_send failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    cs_printf("second      :  0x%x\n", timestamp.second); 
    cs_printf("nano_second :  0x%x\n", timestamp.nano_second);
    
    return 0;
}

sal_cmd_result_t uni_cmd_ptp_sw_port_set(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_port_id_t  portid = 0;
    cs_boolean  enable = 0;
    cs_status  status = CS_E_OK;


    if(argc != 4) return SAL_CMD_INVALID_PARAM;
    
    portid = (cs_sdl_ptp_clk_src_t)iros_strtol(argv[2]);
    enable = (cs_boolean)iros_strtol(argv[3]);
    
    status = epon_request_onu_ptp_sw_port_set(context, 0, 0, portid, enable);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_sw_port_set failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    cs_printf("port_id      :  0x%x\n", portid); 
    cs_printf("enable       :  0x%x\n", enable);
    
    return 0;
}

sal_cmd_result_t cmd_ptp_jitter_test_set(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_boolean  enable = 0;
    cs_status  status = CS_E_OK;


    if(argc != 3) return SAL_CMD_INVALID_PARAM;
    
    enable = iros_strtol(argv[2]);
    
    status = epon_request_onu_ptp_jitter_test(context, 0, 0, enable);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_jitter_test set failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    cs_printf("enable       :  0x%x\n", enable);
    
    return 0;
}

sal_cmd_result_t uni_cmd_ptp_sw_time_set(int argc, char **argv)
{
    cs_callback_context_t context;
    cs_sdl_ptp_time_dir_t dir = 0;
    cs_status  status = 0;
    cs_sdl_ptp_timestamp_t timestamp;

    if(argc != 5) return SAL_CMD_INVALID_PARAM;
    
    dir = (cs_port_id_t)iros_strtol(argv[2]);
    timestamp.second = (cs_uint32)iros_strtol(argv[3]);
    timestamp.nano_second  = (cs_uint32)iros_strtol(argv[4]);
    
    status = epon_request_onu_ptp_update_sw_time(context, 0, 0, dir, timestamp);
    if(status != CS_E_OK){
        cs_printf("epon_request_onu_ptp_update_sw_time failed \n");    
        return cs_status_2_cmd_rc_map(status);       
    }
    
    cs_printf("direction   :  0x%x\n", dir); 
    cs_printf("second      :  0x%x\n", timestamp.second); 
    cs_printf("nano_second :  0x%x\n", timestamp.nano_second);
    
    return 0;
}

#endif


static sal_cmd_reg_struct_t ptp_cmd_list[] = 
{
#ifdef HAVE_SDL_CMD_HELP
    {uni_cmd_ptp_cfg,"cfg","ptp configure",uni_cmd_usage_ptp_cfg,sizeof(uni_cmd_usage_ptp_cfg)/4},
    {uni_cmd_ptp_correction_set,"cor","ptp correction cfg",uni_cmd_usage_ptp_correction,sizeof(uni_cmd_usage_ptp_correction)/4},
    {uni_cmd_tod_cfg, "tod","tod ctrl",uni_cmd_usage_tod_cfg,sizeof(uni_cmd_usage_tod_cfg)/4},
    {uni_cmd_tod_info, "tod_info","tod info set",uni_cmd_usage_tod_info,sizeof(uni_cmd_usage_tod_info)/4},
    {uni_cmd_tod_trigger,"trig_tod","tod trigger",uni_cmd_usage_tod_tod_trigger,sizeof(uni_cmd_usage_tod_tod_trigger)/4},
    {uni_cmd_pps_trigger,"trig_pps","pps trigger",uni_cmd_usage_tod_pps_trigger,sizeof(uni_cmd_usage_tod_pps_trigger)/4},
    {uni_cmd_ptp_src,"src","ptp timer src set",uni_cmd_usage_ptp_clk_src, sizeof(uni_cmd_usage_ptp_clk_src)/4},

#ifdef HAVE_MPORTS    
    {uni_cmd_ptp_sync_send,"sync_send","ptp sync packet send for switch",uni_cmd_usage_ptp_sync_pkt_send, sizeof(uni_cmd_usage_ptp_sync_pkt_send)/4},
    {uni_cmd_ptp_sw_time_set,"sw_time_set","ptp system time set for switch",uni_cmd_usage_ptp_sw_time_set, sizeof(uni_cmd_usage_ptp_sw_time_set)/4},
    {uni_cmd_ptp_sw_eg_get,"sw_eg_timestamp_get","ptp egress timestamp get for switch",uni_cmd_usage_ptp_sw_eg_timestamp_get, sizeof(uni_cmd_usage_ptp_sw_eg_timestamp_get)/4},
    {uni_cmd_ptp_sw_port_set,"sw_port_set","ptp port set for switch",uni_cmd_usage_ptp_sw_port_set, sizeof(uni_cmd_usage_ptp_sw_port_set)/4},
    {cmd_ptp_jitter_test_set,"jitter_test","ptp time jitter test enable",cmd_usage_ptp_jitter_test_set, sizeof(cmd_usage_ptp_jitter_test_set)/4}
#endif

#else
    {uni_cmd_ptp_cfg,"ptp"}//,"ptp ctrl",uni_cmd_usage_ptp_cfg,sizeof(uni_cmd_usage_ptp_cfg)/4},
    {uni_cmd_ptp_correction_set,"ptp_cor"}//,"ptp correction cfg",uni_cmd_usage_ptp_correction,sizeof(uni_cmd_usage_ptp_correction)/4},
    {uni_cmd_tod_cfg, "tod"}//"tod ctrl"},uni_cmd_usage_tod_cfg,sizeof(uni_cmd_usage_tod_cfg)/4},
    {uni_cmd_tod_cfg, "tod_info"}//,"tod info set",uni_cmd_usage_tod_info,sizeof(uni_cmd_usage_tod_info)/4},
    {uni_cmd_tod_trigger,"trig_tod"}//,"tod trigger",uni_cmd_usage_tod_tod_trigger,sizeof(uni_cmd_usage_tod_tod_trigger)/4},
    {uni_cmd_pps_trigger,"trig_pps"}//,"pps trigger",uni_cmd_usage_tod_pps_trigger,sizeof(uni_cmd_usage_tod_pps_trigger)/4},
    {uni_cmd_ptp_src,"src"}//,"ptp timer src set",uni_cmd_usage_ptp_clk_src, sizeof(uni_cmd_usage_ptp_clk_src)/4},

#ifdef HAVE_MPORTS    
    {uni_cmd_ptp_sync_send,"sync_send"}//,"ptp sync packet send for switch",uni_cmd_usage_ptp_sync_pkt_send, sizeof(uni_cmd_usage_ptp_sync_pkt_send)/4},
    {uni_cmd_ptp_sw_time_set,"sw_time_set"}//,"ptp system time set for switch",uni_cmd_usage_ptp_sw_time_set, sizeof(uni_cmd_usage_ptp_sw_time_set)/4},
    {uni_cmd_ptp_sw_eg_get,"sw_eg_timestamp_get"}//,"ptp egress timestamp get for switch",uni_cmd_usage_ptp_sw_eg_timestamp_get, sizeof(uni_cmd_usage_ptp_sw_eg_timestamp_get)/4},
    {uni_cmd_ptp_sw_port_set,"sw_port_set"}//,"ptp port set for switch",uni_cmd_usage_ptp_sw_port_set, sizeof(uni_cmd_usage_ptp_sw_port_set)/4},
#endif
#endif
};

sal_cmd_cfg_t ptp_cmd_cfg = {"ptp", "ptp configuration", ptp_cmd_list, sizeof(ptp_cmd_list)/sizeof(sal_cmd_reg_struct_t)};


cs_status ptp_cmd_self_reg(void)
{
        cs_status ret = CS_E_OK;
        ret = sal_cmd_reg(&ptp_cmd_cfg);
        return ret;
}
#endif //#ifdef HAVE_SDL_CMD


