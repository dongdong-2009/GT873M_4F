#ifndef __GWD_POE_H
#define __GWD_POE_H
#include "iros_config.h"

#ifndef UNI_PORT_MAX
#define UNI_PORT_MAX            CS_UNI_NUMBER
#endif

#define UNLOCK_CPLD 0x55
#define LOCK_CPLD 0
#define GWD_CPLD_VERSION_1 0x1


#define POE_PORT_ENABLE		1
#define POE_PORT_DISABLE	0
#define POE_POWER_ENABLE	1
#define POE_POWER_DISABLE	0

#define PORT_POWER_STAT(power_stat,port) power_stat &=(1 << (port-1))

typedef enum{
	PORT_ADMIN_DOWN,
	PORT_ADMIN_UP
}gwd_port_admin_t;


typedef enum {
    EPON_RETURN_OK = 0,
    EPON_RETURN_ERROR,
    EPON_RETURN_EXIST_OK,
    EPON_RETURN_EXIST_ERROR
} epon_return_code_e;

typedef enum{
    GWD_CPLD_VERSION_REG = 0x00,
    GWD_CPLD_RESERVED_REG = 0x01,
    GWD_CPLD_POWER3_REG = 0x02,
    GWD_CPLD_RESET_REG = 0x03,
    GWD_CPLD_POWER2_REG = 0x04,
    GWD_CPLD_POWER_ALARM_REG = 0x05,
    GWD_CPLD_POWER1_REG = 0x06,
    GWD_CPLD_PROTECT_REG = 0x07
}cpld_register_t;

typedef struct Poe_operation_s{
	unsigned int PoeOperationflag;
	unsigned int PoeOpertable[UNI_PORT_MAX];
	unsigned int CommandOperation[UNI_PORT_MAX];
}PoeOperation_t;

void gwd_poe_init();
extern int gwd_onu_tlv_infor_handle(int length,char *value,int opcode);
extern int gwd_onu_tlv_infor_get(int *len, char **value, int *free_need);
extern epon_return_code_e Gwd_onu_poe_exist_stat_get(unsigned int *poe_stat);

#endif



