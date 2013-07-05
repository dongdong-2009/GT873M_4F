#ifdef __cplusplus
extern  "C"
{
#endif 
  
#include "iros_config.h"
#include <network.h>
#include <cyg/kernel/kapi.h>
#include <pkgconf/io_fileio.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_io.h>
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <pkgconf/memalloc.h>
#include <pkgconf/isoinfra.h>
#include <sys/types.h>
#include <sys/param.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h> 

#include "cli_common.h"
#include "rcp_gwd.h"
#include "rcp_gwd_w.h"
#include "onu_uax.h"
#include "onu_datetype.h"
#include "onu_cli.h"
#include "onu_printf.h"
#include "onu_command_fun.h"
#include "oam.h"
#include "oam_std_pdu.h"
#include "osal_api_packet.h"
#include "packet.h"
//#include "cs_cmn.h"

struct slot_port *my_onu = NULL;
void *my_onu_port;
extern int HERO_W;
cs_int32 my_onu_port_arg;
#define GET_AND_CHECK_RCP_DEV_PTR	if((NULL == (pRcpDev = RCP_Get_Dev_Ptr((cs_ulong32)(my_onu_port)))) || (!RCP_Dev_Is_Valid((cs_ulong32)(my_onu_port)))) \
									{ \
										cli_print(cli,  "  No RCP device found.\r\n"); \
										return CMD_WARNING; \
									}
#define GET_AND_CHECK_RCP_DEV_PTR_FLASH	if((NULL == (pRcpDev = RCP_Get_Dev_Ptr_For_Flash((cs_ulong32)(my_onu_port))))) \
									{ \
										cli_print(cli,  "  No RCP device found.\r\n"); \
										return CMD_WARNING; \
									}
#define GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT	my_onu_port_arg = atoi(argv[0]); \
									if((NULL == (pRcpDev = RCP_Get_Dev_Ptr((cs_ulong32)my_onu_port_arg))) || (!RCP_Dev_Is_Valid((cs_ulong32)my_onu_port_arg))) \
									{ \
										cli_print(cli,  "  No RCP device found on port %d.\r\n", my_onu_port_arg); \
										return CMD_WARNING; \
									}

#define RCP_DISCOVERY_PERIOD_DEF	    10	
#define RCP_KEEP_ALIVE_TIMEOUT_DEF		1

#define PRODUCT_SERIES_REALTEK  0x04
#define PRODUCT_SERIES_UNKNOW  0x0

#define PRODUCT_TYPE_GH1508      0x0007
#define PRODUCT_TYPE_GH1516      0x0008
#define PRODUCT_TYPE_GH1524		 0x0009
#define PRODUCT_TYPE_GH1532		 0x000a
#define PRODUCT_TYPE_UNKNOW    0x0

#define BOARD_TYPE_GH1508          0x0002
#define BOARD_TYPE_GH1516          0x0003
#define BOARD_TYPE_GH1524          0x0004
#define BOARD_TYPE_GH1532          0x0005
#define BOARD_TYPE_UNKNOW         0x0

#define RCP_SHOWRUN_SOURCE_SWITCH	0
#define RCP_SHOWRUN_SOURCE_FLASH	1


struct _XCVR_DATA_{
	cs_uint16 cAddr;
	cs_uint8 * pcName;
	cs_uint16 cLen;
	cs_uint16 cType;
};
struct _XCVR_DATA_ RCPBoardInfoArr[] = {
	{ 0,  "Product-series", 		1, 0 },
	{ 1,  "Product-type", 		2, 1 },
	{ 3,  "Version", 			16, 2 },
	{ 19, "Board-type", 		2, 3 },
	{ 21, "Manufacture SN",	16, 4 },
	{ 37, "Hardware version", 	6, 5 },
	{ 43, "Manufacture date",	10, 6 }	
};
struct _XPORT_RATE_DATA_{
	cs_ulong32  rate;
	cs_uint8  *pcName;
};
struct _XPORT_RATE_DATA_  RCPPortRate[] ={
	{0,     "No limit"},
	{128,  "128Kbps"},
	{256,  "256Kbps"},
	{512,  "512Kbps"},
	{1000, "1Mbps"},
	{2000, "2Mbps"},
	{4000, "4Mbps"},
	{8000, "8Mbps"}
};
typedef enum _RCP_EEPROM_PARAM_
{
    RCP_EEPROM_PRODUCT_SERIES = 0,
    RCP_EEPROM_PRODUCT_TYPE = 1,
    RCP_EEPROM_VERSION = 2,
    RCP_EEPROM_MODULE_TYPE = 3,
    RCP_EEPROM_SERIAL_NUMBER = 4,
    RCP_EEPROM_HW_VERSION = 5,
    RCP_EEPROM_MANUFACTURE_DATE = 6,
    RCP_EEPROM_VALID_FLAG = 7
}RCP_EEPROM_PARAM_T;
void cli_print_w(struct cli_def *cli, cs_int8 *format, ...);
cs_int32 Rcp_Get_Eeprom_Param(RCP_DEV *dev, RCP_EEPROM_PARAM_T type, cs_uint8 *data);
cs_int32 Rcp_Set_Eeprom_Param(RCP_DEV *dev, RCP_EEPROM_PARAM_T type, cs_uint8 *data);
//void sendOamRcpLpbDetectNotifyMsg(cs_uint8 onuPort, cs_uint8 rcpPort, cs_uint8 state, cs_uint16 uvid, cs_uint8 *session);
extern void sendOamLpbDetectNotifyMsg(cs_uint8 onuPort, cs_uint8 rcpPort, cs_uint8 state, cs_uint16 uvid,cs_uint8 *session);
//cs_int32 rcp_device_mgt_showrun(struct vty *vty, cs_int32 source);
cs_int32 rcp_dev_status_check(void);
RCP_RX_RATE Rcp_PortRateToEnum(cs_ulong32 ulRate);
cs_int32 Rcp_Eeprom_Value_Set_Protect(struct cli_def *cli, RCP_DEV *pRcpDev, RCP_EEPROM_PARAM_T eeprm_param,  cs_uint8 *data, cs_ulong32 datalen);
cs_int32 Rcp_Get_Device_Info(RCP_DEV *pRcpDev);

#define RCP_THREAD_STACKSIZE     (3 * 1024)
#define RCPLOOP_THREAD_STACKSIZE (1* 1024)
cs_ulong32 gulRcpFrameHandleRegister = 0;
cs_uint8 rcp_thread_stack[RCP_THREAD_STACKSIZE];
cs_uint8 rcploop_thread_stack[RCPLOOP_THREAD_STACKSIZE];
cyg_handle_t  rcp_thread_handle;
cyg_handle_t  rcploop_thread_handle;
cyg_thread    rcp_thread_obj;
cyg_thread    rcploop_thread_obj;
cs_ulong32 gulEnableEpswitchMgt = 1;
extern cs_ulong32 gulGwdRcpAuth;

cs_ulong32 vlan_dot_1q_enable = 0;
cs_ulong32 gulEthRxTaskReady = 0;
#define EPON_ETHERTYPE_RCP  0x8899
extern RCP_DEV *rcpDevList[MAX_RRCP_SWITCH_TO_MANAGE];
extern cs_ulong32 gulNumOfPortsPerSystem;
extern cs_status app_pkt_reg_handler(cs_uint8 pkt_type, cs_pkt_handler_t hdl);
extern cs_status app_pkt_reg_parser(cs_uint8 pkt_type, cs_pkt_parser_t parser);


/*
 ** Define the commands
 */
 cs_status rcp_pkt_parser(cs_pkt_t *pPkt)
/*                                                                           */
/* INPUTS  : o frame_ptr - oam packet                                        */
/*           o len - oam length                                              */
/*           o port - received the oam source port                           */
/* OUTPUTS : N/A                                                             */
/* DESCRIPTION:                                                              */
/* process any oam packet                                                    */
/* $rtn_hdr_end                                                              */
/*****************************************************************************/
{
    oam_pdu_hdr_t *hdr = NULL;

    hdr = (oam_pdu_hdr_t*)(pPkt->data + pPkt->offset + pPkt->tag_num * sizeof(cs_vlan_hdr_t ));

    if(ntohs(hdr->eth_type) == EPON_ETHERTYPE_RCP)
    {
        pPkt->pkt_type = CS_PKT_RCP; 
        return CS_E_OK;
    }

    return CS_E_NOT_SUPPORT;
}

cs_uint8 rcp_pkt_proc(cs_pkt_t *pPkt)
/*                                                                           */
/* INPUTS  : o frame_ptr - oam packet                                        */
/*           o len - rcp pkt length                                              */
/*           o port - received the rcp source port                           */
/* OUTPUTS : N/A                                                             */
/* DESCRIPTION:                                                              */
/* process rcp packet                                                    */
/* $rtn_hdr_end                                                              */
/*****************************************************************************/
{
    cs_uint8 *frame_ptr;
    cs_uint32 len;
    cs_port_id_t port;


    frame_ptr = pPkt->data + pPkt->offset;
    len = pPkt->len;
    port = pPkt->port;

    RcpFrameRevHandle(port, len, frame_ptr);

    return 1;
}
cs_int32 cli_int_interface_switch(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{    
    extern  cs_ulong32 vlan_dot_1q_enable;
 	
	cs_ulong32 onuport;
    cs_int8    ifName[IFM_NAME_SIZE + 1];
    cs_int8    prompt[64] = { 0 };
	if(CLI_HELP_REQUESTED)
    {
    
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<slot/port>", "Specify ethernet interface's slot and port",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	 if(1 == argc)
    {  

		bzero( ifName, IFM_NAME_SIZE + 1 );
   		snprintf( ifName, IFM_NAME_SIZE, "%s", argv[0] );
		my_onu = (struct slot_port *)iros_malloc(IROS_MID_APP,sizeof(struct slot_port));
		if(!my_onu)
			{
				return CMD_WARNING;
			}
		memset(my_onu,0,sizeof(struct slot_port));
		my_onu = BEGIN_PARSE_PORT_EAND_SLOT(argv[0],my_onu,ifName,cli);
		if(!my_onu)
			{
				iros_free(my_onu);
				cli_print(cli, "%% Invalid input.");
				return COMMAND_SUCCESS;
			}
		onuport = ETH_SLOTPORT_TO_PORTNO(my_onu->ulSlot,my_onu->ulPort);
		if(1 != RCP_Dev_Is_Exist(onuport))
		{
			iros_free(my_onu);
			cli_print( cli, "%% No switch connected to interface %s.\r\n", ifName );
			return CMD_WARNING;
		}
		else
		{
		
			if(vlan_dot_1q_enable == 1)
				{
					cli_print(cli, "  Management vlan VID is : %d.\r\n", rcpDevList[my_onu->ulPort]->mgtVid);
				}
			
		}
		my_onu_port = (void *)onuport;
		
 		strcpy( prompt, ifName );
		cli_set_switch_onuport_mode_enter(cli, MODE_SWITCH, prompt);
		iros_free(my_onu);
	 }
	 else
	 {
        cli_print(cli, "%% Invalid input.");
     }
    
    return COMMAND_SUCCESS;
}



cs_int32 cli_int_get_rcpreg(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_uint16 	usRegAddr,usValue;
	cs_ulong32 	ulStrLen, ulRegLen;
	cs_int8 	*pcTmp = NULL;
	cs_int32 	iRet;
	cs_int8    *outbuff;
	cs_int32 	i;
	RCP_DEV *pRcpDev;


	if(CLI_HELP_REQUESTED)
    {
    
        switch(argc)
        {
      
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>","Onu port" ,
                 NULL);
		case 2:
			return cli_arg_help(cli,0,
				"<regaddr>","Register address(HEX)" ,
				NULL);
		case 3:
			return cli_arg_help(cli, 0,
				"{<length>}*1","Register length",
				NULL);
        default:
            return cli_arg_help(cli, argc > 3, NULL);
        }
	}
	if(argc == 2 || argc == 3)
		{
			GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT

			ulStrLen = 0;
			usRegAddr = (cs_uint16)strtoul(argv[1], &pcTmp, 16);
			ulRegLen = 1;
			if(argc == 3)
			{
				ulRegLen = atoi( argv[2] );
			}
			outbuff = (cs_int8 *)malloc(4096);
			if(NULL == outbuff)
			{
				cli_print(cli,  "  No memory for display.\r\n");
				return CMD_WARNING;
			}
			ulStrLen += ECOS_Sprintf(outbuff + ulStrLen, "     All in HEX\r\n");
			ulStrLen += ECOS_Sprintf(outbuff + ulStrLen, "  Address   Value\r\n");
			ulStrLen += ECOS_Sprintf(outbuff + ulStrLen, "-------------------\r\n");
		
			if(ulRegLen > 32)
				{
					ulRegLen = 32;
				}
			for(i=0; i<ulRegLen; i++)
			{
				iRet = pRcpDev->frcpReadReg(pRcpDev, usRegAddr, &usValue);
				if(0 == iRet)
					ulStrLen += ECOS_Sprintf(outbuff + ulStrLen, 
						"     %04X-->%04X\r\n", usRegAddr, usValue);
				else
					ulStrLen += ECOS_Sprintf(outbuff + ulStrLen, 
						"     %04X-->N.A\r\n", usRegAddr);
				usRegAddr++;
			}
			ulStrLen += ECOS_Sprintf(outbuff + ulStrLen, "-------------------\r\n");

			cli_print(cli,"outbuff %s\n",outbuff);
			free(outbuff);
		}
	else
		{
			cli_print(cli,"%% Invalid input.");
		}

	

	return CLI_OK;
}



cs_int32 cli_int_set_rcpreg(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_uint16 	usRegAddr;
	cs_uint16 usValue;
	cs_int8 	*pcTmp = NULL;
	cs_int32	 	iRet;
	RCP_DEV *pRcpDev;

	
	if(CLI_HELP_REQUESTED)
    {
    	
        switch(argc)
        {
        
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>","Onu port" ,
                 NULL);
		case 2:
			return cli_arg_help(cli,0,
				"<regaddr>","Register address(HEX)" ,
				NULL);
		case 3:
			return cli_arg_help(cli, 0,
				"<value>","Register value(in HEX)",
				NULL);
        default:
            return cli_arg_help(cli, argc > 3, NULL);
        }
	}
	if(3 == argc)
		{
			GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT

			usRegAddr = (cs_uint16)strtoul(argv[1], &pcTmp, 16);
			usValue = (cs_uint16)strtoul(argv[2], &pcTmp, 16);

			if((iRet = pRcpDev->frcpWriteReg(pRcpDev, usRegAddr, usValue)) == 0)
			{
				cli_print(cli,  " RCP Device Register 0x%04X Value: 0x%04X\r\n", usRegAddr, usValue);
			}
			else
			{
				cli_print(cli,  " Write RCP Device Register 0x%04X Value failed(%d).\r\n", usRegAddr, iRet);
			}
	
		}
	
	return CLI_OK;
}



cs_int32 cli_int_get_rcpeeprom(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
    cs_uint16 	usEeAddr,usLen;
    cs_int8 	*pcTmp = NULL;
    cs_uint8     data;
	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;
	
	
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
             return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<eeaddr>", "Eeprom address(HEX)\n",
                 NULL);
		case 3:
           return cli_arg_help(cli, 0,
                "<length>", "Eeprom value length\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 3, NULL);
        }
	}
	if(argc < 2)
		{	
			diag_printf("  Command incomplete.\n");
			return CLI_OK;
		}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	usEeAddr = (cs_uint16)strtoul(argv[1], &pcTmp, 16);
	if(argc == 2)
	{
	    if(RCP_OK !=(ret = RCP_GetEepromValue(pRcpDev, usEeAddr, &data)))
	        cli_print(cli,  "  RCP_GetEepromValue failed(%d).\r\n", ret);
	    else
	        cli_print(cli,  "  data : 0x%x\r\n",  data);
	}
	if(argc == 3)
	{
	    usLen = (cs_uint16)atoi(argv[2]);
	    if(RCP_OK != (ret = RCP_GetEepromField(pRcpDev, usEeAddr, 0, usLen, &data)))     
	        cli_print(cli,  "  RCP_GetEepromValue failed(%d)\r\n", ret);
	    else
	        cli_print(cli,  "  data : 0x%x\r\n", data);
	}
	return CLI_OK;
}

cs_int32 cli_int_set_rcpeeprom(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
    cs_uint16 	usEeAddr,usValue;
    cs_int8 	*pcTmp = NULL;
   
	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;

	
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<eeaddr>", "Eeprom address(in HEX)\n",
                 NULL);
		case 3:
            return cli_arg_help(cli, 0,
                "<value>", "Eeprom value(in HEX)\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 3, NULL);
        }
	}
	if(argc < 2)
		{	
			diag_printf("  Command incomplete.\n");
			return CLI_OK;
		}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	usEeAddr = (cs_uint16)strtoul(argv[1], &pcTmp, 16);
	usValue = (cs_uint8)strtoul(argv[2], &pcTmp, 16);

	if(RCP_OK != (ret = RCP_SetEepromValue(pRcpDev, usEeAddr, usValue)))
	    cli_print(cli,  "  Set eeprom value failed(%d).\r\n", ret);
	return CLI_OK;
}


cs_int32 cli_int_get_rcpphy(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
    cs_uint16 	usphyAddr,usLen;
    cs_ulong32 ulPort;
    cs_int8 	*pcTmp = NULL;
    cs_uint16     data;
	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;

	
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<portnum>", "Rcp port\n",
                 NULL);
		case 3:
            return cli_arg_help(cli, 0,
                "<phyaddr>", "Phy address(in HEX)\n",
                 NULL);
		case 4:
            return cli_arg_help(cli, 0,
                "<length>}*1", "Phy value length\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 4, NULL);
        }
	}
	if(argc < 3)
		{
			diag_printf("  Command incomplete.\n");
			return CLI_OK;
		}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	ulPort = (cs_ulong32)atoi(argv[1]);
	usphyAddr = (cs_uint16)strtoul(argv[2], &pcTmp, 16);
	
	if(argc == 3)
	{
	    if(RCP_OK !=(ret = RCP_GetPhyRegValue(pRcpDev, 1, ulPort, usphyAddr, &data)))
	        cli_print(cli,  "  RCP_GetPhyValue failed(%d).\r\n", ret);
	    else
	        cli_print(cli,  "  data : 0x%x\r\n",  data);
	}
	if(argc == 4)
	{
	    usLen = (cs_uint16)atoi(argv[3]);
	    if(RCP_OK != (ret = RCP_GetPhyRegField(pRcpDev, 1, ulPort,usphyAddr, 0, usLen, &data)))     
	        cli_print(cli,  "  RCP_GetPhyField failed(%d)\r\n", ret);
	    else
	        cli_print(cli,  "  data : 0x%x\r\n", data);
	}
	return CLI_OK;
}


cs_int32 cli_int_set_rcpphy(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 	usphyAddr,usValue;
	cs_ulong32 usPort;
	cs_int8 	*pcTmp = NULL;

	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;

	
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<portnum>", "Rcp port\n",
                 NULL);
		case 3:
            return cli_arg_help(cli, 0,
                "<eeaddr>", "Phy address(in HEX)\n",
                 NULL);
		case 4:
            return cli_arg_help(cli, 0,
                "<value>", "Phy value(in HEX)\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 4, NULL);
        }
	}
	if(argc < 2)
		{
			diag_printf("  Command incomplete.\n");
			return CLI_OK;
		}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	usPort = (cs_ulong32)atoi(argv[1]);
	usphyAddr = (cs_uint16)strtoul(argv[2], &pcTmp, 16);
	usValue = (cs_uint16)strtoul(argv[3], &pcTmp, 16);

	if(RCP_OK != (ret = RCP_SetPhyRegValue(pRcpDev, 1, usPort, usphyAddr, usValue)))
	    cli_print(cli,  "  Set eeprom value failed(%d).\r\n", ret);
	return CLI_OK;
}


cs_int32 cli_int_show_authenkey(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_int8 *pcTmp = NULL;
	cs_uint16  usAuthenKey;
	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;

	
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<value>", "New AuthenKey value to set\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(argc < 2)
		{
			diag_printf("  Command incomplete.\n");
			return CLI_OK;
		}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	if(argc == 1)
	{
	    cli_print(cli,  "  Current AuthenKey : 0x%x%x\r\n", pRcpDev->authenKey[0], pRcpDev->authenKey[1]);
	}
	if(argc == 2)
	{
       usAuthenKey = (cs_uint16)strtoul(argv[1], &pcTmp, 16);
       if(RCP_OK == (ret = RCP_SetAuthenKey(pRcpDev, usAuthenKey)))               
           cli_print(cli,  "  Current AuthenKey : 0x%x%x\r\n", pRcpDev->authenKey[0], pRcpDev->authenKey[1]);
       else
           cli_print(cli,  "  Set authenKey failed(%d).\r\n", ret);
	}
	return CLI_OK;
}

cs_int32 cli_int_set_product_series(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	
	cs_uint8 series;
	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;

	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
           cli_arg_help(cli, 0,
                "realtek", "Realtek series\n",
                 NULL);
		  return cli_arg_help(cli, 0,
                "unknown",  "UnKnown series\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	if (0 == (cs_uint8)strcmp(argv[1], "realtek"))
	{
	    series = PRODUCT_SERIES_REALTEK;
	}
	else 
	{
	    series = PRODUCT_SERIES_UNKNOW;
	}
	
	if(RCP_OK == (ret=Rcp_Eeprom_Value_Set_Protect(cli, pRcpDev, RCP_EEPROM_PRODUCT_SERIES, &series, 1)))
	    cli_print(cli,  "  Set product series success \r\n");
	else
	    cli_print(cli,  "  Set product series failed(%d).\r\n", ret);
	return CLI_OK;
}



cs_int32 cli_int_set_product_type(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;

	cs_uint16 usType;
	cs_uint8 ucData[2];
	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;

	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
               cli_arg_help(cli, 0,
                "gh1508", "Product type GH1508\n",
                 NULL);
			   cli_arg_help(cli, 0,
                "gh1516", "Product type GH1516\n",
                 NULL);
			   cli_arg_help(cli, 0,
                "gh1524", "Product type GH1524\n",
                 NULL);
			   cli_arg_help(cli, 0,
                "gh1532", "Product type GH1532\n",
                 NULL);
			   cli_arg_help(cli, 0,
                "unknown", "Product type UnKnow\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if (0 == strcmp(argv[1], "gh1508"))
	{
		usType = PRODUCT_TYPE_GH1508;
	}
	else if (0 == strcmp(argv[1], "gh1516"))
	{
		usType = PRODUCT_TYPE_GH1516;
	}
	else if (0 == strcmp(argv[1], "gh1524"))
	{
		usType = PRODUCT_TYPE_GH1524;
	}
	else if (0 == strcmp(argv[1], "gh1532"))
	{
		usType = PRODUCT_TYPE_GH1532;
	}
	else 
	{
		usType = PRODUCT_TYPE_UNKNOW;
	}

	ucData[0] = (cs_uint8)(usType>>8);
	ucData[1] = (cs_uint8)(usType&0xFF);

	if(RCP_OK == (ret=Rcp_Eeprom_Value_Set_Protect(cli, pRcpDev, RCP_EEPROM_PRODUCT_TYPE, ucData, 2)))
	    cli_print(cli,  "  Set product type success! \r\n");
	else
	    cli_print(cli,  "  Set product type failed(%d).\r\n", ret);
	return CLI_OK;
}



cs_int32 cli_int_set_board_type(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;

	cs_uint16 usType;
	cs_uint8 ucData[2];
	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;

	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
               cli_arg_help(cli, 0,
                "gh1508", "Product type GH1508\n",
                 NULL);
			   cli_arg_help(cli, 0,
                "gh1516", "Product type GH1516\n",
                 NULL);
			   cli_arg_help(cli, 0,
                "gh1524", "Product type GH1524\n",
                 NULL);
			   cli_arg_help(cli, 0,
                "gh1532", "Product type GH1532\n",
                 NULL);
			   cli_arg_help(cli, 0,
                "unknown", "Product type UnKnow\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	if (0 == strcmp(argv[1], "gh1508"))
	{
		usType = BOARD_TYPE_GH1508;
	}
	else if (0 == strcmp(argv[1], "gh1516"))
	{
		usType = BOARD_TYPE_GH1516;
	}
	else if (0 == strcmp(argv[1], "gh1524"))
	{
		usType = BOARD_TYPE_GH1524;
	}
	else if (0 == strcmp(argv[1], "gh1532"))
	{
		usType = BOARD_TYPE_GH1532;
	}
	else 
	{
		usType = BOARD_TYPE_UNKNOW;
	}
	
	ucData[0] = (cs_uint8)(usType>>8);
	ucData[1] = (cs_uint8)(usType&0xFF);

	if(RCP_OK == (ret=Rcp_Eeprom_Value_Set_Protect(cli, pRcpDev, RCP_EEPROM_MODULE_TYPE, ucData, 2)))
	    cli_print(cli,  "  Set board type success!\r\n");
	else
	    cli_print(cli,  "  Set board type failed(%d).\r\n", ret);
	return CLI_OK;
}



cs_int32 cli_int_set_rcphw_version(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret = RCP_OK;
	cs_ulong32 major, minor, branch;
	cs_uint8 version[7];
	RCP_DEV *pRcpDev;
//	cs_ulong32 	onuPort;

	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<1-9>", "Major version\n",
                 NULL);
		case 3:
            return cli_arg_help(cli, 0,
                "<0-9>", "Minor version\n",
                 NULL);
		case 4:
            return cli_arg_help(cli, 0,
                "<1-9>", "Branch version\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 4, NULL);
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	major = atoi(argv[1]);
	minor = atoi(argv[2]);
	branch = atoi(argv[3]);

	sprintf(version, "V%d.%dB%d", major,minor,branch);

	if(RCP_OK == (ret=Rcp_Eeprom_Value_Set_Protect(cli, pRcpDev, RCP_EEPROM_HW_VERSION, version, 7)))
	    cli_print(cli,  "  Set hardware version success! \r\n");
	else	
	    cli_print(cli,  "  Set hardware version failed(%d).\r\n", ret);

	return CLI_OK;
}


cs_int32 cli_int_set_rcpmanufacture_serial(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret = RCP_OK;
	INT  len;
	INT  i;
	cs_uint8 tmpStr[18];
	cs_uint8 manu_serial[18];
	RCP_DEV *pRcpDev;
//	cs_ulong32 	onuPort;

	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<string>", "Serial number(ie, 1010)\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	if((len = strlen(argv[1])) > 16)
	{
		cli_print( cli, "  The length of serial number must be less than %d.\r\n", 16);
		return CLI_OK;
	}
      for(i=0; i<len; i++)
		tmpStr[i] = toupper(argv[1][i]);
	tmpStr[i] = '\0';
	
	sprintf(manu_serial, "%s", tmpStr);

	if(RCP_OK == (ret=Rcp_Eeprom_Value_Set_Protect(cli, pRcpDev, RCP_EEPROM_SERIAL_NUMBER, manu_serial, 16)))
		cli_print(cli,  "  Set manufacture serial success!\r\n");
	else
		cli_print(cli,  "  Set manufacture serial failed(%d)!\r\n",ret);

	return CLI_OK;
}


cs_int32 cli_int_rcpmanufacture_date(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 year, month, date;
	cs_uint8 manu_date[12];
	RCP_DEV *pRcpDev;
	//cs_ulong32 	onuPort;

	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<2007-2100>", "Year\n",
                 NULL);
		case 3:
            return cli_arg_help(cli, 0,
                "<1-12>", "Month\n",
                 NULL);
		case 4:
            return cli_arg_help(cli, 0,
                "<1-31>", "Date\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 4, NULL);
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	year = atol(argv[1]);
	month = atol(argv[2]);
	date = atol(argv[3]);
	
	sprintf(manu_date, "%ld-%02ld-%02ld", year, month, date);

	if(RCP_OK == (ret=Rcp_Eeprom_Value_Set_Protect(cli, pRcpDev, RCP_EEPROM_MANUFACTURE_DATE, manu_date, 12)))
	    cli_print(cli,  "  Set manufacture date success!\r\n");
	else
	    cli_print(cli,  "  Set manufacture date failed(%d).\r\n", ret);
	return CLI_OK;
}



cs_int32 cli_int_set_rcpmac(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret = 0, i =0;
	cs_uint16 eeAddr;
	cs_uint8    cMac[6], tmpmac1[6], tmpmac2[6];

	RCP_DEV *pRcpDev;


	
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<1-24>", "Onu port\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<H.H.H>", "Mac address\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(argc < 2)
		{
			diag_printf("  Command incomplete.\n");
			return CLI_OK;
		}
	GET_AND_CHECK_RCP_DEV_PTR_FROM_PORT
	if (GWD_RETURN_OK != GetMacAddr(argv[1], cMac))
	{
		cli_print(cli,  "  Invalid MAC address. \r\n");
		return CLI_OK;
	}
	do
	{
		for(eeAddr = 0; eeAddr < RCP_MAC_SIZE; eeAddr++)
		{
			ret  =  RCP_SetEepromValue(pRcpDev, RCP_MAC_BASIC_ADDR + eeAddr, cMac[eeAddr]);
			ret += RCP_SetEepromValue(pRcpDev, RCP_MAC_BASIC_ADDR + 0x20c + eeAddr, cMac[eeAddr]);
			ret += RCP_GetEepromValue(pRcpDev, RCP_MAC_BASIC_ADDR + eeAddr, &tmpmac1[eeAddr]);
			ret += RCP_GetEepromValue(pRcpDev, RCP_MAC_BASIC_ADDR + 0x20c + eeAddr, &tmpmac2[eeAddr]);
		}
		if((RCP_OK == ret) && (0 ==memcmp(cMac, tmpmac1, 6)) && (0 ==memcmp(cMac, tmpmac2, 6)))
		{
			cli_print(cli,  "  Save mac address success.\r\n");
			return CLI_OK;
		}
		else
		{
			i++;
			diag_printf("\r\nSet the sys Mac failed for %d times", i);
		}
	}
	while(i < 5);

	cli_print(cli,  "  Set mac failed.(%d)\r\n", ret);
	return CLI_OK;

}
#if 0
#endif


cs_int32 cli_int_show_mac(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32     ret;
	cs_uint8    cMac[6];
	cs_uint16  eeAddr;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR	
	if(CLI_HELP_REQUESTED)
    {    	
         return cli_arg_help(cli, argc == 1, NULL);
	}
	
	for(eeAddr = 0; eeAddr < RCP_MAC_SIZE; eeAddr++)
	{
		if(RCP_OK != (ret = RCP_GetEepromValue(pRcpDev, RCP_MAC_BASIC_ADDR + eeAddr, &cMac[eeAddr])))
		{
			cli_print(cli,  "  Get mac failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	cli_print(cli,  "  Switch MAC address is %02x%02x.%02x%02x.%02x%02x.\r\n",
		cMac[0], cMac[1], cMac[2], cMac[3], cMac[4], cMac[5]);
	return CLI_OK;
}

cs_int32 cli_int_show_mgt_port(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	RCP_DEV *pRcpDev;
	cs_ulong32 slot,mgtport;
	cs_int32 ret;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {    	
         return cli_arg_help(cli, argc == 1, NULL);
	}
	
	if(RCP_OK != (ret = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtport)))
	{
		cli_print(cli,  "  Get mgt port failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	cli_print(cli,  "  Downlink Management port is : %ld\r\n", mgtport);
	return CLI_OK;
}


cs_int32 cli_int_system_information(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint8 series;
	cs_uint8 pro_type[3];
	cs_uint16 proType;
	cs_uint8 board_type[3];
	cs_uint16 boardType;
	cs_uint8 hwversion[7];
	cs_uint8 serial[17];
	cs_uint8 date[11];
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR	
		
	if(CLI_HELP_REQUESTED)
    {    	
         return cli_arg_help(cli, argc == 1, NULL);
	}
	
	if(RCP_OK == (ret = Rcp_Get_Eeprom_Param(pRcpDev, RCP_EEPROM_PRODUCT_SERIES, &series)))
	{
	    switch(series)
	    {
               case PRODUCT_SERIES_REALTEK :
                   cli_print(cli,  "  Product series is     : Realtek Series.(0x%x)\r\n",series);
                   break;
               default :
                   cli_print(cli,  "  Product series is     : UnKnow Series.(0x%x)\r\n",series);
	    }
	}
	if(RCP_OK == (ret = Rcp_Get_Eeprom_Param(pRcpDev, RCP_EEPROM_PRODUCT_TYPE, pro_type)))
	{
	    proType = ((pro_type[0] << 8) | pro_type[1]);
	    switch(proType)
	    {
               case PRODUCT_TYPE_GH1508 :
                   cli_print(cli,  "  Product type is       : GH1508.(0x%x)\r\n",proType);
                   break;
               case PRODUCT_TYPE_GH1516 :
                   cli_print(cli,  "  Product type is       : GH1516.(0x%x)\r\n",proType);
                   break;
			   case PRODUCT_TYPE_GH1524 :
                   cli_print(cli,  "  Product type is       : GH1524.(0x%x)\r\n",proType);
                   break;			   	
			   case PRODUCT_TYPE_GH1532 :
                   cli_print(cli,  "  Product type is       : GH1532.(0x%x)\r\n",proType);
                   break;			   	
               default :
                   cli_print(cli,  "  Product type is       : UnKnown.(0x%x)\r\n",proType);
	    }
	}
	if(RCP_OK == (ret = Rcp_Get_Eeprom_Param(pRcpDev, RCP_EEPROM_MODULE_TYPE, board_type)))
	{
	    boardType = ((board_type[0] << 8) | board_type[1]);
	    switch(boardType)
	    {
               case BOARD_TYPE_GH1508 :
                   cli_print(cli,  "  Board type is         : GH1508.(0x%x)\r\n",boardType);
                   break;
               case BOARD_TYPE_GH1516 :
                   cli_print(cli,  "  Board type is         : GH1516.(0x%x)\r\n",boardType);
                   break;
               case BOARD_TYPE_GH1524:
                   cli_print(cli,  "  Board type is         : GH1524.(0x%x)\r\n",boardType);
                   break;
               case BOARD_TYPE_GH1532:
                   cli_print(cli,  "  Board type is         : GH1532.(0x%x)\r\n",boardType);
                   break;
               default :
                   cli_print(cli,  "  Board type is         : UnKnown.(0x%x)\r\n",boardType);
	    }
	}
	if(RCP_OK != (ret = Rcp_Get_Eeprom_Param(pRcpDev, RCP_EEPROM_HW_VERSION, hwversion)))
	    cli_print(cli,  "  Get hardware version failed(%d).\r\n", ret);
	else
	{
	    if(strlen(hwversion) == 0)
	            cli_print(cli,  "  Hardware version is   : NULL.\r\n");
	    else
	            cli_print(cli,  "  Hardware version is   : %s.\r\n",hwversion);
	}
	if(RCP_OK != (ret = Rcp_Get_Eeprom_Param(pRcpDev, RCP_EEPROM_SERIAL_NUMBER, serial)))
	    cli_print(cli,  "  Get manufacture serial failed(%d).\r\n", ret);
	else
	{
	    if(strlen(serial) == 0)
	            cli_print(cli,  "  Manufacture serial is : NULL.\r\n");
	    else
	            cli_print(cli,  "  Manufacture serial is : %s.\r\n",serial);
	}
	if(RCP_OK != (ret = Rcp_Get_Eeprom_Param(pRcpDev, RCP_EEPROM_MANUFACTURE_DATE, date)))
	    cli_print(cli,  "  Get manufacture failed(%d).\r\n", ret);
	else
	{
	    if(strlen(date) == 0)
	            cli_print(cli,  "  Manufacture date is   : NULL.\r\n");
	    else
	            cli_print(cli,  "  Manufacture date is   : %s.\r\n",date);
	}
	return CLI_OK;
}


cs_int32 cli_int_port_en(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32    lRet = RCP_OK;
	cs_ulong32  slot, mgtPort, ethIfIndex;
	cs_ulong32 ulPort;
	cs_uint16 enable,link;
	RCP_DEV *pRcpDev;

	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
             cli_arg_help(cli, 0,
                "1", "Enable the port",
                 NULL);
			return cli_arg_help(cli, 0,
                "0", "Disable the port",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR	
		
	if (argc == 1)
	{
		/* Show the port's status */

		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort < 0)
			{
				cli_print(cli, "%% valid input error.");
				return COMMAND_SUCCESS;
			}
			if(ulPort > (pRcpDev->numOfPorts))
			{
				cli_print(cli,   "  Input port number %ld error!", ulPort);
				return CLI_OK;
			}
			if((RCP_OK == (lRet = RCP_GetPortEnable(pRcpDev, 1,ulPort, &enable))) &&
		   		(RCP_OK == (lRet = RCP_GetPortLink(pRcpDev, 1, ulPort, &link))))
				cli_print(cli,  "  Port %ld is %s, and its physical status is %s.", ulPort,
					(enable == 0) ? "enabled" : "disabled", (link == 1) ? "UP" : "DOWN");
			else
				cli_print(cli,  "  Get port status failed.(%d)\r\n", lRet);
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	} 
	if (argc == 2)
	{	
		if(RCP_OK != (lRet = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtPort)))
			{
				cli_print(cli,  "  Get mgt port failed.(%d)", lRet);
				return CLI_OK;
			}

		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort < 0 )
			{
				cli_print(cli, "%% valid input error.");
				return COMMAND_SUCCESS;
			}
			if(ulPort == mgtPort)
			{
				cli_print(cli,  "  Cause port %ld is management port, can't be configed.", ulPort);
				return CLI_OK;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();	
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0],ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > (pRcpDev->numOfPorts))
			{
				cli_print(cli,   "  Input port number %ld error!", ulPort);
				return CLI_OK;
			}
			if (0 == strcmp(argv[1], "0"))
			{
				if(RCP_OK != (lRet = RCP_SetPortEnable(pRcpDev, 1, ulPort, 1, RCP_CONFIG_2_REGISTER)))
				{
					cli_print(cli,  "  Set port %ld disable failed.(%d)\r\n",ulPort, lRet);
					return CLI_OK;
				}
				else
					{
						cli_print(cli,  "  Set port %ld disable Successful.(%d)", ulPort,lRet);
						
					}
			}
			if(0 == strcmp(argv[1],"1"))
			{
				ethIfIndex = IFM_ETH_CREATE_INDEX(PORTNO_TO_ETH_SLOT(pRcpDev->paSlot), PORTNO_TO_ETH_PORTID(pRcpDev->paPort));
				if(RCP_OK != (lRet = RCP_SetPortEnable(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
				{
					cli_print(cli,  "  Set port %ld enable failed.(%d)", ulPort ,lRet);
					return CLI_OK;
				}
				else
					{
						cli_print(cli,  "  Set port %ld enanle Successful.(%d)", ulPort,lRet);
						
					}

			}
			if((0 !=strcmp(argv[1],"0")) && (0 != strcmp(argv[1],"1")))
				{
					cli_print(cli,"egress only 1 or 0");
					return CLI_OK;
				}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
		
		}
 	
		if(argc > 2)
			{
				cli_print(cli,"please can`t input ,max two valid");
				return CLI_OK;
			}
	return CLI_OK;
}




cs_int32 cli_int_port_fc(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32    lRet = RCP_OK;
	cs_ulong32   port,slot,mgtPort;
	cs_uint16 link,fc,configfc;
	RCP_DEV *pRcpDev;
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
            	 cli_arg_help(cli, 0,
                "1", "Enable flow-control",
                 NULL);
			return cli_arg_help(cli, 0,
                "0", "Disable the port",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
		
	GET_AND_CHECK_RCP_DEV_PTR

	if (argc == 1)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], port,pRcpDev->numOfPorts)
		{

			if(port < 0)
			{
				cli_print(cli, "%% valid input error.");
				return COMMAND_SUCCESS;
			}
			if(port > (pRcpDev->numOfPorts))
			{
				cli_print(cli,   "  Input port number %ld error!", port);
				return CLI_OK;
			}
			if(RCP_OK != (lRet = RCP_GetPortLink(pRcpDev, 1, port, &link)))
			{
				cli_print(cli,  "  Get port link status failed.(%d)", lRet);
				return CLI_OK;
			}
			if((RCP_OK == (lRet = RCP_GetPortFC(pRcpDev, 1, port, &fc)))
		   		&&(RCP_OK == (lRet = RCP_GetPortPauseFC(pRcpDev, 1, port, &configfc))))
				cli_print(cli,  "  Port %ld FC configs to %s,current FC is %s.", port,
					(configfc == 1) ? "Enabled" : "Disabled", ((fc == 1) && (link == 1)) ? "Enabled" : "Disabled");
			else
				cli_print(cli,  "  Get port status failed.(%d)", lRet);
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();

	}
	if(argc == 2)
	{
		if(RCP_OK != (lRet = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtPort)))
		{
			cli_print(cli,  "  Get mgt port failed.(%d)", lRet);
			return CLI_OK;
		}
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], port,pRcpDev->numOfPorts)
		{
			if(port < 0)
			{
				cli_print(cli, "%% valid input error.");
				return COMMAND_SUCCESS;
			}
		
			if(port == mgtPort)
			{
				cli_print(cli,  "  Cause port %ld is management port, can't be configed.",port);
				return CLI_OK;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();	
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], port,pRcpDev->numOfPorts)
		{
			
			if(port < 0)
			{
				cli_print(cli, "%% valid input error.");
				return COMMAND_SUCCESS;
			}

			if(port > (pRcpDev->numOfPorts))
			{
				cli_print(cli,   "  Input port number %ld error!", port);
				return CLI_OK;
			}
			if (0 == strcmp(argv[1], "0"))
			{
				if(RCP_OK != (lRet = RCP_SetPortPauseFC(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
				{
					cli_print(cli,  "  Set port %ld pause fc disable failed.(%d)",port, lRet);
				}
				else
					cli_print(cli,  "  Disable port %ld flow control success.", port);
			}
			else
			{
				if(RCP_OK != (lRet = RCP_SetPortPauseFC(pRcpDev, 1, port, 1, RCP_CONFIG_2_REGISTER)))
				{
					cli_print(cli,  "  Set port %ld pause fc enable failed.(%d)",port, lRet);
				}
				else
					cli_print(cli,  "  Enable port %ld flow control success.", port);
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();

	}
	if(argc > 2)
		{
			cli_print(cli, "please can`t ingress valid ,max two valid");
			return  CLI_OK;
		}
	return CLI_OK;
}



	


cs_int32 cli_int_port_link_show(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint8 lport;
	cs_uint16 status[24 + 1] = { 0 };
	RCP_DEV *pRcpDev;
	if(CLI_HELP_REQUESTED)
    {    	
         return cli_arg_help(cli, argc == 1, NULL);
	}
	GET_AND_CHECK_RCP_DEV_PTR
    cli_print(cli,  "\r\n  Port        Status     ");
    cli_print(cli,  "\r\n-------------------");
    for(lport = 1; lport <= pRcpDev->numOfPorts; lport++)
    {
        if(RCP_OK == (ret = RCP_GetPortLink(pRcpDev, 1, lport, &status[lport])))     
            cli_print(cli,  "\r\n  %02d          %s",lport,((status[lport] == 1) ? "UP" : "DOWN"));
        else
            cli_print(cli,  "  Get port %d link status failed!\r\n",lport);
    }   
      cli_print(cli,  "\r\n-------------------\r\n");

	  return CLI_OK;
}



cs_int32 cli_int_port_mode(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32  port,slot, mgtPort;
	cs_uint16 speed,configSpeed,duplex,autonego;
	cs_uint16 mode;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
             cli_arg_help(cli, 0,
                "0", "Auto Negotiate",
                 NULL);
			cli_arg_help(cli, 0,
                "8", "100M Full Duplex",
                 NULL);
			 cli_arg_help(cli, 0,
                "9", "100M Half Duplex",
                 NULL);
			 cli_arg_help(cli, 0,
                "10", "10M Full Duplex",
                 NULL);
			return cli_arg_help(cli, 0,
                "11", "10M Half Duplex",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if (argc == 1)
	{        
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], port,pRcpDev->numOfPorts)
		{
			if(port > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", port);
				return CLI_OK;
			}
			if(RCP_OK != (ret =RCP_GetPortSpeed(pRcpDev, 1, port, &speed)))
				cli_print(cli,  "  Get port speed failed.(%d)\r\n",ret);
			if(RCP_OK != (ret = RCP_GetPortSpeedConfig(pRcpDev, 1, port, &configSpeed)))
				cli_print(cli,  "  Get port config speed failed.(%d)\r\n", configSpeed);
			if(RCP_OK != (ret =RCP_GetPortDuplex(pRcpDev, 1, port, &duplex)))
				cli_print(cli,  "  Get port duplex failed.(%d)\r\n",ret);
			if(RCP_OK != (ret =RCP_GetPortAutoNego(pRcpDev, 1, port, &autonego)))
				cli_print(cli,  "  Get port auto_nego failed.(%d)\r\n",ret);
			cli_print(cli,  "  Port %ld : Auto_nego %s,Max speed %s, Current speed %s,duplex %s.\r\n",
				port,((autonego == 1)? "enabled":"disabled"),(((configSpeed == 1) || (configSpeed == 3)) ? "10M" : "100M"), 
				((speed ==0) ? "10M" : "100M"),((duplex == 1)?"full":"half"));
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	if(argc == 2)
	{
		if(RCP_OK != (ret = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtPort)))
		{
			cli_print(cli,  "  Get mgt port failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], port,pRcpDev->numOfPorts)
		{
			if(port == 0)
				{
					diag_printf("input port error\n");
					return CLI_OK;
				}
			if(port == mgtPort)
			{
				cli_print(cli,  "  Cause port %ld is management port, can't be configed.\r\n",port);
				return CLI_OK;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();	
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], port,pRcpDev->numOfPorts)
		{
			if(port == 0)
				{
					diag_printf("input port error\n");
					return CLI_OK;
				}
			if(port > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", port);
				return CLI_OK;
			}
			mode = (cs_uint16)atoi(argv[1]);
			switch(mode)
			{
				
				case 0:
					if(RCP_OK != (ret = RCP_SetPortAutoNegoEnable(pRcpDev, 1, port, 1, RCP_CONFIG_2_REGISTER)))
					{
						cli_print(cli,  "  Set port %ld auto negotiation failed.(%d)\r\n",port, ret);
					}
					else
					{
						cli_print(cli,"set port %ld auto nego success",port);
					}
					break;
				case 8:
					if(RCP_OK != (ret = RCP_SetPortMode100F(pRcpDev, 1, port, RCP_CONFIG_2_REGISTER)))
					{
						cli_print(cli,  "  Enable phy auto nego failed -->port %ld.(%d)",port, ret);
					}
					else
					{
						cli_print(cli,"set port %ld auto nego success",port);
					}
					break;
				case 9:
					if(RCP_OK != (ret = RCP_SetPortMode100H(pRcpDev, 1, port, RCP_CONFIG_2_REGISTER)))
					{
						cli_print(cli,  "  Enable phy auto nego failed -- >port %ld.(%d)",port, ret);
					}
					else
					{
						cli_print(cli,"set port %ld auto nego success",port);
					}
					break;
				case 10:
					if(RCP_OK != (ret = RCP_SetPortMode10F(pRcpDev, 1, port, RCP_CONFIG_2_REGISTER)))
					{
						cli_print(cli,  "  Enable phy auto nego failed -->port %ld.(%d)",port, ret);
					}
					else
					{
						cli_print(cli,"set port %ld auto nego success",port);
					}
					break;
				case 11:
					if(RCP_OK != (ret = RCP_SetPortMode10H(pRcpDev, 1, port, RCP_CONFIG_2_REGISTER)))
					{
						cli_print(cli,  "  Enable phy auto nego failed -->port %ld.(%d)",port, ret);
					}
					else
					{
						cli_print(cli,"set port %ld auto nego success",port);
					}
					break;
				
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();	
	}
	if(argc > 2)
		{
			cli_print(cli,"you ingress valid wrong\n");
		}
	return CLI_OK;
}




cs_int32 cli_int_port_mode_show(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort;
	cs_uint16 speed,configSpeed,duplex,autonego,link,enable;
	//cs_uint16 mode;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
    if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if (argc == 1)
	{        
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret =RCP_GetPortSpeed(pRcpDev, 1, ulPort, &speed)))
				cli_print(cli,  "  Get port speed failed.(%d)\r\n",ret);
			if(RCP_OK != (ret = RCP_GetPortSpeedConfig(pRcpDev, 1, ulPort, &configSpeed)))
				cli_print(cli,  "  Get port config speed failed.(%d)\r\n", configSpeed);
			if(RCP_OK != (ret =RCP_GetPortDuplex(pRcpDev, 1, ulPort, &duplex)))
				cli_print(cli,  "  Get port duplex failed.(%d)\r\n",ret);
			if(RCP_OK != (ret =RCP_GetPortAutoNego(pRcpDev, 1, ulPort, &autonego)))
				cli_print(cli,  "  Get port auto_nego failed.(%d)\r\n",ret);
			if(RCP_OK != (ret =RCP_GetPortLink(pRcpDev, 1, ulPort, &link)))
				cli_print(cli,  "  Get port auto_nego failed.(%d)\r\n",ret);
			if(RCP_OK != (ret =RCP_GetPortEnable(pRcpDev, 1, ulPort, &enable)))
				cli_print(cli,  "  Get port auto_nego failed.(%d)\r\n",ret);
			cli_print(cli,  "  Port %ld : Auto_nego %s,Max speed %s, Current speed %s,duplex %s.\r\n",
				ulPort,((autonego == 1)? "enabled":"disabled"),(((configSpeed == 1) || (configSpeed == 2)) ? "10M" : "100M"), 
				((speed ==0) ? "10M" : "100M"),((duplex == 1)?"full":"half"));
			cli_print(cli,  "  Physical status is %s, Adminstrator status is %s.\r\n", ((link == 1) ? "UP" : "DOWN"),
				((enable == 1) ? "DOWN" : "UP"));
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	if(argc != 1)
		{
			cli_print(cli,"you ingress valid wrong\n");
		}
	return CLI_OK;
}

cs_int32 cli_int_port_ingress_rate(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32  ulPort;
	cs_ulong32 inRate;
	RCP_RX_RATE  Rate;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
            cli_arg_help(cli, 0,
                "0", "No limit",
                 NULL);
			return cli_arg_help(cli, 0,
                "<128-8000>", "Only 128K,256K, 512K, 1M, 2M, 4M, 8Mbps are supported",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(argc == 1)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetPortRxBandwidth(pRcpDev, 1, ulPort, &Rate)))
				cli_print(cli,  "  Get port rx rate failed.(%d)\r\n", ret);
			else
			{
				cli_print(cli,  "  Port %ld ingress rate control configuration : \r\n", ulPort);
				cli_print(cli,  "        Ingress rate : %s.\r\n",(RCPPortRate[Rate].pcName));
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	if(argc == 2)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			inRate = (cs_ulong32)atol(argv[1]);
			Rate = Rcp_PortRateToEnum(inRate);
			if(Rate == -1)
			{
				cli_print(cli,"set port egress rate error  egress rate max = 8000\n");
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_SetPortRxBandwidth(pRcpDev, 1, ulPort, Rate, RCP_CONFIG_2_REGISTER)))
				cli_print(cli,  "  Set port ingress rate failed.(%d)\r\n", ret);
			else
			{
				cli_print(cli,  "  Port %ld ingress rate config success : \r\n", ulPort);
				cli_print(cli,  "        Ingress rate : %s.\r\n", (RCPPortRate[Rate].pcName));
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	if(argc > 2)
		{
		 	cli_print(cli,"you ingress valid wrong\n");
		}

	return CLI_OK;
}




cs_int32 cli_int_port_egress_rate(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32  ulPort;
	cs_ulong32 eRate;
	RCP_RX_RATE  Rate;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
            cli_arg_help(cli, 0,
                "0", "No limit",
                 NULL);
			return cli_arg_help(cli, 0,
                "<128-8000>", "Only 128K,256K,512K,1M,2M,4M,8Mbps are supported.",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(argc == 1)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				diag_printf("  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetPortTxBandwidth(pRcpDev, 1, ulPort, &Rate)))
				{
					cli_print(cli,  "  Get port tx rate failed.(%d)\r\n", ret);
				}
			else
			{
				cli_print(cli,  "  Port %ld egress rate control configuration : \r\n", ulPort);
				cli_print(cli,  "        Egress rate : %s.\r\n",(RCPPortRate[Rate].pcName));
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	if(argc == 2)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			eRate = (cs_ulong32)atoi(argv[1]);
			Rate = Rcp_PortRateToEnum(eRate);
			if(Rate == -1)
				{
					cli_print(cli,"set port egress rate error  egress rate max = 8000\n");
					return CLI_OK;
				}
			if(RCP_OK != (ret = RCP_SetPortTxBandwidth(pRcpDev, 1, ulPort, Rate, RCP_CONFIG_2_REGISTER)))
				cli_print(cli,  "  Set port egress rate failed.(%d)\r\n", ret);
			else
			{
				cli_print(cli,  "  Port %ld egress rate config success : \r\n", ulPort);
				cli_print(cli,  "        Egress rate : %s.\r\n", (RCPPortRate[Rate].pcName));
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	if(argc > 2)
		{
			cli_print(cli,"you ingress valid wrong\n");
		}
	return CLI_OK;
}


cs_int32 cli_int_port_mirror_to(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret,portonly;
	cs_ulong32 ulPort,slot,mgtPort,port;
	cs_uint16 mirrored,exit;
	RCP_DEV *pRcpDev;
    
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
             cli_arg_help(cli, 0,
                "1" ,"Enable to Mirrored Port",
                 NULL);
            return cli_arg_help(cli, 0,
                "0", "Disable to Mirrored Port",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(RCP_OK != (ret = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtPort)))
	{
		cli_print(cli,  "  Get mgt port failed.(%d)\r\n", ret);
		return CLI_OK;
	}
    if(argc == 0)
	{
		exit = 0;
		for(ulPort = 1; ulPort <= pRcpDev->numOfPorts; ulPort++)
		{
			if(RCP_OK != (ret = RCP_GetMirrorPort(pRcpDev, 1, ulPort, &mirrored)))
			{
				cli_print(cli,  "  Get mirrored port %ld failed.(%d)\r\n",ulPort, ret);
				return CLI_OK;
			}
			else
			{
				if(mirrored == 1)
				{
				      exit ++;
					cli_print(cli,  "  Mirrored Port is : %ld\r\n", ulPort);
				}
			}
		}
		if(exit == 0)
			cli_print(cli,  "  No Mirrored Port.\r\n");
	}
	if(argc == 1)
		{
			cli_print(cli,"input valid error , please port and port status\n");
			return CLI_OK;
		}
	if(argc > 2)
		{
			cli_print(cli,"valid error,max two valid\n");
		}
	if(argc == 2)
	{
	    portonly=0;
        BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
        {
            portonly++;
            if(portonly > 1)
		    {
		        cli_print(cli,  " You can only set one mirrored_to port.\r\n");
			    return CLI_OK;
		    }
            if(ulPort > pRcpDev->numOfPorts)
		    {
				cli_print(cli,   "  Input port number %ld error!\r\n", port);
				return CLI_OK;
		    }
		    if(ulPort == mgtPort)
		    {
			    cli_print(cli,  "  Cause port %ld is management port, can't be configed\r\n", ulPort);
			    return CLI_OK;
		    }
			if((0 != strcmp(argv[1], "1")) &&(0 != strcmp(argv[1], "0")))
				{
						diag_printf("		  Unknown command\n");
						return CLI_OK;
				}
		    if(0 == strcmp(argv[1], "1"))
		    {
                for(port = 1; port <= pRcpDev->numOfPorts; port++)
			    {
				    if(RCP_OK != (ret = RCP_GetMirrorPort(pRcpDev, 1, port, &exit)))
				    {
					    cli_print(cli,  "  Get mirrored port failed.(%d)\r\n", ret);
					    return CLI_OK;
				    }
				    if(exit == 1)
				    {
					    cli_print(cli,  "  Port %ld has already seted to Mirrored Port.\r\n", port);
						cli_print(cli,  "  You can only set one mirrored_to port");
						return CLI_OK;
				    }
			    }
		        if(RCP_OK != (ret = RCP_SetMirrorPort(pRcpDev, 1, ulPort, 1, RCP_CONFIG_2_REGISTER)))
	            {
			        cli_print(cli,  "  Set mirrored port failed.(%d)\r\n", ret);
			        return CLI_OK;
		        }
				else
				{
					cli_print(cli,"set mirrired port success\n");
				}
				
		    }
		    if(0 == strcmp(argv[1], "0"))
		    {
			    if(RCP_OK != (ret = RCP_SetMirrorPort(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
			    {
				    cli_print(cli,  "  Set mirrored port failed.(%d)\r\n", ret);
				    return CLI_OK;
			    }
				else
				{
					cli_print(cli,"set mirrired port success\n");
				}

		    }

		}
	    END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}


cs_int32 cli_int_port_mirror_from(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort, slot, mgtPort;
	cs_uint16 enable;
	cs_uint16 i,mirrorRx[MAX_RCP_PORT_NUM + 1],mirrorTx[MAX_RCP_PORT_NUM + 1];
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
             cli_arg_help(cli, 0,
                "i","Ingress",
                 NULL);
			 cli_arg_help(cli, 0,
                "e","Egress",
                 NULL);
			return cli_arg_help(cli, 0,
                "a","All direction",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 3:
             cli_arg_help(cli, 0,
                "0","Not as a mirror source",
                 NULL);
			return cli_arg_help(cli, 0,
                "1","As a mirror source",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 3)
	{
		if(RCP_OK != (ret = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtPort)))
		{
			cli_print(cli,  "  Get mgt port failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[1], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort == mgtPort)
			{
				cli_print(cli,  "  Cause port %ld is management port, can't be configed.\r\n",ulPort);
				return CLI_OK;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();	
		enable = (cs_uint16)atoi(argv[2]);
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[1], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if((0 == strcmp(argv[0], "i")) |(0 == strcmp(argv[0], "a")))
			{
				if(RCP_OK != (ret = RCP_SetMirrorRxPort(pRcpDev, 1, ulPort, enable, RCP_CONFIG_2_REGISTER)))
				{
					cli_print(cli,  "  Set ingress mirror failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				else
				{
					cli_print(cli,"set ingress mirror success \n");
				}
			}
			if((0 == strcmp(argv[0], "e")) |(0 == strcmp(argv[0], "a")))
			{
				if(RCP_OK != (ret = RCP_SetMirrorTxPort(pRcpDev, 1, ulPort, enable, RCP_CONFIG_2_REGISTER)))
				{
					cli_print(cli,  "  Set ingress mirror failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				else
				{
					cli_print(cli,"set ingress mirror success \n");
				}
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	else
	{
		bzero(mirrorRx, MAX_RCP_PORT_NUM +1);
		bzero(mirrorTx, MAX_RCP_PORT_NUM +1);
		for(i = 1; i <= pRcpDev->numOfPorts; i++)
		{
			if(RCP_OK != (ret = RCP_GetMirrorRxPort(pRcpDev, 1, i, &mirrorRx[i])))
			{
				cli_print(cli,  "  Get port %d ingress mirror failed.(%d)\r\n",i, ret);
				return CLI_OK;
			}
			else
			{
				cli_print(cli,"Get port %d ingress mirror success \n",i);
			}
			if(RCP_OK != (ret = RCP_GetMirrorTxPort(pRcpDev, 1, i, &mirrorTx[i])))
			{
				cli_print(cli,  "  Get port %d egress mirror failed.(%d)\r\n",i, ret);
				return CLI_OK;
			}
			else
			{
				cli_print(cli,"Get port %d egress mirror success \n",i);
			}
		}
		cli_print(cli,  "    Mirror form               direction       \r\n");
		cli_print(cli,  "--------------------------------------------------\r\n");
		for(i = 1; i <= pRcpDev->numOfPorts; i ++)
		{
			if(mirrorRx[i] == 1)
				cli_print(cli,  "%7d%30s\r\n", i, "ingress");
			if(mirrorTx[i] == 1)
				cli_print(cli,  "%7d%30s\r\n", i, "egress");
		}
		cli_print(cli,  "--------------------------------------------------\r\n");
	}
	return CLI_OK;
}


cs_int32 cli_int_port_mirror_show(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort, mirrorPort;
	cs_uint16 mirrorData;
	cs_uint16 i,mirrorRx[MAX_RCP_PORT_NUM + 1],mirrorTx[MAX_RCP_PORT_NUM + 1];
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {    	
         return cli_arg_help(cli, argc == 1, NULL);
	}
	
	mirrorPort = 0;
	bzero(mirrorRx, MAX_RCP_PORT_NUM + 1);
	bzero(mirrorTx, MAX_RCP_PORT_NUM + 1);
	for(ulPort = 1; ulPort <= pRcpDev->numOfPorts; ulPort++)
	{
		if(RCP_OK != (ret = RCP_GetMirrorRxPort(pRcpDev, 1, ulPort, &mirrorRx[ulPort])))
		{
			cli_print(cli,  "  Get ingress mirror failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		if(RCP_OK != (ret = RCP_GetMirrorTxPort(pRcpDev, 1, ulPort, &mirrorTx[ulPort])))
		{
			cli_print(cli,  "  Get egress mirror failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		if(RCP_OK != (ret = RCP_GetMirrorPort(pRcpDev, 1, ulPort, &mirrorData)))
		{
			cli_print(cli,  "  Get mirrored port failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
		{
			if(mirrorData == 1)
				mirrorPort = ulPort;
		}
	}
	cli_print(cli,  "    Mirror_form               Direction         Mirror_to\r\n");
		cli_print(cli,  "------------------------------------------------------------\r\n");
		for(i = 1; i <= pRcpDev->numOfPorts; i ++)
		{
			if(mirrorRx[i] == 1)
				cli_print(cli,  "%7d%30s%15ld\r\n", i, "ingress",mirrorPort);
			if(mirrorTx[i] == 1)
				cli_print(cli,  "%7d%30s%15ld\r\n", i, "egress",mirrorPort);
		}
		cli_print(cli,  "------------------------------------------------------------\r\n");
		return 0;
}


cs_int32 cli_int_vlan_dotlq(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 enable, en;
	cs_uint8 isolate;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "1","Enable the dot1q vlan",
                 NULL);
			return cli_arg_help(cli, 0,
                "0","Disable the dot1q vlaln", 
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(RCP_OK != (ret = RCP_GetVlan8021Q(pRcpDev, &en)))
	{
		cli_print(cli,  "  Get vlan enable failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(RCP_OK != (ret = RCP_GetVlanPortIsolate(pRcpDev, &isolate)))
	{
		cli_print(cli,  "  Get port isolate failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(argc == 0)
		cli_print(cli,  "  Dot1q vlan %s.\r\n", (en == 1) ? "enabled" : "disabled");
	else
	{
		enable = (cs_uint16)atoi(argv[0]);
		if(enable == en)
			cli_print(cli,  "  Dot1q vlan has already %s.\r\n", (en == 1) ? "enabled" : "disabled");
		else
		{
			if(RCP_OK != (ret = RCP_SetVlan8021QEnable(pRcpDev, enable, RCP_CONFIG_2_REGISTER)))
			{
				cli_print(cli,  "  Set dot1q vlan failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			cli_print(cli,  "  %s dot1q vlan SUCCESS.\r\n", (enable == 1) ? "Enable" : "Disable");
		}
	}
	return CLI_OK;
}



cs_int32 cli_int_vlan_port_isolate(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 enable,vlanen;
	cs_uint8 en;
	RCP_DEV  *pRcpDev;
	cs_ulong32 slot,mgtPort;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            	cli_arg_help(cli, 0,
                "1", "Enable isolating of ports",
                 NULL);
            return cli_arg_help(cli, 0,
                "0", "Disable isolating of ports",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(RCP_OK != (ret = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtPort)))
	{
		cli_print(cli,  "  Get mgtPort failed.\r\n");
		return CLI_OK;
	}
	if(RCP_OK != (ret = RCP_GetVlanPortIsolate(pRcpDev, &en)))
	{
		cli_print(cli,  "  Get port_isolate failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(argc == 0)
		cli_print(cli,  "  Port isolate is %s.\r\n", (en == 1) ? "enabled" : "disabled");
	else
	{
		enable = (cs_uint16)atoi(argv[0]);
		if(RCP_OK != (ret = RCP_GetVlan8021Q(pRcpDev, &vlanen)))
		{
			cli_print(cli,  "  Get vlan dot1q failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		if(vlanen == 0)
			cli_print(cli,  "  Vlan dot1q disabled.\r\n");
		else
		{
			if(enable == en)
				cli_print(cli,  "  Port isolate has already %s.\r\n", (en == 1) ? "enabled" : "disabled");
			else
			{
				if(RCP_OK != (ret = RCP_SetVlanPortIsolateEn(pRcpDev, enable, mgtPort, RCP_CONFIG_2_REGISTER)))
				{
					cli_print(cli,  "  Set vlan port_isolate failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				cli_print(cli,  "  Vlan port isolate config SUCCESS.\r\n");
			}
		}
	}
	return CLI_OK;
}


cs_int32 cli_int_vlan_dotlq_add(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 vid,isExist, vlanen;
	cs_uint16 idlenum;
	cs_uint8 en;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<2-4094>", "VLAN id\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(RCP_OK != (ret = RCP_GetVlanPortIsolate(pRcpDev, &en)))
	{
		cli_print(cli,  "  Get vlan port_isolate failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(RCP_OK != (ret = RCP_GetVlan8021Q(pRcpDev, &vlanen)))
	{
		cli_print(cli,  "  Get vlan dot1q failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(vlanen == 0)
	{
		cli_print(cli,  "  Vlan dot1q disabled.\r\n");
		return CLI_OK;
	}
	if(en == 1)
	{
		cli_print(cli,  "  Vlan port_isolate enabled.\r\n");
		return CLI_OK;
	}
	else
	{
		vid = (cs_uint16)(atoi(argv[0])); 

		if(RCP_OK != (ret = RCP_GetVlanIdleNum(pRcpDev, &idlenum)))
		{
			cli_print(cli,  "  Get idle vlanum failed!(%d)\r\n", ret);
			return CLI_OK;
		}
		else
		{
			if(idlenum >= MAX_RCP_VLAN_NUM)
			{
				cli_print(cli,  "  Vlan table has full, couldn't add any more.\r\n");
				return CLI_OK;
			}
			else
			{
				if(RCP_OK != (ret = RCP_GetVlanExist(pRcpDev, vid, &isExist)))
				{
					cli_print(cli,  "  RCP_GetVlanExist failed!(%d)\r\n",ret);
					return CLI_OK;
				}
				if(0x1 == isExist)
					cli_print(cli,  "  The vlan %d has already existed!\r\n", vid);
				else
				{
					if(RCP_OK != (ret = RCP_SetVlanVID(pRcpDev, idlenum, vid, RCP_CONFIG_2_REGISTER)))
					{
						cli_print(cli,  " add vlan failed!(%d)\r\n", ret);
						return CLI_OK;
					}
					else
					{
						cli_print(cli,"add vlan success \n");
					}
				}
			}
		}
	}
	return CLI_OK;
}

cs_int32 cli_int_vlan_dotlq_port_add(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 vid,isExist;
	cs_uint16 vlanum,i;
	cs_uint8 en;
	RCP_DEV  *pRcpDev;
	cs_ulong32 ulPort, slot, mgtPort;
	cs_uint16 tag;
	cs_uint16 exist, vlanen;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<2-4094>", "VLAN id",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)",
                 NULL);
		case 3:
            cli_arg_help(cli, 0,
                "2", "Untagged",
                 NULL);
			return cli_arg_help(cli, 0,
                "1", "Tagged",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 3, NULL);
        }
	}
	if(RCP_OK != (ret = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtPort)))
	{
		cli_print(cli,  "  Get mgt port failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(RCP_OK != (ret = RCP_GetVlanPortIsolate(pRcpDev, &en)))
	{
		cli_print(cli,  "  Get vlan port_isolate failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(RCP_OK != (ret = RCP_GetVlan8021Q(pRcpDev, &vlanen)))
	{
		cli_print(cli,  "  Get vlan dot1q failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(vlanen == 0)
	{
		cli_print(cli,  "  Dot1q vlan disabled.\r\n");
		return CLI_OK;
	}
	if(en == 1)
	{
		cli_print(cli,  "  Vlan port_isolate enabled.\r\n");
		return CLI_OK;
	}
	else
	{
		vid = (cs_uint16)(atoi(argv[0])); 
		tag = (cs_uint16)(atoi(argv[2]));
		
		if(RCP_OK != (ret = RCP_GetVlanExist(pRcpDev, vid, &isExist)))
		{
			cli_print(cli,  "  RCP_GetVlanExist failed.(%d)\r\n",ret);
			return CLI_OK;
		}
		if(0 == isExist)
		{
			cli_print(cli,  "  Vlan %d doesn't exist, pls add it.\r\n",vid);
			return CLI_OK;
		}
		else
		{
			if(tag == 2)
			{
				BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[1], ulPort,pRcpDev->numOfPorts)
				{
					if(ulPort > pRcpDev->numOfPorts)
					{
						cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
						return CLI_OK;
					}
					if(RCP_OK != (ret = RCP_GetVlanNumFromVID(pRcpDev, vid, &vlanum)))
					{
						cli_print(cli,  "'  Get vlanum from vid failed.(%d)\r\n", ret);
						return CLI_OK;
					}
					if(ulPort != mgtPort)
					{
						if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
						{
							cli_print(cli,  "  Port %ld set insert pvid to disable failed.(%d)\r\n", ulPort,ret);
							return CLI_OK;
						}
						if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, ulPort,  vid, RCP_CONFIG_2_REGISTER)))
						{ /*    change default PVID to vid cause port added by untagged mode */
							cli_print(cli,  "  Port %ld set port PVID failed.(%d)\r\n", ulPort,ret);
							return CLI_OK;
						}
					}
					if(RCP_OK != (ret = RCP_SetVlanPortAdd(pRcpDev, 1, ulPort, vlanum, RCP_CONFIG_2_REGISTER)))
					{ /*add port to the vlan */
						cli_print(cli,  "  Add vlan port %ld failed.(%d)\r\n", ulPort,ret);
						return CLI_OK;
					}
					for(i = 0; i < MAX_RCP_VLAN_NUM; i++)
					{   /*delete port in the untagged vlan */
						if(RCP_OK == (ret = RCP_GetVlanPortExist(pRcpDev, 1, ulPort, i, &exist)))
						{
							if(exist == 1 && i != vlanum)
							{
								if(RCP_OK != (ret = RCP_SetVlanPortDelete(pRcpDev, 1, ulPort, i, RCP_CONFIG_2_REGISTER)))
								{
									cli_print(cli,  "  Delete vlan port %ld failed.(%d)\r\n", ulPort,ret);
									return CLI_OK;
								}
							}
						}
						else
						{
							cli_print(cli,  "  Get vlan port %ld exist failed.(%d)\r\n", ulPort,ret);
							return CLI_OK;
						}
					}
					if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
					{  /*  remove tag when an tagged packet output from this port */
						cli_print(cli,  "  Port %ld set outport remove tag failed.(%d)\r\n", ulPort,ret);
						return CLI_OK;
					}
				}
				END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
			}
			else
			{
				BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[1], ulPort,pRcpDev->numOfPorts)
				{
					if(ulPort > pRcpDev->numOfPorts)
					{
						cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
						return CLI_OK;
					}
					if(RCP_OK != (ret = RCP_GetVlanNumFromVID(pRcpDev, vid, &vlanum)))
					{
						cli_print(cli,  "'  Get vlanum from vid failed.(%d)\r\n", ret);
						return CLI_OK;
					}
					if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, ulPort, 1, RCP_CONFIG_2_REGISTER)))
					{
						cli_print(cli,  "  Port %ld insertPVID failed.(%d)\r\n", ulPort,ret);
						return CLI_OK;
					}
					if(RCP_OK != (ret = RCP_SetVlanPortAdd(pRcpDev, 1, ulPort, vlanum, RCP_CONFIG_2_REGISTER)))
					{ /*add port to the vlan */
						cli_print(cli,  "  Add vlan port %ld failed.(%d)\r\n", ulPort,ret);
						return CLI_OK;
					}
					if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, ulPort, 3, RCP_CONFIG_2_REGISTER)))
					{  /*  don't touch when an packet output from this port */
						cli_print(cli,  "  Port %ld set outport don't touch failed.(%d)\r\n", ulPort,ret);
						return CLI_OK;
					}
				}
				END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
			}
		}
	}
	return CLI_OK;
}



cs_int32 cli_int_vlan_dotlq_del(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret,num, tagnum;
	cs_uint16 vid,vlanExist,portExist,existed,port;
	cs_uint16 output,vlanum;
	cs_uint16 pvid, vlanen;
	cs_uint8 en;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<2-4094>", "VLAN id",
                NULL);

        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(RCP_OK != (ret = RCP_GetVlanPortIsolate(pRcpDev, &en)))
	{
		cli_print(cli,  "  Get vlan port_isolate failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(RCP_OK != (ret = RCP_GetVlan8021Q(pRcpDev, &vlanen)))
	{
		cli_print(cli,  "  Get vlan port_isolate failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(vlanen == 0)
	{
		cli_print(cli,  "  Dot1q vlan disabled.\r\n");
		return CLI_OK;
	}
	if(en == 1)
	{
		cli_print(cli,  "  Vlan port_isolate enabled.\r\n");
		return CLI_OK;
	}
	else
	{
		vid = (cs_uint16)(atoi(argv[0])); 

		if(RCP_OK != (ret = RCP_GetVlanExist(pRcpDev, vid, &vlanExist)))
		{
			cli_print(cli,  "  RCP_GetVlanExist failed!(%d)\r\n",ret);
			return CLI_OK;
		}
		if(0x0 == vlanExist)
		{
			cli_print(cli,  "  The vlan %d is not existed!\r\n", vid);
			return CLI_OK;
		}
		else
		{
			if(RCP_OK == (ret = RCP_GetVlanNumFromVID(pRcpDev, vid, &vlanum)))
			{
				for(port= 1; port <= pRcpDev->numOfPorts; port++)
				{
					if(RCP_OK != (ret = RCP_GetVlanPortExist(pRcpDev, 1, port, vlanum, &portExist)))
					{
						cli_print(cli,  "  Get vlan port %d exist failed.(%d)\r\n", port,ret);
						return CLI_OK;
					}
					if(portExist == 1)
					{
						if(RCP_OK != (ret = RCP_GetVlanOutPortTag(pRcpDev, 1, port, &output)))
						{
							cli_print(cli,  "  Get port %d output tag failed.(%d)\r\n", port,ret);
							return CLI_OK;
						}
						if(output == 0)
						{
							if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, port, 1, RCP_CONFIG_2_REGISTER)))
							{/* set the untagged port's PVID to default value */
								cli_print(cli,  "  Set port %d PVID to default vlan failed.(%d)\r\n", port,ret);
								return CLI_OK;							
							}
							if(RCP_OK != (ret = RCP_SetVlanPortAdd(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
							{/* set the untagged port back to default vlan */
								cli_print(cli,  "  Set port %d back to default vlan failed.(%d)\r\n",port,ret);
								return CLI_OK;
							}
						}
						else
						{
							tagnum = 0;
							for(num = 0; num < MAX_RCP_VLAN_NUM; num++)
							{
								if(RCP_OK != (ret = RCP_GetVlanPortExist(pRcpDev, 1, port, num, &existed)))
								{
									cli_print(cli,  "  Get vlan output port %d tag failed.(%d)\r\n", port,ret);
									return CLI_OK;
								}
								else
								{
									if(existed == 1)
										tagnum ++;
								}
							}
							if(RCP_OK != (ret = RCP_GetVlanPVID(pRcpDev, 1, port, &pvid)))
							{
								cli_print(cli,  "  Get vlan port %d PVID failed.(%d)\r\n", port,ret);
								return CLI_OK;
							}
							if(((pvid == 1) && (tagnum == 2)) || (tagnum == 1))
							{
								if(port != 1)
								{
								if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, port, 1, RCP_CONFIG_2_REGISTER)))
								{/* set the tagged port's PVID to default value */
									cli_print(cli,  "  Set port %d PVID to default vlan failed.(%d)\r\n", port,ret);
									return CLI_OK;							
								}
								if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
								{/*  set tagged port's Insert PVID to Disable */
									cli_print(cli,  "  Disable port %d insert PVID failed.(%d)\r\n", port,ret);
									return CLI_OK;
								}
								if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
								{/* set the tagged port's output tagging to remove tag */
									cli_print(cli,  "  Set port %d output tag failed.(%d)\r\n", port,ret);
									return CLI_OK;
								}
								if(RCP_OK != (ret = RCP_SetVlanPortAdd(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
								{/* set the tagged port back to default vlan */
									cli_print(cli,  "  Set port %d back to default vlan failed.(%d)\r\n",port,ret);
										return CLI_OK;
								}
								}
								else
								{	if(tagnum == 2)
									{
									RCP_GetVlanPortExist(pRcpDev, 1, port, 0, &existed);
									if(existed == 1)
									{
									if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
									{/*  set tagged port's Insert PVID to Disable */
										cli_print(cli,  "  Disable port %d insert PVID failed.(%d)\r\n", port,ret);
										return CLI_OK;
									}
									if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
									{/* set the tagged port's output tagging to remove tag */
										cli_print(cli,  "  Set port %d output tag failed.(%d)\r\n", port,ret);
										return CLI_OK;
									}
									}
									}
									if(tagnum == 1)
									{
									RCP_GetVlanPortExist(pRcpDev, 1, port, 0, &existed);
									if(existed == 0)
									{
									if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
									{/*  set tagged port's Insert PVID to Disable */
										cli_print(cli,  "  Disable port %d insert PVID failed.(%d)\r\n", port,ret);
										return CLI_OK;
									}
									if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
									{/* set the tagged port's output tagging to remove tag */
										cli_print(cli,  "  Set port %d output tag failed.(%d)\r\n", port,ret);
										return CLI_OK;
									}
									if(RCP_OK != (ret = RCP_SetVlanPortAdd(pRcpDev, 1, port, 0, RCP_CONFIG_2_REGISTER)))
									{/* set the tagged port back to default vlan */
										cli_print(cli,  "  Set port %d back to default vlan failed.(%d)\r\n",port,ret);
										return CLI_OK;
									}
									}
									}
								}
							}
							if(pvid == vid)
							{
								if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, port, 1, RCP_CONFIG_2_REGISTER)))
								{
									cli_print(cli,  "  Set port %d PVID to default vlan failed.(%d)\r\n", port,ret);
									return CLI_OK;
								}
							}
						}
					}
				}
				/*Delete all the ports in the specified vlan*/
				if(RCP_OK != (ret = RCP_SetVlanPort(pRcpDev, vlanum, 0, RCP_CONFIG_2_REGISTER)) )
				{
					cli_print(cli,  "  Set vlan port list to 0 failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				if(RCP_OK != (ret = RCP_SetVlanVID(pRcpDev, vlanum, 0, RCP_CONFIG_2_REGISTER)))
				{
					cli_print(cli,  "  Set vlan vid to 0 failed.(%d)\r\n", ret);
					return CLI_OK;
				}
			}
			else
			{
				cli_print(cli,  "  Get vlanum from vid failed.(%d)\r\n", ret);
				return CLI_OK;
			}
		}
	}
	cli_print(cli,"delete success\n");
	return CLI_OK;
}


cs_int32 cli_int_vlan_dotlq_port_del(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret,tagnum,num;
	cs_uint16 vid,vlanExist,portExist,existed;
	cs_uint16 vlanum,output,pvid, vlanen;
	cs_uint8 en;
	RCP_DEV  *pRcpDev;
	cs_ulong32 ulPort;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<2-4094>", "VLAN id\n",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(RCP_OK != (ret = RCP_GetVlanPortIsolate(pRcpDev, &en)))
	{
		cli_print(cli,  "  Get vlan port_isolate failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(RCP_OK != (ret = RCP_GetVlan8021Q(pRcpDev, &vlanen)))
	{
		cli_print(cli,  "  Get vlan dot1q failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(vlanen == 0)
	{
		cli_print(cli,  "  Dot1q vlan disabled.\r\n");
		return CLI_OK;
	}
	if(en == 1)
	{
		cli_print(cli,  "  Vlan port_isolate enabled.\r\n");
		return CLI_OK;
	}
	else
	{
		vid = (cs_uint16)(atoi(argv[0])); 
	
		if(RCP_OK != (ret = RCP_GetVlanExist(pRcpDev, vid, &vlanExist)))
		{
			cli_print(cli,  "  RCP_GetVlanExist failed!(%d)\r\n",ret);
			return CLI_OK;
		}
		if(0 == vlanExist)
		{
			cli_print(cli,  "  Vlan %d doesn't exist!\r\n",vid);
			return CLI_OK;
		}
		else
		{
			if(RCP_OK == (ret = RCP_GetVlanNumFromVID(pRcpDev, vid, &vlanum)))
			{
				BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[1], ulPort,pRcpDev->numOfPorts)
				{
					if(ulPort > pRcpDev->numOfPorts)
					{
						cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
						return CLI_OK;
					}
					if(RCP_OK != (ret = RCP_GetVlanPortExist(pRcpDev, 1, ulPort, vlanum, &portExist)))
					{
						cli_print(cli,  "  Get port %ld exist failed.(%d)\r\n", ulPort,ret);
						return CLI_OK;
					}
					if(portExist == 0)
					{
						cli_print(cli,  "  Port %ld is not in vlan %d\r\n", ulPort, vid);
						return CLI_OK;
					}
					else
					{
						if(RCP_OK != (ret = RCP_GetVlanOutPortTag(pRcpDev, 1, ulPort, &output)))
						{
							cli_print(cli,  "  Get port %ld output tag failed.(%d)\r\n", ulPort,ret);
							return CLI_OK;
						}
						if(output == 0)
						{
							if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, ulPort, 1, RCP_CONFIG_2_REGISTER)))
							{/* set the untagged port's PVID to default value */
								cli_print(cli,  "  Set port %ld PVID to default vlan failed.(%d)\r\n", ulPort,ret);
								return CLI_OK;							
							}
							if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
							{/* set the untagged port's output process to default remove tag */
								cli_print(cli,  "  Set port %ld output process to default don't touch failed.(%d)\r\n", ulPort,ret);
								return CLI_OK;
							}
							if(RCP_OK != (ret = RCP_SetVlanPortAdd(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
							{/* set the untagged port back to default vlan */
								cli_print(cli,  "  Set port %ld back to default vlan failed.(%d)\r\n",ulPort,ret);
								return CLI_OK;
							}
						}
						else
						{
							tagnum = 0;
							for(num = 0; num < MAX_RCP_VLAN_NUM; num++)
							{
								if(RCP_OK != (ret = RCP_GetVlanPortExist(pRcpDev, 1, ulPort, num, &existed)))
								{
									cli_print(cli,  "  Get vlan output port %ld tag failed.(%d)\r\n", ulPort,ret);
									return CLI_OK;
								}
								else
								{
									if(existed == 1)
										tagnum ++;
								}
							}
							if(RCP_OK != (ret = RCP_GetVlanPVID(pRcpDev, 1, ulPort, &pvid)))
							{
								cli_print(cli,  "  Get vlan port %ld PVID failed.(%d)\r\n", ulPort,ret);
								return CLI_OK;
							}
							if(((pvid == 1) && (tagnum == 2)) || (tagnum == 1))
							{
								if(ulPort != 1)
								{
								if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, ulPort, 1, RCP_CONFIG_2_REGISTER)))
								{/* set the tagged port's PVID to default value */
									cli_print(cli,  "  Set port %ld PVID to default vlan failed.(%d)\r\n", ulPort,ret);
									return CLI_OK;							
								}
								if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
								{/* set the tagged port's output PVID insert to disable */
									cli_print(cli,  "  Port %ld set insert PVID failed.(%d)\r\n", ulPort,ret);
									return CLI_OK;
								}
								if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
								{/* set the tagged port's output tagging to remove tag */
									cli_print(cli,  "  Set port %ld output tag failed.(%d)\r\n", ulPort,ret);
									return CLI_OK;
								}
								if(RCP_OK != (ret = RCP_SetVlanPortAdd(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
								{/* set the tagged port back to default vlan */
									cli_print(cli,  "  Set port %ld back to default vlan failed.(%d)\r\n",ulPort,ret);
									return CLI_OK;
								}
								}
								else
								{
									if(tagnum == 2)
									{
									RCP_GetVlanPortExist(pRcpDev, 1, ulPort, 0, &existed);
									if(existed == 1)
									{
									if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
									{/*  set tagged port's Insert PVID to Disable */
										cli_print(cli,  "  Disable port %ld insert PVID failed.(%d)\r\n", ulPort,ret);
										return CLI_OK;
									}
									if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
									{/* set the tagged port's output tagging to remove tag */
										cli_print(cli,  "  Set port %ld output tag failed.(%d)\r\n", ulPort,ret);
										return CLI_OK;
									}
									}
									}
									if(tagnum == 1)
									{
									RCP_GetVlanPortExist(pRcpDev, 1, ulPort, 0, &existed);
									if(existed == 0)
									{
									if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
									{/*  set tagged port's Insert PVID to Disable */
										cli_print(cli,  "  Disable port %ld insert PVID failed.(%d)\r\n", ulPort,ret);
										return CLI_OK;
									}
									if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
									{/* set the tagged port's output tagging to remove tag */
										cli_print(cli,  "  Set port %ld output tag failed.(%d)\r\n", ulPort,ret);
										return CLI_OK;
									}
									if(RCP_OK != (ret = RCP_SetVlanPortAdd(pRcpDev, 1, ulPort, 0, RCP_CONFIG_2_REGISTER)))
									{/* set the tagged port back to default vlan */
										cli_print(cli,  "  Set port %ld back to default vlan failed.(%d)\r\n",ulPort,ret);
										return CLI_OK;
									}
									}
									}
								}
							}
							if(pvid == vid)
							{
								if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, ulPort, 1, RCP_CONFIG_2_REGISTER)))
								{
									cli_print(cli,  "  Set port %ld PVID to dufault vlan failed.(%d)\r\n", ulPort,ret);
									return CLI_OK;
								}
							}
						}
						if(RCP_OK != (ret = RCP_SetVlanPortDelete(pRcpDev, 1, ulPort, vlanum, RCP_CONFIG_2_REGISTER)))
						{
							cli_print(cli,  "  Delete port %ld failed.(%d)\r\n", ulPort,ret);
							return CLI_OK;
						}
					}
				}
				END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
			}
			else
			{
				cli_print(cli,  "  Get vlan num from vid failed!(%d)\r\n", ret);
				return CLI_OK;
			}
		}
	}
	return CLI_OK;
}


cs_int32 cli_int_vlan_dotlq_show(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret,counter;
	cs_uint16  num;
	cs_uint8 en;
	cs_ulong32 slot, ulPort, vlanport;
	cs_ulong32 port[MAX_RCP_PORT_NUM]= { 0 };
	cs_ulong32 lport[MAX_RCP_PORT_NUM] = { 0 };
	cs_uint16 output[MAX_RCP_PORT_NUM +1];
	cs_uint16 vid[MAX_RCP_VLAN_NUM] = { 0 };
	cs_uint16  i,t,un,portlist, usVid, vlanen;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "{<1-4094>}", "VLAN id\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}

	if(RCP_OK != (ret = RCP_GetVlanPortIsolate(pRcpDev, &en)))
	{
		cli_print(cli,  "  Get vlan port_isolate failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	cli_print(cli,  "  Vlan port_isolate %s.\r\n", (en == 1)?"enabled":"disabled");
	
	if(RCP_OK != (ret = RCP_GetVlan8021Q(pRcpDev, &vlanen)))
	{
		cli_print(cli,  "  Get 802.1q vlan enable failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(vlanen == 0)
	{
		cli_print(cli,  "  Vlan dot1q disabled.\r\n");
		return CLI_OK;
	}
	counter = 1;
	if(argc == 0)
	{
		num = 1;
		for(ulPort = 1;ulPort <= pRcpDev->numOfPorts;ulPort++)
		{
			if(RCP_OK != (ret = RCP_GetVlanOutPortTag(pRcpDev,  1, ulPort,  &output[ulPort])))
			{
				cli_print(cli,  "  Get port output tag failed.(%d)\r\n", ret);
				return CLI_OK;
			}
		}
		for(i = 0; i < MAX_RCP_VLAN_NUM; i++)
		{
			if(RCP_OK != (ret = RCP_GetVlanVID(pRcpDev, i, &vid[i])))
				break;
			if(RCP_OK != (ret = RCP_GetVlanPort(pRcpDev, i, &port[i])))
				break;
			
			un = t = 0;
			if(vid[i] != 0)
			{	
				cli_print(cli,  "  Vlan ID is %d", vid[i]);
				cli_print(cli,  "  Port member list : \r\n");
				for(portlist = 0;portlist < pRcpDev->numOfPorts;portlist++)
				{	
					if(0x1 == ((port[i] >> portlist) & 0x1))
					{
						pRcpDev->frcpPort2LPort(pRcpDev, &slot, &lport[portlist], 0, portlist);
						ulPort = lport[portlist];
						if(output[ulPort] == 0)
							cli_print(cli,  "    eth1/%ld(u)	", ulPort);
						else
							cli_print(cli,  "    eth1/%ld(t)	", ulPort);
						counter = counter + 1;
					}
					if(counter== 9)
					{
					    cli_print(cli,  "\r\n");
					}
				}
				//cli_print(cli,  "");
				cli_print(cli,  "\r\n");
			}
			counter = 1;
		}
	}
	else
	{
		usVid = (cs_uint16)(atoi(argv[0])); 
		if(RCP_OK != (ret = RCP_GetVlanNumFromVID(pRcpDev, usVid, &num)))
		{
			cli_print(cli,  "  Get vlan num failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		if(RCP_OK != (ret = RCP_GetVlanPort(pRcpDev, num, &vlanport))) 
		{
			cli_print(cli,  "  Get vlan portlist failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		
		for(ulPort = 1;ulPort <= pRcpDev->numOfPorts;ulPort++)
		{
			if(RCP_OK != (ret = RCP_GetVlanOutPortTag(pRcpDev,  1, ulPort,  &output[ulPort])))
			{
				cli_print(cli,  "  Get port output tag failed.(%d)\r\n", ret);
				return CLI_OK;
			}
		}
		cli_print(cli,  "  Vlan ID is %d", usVid);
		for(i = 0;i < pRcpDev->numOfPorts;i++)
		{	
			if(0x1 == ((vlanport >> i) & 0x1))
			{
				pRcpDev->frcpPort2LPort(pRcpDev, &slot, &ulPort, 0, i);
				if(output[ulPort] == 0)
					cli_print(cli,  "    eth1/%ld(u)	", ulPort);
				else
					cli_print(cli,  "    eth1/%ld(t)	", ulPort);
				counter = counter + 1;
			}
			if(counter== 9)
			{
			    cli_print(cli,  "\r\n");
		    }
		}
		counter = 1;
		//cli_print(cli,  "");
		cli_print(cli,  "\r\n");
	}
	return CLI_OK;
}



cs_int32 cli_int_vlan_pvid(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort;
	cs_uint16 pvid;
	RCP_DEV  *pRcpDev;
	
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 1)
	{/* show port's pvid*/
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetVlanPVID(pRcpDev, 1, ulPort, &pvid)))
			{
				cli_print(cli,  "  Get port PVID failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			cli_print(cli,  "  Port %02ld's PVID is : %d.\r\n", ulPort, pvid);
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}


cs_int32 cli_int_vlan_leaky(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 leaky;
	RCP_DEV  *pRcpDev;
	
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
             cli_arg_help(cli, 0,
                "arp", "Arp packet leaky",
                 NULL);
			 cli_arg_help(cli, 0,
                "unicast", "Unicast packet leaky",
                 NULL);
			return cli_arg_help(cli, 0,
                "multicast", "multicast   Multicast packet leaky",
                 NULL);
		case 2:
             cli_arg_help(cli, 0,
                "1", "enable vlan leaky",
                 NULL);
			return cli_arg_help(cli, 0,
                "0", "Disable vlan leaky",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(argc == 1)
	{
		if (0 == strcmp(argv[0], "arp"))
		{
			if(RCP_OK != (ret = RCP_GetVlanArpLeaky(pRcpDev, &leaky)))
				cli_print(cli,  "  Get vlan arp packet leaky failed.(%d)\r\n", ret);
			else
				cli_print(cli,  "  Arp packet leaky %s.\r\n",(leaky == 1) ? "enabled" : "disabled");
		}
		if (0 == strcmp(argv[0], "unicast"))
		{
			if(RCP_OK != (ret = RCP_GetVlanUnicastLeaky(pRcpDev, &leaky)))
				cli_print(cli,  "  Get vlan unicast packet leaky failed.(%d)\r\n", ret);
			else
				cli_print(cli,  "  unicast packet leaky %s.\r\n",(leaky == 1) ? "enabled" : "disabled");
		}
		if (0 == strcmp(argv[0], "multicast"))
		{
			if(RCP_OK != (ret = RCP_GetVlanMulticastLeaky(pRcpDev, &leaky)))
				cli_print(cli,  "  Get vlan multicast packet leaky failed.(%d)\r\n", ret);
			else
				cli_print(cli,  "  multicast packet leaky %s.\r\n",(leaky == 1) ? "enabled" : "disabled");
		}
	}
	else
	{
		if (0 == strcmp(argv[0], "arp"))
		{
			if(0 == strcmp(argv[1], "enable"))
			{
				if(RCP_OK != (ret = RCP_SetVlanArpLeaky(pRcpDev, 1, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Enable vlan arp packet leaky failed.(%d)\r\n", ret);
			}
			else
			{
				if(RCP_OK != (ret = RCP_SetVlanArpLeaky(pRcpDev, 0, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Disable vlan arp packet leaky failed.(%d)\r\n", ret);
			}
		}
		if (0 == strcmp(argv[0], "unicast"))
		{
			if(0 == strcmp(argv[1], "enable"))
			{
				if(RCP_OK != (ret = RCP_SetVlanUnicastLeaky(pRcpDev, 1, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Enable vlan unicast packet leaky failed.(%d)\r\n", ret);
			}
			else
			{
				if(RCP_OK != (ret = RCP_SetVlanUnicastLeaky(pRcpDev, 0, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Disable vlan unicast packet leaky failed.(%d)\r\n", ret);
			}
		}
		if (0 == strcmp(argv[0], "multicast"))
		{
			if(0 == strcmp(argv[1], "enable"))
			{
				if(RCP_OK != (ret = RCP_SetVlanMulticastLeaky(pRcpDev, 1, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Enable vlan multicast packet leaky failed.(%d)\r\n", ret);
			}
			else
			{
				if(RCP_OK != (ret = RCP_SetVlanMulticastLeaky(pRcpDev, 0, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Disable vlan unicast packet leaky failed.(%d)\r\n", ret);
			}
		}
	}
	return CLI_OK;
}


cs_int32 cli_int_vlan_ingress_filtering(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 filter;
	RCP_DEV  *pRcpDev;
	
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "1", "Enable vlan ingress filter",
                 NULL);
			return cli_arg_help(cli, 0,
                "0", "Disable vlan ingress filter",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetVlanIngressFilter(pRcpDev, &filter)))
		{
			cli_print(cli,  "  Get vlan ingress filter failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
			cli_print(cli,  "  Vlan ingress filter %s.\r\n", (filter == 1) ? "enabled" : "disabled");
	}
	else
	{
		if(0 == strcmp(argv[0], "1"))
		{
			if(RCP_OK != (ret = RCP_SetVlanIngressFilter(pRcpDev, 1, RCP_CONFIG_2_REGISTER)))
				cli_print(cli,  "  Set vlan ingress filter failed.(%d)\r\n", ret);
		}
		else
		{
			if(RCP_OK != (ret = RCP_SetVlanIngressFilter(pRcpDev, 0, RCP_CONFIG_2_REGISTER)))
				cli_print(cli,  "  Set vlan ingress filter failed.(%d)\r\n", ret);
		}	
	}
	return CLI_OK;
}



cs_int32 cli_int_vlan_acceptable_frame_types(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 admit;
	RCP_DEV  *pRcpDev;
	
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "all", "Forward untagged and tagged frames",
                 NULL);
			return cli_arg_help(cli, 0,
                "tagged", "Only forward tagged frames",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetVlanIRMember(pRcpDev, &admit)))
		{
			cli_print(cli,  "  Get vlan admit frame types failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
			cli_print(cli,  "  Accepte %s.\r\n", (admit == 0) ? "all frames (tagged and untagged)" : "only tagged frames");
	}
	else
	{
		if(0 == strcmp(argv[0], "all"))
			admit = 0;
		else
			admit = 1;
		if(RCP_OK != (ret = RCP_SetVlanIngressFilter(pRcpDev, admit, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set vlan admit frame types failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
		{
			cli_print(cli,"set vlan admit frame types success \n");
		}
	}
	return CLI_OK;
}



cs_int32 cli_int_vlan_info_show(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16  num;
	cs_ulong32 slot, ulPort;
	cs_ulong32 port[MAX_RCP_PORT_NUM]= { 0 };
	cs_ulong32 lport[MAX_RCP_PORT_NUM] = { 0 };
	cs_int8 untag[MAX_RCP_PORT_NUM * 3 +1], tag[MAX_RCP_PORT_NUM * 3 +1];
	cs_uint16 output[MAX_RCP_PORT_NUM +1];
	cs_uint16 vid[MAX_RCP_VLAN_NUM] = { 0 };
	cs_uint16  i,t,un, vlanen;
	cs_uint8 portlist;
	RCP_DEV  *pRcpDev;
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, argc > 0 , NULL);
		default:
			diag_printf("  Command incomplete.\n");
			 return CLI_OK;
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR
	if(RCP_OK != (ret = RCP_GetVlan8021Q(pRcpDev, &vlanen)))
	{
		cli_print(cli,  "  Get 802.1q vlan enable failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	if(vlanen == 0)
	{
		cli_print(cli,  "  Vlan dot1q disabled.\r\n");
		return CLI_OK;
	}
	num = 1;
	for(ulPort = 1;ulPort <= pRcpDev->numOfPorts;ulPort++)
		{
			if(RCP_OK != (ret = RCP_GetVlanOutPortTag(pRcpDev,  1, ulPort,  &output[ulPort])))
			{
				cli_print(cli,  "  Get port output tag failed.(%d)\r\n", ret);
				return CLI_OK;
			}
		}
		cli_print(cli,  "\r NO.  Vid  forward-port                                     tagged-port");
		cli_print(cli,  "\r---------------------------------------------------------------------------------");
	for(i = 0; i < MAX_RCP_VLAN_NUM; i++)
		{
			if(RCP_OK != (ret = RCP_GetVlanVID(pRcpDev, i, &vid[i])))
				break;
			if(RCP_OK != (ret = RCP_GetVlanPort(pRcpDev, i, &port[i])))
				break;
			
			un = t = 0;
			bzero(untag, MAX_RCP_PORT_NUM * 3 +1);
			bzero(tag, MAX_RCP_PORT_NUM * 3 +1);
			if(vid[i] != 0)
			{
				for(portlist = 0;portlist < pRcpDev->numOfPorts;portlist++)
				{	
					if(0x1 == ((port[i] >> portlist) & 0x1))
					{
						pRcpDev->frcpPort2LPort(pRcpDev, &slot, &lport[portlist], 0, portlist);
						ulPort = lport[portlist];
						if(output[ulPort] == 0)
						{
	                    	sprintf((untag+un*3), "%02d,", lport[portlist]);
							un++;
						}
						else
						{
	                    	sprintf((tag+t*3), "%02d,", lport[portlist]);
							t++;
						}
					}
				}

				cli_print(cli,  "\r  %-2d   %-4d            %-48s %-48s",num++,vid[i],untag,tag);

			}
		}
    cli_print(cli,  "\r---------------------------------------------------------------------------------");
	return CLI_OK;
}


cs_int32 cli_int_vlan_insert_pvid(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort;
	cs_uint16  insert;
	RCP_DEV *pRcpDev;
	
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
             cli_arg_help(cli, 0,
                "1", "enable insert pvid",
                 NULL);
			return cli_arg_help(cli, 0,
                "0", "disable insert pvid",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	
	if(argc == 1)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetVlanInsertPVID(pRcpDev, 1, ulPort, &insert)))
			{
				cli_print(cli,  "  Get vlan insert pvid failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			else
				cli_print(cli,  "  Port %ld's insert pvid is : %s.\r\n", ulPort, (insert == 1) ? "Enable" : "Disable");
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	else
	{
		if((0 != strcmp(argv[1],"1")) && (0 != strcmp(argv[1],"0")))
			{
				cli_print(cli,"input valid error");
				return CLI_OK;
			}
		if(0 == strcmp(argv[1], "1"))
			insert = 1;
		if(0 == strcmp(argv[1],"0"))
			insert = 0;
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_SetVlanInsertPVID(pRcpDev, 1, ulPort, insert, RCP_CONFIG_2_REGISTER)))
			{
				cli_print(cli,  "  Set vlan insert pvid failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			else
			{
				cli_print(cli,"set vlan insert pvid success \n");
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}


cs_int32 cli_int_vlan_output_tag(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort;
	cs_uint16 tag;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
            cli_arg_help(cli, 0,
                "1", "remove output tag",
                 NULL);
            return cli_arg_help(cli, 0,
                "0", "don't touch outpurt tag",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}

	if(argc == 1)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetVlanOutPortTag(pRcpDev, 1, ulPort, &tag)))
			{
				cli_print(cli,  "  Get vlan insert pvid failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			else
				cli_print(cli,  "  Port %ld's output tag is : %s.\r\n", ulPort, (tag == 0) ? "Remove" : "Don't touch");
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	else
	{
		if((0 != strcmp(argv[1],"1")) && (0 != strcmp(argv[1],"0")))
			{
				cli_print(cli,"input valid error\n");
				return CLI_OK;
			}
		if(0 == strcmp(argv[1], "1"))
			tag = 0;
		if(0 == strcmp(argv[1], "0"))
			tag = 3;
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, ulPort, tag, RCP_CONFIG_2_REGISTER)))
			{
				cli_print(cli,  "  Set output tag failed.(%d)\r\n", ret);
				return CLI_OK;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}



cs_int32 cli_int_stat_rx(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort, counter;
	cs_uint16 data,tmpdata;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
            cli_arg_help(cli, 0,
                "byte", "Counted by byte",
                 NULL);
			cli_arg_help(cli, 0,
                "packet", "Counted by Packet",
                 NULL);
			 cli_arg_help(cli, 0,
                "crc", "Counted CRC Packet",
                 NULL);
			return cli_arg_help(cli, 0,
                "collision", "Counted Collision Packet",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}   
	if(argc == 1 )
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetMIBRxObject(pRcpDev, 1,ulPort, &tmpdata)))
				cli_print(cli,  " Get MIB RX object failed!(%d)\r\n",ret);
			else
			{
				switch(tmpdata)
				{
				case 0 :
					cli_print(cli,  "  Port %ld current RX counter is Byte!\r\n", ulPort);
					break;
				case 1 :
					cli_print(cli,  "  Port %ld current RX counter is Packet!\r\n", ulPort);
					break;
				case 2 :
					cli_print(cli,  "  Port %ld current RX counter is CRC packet!\r\n", ulPort);
					break;
				case 3 :
					cli_print(cli,  "  Port %ld current RX counter is Collision packet!\r\n",ulPort);
					break;
				}
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	else
	{
		if(0 == strcmp(argv[1], "byte"))        
			data = 0;
		if(0 == strcmp(argv[1], "packet"))        
			data = 1;
		if(0 == strcmp(argv[1], "crc"))        
			data = 2;
		if(0 == strcmp(argv[1], "collision"))        
			data = 3;
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetMIBRxObject(pRcpDev, 1,ulPort, &tmpdata)))
			{
				cli_print(cli,  " Get MIB RX object failed!(%d)\r\n",ret);
				return CLI_OK;
			}
			if(tmpdata == data)
			{
				if(RCP_OK == (ret = RCP_GetMIBRxCounter(pRcpDev, 1, ulPort, &counter)))
					cli_print(cli,  "  RX %s : %ld\r\n",argv[1], counter);
				else
					cli_print(cli,  "  Get RX counter failed!(%d)\r\n", ret);
			}
			else
			{
				if(RCP_OK != (ret = RCP_SetMIBRxObject(pRcpDev, 1, ulPort, data, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Set Rx object failed!(%d)\r\n",ret);
				else
					cli_print(cli,  "  Change Rx Counter to %s,counter cleared to 0!\r\n",argv[1]);
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}

cs_int32 cli_int_stat_tx(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort, counter;
	cs_uint16 data,tmpdata;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR  
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
             cli_arg_help(cli, 0,
                "byte", "Counted by byte",
                 NULL);
			cli_arg_help(cli, 0,
                "packet", "Counted by Packet",
                 NULL);
			 cli_arg_help(cli, 0,
                "crc", "Counted CRC Packet",
                 NULL);
			return cli_arg_help(cli, 0,
                "collision", "Counted Collision Packet",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(argc == 1 )
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetMIBTxObject(pRcpDev, 1,ulPort, &tmpdata)))
				cli_print(cli,  " Get MIB TX object failed!(%d)\r\n",ret);
			else
			{
				switch(tmpdata)
				{
				case 0 :
					cli_print(cli,  "  Port %ld current TX counter is Byte!\r\n", ulPort);
					break;
				case 1 :
					cli_print(cli,  "  Port %ld current TX counter is Packet!\r\n", ulPort);
					break;
				case 2 :
					cli_print(cli,  "  Port %ld current TX counter is CRC packet!\r\n", ulPort);
					break;
				case 3 :
					cli_print(cli,  "  Port %ld current TX counter is Collision packet!\r\n", ulPort);
					break;
				}
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	else
	{
		if(0 == strcmp(argv[1], "byte"))        
			data = 0;
		if(0 == strcmp(argv[1], "packet"))        
			data = 1;
		if(0 == strcmp(argv[1], "crc"))        
			data = 2;
		if(0 == strcmp(argv[1], "collision"))        
			data = 3;
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetMIBTxObject(pRcpDev, 1,ulPort, &tmpdata)))
			{
				cli_print(cli,  " Get MIB TX object failed!(%d)\r\n",ret);
				return CLI_OK;
			}
			if(tmpdata == data)
			{
				if(RCP_OK == (ret = RCP_GetMIBTxCounter(pRcpDev, 1, ulPort, &counter)))
					cli_print(cli,  "  TX %s : %ld\r\n",argv[1], counter);
				else
					cli_print(cli,  "  Get TX counter failed!(%d)\r\n", ret);
			}
			else
			{
				if(RCP_OK != (ret = RCP_SetMIBTxObject(pRcpDev, 1, ulPort, data, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Set TX object failed!(%d)\r\n",ret);
				else
					cli_print(cli,  "  Change TX Counter to %s,counter cleared to 0!\r\n",argv[1]);
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}


cs_int32 cli_int_stat_diag(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort, counter;
	cs_uint16 data,tmpdata;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR  
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
			cli_arg_help(cli, 0,
                "byte", "Counted by byte",
                 NULL);
			cli_arg_help(cli, 0,
                "packet", "Counted by Packet",
                 NULL);
			 cli_arg_help(cli, 0,
                "crc", "Counted CRC Packet",
                 NULL);
			return cli_arg_help(cli, 0,
                "collision", "Counted Collision Packet",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 2, NULL);
        }
	}
	if(argc == 1 )
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetMIBDnObject(pRcpDev, 1,ulPort, &tmpdata)))
				cli_print(cli,  " Get MIB Diag object failed!(%d)\r\n",ret);
			else
			{
				switch(tmpdata)
				{
				case 0 :
					cli_print(cli,  "  Port %ld current Diag counter is Byte!\r\n", ulPort);
					break;
				case 1 :
					cli_print(cli,  "  Port %ld current Diag counter is Packet!\r\n", ulPort);
					break;
				case 2 :
					cli_print(cli,  "  Port %ld current Diag counter is CRC packet!\r\n", ulPort);
					break;
				case 3 :
					cli_print(cli,  "  Port %ld current Diag counter is Collision packet!\r\n", ulPort);
					break;
				}
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	else
	{
		if(0 == strcmp(argv[1], "byte"))        
			data = 0;
		if(0 == strcmp(argv[1], "packet"))        
			data = 1;
		if(0 == strcmp(argv[1], "crc"))        
			data = 2;
		if(0 == strcmp(argv[1], "collision"))        
			data = 3;
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetMIBDnObject(pRcpDev, 1,ulPort, &tmpdata)))
			{
				cli_print(cli,  " Get MIB Diag object failed!(%d)\r\n",ret);
				return CLI_OK;
			}
			if(tmpdata == data)
			{
				if(RCP_OK == (ret = RCP_GetMIBDiagCounter(pRcpDev, 1, ulPort, &counter)))
					cli_print(cli,  "  Diag %s : %ld\r\n",argv[1], counter);
				else
					cli_print(cli,  "  Get Diag counter failed!(%d)\r\n", ret);
			}
			else
			{
				if(RCP_OK != (ret = RCP_SetMIBDnObject(pRcpDev, 1, ulPort, data, RCP_CONFIG_2_REGISTER)))
					cli_print(cli,  "  Set Diag object failed!(%d)\r\n",ret);
				else
					cli_print(cli,  "  Change Diag Counter to %s,counter cleared to 0!\r\n",argv[1]);
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}



cs_int32 cli_int_broadcast_storm_filtering(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 data;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR  
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "enable", "Enable broadcast storm filtering",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable broadcast storm filtering",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetBroadcastStormFilter(pRcpDev, &data)))
		{
			cli_print(cli,  "  Get broadcast storm filtering failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		cli_print(cli,  "  Broadcast Storm Filtering is %s.\r\n", (data == 0) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0],  "enable"))
			data = 0;
		else
			data = 1;
		if(RCP_OK != (ret = RCP_SetBroadcastStormFilter(pRcpDev, data, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set broadcast strom filtering failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}

cs_int32 cli_int_broadcast_fc(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 data;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR  
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "enable", "Enable braodcast flow control",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable broadcast flow control",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetBroadcastFC(pRcpDev, &data)))
		{
			cli_print(cli,  "  Get broadcast flow control failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		cli_print(cli,  "  Broadcast Flow Control is %s.\r\n", (data == 1) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0],  "enable"))
			data = 1;
		else 
			data = 0;
		
		if(RCP_OK != (ret = RCP_SetBroadcastFC(pRcpDev, data, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set broadcast flow control failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}



cs_int32 cli_int_igmpsnooping(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 data;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR  
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "enable", "Enable igmpsnooping",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable igmpsnooping",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetIGMPSnooping(pRcpDev, &data)))
		{
			cli_print(cli,  "  Get igmpsnooping failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		cli_print(cli,  "  IGMPsnooping is : %s.\r\n", (data == 1) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0],  "enable"))
			data = 1;
		else
			data = 0;
		if(RCP_OK != (ret = RCP_SetIGMPSnooping(pRcpDev, data, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set igmpsnooping failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}



cs_int32 cli_int_multicast_fc(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 data;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR  
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "enable", "Enable multicast flow control",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable multicast flow control",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetMulticastFC(pRcpDev, &data)))
		{
			cli_print(cli,  "  Get multicast flow control failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		cli_print(cli,  "  Multicast Flow Control is %s.\r\n", (data == 1) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0],  "enable"))
			data = 1;
		else
			data = 0;
		
		if(RCP_OK != (ret = RCP_SetMulticastFC(pRcpDev, data, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set multicast flow control failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}


cs_int32 cli_int_qos_dscp_pri_en(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 dscp;
	RCP_DEV *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "enable", "Enable differver priority",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable differver priority",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetQosDiffPriority(pRcpDev, &dscp)))
		{
			cli_print(cli,  "  Get qos diffserver failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
			cli_print(cli,  "  Diffserver Priority is : %s\r\n",(dscp == 1) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0], "enable"))
		{
			if(RCP_OK != (ret = RCP_SetQosMapMode(pRcpDev, 0, RCP_CONFIG_2_REGISTER)))
			{
				cli_print(cli,  "  Set Qos mode failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			dscp = 1;
		}
		else
			dscp = 0;
		
		if(RCP_OK != (ret = RCP_SetQosDiffPriority(pRcpDev, dscp, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set qos diffserver failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}

cs_int32 cli_int_qos_user_pri_en(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 user;
	RCP_DEV *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            cli_arg_help(cli, 0,
                "enable", "Enable 802.1p priority",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable 802.1p priority",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetQosVlanPriority(pRcpDev, &user)))
		{
			cli_print(cli,  "  Get qos dot1p priority failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
			cli_print(cli,  "  Dot1p Priority is : %s\r\n",(user == 1) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0], "enable"))
		{
			if(RCP_OK != (ret = RCP_SetQosMapMode(pRcpDev, 0, RCP_CONFIG_2_REGISTER)))
			{
				cli_print(cli,  "  Set Qos mode failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			user = 1;
		}
		else
			user = 0;
		if(RCP_OK != (ret = RCP_SetQosVlanPriority(pRcpDev, user, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set qos dot1p priority failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}


cs_int32 cli_int_qos_adapted_fc(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 adapted;
	RCP_DEV *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
             cli_arg_help(cli, 0,
                "enable", "Enable adapted flow control",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable adapted flow control",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetQosAdapterFC(pRcpDev, &adapted)))
		{
			cli_print(cli,  "  Get qos adapted fc failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
			cli_print(cli,  "  Adated flow control is : %s\r\n",(adapted == 1) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0], "enable"))
			adapted = 1;
		else
			adapted = 0;
		
		if(RCP_OK != (ret = RCP_SetQosAdapterFC(pRcpDev, adapted, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set qos adapted fc failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}


cs_int32 cli_int_qos_priority_ratio(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 ratio;
	RCP_DEV *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:

			cli_arg_help(cli, 0,
                "4:1", "Frame service rate of High_pri queue to Low_pri queue is 4:1",
                 NULL);
			cli_arg_help(cli, 0,
                "8:1", "Frame service rate of High_pri queue to Low_pri queue is 8:1",
                 NULL);
			cli_arg_help(cli, 0,
                "16:1", "Frame service rate of High_pri queue to Low_pri queue is 16:1",
                 NULL);
			return cli_arg_help(cli, 0,
                "rhigh_pri_first", "Frame service rate is High_priority always",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetQosPrioRatio(pRcpDev, &ratio)))
		{
			cli_print(cli,  "  Get qos adapted fc failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
		{
			switch(ratio)
			{
				case 0:
					cli_print(cli,  "  Frame service rate of High_pri queue to Low_pri queue is 4:1.\r\n");
					break;
				case 1:
					cli_print(cli,  "  Frame service rate of High_pri queue to Low_pri queue is 8:1.\r\n");
					break;
				case 2:
					cli_print(cli,  "  Frame service rate of High_pri queue to Low_pri queue is 16:1.\r\n");
					break;
				case 3:
					cli_print(cli,  "  Frame service rate is High priority always.\r\n");
					break;
			}
		}
	}
	else
	{
		
		if(0 == strcmp(argv[0], "4:1"))
			ratio = 0;
		else if(0 == strcmp(argv[0], "8:1"))
			ratio = 1;
		else if(0 == strcmp(argv[0], "16:1"))
			ratio = 2;
		else if(0 == strcmp(argv[0], "high_pri_first"))
			ratio = 3;
		
		if(RCP_OK != (ret = RCP_SetQosPrioRatio(pRcpDev, ratio, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set qos priority ratio failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}



cs_int32 cli_int_qos_port_priority(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 priority;
	cs_ulong32 ulPort, slot, mgtPort;
	RCP_DEV *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
            cli_arg_help(cli, 0,
                "low", "Set port's priority to low",
                 NULL);
			return cli_arg_help(cli, 0,
                "high", "Set port's priority to high",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 1)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetQosPortPriority(pRcpDev, 1, ulPort, &priority)))
			{
				cli_print(cli,  "  Get qos port's priority failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			else
				cli_print(cli,  "  Port %ld's priority is : %s.\r\n", ulPort, (priority == 1) ? "High Priority" : "Low Priority");
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	else
	{
		if(RCP_OK != (ret = Rcp_Get_MGT_Port(pRcpDev, &slot, &mgtPort)))
		{
			cli_print(cli,  "  Get mgt port failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort == mgtPort)
			{
				cli_print(cli,  "  Cause port %ld is management port, can't be configed.\r\n",ulPort);
				return CLI_OK;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(0 == strcmp(argv[1], "high"))
				priority = 1;
			else
				priority = 0;
			if(RCP_OK != (ret = RCP_SetQosPortPriority(pRcpDev, 1, ulPort, priority, RCP_CONFIG_2_REGISTER)))
			{
				cli_print(cli,  "  Set qos port's priority failed.(%d)\r\n", ret);
				return CLI_OK;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}


cs_int32 cli_int_qos_priority_type(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 type;
	RCP_DEV *pRcpDev;
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, argc > 0 , NULL);
		default:
			diag_printf("  Command incomplete.\n");
			 return CLI_OK;
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR
	if(RCP_OK != (ret = RCP_GetQosMapMode(pRcpDev, &type)))
	{
		cli_print(cli,  "  Get qos mode failed.(%d)\r\n",ret);
		return CLI_OK;
	}
	if(type == 0)
		cli_print(cli,  "  Current QoS priority type is : Port-based.\r\n");
	else if(type == 1)
		cli_print(cli,  "  Current QoS priority type is : DSCP-based.\r\n");
	else if(type == 2)
		cli_print(cli,  "  Current QoS priority type is : 802.1p-based.\r\n");
	return CLI_OK;
}


cs_int32 cli_int_loopback_auto_detect(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 loop;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            	 cli_arg_help(cli, 0,
                "enable", "Enable the loopback auto detect",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable the loopback auto detect",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetLoopDetect(pRcpDev, &loop)))
		{
			cli_print(cli,  "  Get loop detect failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		else
			cli_print(cli,  "  Loop detect is %s.\r\n", (loop == 1) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0], "enable"))
			loop = 1;
		else
			loop = 0;
		if(RCP_OK != (ret = RCP_SetLoopDetect(pRcpDev, loop, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set loop detect failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}


cs_int32 cli_int_atu_learning(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort;
	cs_uint16 learning,enable;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR

	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "{<port_list>}*1", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
		case 2:
            	 cli_arg_help(cli, 0,
                "1", "Enalbe",
                 NULL);
			 return cli_arg_help(cli, 0,
                "0", "Disable",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	
	if(argc == 0)
	{
		for(ulPort = 1; ulPort <= pRcpDev->numOfPorts; ulPort ++)
		{
			if(RCP_OK != (ret = RCP_GetAtuLearning(pRcpDev, 1, ulPort, &learning)))
			{
				cli_print(cli,  "  Get ATU learning failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			else
				cli_print(cli,  "  Port %ld ATU learning is : %s.\r\n", ulPort, (learning == 1) ? "Disabled" : "Enabled");
		}
	}
	else if(argc == 1)
	{
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_GetAtuLearning(pRcpDev, 1, ulPort, &learning)))
			{
				cli_print(cli,  "  Get ATU learning failed.(%d)\r\n", ret);
				return CLI_OK;
			}
			else
				cli_print(cli,  "  Port %ld ATU learning is : %s.\r\n", ulPort, (learning == 1) ? "Disabled" : "Enabled");
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	else if(argc == 2)
	{
		enable = (cs_uint16) atoi(argv[1]);
		if(enable == 1)
			learning = 0;
		else
			learning = 1;
		BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
		{
			if(ulPort > pRcpDev->numOfPorts)
			{
				cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
				return CLI_OK;
			}
			if(RCP_OK != (ret = RCP_SetAtuLearning(pRcpDev, 1, ulPort, learning, RCP_CONFIG_2_REGISTER)))
			{
				cli_print(cli,  "  Set ATU learning failed.(%d)\r\n", ret);
				return CLI_OK;
			}
		}
		END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	}
	return CLI_OK;
}


cs_int32 cli_int_atu_aging(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16  aging, fast;
	//cs_uint16  enable;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
             cli_arg_help(cli, 0,
                "default", "Default atu aging time : 300s",
                 NULL);
			 cli_arg_help(cli, 0,
                "fastaging", "Fast atu aging time : 12s",
                 NULL);
			 return cli_arg_help(cli, 0,
                "disable", "Disable atu table aging",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}

	if(argc == 0)
	{
		if(RCP_OK!= (ret = RCP_GetAtuAgingEnable(pRcpDev, &aging)))
		{
			cli_print(cli,  "  get aut aging enable failed.(%d)\r\n",ret);
			return CLI_OK;
		}
	        else
	        {
	        	if(aging == 1)
	        	{
	        		cli_print(cli,  "  Mac table aging is : Disabled.\r\n");
	        		return CLI_OK;
	        	}
	        	else
	        	{
	        		if(RCP_OK != (ret = RCP_GetAtuFastAgingEnable(pRcpDev, &aging)))
	        		{
	        			cli_print(cli,  "  get atu fast aging enable failed.(%d)\r\n",ret);
	        			return CLI_OK;
	        			}
	        		else
	        		{
					if(aging == 0)
						cli_print(cli,  "  Mac table aging time is default 300s\r\n");
					else if(aging == 1)
						cli_print(cli,  "  Mac table aging time is fast 12s\r\n");
				}
			}
		}
	}
	else
	{
		if(0 == strcmp(argv[0], "disable"))
		{
			aging = 1;
			if(RCP_OK != (ret = RCP_SetAtuAgingEnable(pRcpDev, aging, RCP_CONFIG_2_REGISTER)))
			{
				cli_print(cli,  "set atu aging enable failed.(%d)\r\n",ret);
				return CLI_OK;
			}
		}
		else if(0 == strcmp(argv[0], "default"))
		{
			fast = 0;
			if(RCP_OK != RCP_SetAtuAgingEnable(pRcpDev, 0, RCP_CONFIG_2_REGISTER))
			{
				cli_print(cli,  "set atu aging enable failed.(%d)\r\n",ret);
				return CLI_OK;
	          	}
	          	if(RCP_OK != RCP_SetAtuFastAgingEnable(pRcpDev, fast, RCP_CONFIG_2_REGISTER))
	          	{
				cli_print(cli,  "set atu fast aging enable failed.(%d)\r\n",ret);
				return CLI_OK;
	          	}
		}
		else
		{
			fast = 1;
			if(RCP_OK != RCP_SetAtuAgingEnable(pRcpDev, 0, RCP_CONFIG_2_REGISTER))
			{
				cli_print(cli,  "set atu aging enable failed.(%d)\r\n",ret);
				return CLI_OK;
	          	}
	          	if(RCP_OK != RCP_SetAtuFastAgingEnable(pRcpDev, fast, RCP_CONFIG_2_REGISTER))
	          	{
				cli_print(cli,  "set atu fast aging enable failed.(%d)\r\n",ret);
				return CLI_OK;
	          	}		
	       }
	}
	return CLI_OK;
}

cs_int32 cli_int_atu_filter(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_uint16 data;
	RCP_DEV  *pRcpDev;
	GET_AND_CHECK_RCP_DEV_PTR  
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
           		cli_arg_help(cli, 0,
                "enable", "Enable multicast control packet filter",
                 NULL);
			return cli_arg_help(cli, 0,
                "disable", "Disable multicast control packet filter",
                 NULL);

        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}

	if(argc == 0)
	{
		if(RCP_OK != (ret = RCP_GetAtuCtrlFrameFilter(pRcpDev, &data)))
		{
			cli_print(cli,  "  Get atu ctrl frame control failed.(%d)\r\n", ret);
			return CLI_OK;
		}
		cli_print(cli,  "  Control Frame filter  is : %s.\r\n", (data == 1) ? "Enabled" : "Disabled");
	}
	else
	{
		if(0 == strcmp(argv[0],  "enable"))
			data = 1;
		else 
			data = 0;
		if(RCP_OK != (ret = RCP_SetAtuCtrlFrameFilter(pRcpDev, data, RCP_CONFIG_2_REGISTER)))
		{
			cli_print(cli,  "  Set atu ctrl frame control failed.(%d)\r\n", ret);
			return CLI_OK;
		}
	}
	return CLI_OK;
}




cs_int32 cli_int_cable_test_port(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	cs_ulong32 ulPort,unit,phyPort;
	cs_uint16 link,testPort,txtest,rxtest,txdistance,rxdistance;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<port_list>", "Specify interface's port list(e.g.: 1; 1,2; 1-8)\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}

	BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(argv[0], ulPort,pRcpDev->numOfPorts)
	{
		if(ulPort > pRcpDev->numOfPorts)
		{
			cli_print(cli,   "  Input port number %ld error!\r\n", ulPort);
			return CLI_OK;
		}
		testPort = 0;
		if(RCP_OK == (ret = RCP_GetPortLink(pRcpDev, 1, ulPort, &link)))
		{
			if(link == 1)
			{
				cli_print(cli,  "  Port %ld cable test result:\r\n", ulPort);
				cli_print(cli,  "    RX PAIR : \r\n");
				cli_print(cli,  "      Normal cable.\r\n");
				cli_print(cli,  "  Port %ld cable test result:\r\n", ulPort);
				cli_print(cli,  "    TX PAIR : \r\n");
				cli_print(cli,  "      Normal cable.\r\n");
			}
			else
			{
				if(RCP_OK != (ret = pRcpDev->frcpLPort2Port(pRcpDev, 1, ulPort, &unit, &phyPort)))
				{
					cli_print(cli,  "  Get phyPort failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				testPort  = 0x7fa | (phyPort << 13);
				if(phyPort >= 8)
				{
				    pRcpDev->phyAddr = 24;
				}
				else
				{
				    pRcpDev->phyAddr = 16;
				}
				if(RCP_OK != (ret = RCP_SetPhyRegValue(pRcpDev, 1, 2, 24, testPort)))
				{
					cli_print(cli,  "  Set test rx failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				if(RCP_OK != (ret = RCP_SetPhyRegField(pRcpDev, 1, 2, 29, 14, 1, 0)))
				{
					cli_print(cli,  "  Set test rx dir failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				if(RCP_OK != (ret = RCP_SetPhyRegField(pRcpDev, 1, 2, 24, 11, 1, 1)))
				{
					cli_print(cli,  "  Set test rx start dir failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				do
				{
					if(RCP_OK != (ret = RCP_GetPhyRegValue(pRcpDev, 1, 2, 30, &rxtest)))
					{
						cli_print(cli,  "  Get test rx value failed.(%d)\r\n", ret);
						return CLI_OK;
					}
				}while(0x8000 != (rxtest & 0x8000));
				if(0x8000 == (rxtest & 0x8000))
				{
					cli_print(cli,  "  Port %ld cable test result:\r\n", ulPort);
					cli_print(cli,  "    RX PAIR : \r\n");
					if(0x1000 == (rxtest & 0x3000))
					{
						rxdistance = (rxtest & 0x1ff)/4;
						cli_print(cli,  "      OPEN in %d M, device power down or bad cable.\r\n", rxdistance);
					}
					if(0x2000 == (txtest & 0x3000))
					{
						rxdistance = (rxtest & 0x1ff)/4;
						cli_print(cli,  "      SHORT in %d M, device power down or bad cable.\r\n", rxdistance);
					}
				}
				testPort = 0;
				testPort  = 0x7fa | (phyPort << 13);
				if(RCP_OK != (ret = RCP_SetPhyRegValue(pRcpDev, 1,2, 24, testPort)))
				{
					cli_print(cli,  "  Set test tx failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				if(RCP_OK != (ret = RCP_SetPhyRegField(pRcpDev, 1, 2, 29, 14, 1, 1)))
				{
					cli_print(cli,  "  Set test tx dir failed.(%d)\r\n", ret);
					return CLI_OK;
				}
				if(RCP_OK != (ret = RCP_SetPhyRegField(pRcpDev, 1, 2, 24, 11, 1, 1)))
				{
					cli_print(cli,  "  Set test tx start failed.(%d)\r\n", ret);
				}
				do
				{
					if(RCP_OK != (ret = RCP_GetPhyRegValue(pRcpDev, 1, 2, 30, &txtest)))
					{
						cli_print(cli,  "  Get test tx value failed.(%d)\r\n", ret);
						return CLI_OK;
					}
				}while(0x8000 != (txtest & 0x8000));
				if(0x8000 == (txtest & 0x8000))
				{
					cli_print(cli,  "  Port %ld cable test result:\r\n", ulPort);
					cli_print(cli,  "    TX PAIR : \r\n");
					if(0x1000 == (txtest & 0x3000))
					{
						txdistance = (txtest & 0x1ff)/4;
						cli_print(cli,  "      OPEN in %d M, device power down or bad cable.\r\n", txdistance);
					}
					if(0x2000 == (txtest & 0x3000))
					{
						txdistance = (txtest & 0x1ff)/4;
						cli_print(cli,  "      SHORT in %d M, device power down or bad cable.\r\n", txdistance);
					}
				}
			}
		}
		else
		{
			cli_print(cli,  "  Get port link failed.(%d)\r\n", ret);
			return CLI_OK;
		}	
	}
	END_PARSE_PORT_LIST_TO_PORT_NO_CHECK();
	return CLI_OK;
}


cs_int32 cli_int_mgt_reset(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	RCP_DEV *pRcpDev;
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, argc > 0 , NULL);
		default:
			diag_printf("  Command incomplete.\n");
			 return CLI_OK;
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR

	if(RCP_OK != (ret = RCP_SysHWReset(pRcpDev)))
	{
		cli_print(cli,  "  Set software reset failed.(%d)\r\n", ret);
		return CLI_OK;
	}
	return CLI_OK;
}

cs_int32 cli_int_mgt_config_clear(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret,counter;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
    if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "[all]", "To all devices\n",
                 NULL);

        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
    counter=0;
    ret = RCP_OK;
	cli_print(cli, "%s  Loading factory configuration to %s switch, please wait....", VTY_NEWLINE, (argc == 1)?argv[0]:"this");
    do{
		if(argc == 1)
		{
			if(strcmp(argv[0], "all") == 0)
			{
				ret = ereaseRcpDevCfgInFlash(pRcpDev, 1);
			}
		}
		else
		{
			ret = RCP_SetConfigClear(pRcpDev);
		}
		counter++;
	}while(ret != RCP_OK && counter < 3);
	
	cli_print(cli, "%s  Loading factory configuration successed.",VTY_NEWLINE);
	return CLI_OK;
}


cs_int32 cli_int_mgt_config_save(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 iRet;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "all", "To all devices\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 1)
	{
		if(strcmp(argv[0], "all") == 0)
		{
			iRet = saveAllRcpDevCfgToFlash();
		}
	}
	else
	{
		iRet = saveOneRcpDevCfgToFlash(pRcpDev);
	}
	cli_print(cli, "%s%s", (0 == iRet)?"SUCCESS.":"FAILED!!!", VTY_NEWLINE );

	return CLI_OK;
}


cs_int32 cli_int_show_loop_port(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ret;
	RCP_DEV *pRcpDev;
	cs_ulong32 portlist;
	cs_ulong32 slot, port,phyPort;
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, argc > 0 , NULL);
		default:
			diag_printf("  Command incomplete.\n");
			 return CLI_OK;
        }
	}
	GET_AND_CHECK_RCP_DEV_PTR
	
#if 1
	if(RCP_OK == (ret = RCP_GetLoopPort(pRcpDev, &portlist)))
	{
		if(portlist != 0)
		{
			for(phyPort = 0; phyPort < pRcpDev->numOfPorts; phyPort++)
			{
				if(((portlist & (0x1 << phyPort)) >> phyPort) == 1)
				{
					pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, phyPort);
					cli_print(cli,  "%%Loopback detect on port %ld.\r\n", port);
				}
			}
		}
		else
			cli_print(cli,  "%%No loopback port detected.\r\n");
	}
	else
		cli_print(cli,  "  Get loopback port failed(%d).\r\n", ret);
#endif
	if(pRcpDev->loopAndDown != 0)
	{
		for(phyPort = 0; phyPort < pRcpDev->numOfPorts; phyPort++)
		{
			if(((pRcpDev->loopAndDown & (0x1 << phyPort)) >> phyPort) == 1)
			{
				pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, phyPort);
				cli_print(cli,  "%% port %ld marked looped and has been disabled .\r\n", port);
			}
		}
	}
	else
		cli_print(cli,  "%%No loopback port .\r\n");		
	return CLI_OK;
}

#if 0
cs_int32 cli_int_show_running_config(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	
	cs_uint16  mode;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR
	rcp_device_mgt_showrun(vty, RCP_SHOWRUN_SOURCE_SWITCH);

	return CLI_OK;
}


cs_int32 cli_int_show_startup_config(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 i, ret;
	cs_uint16 data, mode;
	RCP_DEV *pRcpDev;

	GET_AND_CHECK_RCP_DEV_PTR_FLASH

	if(0 == IS_Flash_Data_Valid(pRcpDev))
	{
		cli_print(cli,  "  No config file in flash! \r\n");
	}
	else
	{
		rcp_device_mgt_showrun(vty, RCP_SHOWRUN_SOURCE_FLASH);
	}

	return CLI_OK;
}

#endif
cs_int32 cli_int_mask_alarm_switch(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_uint8 ucMask;
	cs_int32 i;
	RCP_DEV *pRcpDev;

	extern cs_uint8 gucRcpAlarmMask;

	GET_AND_CHECK_RCP_DEV_PTR
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
          	 cli_arg_help(cli, 0,
                "all", "Mask switch status chage alarm",
                 NULL);
		   return cli_arg_help(cli, 0,
                "this", "Unmask switch status chage alarm",
                 NULL);
		case 2:
             cli_arg_help(cli, 0,
                "enable", "",
                 NULL);
		     return cli_arg_help(cli, 0,
                "disable", "",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}

	if (argc == 2) /* set */
	{
    	if(strcmp(argv[1], "enable") == 0)
    		ucMask = RCP_ALARM_STATUS_OFFLINE | RCP_ALARM_STATUS_REGISTER | RCP_ALARM_STATUS_RE_REGISTER;
    	else
    		ucMask = 0;

    	if(strcmp(argv[0], "all") == 0)
    	{
    		gucRcpAlarmMask = ucMask;
			for(i=1; i<MAX_RRCP_SWITCH_TO_MANAGE; i++)
			{
				if(RCP_Dev_Is_Valid(i))
				{
					pRcpDev->alarmMask = gucRcpAlarmMask;
				}
			}
    	}
    	else
    	{
    		pRcpDev->alarmMask = ucMask;
    	}
	}

	cli_print(cli,  "  Swtich status change alarm mask config for all  : %s.\r\n", (0 == gucRcpAlarmMask) ? "NOT masked" : "Masked");
	cli_print(cli,  "  Swtich status change alarm mask config for this : %s.\r\n", (0 == gucRcpAlarmMask) ? "NOT masked" : "Masked");

	return CLI_OK;
}




cs_int32 cli_int_set_switch(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
             cli_arg_help(cli, 0,
                "1", "Enable RCP switch mgt",
                 NULL);
			 return cli_arg_help(cli, 0,
                "0", "Disable RCP switch mgt",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
    if (argc == 0)
     cli_print(cli,  "  RCP switch mgt is %s\r\n", gulEnableEpswitchMgt ? "Enabled" : "Disabled");
    else
      {
          if (0 == strcmp(argv[0], "0"))
              gulEnableEpswitchMgt = 0;
          if (0 == strcmp(argv[0], "1"))
              gulEnableEpswitchMgt = 1;
      }
	return CLI_OK;
}
#if 0
#endif
cs_int32 cmd_switch_show(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_int32 ulIndex, status;
	cs_ulong32 onuPort;
	cs_ulong32 slot, mgtPort;
	cs_int32 counter = 0;
	
	RCP_Say_Hello(-1, 1);
	cyg_thread_delay(IROS_TICK_PER_SECOND / 10);
	for(onuPort = 1; onuPort < gulNumOfPortsPerSystem; onuPort++)
	{
		if(RCP_Dev_Is_Valid(onuPort))
			counter++;
	}
	if(counter == 0)
		cli_print(cli,  "  No RCP switch linked.\r\n");
	else
	{
		cli_print(cli, "  Total RCP Swtich : %d", counter);
		cli_print(cli, "  NO.    Location     MgtPort      MAC(ports)          Status");
		cli_print(cli, "---------------------------------------------------------------");
		ulIndex = 1;
		for(onuPort = 1; onuPort < gulNumOfPortsPerSystem; onuPort++)
		{
			if(1 == RCP_Dev_Is_Exist(onuPort))
				status = 1;
			else
				status = 0;
			if(RCP_Dev_Is_Valid(onuPort))
			{
				rcpDevList[onuPort]->frcpPort2LPort(rcpDevList[onuPort], &slot, &mgtPort, 0, rcpDevList[onuPort]->upLinkPort);
				cli_print(cli,  "   %d      eth1/%lu       eth%lu       %02x%02x.%02x%02x.%02x%02x(%02d)    %s", ulIndex,onuPort, mgtPort,
					rcpDevList[onuPort]->switchMac[0], rcpDevList[onuPort]->switchMac[1], rcpDevList[onuPort]->switchMac[2], 
					rcpDevList[onuPort]->switchMac[3], rcpDevList[onuPort]->switchMac[4], rcpDevList[onuPort]->switchMac[5],
					rcpDevList[onuPort]->numOfPorts,                                             
					(status == 1) ? "UP" : "DOWN");                                            
				ulIndex++;
			}
		}
		cli_error(cli,  "---------------------------------------------------------------\r\n"); 
	}
	return CLI_OK;
}

cs_int32 cli_int_clear_switch(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)
{
	cs_ulong32  onuPort;
	cs_int8    ifName[IFM_NAME_SIZE + 1];
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
            return cli_arg_help(cli, 0,
                "<slot/port>", "Specify ethernet interface's slot and port\n",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc == 1)
	{
		bzero( ifName, IFM_NAME_SIZE + 1 );
   		snprintf( ifName, IFM_NAME_SIZE, "%s", argv[0] );
		if(!my_onu)
			{
				my_onu = (struct slot_port *)malloc(sizeof(struct slot_port));
				if(!my_onu)
					{
						cli_print(cli,"my_onu malloc error\n");
					}
			}
		
		
		my_onu = BEGIN_PARSE_PORT_EAND_SLOT(argv[0],my_onu,ifName,cli);
		if(!my_onu)
			{
				cli_print(cli, "%% Invalid input.");
				return COMMAND_SUCCESS;
			}
		onuPort = ETH_SLOTPORT_TO_PORTNO(my_onu->ulSlot,my_onu->ulPort);
		if(rcpDevList[onuPort]!=NULL)
		{
			if(1 != RCP_Dev_Is_Exist(onuPort))
			{
			/*rcpDevList[onuPort] = NULL;*/
				bzero(rcpDevList[onuPort]->switchMac, RCP_MAC_SIZE);
				bzero(rcpDevList[onuPort]->previousswitchMac, RCP_MAC_SIZE);
				rcpDevList[onuPort]->onlineStatus = rcpDevList[onuPort]->previousOnlineStatus = 0;
				rcpDevList[onuPort]->timeoutFlag = 1;
			
				cli_print( cli, "%% Delete switch %s successed.\r\n", ifName);
				return CMD_WARNING;
			}
			else
				cli_print(cli,  "  Downlink RCP switch's status is UP, can't be deleted.\r\n");
		}
		else
		cli_print(cli,  "  No RCP switch downlink eth %s.\r\n", ifName);
		my_onu_port = (void *)onuPort;
	}
	else
		{
			cli_print(cli, "%% Invalid input.");
			free(my_onu);
		}
	return CLI_OK;
}

 
cs_int32 cli_int_switch_manage_auth(struct cli_def *cli, cs_int8 *command, cs_int8 *argv[], cs_int32 argc)

{
        cs_uint8 gwdOui[4] = {0x00, 0x0f, 0xe9};
        cs_ulong32 onuPort;
		if(CLI_HELP_REQUESTED)
  		 {

     	   switch(argc)
      	  {
   
      	  case 1:
           	 cli_arg_help(cli, 0,
                "enable", "Rcp switch auth enable",
                 NULL);
			  return cli_arg_help(cli, 0,
                "disable", "Rcp switch auth disable",
                 NULL);

          default:
            return cli_arg_help(cli, argc > 1, NULL);
          }
	     }
        if(argc == 0)
                cli_print(cli,  "  GWD rcp switch auth is %s.\r\n", (gulGwdRcpAuth)?"enabled":"disabled");
        else
        {
            if(strcmp(argv[0], "enable") == 0)
            {
                for(onuPort = 1; onuPort < gulNumOfPortsPerSystem; onuPort++)
                {/* enable gwd rcp auth,clear the other rcp switch records*/
                    if(strncpy(rcpDevList[onuPort]->switchMac, gwdOui, 3) != 0)
                        rcpDevList[onuPort] = NULL;
                }
                gulGwdRcpAuth = 1;
            }
            else
                gulGwdRcpAuth = 0;
        }
        return CLI_OK;
}

/*
 ** Tasks
 */

void rcp_dev_monitor(cyg_addrword_t data)
{
	cs_int32 i, ret, error;
	cs_uint32 iKeepAliveTimeout, iDiscovreyPeriod;
	RCP_DEV *pRcpDev;
	cs_uint16 vlanum;
    cs_uint16 vid =0;
	iKeepAliveTimeout = RCP_KEEP_ALIVE_TIMEOUT_DEF;
	iDiscovreyPeriod = RCP_DISCOVERY_PERIOD_DEF;

    while(1) 
    {
		if(gulEnableEpswitchMgt)
		{
			for(i=1; i<MAX_RRCP_SWITCH_TO_MANAGE; i++)
			{
				if(RCP_Dev_Is_Valid(i))
				{
					RCP_Say_Hello(i,vid);
					pRcpDev = RCP_Get_Dev_Ptr(i);
						
					if(pRcpDev->timeoutCounter < iKeepAliveTimeout)
					{
						pRcpDev->timeoutCounter++;
					}
					else	
					{
						pRcpDev->timeoutFlag = 1;
					}
					pRcpDev->previousOnlineStatus = pRcpDev->onlineStatus;
					if(1 == pRcpDev->timeoutFlag)
					{
						pRcpDev->onlineStatus = 0;
					}
					else
					{
						pRcpDev->onlineStatus = 1;
					}
				}
			}
#if 0/* Say broadcast hello with new authenKey will no replay */
			RCP_Say_Hello(0,vid);
#endif
			iDiscovreyPeriod--;
			if(iDiscovreyPeriod <= 0)
			{
				for(i=1; i<MAX_RRCP_SWITCH_TO_MANAGE; i++)
				{
					if(RCP_Dev_Is_Valid(i))
					{
						if(1 == RCP_Dev_Is_Exist(i))
						{
							error=0;
							for(vlanum = 0;vlanum<MAX_RCP_VLAN_NUM;vlanum++)
							{
								if(RCP_OK != (ret = RCP_GetVlanVID(rcpDevList[i], vlanum, &(rcpDevList[i]->vlanVid[vlanum]))))
									error++;
								if(RCP_OK != (ret = RCP_GetVlanPort(rcpDevList[i], vlanum, &(rcpDevList[i]->vlanPort[vlanum]))))
									error++;
							}
							if(error != 0)
								diag_printf("updata vlan task failed.(%d,%d)\r\n", ret, error);
						}
				    }
				}
				RCP_Say_Hello(-1,vid);
				iDiscovreyPeriod = RCP_DISCOVERY_PERIOD_DEF;
			}
		}

		cyg_thread_delay(2 * IROS_TICK_PER_SECOND);

		if(gulEnableEpswitchMgt)
		{
			rcp_dev_status_check();	
		}
    }
	return;
}

cs_int32 rcp_dev_status_check(void) 
{
	extern RCP_DEV *rcpDevList[];
	cs_uint8 isolate;
	cs_ulong32 slot;
	cs_uint16 vlanum;
	cs_uint8 NullSwitchMac[8]="";
	cs_ulong32 previousPort[MAX_RRCP_SWITCH_TO_MANAGE +1];
	cs_ulong32 currentPort[MAX_RRCP_SWITCH_TO_MANAGE+1];
	cs_uint16 portPriority[MAX_RRCP_SWITCH_TO_MANAGE+1];
	cs_uint16 mirrorDest[MAX_RRCP_SWITCH_TO_MANAGE+1];
	cs_uint16 mirrorRx[MAX_RRCP_SWITCH_TO_MANAGE+1];
	cs_uint16 mirrorTx[MAX_RRCP_SWITCH_TO_MANAGE+1];
	cs_callback_context_t     context;
    cs_uint8 reg_status;
	//struct cli_def *cli;
	cs_int32 counter,n,i,j,ret, error, shouldUpdateVlan;

	for(n=1; n<=MAX_RRCP_SWITCH_TO_MANAGE;n++)
	{
		previousPort[n] = 1;
		portPriority[n] = 0;
		mirrorDest[n] = 0;
		mirrorRx[n] = 0;
		mirrorTx[n] = 0;
	}

	for(i=1; i<MAX_RRCP_SWITCH_TO_MANAGE; i++)
	{
		if(RCP_Dev_Is_Valid(i))
		{
			if(1 != RCP_Dev_Is_Exist(i))	/* Switch offline */
			{
				/* Send switch offline alarm */
                if((0 == rcpDevList[i]->onlineStatus)&&(1 == rcpDevList[i]->previousOnlineStatus))
                {
					ret = GWD_RETURN_OK;
					if(!(rcpDevList[i]->alarmMask & RCP_ALARM_STATUS_OFFLINE))
					{
                    	ret = pushOneSwitchStatusChgMsg(rcpDevList[i]->paPort, rcpDevList[i]->switchMac, ONU_SWITCH_STATUS_CHANGE_ALM_OFFLINE, 0);
                    }

					rcpDevList[i]->alarmStatus |= RCP_ALARM_STATUS_OFFLINE;
					rcpDevList[i]->previousOnlineStatus = 0;	/* Next time will not send alarm again */
					rcpDevList[i]->timeoutFlag = 1;
					IROS_LOG_CRI(IROS_MID_OAM, "Switch %d[MAC[6]=0x%02X] offline(%02X,%d).\n", i, rcpDevList[i]->switchMac[5], rcpDevList[i]->alarmMask, ret);
				}

				/* if the same switch apears on the other port, clear the old one */
				if(rcpDevList[i]->alarmStatus & RCP_ALARM_STATUS_OFFLINE)
				{
					for(j=1;j<MAX_RRCP_SWITCH_TO_MANAGE;j++)
					{
						if((RCP_Dev_Is_Exist(j)) && (i!=j))
						{
							if(0 == memcmp(&rcpDevList[i]->switchMac[0], &rcpDevList[j]->switchMac[0], RCP_MAC_SIZE))
							{
								memset(rcpDevList[i]->switchMac, 0, RCP_MAC_SIZE);
								memset(rcpDevList[i]->previousswitchMac, 0, RCP_MAC_SIZE);
								rcpDevList[i]->onlineStatus = rcpDevList[i]->previousOnlineStatus = 0;
								rcpDevList[i]->timeoutFlag = 1;
							}
						}
					}
				}
			}
			else
			{
				shouldUpdateVlan = 0;
				/* New switch found or switch changed */
                if(GWD_RETURN_OK != memcmp(&rcpDevList[i]->switchMac[0], &rcpDevList[i]->previousswitchMac[0], RCP_MAC_SIZE))
                {
                    if(GWD_RETURN_OK != Rcp_Get_Device_Info(rcpDevList[i]))
                    	{
							return GWD_RETURN_ERR;
                    	}

                    shouldUpdateVlan = 1;

					if(GWD_RETURN_OK == memcmp(rcpDevList[i]->previousswitchMac, NullSwitchMac, RCP_MAC_SIZE))	/* New switch found */
					{
						ret = GWD_RETURN_OK;
						if(!(rcpDevList[i]->alarmMask & RCP_ALARM_STATUS_REGISTER))
						{
	                    	ret = pushOneSwitchStatusChgMsg(rcpDevList[i]->paPort, rcpDevList[i]->switchMac, RCP_ALARM_STATUS_REGISTER, 0);
	                    }
						rcpDevList[i]->previousOnlineStatus = rcpDevList[i]->onlineStatus = 1;
						IROS_LOG_CRI(IROS_MID_OAM, "Switch %d[MAC[6]=0x%02X] register(%02X,%d).\n", i, rcpDevList[i]->switchMac[5], rcpDevList[i]->alarmMask, ret);
					}
					else	/* Switch changed */
					{
						if(!(rcpDevList[i]->alarmStatus & RCP_ALARM_STATUS_OFFLINE)) /* Not handled before */
						{
							ret = GWD_RETURN_OK;
							if(!(rcpDevList[i]->alarmMask & RCP_ALARM_STATUS_OFFLINE))
							{
	                    		ret = pushOneSwitchStatusChgMsg(rcpDevList[i]->paPort, rcpDevList[i]->previousswitchMac, ONU_SWITCH_STATUS_CHANGE_ALM_OFFLINE, 0);
		                    }
							IROS_LOG_CRI(IROS_MID_OAM, "Switch %d[MAC[6]=0x%02X] offline(%02X,%d).\n", i, rcpDevList[i]->previousswitchMac[5], rcpDevList[i]->alarmMask, ret);
							rcpDevList[i]->timeoutFlag = 1;

						}

						ret = GWD_RETURN_OK;
						if(!(rcpDevList[i]->alarmMask & RCP_ALARM_STATUS_REGISTER))
						{
	                    	ret = pushOneSwitchStatusChgMsg(rcpDevList[i]->paPort, rcpDevList[i]->switchMac, ONU_SWITCH_STATUS_CHANGE_ALM_REG, 0);
	                    }

						rcpDevList[i]->alarmStatus = RCP_ALARM_STATUS_REGISTER;
						IROS_LOG_CRI(IROS_MID_OAM, "Switch %d[MAC[6]=0x%02X] register(%02X,%d).\n", i, rcpDevList[i]->switchMac[5], rcpDevList[i]->alarmMask, ret);
					}
                    
                    memcpy(rcpDevList[i]->previousswitchMac,rcpDevList[i]->switchMac, RCP_MAC_SIZE);
                    rcpDevList[i]->previousOnlineStatus = rcpDevList[i]->onlineStatus;

					if(RCP_OK != (ret = restoreRcpDevCfgFromFlash(rcpDevList[i])))
					{
						IROS_LOG_CRI(IROS_MID_OAM, "Switch %d[MAC[6]=0x%02X] restore config failed(%d).\n", i, rcpDevList[i]->switchMac[5], ret);
					}
                }
				else	/* Switch re-register */
				{
                    if((rcpDevList[i]->onlineStatus == 1)&&(rcpDevList[i]->previousOnlineStatus == 0))
                    {
                    	shouldUpdateVlan = 1;

						if(!(rcpDevList[i]->alarmStatus & RCP_ALARM_STATUS_OFFLINE))	/* Not send offline alarm */
						{
							IROS_LOG_CRI(IROS_MID_OAM, "Switch %d[MAC[6]=0x%02X] online(no ALC).\n", i, rcpDevList[i]->switchMac[5]);
						}
						else
						{
							ret = GWD_RETURN_OK;
							if(!(rcpDevList[i]->alarmMask & RCP_ALARM_STATUS_RE_REGISTER))
							{
		                    	ret = pushOneSwitchStatusChgMsg(rcpDevList[i]->paPort, rcpDevList[i]->switchMac, ONU_SWITCH_STATUS_CHANGE_ALM_REREG, 0);
		                    }
							IROS_LOG_CRI(IROS_MID_OAM, "Switch %d[MAC[6]=0x%02X] online(%02X, %d).\n", i, rcpDevList[i]->switchMac[5], rcpDevList[i]->alarmMask, ret);
						}
						rcpDevList[i]->previousOnlineStatus = 1;	/* Next time will not send alarm again */
						rcpDevList[i]->alarmStatus = RCP_ALARM_STATUS_RE_REGISTER; /* Mark Re-register alarm has been handled */

						if(RCP_OK != (ret = restoreRcpDevCfgFromFlash(rcpDevList[i])))
						{
							IROS_LOG_CRI(IROS_MID_OAM, "Switch %d[MAC[6]=0x%02X] restore config failed(%d).\n", i, rcpDevList[i]->switchMac[5], ret);
						}
                    }
                }
				rcpDevList[i]->frcpPort2LPort(rcpDevList[i], &slot, &currentPort[i], 0, rcpDevList[i]->upLinkPort);
				rcpDevList[i]->frcpPort2LPort(rcpDevList[i], &slot, &previousPort[i], 0, rcpDevList[i]->previousUplinkPort);
                if((currentPort[i] != previousPort[i]) || shouldUpdateVlan)
				{
					IROS_LOG_CRI(IROS_MID_OAM, "Interface  eth1/%d RCP switch uplink mgt port changed to eth %lu\n", i, currentPort[i]);
					RCP_SetVlanPVID(rcpDevList[i], 1, currentPort[i], 1, RCP_CONFIG_2_REGISTER);
					RCP_SetMirrorRxPort(rcpDevList[i], 1, currentPort[i], 0, RCP_CONFIG_2_REGISTER);
					RCP_SetMirrorTxPort(rcpDevList[i], 1, currentPort[i], 0, RCP_CONFIG_2_REGISTER);
					RCP_SetQosPortPriority(rcpDevList[i], 1, currentPort[i], 1, RCP_CONFIG_2_REGISTER);
					RCP_SetQosPortPriority(rcpDevList[i], 1, previousPort[i], 0, RCP_CONFIG_2_REGISTER);
					
					if(RCP_OK == RCP_GetVlanPortIsolate(rcpDevList[i], &isolate))
					{
						counter = 0;
						if(isolate == 1)
						do{
							ret = RCP_SetVlanMgtPortIsolate(rcpDevList[i], currentPort[i], RCP_CONFIG_2_REGISTER);
							counter ++;
						}while((ret != RCP_OK)&&(counter < 3));
						if(counter >= 3)
							IROS_LOG_CRI(IROS_MID_OAM, "Interface  eth1/%d RCP switch port_isolate reset failed.(%d)\n", i, ret);
					}
					error=0;
					for(vlanum = 0;vlanum<MAX_RCP_VLAN_NUM;vlanum++)
					{
						if(RCP_OK != (ret = RCP_GetVlanVID(rcpDevList[i], vlanum, &(rcpDevList[i]->vlanVid[vlanum]))))
							error++;
						if(RCP_OK != (ret = RCP_GetVlanPort(rcpDevList[i], vlanum, &(rcpDevList[i]->vlanPort[vlanum]))))
							error++;
					}
					if(error != 0)
						diag_printf("rcp_dev_status_check: updata vlan failed(%d,%d)\r\n", ret, error);
					rcpDevList[i]->previousUplinkPort = rcpDevList[i]->upLinkPort;
				}
			}
		}
	}
    epon_request_onu_mpcp_reg_status_get( context, ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, &reg_status);
	if(EPON_RETURN_SUCCESS != reg_status)
	{
			if(RCP_OK != (ret = popAllSwitchStatusChgMsg()))
			{
				if(RCP_NO_MORE != ret)
					IROS_LOG_CRI(IROS_MID_OAM, "Send switch status change oam failed.(%d)\n", ret);
			}
	}
	return GWD_RETURN_OK;
}

#if 1
void rcp_loopdetect_monitor(cyg_addrword_t data)
{
	RCP_DEV *pRcpDev[NUM_PORTS_PER_SYSTEM];
	cs_ulong32 slot, phyPort, mgtPort, ethIfIdx;
	cs_ulong32 portlist[NUM_PORTS_PER_SYSTEM] = {0};
	cs_ulong32 port, loopstatus;
	cs_uint8 rcpPort;
	cs_uint16 portstatus;	
	//cs_uint16 loopen;
	//cs_uint8 OAMsession[8]="";
	cs_int32 i,ret;
    extern cs_ulong32 gulEnableEpswitchMgt;
	while(TRUE) 
	{
	    if(gulEnableEpswitchMgt)
	    {
		for(i=1; i<MAX_RRCP_SWITCH_TO_MANAGE; i++)
		{
			if(RCP_Dev_Is_Valid(i))
			{
				pRcpDev[i] = RCP_Get_Dev_Ptr(i);
				pRcpDev[i]->frcpPort2LPort(pRcpDev[i], &slot, &mgtPort, 0, pRcpDev[i]->upLinkPort);
				ethIfIdx = IFM_ETH_CREATE_INDEX(PORTNO_TO_ETH_SLOT(i), PORTNO_TO_ETH_PORTID(i));
				if(RCP_OK == (ret = RCP_GetLoopPort(pRcpDev[i], &portlist[i])))
				{
					for(phyPort = 0; phyPort < pRcpDev[i]->numOfPorts; phyPort++)
					{
						pRcpDev[i]->frcpPort2LPort(pRcpDev[i], &slot, &port, 0, phyPort);
						rcpPort = (cs_uint8) port;
						loopstatus = IFM_ETH_ALARM_STATUS_LOOP;
						if(((( pRcpDev[i]->loopAndDown& (0x1 << phyPort)) >> phyPort) == 1) && (RCP_OK == RCP_GetPortEnable(pRcpDev[i],slot, port,&portstatus)))
						{
							if (portstatus)
							{
							/*if this port marked loopback and shut down before, clear the loopstatus now*/
								pRcpDev[i]->loopStatus &= ~(0x1 << (port-1));							
							}
							else
							{
							/*If the loopAndDown ports enabled again, clear the loopAndDown status*/
								//sendOamRcpLpbDetectNotifyMsg(pRcpDev[i]->paPort, rcpPort, 2, 0, OAMsession);
								//
								//IFM_config( ethIfIdx, IFM_CONFIG_ETH_ALARM_STATUS_CLEAR, &loopstatus, NULL );
								//VOS_SysLog(LOG_TYPE_TRAP,LOG_WARNING, "Interface  eth%d/%d RCP port %d loopback cleared.", pRcpDev[i]->paSlot,pRcpDev[i]->paPort,rcpPort);								
								pRcpDev[i]->loopAndDown &= ~(0x1 << (port-1));
							}							
						}
													
						if(((portlist[i] & (0x1 << phyPort)) >> phyPort) == 1)
						{
							//sendOamRcpLpbDetectNotifyMsg(pRcpDev[i]->paPort, rcpPort, 1, 0, OAMsession);
							//IFM_config( ethIfIdx, IFM_CONFIG_ETH_ALARM_STATUS_SET, &loopstatus, NULL );
							//VOS_SysLog(LOG_TYPE_TRAP,LOG_WARNING, "Interface  eth%d/%d RCP port %d marked loopback.", pRcpDev[i]->paSlot,pRcpDev[i]->paPort,rcpPort);
							if(port != mgtPort)
							{
								pRcpDev[i]->loopStatus |= (0x1 << (port-1));
								if(RCP_OK == RCP_SetPortEnable(pRcpDev[i], 1, port, 1, RCP_CONFIG_2_REGISTER))
								{
									pRcpDev[i]->loopAndDown |= (0x1 << (port-1));
								}
							}
						} 
					}
				}
			}
		}
	    }
	  cyg_thread_delay(100*12);
	}
	return;
}
#endif
/*-----------------------------------------------------------------------------------------------------------
API:ONU下的设备检测TASK.
-------------------------------------------------------------------------------------------------------------*/
void start_rcp_device_monitor(void)
{
	cs_int32 iRet;
	
    if(!gulRcpFrameHandleRegister)
    {
#ifdef HAVE_EXT_SW_DRIVER
       //iRet = epon_onu_sw_register_frame_handle(RcpFrameRevHandle);
#else
      // iRet = epon_onu_register_special_frame_handle(RcpFrameRevHandle);
	   iRet = app_pkt_reg_handler(CS_PKT_RCP, rcp_pkt_proc);
       iRet |= app_pkt_reg_parser(CS_PKT_RCP, rcp_pkt_parser);
#endif
       if (iRet == EPON_RETURN_FAIL)
       {
             diag_printf("\r\nRegister RCP frame handler failed!");
       }
       else if (iRet == EPON_RETURN_SUCCESS)
       {
             diag_printf("\r\nRegister RCP frame handler success!");
             gulEthRxTaskReady = 1;
       }
       
       /*iRet = Onu_Loop_Detect_Set_FDB(1);
       if (iRet == 0)
       {
             LOOPBACK_DETECT_DEBUG(("\r\nonu_loop_detect_set success!"));
       }
       else
       {
             LOOPBACK_DETECT_DEBUG(("\r\nonu_loop_detect_set failed!"));
       }*/
           
       	gulRcpFrameHandleRegister = 1;
    }

    // create RCP application thread
    cyg_thread_create(TASK_PRIORITY_LOWEST,
                      rcp_dev_monitor,
                      0,
                      "tRcp",
                      &rcp_thread_stack,
                      RCP_THREAD_STACKSIZE,
                      &rcp_thread_handle,
                      &rcp_thread_obj);
    diag_printf("\r\nRCP moniter thread created!\r\n");
	cyg_thread_create(TASK_PRIORITY_LOWEST,
                      rcp_loopdetect_monitor,
                      0,
                      "loopRcp",
                      &rcploop_thread_stack,
                      RCPLOOP_THREAD_STACKSIZE,
                      &rcploop_thread_handle,
                      &rcploop_thread_obj);
    diag_printf("\r\nRCPLOOP moniter thread created!\r\n");
 	cyg_thread_resume(rcp_thread_handle);
	cyg_thread_resume(rcploop_thread_handle);

	//VOS_TaskCreateEx("tRcpM", rcp_dev_monitor, 220, 4*1024, NULL);
	//VOS_TaskCreateEx("tLoopM", rcp_loopdetect, 220, 4*1024, NULL);

	if(0 != Onu_Rcp_Detect_Set_FDB(1))
	{
    	diag_printf("\r\nRCP MAC init failed!\r\n");
    }

	return;
}  
/*
 ** Init functions
 */

#if 0

cs_int32 rcp_device_mgt_init_func()
{
	return GWD_RETURN_OK;
}

cs_int32 rcp_port_show_run(struct vty *vty, cs_int32 source)
{
	cs_int32 ret;
	cs_uint16 data, tmpMode1, tmpMode2, mode,tmp, tmpdata1,tmpdata2;
	cs_ulong32 mgtPort,slot, port, i, RegCnt, portData;
	RCP_RX_RATE rxRate;
	RCP_TX_RATE txRate;
	RCP_DEV *pRcpDev;

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		GET_AND_CHECK_RCP_DEV_PTR
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		GET_AND_CHECK_RCP_DEV_PTR_FLASH
	}
	else
	{
		return CMD_WARNING;
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		pRcpDev->frcpPort2LPort(pRcpDev, &slot, &mgtPort, 0, pRcpDev->upLinkPort);
		if(mgtPort != 1)
			cli_print(cli,  "  uplink mgt port : %d\r\n", mgtPort);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		if(RCP_OK == RCP_GetDeviceID_InFlash(pRcpDev, &data))
		{
			switch(data)
			{
				case GH1508 :
					cli_print(cli,  "!config for GH1508\r\n");
					break;
				case GH1516 :
					cli_print(cli,  "!config for GH1516\r\n");
					break;
				case GH1524 :
					cli_print(cli,  "!config for GH1524\r\n");
					break;
				case GH1532 :
					cli_print(cli,  "!config for GH1532\r\n");
					break;
				default :
					cli_print(cli,  "!config for unknown device\r\n");
			}
		}
		else
		{
			cli_print(cli,  "!get device id failed\r\n");
		}
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_GLOBAL_PORT_CTL_BASE, &data);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_GLOBAL_PORT_CTL_BASE, &data);
	}
	else
	{
		ret = RCP_FAIL;
	}
	
	if(RCP_OK == ret)
	{
		if((data >> 2) & 0x1 == 1)
			cli_print(cli,  "  broadcast fc enable\r\n");
		if((data >> 3) & 0x1 == 1)
			cli_print(cli,  "  multicast fc enable\r\n");
		if((data >> 4) & 0x1 == 1)
			cli_print(cli,  "  broadcast storm filter disable\r\n");
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_RRCP_CTL_BASE, &data);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_RRCP_CTL_BASE, &data);
	}
	else
	{
		ret = RCP_FAIL;
	}

	if(RCP_OK == ret)
		if((data >> 2) & 0x1 == 0)
			cli_print(cli,  "  loopback auto-detect disable\r\n");
	if(RCP_OK == (ret = RCP_GetIGMPSnooping(pRcpDev, &data)))
		if(data != 0)
			cli_print(cli,  "  igmpsnooping enable\r\n");

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_PORT_DISABLE_BASE, &tmpdata1);
		ret +=RCP_Read_Reg(pRcpDev, RT8326_PORT_DISABLE(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_PORT_DISABLE_BASE, &tmpdata1);
		ret +=getRegValueInFlash(pRcpDev, RT8326_PORT_DISABLE(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else
	{
		ret = RCP_FAIL;
	}

	if(RCP_OK == ret)
	{
		for(i = 0; i < pRcpDev->numOfPorts; i++)
		{
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, i);
			if((portData >> i) & 0x1 == 1)
				cli_print(cli,  "  port en %d 0\r\n", port);
				pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i+1);
			/*if((data >> (8+_Pause_Pos)) & 0x1 == 1)
				cli_print(cli,  "  port fc %d 1\r\n", port);*/
		}
	}

	RegCnt = (pRcpDev->numOfPorts) / 2;
	for(i = 0; i < RegCnt; i++)	
	{
		if(RCP_SHOWRUN_SOURCE_SWITCH == source)
		{
			ret = RCP_Read_Reg(pRcpDev, RT8326_PORT_PROPERTY_BASE + i, &data);
		}
		else if(RCP_SHOWRUN_SOURCE_FLASH == source)
		{
			ret = getRegValueInFlash(pRcpDev, RT8326_PORT_PROPERTY_BASE + i, &data);
		}
		else
		{
			ret = RCP_FAIL;
		}
		if(RCP_OK == ret)
		{
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i);
			if((data>>_Pause_Pos) & 0x1 == 1)
				cli_print(cli,  "  port fc %ld 1\r\n", port);
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i+1);
			if((data >> (8+_Pause_Pos)) & 0x1 == 1)
				cli_print(cli,  "  port fc %ld 1\r\n", port);
		}
	}
	for(i = 0; i < RegCnt; i++)	
	{
		if(RCP_SHOWRUN_SOURCE_SWITCH == source)
		{
			ret = RCP_Read_Reg(pRcpDev, RT8326_PORT_PROPERTY_BASE + i, &data);
		}
		else if(RCP_SHOWRUN_SOURCE_FLASH == source)
		{
			ret = getRegValueInFlash(pRcpDev, RT8326_PORT_PROPERTY_BASE + i, &data);
		}
		else
		{
			ret = RCP_FAIL;
		}

		if(RCP_OK == ret)
		{
			tmpMode1 = data & 0xf;
			tmpMode2 = data & 0xf00;
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i);
			if(0 == ((data >> _AutoNegotiation_Pos) & 0x1))
			{
				switch (tmpMode1)
				{
					case 0xf:
						mode = 8;
					break;
					case 0x7:
						mode = 9;
					break;
					case 0x3:
						mode = 10;
					break;
					case 0x1:
						mode = 11;
					break;
					default:
						mode = 8;
				}
				cli_print(cli,  "  port mode %ld %d\r\n", port, mode);
			}
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i+1);
			if(0 == ((data >> (8+_AutoNegotiation_Pos))& 0x1))
			{
				switch (tmpMode2)
				{
					case 0xf00:
						mode = 8;
					break;
					case 0x700:
						mode = 9;
					break;
					case 0x300:
						mode = 10;
					break;
					case 0x100:
						mode = 11;
					break;
					default:
						mode = 8;
				}
				cli_print(cli,  "  port mode %ld %d\r\n", port, mode);
			}
		}
	}
	for(i = 0; i < RegCnt; i++)	
	{
		if(RCP_SHOWRUN_SOURCE_SWITCH == source)
		{
			ret = RCP_Read_Reg(pRcpDev, RT8326_PORT_CTL_BASE + i, &data);
		}
		else if(RCP_SHOWRUN_SOURCE_FLASH == source)
		{
			ret = getRegValueInFlash(pRcpDev, RT8326_PORT_CTL_BASE + i, &data);
		}
		else
		{
			ret = RCP_FAIL;
		}
		if(RCP_OK == ret)
		{
			if((data & 0xf) != 0)
			{
				pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i);
				cli_print(cli,  "  port ingress_rate %d %d\r\n", port,RCPPortRate[(data & 0xf)].rate);
			}
			if((data & 0xf0) != 0)
			{
				pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i);
				cli_print(cli,  "  port egress_rate %d %d\r\n", port,RCPPortRate[(data & 0xf0) >> 4].rate);
			}
			if((data & 0xf00) != 0)
			{
				pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i+1);
				cli_print(cli,  "  port ingress_rate %d %d\r\n", port,RCPPortRate[(data & 0xf00) >> 8].rate);
			}
			if((data & 0xf000) != 0)
			{
				pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, 2*i+1);
				cli_print(cli,  "  port egress_rate %d %d\r\n", port,RCPPortRate[(data & 0xf000) >> 12].rate);
			}
		}		
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_PORT_MIRROR_BASE, &tmpdata1);
		ret +=RCP_Read_Reg(pRcpDev, RT8326_PORT_MIRROR(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_PORT_MIRROR_BASE, &tmpdata1);
		ret +=getRegValueInFlash(pRcpDev, RT8326_PORT_MIRROR(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else
	{
		ret = RCP_FAIL;
	}
	if(RCP_OK == ret)
	{
		for(i = 0; i < pRcpDev->numOfPorts; i++)
		{
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, i);
			if((portData >> i) & 0x1 == 1)
				cli_print(cli,  "  port mirror_to %d enable\r\n", port);
		}
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_RX_MIRROR_BASE, &tmpdata1);
		ret +=RCP_Read_Reg(pRcpDev, RT8326_RX_MIRROR(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_RX_MIRROR_BASE, &tmpdata1);
		ret +=getRegValueInFlash(pRcpDev, RT8326_RX_MIRROR(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else
	{
		ret = RCP_FAIL;
	}
	if(RCP_OK == ret)
	{
		for(i = 0; i < pRcpDev->numOfPorts; i++)
		{
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, i);
			if((portData >> i) & 0x1 == 1)
				cli_print(cli,  "  port mirror_from i %d 1\r\n", port);
		}
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_TX_MIRROR_BASE, &tmpdata1);
		ret +=RCP_Read_Reg(pRcpDev, RT8326_TX_MIRROR(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_TX_MIRROR_BASE, &tmpdata1);
		ret +=getRegValueInFlash(pRcpDev, RT8326_TX_MIRROR(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else
	{
		ret = RCP_FAIL;
	}
	if(RCP_OK == ret)
	{
		for(i = 0; i < pRcpDev->numOfPorts; i++)
		{
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, i);
			if((portData >> i) & 0x1 == 1)
				cli_print(cli,  "  port mirror_from e %d 1\r\n", port);
		}
	}
	return GWD_RETURN_OK;
}

cs_int32 rcp_vlan_show_run( struct vty *vty, cs_int32 source)
{
	cs_int32 ret;
	cs_uint16 data, vid, vlanen, RegDataArray[8], RegCnt, tmpdata1, tmpdata2;
	cs_uint16 tag[MAX_RCP_PORT_NUM]= { 0 };
	cs_ulong32 slot, port,i,j, portData, portlist;
	cs_uint8 isolate;
	RCP_DEV *pRcpDev;

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		GET_AND_CHECK_RCP_DEV_PTR
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		GET_AND_CHECK_RCP_DEV_PTR_FLASH
	}
	else
	{
		return CMD_WARNING;
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_VLAN_CTL_BASE, &data);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_VLAN_CTL_BASE, &data);
	}
	else
	{
		ret = RCP_FAIL;
	}
	if(RCP_OK == ret)
	{
		if(0 == ((data>>_En8021Qaware_Pos)&0x1))
			cli_print(cli,  "  vlan dot1q disable\r\n");
		if(1 == ((data>>_EnIR_TagAdmit_Pos)&0x1))
			cli_print(cli,  "  vlan acceptable_frame_types tagged\r\n");
		if(1 == ((data>>_EnIR_Membset_Pos)&0x1))
			cli_print(cli,  "  vlan ingress_filtering enable\r\n");
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_GetVlanPortIsolate(pRcpDev, &isolate);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = RCP_GetVlanPortIsolate_InFlash(pRcpDev, &isolate);
	}
	else
	{
		ret = RCP_FAIL;
	}

	if(RCP_OK == ret)
	{
		/*RCP_GetVlan8021Q(pRcpDev, &vlanen);*/
		if(RCP_SHOWRUN_SOURCE_SWITCH == source)
		{
			ret = RCP_Read_Reg(pRcpDev, RT8326_VLAN_CTL_BASE, &data);
		}
		else if(RCP_SHOWRUN_SOURCE_FLASH == source)
		{
			ret = getRegValueInFlash(pRcpDev, RT8326_VLAN_CTL_BASE, &data);
		}
		else
		{
			ret = RCP_FAIL;
		}

		if(RCP_OK == ret)
		{
			vlanen = (data>>_En8021Qaware_Pos)&0x1;
		}

		cli_print(cli,  "  vlan port_isolate %s\r\n", (0 == isolate)?"disable":"enable");
		
		if(1 == vlanen)
		{
			
			RegCnt = (pRcpDev->numOfPorts) / 8;
			for(j=0; j<RegCnt; j++)
			{
				if(RCP_SHOWRUN_SOURCE_SWITCH == source)
				{
					ret = RCP_Read_Reg(pRcpDev, RT8326_VLAN_PRI_BASE+j, &(RegDataArray[j]));
				}
				else if(RCP_SHOWRUN_SOURCE_FLASH == source)
				{
					ret = getRegValueInFlash(pRcpDev, RT8326_VLAN_PRI_BASE+j, &(RegDataArray[j]));
				}
				else
				{
					ret = RCP_FAIL;
				}
				if(RCP_OK == ret)
				{
					if(data != 0)
					{
						for(i=0; i<8; i++)
						{
							if(0x3 == ((RegDataArray[j] >> (2*i))&0x3))
							{
								tag[i + j*8] = 1;
							}
							else
							{
								tag[i + j*8] = 0;
							}
						}
					}
				}
			}
			for(i=0; i<MAX_RCP_VLAN_NUM; i++)
			{
				if(RCP_SHOWRUN_SOURCE_SWITCH == source)
				{
					ret = RCP_Read_Reg(pRcpDev, RT8326_VLAN_VID_BASE+3*i, &data);
				}
				else if(RCP_SHOWRUN_SOURCE_FLASH == source)
				{
					ret = getRegValueInFlash(pRcpDev, RT8326_VLAN_VID_BASE+3*i, &data);
				}
				else
				{
					ret = RCP_FAIL;
				}
				if(RCP_OK == ret)
				{
					vid = data&0x0fff;
					if(vid != 0)
					{
						cli_print(cli,  "  interface vlan %d\r\n", vid);
					}
					else
					{
						continue;
					}

					if(RCP_SHOWRUN_SOURCE_SWITCH == source)
					{
						ret = RCP_Read_Reg(pRcpDev, RT8326_VLAN_PORT_MEMBER(i,0), &tmpdata1);
						ret +=RCP_Read_Reg(pRcpDev, RT8326_VLAN_PORT_MEMBER(i,1), &tmpdata2);
                        portlist = (cs_ulong32)tmpdata2;
		                portData = tmpdata1 | portlist<<16;						
					}
					else if(RCP_SHOWRUN_SOURCE_FLASH == source)
					{
						ret = getRegValueInFlash( pRcpDev, RT8326_VLAN_PORT_MEMBER(i,0), &tmpdata1);
						ret +=getRegValueInFlash(pRcpDev, RT8326_VLAN_PORT_MEMBER(i,1), &tmpdata2);
                        portlist = (cs_ulong32)tmpdata2;
		                portData = tmpdata1 | portlist<<16;						
					}
					else
					{
						ret = RCP_FAIL;
					}
					if(RCP_OK == ret)
					{
						if(data!=0)
						{
							for(j=0; j<(pRcpDev->numOfPorts); j++)
							{
								if(((portData >> j)&0x1) == 1)
									cli_print(cli,  "    add port %d %s\r\n", j+1, (tag[j]==1) ? "tagged" : "untagged");
							}
						}			
					}
				}
			}
		}
	}
	return GWD_RETURN_OK;
}

cs_int32 rcp_atu_show_run(struct vty *vty, cs_int32 source)
{
	cs_int32 ret;
	cs_uint16 data, tmpdata1, tmpdata2;
	cs_ulong32 slot, port,i, portData;
	RCP_DEV *pRcpDev;

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		GET_AND_CHECK_RCP_DEV_PTR
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		GET_AND_CHECK_RCP_DEV_PTR_FLASH
	}
	else
	{
		return CMD_WARNING;
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_ALT_CONFIG, &data);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_ALT_CONFIG, &data);
	}
	else
	{
		ret = RCP_FAIL;
	}
	if(RCP_OK == ret)
	{
		if(data & 0x1 == 1)
			cli_print(cli,  "  atu aging disable\r\n");
		if((data >> 1) & 0x1 == 1)
			cli_print(cli,  "  atu aging fastaging\r\n");
		if((data >> 2) & 0x1 != 1)
			cli_print(cli,  " atu filter disable\r\n");
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_ALT_LEARN_BASE, &tmpdata1);
		ret +=RCP_Read_Reg(pRcpDev, RT8326_ALT_LEARN(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_ALT_LEARN_BASE, &tmpdata1);
		ret +=getRegValueInFlash(pRcpDev, RT8326_ALT_LEARN(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else
	{
		ret = RCP_FAIL;
	}
	if(RCP_OK == ret)
	{
		for(i = 0; i < pRcpDev->numOfPorts; i++)
		{
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, i);
			if((portData >> i) & 0x1 == 1)
				cli_print(cli,  "  atu learning %d 0\r\n", port);
		}
	}
	return GWD_RETURN_OK;
}
cs_int32 rcp_qos_show_run(struct vty *vty, cs_int32 source)
{
	cs_int32 ret;
	cs_uint16 data, tmpdata1, tmpdata2;
	cs_uint8 *ratio;
	cs_ulong32 slot, port, mgtPort, i, portData;
	RCP_DEV *pRcpDev;

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		GET_AND_CHECK_RCP_DEV_PTR
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		GET_AND_CHECK_RCP_DEV_PTR_FLASH
	}
	else
	{
		return CMD_WARNING;
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_QOS_CTL_BASE, &data);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_QOS_CTL_BASE, &data);
	}
	else
	{
		ret = RCP_FAIL;
	}
	if(RCP_OK == ret)
	{
		if(data & 0x1 == 1)
			cli_print(cli,  "  qos dscp_pri_en enable\r\n");
		if((data >> 1) & 0x1 == 1)
			cli_print(cli,  "  qos user_pri_en enable\r\n");
		if((data >> 2) & 0x1 == 1)
			cli_print(cli,  "  qos adapted_fc enable\r\n");
		if((data >> 3) & 0x3 != 0)
		{
			switch((data >> 3) & 0x3)
			{
				case 1:
					ratio = "8:1";
				break;
				case 2:
					ratio = "16:1";
				break;
				case 3:
					ratio = "high_pri_first";
				break;
				default:
					ratio = "4:1";
			}
			cli_print(cli,  "  qos priority_ratio %s\r\n", ratio);
		}
	}

	if(RCP_SHOWRUN_SOURCE_SWITCH == source)
	{
		ret = RCP_Read_Reg(pRcpDev, RT8326_PORT_PRI_BASE, &tmpdata1);
		ret +=RCP_Read_Reg(pRcpDev, RT8326_PORT_PRI(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else if(RCP_SHOWRUN_SOURCE_FLASH == source)
	{
		ret = getRegValueInFlash(pRcpDev, RT8326_PORT_PRI_BASE, &tmpdata1);
		ret +=getRegValueInFlash(pRcpDev, RT8326_PORT_PRI(pRcpDev->numOfPorts), &tmpdata2);
		portData = tmpdata1 | ((cs_ulong32)tmpdata2 <<16);
	}
	else
	{
		ret = RCP_FAIL;
	}
	if(RCP_OK == ret)
	{
		for(i = 0; i < pRcpDev->numOfPorts; i++)
		{
			pRcpDev->frcpPort2LPort(pRcpDev, &slot, &port, 0, i);
			if((portData >> i) & 0x1 == 1)
				cli_print(cli,  "  qos port_priority %d high\r\n", port);
		}
	}
	return GWD_RETURN_OK;
}
cs_int32 rcp_device_mgt_showrun(struct vty *vty, cs_int32 source)
{
	cs_ulong32 onuPort;
	
	cli_print(cli,  "!RCP config\r\n");
	rcp_port_show_run(vty, source);
	rcp_vlan_show_run(vty, source);
	rcp_atu_show_run(vty, source);
	rcp_qos_show_run(vty, source);
	cli_print(cli,  "!\r\n");
	return GWD_RETURN_OK;
}
cs_int32 rcp_device_auth_showrun(struct vty *vty)
{
	cs_ulong32 onuPort;
    extern cs_ulong32 gulGwdRcpAuth;
	cli_print(cli,  "!RCP config\r\n");
        if(gulGwdRcpAuth == 0)
            cli_print(cli,  " switch manage auth disable\r\n");
	cli_print(cli,  "!\r\n");
	return GWD_RETURN_OK;
}

LONG rcp_device_mgt_module_init(void)
{
	struct cl_cmd_module *Layer2 = NULL;

	Layer2 = (struct cl_cmd_module *) VOS_Malloc(sizeof (struct cl_cmd_module), MODULE_RPU_ONU_MAN);
	if (!Layer2)
	{
		ASSERT(0);
		return -IFM_E_NOMEM;
	}
	VOS_MemZero((cs_int8 *) Layer2, sizeof (struct cl_cmd_module));
	Layer2->module_name = (cs_int8 *) VOS_Malloc(20, MODULE_RPU_ONU_MAN);
	if (!Layer2->module_name)
	{
		ASSERT(0);
		VOS_Free(Layer2);
		return -IFM_E_NOMEM;
	}

	VOS_StrCpy(Layer2->module_name, "sw-mgt");

	Layer2->init_func = rcp_device_mgt_init_func;
	Layer2->showrun_func = rcp_device_auth_showrun;
	Layer2->next = NULL;
	Layer2->prev = NULL;
	cl_install_module(Layer2);

	return GWD_RETURN_OK;
}

/*
 ** Install the commands
 */
struct cmd_node rcp_device_node = {
	RCP_DEVICE_NODE,
	NULL,
	1
};

LONG    rcp_device_mgt_node_install()
{
	install_node(&rcp_device_node, NULL);
    rcp_device_node.prompt = ( cs_int8 * ) VOS_Malloc( 64, MODULE_RPU_ONU_MAN );
    if ( !rcp_device_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

	install_default(RCP_DEVICE_NODE);

	return GWD_RETURN_OK;
}


LONG    Rcp_Device_Mgt_CliInit()
{
	rcp_device_mgt_node_install();
	rcp_device_mgt_module_init();
	Rcp_Device_Mgt_CommandInstall();

	return GWD_RETURN_OK;
}
#endif

#if 0
void cli_reg_port_statistic_w_cmds(struct cli_command** cmd_root)
{
}
#endif
void cli_reg_rcp_cmd(struct cli_command **cmd_root)
{
	  //  struct cli_command *gwd_switch;
    // switch cmds in config mode
  //  gwd_switch = cli_register_command(cmd_root, NULL, "switch", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Switch remote management");
		//cli_register_command(cmd_root, gwd_switch, "show",    cmd_switch_show,     PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Show switch found");
//
    return;
}



void Rcp_Mgt_init(void)    /*externed in sys_main.c*/
{
	RCP_Init();
//	diag_printf("Rcp_Mgt_init......................................................\n");
	//Rcp_Device_Mgt_CliInit();
	start_rcp_device_monitor();
	
	return;
}
cs_int32 Rcp_Get_Eeprom_Param(RCP_DEV *dev, RCP_EEPROM_PARAM_T type, cs_uint8 *data)
{
	cs_int32 i, ret;
	cs_uint16 eeAddr;
	struct _XCVR_DATA_  *pXcvrArr;

	pXcvrArr = RCPBoardInfoArr;
	eeAddr = (cs_uint16)(RCP_EEPROM_BASIC_ADDR - pXcvrArr[type].cAddr - pXcvrArr[type].cLen);
	for(i = 0; i < pXcvrArr[type].cLen; i++)
	{
	    if(RCP_OK != (ret = RCP_GetEepromValue(dev, eeAddr + i, &(data[i]))))
	        return ret;
	}
	data[i] = '\0';
	return RCP_OK;
}
cs_int32 Rcp_Set_Eeprom_Param(RCP_DEV *dev, RCP_EEPROM_PARAM_T type, cs_uint8 *data)
{
	cs_int32 i, ret;
	cs_uint16 eeAddr;
	struct _XCVR_DATA_  *pXcvrArr;

	pXcvrArr = RCPBoardInfoArr;
	eeAddr = (cs_uint16)(RCP_EEPROM_BASIC_ADDR - pXcvrArr[type].cAddr - pXcvrArr[type].cLen);
	for(i = 0; i < pXcvrArr[type].cLen; i++)
	{
	    if(RCP_OK != (ret = RCP_SetEepromValue(dev, eeAddr+i, *data++)))
	        return ret;
	}
	return RCP_OK;
}

cs_int32 Rcp_Get_Device_Info(RCP_DEV *pRcpDev)
{
	cs_int32 ret;
	cs_uint16 boardType;
	cs_uint8 board_type[3];

	if(RCP_OK == (ret = Rcp_Get_Eeprom_Param(pRcpDev, RCP_EEPROM_MODULE_TYPE, board_type)))
	{
	    boardType = ((board_type[0] << 8) | board_type[1]);
	    switch(boardType)
	    {
	           case BOARD_TYPE_GH1508 :
	               pRcpDev->deviceId = GH1508;
	               pRcpDev->numOfPorts = 8;
	               memcpy(pRcpDev->modelID,"GH1508",6 );
	               break;
	           case BOARD_TYPE_GH1516 :
	               pRcpDev->deviceId = GH1516;
	               pRcpDev->numOfPorts = 16;
	               memcpy(pRcpDev->modelID,"GH1516",6 );
	               break;
	           case BOARD_TYPE_GH1524:
	               pRcpDev->deviceId = GH1524;
	               pRcpDev->numOfPorts = 24;
	               memcpy(pRcpDev->modelID,"GH1524",6 );
	               break;
	           case BOARD_TYPE_GH1532:
	               pRcpDev->deviceId = GH1532;
	               pRcpDev->numOfPorts = 32;
	               memcpy(pRcpDev->modelID,"GH1532",6 );
	               break;
	           default :
	               pRcpDev->numOfPorts = -1;
				   ret = RCP_UNKOWN;
				   break;
	    }
	}

	return ret;
}

#define RCP_ROTE_ERR (-1)
RCP_RX_RATE Rcp_PortRateToEnum(cs_ulong32 ulRate)
{
	cs_int32 i;
	
	for(i=0; i<=RCP_8M; i++)
	{
		if(ulRate <= RCPPortRate[i].rate)
		{
			return (RCP_RX_RATE)i;
		}
	}

	return RCP_ROTE_ERR;
}

/*Rcp manufacture eeprom data set protection, at most for 5 times.     Libl 2011.6.28*/
cs_int32 Rcp_Eeprom_Value_Set_Protect(struct cli_def *cli, RCP_DEV *pRcpDev, RCP_EEPROM_PARAM_T eeprm_param,  cs_uint8 *data, cs_ulong32 datalen)
{
	cs_int32 ret, i = 0;
	cs_int8 cmpdata[18];
	do
	{
		ret  =  Rcp_Set_Eeprom_Param(pRcpDev, eeprm_param, data);
		ret += Rcp_Get_Eeprom_Param(pRcpDev, eeprm_param, cmpdata);
		if((RCP_OK == ret) && (0 ==memcmp(data, cmpdata, datalen)))
			return RCP_OK;		
		else
		{
			i++;
			cli_print(cli,"\r\nSet the eeprom value failed for %d times", i);
		}
	}
	while( i < 5 );

	return RCP_ERROR;

}



int down_cpu_sent(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int a ;
	a = atoi(argv[0]);
	if(CLI_HELP_REQUESTED)
    {

        switch(argc)
        {
   
        case 1:
             cli_arg_help(cli, 0,
                "1", "OFF CPU SEND FOR UNI PORT",
                 NULL);
			 return cli_arg_help(cli, 0,
                "0", "OPEN CPU SEND FOR UNI PORT",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
	}
	if(argc > 1)
		{
			cli_print(cli, "%% Invalid input.");
			return 0;
		}
	if(a == 1)
		{
			HERO_W = 1;
			diag_printf("set cpu send off success\n");
		}
	else
		{
			HERO_W = 0;
			diag_printf("set cpu send open success\n");
		}
   return 1;
}

void cli_switch_gwd_cmd(struct cli_command **cmd_root)
{
	struct cli_command *inter,*show,*show_mgt;
	struct cli_command *show_system,*port,*vlan,*stat;
	struct cli_command *broadcast,*storm,*igmpsnooping,*multicast;
	struct cli_command *qos,*loopback,*atu,*cable,*port_cable,*manage;
	struct cli_command *mgt,*mgt_config,*mask,*maks_alarm,*clear,*rcp_switch,*gwd_switch;;
		//	inter = cli_register_command(cmd_root, 0,     "interface", NULL,					PRIVILEGE_PRIVILEGED, MODE_EXEC, "Select an interface to config");
				//c = cli_register_command(cmd_root, 0,     "configure", NULL,                         PRIVILEGE_PRIVILEGED, MODE_EXEC, "Enter configuration mode");
			      //  cli_register_command(cmd_root, c,     "terminal", cli_int_configure_terminal,    PRIVILEGE_PRIVILEGED, MODE_EXEC, "Configure from the terminal");
			inter = cli_register_command(cmd_root, NULL,  "interface",    NULL,     PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Select an interface to configure");
					cli_register_command(cmd_root, inter, "IFNAME",   cmd_config_int ,PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Interface name");
					cli_register_command(cmd_root, inter, "switch",   cli_int_interface_switch ,PRIVILEGE_PRIVILEGED, MODE_CONFIG, "Config switch connected to the interface");
			 show = cli_register_command(cmd_root, NULL,  "show",  NULL ,   					PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show running system information");
					cli_register_command(cmd_root, show,  "loop_port",  cli_int_show_loop_port,   			PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show loopback ");		
				//	cli_register_command(cmd_root, show,  "running_config", cli_int_show_running_config,   			PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show running-configuration");		
				//	cli_register_command(cmd_root, show,  "startup-config",  cli_int_show_startup_config,   			PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show startup-configuration");		
					cli_register_command(cmd_root, show,  "mac",  cli_int_show_mac,   			PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show switch mac address");		
		 show_mgt = cli_register_command(cmd_root, show,  "mgt",  NULL,   						PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show running system information");	 
		show_system=cli_register_command(cmd_root, show,  "system",  NULL,   			PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show running system information" );
					cli_register_command(cmd_root, show_system,  "information",  cli_int_system_information,   PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show the product information in flash");		
					cli_register_command(cmd_root, show_mgt,     "port",  cli_int_show_mgt_port,   PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show system management port");			
			port =  cli_register_command(cmd_root, NULL,  "port",  NULL,   						PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "FE port config");
					cli_register_command(cmd_root, port,  "en",  cli_int_port_en,  				PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Enable/Disable port");
					cli_register_command(cmd_root, port,  "fc",  cli_int_port_fc,   			PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show/Config port's flow control");
					cli_register_command(cmd_root, port,  "link_show",  cli_int_port_link_show, PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show onu FE port link status");
					cli_register_command(cmd_root, port,  "mode",  cli_int_port_mode, 			PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show/Config port's working-mode");
					cli_register_command(cmd_root, port,  "mode_show",  cli_int_port_mode_show, PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show port's working-mode");
					cli_register_command(cmd_root, port,  "ingress_rate",  cli_int_port_ingress_rate, PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show or config onu FE port ingress rate information");
					cli_register_command(cmd_root, port,  "egress_rate",  cli_int_port_egress_rate, PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show or config onu FE port egress rate information");
					cli_register_command(cmd_root, port,  "mirror_to",  cli_int_port_mirror_to, PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Port mirror config");
					cli_register_command(cmd_root, port,  "mirror_from", cli_int_port_mirror_from, PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Port mirror config");
					cli_register_command(cmd_root, port,  "mirror_show", cli_int_port_mirror_show, PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show port mirror config");
			 vlan = cli_register_command(cmd_root, NULL,  "vlan", NULL, 					PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "vlan table config");
					cli_register_command(cmd_root, vlan,  "leaky", cli_int_vlan_leaky,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Vlan leaky configuration");
					cli_register_command(cmd_root, vlan,  "show", cli_int_vlan_info_show,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show vlan table in hardware");
					cli_register_command(cmd_root, vlan,  "pvid", cli_int_vlan_pvid  ,     PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show port's pvid config");
					cli_register_command(cmd_root, vlan,  "dot1q", cli_int_vlan_dotlq,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "dot1q vlan config");
					cli_register_command(cmd_root, vlan,  "dot1q_show", cli_int_vlan_dotlq_show,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show dot1q VLAN list");
					cli_register_command(cmd_root, vlan,  "dot1q_add", cli_int_vlan_dotlq_add,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Add an dot1q VLAN");
					cli_register_command(cmd_root, vlan,  "dot1q_del", cli_int_vlan_dotlq_del,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Delete an dot1q VLAN");
					cli_register_command(cmd_root, vlan,  "dot1q_port_add", cli_int_vlan_dotlq_port_add,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Add a port into vlan");
					cli_register_command(cmd_root, vlan,  "dot1q_port_del", cli_int_vlan_dotlq_port_del,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Delete an port from VLAN");
					cli_register_command(cmd_root, vlan,  "port_isolate", cli_int_vlan_port_isolate,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "dot1q vlan config");
					cli_register_command(cmd_root, vlan,  "insert_pvid", cli_int_vlan_insert_pvid,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "vlan insert pvid config");
					cli_register_command(cmd_root, vlan,  "output_tag", cli_int_vlan_output_tag,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "vlan output tag config");
					cli_register_command(cmd_root, vlan,  "ingress_filtering", cli_int_vlan_ingress_filtering,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Vlan ingress filter configuration");
					cli_register_command(cmd_root, vlan,  "acceptable_frame_types", cli_int_vlan_acceptable_frame_types,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Vlan acceptable frame types config");
			 stat = cli_register_command(cmd_root, NULL,  "stat", NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Switch statistic");
					cli_register_command(cmd_root, stat,  "rx", cli_int_stat_rx,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "RX data statistic");
					cli_register_command(cmd_root, stat,  "tx", cli_int_stat_tx,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "TX data statistic");
					cli_register_command(cmd_root, stat,  "diag",cli_int_stat_diag,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Diagnositic data statistic");
		broadcast = cli_register_command(cmd_root, NULL,  "broadcast",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Broadcast config");
					cli_register_command(cmd_root, broadcast,  "fc",cli_int_broadcast_fc,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Broadcast strict flow control");
			storm = cli_register_command(cmd_root, broadcast,  "storm",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Broadcast storm config");
					cli_register_command(cmd_root, storm,      "filtering",cli_int_broadcast_storm_filtering,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Broadcast storm filtering control");
	 igmpsnooping = cli_register_command(cmd_root, NULL,       "filtering",cli_int_igmpsnooping,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "IGMPsnooping config");
		multicast = cli_register_command(cmd_root, NULL,       "multicast",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Mulitcast config");
					cli_register_command(cmd_root, multicast,  "fc",cli_int_multicast_fc,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Multicast strict flow control");
			  qos = cli_register_command(cmd_root, NULL, "qos",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "QoS config");
					cli_register_command(cmd_root, qos,  "dscp_pri_en",cli_int_qos_dscp_pri_en,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Qos diffserver priority config");
					cli_register_command(cmd_root, qos,  "user_pri_en",cli_int_qos_user_pri_en,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "QoS 802.1p priority config");
					cli_register_command(cmd_root, qos,  "adapted_fc",cli_int_qos_adapted_fc,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "QoS adapted flow control config");
					cli_register_command(cmd_root, qos,  "priority_ratio",cli_int_qos_priority_ratio,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "QoS priority raito config");
					cli_register_command(cmd_root, qos,  "port_priority",cli_int_qos_port_priority,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Set port's priority config");
					cli_register_command(cmd_root, qos,  "priority_type",cli_int_qos_priority_type,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Show current qos priority based type");
		 loopback = cli_register_command(cmd_root, NULL,  "loopback",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Loopback control");
					cli_register_command(cmd_root, loopback,  "auto-detect",cli_int_loopback_auto_detect,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Loopback auto detect control");
			  atu = cli_register_command(cmd_root, NULL,      "auto-detect",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Mac table config");
					cli_register_command(cmd_root,atu,        "learning",cli_int_atu_learning,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Uni mac learn configuration");
					cli_register_command(cmd_root,atu,        "aging",cli_int_atu_aging,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Atu table aging config");
					cli_register_command(cmd_root,atu,        "filter",cli_int_atu_filter,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Networt Control Frame filtering control");
			cable = cli_register_command(cmd_root,NULL,       "cable",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Ethernet cable test");
	   port_cable = cli_register_command(cmd_root,cable,      "test",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Test ethernet cable characters");
					cli_register_command(cmd_root,port_cable, "port",cli_int_cable_test_port,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Test ports");
			  mgt = cli_register_command(cmd_root,NULL,  "mgt",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Mgt config");
					cli_register_command(cmd_root,mgt,   "reset",cli_int_mgt_reset,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Reset switch and load configuration from EEPROM");
	   mgt_config = cli_register_command(cmd_root,mgt,   "config",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Manage configuration");
					cli_register_command(cmd_root,mgt_config,  "clear",cli_int_mgt_config_clear,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Load factory default configuration");
					cli_register_command(cmd_root,mgt_config,  "save",cli_int_mgt_config_save,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Save current configuration");
			 mask = cli_register_command(cmd_root,NULL,  "mask",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Mask information");
	   maks_alarm = cli_register_command(cmd_root,mask,  "alarm",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Mask alarm");
					cli_register_command(cmd_root,maks_alarm,  "switch",cli_int_mask_alarm_switch,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Mask alarm switch status change");
			clear = cli_register_command(cmd_root,NULL,   "clear",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Delete an off-line switch");
					cli_register_command(cmd_root,clear,  "switch",cli_int_clear_switch,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Delete an off-line switch");
	   rcp_switch = cli_register_command(cmd_root,NULL,   "switch",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Rcp switch config");
		   manage = cli_register_command(cmd_root,rcp_switch,  "manage",NULL,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Rcp switch auth config");
		   		    cli_register_command(cmd_root,manage,  "auth",cli_int_switch_manage_auth,       PRIVILEGE_PRIVILEGED, MODE_SWITCH,    "Rcp switch auth config");
	   gwd_switch = cli_register_command(cmd_root, NULL,   "switch", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Switch remote management");
					cli_register_command(cmd_root, gwd_switch, "show",    cmd_switch_show,     PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Show switch found");

}


void cli_debeg_gwd_cmd(struct cli_command **cmd_root)
{
	struct cli_command *get,*set,*show,*set_product,*set_board,*set_rcpmanufacture,*mgt;
		cli_register_command(cmd_root, NULL, "debug",    cmd_debug_mode_int, PRIVILEGE_PRIVILEGED,   MODE_CONFIG,    "Enter debug mode");
   		cli_register_command(cmd_root, NULL, "regular",  cmd_debug_regular,  PRIVILEGE_PRIVILEGED,   MODE_DEBUG,     "Switch for regular callback");
    	cli_register_command(cmd_root, NULL, "legacy",   cmd_debug_legacy,   PRIVILEGE_PRIVILEGED,   MODE_DEBUG,     "Switch to legacy console CLI");
	get=cli_register_command(cmd_root, NULL,  "get",   NULL,   						PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Get Rcp (register,eeprom,phy value or field)");
		cli_register_command(cmd_root, get,  "rcpreg",  cli_int_get_rcpreg ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Rcp register");
	 	cli_register_command(cmd_root, get,  "rcpeeprom", cli_int_get_rcpeeprom ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Rcp eeprom");		
	 	cli_register_command(cmd_root, get,  "rcpphy", cli_int_get_rcpphy ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Get phy value or field");		
	set=cli_register_command(cmd_root, NULL,  "set",  NULL ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Enable/Disable RCP switch mgt"); 
	 	cli_register_command(cmd_root, set,  "rcpreg",  cli_int_set_rcpreg ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Rcp register");		
	 	cli_register_command(cmd_root, set,  "switch",  cli_int_set_switch ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Rcp register");		
	 	cli_register_command(cmd_root, set,  "rcpeeprom",  cli_int_set_rcpeeprom ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Rcp eeprom");		
	 	cli_register_command(cmd_root, set,  "rcpphy",  cli_int_set_rcpphy ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Set phy value");		
	set_product=cli_register_command(cmd_root, set,  "product",  NULL ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Set product series");
	 	cli_register_command(cmd_root,set_product,  "series", cli_int_set_product_series ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Product series");		
	 	cli_register_command(cmd_root,set_product,  "type", cli_int_set_product_type ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Set product type");		
    set_board=cli_register_command(cmd_root,set,  "board", NULL,   		PRIVILEGE_PRIVILEGED, MODE_SWITCH,     "Set product type");
	 	cli_register_command(cmd_root,set_board,  "type", cli_int_set_board_type,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Set product type");		
	 	cli_register_command(cmd_root, set,  "rcphw_version", cli_int_set_rcphw_version ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Set switch hardware version");		
    set_rcpmanufacture=cli_register_command(cmd_root, set,  "rcpmanufacture", NULL ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Set switch manufacture information");
	 	cli_register_command(cmd_root, set_rcpmanufacture,  "serial", cli_int_set_rcpmanufacture_serial ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Set the manufacture serial number");		
	 	cli_register_command(cmd_root, set_rcpmanufacture,  "date", cli_int_rcpmanufacture_date ,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "Set the manufacture date");		
	 	cli_register_command(cmd_root, set,  "rcpmac", cli_int_set_rcpmac ,   		PRIVILEGE_PRIVILEGED,MODE_DEBUG,     "Set switch mac address");		
	show=cli_register_command(cmd_root, NULL,  "show", NULL,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "show AuthenKey configuration");		
	 	 cli_register_command(cmd_root, show,  "authenKey", cli_int_show_authenkey,   		PRIVILEGE_PRIVILEGED, MODE_DEBUG,     "AuthenKey configuration");		
	mgt=cli_register_command(cmd_root, NULL, "mgt",     NULL,	  PRIVILEGE_UNPRIVILEGED, MODE_DEBUG,   "Test throughput");
	    cli_register_command(cmd_root, mgt, "thrput-test",     down_cpu_sent,	  PRIVILEGE_UNPRIVILEGED, MODE_DEBUG,   "Test throughput");
}

#ifdef __cplusplus
}
#endif

