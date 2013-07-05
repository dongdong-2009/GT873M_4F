#ifndef __ONU_CLI_H_
#define __ONU_CLI_H_

#if 0
typedef 	unsigned int	UINT;
typedef		unsigned char	UCHAR;
typedef		unsigned short	USHORT;
typedef		unsigned long	ULONG;
typedef 	short			SHORT;
typedef		int 			BOOL;
typedef		signed int		INT;
typedef		char 			CHAR;
typedef 	long			LONG;
#endif


#ifndef UINT
#define UINT unsigned int
#endif

#ifndef UCHAR
#define UCHAR unsigned char
#endif

#ifndef SHORT
#define SHORT short
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef INT
#define INT  signed int
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef LONG
#define LONG long
#endif

/*安全起见可以定为32，定为31可以节省空间和进行测试*/
#if (FOR_MRV_BOX_PON)
#define IFM_NAME_SIZE                  (16)
#define IFM_API_NAMELEN                (31)
#define IFM_PHY_ADD_LEN                (6)
#define IFM_MAX_DESCR_LEN              (27)
#else
/*
** 在机架式设备中，为保证主备板间传送的消息定义的一致性，
** 必须保证这些定义在所有板之间是一致的。
*/
#define IFM_NAME_SIZE                  (30)
#define IFM_API_NAMELEN                (31)
#define IFM_PHY_ADD_LEN                (6)
#define IFM_MAX_DESCR_LEN              (63)
#endif
#define COMMAND_SUCCESS              0
#define CMD_WARNING              1
#ifndef TRUE
#define TRUE                (1)
#endif
#ifndef FALSE
#define FALSE               (0)
#endif
#ifndef ULONG
#define ULONG unsigned long
#endif 

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef VOID
 #define VOID void
#endif

#ifndef ECOS_ERROR
#define ECOS_ERROR (-1)
#endif
#ifndef ECOS_OK
#define ECOS_OK 0
#endif

#endif
