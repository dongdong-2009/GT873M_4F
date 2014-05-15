#include <Copyright.h>
/********************************************************************************
* msApiInit.c
*
* DESCRIPTION:
*       MS API initialization routine
*
* DEPENDENCIES:   Platform
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"

#include "MARVELL_BSP_expo.h"
#include "cs_types.h"
#include "mdio.h"
/*
#define MULTI_ADDR_MODE
#define MANUAL_MODE
*/

GT_SYS_CONFIG   cfg;
GT_QD_DEV       diagDev;
GT_QD_DEV       *dev=&diagDev;


/*
 *  Initialize the QuarterDeck. This should be done in BSP driver init routine.
 *	Since BSP is not combined with QuarterDeck driver, we are doing here.
*/

GT_STATUS qdStart(int cpuPort, int useQdSim, int devId) /* devId is used for simulator only */
{
GT_STATUS status;

	/*
	 *  Register all the required functions to QuarterDeck Driver.
	*/
	memset((char*)&cfg,0,sizeof(GT_SYS_CONFIG));
	memset((char*)&diagDev,0,sizeof(GT_QD_DEV));

	if(useQdSim == 0) /* use EV-96122 */
	{
		cfg.BSPFunctions.readMii   = gtBspReadMii;
		cfg.BSPFunctions.writeMii  = gtBspWriteMii;
#ifdef USE_SEMAPHORE
		cfg.BSPFunctions.semCreate = osSemCreate;
		cfg.BSPFunctions.semDelete = osSemDelete;
		cfg.BSPFunctions.semTake   = osSemWait;
		cfg.BSPFunctions.semGive   = osSemSignal;
#else
		cfg.BSPFunctions.semCreate = NULL;
		cfg.BSPFunctions.semDelete = NULL;
		cfg.BSPFunctions.semTake   = NULL;
		cfg.BSPFunctions.semGive   = NULL;
#endif
		gtBspMiiInit(dev);
	}
#if 0
	else	/* use QuaterDeck Simulator (No QD Device Required.) */
	{
		cfg.BSPFunctions.readMii   = qdSimRead;
		cfg.BSPFunctions.writeMii  = qdSimWrite;
#ifdef USE_SEMAPHORE
		cfg.BSPFunctions.semCreate = osSemCreate;
		cfg.BSPFunctions.semDelete = osSemDelete;
		cfg.BSPFunctions.semTake   = osSemWait;
		cfg.BSPFunctions.semGive   = osSemSignal;
#else
		cfg.BSPFunctions.semCreate = NULL;
		cfg.BSPFunctions.semDelete = NULL;
		cfg.BSPFunctions.semTake   = NULL;
		cfg.BSPFunctions.semGive   = NULL;
#endif

		qdSimInit(devId,0);
	}
#endif

	cfg.initPorts = GT_TRUE;	/* Set switch ports to Forwarding mode. If GT_FALSE, use Default Setting. */
	cfg.cpuPortNum = cpuPort;
#ifdef MANUAL_MODE	/* not defined. this is only for sample */
	/* user may want to use this mode when there are two QD switchs on the same MII bus. */
	cfg.mode.scanMode = SMI_MANUAL_MODE;	/* Use QD located at manually defined base addr */
	cfg.mode.baseAddr = 0x10;	/* valid value in this case is either 0 or 0x10 */
#else
#ifdef MULTI_ADDR_MODE
	cfg.mode.scanMode = SMI_MULTI_ADDR_MODE;	/* find a QD in indirect access mode */
	cfg.mode.baseAddr = 1;		/* this is the phyAddr used by QD family device. 
								Valid value are 1 ~ 31.*/
#else
	cfg.mode.scanMode = SMI_AUTO_SCAN_MODE;	/* Scan 0 or 0x10 base address to find the QD */
	cfg.mode.baseAddr = 0;
#endif
#endif
	if((status=qdLoadDriver(&cfg, dev)) != GT_OK)
	{
		MSG_PRINT(("qdLoadDriver return Failed\n"));
		return status;
	}

	MSG_PRINT(("Device ID     : 0x%x\n",dev->deviceId));
	MSG_PRINT(("Base Reg Addr : 0x%x\n",dev->baseRegAddr));
	MSG_PRINT(("No of Ports   : %d\n",dev->numOfPorts));
	MSG_PRINT(("CPU Ports     : %d\n",(int)dev->cpuPortNum));

	/*
	 *  start the QuarterDeck
	*/
	if((status=sysEnable(dev)) != GT_OK)
	{
		MSG_PRINT(("sysConfig return Failed\n"));
		return status;
	}

	MSG_PRINT(("QuarterDeck has been started.\n"));

	return GT_OK;
}
//extern cs_status cs_mdio_speed_set(cs_uint8 	intf_addr, cs_uint32 freq_khz);
int marvellMdioInit()
{
  cs_uint8 i = 0;

  for(i=0; i<MDIO_MAX_DEVICE_NUM; i++)
  {
	  if(GT_OK != cs_mdio_speed_set(i, 4000))
	  {
		  cs_printf("init mdio %d error \r\n",i);
	  }
  }
  return GT_OK;
}
/*******************************************************************************************
  Description   : init the Marvell switch driver for Passave ONU boards
  In params     : cfg       -   pointer to Marvell switch driver configuration structure
                                the following structure variables are mandatory:
                                - cpuPortNum
                                - mode.scanMode
                  ext_conf  -   pointer to Marvell extended configuration structure.
                                (all structure variables are mandatory)
                  dev - pointer to Marvell switch driver info structure
  Out params    : dev - pointer to Marvell switch driver info structure
  Return value  : standard PAS6201 API error codes
********************************************************************************************/
GT_STATUS Marvell_driver_initialize(GT_SYS_CONFIG * cfg, GT_QD_DEV * dev,
	Marvell_ext_switch_conf_t * ext_conf)
{

    GT_STATUS status;
    int i;

	/*
	 *  Register all the required functions to QuarterDeck Driver.
	 */
    cfg->BSPFunctions.readMii   = gtBspReadMii;
	cfg->BSPFunctions.writeMii  = gtBspWriteMii;
#ifdef USE_SEMAPHORE
	cfg->BSPFunctions.semCreate = osSemCreate;
	cfg->BSPFunctions.semDelete = osSemDelete;
	cfg->BSPFunctions.semTake   = osSemWait;
	cfg->BSPFunctions.semGive   = osSemSignal;
#else
	cfg->BSPFunctions.semCreate = NULL;
	cfg->BSPFunctions.semDelete = NULL;
	cfg->BSPFunctions.semTake   = NULL;
	cfg->BSPFunctions.semGive   = NULL;
#endif
	marvellMdioInit();
	gtBspMiiInit(dev);

	if((status=qdLoadDriver(cfg, dev)) != GT_OK)
	{
        MSG_OUT(("qdLoadDriver return Failed(%d)\r\n",status));
		return status;
	}

    MSG_OUT(("\r\nSWITCH: ********************************\r\n"));
    switch(cfg->mode.scanMode)
    {
        case SMI_AUTO_SCAN_MODE :
            MSG_OUT(("SWITCH: * Scan mode     : Auto         *\r\n"));
            break;
        case SMI_MANUAL_MODE :
            MSG_OUT(("SWITCH: * Scan mode     : Manual       *\r\n"));
            break;
        case SMI_MULTI_ADDR_MODE :
            MSG_OUT(("SWITCH: * Scan mode     : MultiAddress *\r\n"));
            break;
		default:
			MSG_OUT(("SWITCH: * Scan mode     : Unknown      *\r\n"));
			break;
    }
	MSG_OUT(("SWITCH: * Device ID     : 0x%x         *\r\n", dev->deviceId));
	MSG_OUT(("SWITCH: * Phy Addr      : 0x%x          *\r\n", dev->phyAddr));
	MSG_OUT(("SWITCH: * Base Reg Addr : 0x%x          *\r\n", dev->baseRegAddr));
	MSG_OUT(("SWITCH: * Num of Ports  : %d           *\r\n", dev->numOfPorts));
	MSG_OUT(("SWITCH: * CPU Port      : %02lu           *\r\n", dev->cpuPortNum));
	for(i=0; i<ext_conf->numOfCasPorts; i++)
		MSG_OUT(("SWITCH: * Cascade Port  : %02lu           *\r\n", ext_conf->cas_ports[i].port));
	MSG_OUT(("SWITCH: * Uplink Port   : %02lu           *\r\n", ext_conf->upLinkPort));

    MSG_OUT(("SWITCH: ********************************\r\n\r\n"));

	/*
	 *  start the QuarterDeck
	*/
	if((status=sysEnable(dev)) != GT_OK)
	{
		MSG_OUT(("sysConfig return Failed\r\n"));
		return status;
	}

	MSG_OUT(("SWITCH: QuarterDeck has been started.\r\n"));

	return GT_OK;
}

