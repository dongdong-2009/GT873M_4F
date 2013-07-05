#ifndef __RCP_SET_MODE__
#define __RCP_SET_MODE__
int RCP_SetPortMode100F(struct _RCP_DEV * b, unsigned long c, unsigned long d, int a);
int RCP_SetPortMode100H(struct _RCP_DEV * b, unsigned long c, unsigned long d, int a);
int RCP_SetPortMode10F(struct _RCP_DEV * b, unsigned long c, unsigned long d, int a);
int RCP_SetPortMode10H(struct _RCP_DEV * b, unsigned long c, unsigned long d, int a);
int RCP_SetQosMapMode(struct _RCP_DEV * b, unsigned long c,  int a);
int RCP_GetLoopPort(   struct _RCP_DEV  *b,   unsigned long *a);
#endif

