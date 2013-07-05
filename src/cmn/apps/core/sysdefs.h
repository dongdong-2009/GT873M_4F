/* Copyright: (c) 2005 ImmenStar Inc. All Rights Reserved. */

#ifndef _SYSDEFS_H_
#define _SYSDEFS_H_

#include <pkgconf/system.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef CYGPKG_HAL_ARM

#include <time.h>
#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>


#ifndef _SYS_BSDTYPES_H_
typedef __signed char              int8_t;
typedef unsigned char             uint8_t;
typedef short                     int16_t;
typedef unsigned short           uint16_t;
typedef int                       int32_t;
typedef unsigned int             uint32_t;
typedef long long                 int64_t;
typedef unsigned long long       uint64_t;
typedef unsigned char u_char;

typedef unsigned int in_addr_t;
#endif
#ifndef _NETINET_IN_H_
struct in_addr {
        in_addr_t s_addr;
};
#endif
#endif

#endif
