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
/*
 *
 * watchdog.c
 *
 * $Id: watchdog.c,v 1.1.4.3.8.1.6.2 2011/09/25 10:52:48 ljin Exp $
 */

#include "iros_config.h"
#include "plat_common.h"
#include "registers.h"
#include "watchdog.h"

/* default 5 min */
static cs_uint32 g_wdt_sec     = 300;
static wdt_mode_t g_wdt_mode = 0x0;
static cs_wdt_isr_t g_wdt_intr_handler = NULL;

static cs_uint32 wdt_dbg_log = 0;

/* module_param(wdt_dbg_log, uint, 0644); */

#define WDT_DBG(format, arg...)    \
do {                                \
    if (wdt_dbg_log)                    \
        cs_printf("%s: " format,    \
                "wdt" , ## arg);    \
} while (0)

#define WDT_ERR(format, arg...)    \
    do {                            \
        cs_printf("error: %s,line %d: " format,    \
            __FILE__,__LINE__, ## arg); \
    } while (0)


cs_uint32 cs_wdt_int_get(void)
{
    PER_WDT_INT_t wdt_int;
    
    /*read watch dog counter*/
    CS_REG_READ(CS_WDT_INT,wdt_int.wrd);
     
    return wdt_int.wrd;
}


cs_uint32 cs_watchdog_counter_get(void)
{
    PER_WDT_CNT_t wdt_cnt1;
    
    /*read watch dog counter*/
    CS_REG_READ(CS_WDT_COUNT,wdt_cnt1.wrd);
     
    return wdt_cnt1.wrd;
}

cs_uint32 cs_wdt_counter_get(void)
{
    PER_WDT_CNT_t wdt_cnt1;
    
    /*read watch dog counter*/
    CS_REG_READ(CS_WDT_COUNT,wdt_cnt1.wrd);
     
    return wdt_cnt1.wrd;
}

cs_status cs_wdt_mode_get(wdt_mode_t* mode)
{
    *mode = g_wdt_mode;
        
    return CS_OK;
}

static cs_int32 cs_wdt_isr(int irq, void *dev_id)
{
    PER_WDT_INT_t wdt_int;
    
    if(irq != WDT_INT_NUM)
    {
        WDT_ERR("wdt: wrong irq number: %d\n", irq);
        return CS_ERROR;
    }

    /* clear the wdt interrupt status */
    wdt_int.bf.wdti = 0x1;
    CS_REG_WRITE(CS_WDT_INT, wdt_int.wrd);
    WDT_DBG("wdt int stat: %d\n", cs_wdt_int_get());
    WDT_DBG("watchdog interrupt");
    if(g_wdt_intr_handler != NULL)
        g_wdt_intr_handler(dev_id);
    
    WDT_DBG("watchdog interrupt finished\n");

    return CS_OK;
}

cs_status cs_wdt_mode_set(wdt_mode_t mode)
{
    PER_WDT_CTRL_t     wdt_ctrl;
    PER_WDT_INT_t     wdt_int;
    PER_WDT_IE_t    wdt_ie;

    g_wdt_mode = mode;
    switch(mode)
    {
        case WDT_RESET:
            CS_REG_READ(CS_WDT_CTRL, wdt_ctrl.wrd);
            wdt_ctrl.bf.rsten = 0x1;
            CS_REG_WRITE(CS_WDT_CTRL, wdt_ctrl.wrd);
            break;
        case WDT_INTERRUPT:
            wdt_int.bf.wdti = 0x1;
            CS_REG_WRITE(CS_WDT_INT, wdt_int.wrd);
            wdt_ie.bf.wdte = 0x1;
            CS_REG_WRITE(CS_WDT_INTEN, wdt_ie.wrd);
            break;
        case WDT_NORMAL:
        default:
            return CS_OK;
    }

    return CS_OK;
}

cs_status cs_wdt_interval_get(cs_uint32* interval)
{
    *interval = g_wdt_sec;
    return CS_OK;
}

cs_status cs_wdt_interval_set(cs_uint32 interval)
{
    PER_WDT_LD_t wdt_ld;
    PER_WDT_LOADE_t wdt_lde;
    
    g_wdt_sec = interval;
    
    if(0xffffffff == interval )
        wdt_ld.wrd = 0xffffffff;
    else
        wdt_ld.wrd = interval * (ARM_IMST_DEV_FREQ / WDT_PRESCALE_VAL);

    CS_REG_WRITE(CS_WDT_LOAD, wdt_ld.wrd);
    wdt_lde.bf.update_pre = 0x1;
    wdt_lde.bf.update_wdt = 0x1;
    CS_REG_WRITE(CS_WDT_LOAD_ENABLE, wdt_lde.wrd);
    return CS_OK;
}

cs_status cs_wdt_intr_handler_get(cs_wdt_isr_t* handler)
{
    *handler = g_wdt_intr_handler;
    return CS_OK;
}

cs_status cs_wdt_intr_handler_set(cs_wdt_isr_t handler, void* data)
{
    g_wdt_intr_handler = handler;
    cs_intr_reg(WDT_INT_NUM, "wdt_irq",cs_wdt_isr,(void*)WDT_INT_NUM, INTR_DSR_CB);        
    /* request_irq(WDT_INT_NUM, cs_wdt_isr, IRQF_DISABLED, "lynxd_wdt", data); */
    return CS_OK;
}

cs_status cs_wdt_enable(void)
{
    PER_WDT_CTRL_t wdt_ctrl;
    
    CS_REG_READ(CS_WDT_CTRL, wdt_ctrl.wrd);
    wdt_ctrl.bf.clksel = 0x1;
    wdt_ctrl.bf.wdten = 0x1;
    CS_REG_WRITE(CS_WDT_CTRL, wdt_ctrl.wrd);
    
    return CS_OK;
}

cs_status cs_wdt_disable(void)
{
    g_wdt_intr_handler = NULL;

    CS_REG_WRITE(CS_WDT_CTRL, 0);
    CS_REG_WRITE(CS_WDT_LOAD_ENABLE, 0);
    CS_REG_WRITE(CS_WDT_INTEN, 0);

    return CS_OK;
}


cs_status cs_wdt_init(void)
{
    cs_uint32 pre_clk;

    pre_clk = WDT_PRESCALE_VAL;
    WDT_DBG("wdt enable 0, pre_clk: %#x\n", pre_clk);

    CS_REG_WRITE(CS_WDT_DIVIDER, WDT_DIVIDER_VAL);
    CS_REG_WRITE(CS_WDT_PRESCALE, WDT_PRESCALE_VAL);
    cs_wdt_mode_set(WDT_NORMAL);
    cs_wdt_interval_set(0xffffffff);
    cs_wdt_enable();
    return CS_OK;
}

cs_status cs_wdt_kick(void)
{
    CS_REG_WRITE(CS_WDT_LOAD_ENABLE, (CS_WDT_LOAD_ENABLE_PS | CS_WDT_LOAD_ENABLE_WDT));
    return CS_OK;
}


