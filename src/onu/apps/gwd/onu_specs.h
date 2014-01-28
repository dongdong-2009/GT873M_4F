#ifndef __SYS_SPECS_H__
#define __SYS_SPECS_H__



#ifdef __cplusplus
extern "C"
{
#endif

/*
* 注意
*
* 1. 本文件改动会重新编译整个工程，如非产品规格相关请不要加于此处。
*
* 2. 目前，平台很多地方如slot，port，vid等都是 base 0 处理的，
*    我们是 base 1 处理的，所以要将值加 1 为妥，将来争取让平台
*    更正，至少可以让我们选择是 base 0 还是 base 1 处理。
*/

/*
* 目前有关产品规格的定义散布很广，不利于更改和维护，
* 本文件定义不同产品规格。原来散布各处的定义一律移除。
* 
* 除了声明静态数据用宏 XXX 以外，其它都用变量 g_ulXXX 操作。
* 如非必须，尽量不要重新定义或间接引用本文件的宏或变量。
* 
* MAXTrunkNum
*   系统内可以创建的最大trunk数目。
* 
* MAXPortOnTrunk
*   trunk 最大成员端口数目。
* 
* MAXVlanNum
*   系统内可以创建的最大vlan数目。
* 
* MAXVlanNumWacky
*   特殊情况下，系统内可以创建的最大vlan数目。
* 
* MAXSuperVlanNum
*   系统内可以创建的最大super vlan数目。
*
* MAXL3InterfaceNum
*   可以配置ip的vlan最大个数
*  
* MAXSlotNum
*   系统内最大槽位号。
* 
* MAXPortOnSlotNum
*   每槽位最大端口数。
*
* EthPortOnCPCNum
*   ???
*
* MAXPortNum
*   系统内最大端口数
*
*
*
*/
	
    #define MAXTrunkNum                     1
    #define MAXPortOnTrunk                  1
    
    /* vlan */
    #define MAXVlanNum                      4095 /* include invalid vid 0, 4095 */
    #define MAXVlanNumWacky                 128
    #define MAXSuperVlanNum                 1
    #define MAXSubVlanOnSuperVlan       	2
    #define MAXL3InterfaceNum               4 /* vlan 配 ip */
    
    /* gvrp dynamic vlan */
    #define MAX_DyNamicVlan_NUM             5
    #define MAX_DyNamicVlan_Wacky_NUM       0     /* if have 5616 */
    #define MAXSlotNum                      2	/* Must >= (real Slot Num + 1) */
    #define MAXPortOnSlotNum                5	/* Must >= (real Port Num + 1) */
    #define EthPortOnCPCNum                 6	/* Must >= (real Port Num + 1) */
    #define MAXPortNum                      (MAXPortOnSlotNum*MAXSlotNum)

    /* fdb */
    #define MAXFdbDropNum                   1
    #define MAXFdbNormalNum                 1

    #define VOS_TASKPRI_GVRP                150
    #define VOS_TASKPRI_FDB                 200

#ifndef DEF_CTRL_CHAN_VLANID
	#define DEF_CTRL_CHAN_VLANID 			2
#endif
	//#define MAX_ROTER_PORT (pRcpDev->numOfPorts)
	#define SysBeginOfPortOnSlot   1
	#define SysEndOfPortOnSlot     g_ulMAXPortOnSlotNum
	#define SysIsValidPortOnSlot( _port ) \
    	((_port)>=SysBeginOfPortOnSlot && (_port)<=SysEndOfPortOnSlot)
#endif




