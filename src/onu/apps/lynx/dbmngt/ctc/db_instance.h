/****************************************************************************
            Software License for Customer Use of Cortina Software
                          Grant Terms and Conditions

IMPORTANT NOTICE - READ CAREFULLY: This Software License for Customer Use
of Cortina Software ("LICENSE") is the agreement which governs use of
software of Cortina Systems, Inc. and its subsidiaries ("CORTINA"),
including computer software (source code and object code) and associated
printed materials ("SOFTWARE").  The SOFTWARE is protected by copyright laws
and international copyright treaties, as well as other intellectual property
laws and treaties.  The SOFTWARE is not sold, and instead is only licensed
for use, strictly in accordance with this document.  Any hardware sold by
CORTINA is protected by various patents, and is sold but this LICENSE does
not cover that sale, since it may not necessarily be sold as a package with
the SOFTWARE.  This LICENSE sets forth the terms and conditions of the
SOFTWARE LICENSE only.  By downloading, installing, copying, or otherwise
using the SOFTWARE, you agree to be bound by the terms of this LICENSE.
If you do not agree to the terms of this LICENSE, then do not download the
SOFTWARE.

DEFINITIONS:  "DEVICE" means the Cortina Systems? Daytona SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems? SDK software.

GRANT OF LICENSE:  Subject to the restrictions below, CORTINA hereby grants
CUSTOMER a non-exclusive, non-assignable, non-transferable, royalty-free,
perpetual copyright license to (1) install and use the SOFTWARE for
reference only with the DEVICE; and (2) copy the SOFTWARE for your internal
use only for use with the DEVICE.

RESTRICTIONS:  The SOFTWARE must be used solely in conjunction with the
DEVICE and solely with Your own products that incorporate the DEVICE.  You
may not distribute the SOFTWARE to any third party.  You may not modify
the SOFTWARE or make derivatives of the SOFTWARE without assigning any and
all rights in such modifications and derivatives to CORTINA.  You shall not
through incorporation, modification or distribution of the SOFTWARE cause
it to become subject to any open source licenses.  You may not
reverse-assemble, reverse-compile, or otherwise reverse-engineer any
SOFTWARE provided in binary or machine readable form.  You may not
distribute the SOFTWARE to your customers without written permission
from CORTINA.

OWNERSHIP OF SOFTWARE AND COPYRIGHTS. All title and copyrights in and the
SOFTWARE and any accompanying printed materials, and copies of the SOFTWARE,
are owned by CORTINA. The SOFTWARE protected by the copyright laws of the
United States and other countries, and international treaty provisions.
You may not remove any copyright notices from the SOFTWARE.  Except as
otherwise expressly provided, CORTINA grants no express or implied right
under CORTINA patents, copyrights, trademarks, or other intellectual
property rights.

DISCLAIMER OF WARRANTIES. THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY
EXPRESS OR IMPLIED WARRANTY OF ANY KIND, INCLUDING ANY IMPLIED WARRANTIES
OF MERCHANTABILITY, NONINFRINGEMENT, OR FITNESS FOR A PARTICULAR PURPOSE,
TITLE, AND NON-INFRINGEMENT.  CORTINA does not warrant or assume
responsibility for the accuracy or completeness of any information, text,
graphics, links or other items contained within the SOFTWARE.  Without
limiting the foregoing, you are solely responsible for determining and
verifying that the SOFTWARE that you obtain and install is the appropriate
version for your purpose.

LIMITATION OF LIABILITY. IN NO EVENT SHALL CORTINA OR ITS SUPPLIERS BE
LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, LOST
PROFITS, BUSINESS INTERRUPTION, OR LOST INFORMATION) OR ANY LOSS ARISING OUT
OF THE USE OF OR INABILITY TO USE OF OR INABILITY TO USE THE SOFTWARE, EVEN
IF CORTINA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
TERMINATION OF THIS LICENSE. This LICENSE will automatically terminate if
You fail to comply with any of the terms and conditions hereof. Upon
termination, You will immediately cease use of the SOFTWARE and destroy all
copies of the SOFTWARE or return all copies of the SOFTWARE in your control
to CORTINA.  IF you commence or participate in any legal proceeding against
CORTINA, then CORTINA may, in its sole discretion, suspend or terminate all
license grants and any other rights provided under this LICENSE during the
pendency of such legal proceedings.
APPLICABLE LAWS. Claims arising under this LICENSE shall be governed by the
laws of the State of California, excluding its principles of conflict of
laws.  The United Nations Convention on Contracts for the International Sale
of Goods is specifically disclaimed.  You shall not export the SOFTWARE
without first obtaining any required export license or other approval from
the applicable governmental entity, if required.  This is the entire
agreement and understanding between You and CORTINA relating to this subject
matter.
GOVERNMENT RESTRICTED RIGHTS. The SOFTWARE is provided with "RESTRICTED
RIGHTS." Use, duplication, or disclosure by the Government is subject to
restrictions as set forth in FAR52.227-14 and DFAR252.227-7013 et seq. or
its successor. Use of the SOFTWARE by the Government constitutes
acknowledgment of CORTINA's proprietary rights therein. Contractor or
Manufacturer is CORTINA.

Copyright (c) 2009 by Cortina Systems Incorporated
****************************************************************************/

#ifndef __DB_INSTANCE_H__
#define __DB_INSTANCE_H__
#ifdef HAVE_DB_MANAGMENT
#include "db_api.h"

#define DB_CUR_VERSION   0x0

enum{
    DB_CTC_LOID_PASSWD_ID = 1,
    DB_CTC_TRM_SVR0_CFG_ID,
    DB_CTC_TRM_SVR1_CFG_ID,
    DB_CTC_TRM_SVR2_CFG_ID,
    DB_CTC_TRM_SVR3_CFG_ID,
    /* admin user information */
    DB_CTC_ADMIN_INFO_ID ,
    /* power user infomation */
    DB_CTC_USER_INFO_ID,
    /* port admin, auto, bc ctrl */
    DB_CTC_PORT_INFO_ID,   
};

#define DB_CTC_LOID_MAX_LEN  24
#define DB_CTC_PWD_MAX_LEN   12

typedef struct{
    cs_uint8 loid[DB_CTC_LOID_MAX_LEN + 1];
    cs_uint8 passwd[DB_CTC_PWD_MAX_LEN + 1];
    cs_uint16 rsvd;
}db_ctc_loid_pwd_t;

/*************** User infomation define *******************/
#define DB_CTC_MAX_USR_LEN  64
#define DB_CTC_MAX_PWD_LEN  64

#define DB_CTC_DEFAULT_ADMIN_NAME  "admin" 
#define DB_CTC_DEFAULT_ADMIN_PWD  "admin" 
#define DB_CTC_DEFAULT_USR_NAME  "user" 
#define DB_CTC_DEFAULT_USR_PWD  "user" 
typedef struct{
    cs_uint8    usr[DB_CTC_MAX_USR_LEN + 1];
    cs_uint8    pwd[DB_CTC_MAX_PWD_LEN + 1];
    cs_uint16    rsvd;
}db_ctc_user_t;

/******************** Port base infomation ********************/
typedef struct{
    cs_uint8             admin;
    cs_uint8             auto_neg;
    cs_uint8             work_mode;
    cs_uint8             pause;
} db_ctc_port_entry_t;

#define DB_CTC_MAX_PORT   4
typedef struct{
    db_ctc_port_entry_t port[DB_CTC_MAX_PORT];
} db_ctc_port_t;

typedef struct{
    cs_uint8    init_flag;
    cs_uint8    uart_id;        /* UART number 0,1,2,3,4*/
    cs_uint8    data_bits;      /* 7/8  */
    cs_uint8    stop_bits;      /* 1/2 */
    cs_uint8    flow_control;   /* None/XONXOFF/HardWare */
    cs_uint8    parity;         /* None/Odd/Even */
    cs_uint8    duplex;         /* full-duplex or half-duplex*/
    cs_uint8    mode;           /* Polling or Interrupt */    
    cs_uint32   baud_rate;          
    cs_uint32   proto_type;     /* protocol type TCP 6/UDP 1*/
    cs_uint32   proto_port;     /* L4 protocol port 1~65535 */
    cs_uint32   client_timeout; /* remote client timeout 0:never thoer:1~65535s*/
    cs_uint32   max_payload;    /* 512 ~1400 bytes */
    cs_uint32   min_payload;    /* 40 ~ max_payload */
    cs_uint32   max_res_time;   /* max response time 25~1000ms */
	#if 1
	cs_uint8	model_flag;		/*0:server model; 1:client model*/
	cs_uint32 remote_server_ip;
	cs_uint16 remote_server_port;
	#endif
}db_ts_cfg_t;

void db_init();
void db_restore();

cs_status  db_ctc_set_default();


#endif
#endif
