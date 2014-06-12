#ifndef __GWD_POE_H
#define __GWD_POE_H

#include "cs_type.h"

#define UNLOCK_CPLD 0x55
#define LOCK_CPLD 0
#define GWD_CPLD_VERSION_1 0x1

typedef enum {
    EPON_RETURN_SUCCESS = 0,
    EPON_RETURN_FAIL,
    EPON_RETURN_EXIST_OK,
    EPON_RETURN_EXIST_ERROR
} epon_return_code_t;

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

void gwd_poe_init();

#endif



