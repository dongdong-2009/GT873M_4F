/********************************************************************************
* switchInit.c
*
* DESCRIPTION:
*       Initialize Marvell switch driver and ports. 
*
* FILE REVISION NUMBER:
*       Liud 2007-05-18 Initial release  
*       Liud 2007-06-07 Reduced to only two basic functions.
*                       Other functions moved to switch_drv.c  
*       
*******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "iros_config.h"

/* Marvell driver API protetype files */
#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvConfig.h>
#include <gtDrvSwRegs.h>
#include <gtVct.h>
#include <gtSem.h>

#include "onu_specs.h"

#include "MARVELL_BSP_expo.h"
#include "switch_expo.h"
#include "switch_drv.h"
#include "cs_cmn.h"
#include "sdl_gpio.h"
#include <cyg/kernel/kapi.h>

/*
#define MULTI_ADDR_MODE
#define MANUAL_MODE
*/
#if (FOR_ONU_PON)
#define CTC_SWITCH_VERSION_MAJOR    1
#define CTC_SWITCH_VERSION_MINOR    7
#define CTC_SWITCH_VERSION_BUILD    3
#endif

#define GW_SWITCH_VERSION_MAJOR 1       
#define GW_SWITCH_VERSION_MINOR 1
#define GW_SWITCH_VERSION_BUILD 34

GT_QD_DEV       *qdMultiDev[N_OF_QD_DEVICES] = {0,};
GT_QD_DEV       *qdDev0 = NULL;
GT_QD_DEV       *qdDev1 = NULL;
GT_QD_DEV       *qdDev2 = NULL;
GT_QD_DEV		*pQdDev = NULL;

#if ((PRODUCT_CLASS == GT861_ONU)||(FOR_MRV_INDUSTRY_SW))
GT_SEM			smiAccessSem;
#endif 

GT_U32   ulMrvTagtype=0;

/* Globals */
/*GT_BOOL sg_switch_atu_used[MAX_ATU_NUM] = { GT_FALSE };*/

GT_SYS_CONFIG sg_switch_cfg;

GT_QD_DEV sg_switch_dev;

GT_QD_DEV sg_switch_dev0;
GT_QD_DEV sg_switch_dev1;

Marvell_ext_switch_conf_t sg_switch_bsp_config;

GT_BOOL sg_mc_strip_port[MAX_SWITCH_PORTS] = { GT_FALSE };
GT_32 sg_policing_cbs[MAX_SWITCH_PORTS] = { 0 };	/* Hold only CBS since EBS is constant and CIR is computed from Switch API */
GT_U8 cos_queue_number = 4;

GT_U8 dev_cpuPort[] = {DEVICE0_CPU_PORT, DEVICE1_CPU_PORT, DEVICE2_CPU_PORT, DEVICE3_CPU_PORT, DEVICE4_CPU_PORT};
GT_U8 dev_phyAddr[] = {DEVICE0_PHY_ADDR, DEVICE1_PHY_ADDR, DEVICE2_PHY_ADDR, DEVICE3_PHY_ADDR, DEVICE4_PHY_ADDR};
GT_U8 dev_wanPort[] = {DEVICE0_WAN_PORT, DEVICE1_WAN_PORT, DEVICE2_WAN_PORT, DEVICE3_WAN_PORT, DEVICE4_WAN_PORT};
GT_U8 dev_casPort[] = {DEVICE0_CASCADE_PORT, DEVICE1_CASCADE_PORT, DEVICE2_CASCADE_PORT, DEVICE3_CASCADE_PORT, DEVICE4_CASCADE_PORT};
#if (PRODUCT_CLASS == GT815)
#ifdef _FOR_GT861_TMP_
GT_U8 master_casPort[] = {DEVICE0_CASCADE_PORT0,DEVICE0_CASCADE_PORT1,DEVICE0_CASCADE_PORT2,DEVICE0_CASCADE_PORT3};
#else
GT_U8 master_casPort[] = {DEVICE0_CASCADE_PORT0};
#endif
#else
GT_U8 master_casPort[] = {DEVICE0_CASCADE_PORT0,DEVICE0_CASCADE_PORT1,DEVICE0_CASCADE_PORT2,DEVICE0_CASCADE_PORT3};
#endif
GT_U8 dev_id[] = {DEVICE0_ID, DEVICE1_ID, DEVICE2_ID, DEVICE3_ID, DEVICE4_ID, DEVICE4_ID};

GT_BOOL master_interswitch_port[MAXPortOnSlotNum];
unsigned int guiPhyBoardSmiAddr = 0;

extern char * malloc(int);
extern void free(void *);

int RevPkt_Marvel_Handler(char *org, char *out, int *len )
{
#if (FOR_MRV_INDUSTRY_SW)
        if (MRVTAG_HEAD == ulMrvTagtype)
        {
       		memcpy(out,org+2,*len-2);/*���ֽ�ͷ*/
               *len -= 2;
			return 0;		
			
        }
        else if (MRVTAG_TRAIL == ulMrvTagtype)
        {
       
			memcpy(out,org,*len-8);/*4 bytes for trailer, 4 bytes for fcs.*/
               *len -= 8;
			return 0;		
        }
		else if(MRVTAG_DSA == ulMrvTagtype)
		{/*add by szk 2012-7-19*/
		/*remove DSA_TAG (4 bytes) after SRC_MAC*/
			memcpy(out,org,12);
			memcpy(out+12,org+16,*len-16);
				*len -= 4;
			return 0;
		}/*end add 2012-7-19*/
        else
        {
       		return -1;
        }
#else
	return -1;
#endif

}
int SndPkt_Marvel_Handler(char *buf,int *len)
{
#if (FOR_MRV_INDUSTRY_SW)
	int i;
        if (MRVTAG_HEAD == ulMrvTagtype)
        {
        	for(i=*len-1;i>=0;i--)
			buf[i+2]=buf[i];	
          	 buf[0]=buf[1]=0;
		*len+=2;	 
			if(*len<64)
			{
				memset(&buf[*len],0, 64-*len);
				*len=64;
			}
		return 0;
        }
        else if (MRVTAG_TRAIL == ulMrvTagtype)
        {

			memset(&buf[*len],0, 4);
			*len+=4;
			if(*len<64)
			{
				memset(&buf[*len],0, 64-*len);
				*len=64;
			}
         	return 0;
        }
		else if(MRVTAG_DSA == ulMrvTagtype)
		{/*add by szk 2012-7-19*/
		/*add DSA_TAG (4 bytes) after SRC_MAC*/
			for(i = *len-1;i >= 12;i--)
			{
				buf[i+4] = buf[i];
			}
			memset(&buf[12],0,4);
			buf[12] = (0x03<<6)|(qdMultiDev[0]->deviceId<<0);/*Forward DSA*/
			buf[13] = qdMultiDev[0]->cpuPortNum<<3;
			buf[14] = 0x00;
			buf[15] = 0x01;
			*len+=4;
			if(*len<64)
			{
				memset(&buf[*len],0, 64-*len);
				*len=64;
			}
			return 0;
		}/*end add 2012-7-19*/
        else
        {
            return -1;
        }
#else
	return -1;
#endif
}

#if(FOR_MRV_INDUSTRY_SW)
void Marvell_Cpu_Port_Update( GT_QD_DEV * dev, GT_SYS_CONFIG * cfg)
{
	if((dev == NULL) ||(cfg == NULL))
		return;
    switch(dev->deviceId)
    {
		case GT_88E6083:
			dev->cpuPortNum = 8;
			cfg->cpuPortNum = 8;
			dev_cpuPort[0] = 8;
			break;
		case GT_88E6097:
			dev->cpuPortNum = 10;
			cfg->cpuPortNum = 10;
			dev_cpuPort[0] = 10;
			break;
		default:
			break;
    }
	return;
}
#endif



/**************************************************************************************
Name: allocate_bridge_mac

Description:
    Build a MAC address based on the prefix 00:0c:d5:64 and the last two byte of
    the MAC address configured to the EEPROM (parameter 'mac').

Returns: Nothing

Paramaters:
    mac (OUT):  The composed MAC address.
**************************************************************************************/
void allocate_bridge_mac(GT_ETHERADDR * mac)
{
	GT_ETHERADDR new_bridge_mac = { {0x00, 0x0C, 0xD5, 0x64, 0x00, 0x00} };
#if (FOR_MRV_ONU_PON)
	/* Change bridge mac LSB according to EEPROM MAC (allow some variation on EDKs) */
	/* Allocated MAC is - 00:0C:D5:64:XX:XX where XX:XX is taken from ONU MAC in EEPROM */
	GT_ETHERADDR new_bridge_mac = { {0x00, 0x0C, 0xD5, 0x64, 0x00, 0x00} };
	GT_ETHERADDR eeprom_mac;
	GT32  size = sizeof (eeprom_mac);
	int     result = PASONU_EMAPPER_get(EMAPPER_EEPROM_MAC_ADDR, &eeprom_mac, &size);

	if (result == S_OK)
	{
		new_bridge_mac.address[4] = eeprom_mac.address[4];
		new_bridge_mac.address[5] = eeprom_mac.address[5];
	}
	else
		MSG_OUT(("SWITCH: Failed to get MAC from eeprom (err %d)\r\n",result));
#elif (FOR_MRV_INDUSTRY_SW)
    GT_ETHERADDR new_bridge_mac = { {0} };

#ifndef _RELEASE_VERSION_
    extern void Get_sys_Mac_bsp(GT_ETHERADDR * mac);
    Get_sys_Mac_bsp(&new_bridge_mac);
#else
    extern void Get_sys_Mac(GT_ETHERADDR * mac);
    Get_sys_Mac(&new_bridge_mac);
#endif
#endif

	memcpy(&mac, &new_bridge_mac, sizeof (*mac));
}

void switch_reset()
{
    cs_callback_context_t context;
//    cs_uint8 status = 0;

//    cs_plat_gpio_read(context, 0, 0, 4, &status);
//    cs_printf("The status of GPIO 4 is %d\r\n",status);
    cs_plat_gpio_mode_set(context, 0, 0, 4, GPIO_MODE_OUTPUT);
    cs_plat_gpio_write(context, 0, 0, 4, 0);
//    cs_plat_gpio_read(context, 0, 0, 4, &status);
//    cs_printf("The status of GPIO 4 is %d\r\n",status);
    cyg_thread_delay(50);
    cs_plat_gpio_write(context, 0, 0, 4, 1);
//    cs_plat_gpio_read(context, 0, 0, 4, &status);
//    cs_printf("The status of GPIO 4 is %d\r\n",status);
	return;
}

extern void mrv_switch_init(IN  GT_QD_DEV    *dev);
/*******************************************************************************************
  Description   : This function will initialize the 88E6095 switch chip.
  In params     : None.
  Out params    : None.
  Return value  : None.
  NOTE			: For GT811/Gt812/GT815 only
********************************************************************************************/
void switch_init()
{
#if (FOR_ONU_PON)
	GT_STATUS          l_ret_val = GT_OK;
    GT_LPORT        i;
 	GT_ERATE_TYPE   gtEgressRateType;
	int j;
	GT_STATUS status;

	j=0;

	for(i=0; i<N_OF_QD_DEVICES; i++)
	{
		qdMultiDev[i] = (GT_QD_DEV*)malloc(sizeof(GT_QD_DEV));

		if(qdMultiDev[i] == NULL)
		{
			MSG_OUT(("SWITCH: malloc GT_QD_DEV failed!\r\n"));
			while(i--)
				free(qdMultiDev[i]);
			return;
		}

		memset((char*)qdMultiDev[i],0,sizeof(GT_QD_DEV));
	}

	allocate_bridge_mac(&(sg_switch_bsp_config.switch_mac));

	memset((char*)&sg_switch_cfg,0,sizeof(GT_SYS_CONFIG));

	for(i=0; i<N_OF_QD_DEVICES; i++)
	{

#if 0 /*((PRODUCT_CLASS == GT815) && (defined(_FOR_GT861_TMP_)))*/
		if(!UNIT_IS_MASTER(i))
		{
			extern int cpldRead(unsigned long add, unsigned char *pucValue);
			BOOL bUnitExist=GT_FALSE;
			unsigned char ucValue;

			cpldRead(1, &ucValue);
			bUnitExist = !(ucValue & (1<<(i-1)));
			if(!bUnitExist)
			{
				MSG_OUT(("SWITCH: Unit %d not exist.\r\n", i));
				continue;
			}
		}
#endif
	    sg_switch_cfg.cpuPortNum = dev_cpuPort[i];
		sg_switch_cfg.initPorts = GT_TRUE;	/* Set switch ports to Forwarding mode. If GT_FALSE, use Default Setting. */

#ifdef MANUAL_MODE	/* not defined. this is only for sample */
		/* user may want to use this mode when there are two QD switchs on the same MII bus. */
		sg_switch_cfg.mode.scanMode = SMI_MANUAL_MODE;	/* Use QD located at manually defined base addr */
		sg_switch_cfg.mode.baseAddr = 0x10;   /* valid value in this case is either 0 or 0x10 */
#else
#ifdef MULTI_ADDR_MODE
		sg_switch_cfg.mode.scanMode = SMI_MULTI_ADDR_MODE;	/* find a QD in indirect access mode */
		sg_switch_cfg.mode.baseAddr = dev_phyAddr[i];		/* this is the phyAddr used by QD family device. 
									                           Valid value are 1 ~ 31.*/
#else
		sg_switch_cfg.mode.scanMode = SMI_AUTO_SCAN_MODE;	/* Scan 0 or 0x10 base address to find the QD */
		sg_switch_cfg.mode.baseAddr = 0;
#endif
#endif /* MANUAL_MODE */

#if (PRODUCT_CLASS == GT815)
		get_switch_cascade_port_info(i, &(sg_switch_bsp_config.numOfCasPorts), ports);
		for(j=0;j<sg_switch_bsp_config.numOfCasPorts;j++)
		{
			sg_switch_bsp_config.cas_ports[j].port = ports[j];
			sg_switch_bsp_config.cas_ports[j].devid = dev_id[j+1];
		}
#else
		sg_switch_bsp_config.numOfCasPorts = 0;
#endif
		sg_switch_bsp_config.upLinkPort = dev_wanPort[i];

		if ((status=Marvell_driver_initialize(&sg_switch_cfg, qdMultiDev[i], &sg_switch_bsp_config)) != GT_OK)
		{
			MSG_OUT(("SWITCH: Marvell BSP init failed(phyAddress = %d)\r\n", dev_phyAddr[i]));
			l_ret_val = status;
			continue;
		}
		else
		{
			MSG_OUT(("SWITCH: %s switch BSP init success\r\n",(i==0)?"Master":"Slave"));
			if(i == 0)
			{
#if (FOR_ONU_PON)
				MSG_OUT(("Setting GE WAN Port(%d)...\r\n", dev_wanPort[i]));
				if((status = switch_ge_port_init(qdMultiDev[i],dev_wanPort[i])) != GT_OK)
				{
					MSG_OUT(("Config master device wan port failed(%d).\r\n", status));
					l_ret_val = status;
				}
#endif
#if 0
				mrv_switch_init(qdMultiDev[i]);
				while(1);
#endif
#if 0
/*(FOR_MRV_INDUSTRY_SW)*/
			/*The Cpu port number should associate with the chip type, update here */
            Marvell_Cpu_Port_Update(qdMultiDev[i], &sg_switch_cfg);
#endif
			if (IS_IN_DEV_GROUP(qdMultiDev[i], DEV_88E6097_FAMILY))
			{
#if(PRODUCT_CLASS != PRODUCTS_GT812C)
#if (!(FOR_MRV_INDUSTRY_SW))
				MSG_OUT(("Setting FE CPU Port(%d)...\r\n", dev_cpuPort[i]));
				if((status = switch_fe_port_init(qdMultiDev[i],dev_cpuPort[i],GT_TRUE)) != GT_OK)
				{
					MSG_OUT(("Config master device cpu port failed(%d).\r\n", status));
					l_ret_val = status;
				}
#endif
#endif

#if (PRODUCT_CLASS == GT815)
				MSG_OUT(("Setting GE Cascade Port(%d)...\r\n", dev_casPort[i]));
				if((status = switch_ge_port_init(qdMultiDev[i],dev_casPort[i])) != GT_OK)
				{
					MSG_OUT(("Config master device wan port failed(%d).\r\n", status));
					l_ret_val = status;
				}
#endif
			}
#if ((defined _GT831_V1_1_ON_GT811_) && (defined _CPU_CHANNEL_USE_HMII_))
				MSG_OUT(("Setting HostMII Port(%d)...\r\n", 9));
				if((status = switch_fe_port_init(qdMultiDev[i],9,GT_TRUE)) != GT_OK)
				{
					MSG_OUT(("Config master device HostMII port failed(%d).\r\n", status));
					l_ret_val = status;
				}
#endif
			}
			else
			{
#if (PRODUCT_CLASS == GT815)
				MSG_OUT(("Setting GE WAN/Cascade Port(%d)...\r\n", dev_wanPort[i]));
				if((status = switch_ge_port_init(qdMultiDev[i],dev_wanPort[i])) != GT_OK)
				{
					MSG_OUT(("Config slave device wan port failed(%d).\r\n", status));
					l_ret_val = status;
				}
#endif
			}
		}

		/* 
			Now, we need to configure Cascading information for each devices.
			1. Set Interswitch port mode for port 8 and 9 for device 0,1,and 2,
				so that switch device can expect Marvell Tag from frames 
				ingressing/egressing this port.
			2. Set CPU Port information (for To_CPU frame) for each port of device.
			3. Set Cascading Port information (for From_CPU fram) for each device.
			4. Set Device ID (if required)
				Note: DeviceID is hardware configurable.
		*/

		/*
			1. Set Interswitch port mode for port 8 and 9 for device 0,1,and 2,
				so that switch device can expect Marvell Tag from frames 
				ingressing/egressing this port.
			2. Set CPU Port information (for To_CPU frame) for each port of device.
		*/			
#if (PRODUCT_CLASS == PRODUCTS_GT812C)

#else  	/* 6096/6097 don't need set cpu port to every user port */
		for(j=0; j<qdMultiDev[i]->numOfPorts; j++)
		{
#if 0
			if(/*(j == dev_cpuPort[i]) || */(j == dev_casPort[i]))
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[i],j,GT_TRUE)) != GT_OK)
				{
					MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_TRUE)\r\n",status,j));
					l_ret_val = status;
					break;
				}
			}
			else
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[i],j,GT_FALSE)) != GT_OK)
				{
					MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_FALSE)\r\n",status,j));
					l_ret_val = status;
					break;
				}
			}
#endif

#if (!(FOR_MRV_INDUSTRY_SW))
			if( j != dev_cpuPort[i])
			{
                if (IS_IN_DEV_GROUP(qdMultiDev[i],DEV_CPU_DEST_PER_PORT))
                {
    				if((status=gprtSetCPUPort(qdMultiDev[i],j,dev_cpuPort[i])) != GT_OK)
    				{
    					MSG_OUT(("gprtSetCPUPort returned %d\r\n",status));
    					l_ret_val = status;
    					break;
    				}
                }
			}
#endif
		}

#if 0
		/*
			3. Set Cascading Port information (for From_CPU fram) for each device.
		*/	 	
		MSG_OUT(("Setting Cascade Port...\r\n"));
		if((status=gsysSetCascadePort(qdMultiDev[i],dev_casPort[i])) != GT_OK)
		{
			MSG_OUT(("gsysSetCascadePort returned %d\r\n",status));
			l_ret_val = status;
			continue;
		}
#endif
#endif /* _SUPPORT_GT815_ */


        if (IS_IN_DEV_GROUP(qdMultiDev[i],DEV_GIGABIT_SWITCH))
        {
    		/*
    			4. Set Device ID (if required)
    		*/	 	
    		MSG_OUT(("Setting Device ID (%d)...\r\n",dev_id[i]));
    		if((status=gsysSetDeviceNumber(qdMultiDev[i],dev_id[i])) != GT_OK)
    		{
    			MSG_OUT(("gsysSetDeviceNumber returned %d\r\n",status));
    			l_ret_val = status;
    			continue;
    		}
        }
		/*
			5. Set Switch Register to default value (if required)
		*/	 	
		if(GT_OK != switch_default_config(qdMultiDev[i]))
		{
			l_ret_val = status;
			continue;
		}
#if (!(FOR_MRV_INDUSTRY_SW))
		if(dev_cpuPort[i] != dev_wanPort[i])
		{
			MSG_OUT(("Set CPU port(%d) Egress rate to 64kbps...\r\n",dev_cpuPort[i]));
			/* Egress 64kbps */
			if (IS_IN_DEV_GROUP(qdMultiDev[i],DEV_ELIMIT_FRAME_BASED))
				gtEgressRateType.kbRate = 64;
           	 	else if (!IS_IN_DEV_GROUP(qdMultiDev[i],DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
                		gtEgressRateType.definedRate = GT_128K;
			else
				gtEgressRateType.definedRate = GT_64K;
	        	if ((status = grcSetEgressRate(qdMultiDev[i], dev_cpuPort[i], &gtEgressRateType)) != GT_OK)
	    		{
		    		MSG_OUT(( "grcSetEgressRate return Failed(%d)\r\n", status));
		    		l_ret_val = status;
	    		}
			/*
			 * Set Egress Flood Mode to  Block Unknown DA on CPU Port. Libl,2012.3
			*/
			if (IS_IN_DEV_GROUP(qdMultiDev[i], DEV_EGRESS_FLOOD))
			{
				if((status = gprtSetEgressFlood(qdMultiDev[i], dev_cpuPort[i], GT_BLOCK_EGRESS_UNKNOWN)) != GT_OK)
				{
					MSG_OUT(("gprtSetEgressFlood return Failed(%d)\r\n", status));
					l_ret_val = status;
				}
			}
			else if(IS_IN_DEV_GROUP(qdMultiDev[i], DEV_PORT_SECURITY))
			{
				if((status = gprtSetForwardUnknown(qdMultiDev[i], dev_cpuPort[i], GT_FALSE)) != GT_OK)
				{
					MSG_OUT(("gprtSetForwardUnknown return Failed(%d)\r\n", status));
					l_ret_val = status;
				}
			}
			
		}
#endif
	       /*wugang add 2011.6.3, disable all the interrupts for init..*/
	       eventSetActive(qdMultiDev[i], 0);
	}

	if(GT_OK == l_ret_val)
	{
		MSG_OUT(("\r\nSWITCH: The switch was initialized successfully\r\n"));
		MSG_OUT(("SWITCH: Marvell DSDT Release version %s \r\n", MSAPI_VERSION));
		MSG_OUT(("SWITCH: GW Implementation Version %d.%d(Build %d on %s)\r\n",
			GW_SWITCH_VERSION_MAJOR, GW_SWITCH_VERSION_MINOR, GW_SWITCH_VERSION_BUILD+7, __DATE__));
	}

	gulTypeAll  = BUCKET_TRAFFIC_BROADCAST |
		          BUCKET_TRAFFIC_UNKNOWN_MULTICAST |
		          BUCKET_TRAFFIC_MULTICAST |
		          BUCKET_TRAFFIC_UNICAST   |
		          BUCKET_TRAFFIC_MGMT_FRAME|
		          BUCKET_TRAFFIC_UNKNOWN_UNICAST;

	gulTypeFlood = BUCKET_TRAFFIC_BROADCAST |
		          BUCKET_TRAFFIC_UNKNOWN_MULTICAST |
		          BUCKET_TRAFFIC_MULTICAST |
		          BUCKET_TRAFFIC_UNKNOWN_UNICAST;

	gulTypeMc   = BUCKET_TRAFFIC_UNKNOWN_MULTICAST |
		          BUCKET_TRAFFIC_MULTICAST;

	gulTypeBc   = BUCKET_TRAFFIC_BROADCAST;

	gulTypeUc   = BUCKET_TRAFFIC_UNKNOWN_UNICAST;

	return;
#else
	extern GT_BOOL boards_unit_to_slot(GT_U32 unit, GT_U8 *slot);
	extern int device_chassis_is_slot_inserted( int slotno );
	extern int device_chassis_get_slot_type( int slotno );

	/*smiAccessSem = semBCreate(0, 1);*/

	switch_master_init(0);

	gulTypeAll  = BUCKET_TRAFFIC_BROADCAST |
		          BUCKET_TRAFFIC_MULTICAST |
		          BUCKET_TRAFFIC_UNICAST   |
		          BUCKET_TRAFFIC_MGMT_FRAME|
		          BUCKET_TRAFFIC_UNKNOWN_UNICAST;

	gulTypeFlood= BUCKET_TRAFFIC_BROADCAST |
		          BUCKET_TRAFFIC_MULTICAST |
		          BUCKET_TRAFFIC_UNKNOWN_UNICAST;

	gulTypeMc   = BUCKET_TRAFFIC_UNKNOWN_MULTICAST |
		          BUCKET_TRAFFIC_MULTICAST;

	gulTypeBc   = BUCKET_TRAFFIC_BROADCAST;

	gulTypeUc   = BUCKET_TRAFFIC_UNKNOWN_UNICAST;
	return;
#endif
}

/*******************************************************************************************
  Description   : This function will initialize the ports by product.
  In params     : None
  Out params    : None.
  Return value  : None.
  NOTE			: None.
********************************************************************************************/
void port_init_by_product(void)
{
#if(FOR_MRV_INDUSTRY_SW)
#if NEW_PRODUCT
#warning "maybe you need to init some ports here."
#endif
	GT_U32 unit = 0;
	int retVal = 0,j=0;
	GT_U8 cpuPortNum = b_product_info->b_l2_2_phy_map[0].port;

	sg_switch_cfg.cpuPortNum = cpuPortNum;
	QD_DEV_PTR->cpuPortNum = cpuPortNum;
	dev_cpuPort[unit] = cpuPortNum;

	if (IS_IN_DEV_GROUP(QD_DEV_PTR, DEV_88E6097_FAMILY))
		switch_fe_port_init(QD_DEV_PTR,cpuPortNum,GT_TRUE);
	for(j=0;j<QD_DEV_PTR->numOfPorts;j++)
	{
		if( j != cpuPortNum)
		{
            if (IS_IN_DEV_GROUP(QD_DEV_PTR,DEV_CPU_DEST_PER_PORT))
            {
				if((retVal=gprtSetCPUPort(QD_DEV_PTR,j,cpuPortNum)) != GT_OK)
				{
					MSG_OUT(("gprtSetCPUPort returned %d\r\n",retVal));
					break;
				}
            }
		}
	}
	{
		GT_ERATE_TYPE   gtEgressRateType;
		MSG_OUT(("Set CPU port(%d) Egress rate to 64kbps...\r\n",cpuPortNum));
		/* Egress 64kbps */
		if (IS_IN_DEV_GROUP(QD_DEV_PTR,DEV_ELIMIT_FRAME_BASED))
			gtEgressRateType.kbRate = 64;
		else if (!IS_IN_DEV_GROUP(QD_DEV_PTR,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
		    gtEgressRateType.definedRate = GT_128K;
		else
			gtEgressRateType.definedRate = GT_64K;

		if ((retVal = grcSetEgressRate(QD_DEV_PTR,cpuPortNum, &gtEgressRateType)) != GT_OK)
		{
			MSG_OUT(( "grcSetEgressRate return Failed(%d)\r\n", retVal));
		}
		/*
		 * Set Egress Flood Mode to  Block Unknown DA on CPU Port. Libl,2012.3
		*/
		if (IS_IN_DEV_GROUP(QD_DEV_PTR, DEV_EGRESS_FLOOD))
		{
			if((retVal = gprtSetEgressFlood(QD_DEV_PTR,cpuPortNum, GT_BLOCK_EGRESS_UNKNOWN)) != GT_OK)
			{
				MSG_OUT(("gprtSetEgressFlood return Failed(%d)\r\n", retVal));
			}
		}
		else if(IS_IN_DEV_GROUP(QD_DEV_PTR, DEV_PORT_SECURITY))
		{
			if((retVal = gprtSetForwardUnknown(QD_DEV_PTR,cpuPortNum, GT_FALSE)) != GT_OK)
			{
				MSG_OUT(("gprtSetForwardUnknown return Failed(%d)\r\n", retVal));
			}
		}
	}
	
	if(IS_IN_DEV_GROUP(QD_DEV_PTR, DEV_88E6083))
	{
        if( (retVal = gtSetCpuFrameMode(0, MRVTAG_TRAIL)) != VOS_OK)
			sys_console_printf("Set the gtSetCpuFrameMode failed (return %d)", retVal);
	}
	else if(IS_IN_DEV_GROUP(QD_DEV_PTR, DEV_88E6097))
	{
        int i;
        unsigned long phy_port;
		int unit =0;
        for (i = 1; i <=gulNumOfPortsPerSystem; i++) 
        {
            boards_logical_to_physical(i, &unit, &phy_port);
        	if(ETH_UR_TYPE_FSM == b_product_info->pL2Type[i])
        	{
        		if ((retVal = gprtSetPortAutoMode(QD_DEV_PTR, phy_port, SPEED_AUTO_DUPLEX_AUTO)) != GT_OK)
		    	{
		    		sys_console_printf( "gprtSetPortAutoMode return Failed(%d,%d)\r\n", phy_port,retVal);
		    	}
				if ((retVal = gpcsSetForceSpeed(QD_DEV_PTR, phy_port, PORT_FORCE_SPEED_100_MBPS)) != GT_OK)
				{
					sys_console_printf( "gpcsSetForceSpeed return Failed(%d)\r\n", retVal);
				}
				if ((retVal = gpcsSetDpxValue(QD_DEV_PTR, phy_port, GT_TRUE)) != GT_OK)
				{
					sys_console_printf( "gpcsSetDpxValue return Failed(%d)\r\n", retVal);
				}
				if ((retVal = gpcsSetForcedDpx(QD_DEV_PTR, phy_port, GT_TRUE)) != GT_OK)
				{
					sys_console_printf( "gpcsSetForcedDpx return Failed(%d)\r\n", retVal);
				}
        	}
			else if(ETH_UR_TYPE_SFP == b_product_info->pL2Type[i])
			{
				if ((retVal = gpcsSetForceSpeed(QD_DEV_PTR, phy_port, PORT_FORCE_SPEED_1000_MBPS)) != GT_OK)
				{
					sys_console_printf( "gpcsSetForceSpeed return Failed(%d)\r\n", retVal);
				}
				if ((retVal = gpcsSetDpxValue(QD_DEV_PTR, phy_port, GT_TRUE)) != GT_OK)
				{
					sys_console_printf( "gpcsSetDpxValue return Failed(%d)\r\n", retVal);
				}
				if ((retVal = gpcsSetForcedDpx(QD_DEV_PTR, phy_port, GT_TRUE)) != GT_OK)
				{
					sys_console_printf( "gpcsSetForcedDpx return Failed(%d)\r\n", retVal);
				}
				if (gprtSetMGMII(QD_DEV_PTR, phy_port, GT_TRUE) != VOS_OK)
				{
					sys_console_printf( "gprtSetMGMII return Failed\r\n");
				}
			}
        }
        if((retVal = gtSetCpuFrameMode(0, MRVTAG_DSA)) != VOS_OK)
			sys_console_printf("Set the gtSetCpuFrameMode failed (return %d)", retVal);
	}
	else
	{
		sys_console_printf("Get the wrong chip type on port_init_by_product\r\n");
	}

#endif
}

/*******************************************************************************************
  Description   : This function will initialize switch chip.
  In params     : unit	- switch unit id.
  Out params   : None.
  Return value  : None
  NOTE			: For product used mrv switch chip.
********************************************************************************************/
GT_STATUS switch_slave_init(GT_U32 unit)
{
	GT_STATUS	l_ret_val = GT_OK;
	GT_STATUS status;
	int		j;
	GT_LPORT	phyPort;

	if(qdMultiDev[unit] == NULL)
	{
		qdMultiDev[unit] = (GT_QD_DEV*)malloc(sizeof(GT_QD_DEV));

		if(qdMultiDev[unit] == NULL)
		{
			MSG_OUT(("SWITCH: malloc GT_QD_DEV failed!\r\n"));
			return GT_FAIL;
		}
	}

	memset((char*)qdMultiDev[unit],0,sizeof(GT_QD_DEV));
	memset((char*)&sg_switch_cfg,0,sizeof(GT_SYS_CONFIG));

#ifdef _RELEASE_VERSION_
#if (PRODUCT_CLASS == GT861_ONU)
	redirect_smi_channel(GT_TRUE);
#endif
#endif

    sg_switch_cfg.cpuPortNum = dev_cpuPort[unit];
	sg_switch_cfg.initPorts = GT_TRUE;	/* Set switch ports to Forwarding mode. If GT_FALSE, use Default Setting. */

#ifdef MANUAL_MODE	/* not defined. this is only for sample */
	/* user may want to use this mode when there are two QD switchs on the same MII bus. */
	sg_switch_cfg.mode.scanMode = SMI_MANUAL_MODE;	/* Use QD located at manually defined base addr */
	sg_switch_cfg.mode.baseAddr = 0x10;   /* valid value in this case is either 0 or 0x10 */
#else
#ifdef MULTI_ADDR_MODE
	sg_switch_cfg.mode.scanMode = SMI_MULTI_ADDR_MODE;	/* find a QD in indirect access mode */
	sg_switch_cfg.mode.baseAddr = dev_phyAddr[unit];		/* this is the phyAddr used by QD family device. 
								                           Valid value are 1 ~ 31.*/
#else
	sg_switch_cfg.mode.scanMode = SMI_AUTO_SCAN_MODE;	/* Scan 0 or 0x10 base address to find the QD */
	sg_switch_cfg.mode.baseAddr = 0;
#endif	/* MULTI_ADDR_MODE */
#endif 	/* MANUAL_MODE */

	sg_switch_bsp_config.cas_ports[0].port = dev_casPort[unit];
	sg_switch_bsp_config.numOfCasPorts = 1;
	sg_switch_bsp_config.upLinkPort = dev_wanPort[unit];

	if ((status=Marvell_driver_initialize(&sg_switch_cfg, qdMultiDev[unit], &sg_switch_bsp_config)) != GT_OK)
	{
		MSG_OUT(("SWITCH: Slave %lu Marvell BSP init failed\r\n", unit));
		return status;
	}
	else
	{
		MSG_OUT(("SWITCH: Slave %lu switch BSP init success\r\n", unit));
	}

	MSG_OUT(("Setting FE WAN/Cascade Port(%d)...\r\n", dev_wanPort[unit]));
	if((status = switch_fe_port_init(qdMultiDev[unit],dev_wanPort[unit],GT_FALSE)) != GT_OK)
	{
		MSG_OUT(("Config slave device wan port failed(%d).\r\n", status));
		l_ret_val = status;
	}
	MSG_OUT(("Setting Master FE WAN/Cascade Port(%d)...\r\n", master_casPort[unit-1]));
	if((status = switch_fe_port_init(qdMultiDev[MASTER_UNIT],master_casPort[unit-1],GT_FALSE)) != GT_OK)
	{
		MSG_OUT(("Config master device cascade port failed(%d).\r\n", status));
		l_ret_val = status;
	}
	
	/* 
		Now, we need to configure Cascading information for each devices.
		1. Set Interswitch port mode for port 8 and 9 for device 0,1,and 2,
			so that switch device can expect Marvell Tag from frames 
			ingressing/egressing this port.
		2. Set CPU Port information (for To_CPU frame) for each port of device.
		3. Set Cascading Port information (for From_CPU fram) for each device.
		4. Set Device ID (if required)
			Note: DeviceID is hardware configurable.
	*/

	/*
		1. Set Interswitch port mode for cascade port,
			so that switch device can expect Marvell Tag from frames 
			ingressing/egressing this port.
		2. Set CPU Port information (for To_CPU frame) for each port of device.
	*/			
	/* 6096/6097 don't need set cpu port to every user port */
	if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_CPU_DEST))
	{
		MSG_OUT(("Setting CPU Port(%d) to DSA mode...\r\n",dev_cpuPort[unit]));
		if((status=gsysSetCPUDest(qdMultiDev[unit],dev_cpuPort[unit])) != GT_OK)
		{
			MSG_OUT(("gsysSetCPUDest returned %d (port %d)\r\n",status,dev_cpuPort[unit]));
			l_ret_val = status;
		}

		if((status=gprtSetFrameMode(qdMultiDev[unit],dev_cpuPort[unit], GT_FRAME_MODE_DSA)) != GT_OK)
		{
			MSG_OUT(("gprtSetFrameMode returned %d (port %d, DSA_MODE)\r\n",status,dev_cpuPort[unit]));
			l_ret_val = status;
		}

		if(GT_OK == get_switch_cascade_port_on_master(unit, &phyPort))
		{
			MSG_OUT(("Setting InterSwitch Port(%lu) on master to DSA mode...\r\n",phyPort));
			if (IS_IN_DEV_GROUP(qdMultiDev[0],DEV_FRAME_MODE))
			{
				if((status=gprtSetFrameMode(qdMultiDev[0],phyPort,GT_FRAME_MODE_DSA)) != GT_OK)
				{
					MSG_OUT(("gprtSetFrameMode returned %d (port %lu, mode GT_FRAME_MODE_DSA)\r\n",status,phyPort));
					l_ret_val = status;
				}
			}
			else
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[0],phyPort,GT_TRUE)) != GT_OK)
				{
					MSG_OUT(("gprtSetInterswitchPort returned %d (port %lu, mode GT_TRUE)\r\n",status,phyPort));
					l_ret_val = status;
				}
			}
		}
		
		if(dev_casPort[unit] != dev_cpuPort[unit])
		{
			MSG_OUT(("Setting cascade Port(%d)...\r\n",dev_casPort[unit]));
			if((status=gprtSetFrameMode(qdMultiDev[unit],dev_casPort[unit], GT_FRAME_MODE_DSA)) != GT_OK)
			{
				MSG_OUT(("gprtSetFrameMode returned %d (port %d, DSA_MODE)\r\n",status,dev_casPort[unit]));
				l_ret_val = status;
			}
		}
	}
	else  	
	{
		MSG_OUT(("Setting CPU Port(%d) and cascade port(%d)...\r\n",dev_cpuPort[unit], dev_casPort[unit]));
		for(j=0; j<qdMultiDev[unit]->numOfPorts; j++)
		{
			if((j == dev_cpuPort[unit]) || (j == dev_casPort[unit]))
			{
				if(dev_cpuPort[0] != dev_wanPort[0])
				{
					MSG_OUT(("Setting InterSwitch Port(%d) to DSA mode...\r\n",j));
					if((status=gprtSetInterswitchPort(qdMultiDev[unit],j,GT_TRUE)) != GT_OK)
					{
						MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_TRUE)\r\n",status,j));
						l_ret_val = status;
						break;
					}

					if(GT_OK == get_switch_cascade_port_on_master(unit, &phyPort))
					{
						MSG_OUT(("Setting InterSwitch Port(%lu) on master to DSA mode...\r\n",phyPort));
						master_interswitch_port[phyPort] = GT_TRUE;
						if (IS_IN_DEV_GROUP(qdMultiDev[0],DEV_FRAME_MODE))
						{
							if((status=gprtSetFrameMode(qdMultiDev[0],phyPort,GT_FRAME_MODE_DSA)) != GT_OK)
							{
								MSG_OUT(("gprtSetFrameMode returned %d (port %lu, mode GT_FRAME_MODE_DSA)\r\n",status,phyPort));
								l_ret_val = status;
							}
						}
						else
						{
							if((status=gprtSetInterswitchPort(qdMultiDev[0],phyPort,GT_TRUE)) != GT_OK)
							{
								MSG_OUT(("gprtSetInterswitchPort returned %d (port %lu, mode GT_TRUE)\r\n",status,phyPort));
								l_ret_val = status;
							}
						}
					}
				}
			}
			else
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[unit],j,GT_FALSE)) != GT_OK)
				{
					MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_FALSE)\r\n",status,j));
					l_ret_val = status;
					break;
				}
			}
			if(j != dev_cpuPort[unit])
			{
				if((status=gprtSetCPUPort(qdMultiDev[unit],j,dev_cpuPort[unit])) != GT_OK)
				{
					MSG_OUT(("gprtSetCPUPort returned %d\r\n",status));
					l_ret_val = status;
					break;
				}
			}
		}
	}

	/*
		3. Set Cascading Port information (for From_CPU fram) for each device.
	*/	 	
	if(dev_cpuPort[unit] != dev_casPort[unit])
	{
		if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_CASCADE_PORT))
		{
			MSG_OUT(("Setting Cascade Port(%d)...\r\n", dev_casPort[unit]));
			if((status=gsysSetCascadePort(qdMultiDev[unit],dev_casPort[unit])) != GT_OK)
			{
				MSG_OUT(("gsysSetCascadePort returned %d\r\n",status));
				l_ret_val = status;
			}
		}
	}
	else
	{
		if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_CASCADE_PORT))
		{
			MSG_OUT(("Setting Cascade Port(%d)...\r\n", 0xE));
			if((status=gsysSetCascadePort(qdMultiDev[unit],0xE)) != GT_OK)
			{
				MSG_OUT(("gsysSetCascadePort returned %d\r\n",status));
				l_ret_val = status;
			}
		}
	}

	/*
		4. Set Device ID (if required)
	*/	 	
	MSG_OUT(("Setting Device ID (%d)...\r\n",dev_id[unit]));
	if((status=gsysSetDeviceNumber(qdMultiDev[unit],dev_id[unit])) != GT_OK)
	{
		MSG_OUT(("gsysSetDeviceNumber returned %d\r\n",status));
		l_ret_val = status;
	}
	/*
		5. Set Switch Register to default value (if required)
	*/	 	
	MSG_OUT(("Setting switch default parameters...\r\n"));
	if((status= switch_default_config(qdMultiDev[unit])) != GT_OK)
	{
		MSG_OUT(("switch_default_config returned %d\r\n",status));
		l_ret_val = status;
	}
#if 0
	if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_FLOOD_BROADCAST))
	{
		if((status=MinimizeCPUTraffic2(qdMultiDev[unit], NULL)) != GT_OK)
		{
			MSG_OUT(("MinimizeCPUTraffic2 returned %d\r\n",status));
			l_ret_val = status;
		}
	}
#endif	
	return l_ret_val;
}
GT_STATUS switch_slave_destroy(GT_U32 unit)
{
	GT_STATUS	l_ret_val = GT_OK;
	GT_STATUS status;
	GT_LPORT	phyPort;

	if(GT_OK == get_switch_cascade_port_on_master(unit, &phyPort))
	{
		MSG_OUT(("Setting InterSwitch Port(%lu) on master to NORMAL mode...\r\n",phyPort));
		if (IS_IN_DEV_GROUP(qdMultiDev[0],DEV_FRAME_MODE))
		{
			if((status=gprtSetFrameMode(qdMultiDev[0],phyPort,GT_FRAME_MODE_NORMAL)) != GT_OK)
			{
				MSG_OUT(("gprtSetFrameMode returned %d (port %lu, mode GT_FRAME_MODE_NORMAL)\r\n",status,phyPort));
				l_ret_val = status;
			}
		}
		else
		{
			if((status=gprtSetInterswitchPort(qdMultiDev[0],phyPort,GT_FALSE)) != GT_OK)
			{
				MSG_OUT(("gprtSetInterswitchPort returned %d (port %lu, mode GT_FALSE)\r\n",status,phyPort));
				l_ret_val = status;
			}
		}
	}

	/*qdMultiDev[unit] = NULL;*/

	return l_ret_val;

}
/*******************************************************************************************
  Description   : This function will initialize the 88E6095 switch chip.
  In params     : unit	- switch unit id.
  Out params    : None.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS switch_master_init(GT_U32 unit)
{
	GT_STATUS	l_ret_val = GT_OK;
	int j;
	GT_STATUS status;

	if(qdMultiDev[unit] == NULL)
	{
		qdMultiDev[unit] = (GT_QD_DEV*)malloc(sizeof(GT_QD_DEV));

		if(qdMultiDev[unit] == NULL)
		{
			MSG_OUT(("SWITCH: malloc GT_QD_DEV failed!\r\n"));
			return GT_FAIL;
		}
	}

	memset(master_interswitch_port,0,sizeof(master_interswitch_port));
	memset((char*)qdMultiDev[unit],0,sizeof(GT_QD_DEV));
	memset((char*)&sg_switch_cfg,0,sizeof(GT_SYS_CONFIG));

#ifdef _RELEASE_VERSION_
#if (PRODUCT_CLASS == GT861_ONU)
	if(redirect_smi_channel(GT_TRUE) != GW_OK)
	{
		MSG_OUT(("SWITCH: Redirect smi channel for switch failed!\r\n"));
		return GT_FAIL;
	}
#endif
#endif
    sg_switch_cfg.cpuPortNum = dev_cpuPort[unit];
	sg_switch_cfg.initPorts = GT_TRUE;	/* Set switch ports to Forwarding mode. If GT_FALSE, use Default Setting. */

#ifdef MANUAL_MODE	/* not defined. this is only for sample */
	/* user may want to use this mode when there are two QD switchs on the same MII bus. */
	sg_switch_cfg.mode.scanMode = SMI_MANUAL_MODE;	/* Use QD located at manually defined base addr */
	sg_switch_cfg.mode.baseAddr = 0x10;   /* valid value in this case is either 0 or 0x10 */
#else
#ifdef MULTI_ADDR_MODE
	sg_switch_cfg.mode.scanMode = SMI_MULTI_ADDR_MODE;	/* find a QD in indirect access mode */
	sg_switch_cfg.mode.baseAddr = dev_phyAddr[unit];		/* this is the phyAddr used by QD family device. 
								                           Valid value are 1 ~ 31.*/
#else
	sg_switch_cfg.mode.scanMode = SMI_AUTO_SCAN_MODE;	/* Scan 0 or 0x10 base address to find the QD */
	sg_switch_cfg.mode.baseAddr = 0;
#endif	/* MULTI_ADDR_MODE */
#endif 	/* MANUAL_MODE */

	sg_switch_bsp_config.numOfCasPorts = sizeof(master_casPort);
	for(j=0; j<sg_switch_bsp_config.numOfCasPorts; j++)
	{
		sg_switch_bsp_config.cas_ports[j].port = master_casPort[j];
		sg_switch_bsp_config.cas_ports[j].devid = dev_id[j+1];
	}
	sg_switch_bsp_config.upLinkPort = dev_wanPort[unit];

	if ((status=Marvell_driver_initialize(&sg_switch_cfg, qdMultiDev[unit], &sg_switch_bsp_config)) != GT_OK)
	{
		MSG_OUT(("SWITCH: Master %lu Marvell BSP init failed\r\n", unit));
		l_ret_val = status;
	}
	else
	{
		MSG_OUT(("SWITCH: Master %lu switch BSP init success\r\n", unit));
	}

	MSG_OUT(("Setting uplink giga Port(%d)...\r\n",dev_wanPort[unit]));
	if((status = switch_ge_port_init(qdMultiDev[unit],dev_wanPort[unit])) != GT_OK)
	{
		MSG_OUT(("Config master device wan port failed(%d).\r\n", status));
		l_ret_val = status;
	}
	/* 
		Now, we need to configure Cascading information for each devices.
		1. Set Interswitch port mode for port 8 and 9 for device 0,1,and 2,
			so that switch device can expect Marvell Tag from frames 
			ingressing/egressing this port.
		2. Set CPU Port information (for To_CPU frame) for each port of device.
		3. Set Cascading Port information (for From_CPU fram) for each device.
		4. Set Device ID (if required)
			Note: DeviceID is hardware configurable.
	*/

	/*
		1. Set Interswitch port mode for cascade port,
			so that switch device can expect Marvell Tag from frames 
			ingressing/egressing this port.
		2. Set CPU Port information (for To_CPU frame) for each port of device.
	*/			
	/* 6096/6097 don't need set cpu port to every user port */
	if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_CPU_DEST))
	{
		MSG_OUT(("Setting CPU Port(%d)...\r\n",dev_cpuPort[unit]));
		if((status=gsysSetCPUDest(qdMultiDev[unit],dev_cpuPort[unit])) != GT_OK)
		{
			MSG_OUT(("gsysSetCPUDest returned %d (port %d)\r\n",status,dev_cpuPort[unit]));
			l_ret_val = status;
		}

		if((status=gprtSetFrameMode(qdMultiDev[unit],dev_cpuPort[unit], GT_FRAME_MODE_DSA)) != GT_OK)
		{
			MSG_OUT(("gprtSetFrameMode returned %d (port %d, DSA_MODE)\r\n",status,dev_cpuPort[unit]));
			l_ret_val = status;
		}

#if 0	/* Only cascade port with FE sub-board need to work in DSA mode */
		for(j=0;j<sg_switch_bsp_config.numOfCasPorts;j++)
		{
			MSG_OUT(("Setting InterSwitch Port...(%lu)\r\n",sg_switch_bsp_config.cas_ports[j].port));
			if((status=gprtSetFrameMode(qdMultiDev[unit],sg_switch_bsp_config.cas_ports[j].port, GT_FRAME_MODE_DSA)) != GT_OK)
			{
				MSG_OUT(("gprtSetFrameMode returned %d (port %lu, DSA_MODE)\r\n",status,dev_casPort[unit]));
				l_ret_val = status;
			}
		}
#endif
	}
	else  	
	{
		MSG_OUT(("Setting CPU Port(%d)...\r\n",dev_cpuPort[unit]));
		for(j=0; j<qdMultiDev[unit]->numOfPorts; j++)
		{
			if(j == dev_cpuPort[unit])
			{
				if(dev_cpuPort[unit] != dev_wanPort[unit])
				{
					MSG_OUT(("Setting CPU Port(%d) to DSA mode...\r\n",dev_cpuPort[unit]));
					if((status=gprtSetInterswitchPort(qdMultiDev[unit],j,GT_TRUE)) != GT_OK)
					{
						MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_FALSE)\r\n",status,j));
						l_ret_val = status;
						break;
					}
				}
			}
			else
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[unit],j,GT_FALSE)) != GT_OK)
				{
					MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_FALSE)\r\n",status,j));
					l_ret_val = status;
					break;
				}
                if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_CPU_DEST_PER_PORT))
                {
    				if((status=gprtSetCPUPort(qdMultiDev[unit],j,dev_cpuPort[unit])) != GT_OK)
    				{
    					MSG_OUT(("gprtSetCPUPort returned %d\r\n",status));
    					l_ret_val = status;
    					break;
    				}
                }
			}
		}
#if 0	/* Only cascade port with FE sub-board need to work in DSA mode */
		for(j=0;j<sg_switch_bsp_config.numOfCasPorts;j++)
		{
			MSG_OUT(("Setting InterSwitch Port(%lu)...\r\n",sg_switch_bsp_config.cas_ports[j].port));
			if((status=gprtSetInterswitchPort(qdMultiDev[unit],sg_switch_bsp_config.cas_ports[j].port,GT_TRUE)) != GT_OK)
			{
				MSG_OUT(("gprtSetInterswitchPort returned %d (port %lu, mode GT_TRUE)\r\n",status,j));
				l_ret_val = status;
				break;
			}
		}
#endif
	}

	/*
		3. Set Cascading Port information (for From_CPU fram) for each device.
	*/	 	
	if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_CASCADE_PORT))
	{
		if((status=gsysSetCascadePort(qdMultiDev[unit],0xF)) != GT_OK)
		{
			MSG_OUT(("gsysSetCascadePort returned %d\r\n",status));
			l_ret_val = status;
		}
	}
	for(j=0;j<sg_switch_bsp_config.numOfCasPorts;j++)
	{
		MSG_OUT(("Setting Cascade Port(%lu)...\r\n",sg_switch_bsp_config.cas_ports[j].port));
		if((status=gsysSetDevRoutingTable(qdMultiDev[unit],sg_switch_bsp_config.cas_ports[j].devid,sg_switch_bsp_config.cas_ports[j].port)) != GT_OK)
		{
			MSG_OUT(("gsysSetDevRoutingTable returned %d\r\n",status));
			l_ret_val = status;
		}
	}

    if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_GIGABIT_SWITCH))
    {
    	/*
    		4. Set Device ID (if required)
    	*/	 	
    	MSG_OUT(("Setting Device ID (%d)...\r\n",dev_id[unit]));
    	if((status=gsysSetDeviceNumber(qdMultiDev[unit],dev_id[unit])) != GT_OK)
    	{
    		MSG_OUT(("gsysSetDeviceNumber returned %d\r\n",status));
    		l_ret_val = status;
    	}
    }
	/*
		5. Set Switch Register to default value (if required)
	*/	 	
	MSG_OUT(("Setting switch default parameters...\r\n"));
	if((status= switch_default_config(qdMultiDev[unit])) != GT_OK)
	{
		MSG_OUT(("switch_default_config returned %d\r\n",status));
		l_ret_val = status;
	}
#if 0
	if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_FLOOD_BROADCAST))
	{
		MSG_OUT(("MinimizeCPUTraffic2...\r\n"));
		if((status=MinimizeCPUTraffic2(qdMultiDev[unit], NULL)) != GT_OK)
		{
			MSG_OUT(("MinimizeCPUTraffic2 returned %d\r\n",status));
			l_ret_val = status;
		}
	}
#endif
#if (PRODUCT_CLASS == GT861_ONU)
	MSG_OUT(("Setting switch port for FPGA (%d)...\r\n",DEVICE0_FPGA_PORT));
	if((status = switch_fe_port_init(qdMultiDev[unit],DEVICE0_FPGA_PORT,GT_FALSE)) != GT_OK)
	{
		MSG_OUT(("Config master device fpga port failed(%d).\r\n", status));
		l_ret_val = status;
	}
	else
	{
		if((status = switch_fpga_port_enable(GT_FALSE)) != GT_OK)
		{
			MSG_OUT(("Disable master device fpga port failed(%d).\r\n", status));
			l_ret_val = status;
		}
	}
	if((status = gprtSetForwardUnknown(qdMultiDev[unit],DEVICE0_FPGA_PORT, GT_FALSE)) != GT_OK)
	{
		MSG_OUT(("Config master device fpga port drop unknown unicast failed(%d).\r\n", status));
		l_ret_val = status;
	}

#endif

	if(dev_cpuPort[unit] != dev_wanPort[unit])
	{
		MSG_OUT(("Setting switch port for CPU (%d)...\r\n",dev_cpuPort[unit]));
		if((status = switch_fe_port_init(qdMultiDev[unit],dev_cpuPort[unit],GT_TRUE)) != GT_OK)
		{
			MSG_OUT(("Config master device CPU port failed(%d).\r\n", status));
			l_ret_val = status;
		}
	}

	/* Create IBC(inter board communication) VLAN */
	if((status = InternalVlanInit(qdMultiDev[unit],DEF_CTRL_CHAN_VLANID)) != GT_OK)
	{
		MSG_OUT(("Create IBC vlan failed(%d).\r\n", status));
		l_ret_val = status;
	}

#if 0	/* Use Hmii to receive downstream packet */
	/* Make WAN port and CPU port isolate */
	if(dev_cpuPort[unit] != dev_wanPort[unit])
	{
		MSG_OUT(("Isolating WAN port(%lu) and CPU port(%lu)...\r\n", dev_wanPort[unit], dev_cpuPort[unit]));
		if((status = IsolateTwoPorts(qdMultiDev[unit], GT_TRUE, dev_wanPort[unit], dev_cpuPort[unit])) != GT_OK)
		{
			MSG_OUT(("IsolateTwoPorts failed(%d).\r\n", status));
			l_ret_val = status;
		}
	}
#endif

#if 0
	if(dev_cpuPort[unit] != dev_wanPort[unit])
	{
		MSG_OUT(("Set CPU port(%lu) Egress rate to 64kbps...\r\n",dev_cpuPort[unit]));
		/* Egress 64kbps */
		if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_ELIMIT_FRAME_BASED))
			gtEgressRateType.kbRate = 64;
		else
			gtEgressRateType.definedRate = GT_64K;
        if ((status = grcSetEgressRate(qdMultiDev[unit], dev_cpuPort[unit], &gtEgressRateType)) != GT_OK)
    	{
    		MSG_OUT(( "grcSetEgressRate return Failed(%d)\r\n", status));
    		l_ret_val = status;
    	}
	}
#endif
	
	return l_ret_val;
}

GT_STATUS switch_default_config(GT_QD_DEV * dev)
{
	GT_STATUS    	l_ret_val = GT_OK;
	GT_STATUS       result = GT_OK;
    GT_LPORT        i, phyPort;
#ifndef _RELEASE_VERSION_
    GT_VTU_ENTRY    vtuEntry;
#endif
 	GT_ERATE_TYPE   gtEgressRateType;
 	GT_U8			pri;
#if 0
	GT_ATU_ENTRY gtAtuEntryBc;
	GT_ATU_ENTRY gtAtuEntryMcLn;
	GT_U8 Mc_LocalNetwork[6] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x01};
#endif
/*#define  _SIMPLEST_*/

#if 1
    if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
    	/* Disable PPU */
    	if ((result = gsysSetPPUEn(dev,GT_FALSE)) != GT_OK)
    	{
    		MSG_OUT(("gsysSetPPUEn return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
    }
#endif	
#ifndef _SIMPLEST_
	MSG_OUT(( "Stage 1\r\n"));
	for(i=0; i<NUM_PORTS_PER_SWITCH; i++)
	{
		/* Disable Energydetect function of the integated PHY */
		/* Default is enabled, which can cause MD1230A line error! */
#ifdef _USE_DSDT_26A_
		gprtSetEnergyDetect(dev,i,GT_FALSE);
#else
		gprtSetEnergyDetect(dev,i,GT_EDETECT_OFF);
#endif
		/* Set PHY to CLASS A mode for avoid CRC error with Smartbits LAN3200A */
		gprtSetClassABDrv(dev,i,GT_TRUE);
	}
	/* Set mtu to the max value of mav switch(1536, 1632 or 2048) */
	if ((result = gsysSetMaxFrameSize(dev,GT_FALSE)) != GT_OK)
	{
		MSG_OUT(( "gsysSetMaxFrameSize return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	/* Restore default value of each port */
	gtEgressRateType.definedRate = GT_NO_LIMIT;
	MSG_OUT(( "Stage 2\r\n"));
	for(phyPort=0; phyPort<NUM_PORTS_PER_SWITCH; phyPort++)
	{
		/* UserPri more importent than ip dscp */
        if ((result = gqosSetPrioMapRule(dev, phyPort, GT_TRUE)) != GT_OK)
    	{
    		MSG_OUT(( "gqosSetPrioMapRule return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
    	/* Port default pri = 0 */
		if (IS_IN_DEV_GROUP(dev,DEV_QoS_FPRI_QPRI))
		{
	        if ((result = gqosSetDefFPri(dev, phyPort, 0)) != GT_OK)
	    	{
	    		MSG_OUT(( "gqosSetDefFPri return Failed(%d)\r\n", result));
	    		l_ret_val = result;
	    	}
    	}
		/* Port mode is Auto */
        if ((result = gprtSetPortAutoMode(dev, phyPort, SPEED_AUTO_DUPLEX_AUTO)) != GT_OK)
    	{
    		MSG_OUT(( "gprtSetPortAutoMode return Failed(%lu,%d)\r\n", phyPort,result));
    		l_ret_val = result;
    	}
        if ((result = gprtPortAutoNegEnable(dev, phyPort, GT_TRUE)) != GT_OK)
    	{
    		MSG_OUT(( "gprtPortAutoNegEnable return Failed(%lu,%d))\r\n", phyPort,result));
    		l_ret_val = result;
    	}
        if ((result = gprtPortRestartAutoNeg(dev, phyPort)) != GT_OK)
    	{
    		MSG_OUT(( "gprtPortRestartAutoNeg return Failed(%lu,%d)\r\n", phyPort,result));
    		l_ret_val = result;
    	}

        /*broadcast frame not treated as multicast frame*/
        if((result = gsysSetFloodBC(dev, GT_TRUE)) != GT_OK)
        {
        	MSG_OUT(( "gsysSetFloodBC return Failed(%lu,%d)\r\n", phyPort,result));
        	l_ret_val = result;
        }
		/* Forward unknown unicast is enable */
        if ((result = gprtSetForwardUnknown(dev, phyPort, GT_TRUE)) != GT_OK)
    	{
    		MSG_OUT(( "gprtSetForwardUnknown return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
        if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
        {
    		/* Forward unknown multicast is enable */
            if ((result = gprtSetDefaultForward(dev, phyPort, GT_TRUE)) != GT_OK)
        	{
        		MSG_OUT(( "gprtSetDefaultForward return Failed(%d)\r\n", result));
        		l_ret_val = result;
        	}
        }
        if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
        {
    		/* Accept all frames */
            if ((result = gprtSetDiscardTagged(dev, phyPort, GT_FALSE)) != GT_OK)
        	{
        		MSG_OUT(( "gprtSetDiscardTagged return Failed(%d)\r\n", result));
        		l_ret_val = result;
        	}
            if ((result = gprtSetDiscardUntagged(dev, phyPort, GT_FALSE)) != GT_OK)
        	{
        		MSG_OUT(( "gprtSetDiscardUntagged return Failed(%d)\r\n", result));
        		l_ret_val = result;
        	}
        }
        if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
        {
    		/* Enable port learning */
            if ((result = gprtSetLearnDisable(dev, phyPort, GT_FALSE)) != GT_OK)
        	{
        		MSG_OUT(( "gprtSetLearnDisable return Failed(%d)\r\n", result));
        		l_ret_val = result;
        	}
        }
		/* Disable drop on lock */
		if (IS_IN_DEV_GROUP(dev,DEV_CASCADE_PORT))
		{
	        if ((result = gprtSetDropOnLock(dev, phyPort, GT_FALSE)) != GT_OK)
	    	{
	    		MSG_OUT(( "gprtSetDropOnLock return Failed(%d)\r\n", result));
	    		l_ret_val = result;
	    	}
	    }
		/* Ingress no limit */
		if (IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH) == GT_OK)
		{
	        if ((result = grcSetPri0Rate(dev, phyPort, GT_NO_LIMIT)) != GT_OK)
	    	{
	    		MSG_OUT(( "grcSetPri0Rate return Failed(%d)\r\n", result));
	    		l_ret_val = result;
	    	}
    	}
		/* Egress no limit */
        if ((result = grcSetEgressRate(dev, phyPort, &gtEgressRateType)) != GT_OK)
    	{
    		MSG_OUT(( "grcSetEgressRate return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
        if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
        {
    		/* No mirror */
            if ((result = gprtSetIngressMonitorSource(dev, phyPort, GT_FALSE)) != GT_OK)
        	{
        		MSG_OUT(( "gprtSetIngressMonitorSource return Failed(%d)\r\n", result));
        		l_ret_val = result;
        	}
            if ((result = gprtSetEgressMonitorSource(dev, phyPort, GT_FALSE)) != GT_OK)
        	{
        		MSG_OUT(( "gprtSetEgressMonitorSource return Failed(%d)\r\n", result));
        		l_ret_val = result;
        	}
        }
        if (IS_IN_DEV_GROUP(dev,DEV_PRIORITY_REMAPPING))
        {
    		/* Tag remap to same value */
    		for(pri = 0; pri < 8; pri++)
    		{
    	        if ((result = gqosSetTagRemap(dev, phyPort, pri, pri)) != GT_OK)
    	    	{
    	    		MSG_OUT(( "gqosSetTagRemap return Failed(%d)\r\n", result));
    	    		l_ret_val = result;
    	    	}
    	    	
    	    }
        }
	    /* 802.1p enable */
        if ((result = gqosUserPrioMapEn(dev, phyPort, GT_TRUE)) != GT_OK)
    	{
    		MSG_OUT(( "gqosUserPrioMapEn return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
	    /* IP priority disable */
        if ((result = gqosIpPrioMapEn(dev, phyPort, GT_FALSE)) != GT_OK)
    	{
    		MSG_OUT(( "gqosIpPrioMapEn return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}

		/* Disable flowcontrol for 6096 and 6095 */
        if ((result = gprtSetPause(dev, phyPort, MRV_FLOW_CONTROL_ENABLE_DEF)) != GT_OK)
    	{
    		MSG_OUT(( "gprtSetPause return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
        if (IS_IN_DEV_GROUP(dev,DEV_FC_WITH_VALUE))
        {
            if ((result = gpcsSetFCValue(dev, phyPort, MRV_FLOW_CONTROL_ENABLE_DEF)) != GT_OK)
        	{
        		MSG_OUT(( "gpcsSetFCValue return Failed(%d)\r\n", result));
        		l_ret_val = result;
        	}
            if ((result = gpcsSetForcedFC(dev, phyPort, GT_TRUE)) != GT_OK)
        	{
        		MSG_OUT(( "gpcsSetForcedFC return Failed(%d)\r\n", result));
        		l_ret_val = result;
        	}
        }
		
#if 0 /*def _RELEASE_VERSION_	*/
	    /* Disable all ports and after plateform initialization finished re-open */
        if ((result = gstpSetPortState(dev, phyPort, GT_PORT_DISABLE)) != GT_OK)
    	{
    		MSG_OUT(( "gstpSetPortState return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
#endif
#if 0 /*def _SUPPORT_GT815_*/
		/* Set ARP to CPU */
        if ((result = gprtSetARPtoCPU(dev, phyPort, GT_TRUE)) != GT_OK)
    	{
    		MSG_OUT(( "gprtSetARPtoCPU return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
#endif /* _SUPPORT_GT815_ */
	}
	MSG_OUT(( "Stage 3\r\n"));
    if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
    	/* No mirror */
        if ((result = gsysSetIngressMonitorDest(dev, 0xF)) != GT_OK)
    	{
    		MSG_OUT(( "gsysSetIngressMonitorDest return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
        if ((result = gsysSetEgressMonitorDest(dev, 0xF)) != GT_OK)
    	{
    		MSG_OUT(( "gsysSetEgressMonitorDest return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
    }
	MSG_OUT(( "Stage 4\r\n"));
	/* Default pri to traffic class */
	for(pri = 0; pri < 8; pri++)
	{
		GT_U8  	ucDefaultUserPrio2Tc[8] = {0, 0, 1, 1, 2, 2, 3, 3};

        if ((result = gcosSetUserPrio2Tc(dev, pri, ucDefaultUserPrio2Tc[pri])) != GT_OK)
    	{
    		MSG_OUT(( "gcosSetUserPrio2Tc return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
	}

#if 0
/*(PRODUCT_CLASS == GIS2109)*/
	if (IS_IN_DEV_GROUP(dev,DEV_88E6083))
	{
		/*Old type of 2109, 0,1,2 are the fiber ports*/
	    for (phyPort=0; phyPort<2; phyPort++)
	    {
	        gprtPortAutoNegEnable(dev, phyPort, GT_FALSE);
	    }
	}
	else if((IS_IN_DEV_GROUP(dev, DEV_88E6097_FAMILY)))
	{
		/*New type of 2109, 6,7,9 are the fiber ports*/
	    for (phyPort=6; phyPort<8; phyPort++)
	    {
	        gprtPortAutoNegEnable(dev, phyPort, GT_FALSE);
	    }
	    gprtPortAutoNegEnable(dev, 9, GT_FALSE);
		gprtPortPowerDown(dev, 9, GT_FALSE);
		
		gpcsSetLinkValue(dev, 9, GT_TRUE);
		gpcsSetForcedLink(dev, 9, GT_TRUE);
		gpcsSetDpxValue(dev, 9, GT_TRUE);
		gpcsSetForcedDpx(dev, 9, GT_TRUE);
		gpcsSetForceSpeed(dev, 9, PORT_FORCE_SPEED_100_MBPS);
	}
	else
	{}
#endif
	if (IS_IN_DEV_GROUP(dev,DEV_FRAME_MODE))
	{
	    for (phyPort=8; phyPort<10; phyPort++)
	    {
			if( GT_OK != gprtSetFrameMode(dev, phyPort, GT_FRAME_MODE_NORMAL))
				MSG_OUT(("Set the port %lu GT_FRAME_MODE_NORMAL failed!\r\n", phyPort));
	    }
	}
#if ((PRODUCT_CLASS == GT861_ONU) || (PRODUCT_CLASS == GT815))
#if 0
    if ((result = gfdbSetLearn2All(dev, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gfdbSetLearn2All return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
#endif
#endif
	/* qos algorithm to wrr */
	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_MIXED_SCHEDULE))
	{
	    if ((result = gsysSetSchedulingMode(dev, GT_TRUE)) != GT_OK)
		{
			MSG_OUT(( "gsysSetSchedulingMode return Failed(%d)\r\n", result));
			l_ret_val = result;
		}
	}
	MSG_OUT(( "Stage 5\r\n"));
	/* Default dscp to traffic class */
	for(pri = 0; pri < 64; pri++)
	{
        if ((result = gcosSetDscp2Tc(dev, pri, pri/16)) != GT_OK)
    	{
    		MSG_OUT(( "gcosSetDscp2Tc return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
	}
	if((result = gvlnSetPortIsolate(dev, GT_FALSE)) != GT_OK)
        {
                MSG_OUT(("gvlnSetPortIsolate return Failed(%d)\r\n", result));
                l_ret_val = result;
        }
#endif /* _SIMPLEST_ */

#if FOR_812_SERIES
#ifdef _RELEASE_VERSION_
	if(ONU_IS_GT811())
#endif
	{
		/* Shut down port 4, 5 to avoid loop cause VxWorks IP stack error 
		 * IPIF_VxWorks.c in line 829: Cannot loan cluster! */
		for(i=4; i<5; i++)
		{

#ifndef _SIMPLEST_
			if ((result = gprtPortPowerDown(dev, i, GT_TRUE)) != GT_OK)
			{
				MSG_OUT(( "gprtPortPowerDown return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
#else
			if ((result = gstpSetPortState(dev, i, GT_PORT_DISABLE)) != GT_OK)
			{
				MSG_OUT(( "gstpSetPortState return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
#endif

		}
	}
#ifndef _PROTECT_SWITCH_
        if ((result = gvlnSetPortIsolate(dev, GT_TRUE)) != GT_OK)
        {
            MSG_OUT(( "gvlnSetPortIsolate return Failed(%d)\r\n", result));
            l_ret_val = result;
        }
#endif
#endif


#ifndef _SIMPLEST_
    /* set vlan default mode */

	MSG_OUT(( "Stage 6\r\n"));
  	for ( phyPort=0; phyPort<dev->maxPorts; phyPort++ )
    {
#if (FOR_MRV_INDUSTRY_SW)
		int unit =0;
		if(IS_IN_DEV_GROUP(QD_DEV_PTR, DEV_88E6097))
		{
			if ((result = gpcsSetDpxValue(dev, phyPort, GT_FALSE)) != GT_OK)
			{
				MSG_OUT(( "gpcsSetDpxValue return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
			if ((result = gpcsSetFCValue(dev, phyPort, GT_FALSE)) != GT_OK)
			{
				MSG_OUT(( "gpcsSetFCValue return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
			if ((result = gpcsSetLinkValue(dev, phyPort, GT_FALSE)) != GT_OK)
			{
				MSG_OUT(( "gpcsSetLinkValue return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
			if ((result = gpcsSetForceSpeed(dev, phyPort, PORT_DO_NOT_FORCE_SPEED)) != GT_OK)
			{
				MSG_OUT(( "gpcsSetForceSpeed return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
			if ((result = gpcsSetForcedDpx(dev, phyPort, GT_FALSE)) != GT_OK)
			{
				MSG_OUT(( "gpcsSetForcedDpx return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
			if ((result = gpcsSetForcedFC(dev, phyPort, GT_FALSE)) != GT_OK)
			{
				MSG_OUT(( "gpcsSetForcedFC return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
			if ((result = gpcsSetForcedLink(dev, phyPort, GT_FALSE)) != GT_OK)
			{
				MSG_OUT(( "gpcsSetForcedLink return Failed(%d)\r\n", result));
				l_ret_val = result;
			}
		}
#endif
        if ((result = gvlnSetPortVlanDot1qMode(dev, phyPort, GT_SECURE)) != GT_OK)
    	{
    		MSG_OUT(( "gvlnSetPortVlanDot1qMode return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
#ifdef _PROTECT_SWITCH_
{
        GT_U8 value;
	PASONU_GPIO_set_direction(15,0);
        PASONU_GPIO_get(15, &value);
	if(value != 1)
        {
            if ((result = gvlnSetPortVlanDot1qMode(dev, phyPort, GT_DISABLE)) != GT_OK)
    	    {
    		MSG_OUT(( "gvlnSetPortVlanDot1qMode return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	    }
        }
}
#endif
    }
#ifndef _RELEASE_VERSION_
  	for ( phyPort=0; phyPort<dev->maxPorts; phyPort++ )
    {	/*IFM_VLAN_DEFAULT_VLAN_ID */
        if ((result = gvlnSetPortVid(dev, phyPort, 1)) != GT_OK)
    	{
    		MSG_OUT(( "gvlnSetPortVid return Failed(%d)\r\n", result));
    		l_ret_val = result;
    	}
    }
	memset(&vtuEntry, 0, sizeof (GT_VTU_ENTRY));
    vtuEntry.DBNum = 1;
    vtuEntry.vid = 1;
  	for ( phyPort=0; phyPort<dev->maxPorts; phyPort++ )
    {
    	vtuEntry.vtuData.memberTagP[phyPort] = MEMBER_EGRESS_UNTAGGED;
	}
    if ((result = gvtuAddEntry(dev, &vtuEntry)) != GT_OK)
	{
		MSG_OUT(( "gvtuAddEntry return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
#endif
#if 0
	/* 6095 not support broadcast flood, 6097 does */
	if (IS_IN_DEV_GROUP(dev, DEV_FLOOD_BROADCAST))
    {
		/* Broadcast atu entry */
	    gtAtuEntryBc.DBNum = 1; 
	    gtAtuEntryBc.exPrio.macFPri = 0;
	    gtAtuEntryBc.exPrio.macQPri = 0;
	    gtAtuEntryBc.exPrio.useMacFPri = GT_FALSE;
	    gtAtuEntryBc.prio = 0;
	    gtAtuEntryBc.trunkMember = GT_FALSE;
	    for (i=0; i<GT_ETHERNET_HEADER_SIZE; i++)
	        gtAtuEntryBc.macAddr.arEther[i] = 0xFF;
	    gtAtuEntryBc.portVec = 0;
	  	for ( phyPort=0; phyPort<dev->maxPorts; phyPort++ )
	  	{/* zhangxinhui �����������˿ں�PON��IAD�ܿ�ʧȥ��ϵ��ԭ���� */
	  	/*if ((phyPort != 8) && (phyPort != 9))*/
	    	gtAtuEntryBc.portVec |= 1<<phyPort;
	  	}
	    gtAtuEntryBc.entryState.mcEntryState = GT_MC_STATIC;
	    if ((result = gfdbAddMacEntry(dev, &gtAtuEntryBc)) != GT_OK)
		{
			MSG_OUT(( "gfdbAddMacEntry return Failed(%d)\r\n", result));
			l_ret_val = result;
		}

#if (FOR_ONU_PON)
		/* Multicast local network management atu entry */
	    gtAtuEntryMcLn.DBNum = 1; 
	    gtAtuEntryMcLn.exPrio.macFPri = 0;
	    gtAtuEntryMcLn.exPrio.macQPri = 0;
	    gtAtuEntryMcLn.exPrio.useMacFPri = GT_FALSE;
	    gtAtuEntryMcLn.prio = 0;
	    gtAtuEntryMcLn.trunkMember = GT_FALSE;
	    for (i=0; i<GT_ETHERNET_HEADER_SIZE; i++)
	        gtAtuEntryMcLn.macAddr.arEther[i] = Mc_LocalNetwork[i];
	    gtAtuEntryMcLn.portVec = 0;
	  	for ( phyPort=0; phyPort<dev->maxPorts; phyPort++ )
	  	{ /* zhangxinhui �����������˿ں�PON��IAD�ܿ�ʧȥ��ϵ��ԭ���� */
	  	/*if ((phyPort != 8) && (phyPort != 9))*/
	    	gtAtuEntryMcLn.portVec |= 1<<phyPort;
	  	}
	    gtAtuEntryMcLn.entryState.mcEntryState = GT_MC_STATIC;

	    if ((result = gfdbAddMacEntry(dev, &gtAtuEntryMcLn)) != GT_OK)
		{
			MSG_OUT(( "gfdbAddMacEntry return Failed(%d)\r\n", result));
			l_ret_val = result;
		}
#endif
	}
        if((result = gfdbSetAgingTimeout(dev, 330)) != GT_OK)
        {
                MSG_OUT(("gfdbSetAgingTimeout return Failed(%d)\r\n", result));
                l_ret_val = result;
        }
#endif
	/* Use mac in EEPROM as bridge mac */
    /*if ((result = Init_bridge_mac(dev)) != GT_OK)
	{
		MSG_OUT(( "Init_bridge_mac return Failed(%d)\r\n", result));
		l_ret_val = result;
	}*/

	/*memset(sg_vlan_modes, 0, sizeof (CTC_STACK_port_vlan_configuration_t) * MAX_SWITCH_PORTS);*/
	/*memset(sg_mc_vlan_filters, 0, sizeof (CTC_STACK_multicast_vlan_t) * MAX_SWITCH_PORTS);*/
    /* TBD set rate limit default mode */
    /* TBD set igmp sooping default mode */
	/* TBD set stp default mode */
	/* TBD set multicast switch default mode */
	/* TBD set multicast tag strip default mode */

	/* ATU (=Marvell address table database) 0 is used by default */
	/*sg_switch_atu_used[DEFAULT_ATU_NUM] = GT_TRUE;*/
#endif /* 0 */

#if 0
	/* Enable PPU */
	if ((result = gsysSetPPUEn(dev,GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gsysSetPPUEn return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
#endif	

	return l_ret_val;
}

GT_STATUS switch_ge_port_init(GT_QD_DEV * dev, GT_LPORT port)
{
	GT_STATUS          l_ret_val = GT_OK;
	GT_STATUS          result = GT_OK;

    /* Set cpu port to force mode */
	if ((result = gpcsSetDpxValue(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetDpxValue return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
#ifdef _DISABLE_UNI_PORT_PAUSE_
	if ((result = gpcsSetFCValue(dev, port, GT_FALSE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetFCValue return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
#else
	if ((result = gpcsSetFCValue(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetFCValue return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
#endif
	if ((result = gpcsSetLinkValue(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetLinkValue return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetForceSpeed(dev, port, PORT_FORCE_SPEED_1000_MBPS)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetForceSpeed return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetForcedDpx(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetForcedDpx return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetForcedFC(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetForcedFC return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetForcedLink(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetForcedLink return Failed(%d)\r\n", result));
		l_ret_val = result;
	}

	return l_ret_val;
}

GT_STATUS switch_fe_port_init(GT_QD_DEV * dev, GT_LPORT port, GT_BOOL pauseEn)
{
	GT_STATUS          l_ret_val = GT_OK;
	GT_STATUS          result = GT_OK;

    /* Set cpu port to force mode */
	if ((result = gpcsSetDpxValue(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetDpxValue return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetFCValue(dev, port, pauseEn)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetFCValue return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetLinkValue(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetLinkValue return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetForceSpeed(dev, port, PORT_FORCE_SPEED_100_MBPS)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetForceSpeed return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetForcedDpx(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetForcedDpx return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetForcedFC(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetForcedFC return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gpcsSetForcedLink(dev, port, GT_TRUE)) != GT_OK)
	{
		MSG_OUT(( "gpcsSetForcedLink return Failed(%d)\r\n", result));
		l_ret_val = result;
	}
	if ((result = gprtSetMGMII(dev, port, GT_FALSE)) != GT_OK)
	{
		MSG_OUT(( "gprtSetMGMII return Failed(%d)\r\n", result));
		l_ret_val = result;
	}

	return l_ret_val;
}

/*
	For the devices that support gsysSetARPDest API:

	0) Remove CPU port from VLAN Member Table.
	   (this sample deals with Port Based Vlan only.)
	1) Mirror ARPs to the CPU with To_CPU Marvell Tag
	2) Convert unicast frames directed to the CPU into To_CPU Marvell Tag
	Assumption : Device ID, Cascading Port, CPU Port, and Interswitch Port are
		already set properly. For more information, please refer to the 
		sample/MultiDevice/msApiInit.c
*/

GT_STATUS MinimizeCPUTraffic1(GT_QD_DEV *dev, GT_U8* macAddr)
{
	GT_STATUS status, lRet;
	int i;
	GT_LPORT memPorts[16], cpuPort;
	GT_U8 memPortsLen, index;
    GT_ATU_ENTRY macEntry;

	cpuPort = (GT_LPORT)dev->cpuPortNum;
	lRet = GT_OK;
	/*
	 *	Remove CPU port from VLAN Member Table.
	*/ 
	for(i=0; i<dev->numOfPorts; i++)
	{
		if((status = gvlnGetPortVlanPorts(dev,(GT_LPORT)i,memPorts,&memPortsLen)) != GT_OK)
		{
			MSG_OUT(("gvlnGetPortVlanPorts return Failed\n"));
			lRet = status;
		}

		for(index=0; index<memPortsLen; index++)
		{
			if (memPorts[index] == cpuPort)
				break;
		}

		if(index != memPortsLen)
		{
			/* CPU Port is the member of the port vlan */
			if((memPortsLen-1) != index)
			{
				memPorts[index] = memPorts[memPortsLen-1];
			}
			memPortsLen--;

			if((status = gvlnSetPortVlanPorts(dev,(GT_LPORT)i,memPorts,memPortsLen)) != GT_OK)
			{
				MSG_OUT(("gvlnSetPortVlanPorts return Failed\n"));
				lRet = status;
			}
		}
	}

	/*
	 *	Mirror ARPs to the CPU with To_CPU Marvell Tag.
	*/
	if((status = gsysSetARPDest(dev,cpuPort)) != GT_OK)
	{
		MSG_OUT(("gsysSetARPDest return Failed\n"));
		lRet = status;
	}

	/*
	 *	Convert unicast frames directed to the CPU into To_CPU Marvell Tag.
	 *  This sample assumes that DBNum is not used. If DBNum is used,
	 *  the macEntry has to be added for each DBNum used.
	*/
	if( macAddr != NULL )
	{
		memset(&macEntry,0,sizeof(GT_ATU_ENTRY));
		memcpy(macEntry.macAddr.arEther,macAddr,6);
		macEntry.portVec = 1 << dev->cpuPortNum;
		macEntry.prio = 0;			/* Priority (2bits). When these bits are used they override
									any other priority determined by the frame's data */
		macEntry.entryState.ucEntryState = GT_UC_TO_CPU_STATIC;
		macEntry.DBNum = 1;
		macEntry.trunkMember = GT_FALSE;

		if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK)
		{
			MSG_OUT(("gfdbAddMacEntry return Failed\n"));
			lRet = status;
		}
	}

	
	return lRet;
}

/*
	For the devices that support gprtSetARPtoCPU API:

	1) Enable ARP to CPU feature fore each port.
	2) Set Egress Flood Mode to be Block Unknown DA on CPU Port.
	3) Add CPU Port's MAC into address table.
	4) Remove Broadcast address from address table.
*/

GT_STATUS MinimizeCPUTraffic2(GT_QD_DEV *dev, GT_U8* macAddr)
{
	GT_STATUS status, lRet;
	int i;
	GT_LPORT cpuPort;
    GT_ATU_ENTRY macEntry;

	cpuPort = (GT_LPORT)dev->cpuPortNum;

	lRet = GT_OK;

	/*
	 *	Mirror ARPs to the CPU with To_CPU Marvell Tag.
	*/
	for(i=0; i<dev->numOfPorts; i++)
	{
		if ((i == cpuPort) || (i == (dev->numOfPorts - 1)))
			continue;

		if((status = gprtSetARPtoCPU(dev,i,GT_TRUE)) != GT_OK)
		{
			MSG_OUT(("gprtSetARPtoCPU return Failed\n"));
			lRet = status;
		}
	}

	/*
	 * Set Egress Flood Mode to be Block Unknown DA on CPU Port.
	*/
	if((status = gprtSetEgressFlood(dev,cpuPort,GT_BLOCK_EGRESS_UNKNOWN)) != GT_OK)
	{
		MSG_OUT(("gprtSetEgressFlood return Failed\n"));
		lRet = status;
	}


	/*
	 *	Add CPU's MAC into address table.
	 *  This sample assumes that DBNum is not used. If DBNum is used,
	 *  the macEntry has to be added for each DBNum used.
	*/
	if( macAddr != NULL )
	{
		memset(&macEntry,0,sizeof(GT_ATU_ENTRY));
		memcpy(macEntry.macAddr.arEther,macAddr,6);
		macEntry.portVec = 1 << dev->cpuPortNum;
		macEntry.prio = 0;			/* Priority (2bits). When these bits are used they override
									any other priority determined by the frame's data */
		macEntry.entryState.ucEntryState = GT_UC_TO_CPU_STATIC;
		macEntry.DBNum = 1;
		macEntry.trunkMember = GT_FALSE;

		if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK)
		{
			MSG_OUT(("gfdbAddMacEntry return Failed\n"));
			lRet = status;
		}

		/*
		 *  Delete BroadCast Entry from address table if exists.
		 *  This sample assumes that DBNum is not used. If DBNum is used,
		 *  the macEntry has to be added for each DBNum used.
		*/
		memset(&macEntry,0,sizeof(GT_ATU_ENTRY));
		memset(macEntry.macAddr.arEther,0xFF,6);
		gfdbDelAtuEntry(dev,&macEntry);
	}
	
	return lRet;
}

/*******************************************************************************************
  Description   : This function will get the cascade port info.
  In params     : unit	- switch unit id.
  Out params    : number	-	number of cascade ports.
  				  ports		-	IDs of cascade ports.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS get_switch_cascade_port_info(GT_U32 unit, GT_U32 *number, GT_U8 ports[NUM_PORTS_PER_SWITCH])
{
	if(UNIT_IS_MASTER(unit))
	{
		*number = sizeof(master_casPort);
		memcpy(ports, master_casPort, *number);
	}
	else
	{
		*number = 1;
		ports[0] = dev_casPort[unit];
	}

	return GT_OK;
}

/*******************************************************************************************
  Description   : This function will get the wan port info.
  In params     : unit	- switch unit id.
  Out params    : port	-	ID of wan port.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS get_switch_wan_port_info(GT_U32 unit, GT_U32 *port)
{
	*port = dev_wanPort[unit];

	return GT_OK;
}

/*******************************************************************************************
  Description   : This function will get the cpu port info.
  In params     : unit	- switch unit id.
  Out params    : port	-	ID of wan port.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS get_switch_cpu_port_info(GT_U32 unit, GT_U32 *port)
{
	*port = dev_cpuPort[unit];

	return GT_OK;
}

/*******************************************************************************************
  Description   : This function will set a new 01-80-c2-00-00-0x packet as a MGMT packet.
  In params     : dev - switch device struct.
  In params    : mgmtValue - the '0x' in 01-80-c2-00-00-0x.
  In params    : en - if GT_TRUE, means set the packet as mgmt, otherwise set the packet as normal packet.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS newMgmtSet(GT_QD_DEV *dev, GT_U8 mgmtValue, GT_BOOL en)
{
    GT_STATUS       retVal = GT_OK; /* Functions return value.      */
	GT_U16			enBits;

    if (mgmtValue > 0x0F)
    {
        /*invalid input.*/
        return GT_FALSE;
    }

	/* If disable, reset the BPDU bit(bit0) from Rsvd2CpuEnables register */
   	if(en == GT_FALSE)
	{
		if((retVal = gsysGetRsvd2CpuEnables(dev,&enBits)) != GT_OK)
		{
	        DBG_INFO(("gsysGetRsvd2CpuEnables failed.\n"));
			return retVal;
		}
		enBits &= ~(0x1<<mgmtValue);

		if((retVal = gsysSetRsvd2CpuEnables(dev,enBits)) != GT_OK)
		{
    	    DBG_INFO(("gsysSetRsvd2CpuEnables failed.\n"));
			return retVal;
		}

		return retVal;
	}

	/* 
		If enable, 
		1) Set MGMT Pri bits, 
		2) Set BPDU bit(bit0) from Rsvd2CpuEnables register,
		3) Enable Rsvd2Cpu
	*/

	if((retVal = gsysGetRsvd2CpuEnables(dev,&enBits)) != GT_OK)
	{
        DBG_INFO(("gsysGetRsvd2CpuEnables failed.\n"));
		return retVal;
	}
	enBits |= (0x1<<mgmtValue);
	if((retVal = gsysSetRsvd2CpuEnables(dev,enBits)) != GT_OK)
	{
        DBG_INFO(("gsysSetRsvd2CpuEnables failed.\n"));
		return retVal;
	}

	if((retVal = gsysSetRsvd2Cpu(dev,GT_TRUE)) != GT_OK)
	{
        DBG_INFO(("gsysSetRsvd2Cpu failed.\n"));
		return retVal;
	}

	return retVal;
}


/*******************************************************************************************
  Description   : This function will get the cascade port number on master for unit.
  In params     : unit	- switch unit id.
  Out params    : port	-	ID of wan port.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS get_switch_cascade_port_on_master(GT_U32 unit, GT_U32 *port)
{
	*port = 0;
	if(UNIT_IS_MASTER(unit))
		return GT_BAD_PARAM;
	if(!UNIT_IS_VALID(unit))
		return GT_BAD_PARAM;
		
	*port = master_casPort[unit-1];

	return GT_OK;
}

GT_STATUS smiChannelSemTake(void)
{
	GT_U32 unit;
	GT_QD_DEV       *dev;
	GT_STATUS	gtRet=GT_OK;
	
	for(unit=0; unit<N_OF_QD_DEVICES; unit++)
	{
		dev = QD_DEV_PTR;
		if( NULL == dev)
			continue;
		else
		{
			/*
		    GT_SEM		multiAddrSem;
		    GT_SEM		atuRegsSem;
		    GT_SEM		vtuRegsSem;
		    GT_SEM		statsRegsSem;
		    GT_SEM		pirlRegsSem;
    		GT_SEM		ptpRegsSem;
		    GT_SEM		tblRegsSem;
		    GT_SEM		phyRegsSem;
		    */

			gtRet += gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);
#if 0/*wug delete. we need to task the basic sem only.*/
			gtRet += gtSemTake(dev,dev->atuRegsSem,OS_WAIT_FOREVER);
			gtRet += gtSemTake(dev,dev->vtuRegsSem,OS_WAIT_FOREVER);
			gtRet += gtSemTake(dev,dev->statsRegsSem,OS_WAIT_FOREVER);
			gtRet += gtSemTake(dev,dev->pirlRegsSem,OS_WAIT_FOREVER);
			gtRet += gtSemTake(dev,dev->ptpRegsSem,OS_WAIT_FOREVER);
			gtRet += gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);
			gtRet += gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);
#endif
		}
	}
	dev = QD_MASTER_DEV_PTR;
#if (PRODUCT_CLASS == GT861_ONU)
	gtRet += gtSemTake(dev,smiAccessSem,OS_WAIT_FOREVER);
#endif	
	return gtRet;
}

GT_STATUS smiChannelSemGive(void)
{
	GT_U32 unit;
	GT_QD_DEV *dev;
	GT_STATUS	gtRet=GT_OK;
	
	for(unit=0; unit<N_OF_QD_DEVICES; unit++)
	{
		dev = QD_DEV_PTR;
		if( NULL == dev)
			continue;
		else
		{
			/*
		    GT_SEM		multiAddrSem;
		    GT_SEM		atuRegsSem;
		    GT_SEM		vtuRegsSem;
		    GT_SEM		statsRegsSem;
		    GT_SEM		pirlRegsSem;
    		GT_SEM		ptpRegsSem;
		    GT_SEM		tblRegsSem;
		    GT_SEM		phyRegsSem;
		    */
			gtRet += gtSemGive(dev,dev->multiAddrSem);
			gtRet += gtSemGive(dev,dev->atuRegsSem);
			gtRet += gtSemGive(dev,dev->vtuRegsSem);
			gtRet += gtSemGive(dev,dev->statsRegsSem);
			gtRet += gtSemGive(dev,dev->pirlRegsSem);
			gtRet += gtSemGive(dev,dev->ptpRegsSem);
			gtRet += gtSemGive(dev,dev->tblRegsSem);
			gtRet += gtSemGive(dev,dev->phyRegsSem);
		}
	}
	dev = QD_MASTER_DEV_PTR;
#if (PRODUCT_CLASS == GT861_ONU)
	gtRet += gtSemGive(dev,smiAccessSem);
#endif
	return gtRet;
}

#if 0 //mtodo ��ʱ�رն�оƬ���ӷ�ʽ�Ķ˿�ģʽ����

GT_STATUS setDSAMode(GT_BOOL mode)
{
	GT_LPORT unit, phyPort;
	GT_STATUS status;
	GT_STATUS l_ret_val=GT_OK;
	
	FOR_UNIT_LOOP(unit)
	{
		if(UNIT_IS_MASTER(unit))
			FOR_UNIT_LOOP_CONTINUE;

		if(NULL == qdMultiDev[unit])
			FOR_UNIT_LOOP_CONTINUE;
			
		if(GT_TRUE == mode)
		{
			if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_FRAME_MODE))
			{
				if((status=gprtSetFrameMode(qdMultiDev[unit],dev_cpuPort[unit], GT_FRAME_MODE_DSA)) != GT_OK)
				{
					MSG_OUT(("gprtSetFrameMode returned %d (port %d, DSA_MODE)\r\n",status,dev_cpuPort[unit]));
					l_ret_val = status;
				}

				if(GT_OK == get_switch_cascade_port_on_master(unit, &phyPort))
				{
					if (IS_IN_DEV_GROUP(qdMultiDev[0],DEV_FRAME_MODE))
					{
						if((status=gprtSetFrameMode(qdMultiDev[0],phyPort,GT_FRAME_MODE_DSA)) != GT_OK)
						{
							MSG_OUT(("gprtSetFrameMode returned %d (port %d, mode GT_FRAME_MODE_DSA)\r\n",status,phyPort));
							l_ret_val = status;
						}
					}
					else
					{
						if((status=gprtSetInterswitchPort(qdMultiDev[0],phyPort,GT_TRUE)) != GT_OK)
						{
							MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_TRUE)\r\n",status,phyPort));
							l_ret_val = status;
						}
					}
				}
		
			}
			else  	
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[unit],dev_cpuPort[unit],GT_TRUE)) != GT_OK)
				{
					MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_TRUE)\r\n",status,dev_cpuPort[unit]));
					l_ret_val = status;
				}
				
				if(GT_OK == get_switch_cascade_port_on_master(unit, &phyPort))
				{
					if (IS_IN_DEV_GROUP(qdMultiDev[0],DEV_FRAME_MODE))
					{
						if((status=gprtSetFrameMode(qdMultiDev[0],phyPort,GT_FRAME_MODE_DSA)) != GT_OK)
						{
							MSG_OUT(("gprtSetFrameMode returned %d (port %d, mode MODE_DSA)\r\n",status,phyPort));
							l_ret_val = status;
						}
					}
					else
					{
						if((status=gprtSetInterswitchPort(qdMultiDev[0],phyPort,GT_TRUE)) != GT_OK)
						{
							MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_TRUE)\r\n",status,phyPort));
							l_ret_val = status;
						}
					}
				}
			}
		}
		else
		{
			if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_FRAME_MODE))
			{
				if((status=gprtSetFrameMode(qdMultiDev[unit],dev_cpuPort[unit], GT_FRAME_MODE_NORMAL)) != GT_OK)
				{
					MSG_OUT(("gprtSetFrameMode returned %d (port %d, NORMAL)\r\n",status,dev_cpuPort[unit]));
					l_ret_val = status;
				}

				if(GT_OK == get_switch_cascade_port_on_master(unit, &phyPort))
				{
					if (IS_IN_DEV_GROUP(qdMultiDev[0],DEV_FRAME_MODE))
					{
						if((status=gprtSetFrameMode(qdMultiDev[0],phyPort,GT_FRAME_MODE_NORMAL)) != GT_OK)
						{
							MSG_OUT(("gprtSetFrameMode returned %d (port %d, mode NORMAL)\r\n",status,phyPort));
							l_ret_val = status;
						}
					}
					else
					{
						if((status=gprtSetInterswitchPort(qdMultiDev[0],phyPort,GT_FALSE)) != GT_OK)
						{
							MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_FALSE)\r\n",status,phyPort));
							l_ret_val = status;
						}
					}
				}
		
			}
			else  	
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[unit],dev_cpuPort[unit],GT_FALSE)) != GT_OK)
				{
					MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_FALSE)\r\n",status,dev_cpuPort[unit]));
					l_ret_val = status;
				}
				
				if(GT_OK == get_switch_cascade_port_on_master(unit, &phyPort))
				{
					if (IS_IN_DEV_GROUP(qdMultiDev[0],DEV_FRAME_MODE))
					{
						if((status=gprtSetFrameMode(qdMultiDev[0],phyPort,GT_FRAME_MODE_NORMAL)) != GT_OK)
						{
							MSG_OUT(("gprtSetFrameMode returned %d (port %d, mode NORMAL)\r\n",status,phyPort));
							l_ret_val = status;
						}
					}
					else
					{
						if((status=gprtSetInterswitchPort(qdMultiDev[0],phyPort,GT_FALSE)) != GT_OK)
						{
							MSG_OUT(("gprtSetInterswitchPort returned %d (port %d, mode GT_FALSE)\r\n",status,phyPort));
							l_ret_val = status;
						}
					}
				}
			}
		}
	}
	
	return l_ret_val;
}
#endif

GT_BOOL isInterswitchPort(GT_LPORT port)
{
	if(port >= MAXPortOnSlotNum)
		return GT_FALSE;
	else
		return master_interswitch_port[port];
}

GT_STATUS InternalVlanDestroy(GT_U32 vid)
{

	GT_STATUS ret = GT_OK;
	GT_VTU_ENTRY entry;

	memset(&entry, 0, sizeof(entry));

	entry.vid = vid;
	entry.DBNum = vid;


	FOR_UNIT_START(GT_32, unit)

	ret |= gvtuDelEntry(QD_DEV_PTR, &entry);

	FOR_UNIT_END

	return (ret == GT_OK)?GT_OK:GT_ERROR;
}

GT_STATUS InternalVlanInit(GT_QD_DEV * dev, GT_U32 vid)
{
    GT_VTU_ENTRY    vtuEntry;
    GT_LPORT		phyPort;
    GT_STATUS		result;
    GT_STATUS		l_ret_val=GT_OK;
	GT_ATU_ENTRY 	atuEntryBc;
	int i;

	memset(&vtuEntry, 0, sizeof (GT_VTU_ENTRY));
    vtuEntry.DBNum = vid;
    vtuEntry.vid = vid;
  	for ( phyPort=0; phyPort<dev->maxPorts; phyPort++ )
    {
    	vtuEntry.vtuData.memberTagP[phyPort] = NOT_A_MEMBER;
	}
	vtuEntry.vtuData.memberTagP[dev->cpuPortNum] = MEMBER_EGRESS_TAGGED;
	vtuEntry.vtuData.memberTagP[dev_wanPort[0]] = MEMBER_EGRESS_TAGGED; /* Need change */
    if ((result = gvtuAddEntry(dev, &vtuEntry)) != GT_OK)
	{
		MSG_OUT(( "gvtuAddEntry return Failed(%d)\r\n", result));
		l_ret_val = result;
	}

	if (!IS_IN_DEV_GROUP(dev,DEV_FLOOD_BROADCAST))
    {
		/* Broadcast atu entry */
	    atuEntryBc.DBNum = vid; 
	    atuEntryBc.exPrio.macFPri = 0;
	    atuEntryBc.exPrio.macQPri = 0;
	    atuEntryBc.exPrio.useMacFPri = GT_FALSE;
	    atuEntryBc.prio = 0;
	    atuEntryBc.trunkMember = GT_FALSE;
	    for (i=0; i<GT_ETHERNET_HEADER_SIZE; i++)
	        atuEntryBc.macAddr.arEther[i] = 0xFF;
	    atuEntryBc.portVec = 0;
	    atuEntryBc.portVec |= (1<<(dev->cpuPortNum));
#if (FOR_ONU_PON)
	    atuEntryBc.portVec |= (1<<(dev_wanPort[0]));	/* Need change */
#endif
	    
	    atuEntryBc.entryState.mcEntryState = GT_MC_STATIC;
	    if ((result = gfdbAddMacEntry(dev, &atuEntryBc)) != GT_OK)
		{
			MSG_OUT(( "gfdbAddMacEntry return Failed(%d)\r\n", result));
			l_ret_val = result;
		}
	}


	return l_ret_val;
}

GT_STATUS InternalVlanJoin(GT_QD_DEV * dev, GT_U32 vid, GT_LPORT phyPort, GT_BOOL tagged)
{
    GT_VTU_ENTRY    vtuEntry;
    GT_BOOL			bFound;
    GT_STATUS		result;
	GT_ATU_ENTRY 	atuEntryBc;
    int i;

	memset(&vtuEntry, 0, sizeof (GT_VTU_ENTRY));

	vtuEntry.DBNum = vid;
	vtuEntry.vid = vid;
	if ((result = gvtuFindVidEntry(dev, &vtuEntry, &bFound)) == GT_FAIL)
	{
		MSG_OUT(( "gvtuFindVidEntry return Failed(%d)\r\n", result));
		return result;
	}

	if(GT_TRUE == bFound)
	{
		vtuEntry.vtuData.memberTagP[phyPort] = tagged?MEMBER_EGRESS_TAGGED:MEMBER_EGRESS_UNTAGGED;
	    if ((result = gvtuAddEntry(dev, &vtuEntry)) != GT_OK)
		{
			MSG_OUT(( "gvtuAddEntry return Failed(%d)\r\n", result));
			return result;
		}

		if (!IS_IN_DEV_GROUP(dev,DEV_FLOOD_BROADCAST))
	    {
			/* Broadcast atu entry */
		    atuEntryBc.DBNum = vid; 
		    for (i=0; i<GT_ETHERNET_HEADER_SIZE; i++)
		        atuEntryBc.macAddr.arEther[i] = 0xFF;
		    if ((result = gfdbFindAtuMacEntry(dev, &atuEntryBc, &bFound )) != GT_OK)
		    {
				MSG_OUT(( "gfdbFindAtuMacEntry return Failed(%d)\r\n", result));
		    	return result;
		    }
		    else
		    {
				if(GT_TRUE == bFound)
				{
				    atuEntryBc.portVec |= (1<<phyPort);
			    
				    atuEntryBc.entryState.mcEntryState = GT_MC_STATIC;
				    if ((result = gfdbAddMacEntry(dev, &atuEntryBc)) != GT_OK)
					{
						MSG_OUT(( "gfdbAddMacEntry return Failed(%d)\r\n", result));
						return result;
					}
				}
				else
				{
					return GT_FAIL;
				}
			}
		}

		return GT_OK;
	}
	else
	{
		return GT_FAIL;
	}
}

GT_STATUS InternalVlanLeave(GT_QD_DEV * dev, GT_U32 vid, GT_LPORT phyPort)
{
    GT_VTU_ENTRY    vtuEntry;
    GT_BOOL			bFound;
    GT_STATUS		result;
	GT_ATU_ENTRY 	atuEntryBc;
    int i;

	memset(&vtuEntry, 0, sizeof (GT_VTU_ENTRY));

	vtuEntry.DBNum = vid;
	vtuEntry.vid = vid;
	if ((result = gvtuFindVidEntry(dev, &vtuEntry, &bFound)) == GT_FAIL)
	{
		MSG_OUT(( "gvtuFindVidEntry return Failed(%d)\r\n", result));
		return result;
	}

	if(GT_TRUE != bFound)
	{
		return GT_OK;
	}
	else
	{
		vtuEntry.vtuData.memberTagP[phyPort] = NOT_A_MEMBER;
	    if ((result = gvtuAddEntry(dev, &vtuEntry)) != GT_OK)
		{
			MSG_OUT(( "gvtuAddEntry return Failed(%d)\r\n", result));
			return result;
		}
		if (!IS_IN_DEV_GROUP(dev,DEV_FLOOD_BROADCAST))
	    {
			/* Broadcast atu entry */
		    atuEntryBc.DBNum = vid; 
		    for (i=0; i<GT_ETHERNET_HEADER_SIZE; i++)
		        atuEntryBc.macAddr.arEther[i] = 0xFF;
		    if ((result = gfdbFindAtuMacEntry(dev, &atuEntryBc, &bFound )) != GT_OK)
		    {
				MSG_OUT(( "gfdbFindAtuMacEntry return Failed(%d)\r\n", result));
		    	return result;
		    }
		    else
		    {
				if(GT_TRUE == bFound)
				{
				    atuEntryBc.portVec &= (~(1<<phyPort));
			    
				    atuEntryBc.entryState.mcEntryState = GT_MC_STATIC;
				    if ((result = gfdbAddMacEntry(dev, &atuEntryBc)) != GT_OK)
					{
						MSG_OUT(( "gfdbAddMacEntry return Failed(%d)\r\n", result));
						return result;
					}
				}
				else
				{
					return GT_FAIL;
				}
			}
		}
		return GT_OK;
	}
}

GT_STATUS IsolateTwoPorts(GT_QD_DEV * dev, GT_BOOL mode, GT_LPORT phyPortA, GT_LPORT phyPortB)
{
    GT_STATUS       retVal = GT_OK; /* Functions return value.      */
    GT_U8           i;
    GT_LPORT        memPorts[TOTAL_NUM_OF_PORTS];
    GT_U8			memPortsLen;
    GT_BOOL			found;

    switch (mode)
    {
        case GT_TRUE:
        	retVal = gvlnGetPortVlanPorts(dev, phyPortA, memPorts, &memPortsLen);
            if (GT_OK == retVal)
            {
                for (i=0; i<memPortsLen; i++)
                {
                	if(memPorts[i] == phyPortB)
                	{
                		memPorts[i] = memPorts[memPortsLen - 1];
                		break;
                	}
                }
                retVal = gvlnSetPortVlanPorts(dev, phyPortA, memPorts, memPortsLen-1);
            }

        	retVal = gvlnGetPortVlanPorts(dev, phyPortB, memPorts, &memPortsLen);
            if (GT_OK == retVal)
            {
                for (i=0; i<memPortsLen; i++)
                {
                	if(memPorts[i] == phyPortA)
                	{
                		memPorts[i] = memPorts[memPortsLen - 1];
                		break;
                	}
                }
                retVal = gvlnSetPortVlanPorts(dev, phyPortB, memPorts, memPortsLen-1);
            }

            break;
        case GT_FALSE:
        	retVal = gvlnGetPortVlanPorts(dev, phyPortA, memPorts, &memPortsLen);
            if (GT_OK == retVal)
            {
				found = GT_FALSE;
                for (i=0; i<memPortsLen; i++)
                {
                	if(memPorts[i] == phyPortB)
                	{
                		found = GT_TRUE;
                		break;
                	}
                }
                if(GT_FALSE == found)
                {
					memPorts[memPortsLen] = phyPortB;
                	retVal = gvlnSetPortVlanPorts(dev, phyPortA, memPorts, memPortsLen+1);
                }
            }

        	retVal = gvlnGetPortVlanPorts(dev, phyPortB, memPorts, &memPortsLen);
            if (GT_OK == retVal)
            {
				found = GT_FALSE;
                for (i=0; i<memPortsLen; i++)
                {
                	if(memPorts[i] == phyPortA)
                	{
                		found = GT_TRUE;
                		break;
                	}
                }
                if(GT_FALSE == found)
                {
					memPorts[memPortsLen] = phyPortA;
                	retVal = gvlnSetPortVlanPorts(dev, phyPortB, memPorts, memPortsLen+1);
                }
            }
            
            break;
        default:
        	break;
    }

    return retVal;
}

GT_STATUS switch_fe_port_pause_ctrl(GT_QD_DEV * dev, GT_LPORT phyPort, GT_BOOL mode)
{
	GT_STATUS ret=GT_OK;
	GT_BOOL gtAnEn;
	GT_BOOL gtDuplex;
	GT_BOOL gtFcVal;
	GT_PHY_PAUSE_MODE gtPauseMode;
	GT_BOOL gtFcForce;
	
	ret = gpcsGetFCValue(dev, phyPort, &gtFcVal);
	ret = gprtGetPhyPause(dev, phyPort, &gtPauseMode);
	ret = gpcsGetForcedFC(dev, phyPort, &gtFcForce);
	switch (mode)
	{
		case GT_TRUE:
			ret += gprtGetDuplex(dev, phyPort, &gtDuplex);
			if ( GT_OK == ret ) 
			{
				if( GT_TRUE == gtDuplex )
					ret += gsysSetFlowCtrlDelay(dev, PORT_SPEED_100_MBPS, 0xE7);
				else
					ret += gsysSetFlowCtrlDelay(dev, PORT_SPEED_100_MBPS, 0x61);
			}
			ret += gprtSetFCThreshold(dev, phyPort, 0x0022);
			ret += gprtSetFCThresholdDrop(dev, phyPort, 0x2222);
			ret += gprtGetPortAutoNegEnable(dev, phyPort, &gtAnEn);
			if (( GT_OK == ret ) && ( GT_TRUE == gtAnEn ))
			{
				if(GT_PHY_NO_PAUSE == gtPauseMode)
				{
					ret += gpcsSetFCValue(dev, phyPort, GT_FALSE);
					ret += gpcsSetForcedFC(dev, phyPort, GT_FALSE);
					ret += gprtSetPause(dev, phyPort, GT_PHY_PAUSE);
					ret += gprtPortRestartAutoNeg(dev, phyPort);
				}
			}
			else
			{
				if(GT_FALSE == gtFcVal)
				{
					ret += gpcsSetFCValue(dev, phyPort, GT_TRUE);
				}
				if(GT_FALSE == gtFcForce)
				{
					ret += gpcsSetForcedFC(dev, phyPort, GT_TRUE);
				}
			}

			break;

		case GT_FALSE:
			ret += gprtSetFCThreshold(dev, phyPort, 0x0321);
			ret += gprtSetFCThresholdDrop(dev, phyPort, 0x4444);
			ret += gprtGetPortAutoNegEnable(dev, phyPort, &gtAnEn);
			if (( GT_OK == ret ) && ( GT_TRUE == gtAnEn ))
			{
				if(GT_PHY_NO_PAUSE != gtPauseMode)
				{
					ret += gprtSetPause(dev, phyPort, GT_PHY_NO_PAUSE);
					ret += gprtPortRestartAutoNeg(dev, phyPort);
				}
			}
			else
			{
				if(GT_FALSE != gtFcVal)
				{
					ret += gpcsSetFCValue(dev, phyPort, GT_FALSE);
				}
				if(GT_FALSE == gtFcForce)
				{
					ret += gpcsSetForcedFC(dev, phyPort, GT_TRUE);
				}
			}
			break;

		default:
			return GT_BAD_PARAM;
	}

	return ret;
}

GT_STATUS switch_fpga_port_enable(GT_BOOL mode)
{
	GT_STATUS result=GT_OK;
	GT_QD_DEV *dev;
	GT_LPORT phyPort;
	GT_PORT_STP_STATE stp_state;

	if(GT_TRUE == mode)
		stp_state = GT_PORT_FORWARDING;
	else
		stp_state = GT_PORT_DISABLE;

	if(NULL == (dev = qdMultiDev[0])) return GT_BAD_PARAM;

	phyPort = DEVICE0_FPGA_PORT;
		
    if ((result = gstpSetPortState(dev, phyPort, stp_state)) != GT_OK)
	{
		MSG_OUT(( "gstpSetPortState return Failed(%d)\r\n", result));
	}

	return result;
}


/*set cpu egress rate to cut the download time.*/
GT_STATUS cpu_rate_fast(int enable)
{
 	GT_ERATE_TYPE   gtEgressRateType;
    int unit = 0;
    if (0 != enable)/*set cpu egress rate no limit.*/
    {
    	/* Egress no limit */
    	if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_ELIMIT_FRAME_BASED))
    		gtEgressRateType.kbRate = 0;
    	else
    		gtEgressRateType.definedRate = GT_NO_LIMIT;
        if (GT_OK != grcSetEgressRate(qdMultiDev[unit], dev_cpuPort[unit], &gtEgressRateType))
        {
            printf(" set cpu rate no limit failed.\r\n");
            return 1;
        }
    }
    else
    {
		/* Egress 64kbps */
		if (IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_ELIMIT_FRAME_BASED))
			gtEgressRateType.kbRate = 64;
        else if (!IS_IN_DEV_GROUP(qdMultiDev[unit],DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
            gtEgressRateType.definedRate = GT_128K;
		else
			gtEgressRateType.definedRate = GT_64K;
        if (GT_OK != grcSetEgressRate(qdMultiDev[unit], dev_cpuPort[unit], &gtEgressRateType))
    	{
            printf(" set cpu rate limit failed.\r\n");
            return 1;
    	}
    }
    return 0;
}

GT_STATUS gt_getswitchunitbylport(GT_LPORT port, GT_32 *unit, GT_32 *lport )
{
	if(port == CS_UPLINK_PHY_PORT)
	{
		*unit = MASTER_UNIT;
		*lport = port;
	}
	else
	{
		*unit = port/NUM_UNI_PORTS_PER_UNIT;
		*lport = port%NUM_UNI_PORTS_PER_UNIT;
	}
	return GT_OK;
}


extern GT_STATUS miiSmiIfReadRegister
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U8        phyAddr,
    IN  GT_U8        regAddr,
    OUT GT_U16       *data
);
extern GT_STATUS miiSmiIfWriteRegister
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U8        phyAddr,
    IN  GT_U8        regAddr,
    IN  GT_U16       data
);
//extern GT_QD_DEV       *qdMultiDev[N_OF_QD_DEVICES];
int mrv_reg_read(unsigned char dev_addr, unsigned char reg_addr, unsigned short *data)
{
    int   retVal;

    retVal = miiSmiIfReadRegister(QD_MASTER_DEV_PTR,dev_addr,reg_addr,data);

    return retVal;
}
int mrv_reg_write(unsigned char dev_addr, unsigned char reg_addr, unsigned short data)
{
    int   retVal;

    retVal = miiSmiIfWriteRegister(QD_MASTER_DEV_PTR,dev_addr,reg_addr,data);

    return retVal;
}



#if 1

#define MRV_SWITCH_ID_REG	0x03
#define MRV_PORT_CONTROL_REG 0x04
#define MRV_GLOBAL_CONTROL_REG 0x04
#define MRV_PHY_SPEC_CONTROL_REG 0x10
#define MRV_PHY_SPEC_CONTROL_II_REG 0x1C
#define MRV_PHY_AUTONEGO_AD_REG 0x04
#define MRV_PHY_CONTROL_REG 0x00
#define MRV_PCS_CONTROL_REG 0x01
#define MRV_PORT_STATUS_REG 0x00

#define MRV_PORT_FORWARDING 0x03
#define MRV_88E6083		0x83
#define MRV_88E6096		0x98
#define MRV_PHY_100_FULL 0x100
#define MRV_PHY_100_HALF 0x80
#define MRV_PHY_10_FULL 0x40
#define MRV_PHY_10_HALF 0x20
#define MRV_PHY_MODE_AUTO_AUTO MRV_PHY_100_FULL|MRV_PHY_100_HALF|MRV_PHY_10_FULL|MRV_PHY_10_HALF

#define MRV_PHY_SPEED 0x2000
#define MRV_PHY_DUPLEX 0x100
#define MRV_PHY_AUTONEGO 0x1000
//extern cs_status cs_mdio_write(cs_uint8 device, cs_uint8 reg, cs_uint16 data);
//extern cs_status cs_mdio_read(cs_uint8 device, cs_uint8 reg, cs_uint32 *data);

void mrv_switch_init_t()
{
	mrv_switch_init(QD_MASTER_DEV_PTR);
}
void mrv_switch_init(IN  GT_QD_DEV    *dev)
{
	unsigned short datatmp = 0;
	unsigned short mask;
	unsigned int numOfPorts;
	unsigned int maxcopperPorts = 6;/*GIS2109系列电口最多6个*/
	unsigned int cpuPortNum;
	unsigned int   deviceId;
	int i;
	int status = -1;

	if((status = miiSmiIfReadRegister(dev,0x10,MRV_SWITCH_ID_REG,&datatmp)) != GT_OK)
		{
//			return ;
		}
	cs_printf("datatmp is 0x%x\r\n",datatmp);
	deviceId = datatmp >> 4;
	cs_printf("datatmp is 0x%x\r\n",deviceId);
	switch(deviceId)
	{
		case MRV_88E6083:
			numOfPorts = 10;
			cpuPortNum = 8;
			break;
		case MRV_88E6096:
			numOfPorts = 11;
			cpuPortNum = 10;
			break;
		default:
			return ;
	}
	cs_printf("mrv_switch_init 11\r\n");
	for(i=0;i<numOfPorts;i++)
	{
		miiSmiIfReadRegister(dev,0x10+i,MRV_PORT_CONTROL_REG,&datatmp);
		datatmp |= MRV_PORT_FORWARDING;
		miiSmiIfWriteRegister(dev,0x10+i,MRV_PORT_CONTROL_REG,datatmp);
	}
	cs_printf("mrv_switch_init 22\r\n");
	miiSmiIfReadRegister(dev,0x1B,MRV_GLOBAL_CONTROL_REG,&datatmp);/*disable PPU*/
	mask = 1U << 14;
	datatmp &= ~mask;
	miiSmiIfWriteRegister(dev,0x1B,MRV_GLOBAL_CONTROL_REG,datatmp);
	cs_printf("mrv_switch_init 33\r\n");
	for(i=0; i<maxcopperPorts; i++)/*只设电口phy，避免设置GIS2109-6T-3G的cpu口(7口)的外接phy，否则bsp下cpu不通*/
	{
		miiSmiIfReadRegister(dev,i,MRV_PHY_SPEC_CONTROL_REG,&datatmp);/*disable Energydetect*/
		mask = 1U << 14;
		datatmp &= ~mask;
		miiSmiIfWriteRegister(dev,i,MRV_PHY_SPEC_CONTROL_REG,datatmp);

		miiSmiIfReadRegister(dev,i,MRV_PHY_SPEC_CONTROL_II_REG,&datatmp);/*Set PHY to CLASS A mode*/
		datatmp |= 1U;
		miiSmiIfWriteRegister(dev,i,MRV_PHY_SPEC_CONTROL_II_REG,datatmp);

		miiSmiIfReadRegister(dev,i,MRV_PHY_AUTONEGO_AD_REG,&datatmp);/*set phy AUTO_AUTO*/
		datatmp |= MRV_PHY_MODE_AUTO_AUTO;
		miiSmiIfWriteRegister(dev,i,MRV_PHY_AUTONEGO_AD_REG,datatmp);

		miiSmiIfReadRegister(dev,i,MRV_PHY_CONTROL_REG,&datatmp);/*enable autoneg、reatart autoneg*/
		datatmp = (datatmp & (MRV_PHY_SPEED | MRV_PHY_DUPLEX)) | MRV_PHY_AUTONEGO;
		if(datatmp & 1U<<11)
			datatmp &= ~(1U<<11);
		else
			datatmp |= 1U<<15;
		miiSmiIfWriteRegister(dev,i,MRV_PHY_CONTROL_REG,datatmp);
	}
	cs_printf("mrv_switch_init 44\r\n");
	return;
}
#endif





