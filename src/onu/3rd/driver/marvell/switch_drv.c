/********************************************************************************
* switchInit.c
*
* DESCRIPTION:
*       Initialize Marvell switch driver and ports. 
*
* FILE REVISION NUMBER:
*       Liud 2007-05-18 Initial release  
*       
*******************************************************************************/

#include <string.h>
#include <stdio.h>
/* Marvell driver API protetype files */
#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvConfig.h>
#include <gtDrvSwRegs.h>
#include <gtVct.h>
#include <gtSem.h>

#include "MARVELL_BSP_expo.h"
#include "switch_expo.h"
#include "switch_drv.h"

#include "cs_types.h"

extern GT_U32   ulMrvTagtype;

extern GT_STATUS cBurstEnum2Number(GT_QD_DEV *dev, GT_BURST_RATE rate, GT_U32 *rLimit);
extern cs_int32 boards_logical_to_physical(cs_ulong32 lport, cs_ulong32 *unit, cs_ulong32 *pport);

/* For GT812 use: Logical to Physical port map */
#if (FOR_ONU_812_AB)
#ifdef _FOR_DARE_HW_
GT_U8 hwPortID[12] = {0, 7, 6, 5, 4, 3, 2, 1, 0, 10, 0, 0};
GT_U8 swPortID[12] = {8, 7, 6, 5, 4, 3, 2, 1, 11, 12, 9, 13};
#else
GT_U8 hwPortID_Dare[12] = {0, 7, 6, 5, 4, 3, 2, 1, 0, 10, 0, 0};
GT_U8 swPortID_Dare[12] = {8, 7, 6, 5, 4, 3, 2, 1, 11, 12, 9, 13};
GT_U8 hwPortID[12] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 10, 0, 0};
GT_U8 swPortID[12] = {1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 9, 13};
#endif
#elif (PRODUCT_CLASS == GT811)	/* TODO LD OK */
#ifdef _FOR_DARE_HW_
GT_U8 hwPortID[12] = {0, 3, 2, 1, 0, 10, 4, 5, 6, 7, 0, 0};
GT_U8 swPortID[12] = {4, 3, 2, 1, 6, 7, 8, 9, 10, 12, 5, 13};
#else
GT_U8 hwPortID[12] = {0, 0, 1, 2, 3, 10, 4, 5, 6, 7, 0, 0};
GT_U8 swPortID[12] = {1, 2, 3, 4, 6, 7, 8, 9, 10, 12, 5, 13};
#endif
#endif

GT_BOOL gtVlnPortIsolate = GT_FALSE;
GT_BOOL gtPortInSeries = GT_FALSE;

GT_U32	gulTypeAll, gulTypeFlood, gulTypeMc, gulTypeBc, gulTypeUc;
TRUNK_MEMBER trunkGroup[MAX_TRUNK_GROUP]={{0}};
GT_U32		balance = 1;


/*******************************************************************************
* gtSetCpuFrameMode
*
* DESCRIPTION:
* 		Set the Cpu port frame mode 
*
* INPUTS:
* 		MrvTagMode:
*			MRVTAG_NONE  (don't use tag)
*			MRVTAG_DSA   (mrv tag use dsa)
*			MRVTAG_HEAD  (mrv tag use header)
*			MRVTAG_TRAIL  (mrv tag use trailer)
* OUTPUTS:
* 		none
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL  - on error
*		GT_NOT_SUPPORTED - device not support this func
* 
*	COMMENTS:
* 		data sheet port register 4.14 - Trailer
* 		data sheet port register 4.8 -9 - Ingress mode
*
*******************************************************************************/
GT_STATUS gtSetCpuFrameMode(GT_LPORT unit, int MrvTagMode)
{
	GT_STATUS retVal = GT_OK;
	if (IS_IN_DEV_GROUP(QD_DEV_PTR, DEV_TRAILER|DEV_TRAILER_P5|DEV_TRAILER_P4P5))
	{
	    if(MrvTagMode == MRVTAG_TRAIL)
	    {
	        retVal = gprtSetTrailerMode(QD_DEV_PTR, QD_DEV_PTR->cpuPortNum, GT_TRUE);
	        retVal = gprtSetIngressMode(QD_DEV_PTR, QD_DEV_PTR->cpuPortNum, GT_TRAILER_INGRESS);
	        ulMrvTagtype = MRVTAG_TRAIL;
	    }
		else if(MrvTagMode == MRVTAG_HEAD)
		{
			/*modify this if necessary in the future*/
	        ulMrvTagtype = MRVTAG_HEAD;
		}
	    else if(MrvTagMode==MRVTAG_NONE)
	    {
	        retVal = gprtSetTrailerMode(QD_DEV_PTR, QD_DEV_PTR->cpuPortNum, GT_TRUE);
	        retVal = gprtSetIngressMode(QD_DEV_PTR, QD_DEV_PTR->cpuPortNum, GT_TRAILER_INGRESS);
	        ulMrvTagtype = MRVTAG_NONE;
	    }
		else
        	retVal = GT_NOT_SUPPORTED;
			
	}
	else if (IS_IN_DEV_GROUP(QD_DEV_PTR, DEV_FRAME_MODE))
	{
	    if(MrvTagMode == MRVTAG_DSA)
	    {
			if((retVal = gsysSetCPUDest(QD_DEV_PTR, QD_DEV_PTR->cpuPortNum)) != GT_OK)
			{
				MSG_OUT(("gsysSetCPUDest returned %d (port %lu)\r\n", retVal, QD_DEV_PTR->cpuPortNum));
			}

			if((retVal = gprtSetFrameMode(QD_DEV_PTR, QD_DEV_PTR->cpuPortNum, GT_FRAME_MODE_DSA)) != GT_OK)
			{
				MSG_OUT(("gprtSetFrameMode returned %d (port %lu, DSA_MODE)\r\n", retVal, QD_DEV_PTR->cpuPortNum));
			}
	        ulMrvTagtype = MRVTAG_DSA;
	    }
		else if(MrvTagMode == MRVTAG_NONE)
		{
			if((retVal = gprtSetFrameMode(QD_DEV_PTR, QD_DEV_PTR->cpuPortNum, GT_FRAME_MODE_NORMAL)) != GT_OK)
			{
				MSG_OUT(("gprtSetFrameMode returned %d (port %lu, NORMAL_MODE)\r\n", retVal, QD_DEV_PTR->cpuPortNum));
			}
	        ulMrvTagtype = MRVTAG_NONE;		
		}		
		else
			retVal = GT_NOT_SUPPORTED;
	}
	else
	{
		retVal = GT_NOT_SUPPORTED;		
	}
    return retVal;
}

/*******************************************************************************
* eventGetActive
*
* DESCRIPTION:
*       This routine get the receive of an hardware driven event.
*
* INPUTS:
*      NONE.
*
* OUTPUTS:
*       eventType - the event type. any combination of the folowing: 
*       	GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL(or GT_ATU_PROB),
*       	GT_ATU_DONE, GT_PHY_INTERRUPT, GT_EE_INTERRUPT, GT_DEVICE_INT,
*			and GT_AVB_INTERRUPT
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Each switch device has its own set of event Types. Please refer to the
*		device datasheet for the list of event types that the device supports.
*
*******************************************************************************/
GT_STATUS eventGetActive
(
    IN GT_QD_DEV *dev,
    IN GT_U16 	 *eventType
)
{
    GT_STATUS   retVal; 
    GT_U8 	len;

    DBG_INFO(("eventGetActive Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_AVB_INTERRUPT))
		len = 9;
	else if (IS_IN_DEV_GROUP(dev,DEV_DEVICE_INTERRUPT))
		len = 8;
	else
		len = 7;

    /* Set the IntEn bit.               */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,0,len,eventType);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gprtGetPortAutoNegEnable
*
* DESCRIPTION:
* 		Get port Auto-Negotiation Enable/Disable mode.
*		This routine simply gets Auto Negotiation bit (bit 12) of Control 
*		Register.
*
* INPUTS:
* 		port    - logical port number
*
* OUTPUTS:
* 		state   - GT_TRUE for enable Auto-Negotiation,
*				- GT_FALSE for disable Auto-Negotiation,
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		data sheet register 0.12 - Auto-Negotiation Enable
* 		data sheet register 4.8, 4.7, 4.6, 4.5 - Auto-Negotiation Advertisement
*
*******************************************************************************/
GT_STATUS gprtGetPortAutoNegEnable
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	OUT GT_BOOL   *state
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;

    DBG_INFO(("gprtGetPortAutoNegEnable Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	if(u16Data & QD_PHY_AUTONEGO)
	{
		*state = GT_TRUE;
	}
	else
	{
		*state = GT_FALSE;
	}


	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}

/*******************************************************************************
* gprtGetPortLoopback
*
* DESCRIPTION:
* Get Internal Port Loopback status. 
*
* INPUTS:
*   port        - logical port number
*
* OUTPUTS:
*   pbResult    - GT_TRUE for enable loopback.
*                 GT_FALSE for disable loopback.
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/

GT_STATUS gprtGetPortLoopback
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	OUT GT_BOOL   *pbResult
)
{
    GT_STATUS       retVal = GT_OK;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_PHY_INFO		phyInfo;
	GT_BOOL 		ppuEn;

    DBG_INFO(("gprtGetPortLoopback Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* Need to disable PPUEn for safe. */
	if(gsysGetPPUEn(dev,&ppuEn) != GT_OK)
	{
		ppuEn = GT_FALSE;
	}

	if(ppuEn != GT_FALSE)
	{
		if((retVal = gsysSetPPUEn(dev,GT_FALSE)) != GT_OK)
		{
	    	DBG_INFO(("Not able to disable PPUEn.\n"));
			return retVal;
		}
	}

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

    /* Read Phy Control Register.  */
   	if(hwGetPhyRegField(dev,hwPort,QD_PHY_CONTROL_REG,14,1,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

    BIT_2_BOOL(u16Data, *pbResult);
    
	/* Restore ppuEn */
	if(ppuEn != GT_FALSE)
	{
		if((retVal = gsysSetPPUEn(dev,ppuEn)) != GT_OK)
		{
	    	DBG_INFO(("Not able to enable PPUEn.\n"));
			return retVal;
		}
	}

	gtSemGive(dev,dev->phyRegsSem);
    return retVal;
}

/*******************************************************************************
* gprtGetAutoNegAdvertise
*
* DESCRIPTION:
*       This routine gets port's Auto Negotiation Advertise Ability.
*
* INPUTS:
* 		port - The logical port number
*
* OUTPUTS:
*       pbResult - GT_TRUE, if Auto Negotiation Advertise Ability is not zero.
*				   GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtGetAutoNegAdvertise
(
	IN  GT_QD_DEV *dev,
	IN  GT_LPORT  port,
	OUT GT_BOOL   *pbResult
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_PHY_INFO	    phyInfo;
	GT_U16			u16Data;

	DBG_INFO(("gprtGetAutoNegAdvertise Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

    /* Fetch the responsible register: */
    if(hwGetPhyRegField(dev, hwPort, QD_PHY_AUTONEGO_AD_REG, 0, 16, &u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

    /* set the ablities of Marvell switch port */
    *pbResult = GT_FALSE;
    if( u16Data & (1 << MDIO_ADV_REG_100BASE_BIT) )
        *pbResult = GT_TRUE;
    if( u16Data & (1 << MDIO_ADV_REG_100BASE_FD_BIT) )
        *pbResult = GT_TRUE;
    if( u16Data & (1 << MDIO_ADV_REG_10BASE_BIT) )
        *pbResult = GT_TRUE;
    if( u16Data & (1 << MDIO_ADV_REG_10BASE_FD_BIT) )
        *pbResult = GT_TRUE;
    if( u16Data & (1 << MDIO_ADV_REG_PAUSE_FD_BIT) )
        *pbResult = GT_TRUE;
    if( u16Data & (1 << MDIO_ADV_REG_ASYM_PAUSE_FD_BIT) )
        *pbResult = GT_TRUE;

	gtSemGive(dev,dev->phyRegsSem);
	return GT_OK;
}

/*******************************************************************************
* gprtGetAutoNegCapability
*
* DESCRIPTION:
*       This routine gets port's Auto Negotiation Capability.
*
* INPUTS:
* 		port - The logical port number
*
* OUTPUTS:
*       pbResult - GT_TRUE, if port has Auto Negotiation capability.
*				   GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtGetAutoNegCapability
(
	IN  GT_QD_DEV *dev,
	IN  GT_LPORT  port,
	OUT GT_BOOL   *pbResult
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_PHY_INFO	    phyInfo;
	GT_U16			u16Data;

	DBG_INFO(("gprtGetAutoNegAdvertise Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

    /* Fetch the responsible register: */
    if(hwGetPhyRegField(dev, hwPort, QD_PHY_STATUS_REG, 3, 1, &u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_STATUS_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

    /* set the ablities of Marvell switch port */
    *pbResult = GT_FALSE;
    if( u16Data )
        *pbResult = GT_TRUE;

	gtSemGive(dev,dev->phyRegsSem);
	return GT_OK;
}

/*******************************************************************************
* gvlnSetPortIsolate
*
* DESCRIPTION:
*       This routine sets port's isolating between each other except uplink port.
*
* INPUTS:
* 		mode    - GT_TRUE, Isolate enable.
*				  GT_FALSE, Isolate disable..
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gvlnSetPortIsolate
(
	IN  GT_QD_DEV *dev,
	IN  GT_BOOL   mode
)
{
    GT_STATUS       retVal = GT_OK; /* Functions return value.      */
    GT_U8           i,j;
    GT_LPORT        memPorts[TOTAL_NUM_OF_PORTS];
    GT_LPORT		wanPort;

    DBG_INFO(("gvlnSetPortIsolate Called.\n"));
/*
    for (i=0; i<dev->numOfPorts; i++)
    {
        retVal = gprtSetProtectedMode(dev, i, mode);
        if (GT_OK != retVal) break;
    }
*/

	if(QD_MASTER_DEV_PTR == dev)
	{
		get_switch_wan_port_info(MASTER_UNIT, &wanPort);
	}
	else
	{
		wanPort = dev->cpuPortNum;
	}
	
    if(GT_OK == retVal)
    {
        switch (mode)
        {
            case GT_TRUE:
                for (i=0; i<dev->numOfPorts; i++)
                {
                    if (i == wanPort)
                    {
                        for (j=0; j<dev->numOfPorts; j++)
                        {
                            if ( j < i) 
                                memPorts[j] = j;
                            else 
                                memPorts[j] = j + 1;
                        }
                        retVal = gvlnSetPortVlanPorts(dev, i, memPorts, dev->numOfPorts - 1);
                        if (GT_OK != retVal) break;
                    }
                    else
                    {
                        memPorts[0] = wanPort;
                        retVal = gvlnSetPortVlanPorts(dev, i, memPorts, 1);
                        if (GT_OK != retVal) break;
                    }
                }
                gtVlnPortIsolate = GT_TRUE;
                break;
            case GT_FALSE:
                for (i=0; i<dev->numOfPorts; i++)
                {
                    for (j=0; j<dev->numOfPorts; j++)
                    {
                        if ( j < i) 
                            memPorts[j] = j;
                        else 
                            memPorts[j] = j + 1;
                    }
                    retVal = gvlnSetPortVlanPorts(dev, i, memPorts, dev->numOfPorts - 1);
                    if (GT_OK != retVal) break;
                }
                gtVlnPortIsolate = GT_FALSE;
                break;
            default:
                break;
        }
    }

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    return retVal;
}

/*******************************************************************************
* gvlnGetPortIsolate
*
* DESCRIPTION:
*       This routine gets port's isolating mode.
*
* INPUTS:
* 		mode    - GT_TRUE, Isolate enable.
*				  GT_FALSE, Isolate disable..
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gvlnGetPortIsolate
(
	IN  GT_QD_DEV *dev,
	IN  GT_BOOL   *mode
)
{
    *mode = gtVlnPortIsolate;
    return GT_OK;
}

/*******************************************************************************
* gvlnSetIsolatePortlist
*
* DESCRIPTION:
*       This routine sets the specified port's isolate with each other 
*
* INPUTS:
*       port         - logical port number to set.
*       memPorts     - array of logical ports in the same vlan.
*       memPortsLen  - number of members in memPorts array
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gvlnSetIsolatePortlist
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_LPORT  memPorts[],
    IN GT_U8     memPortsLen,
    IN GT_BOOL	 mode
)
{
	GT_U16 gtRet = GT_OK;    
	DBG_INFO(("gvlnSetIsolatePortlist Called.\n"));
	
	/* Check if this feature is supported */
	if(IS_VALID_API_CALL(dev,port, DEV_PORT_SECURITY) != GT_OK)
	{
        DBG_INFO(("Not support protected port mode.\n"));
        goto  NOT_SUPPORT_PROTECT_MODE;
	}
	if (IS_IN_DEV_GROUP(dev,DEV_CROSS_CHIP_VLAN))
	{
		DBG_INFO(("Not support protected port mode.\n"));
        goto  NOT_SUPPORT_PROTECT_MODE;
	}

	if(mode == GT_TRUE)
	{	
		gtRet = gprtSetProtectedMode(dev, port, GT_TRUE);
	}
	else
	{
		gtRet = gprtSetProtectedMode(dev, port, GT_FALSE);		
	}
	
NOT_SUPPORT_PROTECT_MODE:

	gtRet = gvlnSetPortVlanPorts(dev, port, memPorts, memPortsLen);
	
    if(gtRet != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return GT_ERROR;
	}
    else
	{
        DBG_INFO(("OK.\n"));
		return GT_OK;
	}
}



/*******************************************************************************
* cPri0RateEnum2Number
*
* DESCRIPTION:
*       This routine convert Pri0 rate enum to number.
*
* INPUTS:
* 		rate    - enum rate.
*
* OUTPUTS:
*       rLimit  - number rate.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS cPri0RateEnum2Number
(
    IN GT_QD_DEV *dev, 
    IN GT_PRI0_RATE rate, 
    OUT GT_U32 *rLimit
)
{
	GT_U32 rateLimit;

	switch(rate)
	{
		case GT_NO_LIMIT :
				rateLimit = 0; /* MAX_RATE_LIMIT; */
				break;
		case GT_64K :
				rateLimit = 64;
				break;
		case GT_128K :
				rateLimit = 128;
				break;
		case GT_256K :
				rateLimit = 256;
				break;
		/*case GT_384K :
				rateLimit = 384;
				break;*/
		case GT_512K :
				rateLimit = 512;
				break;
		/*case GT_640K :
				rateLimit = 640;
				break;
		case GT_768K :
				rateLimit = 768;
				break;
		case GT_896K :
				rateLimit = 896;
				break;*/
		case GT_1M :
				rateLimit = 1000;
				break;
		/*case GT_1500K :
				rateLimit = 1500;
				break;*/
		case GT_2M :
				rateLimit = 2000;
				break;
		case GT_4M :
				rateLimit = 4000;
				break;
		case GT_8M :
				rateLimit = 8000;
				break;
		case GT_16M :
				rateLimit = 16000;
				break;
		case GT_32M :
				rateLimit = 32000;
				break;
		case GT_64M :
				rateLimit = 64000;
				break;
		case GT_128M :
				rateLimit = 128000;
				break;
		case GT_256M :
				rateLimit = 256000;
				break;
		default :
				rateLimit = rate;
				break;
				/*return GT_BAD_PARAM;*/
	}

	*rLimit = rateLimit;
	return GT_OK;
}


/*******************************************************************************************
 Description   : Convert given number Rate Limit to defined Pri0 Rate Limit enum. 
                 ( eg. 64kbps to GT_128K )
 In params     : ulRate - number rate
                 
 Out params    : gtRate - enum rate
 Return value  : GT_OK   - on success
                 GT_FAIL - on error       
********************************************************************************************/
GT_STATUS cNumber2Pri0RateEnum
(
    IN GT_QD_DEV *dev, 
    IN GT_U32 ulRate, 
    OUT GT_PRI0_RATE* gtRate
)
{
	GT_PRI0_RATE    sLimit,  startLimit, endLimit;
	GT_U32          rLimit;
    GT_STATUS       retVal;         /* Functions return value.      */

	if((ulRate == 0) || (ulRate == 100000))
	{
		*gtRate = GT_NO_LIMIT;
		return GT_OK;
	}
		
	startLimit = GT_256M;
	endLimit = GT_64K;
	
	for(sLimit=startLimit;sLimit>=endLimit;sLimit--)
	{
		if((retVal = cPri0RateEnum2Number(dev, sLimit, &rLimit)) != GT_OK)
		{
        	DBG_INFO(("Failed.\n"));
	   	    return retVal;
		}
		if((ulRate / rLimit) == 1)
		{
			*gtRate = sLimit;
			return GT_OK;
		}
	}

	*gtRate = GT_NO_LIMIT;

	return GT_FAIL;
}


/*******************************************************************************************
 Description   : Convert given number Rate Limit to defined Burst Rate Limit enum. 
                 ( eg. 128kbps to GT_BURST_128K )
 In params     : ulRate - number rate
                 
 Out params    : gtRate - enum rate
 Return value  : GT_OK   - on success
                 GT_FAIL - on error       
********************************************************************************************/
GT_STATUS cNumber2BurstRateEnum
(
    IN GT_QD_DEV *dev, 
    IN GT_U32 ulRate, 
    OUT GT_BURST_RATE* gtRate
)
{
	GT_BURST_RATE   sLimit,  startLimit, endLimit;
	GT_U32          rLimit;
    GT_STATUS       retVal;         /* Functions return value.      */

	if((ulRate == 0) || (ulRate == 100000))
	{
		*gtRate = GT_BURST_NO_LIMIT;
		return GT_OK;
	}
		
	startLimit = GT_BURST_256M;
	endLimit = GT_BURST_64K;
	
	for(sLimit=startLimit;sLimit>=endLimit;sLimit--)
	{
		if((retVal = cBurstEnum2Number(dev, sLimit, &rLimit)) != GT_OK)
		{
        	DBG_INFO(("Failed.\n"));
	   	    return retVal;
		}
		if((ulRate / rLimit) == 1)
		{
			*gtRate = sLimit;
			return GT_OK;
		}
	}

	*gtRate = GT_BURST_NO_LIMIT;

	return GT_FAIL;
}

/*******************************************************************************
* cEgressRateEnum2Number
*
* DESCRIPTION:
*       This routine convert Egress rate enum to number.
*
* INPUTS:
* 		rate    - enum rate.
*
* OUTPUTS:
*       rLimit  - number rate.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS cEgressRateEnum2Number
(
    IN GT_QD_DEV *dev, 
    IN GT_ERATE_TYPE rate, 
    OUT GT_U32 *rLimit
)
{
	GT_U32 rateLimit;

	switch(rate.definedRate)
	{
		case GT_NO_LIMIT :
				rateLimit = 0; /* MAX_RATE_LIMIT; */
				break;
		case GT_64K :
				rateLimit = 64;
				break;
		case GT_128K :
				rateLimit = 128;
				break;
		case GT_256K :
				rateLimit = 256;
				break;
		/*case GT_384K :
				rateLimit = 384;
				break;*/
		case GT_512K :
				rateLimit = 512;
				break;
		/*case GT_640K :
				rateLimit = 640;
				break;
		case GT_768K :
				rateLimit = 768;
				break;
		case GT_896K :
				rateLimit = 896;
				break;*/
		case GT_1M :
				rateLimit = 1000;
				break;
		/*case GT_1500K :
				rateLimit = 1500;
				break;*/
		case GT_2M :
				rateLimit = 2000;
				break;
		case GT_4M :
				rateLimit = 4000;
				break;
		case GT_8M :
				rateLimit = 8000;
				break;
		case GT_16M :
				rateLimit = 16000;
				break;
		case GT_32M :
				rateLimit = 32000;
				break;
		case GT_64M :
				rateLimit = 64000;
				break;
		case GT_128M :
				rateLimit = 128000;
				break;
		case GT_256M :
				rateLimit = 256000;
				break;
		default :
				return GT_BAD_PARAM;
	}

	*rLimit = rateLimit;
	return GT_OK;
}

/*******************************************************************************************
 Description   : Convert given number Rate Limit to defined Egress Rate Limit enum. 
                 ( eg. 128kbps to GT_BURST_128K )
 In params     : ulRate - number rate
                 
 Out params    : gtRate - enum rate
 Return value  : GT_OK   - on success
                 GT_FAIL - on error       
********************************************************************************************/
GT_STATUS cNumber2EgressRateEnum
(
    IN GT_QD_DEV *dev, 
    IN GT_U32 ulRate, 
    OUT GT_ERATE_TYPE* gtRate
)
{
	GT_EGRESS_RATE  sLimit,  startLimit, endLimit;
	GT_U32          rLimit;
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_ERATE_TYPE	gtERateType;

	if((ulRate == 0) || (ulRate == 100000))
	{
		gtRate->definedRate = GT_NO_LIMIT;
		return GT_OK;
	}
		
	startLimit = GT_256M;
	endLimit = GT_64K;
	
	for(sLimit=startLimit;sLimit>=endLimit;sLimit--)
	{
		gtERateType.definedRate = sLimit;
		if((retVal = cEgressRateEnum2Number(dev, gtERateType, &rLimit)) != GT_OK)
		{
        	DBG_INFO(("Failed.\n"));
	   	    return retVal;
		}
		if((ulRate / rLimit) == 1)
		{
			gtRate->definedRate = sLimit;
			return GT_OK;
		}
	}

	gtRate->definedRate = GT_NO_LIMIT;

	return GT_FAIL;
}
#ifdef _USE_DSDT_26A_
/*******************************************************************************
* gprtSetEnergyDetect
*
* DESCRIPTION:
*       This routine enables/disables EnergyDetect.
*
* INPUTS:
* 		port - The logical port number
* 		mode - either GT_TRUE(for enable) or GT_FALSE(for disable)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtSetEnergyDetect
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_BOOL   mode
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_STATUS		retVal = GT_OK;
	GT_PHY_INFO		phyInfo;
	GT_BOOL 		ppuEn;
	GT_U16 			u16Data;

	DBG_INFO(("gprtSetEnergyDetect Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* Need to disable PPUEn for safe. */
	if(gsysGetPPUEn(dev,&ppuEn) != GT_OK)
	{
		ppuEn = GT_FALSE;
	}

	if(ppuEn != GT_FALSE)
	{
		if((retVal = gsysSetPPUEn(dev,GT_FALSE)) != GT_OK)
		{
	    	DBG_INFO(("Not able to disable PPUEn.\n"));
	    	gtSemGive(dev,dev->phyRegsSem);
			return retVal;
		}
	}

	if((retVal = hwReadPhyReg(dev,hwPort,0x10,&u16Data)) != GT_OK)
	{
		goto SkipOut1;
	}

	u16Data = mode?(u16Data|0x4000):(u16Data&(~0x4000));

	if((retVal = hwWritePhyReg(dev,hwPort,0x10,u16Data)) != GT_OK)
	{
		goto SkipOut1;
	}

	/* soft reset */
	/*if((retVal = hwPhyReset(dev,hwPort,0xFF)) != GT_OK)
	{
		goto SkipOut1;
	}*/
SkipOut1:
	gtSemGive(dev,dev->phyRegsSem);
	/* Restore ppuEn */
	if(ppuEn != GT_FALSE)
	{
		if((retVal = gsysSetPPUEn(dev,ppuEn)) != GT_OK)
		{
	    	DBG_INFO(("Not able to enable PPUEn.\n"));
			return retVal;
		}
	}
	return retVal;
}
#endif
/*******************************************************************************
* gqosGetQoSWeight
*
* DESCRIPTION:
*       Programmable Round Robin Weights.
*		Each port has 4 output Queues. Queue 3 has the highest priority and 
*		Queue 0 has the lowest priority. When a scheduling mode of port is 
*		configured as Weighted Round Robin queuing mode, the access sequece of the 
*		Queue is 3,2,3,1,3,2,3,0,3,2,3,1,3,2,3 by default.
*		This routine retrieves the access sequence of the Queue.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       weight - access sequence of the queue
*		mode - GT_TRUE for WRR, GT_FALSE for SP scheduling
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gqosGetQoSWeight
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8  *weight,
    OUT GT_BOOL   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          i;
	GT_QoS_WEIGHT	weight_seq;

	if (IS_IN_DEV_GROUP(dev,DEV_PORT_MIXED_SCHEDULE))
	{
		GT_PORT_SCHED_MODE portMode;
		portMode = GT_PORT_SCHED_WEIGHTED_RRB;
		retVal = gprtGetPortSched(dev, 0, &portMode);
		if(GT_PORT_SCHED_WEIGHTED_RRB == portMode)
			*mode = GT_TRUE;
		else
			*mode = GT_FALSE;
	}
	else
		retVal = gsysGetSchedulingMode(dev,mode);

	if(!weight)
	{
		DBG_INFO(("Null pointer in gqosGetQoSWeight \n"));
		return GT_BAD_VALUE;
	}
	else
	{
		memset(weight, 0, 4);
		memset(&weight_seq, 0, sizeof(GT_QoS_WEIGHT));
	}
	
 	if( !IS_IN_DEV_GROUP(dev,DEV_QoS_WEIGHT))
	{
	    weight[0]=1;
	    weight[1]=2;
	    weight[2]=4;
	    weight[3]=8;
 	}
	else
	{
		if((retVal=gsysGetQoSWeight(dev, &weight_seq)) != GT_OK)
		{
			DBG_INFO(("\r\nGet the gsysGetQoSWeight failed (%d) !", retVal));
			return retVal;
		}
		for(i=0;i<weight_seq.len;i++)
		{
			if(weight_seq.queue[i] == 0)
				weight[0]++;
			else if(weight_seq.queue[i] == 1)
				weight[1]++;
			else if(weight_seq.queue[i] == 2)
				weight[2]++;
			else if(weight_seq.queue[i] == 3)
				weight[3]++;
			else{}
		}
	}

    return retVal;
}

/*******************************************************************************
* gqosSetQoSWeight
*
* DESCRIPTION:
*       Programmable Round Robin Weights.
*		Each port has 4 output Queues. Queue 3 has the highest priority and 
*		Queue 0 has the lowest priority. When a scheduling mode of port is 
*		configured as Weighted Round Robin queuing mode, the access sequece of the 
*		Queue is 3,2,3,1,3,2,3,0,3,2,3,1,3,2,3 by default.
*		This routine retrieves the access sequence of the Queue.
*
* INPUTS:
*       weight - access sequence of the queue
*		mode - GT_TRUE for WRR, GT_FALSE for SP scheduling
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gqosSetQoSWeight
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8  *weight,
    OUT GT_BOOL   mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          i, offset;
	GT_QoS_WEIGHT	weight_seq;
	
	if (IS_IN_DEV_GROUP(dev,DEV_PORT_MIXED_SCHEDULE))
	{
		GT_PORT_SCHED_MODE portMode;
		GT_LPORT phyPort;
		
		if(GT_TRUE == mode)
			portMode = GT_PORT_SCHED_WEIGHTED_RRB; 
		else
			portMode = GT_PORT_SCHED_STRICT_PRI; 
		retVal = GT_OK;
		for(phyPort=0; phyPort<dev->numOfPorts; phyPort++)
		{
			retVal += gprtSetPortSched(dev, phyPort, portMode);
		}
	}
	else
		retVal = gsysSetSchedulingMode(dev,mode);
	if(retVal != GT_OK)
	{
    	DBG_INFO(("set gprtSetPortSched failed (%d)\n", retVal));
		return retVal;
	}
	

	if(IS_IN_DEV_GROUP(dev,DEV_QoS_WEIGHT) && (GT_TRUE == mode))
	{
		if(!weight)
		{
    		DBG_INFO(("Null pointer in gqosSetQoSWeight !\n"));
			return GT_BAD_VALUE;
		}
		memset(&weight_seq, 0, sizeof(GT_QoS_WEIGHT));
		
		weight_seq.len = weight[0]+weight[1]+weight[2]+weight[3];
		if( weight_seq.len > 128)
			return GT_BAD_PARAM;
		
		offset=0;
		for(i=0;i<4;i++)
		{
			memset(weight_seq.queue+offset, i, weight[i]);
			offset += weight[i];
		}		
		if((retVal=gsysSetQoSWeight(dev, &weight_seq)) != GT_OK)
		{
			DBG_INFO(("\r\nSet the gsysSetQoSWeight failed (%d) !", retVal));
			return retVal;
		}			
	}

    return retVal;
}


/*******************************************************************************************
 Description   : Convert given defined Burst Size enum to Burst Size number. 
                 ( eg. 12kbytes to GT_BURST_SIZE_12K )
 In params     : gtBurstSize - enum size
                 
 Out params    : ulBurstSize - number size
 Return value  : GT_OK   - on success
                 GT_FAIL - on error       
********************************************************************************************/
GT_STATUS cBurstSizeEnum2Number
(
    IN GT_QD_DEV *dev, 
    IN GT_BURST_SIZE gtBurstSize, 
    OUT GT_U32 *ulBurstSize
)
{
	GT_U32 Size;

	switch(gtBurstSize)
	{
		case GT_BURST_SIZE_12K :
				Size = 12; 
				break;
		case GT_BURST_SIZE_24K :
				Size = 24; 
				break;
		case GT_BURST_SIZE_48K :
				Size = 48; 
				break;
		case GT_BURST_SIZE_96K :
				Size = 96; 
				break;
		default :
				return GT_BAD_PARAM;
	}

	*ulBurstSize = Size;
	return GT_OK;
}

/*******************************************************************************************
 Description   : Convert given number Burst Size to defined Burst Size enum. 
                 ( eg. 12kbytes to GT_BURST_SIZE_12K )
 In params     : ulBurstSize - number size
                 
 Out params    : gtBurstSize - enum size
 Return value  : GT_OK   - on success
                 GT_FAIL - on error       
********************************************************************************************/
GT_STATUS cNumber2BurstSizeEnum
(
    IN GT_QD_DEV *dev, 
    IN GT_U32 ulBurstSize, 
    OUT GT_BURST_SIZE* gtBurstSize
)
{
	GT_BURST_SIZE   sLimit,  startLimit, endLimit;
	GT_U32          rLimit;
    GT_STATUS       retVal;         /* Functions return value.      */

	if(ulBurstSize == 0)
	{
		*gtBurstSize = GT_BURST_SIZE_12K;
		return GT_OK;
	}
		
	startLimit = GT_BURST_SIZE_96K;
	endLimit = GT_BURST_SIZE_12K;
	
	for(sLimit=startLimit;sLimit>=endLimit;sLimit--)
	{
		if((retVal = cBurstSizeEnum2Number(dev, sLimit, &rLimit)) != GT_OK)
		{
        	DBG_INFO(("Failed.\n"));
	   	    return retVal;
		}
		if((ulBurstSize / rLimit) == 1)
		{
			*gtBurstSize = sLimit;
			return GT_OK;
		}
	}

	*gtBurstSize = GT_BURST_SIZE_12K;

	return GT_FAIL;
}

/*******************************************************************************
* gprtSetFCThreshold
*
* DESCRIPTION:
*       This routine sets the threshold for asserting and de-asserting flow 
*		Control(i.e. issuing pause on/pause off). 
*
* INPUTS:
*       port - logical port number
*		threshold - threshold value,
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetFCThreshold
(
	IN GT_QD_DEV			*dev,
	IN GT_LPORT				port,
	IN GT_U16				threshold
)
{
    GT_U16          data;           
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetFCThreshold Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6095_FAMILY | DEV_88E6185_FAMILY))
	{
	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	data = threshold;

    retVal = hwWritePortReg(dev,hwPort, 0x1E, data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}

    return GT_OK;
}

/*******************************************************************************
* gprtSetFCThresholdDrop
*
* DESCRIPTION:
*       This routine sets the threshold for drop frames. 
*
* INPUTS:
*       port - logical port number
*		threshold - threshold value,
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetFCThresholdDrop
(
	IN GT_QD_DEV			*dev,
	IN GT_LPORT				port,
	IN GT_U16				threshold
)
{
    GT_U16          data;           
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetFCThreshold Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6095_FAMILY | DEV_88E6185_FAMILY))
	{
	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	data = threshold;

    retVal = hwWritePortReg(dev,hwPort, 0x1F, data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}

    return GT_OK;
}

/*******************************************************************************
* gprtSetClassABDrv
*
* DESCRIPTION:
*       This routine select phy driver mode between CLASS A and B.
*
* INPUTS:
* 		port - The logical port number
* 		mode - either GT_TRUE(for CLASS A) or GT_FALSE(for CLASS B)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtSetClassABDrv
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_BOOL   mode
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_STATUS		retVal = GT_OK;
	GT_PHY_INFO		phyInfo;
	GT_BOOL 		ppuEn;
	GT_U16 			u16Data;

	DBG_INFO(("gprtSetClassABDrv Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* Need to disable PPUEn for safe. */
	if(gsysGetPPUEn(dev,&ppuEn) != GT_OK)
	{
		ppuEn = GT_FALSE;
	}

	if(ppuEn != GT_FALSE)
	{
		if((retVal = gsysSetPPUEn(dev,GT_FALSE)) != GT_OK)
		{
	    	DBG_INFO(("Not able to disable PPUEn.\n"));
	    	gtSemGive(dev,dev->phyRegsSem);
			return retVal;
		}
	}

	if((retVal = hwReadPhyReg(dev,hwPort,0x1C,&u16Data)) != GT_OK)
	{
		goto SkipOut2;
	}

	if ( mode == (u16Data&(0x0001)) )
	{
		goto SkipOut2;
	}
	
	u16Data = mode?(u16Data|0x0001):(u16Data&(~0x0001));

	if((retVal = hwWritePhyReg(dev,hwPort,0x1C,u16Data)) != GT_OK)
	{
		goto SkipOut2;
	}

	/* soft reset */
	if((retVal = hwPhyReset(dev,hwPort,0xFF)) != GT_OK)
	{
		goto SkipOut2;
	}

SkipOut2:
	gtSemGive(dev,dev->phyRegsSem);
	/* Restore ppuEn */
	if(ppuEn != GT_FALSE)
	{
		if((retVal = gsysSetPPUEn(dev,ppuEn)) != GT_OK)
		{
	    	DBG_INFO(("Not able to enable PPUEn.\n"));
			return retVal;
		}
	}
	return retVal;
}

/*******************************************************************************
* gprtGetPortPowerDown
*
* DESCRIPTION:
* 		Get Enable/disable (power down) status on specific logical port.
*		Phy configuration remains unchanged after Power down.
*
* INPUTS:
* 		port	- logical port number
*
* OUTPUTS:
* 		state	- GT_TRUE: power down
* 				  GT_FALSE: normal operation
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		data sheet register 0.11 - Power Down
*
*******************************************************************************/

GT_STATUS gprtGetPortPowerDown
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	OUT GT_BOOL   *state
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtGetPortPowerDown Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if((retVal=hwGetPhyRegField(dev,hwPort,QD_PHY_CONTROL_REG,11,1,&u16Data)) != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return retVal;
	}

	BIT_2_BOOL(u16Data,*state);

	gtSemGive(dev,dev->phyRegsSem);
	return GT_OK;
}

/*******************************************************************************
* gprtGetPhyPause
*
* DESCRIPTION:
*       This routine will get the pause bit in Autonegotiation Advertisement
*		Register. 
*
* INPUTS:
* 		port  - The logical port number
*
* OUTPUTS:
* 		state - GT_PHY_PAUSE_MODE enum value.
*			   	GT_PHY_NO_PAUSE		- disable pause
* 				GT_PHY_PAUSE		- support pause
*				GT_PHY_ASYMMETRIC_PAUSE	- support asymmetric pause
*				GT_PHY_BOTH_PAUSE	- support both pause and asymmetric pause
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

GT_STATUS gprtGetPhyPause
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	OUT GT_PHY_PAUSE_MODE *state
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;
	GT_PHY_INFO		phyInfo;

	DBG_INFO(("phyGetPause Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}


	/* Read from Phy AutoNegotiation Advertisement Register.  */
	if((retVal=hwGetPhyRegField(dev,hwPort,QD_PHY_AUTONEGO_AD_REG,10,2,&u16Data)) != GT_OK)
	{
		DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	*state = (GT_PHY_PAUSE_MODE)u16Data;

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}

/*******************************************************************************
* gprtGetPortDuplexMode
*
* DESCRIPTION:
* 		Gets duplex mode for a specific logical port.  
*
* INPUTS:
* 		port 	- logical port number
*
* OUTPUTS:
* 		dMode	- dulpex mode GT_TRUE for duplex, GT_FALSE for half.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
GT_STATUS gprtGetPortDuplexMode
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_BOOL   *dMode
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;

    DBG_INFO(("gprtGetPortDuplexMode Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

    if(hwGetPhyRegField(dev,hwPort,QD_PHY_CONTROL_REG,8,1,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	BIT_2_BOOL(u16Data,*dMode);

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}

/*******************************************************************************
* gprtGetPhyDuplexStatus
*
* DESCRIPTION:
* 		Gets duplex mode for a specific logical port.  
*
* INPUTS:
* 		port 	- logical port number
*
* OUTPUTS:
* 		status	- dulpex mode GT_TRUE for duplex, GT_FALSE for half.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
GT_STATUS gprtGetPhyDuplexStatus
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_BOOL   *status
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;

    DBG_INFO(("gprtGetPhyDuplexStatus Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* QD_PHY_STATUS_REG */
    if(hwReadPhyReg(dev,hwPort,17,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,1));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	*status = GT_TRUE;
	if((0 == (u16Data & 0x2000)) && (u16Data & 0x800))
		*status = GT_FALSE;

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}

/*
 * Start Packet Generator.
 * Input:
 *      pktload - enum GT_PG_PAYLOAD (GT_PG_PAYLOAD_RANDOM or GT_PG_PAYLOAD_5AA5)
 *      length  - enum GT_PG_LENGTH  (GT_PG_LENGTH_64 or GT_PG_LENGTH_1514)
 *      tx      - enum GT_PG_TX      (GT_PG_TX_NORMAL or GT_PG_TX_ERROR)
*/
GT_STATUS startPktGenerator
(
    GT_QD_DEV      *dev,
    GT_LPORT       port,
    GT_PG_PAYLOAD  payload,
    GT_PG_LENGTH   length,
    GT_PG_TX       tx
)
{
    GT_STATUS status;
    GT_PG     pktInfo;

    if (dev == 0)
    {
        DBG_INFO(("GT driver is not initialized\n"));
        return GT_FAIL;
    }

    DBG_INFO(("Start Packet Generator for port %i\n",(int)port));

    pktInfo.payload = payload; /* Pseudo-random, 5AA55AA5... */
    pktInfo.length = length;   /* 64 bytes, 1514 bytes */
    pktInfo.tx = tx;           /* normal packet, error packet */

    /*
     *	Start Packet Generator
    */
    if((status = gprtSetPktGenEnable(dev,port,GT_TRUE,&pktInfo)) != GT_OK)
    {
        DBG_INFO(("mdDiagSetPktGenEnable return Failed\n"));
        return status;
    }

    return GT_OK;
}


/*
 * Stop Packet Generator.
 */
GT_STATUS stopPktGenerator(GT_QD_DEV *dev,GT_LPORT port)
{
    GT_STATUS status;

    if (dev == 0)
    {
        DBG_INFO(("GT driver is not initialized\n"));
        return GT_FAIL;
    }

    DBG_INFO(("Stopping Packet Generator for port %i\n",(int)port));

    /*
     *	Start Packet Generator
    */
    if((status = gprtSetPktGenEnable(dev,port,GT_FALSE,NULL)) != GT_OK)
    {
        DBG_INFO(("mdDiagSetPktGenEnable return Failed\n"));
        return status;
    }

    return GT_OK;
}

#if (FOR_MRV_ONU_PON)
void Debug_print_marvell_error_string(GT_STATUS status)
{
    switch (status)
    {
    case GT_ERROR: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_ERROR"); break; /* -1 */
    case GT_OK: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_OK"); break;        /* 00 */
    case GT_FAIL: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_FAIL"); break;
    case GT_BAD_VALUE: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_BAD_VALUE"); break;
    case GT_BAD_PARAM: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_BAD_PARAM"); break;
    case GT_NOT_FOUND: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_NOT_FOUND"); break;
    case GT_NO_MORE: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_NO_MORE"); break;
    case GT_NO_SUCH: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_NO_SUCH"); break;
    case GT_TIMEOUT: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_TIMEOUT"); break;
    case GT_NOT_SUPPORTED: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_NOT_SUPPORTED"); break;
    case GT_ALREADY_EXIST: PASONU_DBG_printf(PASONU_DBG_SHELL, "GT_ALREADY_EXIST"); break;
    default: PASONU_DBG_printf(PASONU_DBG_SHELL, "unknown (%d)",status);
    }
}
#elif (FOR_MRV_INDUSTRY_SW)
void Debug_print_marvell_error_string(GT_STATUS status)
{
    switch (status)
    {
    case GT_ERROR: printf("GT_ERROR"); break; /* -1 */
    case GT_OK: printf("GT_OK"); break;        /* 00 */
    case GT_FAIL: printf("GT_FAIL"); break;
    case GT_BAD_VALUE: printf("GT_BAD_VALUE"); break;
    case GT_BAD_PARAM: printf("GT_BAD_PARAM"); break;
    case GT_NOT_FOUND: printf("GT_NOT_FOUND"); break;
    case GT_NO_MORE: printf("GT_NO_MORE"); break;
    case GT_NO_SUCH: printf("GT_NO_SUCH"); break;
    case GT_TIMEOUT: printf("GT_TIMEOUT"); break;
    case GT_NOT_SUPPORTED: printf("GT_NOT_SUPPORTED"); break;
    case GT_ALREADY_EXIST: printf("GT_ALREADY_EXIST"); break;
    default: printf("unknown (%d)",status);
    }
}
#endif

#ifndef _FOR_TEMP_USE__

/*******************************************************************************
* Marvell_switch_reset
*
* DESCRIPTION:
* 		Soft reset switch.  
*
* INPUTS:
* 		None.
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		VOS_OK 	- on success
* 		VOS_ERROR - on error
*
*******************************************************************************/
int Marvell_switch_reset
(
    IN GT_QD_DEV    *dev
)
{
    int result;
    GT_STATUS status;

    /* flush all ATU entries */
    status = gfdbFlush(dev, GT_FLUSH_ALL);
    CHECK_MARVELL_RESULT_NO_RETURN(gfdbFlush)

    /* flush VTU */
    status = gvtuFlush(dev);
    CHECK_MARVELL_RESULT_NO_RETURN(gvtuFlush)

    /* flush all ports stats */
    status = gstatsFlushAll(dev);
    CHECK_MARVELL_RESULT_NO_RETURN(gstatsFlushAll)

    /* soft reset of the switch  - affects only PPU & PHY */
    status = gsysSwReset(dev);
    CHECK_MARVELL_RESULT_NO_RETURN(gsysSwReset)

    return result;
}

#endif

#ifdef _RELEASE_VERSION_
/*******************************************************************************
* gprtSetPortInSeries
*
* DESCRIPTION:
*       This routine sets port's vlan (1,2)-(3,4)-...
*
* INPUTS:
* 		mode    - GT_TRUE, Port in series enable.
*				  GT_FALSE, Port in series disable..
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtSetPortInSeries
(
	IN  GT_QD_DEV *dev,
	IN  GT_BOOL   mode
)
{
    GT_STATUS       retVal = GT_OK; 	/* Functions return value.      */
    GT_LPORT        unit,hwPort1,hwPort2; 	/* the physical port number     */
    GT_U8           i,j;
    GT_LPORT        memPorts[TOTAL_NUM_OF_PORTS];

    DBG_INFO(("gprtSetPortInSeries Called.\n"));
/*
    for (i=0; i<dev->numOfPorts; i++)
    {
        retVal = gprtSetProtectedMode(dev, i, mode);
        if (GT_OK != retVal) break;
    }
*/
    if(GT_OK == retVal)
    {
        switch (mode)
        {
            case GT_TRUE:
                for (i=0; i<dev->numOfPorts-1; i++)
                {
					boards_logical_to_physical(i, &unit, &hwPort1);
					i++;
					boards_logical_to_physical(i, &unit, &hwPort2);
                    memPorts[0] = hwPort2;
                    retVal = gvlnSetPortVlanPorts(dev, hwPort1, memPorts, 1);
                    if (GT_OK != retVal) break;
                    memPorts[0] = hwPort1;
                   	retVal = gvlnSetPortVlanPorts(dev, hwPort2, memPorts, 1);
                    if (GT_OK != retVal) break;
                }
                gtPortInSeries = GT_TRUE;
                break;
            case GT_FALSE:
                for (i=0; i<dev->numOfPorts; i++)
                {
                    for (j=0; j<dev->numOfPorts; j++)
                    {
                        if ( j < i) 
                            memPorts[j] = j;
                        else 
                            memPorts[j] = j + 1;
                    }
                    retVal = gvlnSetPortVlanPorts(dev, i, memPorts, dev->numOfPorts - 1);
                    if (GT_OK != retVal) break;
                }
                gtPortInSeries = GT_FALSE;
                break;
            default:
            	break;
        }
    }

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    return retVal;
}

/*******************************************************************************
* gprtGetPortInSeries
*
* DESCRIPTION:
*       This routine gets port's in series mode.
*
* INPUTS:
* 		mode    - GT_TRUE, port in series enable.
*				  GT_FALSE, port in series  disable..
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtGetPortInSeries
(
	IN  GT_QD_DEV *dev,
	IN  GT_BOOL   *mode
)
{
    *mode = gtPortInSeries;
    return GT_OK;
}
#endif /* _RELEASE_VERSION_ */
/*******************************************************************************
* gprtGetPhySpeedStatus
*
* DESCRIPTION:
* 		Gets speed mode for a specific logical port.  
*
* INPUTS:
* 		port 	- logical port number
*
* OUTPUTS:
* 		status	- speed mode GT_TRUE for 100M, GT_FALSE for 10M.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
GT_STATUS gprtGetPhySpeedStatus
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_BOOL   *status
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;

    DBG_INFO(("gprtGetPhySpeedStatus Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* QD_PHY_STATUS_REG */
    if(hwReadPhyReg(dev,hwPort,17,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,1));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	*status = GT_TRUE;
	if(0 == (u16Data & 0x4000))
		*status = GT_FALSE;

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}

/*******************************************************************************
* gsysSetFloodBC
*
* DESCRIPTION:
*		Flood Broadcast.
*		When Flood Broadcast is enabled, frames with the Broadcast destination 
*		address will flood out all the ports regardless of the setting of the
*		port's Egress Floods mode (see gprtSetEgressFlood API). VLAN rules and 
*		other switch policy still applies to these Broadcast frames. 
*		When this feature is disabled, frames with the Broadcast destination
*		address are considered Multicast frames and will be affected by port's 
*		Egress Floods mode.
*
* INPUTS:
*		en - GT_TRUE to enable Flood Broadcast, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetFloodBCtmp
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetFloodBC Called.\n"));

	if(GT_OK == gsysSetFloodBC(dev, en))
		return GT_OK;
		
	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6095_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_MANAGEMENT, 12, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetFloodBC
*
* DESCRIPTION:
*		Flood Broadcast.
*		When Flood Broadcast is enabled, frames with the Broadcast destination 
*		address will flood out all the ports regardless of the setting of the
*		port's Egress Floods mode (see gprtSetEgressFlood API). VLAN rules and 
*		other switch policy still applies to these Broadcast frames. 
*		When this feature is disabled, frames with the Broadcast destination
*		address are considered Multicast frames and will be affected by port's 
*		Egress Floods mode.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if Flood Broadcast is enabled, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetFloodBCtmp
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetFloodBC Called.\n"));

	if(GT_OK == gsysGetFloodBC(dev, en))
		return GT_OK;
		
	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6095_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_MANAGEMENT,12,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gstatsGetPortAllCouterGwd
*
* DESCRIPTION:
*		get couter3 states.
*		Since some switch don't support counter3(for exsample, 8083)
*            we have to transfer ourselves to give the explusive API.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		statsCounter - couter3 states.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gstatsGetPortAllCounterGwd(	
    IN  GT_QD_DEV       *dev,
	IN  GT_LPORT		port,
	OUT GT_STATS_COUNTER_SET3	*statsCounter)
{
    GT_STATS_COUNTER_SET3 statsCounter3Set = {0};
    GT_STATS_COUNTER_SET statsCounterGet = {0};

    if (!IS_IN_DEV_GROUP(dev,DEV_RMON_TYPE_3))
    {
        if (GT_OK == gstatsGetPortAllCounters(dev, port, &statsCounterGet))
        {
            statsCounter->Collisions = statsCounterGet.Collisions;
            statsCounter->Deferred = statsCounterGet.Deferred;
            statsCounter->Excessive = statsCounterGet.Excessive;
            statsCounter->Fragments = statsCounterGet.Fragments;
            statsCounter->InBadOctets = statsCounterGet.InBadOctets;
            statsCounter->InBroadcasts = statsCounterGet.InBroadcasts;
            statsCounter->InFCSErr = statsCounterGet.InFCSErr;
            statsCounter->InGoodOctetsHi = 0;
            statsCounter->InGoodOctetsLo = statsCounterGet.InGoodOctets;
            statsCounter->InMACRcvErr = 0;
            statsCounter->InMulticasts = statsCounterGet.InMulticasts;
            statsCounter->InPause = statsCounterGet.InPause;
            statsCounter->InUnicasts = statsCounterGet.InUnicasts;
            statsCounter->Jabber = statsCounterGet.Jabber;
            statsCounter->Late = statsCounterGet.Late;
            statsCounter->Multiple = statsCounterGet.Multiple;
            statsCounter->Octets1023 = statsCounterGet.Out1023Octets;
            statsCounter->Octets127 = statsCounterGet.Out127Octets;
            statsCounter->Octets255 = statsCounterGet.Out255Octets;
            statsCounter->Octets511 = statsCounterGet.Out511Octets;
            statsCounter->Octets64 = statsCounterGet.Out64Octets;
            statsCounter->OctetsMax = statsCounterGet.OutMaxOctets;
            statsCounter->OutBroadcasts = statsCounterGet.OutBroadcasts;
            statsCounter->OutFCSErr = statsCounterGet.OutFCSErr;
            statsCounter->OutMulticasts = statsCounterGet.OutMulticasts;
            statsCounter->OutOctetsHi = 0;
            statsCounter->OutOctetsLo = statsCounterGet.OutGoodOctets;
            statsCounter->OutPause = statsCounterGet.OutPause;
            statsCounter->OutUnicasts = statsCounterGet.OutUnicasts;
            statsCounter->Oversize = statsCounterGet.Oversize;
            statsCounter->Single = statsCounterGet.Single;
            statsCounter->Undersize = statsCounterGet.Undersize;
        }
    }
    else
    {
        if (GT_OK == gstatsGetPortAllCounters3(dev, port, &statsCounter3Set))
        {
            memcpy(statsCounter, &statsCounter3Set, sizeof(GT_STATS_COUNTER_SET3));
        }
        else
        {
            return GT_ERROR;
        }
    }
    return GT_OK;
}

/*******************************************************************************
* gstatsGetPortCounter3AllPkt
*
* DESCRIPTION:
*		This routine gets a all packet counter of the given port
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		statsPktIn - points to 32bit data storage for the MIB counter
*		statsPktOut - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		This function supports Gigabit Switch and Spinnaker family
*
*******************************************************************************/
GT_STATUS gstatsGetPortCounter3AllPkt
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_U32			*statsPktIn,
	OUT GT_U32			*statsPktOut
)
{
    GT_STATUS	retVal;
	GT_STATS_COUNTER_SET3 statsCounterSet;

	DBG_INFO(("gstatsGetPortCounter3AllPkt Called.\n"));

	/* gstatsGetPortAllCounters3 used for 88E6093, 88E6095, 88E6185, and 88E6065 */
	retVal = gstatsGetPortAllCounterGwd(dev, port, &statsCounterSet);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (gstatsGetPortAllCounters3 returned GT_FAIL).\n"));
    	return retVal;
	}
	/* ulAllPacketSumRx */
	*statsPktIn = 	statsCounterSet.Fragments +
					statsCounterSet.InBroadcasts +
					statsCounterSet.InFCSErr +
					statsCounterSet.InMACRcvErr +
					statsCounterSet.InMulticasts +
					statsCounterSet.InPause +
					statsCounterSet.InUnicasts +
					statsCounterSet.Jabber + 
					statsCounterSet.Oversize + 
					statsCounterSet.Undersize;
	/* ulAllPacketSumTx */
	*statsPktOut = 	statsCounterSet.OutBroadcasts +
					statsCounterSet.OutFCSErr +
					statsCounterSet.OutMulticasts +
					statsCounterSet.OutPause +
					statsCounterSet.OutUnicasts +
					statsCounterSet.Collisions +
					statsCounterSet.Deferred +
					statsCounterSet.Excessive +
					statsCounterSet.Late + 
					statsCounterSet.Multiple + 
					statsCounterSet.Single;

	DBG_INFO(("OK.\n"));
	return GT_OK;
}

/*******************************************************************************
* gprtSetPortSpeedOnly
*
* DESCRIPTION:
* 		Sets speed only for a specific logical port. Dont' change others.
*
* INPUTS:
* 		port  - logical port number
* 		speed - port speed.
*				PHY_SPEED_10_MBPS for 10Mbps
*				PHY_SPEED_100_MBPS for 100Mbps
*				PHY_SPEED_1000_MBPS for 1000Mbps
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.13 - Speed Selection (LSB)
* data sheet register 0.6  - Speed Selection (MSB)
*
*******************************************************************************/

GT_STATUS gprtSetPortSpeedOnly
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_PHY_SPEED speed
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_PHY_INFO		phyInfo;
	GT_STATUS		retVal;
    GT_U16 			u16DataMask;

    DBG_INFO(("gprtSetPortSpeedOnly Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	u16DataMask = ~(QD_PHY_SPEED | QD_PHY_SPEED_MSB);
	u16Data = u16Data & u16DataMask;
	switch(speed)
	{
		case PHY_SPEED_10_MBPS:
			
			break;
		case PHY_SPEED_100_MBPS:
			u16Data = u16Data | QD_PHY_SPEED;
			break;
		case PHY_SPEED_1000_MBPS:
			if (!(phyInfo.flag & GT_PHY_GIGABIT))
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_BAD_PARAM;
			}
			u16Data = u16Data | QD_PHY_SPEED_MSB;
			break;
		default:
			gtSemGive(dev,dev->phyRegsSem);
			return GT_FAIL;
	}

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

	retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
  	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}

/*******************************************************************************
* gprtGetPortSpeedOnly
*
* DESCRIPTION:
* 		Gets speed only for a specific logical port. 
*
* INPUTS:
* 		port  - logical port number
*
* OUTPUTS:
* 		speed - port speed.
*				PHY_SPEED_10_MBPS for 10Mbps
*				PHY_SPEED_100_MBPS for 100Mbps
*				PHY_SPEED_1000_MBPS for 1000Mbps
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.13 - Speed Selection (LSB)
* data sheet register 0.6  - Speed Selection (MSB)
*
*******************************************************************************/

GT_STATUS gprtGetPortSpeedOnly
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
OUT GT_PHY_SPEED *speed
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_PHY_INFO		phyInfo;

    DBG_INFO(("gprtSetPortSpeedOnly Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	*speed = (u16Data & QD_PHY_SPEED)? PHY_SPEED_100_MBPS : PHY_SPEED_10_MBPS;
	if ((phyInfo.flag & GT_PHY_GIGABIT) && (u16Data & QD_PHY_SPEED_MSB))
	{
		*speed = PHY_SPEED_1000_MBPS;
	}

  	gtSemGive(dev,dev->phyRegsSem);
	return GT_OK;
}

/*******************************************************************************
* gfdbGetPortAtuLearnLimit
*
* DESCRIPTION:
*       Port's auto learning limit. When the limit is non-zero value, the number
*		of MAC addresses that can be learned on this port are limited to the value
*		specified in this API. When the learn limit has been reached any frame 
*		that ingresses this port with a source MAC address not already in the 
*		address database that is associated with this port will be discarded. 
*		Normal auto-learning will resume on the port as soon as the number of 
*		active unicast MAC addresses associated to this port is less than the 
*		learn limit.
*		CPU directed ATU Load, Purge, or Move will not have any effect on the 
*		learn limit.
*		This feature is disabled when the limit is zero.
*		The following care is needed when enabling this feature:
*			1) dsable learning on the ports
*			2) flush all non-static addresses in the ATU
*			3) define the desired limit for the ports
*			4) re-enable learing on the ports
*
* INPUTS:
*       port  - logical port number
*											  
* OUTPUTS:
*       count - auto learning limit count( 0 ~ 255 )
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if limit > 0xFF
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetPortAtuLearnLimit
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT  	port,
    IN  GT_U32   	*count
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gfdbGetPortAtuLearnLimit Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Check device if this feature is supported. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ATU_LIMIT))
    {
		return GT_NOT_SUPPORTED;
    }

    /* Get the ReadLearnCnt bit. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 15, 1, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	if(data == 1)
	{
	    /* Set the ReadLearnCnt bit. */
    	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 15, 1, 0);
	    if(retVal != GT_OK)
    	{
        	DBG_INFO(("Failed.\n"));
	        return retVal;
    	}
	}

    /* Get the LearnLimit bits. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 0, 8, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	*count = (GT_U32)data;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcSetPri0RateInKbps
*
* DESCRIPTION:
*       This routine sets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number.
*       rate    - ingress data rate limit in Kbps for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached 
*       	  or exceeded. 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_PRI0_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetPri0RateInKbps
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_U32    	   rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit, tmpLimit;

    DBG_INFO(("grcSetPri0Rate Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
    {
		GT_PIRL2_DATA gtPirlData;

/*
 *  typedef: struct GT_PIRL2_BUCKET_DATA
 *
 *  Description: data structure for PIRL2 Bucket programing that is port based.
 *
 *  Fields:
 *		ingressRate   	- commited ingress rate in kbps.
 *						  64kbps ~ 1Mbps    : increments of 64kbps,
 *						  1Mbps ~ 100Mbps   : increments of 1Mbps, and
 *						  100Mbps ~ 200Mbps : increments of 10Mbps
 *						  Therefore, the valid values are:
 *								64, 128, 192, 256, 320, 384,..., 960,
 *								1000, 2000, 3000, 4000, ..., 100000,
 *								110000, 120000, 130000, ..., 200000.
 *      accountQConf    - account discarded frames due to queue congestion
 *      accountFiltered - account filtered frames
 *      mgmtNrlEn 		- exclude management frame from ingress rate limiting calculation
 *      saNrlEn 		- exclude from ingress rate limiting calculation if the SA of the
 *						  frame is in ATU with EntryState that indicates Non Rate Limited.
 *      daNrlEn 		- exclude from ingress rate limiting calculation if the DA of the
 *						  frame is in ATU with EntryState that indicates Non Rate Limited.
 *		samplingMode	- sample one out of so many frames/bytes for a stream of frames
 *		actionMode		- action should be taken when there are not enough tokens
 *						  to accept the entire incoming frame
 *								PIRL_ACTION_ACCEPT - accept the frame
 *								PIRL_ACTION_USE_LIMIT_ACTION - use limitAction
 *		ebsLimitAction 	- action should be taken when the incoming rate exceeds
 *						  the limit.
 *								ESB_LIMIT_ACTION_DROP - drop packets
 *								ESB_LIMIT_ACTION_FC   - send flow control packet
 *		fcDeassertMode	- flow control de-assertion mode when limitAction is
 *						  set to ESB_LIMIT_ACTION_FC.
 *								GT_PIRL_FC_DEASSERT_EMPTY -
 *									De-assert when the ingress rate resource has 
 *									become empty.
 *								GT_PIRL_FC_DEASSERT_CBS_LIMIT -
 *									De-assert when the ingress rate resource has 
 *									enough room as specified by the CBSLimit.
 *		bktRateType		- bucket is either rate based or traffic type based.
 *								BUCKET_TYPE_RATE_BASED, or
 *								BUCKET_TYPE_TRAFFIC_BASED
 *      priORpt         - determine the incoming frames that get rate limited using
 *						  this ingress rate resource.
 *						  		GT_TRUE - typeMask OR priMask
 *						  		GT_FALSE - typeMask AND priMask
 *		priMask         - priority bit mask that each bit indicates one of the four
 *						  queue priorities. Setting each one of these bits indicates
 *						  that this particular rate resource is slated to account for
 *						  incoming frames with the enabled bits' priority.
 *		bktTypeMask		- used if bktRateType is BUCKET_TYPE_TRAFFIC_BASED. 
 *						  any combination of the following definitions:
 *								BUCKET_TRAFFIC_UNKNOWN_UNICAST,
 *								BUCKET_TRAFFIC_UNKNOWN_MULTICAST,
 *								BUCKET_TRAFFIC_BROADCAST,
 *								BUCKET_TRAFFIC_MULTICAST,
 *								BUCKET_TRAFFIC_UNICAST,
 *								BUCKET_TRAFFIC_MGMT_FRAME,
 *								BUCKET_TRAFFIC_ARP,
 *								BUCKET_TRAFFIC_TCP_DATA,
 *								BUCKET_TRAFFIC_TCP_CTRL,
 *								BUCKET_TRAFFIC_UDP,
 *								BUCKET_TRAFFIC_NON_TCPUDP,
 *								BUCKET_TRAFFIC_IMS,
 *								BUCKET_TRAFFIC_POLICY_MIRROR, and
 *								BUCKET_TRAFFIC_PLICY_TRAP
 *		byteTobeCounted	- bytes to be counted for accounting
 *								GT_PIRL_COUNT_ALL_LAYER1,
 *								GT_PIRL_COUNT_ALL_LAYER2, or
 *								GT_PIRL_COUNT_ALL_LAYER3
 *
 */
		if(rate)
		{
			gtPirlData.ingressRate 		= rate;
			gtPirlData.accountQConf 	= GT_FALSE;
			gtPirlData.accountFiltered	= GT_TRUE;
			gtPirlData.mgmtNrlEn		= GT_TRUE;
			gtPirlData.saNrlEn			= GT_TRUE;
			gtPirlData.daNrlEn			= GT_TRUE;
			gtPirlData.samplingMode		= GT_FALSE;
			gtPirlData.actionMode		= PIRL_ACTION_USE_LIMIT_ACTION;
			gtPirlData.ebsLimitAction	= ESB_LIMIT_ACTION_DROP;
			gtPirlData.fcDeassertMode  	= GT_PIRL_FC_DEASSERT_CBS_LIMIT;
			gtPirlData.bktRateType		= BUCKET_TYPE_TRAFFIC_BASED;
			gtPirlData.priORpt			= GT_TRUE;
			gtPirlData.priMask			= 0x01;
			gtPirlData.bktTypeMask		= BUCKET_TRAFFIC_UNKNOWN_UNICAST |
										  BUCKET_TRAFFIC_UNKNOWN_MULTICAST |
										  BUCKET_TRAFFIC_BROADCAST |
										  BUCKET_TRAFFIC_MULTICAST |
										  BUCKET_TRAFFIC_UNICAST   |
										  BUCKET_TRAFFIC_MGMT_FRAME|
										  BUCKET_TRAFFIC_ARP;

			gtPirlData.byteTobeCounted	= GT_PIRL_COUNT_ALL_LAYER3;
			retVal = gpirl2WriteResource(dev, port, 0, &gtPirlData);
		}
		else
		{
			retVal = gpirl2DisableResource(dev, port, 0);
		}
		
		return retVal;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK ) 
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
		rateLimit = (GT_U32)rate;

		if (!IS_IN_DEV_GROUP(dev,DEV_88E6183_FAMILY))
			tmpLimit = GT_GET_RATE_LIMIT2(rateLimit);
		else
			tmpLimit = GT_GET_RATE_LIMIT(rateLimit);

		if((tmpLimit == 0) && (rateLimit != 0))
			rateLimit = 1;
		else
			rateLimit = tmpLimit;

	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,12,(GT_U16)rateLimit );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }
	}
	else
	{
		return GT_BAD_PARAM;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcGetPri0RateInKbps
*
* DESCRIPTION:
*       This routine gets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number to set.
*
* OUTPUTS:
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached 
*       	  or exceeded. 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_PRI0_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetPri0RateInKbps
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_U32    *rate
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			tmpLimit;

    DBG_INFO(("grcGetPri0Rate Called.\n"));

    if(rate == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

	if (IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
    {
		GT_PIRL2_DATA gtPirlData;
		retVal = gpirl2ReadResource(dev, port, 0, &gtPirlData);
		if(GT_OK == retVal)
		{
			*rate = gtPirlData.ingressRate;
		}
		return retVal;
    }

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK ) 
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		tmpLimit = 0;
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,12,(GT_U16*)&tmpLimit );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }

		if (!IS_IN_DEV_GROUP(dev,DEV_88E6183_FAMILY))
			*rate = GT_GET_RATE_LIMIT2(tmpLimit);
		else
			*rate = GT_GET_RATE_LIMIT(tmpLimit);
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,8,3,&data );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }
	    *rate = data;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcSetEgressRateInKbps
*
* DESCRIPTION:
*       This routine sets the port's egress data limit.
*		
*
* INPUTS:
*       port      - logical port number.
*       rateType  - egress data rate limit (GT_ERATE_TYPE union type). 
*					union type is used to support multiple devices with the
*					different formats of egress rate.
*					GT_ERATE_TYPE has the following fields:
*						definedRate - GT_EGRESS_RATE enum type should used for the 
*							following devices:
*							88E6218, 88E6318, 88E6063, 88E6083, 88E6181, 88E6183,
*							88E6093, 88E6095, 88E6185, 88E6108, 88E6065, 88E6061, 
*							and their variations
*						kbRate - rate in kbps that should used for the following 
*							devices:
*							88E6097, 88E6096 with the GT_PIRL_ELIMIT_MODE of 
*								GT_PIRL_ELIMIT_LAYER1,
*								GT_PIRL_ELIMIT_LAYER2, or 
*								GT_PIRL_ELIMIT_LAYER3 (see grcSetELimitMode)
*							64kbps ~ 1Mbps    : increments of 64kbps,
*							1Mbps ~ 100Mbps   : increments of 1Mbps, and
*							100Mbps ~ 1000Mbps: increments of 10Mbps
*							Therefore, the valid values are:
*								64, 128, 192, 256, 320, 384,..., 960,
*								1000, 2000, 3000, 4000, ..., 100000,
*								110000, 120000, 130000, ..., 1000000.
*						fRate - frame per second that should used for the following
*							devices:
*							88E6097, 88E6096 with GT_PIRL_ELIMIT_MODE of 
*								GT_PIRL_ELIMIT_FRAME
*							Valid values are between 7600 and 1488000
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_EGRESS_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
*******************************************************************************/
GT_STATUS grcSetEgressRateInKbps
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_ERATE_TYPE   *rateType
)
{
#ifdef _USE_DSDT_26A_
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit, tmpLimit;
    GT_EGRESS_RATE	rate;

    DBG_INFO(("grcSetEgressRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_EGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK ) 
      return retVal;
	
	if (IS_IN_DEV_GROUP(dev,DEV_ELIMIT_FRAME_BASED))
	{
		return setEnhancedERate(dev,port,rateType);
	}

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
	{
		rateLimit =  rateType->kbRate;

		if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
			tmpLimit = GT_GET_RATE_LIMIT3(rateLimit);
		else if (!IS_IN_DEV_GROUP(dev,DEV_88E6183_FAMILY))
			tmpLimit = GT_GET_RATE_LIMIT2(rateLimit);
		else
			tmpLimit = GT_GET_RATE_LIMIT(rateLimit);

		if((tmpLimit == 0) && (rateLimit != 0))
			rateLimit = 1;
		else
			rateLimit = tmpLimit;

	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,0,12,(GT_U16)rateLimit );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }
	}
	else
	{
		return GT_BAD_PARAM;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
#else
	return grcSetEgressRate(dev, port, rateType);
#endif
}

/*******************************************************************************
* grcGetEgressRateInKbps
*
* DESCRIPTION:
*       This routine gets the port's egress data limit.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       rateType  - egress data rate limit (GT_ERATE_TYPE union type). 
*					union type is used to support multiple devices with the
*					different formats of egress rate.
*					GT_ERATE_TYPE has the following fields:
*						definedRate - GT_EGRESS_RATE enum type should used for the 
*							following devices:
*							88E6218, 88E6318, 88E6063, 88E6083, 88E6181, 88E6183,
*							88E6093, 88E6095, 88E6185, 88E6108, 88E6065, 88E6061, 
*							and their variations
*						kbRate - rate in kbps that should used for the following 
*							devices:
*							88E6097, 88E6096 with the GT_PIRL_ELIMIT_MODE of 
*								GT_PIRL_ELIMIT_LAYER1,
*								GT_PIRL_ELIMIT_LAYER2, or 
*								GT_PIRL_ELIMIT_LAYER3 (see grcSetELimitMode)
*							64kbps ~ 1Mbps    : increments of 64kbps,
*							1Mbps ~ 100Mbps   : increments of 1Mbps, and
*							100Mbps ~ 1000Mbps: increments of 10Mbps
*							Therefore, the valid values are:
*								64, 128, 192, 256, 320, 384,..., 960,
*								1000, 2000, 3000, 4000, ..., 100000,
*								110000, 120000, 130000, ..., 1000000.
*						fRate - frame per second that should used for the following
*							devices:
*							88E6097, 88E6096 with GT_PIRL_ELIMIT_MODE of 
*								GT_PIRL_ELIMIT_FRAME
*							Valid values are between 7600 and 1488000
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_EGRESS_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
*******************************************************************************/
GT_STATUS grcGetEgressRateInKbps
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_ERATE_TYPE  *rateType
)
{
#ifdef _USE_DSDT_26A_
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			tmpLimit;
    GT_EGRESS_RATE  rate;

    DBG_INFO(("grcGetEgressRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_EGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK ) 
      return retVal;
	
    if(rateType == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

	if (IS_IN_DEV_GROUP(dev,DEV_ELIMIT_FRAME_BASED))
	{
		return getEnhancedERate(dev,port,rateType);
	}

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
	{
		tmpLimit = 0;
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,0,12,(GT_U16*)&tmpLimit );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }

		if (!IS_IN_DEV_GROUP(dev,DEV_88E6183_FAMILY))
			rateType->kbRate = GT_GET_RATE_LIMIT2(tmpLimit);
		else
			rateType->kbRate = GT_GET_RATE_LIMIT(tmpLimit);
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,0,3,&data );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }
		
	    rate = (GT_EGRESS_RATE)data;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
#else
	return grcGetEgressRate(dev, port, rateType);
#endif
}

/*******************************************************************************
* grcSetBurstRateInKbps
*
* DESCRIPTION:
*       This routine sets the port's ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*       bsize	- burst size.
*       rate    - ingress data rate limit. These frames will be discarded after 
*				the ingress rate selected is reached or exceeded. 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters 
*								Minimum rate for Burst Size 24K byte is 128Kbps
*								Minimum rate for Burst Size 48K byte is 256Kbps
*								Minimum rate for Burst Size 96K byte is 512Kbps
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS: 
*		If the device supports both priority based Rate Limiting and burst size
*		based Rate limiting, user has to manually change the mode to burst size
*		based Rate limiting by calling gsysSetRateLimitMode.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetBurstRateInKbps
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_BURST_SIZE   bsize,
    IN GT_U32   	   rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit;
	GT_U32			burstSize =0;

    DBG_INFO(("grcSetBurstRateInKbps Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_BURST_RATE))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) || 
			((GT_DEVICE_REV)dev->revision < GT_REV_1))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

	switch (bsize)
	{
		case GT_BURST_SIZE_12K:
			burstSize = 0;
			break;
		case GT_BURST_SIZE_24K:
			if ((rate < 128) && (rate != 0))
				return GT_BAD_PARAM;
			burstSize = 1;
			break;
		case GT_BURST_SIZE_48K:
			if ((rate < 256) && (rate != 0))
				return GT_BAD_PARAM;
			burstSize = 3;
			break;
		case GT_BURST_SIZE_96K:
			if ((rate < 512) && (rate != 0))
				return GT_BAD_PARAM;
			burstSize = 7;
			break;
		default:
			return GT_BAD_PARAM;
	}

	rateLimit = GT_GET_BURST_RATE_LIMIT(burstSize,rate);

	rateLimit |= (GT_U32)(burstSize << 12);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,15,(GT_U16)rateLimit );
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* grcGetBurstRateInKbps
*
* DESCRIPTION:
*       This routine retrieves the port's ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       bsize	- burst size.
*       rate    - ingress data rate limit. These frames will be discarded after 
*				the ingress rate selected is reached or exceeded. 
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetBurstRateInKbps
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_BURST_SIZE   *bsize,
    OUT GT_U32          *rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit, burstSize;
	GT_U16			data;

    DBG_INFO(("grcGetBurstRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_BURST_RATE))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) || 
			((GT_DEVICE_REV)dev->revision < GT_REV_1))
    	{
        	DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
    	}
	}
		
    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,15,&data);
	rateLimit = (GT_U32)data;
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

	burstSize = rateLimit >> 12;
	rateLimit &= 0x0FFF;

/*#define GT_GET_BURST_RATE_LIMIT(_bsize,_kbps)	\
		((_kbps)?(((_bsize)+1)*8000000 / (32 * (_kbps)) + 		\
				(((_bsize)+1)*8000000 % (32 * (_kbps))?1:0))	\
				:0)*/

#define GT_GET_BURST_RATE_LIMIT_FROM_REG(_bsize,_reg)	\
		((_reg)?(((_bsize)+1)*8000000 / (32 * (_reg))):0)
				

	/*retVal = cBurstRateLimit(dev, burstSize, rateLimit, rate);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }*/

    *rate = GT_GET_BURST_RATE_LIMIT_FROM_REG(burstSize, rateLimit);

	switch (burstSize)
	{
		case 0:
			*bsize = GT_BURST_SIZE_12K;
			break;
		case 1:
			*bsize = GT_BURST_SIZE_24K;
			break;
		case 3:
			*bsize = GT_BURST_SIZE_48K;
			break;
		case 7:
			*bsize = GT_BURST_SIZE_96K;
			break;
		default:
			return GT_BAD_VALUE;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



#if (RPU_MODULE_TRUNK == RPU_YES)
/*******************************************************************************
* gsysSetTrunkPortAdd
*
* DESCRIPTION:
* 		Add port to the specified trunk
*
* INPUTS:
* 		port 	- logical port number
*           tm         - trunk member data structure
*
* OUTPUTS:
*           none
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		
*
*******************************************************************************/
GT_STATUS gsysSetTrunkPortAdd(GT_QD_DEV *dev, TRUNK_MEMBER* tm, GT_U32 port)
{
	GT_STATUS status;
	int i;
	GT_U32  trunkId;
	TRUNK_SET* ts;


	trunkId = tm->trunkId;
	
	if((dev == NULL) || (!dev->devEnabled))
		{
			DBG_INFO(("Device is not initialized\n"));
			return GT_FAIL;
		}

	for(i=0; i<tm->nTrunkPort; i++)
	{
		ts = &tm->trunkSet[i];
		
		if(ts->port == port)
		{
			DBG_INFO(("gsysSetTrunkPortAdd port %lu have been added \n",port));
			return status;
		}
	
		/* enabled trunk on the given port */
	}
	if((status = gprtSetTrunkPort(dev,port,GT_TRUE,trunkId)) != GT_OK)
		{
			DBG_INFO(("gprtSetTrunkPort return Failed\n"));
			return status;
		}
	tm->trunkSet[tm->nTrunkPort].port= port;
	tm->nTrunkPort++;
	
	return GT_OK;
}

/*******************************************************************************
* gsysSetTrunkPortDel
*
* DESCRIPTION:
* 		Delete port from the specified trunk
*
* INPUTS:
* 		port 	- logical port number
*           tm         - trunk member data structure
*
* OUTPUTS:
*           none
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		
*
*******************************************************************************/
GT_STATUS gsysSetTrunkPortDel(GT_QD_DEV *dev, TRUNK_MEMBER* tm, GT_U32 port)
{
	GT_STATUS status;
	int i;
	GT_U32 trunkId;
	TRUNK_SET* ts;


	trunkId = tm->trunkId;
	
	if((dev == NULL) || (!dev->devEnabled))
		{
			DBG_INFO(("Device is not initialized\n"));
			return GT_FAIL;
		}

	for(i=0; i<tm->nTrunkPort; i++)
	{
		ts = &tm->trunkSet[i];
		
		if(ts->port == port)
		{
			break;
		}
	
		/* enabled trunk on the given port */
	}
	if(i ==tm->nTrunkPort)
	{
		DBG_INFO(("gsysSetTrunkPortDel port %lu has not added \n",port));
			return status;
	}
	if((status = gprtSetTrunkPort(dev,port,GT_FALSE,trunkId)) != GT_OK)
		{
			DBG_INFO(("gprtSetTrunkPort return Failed\n"));
			return status;
		}
	for(;(i<tm->nTrunkPort)&&(i<MAX_PORT_IN_TRUNK-1);i++)
		tm->trunkSet[i].port=tm->trunkSet[i+1].port;
	
	tm->nTrunkPort--;
	
	return GT_OK;
}


/*******************************************************************************
* gsysSetTrunkPort
*
* DESCRIPTION:
* 		Set the specified port list of the trunk (in port register)
*
* INPUTS:
*           tm         - trunk member data structure
*
* OUTPUTS:
*           none
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		
*
*******************************************************************************/
GT_STATUS gsysSetTrunkPort(GT_QD_DEV *dev, TRUNK_MEMBER* tm)
{
	GT_STATUS status;
	int i;
	GT_U32 trunkId;
	TRUNK_SET* ts;


	trunkId = tm->trunkId;
	
	if((dev == NULL) || (!dev->devEnabled))
		{
			DBG_INFO(("Device is not initialized\n"));
			return GT_FAIL;
		}
	
	for(i=0; i<trunkGroup[trunkId].nTrunkPort; i++)
	{
		ts = &trunkGroup[trunkId].trunkSet[i];
		
		if((status = gprtSetTrunkPort(dev,ts->port,GT_FALSE,trunkId)) != GT_OK)
		{
			DBG_INFO(("gprtSetTrunkPort return Failed\n"));
			return status;
		}
	
		/* enabled trunk on the given port */
	}
	for(i=0; i<tm->nTrunkPort; i++)
	{
		ts = &tm->trunkSet[i];
		
		if((status = gprtSetTrunkPort(dev,ts->port,GT_TRUE,trunkId)) != GT_OK)
		{
			DBG_INFO(("gprtSetTrunkPort return Failed\n"));
			return status;
		}
	
		/* enabled trunk on the given port */
	}
	trunkGroup[trunkId].nTrunkPort = tm->nTrunkPort;
	for(i=0;i<tm->nTrunkPort;i++)
		{
			trunkGroup[trunkId].trunkSet[i]=tm->trunkSet[i];
		}
	
	return GT_OK;
}


/*******************************************************************************
* gsysSetTrunkRoute
*
* DESCRIPTION:
*		This function sets routing information for the given Trunk ID.
*
* INPUTS:
*           tm         - trunk member data structure
*
* OUTPUTS:
*           none
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		
*
*******************************************************************************/
GT_STATUS gsysSetTrunkRoute(GT_QD_DEV *dev, TRUNK_MEMBER* tm)
{
	GT_STATUS status;
	int i;
	GT_U32 trunkId;
	TRUNK_SET* ts;
	GT_U32 portVec = 0;	
	GT_U32 casecadeVec = 0;	/* Port 6 and 7. ToDo : get this value from user or device */



	trunkId = tm->trunkId;

	for(i=0; i<tm->nTrunkPort; i++)
	{
		ts = &tm->trunkSet[i];
		portVec |= (1 << ts->port);
	}

	/*
	 *	Set Trunk Route Table for the given Trunk ID.
	*/ 


	if((dev == NULL) || (!dev->devEnabled))
	{
		DBG_INFO(("Device is not initialized\n"));
		return GT_ERROR;
	}

	if((status = gsysSetTrunkRouting(dev,trunkId,portVec|casecadeVec)) != GT_OK)
	{
		DBG_INFO(("gsysSetTrunkRouting return Failed\n"));
		return status;
	}


	return GT_OK;
}



/*******************************************************************************
* gsysSetTrunkBalancing
*
* DESCRIPTION:
* 		Set the trunk balancing policy on the specified trunk
*
* INPUTS:
*           tm         - trunk member data structure
*
* OUTPUTS:
*           none
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		
*
*******************************************************************************/
GT_STATUS gsysSetTrunkBalancing(GT_QD_DEV *dev, TRUNK_MEMBER* tm)
{
	GT_STATUS status;
	int i,index;
	GT_U32 mask, trunkId;
	TRUNK_SET* ts;
	GT_U32 portVec = 0;	


	trunkId = tm->trunkId;
	
	for(i=0; i<tm->nTrunkPort; i++)
	{
		ts = &tm->trunkSet[i];
		portVec |= (1 << ts->port);
	}

	/*
	 *	Set Trunk Mask Table for load balancing.
	*/ 

	for(i=0; i<8; i++)
	{
		/* choose a port to be used for the given addr combo index */
		index = i % tm->nTrunkPort;
		ts = &tm->trunkSet[index];
		
	
		if((dev == NULL) || (!dev->devEnabled))
		{
			DBG_INFO(("Device is not initialized\n"));
			continue;
		}

		if(portVec == 0)
			continue;

		if((status = gsysGetTrunkMaskTable(dev ,i,&mask)) != GT_OK)
		{
			DBG_INFO(("gsysGetTrunkMaskTable return Failed\n"));
			return status;
		}

		mask &= ~portVec;

		
		mask |= (1 << ts->port);

		if((status = gsysSetTrunkMaskTable(dev,i,mask)) != GT_OK)
		{
			DBG_INFO(("gsysSetTrunkMaskTable return Failed\n"));
			return status;
		}
		else
			DBG_INFO(("\r\ngsysSetTrunkMaskTable success, trunknum =%d, mask=%lx!", i ,mask));

	}

	return GT_OK;
	
}


/*******************************************************************************
* gsysTrunkCheckPort
*
* DESCRIPTION:
* 		Get the trunk port list 
*
* INPUTS:
*           trunkId         - trunk id 
*		portVec	     -  trunk portvec
* OUTPUTS:
*           none
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		
*
*******************************************************************************/
GT_STATUS gsysTrunkCheckPort(GT_QD_DEV *dev,GT_U32 trunkId, GT_U32 portVec)
{
	GT_BOOL en=GT_FALSE;
	int i;
	GT_STATUS status;
	GT_U32 trunk;
	
	for(i=0;i<=dev->maxPorts;i++)
	{
		if((status = gprtGetTrunkPort(dev,i,&en,&trunk)!= GT_OK))
		{
			DBG_INFO(("gprtGetTrunkPort return Failed\n"));
			return status;
		}
		if((GT_TRUE == en)&&(trunkId == trunk))
		{
			if(!(portVec&(1<<i)))
			{
				DBG_INFO(("port %d is in trunk %lu, But on in portVec %08lX\n",i,trunkId,portVec));
				return status;
			}
		}
	}
	for(i=0;i<=dev->maxPorts;i++)
	{
		if(portVec&(1<<i))
		{
			if((status = gprtGetTrunkPort(dev,i,&en,&trunk)!= GT_OK))
			{
				DBG_INFO(("gprtGetTrunkPort return Failed\n"));
				return status;
			}
			if(GT_TRUE != en)
			{
				DBG_INFO(("port %d is in portVec %08lX, But disable on port\n",i, portVec));
				return status;
			}	
			if(trunkId != trunk)
			{
				DBG_INFO(("port %d is in portVec %08lX, trunk %lu, But it is enabled on on port\n",i, portVec,trunk));
				return status;
			}	
		}
	}
	return GT_OK;
}


/*******************************************************************************
* gsysTrunkShowHw
*
* DESCRIPTION:
* 		Show trunk configration
*
* INPUTS:
*		show the current trunk status
*		
* OUTPUTS:
*           none
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL - on error
*
* COMMENTS:
* 		
*
*******************************************************************************/

GT_STATUS gsysTrunkShowHw()
{
	GT_QD_DEV *dev=qdMultiDev[0];
	GT_STATUS status;
	GT_U32 mask;
	int i,j,k;
	GT_U32 portVec = 0;	
	for(i=0;i<MAX_TRUNK_GROUP;i++)
	{
		if((status = gsysGetTrunkRouting(dev,i,&portVec)!= GT_OK))
		{
			DBG_INFO(("gsysGetTrunkRouting return Failed\n"));
			return status;
		}
		if(gsysTrunkCheckPort(dev, i, portVec)!=GT_OK)
		{
			DBG_INFO(("gsysTrunkCheckPort return Failed\n"));
			return status;
		}
		if(!portVec)
			continue;
		DBG_INFO(("---------------------------------------------------------\r\n"));
		DBG_INFO(("TrunkId %d Member:",i));
		for(j=0;j<=dev->maxPorts;j++)
			{
				if(portVec&(1<<j))
					{
						DBG_INFO(("%d\t",j));
					}
			}
		DBG_INFO(("\r\n"));
		for(k=0;k<8;k++)
			{
				if((status = gsysGetTrunkMaskTable(dev ,k,&mask)) != GT_OK)
				{
					DBG_INFO(("gsysGetTrunkMaskTable return Failed\n"));
					return status;
				}
				DBG_INFO(("Path %d: ",k));
				mask&=portVec;
				for(j=0;j<=dev->maxPorts;j++)
				{
					if(mask&(1<<j))
						{
							DBG_INFO(("%d\t",j));
						}
				}
			}
		DBG_INFO(("\r\n"));
	}
	return GT_OK;
}
#endif

