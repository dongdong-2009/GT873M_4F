#include "plat_common.h"
#include "cli_common.h"
#include "db_instance.h"
#include "sys_cfg.h"
#include "sdl_api.h"
#include "cs_utils.h"
#include "mc_type.h"

#ifdef HAVE_TELNET_CLI
#define DFT_ENABLE_PASSWD           "enable"

#define DFT_ADMIN_NAME              "admin"
#define DFT_ADMIN_PASS              "greenway"
#define DFT_USER_NAME               "user"
#define DFT_USER_PASS               "user"
#define ONU_HW_VER_LEN          32


unsigned int regular_count = 0;
unsigned int debug_regular = 0;

extern 
	void onu_hw_version_get(cs_uint8 *hw_vstr, cs_int32 len);
extern int cmd_show_run(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_show_run_system(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_show_run_ctc_vlan(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_show_run_igmp(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_show_run_qos(struct cli_def *cli, char *command, char *argv[], int argc);
extern int cmd_show_run_stp(struct cli_def *cli, char *command, char *argv[], int argc);
extern int do_telnet_legacy_cmd(struct cli_def *cli,char * p);

int cli_int_configure_terminal(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    cli_set_configmode(cli, MODE_CONFIG, NULL);
    return CLI_OK;
}

int cmd_debug_mode_int(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    cli_set_configmode(cli, MODE_DEBUG, "debug");
    return CLI_OK;
}

int cmd_reset(struct cli_def *cli, char *command, char *argv[], int argc)
{
    
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    iros_system_reset(RESET_TYPE_FORCE);
    
    return CLI_OK;
}


extern int restore_factory_setting_reset();
int cmd_reset_factory(struct cli_def *cli, char *command, char *argv[], int argc)
{
    
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
#ifdef HAVE_ZTE_OAM    
    db_zte_set_default();
    db_save_to_flash();
#endif    

    iros_system_reset(RESET_TYPE_FORCE);
    
    return CLI_OK;
}


static int g_cli_led_status = 1;
int cmd_led(struct cli_def *cli, char *command, char *argv[], int argc)
{
    unsigned char  led_mode=0;
        
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "on | off", "Led on or Led off",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {   
        if(strcasecmp(argv[0],"on") == 0){
            led_mode = LED_ON;
			//g_cli_led_cmd = 0;
        }
        else if(strcasecmp(argv[0],"off") == 0){
            led_mode = LED_OFF;
			//g_cli_led_cmd = 1;
        }
        else{
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }

        //TODO: ADD the LED API HERE
		cs_led_proc(LED_LOS, LED_GREEN,led_mode);
		cs_led_proc(LED_PON, LED_GREEN,led_mode);
        cs_led_proc(LED_UPS, LED_GREEN,led_mode);
        cs_led_proc(LED_RUN, LED_GREEN,led_mode);

        g_cli_led_status = led_mode;
    } 
    else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}


#if 0
int cmd_laser(struct cli_def *cli, char *command, char *argv[], int argc)
{
    unsigned char  laser_mode=0;
	cs_callback_context_t context;
	
        
    /* deal with help*/
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "on | off | normal", "Laser on or off or normal",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {   
        if(strcasecmp(argv[0],"on") == 0){
            laser_mode = SDL_PON_LASER_MODE_ON;
        }
        else if(strcasecmp(argv[0],"normal") == 0){
            laser_mode = SDL_PON_LASER_MODE_BURST;
        }
        else if(strcasecmp(argv[0],"off") == 0){
            laser_mode = SDL_PON_LASER_MODE_OFF;
        }
        else{
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }
        
		epon_request_onu_pon_laser_mode_set(context, ONU_DEVICEID_FOR_API,
				ONU_LLIDPORT_FOR_API, laser_mode);
        
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }
	
    return CLI_OK;
}
#else

extern cs_status epon_request_onu_pon_laser_mode_set(
    CS_IN cs_callback_context_t context,
    CS_IN cs_int32            device_id,
    CS_IN cs_int32            llidport,
    CS_IN cs_sdl_pon_laser_mode_t mode
);
int cmd_laser(struct cli_def *cli, char *command, char *argv[], int argc)
{
	cs_callback_context_t context;
	cs_port_id_t  port = CS_PON_PORT_ID;
    cs_sdl_pon_laser_mode_t laser_mode;
	
    
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "on | off", "Laser on or Laser off",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {   
        if(strcasecmp(argv[0],"on") == 0){
            laser_mode = SDL_PON_LASER_MODE_ON;
        }
        else if(strcasecmp(argv[0],"off") == 0){
            laser_mode = SDL_PON_LASER_MODE_OFF;
        }
        else{
            cli_print(cli, "%% Invalid input.");
            return CLI_ERROR;
        }
        
        epon_request_onu_pon_laser_mode_set(context, 0, port, laser_mode);
        
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}

#endif

int cmd_pon_mac(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int mac1[CS_MACADDR_LEN];
    unsigned char mac2[CS_MACADDR_LEN];
    int ret ;

    int i;
    
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<Mac>", "ONU  MAC Address",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }


    if(1 == argc)
    {   
        if(strlen(argv[0]) > 18){
            cli_print(cli,"MAC address configuration is not OK\n");
            return CLI_ERROR;
        }
        
        ret = sscanf(argv[0], "%x:%x:%x:%x:%x:%x", 
            &mac1[0], &mac1[1], &mac1[2], &mac1[3],&mac1[4],&mac1[5]);
        
        if(ret != 6 || mac1[0]&0x01){
            cli_print(cli,"Input MAC is not a unicast MAC\n");
            return CLI_ERROR;
        }

        for(i = 0 ; i < 6; i ++){
            mac2[i] = (unsigned char)mac1[i];
        }

/* Set ONU mac addr.*/
        startup_config_write(CFG_ID_MAC_ADDRESS, CS_MACADDR_LEN, mac2);
        
    } 
    else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}

int cli_get_onu_mac_addr(char *mac)
{
    cs_status ret;

    ret = startup_config_read(CFG_ID_MAC_ADDRESS, CS_MACADDR_LEN, mac);
    
    if (CS_E_OK != ret)
    {
		return 1;
    }

	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
                               mac[0], 
	                        mac[1], 
	                        mac[2], 
	                        mac[3], 
	                        mac[4], 
	                        mac[5]);
    return 0;

}


int cmd_show_pon_mac(struct cli_def *cli, char *command, char *argv[], int argc)
{
    char strMac[32];

    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
	
	cli_get_onu_mac_addr(strMac);

    cli_print(cli, "Onu mac address is %s",strMac);
    return CLI_OK;
}

int cmd_show_version(struct cli_def *cli, char *command, char *argv[], int argc)
{
    char hd_version[32] = "V1.0.0";
    char sw_version[32]= ONU_SW_VERSION;

    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
    
    strcpy(sw_version, ONU_SW_VERSION);
#ifdef HAVE_ZTE_OAM    
    memset(hd_version, 0, sizeof(hd_version));
    onu_hw_version_get(hd_version, ONU_HW_VER_LEN);
#endif

    cli_print(cli, "hardware version: %s", hd_version);

    cli_print(cli,  "software version: %s", sw_version);

    cli_print(cli, "firmware version: %d.%d.%d.%d",
                     cs_atoi(IROS_ONU_APP_VER_MAJOR),
                     cs_atoi(IROS_ONU_APP_VER_MINOR),
                     cs_atoi(IROS_ONU_APP_VER_REVISION),
                     cs_atoi(IROS_ONU_APP_VER_BUILD));
    
    return CLI_OK;
}


int cmd_show_led(struct cli_def *cli, char *command, char *argv[], int argc)
{
    unsigned int  led_mode = 0;
        
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    led_mode = g_cli_led_status;

    cli_print(cli,"The Led is %s \n",led_mode == 1?"on":"off");
    
    return CLI_OK;
}


int cmd_show_ups(struct cli_def *cli, char *command, char *argv[], int argc)
{
    unsigned char  ups_mode = 0;
    unsigned char  ups_status = 0;
    cs_callback_context_t context;
        
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    cs_plat_gpio_read(context, 0, 0, GPIO_PIN7, &ups_mode);

    cli_print(cli,"The UPS is %s \n",ups_mode == 0?"on":"off");

    if(ups_mode == 0){
        cs_plat_gpio_read(context, 0, 0,GPIO_PIN4, &ups_status);
        
        cli_print(cli,"The UPS voltage alarm is %s \n",ups_status == 0?"off":"on");
    }
    
    return CLI_OK;
}

int cmd_show_laser(struct cli_def *cli, char *command, char *argv[], int argc)
{
    unsigned int  laser_mode = 0;
	cs_callback_context_t context;
	
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

	epon_request_onu_pon_laser_mode_get(context, ONU_DEVICEID_FOR_API,
			ONU_LLIDPORT_FOR_API, &laser_mode);

    if(laser_mode == SDL_PON_LASER_MODE_ON){
        cli_print(cli,"The Laser is on\n");
    }
    else if(laser_mode == SDL_PON_LASER_MODE_OFF){
        cli_print(cli,"The Laser is off \n");
    }
    else if(laser_mode == SDL_PON_LASER_MODE_BURST){
        cli_print(cli,"The Laser is normal status\n");
    }
    else{
        cli_print(cli,"The Laser is invalid status\n");
    }
    
    return CLI_OK;
}

#ifdef HAVE_ZTE_OAM /* #ifdef HAVE_ZTE_API */
int cmd_serial_number(struct cli_def *cli, char *command, char *argv[], int argc)
{
    db_zte_sn_t   sn_t; 
    int i;
    cs_status ret = CS_E_OK;
    
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<String>", "ONU serial number",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }
 
    if(1 == argc)
    {   
        if(strlen(argv[0]) > 128){
            cli_print(cli,"Serial number configuration is not OK\n");
            return CLI_ERROR;
        }

        for(i = 0; i < strlen(argv[0]); i ++){
            if((argv[0][i] >= '0' &&  argv[0][i] <= '9')
                ||(argv[0][i] >= 'a' &&  argv[0][i] <= 'z')
                ||(argv[0][i] >= 'A' &&  argv[0][i] <= 'Z'))
                continue;

            cli_print(cli,"Serial number configuration is not OK\n");
            return CLI_ERROR;
        }

        memset(&sn_t,0x00,sizeof(sn_t));
        memcpy(sn_t.sn, argv[0], strlen(argv[0]));
        ret = db_write(DB_ZTE_SN_ID,(cs_uint8 *) &sn_t, sizeof(sn_t));

        if(CS_E_OK != ret){
            cli_print(cli,"Serial number configuration is not OK\n");
            return CLI_ERROR;
        }

        ret = db_save_to_flash();
        if(CS_E_OK != ret){
            cli_print(cli,"Serial number configuration save is not OK\n");
            return CLI_ERROR;
        }
        
    } 
    else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}

int cmd_show_serial_number(struct cli_def *cli, char *command, char *argv[], int argc)
{
    db_zte_sn_t   sn; 
    cs_uint8 onu_sn[DB_ZTE_SN_LEN + 1];
	
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    memset(onu_sn,0x00,sizeof(onu_sn));
    memset(&sn,0x00,sizeof(sn));
    db_read(DB_ZTE_SN_ID,(cs_uint8 *)&sn, sizeof(db_zte_sn_t));
    memcpy(onu_sn,sn.sn,strlen(sn.sn));

    cli_print(cli, "ONU Serial number is %s \n", onu_sn);
    
    return CLI_OK;
}
#endif  /* END_HAVE_ZTE_OAM */

int cmd_ctc_loid_password(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int i;
    cs_status ret = CS_E_OK;
    db_ctc_loid_pwd_t db_loid;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<loid> <password>", "ONU ctc loid and password",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
    }

    if(0 < argc)
    {   
        if(strlen(argv[0]) > DB_CTC_LOID_MAX_LEN){
            cli_print(cli,"User Loid configuration is not OK\n");
            return CLI_ERROR;
        }

        if(argc == 2){
            if(strlen(argv[1]) > DB_CTC_PWD_MAX_LEN){
                cli_print(cli,"User Password configuration is not OK\n");
                return CLI_ERROR;
            }
        }
        
        for(i = 0; i < strlen(argv[0]); i ++){
            if((argv[0][i] >= '0' &&  argv[0][i] <= '9')
                ||(argv[0][i] >= 'a' &&  argv[0][i] <= 'z')
                ||(argv[0][i] >= 'A' &&  argv[0][i] <= 'Z'))
                continue;

            cli_print(cli,"User Loid configuration is not OK\n");
            return CLI_ERROR;
        }
        
        if(argc == 2){
            for(i = 0; i < strlen(argv[1]); i ++){
                if((argv[1][i] >= '0' &&  argv[1][i] <= '9')
                    ||(argv[1][i] >= 'a' &&  argv[1][i] <= 'z')
                    ||(argv[1][i] >= 'A' &&  argv[1][i] <= 'Z'))
                    continue;

                cli_print(cli,"User Password configuration is not OK\n");
                return CLI_ERROR;
            }
        }

        
        memset(&db_loid,0x00,sizeof(db_loid));
        strncpy(db_loid.loid, argv[0],DB_CTC_LOID_MAX_LEN);
        if(argc == 2){
            strncpy(db_loid.passwd,argv[1],DB_CTC_PWD_MAX_LEN);
        }
        ret = db_write(DB_CTC_LOID_PASSWD_ID,(cs_uint8 *) &db_loid, sizeof(db_loid));
        if(ret != CS_E_OK){
            cli_print(cli, "set loid and password failed ,ret = %d \n",ret);
            return CLI_ERROR;
        }

    } 
    else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}

int cmd_show_ctc_loid_password(struct cli_def *cli, char *command, char *argv[], int argc)
{
    db_ctc_loid_pwd_t db_loid;
    cs_int8 loid[DB_CTC_LOID_MAX_LEN + 1];
    cs_int8 password[DB_CTC_PWD_MAX_LEN + 1];

    
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    memset(&db_loid,0x00,sizeof(db_loid));
    db_read(DB_CTC_LOID_PASSWD_ID,(cs_uint8 *) &db_loid, sizeof(db_loid));

    strncpy(loid, db_loid.loid,DB_CTC_LOID_MAX_LEN);
    strncpy(password,db_loid.passwd,DB_CTC_PWD_MAX_LEN);
	
    cli_print(cli, "ONU user Loid is %s \n", loid);
    cli_print(cli, "ONU user Password is %s \n", password);
    
    return CLI_OK;
}

int cmd_ftp_download(struct cli_def *cli, char *command, char *argv[], int argc)
{    
	if(CLI_HELP_REQUESTED)
    {
        cli_print(cli,"ip-address directory username username password password");
		cli_print(cli,"ip-address xxx.xxx.xxx.xxx");/*argv[0]*/
		cli_print(cli,"directory  The firmware file path and name in FTP server.");/*argv[1]*/
        cli_print(cli,"           If the file path and name include space, ");
        cli_print(cli,"           please use \"\" to contain the directory");
		cli_print(cli,"username	  FTP user name");/*argv[2 3]*/
		cli_print(cli,"password	  FTP user password"); /*argv[4 5]*/
        return CLI_OK;        
    }
    if((argc !=6 ) || (strcasecmp(argv[2],"username") != 0)  || (strcasecmp(argv[4],"password") != 0))
    {
        cli_print(cli, "%% Invalid argvs.");
        return CLI_OK;  
    }

	/*call ftp download here*/
    cli_print(cli,"Ftp download %s:%s@%s %s\n",(char *)argv[3],(char *)argv[5],(char *)argv[0],(char *)argv[1]);    
    //ftp_get((char *)argv[0],(char *)argv[3],(char *)argv[5],(char *)argv[1],NULL,0, ftpclient_printf);          
  
    return CLI_OK;
}

int cmd_tftp_download(struct cli_def *cli, char *command, char *argv[], int argc)
{
//    int  tftp_err; 
	if(CLI_HELP_REQUESTED)
    {
        cli_print(cli,"Ip-address Directory");/*argv[0]*/
        cli_print(cli,"Ip-address xxx.xxx.xxx.xxx");/*argv[1]*/
        cli_print(cli,"Directory  The firmware file path and name in TFTP server.");
        cli_print(cli,"           If the file path and name include space,");
        cli_print(cli,"           please use \"\" to contain the directory");
        return CLI_OK;
    }
    if(argc !=2 )
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;  
    }
	/*call tftp download here*/
    cli_print(cli, "tftp download %s %s\n",argv[1],argv[0]);
   // tftp_client_get(argv[1], argv[0], TFTP_STD_PORT, NULL, 0, TFTP_OCTET, &tftp_err);
    
    return CLI_OK;
}

extern cs_status onu_software_version_get(char *sw_version, cs_uint16 sw_version_len);
extern cs_status onu_hardware_version_get(char *hw_version, cs_int16 hw_version_len);

int cmd_show_zte_version(struct cli_def *cli, char *command, char *argv[], int argc)
{
    char hd_version[32] = "v1.0.0";
    char sw_version[32] = ONU_SW_VERSION;

    if (CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
            case 1:
                return cli_arg_help(cli, 1,
                    "detail", "Show ONU detail version info",
                     NULL);
            default:
                cli_arg_help(cli, (argc == 0), NULL);                
                return cli_arg_help(cli, 1,
                    "detail", "Show ONU detail version info",
                     NULL);
        }
    }

    if(argc > 1)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
    
#ifdef HAVE_ZTE_OAM
          strcpy(sw_version, ONU_SW_VERSION);
          memset(hd_version, 0, sizeof(hd_version));
          onu_hw_version_get(hd_version, ONU_HW_VER_LEN);
#endif 
	onu_hardware_version_get(hd_version, sizeof(hd_version));
	onu_software_version_get(sw_version, sizeof(sw_version));
    cli_print(cli,  "Software Version  : %s", sw_version);
    cli_print(cli,  "Hardware Version  : %s", hd_version);    
    return CLI_OK;
}

int cmd_show_zte_version_detail(struct cli_def *cli, char *command, char *argv[], int argc)
{
    char hd_version[32] = "v1.0.0";
    char sw_version[32] = ONU_SW_VERSION;

    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

#ifdef HAVE_ZTE_OAM
    memset(hd_version, 0, sizeof(hd_version));
    onu_hw_version_get(hd_version, ONU_HW_VER_LEN);
#else
    strncpy(hd_version, ONU_HW_VERSION, ONU_HW_VER_LEN);
#endif

    /*cli_print(cli,  "BootPROM Version  : %d.%d.%d.%d", 
					cs_atoi(IROS_ONU_LOADER_VER_MAJOR),
					cs_atoi(IROS_ONU_LOADER_VER_MINOR),
					cs_atoi(IROS_ONU_LOADER_VER_REVISION),
					cs_atoi(IROS_ONU_LOADER_VER_BUILD));*/
    cli_print(cli,  "Software Version  : %s", sw_version);
    cli_print(cli,  "Firmware Version  : %d.%d.%d.%d",
                    cs_atoi(IROS_ONU_APP_VER_MAJOR),
                    cs_atoi(IROS_ONU_APP_VER_MINOR),
                    cs_atoi(IROS_ONU_APP_VER_REVISION),
                    cs_atoi(IROS_ONU_APP_VER_BUILD));
    cli_print(cli,  "Hardware Version  : %s", hd_version);
    cli_print(cli,  "Compiled          : %s %s", __DATE__,__TIME__);    
    return CLI_OK;
}

int cmd_save_config(struct cli_def *cli, char *command, char *argv[], int argc)
{
    cs_status ret = CS_E_OK;
    extern int save_config();
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
    
    ret = db_save_to_flash();
    
    if(ret != CS_E_OK)
    cli_print(cli, "Save to flash fail");
    else
    cli_print(cli, "Save to flash success");
    return CLI_OK;
}




//interface IFNAME
int cmd_config_int(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "IFNAME", "Interface name",
                NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(argc > 1)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    if (strcasecmp(argv[0], "0") == 0)
        cli_set_configmode(cli, MODE_CONFIG_INTF, "if[0]");
    else if (strcasecmp(argv[0], "1") == 0)
        cli_set_configmode(cli, MODE_CONFIG_INTF, "if[1]");
    else
        cli_print(cli, "Unknown interface %s", argv[0]);

    return CLI_OK;
}


//  igmp snooping enable max_ch <1-16>
//  igmp snooping enable

int cmd_snoop_enable(struct cli_def *cli, char *command, char *argv[], int argc)
{
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "max_ch", "Maximum number of channels",
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

    if(0 == argc)
    {
        cli_print(cli, "enable igmp snooping with default channel num");
    }
    else if(2 == argc && MATCH("max_ch", argv[0]))
    {
        cli_print(cli, "enable igmp snooping with max_ch %d", atoi(argv[1]));
    }
    else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}


#if 1
extern cs_status mc_mode_get(cs_dev_id_t device, mc_mode_t *mode);
cs_status mc_mode_set(cs_dev_id_t device, mc_mode_t mode);

extern int igmp_mode_show(mc_mode_t mc_mode)
{
	int ret = 0;
	char mode[15] = {0};
	switch(mc_mode) 
	{
        case MC_SNOOPING:
            strncpy(mode, "snooping", sizeof(mode));
            break;

        case MC_MANUAL:
            strncpy(mode, "manual", sizeof(mode));
            break;

		case MC_PROXY:
            strncpy(mode, "proxy", sizeof(mode));
            break;

		case MC_DISABLE:
            strncpy(mode, "transparent", sizeof(mode));
            break;

        default:
            strncpy(mode, "unknown", sizeof(mode));
            break;
	}
	cs_printf("igmp %s mode\n", mode);
	
	return ret;
}


extern int igmp_mode_config_recover(mc_mode_t mc_mode)
{
	int ret = 0;
	ret = mc_mode_set(0, mc_mode);
	if(CS_E_OK == ret)
	{
		cs_printf("mc_mode_set success\n");
	}
	else
	{
		cs_printf("mc_mode_set failed\n");
	}
	return ret;
}

#define IGMP_MODE_DEFAULT	MC_DISABLE

extern int igmp_mode_tlv_infor_get(int *len, char **value, int *free_need)
{
	int ret = 0;
	mc_mode_t *mc_mode = NULL;
	cs_status status = CS_E_OK;
	if(NULL == len)
	{
		goto error;
	}
	else
	{
		*len = 0;
	}

	if(NULL == value)
	{
		goto error;
		
	}
	else
	{
		*value = NULL;
	}
	
	if(NULL == free_need)
	{
		goto error;
	}
	else
	{
		*free_need = 0;
	}

	mc_mode = (mc_mode_t *)iros_malloc(IROS_MID_APP, sizeof(mc_mode_t));
	memset(mc_mode, 0, sizeof(mc_mode_t));
	*free_need = 1;
	status = mc_mode_get(0, mc_mode);
	if(CS_E_OK == status)
	{
		if(IGMP_MODE_DEFAULT == *mc_mode)
		{
			goto end;
		}
		else
		{
			*len = sizeof(mc_mode_t);
			*value = (char *)mc_mode;
		}
	}
	else
	{
		cs_printf("mc_mode_get failed\n");
		goto error;
	}
	
	ret = 0;
	goto end;
	
error:
	ret = -1;
	
end:
	if((0 == *len)&&(NULL != mc_mode))
	{
		iros_free(mc_mode);
		mc_mode = NULL;
	}
	return ret;
}

extern int igmp_mode_tlv_infor_handle(int len, char *data, int opcode)
{
	int ret = 0;
	mc_mode_t mc_mode;
	
	if(0 != len)
	{
		//do nothing
	}
	else
	{
		goto error;
	}
	
	if(NULL != data)
	{
		//do nothing
	}
	else
	{
		goto error;
	}
	memcpy(&mc_mode, data, len);
	
	if(DATA_RECOVER == opcode)
	{
		igmp_mode_config_recover(mc_mode);
	}
	else if(DATA_SHOW == opcode)
	{
		igmp_mode_show(mc_mode);
	}
	else
	{
		cs_printf("in %s\n", __func__);
	}
	ret = 0;
	goto end;
	
error:
	ret = -1;
	
end:
	return ret;
}

extern int igmp_mode_running_config_show(void)
{
	int ret = 0;
	mc_mode_t mc_mode;
	ret = mc_mode_get(0, &mc_mode);
	if(CS_E_OK == ret)
	{
		if(IGMP_MODE_DEFAULT != mc_mode)
		{
			igmp_mode_show(mc_mode);
		}
	}
	else
	{
		cs_printf("mc_mode_get failed\n");
	}
	
	return ret;
}


extern int cmd_igmp_mode(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if(CLI_HELP_REQUESTED)
	{
		switch(argc)
		{
			case 1:
				cli_arg_help(cli, 0,
				"<cr>", "just enter to execuse command. (show igmp mode)",
				NULL);
				cli_arg_help(cli, 0,
				"0", "set igmp snooping mode",
				NULL);
				cli_arg_help(cli, 0,
				"1", "set igmp auth mode",
				NULL);
				cli_arg_help(cli, 0,
				"2", "set igmp disable mode",
				NULL);
				return CLI_OK;
			case 2:
				return cli_arg_help(cli, 0,
				"<cr>", "just enter to execuse command. (show igmp mode)",
				NULL);
			default:
				return cli_arg_help(cli, argc > 1, NULL);
		}
	}
	if(argc > 1)
	{
		cli_print(cli, "%% Invalid input.");
		return CLI_OK;
	}
	else
	{
		//do nothing
	}

	if(0 == argc)
	{
		cs_status ret = CS_E_OK;
		mc_mode_t mc_mode;
    	ret = mc_mode_get(0, &mc_mode);
		if(CS_E_OK == ret)
		{
			igmp_mode_show(mc_mode);
		}
		else
		{
			cs_printf("mc_mode_get failed\n");
		}
	}
	else if(1 == argc)
	{
		cs_status ret = CS_E_OK;
		int mode = 0;
		mc_mode_t mc_mode;
		mode = iros_strtol(argv[0]);
		switch(mode)
		{
			case 0:
				mc_mode = MC_SNOOPING;
				break;
			case 1:
				mc_mode = MC_MANUAL;
				break;
			case 2:
				mc_mode = MC_DISABLE;
				break;
			default:
				mc_mode = MC_SNOOPING;
				break;
		}
		
		ret = mc_mode_set(0, mc_mode);
		if(CS_E_OK == ret)
		{
			cs_printf("mode :0x%x\n", mc_mode);
			cs_printf("mc_mode_set success\n");
		}
		else
		{
			cs_printf("mc_mode_set failed\n");
		}
	}
	else
	{
		//do nothing
	}

	return CLI_OK;
}

#endif

#if 0
extern onu_slow_path_cfg_cfg_t g_slow_path_ip_cfg;
cs_status mc_mode_save_to_flash(int mc_mode)
{
	#if 0
	cs_printf("in %s, mc_mode :0x%x\n", __func__, mc_mode);
	#endif
	cs_status ret = CS_E_OK;
	g_slow_path_ip_cfg.mc_mode = mc_mode;
	int status = 0;
	status = save_userdata_to_flash((unsigned char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(g_slow_path_ip_cfg));
	if(0 == status)
	{
		
		#if 1
		if(3 == g_slow_path_ip_cfg.mc_mode)
		{
			cs_printf("igmp mode :%d,save_userdata_to_flash success\n", mc_mode);
		}
		else
		{
			//do nothing
		}
		#endif
		
	}
	else
	{
		cs_printf("igmp mode :%d, save_userdata_to_flash failed\n", mc_mode);
	}	
	return ret;
}
extern cs_status mc_mode_get_from_flash(int *mc_mode)
{
	cs_status ret = CS_E_OK;
	onu_slow_path_cfg_cfg_t ip_config;
	int status = 0;
	status = get_userdata_from_flash((char *)&ip_config, GWD_PRODUCT_CFG_OFFSET_W, sizeof(ip_config));
	if(0 != status)
	{
		cs_printf("igmp mode :%d, get_userdata_from_flash failed\n");
		ret = CS_E_ERROR;
	}
	else
	{
		*mc_mode = ip_config.mc_mode;
		ret = CS_E_OK;
	}	
	return ret;
}
extern cs_status mc_mode_init(void)
{
	cs_status ret = CS_E_OK;
	mc_mode_t mc_mode;
	int mode = 0;
	ret = mc_mode_get_from_flash(&mode);
	if(CS_E_OK == ret)
	{
		if(3 != mode)
		{
			goto end;
		}
		else
		{
			//do nothing
		}
		mc_mode = MC_DISABLE;
		if(CS_E_OK == mc_mode_set(0, mc_mode))
		{
			cs_printf("%s success, igmp mode :%d\n", __func__, mc_mode);
		}
		else
		{
			cs_printf("%s failed\n", __func__);
		}
	}
	else
	{
		cs_printf("%s failed\n", __func__);
	}
end:
	return ret;
}
#endif

#if 1
extern onu_slow_path_cfg_cfg_t   g_slow_path_ip_cfg;
extern cs_status mc_mode_get(cs_dev_id_t device, mc_mode_t *mode);
cs_status mc_mode_save_to_config(void)
{
	cs_status ret = CS_E_OK;
	cs_dev_id_t device = 0;
	mc_mode_t mode;
	int mc_mode;
	mc_mode_get(device, &mode);
	mc_mode = mode;
	g_slow_path_ip_cfg.mc_mode = mc_mode;
	return ret;
}
cs_status config_save_to_flash(void)
{
	cs_status ret = CS_E_OK;
	int status = 0;
	status = save_userdata_to_flash((unsigned char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(g_slow_path_ip_cfg));
	if(0 == status)
	{
		//do nothing		
	}
	else
	{
		cs_printf("save_userdata_to_flash failed!\n");
	}
	return ret;
}

cs_status config_get_from_flash(void)
{
	cs_status ret = CS_E_OK;
	int status = 0;
	status = get_userdata_from_flash((char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(g_slow_path_ip_cfg));
	if(0 != status)
	{
		cs_printf("get_userdata_from_flash failed\n");
		ret = CS_E_ERROR;
	}
	else
	{
		ret = CS_E_OK;
	}
	return ret;
}


cs_status mc_mode_show(int mc_mode)
{
	cs_status ret = CS_E_OK;
	char mode[20] = {0};
	switch(mc_mode) 
	{
		case MC_SNOOPING:
			strncpy(mode, "snooping", sizeof(mode));
			break;
	
		case MC_MANUAL:
			strncpy(mode, "manual", sizeof(mode));
			break;
	
		case MC_PROXY:
			strncpy(mode, "proxy", sizeof(mode));
			break;
	
		case MC_DISABLE:
			strncpy(mode, "transparent", sizeof(mode));
			break;
	
		default:
			strncpy(mode, "unknown", sizeof(mode));
			break;
	}
	cs_printf("igmp %s mode\n", mode);
	
	return ret;
}
extern cs_status mc_mode_init(void)
{
	cs_status ret = CS_E_OK;
	mc_mode_t mc_mode;
	int mode = 0;
	mode = g_slow_path_ip_cfg.mc_mode;
	if(3 == mode)
	{
		mc_mode = MC_DISABLE;
		if(CS_E_OK == mc_mode_set(0, mc_mode))
		{
			cs_printf("%s success, igmp mode :%d\n", __func__, mc_mode);
		}
		else
		{
			cs_printf("%s failed\n", __func__);
		}
	}
	else
	{
		//do nothing
	}

	return ret;
}

#endif


int cmd_show_regular(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    cli_print(cli, "cli_regular() has run %u times", regular_count);
    return CLI_OK;
}



int cmd_debug_regular(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    debug_regular = !debug_regular;
    cli_print(cli, "cli_regular() debugging is %s", debug_regular ? "enabled" : "disabled");
    return CLI_OK;
}

extern void do_help(void);
extern int imst_cli_start(char *p);

int cmd_debug_legacy(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }

    if(CHANNEL_SERIAL == cli->channel)
    {
        cli_print(cli, "go to legacy CLI menu...");

        do_help();

#ifdef CYG_HAL_ARM_IMST_OLT
        imst_cli_start("olt->");
#else
        imst_cli_start("onu->");
#endif
    }
	
#ifdef HAVE_TELNET_CLI
	 else if(CHANNEL_TCP == cli->channel)
		{
			cli_print(cli, "go to legacy CLI menu...");

			do_help();
		if(CLI_QUIT == do_telnet_legacy_cmd(cli,"onu->"))
			{
		 		 return CLI_QUIT;
			}
		}
#endif /* HAVE_TELNET_CLI */
    return CLI_OK;
}

void pc(struct cli_def *cli, char *string)
{
    printf("%s\n", string);
}

#if 1
extern int save_user_tlv_data_to_flash(void);
int cmd_save(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int ret = 0;
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
    #if 0
	cs_printf("in %s\n", __func__);
	#endif
	ret = save_user_tlv_data_to_flash();
	if(0 == ret)
	{
		cs_printf("save config to flash success\n");
	}
	else
	{
		cs_printf("save config to flash failed\n");
	}
	
    return CLI_OK;
}

extern int running_config_show(void);

int cmd_show_running_config(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
    #if 0
	cs_printf("in %s\n", __func__);
	#endif
	running_config_show();
    return CLI_OK;
}

extern int start_up_show(void);

int cmd_show_start_up(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int ret = 0;
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
    #if 0
	cs_printf("in %s\n", __func__);
	#endif
	ret = start_up_show();
	if(0 == ret)
	{
		
	}
	else
	{
		cs_printf("show start-up failed\n");
	}
    return CLI_OK;
}

extern int start_up_config_erase(void);
int cmd_erase_config(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int ret = 0;
    if (CLI_HELP_REQUESTED)
        return CLI_HELP_NO_ARGS;

    if(argc > 0)
    {
        cli_print(cli, "%% Invalid input.");
        return CLI_OK;
    }
    #if 0
	cs_printf("in %s\n", __func__);
	#endif
	ret = start_up_config_erase();
	if(0 == ret)
	{
		cs_printf("erase config success\n");
	}
	else
	{
		cs_printf("erase config failed\n");
	}
	
    return CLI_OK;
}

#endif

void cli_reg_usr_cmd(struct cli_command **cmd_root)
{
    struct cli_command *c;
    struct cli_command *igmp, *snoop, *enable;
    struct cli_command *show_run, * show_run_system,*show_onu;
    struct cli_command *show_ver = NULL;
	//struct cli_command *save = NULL;
//	struct cli_command *show = NULL;
	struct cli_command *erase = NULL;
	
#ifdef HAVE_ONU_RSTP
    struct cli_command *   show_stp;
#endif
#ifdef HAVE_MC_CTRL
  struct cli_command * show_igmp ;
#endif


    c = cli_register_command(cmd_root, 0, "configure", NULL,                         PRIVILEGE_PRIVILEGED, MODE_EXEC, "Enter configuration mode");
        cli_register_command(cmd_root, c, "terminal", cli_int_configure_terminal,    PRIVILEGE_PRIVILEGED, MODE_EXEC, "Configure from the terminal");


    c = cli_register_command(cmd_root, 0, "reset", cmd_reset,                         PRIVILEGE_PRIVILEGED, MODE_ANY, "Reset ONU");
        cli_register_command(cmd_root, c, "factorySetting", cmd_reset_factory,    PRIVILEGE_PRIVILEGED, MODE_EXEC, "Reset ONU with factory setting");
        //cli_register_command(cmd_root, 0, "laser", cmd_laser,                         PRIVILEGE_PRIVILEGED, MODE_EXEC, "Laser on/off");
        cli_register_command(cmd_root, 0, "led", cmd_led,                         PRIVILEGE_PRIVILEGED, MODE_EXEC, "Led on/off");
        
    c = cli_register_command(cmd_root, NULL, "show",     NULL,               PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show running system information");
        cli_register_command(cmd_root, c,    "regular",  cmd_show_regular,   PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The counter for the times cli_regular has run");
    show_ver = cli_register_command(cmd_root,  c, "version",cmd_show_zte_version,       PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The version info");
        cli_register_command(cmd_root,  show_ver, "detail", cmd_show_zte_version_detail,PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The detail version info");
	 	//cli_register_command(cmd_root, c,  "authenKey", cli_int_show_authenkey,   		PRIVILEGE_PRIVILEGED, MODE_ANY,     "AuthenKey configuration");		

    show_onu = cli_register_command(cmd_root, c,    "onu",  NULL,   PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The onu information");
#ifdef HAVE_ZTE_OAM   
        cli_register_command(cmd_root, show_onu,    "serial-number",  cmd_show_serial_number,   PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The serial-number information");
#endif
        cli_register_command(cmd_root, show_onu, "ctc-loid-pass", cmd_show_ctc_loid_password, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show CTC loid and password");
        cli_register_command(cmd_root, show_onu,    "laser-status",  cmd_show_laser,   PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The laser-status information");
        cli_register_command(cmd_root, show_onu,    "led-status",  cmd_show_led,   PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The led-status information");
        cli_register_command(cmd_root, show_onu,    "ups-status",  cmd_show_ups,   PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The ups-status information");
        cli_register_command(cmd_root, show_onu,    "mac",  cmd_show_pon_mac,   PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The onu mac information");
        cli_register_command(cmd_root, show_onu,    "version",  cmd_show_version,   PRIVILEGE_UNPRIVILEGED, MODE_ANY, "The software/hardware information");
        

        
    show_run=cli_register_command(cmd_root, c,    "run",  cmd_show_run,   PRIVILEGE_PRIVILEGED, MODE_EXEC, "The system run information");
    show_run_system=cli_register_command(cmd_root, show_run,    "system",  cmd_show_run_system,   PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "The system information");
    /*show_ctc_vlan=cli_register_command(cmd_root, show_run,    "vlan",  cmd_show_run_ctc_vlan,   PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "The ctc vlan information");*/
#ifdef HAVE_MC_CTRL
    show_igmp=cli_register_command(cmd_root, show_run,    "igmp",  cmd_show_run_igmp,   PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "The igmp information");
#endif
    /*show_qos=cli_register_command(cmd_root, show_run,    "qos",  cmd_show_run_qos,   PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "The qos information");*/

#ifdef HAVE_ONU_RSTP
    show_stp=cli_register_command(cmd_root, show_run,    "stp",  cmd_show_run_stp,   PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "The stp information");
#endif


  //  cli_register_command(cmd_root, NULL, "interface",    cmd_config_int,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Select an interface to configure");
    igmp   = cli_register_command(cmd_root, NULL, "igmp",      NULL,     PRIVILEGE_PRIVILEGED, MODE_CONFIG_INTF, "Internet Group Management Protocol (IGMP)");
    snoop  = cli_register_command(cmd_root, igmp, "snooping",  NULL,     PRIVILEGE_PRIVILEGED, MODE_CONFIG_INTF, "IGMP snooping configuration");
    enable = cli_register_command(cmd_root, snoop, "enable",    cmd_snoop_enable,     PRIVILEGE_PRIVILEGED, MODE_CONFIG_INTF, "Enable");

	cli_register_command(cmd_root, NULL, "igmp-mode",  cmd_igmp_mode,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "IGMP mode get/set");
	
#ifdef HAVE_ZTE_OAM   
    cli_register_command(cmd_root, NULL, "serial-number",   cmd_serial_number,   PRIVILEGE_PRIVILEGED,   MODE_CONFIG,     "ONU serial number");
#endif

    cli_register_command(cmd_root, NULL, "ctc-loid-pass", cmd_ctc_loid_password, PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Set CTC loid and password");

	#if 0
    save = cli_register_command(cmd_root,  NULL, "save",NULL, PRIVILEGE_PRIVILEGED, MODE_ANY, "Save ONU config");
    cli_register_command(cmd_root,  save, "config", cmd_save_config,PRIVILEGE_PRIVILEGED, MODE_ANY, "Save ONU current config to flash,such as SN and LOID/Password");
	#endif
	#if 1
	cli_register_command(cmd_root,  NULL, "save", cmd_save,PRIVILEGE_PRIVILEGED, MODE_ANY, "Save ONU current config to flash");
	cli_register_command(cmd_root,  c, "running-config", cmd_show_running_config,PRIVILEGE_PRIVILEGED, MODE_ANY, "Show ONU current config");
	cli_register_command(cmd_root,  c, "start-up", cmd_show_start_up,PRIVILEGE_PRIVILEGED, MODE_ANY, "Show ONU saved config");
	erase = cli_register_command(cmd_root,  NULL, "erase", NULL,PRIVILEGE_PRIVILEGED, MODE_ANY, "Erase");
	cli_register_command(cmd_root,  erase, "config", cmd_erase_config,PRIVILEGE_PRIVILEGED, MODE_ANY, "Erase saved config");
	#endif
    cli_register_command(cmd_root, NULL, "onu-mac",   cmd_pon_mac,   PRIVILEGE_PRIVILEGED,   MODE_CONFIG,     "ONU PON mac");

  //  cli_register_command(cmd_root, NULL, "debug",    cmd_debug_mode_int, PRIVILEGE_PRIVILEGED,   MODE_CONFIG,    "Enter debug mode");
   // cli_register_command(cmd_root, NULL, "regular",  cmd_debug_regular,  PRIVILEGE_PRIVILEGED,   MODE_DEBUG,     "Switch for regular callback");
  //  cli_register_command(cmd_root, NULL, "legacy",   cmd_debug_legacy,   PRIVILEGE_PRIVILEGED,   MODE_DEBUG,     "Switch to legacy console CLI");
  
    return;
}

int check_auth(char *username, char *password, struct cli_def *cli)
{
#if 0
	//check default user
    if( !strcasecmp(username, DFT_ADMIN_NAME) &&
        !strcasecmp(password, DFT_ADMIN_PASS) )
    {
        cli_set_privilege(cli, PRIVILEGE_PRIVILEGED);
       // cli_set_configmode(cli, MODE_CONFIG, NULL);
        
/*#ifdef HAVE_ZTE_OAM
        if(cli->channel== CHANNEL_TCP)
        {
            cli_set_configmode(cli, MODE_ZTE, NULL);
        }        
#endif */
        return CLI_OK;
    }

    if( !strcasecmp(username, DFT_USER_NAME) &&
        !strcasecmp(password, DFT_USER_PASS) )
    {
        cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
        cli_set_configmode(cli, MODE_EXEC, NULL);
        return CLI_OK;
    }

    // can add other user name/passwd here

    return CLI_ERROR;
	#else
db_ctc_user_t admin, usr;

	if(db_read(DB_CTC_ADMIN_INFO_ID, (cs_uint8*)&admin, sizeof(admin)) == CS_OK &&
			db_read(DB_CTC_USER_INFO_ID, (cs_uint8*)&usr, sizeof(usr)) == CS_OK)
	{
	    //check default user
	    if( !strcasecmp(username, DFT_ADMIN_NAME) &&
	        !strcasecmp(password, DFT_ADMIN_PASS) )
	    {
	        cli_set_privilege(cli, PRIVILEGE_PRIVILEGED);
	        cli_set_configmode(cli, MODE_CONFIG, NULL);

	        return CLI_OK;
	    }

		if( !strcasecmp(username, DFT_ADMIN_NAME) &&
			!strcasecmp(password, DFT_ADMIN_PASS) )
		{
			cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
			cli_set_configmode(cli, MODE_EXEC, NULL);
			return CLI_OK;
		}
	}
	else
	{
		//check default user
		if( !strcasecmp(username, DFT_ADMIN_NAME) &&
			!strcasecmp(password, DFT_ADMIN_PASS) )
		{
			cli_set_privilege(cli, PRIVILEGE_PRIVILEGED);
			cli_set_configmode(cli, MODE_CONFIG, NULL);

	/*#ifdef HAVE_ZTE_OAM
			if(cli->channel== CHANNEL_TCP)
			{
				cli_set_configmode(cli, MODE_ZTE, NULL);
			}
	#endif */
			return CLI_OK;
		}

		if( !strcasecmp(username, DFT_USER_NAME) &&
			!strcasecmp(password, DFT_USER_PASS) )
		{
			cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
			cli_set_configmode(cli, MODE_EXEC, NULL);
			return CLI_OK;
		}
	}
    // can add other user name/passwd here

    return CLI_ERROR;
	#endif
}

int regular_callback(struct cli_def *cli)
{
    regular_count++;
    if (debug_regular)
    {
        cli_print(cli, "Regular callback - %u times so far", regular_count);
        cli_reprompt(cli);
    }
    return CLI_OK;
}

int check_enable(char *password)
{
    // diag_printf("%s", password);
    return !strcasecmp(password, DFT_ENABLE_PASSWD);
}


void cli_usr_init(struct cli_def *cli)
{
    // config env
    cli_set_banner(cli, CLI_BANNER);
    cli_set_hostname(cli, HOST_NAME);
    
    
    cli_regular(cli, regular_callback);
    cli_regular_interval(cli, 5); // change to 5 from default 1 second
    cli_set_idle_timeout(cli, DFT_SESSION_TIMEOUT); // set idle timeout

    cli_set_auth_callback(cli, check_auth);
    cli_set_enable_callback(cli, check_enable);

    return;
}

#endif
