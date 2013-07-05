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

DEFINITIONS:  "DEVICE" means the Cortina Systems?Daytona SDK product.
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


/****************************************************************

OVERVIEW

This file contains the I2C driver code

****************************************************************/
#include "cs_types.h"
#include "cs_cmn.h"
#include "os_core.h"
#include "osal_common.h"
#include "i2c.h"
#include "sdl_i2c.h"

#define ONU_OPM_DEVICE_ADDR                 0x51
#define ONU_OPM_EEPROM_TEMP_ADDR            0x60
#define ONU_OPM_EEPROM_SVCC_ADDR            0x62   
#define ONU_OPM_EEPROM_TXPBIAS_ADDR         0x64    
#define ONU_OPM_EEPROM_TXPWR_ADDR           0x66
#define ONU_OPM_EEPROM_RXPWR_ADDR           0x68


cs_status cs_plat_i2c_speed_get (
    	CS_IN   cs_callback_context_t    	context,
    	CS_IN   cs_dev_id_t              	device,
    	CS_IN   cs_llid_t                	llidport,
    	CS_IN   cs_uint8                 	slave_addr,
    	CS_OUT  cs_uint32                	*freq_khz)
{
	return cs_i2c_speed_get(slave_addr,freq_khz);
}

cs_status cs_plat_i2c_speed_set (
    	CS_IN	cs_callback_context_t    	context,
    	CS_IN   cs_dev_id_t              	device,
    	CS_IN   cs_llid_t                	llidport,
    	CS_IN   cs_uint8                 	slave_addr,
    	CS_IN   cs_uint32                	freq_khz)
{
	return cs_i2c_speed_set(slave_addr,freq_khz);
}

cs_status cs_plat_i2c_read (
    	CS_IN   cs_callback_context_t    	context,
    	CS_IN   cs_dev_id_t              	device,
    	CS_IN   cs_llid_t                	llidport,
    	CS_IN   cs_uint8                 	slave_addr,
    	CS_IN   cs_uint8                 	slave_offset,
    	CS_IN   cs_uint32                	len,
    	CS_OUT  cs_uint8                 	*data)
{
	return cs_i2c_read(slave_addr, slave_offset, len, data);
}

cs_status cs_plat_i2c_write (
    	CS_IN   cs_callback_context_t    	context,
    	CS_IN   cs_dev_id_t              	device,
    	CS_IN   cs_llid_t                	llidport,
    	CS_IN   cs_uint8                 	slave_addr,
    	CS_IN   cs_uint8                 	slave_offset,
    	CS_IN   cs_uint32                	len,
    	CS_IN   cs_uint8                 	*data)
{
	return cs_i2c_write(slave_addr, slave_offset, len, data);
}

cs_status cs_plat_i2c_opm_status_read (
    	CS_IN   cs_callback_context_t    	context,
    	CS_IN   cs_dev_id_t              	device,
    	CS_IN   cs_llid_t                	llidport,
    	CS_OUT  cs_int16                 	*temperature,
    	CS_OUT  cs_uint16                	*voltage,
    	CS_OUT  cs_uint16                	*tx_bias,
    	CS_OUT  cs_uint16                	*tx_power,
    	CS_OUT  cs_uint16                	*rx_power)
{
	cs_uint8 out_data[2] = {0};

	if(NULL==temperature || NULL==tx_power || NULL==rx_power || NULL==voltage || NULL==tx_bias)
	{
		return CS_E_ERROR;
	}

	cs_i2c_read(ONU_OPM_DEVICE_ADDR,ONU_OPM_EEPROM_TEMP_ADDR,sizeof(out_data),out_data);
	*temperature = (out_data[0] << 8 | out_data[1]);

	cs_i2c_read(ONU_OPM_DEVICE_ADDR,ONU_OPM_EEPROM_SVCC_ADDR,sizeof(out_data),out_data);
	*voltage  = (out_data[0] << 8 | out_data[1]);			  

	cs_i2c_read(ONU_OPM_DEVICE_ADDR,ONU_OPM_EEPROM_TXPBIAS_ADDR,sizeof(out_data),out_data);
	*tx_bias = (out_data[0] << 8 | out_data[1]);	  

	cs_i2c_read(ONU_OPM_DEVICE_ADDR,ONU_OPM_EEPROM_TXPWR_ADDR,sizeof(out_data),out_data);
	*tx_power = (out_data[0] << 8 | out_data[1]);	  

	cs_i2c_read(ONU_OPM_DEVICE_ADDR,ONU_OPM_EEPROM_RXPWR_ADDR,sizeof(out_data),out_data);
	*rx_power = (out_data[0] << 8 | out_data[1]);	 

	return CS_E_OK;

}



