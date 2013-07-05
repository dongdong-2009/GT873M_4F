#ifndef _VOS_IO_H
#define _VOS_IO_H
#include "onu_datetype.h"

unsigned long ECOS_Sprintf(char *buf ,char *fmt, ...);
int linux_vsnprintf(char *buf, unsigned long size, const char *fmt, va_list args);
int linux_vsprintf(char *buf, const char *fmt, va_list args);

#define sprintf ECOS_Sprintf


#endif

