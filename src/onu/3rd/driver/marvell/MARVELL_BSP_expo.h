/*
**
**  This software is licensed for use according to the terms set by
**  the Passave API license agreement.
**  Copyright Passave Ltd.
**  Ackerstein Towers - A, 9 Hamenofim St.
**  P.O.B 2089, Herziliya Pituach 46120 Israel
**
**
*/

/********************************************************************************
* MARVELL_BSP_expo.h
*
* DESCRIPTION:
*       Passave BSP for Marvell Switch Driver - API header file
*
*******************************************************************************/

#ifndef __BSP_EXPO_IND_H_
#define __BSP_EXPO_IND_H_

#include "msApi.h"

/* ---------- APPLICATION VERSION ---------- */

#define BSP_VER_MAJOR       3
#define BSP_VER_MINOR       4
#define BSP_BUILD           3

// qd_device is MRV 88E6096
#if( FOR_ONU_PON	)
#define N_OF_QD_DEVICES 1


#define DEVICE0_PHY_ADDR	1
#define DEVICE0_CPU_PORT    9
#define DEVICE0_WAN_PORT	9

#define DEVICE1_PHY_ADDR	DEVICE0_PHY_ADDR + 1
#define DEVICE2_PHY_ADDR	DEVICE0_PHY_ADDR + 2
#define DEVICE3_PHY_ADDR	DEVICE0_PHY_ADDR + 3
#define DEVICE4_PHY_ADDR	DEVICE0_PHY_ADDR + 4

#define DEVICE0_ID		DEVICE0_PHY_ADDR
#define DEVICE1_ID		DEVICE1_PHY_ADDR
#define DEVICE2_ID		DEVICE2_PHY_ADDR
#define DEVICE3_ID		DEVICE3_PHY_ADDR
#define DEVICE4_ID		DEVICE4_PHY_ADDR

#define DEVICE0_CASCADE_PORT	0xF
#define DEVICE0_CASCADE_PORT0	0xF
#define DEVICE0_CASCADE_PORT1	0xF
#define DEVICE0_CASCADE_PORT2	0xF
#define DEVICE0_CASCADE_PORT3	0xF
#define DEVICE0_CASCADE_PORT4	0xF

#define DEVICE1_WAN_PORT		9
#define DEVICE1_CPU_PORT		9
#define DEVICE1_CASCADE_PORT	9

#define DEVICE2_WAN_PORT		9
#define DEVICE2_CPU_PORT		9
#define DEVICE2_CASCADE_PORT	9

#define DEVICE3_WAN_PORT		9
#define DEVICE3_CPU_PORT		9
#define DEVICE3_CASCADE_PORT	9

#define DEVICE4_WAN_PORT		9
#define DEVICE4_CPU_PORT		9
#define DEVICE4_CASCADE_PORT	9

#define DEVICE0_FPGA_PORT		6

#endif

extern GT_QD_DEV       *qdMultiDev[N_OF_QD_DEVICES];
extern GT_QD_DEV       *qdDev0;
extern GT_QD_DEV       *qdDev1;
extern GT_QD_DEV       *qdDev2;
extern GT_QD_DEV	   *pQdDev;

#define QD_DEV_PTR		qdMultiDev[unit]
#define QD_MASTER_DEV_PTR		qdMultiDev[0]

#define NUM_UNI_PORTS_PER_SWITCH	11  //mtdo 修改为可配置的端口数

/* ------------------ Usefull marvell related macros ------------------ */

#ifndef _RELEASE_VERSION_
extern int BSP_Console_FD;
#define CHECK_MARVELL_RESULT_AND_RETURN(__drv_func__) \
    if (status != GT_OK) \
    { \
        result = status; \
        return result; \
    }

#define CHECK_MARVELL_RESULT_NO_RETURN(__drv_func__) \
    if (status != GT_OK) \
    { \
        result = status; \
    }

#define MSG_OUT(x) printf x;
#else
#define CHECK_MARVELL_RESULT_AND_RETURN(__drv_func__) 
#define CHECK_MARVELL_RESULT_NO_RETURN(__drv_func__) 
#define MSG_OUT(x)	sys_console_printf x
#endif

/* check if multicast - type GT_ETHERADDR*/
#define IS_MULTICAST(mac)    ( (mac->address[0]) & 1 )

typedef struct
{
	GT_LPORT	port;
	GT_U32		devid;
}GT_CAS_PORT;
/* ---------------- TYPEDEFS -------------------------------- */

typedef struct
{
    GT_BOOL             lan_ports[MAX_SWITCH_PORTS];    /* default: 0-3 */
    GT_ETHERADDR  switch_mac;                     /* default: 00:00:00:00:00:00 */
    GT_CAS_PORT			cas_ports[MAX_SWITCH_PORTS];
	GT_U32				numOfCasPorts;
	GT_LPORT			upLinkPort;
} Marvell_ext_switch_conf_t;


/* ---------- Passave BSP for Marvell switch - functions prototypes ---------- */

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
GT_STATUS Marvell_driver_initialize ( GT_SYS_CONFIG *cfg, GT_QD_DEV *dev, Marvell_ext_switch_conf_t *ext_conf);

/*******************************************************************************************
  Description   : init the Marvell switch mac address from eeprom
  In params     : dev - pointer to Marvell switch driver info structure
                  
  Out params    : no
  Return value  : standard Marvell API error codes
********************************************************************************************/
GT_STATUS Init_bridge_mac ( GT_QD_DEV * dev );

/*******************************************************************************************
  Description   : terminate the Marvell switch driver
  In params     : N/A
  Out params    : N/A
  Return value  : standard PAS6201 API error codes
********************************************************************************************/
GT_STATUS Marvell_driver_term ( void );


/*******************************************************************************************
  Description   : get Marvell switch extended configuration
  In params     : N/A
  Out params    : sw_conf - pointer to Marvell extended configuration structure
  Return value  : standard PAS6201 API error codes
********************************************************************************************/
GT_STATUS Marvell_driver_get_ext_switch_conf (Marvell_ext_switch_conf_t *sw_conf );


/*******************************************************************************************
  Description   : set R/STP loop ports
  In params     : stp_loop_port_1 - loop port 1 (wired to port 2)
                  stp_loop_port_2 - loop port 2 (wired to port 1)
  Out params    : N/A
  Return value  : standard PAS6201 API error codes
********************************************************************************************/
GT_STATUS Marvell_driver_set_stp_loop_ports(GT_8 stp_loop_port_1, GT_8 stp_loop_port_2);


/*******************************************************************************************
  Description   : get the R/STP loop ports
  In params     : N/A
  Out params    : stp_loop_port_1 - pointer to 8-bit value
                  stp_loop_port_1 - pointer to 8-bit value
  Return value  : standard PAS6201 API error codes
********************************************************************************************/
GT_STATUS Marvell_driver_get_stp_loop_ports(GT_8 *stp_loop_port_1, GT_8 *stp_loop_port_2);


/*******************************************************************************************
  Description   : init the Marvell switch driver for Passave ONU boards

                  Note: supported for backward compatibility only.
                  It is recommended to use the Marvell_driver_initialize() API instead.

  In params     : cfg       - pointer to Marvell switch driver configuration structure
                  dev       - pointer to Marvell switch driver info structure
                  mode      - switch scan mode (see Marvell header file msApiDefs.h):
                              SMI_MANUAL_MODE
                              SMI_AUTO_SCAN_MODE
                              SMI_MULTI_ADDR_MODE
                  base_addr - switch base register address, depend on each scan mode:
                              in SMI_MANUAL_MODE     - 0x0 or 0x10
                              in SMI_AUTO_SCAN_MODE  - not applicable (auto scans 0x0 or 0x10)
                              in SMI_MULTI_ADDR_MODE - user defined address (0x1 - 0x1F)

  Out params    : dev       - pointer to Marvell switch driver info structure

  Return value  : standard PAS6201 API error codes
********************************************************************************************/
GT_STATUS Marvell_driver_init ( GT_SYS_CONFIG *cfg, GT_QD_DEV *dev, GT_8 mode, GT_8 base_addr );


/*******************************************************************************************
  Description   : terminate the Marvell switch driver

                  Note: supported for backward compatibility only.
                  It is recommended to use the Marvell_driver_term() API instead.

  In params     : dev       - pointer to Marvell switch driver info structure
  Out params    : N/A
  Return value  : standard PAS6201 API error codes
********************************************************************************************/
GT_STATUS Marvell_driver_terminate ( GT_QD_DEV *dev );


/* --------- general Marvell switch functions -------------- */

/* add/change ATU entry with 1 port */
GT_STATUS Marvell_add_atu_entry ( GT_QD_DEV *marvell_dev, GT_ETHERADDR* mac,
                               GT_8 port, GT_BOOL is_mc );

/* add/change ATU entry with multiple ports */
GT_STATUS Marvell_add_atu_entry_portvec ( GT_QD_DEV *marvell_dev, GT_ETHERADDR* mac,
                                       GT_32 portVec, GT_BOOL is_mc, GT_U16 );

/* del MC/UC entry from Marvell DB 0 (default db) */
GT_STATUS Marvell_del_atu_entry ( GT_QD_DEV *marvell_dev, GT_ETHERADDR* mac);

/* get ATU portVector by MAC */
GT_STATUS Marvell_get_portvec ( GT_QD_DEV *marvell_dev, GT_ETHERADDR* mac, GT_32 *portVec);


/* ----------- debug helper functions ----------------------- */

void Debug_print_marvell_error_string (GT_STATUS status);

void Debug_print_marvell_portvec(GT_32 in_portvec);

void Debug_print_marvell_mac_type(GT_ATU_ENTRY *atuEntry);

void Debug_print_marvell_address_table(GT_QD_DEV *marvell_dev, GT_8 db);

void Debug_print_port_state ( GT_PORT_STP_STATE port_state);

void MessagePrint(char* format, ...);

#endif

