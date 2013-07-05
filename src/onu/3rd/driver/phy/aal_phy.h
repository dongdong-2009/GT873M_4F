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

#ifndef __AAL_PHY_H__
#define __AAL_PHY_H__

#include "cs_types.h"

#define AAL_PHY_ID_GE    0
#define AAL_PHY_ID_MII0  1

typedef volatile union 
{
    struct
    {
#ifdef CS_BIG_ENDIAN 
    cs_uint16 auto_neg_fail                     :  1 ;
    cs_uint16 speed_change                      :  1 ;
    cs_uint16 duplex_change                     :  1 ;
    cs_uint16 page_rx                           :  1 ;
    cs_uint16 auto_neg_success                  :  1 ;
    cs_uint16 link_change                       :  1 ;
    cs_uint16 symbol_err                        :  1 ;
    cs_uint16 false_carrier                     :  1 ;
    cs_uint16 rsvd0                             :  1 ;
    cs_uint16 MDI_crossover_change              :  1 ;
    cs_uint16 rsvd1                             :  4 ;
    cs_uint16 polarity_change                   :  1 ;
    cs_uint16 jabber                            :  1 ;
#else
    cs_uint16 jabber                            :  1 ;
    cs_uint16 polarity_change                   :  1 ;
    cs_uint16 rsvd1                             :  4 ;
    cs_uint16 MDI_crossover_change              :  1 ;
    cs_uint16 rsvd0                             :  1 ;
    cs_uint16 false_carrier                     :  1 ;
    cs_uint16 symbol_err                        :  1 ;
    cs_uint16 link_change                       :  1 ;
    cs_uint16 auto_neg_success                  :  1 ;
    cs_uint16 page_rx                           :  1 ;
    cs_uint16 duplex_change                     :  1 ;
    cs_uint16 speed_change                      :  1 ;
    cs_uint16 auto_neg_fail                     :  1 ;
#endif
    }s;
    cs_uint16 u16;
} cs_aal_phy_int_msk_t;


typedef struct {
    cs_uint32 adv_10base_t_half     :  1 ;                                                                                                                   
    cs_uint32 adv_10base_t_full     :  1 ;                                                                                                                
    cs_uint32 adv_100base_tx_half   :  1 ;                                                                                   
    cs_uint32 adv_100base_tx_full   :  1 ;                                                                         
    cs_uint32 adv_100base_t4        :  1 ;                                                                                                       
    cs_uint32 adv_pause             :  1 ;                                                                                                     
    cs_uint32 adv_1000base_t_half   :  1 ;                                                                                                
    cs_uint32 adv_1000base_t_full   :  1 ;                                                                                               
    cs_uint32 rsrvd                 : 21 ;             
} cs_aal_phy_autoneg_adv_t;

typedef enum {
    AAL_PHY_MDI_AUTO        = 0, /* Auto MDI/MDIX   */
    AAL_PHY_MDI_MDI         = 1, /* Fixed MDI       */
    AAL_PHY_MDI_MDIX        = 2  /* Fixed MDIX      */
} cs_aal_phy_mdi_mode_t;

typedef enum {
    AAL_PHY_SPEED_10        = 0, 
    AAL_PHY_SPEED_100       = 1,
    AAL_PHY_SPEED_1000      = 2 
} cs_aal_phy_speed_mode_t;

typedef struct {
    cs_boolean tx2rx_enable;
    cs_boolean rx2tx_enable;
} cs_aal_phy_lpbk_mode_t;

cs_status aal_phy_reset(cs_uint8 phy_id);
cs_status aal_phy_auto_neg_restart(cs_uint8 phy_id);
cs_status aal_phy_mode_set(cs_uint8 phy_id, cs_aal_phy_speed_mode_t speed, cs_boolean is_full, cs_boolean enable, cs_aal_phy_autoneg_adv_t adv);
cs_status aal_phy_power_set(cs_uint8 phy_id, cs_boolean power_up);
cs_status aal_phy_power_get(cs_uint8 phy_id, cs_boolean *power_up);
cs_status aal_phy_auto_neg_set(cs_uint8 phy_id, cs_boolean enable);
cs_status aal_phy_auto_neg_get(cs_uint8 phy_id, cs_boolean *enable);
cs_status aal_phy_ability_adv_set(cs_uint8 phy_id, cs_aal_phy_autoneg_adv_t adv);
cs_status aal_phy_ability_adv_get(cs_uint8 phy_id, cs_aal_phy_autoneg_adv_t *adv);
cs_status aal_phy_partner_ability_adv_get(cs_uint8 phy_id, cs_aal_phy_autoneg_adv_t *adv);
cs_status aal_phy_link_status_get(cs_uint8 phy_id, cs_boolean *link_up);
cs_status aal_phy_speed_set(cs_uint8 phy_id, cs_aal_phy_speed_mode_t speed);
cs_status aal_phy_speed_get(cs_uint8 phy_id, cs_aal_phy_speed_mode_t *speed);
cs_status aal_phy_duplex_set(cs_uint8 phy_id, cs_boolean duplex);
cs_status aal_phy_duplex_get(cs_uint8 phy_id, cs_boolean *duplex);
cs_status aal_phy_flow_ctrl_set(cs_uint8 phy_id, cs_boolean enable);
cs_status aal_phy_flow_ctrl_get(cs_uint8 phy_id, cs_boolean *enable);
cs_status aal_phy_mdix_set(cs_uint8 phy_id, cs_aal_phy_mdi_mode_t mdix_mode);
cs_status aal_phy_mdix_get(cs_uint8 phy_id, cs_aal_phy_mdi_mode_t *mdix_mode);
cs_status aal_phy_loopback_set(cs_uint8 phy_id, cs_aal_phy_lpbk_mode_t mode);
cs_status aal_phy_loopback_get(cs_uint8 phy_id, cs_aal_phy_lpbk_mode_t *mode);
cs_status aal_phy_int_mask_all(cs_uint8 phy_id);
cs_status aal_phy_int_en_set(cs_uint8 phy_id, cs_aal_phy_int_msk_t data);
cs_status aal_phy_int_en_get(cs_uint8 phy_id, cs_aal_phy_int_msk_t *data);
cs_status aal_phy_int_status_get(cs_uint8 phy_id, cs_aal_phy_int_msk_t *status);
cs_status aal_phy_int_status_clr(cs_uint8 phy_id);
cs_status aal_phy_force_change(cs_uint8 phy_id);
cs_status aal_phy_extend_init(cs_uint8 phy_id);
cs_status aal_phy_eee_set(cs_uint8 phy_id, cs_boolean enable);
cs_status aal_phy_eee_get(cs_uint8 phy_id, cs_boolean *enable);
cs_status  phy_init(void);


#endif /* __AAL_PHY_H__ */

