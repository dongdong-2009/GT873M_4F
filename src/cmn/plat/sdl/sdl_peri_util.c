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

#include "cs_types.h"
#include "os_core.h"
#include "cs_cmn.h"
#include "osal_common.h"
#include "ssp.h"
#include "sdl_ssp.h"
#include "sdl_peri_util.h"


/* Synchronous Serial Port APIs */

#define EEPROM_SIZE             64
#define SSP_EEPROM_OP_WRITE     0x5 /*A5-A0 D7-D0 D15-D0 Write Address AN¨C A0*/
#define SSP_EEPROM_OP_READ      0x6 /*A5-A0 Read Address AN¨C A0*/
#define SSP_EEPROM_OP_EWEN      0x98  /*11XXXX Write Enable*/

#define SSP_RTL_OP_READ         3
#define SSP_RTL_OP_WRITE        2

/* EEPROM */
cs_status cs_plat_ssp_eeprom_read(
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint8                 offset,
    CS_IN       cs_uint8                 length,
    CS_IN_OUT   cs_uint8                 *data
    )
{
    cs_uint32 i;
    cs_uint32 rdata;
    cs_uint16 eeprom[EEPROM_SIZE];
    cs_status ret = CS_E_OK;
    
    if(NULL == data){
        return CS_E_PARAM;
    }

    if((offset+length) > EEPROM_SIZE*2){
        return CS_E_PARAM;
    }
    
    cs_plat_ssp_transaction_begin(context,device,llidport,SSP_SLAVE_ID_1);
    /* set clock at 1.85MHz */
    ret = cs_plat_ssp_init_cfg_set(context,device,llidport,
        SSP_SLAVE_ID_1,1850,SSP_STANDARD_MICRO_WIRE,SSP_TDAT_CPHA_CENTER,
        SSP_IDAT_MODE_CENTER,SSP_DATIN_DATA_PHASE,SSP_DATA_CENTER_ALIGN);

    if(CS_E_OK == ret){
        ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                                         SSP_SLAVE_ID_1,0xF,0x8,SSP_NORMAL);
    }
    else{
        cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_1);
        return ret;
    }

    if(CS_E_OK == ret){
        for(i = 0; i < EEPROM_SIZE; i++)
        {    
            ret = cs_plat_ssp_read(context,device,llidport,SSP_SLAVE_ID_1,
                 ((SSP_EEPROM_OP_READ<<29) | (i<<23)),0,0,&rdata);
            if(CS_E_OK == ret){
                eeprom[i]= 0xffff & rdata;
            }
            else{
                break;
            }
        }
    }
    cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_1);
    
    if(CS_E_OK == ret){
        memcpy((void *)data, (void *)((char *)eeprom + offset), length);
    }
        
    return ret;
}

cs_status cs_plat_ssp_eeprom_write(
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint8                 offset,
    CS_IN       cs_uint8                 length,
    CS_IN       cs_uint8                 *data
    )
{
    cs_uint32 i;
    cs_uint16 eeprom[EEPROM_SIZE];
    cs_status ret = CS_E_OK;
    
    if((offset+length) > EEPROM_SIZE*2){
        return CS_E_PARAM;
    }

    cs_plat_ssp_transaction_begin(context,device,llidport,SSP_SLAVE_ID_1);
    ret = cs_plat_ssp_eeprom_read(context,device,llidport,
                             0, EEPROM_SIZE*2,(char *)eeprom);
    if(CS_E_OK == ret){
        memcpy((void *)((char *)eeprom + offset), (const void *)data, length);
    }
    else{
        cs_printf("eeprom read back failed\n");
        cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_1);
        return ret;
    }

    /* set clock at 1.85MHz */
    ret = cs_plat_ssp_init_cfg_set(context,device,llidport,
        SSP_SLAVE_ID_1,1850,SSP_STANDARD_MICRO_WIRE,SSP_TDAT_CPHA_CENTER,
        SSP_IDAT_MODE_CENTER,SSP_DATIN_DATA_PHASE,SSP_DATA_CENTER_ALIGN);
    for(i=0; i<EEPROM_SIZE; i++)
    {
        if(CS_E_OK == ret){
        ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                            SSP_SLAVE_ID_1,0x0,0x1F,SSP_CMD_ONLY);
        }
        else{
            cs_printf("cs_plat_ssp_frame_cfg_set cmd only failed\n");
            break;
        }

        if(CS_E_OK == ret){
        ret = cs_plat_ssp_write(context,device,llidport,SSP_SLAVE_ID_1,
                            (SSP_EEPROM_OP_EWEN<<24),0,0,0);
        }
        else{
            cs_printf("cs_plat_ssp_write write enable failed\n");
            break;
        }

        if(CS_E_OK == ret){
        ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                            SSP_SLAVE_ID_1,0xF,0x8,SSP_NORMAL);
        }
        else{
            cs_printf("cs_plat_ssp_frame_cfg_set normal failed\n");
            break;
        }

        ret = cs_plat_ssp_write(context,device,llidport,SSP_SLAVE_ID_1,
                            ((SSP_EEPROM_OP_WRITE<<29) | (i<<23)),0,0,eeprom[i]);
        
    }
    cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_1);
    
    return ret;
}


/* SLIC */
cs_status cs_plat_ssp_slic_read(
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_OUT      cs_uint8                 *data
    )
{
    cs_uint32 rdata;
    cs_status ret = CS_E_OK;
    
    if(NULL == data){
        return CS_E_PARAM;
    }

    cs_plat_ssp_transaction_begin(context,device,llidport,SSP_SLAVE_ID_2);
        /* set clock at 8.9MHz */
    ret = cs_plat_ssp_init_cfg_set(context,device,llidport,
        SSP_SLAVE_ID_2,8900,SSP_STANDARD_SPI,SSP_TDAT_CPHA_END,
        SSP_IDAT_MODE_CENTER,SSP_DATIN_CMD_PHASE,SSP_EDGE_ALIGN);

    if(CS_E_OK == ret){
        ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                        SSP_SLAVE_ID_2,0x6,0,SSP_NORMAL);
    }
    else{
        cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_2);
        return ret;
        
    }
    
    ret = cs_plat_ssp_read(context,device,llidport,SSP_SLAVE_ID_2,0,0,0,&rdata);

    *data = rdata & 0xFF;
    cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_2);
    
    return ret;
}

cs_status cs_plat_ssp_slic_write(
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint8                 data
    )
{
    cs_status ret = CS_E_OK;

    cs_plat_ssp_transaction_begin(context,device,llidport,SSP_SLAVE_ID_2);
    /* set clock at 8.9MHz */
    ret = cs_plat_ssp_init_cfg_set(context,device,llidport,
        SSP_SLAVE_ID_2,8900,SSP_STANDARD_SPI,SSP_TDAT_CPHA_END,
        SSP_IDAT_MODE_CENTER,SSP_DATIN_DATA_PHASE,SSP_EDGE_ALIGN);

    if(CS_E_OK == ret){
        ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                        SSP_SLAVE_ID_2,0,0x7,SSP_CMD_ONLY);
    }
    else{
        cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_2);
        return ret;
        
    }
    
    ret = cs_plat_ssp_write(context,device,llidport,SSP_SLAVE_ID_2,(data<<24),0,0,0);
    cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_2);
    
    return ret;
}

/* Switch */
cs_status cs_plat_ssp_switch_read(
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint32                offset,
    CS_OUT      cs_uint32                *data
    )
{
    cs_status ret = CS_E_OK;
    
    if(NULL == data){
        return CS_E_PARAM;
    }

    cs_plat_ssp_transaction_begin(context,device,llidport,SSP_SLAVE_ID_1);
    /* set clock at 2.5MHz */
    ret = cs_plat_ssp_init_cfg_set(context,device,llidport,
        SSP_SLAVE_ID_1,2500,SSP_STANDARD_SPI,SSP_TDAT_CPHA_CENTER,
        SSP_IDAT_MODE_CENTER,SSP_DATIN_CMD_PHASE,SSP_DATA_CENTER_ALIGN);

    if(CS_E_OK == ret){
        ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                        SSP_SLAVE_ID_1,0xF,0x17,SSP_NORMAL);
    }
    else{
        cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_1);
        return ret;
        
    }
    
    ret = cs_plat_ssp_read(context,device,llidport,SSP_SLAVE_ID_1,
        (SSP_RTL_OP_READ<<24 | offset<<8),0,0,data);
    cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_1);
    
    return ret;
}

cs_status cs_plat_ssp_switch_write(
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint32                offset,
    CS_IN       cs_uint32                data
    )
{    
    cs_status ret = CS_E_OK;

    cs_plat_ssp_transaction_begin(context,device,llidport,SSP_SLAVE_ID_1);
    /* set clock at 2.5MHz */
    ret = cs_plat_ssp_init_cfg_set(context,device,llidport,
        SSP_SLAVE_ID_1,2500,SSP_STANDARD_SPI,SSP_TDAT_CPHA_CENTER,
        SSP_IDAT_MODE_CENTER,SSP_DATIN_DATA_PHASE,SSP_DATA_CENTER_ALIGN);

    if(CS_E_OK == ret){
    ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                        SSP_SLAVE_ID_1,0xF,0x17,SSP_NORMAL);
    }
    else{
        cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_1);
        return ret;
        
    }
    
    ret = cs_plat_ssp_write(context,device,llidport,SSP_SLAVE_ID_1,
        (SSP_RTL_OP_WRITE<<24 | offset<<8),0,0,data);
    cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_1);
    
    return ret;
}


