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

DEFINITIONS:  "DEVICE" means the Cortina Systems?LynxD SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems?SDK software.

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
#ifndef _MDIO_H_
#define _MDIO_H_

#define lynxd_per_base		((PER_t*)LYNXD_REG_BASE)

#define CS_MDIO_CFG         (&(lynxd_per_base->MDIO_CFG))
#define CS_MDIO_ADDR        (&(lynxd_per_base->MDIO_ADDR))
#define CS_MDIO_WRDATA      (&(lynxd_per_base->MDIO_WRDATA))
#define CS_MDIO_RDDATA      (&(lynxd_per_base->MDIO_RDDATA))
#define CS_MDIO_CTRL        (&(lynxd_per_base->MDIO_CTRL))

#define CS_MDIO_AUTO_CFG    (&(lynxd_per_base->MDIO_AUTO_CFG))
#define CS_MDIO_AUTO_INTV   (&(lynxd_per_base->MDIO_AUTO_INTV))
#define CS_MDIO_AUTO_RM     (&(lynxd_per_base->MDIO_AUTO_RM))
#define CS_MDIO_AUTO_AADDR  (&(lynxd_per_base->MDIO_AUTO_AADDR))
#define CS_MDIO_IE          (&(lynxd_per_base->MDIO_IE))
#define CS_MDIO_INT         (&(lynxd_per_base->MDIO_INT))
#define CS_MDIO_STAT        (&(lynxd_per_base->MDIO_STAT))

/*
*   MDIO clock frequency = (1/2)*PER_CLK_t*(mdio_pre_scale+1).
*/
#define MDIO_MODE_MANUAL           0x1
#define MDIO_ADDR_RD               (1<<15)
#define MDIO_ADDR_WR               (0<<15)

#define MDIO_CTRL_START            (1<<7)
#define MDIO_CTRL_DONE              0x1

#define MDIO_TIMEOUT               2500

#define MDIO_INT_NUM				12

#define MDIO_MAX_DEVICE_NUM			32

typedef cs_int32 (*mdio_handler_t)(void*);

typedef enum
{
	MDIO_POLLING,
	MDIO_INTERRUPT,
}mdio_mode_t;

typedef struct {
	cs_uint8 	intf_addr;
	cs_uint32   freq_khz;
}mdio_speed_t;


cs_status cs_mdio_init(void);
cs_status cs_mdio_mode_get(mdio_mode_t* mode);
cs_status cs_mdio_mode_set(mdio_mode_t mode);
cs_status cs_mdio_speed_get(cs_uint8 	intf_addr, cs_uint32* freq_khz);
cs_status cs_mdio_speed_set(cs_uint8 	intf_addr, cs_uint32 freq_khz);
cs_status cs_mdio_intr_handler_get(mdio_handler_t	*handler);
cs_status cs_mdio_intr_handler_set(mdio_handler_t	handler, void* data);
cs_status cs_mdio_intr_enable(void);
cs_status cs_mdio_intr_disable(void);
cs_status cs_mdio_read(cs_uint8 device, cs_uint8 reg, cs_uint32 *data);
cs_status cs_mdio_write(cs_uint8 device, cs_uint8 reg, cs_uint16 data);

#endif
