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
#include "plat_common.h"
#include "sys_cfg.h"

cs_status cs_plat_chipset_info_get (
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t                  device,
    CS_IN       cs_llid_t                       llidport,
    CS_OUT    sys_chip_info_t             *pChipset_info)
{
    cs_uint16  chip_version = 0;
    cs_uint32 a, b, c;
    
    if(NULL == pChipset_info) {
        return CS_E_PARAM;
    }
    
    sscanf(ONU_CHIPSET_VENDOR_ID,"%2x%2x",&a,&b);
    pChipset_info->chip_vendor_id[0] = a;
    pChipset_info->chip_vendor_id[1] = b;
    
    onu_get_chip_version(&chip_version);
    pChipset_info->chip_model[0] = (chip_version >> 8) && 0xFF;
    pChipset_info->chip_model[1] = chip_version && 0xFF;

    sscanf(ONU_CHIPSET_VER,"%2x",&a);
    pChipset_info->rev = a;
    
    sscanf(ONU_CHIPSET_MADE_DATE,"%2d",&a);
    sscanf(ONU_CHIPSET_MADE_DATE,"%*[^/]/%2d",&b);
    sscanf(ONU_CHIPSET_MADE_DATE,"%*[^/]/%*[^/]/%2d",&c);
    pChipset_info->date[0] = a;
    pChipset_info->date[1] = b;
    pChipset_info->date[2] = c;

    pChipset_info->fw_ver.major = cs_atoi(IROS_ONU_APP_VER_MAJOR);
    pChipset_info->fw_ver.minor = cs_atoi(IROS_ONU_APP_VER_MINOR);
    pChipset_info->fw_ver.revision = cs_atoi(IROS_ONU_APP_VER_REVISION);
    pChipset_info->fw_ver.build_id = cs_atoi(IROS_ONU_APP_VER_BUILD);

    return CS_E_OK;
}

cs_status cs_plat_onu_info_get (
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t                  device,
    CS_IN       cs_llid_t                       llidport,
    CS_OUT    sys_onu_info_t             *pOnu_info)
{
 
    if(NULL == pOnu_info) {
        return CS_E_PARAM;
    }

    startup_config_read(CFG_ID_VENDOR_ID, CTC_VENDER_ID_LEN, pOnu_info->onu_vendor_id);
    startup_config_read(CFG_ID_ONU_MOD_ID, CTC_MODEL_ID_LEN, pOnu_info->onu_model);
    startup_config_read(CFG_ID_HW_VERSION,CTC_ONU_HW_LEN , pOnu_info->hardware_ver);
    startup_config_read(CFG_ID_MAC_ADDRESS, CS_MACADDR_LEN, pOnu_info->onu_id);
	startup_config_read(CFG_ID_MDIO_DEVID,CTC_MODEL_ID_LEN,pOnu_info->onu_model);
    strcpy( pOnu_info->software_ver,ONU_SW_VERSION);

    return CS_E_OK;
}


