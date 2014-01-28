#ifndef _switch_expo_h
#define _switch_expo_h

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
* 	Liudong     2007-05-21  Initial version based on switch.c
*
*******************************************************************************/


#include "msApiTypes.h"
#include "msApiDefs.h"

/* Should disable flow control for any port congestion will cause wan port congestion */
#define MRV_FLOW_CONTROL_ENABLE_DEF	0	
#define _DISABLE_UNI_PORT_PAUSE_

#define MASTER_UNIT				0
#define UNIT_IS_MASTER(unit) 	( unit == MASTER_UNIT )
#define UNIT_IS_VALID(unit)  	( unit <= N_OF_QD_DEVICES )

#define NUM_UNI_PORTS_PER_SWITCH	11  //mtdo 修改为可配置的端口数

/**************************************************************************************
Name: allocate_bridge_mac

Description:
    Build a MAC address based on the prefix 00:0c:d5:64 and the last two byte of
    the MAC address configured to the EEPROM (parameter 'mac').

Returns: Nothing

Paramaters:
    mac (OUT):  The composed MAC address.
**************************************************************************************/
void allocate_bridge_mac(GT_ETHERADDR * mac);

/*******************************************************************************************
  Description   : This function will initialize the 88E6095 switch chip.
  In params     : None.
  Out params    : None.
  Return value  : None.
********************************************************************************************/
void switch_init();

/*******************************************************************************************
  Description   : This function will initialize the 88E6095 switch chip.
  In params     : unit	- switch unit id.
  Out params    : None.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS switch_slave_init(GT_U32 unit);
GT_STATUS switch_slave_destroy(GT_U32 unit);

/*******************************************************************************************
  Description   : This function will initialize the 88E6095 switch chip.
  In params     : unit	- switch unit id.
  Out params    : None.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS switch_master_init(GT_U32 unit);

/*******************************************************************************************
  Description   : This function will get the cascade port info.
  In params     : unit	- switch unit id.
  Out params    : number	-	number of cascade ports.
  				  ports		-	IDs of cascade ports.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS get_switch_cascade_port_info(GT_U32 unit, GT_U32 *number, GT_U8 ports[NUM_UNI_PORTS_PER_SWITCH]);

/*******************************************************************************************
  Description   : This function will get the wan port info.
  In params     : unit	- switch unit id.
  Out params    : port	-	ID of wan port.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS get_switch_wan_port_info(GT_U32 unit, GT_U32 *port);

/*******************************************************************************************
  Description   : This function will get the cpu port info.
  In params     : unit	- switch unit id.
  Out params    : port	-	ID of wan port.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS get_switch_cpu_port_info(GT_U32 unit, GT_U32 *port);

/*******************************************************************************************
  Description   : This function will get the cascade port number on master for unit.
  In params     : unit	- switch unit id.
  Out params    : port	-	ID of wan port.
  Return value  : GT_OK for success, GT_FAILE for others.
  NOTE			: For GT861 only.
********************************************************************************************/
GT_STATUS get_switch_cascade_port_on_master(GT_U32 unit, GT_U32 *port);

GT_STATUS switch_default_config(GT_QD_DEV * dev);
GT_STATUS switch_ge_port_init(GT_QD_DEV * dev, GT_LPORT port);
GT_STATUS switch_fe_port_init(GT_QD_DEV * dev, GT_LPORT port, GT_BOOL pauseEn);
GT_STATUS smiChannelSemTake(void);
GT_STATUS smiChannelSemGive(void);
GT_STATUS setDSAMode(GT_BOOL mode);
GT_BOOL isInterswitchPort(GT_LPORT port);
GT_STATUS InternalVlanInit(GT_QD_DEV * dev, GT_U32 vid);
GT_STATUS InternalVlanJoin(GT_QD_DEV * dev, GT_U32 vid, GT_LPORT phyPort);
GT_STATUS InternalVlanLeave(GT_QD_DEV * dev, GT_U32 vid, GT_LPORT phyPort);
GT_STATUS IsolateTwoPorts(GT_QD_DEV * dev, GT_BOOL mode, GT_LPORT phyPortA, GT_LPORT phyPortB);
GT_STATUS switch_fe_port_pause_ctrl(GT_QD_DEV * dev, GT_LPORT phyPort, GT_BOOL mode);
GT_STATUS switch_fpga_port_enable(GT_BOOL mode);

#endif /* _switch_expo_h */

