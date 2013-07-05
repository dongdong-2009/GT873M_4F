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

DEFINITIONS:  "DEVICE" means the Cortina Systems(TM) SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems(TM) SDK software.

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
 * ssp.c
 *
 * $Id: ssp.c,v 1.1.4.6.2.7.2.3.2.12 2011/09/25 10:52:47 ljin Exp $
 *
 */

/****************************************************************

OVERVIEW

This file contains the SSP driver code

****************************************************************/
#include "iros_config.h"
#include "plat_common.h"
#include "registers.h"
#include "ssp.h"


static cs_uint32 ssp_access_sem;
static cs_uint32 ssp_tran_lock;
extern cs_uint32  iros_flash_mutex;

#define LYNXD_ASIC_HCLK_KHZ         125000 /* 125 MHz */

static cs_uint32 ssp_dbg = 0;
#define SSP_DBG(format, arg...)    \
do {                                \
    if (ssp_dbg)                    \
     cs_printf("%s: " format,"ssp" , ## arg);    \
} while (0)


cs_status ssp_oper_trigger_done(void)
{
  AHB_SSP_CTRL_t ctrl;
  cs_uint32 cnt = SSP_POLL_TIME;

  /* start the control to receive the data.
    * poll until it finishes */
  CS_REG_READ(IROS_AHB_SSP_CTRL, ctrl.wrd);
  ctrl.bf.sspstart = 1;
  CS_REG_WRITE(IROS_AHB_SSP_CTRL, ctrl.wrd);

  do {
        CS_REG_READ(IROS_AHB_SSP_CTRL, ctrl.wrd);
        if (ctrl.bf.sspdone) {
            break;
        }
   } while (cnt--);

  if (cnt <= 0) {
        /* unable to receive completely before count runs out */
        cs_printf("couldn't complete receiving data\n");
        return CS_E_TIMEOUT;
  }

  /* write sspdone back to register to clear off */
  CS_REG_READ(IROS_AHB_SSP_CTRL, ctrl.wrd);
  ctrl.bf.sspdone = 1;
  CS_REG_WRITE(IROS_AHB_SSP_CTRL, ctrl.wrd);

  return CS_E_OK;

}


cs_status ssp_config_init(ssp_intf_cfg_t *in_cfg)
{
    AHB_SSP_CFG_t cfg;
    AHB_SSP_CLK_t clk;

    if (in_cfg == NULL) return CS_E_PARAM;
    if (0 == in_cfg->speed_kHz) return CS_E_PARAM;

    CS_REG_READ(IROS_AHB_SSP_CLK, clk.wrd);
    clk.bf.counter_load = LYNXD_ASIC_HCLK_KHZ/(2*in_cfg->speed_kHz) -1;
    CS_REG_WRITE(IROS_AHB_SSP_CLK, clk.wrd);
    SSP_DBG("SSP_CLK: 0x%08x\n", clk.wrd);

    CS_REG_READ(IROS_AHB_SSP_CFG, cfg.wrd);
    cfg.bf.sel_ssp_cs           = 0xF  & (1 << in_cfg->ssp_select);
    cfg.bf.micro_wire_cs_sel    = 0xF  & (in_cfg->mwr_ssp_select << in_cfg->ssp_select);
    cfg.bf.tdat_cpha            = 0x1  & in_cfg->tdat_cpha;
    cfg.bf.idat_mode            = 0x1  & in_cfg->idat_mode;
    cfg.bf.datin_cmds           = 0x1  & in_cfg->datin_cmds;
    cfg.bf.edge_align           = 0x1  & in_cfg->edge_align;
    cfg.bf.pre_ssp_dat_cnt      = 0x1F & in_cfg->data_len;
    cfg.bf.ssp_cmd_cnt          = 0x7F & in_cfg->command_len;
    cfg.bf.command_cyc          = 0x1  & in_cfg->command_only;
    CS_REG_WRITE(IROS_AHB_SSP_CFG, cfg.wrd);
    SSP_DBG("SSP_CFG: 0x%08x\n", cfg.wrd);

    return CS_E_OK;
}

cs_status ssp_data_write(cs_int32 data)
{
    AHB_SSP_CFG_t cfg;
    cs_status ret = CS_E_OK;

    /* prepare cfg value */
    CS_REG_READ(IROS_AHB_SSP_CFG, cfg.wrd);
    cfg.bf.read_write = 0;      /* set it as write */
    CS_REG_WRITE(IROS_AHB_SSP_CFG, cfg.wrd);

    CS_REG_WRITE(IROS_AHB_SSP_WDAT, data);

    ret = ssp_oper_trigger_done();

    return ret;
}

cs_status ssp_data_read(cs_int32 *rdata)
{
    AHB_SSP_CFG_t cfg;
    cs_status ret = CS_E_OK;
    cs_int32 read_data;

    /* prepare cfg value */
    CS_REG_READ(IROS_AHB_SSP_CFG, cfg.wrd);
    cfg.bf.read_write = 1;      /* set it as read */
    CS_REG_WRITE(IROS_AHB_SSP_CFG, cfg.wrd);

    ret = ssp_oper_trigger_done();
    if(CS_E_OK == ret){
          /* read the data */
          CS_REG_READ(IROS_AHB_SSP_RDAT, read_data);
          *rdata = read_data;
    }

    return ret;
}

cs_status ssp_cmd_set(cs_uint32 cmd_addr_0,cs_uint32 cmd_addr_1,cs_uint32 cmd_addr_2)
{
    /* write the _data to cmd0 register */
    CS_REG_WRITE(IROS_AHB_SSP_CA0, cmd_addr_0);
    CS_REG_WRITE(IROS_AHB_SSP_CA1, cmd_addr_1);
    CS_REG_WRITE(IROS_AHB_SSP_CA2, cmd_addr_2);

    return CS_E_OK;
}

/* Chip select 3 is used for SSP RXD mux, deselect is requred after use*/
void cs3_deselect(void)
{
    AHB_SSP_CFG_t ssp_cfg;

    /* deselect chip select 3 */
    CS_REG_READ(IROS_AHB_SSP_CFG, ssp_cfg.wrd);
    ssp_cfg.bf.sel_ssp_cs &= 0x7;  
    CS_REG_WRITE(IROS_AHB_SSP_CFG, ssp_cfg.wrd);
}

/* SerDes APIs */
unsigned char reverse8( unsigned char c )
{
    c = ( c & 0x55 ) << 1 | ( c & 0xAA ) >> 1;
    c = ( c & 0x33 ) << 2 | ( c & 0xCC ) >> 2;
    c = ( c & 0x0F ) << 4 | ( c & 0xF0 ) >> 4;
    return c;
}

void ssp_intr_clear()
{
    AHB_SSP_INT_t int_stat;

    CS_REG_READ(IROS_AHB_SSP_INT, int_stat.wrd);
    int_stat.bf.sspi = 1;  
    CS_REG_WRITE(IROS_AHB_SSP_INT, int_stat.wrd);    
}

void ssp_intr_enable()
{
    AHB_SSP_IE_t int_en;

    CS_REG_READ(IROS_AHB_SSP_IE, int_en.wrd);
    int_en.bf.sspe = 1; 
    CS_REG_WRITE(IROS_AHB_SSP_IE, int_en.wrd);    
}

void ssp_intr_hanlder()
{
    AHB_SSP_INT_t int_stat;

    CS_REG_READ(IROS_AHB_SSP_INT, int_stat.wrd);

    // Check if SSP done interrupt raised
    if(int_stat.bf.sspi)
    {
        // clear the INTR status
        ssp_intr_clear();
#ifndef IROSBOOT
        cs_semaphore_post(ssp_access_sem);
#endif
    }
    else
    {
        diag_printf("unknow SSP intr\n");
    }

}

void ssp_init(void)
{
    cs_status   ret = CS_E_OK;
    
    diag_printf("Initializing SSP\n");

#ifndef IROSBOOT
    cs_semaphore_init((cs_uint32 *)&ssp_access_sem,"SSP internal bus sem", 1,0);
    ret = cs_mutex_init((cs_uint32 *)&ssp_tran_lock, "SSP interface mutex",0);
    if (ret != CS_E_OK) {
        diag_printf("cs_mutex_init FAILED ret=0x%x\n",ret);
    }
#endif

    cs_mutex_init(&iros_flash_mutex, "Flashmutex", 0);
    ssp_intr_clear();
    ssp_intr_enable();
}

void ssp_bus_lock(void)
{
    cs_mutex_lock(ssp_tran_lock);
}

void ssp_bus_unlock(void)
{
    cs_mutex_unlock(ssp_tran_lock);
}


