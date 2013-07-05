
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

Copyright (c) 2010 by Cortina Systems Incorporated
****************************************************************************/
#ifndef __SDL_PTP_H__
#define __SDL_PTP_H__

#include "cs_types.h"
#include "plat_common.h"


typedef enum {
    SDL_PTP_CORRECT_DIR_SUB    = 0,
    SDL_PTP_CORRECT_DIR_ADD    = 1
} cs_sdl_ptp_correct_dir_t;

typedef enum {
    SDL_PTP_CLK_SRC_PTP        = 0,
    SDL_PTP_CLK_SRC_MPCP       = 1
} cs_sdl_ptp_clk_src_t;



/*Parameter description:
port_id - PON port or GE port
msg_type- PTP message type mask
          bit 0: 1:PTP message type 0 need go through PTP process;
                 0:PTP message type 0 need not go through PTP process;

          .......

          bit 15:1:PTP message type 15 need go through PTP process;
                 0:PTP message type 15 need not go through PTP process;
enable  - enable PTP process or not
*/

cs_status epon_request_onu_ptp_cfg_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_IN  cs_uint16                 msg_type,
    CS_IN  cs_boolean                enable
);

cs_status epon_request_onu_ptp_cfg_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT  cs_uint16                 *msg_type,
    CS_OUT  cs_boolean                *enable
);


/*Description,
port_id    - PON port or GE port
direction  - asymmetry correction value
value      - the sign of asymmetry correction
*/
cs_status epon_request_onu_ptp_asym_correction_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_IN  cs_sdl_ptp_correct_dir_t  direction,
    CS_IN  cs_uint16                 value
);

cs_status epon_request_onu_ptp_asym_correction_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_sdl_ptp_correct_dir_t  *direction,
    CS_OUT cs_uint16                 *value
);

cs_status epon_request_onu_ptp_capture_eg_timestamp(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_uint32                 *timestamp
);

cs_status epon_request_onu_ptp_latch_ing_timestamp(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_uint32                 *timestamp
);

/*Parameter description:
enable - 1pps+ToD signal TX enable or not
len    - ToD info length
*/
cs_status epon_request_onu_tod_cfg_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_boolean                enable
);

cs_status epon_request_onu_tod_cfg_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_OUT cs_boolean                *enable
);



/*Parameter description:
s_pps     - the value of local timer to trigger 1pps signal
pps_width - 1pps signal width

*/
cs_status epon_request_onu_tod_trigger_pps_signal(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_uint32                 s_pps,
    CS_IN  cs_uint16                 pps_width
);


/*
s_tod - the value of local timer to trigger ToD signal
*/
cs_status epon_request_onu_tod_trigger_tod_signal(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_uint32                 s_tod
);

/*
tod - ToD info, which format is conformance to NMEA 0183
len - ToD info length
*/
cs_status epon_request_onu_tod_info_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_uint8                  len,
    CS_IN  cs_uint8                  *tod
);

cs_status epon_request_onu_tod_info_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_OUT cs_uint8                  *len,
    CS_OUT cs_uint8                  *tod
);

/*
baud_rate - Baud rate refer to ctc 2.1 standard, the baud rate should be 4800,9600,19200,38400, defaut as 4800 
*/
cs_status epon_request_onu_tod_uart_cfg_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_uint32                 baud_rate
);

cs_status epon_request_onu_tod_uart_cfg_get(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_OUT cs_uint32                 *baud_rate
);

/* Src : 0 - ptp counter
         1 - mpcp counter, i.e. ptp counter need to sync to mpcp counter.
*/
cs_status epon_request_onu_ptp_clk_src_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_sdl_ptp_clk_src_t      src
);

#ifdef HAVE_MPORTS

typedef struct{
    cs_uint32            second;
    cs_uint32            nano_second;
}cs_sdl_ptp_timestamp_t;

typedef enum {
    SDL_PTP_TIME_ADJUST_DIR_SUB    = 0, /*timer is adjust by adding the delta */
    SDL_PTP_TIME_ADJUST_DIR_ADD    = 1  /*timer is adjust by subtracting the delta */
} cs_sdl_ptp_time_dir_t;

cs_status epon_request_onu_ptp_capture_sw_eg_timestamp(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_OUT cs_sdl_ptp_timestamp_t    *timestamp);


cs_status epon_request_onu_ptp_sw_port_set(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_port_id_t              port_id,
    CS_IN  cs_boolean                enable);


cs_status epon_request_onu_ptp_update_sw_time(
    CS_IN  cs_callback_context_t     context,
    CS_IN  cs_int32                  device_id,
    CS_IN  cs_int32                  llidport,
    CS_IN  cs_sdl_ptp_time_dir_t     direction,
    CS_IN  cs_sdl_ptp_timestamp_t    timestamp);

cs_status epon_request_onu_ptp_jitter_test (
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_uint8                  enable
);

#endif

cs_status epon_request_onu_mgmt_port_ptp_state_set(
    CS_IN cs_callback_context_t     context,
    CS_IN cs_int32                  device_id,
    CS_IN cs_int32                  llidport,
    CS_IN cs_uint8                            enable
);
#endif

