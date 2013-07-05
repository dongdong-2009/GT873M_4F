#ifndef __ONU_CLI_H__
#define __ONU_CLI_H__

#include "cli_common.h"


#define VTY_MAXHIST 20
#define DEFUN_FLAG     0
#define QDEFUN_FLAG   1
#define MDEFUN_FLAG   2
#define LDEFUN_FLAG   3
#define PDEFUN_FLAG   4

#define PORTNO_TO_ETH_SLOT( _portno )           \
            ( ( ( _portno ) / ( ETH_FILTER_MAX_PORT_NUM_ON_SLOT ) ) + 1 )
#define PORTNO_TO_ETH_PORTID( _portno )         \
            ( ( _portno ) % ( ETH_FILTER_MAX_PORT_NUM_ON_SLOT ) )
            
            
struct cmd_element
{
    char *string;		/* Command specification by string. */
    int (*func)(struct cli_def*, char *, char **, int);
    char *doc;			/* Documentation of this command. */
	int privilege;
	int mode;
	struct cli_command * parent;
    int msg_flag;       /* 标识是DEFUN、QDEFUN等消息 */
    unsigned long p_que_id;     /* 可保存指针和整型值 */
    int daemon;			/* Daemon to which this command belong. */
    int cmdsize;		/* Command index count. */
    char *config;		/* Configuration string */
};
#define DEFUN(cmdname, callback, cmdstr, helpstr,privilege,mode,node)   \
	int callback(struct cli_def*, char *, char **, int);\
	struct cmd_element cmdname = {cmdstr, callback, helpstr ,privilege,mode,node,DEFUN_FLAG};\
	int callback(struct cli_def*cli, char *comment, char **argv,int argc)

int cli_int_interface_switch(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_get_rcpreg(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_rcpreg(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_show_mac(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_show_mgt_port(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_system_information(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_en(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_fc(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_link_show(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_mode(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_mode_show(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_ingress_rate(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_egress_rate(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_mirror_to(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_mirror_from(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_port_mirror_show(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_dotlq(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_port_isolate(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_dotlq_add(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_dotlq_port_add(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_dotlq_del(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_dotlq_port_del(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_dotlq_show(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_pvid(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_leaky(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_ingress_filtering(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_acceptable_frame_types(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_info_show(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_insert_pvid(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_vlan_output_tag(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_stat_rx(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_stat_tx(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_stat_diag(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_broadcast_storm_filtering(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_broadcast_fc(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_igmpsnooping(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_multicast_fc(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_qos_dscp_pri_en(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_qos_user_pri_en(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_qos_adapted_fc(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_qos_priority_ratio(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_qos_port_priority(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_qos_priority_type(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_loopback_auto_detect(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_atu_learning(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_atu_aging(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_atu_filter(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_cable_test_port(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_mgt_reset(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_mgt_config_clear(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_mgt_config_save(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_show_loop_port(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_show_running_config(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_show_startup_config(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_mask_alarm_switch(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_switch(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_clear_switch(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_switch_manage_auth(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_get_rcpeeprom(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_rcpeeprom(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_get_rcpphy(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_rcpphy(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_show_authenkey(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_product_series(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_product_type(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_board_type(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_rcphw_version(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_rcpmanufacture_serial(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_rcpmanufacture_date(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_set_rcpmac(struct cli_def *cli, char *command, char *argv[], int argc);
int cli_int_configure_terminal(struct cli_def *cli, char *command, char *argv[], int argc);
int cmd_config_int(struct cli_def *cli, char *command, char *argv[], int argc);
int cmd_debug_mode_int(struct cli_def *cli, char *command, char *argv[], int argc);
int cmd_debug_regular(struct cli_def *cli, char *command, char *argv[], int argc);
int cmd_debug_legacy(struct cli_def *cli, char *command, char *argv[], int argc);


#endif


