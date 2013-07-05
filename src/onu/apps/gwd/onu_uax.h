#ifndef __ONU_AUX__
#define __ONU_AUX__

#include "onu_datetype.h"
#include "onu_cli.h"

//#define SYS_MASTER_ACTIVE_SLOTNO (sys_state.masteractive_slotno)
#define  MAX_PORT_NUMBER_IN_PORTLIST 100
#define IFM_LICA_MAX_SLOT (MAXSlotNum)
#define IFM_LICA_MAX_PORT (MAXPortOnSlotNum)
#ifdef _SAVE_MEM_
#define IFM_ABILITY_COUNT      (1)
#else
#define IFM_ABILITY_COUNT      (32)
#endif
#define ETH_FILTER_MAX_PORT_NUM_ON_SLOT     ( 4 )
#define VTY_NEWLINE "\r\n"

typedef struct tagDynaArray
{
	USHORT  dem1cap,
	        dem2cap;    /* 每个维的大小,4字节为单位 */
	USHORT  dem1cur;    /* 当前维的值 */
#ifdef _DISTRIBUTE_PLATFORM_
    USHORT  unused;
#endif
	ULONG   **pp;       /* 指向UNONG的二维数组 */
}DynaArray_S;

typedef struct tagIFM_Index_2_NetDAddress_Map
{
    ULONG           ulMaxID;
    ULONG           ulIfCount;
    DynaArray_S     *pDa;
}IFM_ID2ADDR_MAP_S;
typedef struct tagIFM_Ability
{
    ULONG alWhyNoAbility[IFM_ABILITY_COUNT];
}IFM_IF_ABILITY_S;

typedef struct tagExtend_IP
{
    ULONG ulIP;
    ULONG ulMask;
    UCHAR aucDevAddr[ IFM_PHY_ADD_LEN ];
    UCHAR ucActive;
    UCHAR ucExtendIpFlag; /*IFM_EXTIP_VRRP ,IFM_EXTIP_HSRP*/
    struct tagExtend_IP * pNext;
}IFM_EXTEND_IP_S;

typedef struct tagIP_Info
{
    IFM_EXTEND_IP_S     stExtendIP;
    VOID                *pIpInfo;
    ULONG               ulVpnInfo;

}IFM_IP_INFO_S;
typedef struct net_device
{
   	unsigned long               ulIfIndex;                          /* 接口索引 */
    unsigned long               ulUsed;                             /* 是否已使用位 */
    unsigned long               flags;
    unsigned long               ulMTU;
    VOID                *pPrivateData;                      /* 跟具体接口类型相关的私有数据 */
    IFM_IP_INFO_S       stIpInfo;
    IFM_IF_ABILITY_S    stAbility;
    UCHAR               ucPhyState;                         /* 接口的物理状态 */
    UCHAR               ucDefaultAdminState;
    UCHAR               ucAdminState;                       /* 接口的管理状态 */
    CHAR                szName[ IFM_NAME_SIZE + 1 ];        /* 接口名 eg.eth2/1 */
    CHAR                szIfDescr[ IFM_MAX_DESCR_LEN + 1 ]; /* 接口描述 */
}IFM_NET_DEVICE_S;

typedef union tagIFM_If_Index
{
    struct tagBit
    {
#if VOS_LITTLE_ENDIAN
        unsigned other:26;
        unsigned type:6;
#else
        unsigned type:6;
        unsigned other:26;
#endif		
    }bitIfIndex;
    unsigned long ulIfIndex;
}IFM_IF_INDEX_U;

struct slot_port{
	//void*  onuPort;
	unsigned long	ulSlot;
	unsigned long	ulPort;
};
struct roter_slot_port{
	void*  roterPort;
	unsigned long	ulPort;
};

#define BEGIN_PARSE_PORT_LIST_TO_PORT_NO_CHECK(portlist, ifindex,devonuport_num) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = ETH_ParsePortList(portlist,devonuport_num);\
    if(!_pulIfArray)\
    	{\
    		ifindex = 0;\
    	}\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            ifindex = _pulIfArray[_i];\

#define END_PARSE_PORT_LIST_TO_PORT_NO_CHECK() \
        }\
        iros_free(_pulIfArray);\
    }\
}

#define ETH_SLOTPORT_TO_PORTNO( _slot, _port )           \
          ( (  ( _slot )  - 1 ) * ( ETH_FILTER_MAX_PORT_NUM_ON_SLOT ) + (  ( _port )  ) )
            

typedef struct tagIFM_Interface_Function
{
    long( *init ) ( ULONG * ulIfindex, VOID **ppData, CHAR * szName, ULONG ulNameLen, VOID * pData, CHAR ** ppError );
    long( *destroy ) ( VOID * pData, CHAR ** ppError );
    long( *registe ) ( ULONG ulIfindex, VOID * pData, CHAR ** ppError );
    long( *unregiste ) ( ULONG ulIfindex, VOID * pData, VOID ** ppData, CHAR ** ppError );
    long( *phy_up ) ( ULONG ulIfindex, VOID * pData, CHAR ** ppError );
    long( *phy_down ) ( ULONG ulIfindex, VOID * pData, CHAR ** ppError );
    long( *admin_up ) ( ULONG ulIfindex, VOID * pData, CHAR ** ppError );
    LONG( *admin_down ) ( ULONG ulIfindex, VOID * pData, CHAR ** ppError );
    LONG( *config ) ( ULONG ulIfindex, ULONG ulCode, VOID * pData, CHAR ** ppError );
    LONG( *get_info ) ( ULONG ulIfindex, ULONG ulCode, VOID * pData, CHAR ** ppError );
    LONG( *input ) ( ULONG ulAccepter, ULONG ulSender, VOID * pData, CHAR ** ppError );
    LONG( *output ) ( ULONG ulAccepter, ULONG ulSender, VOID * pData, CHAR ** ppError );
    LONG( *join ) ( ULONG ulMember, ULONG ulHost, VOID * pData, CHAR ** ppError );
    LONG( *leave ) ( ULONG ulLeaver, ULONG ulHurted, VOID * pData, CHAR ** ppError );
    LONG( *accepted ) ( ULONG ulMember, ULONG ulHost, VOID * pData, CHAR ** ppError );
    LONG( *abandoned ) ( ULONG ulMember, ULONG ulHost, VOID * pData, CHAR ** ppError );
    LONG( *sub_down ) ( ULONG ulMember, ULONG ulHost, CHAR ** ppError );
    LONG( *sub_up ) ( ULONG ulMember, ULONG ulHost, CHAR ** ppError );
    LONG( *keepalive ) ( ULONG ulIfindex, VOID * pData, CHAR ** ppError );
    LONG( *ext ) ( ULONG ulIfindex, ULONG ulCode, VOID ** ppData, VOID * pData, CHAR ** ppError );
    LONG( *show ) ( ULONG ulIfindex, ULONG ulCode, CHAR ** ppOutBuf, CHAR ** ppError );
    LONG( *show_run ) ( ULONG ulIfindex, VOID * pvty );
    LONG( *index_2_arrayid ) ( ULONG ulIfindex, ULONG * pulArrayID, CHAR ** szErrInfo );
    LONG( *arrayid_2_index ) ( ULONG ulArrayID, ULONG * pulIfindex, CHAR ** szErrInfo );
    LONG( *check_name ) ( CHAR * szName );
    LONG( *insert_netdev ) ( ULONG ulIfindex, IFM_NET_DEVICE_S * pNetd, CHAR ** szErrInfo );
    LONG( *delete_netdev ) ( ULONG ulIfindex, CHAR ** szErrInfo );
    LONG( *find_netdev ) ( ULONG ulIfindex, IFM_NET_DEVICE_S ** ppNetd, CHAR ** szErrInfo );
}IFM_INTERFACE_FUNCTION_S;

struct slot_port * BEGIN_PARSE_PORT_EAND_SLOT(char * argv, struct slot_port* my_onu,char *ifname,struct cli_def*cli_i );
unsigned long  BEGIN_PARSE_TO_PORT(char *argv);
unsigned long * ETH_ParsePortList(char * pcPortList,unsigned long num);
unsigned long LICA_GetSlotIFMType( unsigned long ulSlot );
long IFM_find_netdev( unsigned long ulIfindex, IFM_NET_DEVICE_S ** ppNetd, char ** szErrInfo );
void    cli_set_prompt(struct cli_def *, char *cpPrompt);
long IFM_find_netdev( unsigned long ulIfindex, struct net_device ** ppNetd, char ** szErrInfo );
unsigned long IFM_IFINDEX_GET_TYPE( unsigned long ulIfindex );
long IFM_GetTypeFunc( ULONG ulType, struct tagIFM_Interface_Function ** ppfInterfaceFunction );
unsigned long IFM_ETH_GET_PORT( unsigned long ulIfIndex );
unsigned long IFM_ETH_GET_SLOT( unsigned long ulIfIndex );
unsigned long IFM_ETH_CREATE_INDEX( unsigned long ulSlot, unsigned long ulPort );
struct cli_command * install_cmdelement(struct cli_command** , struct cmd_element * );
struct my_string* cli_judgment_achieve(struct my_string* ,char *,char *);
long cmd_CheckCommand(char *);
long GetMacAddr( char * szStr, char * pucMacAddr );
#endif

