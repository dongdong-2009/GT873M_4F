
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
#include "cs_module.h"
#include "rcp_gwd_w.h"
#include "rcp_gwd.h"
#include "mempool.h"
#include "rtk_api.h"
#include "cs_cmn.h"
#include "startup_cfg.h"
#include "oam.h"
#include "rcp_gwd_w.h"
#include "aal_l2.h"

unsigned long gulGwdRcpAuth = 0;
RCP_DEV *rcpDevList[MAX_RRCP_SWITCH_TO_MANAGE];

RCP_REG *rcpRegList[MAX_RTL_REG_TO_ACCESS];
unsigned short mgtPvid[MAX_RRCP_SWITCH_TO_MANAGE+1] = {};
unsigned char   RcpVlanTag[2] = { 0x81, 0x00 };
unsigned char   RcpEtherType[2] = { 0x88, 0x99 };
unsigned char   RcpAuthenKeyDefault[2] = { 0x23, 0x79 };
unsigned char   RcpAuthenKeyGWD[2] = { 0x23, 0x79 };
unsigned char   RcpHelloDMacDefault[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
unsigned char   rrcpHWPortID[MAX_RCP_PORT_NUM+1] = {0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30};
unsigned char   rrcpSWPortID[MAX_RCP_PORT_NUM+1] = {17, 1, 18, 2, 19, 3, 20, 4, 21, 5, 22, 6, 23, 7, 24, 8, 25, 9, 26, 10, 27, 11, 28, 12, 29, 13, 30, 14, 31, 15, 32, 16, 0};
#if 0
RCP_DEV_INFO gDevBoradInfo[] = {
	{ 1, 16 }
};
RCP_DEV_INFO gDevUnitInfo[] = {
	{ 0, 16 }
};

RCP_LPORT_INFO gDevLPortInfo[][17] = {
	/*   0           1       2       3       4       5       6       7       8       9       10      11       12       13       14       15       16    */
	{{0xFF, 0xFF}, {0, 1}, {0, 3}, {0, 5}, {0, 7}, {0, 9}, {0, 11}, {0, 13}, {0, 15}, {0, 0}, {0, 2}, {0, 4}, {0, 6}, {0, 8}, {0, 10}, {0, 12}, {0, 14}}
};

RCP_PPORT_INFO gDevPPortInfo[][17] = {
	/* 0       1       2       3       4       5       6       7       8       9        10       11       12       13       14       15      16    */
	{{1, 9}, {1, 1}, {1, 10}, {1, 2}, {1, 11}, {1, 3}, {1, 12}, {1, 4}, {1, 13}, {1, 5}, {1, 14}, {1, 6}, {1, 15}, {1, 7}, {1, 16}, {1, 8}, {0, 0}}
}; 
#endif
#if 0
RCP_DEV_INFO gDevBoradInfo[] = {
	{ 1, 8 }
};
RCP_DEV_INFO gDevUnitInfo[] = {
	{ 0, 8 }
};
RCP_LPORT_INFO gDevLPortInfo[][9] = {
	/*   0           1       2       3       4       5       6       7       8       */
	{{0xFF, 0xFF}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}}
};

RCP_PPORT_INFO gDevPPortInfo[][9] = {
	/* 0       1       2       3       4       5       6       7       8        */
	{{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {0, 0}}
};
#endif
#if 1
RCP_DEV_INFO gDevBoradInfo[] = {
	{ 1, 8 },{ 1, 16 },{1, 24},{1, 32}
};
RCP_DEV_INFO gDevUnitInfo[] = {
	{ 0, 8 },{ 0, 16 },{0, 24},{0, 32}
};
RCP_LPORT_INFO gDevLPortInfo[][MAX_RCP_PORT_NUM+1] = {
	/*   0           1       2       3       4       5       6       7       8       */
	{{0xFF, 0xFF}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},{0xFF, 0xFF},{0xFF, 0xFF},{0xFF, 0xFF},{0xFF, 0xFF},{0xFF, 0xFF},{0xFF, 0xFF},{0xFF, 0xFF},{0xFF, 0xFF}},
	/*   0           1       2       3       4       5       6       7       8       9       10      11       12       13       14       15       16    */
	{{0xFF, 0xFF}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15}},		
	/*   0         1      2     3      4      5     6     7      8      9     10    11      12     13     14      15      16      17      18     19     20      21     22      23      24  */
	{{0xFF, 0xFF}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15}, {0, 16}, {0, 17}, {0, 18}, {0, 19}, {0, 20}, {0, 21}, {0, 22}, {0, 23}},
	/*   0         1      2     3      4      5     6     7      8      9     10    11      12     13     14      15      16      17      18     19     20      21     22      23      24    25      26      27     28     29     30      31      32 */
	{{0xFF, 0xFF}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15}, {0, 16}, {0, 17}, {0, 18}, {0, 19}, {0, 20}, {0, 21}, {0, 22}, {0, 23}, {0, 24}, {0, 25}, {0, 26}, {0, 27}, {0, 28}, {0, 29}, {0, 30}, {0, 31}}
};

RCP_PPORT_INFO gDevPPortInfo[][MAX_RCP_PORT_NUM+1] = {
	/* 0       1       2       3       4       5       6       7       8        */
	{{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}},
	/* 0     1      2      3     4      5     6     7      8     9      10      11      12     13      14     15      16    */
	{{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15}, {1, 16}, {0, 0}},
	/* 0      1     2     3      4     5      6      7     8      9      10       11    12     13     14      15       16    17      18      19     20      21     22     23     24   */
	{{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15}, {1, 16}, {1, 17}, {1, 18}, {1, 19}, {1, 20}, {1, 21}, {1, 22}, {1, 23}, {1, 24}, {0, 0}},
	/* 0      1     2     3      4     5      6      7     8      9      10       11    12     13     14      15       16    17      18      19     20      21     22     23     24      25      26     27      28      29     30      31     32*/
	{{1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15}, {1, 16}, {1, 17}, {1, 18}, {1, 19}, {1, 20}, {1, 21}, {1, 22}, {1, 23}, {1, 24}, {1, 25}, {1, 26}, {1, 27}, {1, 28}, {1, 29}, {1, 30}, {1, 31}, {1, 32}, {0, 0}}
};
#endif
/*unsigned short eeSys[] = 
	{0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};*/
unsigned short eeMgt[] =
	{0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d};
unsigned short eeQos[] =
	{
	  0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
	};
unsigned short eePort[] =
	{
	  0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48
	};
unsigned short eeMirror[] =
	{
	  0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b
	};
unsigned short eeBW[] =
	{
	  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f
	};
unsigned short eeVlan[] =
	{
	  0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 
	  0xa0, 0xa1, 0x12, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 
	  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 
	  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	  0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10a, 0x10b, 0x10c, 0x10d, 0x10e, 0x10f,
        0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x117, 0x118, 0x119, 0x11a, 0x11b, 0x11c, 0x11d, 0x11e, 0x11f,
        0x120, 0x121, 0x122, 0x123, 0x124, 0x125, 0x126, 0x127, 0x128, 0x129, 0x12a, 0x12b, 0x12c, 0x12d, 0x12e, 0x12f,
        0x130, 0x131, 0x132, 0x133, 0x134, 0x135, 0x136, 0x137, 0x138, 0x139, 0x13a, 0x13b, 0x13c, 0x13d, 0x13e, 0x13f,
        0x140, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148, 0x149, 0x14a, 0x14b, 0x14c, 0x14d, 0x14e, 0x14f,
        0x150, 0x151, 0x152, 0x153, 0x154, 0x155, 0x156, 0x157, 0x158, 0x159, 0x15a, 0x15b, 0x15c, 0x15d, 0x15e, 0x15f,
        0x160, 0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16a, 0x16b, 0x16c, 0x16d, 0x16e, 0x16f,
        0x170, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x178, 0x179, 0x17a, 0x17b, 0x17c, 0x17d, 0x17e, 0x17f,
	 };
	
cyg_sem_t	semAccessRcpDevList;
cyg_sem_t	semAccessRcpChannel;
extern unsigned long vlan_dot_1q_enable;
extern unsigned long gulEthRxTaskReady;

unsigned char gucRcpAlarmMask;
unsigned char gucRcpAlarmMaskDef = 0;

ALARM_FIFO *gpstAlmFifo;

cs_uint16 gusSwitchCfgRegAddrArray[] = {
	0x0200, 0x0300, 0x0307, 0x0308, 0x030B, 0x0400, 0x0401, 0x0402, 0x0607, 0x060A, 0x060B, 0x060C, 0x060D, 0x060E, 0x060F, 0x0610,
	0x0611, 0x0612, 0x0613, 0x0614, 0x0615, 0x0608, 0x0609, 0x0219, 0x021A, 0x021B, 0x021C, 0x021D, 0x021E, 0x020A, 0x020B, 0x020C, 
	0x020D, 0x020E, 0x020F, 0x0210, 0x0211, 0x0212, 0x0213, 0x0214, 0x0215, 0x0301, 0x0302, 0x030C, 0x030D, 0x030E, 0x030F, 0x0310, 
	0x0311, 0x0312, 0x0313, 0x0314, 0x0315, 0x0316, 0x0317, 0x0319, 0x031A, 0x031B, 0x031D, 0x031E, 0x031F, 0x0320, 0x0321, 0x0322, 
	0x0323, 0x0324, 0x0325, 0x0326, 0x0327, 0x0328, 0x0329, 0x032A, 0x032B, 0x032C, 0x032D, 0x032E, 0x032F, 0x0330, 0x0331, 0x0332, 
	0x0333, 0x0334, 0x0335, 0x0336, 0x0337, 0x0338, 0x0339, 0x033A, 0x033B, 0x033C, 0x033D, 0x033E, 0x033F, 0x0340, 0x0341, 0x0342, 
	0x0343, 0x0344, 0x0345, 0x0346, 0x0347, 0x0348, 0x0349, 0x034A, 0x034B, 0x034C, 0x034D, 0x034E, 0x034F, 0x0350, 0x0351, 0x0352, 
	0x0353, 0x0354, 0x0355, 0x0356, 0x0357, 0x0358, 0x0359, 0x035A, 0x035B, 0x035C, 0x035D, 0x035E, 0x035F, 0x0360, 0x0361, 0x0362, 
	0x0363, 0x0364, 0x0365, 0x0366, 0x0367, 0x0368, 0x0369, 0x036A, 0x036B, 0x036C, 0x036D, 0x036E, 0x036F, 0x0370, 0x0371, 0x0372, 
	0x0373, 0x0374, 0x0375, 0x0376, 0x0377, 0x0378, 0x0379, 0x037A, 0x037B, 0x037C, 0x037D, 0x037E 
	};
cs_uint16 gusSwitchCfgArraySize;
cs_uint16 gusSwitchCfgArraySizeMax = 159; /* Reserve 1 item for 0x5A5A valid flag */
cs_uint16 gusSwitchCfgArraySizeMaxInFlash = 160;
cs_uint16 gusSwitchCfgArrayItemSize;
cs_uint16 *gpusSwitchCfgFileBuf = NULL;
cs_uint16 gusSwitchDataValidFlag = 0x5A5A;
cs_long32 glSwitchCfgFileSizeMaxInFlash;



cs_uint8 gucTxRcpPktBuf[RCP_PKT_MAX_LENGTH];
cs_uint8 gucRxRcpPktBuf[RCP_PKT_MAX_LENGTH];
//extern long device_conf_write_switch_conf_to_flash( char * conf_file, long * conf_len );
//extern long device_conf_read_switch_conf_from_flash( char * conf_file, long * conf_len );
//extern long device_conf_erase_switch_conf_from_flash( void );
extern int CommOnuMsgSend(unsigned char GwOpcode, unsigned int SendSerNo, unsigned char *pSentData,const unsigned short SendDataSize, unsigned char  *pSessionIdfield);

/* Import functions from FLASH driver */
extern int user_data_config_Read(unsigned int offset, unsigned char *buffer, unsigned int size);
extern int user_data_config_Write(unsigned char *buffer, unsigned int size);
extern void dumpPkt(char *comment, int port, unsigned char *buffer, int len);

unsigned long   gulDebugRcp = 0;
#define RCP_DEBUG(str) if( gulDebugRcp ){ diag_printf str ;}
#define DUMPRCPPKT(c, p, b, l)      if(gulDebugRcp) dumpPkt(c, p, b, l)

int RCPLPort2Port(RCP_DEV *dev, unsigned long slot, unsigned long lPort, unsigned long *unit, unsigned long *phyPort)
{
	if(dev->deviceId >= (sizeof(gDevBoradInfo)/sizeof(RCP_DEV_INFO)))
		return RCP_BAD_PARAM;
	
	if((slot > gDevBoradInfo[dev->deviceId].maxSlot) || (slot == 0))
		return RCP_BAD_VALUE;

	if((lPort > gDevBoradInfo[dev->deviceId].maxPortPerSlot) || (lPort == 0))
		return RCP_BAD_VALUE;
/*
	*unit = gDevLPortInfo[dev->deviceId].L2Pmap[(slot-1)*RCP_MAX_PORTS_PER_SLOT + lPort].unit;
	*phyPort = gDevLPortInfo[dev->deviceId].L2Pmap[(slot-1)*RCP_MAX_PORTS_PER_SLOT + lPort].port;
*/
	*unit = gDevLPortInfo[dev->deviceId][(slot-1)*RCP_MAX_PORTS_PER_SLOT + lPort].unit;
	*phyPort = gDevLPortInfo[dev->deviceId][(slot-1)*RCP_MAX_PORTS_PER_SLOT + lPort].port;

	return RCP_OK;
}

int RCPPort2LPort(RCP_DEV *dev, unsigned long *slot, unsigned long *lPort, unsigned long unit, unsigned long phyPort)
{
	if(dev->deviceId >= (sizeof(gDevUnitInfo)/sizeof(RCP_DEV_INFO)))
		return RCP_BAD_PARAM;
	
	if(unit > gDevUnitInfo[dev->deviceId].maxSlot)
		return RCP_BAD_VALUE;

	if(phyPort > gDevUnitInfo[dev->deviceId].maxPortPerSlot)
		return RCP_BAD_VALUE;
/*
	*slot = gDevPPortInfo[dev->deviceId].P2Lmap[unit*RCP_MAX_PORTS_PER_UNIT + phyPort].slot;
	*lPort = gDevPPortInfo[dev->deviceId].P2Lmap[unit*RCP_MAX_PORTS_PER_UNIT + phyPort].port;
*/
	*slot = gDevPPortInfo[dev->deviceId][unit*RCP_MAX_PORTS_PER_UNIT + phyPort].slot;
	*lPort = gDevPPortInfo[dev->deviceId][unit*RCP_MAX_PORTS_PER_UNIT + phyPort].port;

	return RCP_OK;
}

int RCP_Init(void)
{
	int i;

	gusSwitchCfgArrayItemSize = sizeof(cs_uint16);
	gusSwitchCfgArraySize = sizeof(gusSwitchCfgRegAddrArray)/gusSwitchCfgArrayItemSize;

	if(gusSwitchCfgArraySize > gusSwitchCfgArraySizeMax)
	{
		gusSwitchCfgArraySize = gusSwitchCfgArraySizeMax;
	}
	
	gucRcpAlarmMask = gucRcpAlarmMaskDef;

	for(i=0; i<MAX_RRCP_SWITCH_TO_MANAGE; i++)
	{
		rcpDevList[i] = NULL;
	}

	for(i=0; i<MAX_RTL_REG_TO_ACCESS; i++)
	{
		rcpRegList[i] = NULL;
	}

	if(NULL == (gpstAlmFifo = (ALARM_FIFO *)iros_malloc(IROS_MID_MALLOC, sizeof(ALARM_FIFO))))
		return RCP_NO_MEM;

	gpstAlmFifo->lengthLimitMax = MAX_RRCP_SWITCH_TO_MANAGE * 2;
	gpstAlmFifo->lengthLimitMin = 0;
	gpstAlmFifo->occupancy = 0;
	gpstAlmFifo->top = NULL;
	gpstAlmFifo->bottom = NULL;

	glSwitchCfgFileSizeMaxInFlash = gusSwitchCfgArrayItemSize * gusSwitchCfgArraySizeMaxInFlash * MAX_RRCP_SWITCH_TO_MANAGE;
	if(NULL == (gpusSwitchCfgFileBuf = (unsigned short *)iros_malloc(IROS_MID_MALLOC, glSwitchCfgFileSizeMaxInFlash)))
		return RCP_NO_MEM;

	memset(gpusSwitchCfgFileBuf, 0, glSwitchCfgFileSizeMaxInFlash);
	device_conf_read_switch_conf_from_flash((char *)gpusSwitchCfgFileBuf, &glSwitchCfgFileSizeMaxInFlash);

	cyg_semaphore_init(&semAccessRcpDevList, 1);
	cyg_semaphore_init(&semAccessRcpChannel, 1);

	return RCP_OK;
}

cs_int32 RCP_DevList_Update(cs_uint32 parentPort, cs_int8 *pkt)
{
	RCP_HELLO_PAYLOAD *payload;
	unsigned short usAuthenkey;
	//rtk_pri_t  priority;
	cs_callback_context_t context;
	cyg_semaphore_post(&semAccessRcpChannel);

	if(parentPort >= MAX_RRCP_SWITCH_TO_MANAGE)
		return RCP_BAD_PARAM;

	if(NULL == pkt)
		return RCP_BAD_VALUE;
	if(RCP_OK != Rcp_ChipId_AuthCheck(pkt))
		return RCP_UNKOWN;
	cyg_semaphore_wait(&semAccessRcpDevList);
	
	payload = (RCP_HELLO_PAYLOAD *)(pkt + RCP_HELLO_PAYLOAD_OFFSET); 
	if(NULL == rcpDevList[parentPort])
	{
		if(NULL == (rcpDevList[parentPort] = (RCP_DEV *)iros_malloc(IROS_MID_MALLOC, sizeof(RCP_DEV))))
			return RCP_NO_MEM;
		memset(rcpDevList[parentPort], 0, sizeof(RCP_DEV));

		/* Init access function */
		if(vlan_dot_1q_enable == 1)
			epon_onu_sw_get_port_pvid(context, 0, 0, parentPort, &(rcpDevList[parentPort]->mgtVid));
		else
			rcpDevList[parentPort]->mgtVid = 0;
		rcpDevList[parentPort]->frcpReadReg = RCP_Read_Reg;
		rcpDevList[parentPort]->frcpWriteReg = RCP_Write_Reg;

		rcpDevList[parentPort]->semCreate = cyg_semaphore_init;
		rcpDevList[parentPort]->semDelete = cyg_semaphore_destroy;
		rcpDevList[parentPort]->semTake = cyg_semaphore_timed_wait;
		rcpDevList[parentPort]->semGive = cyg_semaphore_post;
		rcpDevList[parentPort]->frcpLPort2Port = RCPLPort2Port;
		rcpDevList[parentPort]->frcpPort2LPort = RCPPort2LPort;

		memcpy(rcpDevList[parentPort]->parentMac, pkt, RCP_MAC_SIZE);
		rcpDevList[parentPort]->parentPort = (unsigned char)parentPort;
		rcpDevList[parentPort]->paSlot = 1;
		rcpDevList[parentPort]->paPort = parentPort;
	}

	memcpy(rcpDevList[parentPort]->switchMac, pkt + RCP_MAC_SIZE, RCP_MAC_SIZE);

	memcpy(rcpDevList[parentPort]->chipID, payload->chipID, sizeof(rcpDevList[parentPort]->chipID));
	memcpy(rcpDevList[parentPort]->vendorID, payload->vendorID, sizeof(rcpDevList[parentPort]->vendorID));

	rcpDevList[parentPort]->upLinkPort = payload->downLinkPort;
	if(vlan_dot_1q_enable == 1)
			epon_onu_sw_get_port_pvid(context, 0, 0, parentPort, &(rcpDevList[parentPort]->mgtVid));
	else
		rcpDevList[parentPort]->mgtVid = 0;

	/* TODO: update deviceID */
	
	/* Update authenKey: use authenKey in the packet to change the authenKey */
	memcpy(rcpDevList[parentPort]->authenKey, payload->authKey, sizeof(payload->authKey));
	if(memcmp(rcpDevList[parentPort]->authenKey, RcpAuthenKeyGWD, 2) != 0)
	{
		usAuthenkey = (unsigned short)(RcpAuthenKeyGWD[0]);
		usAuthenkey <<= 8;
		usAuthenkey += (unsigned short)(RcpAuthenKeyGWD[1]);
		RCP_SetAuthenKey(rcpDevList[parentPort], usAuthenkey);
	}

	rcpDevList[parentPort]->timeoutFlag = 0;
	rcpDevList[parentPort]->timeoutCounter = 0;
	rcpDevList[parentPort]->phyAddr = 0x10;		/* useless . it depends on the phy port number */
	rcpDevList[parentPort]->alarmMask = gucRcpAlarmMask;
	
	cyg_semaphore_post(&semAccessRcpDevList);
	
	return RCP_OK;
}

int RCP_RegList_Update(unsigned long parentPort, char *pkt)
{
	RCP_GET_REPLY_PAYLOAD *payload;
	unsigned short regAddress;
	RCP_REG *pstRcpReg;

	if(parentPort >= MAX_RRCP_SWITCH_TO_MANAGE)
		return RCP_BAD_PARAM;

	if(NULL == pkt)
		return RCP_BAD_VALUE;
		
	if(NULL == rcpDevList[parentPort])
	{
		return RCP_NOT_INITIALIZED;
	}

	payload = (RCP_GET_REPLY_PAYLOAD *)(pkt + 12 + 4 + 4);	/* MACs + VlanTag + RcpTag */
	if(memcmp(rcpDevList[parentPort]->authenKey, &pkt[RCP_HELLO_PAYLOAD_OFFSET], 2) != 0)
	{
		return RCP_UNKOWN;
	}
	regAddress = GET_RCP_PKT_SHORT(payload->regAddress);
	if(NULL == (pstRcpReg = RCP_RegList_Search(rcpDevList[parentPort], regAddress)))
	{
		RCP_DEBUG(("\r\n  RCP_RegList_Update : NOT found 0x%x!", regAddress));
		return RCP_NO_SUCH;
	}
	
	pstRcpReg->value = GET_RCP_PKT_LONG(payload->regValue);
	pstRcpReg->validFlag = 1;
	cyg_semaphore_post(&(pstRcpReg->semAccess));
	
	return RCP_OK;
}

int RCP_RegList_Insert(RCP_REG *reg)
{
	int i;
	
	for(i=0; i<MAX_RTL_REG_TO_ACCESS; i++)
	{
		if(NULL == rcpRegList[i])
		{
			rcpRegList[i] = reg;
			return RCP_OK;
		}
	}

	return RCP_NO_RESOURCE;
}

RCP_REG *RCP_RegList_Search(RCP_DEV *dev, unsigned short regAddr)
{
	int i;

	for(i=0; i<MAX_RTL_REG_TO_ACCESS; i++)
	{
		if(regAddr == rcpRegList[i]->address)
		{
			if(0 == memcmp(dev->switchMac, rcpRegList[i]->dev->switchMac, RCP_MAC_SIZE))
				return rcpRegList[i];
/*			RCP_DEBUG(("\r\n  RCP_RegList_Search : MAC NOT Match [%x-%x-%x-%x-%x-%x] : [%x-%x-%x-%x-%x-%x]!", 
				dev->switchMac[0], dev->switchMac[1], dev->switchMac[2], dev->switchMac[3], dev->switchMac[4], dev->switchMac[5],
				rcpRegList[i]->dev->switchMac[0], rcpRegList[i]->dev->switchMac[1], rcpRegList[i]->dev->switchMac[2], rcpRegList[i]->dev->switchMac[3], rcpRegList[i]->dev->switchMac[4], rcpRegList[i]->dev->switchMac[5]));*/
		}
	}

	/*RCP_DEBUG(("\r\n  RCP_RegList_Search : REG 0x%x NOT FOUND!", regAddr));*/
	return NULL;
}

int RCP_RegList_Delete(RCP_REG *reg)
{
	int i;
	
	for(i=0; i<MAX_RTL_REG_TO_ACCESS; i++)
	{
		if(reg == rcpRegList[i])
		{
			rcpRegList[i] = NULL;
			return RCP_OK;
		}
	}

	return RCP_NO_SUCH;
}

int RCP_Read_Reg(RCP_DEV *dev, unsigned short regAddr, unsigned short *data) 
{
	int iRet;
	RCP_REG *pReg;
    unsigned char   *rcpPktBuf;
    unsigned char   rcpRegAddr[2];
    unsigned char   RcpVid[2];
	cs_callback_context_t     context;
	if((NULL == dev) || (NULL == data))
		return RCP_BAD_PARAM;

	/* New Reg Access Node */
	pReg = (RCP_REG *) iros_malloc(IROS_MID_MALLOC, sizeof(RCP_REG));
	if(NULL == pReg)
		return RCP_NO_MEM;

	memset(pReg, 0, sizeof(RCP_REG));
	pReg->address = regAddr;
	dev->semCreate(&(pReg->semAccess), 0);
	pReg->dev = dev;
	
	/* Insert the node to the list */
	if(RCP_OK != (iRet = RCP_RegList_Insert(pReg)))
		return iRet;

	/* Send GET packet */
	/*if(NULL == ctss_packet_send_by_port_hook)
	{
		iRet = RCP_NOT_INITIALIZED;
		goto ERROR_RETURN;
	}*/
	
	/*rcpPktBuf = (unsigned char *)iros_malloc(IROS_MALLOC_POOL_EXTEND, RCP_PKT_MAX_LENGTH);
	if(NULL == rcpPktBuf)
	{
		iRet = RCP_NO_MEM;
		goto ERROR_RETURN;
	}*/
	rcpPktBuf = gucTxRcpPktBuf;
	memset(rcpPktBuf, 0, RCP_PKT_MAX_LENGTH);
	memcpy(rcpPktBuf, dev->switchMac, 6);
	memcpy(rcpPktBuf + 6, dev->parentMac, 6);

#ifdef _CPU_CHANNEL_USE_HMII_			
	RCP_GET_MGTPORT_VLAN_TAG(dev, ulTagged);
	
	if(vlan_dot_1q_enable == 1 && ulTagged == IFM_VLAN_PORT_TAGGED)
#else
	if(vlan_dot_1q_enable == 1)
#endif
	{
		RcpVid[0] = (((0xff00 & dev->mgtVid) >> 8) | 0xf0);
		RcpVid[1] = (0xff & dev->mgtVid);
		memcpy(rcpPktBuf + 12, RcpVlanTag, 2);
		memcpy(rcpPktBuf + 14, RcpVid, 2);
		memcpy(rcpPktBuf + 16, RcpEtherType, 2);
		memset(rcpPktBuf + 18, REALTEK_PROTOCOL_RRCP, 1);
		memset(rcpPktBuf + 19, REALTEK_RRCP_OPCODE_GET, 1);
		memcpy(rcpPktBuf + 20, dev->authenKey, 2);
		SET_SHORT_TO_RCP_PKT(rcpRegAddr, regAddr);
		memcpy(rcpPktBuf + 22, rcpRegAddr, 2);
	}
	else
	{
		memcpy(rcpPktBuf + 12, RcpEtherType, 2);
		memset(rcpPktBuf + 14, REALTEK_PROTOCOL_RRCP, 1);
		memset(rcpPktBuf + 15, REALTEK_RRCP_OPCODE_GET, 1);
		memcpy(rcpPktBuf + 16, dev->authenKey, 2);
		SET_SHORT_TO_RCP_PKT(rcpRegAddr, regAddr);
		memcpy(rcpPktBuf + 18, rcpRegAddr, 2);
	}
	
	if(!dev->semTake(&semAccessRcpChannel, cyg_current_time() + RCP_RESPONSE_TIMEOUT))
	{
//		diag_printf("RCP_Read_Reg ------------------------timeout(517)\n");
		RCP_DEBUG(("\r\n  RCP_Read_Reg : semAccessRcpChannel timedout!"));
	}
	else
	{
	//	diag_printf("RCP_Read_Reg ------------------------ke yi fa song(522)\n");
	}
	
	RCP_DEBUG(("\r\n  RCP_Read_Reg : 0x%x ", regAddr));
	if(epon_request_onu_frame_send(context, 0, 0, rcpPktBuf, 64, dev->paPort, 0) != CS_OK)
	{
	//	diag_printf("RCP_Read_Reg-----------------------------send error524\n");
		iRet = RCP_FAIL;
		/*if(NULL != rcpPktBuf) iros_free(rcpPktBuf);*/
		goto ERROR_RETURN;
	}
	else
		{
		//	diag_printf("RCP_Read_Reg ------------------------send sucess517)\n");
		}

	/* Wait until reply received */
	if(!dev->semTake(&(pReg->semAccess), cyg_current_time() + RCP_RESPONSE_TIMEOUT))
	{
		//diag_printf("RCP_Read_Reg-------------------------------wait time out532\n");
		iRet = RCP_TIMEOUT;
		goto ERROR_RETURN;
	}
	else
		{
		//diag_printf("RCP_Read_Reg ------------------------wait success(517)\n");
		}
	cyg_thread_delay(1);
	if(1 == pReg->validFlag)
	{
		*data = (unsigned short)pReg->value;
	}
	else
	{
		*data = 0xFFFF;
		iRet = RCP_UNKOWN;
	}

	dev->semGive(&(pReg->semAccess));

ERROR_RETURN:
	dev->semGive(&semAccessRcpChannel);

	dev->semDelete(&(pReg->semAccess));
	RCP_RegList_Delete(pReg);
	iros_free(pReg);

	return iRet;
}

/*For the Loop Detect 32-bit register*/
int RCP_Read_32bit_Reg(RCP_DEV *dev, unsigned short regAddr, unsigned long *data) 
{
	int iRet;
	RCP_REG *pReg;
    unsigned char   *rcpPktBuf;
    unsigned char   rcpRegAddr[2];
    unsigned char   RcpVid[2];
    cs_callback_context_t     context;

	if((NULL == dev) || (NULL == data))
		return RCP_BAD_PARAM;

	/* New Reg Access Node */
	pReg = (RCP_REG *) iros_malloc(IROS_MID_MALLOC, sizeof(RCP_REG));
	if(NULL == pReg)
		return RCP_NO_MEM;

	memset(pReg, 0, sizeof(RCP_REG));
	pReg->address = regAddr;
	dev->semCreate(&(pReg->semAccess), 0);
	pReg->dev = dev;
	
	/* Insert the node to the list */
	if(RCP_OK != (iRet = RCP_RegList_Insert(pReg)))
		return iRet;

	/* Send GET packet */
	/*if(NULL == ctss_packet_send_by_port_hook)
	{
		iRet = RCP_NOT_INITIALIZED;
		goto ERROR_RETURN;
	}*/
	
	/*rcpPktBuf = (unsigned char *)iros_malloc(IROS_MALLOC_POOL_EXTEND, RCP_PKT_MAX_LENGTH);
	if(NULL == rcpPktBuf)
	{
		iRet = RCP_NO_MEM;
		goto ERROR_RETURN;
	}*/
	rcpPktBuf = gucTxRcpPktBuf;
	memset(rcpPktBuf, 0, RCP_PKT_MAX_LENGTH);
	memcpy(rcpPktBuf, dev->switchMac, 6);
	memcpy(rcpPktBuf + 6, dev->parentMac, 6);

#ifdef _CPU_CHANNEL_USE_HMII_				
	RCP_GET_MGTPORT_VLAN_TAG(dev, ulTagged);
	
	if(vlan_dot_1q_enable == 1 && ulTagged == IFM_VLAN_PORT_TAGGED)
#else
	if(vlan_dot_1q_enable == 1)
#endif		
	{
		RcpVid[0] = (((0xff00 & dev->mgtVid) >> 8) | 0xf0);
		RcpVid[1] = (0xff & dev->mgtVid);
		memcpy(rcpPktBuf + 12, RcpVlanTag, 2);
		memcpy(rcpPktBuf + 14, RcpVid, 2);
		memcpy(rcpPktBuf + 16, RcpEtherType, 2);
		memset(rcpPktBuf + 18, REALTEK_PROTOCOL_RRCP, 1);
		memset(rcpPktBuf + 19, REALTEK_RRCP_OPCODE_GET, 1);
		memcpy(rcpPktBuf + 20, dev->authenKey, 2);
		SET_SHORT_TO_RCP_PKT(rcpRegAddr, regAddr);
		memcpy(rcpPktBuf + 22, rcpRegAddr, 2);
	}
	else
	{
		memcpy(rcpPktBuf + 12, RcpEtherType, 2);
		memset(rcpPktBuf + 14, REALTEK_PROTOCOL_RRCP, 1);
		memset(rcpPktBuf + 15, REALTEK_RRCP_OPCODE_GET, 1);
		memcpy(rcpPktBuf + 16, dev->authenKey, 2);
		SET_SHORT_TO_RCP_PKT(rcpRegAddr, regAddr);
		memcpy(rcpPktBuf + 18, rcpRegAddr, 2);
	}
	
	if(!cyg_semaphore_timed_wait(&semAccessRcpChannel, cyg_current_time() + RCP_RESPONSE_TIMEOUT))
	{
		RCP_DEBUG(("\r\n  RCP_Read_32bit_Reg : semAccessRcpChannel timedout!"));
	}
	
	if(epon_request_onu_frame_send(context, 0, 0, rcpPktBuf, 64, dev->paPort, 0) != CS_OK)
	{
		iRet = RCP_FAIL;
		/*if(NULL != rcpPktBuf) iros_free(rcpPktBuf);*/
		goto ERROR_RETURN;
	}

	/* Wait until reply received */
	if(!cyg_semaphore_timed_wait(&(pReg->semAccess), cyg_current_time() + RCP_RESPONSE_TIMEOUT))
	{
		iRet = RCP_TIMEOUT;
		goto ERROR_RETURN;
	}

	if(1 == pReg->validFlag)
	{
		*data = pReg->value;
	}
	else
	{
		*data = 0xFFFFFFFF;
		iRet = RCP_UNKOWN;
	}

	dev->semGive(&(pReg->semAccess));

ERROR_RETURN:
	cyg_semaphore_post(&semAccessRcpChannel);

	dev->semDelete(&(pReg->semAccess));
	RCP_RegList_Delete(pReg);
	iros_free(pReg);

	return iRet;
}
int RCP_Write_Reg(RCP_DEV *dev, unsigned short regAddr, unsigned short data)
{
    unsigned char   *rcpPktBuf;
    unsigned char   rcpRegAddr[2];
    unsigned char   RcpVid[2];
	cs_callback_context_t     context;
	if(NULL == dev)
		return RCP_BAD_PARAM;

	/* Send SET packet */
	/*if(NULL == ctss_packet_send_by_port_hook)
	{
		return RCP_NOT_INITIALIZED;
	}*/

	/*rcpPktBuf = (unsigned char *)iros_malloc(IROS_MALLOC_POOL_EXTEND, RCP_PKT_MAX_LENGTH);
	if(NULL == rcpPktBuf)
	{
		return RCP_NO_MEM;
	}*/
	rcpPktBuf = gucTxRcpPktBuf;
	memset(rcpPktBuf, 0, RCP_PKT_MAX_LENGTH);
	memcpy(rcpPktBuf, dev->switchMac, 6);
	memcpy(rcpPktBuf + 6, dev->parentMac, 6);

#ifdef _CPU_CHANNEL_USE_HMII_					
	RCP_GET_MGTPORT_VLAN_TAG(dev, ulTagged);
	
	if(vlan_dot_1q_enable == 1 && ulTagged == IFM_VLAN_PORT_TAGGED)
#else
	if(vlan_dot_1q_enable == 1)
#endif		
	{
		RcpVid[0] = (((0xff00 & dev->mgtVid) >> 8) | 0xf0);
		RcpVid[1] = (0xff & dev->mgtVid);
		memcpy(rcpPktBuf + 12, RcpVlanTag, 2);
		memcpy(rcpPktBuf + 14, RcpVid, 2);
		memcpy(rcpPktBuf + 16, RcpEtherType, 2);
		memset(rcpPktBuf + 18, REALTEK_PROTOCOL_RRCP, 1);
		memset(rcpPktBuf + 19, REALTEK_RRCP_OPCODE_SET, 1);
		memcpy(rcpPktBuf + 20, dev->authenKey, 2);
		SET_SHORT_TO_RCP_PKT(rcpRegAddr, regAddr);
		memcpy(rcpPktBuf + 22, rcpRegAddr, 2);
		SET_SHORT_TO_RCP_PKT(rcpRegAddr, data);
		memcpy(rcpPktBuf + 24, rcpRegAddr, 2);
	}
	else
	{
		memcpy(rcpPktBuf + 12, RcpEtherType, 2);
		memset(rcpPktBuf + 14, REALTEK_PROTOCOL_RRCP, 1);
		memset(rcpPktBuf + 15, REALTEK_RRCP_OPCODE_SET, 1);
		memcpy(rcpPktBuf + 16, dev->authenKey, 2);
		SET_SHORT_TO_RCP_PKT(rcpRegAddr, regAddr);
		memcpy(rcpPktBuf + 18, rcpRegAddr, 2);
		SET_SHORT_TO_RCP_PKT(rcpRegAddr, data);
		memcpy(rcpPktBuf + 20, rcpRegAddr, 2);
	}
	
	cyg_semaphore_wait(&semAccessRcpChannel);
	if(epon_request_onu_frame_send(context, 0, 0, rcpPktBuf, 64, dev->paPort, 0) != CS_OK)
	{
		/*if(NULL != rcpPktBuf) iros_free(rcpPktBuf);*/
		cyg_semaphore_post(&semAccessRcpChannel);
		return RCP_FAIL;
	}
	else
	{
//		diag_printf("write send right--------------------------------741\n");
	}

	cyg_semaphore_post(&semAccessRcpChannel);
	return RCP_OK;
}


cs_int32 RCP_Say_Hello(cs_int32 parentPort, cs_uint16 broadcastVid)
{	
    cs_uint8 *rcpPktBuf;
 //   cs_uint8 cMac[6];
 	cs_mac_t cMac;
    cs_uint8 *switchMac;
    cs_uint8 *authKey;
    cs_uint8 lport;
    cs_uint8 RcpVid[2];
    cs_int32 iRet, eponRet;
	//cs_uint8 state;

	rtk_pri_t  broadcastVid_w;
	cs_callback_context_t     context;
	cs_sdl_port_link_status_t    link_status;
    cs_status ret = CS_E_OK;
	broadcastVid_w = broadcastVid;
	if(0 == gulEthRxTaskReady)
	{
		return RCP_NOT_INITIALIZED;
	}
	switchMac = RcpHelloDMacDefault;/*ff ff ff ff ff ff*/
	authKey = RcpAuthenKeyDefault;	/* Realtek Hello for discovery  0x23 0x79*/

	rcpPktBuf = gucTxRcpPktBuf;/*len max 80 */
	iRet = RCP_OK;

	if(parentPort > 0)	/* Unicast Hello */
	{
		if(parentPort >= MAX_RRCP_SWITCH_TO_MANAGE)
			return RCP_BAD_PARAM;
		if(NULL != rcpDevList[parentPort])
		{
			switchMac = rcpDevList[parentPort]->switchMac;
			authKey = rcpDevList[parentPort]->authenKey;
			RcpVid[0] = (((rcpDevList[parentPort]->mgtVid & 0xff00) >> 8) | 0xf0);
			RcpVid[1] = (rcpDevList[parentPort]->mgtVid & 0xff);
		}
		lport = parentPort;

      //  ctc_oam_eth_port_link_get_adapt(lport,&state);/*get port state*/

		ret = epon_request_onu_port_link_status_get(context, 
       		   0,0, lport, &link_status);
        if (link_status != SDL_PORT_LINK_STATUS_UP) 
			{
			    return RCP_OK;
        	}
		memset(rcpPktBuf, 0, RCP_PKT_MAX_LENGTH);
		memcpy(rcpPktBuf, switchMac, 6);
		/*
		epon_onu_get_local_mac(cMac);
		*/
	   // ret = startup_config_read(CFG_ID_MAC_ADDRESS, CS_MACADDR_LEN, cMac); /*read onu mac*/ 
		epon_request_onu_pon_mac_addr_get(context, 0, 0, &cMac);
		memcpy(rcpPktBuf + 6, cMac.addr, 6);

		if(vlan_dot_1q_enable == 1)/*tag yes or not*/
		{
			memcpy(rcpPktBuf + 12, RcpVlanTag, 2);
			memcpy(rcpPktBuf + 14, RcpVid, 2);/*have tagged*/
			memcpy(rcpPktBuf + 16, RcpEtherType, 2);
			memset(rcpPktBuf + 18, REALTEK_PROTOCOL_RRCP, 1);
			memset(rcpPktBuf + 19, REALTEK_RRCP_OPCODE_HELLO, 1);
			memcpy(rcpPktBuf + 20, authKey, 2);
		}
		else
		{
			memcpy(rcpPktBuf + 12, RcpEtherType, 2);
			memset(rcpPktBuf + 14, REALTEK_PROTOCOL_RRCP, 1);
			memset(rcpPktBuf + 15, REALTEK_RRCP_OPCODE_HELLO, 1);
			memcpy(rcpPktBuf + 16, authKey, 2);
		}
		if(!cyg_semaphore_timed_wait(&semAccessRcpChannel, cyg_current_time() + 2))
		{
			RCP_DEBUG(("\r\n  Unicast Hello semAccessRcpChannel timedout! "));
		}
		if(epon_request_onu_frame_send(context, 0, 0, rcpPktBuf, 64, lport, 0) != CS_OK)
		{
			RCP_DEBUG(("\r\nepon_onu_sw_send_frame return : %d ", eponRet));
			iRet = RCP_FAIL;
		}
		cyg_semaphore_post(&semAccessRcpChannel);
	}
	else if(parentPort == 0)	/* GWD Hello for keep alive */
	{
		authKey = RcpAuthenKeyGWD;
	}
	else if(parentPort < 0)		/* Broadcast Hello*/
	{
		for(lport = 1; lport < NUM_PORTS_PER_SYSTEM; lport++)
		{
       	   // ctc_oam_eth_port_link_get_adapt(lport,&state);/*get port state*/
		    ret = epon_request_onu_port_link_status_get(context, 
       			 0,0, lport, &link_status);
            if (link_status != SDL_PORT_LINK_STATUS_UP) 
            	{
					continue;
            	}
			/*epon_onu_sw_get_port_pvid(lport, &(broadcastVid));*/
			epon_onu_sw_get_port_pvid(context, 0, 0, lport, &(broadcastVid));
			
			RcpVid[0] = (((0xff00 & broadcastVid) >> 8) | 0xf0);
			RcpVid[1] = (0xff & broadcastVid);

			memset(rcpPktBuf, 0, RCP_PKT_MAX_LENGTH);
			memcpy(rcpPktBuf, switchMac, 6);

			//epon_onu_get_local_mac(cMac);
			epon_request_onu_pon_mac_addr_get(context, 0, 0, &cMac);/*read onu mac*/ 		
			memcpy(rcpPktBuf + 6, cMac.addr, 6);

			if(vlan_dot_1q_enable == 1)
			{
				memcpy(rcpPktBuf + 12, RcpVlanTag, 2);
				memcpy(rcpPktBuf + 14, RcpVid, 2);
				memcpy(rcpPktBuf + 16, RcpEtherType, 2);
				memset(rcpPktBuf + 18, REALTEK_PROTOCOL_RRCP, 1);
				memset(rcpPktBuf + 19, REALTEK_RRCP_OPCODE_HELLO, 1);
				memcpy(rcpPktBuf + 20, authKey, 2);
			}
			else
			{
				memcpy(rcpPktBuf + 12, RcpEtherType, 2);
				memset(rcpPktBuf + 14, REALTEK_PROTOCOL_RRCP, 1);
				memset(rcpPktBuf + 15, REALTEK_RRCP_OPCODE_HELLO, 1);
				memcpy(rcpPktBuf + 16, authKey, 2);
			}

			if(!cyg_semaphore_timed_wait(&semAccessRcpChannel, cyg_current_time() + 20))
			{
				RCP_DEBUG(("\r\n  Braodcast Hello semAccessRcpChannel timedout!"));
			}
			#if 0
		    if(0 != (eponRet = epon_onu_sw_send_frame(lport, rcpPktBuf, 64)))
			{
				RCP_DEBUG(("\r\nepon_onu_sw_send_frame return : %d ", eponRet));
				iRet = RCP_FAIL;
			}
			#endif
		    ret = epon_request_onu_frame_send(context, 0,0, rcpPktBuf, 64, lport, 0);
			cyg_semaphore_post(&semAccessRcpChannel);
		}
	}
	return iRet;
}

int RCP_Dev_Is_Exist(unsigned long parentPort)
{
	//rtk_pri_t  priority;
	cs_callback_context_t context;
	if(parentPort >= MAX_RRCP_SWITCH_TO_MANAGE)
		{
			return RCP_BAD_PARAM;
		}

	if(NULL != rcpDevList[parentPort])
	{
		if(0 == rcpDevList[parentPort]->chipID)
		{	
			return 0;
		}
		else
		{
			if(1 == rcpDevList[parentPort]->onlineStatus)
				{
					return 1;
				}
			else
				{
					return 0;
				}
		}
	}

	/*epon_onu_sw_get_port_pvid(parentPort, &(mgtPvid[parentPort]));*/
	epon_onu_sw_get_port_pvid(context, 0, 0, parentPort, &(mgtPvid[parentPort]));
	RCP_Say_Hello(parentPort, mgtPvid[parentPort]);		/*Unicast Hello*/
	cyg_thread_delay(IROS_TICK_PER_SECOND/10);
	if(NULL != rcpDevList[parentPort])
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
/*-----------------------------------------------------------------------------------------------------------
API:判断ONU端口下设备是不是合法的设备(通过MAC 判断)。

-------------------------------------------------------------------------------------------------------------*/
cs_int32 RCP_Dev_Is_Valid(cs_ulong32 parentPort)
{
	cs_uint8 nullMac[6];
	
	if(parentPort >= MAX_RRCP_SWITCH_TO_MANAGE)
		return RCP_BAD_PARAM;

	if(NULL == rcpDevList[parentPort])
	{
		return 0;
	}
	else
	{
		memset(nullMac, 0, sizeof(nullMac));
		if(0 == memcmp(rcpDevList[parentPort]->switchMac, nullMac, RCP_MAC_SIZE))
		{	
			return 0;
		}
	}
	return 1;
}

int RCP_Dev_Status_Set(unsigned long parentPort, unsigned char up)
{
	if(parentPort >= MAX_RRCP_SWITCH_TO_MANAGE)
		return RCP_BAD_PARAM;

	if(NULL == rcpDevList[parentPort])
		return RCP_NOT_INITIALIZED;
		
	if(up)
	{
		rcpDevList[parentPort]->onlineStatus = 1;
	}
	else
	{
		rcpDevList[parentPort]->onlineStatus = 0;
	}

	return RCP_OK;
}

RCP_DEV *RCP_Get_Dev_Ptr(unsigned long parentPort)
{
	if(parentPort >= MAX_RRCP_SWITCH_TO_MANAGE)
		return NULL;

	return rcpDevList[parentPort];
}

#define CALC_MASK(fieldOffset,fieldLen,mask)        \
            if((fieldLen + fieldOffset) >= 16)      \
                mask = (0 - (1 << fieldOffset));    \
            else                                    \
                mask = (((1 << (fieldLen + fieldOffset))) - (1 << fieldOffset))

int Rcp_ChipId_AuthCheck(char *packet)
{
	int ret;
	unsigned short  chip_id, tmpdata;
	unsigned char   tmp[2];
	
	if(NULL == packet)
		return RCP_ILLEGAL_ADDRESS;
	
	memcpy(tmp , packet + RCP_HELLO_CHIPID_OFFSET, 2);
	tmpdata = (unsigned short)tmp[1];
	chip_id = tmpdata<<8 | tmp[0];
	
	switch(chip_id)
	{
		case RTL_8208OR8316 :
			ret = RCP_OK;
			break;
		case RTL_8324_FOR_GH1524:
			ret = RCP_OK;
			break;
		case RTL_8324_FOR_GH1532:
			ret = RCP_OK;
			break;
		default:
			ret = RCP_UNKOWN;
			break;
	}
	return ret; 		
}		

int RCP_GetEeaddrFromReg
(
    IN  unsigned short regAddr,
    IN  unsigned short fieldOffset,
    IN  unsigned short fieldLength,
    OUT unsigned short *eeAddr,
    OUT unsigned char *eeOffset,
    OUT unsigned char *eeLength
)
{
    int i;

    if((regAddr >= 0x0200) && (regAddr <= 0x0208))
    {
        i = regAddr - 0x0200;
        if(fieldOffset < 8)
        {
            *eeAddr = eeMgt[2*i];
            *eeOffset = (unsigned char)fieldOffset;
        }
        else
        {
            *eeAddr = eeMgt[2*i +1];
            *eeOffset = (unsigned char)fieldOffset - 8;
        }
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if((regAddr >= 0x020A) && (regAddr <= 0x0211))
    {
        i = regAddr - 0x020A;
        if(fieldOffset < 8)
        {
            *eeAddr = eeBW[2*i];
            *eeOffset = (unsigned char)fieldOffset;
        }
        else
        {
            *eeAddr = eeBW[2*i +1];
            *eeOffset = (unsigned char)fieldOffset - 8;
        }
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if((regAddr >= 0x0219) && (regAddr <= 0x021D))
    {
        i = regAddr - 0x0219;
        if(fieldOffset < 8)
        {
            *eeAddr = eeMirror[2*i];
            *eeOffset = (unsigned char)fieldOffset;
        }
        else
        {
            *eeAddr = eeMirror[2*i +1];
            *eeOffset = (unsigned char)fieldOffset - 8;
        }
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if(regAddr == 0x0300)
    {
        *eeAddr = 0x22;
        *eeOffset = (unsigned char)fieldOffset;
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if(regAddr == 0x0308)
    {
        *eeAddr = 0x26;
        *eeOffset = (unsigned char)fieldOffset;
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if(regAddr == 0x030B)
    {
        *eeAddr = 0x28;
        *eeOffset = (unsigned char)fieldOffset;
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if((regAddr >= 0x030C) && (regAddr <= 0x037D))
    {
        i = regAddr - 0x030C;
        if(fieldOffset < 8)
        {
            *eeAddr = eeVlan[2*i];
            *eeOffset = (unsigned char)fieldOffset;
        }
        else
        {
            *eeAddr = eeVlan[2*i +1];
            *eeOffset = (unsigned char)fieldOffset - 8;
        }
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if((regAddr >= 0x0400) && (regAddr <= 0x0402))
    {
        i = regAddr - 0x0400;
        if(fieldOffset < 8)
        {
            *eeAddr = eeQos[2*i];
            *eeOffset = (unsigned char)fieldOffset;
        }
        else
        {
            *eeAddr = eeQos[2*i +1];
            *eeOffset = (unsigned char)fieldOffset - 8;
        }
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if(regAddr == 0x0607)
    {
        *eeAddr = 0x38;
        *eeOffset = (unsigned char)fieldOffset;
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if(regAddr == 0x0608)
    {
        if(fieldOffset < 8)
        {
            *eeAddr = 0x68;
            *eeOffset = (unsigned char)fieldOffset;
        }
        else
        {
            *eeAddr = 0x69;
            *eeOffset = (unsigned char)fieldOffset;
        }
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
    if((regAddr >= 0x060A) && (regAddr <= 0x0611))
    {
        i = regAddr - 0x060A;
        if(fieldOffset < 8)
        {
            *eeAddr = eePort[2*i];
            *eeOffset = (unsigned char)fieldOffset;
        }
        else
        {
            *eeAddr = eePort[2*i +1];
            *eeOffset = (unsigned char)fieldOffset - 8;
        }
        *eeLength = (unsigned char)fieldLength;
    	return RCP_OK;
    }
	return RCP_BAD_PARAM;
}
int RCP_GetRegField
(
    IN  RCP_DEV  *rrcp_dev, 
    IN  unsigned short  regAddr, 
    IN  unsigned short  fieldOffset, 
    IN  unsigned short  fieldLength, 
    OUT unsigned short  *data
)
{
    unsigned short    tmpData;
    unsigned short    mask;
    int ret;
    
    if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, regAddr, &tmpData)))  
        return ret;
    
    CALC_MASK(fieldOffset,fieldLength,mask);
    
    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    return  RCP_OK;
}
int RCP_SetRegisterField
(
    IN  RCP_DEV  *rrcp_dev, 
    IN  unsigned short  regAddr, 
    IN  unsigned short  fieldOffset, 
    IN  unsigned short  fieldLength, 
    IN  unsigned short  data
)
{
    unsigned short tmpData;
    unsigned short mask;
    int ret;
    
    {
        if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, regAddr, &tmpData)))
            return ret;
        CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
        tmpData &= (~mask);
    /* Set the given data into the above reset bits.    */
        tmpData |= ((data << fieldOffset) & mask); 
        
        if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, regAddr, tmpData)))
            return ret;
    }    
    return RCP_OK;
}
int RCP_GetEepromValue
(   
    IN  RCP_DEV  *rrcp_dev, 
    IN  unsigned short   eeAddr, 
    OUT unsigned char   *value
)
{
    unsigned short data;
    int ret;
    data = eeAddr | REALTEK_EEPROM_READ;
    
    if(RCP_OK != (ret = RCP_SetRegisterField(rrcp_dev, RT8326_EEPROM_RW_CTL, _Eeprom_Addr_Pos, 12, data)))
        return ret;
    do
    {
       if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_EEPROM_RW_CTL, _Eeprom_Busy_Pos, 1, &data)))
           return ret;
    }while(data != 0);
    
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_EEPROM_RW_CTL, _Eeprom_Result_Pos, 1, &data)))
        return ret;
    if(data == 0)
    {
       if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_EEPROM_RW_DATA, _Eeprom_Read_Pos, 8, &data)))
           return ret;
       *value = (unsigned char)data;
    }
    else
       return ret;
    return RCP_OK;  
}
int RCP_SetEepromValue
(
    IN  RCP_DEV  *rrcp_dev, 
    IN  unsigned short   eeAddr, 
    IN  unsigned char   value
)
{
    unsigned short data;
    int ret;
    
    data = eeAddr | REALTEK_EEPROM_WRITE;
    if(RCP_OK != (ret = RCP_SetRegisterField(rrcp_dev, RT8326_EEPROM_RW_DATA, _Eeprom_Addr_Pos, 8, value)))
           return ret;
    if(RCP_OK != (ret = RCP_SetRegisterField(rrcp_dev, RT8326_EEPROM_RW_CTL, _Eeprom_Addr_Pos, 12, data)))        return ret; 
        
    do
    {
       if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_EEPROM_RW_CTL, _Eeprom_Busy_Pos, 1, &data)))
           return ret;
    }while(data != 0);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_EEPROM_RW_CTL, _Eeprom_Result_Pos, 1, &data)))
        return ret;
    if(data != 0)
       return ret;
    return RCP_OK;
}
int RCP_GetEepromField
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short   eeAddr, 
    IN  unsigned char   fieldOffset, 
    IN  unsigned char   fieldLen, 
    OUT unsigned char   *value
)
{
    unsigned char  tmpData;
    unsigned char  mask;
    int ret;
    
    if(RCP_OK != (ret = RCP_GetEepromValue(rrcp_dev, eeAddr, &tmpData)))
        return ret;
    CALC_MASK(fieldOffset,fieldLen,mask);
    
    tmpData = (tmpData & mask) >> fieldOffset;
    *value = tmpData;
    
    return RCP_OK;
}
int RCP_SetEepromField
(
    IN  RCP_DEV   *rrcp_dev,
    IN  unsigned short    eeAddr,
    IN  unsigned char    fieldOffset,
    IN  unsigned char    fieldLen,
    IN  unsigned char    value
)
{
    unsigned char tmpData;
    unsigned char mask;
    int ret;
    
    if(RCP_OK != (ret = RCP_GetEepromValue(rrcp_dev, eeAddr, &tmpData)))
        return ret;
    CALC_MASK(fieldOffset,fieldLen,mask);
   
    tmpData &= ~mask;      
    tmpData |= ((value << fieldOffset) & mask);
    
    if(RCP_OK != (ret = RCP_SetEepromValue(rrcp_dev, eeAddr, tmpData)))
        return ret;
    
    return RCP_OK;
}
int RCP_SetRegField
(
    IN  RCP_DEV  *rrcp_dev, 
    IN  unsigned short  regAddr, 
    IN  unsigned short  fieldOffset, 
    IN  unsigned short  fieldLength, 
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    unsigned short eeAddr;
    unsigned char eeOffset;
    unsigned char eeLength;
    if(0x02 == (flag & RCP_CONFIG_EEPROM))
    {
        
        if(RCP_OK != (ret = RCP_GetEeaddrFromReg(regAddr, fieldOffset, fieldLength, &eeAddr, &eeOffset, &eeLength)))
            return ret;
        if(RCP_OK != (ret = RCP_SetEepromField(rrcp_dev, eeAddr, eeOffset, eeLength, (unsigned char)data)))
            return ret;
    }
    if(0x01 == (flag & RCP_CONFIG_REGISTER))     /* config to register */
    {
        if(RCP_OK != (ret = RCP_SetRegisterField(rrcp_dev, regAddr, fieldOffset, fieldLength, data)))
            return ret;
    }
    return RCP_OK;
}
int RCP_SysSWReset(RCP_DEV  *rrcp_dev)
{
    int ret;

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_SYS_RESET_CTL_BASE, _SRST_Pos, 1, 1, RCP_CONFIG_2_REGISTER)))
        return ret;
    return RCP_OK;
}
int RCP_SysHWReset(RCP_DEV  *rrcp_dev)
{
    int ret;
    
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_SYS_RESET_CTL_BASE, _HRST_Pos, 1, 1, RCP_CONFIG_2_REGISTER)))
        return ret;
    return RCP_OK;
}
int RCP_GetSysEeprom
(
    IN  RCP_DEV  *rrcp_dev, 
    OUT unsigned short  *data
)
{
    return RCP_OK;
}
int RCP_GetAuthenkey
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_RRCP_AUTHKEY, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetAuthenKey
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, RT8326_RRCP_AUTHKEY, data)))
        return ret;
    else
    {
        rrcp_dev->authenKey[0] = (unsigned char)((data & 0xFF00) >> 8);
        rrcp_dev->authenKey[1] = (unsigned char)(data & 0x00FF);
    }
    return RCP_OK;
}
int RCP_GetMaxPktLength
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_SWITCH_CTL, _MAX_PKT_LEN_POS, 2, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetMaxPktLength
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_SWITCH_CTL, _MAX_PKT_LEN_POS, 2, data, RCP_CONFIG_REGISTER)))
        return ret;
    return RCP_OK;
}
int RCP_GetMaxPauseCnt
(
    IN  RCP_DEV  *rrcp_dev,
    OUT  unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_SWITCH_CTL, _MAX_PAUSE_CNT_POS, 1, data)))
        return ret;
    return RCP_OK;
}
/*************************************  port status  ***********************/
/*************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current link up status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : link is down
*                 1 : link is up
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
**************************************************/
int RCP_GetPortLink
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_STATUS(phyPort);
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Link_Up_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current speed status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 00 : 10M
*                 01 : 100M
*                 10 : 1000M
*                 11 : 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortSpeed
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
    	{
        	return ret;
    	}
    regAddr = RT8326_PORT_STATUS(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Link_Speed_Pos(phyPort), 2, data)))
    	{
        	return ret;
    	}

    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current duplex status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : duplex half
*                 1 : duplex full
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortDuplex
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short   *data
)
{
    unsigned short  regAddr;
    unsigned long   unit;
    unsigned long   phyPort;
    int ret;
    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_STATUS(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Full_Duplex_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the specific port's current Auto Negotiation status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : auto negotiation disabled
*                 1 : auto negotiation enabled
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortAutoNego
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;
    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_STATUS(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Auto_Negotiation_En_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current flow control status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : flow control disabled
*                 1 : flow control enabled
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short   *data
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit, &phyPort)))
        return ret;
    regAddr = RT8326_PORT_STATUS(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Flow_Ctl_En_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_GetPortLoopDetect
(
    IN  RCP_DEV  *rrcp_dev, 
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    return RCP_OK;
}
int RCP_GetPortTruckFault
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    return RCP_OK;
}
int RCP_GetPortRouterPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    return RCP_OK;
}

/*************************************  global configiration ***************/
int RCP_GetFullDuplexFC
(
    IN  RCP_DEV  *rrcp_dev,
    OUT  unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _FlowControl_FD_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetFullDuplexFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _FlowControl_FD_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetHalfDuplexFC
(
    IN  RCP_DEV  *rrcp_dev,
    OUT  unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _FlowControl_FD_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetHalfDuplexFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _FlowControl_FD_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetBroadcastFC
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _FlowControl_BC_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetBroadcastFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _FlowControl_BC_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the multicast packet strict flow control status.
*
* OUTPUTS:
*       data - 0 : broadcast storm filter enabled(default)
*                 1 : broadcast storm filter disabled
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetMulticastFC
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _FlowControl_MC_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetMulticastFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _FlowControl_MC_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the broadcast storm filter control status.
*
* OUTPUTS:
*       data - 0 : broadcast storm filter enabled(default)
*                 1 : broadcast storm filter disabled
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetBroadcastStormFilter
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _Filter_BC_Storm_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the broadcast storm filter control status.
*
* INPUTS:
*       data - 0 : broadcast storm filter enabled(default)
*                 1 : broadcast storm filter disabled
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetBroadcastStormFilter
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_GLOBAL_PORT_CTL_BASE, _Filter_BC_Storm_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetLoopDetect
(
    IN  RCP_DEV  *rrcp_dev,
    OUT  unsigned short *data
)
{
    int ret; 
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_RRCP_CTL_BASE, _Loop_Fault_Detect_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetLoopDetect
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_RRCP_CTL_BASE, _Loop_Fault_Detect_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetLoopPort
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned long *data
)
{
    int ret; 
	/*0x0101H :Loop Detect Status Register (32-bit Register)*/
    if(RCP_OK != (ret = RCP_Read_32bit_Reg(rrcp_dev, RT8326_PORT_LOOP_DETECT_BASE, data)))
        return ret;
    return RCP_OK;
}
int RCP_GetIGMPSnooping
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_IGMP_SNOOP_BASE, _IGMP_Snoop_Enabled_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetIGMPSnooping
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_IGMP_SNOOP_BASE, _IGMP_Snoop_Enabled_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
/*************************************  MIB information ********************/
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current RX Counter Object status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 00 : RX Byte counter
*                 01 : RX Packet counter
*                 10 : CRC error packet counter
*                 11 : Collision packet counter
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetMIBRxObject
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT  unsigned short  *data
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_MIB_OBJ_SEL(phyPort), RX_MIB_OBJ_Pos(phyPort), 2, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current RX Counter Object status.
*
* INPUTS:
*       port - the logical port number.

*       data - 00 : RX Byte counter
*                 01 : RX Packet counter
*                 10 : CRC error packet counter
*                 11 : Collision packet counter
*       flag - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS: When RX Counter Object changes from one status to anohter,
*                  the RX Counter will be cleared to 0 itself.
*******************************************************/
int RCP_SetMIBRxObject
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_MIB_OBJ_SEL(phyPort), RX_MIB_OBJ_Pos(phyPort), 2, data, flag)))
            return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current TX Counter Object status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 00 : TX Byte counter
*                 01 : TX Packet counter
*                 10 : CRC error packet counter
*                 11 : Collision packet counter
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetMIBTxObject
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_MIB_OBJ_SEL(phyPort), TX_MIB_OBJ_Pos(phyPort), 2, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine Set the spesicfic port's current TX Counter Object status.
*
* INPUTS:
*       port - the logical port number.
*
*       data - 00 : TX Byte counter
*                 01 : TX Packet counter
*                 10 : CRC error packet counter
*                 11 : Collision packet counter
*
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS: When TX Counter Object changes from one status to anohter,
*                  the TX Counter will be cleared to 0 itself.
*******************************************************/
int RCP_SetMIBTxObject
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_MIB_OBJ_SEL(phyPort), TX_MIB_OBJ_Pos(phyPort), 2, data, flag)))
            return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current Diagnositic Counter Object status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 00 : Diag Byte counter
*                 01 : Diag Packet counter
*                 10 : CRC error packet counter
*                 11 : Collision packet counter
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetMIBDnObject
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_MIB_OBJ_SEL(phyPort), DN_MIB_OBJ_Pos(phyPort), 2, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current Diagnositic Counter Object status.
*
* INPUTS:
*       port - the logical port number.
*
*       data - 00 : Diag Byte counter
*                 01 : Diag Packet counter
*                 10 : CRC error packet counter
*                 11 : Collision packet counter
*
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS: When Diagnositic Counter Object changes from one status to 
*                  anohter,the Diagnositic Counter will be cleared to 0 itself.
*******************************************************/
int RCP_SetMIBDnObject
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_MIB_OBJ_SEL(phyPort), DN_MIB_OBJ_Pos(phyPort), 2, data, flag)))
            return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current RX Counter value.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS: When RX Counter Object changes from one status to anohter,
*                  the RX Counter will be cleared to 0 itself.
*******************************************************/
int RCP_GetMIBRxCounter
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned long  *data
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_Read_32bit_Reg(rrcp_dev, RT8326_MIB_RX_CNT(phyPort), data)))
        return ret;
    return RCP_OK;
    
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current TX Counter value.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS: When TX Counter Object changes from one status to anohter,
*                  the TX Counter will be cleared to 0 itself.
*******************************************************/
int RCP_GetMIBTxCounter
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned long  *data
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_Read_32bit_Reg(rrcp_dev, RT8326_MIB_TX_CNT(phyPort), data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current Diagnositic Counter value.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS: When Diagnositic Counter Object changes from one status to 
*                  anohter,the Diagnositic Counter will be cleared to 0 itself.
*******************************************************/
int RCP_GetMIBDiagCounter
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned long  *data
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

    if(RCP_OK != (ret = RCP_Read_32bit_Reg(rrcp_dev, RT8326_MIB_DN_CNT(phyPort), data)))
        return ret;
    return RCP_OK;
}
/******************************  port configiration ************************/

/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current Enalbe configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : port is abled(default)
*                 1 : port is disabled 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS: when disabled,the port will disable packet transmission and 
			receive except the Realtek Remote Control Packet;
*******************************************************/
int RCP_GetPortEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short *data
)
{
    unsigned short regAddr;
    unsigned long unit;
    unsigned long phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_DISABLE(phyPort);
	
	if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Port_Disable_Pos(phyPort), 1, data)))
        return ret;

    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current Enalbe configuration.
*
* INPUTS:
*       port - the logical port number.
*
*       data - 1 : set port disabled
*                 0 : set port enabled 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPortEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short data,
    IN  RCP_FLAG flag
)
{
    unsigned short regAddr;
    unsigned long unit;
    unsigned long phyPort;
    int ret;
    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_DISABLE(phyPort);

    if(RCP_OK != (ret = RCP_SetPhyPowerDown(rrcp_dev, slot, port, data)))
        return ret;
	
	if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, Port_Disable_Pos(phyPort), 1, data, flag)))
        return ret;

    
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current speed configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - <0-3>bits map to 10H,10F,100H,100F
*                1 : poirt with the speed capability.
*                0 : port withou the speed capability.
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortSpeedConfig
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short *data
)
{
    unsigned short regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;
    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
    	{
        	return ret;
    	}
    regAddr = RT8326_PORT_PROPERTY(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Media_Capability_Pos(phyPort), 4, data)))
    	{
        	return ret;
    	}
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current speed capability.
*
* INPUTS:
*       port - the logical port number.
*
*       data - <0-3>bits map to 10H,10F,100H,100F
*                1 : poirt with the speed capability.
*                0 : port withou the speed capability.
*     
*       flag - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPortSpeedConfig
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_PROPERTY(phyPort);

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, Media_Capability_Pos(phyPort), 4, 0, flag)))
        return ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, AutoNegotiation_Pos(phyPort), 1, 0, flag)))
        return ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, Media_Capability_Pos(phyPort), 4, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_SetPortMode100F
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  RCP_FLAG flag
)
{
    int ret;
  
    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyReAutoNego(rrcp_dev, slot, port)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 0)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhySpeed(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyDuplex(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhy100F(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPortSpeedConfig(rrcp_dev, slot, port,  0xf, flag)))
        return ret;
    return RCP_OK;
}
int RCP_SetPortMode100H
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  RCP_FLAG flag
)
{
    int ret;

    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyReAutoNego(rrcp_dev, slot, port)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 0)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhySpeed(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyDuplex(rrcp_dev, slot, port, 0)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhy100H(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPortSpeedConfig(rrcp_dev, slot, port,  0x7, flag)))
        return ret;
    return RCP_OK;
}
int RCP_SetPortMode10F
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  RCP_FLAG flag
)
{
    int ret;

    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyReAutoNego(rrcp_dev, slot, port)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 0)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhySpeed(rrcp_dev, slot, port, 0)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyDuplex(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhy10F(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPortSpeedConfig(rrcp_dev, slot, port,  0x3, flag)))
        return ret;
    return RCP_OK;
}
int RCP_SetPortMode10H
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  RCP_FLAG flag
)
{
    int ret;

    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyReAutoNego(rrcp_dev, slot, port)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 0)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhySpeed(rrcp_dev, slot, port, 0)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyDuplex(rrcp_dev, slot, port, 0)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhy10H(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPortSpeedConfig(rrcp_dev, slot, port,  0x1, flag)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current Auto Negotiation configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : auto negotiation configuration is  diabled
*                 1 : auto negotiation configuration is enabled
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortAutoNegoEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short *data
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_PROPERTY(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, AutoNegotiation_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current Auto Negotiation configuration.
*
* INPUTS:
*       port - the logical port number.
*
*       data - 0 : disable auto negotiation
*                 1 : enable auto negotiation
*
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPortAutoNegoEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short data,
    IN  RCP_FLAG flag
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;
    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_PROPERTY(phyPort);

    if(RCP_OK != (ret = RCP_SetPhyAutoNegoEnable(rrcp_dev, slot, port, 1)))
        return ret;
    if(RCP_OK != (ret = RCP_SetPhyReAutoNego(rrcp_dev, slot, port)))
        return ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, AutoNegotiation_Pos(phyPort), 1, data, flag)))
        return ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, Media_Capability_Pos(phyPort), 4, 0xf, flag)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current flow control configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : flow control configuration is disabled 
*                 1 : flow control configuration is enabled 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortPauseFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short *data
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_PROPERTY(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Pause_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current flow control configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : disable flow control 
*                 1 : enable flow control
*
*       flag - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPortPauseFC
(    
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short   data,
    IN  RCP_FLAG flag
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
    	{
        	return ret;
    	}
    regAddr = RT8326_PORT_PROPERTY(phyPort);

    if(RCP_OK != (ret = RCP_SetPhyReAutoNego(rrcp_dev, slot, port)))
    	{
     	   return ret;
    	}
    if(RCP_OK != (ret = RCP_SetPhyFC(rrcp_dev, slot, port, data)))
    	{
	        return ret;
    	}
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, Pause_Pos(phyPort), 1, data, flag)))
    	{
  	      return ret;
    	}
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current RX rate control configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : rate control configuration is disabled 
*                 1 : 128K
*                 2 : 256K
*                 3 : 512K
*                 4 : 1M
*                 5 : 2M
*                 6 : 4M
*                 7 : 8M
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortRxBandwidth
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT RCP_RX_RATE *data
)
{
    unsigned short  regAddr,rate;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_CTL(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, RX_Bandwidth_Pos(phyPort), 4, &rate)))
        return ret;
    else
        *data = (RCP_RX_RATE)rate;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current RX rate control configuration.
*
* INPUTS:
*       port - the logical port number.
*
*       data - 0 : rate control configuration is disabled 
*                 1 : 128K
*                 2 : 256K
*                 3 : 512K
*                 4 : 1M
*                 5 : 2M
*                 6 : 4M
*                 7 : 8M
*
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPortRxBandwidth
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  RCP_RX_RATE data,
    IN  RCP_FLAG flag
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_CTL(phyPort);

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, RX_Bandwidth_Pos(phyPort), 4, data, flag)))
        return ret;
    return RCP_OK;    
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's current TX rate control configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : rate control configuration is disabled 
*                 1 : 128K
*                 2 : 256K
*                 3 : 512K
*                 4 : 1M
*                 5 : 2M
*                 6 : 4M
*                 7 : 8M
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPortTxBandwidth
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT RCP_TX_RATE *data
)
{
    unsigned short  regAddr,rate;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_CTL(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, TX_Bandwidth_Pos(phyPort), 4, &rate)))
        return ret;
    else
        *data = (RCP_TX_RATE)rate;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current TX rate control configuration.
*
* INPUTS:
*       port - the logical port number.
*
*       data - 0 : rate control configuration is disabled 
*                 1 : 128K
*                 2 : 256K
*                 3 : 512K
*                 4 : 1M
*                 5 : 2M
*                 6 : 4M
*                 7 : 8M
*
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPortTxBandwidth
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  RCP_TX_RATE   data,
    IN  RCP_FLAG flag
)
{
    unsigned short  regAddr;
    unsigned long  unit;
    unsigned long  phyPort;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_CTL(phyPort);

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, TX_Bandwidth_Pos(phyPort), 4, data, flag)))
            return ret;
    return RCP_OK;    
}
/*int RCP_GetPortTrucking
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short *data
)
{
    
}
int RCP_SetPortTrucking
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{

}*/
/******************************* QoS configration **********************/
int RCP_GetQosDiffPriority
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, _DiffServ_Pri_Pos, 1, data)))
        return ret;
    return RCP_OK;    
}
int RCP_SetQosDiffPriority
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, _DiffServ_Pri_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetQosVlanPriority
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, _TagVlan_Pri_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetQosVlanPriority
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, _TagVlan_Pri_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetQosMapMode
(
    IN  RCP_DEV  *rrcp_dev,
    OUT  unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, 0, 2, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetQosMapMode
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, 0, 2, data, flag)))
        return ret;
    return RCP_OK;    
}
int RCP_GetQosAdapterFC
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, _FlowCtrl_Auto_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetQosAdapterFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, _FlowCtrl_Auto_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetQosPrioRatio
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, _HighLow_Ratio_Pos, 2, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetQosPrioRatio
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_QOS_CTL_BASE, _HighLow_Ratio_Pos, 2, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetQosPortPriority
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short  regAddr;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_PRI(phyPort);
	
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Port_Pri_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetQosPortPriority
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    unsigned long unit;
    unsigned long phyPort;
    unsigned short regAddr;
    int ret;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;    
    regAddr = RT8326_PORT_PRI(phyPort);
	 	
   if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, Port_Pri_Pos(phyPort), 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetQosPriorityCfg
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned long *data
)
{
    int ret;
	unsigned short tmpdata1, tmpdata2;
   if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_PORT_PRI_BASE, &tmpdata1)))
        return ret;
   if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_PORT_PRI_BASE + 1, &tmpdata2)))
        return ret;
	*data = (unsigned long)tmpdata2;
	*data <<=16;
	*data |= (unsigned long)tmpdata1;
    return RCP_OK;
}
/******************************* VLAN configration *********************/
/*******************************************************
*DESCRIPTION:
*       This routine get vlan Enable configuration.
*
* INPUTS:
*
* OUTPUTS:
*       data - 0 : vlan is disabled
*                 1 : vlan is enabled
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanEnable
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnVlan_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set vlan Enable configuration.
*
* INPUTS:
*
*       data - 0 : disable vlan
*                 1 : enable vlan
*
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnVlan_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}

int RCP_GetVlanUnicastLeaky
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnUCleaky_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetVlanUnicastLeaky
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnUCleaky_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetVlanMulticastLeaky
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnIPMleaky_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetVlanMulticastLeaky
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnIPMleaky_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetVlanArpLeaky
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnARPleaky_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetVlanArpLeaky
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnARPleaky_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetVlan8021Q
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _En8021Qaware_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetVlan8021Q
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _En8021Qaware_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetVlanPortIsolate
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned char *data
)
{
	*data = rrcp_dev->vlanPortIsolate;
	
    return RCP_OK;
}
int RCP_SetVlanPortIsolate
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned char  data
)
{
    rrcp_dev->vlanPortIsolate = data;
    return RCP_OK;
}
int RCP_GetVlan8021QTagOnly
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnIR_TagAdmit_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetVlan8021QTagOnly
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnIR_TagAdmit_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetVlanIRMember
(
    IN  RCP_DEV  *rrcp_dev,
    OUT  unsigned short *data
)
{
    int ret;

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnIR_Membset_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetVlanIRMember
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnIR_Membset_Pos, 1, data, flag)))
    	return ret;
    return RCP_OK;
}
int RCP_GetVlanIngressFilter
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnIR_Membset_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetVlanIngressFilter
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_VLAN_CTL_BASE, _EnIR_Membset_Pos, 1, data, flag)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the specific port's port-based vlan number.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - the vlan number which the specific port belongs to <0-31>
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanPortIDAssignment     
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_VLAN_IDX(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, VID_Index_Pos(phyPort), 5, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the specific port's port-based vlan number.
*
* INPUTS:
*       port - the logical port number.
*       data - the vlan number which the specific port belongs to <0-31>
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanPortIDAssignment
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_VLAN_IDX(phyPort);
    
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, VID_Index_Pos(phyPort), 5, data, flag)))
            return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the specific port's output tag process configuration
*
* INPUTS:
*       slot-
*       port-
*       vlanum- the specific vlan number <0-31>
* OUTPUTS:
*       data -  0 : remove vlan tag from a tagged frame
*                 1 : insert priority tag into an untagged high priority frame
*                 2 : insert priotity tag into an untagged low-priority frame
*                 3 : don't touch(don't modity the frame)(default value)
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanOutPortTag
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT  unsigned short  *data
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_VLAN_PRI(phyPort);

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, PriTagCtrl_Pos(phyPort), 2, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the specific port's output tag process configuration
*
* INPUTS:
*       slot-
*       port-
*       vlanum- the specific vlan number <0-31>
*       data -  0 : remove vlan tag from a tagged frame
*                 1 : insert priority tag into an untagged high priority frame
*                 2 : insert priotity tag into an untagged low-priority frame
*                 3 : don't touch(don't modity the frame)(default value)
*       flag - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanOutPortTag
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short  data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_VLAN_PRI(phyPort);    

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, PriTagCtrl_Pos(phyPort), 2, data, flag)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the specific vlan number's port list.
*
* INPUTS:
*       vlanum - the specific vlan number <0-31>
\* OUTPUTS:
*       data - <0-31>bit maps to <0-31> phy port.
*                 0 : port is not a member of the vlan number
*                 1 : port is a member of the vlan number
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanPort       
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  vlanum,
    OUT  unsigned long  *data
)
{
    int ret;
	unsigned short tmpdata1, tmpdata2;

    if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_VLAN_PORT_MEMBER(vlanum,0), &tmpdata1)))
        return ret;
	*data = tmpdata1;

    if(rrcp_dev->numOfPorts > 16)
	{
	    if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_VLAN_PORT_MEMBER(vlanum,1), &tmpdata2)))
	        return ret;
		*data = (unsigned long)tmpdata2;
		*data <<=16;
		*data |= (unsigned long)tmpdata1;
	}
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the specific vlan number's port list.
*
* INPUTS:
*       vlanum - the specific vlan number <0-31>
*       data - <0-31>bit maps to <0-31> phy port.
*                 0 : port is not a member of the vlan number
*                 1 : port is a member of the vlan number
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short vlanum,
    IN  unsigned long data,
    IN  RCP_FLAG flag
)
{
    int ret;
    unsigned short tmpdata1, tmpdata2;
    unsigned short eeAddr;
    unsigned char eeOffset;
    unsigned char eeLength;

    if(vlanum > 31)
        return RCP_BAD_VALUE;
    if(0x01 == (flag & RCP_CONFIG_REGISTER)) 
    {
		if(rrcp_dev->numOfPorts <= 16)
		{
			tmpdata1 = (unsigned short)data;
            if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, RT8326_VLAN_PORT_MEMBER(vlanum,0), tmpdata1)))
                 return ret;
		}
		else
		{
			tmpdata1 = (unsigned short)data;
			tmpdata2 = (unsigned short)(data>>16);
            if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, RT8326_VLAN_PORT_MEMBER(vlanum,0), tmpdata1)))
                 return ret;
            if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, RT8326_VLAN_PORT_MEMBER(vlanum,1), tmpdata2)))
                 return ret;
		}
    }
	
    if(0x02 == (flag & RCP_CONFIG_EEPROM))
    {
        if((data & 0xff00) >> 8 == 0)
        {
            if(RCP_OK != (ret = RCP_GetEeaddrFromReg(RT8326_VLAN_PORT_MEMBER(vlanum,0), 
                                       0, 8, &eeAddr, &eeOffset, &eeLength)))
                return ret;
            if(RCP_OK != (ret = RCP_SetEepromField(rrcp_dev, eeAddr, eeOffset, eeLength, (unsigned char)data)))
                return ret;
            if(RCP_OK != (ret = RCP_SetEepromValue(rrcp_dev, eeAddr+1, 0)))
                return ret;
        }
        else
        {
            if(RCP_OK != (ret = RCP_GetEeaddrFromReg(RT8326_VLAN_PORT_MEMBER(vlanum,MAX_RCP_SWITCH_TO_MANAGE), 
                                       8, 8, &eeAddr, &eeOffset, &eeLength)))
                return ret;
            if(RCP_OK != (ret = RCP_SetEepromValue(rrcp_dev, eeAddr-1, (unsigned char)(data & 0xff))))
                return ret;
            if(RCP_OK != (ret = RCP_SetEepromField(rrcp_dev, eeAddr, eeOffset, eeLength, (unsigned char)((data & 0xff00) >> 8))))
                return ret;
        }
    }

    rrcp_dev->vlanPort[vlanum] = data;
    return RCP_OK;
}

/*******************************************************
*DESCRIPTION:
*       This routine get the specific port wehter is a member of the vlanum vlan
*
* INPUTS:
*       slot-
*       port-
*       vlanum- the specific vlan number <0-31>
* OUTPUTS:
*       data -  0 : port is not a member of the vlan number
*                 1 : port is a member of the vlan number
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanPortExist
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    IN  unsigned short vlanum,
    OUT unsigned short *data
)
{
     int ret;
    unsigned long unit;
    unsigned long phyPort;
    unsigned long tmpdata;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    tmpdata = 0x1 << phyPort;
    if(1== ((tmpdata & rrcp_dev->vlanPort[vlanum]) >> phyPort))
    	*data = 1;
    else
    	*data = 0;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine add the specific port to vlanum vlan
*
* INPUTS:
*       slot-
*       port-
*       vlanum- the specific vlan number <0-31>
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanPortAdd
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short vlanum,
    IN  RCP_FLAG flag
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;
    unsigned long portlist,data;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    data = 0x1 << phyPort;
	
	if(RCP_OK != (ret = RCP_GetVlanPort(rrcp_dev, vlanum, &portlist)))
	        return ret;
	else
	{
	    portlist |= data;
	    if(RCP_OK != (ret = RCP_SetVlanPort(rrcp_dev, vlanum, portlist, flag)))
	        return ret;
	}
			
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine delete the specific port from vlanum vlan
*
* INPUTS:
*       slot-
*       port-
*       vlanum- the specific vlan number <0-31>
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanPortDelete
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    IN  unsigned short vlanum,
    IN  RCP_FLAG flag
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;
    unsigned long portlist,data;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
	data = 0x1 << phyPort;
	
	if(RCP_OK != (ret = RCP_GetVlanPort(rrcp_dev, vlanum, &portlist)))
	    return ret;
	else
	{
	    portlist = (portlist & (~data));
	    if(RCP_OK != (ret = RCP_SetVlanPort(rrcp_dev, vlanum, portlist, flag)))
	        return ret;
	}
	
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the specific vlan number's VID.
*
* INPUTS:
*       vlanum - the specific vlan number <0-31>
*
* OUTPUTS:
*       data - vid
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanVID      
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  vlanum,
    OUT  unsigned short  *data
)
{
    int ret;

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_VLAN_VID(vlanum, vlanum), _Vlan_VID_Pos, _Vlan_VID_Reg_Width, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the specific vlan number's VID.
*
* INPUTS:
*       vlanum - the specific vlan number <0-31>
*       data - vid
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanVID
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  vlanum,
    IN  unsigned short  data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    unsigned short eeAddr;
    unsigned char eeOffset;
    unsigned char eeLength;

    if(vlanum > 31)
        return RCP_BAD_VALUE;
    if(0x01 == (flag & RCP_CONFIG_REGISTER)) 
    {
        if(RCP_OK != (ret = RCP_SetRegisterField(rrcp_dev, RT8326_VLAN_VID(vlanum, vlanum), _Vlan_VID_Pos, _Vlan_VID_Reg_Width, data)))
            return ret;
    }
    if(0x02 == (flag & RCP_CONFIG_EEPROM))
    {
        if((data & 0xff00) >> 8 == 0)
        {
            if(RCP_OK != (ret = RCP_GetEeaddrFromReg(RT8326_VLAN_VID(vlanum, vlanum), 0, 8, &eeAddr, &eeOffset, &eeLength)))
                return ret;
            if(RCP_OK != (ret = RCP_SetEepromField(rrcp_dev, eeAddr, eeOffset, eeLength, (unsigned char)data)))
                return ret;
            if(RCP_OK != (ret = RCP_SetEepromValue(rrcp_dev, eeAddr+1, 0)))
                return ret;
        }
        else
        {
            if(RCP_OK != (ret = RCP_GetEeaddrFromReg(RT8326_VLAN_VID(vlanum, vlanum), 8, 8, &eeAddr, &eeOffset, &eeLength)))
                return ret;
            if(RCP_OK != (ret = RCP_SetEepromValue(rrcp_dev, eeAddr-1, (unsigned char)(data & 0xff))))
                return ret;
            if(RCP_OK != (ret = RCP_SetEepromField(rrcp_dev, eeAddr, eeOffset, eeLength, (unsigned char)((data & 0xff00) >> 8))))
                return ret;
        }
    }
    rrcp_dev->vlanVid[vlanum] = data;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the specific port's PVID.
*
* INPUTS:
*       slot -
*       port -
* OUTPUTS:
*       data - pvid
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanPVID(
    IN  RCP_DEV  *rrcp_dev, 
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short  *data
)
{
    int ret;
    unsigned short vlanum;

    if(RCP_OK != (ret = RCP_GetVlanPortIDAssignment(rrcp_dev, slot, port, &vlanum)))
        return ret;
    if(RCP_OK != (ret = RCP_GetVlanVID(rrcp_dev, vlanum, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the specific port's PVID.
*
* INPUTS:
*       slot -
*       port -
*       data - pvid
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanPVID(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    IN  unsigned short  data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    unsigned short vlanum;

    if(RCP_OK != (ret = RCP_GetVlanNumFromVID(rrcp_dev, data, &vlanum)))
        return ret;
    if(RCP_OK != (ret = RCP_SetVlanPortIDAssignment(rrcp_dev, slot, port, vlanum, flag)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesific port's Insert PVID status.
*
* INPUTS:
*       port - the specific logical port.
*
* OUTPUTS:
*       data - 0x1: Insert PVID is enabled.
*                 0x0: Insert PVID is disabled.
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanInsertPVID
(    
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_VLAN_PVID_CTL_BASE;

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, phyPort, _Vlan_PVID_CTL_Reg_Width, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesific port's Insert PVID.
*
* INPUTS:
*       port - the specific logical port.
*       data - 0x1: enable Insert PVID.
*                 0x0: disable Insert PVID.
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetVlanInsertPVID  
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    IN  unsigned short data,
    IN  RCP_FLAG flag
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_VLAN_PVID_CTL_BASE;

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, phyPort, _Vlan_PVID_CTL_Reg_Width, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_SetVlanInsert
(
	IN RCP_DEV * rrcp_dev, 
	IN unsigned short data, 
	IN RCP_FLAG flag
)
{
	int ret;
    unsigned short eeAddr;
    unsigned char eeOffset;
    unsigned char eeLength;
	if(0x01 == (flag & RCP_CONFIG_REGISTER)) 
	{
		if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, RT8326_VLAN_PVID_CTL_BASE, data)))
			return ret;
	}
	if(0x02 == (flag & RCP_CONFIG_EEPROM))
	{
		if(RCP_OK != (ret = RCP_GetEeaddrFromReg(RT8326_VLAN_PVID_CTL_BASE, 0, 8, &eeAddr, &eeOffset, &eeLength)))
                return ret;
		if((data & 0xff00) >> 8 == 0)
        	{
	            if(RCP_OK != (ret = RCP_SetEepromField(rrcp_dev, eeAddr, eeOffset, eeLength, (unsigned char)data)))
	                return ret;
	            if(RCP_OK != (ret = RCP_SetEepromValue(rrcp_dev, eeAddr+1, 0)))
	                return ret;	
        	}
        	else
        	{
	            if(RCP_OK != (ret = RCP_SetEepromValue(rrcp_dev, eeAddr-1, (unsigned char)(data & 0xff))))
	                return ret;
	            if(RCP_OK != (ret = RCP_SetEepromField(rrcp_dev, eeAddr, eeOffset, eeLength, (unsigned char)((data & 0xff00) >> 8))))
	                return ret;
        	}
	}
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine judge wether the specific vlan is existed.
*
* INPUTS:
*       vid - the specific vlan's vid.
*       totalvlanmum - the total number of existed vlan.
*
* OUTPUTS:
*       isExist - 0 : the specific vlan is not existed.
*                   1 : the specific vlan is existed.
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanExist
(
    IN  RCP_DEV *rrcp_dev,
    IN  unsigned short vid, 
    OUT unsigned short *isExist
)
{
    unsigned short i;
    
    for(i = 0; i < MAX_RCP_VLAN_NUM; i++)
    {
        if(rrcp_dev->vlanVid[i] == vid)
        {
		*isExist = 1;
		return RCP_OK;
        }
        else
        {
		*isExist = 0;
		continue;
        }
    }
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesific vlan entry's number in vlan table from VID.
*
* INPUTS:
*       vid - the specific vlan's vid.
*
* OUTPUTS:
*       vlanum - the specific vlan's vlan number <0-31>.
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanNumFromVID
(
    IN  RCP_DEV *rrcp_dev, 
    IN  unsigned short vid, 
    OUT unsigned short *vlanum
)
{
    unsigned short i;
    *vlanum = 0;
    
    for(i = 0; i < MAX_RCP_VLAN_NUM; i++)
    {
        if(rrcp_dev->vlanVid[i] == vid)
        {
		*vlanum = i;
		return RCP_OK;
        }
    }
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the idle vlan entry's number in vlan table.
*
* INPUTS:
*
* OUTPUTS:
*       idlenum - the idle vlan entry's number <0-31>.
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetVlanIdleNum
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short  *idlenum
)
{
    unsigned short num;
    
    for(num = 1; num < MAX_RCP_VLAN_NUM; num ++)
    {
        if(rrcp_dev->vlanVid[num] == 0)
        {
		*idlenum = num;
		return RCP_OK;
        }
        if((num == MAX_RCP_VLAN_NUM-1)&&(rrcp_dev->vlanVid[num]!=0))
        {
		*idlenum = 0;
		return RCP_OK;
        }
    }
    return RCP_OK;
}
int RCP_GetVlanExistNum
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short *totalnum
)
{
    unsigned short num;
    
    for(num = 0; num < MAX_RCP_VLAN_NUM; num++)
    {
        if(rrcp_dev->vlanVid[num] != 0)
        	(*totalnum)++;
        else
        	continue;
    }
    return RCP_OK;
}
int RCP_SetVlanMgtPortIsolate(RCP_DEV *pRcpDev, unsigned long mgtPort, RCP_FLAG flag)
{
	int ret;
	unsigned short vlanum;
	unsigned long port, unit, phyPort;
    unsigned long portlist, data, tmpPortList;

	/* VLAN Item of index = 0 used for mgtPort */
	if(RCP_OK != (ret = RCP_SetVlanVID(pRcpDev, 0, 0x1, flag)))
		return ret;

    if(RCP_OK != (ret = RCP_SetVlanPort(pRcpDev, 0, 0xffffffff, flag)))
	   return ret;	

	vlanum = 1;
	if(RCP_OK != (ret = pRcpDev->frcpLPort2Port(pRcpDev, 1, mgtPort, &unit, &phyPort)))
	        return ret;
    tmpPortList= 0x1 << phyPort;

	for(port = 1; port <= pRcpDev->numOfPorts; port++)
	{
		if(RCP_OK != (ret = RCP_SetVlanOutPortTag(pRcpDev, 1, port, 0, flag)))
			return ret;
		if(port == mgtPort) continue;
			
		if(RCP_OK != (ret = pRcpDev->frcpLPort2Port(pRcpDev, 1, port, &unit, &phyPort)))
		    return ret;
		
		portlist = 0x1 << phyPort;
	    data = tmpPortList | portlist;
		if(RCP_OK != (ret = RCP_SetVlanPort(pRcpDev, vlanum, data, flag)))
			return ret;				
		if(RCP_OK != (ret = RCP_SetVlanVID(pRcpDev, vlanum, vlanum+1, flag)))
				return ret;
		if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, port, vlanum+1, flag)))
				return ret;
		vlanum++;
	}		


	/* Clear not used */
	for(vlanum = pRcpDev->numOfPorts; vlanum < MAX_RCP_VLAN_NUM; vlanum++)
	{
		if(RCP_OK != (ret = RCP_SetVlanPort(pRcpDev, vlanum, 0, flag)))
			return ret;
		if(RCP_OK != (ret = RCP_SetVlanVID(pRcpDev, vlanum, 0, flag)))
			return ret;

	}

	if(RCP_OK != (ret = RCP_SetVlanPVID(pRcpDev, 1, mgtPort, 1, flag)))
		return ret;
	if(RCP_OK != (ret = RCP_SetVlanInsert(pRcpDev, 0, flag)))
		return ret;
	return RCP_OK;
}

/******************************* port mirror configration **************/
/*******************************************************
*DESCRIPTION:
*       This routine get the specific port's mirror configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : the specific port is not a Mirrored-Port
*                 1 : the specific port is a Mirrored-Port
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetMirrorPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_MIRROR(phyPort);
    
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, Port_Mirror_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
/*
* port mirror , need modify
*/

int RCP_GetMirrorDestPort
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned long  *data
)
{
    int ret;
    unsigned short tmpdata1, tmpdata2;
    
	if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_PORT_MIRROR_BASE, &tmpdata1)))
	    return ret;
	*data = (unsigned long)tmpdata1;
	
    if(rrcp_dev->numOfPorts > 16)
    {
		if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_PORT_MIRROR_BASE + 1, &tmpdata2)))
    	    return ret;
		*data = (unsigned long)tmpdata2;
		*data <<= 16;
		*data |= (unsigned long)tmpdata1;
    }
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the specific port's mirror configuration.
*
* INPUTS:
*       port - the logical port number.
*       data - 0 : set the specific port to be not a Mirrored-Port
*                 1 : set the specific port to be a Mirrored-port
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS: can't set more than one port to be the Mirrored-Port.when 
*                  there's a port has been setted to be a Mirrored-Port,other 
*                  ports couldn't set to be a Mirrored_Port once more.
*******************************************************/
int RCP_SetMirrorPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_PORT_MIRROR(phyPort);
    
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, Port_Mirror_Pos(phyPort), 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_SetMirrorDestPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  data
)
{
    int ret;
    unsigned short tmpdata1, tmpdata2; 
	
	tmpdata1 = (unsigned short) data;
	if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, RT8326_PORT_MIRROR_BASE, tmpdata1)))
	    return ret;
    if(rrcp_dev->numOfPorts > 16)
    {
		tmpdata2= (unsigned short) (data >>16);
		if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, RT8326_PORT_MIRROR_BASE + 1, tmpdata2)))
    	    return ret;
    }

    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the specific port's RX data mirror configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : the specific port's RX data is not mirrored to the Mirrored-Port
*                 1 : the specific port's RX data is mirrored to the Mirrored-Port
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetMirrorRxPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_RX_MIRROR(phyPort);
    
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, RX_Mirror_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_GetMirrorRxSrcPort
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned long  *data
)
{
    int ret;
    unsigned short tmpdata1, tmpdata2;
    
	if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_RX_MIRROR_BASE, &tmpdata1)))
	    return ret;
	*data = (unsigned long)tmpdata1;
    if(rrcp_dev->numOfPorts > 16)
    {
		if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_RX_MIRROR_BASE + 1, &tmpdata2)))
    	    return ret;
		*data = (unsigned long)tmpdata2;
		*data <<=16;
		*data |= (unsigned long)tmpdata1;
    }

    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the specific port's RX data mirror configuration.
*
* INPUTS:
*       port - the logical port number.
*       data - 0 : set the specific port's RX data not to the Mirrored-Port
*                 1 : set the specific port's RX data to the Mirrored-Port
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetMirrorRxPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_RX_MIRROR(phyPort);
    
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, RX_Mirror_Pos(phyPort), 1, data, flag)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the specific port's TX data mirror configuration.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0 : the specific port's TX data is not mirrored to the Mirrored-Port
*                 1 : the specific port's TX data is mirrored to the Mirrored-Port
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetMirrorTxPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    OUT unsigned short  *data
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_TX_MIRROR(phyPort);
    
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, TX_Mirror_Pos(phyPort), 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_GetMirrorTxSrcPort
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned long  *data
)
{
    int ret;
    unsigned short tmpdata1, tmpdata2;
    
	if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_TX_MIRROR_BASE, &tmpdata1)))
	    return ret;
	*data = (unsigned long)tmpdata1;
    if(rrcp_dev->numOfPorts > 16)
    {
		if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_TX_MIRROR_BASE + 1, &tmpdata2)))
    	    return ret;
		*data = (unsigned long)tmpdata2;
		*data <<=16;
		*data |= (unsigned long)tmpdata1;
    }

    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the specific port's TX data mirror configuration.
*
* INPUTS:
*       port - the logical port number.
*       data - 0 : set the specific port's TX data not to the Mirrored-Port
*                 1 : set the specific port's TX data to the Mirrored-Port
*       flag -
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetMirrorTxPort
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short  data,
    IN  RCP_FLAG flag
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_TX_MIRROR(phyPort);
    
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, TX_Mirror_Pos(phyPort), 1, data, flag)))
        return ret;
    return RCP_OK;
}
/******************************* PHY access ***********************/
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy register value.
*
* INPUTS:
*       port - the logical port number.
*       phyReg - phy register address.
* OUTPUTS:
*       data - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhyRegValue
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short phyReg,
    OUT unsigned short *data
)
{
    int ret;
    unsigned short phyAddr;
    unsigned long unit;
    unsigned long phyPort;
    unsigned short value = 0;
    unsigned short tmpdata;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;

	/*0x10 0x08 0x0 : rcpdev->phyaddr , for port phy address id*/
	if(phyPort <= 15)
        phyAddr = 0x10 | phyPort;   
	else if(phyPort >15 && phyPort <= 23)
		phyAddr = 0x08 | (phyPort-16);
	else 
		phyAddr = 0x00 | (phyPort-24);
	
    value = value | (phyAddr << 5) | phyReg;
    
    if(RCP_OK != (ret = RCP_SetRegisterField(rrcp_dev, RT8326_ACCESS_CTL, 0, 15, value)))
        return ret;
        
    do  
        if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_ACCESS_CTL, PHY_Busy_Pos, 1, &tmpdata)))
            return ret;
    while(tmpdata != 0);

    if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_ACCESS_READ, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's phy register value.
*
* INPUTS:
*       port - the logical port number.
*       phyReg - phy register address.
*       data - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhyRegValue
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short phyReg,
    IN  unsigned short data
)
{
    int ret;
    unsigned long unit;
    unsigned long phyPort;
    unsigned short phyAddr;
    unsigned short value = 0;
    unsigned short tmpData;

    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
	
	/*0x10 0x08 0x0 : rcpdev->phyaddr , for port phy address id*/
	if(phyPort <= 15)
        phyAddr = 0x10 | phyPort;   
	else if(phyPort >15 && phyPort <= 23)
		phyAddr = 0x08 | (phyPort-16);
	else 
		phyAddr = 0x00 | (phyPort-24);
	
    value = value | (0x1 << PHY_RW_Pos) | (phyAddr << 5) | phyReg;

    if(RCP_OK != (ret = RCP_Write_Reg(rrcp_dev, RT8326_ACCESS_WRITE, data)))
               return ret;
    do
    {
        if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_ACCESS_CTL, PHY_Busy_Pos, 1, &tmpData)))
            return ret;
    }while(tmpData != 0);
    if(RCP_OK != (ret = RCP_SetRegisterField(rrcp_dev, RT8326_ACCESS_CTL, 0, 15, value)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*       phyReg - phy register address.
*       offset - phy register address offset.
*       length - phy register address length.
*
* OUTPUTS:
*       data - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhyRegField
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short phyReg,
    IN  unsigned char offset,
    IN  unsigned char length,
    OUT unsigned short *data
)
{
    int ret;
    unsigned short tmpData;
    unsigned short mask;
    
    if(RCP_OK != (ret = RCP_GetPhyRegValue(rrcp_dev, slot, port, phyReg, &tmpData)))
        return ret;
        
    CALC_MASK(offset,length,mask);
    
    tmpData = (tmpData & mask) >> offset;
    *data = tmpData;
    
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*       phyReg - phy register address.
*       offset - phy register address offset.
*       length - phy register address length.
*       data - 
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhyRegField
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port,
    IN  unsigned short phyReg,
    IN  unsigned char offset,
    IN  unsigned char length,
    IN  unsigned short data
)
{
    int ret;
    unsigned short tmpData;
    unsigned short mask;
    
    if(RCP_OK != (ret = RCP_GetPhyRegValue(rrcp_dev, slot, port, phyReg, &tmpData)))
        return ret;
    CALC_MASK(offset,length,mask);
   
    tmpData &= (~mask);      
    tmpData |= ((data << offset) & mask);

    if(RCP_OK != (ret = RCP_SetPhyRegValue(rrcp_dev, slot, port, phyReg, tmpData)))
        return ret;
    
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : 100M
*                 0x0 : 10M
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhySpeed
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port, 
    OUT  unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 0, 13, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current speed configuration.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : 100M
*                 0x0 : 10M

*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhySpeed
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port, 
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 0, 13, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy AN status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : enable
*                 0x0 : disable
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhyAutoNegoEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long  slot,
    IN  unsigned long  port, 
    OUT  unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 0, 12, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's phy AN.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : enable
*                 0x0 : disable
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhyAutoNegoEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port, 
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 0, 12, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine reset the spesicfic port's phy.
*
* INPUTS:
*       port - the logical port number.
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhyReset
(
    IN  RCP_DEV *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port
)
{
    int ret;

    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 0, 15, 1, 1)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine restart the spesicfic port's phy AN.
*
* INPUTS:
*       port - the logical port number.
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhyReAutoNego
(
    IN  RCP_DEV *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port
)
{
    int ret;

    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 0, 9, 1, 1)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy power down status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : power is down
*                0x0 : power is up
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhyPowerDown
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 0, 11, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's current speed configuration.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : power down
*                 0x0 : power up
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhyPowerDown
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 0, 11, 1,data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy duplex status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : enable
*                 0x0 : disable
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhyDuplex
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 0, 8, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's phy duplex.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : enable
*                 0x0 : disable
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhyDuplex
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port, 
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 0, 8, 1,data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy flow control status.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : enable
*                 0x0 : disable
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhyFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 4, 10, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's phy flow control.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : enable
*                 0x0 : disable
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhyFC
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port, 
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 4, 10, 1,data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : with the capability of 10H
*                 0x0 : without the capabiltity of 10H
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhy10H
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 4, 5, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : with the capability of 10H
*                 0x0 : without the capabiltity of 10H
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhy10H
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port, 
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 4, 5, 1,data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : with the capability of 10F
*                 0x0 : without the capabiltity of 10F
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhy10F
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 4, 6, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine set the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : with the capability of 10F
*                 0x0 : without the capabiltity of 10F
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhy10F
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port, 
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 4, 6, 1,data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : with the capability of 100H
*                 0x0 : without the capabiltity of 100H
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhy100H
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 4, 7, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : with the capability of 100H
*                 0x0 : without the capabiltity of 100H
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhy100H
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port, 
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 4, 7, 1,data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       data - 0x1 : with the capability of 100F
*                 0x0 : without the capabiltity of 100F
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_GetPhy100F
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetPhyRegField(rrcp_dev, slot, port, 4, 8, 1, data)))
        return ret;
    return RCP_OK;
}
/*******************************************************
*DESCRIPTION:
*       This routine get the spesicfic port's phy speed and duplex capability.
*
* INPUTS:
*       port - the logical port number.
*       data - 0x1 : with the capability of 100F
*                 0x0 : without the capabiltity of 100F
*
* RETURNS:
*       GT_OK   - on success
*
* COMMENTS:
*******************************************************/
int RCP_SetPhy100F
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port, 
    IN  unsigned short data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetPhyRegField(rrcp_dev, slot, port, 4, 8, 1,data)))
        return ret;
    return RCP_OK;
}
/*******************************atu configiration*********************/
int RCP_GetAtuAgingEnable
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short *data
)
{
   int ret;

    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_ALT_CONFIG, _ALT_AGING_POS, 1, data)))
        return ret;
    return RCP_OK;
    
}
int RCP_SetAtuAgingEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret; 

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_ALT_CONFIG, _ALT_AGING_POS, 1, data, flag)))
            return ret;
    return RCP_OK;
}
int RCP_GetAtuFastAgingEnable
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_ALT_CONFIG, _ALT_FAST_AGING_POS, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetAtuFastAgingEnable
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short data,
    IN  RCP_FLAG flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_ALT_CONFIG, _ALT_FAST_AGING_POS, 1, data, flag)))
        return ret;
    return RCP_OK;
}

/*
* atu learnging , need modify
*/
int RCP_GetAtuLearning
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    OUT unsigned short *data
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;
    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_ALT_LEARN(phyPort);
	
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, regAddr, ALT_Learn_POS(phyPort), 1, data)))
        return ret;
    return RCP_OK;
    
}
int RCP_GetAtuLearningList
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_ALT_LEARN_BASE, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetAtuLearning
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned long slot,
    IN  unsigned long port,
    IN  unsigned short data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    unsigned long  unit;
    unsigned long  phyPort;
    unsigned short regAddr;
	
    if(RCP_OK != (ret = rrcp_dev->frcpLPort2Port(rrcp_dev, slot,port, &unit,&phyPort)))
        return ret;
    regAddr = RT8326_ALT_LEARN(phyPort);

    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, regAddr, ALT_Learn_POS(phyPort), 1, data, flag)))
        return ret;
    return RCP_OK;
} 
int RCP_GetAtuCtrlFrameFilter
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK!= (ret = RCP_GetRegField(rrcp_dev, RT8326_ALT_CONFIG, _ALT_CTRL_FILTER_POS, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_SetAtuCtrlFrameFilter
(
    IN  RCP_DEV  *rrcp_dev,
    IN  unsigned short  data,
    IN  RCP_FLAG  flag
)
{
    int ret;
    if(RCP_OK != (ret = RCP_SetRegField(rrcp_dev, RT8326_ALT_CONFIG, _ALT_CTRL_FILTER_POS, 1, data, flag)))
        return ret;
    return RCP_OK;
}
int RCP_GetTrunkFault
(
    IN  RCP_DEV  *rrcp_dev,
    OUT  unsigned short *data
)
{
    int ret;
    if(RCP_OK!= (ret = RCP_GetRegField(rrcp_dev, RT8326_SYS_FAULT_BASE, _Trunk_Fault_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_GetLoopFault
(
    IN  RCP_DEV  *rrcp_dev,
    OUT  unsigned short *data
)
{
    int ret;
    if(RCP_OK!= (ret = RCP_GetRegField(rrcp_dev, RT8326_SYS_FAULT_BASE, _Loop_Fault_Pos, 1, data)))
        return ret;
    return RCP_OK;
}
int RCP_GetFaultTrunkGroup
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned short *data
)
{
    int ret;
    if(RCP_OK != (ret = RCP_GetRegField(rrcp_dev, RT8326_SYS_FAULT_BASE, _FaultTkGroup_Pos, 6, data)))
        return ret;
    return RCP_OK;
}
/******************************* SNMP configration *****************/
void convertOidToChar(oid * oidMac,unsigned char *charMac)
{
	int i;
	for(i=0;i<RCP_MAC_SIZE;i++)
		*charMac++ = (unsigned char)*oidMac++;
}

void convertMacToOid(oid * oidMac,unsigned char *charMac)
{
	int i;
	for(i = 0; i < RCP_MAC_SIZE; i ++)
		*oidMac++ = (oid)*charMac++;
}
int RCP_GetDevIndexByMac(oid *mac, unsigned long *devId)
{
	int i,flag;
	unsigned char mac2[RCP_MAC_SIZE]={0};
	convertOidToChar(mac,mac2);
	flag = 0;
	for(i=1; i<MAX_RRCP_SWITCH_TO_MANAGE;i++)
	{
		if((RCP_Dev_Is_Valid(i)) && (0==memcmp(mac2,rcpDevList[i]->switchMac,RCP_MAC_SIZE)))
		{
			*devId = i;
			return RCP_OK;
		}
		else
			flag++;
	}
	if(flag == MAX_RRCP_SWITCH_TO_MANAGE)
		return RCP_ERROR;
	return RCP_OK;
}
int RCP_GetValidIndex(oid *mac, unsigned long *index)
{
	int i=0;
	int flag = 0;
	unsigned char mac2[RCP_MAC_SIZE];
	convertOidToChar(mac,mac2);
	for(i=1; i < MAX_RRCP_SWITCH_TO_MANAGE; i++)
	{
		if(RCP_Dev_Is_Valid(i))
		{
			if(0 == memcmp(mac2,rcpDevList[i]->switchMac,RCP_MAC_SIZE))
			{
				*index = i;
				return RCP_OK;
			}
			else
				flag ++;
		}
	}
	if(flag == 0)
		return RCP_ERROR;
	return RCP_OK;
}
int RCP_GetFirstIndex(unsigned long *index)
{
	int i;
	int flag = 0;
	for(i=1; i < MAX_RRCP_SWITCH_TO_MANAGE; i++)
	{
		if(RCP_Dev_Is_Valid(i))
		{
			*index = i;
			return RCP_OK;
		}
		else
			flag ++;
	}
	if(flag == MAX_RRCP_SWITCH_TO_MANAGE)
		return RCP_ERROR;
	return RCP_OK;
}
int RCP_GetNextIndex(unsigned long index, unsigned long *nextIndex)
{
	int i;
	int flag = 0;
	if(index == 0)
		return RCP_ERROR;
	for(i = index+1; i < MAX_RRCP_SWITCH_TO_MANAGE; i++)
	{
		if((RCP_Dev_Is_Valid(i)) && i != 0)
		{
			*nextIndex = i;
			return RCP_OK;
		}
		else
			flag ++;
	}
	if(flag == MAX_RRCP_SWITCH_TO_MANAGE - index -1)
		return RCP_ERROR;
	return RCP_OK;
}
#if 0
int RCP_GetPortList16SNMP(unsigned short portList, unsigned short *newList)
{
	int i;
	unsigned short tmp1, tmp2,tmp3;
	tmp1 = tmp2 = tmp3 = 0;
	for(i = 8; i > 0; i--)
	{
		tmp1 = (portList & (0x1 << (8-i))) << (2*i -1);
		tmp2 = (portList & (0x1 << (7+i))) >> (2*i -1);
		tmp3 = tmp3 | tmp1 |tmp2;
	}
	*newList = tmp3;
	return RCP_OK;
}
int RCP_GetPortListSNMP(RCP_DEV * rcpDev, unsigned short portList, unsigned short *newList)
{
	int i;
	unsigned char portList8;
	unsigned char newList8;
	unsigned short newList16,move1,move2;
	if(rcpDev->numOfPorts==16)
	{
	    RCP_GetPortList16SNMP(portList, &newList16);
        move1 = (newList16 & 0xff) << 8;
        move2 = newList16 >> 8;
	    *newList = move1 | move2 ;
	}
	else
	{
	    portList8 = (unsigned char) portList;
        RCP_GetPortList8SNMP(portList8, &newList8);
        *newList = 0x0000 | newList8;
        
	}
	
	return RCP_OK;
}
#endif
int RCP_GetPortList8SNMP(unsigned char portList, unsigned char *newList)
{
	int i;
	unsigned char tmp1, tmp2,tmp3;
	tmp1 = tmp2 = tmp3 = 0;
	for(i = 4; i > 0; i--)
	{
		tmp1 = (portList & (0x1 << (4-i))) << (2*i -1);
		tmp2 = (portList & (0x1 << (3+i))) >> (2*i -1);
		tmp3 = tmp3 | tmp1 |tmp2;
	}
	*newList = tmp3;
	return RCP_OK;
}
/* change the portlist  to  SNMP_PortList    */
int RCP_PortListToSNMP(RCP_DEV * rcpDev, unsigned long portList, unsigned char *newList)
{
	int i;
	unsigned long tmp1;
	for(i = 0; i < rcpDev->numOfPorts; i++)
	{
	    tmp1 = (portList & (0x1 << (i))) >> i;
	     
	    newList[(i/8)] = (tmp1 << (7- (i%8)))| newList[(i/8)];
	}
	
	return RCP_OK;
}

/* change the SNMP_portlist  to   PortList    */
int RCP_SNMPToPortList(RCP_DEV * rcpDev, unsigned long* portList, unsigned char *newList)
{
	int i;
	unsigned long tmp1,tmp2;

	tmp2 = 0;
    for(i = 0; i < rcpDev->numOfPorts; i++)
	{
	    tmp1 = (newList[(i/8)] & (0x1 << (7 -(i%8)))) >> (7 -(i%8));
	    tmp2 = (tmp1 << i) | tmp2;
	}
	*portList = tmp2;
    return RCP_OK;
}

int RCP_SetVlanPortIsolateEn
(
	RCP_DEV * rcpDev, 
	unsigned short data,
	unsigned long mgtPort,
	RCP_FLAG flag
)
{
	int ret;
	unsigned long port;
	unsigned short vlanum;
	
	if(data == 0)
	{
        if(RCP_OK != (ret = RCP_SetVlanVID(rcpDev, 0, 0x1, flag)))
		 	return ret;

	    if(RCP_OK != (ret = RCP_SetVlanPort(rcpDev, 0, 0xffffffff, flag)))
		   return ret;
		
		for(port = 1; port <= rcpDev->numOfPorts; port++)
		{
			if(RCP_OK != (ret = RCP_SetVlanInsertPVID(rcpDev, 1, port, 0, flag)))
				return ret;
			if(RCP_OK != (ret = RCP_SetVlanPVID(rcpDev, 1, port, 1, flag)))
				return ret;
		}
		for(vlanum = 1; vlanum < rcpDev->numOfPorts; vlanum++)
		{
			if(RCP_OK != (ret = RCP_SetVlanVID(rcpDev, vlanum, 0, flag)))
				return ret;
			if(RCP_OK != (ret = RCP_SetVlanPort(rcpDev, vlanum, 0, flag)))
				return ret;
		}
		if(RCP_OK != (ret = RCP_SetVlanPortIsolate(rcpDev, 0)))
			return ret;
	}
	else
	{
		if(RCP_OK != (ret = RCP_SetVlanMgtPortIsolate(rcpDev, mgtPort, flag)))
			return ret;
		if(RCP_OK != (ret = RCP_SetVlanPortIsolate(rcpDev, 1)))
		    return ret;
	}
	return RCP_OK;
}


int RCP_GetVlanUntagPortList
(
    RCP_DEV  *rrcp_dev,
    unsigned long  *portList
)
{
	/*Lbl ,2011,4.25*/
	
    int i, j, ret;
	unsigned short data[4];

	for(j = 0; j < (rrcp_dev->numOfPorts /_Vlan_Pri_Reg_Packed); j++)
    {
	    if(RCP_OK != (ret = RCP_Read_Reg(rrcp_dev, RT8326_VLAN_PRI_BASE+j, &data[j])))
	        return ret;
	    for(i = 0; i < _Vlan_Pri_Reg_Packed; i++)
		{
	        if(((data[j] & (0x3 << (2 * i ))) >> (2 * i)) == 0)
		    {
			    *portList |= 0x1 << (i + _Vlan_Pri_Reg_Packed * j);
		    }
		}
	}
    return RCP_OK;
} 
int RCP_SetVlanUntagPortList
(
    RCP_DEV  *rcpDev,
    unsigned short vlanum,
    unsigned long  *portList,
    unsigned long  port_len,
    RCP_FLAG flag
)
{
	unsigned short usVid, usExist, usVlan;
	unsigned long phyPort, slot, lPort, mgtPort;
	int ret;
	
	rcpDev->frcpPort2LPort(rcpDev, &slot, &mgtPort, 0, rcpDev->upLinkPort);
	if(RCP_OK != (ret = RCP_GetVlanVID(rcpDev, vlanum, &usVid)))
		return ret;
	for(phyPort= 0; phyPort < port_len; phyPort++)
	{
		if(0x1 == ((*portList >> phyPort) & 0x1))
		{
			rcpDev->frcpPort2LPort(rcpDev, &slot, &lPort, 0, phyPort);
			for(usVlan = 0; usVlan < MAX_RCP_VLAN_NUM; usVlan++)  
			{
				if(RCP_OK == (ret = RCP_GetVlanPortExist(rcpDev, 1, lPort, usVlan, &usExist)))
				{
					if(usExist == 1 && usVlan != vlanum)
						if(RCP_OK != (ret = RCP_SetVlanPortDelete(rcpDev, 1, lPort, usVlan, flag)))
							return ret;
				}
				else
					return ret;
			}
			if(RCP_OK != (ret = RCP_SetVlanPortAdd(rcpDev, 1, lPort, vlanum, flag)))
				return ret; 
			if(lPort != mgtPort)
			{
				if(RCP_OK != (ret = RCP_SetVlanPVID(rcpDev, 1, lPort, usVid, flag)))
					return ret; 
			}
			if(RCP_OK != (ret = RCP_SetVlanInsertPVID(rcpDev, 1, lPort, 0, flag)))
				return ret; 
			if(RCP_OK != (ret = RCP_SetVlanOutPortTag(rcpDev, 1, lPort, 0, flag)))
				return ret; 
		}
	}
	return RCP_OK;
}
int RCP_SetVlanTaggedPortList
(
    RCP_DEV  *rcpDev,
    unsigned short vlanum,
    unsigned long  *portList,
    unsigned long  port_len,
    RCP_FLAG flag
)
{
	unsigned long phyPort, slot, lPort;
	int ret;
	
	for(phyPort = 0; phyPort < port_len; phyPort++)
	{
		if(0x1 == ((*portList >> phyPort) & 0x1))
		{
			rcpDev->frcpPort2LPort(rcpDev, &slot, &lPort, 0, phyPort);
			if(RCP_OK != (ret = RCP_SetVlanInsertPVID(rcpDev, 1, lPort, 1, flag)))
				return ret; 
			if(RCP_OK != (ret = RCP_SetVlanPortAdd(rcpDev, 1, lPort, vlanum, flag)))
				return ret; 
			if(RCP_OK != (ret = RCP_SetVlanOutPortTag(rcpDev, 1, lPort, 3, flag)))
				return ret; 
		}
	}
	return RCP_OK;
}
int RCP_SetVlanPortListDelete
(
	RCP_DEV  *rcpDev,
	unsigned short vlanum,
	unsigned long  *portList,
	unsigned long  port_len,
	RCP_FLAG flag
)
{
	int ret;
	unsigned long phyPort, slot, ulPort;
	unsigned short output, tagnum, num;
	unsigned short existed, pvid, vid;
	
	if(RCP_OK != (ret = RCP_GetVlanVID(rcpDev, vlanum, &vid)))
		return ret;	
	for( phyPort = 0; phyPort < port_len; phyPort++)
	{
		if(0x1 == ((*portList >> phyPort) & 0x1))
		{
			rcpDev->frcpPort2LPort(rcpDev, &slot, &ulPort, 0, phyPort);
			if(RCP_OK != (ret = RCP_GetVlanOutPortTag(rcpDev, 1, ulPort, &output)))
				return ret;
			if(output == 0)
			{
				if(RCP_OK != (ret = RCP_SetVlanPVID(rcpDev, 1, ulPort, 1, flag)))
					return ret;							
				if(RCP_OK != (ret = RCP_SetVlanOutPortTag(rcpDev, 1, ulPort, 0, flag)))
					return ret;
				if(RCP_OK != (ret = RCP_SetVlanPortAdd(rcpDev, 1, ulPort, 0, flag)))
					return ret;
			}
			else
			{
				tagnum = 0;
				for(num = 0; num < MAX_RCP_VLAN_NUM; num++)
				{
					if(RCP_OK != (ret = RCP_GetVlanPortExist(rcpDev, 1, ulPort, num, &existed)))
						return ret;
					else
					{
						if(existed == 1)
							tagnum ++;
					}
				}
				if(RCP_OK != (ret = RCP_GetVlanPVID(rcpDev, 1, ulPort, &pvid)))
					return ret;
				if(((pvid == 1) && (tagnum == 2)) || (tagnum == 1))
				{
					if(ulPort != 1)
					{
						if(RCP_OK != (ret = RCP_SetVlanPVID(rcpDev, 1, ulPort, 1, flag)))
							return ret;							
						if(RCP_OK != (ret = RCP_SetVlanInsertPVID(rcpDev, 1, ulPort, 0, flag)))
							return ret;
						if(RCP_OK != (ret = RCP_SetVlanOutPortTag(rcpDev, 1, ulPort, 0, flag)))
							return ret;
						if(RCP_OK != (ret = RCP_SetVlanPortAdd(rcpDev, 1, ulPort, 0, flag)))
							return ret;
					}
					else
					{
						if(tagnum == 2)
						{
							RCP_GetVlanPortExist(rcpDev, 1, ulPort, 0, &existed);
							if(existed == 1)
							{
								if(RCP_OK != (ret = RCP_SetVlanInsertPVID(rcpDev, 1, ulPort, 0, flag)))
									return ret;
								if(RCP_OK != (ret = RCP_SetVlanOutPortTag(rcpDev, 1, ulPort, 0, flag)))
										return ret;
							}
						}
						if(tagnum == 1)
						{
							RCP_GetVlanPortExist(rcpDev, 1, ulPort, 0, &existed);
							if(existed == 0)
							{
								if(RCP_OK != (ret = RCP_SetVlanInsertPVID(rcpDev, 1, ulPort, 0, flag)))
									return ret;
								if(RCP_OK != (ret = RCP_SetVlanOutPortTag(rcpDev, 1, ulPort, 0, flag)))
									return ret;
								if(RCP_OK != (ret = RCP_SetVlanPortAdd(rcpDev, 1, ulPort, 0, flag)))
									return ret;
							}
						}
					}
				}
				if(pvid == vid)
				{
					if(RCP_OK != (ret = RCP_SetVlanPVID(rcpDev, 1, ulPort, 1, flag)))
						return ret;
				}
			}
			if(RCP_OK != (ret = RCP_SetVlanPortDelete(rcpDev, 1, ulPort, vlanum, flag)))
				return ret;
		}
	}
	return RCP_OK;
}
int RCP_SetVlanSave
(
	RCP_DEV *rcpDev,
	unsigned short vlanum
)
{
	int ret;
	unsigned short i, vid;
	unsigned char eeValue1, eeValue2;
	unsigned short data;
	unsigned long portlist;
	
	if(RCP_OK != (ret = RCP_GetVlanVID(rcpDev, vlanum, &vid)))
		return ret;
	if(RCP_OK != (ret = RCP_SetVlanVID(rcpDev, vlanum, vid, RCP_CONFIG_2_EEPROM)))
		return ret;
	for(i = 0; i < MAX_RCP_VLAN_NUM; i++)
	{
		if(RCP_OK != (ret = RCP_GetVlanPort(rcpDev, i, &portlist)))
			return ret;
		
		if(RCP_OK != (ret = RCP_SetVlanPort(rcpDev, i, portlist, RCP_CONFIG_2_EEPROM)))
			return ret;
	}
	if(RCP_OK != (ret = RCP_Read_Reg(rcpDev, RT8326_VLAN_PVID_CTL_BASE, &data)))
		return ret;
	eeValue1 = (unsigned char)(data & 0xff);
	eeValue2 = (unsigned char)(data &0xff00);
	if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0x17a, eeValue1)))
		return ret;
	if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0x17b, eeValue2)))
		return ret;
	for(i = 0x0; i < 0x2; i++)
	{	
		data = 0;
		if(RCP_OK != (ret = RCP_Read_Reg(rcpDev, RT8326_VLAN_PRI_BASE +i, &data)))
			return ret;
		eeValue1 = (unsigned char)(data & 0xff);
		eeValue2 = (unsigned char)((data >> 8) &0xff);
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0xb2+i*2, eeValue1)))
			return ret;
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0xb3+i*2, eeValue2)))
			return ret;
	}
	for(i = 0x0; i < 0x8; i++)
	{
		data = 0;
		if(RCP_OK != (ret = RCP_Read_Reg(rcpDev, 0x030c+i, &data)))
			return ret;
		eeValue1 = (unsigned char)(data & 0xff);
		eeValue2 = (unsigned char)((data >> 8) &0xff);
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0x98+i*2, eeValue1)))
			return ret;
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0x99+i*2, eeValue2)))
			return ret;
	}
	return RCP_OK;
}
int RCP_SetVlanEnableSave
(
	RCP_DEV *rcpDev,
	unsigned short enable
)
{
	int ret;
	unsigned short i;
	unsigned long slot, mgtPort;
	rcpDev->frcpPort2LPort(rcpDev, &slot, &mgtPort, 0, rcpDev->upLinkPort);
	if(enable == 1)
	{
		do{
			ret = RCP_SetVlanMgtPortIsolate(rcpDev, mgtPort, RCP_CONFIG_2_EEPROM);
		}while(ret != RCP_OK);
		for(i = 0x0; i < 0x8f; i++)
		{
			if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0xea+i, 0x0)))
				return ret;
		}
	}
	else
	{
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0xba, 0xff)))
			return ret;
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0xbe, 0x1)))
			return ret;
		for(i = 0x0; i < 0x30; i++)
		{
			if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0xc0+i, 0x0)))
				return ret;
		}	
	}
	return RCP_OK;
}
int RCP_SetVlanDelete
(
	RCP_DEV  *rcpDev,
	unsigned short vlanum,
	RCP_FLAG flag
)
{
	int ret;
	unsigned long  portlist;
	if(RCP_OK != (ret = RCP_GetVlanPort(rcpDev, vlanum, &portlist)))
		return ret;
	if(RCP_OK != (ret = RCP_SetVlanPortListDelete(rcpDev, vlanum, &portlist, rcpDev->numOfPorts, flag)))
		return ret;
	if(RCP_OK != (ret = RCP_SetVlanVID(rcpDev, vlanum, 0, flag)))
		return ret;
	return RCP_OK;
}
int RCP_GetQosPriPort
(
	RCP_DEV  *rcpDev,
	unsigned long *data
)
{
	int ret;
	unsigned short tmpdata1, tmpdata2;
	if(RCP_OK != (ret = RCP_Read_Reg(rcpDev, RT8326_PORT_PRI_BASE, &tmpdata1)))
		return ret;
	if(RCP_OK != (ret = RCP_Read_Reg(rcpDev, RT8326_PORT_PRI_BASE + 1, &tmpdata2)))
		return ret;
	*data = (unsigned long)tmpdata2;
	*data <<=16;
	*data |= (unsigned long)tmpdata1;
	return RCP_OK;
}
int RCP_SetConfigClear
(
	RCP_DEV  *rcpDev
)
{
	int ret;
#if 0
	unsigned short  eeAddr;
	unsigned char value;
	unsigned long slot, mgtPort;

	
	rcpDev->frcpPort2LPort(rcpDev, &slot, &mgtPort, 0, rcpDev->upLinkPort);
	if(RCP_OK != (ret = RCP_SetVlanPortIsolateEn(rcpDev, 1, mgtPort, RCP_CONFIG_2_BOTH)))
		return ret;
	for(eeAddr = 0x0; eeAddr < 0xb5; eeAddr++)
	{
		ret = RCP_OK;
		value = 0;
		if(RCP_OK != (ret = RCP_GetEepromValue(rcpDev, 0x210 + eeAddr, &value)))
			return ret;
		ret = RCP_OK;
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0x4 + eeAddr, value)))
			return ret;
	}

	if(rcpDev->deviceId==GH1516)
    {
	    for(eeAddr = 0x0; eeAddr < 0x60; eeAddr++)
	       {
		      if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0x11a + eeAddr, 0)))
		      return ret;
	       }
    }
    else
    {
       for(eeAddr = 0x0; eeAddr < 0x90; eeAddr++)
	      {
		     if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0xea + eeAddr, 0)))
		     { 
			   return ret;
		     }
	      }
  
    }
	
	if(RCP_OK != (ret = RCP_SetQosPortPriority(rcpDev, 1, mgtPort, 1, RCP_CONFIG_2_EEPROM)))
		return ret;
	if(RCP_OK != (ret = RCP_SysHWReset(rcpDev)))
		return ret;
	return RCP_OK;
#else
	if(RCP_OK != (ret = ereaseRcpDevCfgInFlash(rcpDev, 0) ))
		return ret;
	return RCP_SysHWReset(rcpDev);
#endif
}
int RCP_GetPortMauType
(
	RCP_DEV  *rcpDev,
	unsigned long slot, 
	unsigned long port,
	oid *mau,
	unsigned long *mau_len
)
{
#if 0
	int ret;
	unsigned short usSpeed,usValue;
	oid mau_oid[9] = {1, 3, 6, 1, 2, 1, 26, 4, 0};
	unsigned long oid_len = 8;
	
	if(RCP_OK != (ret = RCP_GetPortSpeed(rcpDev, 1, port, &usSpeed)))
	  	return ret;
	if(RCP_OK != (ret = RCP_GetPortDuplex(rcpDev, 1, port, &usValue)))
	  	return ret;

	if((usSpeed == 1) && (usValue == 1))
		mau_oid[oid_len] = dot3MauType100BaseTXFD;
	else if((usSpeed == 1) && (usValue == 0))
	  	mau_oid[oid_len] = dot3MauType100BaseTXHD;
	else if((usSpeed == 0) && (usValue == 1))
	  	mau_oid[oid_len] = dot3MauType10BaseTFD;
	else if((usSpeed == 0) && (usValue == 0))
	  	mau_oid[oid_len] = dot3MauType10BaseTHD;

	oid_len++;
	*mau_len = oid_len;
	memcpy(mau, mau_oid, sizeof(mau_oid));
#endif
	return RCP_OK;
}
int RCP_SetPortMauType
(
	RCP_DEV *dev,
	unsigned long slot,
	unsigned long port,
	unsigned short mau,
	unsigned char data,
	RCP_FLAG flag
)
{
#if 0
	int ret;
	if(data == 2)
	{
		switch(mau)
		{
			case dot3MauType10BaseTHD :
			    	 if(RCP_OK != (ret = RCP_SetPortMode10H(dev, slot, port, flag)))
			    	 	return ret;
			    	 break;
			case dot3MauType10BaseTFD :
			    	 if(RCP_OK != (ret = RCP_SetPortMode10F(dev, slot, port, flag)))
			    	 	return ret;
			    	 break;
			case dot3MauType100BaseTXHD :
			    	 if(RCP_OK != (ret = RCP_SetPortMode100H(dev, slot, port, flag)))
			    	 	return ret;
			    	 break;
			case dot3MauType100BaseTXFD :
			    	 if(RCP_OK != (ret = RCP_SetPortMode100F(dev, slot, port, flag)))
			    	 	return ret;
			    	 break;
			default :
				return RCP_ERROR;
		}
	}
	else
	{
		if(RCP_OK != (ret = RCP_SetPortAutoNegoEnable(dev, slot, port, 1, flag)))
			return ret;
	}
#endif
	return RCP_OK;
}
int RCP_SetVlan8021QEnable
(
	RCP_DEV  *rcpDev,
	unsigned short enable,
	RCP_FLAG flag
)
{
	int ret;
	unsigned short vlanum;
	unsigned long port;
	if(enable == 0)
	{
        if(RCP_OK != (ret = RCP_SetVlanVID(rcpDev, 0, 0x1, flag)))
		 	return ret;
        if(rcpDev->deviceId == GH1516)
	    {
		    if(RCP_OK != (ret = RCP_SetVlanPort(rcpDev, 0, 0xffff, flag)))
			   return ret;
	    }
	    else if(rcpDev->deviceId == GH1508)
	    {
		    if(RCP_OK != (ret = RCP_SetVlanPort(rcpDev, 0, 0xff, flag)))
			   return ret;
	    }
		else if(rcpDev->deviceId == GH1524)
		{
		    if(RCP_OK != (ret = RCP_SetVlanPort(rcpDev, 0, 0xffffff, flag)))
			   return ret;
		}
		else  /*GH1532 ?*/
		{
		    if(RCP_OK != (ret = RCP_SetVlanPort(rcpDev, 0, 0xffffffff, flag)))
			   return ret;
		}
		for(vlanum = 1; vlanum < MAX_RCP_VLAN_NUM; vlanum++)
		{
			if(RCP_OK != (ret = RCP_SetVlanVID(rcpDev, vlanum, 0, flag)))
				return ret;
			if(RCP_OK != (ret = RCP_SetVlanPort(rcpDev, vlanum, 0, flag)))
				return ret;
		}
		for(port = 1; port <= rcpDev->numOfPorts; port++)
		{
			if(RCP_OK != (ret = RCP_SetVlanPVID(rcpDev, 1, port, 1, flag)))
				return ret;
			if(RCP_OK != (ret = RCP_SetVlanOutPortTag(rcpDev, 1, port, 3, flag)))
				return ret;
		}
		if(RCP_OK != (ret = RCP_Write_Reg(rcpDev, RT8326_VLAN_PVID_CTL_BASE, 0)))
			return ret;
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0x17a, 0)))
			return ret;
		if(RCP_OK != (ret = RCP_SetEepromValue(rcpDev, 0x17b, 0)))
			return ret;
		if(RCP_OK != (ret = RCP_SetVlanPortIsolate(rcpDev, 0)))
			return ret;
	}
	else
	{
		for(port = 1; port <= rcpDev->numOfPorts; port++)
		{
			if(RCP_OK != (ret = RCP_SetVlanOutPortTag(rcpDev, 1, port, 0, flag)))
				return ret;
		}
	}
	if(RCP_OK != (ret = RCP_SetVlan8021Q(rcpDev, enable, flag)))
		return ret;
	return RCP_OK;
}
int Rcp_Get_MGT_Port(RCP_DEV *dev, unsigned long *slot, unsigned long *port)
{
	int ret;
	
	if(RCP_OK != (ret = dev->frcpPort2LPort(dev, slot, port, 0, dev->upLinkPort)))
		return ret;
	return RCP_OK ;
}
int sendOamRcpLpbDetectNotifyMsg(unsigned char onuPort, unsigned char rcpPort, unsigned char state, unsigned short uvid,unsigned char *session)

{
	int ret; 
	/*modified by wangxiaoyu 2008-06-06
	修正堆栈溢出操作错误，temp[4]-->temp[16]
	*/
	char temp[16]={0};

	*(temp) = ONU_PORT_LOOP_ALARM;
	*(temp+1) = 1;	/* 0: no switch. 1: has switch */
	*(temp+2) = rcpPort;
	*(temp+3) = 1;
	*(temp+4) = onuPort;
	*(temp+5) = state;
	*(temp+6) = uvid>>8;
	*(temp+7) = uvid&0xff;
	memcpy(&temp[8], rcpDevList[onuPort]->switchMac, RCP_MAC_SIZE);

	if(0 != (ret = CommOnuMsgSend(ALARM_REQ,0,temp,14, session)))
		diag_printf("Send ALARM_REQ for lpb detect of port %d trap failed.\r\n", onuPort);

	return ret;	
}

int pushOneSwitchStatusChgMsg(unsigned char onuPort, unsigned char* switchMac, unsigned char msgType, unsigned char reason)
{
	ALARM_FIFO_ENTRY *pMsg;
	
	if(NULL == gpstAlmFifo)
		return RCP_NOT_INITIALIZED;

	if(gpstAlmFifo->occupancy >= gpstAlmFifo->lengthLimitMax)
		return RCP_NO_RESOURCE;

	if(NULL == (pMsg = (ALARM_FIFO_ENTRY *)iros_malloc(IROS_MID_MALLOC, sizeof(ALARM_FIFO_ENTRY))))
		return RCP_NO_MEM;

	if(NULL == (pMsg->MsgBody = (unsigned char *)iros_malloc(IROS_MID_MALLOC, ONU_SWITCH_STATUS_CHANGE_ALARM_LEN)))
	{
		iros_free(pMsg);
		return RCP_NO_MEM;
	}

	pMsg->MsgBody[0] = ONU_SWITCH_STATUS_CHANGE_ALARM;
	/* Port ID */
	pMsg->MsgBody[1] = 0;
	pMsg->MsgBody[2] = 0;
	pMsg->MsgBody[3] = 1;
	pMsg->MsgBody[4] = onuPort;
	/* Alarm type: register/reregister/offline */
	pMsg->MsgBody[5] = msgType;
	/* Switch MAC */
	memcpy(&(pMsg->MsgBody[6]), switchMac, RCP_MAC_SIZE);
	/* Offline reason */
	pMsg->MsgBody[12] = reason;
	/* Reserved : no use */
	pMsg->MsgBody[13] = 0;

	pMsg->MsgLen = ONU_SWITCH_STATUS_CHANGE_ALARM_LEN;
	pMsg->pNext = gpstAlmFifo->top;
	pMsg->pPrevious = NULL;

	if(NULL != gpstAlmFifo->top)
	{
		(gpstAlmFifo->top)->pPrevious = pMsg;
	}
	else
	{
		gpstAlmFifo->bottom = pMsg;
	}

	gpstAlmFifo->top = pMsg;

	gpstAlmFifo->occupancy++;
	
	return RCP_OK;
}

int popAllSwitchStatusChgMsg(void)
{
	unsigned char oamSession[8];
	ALARM_FIFO_ENTRY *pMsg;
	
	if(NULL == gpstAlmFifo)
		return RCP_NOT_INITIALIZED;

	if(0 == gpstAlmFifo->occupancy)
		return RCP_NO_MORE;

	pMsg = gpstAlmFifo->bottom;
	if((NULL == pMsg) || (NULL == pMsg->MsgBody))
		return RCP_NOT_FOUND;

	memset(oamSession, 0, sizeof(oamSession));

	while(NULL != pMsg)
	{
		if(0 != CommOnuMsgSend(ALARM_REQ,0,pMsg->MsgBody,pMsg->MsgLen, oamSession))
		{
			printf("Send ALARM_REQ for switch change of port %d(%d) failed.\r\n", pMsg->MsgBody[4], pMsg->MsgBody[5]);
			return RCP_FAIL;
		}

		if(NULL != pMsg->pPrevious)
		{
			(pMsg->pPrevious)->pNext = NULL;
		}

		gpstAlmFifo->bottom = pMsg->pPrevious;
		iros_free(pMsg);
		pMsg = gpstAlmFifo->bottom;
		gpstAlmFifo->occupancy--;
	}

	gpstAlmFifo->top = NULL;
	
	return RCP_OK;
}
int saveAllRcpDevCfgToFlash(void)
{
	int i, j;
	unsigned short *pAddrBuf, *pDataBuf;
	unsigned long bufLenPerDev, fileLen;
	RCP_DEV *pRcpDev;
	unsigned short tmpVlanPortIsolate, tmpDeviceId;
	
	if(NULL == gpusSwitchCfgFileBuf)
	{
		return RCP_NOT_INITIALIZED;
	}

	/* Prepare address index map */
	pAddrBuf = gpusSwitchCfgFileBuf;
	bufLenPerDev = gusSwitchCfgArraySizeMaxInFlash * gusSwitchCfgArrayItemSize;
	memset(pAddrBuf, 0, bufLenPerDev);
	memcpy(pAddrBuf, gusSwitchCfgRegAddrArray, (unsigned long)(gusSwitchCfgArraySize * gusSwitchCfgArrayItemSize));
	pAddrBuf[gusSwitchCfgArraySizeMax-1] = gusSwitchCfgArraySize;
	pAddrBuf[gusSwitchCfgArraySizeMax] = gusSwitchDataValidFlag;

	for(i=1; i<MAX_RRCP_SWITCH_TO_MANAGE; i++)
	{
		if(RCP_Dev_Is_Exist(i))
		{
			pRcpDev = RCP_Get_Dev_Ptr(i);
			pDataBuf = gpusSwitchCfgFileBuf + gusSwitchCfgArraySizeMaxInFlash * i;
			memset(pDataBuf, 0, bufLenPerDev);
			pDataBuf[gusSwitchCfgArraySizeMax] = gusSwitchDataValidFlag;
			for(j=0; j<gusSwitchCfgArraySize; j++)
			{
				if(RCP_OK != RCP_Read_Reg(pRcpDev, pAddrBuf[j], pDataBuf+j))
				{
					pDataBuf[gusSwitchCfgArraySizeMax] = 0;
				}
			}
			tmpVlanPortIsolate = (pRcpDev->vlanPortIsolate)? 1 : 0;
			tmpDeviceId = pRcpDev->deviceId;
			tmpDeviceId <<= 8;
			pDataBuf[gusSwitchCfgArraySizeMax-1] = tmpVlanPortIsolate | tmpDeviceId;
		}
	}

	fileLen = glSwitchCfgFileSizeMaxInFlash;
	return device_conf_write_switch_conf_to_flash((char *)gpusSwitchCfgFileBuf, fileLen);
}

int saveOneRcpDevCfgToFlash(RCP_DEV  *rcpDev)
{
	int j;
	unsigned short *pAddrBuf, *pDataBuf;
	unsigned long bufLenPerDev, fileLen;
	unsigned short tmpVlanPortIsolate, tmpDeviceId;
	
	if(NULL == gpusSwitchCfgFileBuf)
	{
		return RCP_NOT_INITIALIZED;
	}

	/* Prepare address index map */
	pDataBuf = pAddrBuf = gpusSwitchCfgFileBuf;
	bufLenPerDev = gusSwitchCfgArraySizeMaxInFlash * gusSwitchCfgArrayItemSize;
	memset(pAddrBuf, 0, bufLenPerDev);
	memcpy(pAddrBuf, gusSwitchCfgRegAddrArray, (unsigned long)(gusSwitchCfgArraySize * gusSwitchCfgArrayItemSize));
	pAddrBuf[gusSwitchCfgArraySizeMax-1] = gusSwitchCfgArraySize;
	pAddrBuf[gusSwitchCfgArraySizeMax] = gusSwitchDataValidFlag;

	if(RCP_Dev_Is_Exist(rcpDev->parentPort))
	{
		pDataBuf = gpusSwitchCfgFileBuf + gusSwitchCfgArraySizeMaxInFlash * (rcpDev->parentPort);
		memset(pDataBuf, 0, bufLenPerDev);
		pDataBuf[gusSwitchCfgArraySizeMax] = gusSwitchDataValidFlag;
		for(j=0; j<gusSwitchCfgArraySize; j++)
		{
			if(RCP_OK != RCP_Read_Reg(rcpDev, pAddrBuf[j], pDataBuf+j))
			{
				pDataBuf[gusSwitchCfgArraySizeMax] = 0;
			}
		}
		tmpVlanPortIsolate = (rcpDev->vlanPortIsolate)? 1 : 0;
		tmpDeviceId = rcpDev->deviceId;
		tmpDeviceId <<= 8;
		pDataBuf[gusSwitchCfgArraySizeMax-1] = tmpVlanPortIsolate | tmpDeviceId;
	}

	fileLen = glSwitchCfgFileSizeMaxInFlash;
	return device_conf_write_switch_conf_to_flash((char *)gpusSwitchCfgFileBuf, fileLen);
}

int restoreRcpDevCfgFromFlash(RCP_DEV  *rcpDev)
{
	unsigned short *pAddrBuf, *pDataBuf, arraySize, deviceIdInFlash;
	int j, iRet;

	iRet = RCP_OK;
	if(gpusSwitchCfgFileBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
	{
		pAddrBuf = gpusSwitchCfgFileBuf;
		pDataBuf = gpusSwitchCfgFileBuf + gusSwitchCfgArraySizeMaxInFlash * (rcpDev->parentPort);
		arraySize = gpusSwitchCfgFileBuf[gusSwitchCfgArraySizeMax - 1];
		
		if(pDataBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
		{
			deviceIdInFlash = (pDataBuf[gusSwitchCfgArraySizeMax-1]) >> 8;
			if(deviceIdInFlash == rcpDev->deviceId)
			{
				for(j=0; j<arraySize; j++)
				{
					if(RCP_OK != (iRet = RCP_Write_Reg(rcpDev, pAddrBuf[j], pDataBuf[j])))
					{
						break;
					}
				}
				rcpDev->vlanPortIsolate = (pDataBuf[gusSwitchCfgArraySizeMax-1]&0x0001)? 1 : 0;
			}
			else
			{
				iRet = RCP_GetVlanPortIsolate_InEeprom(rcpDev, &(rcpDev->vlanPortIsolate));
			}
		}
		else
		{
			iRet = RCP_GetVlanPortIsolate_InEeprom(rcpDev, &(rcpDev->vlanPortIsolate));
		}
	}
	else
	{
		iRet = RCP_GetVlanPortIsolate_InEeprom(rcpDev, &(rcpDev->vlanPortIsolate));
	}

	return iRet;
}

int ereaseRcpDevCfgInFlash(RCP_DEV  *rcpDev, int allDev)
{
	unsigned short *pDataBuf;
	unsigned long  fileLen;

	if(1 == allDev)
	{
	//	device_conf_erase_switch_conf_from_flash();
		memset(gpusSwitchCfgFileBuf, 0, glSwitchCfgFileSizeMaxInFlash);
	}
	else
	{
		pDataBuf = gpusSwitchCfgFileBuf + gusSwitchCfgArraySizeMaxInFlash * (rcpDev->parentPort);
		if(pDataBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
		{
			pDataBuf[gusSwitchCfgArraySizeMax] = 0;
			fileLen = glSwitchCfgFileSizeMaxInFlash;
			return device_conf_write_switch_conf_to_flash((char *)gpusSwitchCfgFileBuf, fileLen);
		}
	}

	return RCP_OK;
}

int IS_Flash_Data_Valid(RCP_DEV  *rcpDev)
{
	unsigned short *pDataBuf;

	pDataBuf = gpusSwitchCfgFileBuf + gusSwitchCfgArraySizeMaxInFlash * (rcpDev->parentPort);
	return (pDataBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)? 1 : 0;
}

int getRegValueInFlash(RCP_DEV  *rcpDev, unsigned short usRegAddr, unsigned short *pRegValue)
{
	unsigned short *pAddrBuf, *pDataBuf, usLen, i, usFound;
	
	if(gpusSwitchCfgFileBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
	{
		pAddrBuf = gpusSwitchCfgFileBuf;
		usLen = gpusSwitchCfgFileBuf[gusSwitchCfgArraySizeMax - 1];
	}
	else
	{
		pAddrBuf = gusSwitchCfgRegAddrArray;
		usLen = gusSwitchCfgArraySize;
	}

	pDataBuf = gpusSwitchCfgFileBuf + gusSwitchCfgArraySizeMaxInFlash * (rcpDev->parentPort);

	if(pDataBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
	{
		usFound = 0;
		for(i=0; i<usLen; i++)
		{
			if(pAddrBuf[i] == usRegAddr)
			{
				*pRegValue = pDataBuf[i];
				return RCP_OK;
			}
		}

		return RCP_NOT_FOUND;
	}
	else
	{
		return RCP_NOT_INITIALIZED;
	}
}

int RCP_GetVlanPortIsolate_InFlash
(
    IN  RCP_DEV  *rcpDev,
    OUT unsigned char *data
)
{
    int iRet;
	unsigned short *pDataBuf;

	iRet = RCP_OK;
	if(gpusSwitchCfgFileBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
	{
		pDataBuf = gpusSwitchCfgFileBuf + gusSwitchCfgArraySizeMaxInFlash * (rcpDev->parentPort);
		
		if(pDataBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
		{
			*data = (pDataBuf[gusSwitchCfgArraySizeMax-1]&0x0001)? 1 : 0;
		}
		else
		{
			iRet = RCP_NOT_INITIALIZED;
		}
	}
	else
	{
		iRet = RCP_NOT_INITIALIZED;
	}
    return iRet;
}

int RCP_GetVlanPortIsolate_InEeprom
(
    IN  RCP_DEV  *rrcp_dev,
    OUT unsigned char *data
)
{
    int ret;

    if(RCP_OK != (ret = RCP_GetEepromField(rrcp_dev, 0x28, 7, 1, data)))
    {
		return ret;
    }
    return RCP_OK;
}

int RCP_GetDeviceID_InFlash
(
    IN  RCP_DEV  *rcpDev,
    OUT unsigned short *data
)
{
    int iRet;
	unsigned short *pDataBuf, deviceIdInFlash;

	iRet = RCP_OK;
	if(gpusSwitchCfgFileBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
	{
		pDataBuf = gpusSwitchCfgFileBuf + gusSwitchCfgArraySizeMaxInFlash * (rcpDev->parentPort);
		
		if(pDataBuf[gusSwitchCfgArraySizeMax] == gusSwitchDataValidFlag)
		{
			deviceIdInFlash = (pDataBuf[gusSwitchCfgArraySizeMax-1]) >> 8;
			*data = deviceIdInFlash;
		}
		else
		{
			iRet = RCP_NOT_INITIALIZED;
		}
	}
	else
	{
		iRet = RCP_NOT_INITIALIZED;
	}
    return iRet;
}

RCP_DEV *RCP_Get_Dev_Ptr_For_Flash(unsigned long parentPort)
{
	unsigned short data;
//	rtk_pri_t priority;
	cs_callback_context_t context;
	if(NULL == rcpDevList[0])
	{
		if(NULL == (rcpDevList[0] = (RCP_DEV *)iros_malloc(IROS_MID_MALLOC, sizeof(RCP_DEV))))
			return NULL;
		memset(rcpDevList[0], 0, sizeof(RCP_DEV));
		/* Init access function */
		if(vlan_dot_1q_enable == 1)
		epon_onu_sw_get_port_pvid(context, 0, 0, parentPort, &(rcpDevList[0]->mgtVid));

		else
			rcpDevList[0]->mgtVid = 0;
		rcpDevList[0]->frcpReadReg = RCP_Read_Reg;
		rcpDevList[0]->frcpWriteReg = RCP_Write_Reg;

		rcpDevList[0]->semCreate = cyg_semaphore_init;
		rcpDevList[0]->semDelete = cyg_semaphore_destroy;
		rcpDevList[0]->semTake = cyg_semaphore_timed_wait;
		rcpDevList[0]->semGive = cyg_semaphore_post;
		rcpDevList[0]->frcpLPort2Port = RCPLPort2Port;
		rcpDevList[0]->frcpPort2LPort = RCPPort2LPort;
	}

	rcpDevList[0]->parentPort = (unsigned char)parentPort;
	rcpDevList[0]->paSlot = 1;
	rcpDevList[0]->paPort = parentPort;

	rcpDevList[0]->deviceId = GH1508;
	rcpDevList[0]->numOfPorts = 8;

	if(RCP_OK == RCP_GetDeviceID_InFlash(rcpDevList[0], &data))
	{
		if(GH1516 == data)
		{
			rcpDevList[0]->deviceId = GH1516;
			rcpDevList[0]->numOfPorts = 16;
		}
		if(GH1524 == data)
		{
			rcpDevList[0]->deviceId = GH1524;
			rcpDevList[0]->numOfPorts = 24;
		}
		if(GH1532 == data)
		{
			rcpDevList[0]->deviceId = GH1532;
			rcpDevList[0]->numOfPorts = 32;
		}
	}
	
	return rcpDevList[0];
}
/*-------------------------------------------------------------------------------------------------------------
PAI描述:对RCP包的处理，ETH_TYPE_RRCP的类型是8899,判断RCP的合法性，通过某成员状态，
来调用RCP_Devlist_update();
---------------------------------------------------------------------------------------------------------------*/
cs_int32 rrcp_packet_handler(cs_uint32 srcPort, cs_uint32 len, cs_uint8 *packet)
{
	cs_int32 iRet = RCP_OK;
	enet_format_t *p_stEnetParse;
    cs_uint8 oui[3] = {0x00, 0x0f, 0xe9};
	
	if(NULL == packet)
		return RCP_BAD_VALUE;

	p_stEnetParse = (enet_format_t *) packet;
    if(gulGwdRcpAuth)
    {
        if(memcmp(packet + RCP_MAC_SIZE, oui, 3) != 0)
            return RCP_ILLEGAL_AUTHKEY;
    }
	if(ntohs(p_stEnetParse->en_pro_II) != ETH_TYPE_RRCP)
		return RCP_BAD_VALUE;

	switch(ntohs(p_stEnetParse->en_ver))
	{
		case REALTEK_RRCP_HELLO_REPLY:
			iRet = RCP_DevList_Update(srcPort, packet);
		    break;
	  	case REALTEK_RRCP_GET_REPLY:
			iRet = RCP_RegList_Update(srcPort, packet);
			break;
		default:
			iRet = RCP_NOT_SUPPORTED;
    		RCP_DEBUG(("\r\n unknown RCP packet received!\r\n "));
	     	break;
	}

	return iRet;
}
/*---------------------------------------------------------------------------------------------------------
API描述:Rcpframerevhandle();判断RCP帧时候有TIGGED，如果有那么不做处理，没有加上，
帧长度改变。
-----------------------------------------------------------------------------------------------------------*/
epon_onu_frame_handle_result_t RcpFrameRevHandle(cs_uint32 portid ,cs_uint32  len, cs_uint8 *frame)
{
	cs_uint32 newLength;
	cs_int32 iRet;
	
    if ((len<16) || (frame==NULL) || (len >= (RCP_PKT_MAX_LENGTH-4)))
    {
        return EPON_ONU_FRAME_NOT_HANDLED;
    }

    RCP_DEBUG(("\r\n------------------------------------ "));
    DUMPRCPPKT("\r\nRcpFrameRcv : ", portid, frame, len);
    RCP_DEBUG(("\r\n------------------------------------ "));
    
	/* Tagged or not? */
	if((frame[12]==0x81) && (frame[13]==0))
	{
		iRet = rrcp_packet_handler(portid, len, frame);
	}
	else
	{
		memset(gucRxRcpPktBuf, 0, RCP_PKT_MAX_LENGTH);
		memcpy(gucRxRcpPktBuf, frame, RCP_MAC_SIZE+RCP_MAC_SIZE);
		gucRxRcpPktBuf[12] = 0x81;
		gucRxRcpPktBuf[13] = 0;
		gucRxRcpPktBuf[14] = 0;
		gucRxRcpPktBuf[15] = 0x01;
		memcpy(gucRxRcpPktBuf+16, frame+12, len-12);
		newLength = len + 4;
		iRet = rrcp_packet_handler(portid, newLength, gucRxRcpPktBuf);
	}

    if(0 == iRet)
    {
        return EPON_ONU_FRAME_HANDLED;
    }
    else
    {
    	RCP_DEBUG(("\r\n rrcp_packet_handler failed!(%d)\r\n ", iRet));
        return EPON_ONU_FRAME_NOT_HANDLED;
    }
}

int Onu_Rcp_Detect_Set_FDB(unsigned char  opr)
{
        int iRet = EPON_RETURN_SUCCESS; 
		int i = 0;
    	cs_mac_t cMac;
		cs_uint8 switch_chip_id=0;
        cs_aal_fdb_entry_t fdb_entry;
		cs_callback_context_t     context;
        //rtk_chip_id_t sw_chiptype ;
		startup_config_read(CFG_ID_SWITCH_CHIP_ID, 1, &switch_chip_id);
        if (switch_chip_id && (switch_chip_id != 1))
        {
               return 1;
        }
        RCP_DEBUG(("\r\nOnu_RCP_Detect_Set_FDB func sw_chiptype is : 0x%x\r\n", switch_chip_id));
     
        if (opr)
        {
            /*add fdb*/
            memset(&fdb_entry, 0, sizeof(fdb_entry));
			//startup_config_read(CFG_ID_MAC_ADDRESS, CS_MACADDR_LEN, cMac);
			epon_request_onu_pon_mac_addr_get(context, 0, 0, &cMac);
            memcpy(&fdb_entry.mac, cMac.addr, 6);
			    for (i=0; i<64; i++)
   				 {
       				 if (aal_fdb_entry_set(i, &fdb_entry) != CS_E_OK)
       					 {    
           					 cs_halt("fdb entry init FAILED\n");
        				 }
    			 }
		//	aal_fdb_entry_set()
			#if 0
            fdb_entry.status = FDB_ENTRY_MGMT;
            
            if (sw_chiptype == EPON_SWITCH_6045)
                fdb_entry.egress_portmap = 0x10;
            
            else if (sw_chiptype == EPON_SWITCH_6046)
                fdb_entry.egress_portmap = 0x400;
            
            iRet = epon_onu_sw_add_fdb_entry(&fdb_entry);
            if (iRet != EPON_RETURN_SUCCESS)
            {
                RCP_DEBUG(("\r\nOnu_Rcp_Detect_Set_FDB func fdb add error."));
            }
			#endif
        }

        return iRet;
}

int device_conf_write_switch_conf_to_flash(char * buf, long int length)
{
	unsigned char *tempBuff = NULL;
	unsigned char *pConfig = NULL;
    int ret=0;

    if(buf == NULL || length > FLASH_GWD_RCG_SWITCH_CFG_MAX_SIZE || length < 0)
    {
    	ret = GWD_RETURN_ERR;
    	goto END;
    }

    tempBuff = iros_malloc(IROS_MID_APP, FLASH_USER_DATA_MAX_SIZE);
	if(tempBuff == NULL) {
       diag_printf("Config save failed\n");
       ret= GWD_RETURN_ERR;
	   goto END;
	}

	memset(tempBuff, 0x00, FLASH_USER_DATA_MAX_SIZE);
	user_data_config_Read(0,tempBuff, FLASH_USER_DATA_MAX_SIZE);
	pConfig = (unsigned char *)(tempBuff+FLASH_GWD_RCG_SWITCH_CFG_OFFSET);
	memcpy(pConfig,buf,length);

	user_data_config_Write(tempBuff, FLASH_USER_DATA_MAX_SIZE);
	if (tempBuff !=NULL ) {
	    iros_free(tempBuff);
	}

#if 0
	diag_printf("rcp switch cfg save ok!\r\n");
	{
		int i = 0;
		for(i=0; i<length; i++)
		{
			if(!(i&0xf))
				diag_printf("\r\n");
			diag_printf("%02x ", (int)buf[i]);
		}
	}
#endif

END:
    return ret;
}

int device_conf_read_switch_conf_from_flash(char * buf, long int *length)
{
	unsigned char *tempBuff = NULL;
	unsigned char *pConfig = NULL;
    int ret=0;

    if(buf == NULL || length == NULL ||
    		(length != NULL && (*length < 0 || *length > (FLASH_GWD_RCG_SWITCH_CFG_MAX_SIZE))))
    {
    	ret = GWD_RETURN_ERR;
    	goto END;
    }

    tempBuff = iros_malloc(IROS_MID_APP, FLASH_USER_DATA_MAX_SIZE);
	if(tempBuff == NULL) {
       diag_printf("Config save failed\n");
       ret= GWD_RETURN_ERR;
	   goto END;
	}

	memset(tempBuff, 0x00, FLASH_USER_DATA_MAX_SIZE);
	user_data_config_Read(0,tempBuff, FLASH_USER_DATA_MAX_SIZE);
	pConfig = (unsigned char *)(tempBuff+FLASH_GWD_RCG_SWITCH_CFG_OFFSET);

	memcpy(buf, pConfig, *length);

	if (tempBuff !=NULL ) {
	    iros_free(tempBuff);
	}

#if 0
	diag_printf("rcp switch cfg read ok!\r\n");
	{
		int i = 0;
		for(i=0; i<*length; i++)
		{
			if(!(i&0xf))
				diag_printf("\r\n");
			diag_printf("%02x ", (int)buf[i]);
		}
	}
#endif

END:
    return ret;
}

