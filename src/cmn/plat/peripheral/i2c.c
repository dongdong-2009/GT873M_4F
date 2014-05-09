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

#include "iros_config.h"
#include "plat_common.h"
#include "registers.h"
#include "i2c.h"

static cs_uint32 i2c_dbg_log = 0;

/*module_param(i2c_dbg_log, uint, 0644);*/

#define I2C_DBG(format, arg...)	\
do {								\
	if (i2c_dbg_log)					\
		cs_printf("%s: " format,	\
				"i2c" , ## arg);	\
} while (0)

#define I2C_ERR(format, arg...)	\
	do {							\
		cs_printf("error: %s,line %d: " format,	\
			__FILE__,__LINE__, ## arg); \
	} while (0)


static i2c_mode_t g_i2c_mode = 0x0;
static i2c_handler_t g_i2c_handler = NULL;
static i2c_speed_t g_i2c_speed[I2C_MAX_DEVICE_NUM];
static cs_uint32 g_i2c_mutex;

static cs_int32 cs_i2c_isr(int irq, void *dev_id)
{
	if(irq != I2C_INT_NUM)
	{
		I2C_ERR("lynxd i2c: wrong irq number: %d\n", irq);
		return CS_ERROR;
	}

	/* clear the wdt interrupt status */
	CS_REG_WRITE(CS_BIW_INT, 0x1);
	if(g_i2c_handler != NULL)
		g_i2c_handler(dev_id);

	return CS_OK;
}

cs_status cs_i2c_mode_get(i2c_mode_t* mode)
{
	*mode = g_i2c_mode;
	return CS_OK;
}

cs_status cs_i2c_mode_set(i2c_mode_t mode)
{
	cs_uint32 val32;

	g_i2c_mode = mode;
	switch(mode)
	{
		case I2C_POLLING:
		break;
		case I2C_INTERRUPT:
			CS_REG_READ(CS_BIW_IE, val32);
		    val32 |= 0x1;
		    CS_REG_WRITE(CS_BIW_IE,val32);
		break;
	}

	return CS_OK;

}

cs_status cs_i2c_speed_get(cs_uint8 slave_addr, cs_uint32* freq_khz)
{
	cs_uint8 i=0;

	for(i=0; i<I2C_MAX_DEVICE_NUM; i++)
	{
		if(g_i2c_speed[i].slave_addr == slave_addr)
		{
			*freq_khz = g_i2c_speed[i].freq_khz;
			return CS_OK;
		}
	}

	I2C_ERR("get i2c speed error");
	*freq_khz = 0x0;

	return CS_ERROR;
}

cs_status cs_i2c_speed_set(cs_uint8 slave_addr, cs_uint32 freq_khz)
{
	cs_uint8 i=0;
	cs_uint8 index=I2C_MAX_DEVICE_NUM;

	I2C_DBG("i2c speed set slave addr: %d, freq_khz: %d\n", slave_addr, freq_khz);

	for(i=0; i<I2C_MAX_DEVICE_NUM; i++)
	{
		if(g_i2c_speed[i].slave_addr == slave_addr )
		{
			I2C_DBG("i2c speed set succeed ");
			g_i2c_speed[i].freq_khz = freq_khz;
			return CS_OK;
		}

		I2C_DBG("i2c speed set %d\n", i);

		if(g_i2c_speed[i].slave_addr==0x0 && index==I2C_MAX_DEVICE_NUM)
			index = i;
	}

	if(index == I2C_MAX_DEVICE_NUM)
	{
		I2C_ERR("set i2c speed error");
		return CS_ERROR;
	}

	I2C_DBG("i2c speed set index %d\n", index);
	g_i2c_speed[index].slave_addr = slave_addr;
	g_i2c_speed[index].freq_khz = freq_khz;

	I2C_DBG("i2c speed set succeed 1");
	return CS_OK;

}

cs_status cs_i2c_intr_handler_get(i2c_handler_t* handler)
{
	*handler = g_i2c_handler;
	return CS_OK;
}

cs_status cs_i2c_intr_handler_set(i2c_handler_t handler, void* data)
{
	g_i2c_handler = handler;
	cs_intr_reg(I2C_INT_NUM, "i2c_irq", cs_i2c_isr, NULL, INTR_DSR_CB);
	/* request_irq(LYNXD_I2C_INT_NUM, cs_i2c_isr, IRQF_DISABLED, "lynxd_i2c", data); */
	return CS_OK;
}

cs_status cs_i2c_intr_enable(void)
{
	PER_BIW_IE_t i2c_ie;

	i2c_ie.bf.biw0e = 0x1;
	CS_REG_WRITE(CS_BIW_IE, i2c_ie.wrd);

	return CS_OK;
}

cs_status cs_i2c_intr_disable(void)
{
	PER_BIW_IE_t i2c_ie;

	i2c_ie.bf.biw0e = 0x0;
	CS_REG_WRITE(CS_BIW_IE, i2c_ie.wrd);

	return CS_OK;
}

static void i2c_check_ack(void)
{
    PER_BIW0_ACK_t  ack;
    cs_uint32 loopcount =0;

    do
    {
        if(loopcount++ > I2C_TIMEOUT)
        {
            I2C_ERR("Check ACK timeout\n");
            return;
        }

        CS_REG_READ(CS_BIW0_ACK, ack.wrd);
    } while((ack.bf.ack_out));

    I2C_DBG("ack_out at %d\n", loopcount);
}

static void i2c_wait(void)
{
    PER_BIW0_CTRL_t ctrl;
    cs_uint32 loopcount =0;

    do
    {
        if(loopcount++ > I2C_TIMEOUT)
        {
            I2C_ERR("i2c wait ctrl done failed\n");
            return;
        }
        CS_REG_READ(CS_BIW0_CTRL, ctrl.wrd);
    } while(!(ctrl.bf.biwdone));

    I2C_DBG("biwdone at %d\n", loopcount);

    /* Write back to clear done flag */
    CS_REG_WRITE(CS_BIW0_CTRL, ctrl.wrd);
}

cs_status cs_i2c_read( cs_uint32 slave_addr, cs_uint32 offset, cs_uint32 len, cs_uint8* data)
{
	cs_uint32 reg_data;
    cs_uint32 byte_data = 0;
    cs_uint32 byte_idx = 0;
    PER_BIW0_CTRL_t ctrl_value;
	PER_BIW0_CFG_t	cfg;
	cs_uint8 i=0;

	I2C_DBG("read: slave_id: 0x%x, offset: %d, len: %d\n", slave_addr, offset, len);
	cs_mutex_lock(g_i2c_mutex);

	for(i=0; i<I2C_MAX_DEVICE_NUM; i++)
	{
		if(g_i2c_speed[i].slave_addr == slave_addr)
			break;
	}
	if(i==I2C_MAX_DEVICE_NUM || g_i2c_speed[i].freq_khz==0x0){
		I2C_ERR("not set i2c speed");
		cs_mutex_unlock(g_i2c_mutex);
		return CS_ERROR;
	}

	CS_REG_READ(CS_BIW0_CFG, cfg.wrd);
    cfg.bf.prer = ARM_IMST_DEV_FREQ/(g_i2c_speed[i].freq_khz*1000)/I2C_DIVIDER - 1;
    cfg.bf.core_en = 0x1;
    CS_REG_WRITE(CS_BIW0_CFG, cfg.wrd);

    /* Send slave address and write */
    CS_REG_WRITE (CS_BIW0_TXR, slave_addr << 0x1);

    ctrl_value.wrd = 0;
    ctrl_value.bf.start = 1;
    ctrl_value.bf.write = 1;
    CS_REG_WRITE (CS_BIW0_CTRL, ctrl_value.wrd);
    i2c_wait();
    i2c_check_ack();

    /* set 2nd slave offset */
    CS_REG_WRITE (CS_BIW0_TXR, offset);

    ctrl_value.wrd = 0;
    ctrl_value.bf.write = 1;
    CS_REG_WRITE (CS_BIW0_CTRL, ctrl_value.wrd);
    i2c_wait();
    i2c_check_ack();

    /* send slave offset with start condition */
    /* set read flag LSB to 1 */
    reg_data = (slave_addr << 0x01) + 0x1;
    CS_REG_WRITE (CS_BIW0_TXR, reg_data);

    ctrl_value.wrd = 0;
    ctrl_value.bf.start = 1;
    ctrl_value.bf.write = 1;
    CS_REG_WRITE (CS_BIW0_CTRL, ctrl_value.wrd);
    i2c_wait();
    i2c_check_ack();

    for(byte_idx = 0; byte_idx < len - 1; byte_idx++)
    {
        ctrl_value.wrd = 0;
        ctrl_value.bf.read = 1;
        ctrl_value.bf.ack_in = 0;   /* ACK after read */
        CS_REG_WRITE(IROS_PER_BIW0_CTRL, ctrl_value.wrd);
        i2c_wait();

        CS_REG_READ(IROS_PER_BIW0_RXR, reg_data);
        byte_data = reg_data & 0x000000FF;
        data[byte_idx] = byte_data;
    }

    // read last byte, NACK and then generate STOP
    ctrl_value.wrd = 0;
    ctrl_value.bf.read = 1;
    ctrl_value.bf.stop = 1;
    ctrl_value.bf.ack_in = 1;
    CS_REG_WRITE (IROS_PER_BIW0_CTRL, ctrl_value.wrd);
    i2c_wait();

    CS_REG_READ(IROS_PER_BIW0_RXR, reg_data);
    byte_data = reg_data & 0x000000FF;
    data[byte_idx] = byte_data;

    cs_mutex_unlock(g_i2c_mutex);
    I2C_DBG("i2c read finished");

    return CS_OK;

}

cs_status cs_i2c_write(cs_uint32 slave_addr, cs_uint32 offset, cs_uint32 len, cs_uint8* data)
{

    PER_BIW0_CTRL_t ctrl_value;
    PER_BIW0_CFG_t	cfg;
    cs_uint32 reg_data;
    cs_uint32 byte_idx = 0;
	cs_uint8 i=0;

	cs_mutex_lock(g_i2c_mutex);
	I2C_DBG("write: slave_id: 0x%x, offset: %d, len: %d, data: 0x%x\n", slave_addr, offset, len, *data);

	for(i=0; i<I2C_MAX_DEVICE_NUM; i++)
	{
		if(g_i2c_speed[i].slave_addr == slave_addr)
			break;
	}
	if(i==I2C_MAX_DEVICE_NUM || g_i2c_speed[i].freq_khz==0x0){
		I2C_ERR("not set i2c speed\n");
		return CS_ERROR;
	}

	CS_REG_READ(CS_BIW0_CFG, cfg.wrd);
    cfg.bf.prer = ARM_IMST_DEV_FREQ/(g_i2c_speed[i].freq_khz*1000)/I2C_DIVIDER - 1;
    cfg.bf.core_en = 0x1;
    CS_REG_WRITE(CS_BIW0_CFG, cfg.wrd);

    /* Send slave address and write */
    CS_REG_WRITE(CS_BIW0_TXR, slave_addr << 0x01);

    ctrl_value.wrd = 0;
    ctrl_value.bf.start = 1;
    ctrl_value.bf.write = 1;
    CS_REG_WRITE(CS_BIW0_CTRL, ctrl_value.wrd);
    i2c_wait();
    i2c_check_ack();

    CS_REG_WRITE(CS_BIW0_TXR, offset);
    ctrl_value.wrd = 0;
    ctrl_value.bf.write = 1;
    CS_REG_WRITE(CS_BIW0_CTRL, ctrl_value.wrd);
    i2c_wait();
    i2c_check_ack();

    /* begin to write data */
    for(byte_idx = 0; byte_idx < len; byte_idx++)
    {
        /* write to slave */
        reg_data = data[byte_idx];
        CS_REG_WRITE(CS_BIW0_TXR, reg_data);
        ctrl_value.wrd = 0;
        ctrl_value.bf.write = 1;
        CS_REG_WRITE(CS_BIW0_CTRL, ctrl_value.wrd);
        i2c_wait();

        i2c_check_ack();

        I2C_DBG("I2C write: offset 0x%0x data 0x%0x\n", offset + byte_idx, reg_data);
    }

    /* stop */
    ctrl_value.wrd = 0;
    ctrl_value.bf.stop = 1;
    CS_REG_WRITE(CS_BIW0_CTRL, ctrl_value.wrd);
    i2c_wait();

    I2C_DBG("write done\n");
    cs_mutex_unlock(g_i2c_mutex);

    return CS_OK;

}

cs_status cs_i2c_init(void)
{
    PER_BIW0_CFG_t  cfg;
   	cs_status rt;
   	cs_uint8 i=0;

    /* Caculation Formula: <prer> = CPU_Peripheral_CLK/(5 * I2C_CLK) - 1 */
    cfg.bf.prer = ARM_IMST_DEV_FREQ/(I2C_CLK_KHZ_CONFIG*1000)/I2C_DIVIDER - 1;
    cfg.bf.core_en = 0x1;  /* enable */
    CS_REG_WRITE(CS_BIW0_CFG, cfg.wrd);

	for(i=0; i<I2C_MAX_DEVICE_NUM; i++)
	{
		g_i2c_speed[i].slave_addr = 0x0;
		g_i2c_speed[i].freq_khz= 0x0;
	}

	rt = cs_mutex_init(&g_i2c_mutex,  "i2c mutex",0);
    if(rt!=CS_OK)
    {
        I2C_ERR("i2c creat mutex fail");
    }

    /* Init OPM speed */
    cs_i2c_speed_set(0x50,70);
    cs_i2c_speed_set(0x51,70);
    
    /* Init PSE speed */
    cs_i2c_speed_set(0x20,70);
    cs_i2c_speed_set(0x21,70);
    cs_i2c_speed_set(0x22,70);
    cs_i2c_speed_set(0x23,70);
    return CS_OK;
}

static void i2c_dev_mode(cs_uint32 upaddr, cs_uint32 burst, cs_uint32 trig_sel, cs_uint32 rd_en, cs_uint32 adr_len, cs_uint32 word_cnt)
{
    PER_BIW_MODE0_t reg_data;

    reg_data.bf.div_id = upaddr;
    reg_data.bf.biw_word_cnt = word_cnt;
    reg_data.bf.blk_adr_len = adr_len;
    reg_data.bf.blk_rd_en = rd_en;
    reg_data.bf.biw_trg_sel = trig_sel;
    reg_data.bf.biw_burst = burst;

    CS_REG_WRITE(CS_BIW_MODE0, reg_data.wrd);
}

void i2c_burst_rd_cmd( cs_uint32 addr, cs_uint32 word_cnt,cs_uint32 data[])
{
    cs_uint32 addr0, addr8, i;
    PER_BIW0_CFG_t cfg;
    PER_BIW0_CTRL_t ctrl_value;

    if(word_cnt >16)
    {
        I2C_ERR("biw burst mode max 16 words\n");
        return ;
    }

    addr0 = addr & 0xff;
    addr8 = (addr >> 8) & 0xff;
    CS_REG_READ(CS_BIW0_CFG, cfg.wrd);
    cfg.bf.biw_srt_adr = addr0;
    CS_REG_WRITE (CS_BIW0_CFG, cfg.wrd);

    i2c_dev_mode(addr8<<1, I2C_MODE_BURST, I2C_MODE_TRG_SEL_FROM_BIW, I2C_MODE_BLK_RD_EN_AUTO, I2C_MODE_BLK_ADR_LEN_8BIT, word_cnt-1);

    /* start read */
    ctrl_value.wrd = 0;
    ctrl_value.bf.start = 1;
    ctrl_value.bf.read = 1;
    CS_REG_WRITE (CS_BIW0_CTRL, ctrl_value.wrd);
    i2c_wait();

    for (i=0; i<word_cnt; i++) {
        CS_REG_READ(CS_BIW_MEM,  data[i] );
    }

    /* clean up to normal mode */
    CS_REG_WRITE(CS_BIW_MODE0, 0);
    i2c_wait();
}


