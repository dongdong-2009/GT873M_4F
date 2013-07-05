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
 
DEFINITIONS:  "DEVICE" means the Cortina Systems LynxD product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems software.
 
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
PROFITS, BUSINESS INTERRUPTION, OR LOST INFORMATION) OR ANY LOSS ARISING CS_OUT
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


#ifndef __AAL_PHY_UTIL_H__
#define __AAL_PHY_UTIL_H__
#include "aal_phy.h"

#include "cs_types.h"

#define PHY_REG_MAX_LEN       (16)
#define PHY_DEV_NAME_MAX_LEN    (32)

/*Organizationally Unique Identifier (OUI)     */
#define PHY_OUI_BROADCOM                  0x000818  /* Broadcom Corporation          */
#define PHY_OUI_BROADCOM_54612            0x00d897  /* Broadcom Corporation for 54612e */
#define PHY_OUI_REALTEK                   0x000732  /* Realtek Corporation          */
#define PHY_OUI_VITESSE                   0x0001c1

/*Manufacturer¡¯s Model Number                 */
#define PHY_MODEL_BROADCOM_BCM5461        0x000c    /* BCM5461 10/100/1000baseTX PHY */
#define PHY_MODEL_BROADCOM_BCM54612       0x0026    /* BCM54612e PHY                 */
#define PHY_MODEL_REALTEK_RTL8211         0x0011    /* RTL8211 10/100/1000baseTX PHY */
#define PHY_MODEL_REALTEK_RTL8201         0x0001    /* RTL8201 10/100/1000baseTX PHY */
#define PHY_MODEL_VITESSE_8641            0x0003    /* VITESSE 10/100/1000baseTX PHY */


/*Ethernet PHY IEEE Standard register address  */
#define PHY_CTRL_REG                  0
#define PHY_STATUS_REG                1
#define PHY_ID_1_REG                  2
#define PHY_ID_2_REG                  3
#define PHY_AUTONEG_ADV_REG           4
#define PHY_LP_ABILITY_BASE_REG       5
#define PHY_LP_AUTONEG_EXT_REG        6
#define PHY_AUTONEG_NEXT_PAGE_REG     7
#define PHY_LP_NEXT_PAGE_REG          8
#define PHY_1000BASET_CTRL_REG        9
#define PHY_1000BASET_STATUS_REG      10
#define PHY_RSVD1_REG                 11
#define PHY_RSVD2_REG                 12
#define PHY_RSVD3_REG                 13
#define PHY_RSVD4_REG                 14
#define PHY_1000BSET_STATUS_EXT_REG   15
#define PHY_SPECIFIC_STATUS_REG       17

typedef struct {
    cs_status (*init)(cs_uint8);
    cs_status (*reset)(cs_uint8 );
    cs_status (*auto_neg_restart)(cs_uint8 );
    cs_status (*power_set)(cs_uint8, cs_boolean );
    cs_status (*power_get)(cs_uint8, cs_boolean*);
    cs_status (*auto_neg_set)(cs_uint8, cs_boolean );
    cs_status (*auto_neg_get)(cs_uint8, cs_boolean*);
    cs_status (*ability_adv_set)(cs_uint8, cs_aal_phy_autoneg_adv_t );
    cs_status (*ability_adv_get)(cs_uint8, cs_aal_phy_autoneg_adv_t*);
    cs_status (*partner_ability_adv_get)(cs_uint8, cs_aal_phy_autoneg_adv_t*);
    cs_status (*link_status_get)(cs_uint8, cs_boolean* );
    cs_status (*speed_set)(cs_uint8, cs_aal_phy_speed_mode_t );
    cs_status (*speed_get)(cs_uint8, cs_aal_phy_speed_mode_t*);
    cs_status (*duplex_set)(cs_uint8, cs_boolean );
    cs_status (*duplex_get)(cs_uint8, cs_boolean*);
    cs_status (*flow_ctrl_set)(cs_uint8, cs_boolean );
    cs_status (*flow_ctrl_get)(cs_uint8, cs_boolean*);
    cs_status (*mdix_set)(cs_uint8, cs_aal_phy_mdi_mode_t );
    cs_status (*mdix_get)(cs_uint8, cs_aal_phy_mdi_mode_t*);
    cs_status (*loopback_set)(cs_uint8, cs_aal_phy_lpbk_mode_t);
    cs_status (*loopback_get)(cs_uint8, cs_aal_phy_lpbk_mode_t*);
    cs_status (*int_mask_all)(cs_uint8);
    cs_status (*int_en_set)(cs_uint8, cs_aal_phy_int_msk_t);
    cs_status (*int_en_get)(cs_uint8, cs_aal_phy_int_msk_t*);
    cs_status (*int_status_get)(cs_uint8, cs_aal_phy_int_msk_t*);
    cs_status (*int_status_clr)(cs_uint8);
    cs_status (*force_change)(cs_uint8);
    cs_status (*mode_set)(cs_uint8, cs_aal_phy_speed_mode_t,  cs_boolean, cs_boolean,cs_aal_phy_autoneg_adv_t);
    cs_status (*eee_set)(cs_uint8, cs_boolean );
    cs_status (*eee_get)(cs_uint8, cs_boolean*);
    cs_status (*eee_lnkprt_get)(cs_uint8, cs_boolean*);
    
}phy_drv_t;

typedef struct {
    const char  name[PHY_DEV_NAME_MAX_LEN];
    cs_uint32   oui;
    cs_uint32   model;
    phy_drv_t   drv;
}phy_dev_t;

cs_status phy_regfield_read
(
    cs_uint8   phy_dev, 
    cs_uint8   reg, 
    cs_uint8   offset, 
    cs_uint8   length, 
    cs_uint16 *p_val16
);

cs_status phy_regfield_write
(
    cs_uint8   phy_dev, 
    cs_uint8   reg, 
    cs_uint8   offset, 
    cs_uint8   length, 
    cs_uint16  val16
);

cs_status phy_regfield_mask_write
(
    cs_uint8   phy_dev, 
    cs_uint8   reg, 
    cs_uint16  mask,
    cs_uint16  val16
);

cs_status  phy_util_init(void);


#endif //__AAL_PHY_UTIL_H__

