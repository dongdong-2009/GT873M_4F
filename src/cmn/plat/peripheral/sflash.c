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
 * $Id: sflash.c,v 1.1.6.2 2011/09/25 10:52:47 ljin Exp $
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
#include "sflash.h"


#ifdef HAVE_SERIAL_FLASH

#ifndef IROSBOOT

/* Definitions for sflash test */
static  cs_uint32 sflash_test_len[]={0x1000,0x2000,0x10000,0x20000,0x631a9,0x80000,0x100000,0};      
static cs_uint32 test_base = 0x40300020; /* select  a 256 bytes unaligned address */
static cs_uint32 start_block = 0x30;
bool sflash_detail_info= FALSE;

#define MEM_BUF           0x10700000
#define SFLASH_DBG  if(sflash_detail_info) cs_printf 

extern bool  do_memcmp(const void *m1, const void *m2, unsigned int n);
#endif

cs_int32 ssp_set_config(ssp_intf_cfg_t *in_cfg)
{
    AHB_SSP_CFG_t cfg;
    AHB_SSP_CLK_t clk;

    if (in_cfg == NULL) return -1;
    if (0 == in_cfg->speed_kHz) return -1;

    CS_REG_READ(IROS_AHB_SSP_CLK, clk.wrd);
    clk.bf.counter_load = in_cfg->speed_kHz;
    CS_REG_WRITE(IROS_AHB_SSP_CLK, clk.wrd);
    // diag_printf("SSP_CLK: 0x%08x\n", clk.wrd);

    CS_REG_READ(IROS_AHB_SSP_CFG, cfg.wrd);
    cfg.bf.sel_ssp_cs           = 0xF  & in_cfg->ssp_select;
    cfg.bf.micro_wire_cs_sel    = 0xF  & in_cfg->mwr_ssp_select;
    cfg.bf.pre_ssp_dat_cnt      = 0x1F & in_cfg->data_len;
    cfg.bf.ssp_cmd_cnt          = 0x7F & in_cfg->command_len;
    cfg.bf.command_cyc          = 0x1  & in_cfg->command_only;
    cfg.bf.edge_align           = 0x1  & in_cfg->edge_align;
    cfg.bf.datin_cmds           = 0x1  & in_cfg->datin_cmds;
    cfg.bf.idat_mode            = 0x1  & in_cfg->idat_mode;
    cfg.bf.tdat_cpha            = 0x1  & in_cfg->tdat_cpha;
    cfg.bf.read_write           = 0x1  & in_cfg->read_write;
    CS_REG_WRITE(IROS_AHB_SSP_CFG, cfg.wrd);
    //diag_printf("SSP_CFG: 0x%08x\n", cfg.wrd);

    return 0;
}

cs_int32 ssp_get_config(ssp_intf_cfg_t *out_cfg)
{
    AHB_SSP_CFG_t cfg;
    AHB_SSP_CLK_t clk;
    cs_uint16 per_clk_KHz, prer;

    if (out_cfg == NULL) return -1;

    per_clk_KHz = SSP_CLOCK;
    /*
     * The equation to compute the data to be programmed is
     * Speed_Khz = (prer + 1) * 2) /(per_clk_Khz * 1000) ;
     */
    CS_REG_READ(IROS_AHB_SSP_CLK, clk.wrd);
    prer = (((clk.bf.counter_load + 1) * 2) /(per_clk_KHz * 1000));
    out_cfg->speed_kHz = prer;

    CS_REG_READ(IROS_AHB_SSP_CFG, cfg.wrd);
    out_cfg->ssp_select = cfg.bf.sel_ssp_cs;
    out_cfg->mwr_ssp_select = cfg.bf.micro_wire_cs_sel;
    out_cfg->data_len = cfg.bf.pre_ssp_dat_cnt;
    out_cfg->command_len = cfg.bf.ssp_cmd_cnt;
    out_cfg->command_only = cfg.bf.command_cyc;

    return 0;
}

/* Serial Flash Drivers */
static void ssp_sflash_triggle_cmd()
{
    AHB_SSP_CTRL_t ctrl;
    cs_uint32 cnt=0;
    /* set the control before invoking write command */
    CS_REG_READ(IROS_AHB_SSP_CTRL, ctrl.wrd);
    ctrl.bf.sspstart = 1;
    CS_REG_WRITE(IROS_AHB_SSP_CTRL, ctrl.wrd);

    /* polling until cmd done */
    cnt = SSP_POLL_TIME;

    while(cnt){
        cnt--;
        CS_REG_READ(IROS_AHB_SSP_CTRL, ctrl.wrd);
        if (ctrl.bf.sspdone) break;
    }

    if (cnt <= 0) {
    //    cs_printf("SSP write timeout\n");
        return ;
    }
    /* write sspdone back to register to clear off */
    CS_REG_READ(IROS_AHB_SSP_CTRL, ctrl.wrd);
    ctrl.bf.sspdone = 1;
    CS_REG_WRITE(IROS_AHB_SSP_CTRL, ctrl.wrd);
}

static void ssp_sflash_write_done()
{
    ssp_intf_cfg_t cfg;
    unsigned int val32=0;

    memset(&cfg, 0, sizeof(cfg));
    cfg.speed_kHz = 0x10;
    cfg.ssp_select = 0x1;
    cfg.mwr_ssp_select = 0x0;  // each bit, 0: spi 1:MWR
    cfg.data_len = 0x1f;
    cfg.command_len = 0x7;      // to read 8 bit, include 1 additional bit
    cfg.command_only = 0;       // 1: command-only, 0: Normal
    cfg.read_write = SSP_CMD_READ; 
    ssp_set_config(&cfg);  
    CS_REG_WRITE(IROS_AHB_SSP_CA0, (SSP_SFLASH_OP_RDSR<<24) ); 
    
    do{
         /*Check WIP done*/         
         ssp_sflash_triggle_cmd();
         CS_REG_READ(IROS_AHB_SSP_RDAT,val32);
    } while( val32 & SSP_SFLASH_SR_WIP );
}


static void ssp_sflash_cfg_cmd(cs_uint32 cmd)
{
    ssp_intf_cfg_t cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.speed_kHz = 0x10;
    cfg.ssp_select = 0x1;
    cfg.mwr_ssp_select = 0x0;  // each bit, 0: spi 1:MWR
    cfg.data_len = 0x0;
    cfg.command_len = 0x7;      // to read 8 bit, include 1 additional bit
    cfg.command_only = 1;       // 1: command-only, 0: Normal
    cfg.read_write=0;           // 1: read;   0: write
    ssp_set_config(&cfg);

    /* write the _data to cmd0 register */
    CS_REG_WRITE(IROS_AHB_SSP_CA0, cmd<<24);
    CS_REG_WRITE(IROS_AHB_SSP_CA1, 0);
    CS_REG_WRITE(IROS_AHB_SSP_CA2, 0);

    ssp_sflash_triggle_cmd();
}


static void ssp_sflash_query(cs_uint32 * data)
{    
    ssp_intf_cfg_t cfg;
    cs_uint32 val32 = 0xFFFFFFFF;
    cs_uint8 id[4] = {0};
    cs_uint16* flash_id = (cs_uint16*)data;

    #ifndef IROSBOOT
    ssp_bus_lock();
    #endif
    memset(&cfg, 0, sizeof(cfg));
    cfg.speed_kHz = 0x10;
    cfg.ssp_select = 0x1;
    cfg.mwr_ssp_select = 0x0;  // each bit, 0: spi 1:MWR
    cfg.data_len = 0x1F;
    cfg.command_len = 0x7;      // to read 8 bit, include 1 additional bit
    cfg.command_only = 0;       // 1: command-only, 0: Normal
    cfg.read_write = SSP_CMD_READ; 
    ssp_set_config(&cfg);  
    CS_REG_WRITE(IROS_AHB_SSP_CA0, (SSP_SFLASH_OP_RDID<<24) ); 
    
    /*Check WIP done*/         
    ssp_sflash_triggle_cmd();
    CS_REG_READ(IROS_AHB_SSP_RDAT,val32);
    *data = val32;
        /* Manufacture ID */
    id[0] = (val32 & 0xFF000000) >> 24;
    /* Device ID0 */
    id[1] = (val32 & 0xFF0000) >> 16;
    /* Device ID1 */
    id[2] = (val32 & 0xFF00) >> 8;
    /* Extention Device ID0 */
    id[3] = (val32 & 0xFF);

    flash_id[0] = id[0] & 0xFF;
    flash_id[1] = (id[1] << 8) | id[2];
    #ifndef IROSBOOT
    ssp_bus_unlock();
    #endif
    
}

static void ssp_sflash_cfg_data(cs_uint32 data_len)
{
    ssp_intf_cfg_t cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.speed_kHz = 0x10;
    cfg.ssp_select = 0x1;
    cfg.mwr_ssp_select = 0x0;   // each bit, 0: spi 1:MWR
    cfg.data_len = data_len;     // real len= data_len+1;
    cfg.command_len = 0x1F;      // to read 8 bit, include 1 additional bit
    cfg.command_only = 0;        // 1: command-only, 0: Normal
    cfg.read_write =  SSP_CMD_WRITE;      /* set it as write */
    ssp_set_config(&cfg);
}

static void sflash_write_byte(cs_uint32 _addr, cs_uint8 _data)
{
    ssp_sflash_cfg_cmd(SSP_SFLASH_OP_WREN); 
    
    ssp_sflash_cfg_data(SSP_PRE_DATA_CNT_8BIT);
    /* write the _data to cmd0 register */
    CS_REG_WRITE(IROS_AHB_SSP_CA0, (SSP_SFLASH_OP_PP<<24) | _addr);
    CS_REG_WRITE(IROS_AHB_SSP_WDAT, _data);
    
    ssp_sflash_triggle_cmd();
    ssp_sflash_write_done();

}

static void sflash_write_word(cs_uint32 _addr, cs_uint32 _data)
{

    ssp_sflash_cfg_cmd(SSP_SFLASH_OP_WREN);
    
    ssp_sflash_cfg_data(SSP_PRE_DATA_CNT_32BIT);
    /* write the _data to cmd0 register */
    CS_REG_WRITE(IROS_AHB_SSP_CA0, (SSP_SFLASH_OP_PP<<24) | _addr);
    CS_REG_WRITE(IROS_AHB_SSP_WDAT, _data);

    ssp_sflash_triggle_cmd();
    ssp_sflash_write_done();

}

static void sflash_sector_erase(cs_uint32 s_addr) {
    
    AHB_SSP_CFG_t cfg;
    cs_uint32 sflash_addr;

    #ifndef IROSBOOT
    ssp_bus_lock();
    #endif
    sflash_addr = s_addr - SFLASH_BASE;
    ssp_sflash_cfg_cmd(SSP_SFLASH_OP_WREN);

    ssp_sflash_cfg_data(SSP_PRE_DATA_CNT_8BIT);
    /* prepare cfg value */
    CS_REG_READ(IROS_AHB_SSP_CFG, cfg.wrd);
    cfg.bf.command_cyc = 1;
    CS_REG_WRITE(IROS_AHB_SSP_CFG, cfg.wrd);    

    /* write the _data to cmd0 register */
    CS_REG_WRITE(IROS_AHB_SSP_CA0, (SSP_SFLASH_OP_SE<<24) | sflash_addr);   
    ssp_sflash_triggle_cmd();
    ssp_sflash_write_done();
    #ifndef IROSBOOT
    ssp_bus_unlock();
    #endif
}

static void sflash_block_write_force(cs_uint32 src, cs_uint32 dest, cs_uint32 len)
{
    
    cs_uint32 val32=0,write_len=0,first_write=TRUE;
    /* external SDRAM */
   if(src< 0x10000000)
    {
        src |= 0x10000000;        
    }
    dest -= SFLASH_BASE;
    cache_flush_data(src,len);
    CS_REG_WRITE(IROS_AHB_SSP_CLK,    0x10);     /* Set clock speed  */   
    CS_REG_WRITE(IROS_AHB_SSP_BST1,   dest);     /* write to sflash address*/
    CS_REG_WRITE(IROS_AHB_SSP_BST2,   src );     /* src address */ 
    
     while(len>0){
             cs_uint32 cnt = SSP_POLL_TIME; 
             write_len = (len > SFLASH_MAX_WRITE_PAGE_SIZE) ? SFLASH_MAX_WRITE_PAGE_SIZE : len;
             len -=write_len;
             
             /*WREN*/
             CS_REG_WRITE(IROS_AHB_SSP_CFG,  0x10000781);   // Set brust enable   
             CS_REG_WRITE(IROS_AHB_SSP_CA0, SSP_SFLASH_OP_WREN<<24);
             ssp_sflash_triggle_cmd();
     
             CS_REG_WRITE(IROS_AHB_SSP_BST0,  ((0x4e01<<16) | (write_len*8-1)) );  // Read from 8MB sflash page size is 8KB
    
             
             if(first_write){
                 CS_REG_WRITE(IROS_AHB_SSP_INT , 0x2);          // Clear up the done status
                 CS_REG_WRITE(IROS_AHB_SSP_CTRL,  0x40      );   // Trigger to start first burst operation
                 first_write=FALSE;
                 }
             else{
                 CS_REG_WRITE(IROS_AHB_SSP_INT , 0x2);               // Clear up the done status
                 CS_REG_WRITE(IROS_AHB_SSP_CTRL,  0x20      );  // Trigger to continue burst operation
             }

             while(cnt)
                {
                    cnt--;
                    CS_REG_READ(IROS_AHB_SSP_INT ,  val32);        // check bit1 set block access done  
                    if(BITON(val32, 1)) break;
                }
             if (cnt <= 0) {
                 return;
             }
             ssp_sflash_write_done();
             CS_REG_WRITE(IROS_AHB_SSP_INT,  0x2);              // Clear up the done status
    }

    return;
}

static void sflash_block_write(cs_uint32 src, cs_uint32 dest, cs_uint32 len)
{
    
     cs_uint32 val32=0,write_len=0,first_write=TRUE;
     cs_uint8* allign_ptr = NULL;

    if( (dest< SFLASH_BASE) || (dest > (SFLASH_BASE + SFLASH_SIZE)))
    {
     //   cs_printf("Overflow sflash address space, check dest address");
        return;
    }
    
    if(len > SFLASH_SIZE )
    {
      //  cs_printf("len Overflow, check len");
      
        return;
    }
    /* src should be 4-bytes alligned */
    if(src & 0x3)
    {
        if((allign_ptr = iros_malloc(IROS_MID_SYS, len+4)) != NULL)
        {
            cs_uint8 *tmp = NULL;
            tmp = (cs_uint8*)(((cs_uint32)allign_ptr+4) & 0xfffffffc);
            memcpy(tmp, (void*)src, len);
            src = (cs_uint32)tmp;
        }
        else
        {
            cs_printf("block write:malloc fail\n");
            return;
        }
    }
    
   /* Block write use DMA, flush data cache*/ 
   cache_flush_data(src,len);
    /* external SDRAM */
   if(src< 0x10000000)
    {
        src |= 0x10000000;        
    }
    dest -= SFLASH_BASE;
    
    #ifndef IROSBOOT
    ssp_bus_lock();
    #endif     
     CS_REG_WRITE(IROS_AHB_SSP_CLK,    0x10);     /* Set clock speed  */   
     CS_REG_WRITE(IROS_AHB_SSP_BST1,   dest);     /* write to sflash address*/
     CS_REG_WRITE(IROS_AHB_SSP_BST2,   src );     /* src address */ 
    
     while(len>0){
             cs_uint32 cnt = SSP_POLL_TIME; 
             write_len = (len > SFLASH_MAX_WRITE_PAGE_SIZE) ? SFLASH_MAX_WRITE_PAGE_SIZE : len;
             len -=write_len;
             
             /*WREN*/
             CS_REG_WRITE(IROS_AHB_SSP_CFG,  0x10000781);   // Set brust enable   
             CS_REG_WRITE(IROS_AHB_SSP_CA0, SSP_SFLASH_OP_WREN<<24);
             ssp_sflash_triggle_cmd();
     
             CS_REG_WRITE(IROS_AHB_SSP_BST0,  ((0x4e01<<16) | (write_len*8-1)) );  // Read from 8MB sflash page size is 8KB
    
             
             if(first_write){
                 CS_REG_WRITE(IROS_AHB_SSP_INT , 0x2);          // Clear up the done status
                 CS_REG_WRITE(IROS_AHB_SSP_CTRL,  0x40      );   // Trigger to start first burst operation
                 first_write=FALSE;
                 }
             else{
                 CS_REG_WRITE(IROS_AHB_SSP_INT , 0x2);               // Clear up the done status
                 CS_REG_WRITE(IROS_AHB_SSP_CTRL,  0x20      );  // Trigger to continue burst operation
             }

             while(cnt)
                {
                    cnt--;
                    CS_REG_READ(IROS_AHB_SSP_INT ,  val32);        // check bit1 set block access done  
                    if(BITON(val32, 1)) break;
                }
             if (cnt <= 0) {
                // cs_printf("block access tiemout[0x%08x]\n",val32);
                 #ifndef IROSBOOT
                  ssp_bus_unlock();
                 #endif
                 if(allign_ptr != NULL)
                 {
                    iros_free(allign_ptr);
                 }
                 return;
             }
            // hal_delay_us (5000);
             ssp_sflash_write_done();
             CS_REG_WRITE(IROS_AHB_SSP_INT,  0x2);              // Clear up the done status
    }
    #ifndef IROSBOOT
    ssp_bus_unlock();
   #endif
   if(allign_ptr != NULL)
   {
        iros_free(allign_ptr);
   }
}

static void ssp_sflash_block_read(cs_uint32 src, cs_uint32 dest, cs_uint32 len)
{
    cs_uint32 val32=0,read_len=0,first_read=TRUE;
    if( (src< SFLASH_BASE) || (src > (SFLASH_BASE + SFLASH_SIZE)))
    {
       // cs_printf("Overflow sflash address space, check src address");
        return;
    }
    src -= SFLASH_BASE;
    if(len > SFLASH_SIZE )
    {
      //  cs_printf("len Overflow, check len");
        return;
    }
    
   /* Block read use DMA, flush data cache*/ 
   cache_flush_data(dest,len);
        /* external SDRAM */
   if(dest < 0x10000000)
    {
        dest |= 0x10000000;        
    }
    #ifndef IROSBOOT
    ssp_bus_lock();
    #endif
    CS_REG_WRITE(IROS_AHB_SSP_CLK,  0x10 );        // Set clock speed
    CS_REG_WRITE(IROS_AHB_SSP_CFG,  0x10000781);   // Set brust enable    
    
    CS_REG_WRITE(IROS_AHB_SSP_BST1,   src        );   // read from sflash address 0
    CS_REG_WRITE(IROS_AHB_SSP_BST2,  (cs_uint32)dest);   // dest address 

    while(len>0){
            cs_uint32 cnt = SSP_POLL_TIME;            
            read_len = (len > SFLASH_MAX_READ_PAGE_SIZE) ? SFLASH_MAX_READ_PAGE_SIZE : len;
            len -=read_len;
            
            CS_REG_WRITE(IROS_AHB_SSP_BST0,  ((0x4f01<<16) | (read_len*8-1)) );  // Read from 8MB sflash page size 
            
            if(first_read){
                CS_REG_WRITE(IROS_AHB_SSP_INT,   0x2);               // Clear up the done status
                CS_REG_WRITE(IROS_AHB_SSP_CTRL,  0x40);       // Trigger to start first burst operation
                first_read=FALSE;
                }
            else{
                /* Wait read done */
                while(cnt)
                {
                    cnt--;
                    CS_REG_READ(IROS_AHB_SSP_BST_STAT ,val32);        // check bit1 set block access done  
                    if(SBST_ACC_RDONE == (val32 & SBST_ACC_MASK))
                        break;
                }
                if (cnt <= 0) {
                    //  cs_printf("block access timeout val[0x%08x]\n",val32);
                    #ifndef IROSBOOT
                    ssp_bus_unlock();
                    #endif
                    return;
                }
                CS_REG_WRITE(IROS_AHB_SSP_INT ,  0x2);               // Clear up the done status                   
                CS_REG_WRITE(IROS_AHB_SSP_CTRL,  0x20); 
            }
            cnt = SSP_POLL_TIME;
            while(cnt)
            {
                cnt--;
                CS_REG_READ(IROS_AHB_SSP_INT ,  val32);        // check bit1 set block access done  
                if(BITON(val32, 1)) break;
            }
            if (cnt <= 0) {
              //  cs_printf("block access timeout val[0x%08x]\n",val32);
                #ifndef IROSBOOT
                ssp_bus_unlock();
                #endif
                return;
            }
            CS_REG_WRITE(IROS_AHB_SSP_INT , 0x2);               // Clear up the done status            
   }  

   #ifndef IROSBOOT
    ssp_bus_unlock();
   #endif
}


/*****************************************************************************/
/* $rtn_hdr_start  flash_query                                         */
/* CATEGORY   : Device                                                       */
/* ACCESS     : public                                                       */
/* BLOCK      : General                                                      */
/* CHIP       : CS8016                                                       */
/*                                                                           */
void flash_query(void * data)
/*                                                                           */
/* INPUTS  : o block       Block address                                     */
/* OUTPUTS : ----                                                            */
/* RETURNS : o ---                                                           */
/* DESCRIPTION:                                                              */
/*     This function is used to erase a block                                */
/* $rtn_hdr_end                                                              */
/*****************************************************************************/
{
    ssp_sflash_query((cs_uint32 * )data);
}

/*****************************************************************************/
/* $rtn_hdr_start  FLASH ERASE BLOCK                                         */
/* CATEGORY   : Device                                                       */
/* ACCESS     : public                                                       */
/* BLOCK      : General                                                      */
/* CHIP       : CS8016                                                       */
/*                                                                           */
int flash_erase_block(cs_uint32 s_addr,unsigned int block_size)
/*                                                                           */
/* INPUTS  : o block       Block address                                     */
/* OUTPUTS : ----                                                            */
/* RETURNS : o ---                                                           */
/* DESCRIPTION:                                                              */
/*     This function is used to erase a block                                */
/* $rtn_hdr_end                                                              */
/*****************************************************************************/
{

    sflash_sector_erase(s_addr);

    return 0;
}
int flash_force_erase_block(cs_uint32 s_addr,unsigned int block_size)
{
    
    AHB_SSP_CFG_t cfg;
    cs_uint32 sflash_addr;

    sflash_addr = s_addr - SFLASH_BASE;
    ssp_sflash_cfg_cmd(SSP_SFLASH_OP_WREN);

    ssp_sflash_cfg_data(SSP_PRE_DATA_CNT_8BIT);
    /* prepare cfg value */
    CS_REG_READ(IROS_AHB_SSP_CFG, cfg.wrd);
    cfg.bf.command_cyc = 1;
    CS_REG_WRITE(IROS_AHB_SSP_CFG, cfg.wrd);    

    /* write the _data to cmd0 register */
    CS_REG_WRITE(IROS_AHB_SSP_CA0, (SSP_SFLASH_OP_SE<<24) | sflash_addr);   
    ssp_sflash_triggle_cmd();
    ssp_sflash_write_done();
    return 0;
}

void sflash_program_buf_word(cs_uint32 dest,cs_uint32 src,cs_uint32 len) 
{
    cs_uint32 i=0,odd=0,writed_len=0,wrd_cnd=0; 
    cs_uint32 *p_int = 0; 

    if( (dest< SFLASH_BASE) || (dest > (SFLASH_BASE + SFLASH_SIZE)))
    {
        cs_printf("Overflow sflash address space, check src address");
        return;
    }
    if((len > SFLASH_SIZE ) || (0 == len))
    {
        cs_printf("check length");
        return;
    }
    dest -= SFLASH_BASE;
    
    //deal with odd at first
    odd=4-src%4;
    #ifndef IROSBOOT
    ssp_bus_lock();
    #endif

    while((writed_len<odd) && (odd != 4))
    {
        sflash_write_byte(dest+writed_len, *(cs_uint8 *)src);
        src +=1;
        writed_len++;
    }
    // src is 4 bound now
    //write the 4 bound data
    p_int = (cs_uint32 *)src;
    wrd_cnd= (len - writed_len)/4;        
    for(i=0;i<wrd_cnd;i++){//htonl
        sflash_write_word((dest+writed_len),  htonl(*p_int));
        p_int++;
        writed_len+=4;
        src +=4;
    }  
    //deal with the odd bytes at the tail
    while(writed_len<len)
    {
        sflash_write_byte((dest+writed_len), *(cs_uint8 *)src);
        src +=1;
        writed_len++;                
    }

    #ifndef IROSBOOT
    ssp_bus_unlock();
    #endif
}
int flash_program_buf_word(void* dest, cs_uint32 data)
{
    sflash_program_buf_word((cs_uint32)dest,(cs_uint32)&data,4);
    return 0;
}

int flash_program_buf_force(cs_uint32 dest, cs_uint32 src,cs_uint32 len,unsigned long block_mask, int buffer_size)
{
    sflash_block_write_force(src,dest,len);
    return 0;
}
/*****************************************************************************/
/* $rtn_hdr_start  FLASH ERASE BLOCK                                         */
/* CATEGORY   : Device                                                       */
/* ACCESS     : public                                                       */
/* BLOCK      : General                                                      */
/* CHIP       : CS8016                                                       */
/*                                                                           */
int flash_program_buf(cs_uint32 dest, cs_uint32 src,cs_uint32 len,unsigned long block_mask, int buffer_size)
/*                                                                           */
/* INPUTS  : o block       Block address                                     */
/* OUTPUTS : ----                                                            */
/* RETURNS : o ---                                                           */
/* DESCRIPTION:                                                              */
/*     This function is used to erase a block                                */
/* $rtn_hdr_end                                                              */
/*****************************************************************************/
{
    cs_uint32 first_write_len = 0,block_write_len = 0;
    cs_uint32 last_write_len = 0; /* process 16bytes unaligned at tail */

     if(0 == len){
        return 0;
     }
     
    /* block write length should be 16bytes aligned */
    if(len < 16)
    {
        sflash_program_buf_word(dest,src,len);
        return 0;
    }
    else
    {
        /* Source address should be 256 bytes aligned */
        first_write_len = (0x100 - (dest & 0xFF));
        /* process unaligned data */
        if(first_write_len >= len)
        {
            sflash_program_buf_word(dest,src,len);
            return 0;
         }
        else if(0x100 != first_write_len)
        {
            sflash_program_buf_word(dest,src,first_write_len);
        }
        else /* The dest is already 256 aligend */
        {
            first_write_len = 0;
        }
        /* now dest is 256 aligned */ 
        /* next check block write length should be 256bytes aligned*/
        block_write_len = len -first_write_len;
        last_write_len = block_write_len & 0xFF;
        block_write_len = len - first_write_len - last_write_len;
        sflash_block_write((src+first_write_len),(dest+first_write_len),block_write_len);

        /* write unaligned data */
        if(last_write_len > 0)
            sflash_program_buf_word((dest+first_write_len+block_write_len),
                                                    (src+first_write_len+block_write_len),last_write_len);
    }

    return 0;
}

void sflash_read(cs_uint32 dest, cs_uint32 src, cs_uint32 len)
{

    if( (src< SFLASH_BASE) || (src > (SFLASH_BASE + SFLASH_SIZE)))
    {
        cs_printf("Overflow sflash address space, check src address");
        return;
    }
    if(len > SFLASH_SIZE )
    {
        cs_printf("len Overflow, check len");
        return;
    }
#ifndef IROSBOOT
    ssp_bus_lock();
#endif
    memcpy((void *)dest, (const void *)src, len);   
#ifndef IROSBOOT
    ssp_bus_unlock();
#endif
}

void sflash_block_read(cs_uint32 dest, cs_uint32 src, cs_uint32 len)
{
    cs_uint32 byte_read_len = 0,block_read_len = 0;

    /*block write length must be 4bytes aligned*/
    byte_read_len = len % 4;
    block_read_len = len -byte_read_len;
    ssp_sflash_block_read(src,dest,block_read_len);

     /* write unaligned data */
     if(byte_read_len > 0)
        sflash_read((dest+block_read_len),(src+block_read_len),byte_read_len);
}


#ifndef IROSBOOT
static void sflash_write_test(cs_uint32 len)
{
    cs_uint32 block_num=0,loop=0;
    cs_uint64 ticks = 0;
    
    block_num = len/SFLASH_BLOCK_SIZE+1;
    /*Normal write*/
    SFLASH_DBG("*****************Normal write test%4d    KB Data start****************\n",len/0x400);
    SFLASH_DBG("1. Format memory buf\n");
    for(loop=0;loop < len; loop +=4)
        {
           *(cs_uint32 *)(MEM_BUF+loop)= rand();
        }
    SFLASH_DBG("2. Erase sflash\n");
    for(loop=start_block;loop < (start_block+block_num);loop++){
        ticks = cs_current_time();
        flash_erase_block((cs_uint32)(SFLASH_BASE + loop*SFLASH_BLOCK_SIZE),0);
        ticks = cs_current_time() - ticks;
        cs_printf("erase  ticks: %lld\n",ticks);
        SFLASH_DBG("Erase 0x%08x\n",loop*SFLASH_BLOCK_SIZE);
    } 
    SFLASH_DBG("3. Writing sflash\n");
    ticks = cs_current_time();
    sflash_program_buf_word(test_base,MEM_BUF, len);
    ticks = cs_current_time() - ticks;      
    SFLASH_DBG("4. Verifing writed data\n");
    if(do_memcmp((const void *) MEM_BUF, (const void *) test_base, len)){
        SFLASH_DBG("write verify passed\n");
        cs_printf("Normal write test %4d  KB Data src 0x%x  dest 0x%x len 0x%x ticks: %lld\n", len/0x400,MEM_BUF, test_base, len,ticks);
        }
    else
        cs_printf("Write verify failed\n");

    /*Block write*/
    SFLASH_DBG("Block write test %4d  KB Data Start\n",len/0x400);
    SFLASH_DBG("1. Format memory buf\n");
    for(loop=0;loop < len; loop +=4)
        {
           *(cs_uint32 *)(MEM_BUF+loop)= rand();
        }
    SFLASH_DBG("2. Erase sflash\n");
    for(loop=start_block;loop < (start_block+block_num);loop++){
        flash_erase_block((cs_uint32)(SFLASH_BASE + loop*SFLASH_BLOCK_SIZE),0);
        SFLASH_DBG("Erase 0x%08x\n",loop*SFLASH_BLOCK_SIZE);
    } 
    SFLASH_DBG("3. Writing sflash\n");
    ticks = cs_current_time();
    flash_program_buf(test_base,MEM_BUF, len,0,0);
    ticks = cs_current_time() - ticks;
    SFLASH_DBG("Block write src 0x%x  dest 0x%x len 0x%x ticks: %lld\n", MEM_BUF, test_base, len,ticks);
    SFLASH_DBG("4. Verifing writed data\n");
    if(do_memcmp((const void *) MEM_BUF, (const void *) test_base, len)){
        SFLASH_DBG("write verify passed\n");
        cs_printf("Block  write test %4d  KB Data src 0x%x  dest 0x%x len 0x%x ticks: %lld\n", len/0x400,MEM_BUF, test_base, len,ticks);        
        }
    else
        cs_printf("Block write verify failed\n");
}


static void sflash_read_test(cs_uint32 len)
{
    cs_uint64 ticks = 0;
    
    /*Normal READ*/
    SFLASH_DBG("Normal Read test %4d    KB Data Start\n",len/0x400);
    SFLASH_DBG("1. Format memory buf\n");
    memset((void *)MEM_BUF, 0, len); 

    SFLASH_DBG("2. Read from sflash\n");
    ticks = cs_current_time();
    sflash_read(MEM_BUF, test_base, len);
    ticks = cs_current_time() - ticks;
    SFLASH_DBG("Read src 0x%x  dest 0x%x len 0x%x ticks: %lld\n", test_base, MEM_BUF, len,ticks); 
    SFLASH_DBG("3. Verifing read data\n");
    if(do_memcmp((const void *) MEM_BUF, (const void *) test_base, len)){
        SFLASH_DBG("Read verify passed\n");
        cs_printf("Normal Read  test %4d  KB Data src 0x%x  dest 0x%x len 0x%x ticks: %lld\n", len/0x400,MEM_BUF, test_base, len,ticks);
        }
    else
        cs_printf("Read verify failed\n"); 
    
    /*Block READ*/
        SFLASH_DBG("Block Read test %4d  KB Data Start\n",len/0x400);
        SFLASH_DBG("1. Format memory buf\n");
        memset((void *)MEM_BUF, 0, len); 
        SFLASH_DBG("2. Read from sflash\n");
        ticks = cs_current_time();
        sflash_block_read(MEM_BUF, test_base, len);
        ticks = cs_current_time() - ticks;
        SFLASH_DBG("Block  Read src 0x%x  dest 0x%x len 0x%x ticks: %lld\n", test_base, MEM_BUF, len,ticks);  
        SFLASH_DBG("3. Verifing read data\n");
        if(do_memcmp((const void *) MEM_BUF, (const void *) test_base, len)){
            SFLASH_DBG("Block Read verify passed\n");
            cs_printf("Block  Read  test %4d  KB Data src 0x%x  dest 0x%x len 0x%x ticks: %lld\n", len/0x400,MEM_BUF, test_base, len,ticks);
        }
        else
            cs_printf("Block Read verify failed\n");
    
}


void sflash_test()
{
        cs_uint32 loop=0;
        
    while(sflash_test_len[loop] != 0 ){
        cs_printf("\nTest sflash %4d    KB R/W\n",sflash_test_len[loop]/0x400);
        sflash_read_test(sflash_test_len[loop]);
        sflash_write_test(sflash_test_len[loop]);
        loop++;
    }

}
#endif 

#endif /* HAVE_SERIAL_FLASH */

