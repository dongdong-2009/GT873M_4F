//#include "iros_config.h"
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
#include "i2c.h"  // UART driver is based on I2C, Hardware is I2C <=> exar UART
#include "gpio.h" // UART interrupt use GPIO pins 
#include "uart.h"
#include "oam.h"
#include "gwd_eth_loop_detect.h"
#include "rcp_gwd_w.h"
#include "sdl_port.h"
#include "sdl_fdb.h"
#include "sdl.h"
#include "sdl_vlan.h"
#include "packet.h"
#include "oam_std_pdu.h"
#include "lynxd_registers.h"
#include "registers.h"
#include "aal_l2.h"
#include "sdl_pktctrl.h"
#define LOOPDELETADD
epon_timer_t  g_run_led_tmr;
extern cyg_handle_t m0;
cs_uint32  g_onu_alarm_led_timer;
cs_uint32 led_cout = 0;
#define LPB_OAM_SEND_INTERVAL 1
#define LOOP_DETECT_THREAD_STACKSIZE     (6 * 1024)
#define __VLAN_MAX                      (8*UNI_PORT_MAX)
//extern  __vlan_t __s_vlan_table[__VLAN_MAX];
extern __vlan_t __c_vlan_table[__VLAN_MAX];
LPB_CTRL_LIST *g_lpb_detect_ctrl_head = NULL, *g_lpb_detect_ctrl_tail = NULL;
cs_uint32 MAY_DOWN = 0;
cs_ulong32 timeCounter = 0;
cs_ulong32 gulLoopDetectMode = 1;
cs_ulong32 gulLoopDetectFrameHandleRegister = 0;
OAM_ONU_LPB_DETECT_FRAME local_onu_lpb_detect_frame = {0, 0, 0, 0, {0, 0, 0, 0, 0, 0},30, 1, 3, 3};

cs_ulong32 gulPortDownWhenLpbFound[NUM_PORTS_PER_SYSTEM+1] = { 0 };
cs_ulong32 gulPortWakeupCounter[NUM_PORTS_PER_SYSTEM+1] = { 0 };
cs_ulong32 gulLoopRecFlag[NUM_PORTS_PER_SYSTEM+1] = { 0 };
cs_ulong32 gulLoopIgnorePortDefault[NUM_PORTS_PER_SYSTEM+1] = {0};
cs_ulong32 gulLoopIgnorePort[NUM_PORTS_PER_SYSTEM+1] = { 0 };

cs_uint8 loop_detect_thread_stack[LOOP_DETECT_THREAD_STACKSIZE];
cyg_handle_t  loop_detect_thread_handle;
cyg_thread    loop_detect_thread_obj;
extern cyg_handle_t m0;
cs_ulong32   gulDebugLoopBackDetect = 0;
#define LOOPBACK_DETECT_DEBUG(str) if( gulDebugLoopBackDetect ){ diag_printf str ;}
#define DUMPGWDPKT(c, p, b, l)      if(gulDebugLoopBackDetect) {dumpPkt(c, p, b, l);}

cs_ulong32 gulNumOfPortsPerSystem = NUM_PORTS_PER_SYSTEM;

ALARM_LOOP sendLoopAlarmOam;

cs_uint8 loop_detect_mac[6] = {0x00, 0x0F, 0xE9, 0x04, 0x8E, 0xDF};

cs_uint8 phy_log_map[NUM_UNITS_PER_SYSTEM][PHY_PORT_MAX+1] = {
    /* PHY_PORT_FE0 ~ FE7, MII, EXPAN, SMP */
    {1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 9, 0xFF} /* uint 0 */
};

#if(PRODUCT_CLASS == PRODUCTS_GT811D || PRODUCT_CLASS == PRODUCTS_GT811G ||	PRODUCT_CLASS == PRODUCTS_GT873_M_4F4S)
log_phy_map_t log_phy_map[NUM_PORTS_PER_SYSTEM] = {
     {0, (PHY_PORT_FE0+0)}
    ,{0, (PHY_PORT_FE0+1)}
    ,{0, (PHY_PORT_FE0+2)}
    ,{0, (PHY_PORT_FE0+3)}
    ,{0, (PHY_PORT_FE0+10)}
};

#elif(PRODUCT_CLASS == PRODUCTS_GT812C)
log_phy_map_t log_phy_map[NUM_PORTS_PER_SYSTEM] = {
     {0, (PHY_PORT_FE0+0)}
    ,{0, (PHY_PORT_FE0+1)}
    ,{0, (PHY_PORT_FE0+2)}
    ,{0, (PHY_PORT_FE0+3)}
    ,{0, (PHY_PORT_FE0+4)}
    ,{0, (PHY_PORT_FE0+5)}
    ,{0, (PHY_PORT_FE0+6)}
    ,{0, (PHY_PORT_FE0+7)}
    ,{0, (PHY_PORT_FE0+10)}
};
#endif

cs_int8 port_loop_back_session[8]="";

extern OAM_ONU_LPB_DETECT_FRAME oam_onu_lpb_detect_frame, tframe;
long ethLoopBackDetectActionCall( cs_int32 enable, cs_int8 * oamSession);
cs_int32 sendOamLpbDetectNotifyMsg(cs_uint8 port, cs_uint8 state, cs_uint16 uvid,cs_uint8 *session, ALARM_LOOP *pAlarmInfo);
long ethLoopBackDetectActionCall( cs_int32 enable, cs_int8 * oamSession);
epon_onu_frame_handle_result_t loopbackFrameRevHandle(cs_uint32  portid ,cs_uint32  len, cs_uint8  *frame);
extern epon_return_code_t epon_request_ctc_onu_phy_admin_state_read (cs_uint8 port, cs_uint32 *state);
extern cs_int32 CommOnuMsgSend(cs_uint8 GwOpcode,  cs_uint32 SendSerNo, cs_uint8 *pSentData,const cs_uint16 SendDataSize, cs_uint8  *pSessionIdfield);
extern cs_int32 GwGetOltType(cs_uint8 *mac, GWD_OLT_TYPE *type);
extern cs_int32 GwGetPonSlotPort(cs_uint8 *mac, GWD_OLT_TYPE type, cs_ulong32 *slot, cs_ulong32 *port);
extern cs_port_id ifm_port_id_make(cs_physical_port_id phy_id, cs_logical_link_id llid, cs_port_type port_type);
extern rtk_api_ret_t rtk_chip_id_get(rtk_chip_id_t *chip);
extern cs_status app_pkt_reg_handler(cs_uint8 pkt_type, cs_pkt_handler_t hdl);


cs_int32 boards_logical_to_physical(cs_ulong32 lport, cs_ulong32 *unit, cs_ulong32 *pport)
{
    cs_int32 lport_0 = (lport-1);

    /* check whether logical port number is legal or not */
    if ((lport_0 < 0) || (lport_0 >= NUM_PORTS_PER_SYSTEM)) {
        return 0;
    }
    *unit = log_phy_map[lport_0].unit;
    *pport = log_phy_map[lport_0].physical_port;

    return 1;
}

cs_int32 boards_physical_to_logical(cs_ulong32 unit, cs_ulong32 pport, cs_ulong32 *lport)
{
    /* check whether logical port number is legal or not */
    if ((unit >= NUM_UNITS_PER_SYSTEM) || (pport > PHY_PORT_MAX)) {
        return 0;
    }

    *lport = phy_log_map[unit][pport];
    if ( *lport == 0xFF ) {
        return 0;
    }

    return 1;
}

cs_int32 gtGetSrcPortForMac(cs_int8 *mac, cs_uint16 vid, cs_ulong32 *pLogicPort)
{
		int gtRet = GWD_RETURN_ERR;
		int     i;
		//unsigned int uiPortVect;
		cs_boolean gtFound = EPON_FALSE;

		cs_uint16 idx = 0, nextidx=0,pvid = 0;
		cs_sdl_fdb_entry_t entry;
		cs_callback_context_t context;

		if(NULL == mac)
			return GWD_RETURN_ERR;

		*pLogicPort = 0xFF;
		for (i = 1; i < (UNI_PORT_MAX+1); i++)
			{
				if(epon_onu_sw_get_port_pvid(context, 0, 0, i, &pvid ) != CS_E_OK)
						return GWD_RETURN_ERR;
				while(epon_request_onu_fdb_entry_get_byindex_per_port(context, 0, 0, i, SDL_FDB_ENTRY_GET_MODE_ALL,
							idx, &entry, &nextidx) == CS_OK)
					{
						idx=nextidx;
						LOOPBACK_DETECT_DEBUG(("\r\nfind vid %d smac %02x%02x:%02x%02x:%02x%02x\r\n", vid,
								(int)mac[0],(int)mac[1],(int)mac[2],
								(int)mac[3],(int)mac[4],(int)mac[5]));
						LOOPBACK_DETECT_DEBUG(("entry:  vid %d  mac %02x%02x:%02x%02x:%02x%02x\r\n", entry.vlan_id,
								(int)entry.mac.addr[0],(int)entry.mac.addr[1],(int)entry.mac.addr[2],
								(int)entry.mac.addr[3],(int)entry.mac.addr[4],(int)entry.mac.addr[5]));
						if(entry.vlan_id == 0)
							entry.vlan_id = pvid;
						#if 1
						if(entry.vlan_id == vid &&
								(!memcmp(mac, entry.mac.addr, CS_MACADDR_LEN)))
						{
						#else
						if(!memcmp(mac, entry.mac.addr, CS_MACADDR_LEN))
							{
						#endif

							gtFound = EPON_TRUE;
							break;
						}

					}
				if (EPON_TRUE == gtFound)
					{
						LOOPBACK_DETECT_DEBUG(("\r\nMac found in vlan %d", vid));
						{
							LOOPBACK_DETECT_DEBUG(("\r\nMac found in the right vlan"));
							boards_physical_to_logical(0, i-1, pLogicPort);
							gtRet = GWD_RETURN_OK;
							gtFound = EPON_FALSE;
							///return gtRet;
						}
					}
				else
					{
						LOOPBACK_DETECT_DEBUG(("\r\nMac NOT found in vlan %d", vid));
						//gtRet = GWD_RETURN_ERR;
					}

			}
#if 0
				if (EPON_TRUE == gtFound)
					{
						LOOPBACK_DETECT_DEBUG(("\r\nMac found in vlan %d", vid));
						{
							diag_printf("mac i port:%d\n",i);
							LOOPBACK_DETECT_DEBUG(("\r\nMac found in the right vlan"));
							boards_physical_to_logical(0, i-1, pLogicPort);
							gtRet = GWD_RETURN_OK;
						}
					}
				else
					{
						LOOPBACK_DETECT_DEBUG(("\r\nMac NOT found in vlan %d", vid));
						gtRet = GWD_RETURN_ERR;
					}

					}

						if (EPON_TRUE == gtFound)
							{
								uiPortVect = 0x1 << i;
								for(j=0; j < __VLAN_MAX; j++)
									{
										if(__c_vlan_table[j].valid)
										{
											if(__c_vlan_table[j].vlan.vid == vid)
											{
												if((0 == (__c_vlan_table[j].vlan.mbr & uiPortVect)) &&
												   (0 == (__c_vlan_table[j].vlan.utg & uiPortVect)))
													{
														LOOPBACK_DETECT_DEBUG(("\r\nMac found in wrong vlan %d", vid));
														*pLogicPort = 0xFF;
														gtRet = GWD_RETURN_ERR;
														return gtRet;
													}
													else
													{
														LOOPBACK_DETECT_DEBUG(("\r\nMac found in the right vlan"));
														boards_physical_to_logical(0, i, pLogicPort);
														gtRet = GWD_RETURN_OK;
														return gtRet;
													}
											}
											else
											{
												LOOPBACK_DETECT_DEBUG(("\r\nMac NOT found in vlan %d", vid));
												gtRet = GWD_RETURN_ERR;
												return gtRet;
											}
										}
										else
										{
											LOOPBACK_DETECT_DEBUG(("\r\nepon_onu_sw_search_fdb_entry failed %d", gtRet));
											gtRet = GWD_RETURN_ERR;
											return gtRet;
										}
							}
						}
					}
			}
				
#endif	
		return gtRet;
}

cs_int32 boards_port_is_uni(cs_ulong32 lport)
{
	return (lport < gulNumOfPortsPerSystem)? 1 : 0;
}

cs_ulong32 IFM_ETH_GET_SLOT( cs_ulong32 ulIfIndex )
{
    IFM_ETH_IF_INDEX_U unIfIndex;
    cs_ulong32 ulSlot = 0;

    unIfIndex.ulPhyIfIndex = ulIfIndex;
    ulSlot = unIfIndex.phy_slot_port.slot;

    return ulSlot;
}

cs_ulong32 IFM_ETH_GET_PORT( cs_ulong32 ulIfIndex )
{
    IFM_ETH_IF_INDEX_U unIfIndex;
    cs_ulong32 ulPort = 0;

    unIfIndex.ulPhyIfIndex = ulIfIndex;
    ulPort = unIfIndex.phy_slot_port.port;

    return ulPort;
}

cs_ulong32 IFM_ETH_CREATE_INDEX( cs_ulong32 ulSlot, cs_ulong32 ulPort )
{
    IFM_ETH_IF_INDEX_U unIfIndx;

    /* this is a union, so the first line is used to clear the structure */
    unIfIndx.ulPhyIfIndex = 0;
    unIfIndx.phy_slot_port.type = IFM_ETH_TYPE;
    unIfIndx.phy_slot_port.slot = ulSlot;
    unIfIndx.phy_slot_port.port = ulPort;
    unIfIndx.phy_slot_port.subif = 0;

    return unIfIndx.ulPhyIfIndex;
}
static cs_boolean __vlan_table_lookup_w
(
    __vlan_t            *vlan_table, 
    cs_uint16           vid, 
    cs_uint16           *index          /* if false, index==first empty entry; if true, index==vid entry */
)
{
    int i;
    cs_boolean empty_found = EPON_FALSE;

    for(i = 0; i < __VLAN_MAX; ++i)
    {
        if( (!vlan_table[i].valid) && (!empty_found) )
        {
            *index = i;
            empty_found = EPON_TRUE;
            continue;
        }
    
        if( (vlan_table[i].valid) && (vid == vlan_table[i].vlan.vid) )
        {   
            *index = i;
            return EPON_TRUE;
        }
    }

    if(!empty_found) /* table is full */
    {
        *index = 0xffff;
    }

    return EPON_FALSE;
}
cs_int32 IFM_GET_FIRST_PORTONVLAN(cs_ulong32 *ulport, cs_uint16 vid)
{
	cs_int32 i;
	cs_ulong32 lport;
	cs_uint16  index;
	if(__vlan_table_lookup_w(__c_vlan_table,vid,&index))
		{
			for(i=0; i<PHY_PORT_MAX; i++)
			{
				if((__c_vlan_table[index].vlan.mbr & (1<<i)) || (__c_vlan_table[index].vlan.utg & (1<<i)))
				{
					if(boards_physical_to_logical(0, i, &lport))
					{
						*ulport = lport;
						return GWD_RETURN_OK;
					}
				}
			}
		}

	return GWD_RETURN_ERR;
}

cs_uint8* onu_product_name_get(cs_uint8 productID)
{
	switch(productID)
	{
		case DEVICE_TYPE_GT821:
			return "GT821";

		case DEVICE_TYPE_GT831:
			return "GT831";

		case DEVICE_TYPE_GT831_B:
		case DEVICE_TYPE_GT831_B_CATV:
			return "GT831_B";
			
		case DEVICE_TYPE_GT810:
			return "GT810";

		case DEVICE_TYPE_GT816:
			return "GT816";

		case DEVICE_TYPE_GT811:
			return "GT811";
		case DEVICE_TYPE_GT811_A:
			return "GT811_A";

		case DEVICE_TYPE_GT812:
			return "GT812";
		case DEVICE_TYPE_GT812_A:
			return "GT812_A";

		case DEVICE_TYPE_GT813:
			return "GT813";

		case DEVICE_TYPE_GT865:
			return "GT865";

		case DEVICE_TYPE_GT861:
			return "GT861";

		case DEVICE_TYPE_GT815:
			return "GT815";

		case DEVICE_TYPE_GT812PB:
			return "GT812_B";

		case DEVICE_TYPE_GT866:
			return "GT866";

		case DEVICE_TYPE_GT863:
			return "GT863";
		case DEVICE_TYPE_GT871B:
		    return "GT871B";
		case DEVICE_TYPE_GT871R:
		    return "GT871R";
		case DEVICE_TYPE_GT872:
		    return "GT872";
		case DEVICE_TYPE_GT873:
		    return "GT873";
		case DEVICE_TYPE_GT811D:
			return "GT811D";
	    case DEVICE_TYPE_GT811G:
			return "GT811G";
		default:
			return "UNKNOWN";
	}
}

void EthPortLoopBackDetectTask(cyg_addrword_t data)
{
    cs_uint16 intervalTime;
	while(oam_onu_lpb_detect_frame.enable && gulLoopDetectMode)
	{
        LOOPBACK_DETECT_DEBUG(("\r\nEthPortLoopBackDetectTask called"));
	    if(timeCounter == LPB_OAM_SEND_INTERVAL)
        	timeCounter = 0;
    	else
        	timeCounter++;
    	if(LOOP_DETECT_LOCAL_DFT == local_onu_lpb_detect_frame.enable)
        	intervalTime = oam_onu_lpb_detect_frame.interval;
        else
        	intervalTime = local_onu_lpb_detect_frame.interval;
        ethLoopBackDetectActionCall(oam_onu_lpb_detect_frame.enable, port_loop_back_session);
        //VOS_TaskDelay((intervalTime)*VOS_TICK_SECOND);
		cyg_thread_delay((intervalTime)*IROS_TICK_PER_SECOND);
	}	
    LOOPBACK_DETECT_DEBUG(("\r\nEthPortLoopBackDetectTask exit!!!"));
	return;
}

/*jiangxt added, 20111008.*/
cs_uint32 Onu_Loop_Detect_Set_FDB(cs_boolean  opr)
{
       // cs_int32 iRet; 
        cs_sdl_fdb_entry_t fdb_entry;
		//cs_port_id_t i;
        rtk_chip_id_t sw_chiptype;
		//cs_callback_context_t context;
#if 0 // commented by wangxy 2014-02-12
		rtk_chip_id_get(&sw_chiptype);
        if (sw_chiptype && (sw_chiptype != 1))
        {
               return 1;
        }
        LOOPBACK_DETECT_DEBUG(("\r\nOnu_Loop_Detect_Set_FDB func sw_chiptype is : %x", sw_chiptype));
#endif

        if (opr)
        {
            /*add fdb*/
            memset(&fdb_entry, 0, sizeof(fdb_entry));
            memcpy(&fdb_entry.mac.addr, &loop_detect_mac, 6);
            fdb_entry.type = SDL_FDB_ENTRY_STATIC;
            
            if (sw_chiptype == RTK_CHIP_8305)
            	{
            	#if 0
            		for(i = CS_UNI_PORT_ID1; i <= CS_UNI_PORT_ID4; i++)
            			{
            				fdb_entry.port = i;
            				iRet = epon_request_onu_fdb_entry_add(context, 0, 0, &fdb_entry);
						    if (iRet != EPON_RETURN_SUCCESS)
				            {
				                LOOPBACK_DETECT_DEBUG(("\r\nOnu_Loop_Detect_Set_FDB func fdb add error for port %d.",fdb_entry.port));
				               // return 1;
				            }
            			}
				#endif
            	}
			else
				{
				#if 0
					 for(i = CS_UNI_PORT_ID1; i <= CS_UNI_PORT_ID4; i++)
            			{
            				fdb_entry.port = i;
            				iRet = epon_request_onu_fdb_entry_add(context, 0, 0, &fdb_entry);
						    if (iRet != EPON_RETURN_SUCCESS)
				            {
				                LOOPBACK_DETECT_DEBUG(("\r\nOnu_Loop_Detect_Set_FDB func fdb add error for port %d.",fdb_entry.port));
				               // return 1;
				            }
            			}
				#endif
				}
            
        }

        return 0;
}

cs_uint8 gwd_loopdetect_pkt_proc(cs_pkt_t *pPkt)
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

    loopbackFrameRevHandle(port, len, frame_ptr);

    return 1;
}
cs_status gwd_loopdetect_pkt_parser(cs_pkt_t *pPkt)
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
#if 0
    oam_pdu_hdr_t *hdr = NULL;

    hdr = (oam_pdu_hdr_t*)(pPkt->data + pPkt->offset + pPkt->tag_num * sizeof(cs_vlan_hdr_t ));

    if(ntohs(hdr->eth_type) == EPON_ETHERTYPE_IP)
    {
        pPkt->pkt_type = CS_PKT_GWD_LOOPDETECT; 
        return CS_E_OK;
    }
#else
	epon_ether_header_lb_t *hdr = NULL;

	LOOP_DETECT_FRAME_DATA * pdata = NULL;

	int offset = 0;
#ifdef __loop_debug__
    unsigned char testmac[6] = {0x00,0x0f,0xe9,0x04,0x8e,0xdf};
    int i =0;
#endif
    hdr = (epon_ether_header_lb_t*)(pPkt->data + pPkt->offset);
#ifdef __loop_debug__
    if(!memcmp(testmac,hdr->dst,6))
    {
       diag_printf("=============================================================\n");
       for(i = 0; i < 64; i++)
       {
            if(i % 16 == 0)
                diag_printf("\n");
            diag_printf("0x%02x ",pPkt->data[i]);
       }
       diag_printf("\n");
       diag_printf("=============================================================\n");
       diag_printf("\n");
    }
#endif
    if(ntohs(hdr->ethertype) == EPON_ETHERTYPE_DOT1Q)
    	offset = 16;
    else
    	offset = 12;

	pdata = (LOOP_DETECT_FRAME_DATA*)(pPkt->data+pPkt->offset+offset);

	if(ntohs(pdata->Ethtype) == ETH_TYPE_LOOP_DETECT &&
			ntohs(pdata->LoopFlag) == LOOP_DETECT_CHECK)
	{
		pPkt->eth_type = CS_PKT_GWD_LOOPDETECT;
		return CS_E_OK;
	}
#endif

    return CS_E_NOT_SUPPORT;
}
long EthLoopbackDetectControl(cs_ulong32 oamEnable, cs_ulong32 localEnable)
{
	//cs_uint8   szName[32];
      cs_ulong32 	enable;
      cs_int32 iRet;  /*jiangxt added, 20111008.*/
	  #ifdef LOOPDELETADD
	cs_callback_context_t     context;
	  #endif
    enable = oamEnable && localEnable;
	if (enable)
	{
        LOOPBACK_DETECT_DEBUG(("\r\nEthLoopbackDetectControl : enable = %lu", enable));
		/* Enable */
		if (oam_onu_lpb_detect_frame.enable && gulLoopDetectMode)
			return GWD_RETURN_OK;

		/* Enable */
		oam_onu_lpb_detect_frame.enable = 1;
        gulLoopDetectMode = LOOP_DETECT_MODE_OLT;

        if(!gulLoopDetectFrameHandleRegister)
        {
            /*jiangxt added, 20111008.*/
           #ifdef HAVE_EXT_SW_DRIVER
           iRet = epon_onu_sw_register_frame_handle(loopbackFrameRevHandle);
           #else
          //iRet = epon_onu_register_special_frame_handle(loopbackFrameRevHandle);
		  
#ifdef LOOPDELETADD
    		epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_UP_STREAM,CS_PKT_GWD_LOOPDETECT,DST_CPU);
   			epon_request_onu_spec_pkt_dst_set(context, 0, 0, CS_DOWN_STREAM, CS_PKT_GWD_LOOPDETECT, DST_CPU);
#endif
			 iRet = app_pkt_reg_handler(CS_PKT_GWD_LOOPDETECT, gwd_loopdetect_pkt_proc);
			 iRet |= app_pkt_reg_parser(CS_PKT_GWD_LOOPDETECT, gwd_loopdetect_pkt_parser);
			 
           #endif
           if (iRet == EPON_RETURN_FAIL)
           {
                 LOOPBACK_DETECT_DEBUG(("\r\nepon_onu_register_special_frame_handle failed!"));
           }
           else if (iRet == EPON_RETURN_SUCCESS)
           {
                 LOOPBACK_DETECT_DEBUG(("\r\nepon_onu_register_special_frame_handle success!"));
           }
           
           iRet = Onu_Loop_Detect_Set_FDB(1);
           if (iRet == 0)
           {
                 LOOPBACK_DETECT_DEBUG(("\r\nonu_loop_detect_set success!"));
           }
           else
           {
                 LOOPBACK_DETECT_DEBUG(("\r\nonu_loop_detect_set failed!"));
           }
           /*added end, jiangxt.*/
               
           	gulLoopDetectFrameHandleRegister = 1;

			/* Tx and lookup Task */
			//sprintf(szName, "tLoopDetect");
			//VOS_TaskCreate(szName, TASK_PRIORITY_LOWEST, EthPortLoopBackDetectTask, lTaskArg);
	        // create ONU application thread
	        cyg_thread_create(TASK_PRIORITY_LOWEST,
	                          EthPortLoopBackDetectTask,
	                          0,
	                          "tLoopDetect",
	                          &loop_detect_thread_stack,
	                          LOOP_DETECT_THREAD_STACKSIZE,
	                          &loop_detect_thread_handle,
	                          &loop_detect_thread_obj);
	        diag_printf("\r\nloop_detect_thread created\n");
         	cyg_thread_resume(loop_detect_thread_handle);
       }
	}
	else
	{
        LOOPBACK_DETECT_DEBUG(("\r\nEthLoopbackDetectControl : enable = %lu", enable));
		/* Disable */
		/*if (0 == oam_onu_lpb_detect_frame.enable)
			return GWD_RETURN_OK;*/
		/* Stop */
		if(oamEnable == 0)
            oam_onu_lpb_detect_frame.enable = 0;
        else
            oam_onu_lpb_detect_frame.enable = 1;
        if(localEnable == 0)
            gulLoopDetectMode = LOOP_DETECT_MODE_DISABLE;
        else
            gulLoopDetectMode = LOOP_DETECT_MODE_OLT;

		cyg_thread_kill(loop_detect_thread_handle);
        gulLoopDetectFrameHandleRegister = 0;
	}

	return GWD_RETURN_OK;
}

OAM_ONU_LPB_DETECT_CTRL *getVlanLpbStasNode(cs_uint16 vid)
{
	LPB_CTRL_LIST *pNode = g_lpb_detect_ctrl_head;
	
	while(pNode != NULL)
	{
		if(pNode->ctrlnode->vid != 0)
			break;
		pNode = pNode->next;
	}

	return ((pNode!=NULL)?pNode->ctrlnode:NULL);
}

void deleteLpbStatsNode( cs_uint16 vid )
{
	LPB_CTRL_LIST *pNode = g_lpb_detect_ctrl_head, *ppre = NULL;

	while(pNode != NULL)
	{
		if(pNode->ctrlnode->vid != 0)
		{
			if(pNode == g_lpb_detect_ctrl_head)
			{
				g_lpb_detect_ctrl_head = g_lpb_detect_ctrl_head->next;
				if(g_lpb_detect_ctrl_head == NULL)
					g_lpb_detect_ctrl_tail = NULL;
			}
			else if(pNode == g_lpb_detect_ctrl_tail)
			{
				g_lpb_detect_ctrl_tail = ppre;
				ppre->next = NULL;
			}
			else
			{
				ppre->next = pNode->next;
			}

			iros_free(pNode->ctrlnode);
			iros_free(pNode);
			break;
			
		}
		else
		{
			ppre = pNode;
			pNode = pNode->next;
		}
	}
}

static void reportPortsLpbStatus( cs_uint16 vid, cs_int8 *session )
{
	cs_int32 i=0, almstats = 0;
	LPB_CTRL_LIST *pNode = g_lpb_detect_ctrl_head;
    cs_sdl_port_admin_t port_admin = 0;
	cs_callback_context_t context;
	OAM_ONU_LPB_DETECT_CTRL *pCtrl = NULL;
	//cs_printf("report_status\n");
	while(pNode != NULL)
	{
		if(pNode->ctrlnode->vid != 0)
			break;
		pNode = pNode->next;
	}

    if(!pNode)
		return;

	pCtrl = pNode->ctrlnode;
	
	//���˿�admin״̬�Ѿ�ʹ�ܣ������shutdown��־
	for(i=1; i<gulNumOfPortsPerSystem; i++)
	{
		if(pCtrl->lpbportdown[i])
		{
			port_admin = SDL_PORT_ADMIN_DOWN;
			if((epon_request_onu_port_admin_get(context,ONU_DEVICEID_FOR_API, 
				ONU_LLIDPORT_FOR_API, i, &port_admin)== GWD_RETURN_OK)
				&& (SDL_PORT_ADMIN_UP == port_admin))
			{
				pCtrl->lpbportdown[i] = 0;
            	LOOPBACK_DETECT_DEBUG(("\r\nSet vlan(%d)lpbportdown[%d] = 0,cause port %d's admin is UP", vid, i,i));
			}
		}

		if( pCtrl->lpbmask[i]&&(pCtrl->lpbClearCnt[i] == 0))
		{
			//IFM_config( ethIfIdx, IFM_CONFIG_ETH_ALARM_STATUS_SET, &loopstatus, NULL );
            if(timeCounter == LPB_OAM_SEND_INTERVAL)
            {
            //	cs_printf("report send:\n");
            	sendOamLpbDetectNotifyMsg(i, 1, 1, session, &(pCtrl->alarmInfo[i]));
                LOOPBACK_DETECT_DEBUG(("\r\nReport vlan(%d)port(%d)Looped", vid, i));
            }
		}
		if(pCtrl->lpbmask[i])
			{
				//cs_printf("....\n");
        		almstats ++;
			}

	}

	if(almstats == 0)//added by wangxiaoyu 2009-05-08
	{ /*all ports' lpbmask cleared,then delete the node */
		deleteLpbStatsNode(pCtrl->vid);
		LOOPBACK_DETECT_DEBUG(("\r\nDelete lpb stat info node in vlan %d", pCtrl->vid));
	}
}
void freeLpbStatusList(void)
{
	LPB_CTRL_LIST *pList = NULL;
	while(g_lpb_detect_ctrl_head)
	{
		pList = g_lpb_detect_ctrl_head;
		iros_free(pList->ctrlnode);
		g_lpb_detect_ctrl_head = pList->next;
		iros_free(pList);
	}
	g_lpb_detect_ctrl_tail = NULL;
}

cs_int32 clsPortLpbStatus(const cs_uint16 vid, const cs_int8 *ss)
{ /* no loop found,port with lpbmask and not lpbportdown then lpbClearCnt++*/
    cs_ulong32 i;
    OAM_ONU_LPB_DETECT_CTRL *pCtrl = getVlanLpbStasNode(vid);
	//cs_printf("cls\n");
	for(i = 1; i < gulNumOfPortsPerSystem; i++)
    {
    	//cs_printf("mask:%d lpddown:%d  clearcunt:%d\n",pCtrl->lpbmask[i],pCtrl->lpbportdown[i],pCtrl->lpbClearCnt[i]);
        if(pCtrl->lpbmask[i] == 1&&pCtrl->lpbportdown[i] != 1)
        {
			if(0 == gulLoopRecFlag[i]) /* Loop alarm oam has been sent */
			{
	            pCtrl->lpbClearCnt[i]++;
				//cs_printf("clear count:%d",pCtrl->lpbClearCnt[i]);
	            LOOPBACK_DETECT_DEBUG(("\r\nclsPortLpbStatus calls,lpbClearCnt[%lu]++=%d", i,pCtrl->lpbClearCnt[i]));
			}

         }
    }
	return 0;
}

cs_int32 setPortLpbStatus(const cs_uint16 vid, const cs_int32 lport, const cs_int32 shutdown, const cs_int8 * ss, const void *pInfo)
{
	cs_int32 ret = GWD_RETURN_ERR;
	
	OAM_ONU_LPB_DETECT_CTRL *pCtrl = getVlanLpbStasNode(vid);
	
	if(pCtrl != NULL)
	{
		//pCtrl->lpbStateChg[lport] = (pCtrl->lpbmask[lport])?0:1; //added by wangxiaoyu 2009-03-17
		pCtrl->lpbClearCnt[lport] = 0;
        pCtrl->lpbmask[lport] = 1;
		LOOPBACK_DETECT_DEBUG(("\r\nVlan(%d) found (&pCtrl = %p) and set lpbmask[%d]=%d(lport = %d)", vid, pCtrl, lport, pCtrl->lpbmask[lport], lport));
		
		if(shutdown == 0)
			pCtrl->lpbportdown[lport] = 0;
		else
		{

			if(((!local_onu_lpb_detect_frame.enable)&&(pCtrl->lpbportwakeupcounter[lport] < oam_onu_lpb_detect_frame.maxwakeup))||
            	((local_onu_lpb_detect_frame.enable)&&(pCtrl->lpbportwakeupcounter[lport] < local_onu_lpb_detect_frame.maxwakeup)))
			{
				pCtrl->slpcounter[lport] = 0;
                gulPortWakeupCounter[lport] = pCtrl->lpbportwakeupcounter[lport] ;
				LOOPBACK_DETECT_DEBUG(("\r\nrepeated wakeup -- %d in vlan %d", pCtrl->lpbportwakeupcounter[lport], pCtrl->vid));
				LOOPBACK_DETECT_DEBUG(("\r\nmaximum wakeup reapeater is -- %d", oam_onu_lpb_detect_frame.maxwakeup));
			}
			else if(pCtrl->lpbportwakeupcounter[lport] == 255)
			{
				pCtrl->lpbportwakeupcounter[lport] = 0;
                gulPortWakeupCounter[lport] = pCtrl->lpbportwakeupcounter[lport];
				pCtrl->slpcounter[lport] = 0;
				LOOPBACK_DETECT_DEBUG(("\r\nwakeup task start in vlan %d", pCtrl->vid));
			}
			else
			{
				pCtrl->lpbportwakeupcounter[lport] = 255;
                gulPortWakeupCounter[lport] = pCtrl->lpbportwakeupcounter[lport];
				LOOPBACK_DETECT_DEBUG(("\r\nwakeup task stop in vlan %d", pCtrl->vid));
			}
			
			pCtrl->lpbportdown[lport] = 1;
		}

		if(NULL != pInfo)
		{
			memcpy(&(pCtrl->alarmInfo[lport]), pInfo, sizeof(ALARM_LOOP));
		}
		
		ret = GWD_RETURN_OK;
	}
	else
	{
		pCtrl = (OAM_ONU_LPB_DETECT_CTRL*)iros_malloc(IROS_MID_EXCEPT, sizeof(OAM_ONU_LPB_DETECT_CTRL));
		if(pCtrl != NULL)
		{			
			LPB_CTRL_LIST *pList = (LPB_CTRL_LIST *)iros_malloc(IROS_MID_EXCEPT, sizeof(LPB_CTRL_LIST));
			memset(pCtrl, 0, sizeof(OAM_ONU_LPB_DETECT_CTRL));
			if(pList == NULL)
			{
				iros_free(pCtrl);
			}
			else
			{
				memset(pList, 0, sizeof(LPB_CTRL_LIST));
				
				pCtrl->vid = vid;
				pCtrl->lpbmask[lport] = 1;
				pCtrl->lpbStateChg[lport] = 1;
				LOOPBACK_DETECT_DEBUG(("\r\nNew vlan(%d) (&pCtrl = %p) and set lpbmask[%d]: %d", vid, pCtrl, lport, pCtrl->lpbmask[lport]));
				
				if(shutdown == 0)
					pCtrl->lpbportdown[lport] = 0;
				else
				{
					pCtrl->lpbportwakeupcounter[lport] = 0;
                    gulPortWakeupCounter[lport] = pCtrl->lpbportwakeupcounter[lport];
					LOOPBACK_DETECT_DEBUG(("\r\nInit wakeup counter in vlan %d", pCtrl->vid));
					pCtrl->lpbportdown[lport] = 1;
				}

				pList->ctrlnode = pCtrl;
				pList->next = NULL;

				if(g_lpb_detect_ctrl_head == NULL && g_lpb_detect_ctrl_tail == NULL)
				{
					g_lpb_detect_ctrl_head = pList;
					g_lpb_detect_ctrl_tail = pList;
					LOOPBACK_DETECT_DEBUG(("\r\nNew lpb control list"));
				}
				else
				{
					g_lpb_detect_ctrl_tail->next = pList;
					g_lpb_detect_ctrl_tail = g_lpb_detect_ctrl_tail->next;
					LOOPBACK_DETECT_DEBUG(("\r\nNew lpb control node"));
				}

				if(NULL != pInfo)
				{
					memcpy(&(pCtrl->alarmInfo[lport]), pInfo, sizeof(ALARM_LOOP));
				}
				ret = GWD_RETURN_OK;
			}
		}
	}

	return ret;

}

/*begin:
modified by wangxiaoyu,�޸��ж�������forѭ����1��ʼ*/
static cs_int32 resetLpbPort( cs_int32 force, cs_uint8 *session )
{	
	LPB_CTRL_LIST *pList = g_lpb_detect_ctrl_head;
	cs_callback_context_t context;
	cs_ulong32 lport;
	cs_int32 ret;
	
	while(pList)
	{
		//�������ǲ��رն˿ڣ���ԭ���رյĶ˿����´�
		if((force || (!oam_onu_lpb_detect_frame.policy&0x0001))||
        	(force || (!local_onu_lpb_detect_frame.policy&0x0001)))
		{
			for (lport = 1; lport < gulNumOfPortsPerSystem; lport++)
			{
				if(1 == pList->ctrlnode->lpbportdown[lport])
				{
					epon_request_onu_port_admin_set(context,ONU_DEVICEID_FOR_API, 
													ONU_LLIDPORT_FOR_API, lport, SDL_PORT_ADMIN_UP);
					if(GWD_RETURN_OK == (ret = sendOamLpbDetectNotifyMsg(lport, 2, pList->ctrlnode->vid, session, &(pList->ctrlnode->alarmInfo[lport]))))
					{
						pList->ctrlnode->lpbportdown[lport] = 0;
						pList->ctrlnode->lpbmask[lport] = 0;
						pList->ctrlnode->lpbClearCnt[lport] = 0;
					}
                    LOOPBACK_DETECT_DEBUG(("\r\nsendOamLpbDetectNotifyMsg2,%lu(ret=%d)", lport, ret));
					//IFM_config( ulIfIndex, IFM_CONFIG_ETH_ALARM_STATUS_CLEAR, &loopstatus, NULL );
				}
			}
		}
		else
		{
			LOOPBACK_DETECT_DEBUG(("\r\nresetport deny:"));
			/*LOOPBACK_DETECT_DEBUG(("\r\nlpbPortMask: %d", pList->ctrlnode->lpbmask));*/
		}
		
		pList = pList->next;
	}

	return GWD_RETURN_OK;
}

/*begin:
modified by wangxiaoyu 2008-05-14
add uvid
*/
extern rtk_api_ret_t rtk_svlan_defaultSvlan_get(rtk_port_t port, rtk_vlan_t *pSvid);
cs_int32 sendOamLpbDetectNotifyMsg(cs_uint8 port, cs_uint8 state, cs_uint16 uvid,cs_uint8 *session, ALARM_LOOP *pAlarmInfo)
{
	cs_int32 ret; 
	/*modified by wangxiaoyu 2008-06-06
	�����ջ�����������temp[4]-->temp[16]
	*/
    cs_uint8 ucSlot, ucPort;
    ALARM_LOOP *alarmLoop;
    cs_uint8 temp[40]= { 0 };
	//unsigned int pSvid = 0;
	if((NULL == session) || (NULL == pAlarmInfo)) 
	{
		return GWD_RETURN_ERR;
	}

    ucSlot = 1;
    ucPort = port;
	//rt_port = port -1;
	//rtk_svlan_defaultSvlan_get(rt_port,&pSvid);
    alarmLoop = (ALARM_LOOP *)temp;
    alarmLoop->alarmFlag = ONU_PORT_LOOP_ALARM;
    alarmLoop->portNum[2] = ucSlot;
    alarmLoop->portNum[3] = ucPort;
    alarmLoop->loopstate = state;
    alarmLoop->vlanid = htons(uvid);
    if(1 /*gulLoopRecFlag[port]*/)
    {   
        if(pAlarmInfo->onuLocation[1] >= 1 && pAlarmInfo->onuLocation[1] <= MAX_GWD_OLT_SLOT
                    && pAlarmInfo->onuLocation[2] >= 1 && pAlarmInfo->onuLocation[2] <= MAX_GWD_OLT_PORT
                    && pAlarmInfo->oltType > GWD_OLT_NONE && pAlarmInfo->oltType < GWD_OLT_NOMATCH)
        {
            alarmLoop->externFlag = 1;
            alarmLoop->oltType = pAlarmInfo->oltType;
            memcpy(alarmLoop->oltMac, pAlarmInfo->oltMac, 6);
            memcpy(alarmLoop->onuLocation, pAlarmInfo->onuLocation, 4);
            alarmLoop->onuType = pAlarmInfo->onuType;
            memcpy(alarmLoop->onuMac, pAlarmInfo->onuMac, 6);
            memcpy(alarmLoop->onuPort, pAlarmInfo->onuPort, 4);
        }
    }
	ret = CommOnuMsgSend(ALARM_REQ,0,(cs_uint8*)alarmLoop,sizeof(ALARM_LOOP), session);
	LOOPBACK_DETECT_DEBUG(("\r\nSend ALARM_REQ for lpb detection %s! state is %d    port %d/%d", (GWD_RETURN_OK != ret) ? "ERROR" : "SUCCESS", state, 1, port));
	/*if(state == 1)
		VOS_SysLog(LOG_TYPE_ETH, LOG_WARNING, "send eth %d/%d marked loopback status.", PORTNO_TO_ETH_SLOT(port), PORTNO_TO_ETH_PORTID(port));
	else
		VOS_SysLog(LOG_TYPE_ETH, LOG_WARNING, "send eth %d/%d clear loopback status.", PORTNO_TO_ETH_SLOT(port), PORTNO_TO_ETH_PORTID(port));*/
    gulLoopRecFlag[port] = 0;
	
	return ret;	
}
void lpbDetectWakeupPorts(cs_uint16 usVid)
{
    cs_int32 portnum, ret;
	cs_callback_context_t context;
	OAM_ONU_LPB_DETECT_CTRL *pCtrl = getVlanLpbStasNode(usVid);
        if(pCtrl != NULL)
        {
        	//cs_printf("wakeup port 1\n");
            for (portnum = 1; portnum < gulNumOfPortsPerSystem; portnum++)
            {
    			/*ethIfIndex = IFM_ETH_CREATE_INDEX(PORTNO_TO_ETH_SLOT(portnum), PORTNO_TO_ETH_PORTID(portnum));*/
                if((1== pCtrl->lpbmask[portnum])&&(10<=pCtrl->lpbClearCnt[portnum]))
                {
                	//cs_printf(".......................................3\n");
    				if(GWD_RETURN_OK != (ret = sendOamLpbDetectNotifyMsg(portnum, 2, usVid, port_loop_back_session, &(pCtrl->alarmInfo[portnum]))))
    				{
    					IROS_LOG_CRI(IROS_MID_APP, "sendOamLpbDetectNotifyMsg failed!\n");
    				}
    				pCtrl->lpbmask[portnum] = 0;
    				pCtrl->lpbportwakeupcounter[portnum] = 0;
    				pCtrl->slpcounter[portnum] = 0;
    				pCtrl->lpbClearCnt[portnum] = 0;
    				/*IFM_config( ethIfIndex, IFM_CONFIG_ETH_ALARM_STATUS_CLEAR, &loopstatus, NULL );
    				VOS_SysLog(LOG_TYPE_TRAP, LOG_INFO,"Interface  eth%d/%d no loop found in vlan %d for 10 intervals,clear status", PORTNO_TO_ETH_SLOT(portnum), PORTNO_TO_ETH_PORTID(portnum), usVid);*/

					LOOPBACK_DETECT_DEBUG(("\r\nVlan(%d)port(%d)no loop in 10 intervals,clear status(%d)",usVid, portnum, ret));
               	    LOOPBACK_DETECT_DEBUG(("\r\nsendOamLpbDetectNotifyMsg2,%d(ret=%d)", portnum, ret));
           	   }
           	   if((1== pCtrl->lpbmask[portnum])&&(10>=pCtrl->lpbClearCnt[portnum]&&(1==pCtrl->lpbportdown[portnum])))
           		 {
           		 //	cs_printf("......................................2\n");
						LOOPBACK_DETECT_DEBUG(("\r\nVlan %d's lpbportwakeupcounter[%d] : %d, slpcounter[%d] : %d",usVid, portnum, pCtrl->lpbportwakeupcounter[portnum], portnum, pCtrl->slpcounter[portnum]));
		                if(((!local_onu_lpb_detect_frame.enable)&&( pCtrl->lpbportwakeupcounter[portnum] < oam_onu_lpb_detect_frame.maxwakeup ) && 
		                    ((++(pCtrl->slpcounter[portnum])) >= oam_onu_lpb_detect_frame.waitforwakeup))
		                ||((local_onu_lpb_detect_frame.enable)&&(pCtrl->lpbportwakeupcounter[portnum] < local_onu_lpb_detect_frame.maxwakeup)&&
		                    ((++(pCtrl->slpcounter[portnum])) >= local_onu_lpb_detect_frame.waitforwakeup)))
		                {
		                //	cs_printf(".............................................4\n");
							//IFM_admin_up(ethIfIndex, NULL, NULL);
						//	diag_printf("hero @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
							epon_request_onu_port_admin_set(context,ONU_DEVICEID_FOR_API, 
															ONU_LLIDPORT_FOR_API, portnum, SDL_PORT_ADMIN_UP);
							//diag_printf("portnum:%d\n",portnum);
						//	diag_printf("lus @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
							if(GWD_RETURN_OK != (ret = sendOamLpbDetectNotifyMsg(portnum, 2, usVid, port_loop_back_session, &(pCtrl->alarmInfo[portnum]))))
							{
								LOOPBACK_DETECT_DEBUG(("sendOamLpbDetectNotifyMsg failed!"));		
							}
						    pCtrl->lpbportdown[portnum] = 0;
			                pCtrl->slpcounter[portnum] = 0;
			                pCtrl->lpbportwakeupcounter[portnum]++;
			                gulPortDownWhenLpbFound[portnum] = 0;
			                IROS_LOG_CRI(IROS_MID_APP,"Interface  eth%d/%d wakeup(%d) in vlan %d for loopback\n", 1, portnum, pCtrl->lpbportwakeupcounter[portnum], usVid);
		                    LOOPBACK_DETECT_DEBUG(("\r\nVlan %d's port %d admin up and wakeupcouter++(%d)", usVid, portnum, ret));
		                	LOOPBACK_DETECT_DEBUG(("\r\nsendOamLpbDetectNotifyMsg2,%d(ret=%d)", portnum, ret));
		                }
            }
        }
    }
}
//unsigned char olt_mac_addr[6];
void getOltMacAddress(epon_macaddr_t * olt_mac)
{
    volatile EPON_t *g_epon_base = (volatile EPON_t *)&g_lynxd_reg_ptr->epon;

    ONU_MAC_MPCP_OLTMAC1_t olt_mac1 = g_epon_base->MPCP_OLTMAC1;
    ONU_MAC_MPCP_OLTMAC0_t olt_mac0 = g_epon_base->MPCP_OLTMAC0;

    cs_uint8 *mac = (cs_uint8*)olt_mac;

    if(mac == NULL)
    	return;

    mac[0] = (olt_mac1.bf.olt_mac >> 8) & 0xff;
    mac[1] = (olt_mac1.bf.olt_mac) & 0xff;
    mac[2] = (olt_mac0.bf.olt_mac >>24) & 0xff;
    mac[3] = (olt_mac0.bf.olt_mac >>16) & 0xff;
    mac[4] = (olt_mac0.bf.olt_mac >>8)  & 0xff;
    mac[5] = (olt_mac0.bf.olt_mac) & 0xff;

}
//extern cs_status aal_ma_tx(cs_uint8 *buf, cs_uint16 len, cs_aal_port_id_t port, cs_uint8 pri);
//extern rtk_int32 rtk_cpu_pkt_tx(rtk_uint8 *pkt_in, rtk_uint16 in_len, rtk_uint8 *pkt_out, rtk_uint16 *out_len, rtk_uint8 d_port);
//extern void rtk_fram_send(cs_uint8*LoopBackDetectPktBuffer,cs_uint8*pkt_buf,cs_uint16*buf_len,cs_uint32 port);
extern cs_status epon_request_onu_frame_send_w (
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_uint8                  *frame_buf,
    CS_IN cs_uint16                 frame_len,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_uint8                  priority);
cs_int32 lpbDetectTransFrames(cs_uint16 usVid)
{
	//diag_printf("1111111111111111111111111111111111111111111111111111\n");
    cs_int32 i, iRet;
    GWD_OLT_TYPE type;
    cs_ulong32 ulSlot, ulPon;
    epon_macaddr_t olt_mac_addr;
    cs_mac_t sysMac;
    cs_int8 loopVid[2] = { 0 };
    cs_uint8   LoopBackDetectPktBuffer[64], DevId;

    LOOP_DETECT_FRAME *packet_head = NULL;
	cs_uint16 OnuLlid;
	cs_callback_context_t     context;
	//cs_uint32 dport;
   // epon_port_oper_status_t lb_port_opr_status;
	cs_sdl_port_link_status_t lb_port_opr_status;
    memset(&sysMac,0,sizeof(cs_mac_t));
    memcpy(sysMac.addr, oam_onu_lpb_detect_frame.smac, CS_MACADDR_LEN);
	//dport = ifm_port_id_make(ONU_GE_PORT, 0, EPON_PORT_UNI);
    aal_pon_mac_addr_get(&sysMac); 
	getOltMacAddress(&olt_mac_addr);
    GwGetOltType(olt_mac_addr, &type);
    GwGetPonSlotPort(olt_mac_addr, type, &ulSlot, &ulPon);
    loopVid[0] = (cs_uint8)((usVid >> 8) | 0xe0);
    loopVid[1] = (cs_uint8)(0xff & usVid);

    memset(LoopBackDetectPktBuffer, 0, sizeof(LoopBackDetectPktBuffer));
    packet_head = (LOOP_DETECT_FRAME*)LoopBackDetectPktBuffer;
    
    memcpy(packet_head->Destmac, loop_detect_mac, 6);
    memcpy(packet_head->Srcmac, oam_onu_lpb_detect_frame.smac, 6);
    packet_head->Tpid = htons(0x8100);
    memcpy(packet_head->Vid, loopVid, 2);
    packet_head->Ethtype = htons(ETH_TYPE_LOOP_DETECT);
    packet_head->LoopFlag = htons(LOOP_DETECT_CHECK);
    packet_head->OltType = type;
    packet_head->OnuType = (cs_uint8)DEVICE_TYPE_GT811G;
    memset(packet_head->OnuLocation, 0, 4);
    memset(&packet_head->OnuLocation[1], (cs_uint8)ulSlot, 1);
    memset(&packet_head->OnuLocation[2], (cs_uint8)ulPon, 1);
    aal_mpcp_llid_get(&OnuLlid);
    memset(&packet_head->OnuLocation[3], (cs_uint8)OnuLlid, 1);
    memcpy(packet_head->Onumac, &sysMac.addr, 6);
    packet_head->OnuVid =htons(usVid);
    packet_head->Onuifindex = 0;
	//cs_uint8 pkt_buf[1544];
	//cs_uint16 buf_len=72;
    for (i = 0; i < 64 - 38; i++)
    {
	    LoopBackDetectPktBuffer[38 + i] = (cs_uint8) i;
    }

    DevId = 1;
    for(i=0; i < 4; i++)
    {
        //epon_onu_sw_read_port_admin_status(i+1, &lb_port_admin_status);
        epon_request_onu_port_link_status_get(context, ONU_DEVICEID_FOR_API,
                                                      ONU_LLIDPORT_FOR_API,i+1,&lb_port_opr_status);
      //  epon_request_onu_port_link_status_get(i+1, &lb_port_opr_status);
        if (lb_port_opr_status == SDL_PORT_LINK_STATUS_UP)
        {        
		/*LoopBackDetectPktBuffer[12] = 0xC0 + DevId;*/			/* Use Forward DSA tag: forwarding tag/untag based on switch rule */
		/*LoopBackDetectPktBuffer[12] = 0x40 + DevId;*/		/* Use FromCPU DSA tag: forwarding tag/untag based on CPU control */
		/*LoopBackDetectPktBuffer[13] = (cs_int8)(i<<3);*/
		/*LoopBackDetectPktBuffer[12] = 0xFF;
		LoopBackDetectPktBuffer[13] = 0xFE;*/
		DUMPGWDPKT("\r\nLoopDetectsendPkt : ", i+1, LoopBackDetectPktBuffer, sizeof(LoopBackDetectPktBuffer));
		//diag_printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",i);
           // iRet = epon_request_onu_frame_send(i+1, LoopBackDetectPktBuffer, 64);
        // aal_ma_tx(LoopBackDetectPktBuffer, 64, AAL_PORT_ID_GE, ONU_CPU_PKT_PRIO);
		//__sw_pkt_tx_func(LoopBackDetectPktBuffer,64, pkt_buf,&buf_len,i);
		//rtk_fram_send(LoopBackDetectPktBuffer,pkt_buf,&buf_len,i);
		#if 1
		iRet = epon_request_onu_frame_send_w(context,  ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, LoopBackDetectPktBuffer,
       						 64, i+1,ONU_CPU_PKT_PRIO);
		#endif
		LOOPBACK_DETECT_DEBUG(("\r\nepon_onu_sw_send_frame(v%d,p%d) return %d.", usVid, i+1, iRet));
        }
        else
        {
			LOOPBACK_DETECT_DEBUG(("\r\nPort %d no need send for not up.", i+1));
        }
    }
    return GWD_RETURN_OK;
}
#if 0
{
        epon_ether_header_lb_t frame;
        epon_ether_header_lb_vlan_t frame1;
        cs_boolean opr = 0;
        epon_macaddr_t mac;
        epon_port_oper_status_t lb_port_opr_status;
        epon_loop_detect_status tmp_status;
        cs_uint8 i = 0;
#ifdef HAVE_EXT_SW_DRIVER
        epon_return_code_t rc = EPON_RETURN_SUCCESS;
        //epon_port_admin_status_t lb_port_admin_status;
        epon_sw_vlan_config_t config;
        cs_boolean        result;
#endif
        epon_onu_port_status_t opr_cfg;
        unsigned cs_int32 status;
        epon_port_admin_status_t admin_status = ifm_port_admin_status_get(0x30400000);
        
        if(ifm_port_admin_status_get(0x30400000) != EPON_PORT_ADMIN_UP)
        {
			LOOPBACK_DETECT_DEBUG(("\r\nUNI port link down, can't send loop frame."));
            return GWD_RETURN_ERR;
        }
        
        epon_onu_ifm_port_status_read(0x30400000, &opr_cfg);
        lb_port_opr_status = opr_cfg.oper;
        if(lb_port_opr_status != EPON_PORT_OPER_UP)
        {
			LOOPBACK_DETECT_DEBUG(("\r\nUNI port opr status down, can't send loop frame."));
            return GWD_RETURN_ERR;
        }
        
        //build frame
        memset(&frame, 0, sizeof(frame));
        memset(&frame1, 0, sizeof(frame1));
        memcpy(frame.dst, loop_detect_mac, EPON_MACADDR_LEN);
        memcpy(frame.src, onu_node.macaddr, EPON_MACADDR_LEN);
        frame.ethertype = htons(EPON_ETHERTYPE_MGMT);

        //set marvell port id in frame
        //send frame on all 4 marvell ports
        for(i=0; i<epon_onu_read_port_num(); i++)
        {
            //epon_onu_sw_read_port_admin_status(i+1, &lb_port_admin_status);
            epon_onu_sw_read_port_oper_status(i+1, &lb_port_opr_status);
            IROS_LOG_CRI(IROS_MODULE_ID_STP,"port %d opr %d\n", i+1, lb_port_opr_status);
            IROS_LOG_CRI(IROS_MODULE_ID_STP, "oper status = %d , loopstatus = %d \n",lb_port_opr_status,loop_detect_status[i]);
            if (lb_port_opr_status == EPON_PORT_OPER_UP)
            {        
                frame.lb_port = htonl(i+1);
                rc = epon_onu_sw_send_frame(i+1, (epon_uint8_t *)&frame, sizeof(frame));
                IROS_LOG_CRI(IROS_MODULE_ID_STP,"Send untag LD frame to port %d rc = %d\n", i+1, rc);
            }
        }
        memcpy(frame1.dst, mac, EPON_MACADDR_LEN);
        memcpy(frame1.src, onu_node.macaddr, EPON_MACADDR_LEN);
        frame1.ethertype = htons(EPON_ETHERTYPE_MGMT);
        frame1.tpid = htons(0x8100);
        frame1.lb_port = htonl(i+1);
        result = OK;
        memset(&config, 0, sizeof(config));
        while(1)
        {
            if(EPON_RETURN_SUCCESS != epon_onu_sw_get_next_vlan_entry(0, &config, &result))
                    break;
            if(result == EPON_FALSE)
                    break;
            frame1.vlan = htons(config.vlan);
            IROS_LOG_CRI(IROS_MODULE_ID_STP,"Get vlan entry id %d portmap 0x%x result %d\n", config.vlan, config.tagged_portmap, result);
            for(i=0; i<epon_onu_read_port_num(); i++)
            {
                if(config.tagged_portmap & (1 << i))
                    continue;

                epon_onu_sw_read_port_oper_status(i+1, &lb_port_opr_status);
                IROS_LOG_CRI(IROS_MODULE_ID_STP,"port %d opr %d\n", i+1, lb_port_opr_status);
                if (lb_port_opr_status == EPON_PORT_OPER_UP) {
                    frame1.lb_port = htonl(i+1);
                    DUMPLDPKT("LD send", (epon_uint8_t *)&frame1, sizeof(frame1));
                    rc = epon_onu_sw_send_frame(i+1, (epon_uint8_t *)&frame1, sizeof(frame1));
                    IROS_LOG_CRI(IROS_MODULE_ID_STP,"Send tag %d LD frame to port %d rc = %d\n", config.vlan, i+1, rc);
                }
            }
        }
}
#endif

cs_ulong32 gulTmpForOnuPort; 	
void lpbDetectCheckMacTable(cs_uint16 usVid, cs_int8 * oamSession)
{
    OAM_ONU_LPB_DETECT_CTRL *pCtrl = getVlanLpbStasNode(usVid);
    cs_sdl_port_admin_t port_admin = 0;
	cs_callback_context_t context;
    cs_int32 gtRet;
    cs_uint8  *mac = oam_onu_lpb_detect_frame.smac;
    cs_ulong32   lport = 0;
	if(EPON_RETURN_SUCCESS == (gtRet = gtGetSrcPortForMac(mac, usVid, &lport)))
	{
		//diag_printf("################################################################%ld\n",lport);
	    if (boards_port_is_uni(lport))
	    { /*find lport looped */
			LOOPBACK_DETECT_DEBUG(("\r\nLoopback detected on port %d/%lu in vlan %d.", 1, lport, usVid));
            LOOPBACK_DETECT_DEBUG(("\r\nSet lpbClear[%lu] = 0, lpbMask[%lu] = 1", lport, lport));
            if(gulLoopIgnorePort[lport])
            {
                LOOPBACK_DETECT_DEBUG(("\r\nPort %d/%lu ignore loopback detect", 1, lport));
            }
            else
            {
                if(((!local_onu_lpb_detect_frame.enable)&&(oam_onu_lpb_detect_frame.policy&0x0001))||
                	((local_onu_lpb_detect_frame.enable)&&(local_onu_lpb_detect_frame.policy&0x0001)))
        		{
        		    //cs_ulong32 ulportif = IFM_ETH_CREATE_INDEX(PORTNO_TO_ETH_SLOT(lport), PORTNO_TO_ETH_PORTID(lport));
					
                        //IFM_GetIfAdminStatusApi(ulportif, &status);
                       // epon_onu_sw_read_port_admin_status(lport, &port_admin);
						epon_request_onu_port_admin_get(context,ONU_DEVICEID_FOR_API, 
										ONU_LLIDPORT_FOR_API, lport, &port_admin);
                        LOOPBACK_DETECT_DEBUG(("\r\nport %lu admin status is : %d", lport,port_admin));
                        if(port_admin == SDL_PORT_ADMIN_UP)
                        { /* loopback port has not shuted down */
                    		//if(IFM_admin_down(ulportif, NULL, NULL) == GWD_RETURN_OK)
							//diag_printf("hero down $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
		                    if(epon_request_onu_port_admin_set(context,ONU_DEVICEID_FOR_API,ONU_LLIDPORT_FOR_API,
																	lport, SDL_PORT_ADMIN_DOWN) == GWD_RETURN_OK)
					  		{
					  		//	diag_printf("lport:%ld\n",lport);
					  		//	diag_printf("sut down$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	                			gulPortDownWhenLpbFound[lport] = 1;
	                            setPortLpbStatus(usVid, lport, 1, oamSession, NULL);
	                			LOOPBACK_DETECT_DEBUG(("\r\nadmin down port %lu in vlan %d OK", lport, usVid));
	                            LOOPBACK_DETECT_DEBUG(("\r\nset lpbportdown[%lu] = 1", lport));
	                            IROS_LOG_CRI(IROS_MID_APP, "Interface  eth%d/%lu marked loopback in vlan %d.\n", 1, lport, usVid);
	                		}
	                		else
	                			LOOPBACK_DETECT_DEBUG(("\r\nadmin down fail for port %lu, in vlan %d", lport, usVid));
                  
                        }
				}
		        else
				{
	                 if(pCtrl->lpbmask[lport] != 1)
	                 {
	                      LOOPBACK_DETECT_DEBUG(("\r\nNot shutdown port , lpbmask[%lu] : %d", lport, pCtrl->lpbmask[lport]));
	                      setPortLpbStatus(usVid, lport, 0, oamSession, NULL);
	                      IROS_LOG_CRI(IROS_MID_APP, "Interface  eth%d/%lu marked loopback in vlan %u\n", 1, lport, usVid);
	                 }
					 else
					 {
					 	   pCtrl->lpbClearCnt[lport] = 0;
					 }
				}
            }
	    }
		else
	    {
		    LOOPBACK_DETECT_DEBUG(("\r\nno loopback port detected in vlan %d(p%lu)", usVid, lport));
	        clsPortLpbStatus( usVid, oamSession);
		}
	}
	else
	{
		LOOPBACK_DETECT_DEBUG(("\r\nLoopback detect mac lookup failed(%d) in vlan %d.", gtRet, usVid));
	    clsPortLpbStatus(usVid,oamSession);
	}
	return;
}

long lpbDetectRevPacketHandle(cs_int8 *packet, cs_ulong32 len, cs_ulong32 slot, cs_ulong32 port, cs_uint16 vid)
{
    cs_int32 iRet, printFlag;
    cs_mac_t sysMac;
    LOOP_DETECT_FRAME_DATA *revLoopFrame;
	cs_callback_context_t context;
    cs_ulong32 ulLoopPort, onuIfindex, ulslot, ulport;

    if(NULL == packet)
        return GWD_RETURN_ERR;

    ulslot = 1;
    ulLoopPort = port;
    ulport = ulLoopPort;
    
    /*jiangxt added, 20111010.*/
    cs_uint8 SrcMac[6];
    memcpy(SrcMac, packet + 6, 6);
    if ((packet[12] == 0x81) && (packet[13] == 0x00))
    {
        LOOPBACK_DETECT_DEBUG(("\r\nLoopback frame with vlan tag. "));
        revLoopFrame = (LOOP_DETECT_FRAME_DATA *)(packet + 16);
    }
    else
    {
        LOOPBACK_DETECT_DEBUG(("\r\nLoopback frame with no vlan tag. "));
        revLoopFrame = (LOOP_DETECT_FRAME_DATA *) (packet + 12);
    }
    /*add end*/

    aal_pon_mac_addr_get(&sysMac);    
    if(ntohs(revLoopFrame->LoopFlag) != LOOP_DETECT_CHECK)
    {
        LOOPBACK_DETECT_DEBUG(("\r\nLoopFlag error "));
        return GWD_RETURN_ERR;
    }
    iRet = GWD_RETURN_OK;

    if(!gulLoopIgnorePort[ulLoopPort])
    {
        /* if the loop port is PON port, report the first member in VLAN of vid */
        if(!boards_port_is_uni(ulLoopPort))
        {
            LOOPBACK_DETECT_DEBUG(("\r\nThe loop port is PON port!"));
            cs_ulong32 lport;

            if(IFM_GET_FIRST_PORTONVLAN(&lport, vid) == GWD_RETURN_OK)
            {
                ulLoopPort = lport;
            }     
	  }
    }
    else /*I move the else out from the upper if loop, jiangxt, 20111010.*/
    {
	  LOOPBACK_DETECT_DEBUG(("\r\nport %lu/%lu ignored loop detect", ulslot, ulLoopPort));
	  return GWD_RETURN_OK;
    }
    
    gulLoopRecFlag[ulLoopPort]  = 1;
    memset(&sendLoopAlarmOam, 0, sizeof(ALARM_LOOP));
    sendLoopAlarmOam.oltType = revLoopFrame->OltType;
    sendLoopAlarmOam.onuType = revLoopFrame->OnuType;
    memcpy(sendLoopAlarmOam.onuLocation,revLoopFrame->OnuLocation, 4);
    memcpy(sendLoopAlarmOam.onuMac, revLoopFrame->Onumac, 6);
    onuIfindex = revLoopFrame->Onuifindex;
    if(onuIfindex != 0)
    {
        sendLoopAlarmOam.onuPort[2]= IFM_ETH_GET_SLOT(onuIfindex);
        sendLoopAlarmOam.onuPort[3] = IFM_ETH_GET_PORT(onuIfindex);
    }
    else
    	memset(sendLoopAlarmOam.onuPort, 0, 4);
    printFlag = 0;
    if(revLoopFrame->OnuLocation[1] >= 1&&revLoopFrame->OnuLocation[2] <= MAX_GWD_OLT_SLOT
    	&&revLoopFrame->OnuLocation[2] >= 1&&revLoopFrame->OnuLocation[2] <= MAX_GWD_OLT_PORT
        &&revLoopFrame->OltType > GWD_OLT_NONE &&revLoopFrame->OltType < GWD_OLT_NOMATCH)
    {
        printFlag = 1;
    }
                
    memcpy(sendLoopAlarmOam.oltMac, SrcMac, 6);
    
    if(((!local_onu_lpb_detect_frame.enable)&&(oam_onu_lpb_detect_frame.policy&0x0001))||
    	((local_onu_lpb_detect_frame.enable)&&(local_onu_lpb_detect_frame.policy&0x0001)))
    {
        LOOPBACK_DETECT_DEBUG(("\r\nBegin check port status!"));
        cs_sdl_port_admin_t port_admin = 0;
        epon_request_onu_port_admin_get(context,ONU_DEVICEID_FOR_API, 
										ONU_LLIDPORT_FOR_API, ulport, &port_admin);
        if(SDL_PORT_ADMIN_UP == port_admin)
        { /* loopback port has not shuted down */
     //   cs_printf("rec other onu loopfram for port down\n");
           if(epon_request_onu_port_admin_set(context,ONU_DEVICEID_FOR_API,ONU_LLIDPORT_FOR_API,
											ulport, SDL_PORT_ADMIN_DOWN) == GWD_RETURN_OK)
            {
                LOOPBACK_DETECT_DEBUG(("\r\nprintFlag : %d", printFlag));
                gulPortDownWhenLpbFound[ulLoopPort] = 1;
                LOOPBACK_DETECT_DEBUG(("\r\nreceive packet,admin down port %lu/%lu in vlan %d OK", ulslot, ulport, vid));
                setPortLpbStatus(vid, ulLoopPort, 1, port_loop_back_session, (void *)&sendLoopAlarmOam);
                reportPortsLpbStatus(vid, port_loop_back_session);
                if(0 == printFlag)
                {
                	IROS_LOG_CRI(IROS_MID_EXCEPT,"Interface  eth%lu/%lu marked loopback in vlan %d.\n", ulslot, ulport, vid);
                }
                else if(onuIfindex == 0)
                {
                	diag_printf( "Interface  eth%lu/%lu loop[%s(%02x%02x.%02x%02x.%02x%02x)%d/%d %s(%02x%02x.%02x%02x.%02x%02x)V(%d)]\n",
                    ulslot, ulport, (revLoopFrame->OltType == 1)?"GFA6100":"GFA6700", SrcMac[0], SrcMac[1],SrcMac[2],SrcMac[3],SrcMac[4],SrcMac[5],
                    revLoopFrame->OnuLocation[1], revLoopFrame->OnuLocation[2], onu_product_name_get(revLoopFrame->OnuType), revLoopFrame->Onumac[0],
                    revLoopFrame->Onumac[1],revLoopFrame->Onumac[2],revLoopFrame->Onumac[3],revLoopFrame->Onumac[4],revLoopFrame->Onumac[5],vid);
                }
                else
                {
                	diag_printf( "Interface  eth%lu/%lu loop[%s(%02x%02x.%02x%02x.%02x%02x)%d/%d %s(%02x%02x.%02x%02x.%02x%02x)P(%d/%d)V(%d)]\n",
                    ulslot, ulport, (revLoopFrame->OltType == 1)?"GFA6100":"GFA6700", SrcMac[0], SrcMac[1],SrcMac[2],SrcMac[3],SrcMac[4],SrcMac[5],
                    revLoopFrame->OnuLocation[1], revLoopFrame->OnuLocation[2],onu_product_name_get(revLoopFrame->OnuType), revLoopFrame->Onumac[0],revLoopFrame->Onumac[1],
                    revLoopFrame->Onumac[2],revLoopFrame->Onumac[3],revLoopFrame->Onumac[4],revLoopFrame->Onumac[5],sendLoopAlarmOam.onuPort[2], sendLoopAlarmOam.onuPort[3],vid);
                }
            }
            else
                LOOPBACK_DETECT_DEBUG(("\r\nreceiver packet,admin down fail for port %lu/%lu, in vlan %d", ulslot, ulport, vid));
        }
        else
        {
            LOOPBACK_DETECT_DEBUG(("\r\n loop pkt reveived from down port!\r\n"));
        }
    }
    else
    {
        OAM_ONU_LPB_DETECT_CTRL *pCtrl = getVlanLpbStasNode(vid);
        if(pCtrl->lpbmask[ulLoopPort] != 1)
        {
            LOOPBACK_DETECT_DEBUG(("\r\nNot shutdown port , lpbmask[%lu] : %d", ulLoopPort, pCtrl->lpbmask[ulLoopPort]));
	     	setPortLpbStatus(vid, ulLoopPort, 0, port_loop_back_session, (void *)&sendLoopAlarmOam);

            if(0 == printFlag)
            {
                IROS_LOG_CRI(IROS_MID_EXCEPT, "Interface  eth%lu/%lu marked loopback in vlan %d.\n", ulslot, ulport, vid);
            }
            else if(onuIfindex == 0)
            {
                diag_printf( "Interface  eth%lu/%lu loop[%s(%02x%02x.%02x%02x.%02x%02x)%d/%d %s(%02x%02x.%02x%02x.%02x%02x)V(%d)]\n",
                ulslot, ulport, (revLoopFrame->OltType == 1)?"GFA6100":"GFA6700", SrcMac[0], SrcMac[1],SrcMac[2],SrcMac[3],SrcMac[4],SrcMac[5],
                revLoopFrame->OnuLocation[1], revLoopFrame->OnuLocation[2], onu_product_name_get(revLoopFrame->OnuType), revLoopFrame->Onumac[0],
                revLoopFrame->Onumac[1],revLoopFrame->Onumac[2],revLoopFrame->Onumac[3],revLoopFrame->Onumac[4],revLoopFrame->Onumac[5],vid);
            }
            else
            {
                diag_printf( "Interface  eth%lu/%lu loop[%s(%02x%02x.%02x%02x.%02x%02x)%d/%d %s(%02x%02x.%02x%02x.%02x%02x)P(%d/%d)V(%d)]\n",
                ulslot, ulport, (revLoopFrame->OltType == 1)?"GFA6100":"GFA6700",SrcMac[0], SrcMac[1],SrcMac[2],SrcMac[3],SrcMac[4],SrcMac[5], 
                revLoopFrame->OnuLocation[1], revLoopFrame->OnuLocation[2],onu_product_name_get(revLoopFrame->OnuType), revLoopFrame->Onumac[0],revLoopFrame->Onumac[1],
                revLoopFrame->Onumac[2],revLoopFrame->Onumac[3],revLoopFrame->Onumac[4],revLoopFrame->Onumac[5],sendLoopAlarmOam.onuPort[2], sendLoopAlarmOam.onuPort[3],vid);
            }
        }
        else
        {
	      pCtrl->lpbClearCnt[ulLoopPort] = 0;
        }
    }
	
    return iRet;
}

long ethLoopBackDetectActionCall( cs_int32 enable, cs_int8 * oamSession)
{
	cs_int32 ret;
	cs_int32 need_to_send;
    cs_sdl_port_link_status_t status;
	cs_uint16 vid =0;
	//cs_uint16  index;
	if(enable)
	{
		LOOPBACK_DETECT_DEBUG(("\r\n----------^_^ new loopback detect interval start here ^_^----------"));
		resetLpbPort(0, oamSession);

		if((!local_onu_lpb_detect_frame.enable)&&(oam_onu_lpb_detect_frame.vid != 0))
		{
		/*detect loopback in the specific vlan, wakeup the shutdown ports which have not reached wakeup threshold in the specfic vlan*/
			LOOPBACK_DETECT_DEBUG(("\r\nOLT config detect in unique vlan(%d)",oam_onu_lpb_detect_frame.vid));
			lpbDetectWakeupPorts(oam_onu_lpb_detect_frame.vid);
			/*Send detect packet in specific vlan*/
			if(GWD_RETURN_OK != (ret = lpbDetectTransFrames(oam_onu_lpb_detect_frame.vid)))
			LOOPBACK_DETECT_DEBUG(("\r\nlpbDetectTransFrames failed(%d).", ret));
			/*detect loopback in the specific vlan, check mac table cs_int32 the specific vlan*/
			lpbDetectCheckMacTable(oam_onu_lpb_detect_frame.vid, oamSession);
			reportPortsLpbStatus(oam_onu_lpb_detect_frame.vid, oamSession);
		}
		else if((local_onu_lpb_detect_frame.enable)&&(local_onu_lpb_detect_frame.vid != 0))
		{
			LOOPBACK_DETECT_DEBUG(("\r\nONU config detect in unique vlan(%d)", local_onu_lpb_detect_frame.vid));
			lpbDetectWakeupPorts(local_onu_lpb_detect_frame.vid);
			/*Send detect packet in specific vlan*/
			if(GWD_RETURN_OK != (ret = lpbDetectTransFrames(local_onu_lpb_detect_frame.vid)))
			LOOPBACK_DETECT_DEBUG(("\r\nlpbDetectTransFrames failed(%d).", ret));
			/*detect loopback in the specific vlan, check mac table cs_int32 the specific vlan*/
			lpbDetectCheckMacTable(local_onu_lpb_detect_frame.vid, oamSession);
			reportPortsLpbStatus(local_onu_lpb_detect_frame.vid, oamSession);
		}
		else
             {
	        	LOOPBACK_DETECT_DEBUG(("\r\nDetect in all vlan"));
        		cs_callback_context_t context;
    			cs_sdl_vlan_tag_t vdef;
    			cs_sdl_vlan_cfg_t vcfg[MAX_VLAN_RULE_PER_PORT];
    			cs_sdl_vlan_mode_t vmode;
    			cs_uint16 vnum = 0, i = 0;
				cs_port_id_t uni_port;
				for(uni_port = 1; uni_port < UNI_PORT_MAX + 1; uni_port++)
					{
						i =0;
			            if(EPON_RETURN_SUCCESS != epon_request_onu_vlan_get(context, 0, 0, uni_port, &vdef, &vmode, vcfg, &vnum))
			                    break;
						vnum++;
						while(i < vnum)
							{
				        		vid = (i==vnum-1)?vdef.vid:vcfg[i].c_vlan.vid;
				        		if(vid != 0)
				        		{
				        	  	  	lpbDetectWakeupPorts(vid);
										
									need_to_send = 0;

									epon_request_onu_port_link_status_get(context, 0, 0, uni_port, &status);

									if(status == EPON_PORT_OPER_UP)
									{
						    	            need_to_send = 1;
						    	    }	
						    	  	

									LOOPBACK_DETECT_DEBUG(("\r\nVlan %d %s check.", vid, (1 == need_to_send)?"need":"no need"));
							        if (1 == need_to_send)
							        {							            need_to_send = 0;
										/*check the MacTable first ,in case that the ports looped in defferent vlan cannot be detected */
							            lpbDetectCheckMacTable(vid,oamSession);   
					    				if(GWD_RETURN_OK != (ret = lpbDetectTransFrames(vid)))
					                    	LOOPBACK_DETECT_DEBUG(("\r\nlpbDetectTransFrames failed(%d).", ret));

					                }
						            else
						            {
						               	clsPortLpbStatus(vid, oamSession);
						            }
								
				    				reportPortsLpbStatus(vid, oamSession);
									
				        		}

								i++;
			                }
					}
				}
		}  	        
	else
		{
			LOOPBACK_DETECT_DEBUG(("\r\ndisable loopback detect"));
			resetLpbPort(1, oamSession);
			freeLpbStatusList();
		}

	/*check local port loop alarm and set loop alarm led*/
#if (HAVE_LOOP_LED)
	gwdEthPortLoopLedAction();
#endif
	return 0;	
}

epon_onu_frame_handle_result_t loopbackFrameRevHandle(cs_uint32  portid ,cs_uint32  len, cs_uint8  *frame)
{
    cs_ether_header_lb *plb_frame = NULL;
	cs_uint16 vid;
	cs_uint16 ether_type;
    LOOPBACK_DETECT_DEBUG(("\r\nloopbackFrameRevHandle Func IN! "));

    if ((len==0) || (frame==NULL)) 
    {
        return EPON_ONU_FRAME_HANDLED;	// No need handle
    }

    plb_frame = (cs_ether_header_lb *)frame;
    ether_type = ntohs(plb_frame->ethertype);
    if (ether_type == EPON_ETHERTYPE_DOT1Q) 
    {
        cs_ether_header_lb_vlan *plb_vlan_frame = (cs_ether_header_lb_vlan *)frame;
        vid = ntohs(plb_vlan_frame->vlan);
        vid &= 0x0FFF;
    }
    else
    {
    	vid = 1;
    }
    
    LOOPBACK_DETECT_DEBUG(("\r\n************************************ "));
    DUMPGWDPKT("\r\nLoopDetectFrameRevPKT : ", portid, frame, len);
    LOOPBACK_DETECT_DEBUG(("\r\n************************************ "));
    
    if(0 == lpbDetectRevPacketHandle(frame, len, 1, portid, vid))
        return EPON_ONU_FRAME_HANDLED;
    else
        return EPON_ONU_FRAME_NOT_HANDLED;
}
cs_uint32 alarm_led_start = 0;
cs_uint32 LED_TAXT = 1;
void onu_tmfunc_run_led(void*date)
{
	if(led_cout == 0)
		{
    		cs_gpio_write(13, (unsigned char)1);
			led_cout ++;
		}
	else
		{
    		cs_gpio_write(13, (unsigned char)0);
			led_cout = 0;
		}
		cs_timer_add(300, onu_tmfunc_run_led, NULL);
}

#if 0
void epon_onu_start_alarm_led()
{
	if(LED_STAT)
		{
			cs_printf("0\n");
			LED_STAT = 0;
			cs_printf("2\n");
			if ( CS_E_OK == cs_gpio_mode_set(13, GPIO_OUTPUT))
			{	
				cs_printf("3\n");
				onu_tmfunc_run_led();								
			}	
		}
}
#else
void epon_onu_start_alarm_led()
{	
	if(LED_TAXT)
		{
			LED_TAXT = 0;
			if ( CS_E_OK == cs_gpio_mode_set(13, GPIO_OUTPUT))
			{	
				alarm_led_start = 1;
				g_onu_alarm_led_timer = cs_timer_add(10, onu_tmfunc_run_led, NULL);	
			}	
		}
}
#endif
//static cs_ulong32 cs_int32 g_portloopstatus = 0;

void epon_onu_stop_alarm_led()
{
	if(alarm_led_start)
		{
			alarm_led_start = 0;
			LED_TAXT = 1;
			if ( CS_E_OK == cs_gpio_mode_set(13, GPIO_OUTPUT))
			{	
				if(g_onu_alarm_led_timer)
					{
						cs_timer_del(g_onu_alarm_led_timer);
					}
				cs_gpio_write(13, (unsigned char)0);								
			}
		}
	else
		{
			if ( CS_E_OK == cs_gpio_mode_set(13, GPIO_OUTPUT))
				{	
					if(g_onu_alarm_led_timer)
						{
							cs_timer_del(g_onu_alarm_led_timer);
						}
					cs_gpio_write(13, (unsigned char)0);								
				}
		}
}
void
onu_event_port_loop(gwd_ethloop_msg_t *msg)
{

//    g_portloopstatus |= 1<<msg->portid;
    epon_onu_start_alarm_led();
}

void
onu_event_port_loop_clear(gwd_ethloop_msg_t *msg)
{


    	epon_onu_stop_alarm_led();

}

#if 0
cs_int32 gwdEthPortLoopMsgBuildAndSend(cs_ulong32 cs_int32 portid, cs_ulong32 cs_int32 vid, cs_ulong32 cs_int32 status)
#else
cs_int32 gwdEthPortLoopMsgBuildAndSend(unsigned int status)
#endif
{
    gwd_ethloop_msg_t *msg = NULL;

    msg = iros_malloc(IROS_MID_APP, sizeof(gwd_ethloop_msg_t));
    if (NULL == msg) {
        IROS_LOG_CRI(IROS_MID_APP, "memory is not enough!\n");
        return (-1);
    }

    msg->msgtype = IROS_MSG_GWD_PORT_LOOP_EVENT;
#if 0
    msg->portid = portid;
    msg->vid = vid;
#endif
    msg->loopstatus = status;
    if (!cyg_mbox_tryput(m0, msg)) {
        IROS_LOG_CRI(IROS_MID_EVENT, "too many gwd port loop event!\n");
        iros_free(msg);
    }
    return 0;
}

cs_int32 gwdEthPortLoopLedAction()
{
	cs_int32 ret = GWD_RETURN_ERR;
	LPB_CTRL_LIST *pNode = g_lpb_detect_ctrl_head;

	OAM_ONU_LPB_DETECT_CTRL *pCtrl = NULL;

	cs_int32 found = 0;

	while(pNode != NULL)
	{
		pCtrl = pNode->ctrlnode;
		if(pCtrl)
		{
			cs_int32 iport = 0;
			for(iport=1; iport < NUM_PORTS_PER_SYSTEM; iport++)
			{
				if(pCtrl->lpbmask[iport])
				{
					found = 1;
					break;
				}
			}
			if(found)
				break;
		}
		pNode = pNode->next;
	}
	ret = gwdEthPortLoopMsgBuildAndSend(found?GWD_ETH_PORT_LOOP_ALARM:GWD_ETH_PORT_LOOP_ALARM_CLEAR);

	return ret;
}

/*end*/

