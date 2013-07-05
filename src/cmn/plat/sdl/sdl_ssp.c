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
#include "cs_cmn.h"
#include "os_core.h"
#include "ssp.h"
#include "sdl_ssp.h"

static ssp_intf_cfg_t ssp_config[SSP_SLAVE_ID_MAX];

/*
* This function is used to claim SSP bus for specified slave device. 
* The invoking thread will get sole access to the bus, if this API 
* return success. If the bus is claimed already by other slave device,
* the invoking thread will be blocked until it get the access.
*/
cs_status cs_plat_ssp_transaction_begin (
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint8                 slave_id
    )
{
    ssp_bus_lock();

    return CS_E_OK;
}

cs_status cs_plat_ssp_transaction_end (
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint8                 slave_id
    )
{
    ssp_bus_unlock();

    return CS_E_OK;
}


/* Synchronous Serial Port APIs */
cs_status cs_plat_ssp_init_cfg_get (
        CS_IN   cs_callback_context_t   context,
        CS_IN   cs_dev_id_t             device,
        CS_IN   cs_llid_t               llidport,
        CS_IN   cs_uint8                slave_id,
        CS_OUT  cs_uint32               *freq_khz,
        CS_OUT  cs_uint8                *standard,
        CS_OUT  cs_uint8                *tdat_cpha,
        CS_OUT  cs_uint8                *idat_mode,
        CS_OUT  cs_uint8                *datin_cmds,
        CS_OUT  cs_uint8                *edge_align
        )
{
    if(slave_id >= SSP_SLAVE_ID_MAX){
        return CS_E_PARAM;
    }

    if((NULL == freq_khz) || (NULL == standard) || (NULL == tdat_cpha) ||
        (NULL == idat_mode) || (NULL == datin_cmds) || (NULL == edge_align)){
        return CS_E_PARAM;
    }
    
    *freq_khz   = ssp_config[slave_id].speed_kHz;
    *standard   = ssp_config[slave_id].mwr_ssp_select;
    *tdat_cpha  = ssp_config[slave_id].tdat_cpha;
    *idat_mode  = ssp_config[slave_id].idat_mode;
    *datin_cmds = ssp_config[slave_id].datin_cmds;
    *edge_align = ssp_config[slave_id].edge_align;
    
    return CS_OK;
}

cs_status cs_plat_ssp_init_cfg_set (
        CS_IN   cs_callback_context_t   context,
        CS_IN   cs_dev_id_t             device,
        CS_IN   cs_llid_t               llidport,
        CS_IN   cs_uint8                slave_id,
        CS_IN   cs_uint32               freq_khz,
        CS_IN   cs_uint8                standard,
        CS_IN   cs_uint8                tdat_cpha,
        CS_IN   cs_uint8                idat_mode,
        CS_IN   cs_uint8                datin_cmds,
        CS_IN   cs_uint8                edge_align
        )
{
    cs_status ret = CS_E_OK;
    
    if(slave_id >= SSP_SLAVE_ID_MAX){
        return CS_E_PARAM;
    }

    ssp_config[slave_id].ssp_select        = slave_id;
    ssp_config[slave_id].speed_kHz         = freq_khz;
    ssp_config[slave_id].mwr_ssp_select    = standard;
    ssp_config[slave_id].tdat_cpha         = tdat_cpha;
    ssp_config[slave_id].idat_mode         = idat_mode;
    ssp_config[slave_id].datin_cmds        = datin_cmds;
    ssp_config[slave_id].edge_align        = edge_align;
   
    return ret;
}

cs_status cs_plat_ssp_frame_cfg_get (
        CS_IN   cs_callback_context_t   context,
        CS_IN   cs_dev_id_t             device,
        CS_IN   cs_llid_t               llidport,
        CS_IN   cs_uint8                slave_id,
        CS_OUT  cs_uint8                *data_len,
        CS_OUT  cs_uint8                *cmd_len,
        CS_OUT  cs_uint8                *cmd_cycle
        )
{
    if(slave_id >= SSP_SLAVE_ID_MAX){
        return CS_E_PARAM;
    }

    *data_len   = ssp_config[slave_id].data_len;
    *cmd_len    = ssp_config[slave_id].command_len;
    *cmd_cycle  = ssp_config[slave_id].command_only;
    
    return CS_OK;
}

cs_status cs_plat_ssp_frame_cfg_set (
        CS_IN   cs_callback_context_t   context,
        CS_IN   cs_dev_id_t             device,
        CS_IN   cs_llid_t               llidport,
        CS_IN   cs_uint8                slave_id,
        CS_IN   cs_uint8                data_len,
        CS_IN   cs_uint8                cmd_len,
        CS_IN   cs_uint8                cmd_cycle
        )
{
    cs_status ret = CS_E_OK;
    
    if(slave_id >= SSP_SLAVE_ID_MAX){
        return CS_E_PARAM;
    }

    ssp_config[slave_id].data_len      = data_len;
    ssp_config[slave_id].command_len   = cmd_len;
    ssp_config[slave_id].command_only  = cmd_cycle;
       
    return ret;
}

cs_status cs_plat_ssp_read (
        CS_IN   cs_callback_context_t   context,
        CS_IN   cs_dev_id_t             device,
        CS_IN   cs_llid_t               llidport,
        CS_IN   cs_uint8                slave_id,
        CS_IN   cs_uint32               cmd_addr_0,
        CS_IN   cs_uint32               cmd_addr_1,
        CS_IN   cs_uint32               cmd_addr_2,
        CS_OUT  cs_uint32               *data
        )
{
    cs_status ret = CS_E_OK;

    if(NULL == data){
        return CS_E_PARAM;
    }

    ret = ssp_config_init(&ssp_config[slave_id]);
    if(CS_E_OK == ret){
        ret = ssp_cmd_set(cmd_addr_0,cmd_addr_1,cmd_addr_2);
        if(CS_E_OK == ret){
            ret = ssp_data_read(data);
        }
    }
    return ret;
}

cs_status cs_plat_ssp_write (
        CS_IN   cs_callback_context_t   context,
        CS_IN   cs_dev_id_t             device,
        CS_IN   cs_llid_t               llidport,
        CS_IN   cs_uint8                slave_id,
        CS_IN   cs_uint32               cmd_addr_0,
        CS_IN   cs_uint32               cmd_addr_1,
        CS_IN   cs_uint32               cmd_addr_2,
        CS_IN   cs_uint32               data
        )
{
    cs_status ret = CS_E_OK;

    ret = ssp_config_init(&ssp_config[slave_id]);
    if(CS_E_OK == ret){
        ret = ssp_cmd_set(cmd_addr_0,cmd_addr_1,cmd_addr_2);
        if(CS_E_OK == ret){
            ret = ssp_data_write(data);
        }
    }
    
    return ret;
}

cs_status cs_plat_ssp_serdes_read(
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint8                 offset,
    CS_OUT      cs_uint8                 *data
    )
{
    cs_status   ret = CS_E_OK;
    cs_uint8    raddr_rev;
    cs_uint32   read_data;
    
    if(NULL == data){
        return CS_E_PARAM;
    }

    raddr_rev = reverse8(offset);
    cs_plat_ssp_transaction_begin(context,device,llidport,SSP_SLAVE_ID_3);
    /* set clock at 8.9MHz */
    ret = cs_plat_ssp_init_cfg_set(context,device,llidport,
        SSP_SLAVE_ID_3,8900,SSP_STANDARD_SPI,SSP_TDAT_CPHA_CENTER,
        SSP_IDAT_MODE_CENTER,SSP_DATIN_CMD_PHASE,SSP_DATA_CENTER_ALIGN);

    if(CS_E_OK == ret){
        ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                        SSP_SLAVE_ID_3,0x7,0x9,SSP_NORMAL);
    }
    else{
        cs3_deselect();
        cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_3);
        return ret;
        
    }
    
    ret = cs_plat_ssp_read(context,device,llidport,SSP_SLAVE_ID_3,
                            (0x01<<30 | raddr_rev<<22),0,0,&read_data);
    *data = reverse8(read_data & 0x000000ff);
    
    cs3_deselect();
    cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_3);
    
    return ret;
    
}

cs_status cs_plat_ssp_serdes_write(
    CS_IN       cs_callback_context_t    context,
    CS_IN       cs_dev_id_t              device,
    CS_IN       cs_llid_t                llidport,
    CS_IN       cs_uint8                 offset,
    CS_IN       cs_uint8                 data
    )
{   
    cs_status ret = CS_E_OK;
    cs_uint8 waddr_rev = reverse8(offset);
    cs_uint8 wdata_rev = reverse8(data);

    cs_plat_ssp_transaction_begin(context,device,llidport,SSP_SLAVE_ID_3);
    /* set clock at 8.9MHz */
    ret = cs_plat_ssp_init_cfg_set(context,device,llidport,
        SSP_SLAVE_ID_3,8900,SSP_STANDARD_SPI,SSP_TDAT_CPHA_CENTER,
        SSP_IDAT_MODE_CENTER,SSP_DATIN_DATA_PHASE,SSP_DATA_CENTER_ALIGN);

    if(CS_E_OK == ret){
        ret = cs_plat_ssp_frame_cfg_set(context,device,llidport,
                        SSP_SLAVE_ID_3,0x7,0x9,SSP_NORMAL);
    }
    else{
        cs3_deselect();
        cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_3);
        return ret;
        
    }
    
    ret = cs_plat_ssp_write(context,device,llidport,SSP_SLAVE_ID_3,
                             (waddr_rev<<22),0,0,wdata_rev);
    cs3_deselect();
    cs_plat_ssp_transaction_end(context,device,llidport,SSP_SLAVE_ID_3);
    
    return ret;
}


