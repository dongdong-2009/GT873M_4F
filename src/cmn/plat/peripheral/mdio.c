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

DEFINITIONS:  "DEVICE" means the Cortina Systems  SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems SDK software.

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


#include "iros_config.h"
#include "plat_common.h"
#include "registers.h"
#include "mdio.h"

static mdio_mode_t g_mdio_mode = 0x0;
static mdio_handler_t g_mdio_handler = NULL;
static cs_uint32 g_mdio_mutex;
static mdio_speed_t g_mdio_speed[MDIO_MAX_DEVICE_NUM];

#if 0 
static cs_uint32 mdio_dbg = 0;

/*module_param(mdio_dbg, uint, 0644);*/

#define MDIO_DBG(format, arg...)	\
do {								\
	if (mdio_dbg)					\
		cs_printf("%s: " format,	\
				"mdio" , ## arg);	\
} while (0)
#endif

#define MDIO_ERR(format, arg...)	\
	do {							\
		cs_printf("error: %s,line %d: " format,	\
			__FILE__,__LINE__, ## arg); \
	} while (0)


static cs_int32 cs_mdio_isr(int irq, void *dev_id)
{
	if(irq != MDIO_INT_NUM)
	{
		MDIO_ERR("lynxd mdio: wrong irq number: %d\n", irq);
		return CS_ERROR;
	}

	/* clear the wdt interrupt status */
	CS_REG_WRITE(CS_MDIO_INT, 0x3);
	if(g_mdio_handler != NULL)
		g_mdio_handler(dev_id);

	return CS_OK;
}

cs_status cs_mdio_mode_get(mdio_mode_t* mode)
{
	*mode = g_mdio_mode;
	return CS_OK;
}

cs_status cs_mdio_mode_set(mdio_mode_t mode)
{
	PER_MDIO_CFG_t mdio_cfg;

	g_mdio_mode = mode;
	switch(mode)
	{
		case MDIO_POLLING:
		    CS_REG_READ(CS_MDIO_CFG, mdio_cfg.wrd);
		    mdio_cfg.bf.mdio_manual = 0x1;
		    CS_REG_WRITE(CS_MDIO_CFG,mdio_cfg.wrd);
			break;
		case MDIO_INTERRUPT:
		    CS_REG_READ(CS_MDIO_CFG, mdio_cfg.wrd);
		    mdio_cfg.bf.mdio_manual = 0x0;
		    mdio_cfg.bf.mdio_manual = 0x0;
		    CS_REG_WRITE(CS_MDIO_CFG,mdio_cfg.wrd);

			break;
		default:
			break;
	}

	return CS_OK;
}

cs_status cs_mdio_speed_get(cs_uint8 	intf_addr, cs_uint32* freq_khz)
{
	cs_uint8 i = 0;

	for(i=0; i<MDIO_MAX_DEVICE_NUM; i++)
	{
		if(g_mdio_speed[i].intf_addr == intf_addr)
		{
			*freq_khz = g_mdio_speed[i].freq_khz;
			return CS_OK;
		}
	}

	MDIO_ERR("get mdio speed error");
	*freq_khz = 0x0;
	return CS_ERROR;
}

cs_status cs_mdio_speed_set(cs_uint8 	intf_addr, cs_uint32 freq_khz)
{
	cs_uint8 i = 0;
	cs_uint8 index = MDIO_MAX_DEVICE_NUM;

	for(i=0; i<MDIO_MAX_DEVICE_NUM; i++)
	{
		if(g_mdio_speed[i].intf_addr == intf_addr )
		{
			g_mdio_speed[i].freq_khz = freq_khz;
			return CS_OK;
		}

		if(g_mdio_speed[i].intf_addr==0x0 && index==MDIO_MAX_DEVICE_NUM)
			index = i;
	}

	if(index == MDIO_MAX_DEVICE_NUM)
	{
		MDIO_ERR("set mdio speed error");
		return CS_ERROR;
	}

	g_mdio_speed[index].intf_addr = intf_addr;
	g_mdio_speed[index].freq_khz = freq_khz;

	return CS_OK;

}

cs_status cs_mdio_intr_handler_get(mdio_handler_t* handler)
{
	*handler = g_mdio_handler;
	return CS_OK;
}

cs_status cs_mdio_intr_handler_set(mdio_handler_t	handler, void* data)
{
	g_mdio_handler = handler;
	cs_intr_reg(MDIO_INT_NUM, "mdio_irq",cs_mdio_isr, NULL, INTR_DSR_CB);
	/* request_irq(LYNXD_MDIO_INT_NUM, cs_mdio_isr, IRQF_DISABLED, "lynxd_mdio", data); */
	return CS_OK;
}

cs_status cs_mdio_intr_enable(void)
{
	return CS_OK;
}
cs_status cs_mdio_intr_disable(void)
{
	return CS_OK;
}

cs_status cs_mdio_read(cs_uint8 device, cs_uint8 reg, cs_uint32 *data)
{
	PER_MDIO_ADDR_t mdio_add;
	PER_MDIO_CTRL_t mdio_ctrl;
	PER_MDIO_RDDATA_t mdio_rddata;
	PER_MDIO_CFG_t mdio_cfg;
    cs_uint32 mdio_cfg_val=0;
	cs_uint32 i=0;

	cs_mutex_lock(g_mdio_mutex);

	for(i=0; i<MDIO_MAX_DEVICE_NUM; i++)
	{
		if(g_mdio_speed[i].intf_addr == device)
			break;
	}
	if(i==MDIO_MAX_DEVICE_NUM || g_mdio_speed[i].freq_khz==0x0){
		MDIO_ERR("not set mdio speed\n");
        cs_mutex_unlock(g_mdio_mutex);
		return CS_ERROR;
	}
    mdio_cfg_val=(32*1000/g_mdio_speed[i].freq_khz)-1;

    CS_REG_READ(CS_MDIO_CFG, mdio_cfg.wrd);
    mdio_cfg.bf.mdio_pre_scale = mdio_cfg_val;
    CS_REG_WRITE(CS_MDIO_CFG,mdio_cfg.wrd);

	CS_REG_READ(CS_MDIO_ADDR, mdio_add.wrd);
	mdio_add.bf.mdio_rd_wr = 0x1;
	mdio_add.bf.mdio_addr = device;
	mdio_add.bf.mdio_offset = reg;
    CS_REG_WRITE(CS_MDIO_ADDR, mdio_add.wrd);

	CS_REG_READ(CS_MDIO_CTRL, mdio_ctrl.wrd);
	mdio_ctrl.bf.mdiostart = 0x1;
    CS_REG_WRITE(CS_MDIO_CTRL, mdio_ctrl.wrd);

    for(i=0; i<MDIO_TIMEOUT; i++) {
	    CS_REG_READ(CS_MDIO_CTRL, mdio_ctrl.wrd);
	    if (mdio_ctrl.bf.mdiodone)
	    {
	        CS_REG_READ(CS_MDIO_RDDATA, mdio_rddata.wrd);
	        CS_REG_WRITE(CS_MDIO_CTRL, mdio_ctrl.wrd);
	        break;
	    }
    }

	cs_mutex_unlock(g_mdio_mutex);

    if(i==MDIO_TIMEOUT)
        return CS_E_TIMEOUT;

    *data = mdio_rddata.bf.mdio_rddata;
    return CS_OK;

}

cs_status cs_mdio_write(cs_uint8 device, cs_uint8 reg, cs_uint16 data)
{
	PER_MDIO_ADDR_t mdio_add;
	PER_MDIO_CTRL_t mdio_ctrl;
	PER_MDIO_WRDATA_t mdio_wrdata;
	PER_MDIO_CFG_t mdio_cfg;
    cs_uint32 mdio_cfg_val=0;
    cs_uint32 i = 0;

    cs_mutex_lock(g_mdio_mutex);

	for(i=0; i<MDIO_MAX_DEVICE_NUM; i++)
	{
		if(g_mdio_speed[i].intf_addr == device)
			break;
	}
	if(i==MDIO_MAX_DEVICE_NUM || g_mdio_speed[i].freq_khz == 0x0){
		MDIO_ERR("not set mdio speed\n");
		return CS_ERROR;
	}

    mdio_cfg_val=(32*1000/g_mdio_speed[i].freq_khz)-1;

    CS_REG_READ(CS_MDIO_CFG, mdio_cfg.wrd);
    mdio_cfg.bf.mdio_pre_scale = mdio_cfg_val;
    CS_REG_WRITE(CS_MDIO_CFG,mdio_cfg.wrd);

	CS_REG_READ(CS_MDIO_ADDR, mdio_add.wrd);
	mdio_add.bf.mdio_rd_wr = 0x0;
	mdio_add.bf.mdio_addr = device;
	mdio_add.bf.mdio_offset = reg;
    CS_REG_WRITE(CS_MDIO_ADDR, mdio_add.wrd);

	CS_REG_READ(CS_MDIO_WRDATA, mdio_wrdata.wrd);
	mdio_wrdata.bf.mdio_wrdata = data;
    CS_REG_WRITE(CS_MDIO_WRDATA, mdio_wrdata.wrd);

	CS_REG_READ(CS_MDIO_CTRL, mdio_ctrl.wrd);
	mdio_ctrl.bf.mdiostart = 0x1;
    CS_REG_WRITE(CS_MDIO_CTRL, mdio_ctrl.wrd);

    for(i=0; i<MDIO_TIMEOUT; i++) {
        CS_REG_READ(CS_MDIO_CTRL, mdio_ctrl.wrd);
        if (mdio_ctrl.bf.mdiodone){
            CS_REG_WRITE(CS_MDIO_CTRL, mdio_ctrl.wrd);
            break;
        }
    }
	cs_mutex_unlock(g_mdio_mutex);

    if(i==MDIO_TIMEOUT)
        return CS_E_TIMEOUT;

    return CS_OK;

}

cs_status cs_mdio_init(void)
{
	cs_status rt;
  cs_int32 i;

  for(i=0; i<MDIO_MAX_DEVICE_NUM; i++)
  {
  	g_mdio_speed[i].intf_addr = i;
   	g_mdio_speed[i].freq_khz = 4000;
	}

	rt = cs_mutex_init(&g_mdio_mutex,  "mdio mutex",0);
    if(rt!=CS_OK)
    {
        MDIO_ERR("mdio creat mutex fail");
    }

	cs_mdio_mode_set(MDIO_POLLING);

	return CS_OK;
}


