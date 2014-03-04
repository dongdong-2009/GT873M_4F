#ifndef __IROS_CONFIG_H
#include "products_config.h"

#define MODULE_YES	1
#define MODULE_NO	0

#define PRODUCTS_GT811D			1
#define PRODUCTS_GT811G			2
#define PRODUCTS_GT873_M_4F4S	3
#define PRODUCTS_GT812C			4

#if (PRODUCT_CLASS == PRODUCTS_GT811D)
#define HAVE_SWITCH_SPEED_100	MODULE_YES
#define HAVE_SWITCH_SPEED_1000	MODULE_NO
#define GE_RATE_LIMIT			MODULE_NO

#elif (PRODUCT_CLASS == PRODUCTS_GT811G)
#define HAVE_SWITCH_SPEED_100	MODULE_NO
#define HAVE_SWITCH_SPEED_1000	MODULE_YES
#define GE_RATE_LIMIT			MODULE_YES

#elif (PRODUCT_CLASS == PRODUCTS_GT873_M_4F4S)
#define HAVE_SWITCH_SPEED_100	MODULE_YES
#define HAVE_SWITCH_SPEED_1000	MODULE_NO
#define GE_RATE_LIMIT			MODULE_NO
#define HAVE_TERMINAL_SERVER 1

#elif(PRODUCT_CLASS == PRODUCTS_GT812C)
#define HAVE_SWITCH_SPEED_100	MODULE_YES
#define HAVE_SWITCH_SPEED_1000	MODULE_NO
#define GE_RATE_LIMIT			MODULE_NO
#else
#warning "PRODUCT_CLASS define is unknnow!!"
#endif

#define FOR_ONU_PON (PRODUCT_CLASS == PRODUCTS_GT812C ||  \
											PRODUCT_CLASS == PRODUCTS_GT811D || \
											PRODUCT_CLASS == PRODUCTS_GT811G || \
											PRODUCTS_GT873_M_4F4S == PRODUCTS_GT873_M_4F4S)

#define __IROS_CONFIG_H
#define HAVE_MPORTS 1
#define HAVE_SYS_MEMPOOL 1
#define HAVE_MEM_TRACE 1
#define HAVE_MINI_CLI 1
#define HAVE_LOG_THREAD 1
#define HAVE_EXCEPTION_HANDLER 1
#define HAVE_CPULOAD_PER_THREAD 1
#define HAVE_IP_STACK 1
#define HAVE_WEB_SERVER 1
#define HAVE_FLASH_FS 1
#define HAVE_CTC_OAM 1
#define HAVE_POSIX 1
#define HAVE_SERIAL_FLASH 1
#define HAVE_SDL_CMD 1
#define HAVE_SDL_CMD_CTC 1
#define HAVE_SDL_CMD_HELP 1
#define HAVE_SDL_CTC 1
#define HAVE_DB_MANAGMENT 1
#define HAVE_MC_CTRL 1
#define HAVE_LOG_FILE 1
//#define HAVE_LOOP_DETECT 1
#define HAVE_PPPOE 1
#define HAVE_PPP 1
#define HAVE_PTP 1
#define HAVE_TELNET_CLI 1
#define HAVE_MIF 1
#define HAVE_LOOP_LED 1

#if(PRODUCT_CLASS == PRODUCTS_GT811D || PRODUCT_CLASS == PRODUCTS_GT811G || PRODUCT_CLASS == PRODUCTS_GT873_M_4F4S)
#define HAVE_RTK_CMD 1
#define CS_UNI_NUMBER 4
#define CS_UPLINK_PHY_PORT 6
#elif(PRODUCT_CLASS == PRODUCTS_GT812C)
#define HAVE_MRV_CMD 1
#define CS_UNI_NUMBER 8
#define CS_UPLINK_PHY_PORT 10
#else
#define CS_UNI_NUMBER 1
#define CS_UPLINK_PHY_PORT 0
#endif

#define HAVE_SW_DBG_CMD (HAVE_MRV_CMD | HAVE_RTK_CMD)

#define IGMP_TRANSPARENT_CONFIG	 MODULE_YES


#endif
