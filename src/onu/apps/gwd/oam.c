#include "iros_config.h"
#include <string.h>
#include <math.h>
#include "cli_common.h"
#include "oam.h"
#include "timer.h"
#include "oam_core.h"
#include "mc_type.h"
#include "oam_supp.h"
#include "oam_client.h"
#include "cs_types.h"
#include "sdl_mpcp_cmn.h"
#include "aal_l2.h"
#include "sdl_fdb.h"
#include "sys_cfg.h"
#include "make_file.h"
#include "sockets.h"

#if (PRODUCTS == PRODUCTS_GT811D)
	const unsigned char SYS_SOFTWARE_MAJOR_VERSION_NO = 1;
	const unsigned char SYS_SOFTWARE_RELEASE_VERSION_NO = 1;
	const unsigned char SYS_SOFTWARE_BRANCH_VERSION_NO = 8;
	const unsigned char SYS_SOFTWARE_DEBUG_VERSION_NO = 1;

	const unsigned char SYS_HARDWARE_MAJOR_VERSION_NO = 2;
	const unsigned char SYS_HARDWARE_RELEASE_VERSION_NO = 1;
	const unsigned char SYS_HARDWARE_BRANCH_VERSION_NO = 1;
	const unsigned char SYS_HARDWARE_DEBUG_VERSION_NO = 1;
#endif

#if (PRODUCTS == PRODUCTS_GT811G)
	const unsigned char SYS_SOFTWARE_MAJOR_VERSION_NO = 1;
	const unsigned char SYS_SOFTWARE_RELEASE_VERSION_NO = 1;
	const unsigned char SYS_SOFTWARE_BRANCH_VERSION_NO = 7;
	const unsigned char SYS_SOFTWARE_DEBUG_VERSION_NO = 1;

	const unsigned char SYS_HARDWARE_MAJOR_VERSION_NO = 2;
	const unsigned char SYS_HARDWARE_RELEASE_VERSION_NO = 1;
	const unsigned char SYS_HARDWARE_BRANCH_VERSION_NO = 1;
	const unsigned char SYS_HARDWARE_DEBUG_VERSION_NO = 1;
#endif

#if (PRODUCTS == PRODUCTS_GT873_M_4F4S)
	const unsigned char SYS_SOFTWARE_MAJOR_VERSION_NO = 1;
	const unsigned char SYS_SOFTWARE_RELEASE_VERSION_NO = 1;
	const unsigned char SYS_SOFTWARE_BRANCH_VERSION_NO = 12;
	const unsigned char SYS_SOFTWARE_DEBUG_VERSION_NO = 1;

	const unsigned char SYS_HARDWARE_MAJOR_VERSION_NO = 2;
	const unsigned char SYS_HARDWARE_RELEASE_VERSION_NO = 1;
	const unsigned char SYS_HARDWARE_BRANCH_VERSION_NO = 1;
	const unsigned char SYS_HARDWARE_DEBUG_VERSION_NO = 1;
#endif

unsigned char gwdOamTrace = 0;
#define GWDOAMTRC               if(gwdOamTrace) diag_printf
cs_llid_t active_pon_port = CS_PON_PORT_ID;
cs_uint32 STOP_UART_SEND_RCE = 1;
#ifndef TOUPPER
#define TOUPPER(c)	(c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c)
#endif

char GwONU831BootVer[]="V1.0";
char GwOUI[3] = {0x00, 0x0f, 0xe9};
char CTCOUI[3] = {0x11, 0x11, 0x11}; 
char gwd_vendor_id[4] = { 'G', 'W', 'D', 'L' }; 

cs_uint32 uart_mode_atc = 0;
unsigned long file_op_msg_que = 0;
unsigned long cli_msg_que = 0;
GWTT_OAM_MESSAGE_NODE GwOamMessageListHead={0};
cyg_sem_t	OamListSem;
unsigned long gulOamSendSerNo = 1;
unsigned long	gulGwOamConnect = 0;

#ifndef PPPOE_RELAY_PACKET_DEBUG
#define PPPOE_RELAY_PACKET_DEBUG(str) if( gulDebugPppoeRrelay ){ OAMDBGERR str ;}
#endif

#ifndef DHCP_RELAY_PACKET_DEBUG
#define DHCP_RELAY_PACKET_DEBUG(str) if( gulDebugEthDhcpSwitch ){ OAMDBGERR str ;}
#endif

static int GwOamInformationRequest(GWTT_OAM_MESSAGE_NODE *pRequest );
static int GwOamMessageListNodeRem(GWTT_OAM_MESSAGE_NODE *pNode);
static int GwOamAlarmResponse(GWTT_OAM_MESSAGE_NODE *pRequest );
short int CtcOnuMsgReveive(CTC_OAM_MESSAGE_NODE **ppMessage,unsigned char *MessagData, unsigned short PayloadLength);

void OamMessageRecevTimeOut(epon_timer_t *timer);
int sys_show_version_descript_to_buf(char *buf);
char *mn_get_sysname(void);
unsigned long   Igmp_Snoop_OAM_Req_Recv(GWTT_OAM_MESSAGE_NODE *pRequest);
unsigned long   Igmp_Snoop_OAM_Resp_Recv(GWTT_OAM_MESSAGE_NODE *pResponse);
int Debug_Print_Rx_OAM(GWTT_OAM_MESSAGE_NODE *pMessage);
int Debug_Print_Tx_OAM(GWTT_OAM_HEADER *avender, unsigned char *pSentData);
int Debug_Print_Rx_OAM_Unkown(unsigned char *pBuffer, unsigned short len);

int cmd_igmp_disable(struct cli_def * cli, char *command, char *argv[], int argc);


unsigned long   gulDebugOamRx = 0;
unsigned long   gulDebugOamTx = 0;
unsigned long   gulDebugOamRxCount = 0;
unsigned long   gulDebugOamTxCount = 0;
unsigned long   gulDebugOamFileOp = 0;

OAM_ONU_LPB_DETECT_FRAME oam_onu_lpb_detect_frame, tframe;
unsigned long lpb_detect_timeout_timer = 0;

unsigned long gulIpAddrInEeprom;

ONU_SYS_INFO_TOTAL onu_system_info_total;
ONU_SYS_INFO_ABILITY onu_info_ability;
unsigned char *pStrGwdSwVer;
unsigned char *pStrGwdHwVer;

struct {
    epon_timer_t    timer;
} gwd_oam_timer;

unsigned char *irosbootver = "iROSBoot ONU 02.08.01 1286761672 Oct 11 2010";
int Onu_Sysinfo_Get(void);
int Onu_Sysinfo_Save(void);

int GwOamMessageListInit(void)
{
	cyg_semaphore_init(&OamListSem, 1);
	return 0;
}

//extern void oam_send(unsigned int port, unsigned char *buf, int len);
extern void oam_send(cs_llid_t llid,cs_port_id_t port, cs_uint8 *buf, cs_uint32 len);


/* Import functions from FLASH driver */
extern int user_data_config_Read(unsigned int offset, unsigned char *buffer, unsigned int size);
extern int user_data_config_Write(unsigned char *buffer, unsigned int size);

/* Import functions from SW driver*/
void swt_show_fdb_entry(cs_sdl_fdb_entry_t *entry);

/*******************************************************************
* GwOadMessageListGetNode
********************************************************************/

static GWTT_OAM_MESSAGE_NODE *GwOamMessageListGetNode(unsigned int SerNo)
{
	GWTT_OAM_MESSAGE_NODE *pNode;

	GWDOAMTRC("  GWD OAM handler - GwOamMessageListGetNode - SerNo: 0x%x.\n", SerNo);
	cyg_semaphore_wait(&OamListSem);
	GWDOAMTRC("  GWD OAM handler - GwOamMessageListGetNode - semWait: ok.\n");
	pNode = GwOamMessageListHead.next;
	while(NULL!=pNode)
	{
	
		if(SerNo==pNode->SendSerNo)
		{
			break;
		}
		else
		{
			pNode=pNode->next;
		}
	}
	cyg_semaphore_post(&OamListSem);
	return pNode;

}

/*******************************************************************
* GwOadMessageListNodeNew
********************************************************************/

static GWTT_OAM_MESSAGE_NODE *GwOamMessageListNodeNew(unsigned short MessLen)
{
	GWTT_OAM_MESSAGE_NODE *pNode=NULL;

	pNode = iros_malloc(IROS_MID_OAM, sizeof(GWTT_OAM_MESSAGE_NODE));
	if(NULL!=pNode)
	{
		memset(pNode, '\0', sizeof(GWTT_OAM_MESSAGE_NODE));
	}
	else
	{
		return NULL;
	}

	if(0 !=MessLen )
	{
		pNode->pPayLoad = iros_malloc(IROS_MID_OAM, MessLen+1);
		if(NULL != pNode->pPayLoad )
		{
			memset(pNode->pPayLoad, '\0', MessLen+1);
		}
		else
		{
			iros_free(pNode);
			return NULL;
		}
	}
	return pNode;
}
#if 0
/*******************************************************************
* CtcOamMessageListNodeNew
*
*******************************************************************/

static CTC_OAM_MESSAGE_NODE *CtcOamMessageListNodeNew(unsigned short MessLen)
{
	CTC_OAM_MESSAGE_NODE *pNode=NULL;

	pNode = VOS_Malloc(sizeof(CTC_OAM_MESSAGE_NODE), MODULE_RPU_OAM);
	if(NULL!=pNode)
	{
		memset(pNode, '\0', sizeof(CTC_OAM_MESSAGE_NODE));
	}
	else
	{
		return NULL;
	}

	if(0 !=MessLen )
	{
		pNode->pPayLoad = VOS_Malloc(MessLen+1, MODULE_RPU_OAM);
		if(NULL != pNode->pPayLoad )
		{
			memset(pNode->pPayLoad, '\0', MessLen+1);
		}
		else
		{
			VOS_Free(pNode);
			return NULL;
		}
	}
	return pNode;
}

/*******************************************************************
* CtcOamMessageListNodeFree 
*
*******************************************************************/
void CtcOamMessageListNodeFree(CTC_OAM_MESSAGE_NODE *pNode)
{
	if(NULL == pNode)
		return;
	
	if(NULL != pNode->pPayLoad)
		VOS_Free(pNode->pPayLoad);
	VOS_Free(pNode);

	return;
}

/*******************************************************************
* CtcOamRecevOpcodeCheck 
*
*******************************************************************/
int CtcOamRecevOpcodeCheck(unsigned char CtcOpcode )
{
	switch(CtcOpcode)
	{
		case Extended_Variable_Request:
		case Extended_Variable_Set_Request:
		case Extended_Variable_Churning:
		case Extended_Variable_DBA:
			break;
		default:
			return OAM_MESSAGE_RECEV_OPCODE_ERR;
	}
	return OAM_MESSAGE_RECEV_OPCODE_OK;
}

#endif


/*******************************************************************
* GwOadMessageListNodeFree 
*
*******************************************************************/
void GwOamMessageListNodeFree(GWTT_OAM_MESSAGE_NODE *pNode)
{
	if(NULL == pNode)
		return;
	if(OK!=GwOamMessageListNodeRem(pNode))
		IROS_LOG_CRI(IROS_MID_OAM, "GwOamMessageListNodeFree::GwOamMessageListNodeRem failed\n");
	if(NULL != pNode->pPayLoad)
		iros_free(pNode->pPayLoad);
	iros_free(pNode);

	return;
}

/*******************************************************************
* GwOadMessageListNodeAdd 
*
*******************************************************************/
static void GwOamMessageListNodeAdd(GWTT_OAM_MESSAGE_NODE *pNode)
{
	if(NULL == pNode)
		return;
	if(NULL != GwOamMessageListGetNode(pNode->SendSerNo))
		IROS_LOG_CRI(IROS_MID_OAM, "GwOamMessageListNodeAdd::GwOamMessageListGetNode failed\n");
	cyg_semaphore_wait(&OamListSem);
	pNode->next = GwOamMessageListHead.next;
	GwOamMessageListHead.next = pNode;
	cyg_semaphore_post(&OamListSem);
}

/*******************************************************************
* GwOamMessageListNodeRem 
*
*******************************************************************/
static int GwOamMessageListNodeRem(GWTT_OAM_MESSAGE_NODE *pNode)
{
	GWTT_OAM_MESSAGE_NODE *pPrNode=NULL;

	if(NULL == pNode)
		return GWD_RETURN_ERR;
	cyg_semaphore_wait(&OamListSem);
	pPrNode = &GwOamMessageListHead;
	while(NULL!=pPrNode->next)
	{
		if(pNode == pPrNode->next)
		{
			pPrNode->next=pNode->next;
			pNode->next = NULL;
			cyg_semaphore_post(&OamListSem);
			return GWD_RETURN_OK;
		}
		else
		{
			pPrNode=pPrNode->next;
		}
	}
	cyg_semaphore_post(&OamListSem);
	return ERROR;
}


/*******************************************************************
* GwOamRecevOpcodeCheck 
*
*******************************************************************/
int GwOamRecevOpcodeCheck(unsigned char GwOpcode )
{
	switch(GwOpcode)
	{
		case EQU_DEVICE_INFO_REQ:
		case ALARM_RESP:
		case FILE_READ_WRITE_REQ:
		case FILE_TRANSFER_DATA:
		case FILE_TRANSFER_ACK:
		case SNMP_TRAN_REQ:
		case CLI_REQ_TRANSMIT:
		case IGMP_AUTH_TRAN_REQ:
		case IGMP_AUTH_TRAN_RESP:
		case CLI_PTY_TRANSMIT:
		case IGMP_TVM_REQ:
		case IGMP_TVM_RESP:
			break;
		default:
			return OAM_MESSAGE_RECEV_OPCODE_ERR;
	}
	return OAM_MESSAGE_RECEV_OPCODE_OK;
}

/*******************************************************************
* GwOamSendOpcodeCheck 
*
*******************************************************************/
int GwOamSendOpcodeCheck(unsigned char GwOpcode)
{
	switch(GwOpcode)
	{
		case EQU_DEVICE_INFO_RESP:
		case ALARM_REQ:
		case FILE_READ_WRITE_REQ:
		case FILE_TRANSFER_DATA:
		case FILE_TRANSFER_ACK:
		case SNMP_TRAN_RESP:
		case SNMP_TRAN_TRAP:		
		case CLI_RESP_TRANSMIT:
		case IGMP_AUTH_TRAN_REQ:
		case IGMP_AUTH_TRAN_RESP:
		case CLI_PTY_TRANSMIT:
		case IGMP_TVM_REQ:
		case IGMP_TVM_RESP:
			break;
		default:
			return OAM_MESSAGE_RECEV_OPCODE_ERR;
	}
	return OAM_MESSAGE_RECEV_OPCODE_OK;

}

/*******************************************************************
* CommOnuMsgReveive 
*
*******************************************************************/
int CommOnuMsgReveive(GWTT_OAM_MESSAGE_NODE **ppMessage,unsigned char *MessagData)
{
	GWTT_OAM_MESSAGE_NODE *pMessageNode=NULL;
	GWTT_OAM_HEADER *vender_header=NULL;
	unsigned char *payload=NULL;
	unsigned short PayLoadOffset=0;
	unsigned short PayloadLength=0;
	unsigned int WholePktLen=0;

	if(NULL == ppMessage)
		return OAM_MESSAGE_RECEV_ERR;	
	GWDOAMTRC("  GWD OAM handler - CommOnuMsgReveive\n");
	vender_header = (GWTT_OAM_HEADER*)MessagData;
	if (0 == memcmp(vender_header->oui, GwOUI, sizeof(GwOUI)))
	{
		/* GWTT Extend OAM */
		if(OAM_MESSAGE_RECEV_OPCODE_ERR==GwOamRecevOpcodeCheck(vender_header->opCode))
			return OAM_MESSAGE_RECEV_OPCODE_ERR;	
		GWDOAMTRC("  GWD OAM handler - CommOnuMsgReveive - opCode check OK.\n");
		WholePktLen = ntohs(vender_header->wholePktLen);
		GWDOAMTRC("  GWD OAM handler - CommOnuMsgReveive - PktLen = %d.\n", WholePktLen);
		if(OAM_DATA_LEN < WholePktLen)
			return OAM_MESSAGE_RECEV_TOO_LONG;
		payload = MessagData+sizeof(GWTT_OAM_HEADER);
		PayLoadOffset = ntohs(vender_header->payloadOffset);
		PayloadLength = ntohs(vender_header->payLoadLength);

	 	if (0 == PayLoadOffset)
	 	{
			pMessageNode = GwOamMessageListGetNode(vender_header->senderSerNo);
			if(NULL!=pMessageNode)
				return OAM_MESSAGE_RECEV_ERR;

			if(WholePktLen==PayloadLength)
			{
				pMessageNode = GwOamMessageListNodeNew(WholePktLen);
				if(NULL == pMessageNode)
					return OAM_MESSAGE_RECEV_NO_MEM;
				pMessageNode->GwOpcode=vender_header->opCode;
				pMessageNode->RevPktLen=PayloadLength;
				pMessageNode->WholePktLen=PayloadLength;
				pMessageNode->SendSerNo=vender_header->senderSerNo;
				memcpy(pMessageNode->SessionID,vender_header->sessionId,8);
				memcpy(pMessageNode->pPayLoad,payload,PayloadLength);
				GwOamMessageListNodeAdd(pMessageNode);	
				*ppMessage = pMessageNode;
				GWDOAMTRC("  GWD OAM handler - CommOnuMsgReveive - success 1.\n");
				return OAM_MESSAGE_RECEV_OK;
			}
			else
			{
				pMessageNode = GwOamMessageListNodeNew(WholePktLen);
				if(NULL == pMessageNode)
					return OAM_MESSAGE_RECEV_NO_MEM;
				pMessageNode->GwOpcode=vender_header->opCode;
				pMessageNode->RevPktLen=PayloadLength;
				pMessageNode->WholePktLen=WholePktLen;
				pMessageNode->SendSerNo=vender_header->senderSerNo;
				memcpy(pMessageNode->SessionID,vender_header->sessionId,8);
				memcpy(pMessageNode->pPayLoad,payload,PayloadLength);
				GwOamMessageListNodeAdd(pMessageNode);	
				*ppMessage = NULL;
				(pMessageNode->TimerID).opaque = (void *)(pMessageNode->SendSerNo);
				epon_timer_add(&(pMessageNode->TimerID), OamMessageRecevTimeOut, WAIT_TIME_FOR_OAM_MESSAGE);
				/*pMessageNode->TimerID=VOS_TimerCreate(MODULE_RPU_OAM, (unsigned long)NULL, WAIT_TIME_FOR_OAM_MESSAGE, OamMessageRecevTimeOut, pMessageNode->SendSerNo, VOS_TIMER_NO_LOOP);
				if(0 == pMessageNode->TimerID)
				{
					GwOamMessageListNodeFree(pMessageNode);
					return OAM_MESSAGE_RECEV_TIMER_ERR;
				}*/			
			}
		}
		else
		{
			pMessageNode = GwOamMessageListGetNode(vender_header->senderSerNo);
			if(NULL==pMessageNode)
			{
				return OAM_MESSAGE_RECEV_ERR;
			}
			if(((pMessageNode->RevPktLen+PayloadLength) > pMessageNode->WholePktLen)||
				(pMessageNode->RevPktLen!= PayLoadOffset))			
			{
				return OAM_MESSAGE_RECEV_ERR;
			}
			if((pMessageNode->RevPktLen+PayloadLength) == pMessageNode->WholePktLen)
			{
				epon_timer_del(&(pMessageNode->TimerID));
				memcpy(pMessageNode->pPayLoad+pMessageNode->RevPktLen,payload,PayloadLength);
				pMessageNode->RevPktLen = pMessageNode->WholePktLen;
				*ppMessage = pMessageNode;
				GWDOAMTRC("  GWD OAM handler - CommOnuMsgReveive - success 2.\n");
				return OAM_MESSAGE_RECEV_OK;
			}
			else
			{
				epon_timer_del(&(pMessageNode->TimerID));
				epon_timer_add(&(pMessageNode->TimerID), (pMessageNode->TimerID).tmfunc, WAIT_TIME_FOR_OAM_MESSAGE);
				memcpy(pMessageNode->pPayLoad+pMessageNode->RevPktLen,payload,PayloadLength);
				pMessageNode->RevPktLen+=PayloadLength;
				*ppMessage = NULL;
				return OAM_MESSAGE_RECEV_NOT_COMPLETE;
			}		
		}	
	}
	
	return OAM_MESSAGE_RECEV_ERR;
}



static int GwCommOamHeadBuild(GWTT_OAM_HEADER *pHead,  unsigned char GwOpcode,unsigned int SendSerNo,const unsigned short SendDataSize,unsigned char  *pSessionIdfield)
{
	oam_if_t *oamif = oam_intf_find(oam_sdl_get_llid());

    if(!oamif)
            return GWD_RETURN_ERR;

	oam_build_pdu_hdr(oamif, (oam_pdu_hdr_t *)pHead, OAM_PDU_CODE_ORG_SPEC);

	if((NULL == pHead)||(NULL == pSessionIdfield))
		return GWD_RETURN_ERR;
	if(OAM_MESSAGE_RECEV_OPCODE_ERR == GwOamSendOpcodeCheck(GwOpcode))
		return GWD_RETURN_ERR;

	pHead->oui[0] = 0x00;
	pHead->oui[1] = 0x0F;
	pHead->oui[2] = 0xE9;
	pHead->opCode = GwOpcode;
	pHead->senderSerNo = SendSerNo;
	pHead->wholePktLen = SendDataSize;
	if(NULL != pSessionIdfield)
		memcpy(pHead->sessionId,pSessionIdfield,8);
	return GWD_RETURN_OK;
}
int Gwd_OAM_get_length_negotiation(unsigned short *pusOAMFrameLen)
{
#define GWD_OAM_PKT_LENGTH	128

	*pusOAMFrameLen = GWD_OAM_PKT_LENGTH;

	return GWD_RETURN_OK;
}

/*******************************************************************
* CommOnuMsgSend 
*
*******************************************************************/
int CommOnuMsgSend(unsigned char GwOpcode, unsigned int SendSerNo, unsigned char *pSentData,const unsigned short SendDataSize, unsigned char  *pSessionIdfield)
{
	unsigned char OamFrame[2048] = {0};
	GWTT_OAM_HEADER *avender;
	unsigned short DataLenSended=0;
	unsigned short usOAMFrameLen;
	unsigned short usOAMPayloadLenGW;
	int	bSlowProtocol = FALSE;
	int	iSendPacketNumber = 0;
	cs_llid_t llid;
	cs_callback_context_t context;
	if(epon_request_onu_mpcp_llid_get(context, 0, 0, &llid) != CS_OK)
		return GWD_RETURN_ERR;
	if( GWD_RETURN_OK != Gwd_OAM_get_length_negotiation(&usOAMFrameLen) )
	{
		return GWD_RETURN_ERR;
	}
	else
	{
		usOAMPayloadLenGW = usOAMFrameLen - OAM_OVERHEAD_LEN_STD - OAM_OVERHEAD_LEN_GW;
	}

	if (CLI_RESP_TRANSMIT == GwOpcode)
	{
		bSlowProtocol = TRUE;
	}
	
	gulDebugOamTxCount = 0;
	if(SendDataSize > (OAM_DATA_LEN-sizeof(GWTT_OAM_HEADER)))
		return GWD_RETURN_ERR;
	avender = (GWTT_OAM_HEADER *)OamFrame;
	if(GWD_RETURN_OK != GwCommOamHeadBuild(avender,GwOpcode,SendSerNo,SendDataSize,pSessionIdfield))
		return OAM_MESSAGE_SEND_ERROR;
	
	if(usOAMPayloadLenGW < SendDataSize)
	{
		while((usOAMPayloadLenGW+DataLenSended) < SendDataSize)
		{
			avender->payLoadLength = usOAMPayloadLenGW;
			avender->payloadOffset = DataLenSended;
			memset(OamFrame+sizeof(GWTT_OAM_HEADER), '\0',2048-sizeof(GWTT_OAM_HEADER));
			memcpy(OamFrame+sizeof(GWTT_OAM_HEADER),pSentData+DataLenSended,usOAMPayloadLenGW);
			oam_send(llid,active_pon_port, (unsigned char *)avender,(int)(usOAMPayloadLenGW + sizeof(GWTT_OAM_HEADER)));
			gulDebugOamTxCount++;
            OAM_TX_PACKET_DEBUG((avender, pSentData+DataLenSended));

			DataLenSended+=usOAMPayloadLenGW;

			iSendPacketNumber ++;
			if ((0 == (iSendPacketNumber % 10)) && (TRUE == bSlowProtocol))
			{
				cyg_thread_delay(1); /* 1 tick 10ms */
			}
		}

		avender->payLoadLength = SendDataSize-DataLenSended;
		avender->payloadOffset = DataLenSended;
		memset(OamFrame+sizeof(GWTT_OAM_HEADER), '\0',2048-sizeof(GWTT_OAM_HEADER));
		memcpy(OamFrame+sizeof(GWTT_OAM_HEADER),pSentData+DataLenSended,SendDataSize-DataLenSended);
		oam_send(llid,active_pon_port, (unsigned char *)avender,(int)(sizeof(GWTT_OAM_HEADER) + SendDataSize - DataLenSended));
		gulDebugOamTxCount++;
        OAM_TX_PACKET_DEBUG((avender, pSentData+DataLenSended));
		return GWD_RETURN_OK;
	}
	else
	{
		avender->payLoadLength = SendDataSize;
		avender->payloadOffset = 0;
		memcpy(OamFrame+sizeof(GWTT_OAM_HEADER),pSentData,SendDataSize);

		oam_send(llid,active_pon_port, (unsigned char *)avender,(int)(sizeof(GWTT_OAM_HEADER)+SendDataSize));
		gulDebugOamTxCount++;
        OAM_TX_PACKET_DEBUG((avender, pSentData));
		return GWD_RETURN_OK;
	}
}


void OamMessageRecevTimeOut(epon_timer_t *timer)
{
	GWTT_OAM_MESSAGE_NODE *pNode;
	unsigned int SerNo = (unsigned int)(timer->opaque);

	pNode = GwOamMessageListGetNode(SerNo);
	if(NULL != pNode)
		GwOamMessageListNodeFree(pNode);
}

#if 0
static long GwOamIGMPResponseRecv(GWTT_OAM_MESSAGE_NODE *pRequest )
{
	if(NULL == pRequest)            return GWD_RETURN_ERR;
	if(NULL == pRequest->pPayLoad)	return GWD_RETURN_ERR;

	VOS_SysLog(LOG_TYPE_OAM, LOG_DEBUG_OUT, "Receive AUTH_RESP packet SerNo = %d PktLen = %d\r\n", VOS_HTONL(pRequest->SendSerNo),pRequest->RevPktLen);

	return(Igmp_Snoop_AuthRespond(pRequest->pPayLoad, pRequest->WholePktLen));
	return GWD_RETURN_OK;
}

static long GwOamIGMPRequireRecv(GWTT_OAM_MESSAGE_NODE *pRequest )
{
	if(NULL == pRequest)            return GWD_RETURN_ERR;
	if(NULL == pRequest->pPayLoad)	return GWD_RETURN_ERR;

	VOS_SysLog(LOG_TYPE_OAM, LOG_DEBUG_OUT, "Receive AUTH_RESP packet SerNo = %d PktLen = %d\r\n", VOS_HTONL(pRequest->SendSerNo),pRequest->RevPktLen);

	return(Igmp_Snoop_AuthRespond(pRequest->pPayLoad, pRequest->WholePktLen));
	return GWD_RETURN_OK;
}
#endif

/*============================================================*/


long GwOamIGMPRequireSend(char *pPkt, long lLen)
{
	unsigned char pSessId[8];
	
	memset(pSessId, 0, sizeof(pSessId));
	gulOamSendSerNo ++;
	pSessId[6] = gulOamSendSerNo;
	return(CommOnuMsgSend(IGMP_AUTH_TRAN_REQ, gulOamSendSerNo, pPkt, lLen, pSessId));
}


long GwOamIGMPRespondSend(char *pPkt, long lLen)
{
	unsigned char pSessId[8];

	memset(pSessId, 0, sizeof(pSessId));
	gulOamSendSerNo ++;
	pSessId[6] = gulOamSendSerNo;
	return(CommOnuMsgSend(IGMP_AUTH_TRAN_RESP, gulOamSendSerNo, pPkt, lLen, pSessId));
}

void Gwd_Oam_Handle(unsigned int port, unsigned char *frame, unsigned int len)
{
	GWTT_OAM_MESSAGE_NODE *pMessage = NULL;
	int iRet; 
	/*extern unsigned long cl_sn_service_status ;*/
    if(!frame || !port)
        return;

	if(len < sizeof(GWTT_OAM_HEADER))
		return;

	GWDOAMTRC("Gwd_Oam_Handle - (%d, %x %x %x %x %x %x %d)\n", port, 
		frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], len);
	if(OAM_MESSAGE_RECEV_OK != (iRet = CommOnuMsgReveive(&pMessage, frame)))
	{
        IROS_LOG_MAJ(IROS_MID_OAM, "GW OAM receiving error!\r\n");
		GWDOAMTRC("  GWD OAM handler - CommOnuMsgReveive - failed(%d).\n", iRet);
		return;
	}
	if(NULL == pMessage)
	{
		return;
	}
	/* Then process the packet */
	gulDebugOamRxCount++;
    OAM_RX_PACKET_DEBUG((pMessage));
	switch(pMessage->GwOpcode)
	{
		case EQU_DEVICE_INFO_REQ:
			GWDOAMTRC("Gwd_Oam_Handle - EQU_DEVICE_INFO_REQ received.\n");
			if(GWD_RETURN_OK != (iRet = GwOamInformationRequest(pMessage)))
			{
				IROS_LOG_MAJ(IROS_MID_OAM, "Generate OAM(Information Request) response Error!(%d)", iRet);
				GWDOAMTRC("Gwd_Oam_Handle - EQU_DEVICE_INFO_REQ failed.(%d)\n", iRet);
			}
			GwOamMessageListNodeFree(pMessage);
			pMessage = NULL;
			break;

		case ALARM_RESP:
			if(GWD_RETURN_OK != GwOamAlarmResponse(pMessage))
			{
				IROS_LOG_MAJ(IROS_MID_OAM, "Deal with OLT Alarm response Error!");
			}
			GwOamMessageListNodeFree(pMessage);
			pMessage = NULL;
			break;

		case FILE_READ_WRITE_REQ:
		case FILE_TRANSFER_DATA:
		case FILE_TRANSFER_ACK:
		case SNMP_TRAN_REQ:
		case CLI_REQ_TRANSMIT:
		case IGMP_AUTH_TRAN_REQ:
	    case IGMP_AUTH_TRAN_RESP:
		case CLI_PTY_TRANSMIT:
		default:
			GWDOAMTRC("Gwd_Oam_Handle - unknown opcode(%d) received.\n", pMessage->GwOpcode);
			IROS_LOG_MAJ(IROS_MID_OAM, "Received an unknown packet(0x%x), drop it!\r\n", pMessage->GwOpcode);
			GwOamMessageListNodeFree(pMessage);
			break;
	}
}

int GwGetOltType(unsigned char *mac, GWD_OLT_TYPE *type)
{
	unsigned char gwPonMac[6] = { 0x00, 0x0c, 0xd5, 0x00, 0x01, 0x00 };
				
	if( 0 == memcmp(mac, gwPonMac, 5)) /* old type, parsed as before*/
	{
		if((mac[5] == 0x0) ||(mac[5] == 0x10) ||
		(mac[5] == 0x20) ||(mac[5] == 0x30))
			*type = GWD_OLT_GFA6100;
		else if(mac[5] >= 0xec)
			*type = GWD_OLT_GFA6700;
		else
			*type = GWD_OLT_NOMATCH;
	}
	else if( 0 == memcmp(mac, gwPonMac, 3)) /*new type, parsed in new formula*/
	{
		if(mac[3] == 0x61)
			*type = GWD_OLT_GFA6100;
		else if(mac[3] == 0x67)
			*type = GWD_OLT_GFA6700;
		else if(mac[3] == 0x69)
			*type = GWD_OLT_GFA6900;	
		else 
			*type = GWD_OLT_NOMATCH;
	}
	else
		*type = GWD_OLT_NONE;

	return GWD_RETURN_OK;
}

int GwGetPonSlotPort(unsigned char *mac, GWD_OLT_TYPE type, unsigned long *slot, unsigned long *port)
{
	unsigned char ponMac;
	unsigned char gwPonMac[6] = { 0x00, 0x0c, 0xd5, 0x00, 0x01, 0x00 };
	ponMac = mac[5];
	if( 0 == memcmp(mac, gwPonMac, 5)) /* old type, parsed as before*/	
	{
		switch(type)
		{
			case GWD_OLT_GFA6100 :
				switch(ponMac)
				{
					case 0 :
						*slot = 2; *port = 1;
						break;
					case 0x10 : 
						*slot = 2; *port = 2;
						break;
					case 0x20 :
						*slot = 3; *port = 1;
						break;
					case 0x30 :
						*slot = 3; *port = 2;
						break;
					default:
						return GWD_RETURN_ERR;
				}
				break;
			case GWD_OLT_GFA6700 :
				*slot = ((ponMac & 0x1c)>>2) + 1;
				*port = ((ponMac & 0x3)) + 1;
				break;
			default :				  /*others*/
				*slot = 0xff;
				*port = 0xff;
				return GWD_RETURN_ERR;
		}
	}
	else if( 0 == memcmp(mac, gwPonMac, 3))	/*new type, parsed in new formula*/
	{
		*slot = mac[4];
		*port = mac[5];
	}
	else 
		return GWD_RETURN_ERR;

	return GWD_RETURN_OK;
}
static int GwOamInformationRequest(GWTT_OAM_MESSAGE_NODE *pRequest )
{
	unsigned char ver[4] = {1, 1, 1, 1};
	unsigned char Response[1024]={'\0'},*ptr, *pReq;
	unsigned char temp[128];
	int ResLen=0;
//	int i;
	unsigned short device_type;
	int tmpRet;

	if(NULL == pRequest)
		return GWD_RETURN_ERR;
	if(EQU_DEVICE_INFO_REQ != pRequest->GwOpcode)
		return GWD_RETURN_ERR;
	if(NULL == pRequest->pPayLoad)
		return GWD_RETURN_ERR;
	switch(*pRequest->pPayLoad)
	{
		case ONU_INFOR_GET:
		{
			GWDOAMTRC("EQU_DEVICE_INFO_REQ - ONU_INFOR_GET received.\n");
			ptr = Response;
			/* Payload */
			*ptr++  = ONU_INFOR_GET;	/* type : 1 for opCode 1's reply */

			/* Device Type */
			device_type = DEVICE_TYPE_GT811_A;
			SET_SHORT(ptr, device_type);
			ptr += sizeof(short);
			/* OUI */
			*ptr ++ = 0x00;
			*ptr ++ = 0x0f;
			*ptr ++ = 0xe9;

			/* Contents */
			memset(temp, '\0', sizeof(temp));
			//ResLen = sprintf(temp, "V%d.%dB%d",SYS_HARDWARE_MAJOR_VERSION_NO, SYS_HARDWARE_RELEASE_VERSION_NO, SYS_HARDWARE_BRANCH_VERSION_NO);
			ResLen = sprintf(temp, "%s",onu_system_info_total.hw_version);
			*ptr++ = ResLen;
			sprintf(ptr,"%s",temp);
			ptr += ResLen;

			/*Boot Version*/
			if (ver[0] != 0)
			{
				ResLen = sprintf(temp, "%s",irosbootver);
				*ptr++ = ResLen;
				sprintf(ptr,"%s",temp);
				ptr += ResLen;
			}
			else
			{
				*ptr ++= (strlen(GwONU831BootVer));
				sprintf(ptr,"%s",GwONU831BootVer);
				ptr += (strlen(GwONU831BootVer));
			}
			
			/*Software version*/
			memset(temp, '\0', sizeof(temp));
			//ResLen = sprintf(temp, "%s",onu_system_info_total.sw_version);
			#if 1
			sprintf(temp,"V%dR%02dB%03d",
							SYS_SOFTWARE_MAJOR_VERSION_NO,
							SYS_SOFTWARE_RELEASE_VERSION_NO,
							SYS_SOFTWARE_BRANCH_VERSION_NO);
			#endif
			*ptr++ = strlen(temp);
			sprintf(ptr, "%s",temp);
			ptr += strlen(temp);

			/*Firmware version*/
			memset(temp, '\0', sizeof(temp));
			ResLen = sprintf(temp, "V%s.%s.%s.%s",IROS_ONU_APP_VER_MAJOR,IROS_ONU_APP_VER_MINOR,IROS_ONU_APP_VER_REVISION,IROS_ONU_APP_VER_BUILD);
			*ptr++ = ResLen;
			sprintf(ptr,"%s",temp);
			ptr += ResLen;

			/*Onu name*/
			ResLen = strlen(onu_system_info_total.device_name);
			if (ResLen > 128)
				ResLen = 128;
			*ptr++ = ResLen;
			memcpy(ptr, onu_system_info_total.device_name, ResLen);
			ptr += ResLen;

			/*Description*/
			ResLen = strlen("GT811-CTC-Ready");
			if (ResLen > 128)
				ResLen = 128;
			*ptr++ = ResLen;
			memcpy(ptr, "GT811-CTC-Ready", ResLen);
			ptr += ResLen;

			/*Location*/
			ResLen = strlen("Beijing China");
			if (ResLen > 128)
				ResLen = 128;
			*ptr ++= ResLen;
			memcpy(ptr,"Beijing China", ResLen);
			ptr += ResLen;

			/*Vendor*/
			ResLen = strlen("GW Delight");
			if (ResLen > 128)
				ResLen = 128;
			*ptr ++= ResLen;
			memcpy(ptr, "GW Delight", ResLen);
			ptr += ResLen;

			/*Serial Number*/
			ResLen = strlen(onu_system_info_total.serial_no);
			*ptr ++ = ResLen;
			memcpy(ptr, onu_system_info_total.serial_no, ResLen);
			ptr += ResLen;
			
			/*Manufacture Date*/
			ResLen = strlen(onu_system_info_total.hw_manufature_date);
			*ptr ++ = ResLen;
			memcpy(ptr, onu_system_info_total.hw_manufature_date, ResLen);
			ptr += ResLen;

			/*auto request*/

			*ptr ++ =0;

			/*slot num*/
			*ptr ++=0;

			/*ablity*/
			*ptr ++= 0xfe;
			
			*ptr ++= 3;

			*ptr ++= 0x80;

			ResLen = ((unsigned long)ptr-(unsigned long)Response);			

			gulGwOamConnect = 1;
			break;
		}
		case ONU_INFOR_SET:
		{
			unsigned char nameLen, descrLen, locationLen;
			
			tmpRet = ONU_INFOR_SET<<8;
			pReq = pRequest->pPayLoad+1;

			ptr = Response;
			/* Payload */
			*ptr++  = ONU_INFOR_SET;	

			/* Name */
			nameLen = *pReq;
			pReq ++;
			if (nameLen)
			{
				unsigned char tmpLen;
				if (nameLen > sizeof(onu_system_info_total.device_name))
					tmpLen = sizeof(onu_system_info_total.device_name);
				else
					tmpLen = nameLen;
				
				Onu_Sysinfo_Get();
				memset(onu_system_info_total.device_name, 0, sizeof(onu_system_info_total.device_name));
				memcpy(onu_system_info_total.device_name, pReq, tmpLen);
				Onu_Sysinfo_Save();
				/* Success */
				*ptr ++ = 1;
				pReq += nameLen;
			}

			/* Description */
			descrLen = *pReq;
			pReq ++;
			if (descrLen)
			{
#if 0
				char * errmsg = "";
				int ret;
				
				if (descrLen > 255)
					return (tmpRet|S_BAD_PARAM);
				
				VOS_MemZero(szTmp, 256);
				memcpy(szTmp, pReq, descrLen);

				ret = mn_set_sysdescr( szTmp , &errmsg, 1 );

				if (ret)
				{
					/* Success */
					*ptr ++ = 1;
				}
				else
				{
					/* Failed */
					*ptr ++ = 2;
				}
#else
				/* Success */
				*ptr ++ = 1;
#endif				
				pReq += descrLen;
			}

			/* Location */
			locationLen = *pReq;
			pReq ++;
			if (locationLen)
			{
#if 0
				char * errmsg = "";
				int ret;

				if (locationLen > 255)
					return (tmpRet|S_BAD_CONFIGURATION);
				
				VOS_MemZero(szTmp, 256);
				memcpy(szTmp, pReq, locationLen);

				ret = mn_set_syslocation( szTmp, &errmsg, 1 );

				if (ret)
				{
					/* Success */
					*ptr ++ = 1;
				}
				else
				{
					/* Failed */
					*ptr ++ = 2;
				}

#else
				/* Success */
				*ptr ++ = 1;
#endif				
				pReq += locationLen;
			}
			ResLen = ((unsigned int)ptr-(unsigned int)Response);
			break;
		}
		case ONU_REALTIME_SYNC:
		{
#if 0
			unsigned short usValue;
			unsigned char ucValue;
			int	i = 0;
			extern char   g_cSetTime[20];
			
			tmpRet = ONU_REALTIME_SYNC<<8;
			pReq = pRequest->pPayLoad+1;

			ptr = g_cSetTime;

			/* Year */
			/*usValue = *((unsigned short *)pReq);*//* Will cause exception: maybe because pReq not odd address */
			usValue = pReq[i];
			i++;
			usValue = usValue << 8;
			usValue += pReq[i];
			if (usValue < 1980 ||usValue > 2079)   //according to OLT system time range, added by dushb 2009-11-12
				return (tmpRet|S_OUT_OF_RANGE);
			ResLen = sprintf(ptr,"%4d/",usValue);
			ptr += ResLen;
			i++;

			/* Month */
			ucValue = pReq[i];
			if (ucValue > 12) 
				return (tmpRet|S_BAD_PARAM);
			ResLen = sprintf(ptr,"%02d/",ucValue);
			ptr += ResLen;
			i++;

			/* Day */
			ucValue = pReq[i];
			if (ucValue > 31) 
				return (tmpRet|S_BAD_CONFIGURATION);
			ResLen = sprintf(ptr,"%02d:",ucValue);
			ptr += ResLen;
			i++;

			/* Hour */
			ucValue = pReq[i];
			if (ucValue > 24) 
				return (tmpRet|S_NO_RESOURCES);
			ResLen = sprintf(ptr,"%02d:",ucValue);
			ptr += ResLen;
			i++;
			
			/* Minute */
			ucValue = pReq[i];
			if (ucValue > 59) 
				return (tmpRet|S_NOT_FOUND);
			ResLen = sprintf(ptr,"%02d:",ucValue);
			ptr += ResLen;
			i++;
			
			/* Second */
			ucValue = pReq[i];
			if (ucValue > 59) 
				return (tmpRet|S_ALREADY_EXISTS);
			ResLen = sprintf(ptr,"%02d",ucValue);
			ptr += ResLen;
			i++;

			ResLen = i+1;
			memcpy(Response,pRequest->pPayLoad,ResLen);

			cl_do_set_time_nouser(NULL);

#if (RPU_YES == RPU_MODULE_TIMING_PKT)

            if((0 == TimingPkt_TaskID)&&(TIMPKT_SEND_ENABLE == gulTimingPacket))/*ֻ��ʹ��ʱ����һ��*/
            {
                TimingPkt_TaskID = VOS_TaskCreate("tEthTx", 220, (VOS_TASK_ENTRY) txEthTask, NULL);/*�������ȼ�= port monitor*/
                VOS_ASSERT(TimingPkt_TaskID != 0);
            }
#endif
			
#endif
			break;
		}

    case ACCESS_IDENTIFIER:
    {
#if 0
        int iret;
        extern PASONU_flow_desc_t rxEthTaskfdHigh;

        pReq = pRequest->pPayLoad+1;
        ptr = Response + 1;
        ResLen = pRequest->RevPktLen;
        
        memcpy(Response,pRequest->pPayLoad,pRequest->RevPktLen);
      
        pReq++;

        if (*pReq > RELAY_TYPE_DHCP)
        {
            *(ptr ++) = 2;
            break;
        }

#if (RPU_MODULE_PPPOE_RELAY == RPU_YES)
        if (RELAY_TYPE_PPPOE == *pReq)
        {
            PPPOE_RELAY_PACKET_DEBUG(("\r\n received pppoe relay-OAM pkt!\r\n"));

            PPPOE_RELAY_PACKET_DEBUG(("\r\n relay-mode=%d\r\n",*pReq));

            pReq++;
            
            if (*pReq > PPPOE_DSL_FORUM_MODE)
            {
                *(ptr ++) = 2;/*ʧ��*/
                break;
            }
            
            if(PPPOE_RELAY_DISABLE == *pReq)/*����?����ʾ��ֹ״̬*/
            {
                if (PPPOE_RELAY_DISABLE == g_PPPOE_relay)
                {
                    /*do nothing*/
                    PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is already disabled! \r\n"));
                }
                else
                {
                    iret = PASONU_CLASSIFIER_remove_filter(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8863);
                    if ((S_OK != iret)&&(S_NOT_FOUND != iret))
                    {
                        ASSERT(0);
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is disabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }
                    
                    if (S_OK != PasOnuClassL2RuleAdd(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8863, 0, 2))
                    {
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is disabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }

                    if (S_OK != PasOnuClassL2RuleAdd(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8864, 0, 3))
                    {
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is disabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }

#if ((!FOR_812_SERIES)&&(!FOR_BCM_ONU_PON_VOICE))/*GT812��������*/
                    if (S_OK != PASONU_PQUEUE_set_ingress_limit(PQ_RX_CPU_UNI, 1, 0))
                    {
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is disabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }
#endif
                    g_PPPOE_relay = PPPOE_RELAY_DISABLE;
                    PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is disabled successfully! \r\n"));
                }
                *(ptr ++) = 1;
                
                break;
            }
            else
            {
                if (PPPOE_RELAY_ENABLE == g_PPPOE_relay)
                {
                    /*do nothing*/
                    PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is already enabled! \r\n"));
                }
                else
                {
                    if (S_OK != PasOnuClassL2RuleAdd(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8863, 0, 2))
                    {
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is enabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }
                            
                    if (S_OK != PasOnuClassL2RuleAdd(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8864, 0, 3))
                    {
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is enabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }


                    iret = PASONU_CLASSIFIER_remove_filter(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8863);
                    if ((S_OK != iret)&&(S_NOT_FOUND != iret))
                    {
                        ASSERT(0);
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is enabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }
                    
                    iret = PASONU_CLASSIFIER_add_filter(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8863, PASONU_PASS_CPU, rxEthTaskfdHigh);
                    if ((S_OK != iret)&&(S_ALREADY_EXISTS != iret))
                    {
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is enabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }

#if ((!FOR_812_SERIES)&&(!FOR_BCM_ONU_PON_VOICE))/*GT812��GT863��GT866��������*/
                    if (S_OK != PASONU_PQUEUE_set_ingress_limit(PQ_RX_CPU_UNI, 1, 63))
                    {
                        PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is enabled failed! \r\n"));
                        *(ptr ++) = 2;
                        break;
                    }
#endif
                    
                    g_PPPOE_relay = PPPOE_RELAY_ENABLE;
                    PPPOE_RELAY_PACKET_DEBUG(("\r\n pppoe relay is enabled successfully! \r\n"));
                }
            }
        
            if (PPPOE_GWD_PRIVITE_MODE == *pReq)
            {
                PPPOE_relay_mode = PPPOE_GWD_PRIVITE_MODE;
            }
            else
            {
                PPPOE_relay_mode = PPPOE_DSL_FORUM_MODE;
            }
      
            pReq ++;
      
            /*��ȡ�ַ�*/

            PPPOE_RELAY_PACKET_DEBUG(("\r\n olt-relay-string = %s\r\n",pReq));
            
            if(pRequest->RevPktLen - 4 > ((PPPOE_GWD_PRIVITE_MODE == PPPOE_relay_mode)?40:50))
            {
                PPPOE_RELAY_PACKET_DEBUG(("\r\n circuit id value is too long! \r\n"));
                PASONU_CLASSIFIER_remove_filter(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8863);
                PASONU_PQUEUE_set_ingress_limit(PQ_RX_CPU_UNI, 1, 0);
                PasOnuClassL2RuleAdd(PASONU_UPSTREAM, FRAME_ETHERTYPE, 0x8863, 0, 1);
                g_PPPOE_relay = PPPOE_RELAY_DISABLE;/*�ָ���disable��״̬��mode�Ͳ�����*/
                *(ptr ++) = 2;
                break;
            }
        
            pppoe_circuitid_value_lenth = pRequest->RevPktLen - 4;/*4���ֽڷֱ���msg_type,result,relay_type,relay_mode*/
            memcpy(pppoe_circuitid_value_head, pReq, pppoe_circuitid_value_lenth);
            *(pppoe_circuitid_value_head + pppoe_circuitid_value_lenth) = '\0';/*���ڴ�ӡstring*/        

        }
#endif

        if(RELAY_TYPE_DHCP == *pReq)/*����pppoe_relay������*/
        {
            DHCP_RELAY_PACKET_DEBUG(("\r\n received dhcp relay-OAM pkt!\r\n"));
            
            DHCP_RELAY_PACKET_DEBUG(("\r\n relay-mode=%d\r\n",*pReq));

            pReq++;
            
            if (*pReq > DHCP_OPTION82_RELAY_MODE_STD)
            {
                *(ptr ++) = 2;/*ʧ��*/
                break;
            }
            
            if(0 == *pReq)/*����?����ʾ��ֹ״̬*/
            {
                if (0 == g_DHCP_OPTION82_Relay)
                {
                    /*do nothing*/
                    DHCP_RELAY_PACKET_DEBUG(("\r\n dhcp relay is already disabled! \r\n"));
                }
                else
                {
    				PasOnuFilterL3L4RuleDelete(PASONU_UPSTREAM, PASONU_TRAFFIC_UDP_PORT,
    					PASONU_TRAFFIC_SOURCE, 0, DHCP_SERVER_PORT);					
#if (!FOR_812_SERIES)/*GT812��������*/
                    PASONU_PQUEUE_set_ingress_limit(PQ_RX_CPU_UNI, 0, 0);
#endif
                    g_DHCP_OPTION82_Relay = 0;
                    DHCP_RELAY_PACKET_DEBUG(("\r\n dhcp relay is disabled successfully! \r\n"));
                }
                *(ptr ++) = 1;
                
                break;
            }
            else
            {
                if (1 == g_DHCP_OPTION82_Relay)
                {
                    /*do nothing*/
                    DHCP_RELAY_PACKET_DEBUG(("\r\n dhcp relay is already enabled! \r\n"));
                }
                else
                {
        			PasOnuFilterL3L4RuleAdd(PASONU_UPSTREAM, PASONU_TRAFFIC_UDP_PORT,
        				PASONU_TRAFFIC_SOURCE, 0, DHCP_SERVER_PORT, PASONU_PASS_CPU);
#if (!FOR_812_SERIES)/*GT812��������*/
                    PASONU_PQUEUE_set_ingress_limit(PQ_RX_CPU_UNI, 0, 13);
#endif
                    
                    g_DHCP_OPTION82_Relay = 1;
                    DHCP_RELAY_PACKET_DEBUG(("\r\n dhcp relay is enabled successfully! \r\n"));
                }
            }
        
            if (DHCP_OPTION82_RELAY_MODE_CTC == *pReq)
            {
                g_DHCP_OPTION82_Relay_Mode = DHCP_OPTION82_RELAY_MODE_CTC;
            }
            else
            {
                g_DHCP_OPTION82_Relay_Mode = DHCP_OPTION82_RELAY_MODE_STD;
            }
      
            pReq ++;
      
            /*��ȡ�ַ�*/

            DHCP_RELAY_PACKET_DEBUG(("\r\n olt-relay-string=%s\r\n",pReq));
            
            if(pRequest->RevPktLen - 4 > 40)
            {
                DHCP_RELAY_PACKET_DEBUG(("\r\n circuit id value is too long! \r\n"));
                *(ptr ++) = 2;
                break;
            }
        
            dhcp_circuitid_value_lenth = pRequest->RevPktLen - 4;/*4���ֽڷֱ���msg_type,result,relay_type,relay_mode*/
            memcpy(dhcp_circuitid_value_head, pReq, dhcp_circuitid_value_lenth);
            *(dhcp_circuitid_value_head + dhcp_circuitid_value_lenth) = '\0';/*���ڴ�ӡstring*/        

        }
      
        *(ptr ++) = 1;
#endif
        break;
    }      

		case ONU_LPB_DETECT:
		{
			int nv = 0;
#if !HAVE_LOOP_DETECT
			extern unsigned long gulLoopDetectMode;
			extern long EthLoopbackDetectControl(unsigned long oamEnable, unsigned long localEnable);
#endif
			GWDOAMTRC("EQU_DEVICE_INFO_REQ - ONU_LPB_DETECT received.\n");

			memset(&tframe, 0, sizeof(OAM_ONU_LPB_DETECT_FRAME));
			memcpy(&tframe, pRequest->pPayLoad, pRequest->WholePktLen);
			memcpy(&(oam_onu_lpb_detect_frame.smac), &(tframe.smac), 6);
			GWDOAMTRC("  ONU_LPB_DETECT - smac : %x-%x-%x-%x-%x-%x\n", 
				tframe.smac[0], tframe.smac[1], tframe.smac[2], 
				tframe.smac[3], tframe.smac[4], tframe.smac[5]);

			nv = ntohs(tframe.vid);
			oam_onu_lpb_detect_frame.vid = nv;
			GWDOAMTRC("  ONU_LPB_DETECT - vid : %d\n", nv); 
			
			nv = ntohs(tframe.interval);
			oam_onu_lpb_detect_frame.interval = (nv)?nv:10;
			GWDOAMTRC("  ONU_LPB_DETECT - interval : %d\n", nv); 

			nv = ntohs(tframe.policy);
			oam_onu_lpb_detect_frame.policy = nv;
			GWDOAMTRC("  ONU_LPB_DETECT - policy : %d\n", nv); 

			if(LPB_OLD_VER_LEN == pRequest->WholePktLen)
			{
				oam_onu_lpb_detect_frame.waitforwakeup = 3;
				oam_onu_lpb_detect_frame.maxwakeup = 3;
			}
			else
			{
				nv = ntohs(tframe.waitforwakeup);
				oam_onu_lpb_detect_frame.waitforwakeup = nv;

				nv = ntohs(tframe.maxwakeup);
				oam_onu_lpb_detect_frame.maxwakeup = nv;
			}
			GWDOAMTRC("  ONU_LPB_DETECT - waitforwakeup : %d\n", oam_onu_lpb_detect_frame.waitforwakeup); 
			GWDOAMTRC("  ONU_LPB_DETECT - maxwakeup : %d\n", oam_onu_lpb_detect_frame.maxwakeup); 
			GWDOAMTRC("  ONU_LPB_DETECT - enable : %d\n", oam_onu_lpb_detect_frame.enable); 

#if !HAVE_LOOP_DETECT
			EthLoopbackDetectControl(tframe.enable, gulLoopDetectMode);
#endif
		}
		return GWD_RETURN_OK;	

#if 0
		case IP_RESOURCE_ALLOC:
			return spawnOnuUpdateTask(pRequest->pPayLoad+2, pRequest->WholePktLen-2, pRequest->SessionID);
			break;
		case IP_RESOURCE_FREE:
			g_ftpDataPathCtrl = 0;
			return sendIpSourcManAck(IP_RESOURCE_FREE, 0, pRequest->SessionID);
			break;
#endif

		default:
		{
			IROS_LOG_MAJ(IROS_MID_OAM, "OAM INFO request (%d) no suportted!", *pRequest->pPayLoad);
			GWDOAMTRC("EQU_DEVICE_INFO_REQ - unknown received.(%d)\n", *pRequest->pPayLoad);
			return GWD_RETURN_ERR;
		}
	}
	#if 0
	for(i=0;i < ResLen;i++)
		{
			if(i%20 == 0)
				{
				diag_printf("\n");
				}
			diag_printf("0x%x\n",Response[i]);
		}
	#endif
	return (CommOnuMsgSend(EQU_DEVICE_INFO_RESP, pRequest->SendSerNo, Response, ResLen, pRequest->SessionID));
}

static int GwOamAlarmResponse(GWTT_OAM_MESSAGE_NODE *pRequest )
{
	if(NULL == pRequest)
		return GWD_RETURN_ERR;
	if(NULL == pRequest->pPayLoad)
		return GWD_RETURN_ERR;
	switch((*pRequest->pPayLoad))
	{
		case ONU_TEMPRATURE_ALARM:
		{	
			break;
		}
		case ONU_ETH_PORT_STATE:
		{	
			break;
		}
		case ONU_ETH_PORT_ABILITY:
		{	
			break;
		}
		case ONU_ETH_WORK_STOP:
		{	
			break;
		}
		case ONU_STP_EVENT:
		{	
			break;
		}
		case ONU_DEVICE_INFO_CHANGE:
		{	
			break;
		}
		case ONU_SWITCH_STATUS_CHANGE_ALARM:
		{	
	        break;
		}
		case ONU_FILE_DOWNLOAD:
		{
#if 0
			if(UPGRADE_RESULT_OK == *(pRequest->pPayLoad + 3))
			{
				switch(*(pRequest->pPayLoad + 2))
				{
					case SOFTWARE_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_NOTICE, "Software Upgrade successed!");
						queueFileopCmd(pRequest->SessionID, FCMD_RECVALAMRESP);
						break;
					}
					case FIRMWARE_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_NOTICE, "Firmware Upgrade successed!");
						break;
					}
					case BOOT_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_NOTICE, "Boot file Upgrade successed!");
						break;
					}
					case CONFIG_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_NOTICE, "Config file Upgrade successed!");
						break;
					}
					case VOICE_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_NOTICE, "Voip Software Upgrade successed!");
						queueFileopCmd(pRequest->SessionID, FCMD_RECVALAMRESP);
						break;
					}
#if( RPU_HAVE_FPGA == RPU_YES )				
					case FPGA_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_NOTICE, "FPGA Upgrade successed!");
						queueFileopCmd(pRequest->SessionID, FCMD_RECVALAMRESP);
						break;
					}
#endif
					default:
						break;
				}				
				/* DEV_ResetMySelf(); */
			}
			else
			{
				switch(*(pRequest->pPayLoad + 2))
				{
					case SOFTWARE_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_ERR, "Software Upgrade failed!");
						queueFileopCmd(pRequest->SessionID, FCMD_RECVALAMRESP);
						break;
					}
					case FIRMWARE_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_ERR, "Firmware Upgrade failed!");
						break;
					}
					case BOOT_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_ERR, "Boot file Upgrade failed!");
						break;
					}
					case CONFIG_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_ERR, "Config file Upgrade failed!");
						break;
					}
					case VOICE_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_NOTICE, "Voip Software Upgrade successed!");
						queueFileopCmd(pRequest->SessionID, FCMD_RECVALAMRESP);
						break;
					}
#if( RPU_HAVE_FPGA == RPU_YES )				
					case FPGA_UPGRADE:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_ERR, "FPGA Upgrade failed!");
						queueFileopCmd(pRequest->SessionID, FCMD_RECVALAMRESP);
						break;
					}
#endif
					default:
					{
						VOS_SysLog(LOG_TYPE_OAM, LOG_ERR, "OAM Alarn response no suportted!");
						break;
					}
				}
			}
#endif
			break;
		}
		default:
			return GWD_RETURN_ERR;
	}
	return GWD_RETURN_OK;
}


/* Functions for debug */
int Debug_Print_Rx_OAM(GWTT_OAM_MESSAGE_NODE *pMessage)
{
    int i;
    diag_printf("\r\nRx OAM packet as following:\r\n");
    diag_printf("    GwOpCode:    %02X", pMessage->GwOpcode);
    switch (pMessage->GwOpcode)
    {
        case EQU_DEVICE_INFO_REQ:
            diag_printf("(EQU_DEVICE_INFO_REQ)\r\n");
            break;
        case EQU_DEVICE_INFO_RESP:
            diag_printf("(EQU_DEVICE_INFO_RESP)\r\n");
            break;
        case ALARM_REQ:
            diag_printf("(ALARM_REQ)\r\n");
            break;
        case ALARM_RESP:
            diag_printf("(ALARM_RESP)\r\n");
            break;
        case FILE_READ_WRITE_REQ:
            diag_printf("(FILE_READ_WRITE_REQ)\r\n");
            break;
        case FILE_RESERVER:
            diag_printf("(FILE_RESERVER)\r\n");
            break;
        case FILE_TRANSFER_DATA:
            diag_printf("(FILE_TRANSFER_DATA)\r\n");
            break;
        case FILE_TRANSFER_ACK:
            diag_printf("(FILE_TRANSFER_ACK)\r\n");
            break;
        case CHURNING:
            diag_printf("(CHURNING)\r\n");
            break;
        case DBA:
            diag_printf("(DBA)\r\n");
            break;
        case SNMP_TRAN_REQ:
            diag_printf("(SNMP_TRAN_REQ)\r\n");
            break;
        case SNMP_TRAN_RESP:
            diag_printf("(SNMP_TRAN_RESP)\r\n");
            break;
        case SNMP_TRAN_TRAP:
            diag_printf("(SNMP_TRAN_TRAP)\r\n");
            break;
        case CLI_REQ_TRANSMIT:
            diag_printf("(CLI_REQ_TRANSMIT)\r\n");
            break;
        case CLI_RESP_TRANSMIT:
            diag_printf("(CLI_RESP_TRANSMIT)\r\n");
            break;
        case IGMP_AUTH_TRAN_REQ:
            diag_printf("(IGMP_AUTH_TRAN_REQ)\r\n");
            break;
        case IGMP_AUTH_TRAN_RESP:
            diag_printf("(IGMP_AUTH_TRAN_RESP)\r\n");
            break;
        case CLI_PTY_TRANSMIT:
            diag_printf("(CLI_PTY_TRANSMIT)\r\n");
            break;
        default:
            diag_printf("(unknown)\r\n");
            break;
    }
    diag_printf("    SendSerNo:   %u\r\n", pMessage->SendSerNo);
    diag_printf("    WholePktLen: %u\r\n", pMessage->WholePktLen);
    diag_printf("    RevPktLen:   %u\r\n", pMessage->RevPktLen);
    diag_printf("    SessionID:   ");
    for (i=0; i<8; i++)
        diag_printf("%02X", pMessage->SessionID[i]);
    diag_printf("\r\n    TimerID:     %u\r\n", (unsigned int)((pMessage->TimerID).opaque));
    diag_printf("    Payload: \r\n");
    for (i=0; i<pMessage->RevPktLen; i++)
    {
        if ((i % 16) == 0)
            diag_printf("        ");
        diag_printf("%02X ", pMessage->pPayLoad[i]);
        if ((i % 16) == 15)
            diag_printf("\r\n");
    }
    diag_printf("\r\n");
    diag_printf("Total Rx OAM frames'number: %u\r\n", (unsigned int)gulDebugOamRxCount);
    return GWD_RETURN_OK;
}

int Debug_Print_Tx_OAM(GWTT_OAM_HEADER *avender, unsigned char *pSentData)
{
    int i;
	unsigned short wholePktLen;			/* The whole packet length, including the fragments */
	unsigned short payloadOffset;		/* Offset in the entire packet */
	unsigned short payLoadLength;		/* Payload length in this packet */

	wholePktLen = /*VOS_NTOHS*/(avender->wholePktLen); 		/* LD modified*/
	payloadOffset = /*VOS_NTOHS*/(avender->payloadOffset);	/* LD modified*/
	payLoadLength = /*VOS_NTOHS*/(avender->payLoadLength);	/* LD modified*/
	
    diag_printf("\r\nTx OAM packet as following:\r\n");
    diag_printf("    GwOUI:        %02X-%02X-%02X\r\n", avender->oui[0], avender->oui[1], avender->oui[2]);
    diag_printf("    GwOpCode:     %02X", avender->opCode);
    switch (avender->opCode)
    {
        case EQU_DEVICE_INFO_REQ:
            diag_printf("(EQU_DEVICE_INFO_REQ)\r\n");
            break;
        case EQU_DEVICE_INFO_RESP:
            diag_printf("(EQU_DEVICE_INFO_RESP)\r\n");
            break;
        case ALARM_REQ:
            diag_printf("(ALARM_REQ)\r\n");
            break;
        case ALARM_RESP:
            diag_printf("(ALARM_RESP)\r\n");
            break;
        case FILE_READ_WRITE_REQ:
            diag_printf("(FILE_READ_WRITE_REQ)\r\n");
            break;
        case FILE_RESERVER:
            diag_printf("(FILE_RESERVER)\r\n");
            break;
        case FILE_TRANSFER_DATA:
            diag_printf("(FILE_TRANSFER_DATA)\r\n");
            break;
        case FILE_TRANSFER_ACK:
            diag_printf("(FILE_TRANSFER_ACK)\r\n");
            break;
        case CHURNING:
            diag_printf("(CHURNING)\r\n");
            break;
        case DBA:
            diag_printf("(DBA)\r\n");
            break;
        case SNMP_TRAN_REQ:
            diag_printf("(SNMP_TRAN_REQ)\r\n");
            break;
        case SNMP_TRAN_RESP:
            diag_printf("(SNMP_TRAN_RESP)\r\n");
            break;
        case SNMP_TRAN_TRAP:
            diag_printf("(SNMP_TRAN_TRAP)\r\n");
            break;
        case CLI_REQ_TRANSMIT:
            diag_printf("(CLI_REQ_TRANSMIT)\r\n");
            break;
        case CLI_RESP_TRANSMIT:
            diag_printf("(CLI_RESP_TRANSMIT)\r\n");
            break;
        case IGMP_AUTH_TRAN_REQ:
            diag_printf("(IGMP_AUTH_TRAN_REQ)\r\n");
            break;
        case IGMP_AUTH_TRAN_RESP:
            diag_printf("(IGMP_AUTH_TRAN_RESP)\r\n");
            break;
        case CLI_PTY_TRANSMIT:
            diag_printf("(CLI_PTY_TRANSMIT)\r\n");
            break;
        default:
            diag_printf("(unknown)\r\n");
            break;
    }
    diag_printf("    SendSerNo:    %u\r\n", (unsigned int)(avender->senderSerNo));
    diag_printf("    WholePktLen:  %u\r\n", wholePktLen);
    diag_printf("    PayloadOffSet:%u\r\n", payloadOffset);
    diag_printf("    payLoadLength:%u\r\n", payLoadLength);
    diag_printf("    SessionID:    ");
    for (i=0; i<8; i++)
        diag_printf("%02X", avender->sessionId[i]);
    diag_printf("\r\n    Payload: \r\n");
    for (i=0; i<payLoadLength; i++)
    {
        if ((i % 16) == 0)
            diag_printf("        ");
        diag_printf("%02X ", pSentData[i]);
        if ((i % 16) == 15)
            diag_printf("\r\n");
    }
    diag_printf("\r\n");
    diag_printf("Total Tx OAM frames'number this time: %u\r\n", (unsigned int)gulDebugOamTxCount);
    return GWD_RETURN_OK;
}

int Debug_Print_Rx_OAM_Unkown(unsigned char *pBuffer, unsigned short len)
{
    int i;
    diag_printf("\r\nRx unknown OAM packet as following:\r\n");
    diag_printf("    Payload: \r\n");
    for (i=0; i<len; i++)
    {
        if ((i % 16) == 0)
            diag_printf("        ");
        diag_printf("%02X ", pBuffer[i]);
        if ((i % 16) == 15)
            diag_printf("\r\n");
    }
	return 0;
}

void ONU_Oam_BCStorm_Trap_Report_API(unsigned long slot, unsigned long port, unsigned char operate, unsigned char state,unsigned char *session)
{
	char temp[16]={0};
	
	*(temp) = 22;
	*(temp+3) = slot;
	*(temp+4) = port;
	*(temp+5) = state;
	*(temp+6) = operate;

	if(GWD_RETURN_OK != CommOnuMsgSend(ALARM_REQ,0,temp,7, session))
		diag_printf("Send ALARM_REQ for port %u broadcast storm detect trap failed.\r\n", (unsigned int)port);
	return;
}

int Onu_Sysinfo_Save_To_Flash(VOID)
{
	unsigned char *tempBuff = NULL;
    unsigned char *buff=NULL;
    int size=0;
	unsigned char *pConfig = NULL;
    int ret=0;

    buff=(unsigned char  *)&onu_system_info_total;
    size =sizeof (onu_system_info_total);       
       
    tempBuff = iros_malloc(IROS_MID_OAM, FLASH_USER_DATA_MAX_SIZE);
	if(tempBuff == NULL) {
       diag_printf("Config save failed\n");
       ret= GWD_RETURN_ERR; 
	   goto END;
	}
	memset(tempBuff, 0x00, FLASH_USER_DATA_MAX_SIZE);
	user_data_config_Read(0,tempBuff, FLASH_USER_DATA_MAX_SIZE);
	pConfig = (unsigned char *)(tempBuff + GWD_PRODUCT_CFG_OFFSET);
	memcpy(pConfig,buff,size);

	user_data_config_Write(tempBuff, FLASH_USER_DATA_MAX_SIZE);
	if (tempBuff !=NULL ) {
	    iros_free(tempBuff);
	}

END:
    return ret;
}

int Onu_Sysinfo_Get_From_Flash(VOID)
{
#if 1
	int ret=GWD_RETURN_OK;
	int iLastChar;
	//unsigned char ucsDeviceNameDef[] = "GT811_D";

	memset(&onu_system_info_total, 0, sizeof(onu_system_info_total));
	
	if (GWD_RETURN_OK != (ret = get_userdata_from_flash((unsigned char *)&onu_system_info_total, GWD_PRODUCT_CFG_OFFSET,  sizeof(onu_system_info_total))))
	{
		memset(&onu_system_info_total, 0, sizeof(onu_system_info_total));
		IROS_LOG_MAJ(IROS_MID_OAM, "Read system info from flash failed.(%d)\r\n", ret);
		ret = GWD_RETURN_ERR;
	}
	/* Avoid invalid string data */
//	if('E' != onu_system_info_total.valid_flag)
	//{
	//	memcpy(onu_system_info_total.device_name, ucsDeviceNameDef, sizeof(ucsDeviceNameDef));
	//}
	iLastChar = sizeof(onu_system_info_total.device_name) - 1;
	onu_system_info_total.device_name[iLastChar] = '\0';
	iLastChar = sizeof(onu_system_info_total.serial_no) - 1;
	onu_system_info_total.serial_no[iLastChar] = '\0';
	iLastChar = sizeof(onu_system_info_total.hw_manufature_date) - 1;
	onu_system_info_total.hw_manufature_date[iLastChar] = '\0';

	onu_system_info_total.product_type = DEVICE_TYPE_GT870;
	sprintf(onu_system_info_total.sw_version, "V%dR%02dB%03d", 
		SYS_SOFTWARE_MAJOR_VERSION_NO,
		SYS_SOFTWARE_RELEASE_VERSION_NO,
		SYS_SOFTWARE_BRANCH_VERSION_NO);
	
	return ret;
	#else
	int ret=GWD_RETURN_OK;
	int iLastChar;
	unsigned char ucsDeviceNameDef[] = "GT810_D";

	memset(&onu_system_info_total, 0, sizeof(onu_system_info_total));
#if 0
	if (GWD_RETURN_OK != (ret = user_data_config_Read(GWD_PRODUCT_CFG_OFFSET, (unsigned char *)&onu_system_info_total, sizeof(onu_system_info_total))))
#else
	if (GWD_RETURN_OK != (ret = get_userdata_from_flash((unsigned char *)&onu_system_info_total, GWD_PRODUCT_CFG_OFFSET,  sizeof(onu_system_info_total))))
#endif
	{
		memset(&onu_system_info_total, 0, sizeof(onu_system_info_total));
		IROS_LOG_MAJ(IROS_MID_OAM, "Read system info from flash failed.(%d)\r\n", ret);
		ret = GWD_RETURN_ERR;
	}
	/* Avoid invalid string data */
	if('E' != onu_system_info_total.valid_flag)
	{
		memcpy(onu_system_info_total.device_name, ucsDeviceNameDef, sizeof(ucsDeviceNameDef));
	}
	iLastChar = sizeof(onu_system_info_total.device_name) - 1;
	onu_system_info_total.device_name[iLastChar] = '\0';
	iLastChar = sizeof(onu_system_info_total.serial_no) - 1;
	onu_system_info_total.serial_no[iLastChar] = '\0';
	iLastChar = sizeof(onu_system_info_total.hw_manufature_date) - 1;
	onu_system_info_total.hw_manufature_date[iLastChar] = '\0';

	onu_system_info_total.product_type = DEVICE_TYPE_GT870;
	sprintf(onu_system_info_total.sw_version, "V%dR%02dB%03d", 
		SYS_SOFTWARE_MAJOR_VERSION_NO,
		SYS_SOFTWARE_RELEASE_VERSION_NO,
		SYS_SOFTWARE_BRANCH_VERSION_NO);
	
	return ret;
	#endif
}

int Onu_Sysinfo_Save(void)
{
#if 0
	/* Save to flash */
	onu_system_info_total.product_type = DEVICE_TYPE_GT870;
	onu_system_info_total.valid_flag = 'E';
	/*sprintf(onu_system_info_total.sw_version, "V%dR%02dB%03d", 
		SYS_SOFTWARE_MAJOR_VERSION_NO,
		SYS_SOFTWARE_RELEASE_VERSION_NO,
		SYS_SOFTWARE_BRANCH_VERSION_NO);
	sprintf(onu_system_info_total.hw_version, "V%d.%d", 
		SYS_HARDWARE_MAJOR_VERSION_NO,
		SYS_HARDWARE_RELEASE_VERSION_NO);*/
	
	return Onu_Sysinfo_Save_To_Flash();
#else
#if 0
	unsigned char *tempBuff = NULL;
#endif
    unsigned char *buff=NULL;
    int size=0;
#if 0
	unsigned char *pConfig = NULL;
#endif
    int ret=0;

    buff=(unsigned char  *)&onu_system_info_total;
    size =sizeof (onu_system_info_total);

#if 0
    tempBuff = iros_malloc(IROS_MID_OSAL, FLASH_USER_DATA_MAX_SIZE);
	if(tempBuff == NULL) {
       diag_printf("Config save failed\n");
       ret= GWD_RETURN_ERR; 
	   goto END;
	}
    
	memset(tempBuff, 0x00, FLASH_USER_DATA_MAX_SIZE);
	user_data_config_Read(0,tempBuff, FLASH_USER_DATA_MAX_SIZE);
	pConfig = (unsigned char *)(tempBuff + GWD_PRODUCT_CFG_OFFSET);
	memcpy(pConfig,buff,size);

	user_data_config_Write(tempBuff, FLASH_USER_DATA_MAX_SIZE);
	if (tempBuff !=NULL ) {
	    iros_free(tempBuff);
	}

END:
#else
	ret = save_userdata_to_flash(buff, GWD_PRODUCT_CFG_OFFSET, size);
#endif
    return ret;
#endif
}

int Onu_Sysinfo_Get(void)
{
    return Onu_Sysinfo_Get_From_Flash();
}

extern void epon_onu_start_alarm_led();
extern void epon_onu_stop_alarm_led();
extern void onu_interrupt_disable_w(cs_uint32 uart_port);
extern void onu_interrupt_enable_w(cs_uint32 uart_port);
#if 1
void uart_resd_write_active(void*date)
{
	//cs_printf("uart acticv sed or rec\n");
	cs_uint32 port;
	STOP_UART_SEND_RCE = 1;
	for(port = 1; port < 5; port++)
		{
			onu_interrupt_enable_w(port);
		}
}
#endif

#if 1
extern cs_status ip_mode_set(int mode);
extern cs_status ip_mode_get(int *mode);
#endif




typedef unsigned int in_addr_t;
extern int set_uart_cfg(cs_uint32 uart_port,cs_uint32 act);
extern cyg_uint32 iros_iszero( const char *nptr);
extern in_addr_t inet_addr(const char *cp);
extern int cs_atoi(char *str);
extern onu_slow_path_cfg_cfg_t   g_slow_path_ip_cfg;
extern cs_status app_ipintf_add_ip_address(cs_uint32 localIp, cs_uint32 gwIp, cs_uint32 mask);

#if 0
int cmd_onu_uart_ip_set(struct cli_def *cli, char *command, char *argv[], int argc)
{
		cyg_uint32 g_uart_ip ; 
		cyg_uint32 g_uart_mask;   
		cyg_uint32 g_uart_gateway; 
		cs_int16 vlan;
		unsigned char *buff=NULL;
		int size=0;
		int ret;
		if(argc!=4)
		 {
		     cli_print(cli, "%% Invalid input");
		     cli_print(cli, " <IP address>  <Subnet Mask> <GateWay> <Management Vlan>");  
		     return CLI_ERROR;
		 }
		 if( ((!iros_iszero(argv[3]))&&(iros_strtol(argv[3])==0))
		     ||(!(iros_strtol(argv[3])>=0 && iros_strtol(argv[3]) <=4094) ))
		 {
		     cli_print(cli, "%% Invalid Vlan ID");
		     return CLI_ERROR;
		 };
		g_uart_ip = htonl(inet_addr(argv[0]));
		g_uart_mask = htonl(inet_addr(argv[1]));
		g_uart_gateway = htonl(inet_addr(argv[2]));
		vlan = iros_strtol(argv[3]);
		ret = get_userdata_from_flash((unsigned char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_INBAND, sizeof(g_slow_path_ip_cfg));
		if(ret)
			{
				cs_printf("get ipdate fail..\n");
			}
		g_slow_path_ip_cfg.uart_ip=g_uart_ip;
		g_slow_path_ip_cfg.uart_mask=g_uart_mask;
		g_slow_path_ip_cfg.uart_gateway=g_uart_gateway;
		g_slow_path_ip_cfg.uart_vlan=vlan;
		buff=(unsigned char *)&g_slow_path_ip_cfg;
		size=sizeof(g_slow_path_ip_cfg);
		ret = save_userdata_to_flash(buff, GWD_PRODUCT_CFG_OFFSET_INBAND, size);
		if(ret)
		{
		 cs_printf( "%% Save fail .");
		}

		cs_printf("set uart server ip success\n");

		#if 1
		ip_mode_set(0);
		cs_uint32 device_ip = 0;
		cs_uint32 device_mask = 0;
		cs_uint32 device_gateway = 0;
		device_ip = g_uart_ip;
		device_mask = g_uart_mask;
		device_gateway = g_uart_gateway;
		app_ipintf_add_ip_address(device_ip, device_gateway, device_mask);
		ip_mode_set(1);
		#endif

  return 1;
}
#endif

#if 0
int cmd_onu_uart_ip_show(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int ret;
	struct sockaddr_in addrp;
	extern char *inet_ntoa(struct in_addr addr);
	ret = get_userdata_from_flash((unsigned char *)&g_slow_path_ip_cfg, GWD_PRODUCT_CFG_OFFSET_W, sizeof(g_slow_path_ip_cfg));
	if(ret)
		{
			cs_printf("get ipdate fail..\n");
		}

	#if 0
	cli_print(cli,"=========================================================================");
    memset(&addrp, 0, sizeof(addrp));
    addrp.sin_addr.s_addr = htonl(g_slow_path_ip_cfg.inband_ip);
    cli_print(cli,"SERVER IP - %s", inet_ntoa(addrp.sin_addr));
     addrp.sin_addr.s_addr = htonl(g_slow_path_ip_cfg.inband_ip_mask);
    cli_print(cli,"SERVER IP Submask - %s", inet_ntoa(addrp.sin_addr));
     addrp.sin_addr.s_addr = htonl(g_slow_path_ip_cfg.inband_gateway);
    cli_print(cli,"SERVER IP Gateway - %s", inet_ntoa(addrp.sin_addr));
    cli_print(cli,"SERVER IP Management Vlan - %d", g_slow_path_ip_cfg.inband_vlanid);
	cli_print(cli,"=========================================================================");
	#else
	cli_print(cli,"=========================================================================");
    memset(&addrp, 0, sizeof(addrp));
    addrp.sin_addr.s_addr = htonl(g_slow_path_ip_cfg.uart_ip);
    cli_print(cli,"SERVER IP - %s", inet_ntoa(addrp.sin_addr));
     addrp.sin_addr.s_addr = htonl(g_slow_path_ip_cfg.uart_mask);
    cli_print(cli,"SERVER IP Submask - %s", inet_ntoa(addrp.sin_addr));
     addrp.sin_addr.s_addr = htonl(g_slow_path_ip_cfg.uart_gateway);
    cli_print(cli,"SERVER IP Gateway - %s", inet_ntoa(addrp.sin_addr));
    cli_print(cli,"SERVER IP Management Vlan - %d", g_slow_path_ip_cfg.uart_vlan);
	cli_print(cli,"=========================================================================");
	#endif


	return 1;
}

#endif

int cmd_onu_uart_mode_chang(struct cli_def *cli, char *command, char *argv[], int argc)
{
	cs_uint32 num;
	unsigned char act;
	cs_uint8 gpio_id;
	cs_uint32 port;
	cs_uint32 ret;
	if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<1-4>", "uart number",
                 NULL);
        case 2:
            return cli_arg_help(cli, 0,
                "[0|1]", "0:RS485,1:RS232",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }
	num = atoi(argv[0]);
	switch(num)
		{
			case 1:
				gpio_id = 14;
				break;
			case 2:
				gpio_id = 12;
				break;
			case 3:
				gpio_id = 10;
				break;
			case 4:
				gpio_id = 13;
				break;
				default:
					cs_printf("uart number input error  only <uartnum:1-4>\n");
					return 0;
		}
 	if(1 == argc)
	    {
	    	
			if(num > 4 || num < 1)
				{
					cs_printf("uart number input error\n");
					return 0;
				}

			if ( CS_E_OK == cs_gpio_mode_set(gpio_id, GPIO_OUTPUT))
			{																 			
			   if(cs_gpio_read(gpio_id, &act) == EPON_RETURN_SUCCESS)
		    	{
		    		cli_print(cli, "current uart %d mode is %s", num,act?"RS232":"RS485");
		    	}
		    	else
		    		cli_print(cli, "uart mode get fail!");								
			}
	    }
    else if(2 == argc)
	    {
			act = atoi(argv[1]);

			if(num < 1 || num > 4)
				return CLI_ERROR_ARG;

			if(act > 1)
				return CLI_ERROR_ARG;

			if ( CS_E_OK == cs_gpio_mode_set(gpio_id, GPIO_OUTPUT))
			{		

		     STOP_UART_SEND_RCE = 0;
			#if 1
			   for(port = 1; port < 5; port++)
			   	{
			   		onu_interrupt_disable_w(port);
			   	}
			#endif
			   if(cs_gpio_write(gpio_id, act) == EPON_RETURN_SUCCESS)
		    	{
		    	#if 1
					cs_timer_add(4,uart_resd_write_active,NULL);
					if(act)
						{
							//cs_printf("................................1\n");
							act = 0;
							ret=set_uart_cfg(num,act);
							if(ret)
								cs_printf("set uart %d duplex ful success\n",num);
						}
					else
						{
							//cs_printf(".......................................2\n");
							act = 1;
							ret=set_uart_cfg(num,act);
							if(ret)
								cs_printf("set uart %d duplex hel success\n",num);
						}
				#endif
		    		cli_print(cli, "set uart %d mode is %s success", num,act?"RS485":"RS232");
		    	}
		    	else
		    		cli_print(cli, "set uart %d mode %s fail!",num,act?"RS232":"RS485");								
			}

	    }
	else
	    {
	        cli_print(cli, "%% Invalid input.");
	    }
	return 0;
}
int cmd_onu_mgt_gpiotest(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int num;
	unsigned char act;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<0-15>", "gpio number",
                 NULL);
        case 2:
            return cli_arg_help(cli, 0,
                "[0|1]", "0:low level,1:high level",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {
    	num = atoi(argv[0]);
    	if(cs_gpio_read(num, &act) == EPON_RETURN_SUCCESS)
    	{
    		cli_print(cli, "current level is %s", act?"high":"low");
    	}
    	else
    		cli_print(cli, "gpio status get fail!");
    }
    else if(2 == argc)
    {
		num = atoi(argv[0]);
		act = atoi(argv[1]);

		if(num < 0 || num > 15)
			return CLI_ERROR_ARG;

		if(act > 1)
			return CLI_ERROR_ARG;

		if(cs_gpio_write(num, act) != EPON_RETURN_SUCCESS)
			cli_print(cli, "gpio status set fail!");

    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}

int cmd_onu_mgt_gpiodirect(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int num;
	gpio_mode_t direct;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<0-15>", "gpio number",
                 NULL);
        case 2:
            return cli_arg_help(cli, 0,
                "[0|1]", "0:output,1:input",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {
    	char c_dir[80]= "";
    	num = atoi(argv[0]);
    	cs_gpio_mode_get(num,&direct);
    	switch (direct)
    	{
    	case GPIO_OUTPUT:
    		strcpy(c_dir, "output");
    		break;
    	case GPIO_INPUT:
    		strcpy(c_dir, "input");
    		break;
    	case GPIO_INPUT_FALLING:
    		strcpy(c_dir, "failling");
    		break;
		case GPIO_INPUT_RISING:
    		strcpy(c_dir, "rising");
    		break;
    	default:
    		strcpy(c_dir, "unknown");
    		break;
    	}
    	cli_print(cli, "current direction is %s", c_dir);
    }
    else if(2 == argc)
    {
		num = atoi(argv[0]);
		direct = atoi(argv[1]);

		if(num < 0 || num > 15)
			return CLI_ERROR_ARG;

		if(direct > 1)
			return CLI_ERROR_ARG;

		if(cs_gpio_mode_set(num, direct) != EPON_RETURN_SUCCESS)
			cli_print(cli, "gpio direction set fail!");

    } else
    {
        cli_print(cli, "%% Invalid input.");
    }

    return CLI_OK;
}


int cmd_onu_mgt_config_product_date(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int year, month, date;
        
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<2007-2100>", "Year",
                 NULL);
        case 2:
            return cli_arg_help(cli, 0,
                "<1-12>", "Month",
                 NULL);
        case 3:
            return cli_arg_help(cli, 0,
                "<1-31>", "date",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(3 == argc)
    {   
		year = atoi(argv[0]);
		month = atoi(argv[1]);
		date = atoi(argv[2]);

		Onu_Sysinfo_Get();
		sprintf(onu_system_info_total.hw_manufature_date, 
			   	"%d-%02d-%02d", year, month, date);
        
		if (GWD_RETURN_OK != Onu_Sysinfo_Save())
		{
			cli_print(cli, "  System information save error!\r\n");
		}
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}

int cmd_onu_mgt_config_product_hw_version(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int v_major, v_rel;
        
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<1-9>", "Major version",
                 NULL);
        case 2:
            return cli_arg_help(cli, 0,
                "<1-9>", "Release version",
                 NULL);
	//	case 3:
          //  return cli_arg_help(cli, 0,
            //    "<1-9>", "B version",
             //    NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(2 == argc)
    {   
		v_major = atoi(argv[0]);
		v_rel = atoi(argv[1]);
	//	V_B = atoi(argv[2]);

		sprintf(onu_system_info_total.hw_version, "V%d.%d", 
			v_major, v_rel);
        
		if (GWD_RETURN_OK != Onu_Sysinfo_Save())
		{
			cli_print(cli, "  System information save error!\r\n");
		}
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}

int startup_config_field_idx_get_w(cs_uint32 type)
{
    int i;
   // int total_field = 0;

   // total_field = sizeof(glb_field)/sizeof(glb_field[0]);
    for(i = 0 ; i < 20 ; i++)
    {
        if(glb_field[i].type == type)
            return i;
    }

    return -1;
}
int cmd_set_mod_id(struct cli_def *cli, char *command, char *argv[], int argc)
{
     int type = 0;
    int field_idx = -1;
    cfg_field_t *pField = NULL;
	unsigned char buff[32];
    unsigned char *pChar = buff;
  //  unsigned short *pShort = (unsigned short *)buff;
   // unsigned int *pInt = (unsigned int *)buff;
	 if(CLI_HELP_REQUESTED)
   	 {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "type number", "only == 6",
                 NULL);
		case 2:
            return cli_arg_help(cli, 0,
                "<xx:xx:xx:xx>", "only 4 byte",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
		
     }
	if(argc > 1)
		{
		    type = atoi(argv[0]);
		    field_idx = startup_config_field_idx_get_w(type);
		    if(field_idx == -1)
		    {
		        cs_printf("Invalid type.\n");
		        return -1;
		    }
		    pField = &glb_field[field_idx];
			memset(pChar , 0 , sizeof(buff));
		    if(cs_str2hex(argv[1], (char *)pChar, pField->value_max_len))
		        cs_printf("\n%s\n",pField->help_str);
		    else
		    {
		        if(pField->type == CFG_ID_MAC_ADDRESS && (pChar[0]&0x01))
		        {
		            cs_printf("ONU MAC must not be a multicast mac\n");
		            return 1;
		        }
		        startup_config_write(pField->type, pField->value_max_len, (unsigned char *)pChar);
		    }
		}
	else
		{
			cs_printf("input error\n");
		}
	return 0;
	
}
int cmd_onu_mgt_config_product_sn(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int  len, i;
	unsigned char tmpStr[18];

        
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<string>", "Manufacture serial number(length<16)",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {   
		if((len = strlen(argv[0])) > 16)
		{
			cli_print(cli, "  The length of serial number must be less than %d.\r\n", 16);
			return CLI_OK;
		}

		for(i=0; i<len; i++)
			tmpStr[i] = TOUPPER(argv[0][i]);
		tmpStr[i] = '\0';
		
		Onu_Sysinfo_Get();
		sprintf(onu_system_info_total.serial_no, "%s", tmpStr);

		if (GWD_RETURN_OK != Onu_Sysinfo_Save())
		{
			cli_print(cli, "  System information save error!\r\n");
		}

		return CLI_OK;
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}

int cmd_onu_mgt_config_device_name(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int  len, i;
	unsigned char tmpStr[129];

        
    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<string>", "Device name(length<=128)",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(1 == argc)
    {   
		if((len = strlen(argv[0])) > 128)
		{
			cli_print(cli, "  The length of device name cannot be more than %d.\r\n", 128);
			return CLI_OK;
		}

		for(i=0; i<len; i++)
			tmpStr[i] = TOUPPER(argv[0][i]);
		tmpStr[i] = '\0';
		
		Onu_Sysinfo_Get();
		sprintf(onu_system_info_total.device_name, "%s", tmpStr);

		if (GWD_RETURN_OK != Onu_Sysinfo_Save())
		{
			cli_print(cli, "  System information save error!\r\n");
		}

		return CLI_OK;
    } else
    {
        cli_print(cli, "%% Invalid input.");
    }
    
    return CLI_OK;
}

int cmd_show_system_information(struct cli_def *cli, char *command, char *argv[], int argc)
{
#ifdef HAVE_TERMINAL_SERVER
	char onu_type[] = "GT873M_4F4S";
#else
	#ifdef HAVE_SWITCH_SPEED_1000
	char onu_type[] = "GT811G";
	#endif
	#ifdef HAVE_SWITCH_SPEED_100
	char onu_type[] = "GT811D";
	#endif
#endif
	long lRet = GWD_RETURN_OK;
    char strMac[32];
	extern int cli_get_onu_mac_addr(char *mac);
	extern char const iros_version[];
	
    cli_get_onu_mac_addr(strMac);
        
	lRet = Onu_Sysinfo_Get();
	if (lRet != GWD_RETURN_OK)
	{
		cli_print(cli, "  Get product information from flash with error.\r\n");
		return CLI_OK;
	}
	else
	{
	#if 1
		cli_print(cli,  "\n  Product information as following--");
		cli_print(cli,  "    ONU type         : %s", onu_type);
		cli_print(cli,  "    DeiveName        : %s", onu_system_info_total.device_name);
		cli_print(cli,  "    Hardware version : %s", onu_system_info_total.hw_version);
		cli_print(cli,  "    Software version : %s", onu_system_info_total.sw_version);
		cli_print(cli,  "    Firmware version : %s", iros_version);
		cli_print(cli,  "    Bootload version : %s", irosbootver);
		cli_print(cli,  "    Manufature date  : %s", onu_system_info_total.hw_manufature_date);
		cli_print(cli,  "\n    Serial number    : %s", onu_system_info_total.serial_no);
    	cli_print(cli,  "    Onu mac address  : %s", strMac);
	#else
		cs_printf("Product information as following--\n");
		cs_printf("    ONU type         : %s\n", "GT811D");
		cs_printf("    DeiveName        : %s\n", onu_system_info_total.device_name);
		cs_printf("    Hardware version : %s\n", onu_system_info_total.hw_version);
		cs_printf("    Software version : %s\n", onu_system_info_total.sw_version);
		cs_printf("    Firmware version : %s\n", iros_version);
		cs_printf("    Bootload version : %s\n", irosbootver);
		cs_printf("    Manufature date  : %s\n", onu_system_info_total.hw_manufature_date);
		cs_printf("    Serial number    : %s\n", onu_system_info_total.serial_no);
    	cs_printf("    Onu mac address  : %s\n", strMac);
	#endif

		return CLI_OK;
	}
}

int cmd_show_optical_diagnostics(struct cli_def *cli, char *command, char *argv[], int argc)
{
	long lRet = GWD_RETURN_OK;
	//epon_return_code_t ret = EPON_RETURN_SUCCESS;
	cs_int16 temp = 0;
	cs_uint16 vcc = 0, bias =0, txpow =0, rxpow=0;
	cs_status ret = CS_E_OK;
    cs_callback_context_t   context;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        default:
            return cli_arg_help(cli, 1, NULL);
        }
    }


    ret = cs_plat_i2c_opm_status_read(context,0,0,&temp,
            &vcc, &bias, &txpow,&rxpow);
//	ret = epon_request_ctc_onu_opm_diagnostics_read(&temp, &vcc, &bias, &txpow, &rxpow);
	if (lRet != EPON_RETURN_SUCCESS)
	{
		cli_print(cli, "  Get optical module diagnostics from I2C with error.\r\n");
		return CLI_OK;
	}
	else
	{
		double txdbm = 0.9, rxdbm = 0.0;
		temp = temp/256;
		vcc = (cs_uint16)(vcc*0.1);
		bias = bias*0.002;
		txdbm = txpow;
		rxdbm = rxpow;

		txdbm = 10*log10(txdbm*0.0001);
		rxdbm = 10*log10(rxdbm*0.0001);
		cli_print(cli,  "\n  optical module diagnostics as following--");
		cli_print(cli,  "    temperature      : %d  cel", temp);
		cli_print(cli,  "    voltage          : %d  mV", vcc);
		cli_print(cli,  "    bias current     : %d  mA", bias);
		cli_print(cli,  "    tx power         : %4.1f  dbm", txdbm);
		cli_print(cli,  "    rx power         : %4.1f  dbm", rxdbm);

		return CLI_OK;
	}
}
void swt_show_fdb_entry(cs_sdl_fdb_entry_t *entry)
{
        int i = 0;
        
        diag_printf("    Addr: ");
        for(i=0; i<6; i++)
        {
            diag_printf("%02x ", entry->mac.addr[i]);
        }
        
       diag_printf("    status:%d",entry->type);
        diag_printf("    vlan Id:%4d", entry->vlan_id);
        diag_printf("    egrss ports : %d",entry->port);
        diag_printf("\n");
}
#if 0
int cmd_show_atu(struct cli_def *cli, char *command, char *argv[], int argc)
{
	epon_vlan_learning_t mv_fdb_learningmode = EPON_SHARED_VLAN_LEARNING;
    cs_sdl_fdb_entry_t entry;
	cs_uint16 idx = 0, nextidx=0;
	cs_callback_context_t context;
	int i,j;
    cs_uint16 port = 0xff, vlanid = 0;

    cs_uint32 index = 0;

    memset(&entry, 0, sizeof(entry));

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        case 1:
            return cli_arg_help(cli, 0,
                "<1-4|11>", "port id, 11--indicates the nni port",
                 NULL);
        case 2:
            return cli_arg_help(cli, 0,
                "<1-4094>", "vlan id",
                 NULL);
        default:
            return cli_arg_help(cli, argc > 1, NULL);
        }
    }

    if(argc >= 1)
    	port = iros_strtol(argv[0]);

    if(argc == 2)
    	vlanid = iros_strtol(argv[1]);

    cli_print(cli,  "\n-------------- FDB ENTRIES ----------------- \n");

    cli_print(cli, "fdb learning mode is %s\n", mv_fdb_learningmode == EPON_SHARED_VLAN_LEARNING ?"SVL":"IVL");
    entry.vlan_id = vlanid;
	for(i = 1; i < 5; i++)
		{
			diag_printf("port:%d\n",i);
		   while(epon_request_onu_fdb_entry_get_byindex_per_port(context, 0, 0, i, SDL_FDB_ENTRY_GET_MODE_ALL,
									idx, &entry, &nextidx) == CS_OK)
		    	{
		    		for(j=0; j < 6; j++)
		    			{
		    				diag_printf("%02x\n",entry.mac.addr[1]);
		    			}
	    			idx = nextidx;
	    			if(idx > 16)
	    				{
							break;
	    				}
					 if (entry.vlan_id == 0) {
			            continue;
			        }
			        /* learning entry aging out */
			        
			        if ((entry.mac.addr[0] == 0) &&
			            (entry.mac.addr[1] == 0) &&
			            (entry.mac.addr[2] == 0) &&
			            (entry.mac.addr[3] == 0) &&
			            (entry.mac.addr[4] == 0) &&
			            (entry.mac.addr[5] == 0)) {
			            continue;
			        	}
	                swt_show_fdb_entry(&entry);
		        }
		}

    if(index == 0)
        cli_print(cli,  "No entry in FDB!");

    cli_print(cli,  "\n------------- FDB ENTRIES END -------------- ");

    return CLI_OK;
}
#else

cs_uint32 learn_buf[4]={1,1,1,1};

int fdb_learn_set(int portid, int en)
{
	int ret = CS_E_OK;
 	cs_status  status = 0;
	cs_callback_context_t context;
	
    if (en == 1) 
	{
		status = epon_request_onu_mac_learn_set(context, 0, 0, portid, 1);
    }
	else if (en == 0) 
    {
        status = epon_request_onu_mac_learn_set(context, 0, 0, portid, 0);
    } 
	else 
	{
        goto error;
    }
	
	if(status == CS_E_OK)
	{
		if(en == 1)
		{	
			learn_buf[portid-1] = 1;
		}
		else
		{
			learn_buf[portid-1] = 0;
		}
	}

	ret = CS_E_OK;
	goto end;
	
error:
	ret = CS_E_ERROR;
end:
	return ret;
}


int fdb_learn_get(int portid, int *en)
{
	int ret = 0;

	*en = learn_buf[portid-1];
	return ret;
}


int cmd_show_atu(struct cli_def * cli, char *command, char *argv[], int argc)
{
	int ret = CLI_OK;
	cs_callback_context_t context;
	cs_uint16 idx = 0, next = 0;

	cs_sdl_fdb_entry_t entry;

    // deal with help
    if(CLI_HELP_REQUESTED)
    {
        switch(argc)
        {
        default:
            return cli_arg_help(cli, 1, NULL);
        }
    }

    cli_print(cli, "====== FDB SW table is shown:======");
    cli_print(cli, "index   mac_address        vid   port type ");

    while(epon_request_onu_fdb_entry_get_byindex(context, 0, 0, SDL_FDB_ENTRY_GET_MODE_ALL, idx, &entry, &next) == CS_OK)
    {
    	cs_uint16 vid = entry.vlan_id?entry.vlan_id:1;
    	idx = next;
        cli_print(cli, " %2d   %02x:%02x:%02x:%02x:%02x:%02x %6d   %2d   %2d  ", idx,
            entry.mac.addr[0],
            entry.mac.addr[1],
            entry.mac.addr[2],
            entry.mac.addr[3],
            entry.mac.addr[4],
            entry.mac.addr[5],
            vid,
            entry.port,
            entry.type);
    }
    cli_print(cli, "====== Totally %2d SW entries====\n", idx);

	return ret;
}


#define GW_CLI_INCOMPLETE_MSG "%% Command incomplete.\r\n"

int cmd_oam_atu_learn(struct cli_def * cli, char *command, char *argv[], int argc)
{
	int ret = CLI_OK;
	
	int en = 0, portid = 0;

	if(CLI_HELP_REQUESTED)
	{
		switch (argc)
		{
			case 1:
				return cli_arg_help(cli, 0, 
					"<portlist>", "Input one fe port number", NULL );
				break;
			case 2:
				return cli_arg_help(cli, 0,
					"{[1|0]}*1", "1 enable; 0 disable", NULL);
				break;

			default:
				return cli_arg_help(cli, argc > 1, NULL  );
				break;
		}
	}
 
	if(argc >= 1)
	{
		portid = atoi(argv[0]);

		if(argc == 2)
			en = atoi(argv[1]);

		if(argc == 2 )
		{
			if(CS_E_OK != fdb_learn_set(portid,  en))
				cli_print(cli, "atu learning set %s fail!\r\n", en?"enable":"disable");
		}
		else
		{
			if(CS_E_OK != fdb_learn_get(portid, &en))
				cli_print(cli, "get port %d atu learning fail!\r\n", portid);
			else
				cli_print(cli,"Port %d source mac address learn is %s\r\n",portid,en?"enable":"disable");
		}
	}
	else
	{
		cli_print(cli, GW_CLI_INCOMPLETE_MSG);
		return CLI_ERROR;
	}
	return ret;
}

extern cs_status epon_request_onu_fdb_age_set (
    CS_IN cs_callback_context_t          context,
    CS_IN cs_int32                       device_id,
    CS_IN cs_int32                       llidport,
    CS_IN cs_uint32                      aging_time
);
cs_status epon_request_onu_fdb_age_get (
    CS_IN  cs_callback_context_t                   context,
    CS_IN  cs_int32                                device_id,
    CS_IN  cs_int32                                llidport,
    CS_OUT cs_uint32                               *aging_time
);

int fdb_aging_set(cs_uint32 timer)
{
	int ret = CS_E_OK;
    cs_callback_context_t context;

    cs_status  status = 0;

    status = epon_request_onu_fdb_age_set(context, 0, 0, timer);
    if(status != CS_E_OK)
	{
		ret = CS_E_ERROR;
    }
	else
	{
		ret = CS_E_OK;
	}

    return ret;
}

int fdb_aging_get(cs_uint32 *timer)
{
	int ret = CS_E_OK;
    cs_callback_context_t context;

    cs_status  status = 0;

    status = epon_request_onu_fdb_age_get(context, 0, 0, timer);
    if(status != CS_E_OK)
	{
		ret = CS_E_ERROR;
    }
	else
	{
		ret = CS_E_OK;
	}

    return ret;
}

int cmd_oam_atu_age(struct cli_def * cli, char *command, char *argv[], int argc)
{
	int ret = CS_E_OK;
	int age = 0;

	if(CLI_HELP_REQUESTED)
	{
		switch (argc)
		{
			case 1:
				return cli_arg_help(cli, 0,
					"<0-600>", "l2 age time unit sec, 0: disable aging", NULL);

			default:
				return cli_arg_help(cli, argc > 1, NULL  );
		}
	}

	if(argc == 1)
	{
		age = atoi(argv[0]);
		if(age < 0 ||age > 600)
			{
				cli_print(cli,"set aging time error \n");
				return CLI_ERROR;
			}
		if(CS_E_OK != fdb_aging_set(age))
			cli_print(cli, "atu age set %d fail!\r\n", age);
		else
			cli_print(cli,"atu aging time set %d sucess",age);
	}
	else
	{
		if(CS_E_OK != fdb_aging_get(&age))
			cli_print(cli, "get atu aging time fail!\r\n");
		else
			cli_print(cli, "Mac table aging time is %d seconds (PAS & BCM).\r\n", age);
	}
	
	return ret;
}


extern int fdb_static_list_add(char *mac, int port, int vlan);
extern int fdb_static_list_del(char *mac, int vlan_id);

#define GW_VLAN_MAX 4094
#define GW_VLAN_LAS 1

#define GW_ONUPORT_MAX 4
#define GW_ONUPORT_LAS 1

#define GW_PORT_PRI_MAX 7
#define GW_PORT_PRI_LAS 0

#define gw_uint8	cs_uint8
#define gw_uint16	cs_uint16
#define gw_uint32	cs_uint32


int cmd_static_mac_add_fdb(struct cli_def *cli, char *command, char *argv[], int argc)
{
	gw_uint32 gw_port;
	gw_uint16 gw_vlan;
	gw_uint32 gw_pri;
	if (CLI_HELP_REQUESTED) {
		switch (argc) {
		case 1:
			return cli_arg_help(cli, 0, "xxxx.xxxx.xxxx", "Please input the mac address",
					NULL );
		case 2:
		    return cli_arg_help(cli, 0, "<port_list>", "Please input the port_list",
					NULL );
		case 3:
			return cli_arg_help(cli, 0, "<1-4094>", "Please input vlan id",
					NULL );
		case 4:
			return cli_arg_help(cli, 0, "<0-7>", "MAC address's priority",
					NULL );			
		default:
			return cli_arg_help(cli, argc > 3, NULL );

		}
	}

	if(argc == 4)
	{
		gw_port = iros_strtol(argv[1]);
		if(gw_port > GW_ONUPORT_MAX || gw_port < GW_ONUPORT_LAS)
		{
			cli_print(cli,"port error\n");
			return -1;
		}
		gw_vlan = iros_strtol(argv[2]);
		if(gw_vlan >GW_VLAN_MAX ||gw_vlan < GW_VLAN_LAS)
		{
			cli_print(cli,"vlan error\n");
			return -1;
		}	
		gw_pri = iros_strtol(argv[3]);
		if(gw_pri < GW_PORT_PRI_LAS || gw_pri > GW_PORT_PRI_MAX)
		{
			cli_print(cli,"pri error\n");
		}
		char mac[13] = {0};
		strncpy(&mac[0], &argv[0][0], 4);
		strncpy(&mac[4], &argv[0][5], 4);
		strncpy(&mac[8], &argv[0][10], 4);
		mac[12] = '\0';
		if(fdb_static_list_add(mac,gw_port,gw_vlan) != CS_E_OK)
		{
			cli_print(cli,"add static mac fail\n");
		}
		else
		{
			cli_print(cli,"add static mac success\n");
		}
	}
	else
	{
		cli_print(cli,"%%input error\n");
	}
	return CLI_OK;
}



int cmd_static_mac_del_fdb(struct cli_def *cli, char *command, char *argv[], int argc)
{
	gw_uint16 gw_vlan;
	if (CLI_HELP_REQUESTED) 
	{
		switch (argc) 
		{
			case 1:
				return cli_arg_help(cli, 0, "xxxx.xxxx.xxxx", "Please input the mac address",
						NULL );
			case 2:
				return cli_arg_help(cli, 0, "<1-4094>", "Please input vlan id",
						NULL );
			default:
				return cli_arg_help(cli, argc > 1, NULL );
		}

	}
	if(argc == 2)
	{
		gw_vlan = iros_strtol(argv[1]);
		if(gw_vlan >GW_VLAN_MAX ||gw_vlan < GW_VLAN_LAS)
		{
			cli_print(cli,"vlan error\n");
			return -1;
		}
		char mac[13] = {0};
		strncpy(&mac[0], &argv[0][0], 4);
		strncpy(&mac[4], &argv[0][5], 4);
		strncpy(&mac[8], &argv[0][10], 4);
		mac[12] = '\0';
		if(fdb_static_list_del(mac,gw_vlan) != CS_E_OK)
		{
			cli_print(cli,"del static mac fail\n");
		}
		else
		{
			cli_print(cli,"del static mac success\n");
		}
	}
	else
	{
		cli_print(cli,"%%input error\n");
	}		
	return CLI_OK;
}


extern int show_port_statistic(struct cli_def * cli, int portid);


#define NUM_PORTS_PER_SYSTEM 5
#define NUM_PORTS_MINIMUM_SYSYTEM 1
int cmd_stat_port_show(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int portid = 0;
	int i = 0;
	if(CLI_HELP_REQUESTED)
	{
		switch (argc)
		{
			case 1:
				return cli_arg_help(cli, 0,
					"<port_list>", "Input one fe port number", NULL );
				break;

			default:
				return cli_arg_help(cli, argc > 1, NULL  );
				break;
		}
	}

	if(argc == 1)
	{
		portid = atoi(argv[0]);
		
		if(portid < NUM_PORTS_MINIMUM_SYSYTEM || portid >= NUM_PORTS_PER_SYSTEM)
		{
			cli_print(cli,"input port error <1-4>\n");
			return CLI_ERROR;
		}
		
		show_port_statistic(cli, portid);
	}
	else
	{		
		for (i = 1; i <= 2; i++)
			{
				cli_print(cli,"===========================port %d stat===========================",i);
				show_port_statistic(cli, i);
				cs_thread_delay(5);
			}
		for(i = 3; i <= 4; i++)
			{
				cli_print(cli,"===========================port %d stat===========================",i);
				show_port_statistic(cli, i);
				cs_thread_delay(5);
			}
	}

	

	return CLI_OK;
}



extern int port_aal_isolation_set(int enable);
extern int port_aal_isolation_get(int *status);
int cmd_oam_port_isolate(struct cli_def *cli, char *command, char *argv[], int argc)
{

	int en = 0;
	
	if(CLI_HELP_REQUESTED)
	{
		switch (argc)
		{
			case 1:
				return cli_arg_help(cli, 0, 
					"{[0|1]}*1", "isolate 1 enable; 0 disable", NULL);
			default:
				return cli_arg_help(cli, argc > 1, NULL  );
		}
	}


	if(argc >= 1)
	{
		if(argc == 1)
		{
			en = atoi(argv[0]);
		}

		if(port_aal_isolation_set(en) != CS_E_OK)
			cli_print(cli, "set all port isolate %s fail!\r\n", en?"enabled":"disabled");
		else
			{
				if(en)
					cli_print(cli,"set all port isolate enable success\n");
				else
					cli_print(cli,"set all port isolate disable success\n");
			}
	}
	else
	{	
		if(port_aal_isolation_get(&en) != CS_E_OK)
			cli_print(cli, "get port isolate fail!\r\n");
		else
			cli_print(cli, "Port isolate is %s\r\n", en?"enabled":"disabled");

	}
	

	return CLI_OK;
}


#endif


#if 1
#define BC_STORM_THRESHOLD_MAX 2000000
#define BC_STORM_THRESHOLD_LAS 10
#define PORT_DOWN_ENABLE 1
#define PORT_DOWN_DISABLE 0


extern broadcast_storm_s broad_storm;

int cmd_bsctrl_policy(struct cli_def *cli, char *command, char *argv[], int argc)
{

	//int portid = 0;
	int len;
	int storm_stat;
	
	if(CLI_HELP_REQUESTED)
	{
		switch (argc)
		{
			case 1:
				return cli_arg_help(cli, 0,
					"[enable|disable]", "enable: port down; disable: only limit rate", NULL );

			default:
				return cli_arg_help(cli, argc > 1, NULL  );
		}
	}
	len = strlen(argv[0]);
	if(argc == 0)
		{
			storm_stat = broad_storm.gulBcStormStat? 1:0;
			if(storm_stat)
				cli_print(cli,"port will be link down when broadcast storm happened.");
			else
				cli_print(cli,"Port will be rate limit when broadcast storm happened.");
		}
	if(argc == 1)
		{
			if((strncmp("enable",argv[0],len)) && (strncmp("disable",argv[0],len)))
				{
					cli_print(cli,"%% Invalid input.\n");
					return CLI_ERROR;
				}
			else
				{
					if(!strncmp("enable",argv[0],len))
						{
							broad_storm.gulBcStormStat = PORT_DOWN_ENABLE;
						}
					if(!strncmp("disable",argv[0],len))
						{
							broad_storm.gulBcStormStat = PORT_DOWN_DISABLE;
						}
					cli_print(cli,"set portdown stat success\n");
				}
		}
	if(argc > 1)
		{
			cli_print(cli,"%% Invalid input.\n");
			return CLI_ERROR;
		}
	return CLI_OK;
}

extern int broadcast_storm_threshold_set(cs_uint32 gw_threshold)
{
	int ret = 0;
	if(gw_threshold > BC_STORM_THRESHOLD_MAX || gw_threshold < BC_STORM_THRESHOLD_LAS)
	{
		cs_printf("set broadcast storm threshold fail\n");
		return CLI_ERROR;
	}
	else
	{
		broad_storm.gulBcStormThreshold = gw_threshold;
	}
	return ret;
}

extern int broadcast_storm_threshold_get(cs_uint32 *gw_threshold)
{
	int ret = 0;
	if(NULL != gw_threshold)
	{
		*gw_threshold = broad_storm.gulBcStormThreshold;
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	return ret;
}


#define GW_THRESHOLD_DEFAULT	1000
extern int broadcast_storm_threshold_default_get(cs_uint32 *gw_threshold)
{
	int ret = 0;
	*gw_threshold = GW_THRESHOLD_DEFAULT;
	return ret;
}

extern int broadcast_storm_threshold_default_check(cs_uint32 gw_threshold)
{
	int ret = 0;
	cs_uint32 gw_threshold_default = 0;
	broadcast_storm_threshold_default_get(&gw_threshold_default);
	if(gw_threshold_default == gw_threshold)
	{
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	return ret;
}


extern int broadcast_storm_threshold_tlv_infor_get(int *len, char **value, int *free_need)
{
	int ret = 0;
	cs_uint32 *gw_threshold = NULL;
	cs_uint32 value_len = 0;
	int status = 0;
	
	if(NULL != len)
	{
		*len = 0;
	}
	else
	{
		goto error;
	}

	if(NULL != value)
	{
		*value = NULL;
	}
	else
	{
		goto error;
	}
	
	if(NULL != free_need)
	{
		*free_need = 0;
	}
	else
	{
		goto error;
	}
	ret = 0;
	value_len = sizeof(cs_uint32);
	gw_threshold = (cs_uint32 *)iros_malloc(IROS_MID_APP, value_len);
	memset(gw_threshold, 0, value_len);
	*free_need = 1;
	status = broadcast_storm_threshold_get(gw_threshold);
	if(0 == status)
	{
		if(0 != broadcast_storm_threshold_default_check(*gw_threshold))
		{
			*len = value_len;
			*value = (char *)gw_threshold;
		}
		else
		{
			//do nothing
		}
	}
	else
	{
	
	}

	goto end;
	
error:
	ret = -1;
	
end:
	if((0 == *len)&&(NULL != gw_threshold))
	{
		iros_free(gw_threshold);
		gw_threshold = NULL;
	}
	return ret;
}


extern int broadcast_storm_module_default_config_recover()
{
	int ret = 0;
	cs_uint32 gw_threshold = 0;
	broadcast_storm_threshold_default_get(&gw_threshold);
	broadcast_storm_threshold_set(gw_threshold);
	return ret;
}


extern int broadcast_storm_threshold_tlv_infor_handle(int len, char *data, int opcode)
{
	int ret = 0;
	cs_uint32 gw_threshold = 0;
	
	if(0 != len)
	{
		//do nothing	
	}
	else
	{
		goto error;
	}

	if(NULL != data)
	{
		//do nothing	
	}
	else
	{
		goto error;
	}
	
	memcpy(&gw_threshold, data, sizeof(cs_uint32));
	if(DATA_RECOVER == opcode)
	{
		broadcast_storm_threshold_set(gw_threshold);
	}
	else if(DATA_SHOW == opcode)
	{
		cs_printf("\n----------------------------------------------------------\n");
		cs_printf("broadcast storm threshold %d\n", gw_threshold);
		cs_printf("----------------------------------------------------------\n");
	}
	else
	{
		cs_printf("in %s, wrong opcode :0x%x\n", __func__, opcode);
	}
	goto end;
error:
	ret = -1;
	
end:
	return ret;
}

extern int broadcast_storm_threshold_running_config_show(void)
{
	int ret = 0;
	cs_uint32 gw_threshold;
	broadcast_storm_threshold_get(&gw_threshold);
	if(0 != broadcast_storm_threshold_default_check(gw_threshold))
	{
		cs_printf("\n----------------------------------------------------------\n");
		cs_printf("broadcast storm threshold %d\n", gw_threshold);
		cs_printf("----------------------------------------------------------\n");
	}
	return ret;
}




int cmd_bsctrl_threshold(struct cli_def *cli, char *command, char *argv[], int argc)
{

	cs_uint32 gw_threshold = 0;

	gw_threshold = atoi(argv[0]);
	if(CLI_HELP_REQUESTED)
	{
		
		switch (argc)
		{
			case 1:
				return cli_arg_help(cli, 0,
					"<10-2000000>", "unit: packets per second", NULL );
			default:
				return cli_arg_help(cli, argc > 1, NULL  );
		}

	}
	if(argc == 1)
		{
			if(gw_threshold > BC_STORM_THRESHOLD_MAX || gw_threshold < BC_STORM_THRESHOLD_LAS)
				{
					cli_print(cli,"set broadcast storm threshold fail\n");
					return CLI_ERROR;
				}
			broad_storm.gulBcStormThreshold = gw_threshold;
			cli_print(cli,"set broadcast storm threshold success\n");
		}
	else
		{
			cli_print(cli,"%% Invalid input.\n");
		}

	return CLI_OK;
}
int cmd_bsctrl_threshold_get(struct cli_def *cli, char *command, char *argv[], int argc)
{
	cs_uint64 threshold;
	
	threshold = broad_storm.gulBcStormThreshold;

	cli_print(cli,"broadcast storm threshold %lld",threshold);
	return CLI_OK;
}

#endif

#if 0
extern int cmd_onu_uart_ip_show(struct cli_def *cli, char *command, char *argv[], int argc);
int cmd_onu_uart_ip_set(struct cli_def *cli, char *command, char *argv[], int argc);

#endif

#if 1
extern int cmd_laser(struct cli_def *cli, char *command, char *argv[], int argc);
#endif
void cli_reg_gwd_cmd(struct cli_command **cmd_root)
{
	extern void cli_reg_rcp_cmd(struct cli_command **cmd_root);
    struct cli_command *set;
    struct cli_command *show, *sys, *atu;
    struct cli_command *gpio;
	#if 0
	struct cli_command *mode_ch;
	#endif
	
	#if 1
	struct cli_command *cp = NULL;
	#endif

	struct cli_command *stat = NULL;
	struct cli_command *vlan = NULL;

    // set cmds in config mode
    set = cli_register_command(cmd_root, NULL, "set", NULL, PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "Set system information");
    	cli_register_command(cmd_root, set, "date",    cmd_onu_mgt_config_product_date,     PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "Manufacture date");
    	cli_register_command(cmd_root, set, "serial",    cmd_onu_mgt_config_product_sn,     PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "Manufacture serial number(<16)");
    	cli_register_command(cmd_root, set, "devicename",    cmd_onu_mgt_config_device_name,     PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "Device name(<=128)");
    	cli_register_command(cmd_root, set, "hw-version",    cmd_onu_mgt_config_product_hw_version,     PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "Hardware version");
		cli_register_command(cmd_root, set, "mon_id",cmd_set_mod_id,PRIVILEGE_UNPRIVILEGED, MODE_CONFIG, "set mode id");

    // display cmds in config mode
    show  = cli_register_command(cmd_root, NULL, "display", NULL, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show information");
    sys  = cli_register_command(cmd_root, show, "product", cmd_show_system_information, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "System information");
    		cli_register_command(cmd_root, show, "opm", cmd_show_optical_diagnostics, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Optical module diagnostics");

    gpio = cli_register_command(cmd_root, NULL, "gpio", NULL, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "gpio test");
    	cli_register_command(cmd_root, gpio, "direction", cmd_onu_mgt_gpiodirect, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Gpio direction test");
    	cli_register_command(cmd_root, gpio, "status", cmd_onu_mgt_gpiotest, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Gpio status test");
	#if 0
    mode_ch = cli_register_command(cmd_root, NULL, "uart", NULL, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "uart test");
 			cli_register_command(cmd_root, mode_ch, "ip_show",cmd_onu_uart_ip_show, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "show uart server ip");
			cli_register_command(cmd_root, mode_ch, "ip_set",cmd_onu_uart_ip_set, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "show uart server ip");
			cli_register_command(cmd_root, mode_ch, "mode",cmd_onu_uart_mode_chang, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "uart mode set");
	#endif
		#if 0
    	atu = cli_register_command(cmd_root, NULL, "atu", NULL, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Atu information");
    		  cli_register_command(cmd_root, atu, "show", cmd_show_atu, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show information");
		#else
		atu = cli_register_command(cmd_root, NULL, "atu", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "atu command");
		cli_register_command(cmd_root, atu, "learning", cmd_oam_atu_learn, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "learning enable");
		cli_register_command(cmd_root, atu, "aging", cmd_oam_atu_age, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "age set");
		cli_register_command(cmd_root, atu, "show", cmd_show_atu, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "show information");
		cli_register_command(cmd_root, atu, "static_add", cmd_static_mac_add_fdb, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "static fdb mac add");
		cli_register_command(cmd_root, atu, "static_del", cmd_static_mac_del_fdb, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "static fdb mac del");
		
		stat = cli_register_command(cmd_root, NULL, "stat", NULL,  PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "stat command");
		cli_register_command(cmd_root, stat, "port_show", cmd_stat_port_show, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "port statistic show");

		vlan = cli_register_command(cmd_root, NULL, "vlan", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "vlan command");
		cli_register_command(cmd_root, vlan, "port_isolate", cmd_oam_port_isolate, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "isolate command");

		cli_register_command(cmd_root, 0, 		"laser", 		cmd_laser,          PRIVILEGE_PRIVILEGED, MODE_EXEC, "Laser on/off");
		#endif
	
	#if 1
	 // portdown {[enable|disable]}*1
	cp = cli_register_command(cmd_root, NULL, "broadcast", NULL, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Broadcast config");
	cp = cli_register_command(cmd_root, cp, "storm", NULL, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Broadcast storm config");
	cli_register_command(cmd_root, cp, "portdown", cmd_bsctrl_policy, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "port down config");
	cli_register_command(cmd_root, cp, "threshold", cmd_bsctrl_threshold, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "threshold config");
	cli_register_command(cmd_root, cp, "threshold_get", cmd_bsctrl_threshold_get, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "threshold config get");
	#endif
	
    // RCP switch cmds in config mode
	//cli_reg_rcp_cmd(cmd_root);
    return;
}
static oam_vendor_handlers_t gwd_oam_handlers = {
		NULL, NULL, NULL, NULL, Gwd_Oam_Handle
};

extern broadcast_storm_s broad_storm;
#define ENABLE	1
#define DISABLE	0
void gw_broadcast_storm_init()
{
	broad_storm.gulBcStormThreshold = 1000;
	broad_storm.gulBcStormStat = DISABLE;
	return;
}

void gwd_onu_init(void)
{
extern void Rcp_Mgt_init(void);
//extern void cli_switch_gwd_cmd(struct cli_command **cmd_root);
//extern void cli_debeg_gwd_cmd(struct cli_command **cmd_root);
//extern void cli_reg_rcp_cmd(struct cli_command **cmd_root);
	GwOamMessageListInit();
	Onu_Sysinfo_Get();

	pStrGwdSwVer = onu_system_info_total.sw_version;
	pStrGwdHwVer = onu_system_info_total.hw_version;
	oam_vendor_handler_register(GwOUI, gwd_oam_handlers);
	
	Rcp_Mgt_init();
	gw_broadcast_storm_init();
	#if 0
	if(registerUserCmdInitHandler("gwd", cli_reg_gwd_cmd) != CS_OK)
		cs_printf("regist gwd cmds fail!\r\n");

	if(registerUserCmdInitHandler("rcp-switch", cli_switch_gwd_cmd) != CS_OK)
		cs_printf("regist rcp  switch cmds fail!\r\n");

	if(registerUserCmdInitHandler("rcp-switch-debug", cli_debeg_gwd_cmd) != CS_OK)
		cs_printf("regist rcp  switch debug cmds fail!\r\n");

	if(registerUserCmdInitHandler("rcp-switch-show", cli_reg_rcp_cmd) != CS_OK)
		cs_printf("regist rcp  switch show cmds fail!\r\n");
	#endif
}

#if 1
cs_status onu_hardware_version_get(char *hw_version, cs_int16 hw_version_len)
{
	cs_status ret = CS_E_OK;
	memset(hw_version, '\0', hw_version_len);
#if 0
	ret = startup_config_read(CFG_ID_HW_VERSION, 32, hw_version);
	return ret;
#endif
	if(CS_E_OK != Onu_Sysinfo_Get())
	{
		ret = CS_E_ERROR;
		goto end;
	}
	else
	{
		//do nothing
	}
	memcpy(hw_version, onu_system_info_total.hw_version, sizeof(onu_system_info_total.hw_version));

end:
	return ret;

	
}
#endif

#if 1
cs_status onu_software_version_get(char *sw_version, cs_uint16 sw_version_len)
{
	cs_status ret = CS_E_OK;
	memset(sw_version, '\0', sw_version_len);
	sprintf(sw_version,"V%dR%02dB%03d",
			SYS_SOFTWARE_MAJOR_VERSION_NO,
			SYS_SOFTWARE_RELEASE_VERSION_NO,
			SYS_SOFTWARE_BRANCH_VERSION_NO);
	return ret;	
}
#endif



