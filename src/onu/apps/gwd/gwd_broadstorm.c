#if 0
#include <network.h>
#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_io.h>
#include <cyg/io/file.h>
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <pkgconf/memalloc.h>
#include <pkgconf/isoinfra.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socketvar.h>
#include <cyg/fileio/fileio.h>
#include <pkgconf/io_fileio.h>
#include <cyg/io/file.h>
#include "../include/gw_types.h"
#include "oam.h"
#include "../include/gw_os_api_core.h"
#include "../include/gwdonuif.h"
#include "gwdonuif_interval.h"
#include "gw_log.h"
#else
#include <string.h>
#include <stdlib.h>

#include "cs_types.h"
#include "oam.h"
#include "osal_api_core.h"
#include "sdl_port.h"
#include "port_stats.h"
#include "iros_config.h"
#endif

//#define __DEBUG__	1

#if 1
#define gw_int8				cs_int8
#define gw_int32			cs_uint32
#define gw_uint8			cs_uint8
#define gw_uint16			cs_uint16
#define gw_uint32			cs_uint32
#define gw_uint64 			cs_uint64
#define gw_float			float


#define gw_return_code_t	cs_status
#define GW_RETURN_SUCCESS	CS_E_OK
#define GW_OK				CS_E_OK
#define GW_RETURN_FAIL		CS_E_ERROR


#define GW_MACADDR_LEN		6
typedef unsigned char	gw_macaddr_t[GW_MACADDR_LEN];


typedef struct {
        gw_uint64 RxFramesOk;
        gw_uint64 RxUnicasts;
        gw_uint64 RxMulticasts;
        gw_uint64 RxBroadcasts;
        gw_uint64 Rx64Octets;
        gw_uint64 Rx127Octets;
        gw_uint64 Rx255Octets;
        gw_uint64 Rx511Octets;
        gw_uint64 Rx1023Octets;
        gw_uint64 RxMaxOctets;
        gw_uint64 RxJumboOctets;
        gw_uint64 RxUndersize;
        gw_uint64 RxOversize;
        gw_uint64 RxFragments;
        gw_uint64 RxJabber;
        gw_uint64 RxFCSErrors;
        gw_uint64 RxDiscards;
        gw_uint64 RxAlignErrors;
        gw_uint64 RxIntMACErrors;
        gw_uint64 RxPppoes;
        gw_uint64 RxQueueFull;
        gw_uint64 RxPause;
        gw_uint64 RxOctetsOkMsb;
        gw_uint64 RxOctetsOKLsb;
        gw_uint64 RxError;
        gw_uint64 TxFramesOk;
        gw_uint64 TxUnicasts;
        gw_uint64 TxMulticasts;
        gw_uint64 TxBroadcasts;
        gw_uint64 Tx64Octets;
        gw_uint64 Tx127Octets;
        gw_uint64 Tx255Octets;
        gw_uint64 Tx511Octets;
        gw_uint64 Tx1023Octets;
        gw_uint64 TxMaxOctets;
        gw_uint64 TxJumboOctets;
        gw_uint64 TxDeferred;
        gw_uint64 TxTooLongFrames;
        gw_uint64 TxCarrierErrFrames;
        gw_uint64 TxSqeErrFrames;
        gw_uint64 TxSingleCollisions;
        gw_uint64 TxMultipleCollisions;
        gw_uint64 TxExcessiveCollisions;
        gw_uint64 TxLateCollisions;
        gw_uint64 TxMacErrFrames;
        gw_uint64 TxQueueFull;
        gw_uint64 TxPause;
        gw_uint64 TxOctetsOk;
        gw_uint64 TxError;
} __attribute__((packed)) gw_onu_counter_t;

typedef struct{
	gw_uint32 rxrate;
	gw_uint32 txrate;
	gw_onu_counter_t counter;
} __attribute__((packed)) gw_onu_port_counter_t;


typedef enum{
	PORT_ADMIN_DOWN,
	PORT_ADMIN_UP
}gwd_port_admin_t;


typedef struct gw_tm {
    int tm_sec;      // seconds after the minute - [0..61] 
                     //   (note 61 allows for two leap seconds)
    int tm_min;      // minutes after the hour - [0..59]
    int tm_hour;     // hours since midnight - [0..23]
    int tm_mday;     // days of the month - [1..31]
    int tm_mon;      // months since January - [0..11]
    int tm_year;     // years since 1900
    int tm_wday;     // days since Sunday - [0..6]
    int tm_yday;     // days since January 1 - [0..365]
    int tm_isdst;    // Daylight Saving Time flag - positive if DST is in
                     // effect, 0 if not in effect, and negative if the info
                     // is not available
}localtime_tm;


#if 0
typedef struct{
	gw_uint64 gulBcStormThreshold;
	gw_uint64 gulBcStormStat;
}broadcast_storm_s;
#endif

broadcast_storm_s broad_storm;


enum{
	LIB_IF_ONU_LLID_GET,
	LIB_IF_SYSINFO_GET,
	LIB_IF_SYSCONF_SAVE,
	LIB_IF_SYSCONF_RESTORE,
	LIB_IF_PORTSEND,
	LIB_IF_OAM_HDR_BUILDER,

	LIB_IF_PORT_ADMIN_GET,
	LIB_IF_PORT_ADMIN_SET,
	LIB_IF_PORT_OPER_STATUS_GET,
	LIB_IF_PORT_MODE_GET,
	LIB_IF_PORT_MODE_SET,	
	LIB_IF_PORT_ISOLATE_GET,
	LIB_IF_PORT_ISOLATE_SET,
	LIB_IF_PORT_STATISTIC_GET,

	LIB_IF_VLAN_ENTRY_GETNEXT,
	LIB_IF_VLAN_ENTRY_GET,
	
	LIB_IF_FDB_ENTRY_GET,
	LIB_IF_FDB_ENTRY_GETNEXT,
	LIB_IF_FDB_MGT_MAC_SET,
	LIB_IF_ATU_LEARN_GET,
	LIB_IF_ATU_LEARN_SET,
	LIB_IF_ATU_AGE_GET,
	LIB_IF_ATU_AGE_SET,	

	LIB_IF_ONU_MAC_SET,

	LIB_IF_OPM_GET,
	LIB_IF_LASER_GET,
	LIB_IF_LASER_SET,
	
	LIB_IF_PORT_LOOP_EVENT_POST,
	LIB_IF_SPECIAL_PKT_HANDLER_REGIST,
	LIB_IF_SYSTERM_CURRENT_TIME_GET,
	LIB_IF_BROADCAST_SPEED_LIMIT,
	LIB_IF_LOCALTIME_GET,
	LIB_IF_STATIC_MAC_ADD,
	LIB_IF_STATIC_MAC_DEL,
	LIB_IF_ONU_REGISTER_GET,
	//LIB_IF_ONU_REBOOT,

};


typedef enum {
        GW_PORT_ADMIN_DOWN,
        GW_PORT_ADMIN_UP,
} gw_port_admin_status_t;


enum{
	GW_LOG_LEVEL_DEBUG,
	GW_LOG_LEVEL_INFO,
	GW_LOG_LEVEL_MINOR,
	GW_LOG_LEVEL_MAJOR,
	GW_LOG_LEVEL_CRI,
};



#define gw_printf			cs_printf
#define gw_thread_delay		cs_thread_delay

static gw_int32 gw_log(gw_int32 level, const gw_int8 *String, ...)
{
	return 0;
}

extern cs_status port_storm_limit_set(cs_port_id_t port, cs_sdl_rl_mod_e mode, cs_sdl_storm_ctrl_e type, cs_sdl_policy_t rate);
extern cs_status app_onu_port_stats_get(
        cs_port_id_t port_id,
        oam_port_uni_stats_t *uni_stats);

static cs_status onu_port_stats_get_sdl(cs_port_id_t port_id, char *uni_status)
{
#if 0
	cs_printf("in onu_port_stats_get_sdl\n");
#endif

	cs_status ret = CS_E_OK;
	gw_onu_port_counter_t *pd = NULL;
	pd = (gw_onu_port_counter_t *)uni_status;
	oam_port_uni_stats_t uni_stats;
	cs_port_id_t port = port_id;
	ret = app_onu_port_stats_get(port, &uni_stats);
	pd->counter.RxBroadcasts = uni_stats.rxbcfrm_cnt;
	pd->counter.TxBroadcasts = uni_stats.txbcfrm_cnt;
	return ret;
}

static cs_status current_time_get(cs_uint64 *time)
{
#if 0
	cs_printf("in current_time_get\n");
#endif

	cs_status ret = CS_E_OK;
	*time = cs_current_time();
	return ret;
}



extern cs_status epon_request_onu_port_admin_get(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_OUT cs_sdl_port_admin_t      *admin
);

static cs_status gwd_onu_sw_port_admin_status_get(cs_uint32 logical_port, gwd_port_admin_t *port_status)
{
#if 0
	cs_printf("in gwd_onu_sw_port_admin_status_get\n");
#endif
	cs_status ret = CS_E_OK;
	cs_callback_context_t context;
	cs_int32 device_id = 0;
	cs_int32 llidport = 0;
	cs_port_id_t port_id;
	cs_sdl_port_admin_t admin;

	device_id = 0;
	llidport = 0;
	port_id = logical_port;

	ret = epon_request_onu_port_admin_get(context, device_id, llidport, port_id, &admin);
	if(SDL_PORT_ADMIN_UP == admin)
	{
		*port_status = PORT_ADMIN_UP;
	}
	else
	{
		*port_status = PORT_ADMIN_DOWN;
	}
	
	return ret;
}

extern cs_status epon_request_onu_port_admin_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_port_id_t              port_id,
    CS_IN cs_sdl_port_admin_t       admin
);

static cs_status gwd_onu_sw_port_admin_status_set(cs_uint32 logical_port, gwd_port_admin_t port_status)
{
#if 0
	cs_printf("in gwd_onu_sw_port_admin_status_set\n");
#endif

	cs_status ret = CS_E_OK;
	cs_callback_context_t context;
	cs_int32 device_id;
	cs_int32 llidport;
	cs_port_id_t port_id;
	cs_sdl_port_admin_t admin;

	device_id = 0;
	llidport = 0;
	port_id = logical_port;

	if(PORT_ADMIN_UP == port_status)
	{
		admin = SDL_PORT_ADMIN_UP;
	}
	else
	{
		admin = SDL_PORT_ADMIN_DOWN;
	}
	ret = epon_request_onu_port_admin_set(context, device_id, llidport, port_id, admin);
	return ret;
}

#if 0
static cs_status epon_onu_sw_set_port_stormctrl()
{
	cs_status ret = CS_E_OK;
	
	return ret;	
}
#endif

#endif

extern int CommOnuMsgSend(unsigned char GwOpcode, unsigned int SendSerNo, unsigned char *pSentData,const unsigned short SendDataSize, unsigned char  *pSessionIdfield);
#if 0
extern gw_macaddr_t g_sys_mac;
extern gw_uint32 g_uni_port_num;
#else
#endif


gw_uint64 gulCurrentpktCntIn[NUM_PORTS_PER_SYSTEM-1] = {0};
gw_uint64 gulCurrentpktCntOut[NUM_PORTS_PER_SYSTEM-1] = {0};
gw_uint64 gulOctRateIn[NUM_PORTS_PER_SYSTEM-1] = {0};
gw_uint64 gulOctRateOut[NUM_PORTS_PER_SYSTEM-1] = {0};
gw_uint64 gulLastTick4PortMon[NUM_PORTS_PER_SYSTEM-1] = {0};
gw_uint64 gulCurrentTick4PortMon[NUM_PORTS_PER_SYSTEM-1] = {0};
#define ENABLE 1

gw_return_code_t gwd_onu_port_bcstorm_date_clear(gw_uint32 port)
{
	gulCurrentpktCntOut[port]= 0;
	gulCurrentpktCntIn[port] = 0;
	gulOctRateOut[port] = 0;
	gulOctRateIn[port] = 0;
	gulCurrentTick4PortMon[port] = 0;
	gulLastTick4PortMon[port] = 0;
	return GW_RETURN_SUCCESS;
}
gw_return_code_t gwd_onu_sw_bcstorm_msg_send(gw_uint32 slot, gw_uint32 port,gw_uint8 operate, gw_uint8 state,gw_uint8*session)
{
	char temp[16]={0};

	*(temp) = 22;
	*(temp+3) = slot;
	*(temp+4) = port;
	*(temp+5) = state;
	*(temp+6) = operate;
	if(GW_OK != CommOnuMsgSend(ALARM_REQ,0,temp,7, session))
		{
			gw_printf("Send ALARM_REQ for port %d broadcast storm detect trap failed\n", port);
			return GW_RETURN_FAIL;
		}
	return GW_RETURN_SUCCESS;
}
gw_return_code_t gwd_logical_to_physical(gw_uint32 lport, gw_uint32 *pport)
{
    *pport = (lport-1);

    /* check whether logical port number is legal or not */
    if ((*pport < 0) || (*pport >= NUM_PORTS_PER_SYSTEM)) {
        return GW_RETURN_FAIL;
    }
    return GW_RETURN_SUCCESS;
}
gw_return_code_t gwd_port_current_pkt_status_save(gw_uint32 port,gw_onu_counter_t stat_date)
{
	gulCurrentpktCntIn[port] = stat_date.RxBroadcasts;
	gulOctRateOut[port] = stat_date.TxBroadcasts;
	return GW_RETURN_SUCCESS;
}
gw_return_code_t gwd_port_rate_update(gw_uint32 port)
{
	gw_uint64 ulIntervalTick;
	gw_float fRate;
	gw_int32 len = sizeof(gw_onu_port_counter_t);
	gw_onu_port_counter_t * pd;
	char *data = malloc(len);
	if (data) {
			memset(data, 0, len);
			#if 0
			if (GW_OK != call_gwdonu_if_api(LIB_IF_PORT_STATISTIC_GET, 3, port+1,
							data, &len))
			//端口统计
			#else
			//端口统计
			cs_status ret = CS_E_OK;
			cs_port_id_t port_id;
			port_id = port + 1;
			#if 0
			cs_printf("port_id :0x%x\n", port_id);
			#endif
			ret = onu_port_stats_get_sdl(port_id, data);
			if(CS_E_OK != ret)
				
			#endif
				{
					gw_printf("get port %d statistic fail!\r\n", port+1);
				}
			else
				{
					 pd = (gw_onu_port_counter_t*)data;
				}
		}
#ifdef __NOT_USE__
	if(!gwd_portstats_get_current(port, stat_date)){
	gw_printf("get gwd current port status fail\n");
	return GW_RETURN_FAIL;
	}
#endif

	#if 0
	call_gwdonu_if_api(LIB_IF_SYSTERM_CURRENT_TIME_GET, 1, &gulCurrentTick4PortMon[port]);
	//获取系统时间
	#else
	//获取系统时间
	current_time_get(&gulCurrentTick4PortMon[port]);
	#endif
#ifdef __NOT_USE__
	gulCurrentTick4PortMon[port] = cyg_current_time();
#endif
	if (gulCurrentTick4PortMon[port] > gulLastTick4PortMon[port])
		{
    		ulIntervalTick = gulCurrentTick4PortMon[port] - gulLastTick4PortMon[port];
			#ifdef __DEBUG__
			if(port == 0)
				{
					gw_printf("tick:%lld\n",ulIntervalTick);
				}
			#endif
		}
    else
    	{
    		ulIntervalTick = 0xFFFFFFFF - (gulLastTick4PortMon[port] - gulCurrentTick4PortMon[port]);
			#ifdef __DEBUG__
			if(port == 0)
				{
					gw_printf("else tick:%lld\n",ulIntervalTick);
				}
			#endif
    	}

	if (pd->counter.RxBroadcasts >= gulCurrentpktCntIn[port] )
	{
	#ifdef __DEBUG__
		if(port == 0)
			{
				gw_printf("RX RATE:%lld\n",pd->counter.RxBroadcasts);
				gw_printf("His RATE:%lld\n",gulCurrentpktCntIn[port]);
				gw_printf("rate:%lld\n",(pd->counter.RxBroadcasts - gulCurrentpktCntIn[port]));
			}
	#endif
		fRate = (gw_float)((pd->counter.RxBroadcasts - gulCurrentpktCntIn[port]))/(gw_float)ulIntervalTick*100;
	}
	else
	{
	#ifdef __DEBUG__
			if(port == 0)
			{
				gw_printf("else RX RATE:%lld\n",pd->counter.RxBroadcasts);
				gw_printf("else His RATE:%lld\n",gulCurrentpktCntIn[port]);
				gw_printf("else rate:%lld\n",(pd->counter.RxBroadcasts - gulCurrentpktCntIn[port]));
			}
	#endif
		fRate = (gw_float)((0xFFFFFFFF - (gulCurrentpktCntIn[port] - pd->counter.RxBroadcasts)))/(gw_float)ulIntervalTick*1000;
	}
	gulCurrentpktCntIn[port] = pd->counter.RxBroadcasts;
	#ifdef __DEBUG__
	if(port == 0)
		{
			gw_printf("frate:%lld\n",(gw_uint64)fRate);
		}
	#endif
	gulOctRateIn[port] = (gw_uint64)fRate;
	if ( pd->counter.TxBroadcasts >= gulCurrentpktCntOut[port] )
	{
		fRate = (gw_float)((pd->counter.TxBroadcasts - gulCurrentpktCntOut[port]))/(gw_float)ulIntervalTick*100;
	}
	else
	{
		fRate = (gw_float)((0xFFFFFFFF - (gulCurrentpktCntOut[port] - pd->counter.TxBroadcasts)))/(gw_float)ulIntervalTick*1000;
	}
	gulCurrentpktCntOut[port] = pd->counter.TxBroadcasts;
	gulOctRateOut[port] = (gw_uint64)fRate;
	gulLastTick4PortMon[port] = gulCurrentTick4PortMon[port];
	free(data);
	return GW_RETURN_SUCCESS;

}
#ifndef UNI_PORT_MAX
#define UNI_PORT_MAX            CS_UNI_NUMBER
#endif
void broad_storm_thread(void* data)
{
#if 1
	cs_printf("broad storm thread created\n");
#endif
	gw_uint32 logical_port;
	gw_uint32 physical_port;
	gwd_port_admin_t port_status;
	gw_return_code_t ret;
	gw_uint32 slot = 1;
	gw_uint8 OAMsession[8]="";
	gw_uint16 ulBcStormEventCnt[NUM_PORTS_PER_SYSTEM-1] = {0};
    gw_uint16 ulBcStormStopCnt[NUM_PORTS_PER_SYSTEM-1] = {0};
	gw_uint16 havebroadcaststorm[NUM_PORTS_PER_SYSTEM-1] = {0};
	gw_uint16 havebroadcaststorm_end[NUM_PORTS_PER_SYSTEM-1] = {0};
	gw_uint16 timeCouter[NUM_PORTS_PER_SYSTEM-1] = {0};
	gw_uint16 startCouter[NUM_PORTS_PER_SYSTEM-1] = {0};
	#if 0
	call_gwdonu_if_api(LIB_IF_SYSINFO_GET, 2,  g_sys_mac, &g_uni_port_num);
	//功能?
	#else
	//功能?
	#endif
//	localtime_tm tm;
	while(true)
	{
		for(logical_port=1; logical_port < UNI_PORT_MAX+1; logical_port++)
		{
			if(gwd_logical_to_physical(logical_port,&physical_port))
			{
				return;
			}
			#if 0
			ret = call_gwdonu_if_api(LIB_IF_PORT_OPER_STATUS_GET, 2, logical_port, &port_status);
			//获取端口状态
			#else
			//获取端口状态
			ret = gwd_onu_sw_port_admin_status_get(logical_port,&port_status);
			if(CS_E_OK != ret)
			{
				#if 1
				cs_printf("gwd_onu_sw_port_admin_status_get failed, return :0x%x\n", ret);
				#endif
				return;
			}
			else
			{
				//do nothing
				#if 0
				cs_printf("gwd_onu_sw_port_admin_status_get success, port_status :0x%x\n", port_status);
				#endif
			}
			#endif
			
				
			if(port_status == GW_PORT_ADMIN_UP)
			{
				if(gwd_port_rate_update(physical_port))
				{
					return;
				}
				else
				{
					//do nothing
				}
				
				#ifdef __DEBUG__
				if(logical_port == 1)
				{
					cs_printf("port rate:%d\n",gulOctRateIn[physical_port]);
				}
				#endif
				
				//gwd_port_current_pkt_status_save(physical_port,dsts);
				if(gulOctRateIn[physical_port] > broad_storm.gulBcStormThreshold)
				{

					ulBcStormEventCnt[physical_port]++;
					ulBcStormStopCnt[physical_port] = 0;
                    timeCouter[physical_port] = 0;
					
					#ifdef __DEBUG__
					if(logical_port == 1)
					{					
						cs_printf("event count:%d (line:%d)\n",ulBcStormEventCnt[physical_port],__LINE__);					
					}
					#endif
				
				}
				else
				{
					ulBcStormEventCnt[physical_port] = 0;
					if(startCouter[physical_port] == 0)
					{
                    	timeCouter[physical_port] = 0;
					}
                    else
                    {
						timeCouter[physical_port]++;
                    }
					if(havebroadcaststorm_end[physical_port])
					{
						ulBcStormStopCnt[physical_port]++;
						if(ulBcStormStopCnt[physical_port] > 3)
						{
							#if 0
						    call_gwdonu_if_api(LIB_IF_BROADCAST_SPEED_LIMIT, 3, logical_port, 3,0);
							//端口限速
							#else
							cs_status ret = CS_E_OK;
							cs_port_id_t port;
							cs_sdl_rl_mod_e mode;
							cs_sdl_storm_ctrl_e type;
							cs_sdl_policy_t rate;

							port = logical_port;
							mode = SDL_RL_MOD_BPS;
							type = SDL_STORM_TYPE_BC;
							rate.enable = 0;
							rate.rate = 0;

							
						#ifdef __DEBUG__
						if(logical_port == 1)
						{					
							cs_printf("STOP STORM LIMIT NOW\n");					
						}
						#endif
							
							ret = port_storm_limit_set(port, mode, type, rate);
							if(CS_E_OK != ret)
							{
								cs_printf("port_storm_limit_set faied\n");
							}
							else
							{
								//do nothing
								#if 0
								cs_printf("port_storm_limit_set success\n");
								#endif
							}
							
							#endif
							
							startCouter[physical_port] = 1;
							timeCouter[physical_port] = 0;
							havebroadcaststorm_end[physical_port]=0;
							havebroadcaststorm[physical_port] = 0;
						}
						else
						{
							//do nothing
						}
					}
					else
					{
						//do nothing
					}
				}
				if(ulBcStormEventCnt[physical_port] > 3)
				{
					if(broad_storm.gulBcStormStat == ENABLE)
					{
						#if 0
						ret = call_gwdonu_if_api(LIB_IF_PORT_ADMIN_SET, 2, logical_port, GW_PORT_ADMIN_DOWN);
						//设置端口状态
						#else
						//设置端口状态
						#ifdef __DEBUG__
						if(logical_port == 1)
						{					
							cs_printf("PORT STOM STAT ENABLE\n");					
						}
						#endif
						gwd_port_admin_t status;
						status = GW_PORT_ADMIN_DOWN;
						ret = gwd_onu_sw_port_admin_status_set(logical_port,status);
						if(CS_E_OK != ret)
						{
							cs_printf("gwd_onu_sw_port_admin_status_set fail, ret: 0x%x\n", ret);
							return;
						}
						else
						{
							#if 0
							cs_printf("gwd_onu_sw_port_admin_status_set success, status :0x%x\n", status);
							#endif
						}
						#endif
						
						gwd_onu_sw_bcstorm_msg_send(slot, logical_port, 2, 1,OAMsession);
						//gw_printf("shutdown gwd onu port %d\n",logical_port);
						//gw_time_get(&tm);
						gw_log(GW_LOG_LEVEL_MAJOR,"Interface  eth1/%d detected Broadcast Storm,port shutdown",logical_port);
					}
					else
					{
						if(!havebroadcaststorm[physical_port])
						{
							#if 0
							if(call_gwdonu_if_api(LIB_IF_BROADCAST_SPEED_LIMIT, 3, logical_port, 3,64))
							//端口限速
							#else
								//端口限速
							
							#ifdef __DEBUG__
							if(logical_port == 1)
							{					
								cs_printf("PORT STORM SEEP LIMIT NOW\n");					
							}
							#endif
							
							cs_status ret = CS_E_OK;
							cs_port_id_t port;
							cs_sdl_rl_mod_e mode;
							cs_sdl_storm_ctrl_e type;
							cs_sdl_policy_t rate;

							port = logical_port;
							mode = SDL_RL_MOD_BPS;
							type = SDL_STORM_TYPE_BC;
							rate.enable = 1;
							rate.rate = 64;
							
							ret = port_storm_limit_set(port, mode, type, rate);
							if(CS_E_OK != ret)
							{
								gw_printf("Broadcast storm speed limit failure\n");
								gw_printf("port_storm_limit_set failed\n");
							}
							else
							{
								#if 0
								cs_printf("port :0x%x\n", port);
								cs_printf("port_storm_limit_set success\n");
								#endif
							}
							#endif
								
							gwd_onu_sw_bcstorm_msg_send(slot, logical_port, 1, 1,OAMsession);

							gw_log(GW_LOG_LEVEL_MAJOR,"Interface  eth1/%d detected Broadcast Storm,rate limited to 64K.",logical_port);
							
							havebroadcaststorm[physical_port] = 1;
							havebroadcaststorm_end[physical_port]=1;
						}
						else
						{
							//do nothing
						}
					}
					ulBcStormEventCnt[physical_port] = 0;
				}
				if(timeCouter[physical_port] > 2)
				{
					gwd_onu_sw_bcstorm_msg_send(slot, logical_port, 1, 2,OAMsession);
					if(gulOctRateIn[physical_port])
					{
						//gw_time_get(&tm);
						gw_log(GW_LOG_LEVEL_MAJOR,"Interface  eth1/%d Broadcast Storm stopped, rate back to %dKbps.",logical_port,gulOctRateIn[physical_port]);
						//gw_log(GW_LOG_LEVEL_MAJOR,"Interface  eth0/%d Broadcast Storm stopped, rate back to %dKbps.",logical_port,gulOctRateIn[physical_port]);
					}
					else
					{
						//gw_time_get(&tm);
						gw_log(GW_LOG_LEVEL_MAJOR,"Interface  eth1/%d Broadcast Storm stopped, rate back to No Limit.",logical_port,gulOctRateIn[physical_port]);
						//gw_log(GW_LOG_LEVEL_MAJOR,"Interface  eth0/%d Broadcast Storm stopped, rate back to No Limit.",logical_port);
					}
					startCouter[physical_port] = 0;
					timeCouter[physical_port] = 0;
					//havebroadcaststorm[physical_port] = 0;
				}
				else
				{
					//do nothing
				}
			}
			else
			{
				gwd_onu_port_bcstorm_date_clear(physical_port);
			}
		}
		gw_thread_delay(10000);
	}
	gw_printf("=======================================\n");
	gw_printf("====Broadcast storm thread exit========\n");
	gw_printf("=======================================\n");
}

extern cs_status port_bc_storm_no_limit(cs_port_id_t port)
{
	cs_status ret = CS_E_OK;
	cs_sdl_rl_mod_e mode;
	cs_sdl_storm_ctrl_e type;
	cs_sdl_policy_t rate;

	mode = SDL_RL_MOD_BPS;
	type = SDL_STORM_TYPE_BC;
	rate.enable = 0;
	rate.rate = 0;
	ret = port_storm_limit_set(port, mode, type, rate);
	if(CS_E_OK != ret)
	{
		gw_printf("Broadcast storm speed limit failure\n");
		gw_printf("port_storm_limit_set failed\n");
	}
	else
	{
		#if 1
		cs_printf("port :%d,broadcast packet no limit success\n", port);
		#endif
	}
	return ret;
}

cs_status uni_port_num_get(int *num);
extern cs_status all_port_bc_storm_no_limit(void)
{
	cs_status ret = CS_E_OK;
	cs_port_id_t port;
	int i = 0;
	int uni_num = 0;

	uni_port_num_get(&uni_num);
	for(i=0;i<uni_num;i++)
	{
		port = CS_UNI_PORT_ID1 + i;
		port_bc_storm_no_limit(port);
	}
	return ret;
}

