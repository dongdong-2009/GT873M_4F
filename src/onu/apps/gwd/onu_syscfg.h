#ifndef _SYSCFG_H_
#define _SYSCFG_H_

#ifdef	__cplusplus
extern "C"{
#endif
/* Please use -D_RELEASE_VERSION_ opition in Tornado compiler. *\
 * For changing this defination will cause lots of files have to be recompile.
 * You can create two Projects in tornado for debug and release version separately.
 * And in such way, we can save much compiling time for debug and release at the same time */
/*#define _RELEASE_VERSION_*/	
/*#undef  _RELEASE_VERSION_*/

#define _VOS_22_CHANGE_
/*----------------------------------------------------------------------*/
/*
 * 下面四个宏定义了版本信息，可根据需要在编译前修改，修改后必须重新编译。
 */
#define RPU_VERSION           "1.00"
#define RPU_VERSION_PRODUCT   "PSR"
#define RPU_COPY_TIME         "2001-2002"
#define RPU_CMO_NAME          "Ipsion Networks"
/*----------------------------------------------------------------------*/
#define _VOS_NEW_STACK_

/*----------------------------------------------------------------------*/
/*
 * 下面两个宏定义了“开”和“关”两个状态，这两个宏被后面的
 * 宏引用。例如：
 *
 *	#define	RPU_MODULE_RTPRO_OSPF	RPU_NO
 *	#define	RPU_MODULE_RTPRO_RIP	RPU_YES
 *
 * 表示动态路由协议OSPF被关闭，而动态路由协议RIP被开启，这
 * 样，系统在编译时，就可以拆掉OSPF模块而保留RIP模块。
 * 需要注意的是有些意义上互斥的一对宏，不能被同时置为
 * “RPU_YES”或“RPU_NO”，这一点将体现在后面的叙述中。
 * 注意，请不要修改这两个宏的值。
 */
#define	RPU_YES		1
#define	RPU_NO		0
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*
 * 下面两个宏描述编译的版本是运行在ROM板上，还是RAM板上，
 * 这两个宏是互斥的，它们不能被置为相同的值。这一对宏可以使
 * RPU软件认出硬件平台是否支持调试，以决定针对不同类型的单
 * 板（指RAM板和ROM板）做不同的动作。RPU软件将如下使用
 * 这两个宏：
 *
 *	#if	( RPU_BOARD_RAM == RPU_YES )
 *	{
 *		这是调试板，可以嵌入特定的调试代码
 *	}
 *	#else
 *	{
 *		非调试板，不能做特殊处理
 *	}
 *	#endif
 *
 * 注意，这里的调试代码是针对不同硬件单板的。
 * RPU软件将尽量少得使用该宏。
 */
#define	RPU_BOARD_RAM			RPU_YES
#define	RPU_BOARD_ROM			RPU_NO
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*
 * 下面的宏描述RPU软件即将运行的硬件平台是否支持实时钟。
 * 对于那些不支持实时钟的硬件平台，RPU的VOS需要使用操作系统提供的调用
 * 来设置系统启动时的年月日和时间；而对于自身就支持实时钟的硬件平台，
 * 这件事情是不应该再做的。例如，RPU的VOS会如下使用这个宏：
 *
 *	#if( RPU_BOARD_REAL_TIMER == RPU_NO )
 *	{
 *		设置系统的启动时间
 *	}
 *	else
 *	{
 *		不再设置系统的启动时间
 *	}
 *	#endif
 *
 * 请根据采用的硬件平台的具体情况来设置该宏的值。
 */
#define	RPU_BOARD_REAL_TIMER	RPU_YES
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*
 * 下面两个宏的开与关可以决定编译出的RPU版本是“发行版本”
 * 还是“调试版本”，这两个宏的设置也是互斥的。在调试版本
 * 中，应用程序可以嵌入大量的调试信息，而在发行版本中，这是
 * 不允许的。请注意，通过这两个宏控制的调试信息与具体的硬件
 * 平台无关，也就是说，这样的调试信息在ROM板上和RAM板上是
 * 一样的；而需要区分硬件平台的调试信息由宏
 * “RPU_BOARD_RAM”和“RPU_BOARD_ROM”来确定（参见
 * 前面的介绍）。RPU软件将如下使用这两个宏：
 *
 *	#if	( RPU_VERSION_RELEASE == RPU_NO )
 *	{
 *		调试版本，可以嵌入调试代码
 *	}
 *	#else
 *	{
 *		发行版本，严禁嵌入调试代码
 *	}
 *	#endif
 *
 * 或
 *
 *	#if	( RPU_VERSION_DEBUG == RPU_YES )
 *	{
 *		调试版本，可以嵌入调试代码
 *	}
 *	#else
 *	{
 *		发行版本，严禁嵌入调试代码
 *	}
 *	#endif
 */
#define	RPU_VERSION_RELEASE		RPU_NO
#define	RPU_VERSION_DEBUG		RPU_YES
/*----------------------------------------------------------------------*/
    /* 定义产品类别 */
#define broadcomppc824x      		0x1
#define broadcomppc75x       		0x2
#define galileoppc824x       		0x3
#define broadcommips4131     	0x4
#define broadcomxscale2400   	0x5
#define presterappc824x      		0x6
#define isurf						0x7
#define GT831					0x8
#define EPON3					0x9
#define GT810					0xa
#define GT812                   0xb
#define GT811                   0xc
#define GT861_ONU				0xd
#define GT861_IAD				0xe
#define GT815                   0xf
#define GT812PB                 0x10
#define GT866_ONU               0x11
#define GT866_IAD				0x12
#define GT831B					0x13

#define GT863_ONU   			0x14
#define GT813                   0x15
/* Define PAS Firmware version */
#define _FW_V1_4_B0_3_			0x1
#define _FW_V1_4_B7_4_			0x7

#define BCM53242	0
#define BCM53212	1
#define BCM5324     2

#define FOR_ONU_812_AB ((PRODUCT_CLASS == GT812) || (PRODUCT_CLASS == GT812PB))
#define FOR_812_SERIES (FOR_ONU_812_AB || (PRODUCT_CLASS == GT811))

#define FOR_BCM_ONU_PON (PRODUCT_CLASS == GT866_ONU || PRODUCT_CLASS == GT863_ONU || PRODUCT_CLASS == GT813)

#define FOR_BCM_ONU_PON_VOICE (FOR_GT863_ONU_SERIS)

#define FOR_GT863_ONU_SERIS (PRODUCT_CLASS == GT863_ONU || PRODUCT_CLASS == GT866_ONU)

#define FOR_MRV_BOX_PON	((PRODUCT_CLASS == GT815) || FOR_812_SERIES || (PRODUCT_CLASS == GT831B))
#define FOR_MRV_ONU_PON_NO_811	((PRODUCT_CLASS == GT861_ONU) || (PRODUCT_CLASS == GT815) || FOR_ONU_812_AB || (PRODUCT_CLASS == GT831B))
#define FOR_MRV_ONU_PON	(FOR_MRV_ONU_PON_NO_811 || (PRODUCT_CLASS == GT811))

#define FOR_MULITY_PORT_BOX_ONU_PON	((PRODUCT_CLASS == GT831) || FOR_MRV_BOX_PON || FOR_BCM_ONU_PON)
#define FOR_SINGLE_PORT_BOX_ONU_PON	((PRODUCT_CLASS == GT810))
#define FOR_BOX_ONU_PON	(FOR_MULITY_PORT_BOX_ONU_PON || FOR_SINGLE_PORT_BOX_ONU_PON)
#define FOR_CHASSIS_ONU_PON	(PRODUCT_CLASS == GT861_ONU)

#define FOR_ONU_PON 	(FOR_CHASSIS_ONU_PON || FOR_BOX_ONU_PON)
#define FOR_MULITY_PORT_ONU_PON	(FOR_CHASSIS_ONU_PON || FOR_MULITY_PORT_BOX_ONU_PON)
#define FOR_ONU_IAD 	((PRODUCT_CLASS == GT861_IAD) || (PRODUCT_CLASS == isurf) || (PRODUCT_CLASS == GT866_IAD))
#define FOR_ONU_PON_NO_CTRL_CHANNEL 	(FOR_MRV_BOX_PON || FOR_SINGLE_PORT_BOX_ONU_PON)
#define FOR_ONU_PON_NO_831 	(FOR_MRV_ONU_PON || FOR_SINGLE_PORT_BOX_ONU_PON || FOR_BCM_ONU_PON)

#define INCLUDE_IN_ONU 	(FOR_ONU_PON || FOR_ONU_IAD)
#define EXCLUDED_IN_ONU (!INCLUDE_IN_ONU)
#define INCLUDE_IN_ONU_NO_ISURF	 (FOR_ONU_PON || (PRODUCT_CLASS == GT861_IAD) || (PRODUCT_CLASS == GT866_IAD))
#define INCLUDE_IN_BOX_ONU 	(FOR_BOX_ONU_PON || FOR_ONU_IAD)

/*----------------------------------------------------------------------*/
/*
 * 下面两个宏为RPU软件定义另外字节顺序，也是
 * 两个互斥的宏。各个产品应该根据所采用CPU的类别恰当地设置
 * 这两个宏。RPU软件将如下使用这两个宏：
 *
 *	#if	( RPU_LITTLE_ENDIAN == RPU_NO )
 *	{
 *		按照BIG_ENDIAN字节顺序定义数据结构
 *	}
 *	#else
 *	{
 *		按照LITTLE_ENDIAN字节顺序定义数据结构
 *	}
 *	#endif
 *
 * 或
 *
 *	#if	( RPU_BIG_ENDIAN == RPU_YES )
 *	{
 *		按照BIG_ENDIAN字节顺序定义数据结构
 *	}
 *	#else
 *	{
 *		按照LITTLE_ENDIAN字节顺序定义数据结构
 *	}
 *	#endif
 */
#if (PRODUCT_CLASS == isurf)
#define	LITTLE_ENDIAN		RPU_NO
#define	BIG_ENDIAN		    RPU_YES
#ifdef _DISTRIBUTE_PLATFORM_
#define 	VOS_BIG_ENDIAN			VOS_YES
#endif
#endif

#if (FOR_ONU_PON || (PRODUCT_CLASS == GT861_IAD) || (PRODUCT_CLASS == GT866_IAD))
#define	LITTLE_ENDIAN		RPU_YES
#define	BIG_ENDIAN		    RPU_NO
#ifdef _DISTRIBUTE_PLATFORM_
#define 	VOS_BIG_ENDIAN			VOS_NO
#define   VOS_LITTLE_ENDIAN			VOS_YES
#endif
#endif

/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*
 * 下面几个宏描述了RPU软件即将运行的硬件平台的CPU的类型。但
 * 是，除了VOS模块，RPU的其他协议或功能模块将尽量不使用这几
 * 个宏。这几个宏的定义也是互斥的，也就是说，在一次编译时，
 * 它们中只能有一个被定义成RPU_YES，其余的都是RPU_NO。
 */
#define	RPU_CPU_mPPC_8240		RPU_NO
#define	RPU_CPU_mPPC_750		RPU_YES
#define	RPU_CPU_x86			    RPU_NO
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*
 * 下面几个宏描述了RPU软件即将运行的OS平台的类型。请根据实际情况将
 * 其中的一个置为RPU_YES，其余的全部置为RPU_NO。但是，除了VOS模块，
 * RPU的其他协议或功能模块将尽量不使用这几个宏。
 */
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*
 * 这几个宏描述了RPU软件即将运行的RTOS平台所使用的编译器的
 * 类型。但是，除了VOS模块，建议其他协议或功能模块尽量不要
 * 使用这几个宏。
 * 这里需要说明的是：pSOS操作系统在Windows3.11下的环境采用
 * 了Green Hill Software公司的一个叫做C-68000的编译器（使用
 * 宏RPU_COMPILER_GHC_68000来标识）；pSOS操作系统在
 * Windows95或Windows98下的环境下采用了CC386编译器（使用宏
 * RPU_COMPILER_CC386来标识）。而上述两种编译器可以识别的
 * 字节对齐的预处理宏是不同的，C-68000识别pack，而CC386识别
 * align。举例如下：如果应用程序在希望指定编译器按1字节对齐数
 * 据结构TEST_S，应该如下书写程序：
 *
 *	#if( RPU_COMPILER_GHC_68000 == RPU_YES || RPU_COMPILER_GCC == RPU_YES )
 *	#pragma  pack( 1 )
 *	#elif( RPU_COMPILER_CC386 == RPU_YES )
 *	#pragma  align( tagTest = 1 )
 *	#endif
 *
 *	typedef  struct  tagTest
 *	{
 *		......
 *	} TEST_S;
 *
 *	#if( RPU_COMPILER_GHC_68000 == RPU_YES  || RPU_COMPILER_GCC == RPU_YES )
 *	#pragma  pack
 *	#elif( RPU_COMPILER_CC386 == RPU_YES )
 *	#pragma  align
 *	#endif
 *
 * （请注意，上面的例子并不完整，只是用来说明问题而已！）
 * 关于pack和align的详细说明，请参考编译器携带的帮助文件。
 */
#define	RPU_COMPILER_CC386		RPU_NO
#define	RPU_COMPILER_GHC_68000	RPU_NO
#define	RPU_COMPILER_GCC		RPU_YES
#define	RPU_COMPILER_DCC		RPU_NO
#define	RPU_COMPILER_MSVC		RPU_NO
/*----------------------------------------------------------------------*/

/************************************************************************
 * RPU软件产品宏
 ***********************************************************************/
/* #define _L3_CHASIS_SWITCH_ 			RPU_YES */
/*#define _ROUTER_			RPU_NO*/
/************************************************************************/

/************************************************************************
 * 分布式路由软件宏定义
 ***********************************************************************/
/*#define VOS_V2			RPU_YES
***********************************************************************


 *                                                                      *
 * 以下的宏描述了RPU软件中各协议模块宏和功能模块，通过调整这些宏        *
 * 的开关状态，可以在编译时拆离不需要的协议或功能。但是，对于RPU        *
 * 的一些核心模块，不允许拆离，也就是说，描述这些模块的宏必须被置       *
 * 为RPU_YES（开）状态。                                                *
 *                                                                      *
 * 还需要注意的一点是，根据协议和功能的特点，RPU的一些模块之间存在      *
 * 一定的依赖关系；而存在依赖关系的若干个模块必须同时被开启或关闭。     *
 * 也就是说：如果A模块依赖于B模块，那么如果需要开启A模块时，B模块       *
 * 也必须被开启。                                                       *
 *                                                                      *
 * 由于RPU的核心模块必须被开启，所以凡是与核心模块的依赖关系不再描述。  *
 *                                                                      *
 ************************************************************************/
/*********************
 * RPU软件的启动部分 *
 *********************/
/*----------------------------------------------------------------------*/
/*
 * RPU的软件的设备管理模块。
 * 使用RPU软件，该项宏必须被开启。
 */
#define	RPU_MODULE_DEVICE			RPU_YES
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* FIB同步模块 */
/*----------------------------------------------------------------------*/
#define	RPU_MODULE_FSYN   			RPU_YES
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/


/*************************
 * RPU软件的加载部分 *
 *************************/
/*----------------------------------------------------------------------*/
/*
 * 使用RPU软件，下面的两项宏必须被开启。
 */
#define	RPU_MODULE_LOAD			RPU_NO
#ifdef _DISTRIBUTE_PLATFORM_
#define   LMU_MODULE_LOAD                 RPU_NO
#endif

#define RPU_MODULE_IEM          RPU_NO

#define RPU_MODULE_CDP          RPU_NO
/*
 热备份模块。
 */
#define	RPU_MODULE_BACKUP		RPU_NO


/*
 * 接口管理
 */

#define	RPU_MODULE_IFM			RPU_YES


/*
 * RIP模块
 */
#define RPU_MODULE_RIP          RPU_NO
#define RPU_MODULE_OSPF         RPU_NO
#define RPU_MODULE_ISIS          RPU_NO
#define RPU_MODULE_BGP          RPU_NO
#define RPU_MODULE_EIGRP        RPU_NO

/*
* IP MULTICAST ROUTING
*/
#define RPU_MODULE_MIP			RPU_NO

#define RPU_MODULE_MROUTE       RPU_MODULE_MIP
#define RPU_MODULE_IGMP         RPU_MODULE_MIP
#define RPU_MODULE_PIM          RPU_MODULE_MIP

/*
 *Rtm模块
 */

#define RPU_MODULE_RTM          RPU_YES


/*
 *LSP管理模块
 */

#define RPU_MODULE_LSPM         RPU_NO


/*
 *LDP模块
 */

#define RPU_MODULE_LDP          RPU_NO


/*
 *RSVP模块
 */

#define RPU_MODULE_RSVP         RPU_NO

/*
 *NAT模块
 */

#define RPU_MODULE_NAT        RPU_NO

/*
 *STP模块
 */
#define RPU_MODULE_STP	        RPU_NO

#ifdef _DISTRIBUTE_PLATFORM_
/*
* RPU_MODULE_SUPERVISOR
*/
#define RPU_MODULE_SUPERVISOR    RPU_YES

/*
* _MAIL_SEND_
*/
#define _MAIL_SEND_    RPU_NO

/*
 *_HOOK_
 */
#define _HOOK_		RPU_NO

/*
 *ERPP模块
 */
#define RPU_MODULE_ERRP	        RPU_NO

/*
 *LDT模块
 */
#define RPU_MODULE_LDT          RPU_NO

/*
 *IGMP_SNOOP模块
 */
#define RPU_MODULE_IGMP_SNOOP    RPU_YES

/*
 * NP 支持剪裁宏
 */
#define RPU_MODULE_NP            RPU_NO

/*
 *CHDLC 支持裁减宏             
 */
#define LMU_MODULE_CHDLC        RPU_NO

/*
 *ARP 支持裁减宏             
 */
#define LMU_MODULE_ARP           RPU_YES

/*
 *POS 支持裁减宏             
 */
#define RPU_MODULE_POS          RPU_NO

/*
 *ATM 支持裁减宏             
 */
#define RPU_MODULE_ATM          RPU_NO
/*
 *Port Filter裁减
 */
#define RPU_MODULE_FILTER       RPU_NO
#endif

/*
 *CLI模块
 */
#define RPU_MODULE_CLI	        RPU_YES

/*
 *SNMP 模块
 */
#define RPU_MODULE_SNMP         RPU_YES

/*
 *Web 模块
 */
#define RPU_MODULE_MANA_WEB         RPU_NO

/*
 *GRE管理模块
 */

#define RPU_MODULE_GRE          RPU_NO

/*
 *ARP模块
 */
#define RPU_MODULE_ARP	        RPU_YES
/*
 *Manage模块
 */
#define RPU_MODULE_MANAGE       RPU_YES

/*
 *Manage模块
 */
#define RPU_MODULE_XMODEM       RPU_NO

/*
 *Ftp client模块
 */
#define RPU_MODULE_FTP_CLIENT	RPU_NO

/*
 *Ftp server模块
 */
#define RPU_MODULE_FTP_SERVER	RPU_NO

/*
 *SysLog模块
 */
#define RPU_MODULE_SYSLOG       RPU_YES


/*
* VFS 模块
*/
#define RPU_MODULE_VFS			RPU_NO

/*
 *FDB 模块
 */
 #if(FOR_BCM_ONU_PON)
#define RPU_MODULE_FDB          RPU_YES
#else
#define RPU_MODULE_FDB          RPU_NO
#endif

/*
 *Qos 模块
*/
#define RPU_MODULE_QOS			RPU_NO

/*
 *VRRP 模块
 */
#define RPU_MODULE_VRRP         RPU_NO

/*
 *HSRP 模块
 */
#ifndef _DISTRIBUTE_PLATFORM_
#define RPU_MODULE_HSRP         RPU_NO
#else
#define RPU_MODULE_HSRP         RPU_NO
#endif

/*
 *SNTP 模块
 */
#define RPU_MODULE_SNTP         RPU_YES
/*
 * HOT模块
 */
#define RPU_MODULE_HOT          RPU_NO

/*
 * L2VPN模块
 */
#define RPU_MODULE_L2VPN        RPU_NO


/*
 * VPLS模块
 */
#define RPU_MODULE_VPLS        RPU_NO

/*
 * L3VPN模块
 */
#define RPU_MODULE_L3VPN        RPU_NO


/*
 * DHCP 模块
 */
#define RPU_MODULE_DHCP         RPU_YES

/*
 * UDP FORWARDING 模块
 */
#define RPU_MODULE_UDP_FORWARD	RPU_NO

/*
 * GVRP 模块
 */
#define RPU_MODULE_GVRP	        RPU_NO

/*
 * RADIUS模块
 */
#define RPU_MODULE_RADIUS       RPU_NO

/*
 *NAS模块
 */
#define RPU_MODULE_NAS          RPU_NO

/*
 *IEEE 802.1X模块
 */
#define RPU_MODULE_DOT1X        RPU_NO

/*
 *HLINK模块
 */
#define RPU_MODULE_HLK       RPU_NO

/*
 *PPP模块
 */
#define RPU_MODULE_PPP          RPU_NO

/*
 *PPPOE模块
 */
#define RPU_MODULE_PPPOE        RPU_NO

/*
*PPPOE侦听
*/
#define RPU_MODULE_PPPOE_SNOOPING	RPU_NO

/*
*PPPOE中继
*/
#define RPU_MODULE_PPPOE_RELAY  RPU_NO 

/*
*定时发送报文
*/
#define RPU_MODULE_TIMING_PKT  RPU_NO

/*
跨vlan组播
*/
#define RPU_MODULE_IGMP_TVM     RPU_NO

/*
 *DHCPC模块
 */
#define RPU_MODULE_DHCPC        RPU_YES

/*
 *DHCP Server模块
 */
#define RPU_MODULE_DHCPS        RPU_YES

/*
 *TACACS+模块
 */
#define RPU_MODULE_TACC        RPU_NO

/*
 *COPS(PEP)模块
 */
#define RPU_MODULE_COPS		RPU_NO

/*
 *HTTP Redirect模块
 */
#define RPU_MODULE_RED		RPU_NO

/*
 *SSH 模块
 */
#define RPU_MODULE_SSH 	RPU_NO

/*
  * CPU使用率统计和死循环统计
  */
#define RPU_MODULE_CPU          RPU_YES

/*
* TRACEROUTE
*/
#define _CONFIG_TRACERT_  
/*
* TRACEROUTE
*/
#define _CONFIG_DNS_            RPU_YES

/*
  * TARGET SHELL命令行上启动shell
  */
#define RPU_MODULE_TARGET_SHELL   RPU_YES

/*
  * VOS ACTIVE SHELL在命令行死掉时启动shell
  */
#ifndef  _DISTRIBUTE_PLATFORM_
#define RPU_MODULE_ACTIVE_SHELL   RPU_YES
#else
#define RPU_MODULE_ACTIVE_SHELL   RPU_YES
#endif

#define RPU_MODULE_CTC_OAM 	RPU_NO

/*
  * 主控板上可以操作线路板或网板的shell,接口在主控板显示
  */
#define RPU_MODULE_RPU_TO_LIC_SHELL   RPU_YES
/*
  * 主控板上可以输入命令行在线路板和网板上执行,主控板输出结果
  */
#define RPU_MODULE_CLI_TO_LIC   RPU_YES

#define RPU_HAVE_FPGA			RPU_NO

#define RPU_MODULE_VOICE		RPU_NO

#define RPU_MODULE_E1		RPU_NO

#define RPU_MODULE_VOIP		RPU_NO

#define RPU_LOAD_CFG_FILE_FROM_MASTER RPU_NO

#define RPU_MODULE_EPON_ALL RPU_NO

#define RPU_MODULE_XCVR_MON RPU_NO

#define RPU_MODULE_POE 		RPU_NO

#define RPU_MODULE_CPLD		RPU_NO

#define RPU_MODULE_CTRL_CHANNEL		RPU_NO

#ifdef _DISTRIBUTE_PLATFORM_
/* 包含产品自己的模块剪裁配置文件 */
/* 在makefile/superset/superset.h，不同产品可以关闭不需要的模块 */
#include "superset.h"
#endif
/************************************************************************
 *                                                                      *
 * 以下的宏描述了RPU软件中一些与系统相关的常数。                        *
 * 通过调整这些常数的值，可以使RPU适应不同的系统的要求。                *
 * 对这些常数的修改，应该由经验丰富的系统人员来更改，                   *
 * 除非有确切的理由，否则，请不要轻易改动这些常数。                     *
 *                                                                      *
 ************************************************************************/
/*----------------------------------------------------------------------*/
/* #define */
/*----------------------------------------------------------------------*/


/************************************************************************
 *                                                                      *
 * 以下是为RPU软件的各个部分定义的调试宏。                              *
 *                                                                      *
 * 考虑到分模块调试的方便性，为RPU的各部分定义如上的调试                *
 * 宏。定义这些宏的意义在于，可以控制RPU的最终版本只含有某              *
 * 一部分的调试代码，从而控制编译出版本的大小，便于调试；这             *
 * 些宏与RPU的各个部分是一一对应的（鉴于VOS的特殊性，将                 *
 * VOS的调试宏按照VOS的子模块进一步细化）。这样的宏必须和               *
 * RPU的版本宏（即RPU_VERSION_DEBUG和                                   *
 * RPU_VERSION_RELEASE）配合使用，也就是说，如果宏                      *
 * RPU_DEBUG_IP是被打开的，但是宏RPU_VERSION_DEBUG被                    *
 * 关闭并且宏RPU_VERSION_RELEASE被打开，那么，关于IP部分                *
 * 的调试代码同样不能被编译进版本中。                                   *
 * 为了便于编程，各模块可以使用宏RPU_DEBUG_IS_ON简化代                  *
 * 码（宏RPU_DEBUG_IS_ON的定义随后给出）：                              *
 * 例如：                                                               *
 *                                                                      *
 *	#if( RPU_DEBUG_IS_ON( RPU_DEBUG_MPLS ) == RPU_YES )                   *
 *	{                                                                   *
 *		调试代码                                                        *
 *	}                                                                   *
 *	#endif                                                              *
 *                                                                      *
 ************************************************************************/
#define	RPU_DEBUG_DEVICE		RPU_NO
#define	RPU_DEBUG_LOAD			RPU_NO
#define	RPU_DEBUG_LICA			RPU_NO
#define	RPU_DEBUG_IPC			RPU_NO
#define	RPU_DEBUG_RS			RPU_NO
#define	RPU_DEBUG_VRRP			RPU_NO

#define	RPU_DEBUG_IS_ON( RPU_DEBUG ) \
		( \
		    ( RPU_VERSION_DEBUG == RPU_YES ) \
		  ? ( RPU_YES ) \
		  : ( \
				( RPU_DEBUG == RPU_YES ) \
			  ? ( RPU_YES ) \
			  : ( RPU_NO ) \
			) \
		)

/* LSP和LDP、RSVP的依赖关系 */
#if (RPU_MODULE_LSPM == RPU_NO)
#undef  RPU_MODULE_LDP
#undef  RPU_MODULE_RSVP
#define RPU_MODULE_LDP         RPU_NO
#define RPU_MODULE_RSVP        RPU_NO
#elif((RPU_MODULE_LSPM == RPU_YES) && (RPU_MODULE_LDP == RPU_NO) && (RPU_MODULE_RSVP == RPU_NO))
#error if RPU_MODULE_LSPM is on, either RPU_MODULE_LDP or RPU_MODULE_RSVP is on !
#endif

/* L3VPN和LSPM及BGP的依赖关系 */
#if (RPU_MODULE_LSPM == RPU_NO || RPU_MODULE_BGP == RPU_NO)
#undef RPU_MODULE_L3VPN
#define RPU_MODULE_L3VPN       RPU_NO
#endif /* (RPU_MODULE_LSPM == RPU_NO || RPU_MODULE_BGP == RPU_NO) */


/* LDP相关 */
#if (RPU_MODULE_LDP == RPU_YES)
#define VOS_MPLS_LDP        1                   /* MPLS LDP模块 */
#define VOS_MPLS_L2VPN      1                   /* MPLS L2VPN模块 */
#define VOS_MPLS_L2VPN_EX   1 
#define VOS_MPLS_VPLS       1 
#define VOS_MPLS_MIB        1                   /*MPLS_MIB支持*/
#else /* RPU_MODULE_LDP == RPU_NO */
#define VOS_MPLS_LDP        0                   /* MPLS LDP模块 */
#define VOS_MPLS_L2VPN      0                   /* MPLS L2VPN模块 */
#endif /* RPU_MODULE_LDP == RPU_YES */
/* L2VPN相关 */
#if (RPU_MODULE_L2VPN == RPU_NO)
#undef  VOS_MPLS_L2VPN
#undef  VOS_MPLS_L2VPN_EX
#define VOS_MPLS_L2VPN      0                   /* MPLS L2VPN模块 */
#define VOS_MPLS_L2VPN_EX   0 
#endif
/* VPLS */
#if(RPU_MODULE_VPLS == RPU_NO)
#undef  VOS_MPLS_VPLS
#define VOS_MPLS_VPLS       0
#endif

/* RSVP相关 */
#if (RPU_MODULE_RSVP == RPU_YES)
#define VOS_MPLS_RSVPTE     1                   
#else /* RPU_MODULE_RSVP == RPU_NO */
#define VOS_MPLS_RSVPTE     0                   /* MPLS RSVP-TE模块 */
#endif /* RPU_MODULE_RSVP == RPU_YES */

/* MPLS软转发相关 */
#if (RPU_MODULE_LSPM == RPU_YES)
#define VOS_MPLS_LSPM       1                   /* MPLS LSP管理模块 */
#define VOS_MPLS_SOFTFW     1                   /* MPLS 软转发 */
#else
#define VOS_MPLS_LSPM       0
#define VOS_MPLS_SOFTFW     0                   /* MPLS 软转发 */
#endif /* (RPU_MODULE_LSPM == RPU_YES) */

/* GRE相关 */
#if (RPU_MODULE_GRE == RPU_YES)            
#define VOS_GRE	            1                   /*GRE 模块*/
#else
#define VOS_GRE	            0    
#endif /* (RPU_MODULE_LSPM == RPU_YES) */

#if (RPU_MODULE_UDP_FORWARD == RPU_YES)
#define VOS_UDPF_SOFTFW       1   /*主控板*/
#define VOS_UDPF_LICAFW       0   /*接口板*/
#else
#define VOS_UDPF_SOFTFW       0
#endif

/* BGP相关 */
#if (RPU_MODULE_BGP == RPU_YES)
#define VOS_ROUTE_BGP        1       	/* BGP协议模块 */
#define VOS_ROUTE_BGP_MD5    1       	/* BGP MD5特性 */
#if (RPU_MODULE_SNMP == RPU_YES)
#define VOS_ROUTE_BGP_MIB     1           /*BGP 网管特性支持*/
#endif
#endif /* (VOS_ROUTE_BGP == RPU_YES) */

/* L3VPN相关 */
#if (RPU_MODULE_L3VPN == RPU_YES)
#define VOS_VRF_MPLS_VPN	1
#if (RPU_MODULE_SNMP == RPU_YES)
#define VOS_ROUTE_3LVPN_MIB     1           /*3LVPN 网管特性支持*/
#endif
#endif /* (RPU_MODULE_L3VPN == RPU_YES) */


/*NAT 相关*/
#if (RPU_MODULE_NAT == RPU_YES)
#define VOS_IP_NAT                 1                    /*NAT*/
#else
#define VOS_IP_NAT                 0
#endif
#if ( VOS_IP_NAT && VOS_VRF_MPLS_VPN )
#define VOS_IP_NAT_VPN 		       1 
#endif


/* DHCP Relay相关 */
#if (RPU_MODULE_DHCP == RPU_YES && RPU_MODULE_L3VPN == RPU_YES)
#define VOS_VRF_MPLS_VPN_DHCPR   1
#endif

/* 组播相关 */
#if (RPU_MODULE_MROUTE == RPU_YES && RPU_MODULE_IGMP == RPU_YES && RPU_MODULE_PIM == RPU_YES)
#define VOS_MULTICAST_ROUTING 	 1    		  /* 支持组播路由转发 */
#endif

#if VOS_MULTICAST_ROUTING
#define VOS_ROUTE_MROUTE_SOFTWEAR_FORWARD 1 /*软件转发*/
#define VOS_ROUTE_IGMP    1       	/* IGMP协议模块 */
#ifndef _DISTRIBUTE_PLATFORM_
#define VOS_ROUTE_IGMP_BROADING 1 /* 是否支持在valn内广播igmp报文(转发的时候) */
#else
#define VOS_ROUTE_IGMP_BROADING 0 /* 是否支持在valn内广播igmp报文(转发的时候) */
#endif
#define VOS_ROUTE_PIM   	1		/* PIM协议模块 */
#define VOS_ROUTE_PIM_SM   1      /* PIM-SM */
#if VOS_ROUTE_PIM_SM
#define VOS_ROUTE_PIM_DM   1      /* PIM-DM */
#define VOS_ROUTE_PIM_ACCURACY_PORT_FORWARD       1/* pim 基于端口的精确转发 */
#endif
#define VOS_ROUTE_PIM_MULTI	1		/* PIM 多实例支持 */


#define VOS_ROUTE_MROUTE_NM 0		/* 组播网管功能的支持 */
#endif

#ifdef _DISTRIBUTE_PLATFORM_
#define VOS_ROUTE_MVPN		0
#else
#if VOS_MULTICAST_ROUTING && VOS_VRF_MPLS_VPN && VOS_ROUTE_PIM_MULTI
#define VOS_ROUTE_MVPN		1		/*组播VPN支持*/
#endif
#endif

#if VOS_ROUTE_PIM_ACCURACY_PORT_FORWARD
#define VOS_ROUTE_STATIC_MROUTE    1/*支持静态组播*/
#endif


/* 访问控制相关 */
#undef _MN_HAVE_NMS_ACCESS_CONTROL_

/*其它模块对VFS的依赖*/
#if (RPU_MODULE_VFS == RPU_NO)
#define RPU_MODULE_FTP_CLIENT  RPU_NO
#define RPU_MODULE_FTP_SERVER  RPU_NO
#endif

#ifdef _DISTRIBUTE_PLATFORM_
#define VOS_PRODUCT_FAKE  1               /* 用于多播模块的产品支持 */
#endif

#ifdef _DISTRIBUTE_PLATFORM_
#define VOS_SSHD  1 			  /*支持Secure Shell */
#define VOS_LIB_CRYPTO 1
#endif

#ifdef _DISTRIBUTE_PLATFORM_
#define _SWITCH_PLATFOMR2_L3PORT_SUPPORT_        /*3层端口支持,per l3 port per vlan */
#define _DISTRIBUTE_PLATFORM_SYSLOG2NVRAM_SUPPORT_  /*支持syslog到nvram和 bootlog到navram */
#if (INCLUDE_IN_ONU)
#undef _DISTRIBUTE_PLATFORM_SYSLOG2NVRAM_SUPPORT_  /*支持syslog到nvram和 bootlog到navram */
#endif

#define _DISTRIBUTE_PLATFORM_PACKET_PROCESS_		 /*支持接口板进行包解析*/
#define _SWITCH_PLATFOMR2_MIRROR_SUPPORT_        /*支持端口镜像*/
#define _DISTRIBUTE_PLATFORM_ETH0_SUPPORT_          /* 支持 */
#endif

#ifdef _DISTRIBUTE_PLATFORM_
/* 产品规格预置 */
#include "sys/main/sys_specs.h"
#endif

#ifdef	__cplusplus
}
#endif	/* end of __cplusplus */

#endif	/* end of _SYSCFG_H_ */

