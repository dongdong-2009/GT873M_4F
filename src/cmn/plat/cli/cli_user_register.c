#include "iros_config.h"
#include "cli_common.h"
#include "plat_common.h"
#include "cli.h"
#include "app_ip_init.h"
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <pkgconf/memalloc.h>
#include <pkgconf/isoinfra.h>
#include <stdlib.h>
#include <stdio.h>
#include "sockets.h"
#ifdef HAVE_ONU_RSTP
#include "stp_api.h"
#endif
#ifdef HAVE_IP_STACK
#include "app_ip_api.h"
#endif
#ifdef HAVE_MC_CTRL
#include "mc_api.h"
#include "mc_ctcctrl.h"
#endif
#include "port_stats.h"
#include "sdl_api.h"
#include "packet.h"
#include "app_ip_mgnt.h"
#define cli_ntohs(x)        ((((x) >> 8) & 0xff) | (((x) << 8) & 0xff00))
cs_uint8  uni_max_port_num=0;
//#ifdef HAVE_IP_STACK
// for out-band mgmt
cyg_uint32 g_outband_ip     = (cyg_uint32)0xc0a80001;   //192.168.134.1
cyg_uint32 g_outband_mask   = (cyg_uint32)0xffffff00;   //255.255.255.0
cyg_uint32 g_outband_gateway = (cyg_uint32)0xc0a8000a;   //192.168.134.10
extern cyg_uint8 pon_mgmt_mode ;
// For in-band mgmt,Inband disable by default
cyg_uint32 g_inband_ip     = (cyg_uint32)0xc0a80001;   //192.168.133.1
cyg_uint32 g_inband_mask   = (cyg_uint32)0xffffff00;   //255.255.255.0
cyg_uint32 g_inband_gateway = (cyg_uint32)0xc0a8000a;   //192.168.134.10


cs_uint32 inband_chang = 0;
cs_uint32 outband_chang = 0;

#define SLOW_PATH_MAGIC_VALUE           0x170a53ee
cyg_uint16 arp_vlan[2]= {0,0};
typedef struct {
        /*Outband IP*/
        epon_ipaddr_t   host_ip;    
        epon_mask_t     host_mask;
        epon_ipaddr_t     gateway;
        /*Inband IP*/
        epon_ipaddr_t   inband_ip;
        epon_mask_t     inband_mask;
        epon_ipaddr_t     inband_gateway;
} __attribute__((packed)) epon_onu_ip_config_t;
epon_onu_ip_config_t ipconfig;
#define GWD_PRODUCT_CFG_OFFSET_W  (1024*20)
int ip_address_check_valid( struct cli_def *cli, char * ip_address )
{
   int   value[4];
   char  buffer[16];
   int   i=0;
   int  mini_str_len=0;
   int  value_count=0;
   int  str_len=0;

    memset(value, 0, sizeof(value));
    
    str_len=strlen(ip_address) ;

    if ( str_len>16 ) {
    cli_print(cli, "%% Invalid input");
    return CLI_ERROR;
    }

   strcpy(buffer, ip_address);

   for (i=0; i <str_len;i++) {
        if ( (buffer[i] !='.')
             && (( buffer[i] < '0' ) ||(buffer[i] >'9') )) {
                cli_print(cli, "%% Invalid input");
                return CLI_ERROR;
        }
   }

    /*10.1.1.10*/
       for ( i=0; i < str_len ; i++) {
            if (value_count>=3) {
                value[value_count]=atoi(buffer+i);
                value_count++;
                break;
            }else {
                if ( buffer[i]=='.' ) {
                     buffer[i]='\0';
                     value[value_count]=atoi(buffer+i-mini_str_len);
                     value_count++;
                     mini_str_len=0;
                }else {
                    mini_str_len++;
                }
            }
       }


      if ( ((value[0]==0 )&& (value[1]==0 )&& (value[2]==0 )&& (value[3]==0 ))
            || ((value[0]==255 )&& (value[1]==255 )&& (value[2]==255 )&& (value[3]==255 ))) {
            cli_print(cli, "%% Invalid input ");
            return CLI_ERROR;
      }

       for (i=0; i < 4; i++) {

            if (value[i] > 255) {
                cli_print(cli, "%% value[i]=%d in  Invalid input",  value[i]);
                return CLI_ERROR;
            }
       }

    if (value_count !=4) {
        cli_print(cli, "%% Invalid input");
        return CLI_ERROR;
    }


    return CLI_OK;

}

int cmd_arp_show(struct cli_def *cli, char *command, char *argv[], int argc)
{
    // deal with help
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(0 == argc) {
        epon_request_onu_arp_show(0);
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}

int cmd_arp_clear(struct cli_def *cli, char *command, char *argv[], int argc)
{
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<host>", "IP address with X.X.X.X format",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if (ip_address_check_valid(cli, argv[0]) !=CLI_OK) {
      cli_print(cli, "%% Invalid input");
      return CLI_ERROR;
    }

    if(1 == argc) {
        epon_request_onu_arp_del(argv[0]);
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}

int cmd_ping_host(struct cli_def *cli, char *command, char *argv[], int argc)
{

    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<IP address>", "X.X.X.X",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {

      if (ip_address_check_valid(cli, argv[0]) !=CLI_OK) {
        cli_print(cli, "%% Invalid input, .");
        return CLI_ERROR;
      }
	  
	  extern cs_status app_ipintf_ping(char *host);
		app_ipintf_ping(argv[0]);

    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}
cyg_uint32 iros_iszero( const char *nptr)
{
    if(strcmp(nptr, "0") == 0)
    {
        return 1;
    }
    else if (strcmp(nptr, "0x0") == 0)
    {
        return 1;
    }
    else if (strcmp(nptr, "0X0") == 0)
    {
        return 1;
    }
    else if (strcmp(nptr, "00") == 0)
    {
        return 1;
    }
    return 0;

}
extern cs_uint32 inet_addr(const char *cp);
extern ipintf_info_t ipintf_info;

onu_slow_path_cfg_cfg_t   g_slow_path_ip_cfg;


int cmd_laser_set(struct cli_def *cli, char *command, char *argv[], int argc)
{
    unsigned char  laser_mode=0;
    int                  value=0;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<0-1>", "0==disable, 1=enable",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {
        value= atoi(argv[0]);
        if (value > 1) {
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }

        if (value==0) {
            laser_mode=1;
        }else {
            laser_mode=3;
        }

		/*epon_request_onu_pon_laser_mode_set(context, ONU_DEVICEID_FOR_API,
				ONU_LLIDPORT_FOR_API, laser_mode);*/

    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}
extern cs_boolean gwd_portstats_get_rate(cs_port_id_t portid, cs_uint64 *txrate, cs_uint64 * rxrate);
int cmd_statistics_uni(struct cli_def *cli, char *command, char *argv[], int argc)
{
     cs_port_id_t portid=0;
     oam_port_uni_stats_t uni_stats;
	 cs_uint64 zero_cnt = 0;
     cs_uint64 rxrate = 0,txrate = 0;
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
#ifdef HAVE_MPORTS
        case 1:
            return cli_arg_help(cli, 0,
                "<uni id>", "value in  (1, 4)",
                 NULL);
#else
        case 1:
            return cli_arg_help(cli, 0,
                "<uni id>", "value in  (1, 1)",
                 NULL);
#endif


        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }


    if(1 == argc)
    {
        portid=atoi(argv[0]);

        if ((portid > uni_max_port_num)
            || (portid < 1) ) {
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }
        //portid--;
        
        if(CS_E_OK != app_onu_port_stats_get(portid, &uni_stats))
        {
            return CS_E_ERROR;
        }
		cli_print(cli,"----------------------------------------------------------------------");
        cli_print(cli,"%-30s: %lld", "RxFramesOk", uni_stats.rxfrm_cnt);
        cli_print(cli,"%-30s: %lld","RxUnicasts" , uni_stats.rxucfrm_cnt);
        cli_print(cli,"%-30s: %lld", "RxMulticasts", uni_stats.rxmcfrm_cnt);
        cli_print(cli,"%-30s: %lld", "RxBroadcasts", uni_stats.rxbcfrm_cnt);
        cli_print(cli,"%-30s: %lld", "Rx64Octets",  uni_stats.rxstatsfrm64_cnt);
        cli_print(cli,"%-30s: %lld", "Rx127Octets",  uni_stats.rxstatsfrm65_127_cnt);
        cli_print(cli,"%-30s: %lld", "Rx255Octets",uni_stats.rxstatsfrm128_255_cnt);
        cli_print(cli,"%-30s: %lld", "Rx511Octets", uni_stats.rxstatsfrm256_511_cnt);
        cli_print(cli,"%-30s: %lld", "Rx1023Octets", uni_stats.rxstatsfrm512_1023_cnt);
        cli_print(cli,"%-30s: %lld","RxMaxOctets",  uni_stats.rxbyte_cnt);
        cli_print(cli,"%-30s: %lld", "RxJumboOctets", zero_cnt);
        cli_print(cli,"%-30s: %lld", "RxUndersize", uni_stats.rxundersizefrm_cnt);
        cli_print(cli,"%-30s: %lld", "RxOversize", uni_stats.rxoversizefrm_cnt);
        cli_print(cli,"%-30s: %lld", "RxFragments", zero_cnt);
        cli_print(cli,"%-30s: %lld", "RxJabber",  uni_stats.rxjabberfrm_cnt);
        cli_print(cli,"%-30s: %lld",  "RxFCSErrors",  (cs_uint64)uni_stats.fcs_errors);
        cli_print(cli,"%-30s: %lld", "RxDiscards", zero_cnt);
        cli_print(cli,"%-30s: %lld", "RxAlignErrors", zero_cnt);
        cli_print(cli,"%-30s: %lld", "RxIntMACErrors", zero_cnt);
        cli_print(cli,"%-30s: %lld", "RxPppoes",  zero_cnt);
        cli_print(cli,"%-30s: %lld", "RxQueueFull", zero_cnt);
        cli_print(cli,"%-30s: %lld", "RxPause" , uni_stats.rxpausefrm_cnt);
        cli_print(cli,"%-30s: %lld", "RxOctetsOkMsb",  zero_cnt);
        cli_print(cli,"%-30s: %lld", "RxOctetsOkLsb", uni_stats.rxbyte_cnt);
        cli_print(cli,"%-30s: %lld","TxFramesOk",  uni_stats.txfrm_cnt);
        cli_print(cli,"%-30s: %lld","TxUnicasts" , uni_stats.txucfrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxMulticasts", uni_stats.txmcfrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxBroadcasts", uni_stats.txbcfrm_cnt);
        cli_print(cli,"%-30s: %lld", "Tx64Octets", uni_stats.txstatsfrm64_cnt);
        cli_print(cli,"%-30s: %lld", "Tx127Octets", uni_stats.txstatsfrm65_127_cnt);
        cli_print(cli,"%-30s: %lld", "Tx255Octets", uni_stats.txstatsfrm128_255_cnt);
        cli_print(cli,"%-30s: %lld", "Tx511Octets" ,  uni_stats.txstatsfrm256_511_cnt);
        cli_print(cli,"%-30s: %lld", "Tx1023Octets", uni_stats.txstatsfrm512_1023_cnt);
        cli_print(cli,"%-30s: %lld", "TxMaxOctets",  uni_stats.txbyte_cnt);
        cli_print(cli,"%-30s: %lld", "TxJumboOctets", zero_cnt);
        cli_print(cli,"%-30s: %lld","TxDeferred",  zero_cnt);
        cli_print(cli,"%-30s: %lld","TxTooLongFrames",  uni_stats.txoversizefrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxCarrierErrFrames",  zero_cnt);
        cli_print(cli,"%-30s: %lld", "TxSqeErrFrames", zero_cnt);
        cli_print(cli,"%-30s: %lld", "TxSingleCollisions", uni_stats.txsinglecolfrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxMultipleCollisions",  uni_stats.txmulticolfrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxExcessiveCollisions", uni_stats.txexesscolfrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxLateCollisions", uni_stats.txlatecolfrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxMacErrFrames", zero_cnt);
        cli_print(cli,"%-30s: %lld", "TxQueueFull ", uni_stats.txexesscolfrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxPause",  uni_stats.txpausefrm_cnt);
        cli_print(cli,"%-30s: %lld", "TxOctetsOk" , uni_stats.txbyte_cnt);
		cli_print(cli,"-----------------------------------------------------------------------");
		#if 1
		if(gwd_portstats_get_rate(portid-1, &txrate, &rxrate) == EPON_TRUE)
			{
				cli_print(cli, "  tx port rate %lld bps\t\t rx port rate %lld bps", txrate, rxrate);
			}
		else
			{
					return(-1);
			}
		#endif
    } 
    else
    {
        cli_print(cli, "%% Invalid input.");
    }


    return CLI_OK;
}

int cmd_statistics_cpu(struct cli_def *cli, char *command, char *argv[], int argc)
{
	/*cs_uint32 zero_cnt = 0;*/

    // deal with help
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;
	
    if(0 == argc)
    {
        /*cli_print(cli,"%-30s: %d", "frm_tx_ok", zero_cnt);
        cli_print(cli,"%-30s: %d", "frm_tx_olt", zero_cnt);
        cli_print(cli,"%-30s: %d", "frm_tx_imstar", zero_cnt);
        cli_print(cli,"%-30s: %d", "frm_tx_err", zero_cnt);
        cli_print(cli,"%-30s: %d", "frm_rx_err", zero_cnt);
        cli_print(cli,"%-30s: %d", "frm_rx_onu", zero_cnt);*/
        cli_print(cli,"%-30s: %d", "frm_rx_eapol", app_pkt_get_counter(CS_PKT_8021X));
        cli_print(cli,"%-30s: %d", "frm_rx_igmp", app_pkt_get_counter(CS_PKT_GMP));
        cli_print(cli,"%-30s: %d", "frm_rx_imstar", app_pkt_get_counter(CS_PKT_MYMAC));
        /*cli_print(cli,"%-30s: %d", "frm_rx_mgmt", app_pkt_get_counter(CS_PKT_MYMAC));
        cli_print(cli,"%-30s: %d", "frm_rx_dropped", zero_cnt);
        cli_print(cli,"%-30s: %d", "frm_rx_flushed", zero_cnt);*/
        cli_print(cli,"%-30s: %d", "frm_rx_ip", app_pkt_get_counter(CS_PKT_IP));
        cli_print(cli,"%-30s: %d", "frm_rx_arp",app_pkt_get_counter(CS_PKT_ARP));
        cli_print(cli,"%-30s: %d", "frm_rx_dhcp",app_pkt_get_counter(CS_PKT_DHCP));
        cli_print(cli,"%-30s: %d", "frm_rx_stp",app_pkt_get_counter(CS_PKT_BPDU));
        /*cli_print(cli,"%-30s: %d", "frm_rx_onu_mii", zero_cnt);
        cli_print(cli,"%-30s: %d", "frm_rx_imstar_mii", zero_cnt);
        cli_print(cli,"%-30s: %d", "frm_rx_dropped_mii", zero_cnt);*/
    } 
    else
    {
        cli_print(cli, "%% Invalid input.");
    }


    return CLI_OK;
}


int cmd_gpio_set(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int                 pin_number=0;
    int                 status=0;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<pin id>", "value in  (12, 12)",
                 NULL);

         case 2:
            return cli_arg_help(cli, 0,
            "<status>", " set pin status, value in (0, 1)",
             NULL);

        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(2 == argc)
    {
        pin_number=atoi(argv[0]);
        status=atoi(argv[1]);

        if (pin_number != 12) {
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }
        if ((status != 0 ) && (status !=1) ) {
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }

//        epon_onu_GPIO_write_status( pin_number,  status);

    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}


int cmd_gpio_get(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int                 pin_number=0;
    unsigned char status=0;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<pin id>", "value in  (12, 12)",
                 NULL);

        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {
        pin_number=atoi(argv[0]);

        if (pin_number != 12) {
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }

//        epon_onu_GPIO_read_status(pin_number, &status);
        cli_print(cli, "pin %d status is %d ", pin_number , status);

    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}

#if 0//ndef HAVE_MPORTS
void cli_callback_onu_qos_mode_get(
        cs_callback_context_t                       context,
        epon_result_code_t                            code,
        epon_onu_qos_mode_config2_t                   *result)

/*                                                              */
/* INPUTS     : o callback context pointer                      */
/*              o result code defined in iros_common.h          */
/* OUTPUTS    : ----                                            */
/* RETURNS    : o EPON_RETURN_XXX (operation status)            */
/* DESCRIPTION:                                                 */
/* sample callback function for onu qos_mode2 get               */
/*                                                              */
/* $rtn_hdr_end                                                 */
/****************************************************************/
{
    if (code != EPON_RESULT_SUCCESS || !result) {
        return;
    }

    memcpy(&qos_result, result, sizeof(epon_onu_qos_mode_config2_t));

}
#endif

#if 0
int cmd_printf_qos_infor(struct cli_def * cli)
{
    cs_int32    i=0;
    cs_int8     us_policy_enable=0;
    cs_int8     dn_policy_enable=0;
    cs_uint8 status=0;

//    epon_sw_port_inratelimit_mode_t  mode;
#ifdef HAVE_MPORTS
//    cs_uint32   rate=0;
    epon_sw_port_policy_struct_t ingress_limit, egress_limit;
#else

#ifdef HAVE_CTC_OAM
    oam_ctc_onu_port_policing_struct    config;
    oam_ctc_onu_ds_rate_limit_struct   downstream_config;
#endif
    epon_bc_unknown_uc_police_config2_t    storm_config;
    cs_callback_context_t     context;

    context.invoke_id =0;
    context.apps_data = 0;
#endif


    cli_print(cli, "!QOS");

    for (i=1; i < uni_max_port_num+1 ;i++) {
        cli_print(cli, "!ethernet port %d",  i);

		ctc_oam_eth_port_pause_get_adapt(i,&status);
#ifdef HAVE_MPORTS
       // epon_onu_sw_get_port_stormctrl(i, &mode, &rate);
        epon_onu_sw_read_port_eth_pause(i,  &status);
        epon_onu_sw_read_port_ingress_policy(i, &ingress_limit);
        us_policy_enable=ingress_limit.enable;
        epon_onu_sw_get_port_egress_rate_limit(i , &egress_limit);
        dn_policy_enable=egress_limit.enable;
#else
         epon_request_onu_qos_mode_get2(cli_callback_onu_qos_mode_get,  context, 0);
        memset( &storm_config, 0, sizeof( storm_config));
        storm_config.port_id=0x30400000;
        epon_onu_get_port_storm_ctrl(&storm_config);
#ifdef HAVE_CTC_OAM
        epon_request_ctc_onu_eth_port_pause_read(i, &status);
        epon_request_ctc_onu_eth_port_policing_read(i, &config);
        us_policy_enable=config.portOper;
        epon_request_ctc_onu_eth_ds_rate_limit_read(i, &downstream_config);
        dn_policy_enable= downstream_config.portOper;
#endif

        cli_print(cli ,"%-30s: %s  \n", "qos mode",  qos_result.dscp==EPON_QOS_DSCP ? "DSCP":"DOT1P");
        if ( storm_config.bc_police_en==1 ) {
            cli_print(cli, "%-30s: rate(%d)" , "broadcast rate limit enable", storm_config.bc_police_rate);
        } else {
             cli_print(cli, "%-30s" , "broadcast rate limit disable");
        }
#endif

//        cli_print(cli, "%-30s: %d", "storm control mode",  mode);
  //      cli_print(cli, "%-30s: %dKbps","rate" , rate);

        cli_print(cli, "%-30s: %d" , "ethernet pause status", status);
        cli_print(cli, "%-30s: %d" ,"ingress policy",us_policy_enable);
        cli_print(cli, "%-30s: %d\n" ,"egress policy", dn_policy_enable);


    }

    cli_print(cli, "\r\n");

   return 0;
}

int cmd_printf_classification_infor(struct cli_def * cli)
{
    cs_uint8                    num=0;
    int                                   ret=0;
    cs_int32                    portid=0;
    oam_ctc_onu_classification_struct   rule[CTC_CLASS_RULES_MAX];
    cs_int32                    i=0;
    unsigned char                   entries_num=0;

     cli_print(cli, "!Classification");

#ifdef HAVE_CTC_OAM
       for (portid=1;portid< uni_max_port_num+1; portid++) {
            ret=epon_request_ctc_onu_classify_rules_read(portid, &num, rule);
            if (ret ==EPON_RETURN_SUCCESS) {
                for ( i=0; i < num; i++ ) {
                    cli_print(cli, "%-30s: %d", "precedence",  rule[i].precedence);
                    cli_print(cli, "%-30s: %d", "len", rule[i].len);
                    cli_print(cli, "%-30s: %d", "queueMapped", rule[i].queueMapped);
                    cli_print(cli, "%-30s: %d", "priMarking", rule[i].priMark);
                    cli_print(cli, "%-30s: %d", " entries number", rule[i].entries);
                    for(entries_num = 0; entries_num < rule[i].entries; entries_num++) {
                         cli_print(cli, "%-30s: %d", "entry", entries_num);
                        cli_print(cli, "%-30s: %d", "fieldSelect", rule[i].fselect[entries_num].fieldSelect);
                        cli_print(cli, "%-30s: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                                "matchValue",
                                rule[i].fselect[entries_num].matchValue[0],
                                rule[i].fselect[entries_num].matchValue[1],
                                rule[i].fselect[entries_num].matchValue[2],
                                rule[i].fselect[entries_num].matchValue[3],
                                rule[i].fselect[entries_num].matchValue[4],
                                rule[i].fselect[entries_num].matchValue[5]);
                        cli_print(cli, "%-30s: %d", "validationOper", rule[i].fselect[entries_num].validationOper);
                    }
                    cli_print(cli, "\r\n");
                }
            }
       }
#endif

    cli_print(cli, "\r\n");

    return 0;
}
#endif

int cmd_printf_system_infor(struct cli_def *cli)
{
	show_system_status();
	return 0;
}

#if 0
int cmd_printf_ctc_vlan_infor(struct cli_def *cli)
{
    cs_uint8  buffer[1280];
    int                 i=0,j=0;
    cs_uint8 vlan_mode;
    cs_uint32 len;
    cs_uint8 * config= (cs_uint8 *) buffer;

    cli_print(cli, "!CTC VLAN");


    for (i=1; i < uni_max_port_num+1; i++) {
        epon_request_ctc_onu_port_vlan_read(i , &vlan_mode, &len, config);
        cli_print(cli, "!ethernet port %d",  i);
        if (vlan_mode==CTC_VLAN_MODE_TRANSPARENT) {
            cli_print(cli, "%-30s", "port transparent VLAN\n");
        }
        if (vlan_mode==CTC_VLAN_MODE_TAG) {
            cli_print(cli, "%-30s: tpid:%d, default VLAN ID:%d\n",  " port tag VLAN", ((config[0]<<8)&0xff00)+config[1],  ((config[2]<<8)&0xff00)+config[3]);
        }

        if (vlan_mode ==CTC_VLAN_MODE_TRANSLATE) {
            cli_print(cli, "%-30s: tpid:%d, default VLAN ID:%d", "port translate VLAN", (config[0]<<8)+config[1],  (config[2]<<8)+config[3]);
            for (j=0; j < (len-4)/8;j++ ) {
                cli_print(cli, "%-30s VLAN ID pair:(%d, %d)" , " ", ((config[j*8+4+2]<<8)&0xff00)+config[j*8+4+3],  ((config[j*8+4+6]<<8)&0xff00)+config[j*8+4+7]);
            }
        }

    }


    cli_print(cli, "\r\n");

    return 0;

}
#endif

#ifdef HAVE_ONU_RSTP
int cmd_printf_stp_infor(struct cli_def *cli)
{
    cs_onu_stp_state_config_t             bridge_state;
    cs_onu_port_stp_state_config_t     port_state;
    cs_int32                                    i=0;
    char * state_table[]={"unkown state","Disable", "Blocking", "Listening","Learning", "Forward", "Broken" };
    char * role_table[]={"DisabledPort","AlternatePort", "BackupPort", "RootPort" , "DesignatedPort","NonStpPort" };


    app_stp_bridge_state_get(&bridge_state);
    /*show Bridge infor*/
    cli_print(cli, "!RSTP");
    cli_print(cli, "%-30s: %s", "bridge RSTP:", bridge_state.stp_enable==1 ? "enabled" :"disabled" );
    cli_print(cli, "%-30s: %d.-%02x%02x.%02x%02x.%02x%02x"
                , "The bridge ID (Pri.-MAC)"
                , bridge_state.bridge_prio,bridge_state.bridge_addr[0], bridge_state.bridge_addr[1]
                , bridge_state.bridge_addr[2], bridge_state.bridge_addr[3]
                , bridge_state.bridge_addr[4], bridge_state.bridge_addr[5]);

    cli_print(cli, "%-30s: Hello Time %d sec, Max Age %d sec, Forward Delay %d sec"
                , "The bridge times"
                , bridge_state.hello_time, bridge_state.max_age, bridge_state.forward_delay  );
    cli_print(cli, "%-30s: %d.-%02x%02x.%02x%02x.%02x%02x"
                ,"Root bridge ID(Pri.-MAC)"
                , bridge_state.root_bridge_prio,bridge_state.root_bridge_addr[0], bridge_state.root_bridge_addr[1]
                , bridge_state.root_bridge_addr[2], bridge_state.root_bridge_addr[3]
                , bridge_state.root_bridge_addr[4], bridge_state.root_bridge_addr[5]);

    cli_print(cli, "%-30s: %lld"
        , "Root path cost"
        , bridge_state.root_path_cost );

    cli_print(cli, "%-30s: %d"
        , "Root port"
        , bridge_state.root_port);

    cli_print(cli, "\r\n");
    /*show port infor*/
    for (i=1; i <uni_max_port_num+1  ;i++) {
        cli_print(cli, "!");
        app_stp_port_state_get(i , &port_state);
        cli_print(cli, "Port %d  of bridge is %s", i, state_table[(int)port_state.state] );
        cli_print(cli, "%-30s: %s"
                            , "Port spanning tree protocol", port_state.stp_enable==1 ?"enabled":"disable");
        cli_print(cli, "%-30s: %s", "Port role",  role_table[port_state.role] );
        cli_print(cli, "%-30s: %lld","Port path cost", port_state.path_cost );
        cli_print(cli, "%-30s: %d", "Port priority", port_state.port_prio );

        cli_print(cli, "%-30s:%d.-%02x%02x.%02x%02x.%02x%02x"
                , "Designated bridge ID(Pri.-MAC)"
                , port_state.designated_bridge_prio,port_state.designated_bridge_addr[0], port_state.designated_bridge_addr[1]
                , port_state.designated_bridge_addr[2], port_state.designated_bridge_addr[3]
                , port_state.designated_bridge_addr[4], port_state.designated_bridge_addr[5]);

        cli_print(cli, "The Port is a %s port", port_state.oper_edge==1 ?"edged":"non-edged");

        cli_print(cli, "Connected to a %s LAN segment", port_state.oper_point2point==1 ?"point-to-point ":"non-point-to-point ");

        cli_print(cli, "BPDU sent:" );
        cli_print(cli, "RST: %lld, Config BPDU: %lld, TCN: %lld "
                , port_state.tx_rstp_bpdu_cnt, port_state.tx_cfg_bpdu_cnt, port_state.tx_tcn_bpdu_cnt);

        cli_print(cli, "BPDU received:" );
        cli_print(cli, "RST: %lld, Config BPDU: %lld, TCN: %lld "
                , port_state.rx_rstp_bpdu_cnt, port_state.rx_cfg_bpdu_cnt, port_state.rx_tcn_bpdu_cnt);
        cli_print(cli, "\r\n");

    }


    return 0;

}
#endif

#if 0//ndef HAVE_MPORTS
void cmd_onu_fdb_get_callback(cs_callback_context_t context,
                        epon_result_code_t code, epon_result_fdb_get_t *result)

{
        if (code != EPON_RESULT_SUCCESS) {
                return;
        }

        fdb_old_time=result->config.aging_time;

}
#endif


int cmd_printf_age_time_infor(struct cli_def *cli)
{
    cs_uint32  time = 0;
    cs_callback_context_t     context;

    epon_request_onu_fdb_age_get(context, 0, 0, &time);

    cli_print(cli, "!");
    cli_print(cli, "%-30s: %d", "FDB age time", time);
    cli_print(cli, "\r\n");

    return 0;
}

#ifdef HAVE_MPORTS
int cmd_printf_isolation_infor(struct cli_def *cli)
{
    cs_boolean enable=0;
	cs_callback_context_t context;

    cli_print(cli, "!isolation");
    epon_request_onu_port_isolation_get(context, 0, 0, &enable);
    cli_print(cli, "%s isolation\n", enable==1 ? "enable": "disable");
	
    cli_print(cli, "\r\n");

    return 0;
}
#endif

#if 0
int cmd_printf_mirror_infor(struct cli_def *cli)
{
    cs_int32    i=0;
    epon_sw_port_mirror_mode_t mode;
    cs_port_id_t  ingress_port_id,   egress_port_id=0;
    char *p_str=NULL;

    cli_print(cli, "!mirror");
    for ( i=1; i < uni_max_port_num+1; i++) {
        cli_print(cli, "!ethernet port %d", i);
        epon_onu_sw_get_port_mirrormode(i , &mode);
        switch (mode) {
            case    PORT_MIRROR_NORMAL:
                        p_str="port disable mirror\n";
                        break;

            case    PORT_MIRROR_INGRESS:
                        p_str="port ingress mirror\n";
                        break;

            case    PORT_MIRROR_EGRESS:
                        p_str="port egress mirror\n";
                        break;

            case    PORT_MIRROR_BOTH:
                        p_str="port egress and ingress mirror\n";
                        break;

            default :
                       p_str="porterror mirror";
        }

        cli_print(cli, "%-30s",  p_str );
    }

    epon_onu_sw_get_monitordst(&ingress_port_id , &egress_port_id);
    cli_print(cli, "ingress port:%4d, egress port:%4d" , ingress_port_id, egress_port_id );

    cli_print(cli, "\r\n");
    return 0;

}

int cmd_printf_fdb_infor(struct cli_def *cli)
{

    int i = 0;
    int count=0;
    char  mac_buffer[24];

#ifdef HAVE_MPORTS
    epon_sw_fdb_entry_t entry;
    cs_boolean   result;
    char  port_buffer[24];
#else
    onu_aal_fdb_t   entry;
    cs_int32    ret=0;
    cs_int32    fdb_index=0;
#endif



    cli_print(cli, "!FDB");
    cli_print(cli, "%-30s %-8s %-10s", "MAC address", "vlan Id", "egress port");




#ifdef HAVE_MPORTS
    memset(&entry, 0, sizeof(entry));
    entry.vlan_id = 0;
    while( (EPON_RETURN_SUCCESS == epon_onu_sw_get_next_fdb_entry(0, &entry, &result))
            && result )
    {
        if (entry.status !=2) {
            continue;
        }
        count=0;
        for(i=1;i<uni_max_port_num+1 ;i++) {
            if(entry.egress_portmap & (1<< (i-1))) {
                count+=sprintf(port_buffer+count, "%d ",i );
            }
        }
        count=0;
        for (i=0; i <6; i++) {
            count+=sprintf(mac_buffer+count, "%02x%s", entry.addr[i], i==5?" ":"-");
        }
        cli_print(cli, "%-30s %-8d %-10s", mac_buffer, entry.vlan_id, port_buffer);

    }

#else

    for ( fdb_index=ONU_L2_START; fdb_index <ONU_L2_END ;fdb_index++) {
        ret=epon_onu_get_fdb_entry (fdb_index , &entry );
        if ( (ret ==0)
            && (entry.status== EPON_FDB_ENTRY_MGMT )) {
             count=0;
            for (i=0; i <6; i++) {
                count+=sprintf(mac_buffer+count, "%02x%s", entry.mac[i], i==5?" ":"-");
            }
            cli_print(cli, "%-30s %-8d %-10d", mac_buffer, entry.vlanid, 1);
        }
    }

#endif

    cli_print(cli, "\r\n");

    return 0;
}
#endif

int cmd_show_run_system(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;




    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }


    /*system inforamtion*/
    cmd_printf_system_infor(cli);
    return 0;


}

#ifdef HAVE_CTC_OAM
int cmd_show_run_ctc_vlan(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;




    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }


   // cmd_printf_ctc_vlan_infor( cli);
     return 0;

}
#endif

#ifdef  HAVE_MC_CTRL
typedef struct {
    cs_uint16 userId;
    cs_uint16 vlanId;
    cs_uint8 mc_da[6];
} __attribute__((packed)) oam_ctc_onu_mc_control_vlan_mac_t;

int cmd_printf_igmp_infor(struct cli_def *cli)
{
    cs_sdl_mc_vlan_act_t   opr=0;
	mc_mode_t igmp_mode;
    cs_int32    i=0,j=0;
    cs_int32    portid=0;
    cs_uint16  max_group=0;
    cs_uint8    buffer[1280];
    cs_uint16 num=0;
	mc_ipv4_grp_t v4_tbl[64];
	cs_uint32 v4_tbl_len = 64;
	mc_ipv6_grp_t v6_tbl[64];
	cs_uint32 v6_tbl_len = 64;
	cs_uint32 id;
    unsigned int   out_group=0, port_map=0;
    cs_sdl_vlan_cfg_t             swap_rule[20];
    unsigned short  * pVlanId=(unsigned short  *) buffer;
    char                temp_buffer[128];
    int                 str_len=0;
    oam_ctc_onu_mc_control_vlan_mac_t *pDaVid= (oam_ctc_onu_mc_control_vlan_mac_t *)buffer;
	
    cs_status    ret = CS_E_OK;
	cs_callback_context_t context;
	
    /*show multicast mode*/
    ret = mc_mode_get(0,&igmp_mode);
    cli_print(cli, "!IGMP");
    cli_print(cli, "%-30s:%s\n" ,"IGMP mode",  igmp_mode == MC_SNOOPING ? "IGMP snooping": "CTC IGMP");

    if (igmp_mode == MC_SNOOPING) {
        /*IGMP group entry*/
        str_len=0;

        cli_print(cli, "%-30s %-8s", "IPv4 multicast group", "port id");
        
        mc_get_grp_tbl(0, v4_tbl, &v4_tbl_len, v6_tbl, &v6_tbl_len);

        for(id = 0; id < v4_tbl_len; id++) {
			out_group = v4_tbl[id].grp;
			port_map = v4_tbl[id].port_map;
			
            sprintf(buffer, "%d.%d.%d.%d "
            , ((out_group&0xff000000) >>24), ((out_group&0xff0000) >>16), (( out_group&0xff00) >>8),  (out_group &0xff) );
            str_len=0;
            for (i=1; i <uni_max_port_num+1; i++) {
                if (port_map & (0x1 << i)){
                    str_len+=sprintf(temp_buffer+str_len, "%d ", i );
                }
            }
            cli_print(cli, "%-30s %-8s", buffer, temp_buffer);
        }

        cli_print(cli, "%-30s %-8s", "IPv6 multicast group", "port id");
        for(id = 0; id < v6_tbl_len; id++) {
			port_map = v6_tbl[id].port_map;
			
            sprintf(buffer, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x ", 
					v6_tbl[id].grp[1], v6_tbl[id].grp[0],
                    v6_tbl[id].grp[3], v6_tbl[id].grp[2],
                    v6_tbl[id].grp[5], v6_tbl[id].grp[4],
                    v6_tbl[id].grp[7], v6_tbl[id].grp[6]);
			
            str_len=0;
            for (i=1; i <uni_max_port_num+1; i++) {
                if (port_map & (0x1 << i)){
                    str_len+=sprintf(temp_buffer+str_len, "%d ", i );
                }
            }
            cli_print(cli, "%-30s %-8s", buffer, temp_buffer);
			
    	}

        cli_print(cli, "\r\n");

    }else     {
        cli_print(cli, "IGMP control list :");

		mc_ctc_ctrl_get_entries(buffer, &num);
        cli_print(cli, "%-30s %-6s %-10s", "MAC", "portid", "VLAN ID");

        str_len=0;
        for (i=0; i < num; i++) {
           sprintf(temp_buffer,"%02x-%02x-%02x-%02x-%02x-%02x", 
		   	pDaVid[i].mc_da[0], pDaVid[i].mc_da[1], 
		   	pDaVid[i].mc_da[2], pDaVid[i].mc_da[3],
            pDaVid[i].mc_da[4], pDaVid[i].mc_da[5] );
            cli_print(cli, "%-30s %-6d %-10d", temp_buffer, cli_ntohs(pDaVid[i].userId),cli_ntohs(pDaVid[i].vlanId));

        }
        cli_print(cli, "\r\n");
    }

    for (portid = 1 ; portid < uni_max_port_num+1; portid++) {
        cli_print(cli , "!ethernet port %d", portid );
        if ( igmp_mode == MC_SNOOPING) {
			
             /*max group*/
            mc_port_max_grp_get(0, portid, &max_group);
            cli_print(cli, "%-30s:%d","max multicast group" , max_group);

              /*multicast vlan*/
			  ret = epon_request_onu_mc_vlan_get(context,0,0, portid, pVlanId, &num);
            if (ret == 0) {
                str_len = 0;
                for (j=0; j < num; j++) {
                    str_len += sprintf(temp_buffer+str_len, "%d%s",cli_ntohs(pVlanId[j]) ,((j+1)%8)==0?"\r\n":" ");
                }
                if ( num >0 ) {
                    cli_print(cli, "%-30s: %s",  "CTC multicast VLAN", temp_buffer);
                }
            }
        }

        /*Multicast Tagstrip*/
        str_len = 0;

		epon_request_onu_mc_vlan_action_get(context,0,0, portid, &opr, swap_rule,&num);

        if (opr == SDL_MC_VLAN_TAG_KEEP) {
            str_len += sprintf(buffer+str_len, "port no tag strip" );
        }
        if (opr == SDL_MC_VLAN_TAG_STRIP) {
            str_len+=sprintf(buffer+str_len, "port tag strip" );
        }

        if (opr == SDL_MC_VLAN_TAG_SWAP) {
			
            str_len+=sprintf (buffer+str_len, "%s " , "port tag replace multicast vlan by iptv vlan:");
            for (j=0; j < num;  ++j) {
                str_len+=sprintf(buffer+str_len, " (%d, %d)", swap_rule[j].c_vlan.vid, swap_rule[j].s_vlan.vid);
            }

        }
		
        cli_print (cli, "%-30s"  , buffer);


        cli_print(cli, "\r\n");
    }


    cli_print(cli, "\r\n");

    return CLI_OK;

}

int cmd_show_run_igmp(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }


    cmd_printf_igmp_infor( cli);
     return 0;

}
#endif

int cmd_show_run_qos(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

     return 0;
}

#ifdef HAVE_ONU_RSTP
int cmd_show_run_stp(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }


    cmd_printf_stp_infor( cli);

     return 0;

}
#endif

int cmd_show_run(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "system", "system information", "vlan", "ctc VLAN information", "igmp", "the IGMP information",
                "qos", "the QOS information", "stp", "the STP information"
                "<cr>", "",
                NULL);
        case 2:
            return cli_arg_help(cli, 0,
                "<1-16>", "default value 16",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }


    /*system inforamtion*/
    cmd_printf_system_infor(cli);


#ifdef HAVE_MC_CTRL
    /*IGMP*/
    cmd_printf_igmp_infor(cli);
#endif

#ifdef  HAVE_ONU_RSTP
    /*STP*/
    cmd_printf_stp_infor(cli);
#endif

    /*FDB age time*/
    cmd_printf_age_time_infor(cli);

#ifdef HAVE_MPORTS
    /*port isolation mode*/
    cmd_printf_isolation_infor(cli);
#endif

    return CLI_OK;
}
int cmd_port_stats_enable(struct cli_def *cli, char *command, char *argv[], int argc);
int cmd_port_stats_disable(struct cli_def *cli, char *command, char *argv[], int argc);
extern cs_status ctc_onu_stats_monitor_start(
        cs_port_id_t port,
        cs_uint32 interval);
extern cs_status ctc_onu_stats_monitor_stop(cs_port_id_t port);
int cmd_port_stats_enable(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int  ret = CLI_ERROR;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
        	return cli_arg_help(cli, 0,
        			"<portid>", "port id selected",
        			NULL);
        case 2:
            return cli_arg_help(cli, 0,
                "<0-3600>", "statistic interval, unit:second",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(2 == argc)
    {
    	cs_port_id_t port = atoi(argv[0]);
    	cs_uint32 interval = atoi(argv[1]);

    	if(port >= 1 && port <= 4 && interval >= 0 && interval <=3600)
    	{
			ctc_onu_stats_monitor_start(port, interval);
			cli_print(cli,"port %d stats enable success\n",port);
    	}
    	else
    	{
    		cli_print(cli, "%% Invalid input.");
    	}
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return ret;
}
int cmd_port_stats_disable(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int  ret = CLI_ERROR;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
        	return cli_arg_help(cli, 0,
        			"<portid>", "port id selected",
        			NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {
    	cs_uint32 port = atoi(argv[0]);

    	if(port >= 1 && port <= 4)
    	{
			ctc_onu_stats_monitor_stop(port);
			cli_print(cli,"port %d stats disable success\n",port);
    	}
    	else
    	{
    		cli_print(cli, "%% Invalid input.");
    	}
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return ret;
}

extern int cmd_if_config(struct cli_def *cli, char *command, char *argv[], int argc);

void user_register_command_entry(struct cli_command **cmd_root)
{
#ifdef HAVE_IP_STACK
    struct cli_command *arp, *arp_show, *arp_clear;
    struct cli_command *ping;
	struct cli_command *ifconfig;
#endif
    struct cli_command *statistics, * statistics_cpu;
    struct cli_command *portstats;

    if(CS_E_OK != startup_config_read(CFG_ID_SWITCH_PORT_NUM, 1, &uni_max_port_num))
    {
    	#if 0
        uni_max_port_num = 1;
		#else
		uni_max_port_num = 4;
		#endif
    }

#ifdef HAVE_IP_STACK
    /*ARP*/
    arp = cli_register_command(cmd_root, NULL, "arp",  NULL,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "ARP configuration");
    arp_show = cli_register_command(cmd_root, arp, "show",  cmd_arp_show, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Show ARP table");
    arp_clear = cli_register_command(cmd_root, arp, "clear",  cmd_arp_clear,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Clear ARP table");

    /*Ping*/
    ping = cli_register_command(cmd_root, NULL, "ping",  cmd_ping_host,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Ping the specified host");
    ifconfig = cli_register_command(cmd_root, NULL, "ifconfig",  cmd_if_config,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Set ONU device ip And Management");   

#endif

    /*statistics*/
    statistics = cli_register_command(cmd_root, NULL, "statistics",  NULL,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Show system statistics information");
    /*statistics_pon = cli_register_command(cmd_root, statistics, "pon",  cmd_statistics_pon,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Show pon statistics");*/
    portstats = cli_register_command(cmd_root, statistics, "port", NULL, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "port statistics");
		cli_register_command(cmd_root, portstats, "enable", cmd_port_stats_enable, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "enable command");
		cli_register_command(cmd_root, portstats, "disable",cmd_port_stats_disable, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "disable command");
		cli_register_command(cmd_root, portstats, "show", cmd_statistics_uni, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "show port statistics");

    statistics_cpu = cli_register_command(cmd_root, statistics, "cpu",  cmd_statistics_cpu,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Show cpu statistics");

#if 0
    /*GPIO*/
    gpio = cli_register_command(cmd_root, NULL, "gpio",  NULL,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "GPIO configuration");
    gpio_set = cli_register_command(cmd_root, gpio, "set",  cmd_gpio_set,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Set GPIO pin");
    gpio_get = cli_register_command(cmd_root, gpio, "get",  cmd_gpio_get,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Get GPIO pin status");
#endif
    return;
}


#if 1
typedef struct device_ip
{
	cs_uint32	device_ip;		//采用本地字节序
	cs_uint32	device_mask;	//采用本地字节序
	cs_uint32	device_gateway;	//采用本地字节序
	cs_uint16	device_vlan;	//采用本地字节序
}device_ip_t;

extern char *inet_ntoa(struct in_addr ina);

extern int inet_aton(const char *cp, struct in_addr *addr);

extern cs_status app_ipintf_add_ip_address(cs_uint32 localIp, cs_uint32 gwIp, cs_uint32 mask);
extern cs_status ip_mode_set(int mode);


static int ip_mode = 1;		//0-uart,1-device, 2-pty
extern cs_status ip_mode_set(int mode)
{
	cs_status ret = CS_E_OK;
	ip_mode = mode;

	return ret;
}
extern cs_status ip_mode_get(int *mode)
{
	cs_status ret = CS_E_OK;
	*mode = ip_mode;

	return ret;
}

extern cs_status ip_check(cs_uint32 ip)
{
	cs_status ret = CS_E_OK;
	if(0xffffffff == ip || 0x0 == ip)
	{
		ret = CS_E_ERROR;
	}
	else
	{
		ret = CS_E_OK;
	}
	return ret;
}

extern cs_status mask_check(cs_uint32 mask)
{
	cs_status ret = CS_E_OK;
	return ret;

}

extern cs_status gateway_check(cs_uint32	gateway)
{
	cs_status ret = CS_E_OK;
	return ret;

}


extern cs_status vlan_check(cs_uint16 vlan)
{
	cs_status ret = CS_E_OK;
	return ret;

}


static device_ip_t device_ip_info;

extern cs_status ip_info_save_to_global(cs_uint32 ip, cs_uint32 mask, cs_uint32	gateway, cs_uint16 vlan)
{
	cs_status ret = CS_E_OK;
	device_ip_info.device_ip = ip;
	device_ip_info.device_mask = mask;
	device_ip_info.device_gateway = gateway;
	device_ip_info.device_vlan = vlan;
	
	return ret;	
}

extern cs_status ip_info_get_from_global(cs_uint32 *ip, cs_uint32 *mask, cs_uint32 *gateway, cs_uint16 *vlan)
{
	cs_status ret = CS_E_OK;

	*ip = device_ip_info.device_ip;
	*mask = device_ip_info.device_mask;
	*gateway = device_ip_info.device_gateway;
	*vlan = device_ip_info.device_vlan;
	
	return ret;	
}


static cs_status ip_info_save_to_flash(cs_uint32 ip, cs_uint32 mask, cs_uint32	gateway, cs_uint16 vlan)
{
	cs_status ret = CS_E_OK;
	onu_slow_path_cfg_cfg_t ip_cfg;
	ret = get_userdata_from_flash((char *)&ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(ip_cfg));
	if(0 != ret)
	{
		cs_printf("get ipdate fail..\n");
	}
	else
	{
		//do nothing
		#if 0
		cs_printf("get ip data success\n");
		#endif
	}

	ip_cfg.device_ip = ip;
	ip_cfg.device_mask = mask;
	ip_cfg.device_gateway = gateway;
	ip_cfg.device_vlan = vlan;

	ret = save_userdata_to_flash((char *)&ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(ip_cfg));
	if(0 != ret)
	{
		cs_printf("save_userdata_to_flash failed\n");
	}
	else
	{
		#if 0
		cs_printf("save_userdata_to_flash success\n");
		#endif
	}

	return ret;

}


extern cs_status ip_info_get_from_flash(cs_uint32 *ip, cs_uint32 *mask, cs_uint32 *gateway, cs_uint16 *vlan)
{
	cs_status ret = CS_E_OK;
	onu_slow_path_cfg_cfg_t ip_cfg;
	ret = get_userdata_from_flash((char *)&ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(ip_cfg));
	if(0 != ret)
	{
		cs_printf("get ipdate fail..\n");
	}
	else
	{
		//do nothing
		#if 0
		cs_printf("get ip data success\n");
		#endif
	}

	*ip = ip_cfg.device_ip;
	*mask = ip_cfg.device_mask;
	*gateway = ip_cfg.device_gateway;
	*vlan = ip_cfg.device_vlan;

	return ret;

}


extern int cmd_if_config(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				return cli_arg_help(cli, 0,
				"<cr>", "show ip configure",
				"<IP address>", "ip configure",
				"save", "save config to flash",
				NULL);
				break;
			case 2:
				return cli_arg_help(cli, 0,
				"<Subnet Mask>", "ip configure",
				NULL);
				break;
			case 3:
				return cli_arg_help(cli, 0,
				"<GateWay>", "ip configure",
				NULL);
				break;
			case 4:
				return cli_arg_help(cli, 0,
				"<Management Vlan>", "ip configure",
				NULL);
				break;
			case 5:
				return cli_arg_help(cli, 0,
				"<cr>", "finish ip configure",
				NULL);
				break;
			default:
			return cli_arg_help(cli, argc > 1, NULL);
		}
	}


	#if 0
	cs_printf("argc :0x%x\n", argc);
	#endif

	int valid = 0;
	int argc_valid[] = {0, 1,4,};
	int i = 0;
	int len = 0;
	len = sizeof(argc_valid)/sizeof(argc_valid[0]);
	for(i=0;i<len;i++)
	{
		if(argc == argc_valid[i])
		{
			valid = 1;
			break;
		}
		else
		{
			//do nothing
		}
	}

	
	if(0 == valid)
	{
		cli_print(cli, "%% Invalid input");
		return CLI_ERROR;
	}
	else
	{
		//do nothing
	}

	if(0 == argc)
	{
		#if 0
		cs_printf("show ip configure\n");
		#endif

		cs_status ret = CS_E_OK;
		cs_uint32	device_ip;
		cs_uint32	device_mask;
		cs_uint32	device_gateway;
		cs_uint16	device_vlan;
		
		ret = ip_info_get_from_global(&device_ip, &device_mask, &device_gateway, &device_vlan);
		if(CS_E_OK != ret)
		{
			cs_printf("ip_info_get_from_global fail\n");
		}
		else
		{
			#if 0
			cs_printf("ip_info_get_from_global success\n");
			#endif
		}

		#if 0
		ip_info_get_from_flash(&device_ip, &device_mask, &device_gateway, &device_vlan);
		app_ipintf_add_ip_address(ntohl(device_ip), device_vlan, ntohl(device_mask));
		#endif

		struct sockaddr_in addrp[3];
		addrp[0].sin_addr.s_addr = htonl(device_ip);
		addrp[1].sin_addr.s_addr = htonl(device_mask); 
		addrp[2].sin_addr.s_addr = htonl(device_gateway);
		
		cli_print(cli, "ip from network card:\n");		
		cli_print(cli,"=========================================================================");
		cli_print(cli,"IP - %s", inet_ntoa(addrp[0].sin_addr));
		cli_print(cli,"IP Submask - %s", inet_ntoa(addrp[1].sin_addr));
		cli_print(cli,"IP Gateway - %s", inet_ntoa(addrp[2].sin_addr));
		cli_print(cli,"IP Management Vlan - %d", device_vlan);
		cli_print(cli,"=========================================================================");

		cli_print(cli, "\n");
		cli_print(cli, "ip from flash:\n");
		ip_info_get_from_flash(&device_ip, &device_mask, &device_gateway, &device_vlan);
		addrp[0].sin_addr.s_addr = htonl(device_ip);
		addrp[1].sin_addr.s_addr = htonl(device_mask); 
		addrp[2].sin_addr.s_addr = htonl(device_gateway);
		cli_print(cli,"=========================================================================");
		cli_print(cli,"IP - %s", inet_ntoa(addrp[0].sin_addr));
		cli_print(cli,"IP Submask - %s", inet_ntoa(addrp[1].sin_addr));
		cli_print(cli,"IP Gateway - %s", inet_ntoa(addrp[2].sin_addr));
		cli_print(cli,"IP Management Vlan - %d", device_vlan);
		cli_print(cli,"=========================================================================");
		
	}
	else
	{
		//do nothing
	}

	if(1 == argc)
	{
		#if 1
		cs_printf("argv[0] :%s\n", argv[0]);
		#endif
		char str[] = "save";
		strcmp(str, argv[0]);
		cs_uint32	device_ip;
		cs_uint32	device_mask;
		cs_uint32	device_gateway;
		cs_uint16	device_vlan;

		cs_status ret = CS_E_OK;
		ret = ip_info_get_from_global(&device_ip, &device_mask, &device_gateway, &device_vlan);
		if(CS_E_OK != ret)
		{
			cs_printf("ip_info_get_from_global fail\n");
		}
		else
		{
			#if 0
			cs_printf("ip_info_get_from_global success\n");
			#endif
		}

		#if 1
		ret = ip_info_save_to_flash(device_ip, device_mask, device_gateway, device_vlan);
		if(CS_E_OK != ret)
		{
			cs_printf("ip_info_save_to_flash fail\n");
		}
		else
		{
			#if 0
			cs_printf("ip_info_save_to_flash success\n");
			#endif
		}
		#endif

		
	}
	else
	{
		//do nothing
	}

	if(4 == argc)
	{
		#if 1
		cs_printf("ip configure\n");
		#endif
		cs_uint32	device_ip;
		cs_uint32	device_mask;
		cs_uint32	device_gateway;
		cs_uint16	device_vlan;
		#if 1
		device_ip = ntohl(inet_addr(argv[0]));
		device_mask = ntohl(inet_addr(argv[1]));
		device_gateway = ntohl(inet_addr(argv[2]));
		device_vlan = (atoi(argv[3]));
		#else
		device_ip = htonl(inet_aton(argv[0], NULL));
		device_mask = htonl(inet_aton(argv[1], NULL));
		device_gateway = htonl(inet_aton(argv[2], NULL));
		device_vlan = htons(atoi(argv[3]));
		#endif

		#if 1
		cs_printf("device_ip :0x%x\n", device_ip);
		cs_printf("device_mask :0x%x\n", device_mask);
		cs_printf("device_gateway :0x%x\n", device_gateway);
		cs_printf("device_vlan :0x%x\n", device_vlan);
		#endif

		if(CS_E_OK != ip_check(device_ip))
		{
			cli_print(cli, "%% Invalid ip");
			return CLI_ERROR;
		}
		else
		{
			//do nothing
			#if 0
			cs_printf("ip_check OK\n");
			#endif
		}

		if(CS_E_OK != mask_check(device_mask))
		{
			cli_print(cli, "%% Invalid mask");
			return CLI_ERROR;
		}
		else
		{
			//do nothing
			#if 0
			cs_printf("mask_check OK\n");
			#endif
		}

		if(CS_E_OK != gateway_check(device_gateway))
		{
			cli_print(cli, "%% Invalid gateway");
			return CLI_ERROR;
		}
		else
		{
			//do nothing
			#if 0
			cs_printf("gateway_check OK\n");
			#endif
		}

		if(CS_E_OK != vlan_check(device_vlan))
		{
			cli_print(cli, "%% Invalid vlan");
			return CLI_ERROR;
		}
		else
		{
			//do nothing
			#if 0
			cs_printf("vlan_check OK\n");
			#endif
		}

		ip_mode_set(1);
		cs_status ret = CS_E_OK;
		ret = app_ipintf_add_ip_address((device_ip), device_vlan, (device_mask));
		if(CS_E_OK != ret)
		{
			cs_printf("***app_ipintf_add_ip_address fail\n");
		}
		else
		{
			#if 1
			cs_printf("***app_ipintf_add_ip_address success\n");
			#endif
		}
		ip_mode_set(1);

		#if 0
		ret = ip_info_save_to_global(device_ip, device_mask, device_gateway, device_vlan);
		if(CS_E_OK != ret)
		{
			cs_printf("ip_info_save_to_global fail\n");
		}
		else
		{
			#if 1
			cs_printf("ip_info_save_to_global success\n");
			#endif
		}
		#endif
		
	}
	else
	{
		//do nothing
	}
	return CLI_OK;
}

#endif



#ifdef HAVE_TERMINAL_SERVER


#if 1
//定义在其他模块中

extern cs_status ip_check(cs_uint32 ip);
extern cs_status mask_check(cs_uint32 mask);
extern cs_status gateway_check(cs_uint32	gateway);
extern cs_status vlan_check(cs_uint16 vlan);

extern cs_status uart_server_enable(cs_uint8 uart);
extern void ts_disable(cs_uint8 uart);
extern cs_status uart_server_disable(cs_uint8 uart);
extern cs_status uart_ip_info_get_from_global(cs_uint32 *uart_ip, cs_uint32 *uart_mask, cs_uint32 *uart_gateway, cs_uint16 *uart_vlan);
extern cs_status uart_ip_info_save_to_global(cs_uint32 uart_ip, cs_uint32 uart_mask, cs_uint32 uart_gateway, cs_uint16 uart_vlan);
extern cs_status uart_ip_info_save_to_flash(cs_uint32 ip, cs_uint32 mask, cs_uint32	gateway, cs_uint16 vlan);
extern cs_status uart_ip_info_get_from_flash(cs_uint32 *ip, cs_uint32 *mask, cs_uint32 *gateway, cs_uint16 *vlan);
extern cs_status ip_mode_set(int mode);
extern cs_status ip_mode_get(int *mode);

#endif


#if 1
//定义在本模块中
extern void cli_uart_gwd_cmd(struct cli_command **cmd_root);
static int cmd_uart_server_enable(struct cli_def *cli, char *command, char *argv[], int argc);
static int cmd_uart_server_disable(struct cli_def *cli, char *command, char *argv[], int argc);
static int cmd_uart_client_enable(struct cli_def *cli, char *command, char *argv[], int argc);
static int cmd_uart_client_disable(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_uart_client_set_server_ip(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_uart_client_set_server_port(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_uart_client_get_server_ip(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_uart_client_get_server_port(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_uart_mode_set(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_uart_mode_get(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_onu_uart_ip_save(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_uart_configure(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_onu_uart_ip_show(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_onu_uart_ip_set(struct cli_def *cli, char *command, char *argv[], int argc);


#endif


void cli_uart_gwd_cmd(struct cli_command **cmd_root)
{
	struct cli_command *uart = NULL;
	struct cli_command *uart_server = NULL;
	struct cli_command *uart_client = NULL;
	struct cli_command *mode_ip = NULL;
	struct cli_command *uart_mode = NULL;
	struct cli_command *set_server = NULL;
	struct cli_command *get_server = NULL;
	
	uart = cli_register_command(cmd_root, NULL, "uart",  NULL,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart configuration set");
	
	uart_server = cli_register_command(cmd_root, uart, "server",NULL, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart server configuration set");
	cli_register_command(cmd_root, uart_server, "enable", cmd_uart_server_enable, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart server enable");
 	cli_register_command(cmd_root, uart_server, "disable", cmd_uart_server_disable, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart server disable");

	cli_register_command(cmd_root, uart, "configure",cmd_uart_configure, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart configuration");
	
	uart_client = cli_register_command(cmd_root, uart, "client",NULL, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client configuration set");
	cli_register_command(cmd_root, uart_client, "enable", cmd_uart_client_enable, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client enable");
 	cli_register_command(cmd_root, uart_client, "disable", cmd_uart_client_disable, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client disable");
	set_server = cli_register_command(cmd_root, uart_client, "set_server", NULL, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client set server");
	cli_register_command(cmd_root, set_server, "ip", cmd_uart_client_set_server_ip, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client set server ip");
	cli_register_command(cmd_root, set_server, "port", cmd_uart_client_set_server_port, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client set server port");
	get_server = cli_register_command(cmd_root, uart_client, "get_server", NULL, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client get server");
	cli_register_command(cmd_root, get_server, "ip", cmd_uart_client_get_server_ip, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client get server ip");
	cli_register_command(cmd_root, get_server, "port", cmd_uart_client_get_server_port, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart client get server port");
	
	mode_ip = cli_register_command(cmd_root, uart, "ip",NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "show uart server ip");
	cli_register_command(cmd_root, mode_ip, "get",cmd_onu_uart_ip_show, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "show uart server ip");
	cli_register_command(cmd_root, mode_ip, "set",cmd_onu_uart_ip_set, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "show uart server ip");
	cli_register_command(cmd_root, mode_ip, "save",cmd_onu_uart_ip_save, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "show uart server ip");
	
	uart_mode = cli_register_command(cmd_root, uart, "mode",  NULL,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "uart mode configuration");
	cli_register_command(cmd_root, uart_mode, "set",  cmd_uart_mode_set,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Set GPIO pin");
	cli_register_command(cmd_root, uart_mode, "get",  cmd_uart_mode_get,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Get GPIO pin");
	
	
}

int cmd_uart_server_enable(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				return cli_arg_help(cli, 0, "<uart>",NULL);
			default:
				return cli_arg_help(cli, argc > 1 ,NULL);
		}
	}
	else
	{
		//do nothing
	}
	if(1 == argc)
	{
		cs_uint8 uart = 0;
		uart = atoi(argv[0]);
		cs_status ret = CS_E_OK;
		ret = uart_server_enable(uart);
		if(CS_E_OK == ret)
		{
			#if 0
			cli_print(cli, "uart server enable success");
			#endif
		}
		else
		{
			#if 0
			cli_print(cli, "uart server enable failed");
			#endif
		}
	}
	else
	{
		cs_printf("invalid input\n");
	}
	
	return CLI_OK;
}



int cmd_uart_server_disable(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				return cli_arg_help(cli, 0, "<uart>",NULL);
			default:
				return cli_arg_help(cli, argc > 1 ,NULL);
		}
	}
	else
	{
		//do nothing
	}
	
	if(1 == argc)
	{
		cs_uint8 uart = 0;
		uart = atoi(argv[0]);
		cs_status ret = CS_E_OK;
		ret = uart_server_disable(uart);
		if(CS_E_OK == ret)
		{
			cli_print(cli, "uart server disable success");
		}
		else
		{
			cli_print(cli, "uart server disable failed");
		}
	}
	else
	{
		cs_printf("invalid input\n");
	}

	
	return CLI_OK;
}

extern cs_status uart_client_enable(cs_uint8 uart);

int cmd_uart_client_enable(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				return cli_arg_help(cli, 0, "<uart>",NULL);
			default:
				return cli_arg_help(cli, argc > 1 ,NULL);
		}
	}
	else
	{
		//do nothing
	}

	if(1 == argc)
	{	
		cs_uint8 uart = 0;
		uart = atoi(argv[0]);
		cs_status ret = CS_E_OK;
		ret = uart_client_enable(uart);
		if(CS_E_OK == ret)
		{
			#if 0
			cli_print(cli, "uart client enable success");
			#endif
		}
		else
		{
			#if 0
			cli_print(cli, "uart client enable failed");
			#endif
		}
	}
	else
	{
		cs_printf("invalid input\n");
	}
	
	return CLI_OK;
}

extern cs_status uart_client_disable(cs_uint8 uart);

int cmd_uart_client_disable(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				return cli_arg_help(cli, 0, "<uart>",NULL);
			default:
				return cli_arg_help(cli, argc > 1 ,NULL);
		}
	}
	else
	{
		//do nothing
	}
	
	if(1 == argc)
	{
		cs_uint8 uart = 0;
		uart = atoi(argv[0]);
		cs_status ret = CS_E_OK;
		ret = uart_client_disable(uart);
		if(CS_E_OK == ret)
		{
			cli_print(cli, "uart client disable success");
		}
		else
		{
			cli_print(cli, "uart client disable failed");
		}
	}
	else
	{
		cli_print(cli, "invalid input");
	}

	
	return CLI_OK;
}






/**********************************************************************************************************************************************
*函数名：cmd_uart_mode_set
*函数功能描述：设置 gpio 的状态
*函数参数：
*函数返回值：int ,成功返回0
*作者：朱晓辉
*函数创建日期：2013.1.25
*函数修改日期：尚未修改
*修改人：尚未修改
*修改原因：尚未修改
*版本：1.0
*历史版本：无
**********************************************************************************************************************************************/
extern int cmd_uart_mode_set(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int pin_number=0;
    int status=0;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<pin id>", "value in  (1, 15)",
                 NULL);

         case 2:
            return cli_arg_help(cli, 0,
            "<status>", " set pin status, value in (0, 1)",
             NULL);

        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(2 == argc)
    {
        pin_number=atoi(argv[0]);
        status=atoi(argv[1]);

        if (pin_number >= CS_GPIO_MAX_NUM || pin_number < 0) 
		{
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }
        if ((status != 0 ) && (status !=1) ) 
		{
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }

		cs_callback_context_t context_gpio;
		cs_plat_gpio_mode_set(context_gpio, 0, 0, pin_number, GPIO_MODE_OUTPUT);
		cs_status ret = CS_E_OK;
		ret = cs_plat_gpio_write(context_gpio, 0, 0, pin_number, status);
		if(CS_E_OK == ret)
		{
			cli_print(cli, "gpio %d set %d seccess\n", pin_number, status);
		}
		else 
		{
			cli_print(cli, "gpio %d set %d failed\n", pin_number, status);
		}
    } 
	else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;	
}



/**********************************************************************************************************************************************
*函数名：cmd_uart_mode_get
*函数功能描述：获取gpio 的状态
*函数参数：
*函数返回值：int ,成功返回0
*作者：朱晓辉
*函数创建日期：2013.1.25
*函数修改日期：尚未修改
*修改人：尚未修改
*修改原因：尚未修改
*版本：1.0
*历史版本：无
**********************************************************************************************************************************************/
int cmd_uart_mode_get(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int pin_number=0;
    cs_uint8 status=0;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<pin id>", "value in  (1, 15)",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
		return CLI_OK;
    }

    if(1 == argc)
    {
        pin_number=atoi(argv[0]);

        if (pin_number >= CS_GPIO_MAX_NUM || pin_number < 0) 
		{
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }

		cs_callback_context_t context_gpio;
		cs_plat_gpio_mode_set(context_gpio, 0, 0, pin_number, GPIO_MODE_OUTPUT);
		cs_status ret = CS_E_OK;
		ret = cs_plat_gpio_read(context_gpio, 0, 0, pin_number, &status);
		if(CS_E_OK == ret)
		{
			cli_print(cli, "gpio %d: %d\n", pin_number, status);
		}
		else 
		{
			cli_print(cli, "gpio %d get failed\n", pin_number);
		}
		
		cli_print(cli, "gpio %d: %d\n", pin_number, status);
    } 
	else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;	
}


/**********************************************************************************************************************************************
*函数名：cmd_onu_uart_ip_save
*函数功能描述：将串口ip 信息保存
*函数参数：
*函数返回值：int ,成功返回0
*作者：朱晓辉
*函数创建日期：2013.5.27
*函数修改日期：尚未修改
*修改人：尚未修改
*修改原因：尚未修改
*版本：1.0
*历史版本：无
**********************************************************************************************************************************************/
extern int cmd_onu_uart_ip_save(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				return cli_arg_help(cli, 0,
				"<cr>", "save uart ip configuration",
				NULL);
				break;

			default:
			return cli_arg_help(cli, argc > 1, NULL);
		}
	}


	cs_status ret = CS_E_OK;
	cs_uint32	uart_ip;
	cs_uint32	uart_mask;
	cs_uint32	uart_gateway;
	cs_uint16	uart_vlan;
	
	ret = uart_ip_info_get_from_global(&uart_ip, &uart_mask, &uart_gateway, &uart_vlan);
	if(CS_E_OK != ret)
	{
		cs_printf("ip_info_get_from_global fail\n");
	}
	else
	{
#if 1
		cs_printf("ip_info_get_from_global success\n");
#endif
	}
	
#if 1
	ret = uart_ip_info_save_to_flash(uart_ip, uart_mask, uart_gateway, uart_vlan);
	if(CS_E_OK == ret)
	{
		// do nothing
	}
	else
	{
		cs_printf("uart_ip_info_save_to_flash failed\n");
	}
#endif

	
	return CLI_OK;
}





extern int cmd_onu_uart_ip_show(struct cli_def *cli, char *command, char *argv[], int argc)
{

	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				return cli_arg_help(cli, 0,
				"<cr>", "show uart ip information",
				NULL);
				break;

			default:
			return cli_arg_help(cli, argc > 1, NULL);
		}
	}


	cs_status ret = CS_E_OK;
	cs_uint32	uart_ip;
	cs_uint32	uart_mask;
	cs_uint32	uart_gateway;
	cs_uint16	uart_vlan;
	
	ret = uart_ip_info_get_from_global(&uart_ip, &uart_mask, &uart_gateway, &uart_vlan);
	if(CS_E_OK != ret)
	{
		cs_printf("ip_info_get_from_global fail\n");
	}
	else
	{
	#if 0
		cs_printf("ip_info_get_from_global success\n");
	#endif
	}
	
#if 0
	ip_info_get_from_flash(&device_ip, &device_mask, &device_gateway, &device_vlan);
	app_ipintf_add_ip_address(ntohl(device_ip), device_vlan, ntohl(device_mask));
#endif
	
	struct sockaddr_in addrp[3];
	addrp[0].sin_addr.s_addr = htonl(uart_ip);
	addrp[1].sin_addr.s_addr = htonl(uart_mask); 
	addrp[2].sin_addr.s_addr = htonl(uart_gateway);
	
	cli_print(cli,"=========================================================================");
	cli_print(cli,"uart IP - %s", inet_ntoa(addrp[0].sin_addr));
	cli_print(cli,"uart IP Submask - %s", inet_ntoa(addrp[1].sin_addr));
	cli_print(cli,"uart IP Gateway - %s", inet_ntoa(addrp[2].sin_addr));
	cli_print(cli,"uart IP Management Vlan - %d", uart_vlan);
	cli_print(cli,"=========================================================================");



	return 1;
}



extern int cmd_onu_uart_ip_set(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				return cli_arg_help(cli, 0,
				"<IP address>", "ip configure",
				NULL);
				break;
			case 2:
				return cli_arg_help(cli, 0,
				"<Subnet Mask>", "ip configure",
				NULL);
				break;
			case 3:
				return cli_arg_help(cli, 0,
				"<GateWay>", "ip configure",
				NULL);
				break;
			case 4:
				return cli_arg_help(cli, 0,
				"<Management Vlan>", "ip configure",
				NULL);
				break;
			case 5:
				return cli_arg_help(cli, 0,
				"<cr>", "finish ip configure",
				NULL);
				break;
			default:
			return cli_arg_help(cli, argc > 1, NULL);
		}
	}

	if(4 == argc)
	{
		cs_uint32	uart_ip;
		cs_uint32	uart_mask;
		cs_uint32	uart_gateway;
		cs_uint16	uart_vlan;
		uart_ip = ntohl(inet_addr(argv[0]));
		uart_mask = ntohl(inet_addr(argv[1]));
		uart_gateway = ntohl(inet_addr(argv[2]));
		uart_vlan = (atoi(argv[3]));

		if(CS_E_OK != ip_check(uart_ip))
		{
			cli_print(cli, "%% Invalid ip");
			return CLI_ERROR;
		}
		else
		{
			//do nothing
			#if 0
			cs_printf("ip_check OK\n");
			#endif
		}

		if(CS_E_OK != mask_check(uart_mask))
		{
			cli_print(cli, "%% Invalid mask");
			return CLI_ERROR;
		}
		else
		{
			//do nothing
			#if 0
			cs_printf("mask_check OK\n");
			#endif
		}

		if(CS_E_OK != gateway_check(uart_gateway))
		{
			cli_print(cli, "%% Invalid gateway");
			return CLI_ERROR;
		}
		else
		{
			//do nothing
			#if 0
			cs_printf("gateway_check OK\n");
			#endif
		}

		if(CS_E_OK != vlan_check(uart_vlan))
		{
			cli_print(cli, "%% Invalid vlan");
			return CLI_ERROR;
		}
		else
		{
			//do nothing
			#if 0
			cs_printf("vlan_check OK\n");
			#endif
		}

		ip_mode_set(0);
		cs_status ret = CS_E_OK;
		ret = app_ipintf_add_ip_address((uart_ip), uart_vlan, (uart_mask));
		if(CS_E_OK != ret)
		{
			cs_printf("***app_ipintf_add_ip_address fail\n");
		}
		else
		{
			#if 1
			cs_printf("***app_ipintf_add_ip_address success\n");
			#endif
		}
		ip_mode_set(1);

		#if 0
		ret = uart_ip_info_save_to_global(uart_ip, uart_mask, uart_gateway, uart_vlan);
		if(CS_E_OK != ret)
		{
			cs_printf("ip_info_save_to_global fail\n");
		}
		else
		{
			#if 0
			cs_printf("ip_info_save_to_global success\n");
			#endif
		}
		#endif
	}
	else
	{
		cli_print(cli, "invalid input");
	}


  return 1;
}


#endif

