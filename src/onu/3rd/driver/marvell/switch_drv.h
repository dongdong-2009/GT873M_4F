#ifndef _switch_drv_h
#define _switch_drv_h

/********************************************************************************
*        Copyright(c), 2000-2007, GW Technologies Co., Ltd. 
*                    All Rights Reserved
*
* FILE NAME: switch_expo.h
* DESCRIPTION:
* 	GW added Marvell switch functions.
*
* DEPENDENCIES:
*
* FILE REVISION HISTORY:
* 	Author		Date		Changed
* 	Liudong     2007-06-07  Initial version based on switch_drv.c
*
*******************************************************************************/

#include "msApiTypes.h"
#include "msApiDefs.h"

/* Definition for Date Types */
#define ENABLE 1
#define DISABLE 0

/* Definition for PHY Registers address which is not defined in gtDrvSwRegs.h */
#define QD_PHY_STATUS_REG			1
#define QD_PHY_ID_HI_REG			2
#define QD_PHY_ID_LO_REG			3

/* register 4 advertising register */
#define MDIO_ADV_REG_SELECTOR_LSB        0
#define MDIO_ADV_REG_10BASE_BIT          5
#define MDIO_ADV_REG_10BASE_FD_BIT       6
#define MDIO_ADV_REG_100BASE_BIT         7
#define MDIO_ADV_REG_100BASE_FD_BIT      8
#define MDIO_ADV_REG_100BASE_T4_BIT      9
#define MDIO_ADV_REG_PAUSE_FD_BIT        10
#define MDIO_ADV_REG_ASYM_PAUSE_FD_BIT   11
#define MDIO_ADV_REG_RESERVED_BIT        12


/* --------- Switch ATU related definitions ------------ */
#define MAX_ATU_NUM         255
#define INVALID_ATU_NUM     0xFFFF
#define DEFAULT_ATU_NUM     0

/* --------- Switch QoS related definitions ------------ */
#define QOS_QUEUE_NUM   4
#define HIGHEST_TRAFFIC_CLASS 3
#define HIGHEST_USER_PRIORITY 7

/* Define COS Priority Queue Policy */
#define STRICT_PRIORITY 1
#define WEIGHT_RR       2

/* Define Chip  */
#define TOTAL_NUM_OF_PORTS      11
/*
#define MULTI_ADDR_MODE
#define MANUAL_MODE
*/

#define PORT_LINK_DOWN      	0
#define PORT_LINK_UP        	1
#define PORT_LINK_CHG			2
#define PORT_LINK_CONSTENT		3

#define MRVTAG_NONE             0/*mrv don't use tag*/
#define MRVTAG_DSA              1/*mrv tag use dsa.*/
#define MRVTAG_HEAD             2/*mrv tag use header.*/
#define MRVTAG_TRAIL            3/*mrv tag use trailer.*/

#define MRVTAG_DSA_TO_CPU       0x0   /*to cpu dsa tag*/
#define MRVTAG_DSA_FROM_CPU     0x1   /*from cpu dsa tag*/
#define MRVTAG_DSA_TO_SNIFFER   0x2   /*to sniffer dsa tag*/
#define MRVTAG_DSA_FORWARD      0x3   /*forward dsa tag*/

#define MRVSW_INT_ACTIVE        0/*we have a switch interrupt now.*/
#define MRVSW_INT_NOACTIVE      1/*we don't have a switch interrupt.*/

/*有点无奈，没有产品相关度低的应用层头文件*/
#define BC_STORM_THR_DEFAULT 1000
#define BC_STORM_PWR_DOWN_DEFAULT 0	/* Only for Laiyang set to 1 */

//extern unsigned long  ulMrvTagtype;
extern GT_U8 hwPortID[12];
extern GT_U8 swPortID[12];

extern GT_U32	gulTypeAll;
extern GT_U32   gulTypeFlood;
extern GT_U32   gulTypeMc;
extern GT_U32   gulTypeBc;
extern GT_U32   gulTypeUc;

#define GT812_LPORT_2_PORT(_lport)      (hwPortID[_lport])
#define GT812_PORT_2_LPORT(_port)       (swPortID[_port])

#define QD_REG_INGRESS_EXCEED		0x15

#define MAX_PORT_IN_TRUNK 8
#define MAX_TRUNK_GROUP 16

typedef struct _TRUNK_SET {
	GT_U32	port;
} TRUNK_SET;

typedef struct _TRUNK_MEMBER {
	GT_U32 valid;
	GT_U32	trunkId;
	GT_U32	nTrunkPort;
	TRUNK_SET trunkSet[MAX_PORT_IN_TRUNK];
} TRUNK_MEMBER;

extern TRUNK_MEMBER trunkGroup[];
extern GT_U32		balance;

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
GT_STATUS gtSetCpuFrameMode(GT_LPORT unit, int MrvTagMode);


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
);


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
);

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
	IN GT_BOOL   *pbResult
);

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
);

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
);

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
);

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
);
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
);

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
);

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
);

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
);

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
);
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
);
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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

/*
 * Stop Packet Generator.
 */
GT_STATUS stopPktGenerator(GT_QD_DEV *dev,GT_LPORT port);

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
);

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
);

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
);
#endif /* _RELEASE_VERSION_ */

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
	OUT GT_STATS_COUNTER_SET3	*statsCounter);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
GT_STATUS gsysSetTrunkPortAdd
(	
	GT_QD_DEV *dev, 
	TRUNK_MEMBER* tm, 
	GT_U32 port
);


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
GT_STATUS gsysSetTrunkPortDel
(
	GT_QD_DEV *dev, 
	TRUNK_MEMBER* tm, 
	GT_U32 port
);

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
GT_STATUS gsysSetTrunkPort
(
	GT_QD_DEV *dev, 
	TRUNK_MEMBER* tm
);

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
GT_STATUS gsysSetTrunkRoute
(
	GT_QD_DEV *dev, 
	TRUNK_MEMBER* tm
);

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
GT_STATUS gsysSetTrunkBalancing
(
	GT_QD_DEV *dev, 
	TRUNK_MEMBER* tm
);

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
GT_STATUS gsysTrunkCheckPort
(
	GT_QD_DEV *dev,
	GT_U32 trunkId, 
	GT_U32 portVec
);

/*******************************************************************************
* gsysTrunkShowHw
*
* DESCRIPTION:
* 		show trunk configration 
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
GT_STATUS gsysTrunkShowHw();

#endif

#endif /* _switch_drv_h */
