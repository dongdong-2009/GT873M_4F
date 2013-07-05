#ifndef _PTP_PKT_H
#define _PTP_PKT_H

#include "ptp_type.h"

#define PTP_HDR_SIZE    34
#define PTP_TIMESTAMP_SIZE  10
#define PTP_PORT_ID_SIZE    10

#define PTP_MSG_TYPE_OFFSET 0
#define PTP_CF_OFFSET   8
#define PTP_CF_SIZE        8
#define PTP_FLAG_OFFSET     6
#define PTP_SRC_PORT_ID_OFFSET  20
#define PTP_SRC_PORT_ID_SIZE    10
#define PTP_SEQ_ID_OFFSET   30
#define PTP_SEQ_ID_SIZE         2

typedef struct {
    cs_uint8 transportSpecific : 4,
        messageType : 4;
    cs_uint8 rsvd : 4,
        versionPTP : 4;
    cs_uint16 messageLength;
    cs_uint8 domainNumber;
    cs_uint8 rsvd1;
    cs_uint16 flagField;
    cs_uint64 correctionField;
    cs_uint32 rsvd2;
    cs_uint8 sourcePortIdentity[10];
    cs_uint16 sequenceId;
    cs_uint8 controlField;
    cs_uint8 logMessageInterval;
} __attribute__((packed)) ptp_cmn_msg_hdr_t;

typedef struct {
    ptp_cmn_msg_hdr_t hdr;
    cs_uint8 originTimestamp[10];
} __attribute__((packed)) ptp_sync_pdu_t;

typedef struct {
    ptp_cmn_msg_hdr_t hdr;
    cs_uint8 originTimestamp[10];
} __attribute__((packed)) ptp_delay_req_pdu_t;

typedef struct {
    ptp_cmn_msg_hdr_t hdr;
    cs_uint8 preciseOriginTimestamp[10];
} __attribute__((packed)) ptp_follow_up_pdu_t;

typedef struct {
    ptp_cmn_msg_hdr_t hdr;
    cs_uint8 receiveTimestamp[10];
    cs_uint8 requestingPortIdentity[10];
} __attribute__((packed)) ptp_delay_resp_pdu_t;

typedef struct {
    ptp_cmn_msg_hdr_t hdr;
    cs_uint8 originTimestamp[10];
    cs_uint8 rsvd[10];
} __attribute__((packed)) ptp_pdelay_req_pdu_t;

typedef struct {
    ptp_cmn_msg_hdr_t hdr;
    cs_uint8 requestReceiptTimestamp[10];
    cs_uint8 requestingPortIdentity[10];
} __attribute__((packed)) ptp_pdelay_resp_pdu_t;

typedef struct {
    ptp_cmn_msg_hdr_t hdr;
    cs_uint8 responseOriginTimestamp[10];
    cs_uint8 requestingPortIdentity[10];
} __attribute__((packed)) ptp_pdelay_resp_follow_up_pdu_t;

#endif /* #ifndef _PTP_PKT_H */

