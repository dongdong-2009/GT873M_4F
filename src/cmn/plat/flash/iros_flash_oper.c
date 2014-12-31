/*****************************************************************************/
/* This file contains unpublished documentation and software                 */
/* proprietary to Cortina Systems Incorporated. Any use or disclosure,       */
/* in whole or in part, of the information in this file without a            */
/* written consent of an officer of Cortina Systems Incorporated is          */
/* strictly prohibited.                                                      */
/* Copyright (c) 2002 by Cortina Systems Incorporated.                       */
/*****************************************************************************/
/*
 * iros_flash_oper.c
 */


#include "iros_config.h"
#include "cs_types.h"
#include "plat_common.h"
#include <pkgconf/hal.h>
#include <cyg/hal/hal_intr.h>           // interrupt disable/restore
#include "iros_flash_oper.h"
#include "image.h"
#include "iros_flash_partition.h"
#include "flash_dev_driver.h"
#include "gwd_poe.h"
//#define FLASH_DEBUG

FLASH_FAMILY_TYPE_T flash_family_type = FLASH_FAMILY_TYPE_NULL;
flash_dev_t flash_dev;
iros_flash_partition_status_t partition_status;
cs_uint32  iros_flash_mutex=0;

//unsigned int max_blob_size = (1024 * 32 * 15);   /* 480 KB compressed */
//unsigned int flash_block_size = 0x8000; /* 32 KB */


/* flash_driver.c functions */
extern void oper_flash_read_id(void*);
extern int oper_flash_write(void* , cyg_uint16);
extern int oper_flash_erase_block(void*);
extern int oper_flash_force_erase_block(void* block);
extern int oper_flash_lock_block(void*);
extern int oper_flash_unlock_block(void*);
extern int oper_flash_bulk_write(void*, void*, int);
extern int oper_flash_force_write(void* _addr, void* _data, int len);



#if defined(IROSBOOT)
#define IROS_MALLOC_BOOT_POOL_SIZE   1280  /* bytes */
static char iros_malloc_boot_pool[IROS_MALLOC_BOOT_POOL_SIZE];
static int iros_malloc_boot_pool_next_free = 0;
char *iros_malloc_boot(int size) {
    char * return_ptr;
    if ((iros_malloc_boot_pool_next_free + size) >= IROS_MALLOC_BOOT_POOL_SIZE) {
        diag_printf("boot malloc pool too small\n");
        diag_printf("reboot\n");
        hal_delay_us(10000);
        HAL_PLATFORM_RESET();
    }
    return_ptr = &iros_malloc_boot_pool[iros_malloc_boot_pool_next_free];
    iros_malloc_boot_pool_next_free += size;
    return return_ptr;
}
#endif


#ifdef FLASH_DEBUG

void partition_status_dump(iros_flash_partition_status_t *p) {
    int i;
    if (p) {
        diag_printf("partition_status at %p\n", p);
        diag_printf("    work_part_id: %d\n", p->work_part_id);
        diag_printf(" work_part_index: %d\n", p->work_part_index);
        diag_printf("super_block_part_index: %d\n", p->super_block_part_index);
        for (i = 0; i < IROS_FLASH_PARTITION_TAB_MAX; i++) {
            if (flash_dev.info.super_block.part_tab[i].part_id) {
                diag_printf(" part index %d part_id %d part_status 0x%x part_loc_erase %p\n", i,
                            flash_dev.info.super_block.part_tab[i].part_id,
                            p->part_status[i],
                            p->part_loc_erase[i]);
            }
        }
        diag_printf(" part_status_any: 0x%x part_loc_erase_any %p\n",
                    p->part_status_any, p->part_loc_erase_any);
    }
}

void flash_info_dump(flash_info_t *p) {
    int i;
    if (p) {
        diag_printf("flash_info at %p\n", p);
        diag_printf("    flash_id:        0x%08x\n", p->flash_id);
        diag_printf("    super_block_loc: %p\n", p->super_block_loc);
        diag_printf("    super_block\n");
        diag_printf("    super_block.head_sig: 0x%08x\n", p->super_block.head_sig);
        diag_printf("    super_block.flash_id: 0x%08x\n", p->super_block.flash_id);
        diag_printf("    super_block.flash_size: 0x%08x\n", p->super_block.flash_size);
        diag_printf("    super_block.block_size: 0x%08x\n", p->super_block.block_size);
        for (i = 0; i < IROS_FLASH_PARTITION_TAB_MAX; i++) {
            if (p->super_block.part_tab[i].part_id) {
                diag_printf("    index %d  part_id %d part_type %d part_loc %p part_size 0x%08x\n",
                            i,
                            p->super_block.part_tab[i].part_id,
                            p->super_block.part_tab[i].part_type,
                            p->super_block.part_tab[i].part_loc,
                            p->super_block.part_tab[i].part_size);
            }
        }
        diag_printf("    super_block.tail_sig: 0x%08x\n\n", p->super_block.tail_sig);

    }
}

void dump_flash_dev(flash_dev_t *p) {
    diag_printf("flash_dev_t dump at %p\n", p);
    flash_info_dump(&p->info);
#if 0
    diag_printf("   erase 0x%08x\n", p->erase);
    diag_printf("   write 0x%08x\n", p->write);
    diag_printf("   lock 0x%08x\n", p->lock);
    diag_printf("   unlock 0x%08x\n", p->unlock);
#endif
    return;
}

#endif


cs_uint32 flash_part_id_to_part_index(cs_uint32 part_id) {
    cs_uint32 i;
    for (i = 0; i < IROS_FLASH_PARTITION_TAB_MAX; i++) {
        if (flash_dev.info.super_block.part_tab[i].part_id == part_id) {
            return i;
        }
    }
    return IROS_FLASH_PARTITION_INDEX_ANY;
}


void show_flash_partition_help(void) {
    int i;
    diag_printf("  f  p                      - show current partitions\n");

    diag_printf("  f  d id                   - delete partiton id\n");
    diag_printf("  f  c type start size [id] - create a new partion id\n");
    // show type table here : TBD
    for (i = 0; i < IROS_FLASH_PARTITION_TYPE_MAX; i++) {
        diag_printf("       type code: 0x%0x %s\n", i, part_type_to_str[i]);
    }
    diag_printf("  f  a                    - apply partition changes\n");

    return;
}

void flash_partition_table_show(void) {
    int i;

    iros_flash_super_block_t *p;

    p = &flash_dev.info.super_block;

    for (i = 0; i < IROS_FLASH_PARTITION_TAB_MAX; i++) {
        if (p->part_tab[i].part_id) {
            diag_printf("Partition Index 0x%x ID 0x%08x Type 0x%x Start 0x%08x Size 0x%08x %s\n",
                        i,
                        p->part_tab[i].part_id,
                        p->part_tab[i].part_type,
                        (unsigned int) p->part_tab[i].part_loc,
                        p->part_tab[i].part_size,
                        part_type_to_str_short[p->part_tab[i].part_type]);
        }
    }

    return;
}


void flash_partition_delete(cs_uint32 id) {
    cs_uint32 part_index;
    iros_flash_super_block_t *p;
    p = &flash_dev.info.super_block;
    part_index = flash_part_id_to_part_index(id);
    if ((p->part_tab[part_index].part_type == IROS_FLASH_PARTITION_TYPE_SUPER_BLOCK) ||
        (p->part_tab[part_index].part_type == IROS_FLASH_PARTITION_TYPE_BOOT)) {
        diag_printf("Err: not allowed\n");
        return;
    }
    if (part_index != IROS_FLASH_PARTITION_INDEX_ANY) {
        diag_printf("deleted partition index 0x%x id 0x%x\n",
                    part_index,
                    p->part_tab[part_index].part_id);
        p->part_tab[part_index].part_id = 0;
    }
}


void flash_partition_create(cs_uint32 type, char *start, cs_uint32 size, cs_uint32 id) {
    int i;
    iros_flash_super_block_t *p;
    p = &flash_dev.info.super_block;
    switch(type) {
        case IROS_FLASH_PARTITION_TYPE_SUPER_BLOCK:
        case IROS_FLASH_PARTITION_TYPE_BOOT:
        {
            diag_printf("Err: not allowed\n");
            break;
        }
        case IROS_FLASH_PARTITION_TYPE_BLOB_ECOS:
        case IROS_FLASH_PARTITION_TYPE_STARTUP_CONFIG:
        case IROS_FLASH_PARTITION_TYPE_JFFS2:
        case IROS_FLASH_PARTITION_TYPE_DATA:
        case IROS_FLASH_PARTITION_TYPE_XIP:
        {
            for (i = 0; i < IROS_FLASH_PARTITION_TAB_MAX; i++) {
                if (p->part_tab[i].part_id == 0) {
                    // found
                    if (id == 0) {
                        id = (cyg_uint32) (&p->part_tab[i]); // assign a unique id
                    }
                    p->part_tab[i].part_id = id;
                    p->part_tab[i].part_loc = start;
                    p->part_tab[i].part_type = type;
                    p->part_tab[i].part_size = size;
                    diag_printf("created partition index 0x%x id 0x%x\n", i, id);
                    break;
                }
            }
        }
        default:
            break;
    }
}

int
flash_memcmp(unsigned char *m1, unsigned char *m2, int len)
{
    int i;

    for (i=0;i<len;i++) {
        if (m1[i] != m2[i]) {
            return (i + 1);
        }
    }
    return 0;
}

void flash_partition_table_update(void) {
    // if modified, update, otherwise ignore
    if (flash_memcmp((char *)&flash_dev.info.super_block,
                     flash_dev.info.super_block_loc,
                     sizeof(iros_flash_super_block_t)) != 0)
    {
        cs_uint32 part_index = flash_part_id_to_part_index(IROS_FLASH_PARTITION_SUPER_BLOCK_ID);
        if (part_index != IROS_FLASH_PARTITION_INDEX_ANY) {
            int len;
            char *des = flash_dev.info.super_block_loc;
            if (flash_part_write_init(part_index, des)) { /* prepare to write flash */
                len = flash_write(des, (char *)&flash_dev.info.super_block, sizeof(iros_flash_super_block_t));
                flash_part_write_done(part_index);
                flash_write_verify(des, (char *)&flash_dev.info.super_block, sizeof(iros_flash_super_block_t));
                diag_printf("Applied the modified partition table\n");
            }
        }
    } else {
        diag_printf("Partition table not modified.\n");
    }
}


// flash write protection and partition control
// return 1 ok; 0 failed
// start_loc should be aligned to the block size
int flash_part_write_init(cs_uint32 part_index, char * start) {
    char *start_loc;
    int return_status = 0;
    cs_uint32 block_size = flash_dev.info.super_block.block_size;

    cs_mutex_lock(iros_flash_mutex);
    start_loc = (char *)(((unsigned int)start) & (~(block_size - 1))); // aligned to the block size;
    if (part_index == IROS_FLASH_PARTITION_INDEX_ANY) {
        partition_status.work_part_id = IROS_FLASH_PARTITION_ID_ANY;
        partition_status.work_part_index = IROS_FLASH_PARTITION_INDEX_ANY;
        partition_status.part_status_any = IROS_FLASH_PARTITION_WRITE;
        partition_status.part_loc_erase_any = start_loc;
        return_status = 1;
    } else {
        if ((part_index >= 0) && (part_index < IROS_FLASH_PARTITION_TAB_MAX)) {
            if (flash_dev.info.super_block.part_tab[part_index].part_id) {
                partition_status.work_part_id = flash_dev.info.super_block.part_tab[part_index].part_id;
                partition_status.work_part_index = part_index;
                partition_status.part_status[part_index] = IROS_FLASH_PARTITION_WRITE;
                partition_status.part_loc_erase[part_index] = start_loc;
                // check the loc is within the partition
                if ((start - start_loc) < flash_dev.info.super_block.part_tab[part_index].part_size) {
                    return_status = 1; // ok
                }
            }
        }
    }
    if (return_status == 0) {
        cs_mutex_unlock(iros_flash_mutex);
        diag_printf("Not ready to write flash at 0x%08x\n",(unsigned int) start_loc);
    }
    return return_status; // error
}

// flash partition write sequence done
int flash_part_write_done(cs_uint32 part_index) {
    int return_status = 0;
    if (part_index == IROS_FLASH_PARTITION_INDEX_ANY) {
        partition_status.work_part_id = 0;
        partition_status.work_part_index = 0;
        partition_status.part_status_any = 0;
        partition_status.part_loc_erase_any = 0;
        return_status = 1;
    } else {
        if ((part_index >= 0) && (part_index < IROS_FLASH_PARTITION_TAB_MAX)) {
            if (flash_dev.info.super_block.part_tab[part_index].part_id) {
                partition_status.work_part_id = 0;
                partition_status.work_part_index = 0;
                partition_status.part_status[part_index] = 0;
                partition_status.part_loc_erase[part_index] = 0;
                return_status = 1;
            }
        }
    }
    cs_mutex_unlock(iros_flash_mutex);
    
    return return_status;
}

bool flash_write_verify(char *faddr, char *p, int len) {
    int i;
    // verify char by char
    for (i = 0; i < len; i++) {
        if (faddr[i] != p[i]) {
            diag_printf("Verifying failed at 0x%x\n", i);
            return false;
        }
    }
    //diag_printf("Verifying passed\n");
    return true;
}


// no return;
void platform_reset(void) {
    diag_printf("reboot\n");
    hal_delay_us(10000);
    HAL_PLATFORM_RESET();
}

void upgrade_confirm(current_blob_desc_t *p_blob_desc) {
        blob_info_desc_t *blob_info;
        unsigned int short is_not_confirmed;
        unsigned int short is_not_executed;
        if (p_blob_desc == 0) {
                p_blob_desc = &current_blob_desc;
        }
        blob_info = blob_info_tab[p_blob_desc->blob_idx];
        is_not_confirmed = 0;
        is_not_executed = 0;
        flash_write((char *)&blob_info->is_not_confirmed, (char *)&is_not_confirmed, sizeof(unsigned int short));
        flash_write((char *)&blob_info->is_not_executed, (char *)&is_not_executed, sizeof(unsigned int short));
};


void flash_unlock_block_main(cs_uint32 faddr) {
    int return_status = 0;

    /* Unlock flash block */
    return_status = oper_flash_unlock_block((void*)faddr);

    if (return_status != 0) {
        diag_printf("Unlock failed! Bad flash\n");
    }

    return;
}

void flash_erase_block_main(cs_uint32 faddr) {
    int return_status = 0;

    /* Unlock flash block */
    return_status = oper_flash_unlock_block((void*)faddr);

    if ( return_status != 0 ) {
        diag_printf("Unlock failed. Bad flash!\n");
    }

    /* Erase the block */
    return_status = oper_flash_erase_block((void*)faddr);

    if ( return_status != 0 ) {
        diag_printf("Erase failed. Bad flash!\n");
    }

}

int flash_write_half_word_without_erase(cs_uint32 faddr, unsigned short int val) {

    int return_status = 0;
    return_status = oper_flash_write((void *)faddr, (cs_uint16)val);

    if (return_status != 0) {
        diag_printf("Write failed. Bad flash! addr 0x%08x 0x%04x\n", faddr, val);
        return 1;
    }
    return 0;
}

void *memcpy(void *dest, const void *src, size_t n);

void flash_writer_setup(void)
{
    int i;
    bool found;
    static bool flash_writer_setup_init = false;

    /*
     * flash_writer_setup is called at two places one in main.c
     * and other in flashiodev.c (for jffs2 supported branch).
     * Since the init flag will help keep track calling this once.
     */
    if (flash_writer_setup_init) return;
    flash_writer_setup_init = true;

    //cs_mutex_init(&iros_flash_mutex, "Flashmutex", 0);
//#ifndef HAVE_FLASH_FS
    int ret;
    ret = flash_hwr_init();
    if(ret != FLASH_ERR_OK)
    {
        diag_printf("flash_hwr_init failed: 0x%02x\n", ret);
    }
//#endif

    memset(&flash_dev, 0, sizeof(flash_dev_t));
    unsigned int id;

    /* Read flash id */
    oper_flash_read_id(&id);
    diag_printf("flash id: 0x%08x\n", id);

    // search the supported flash info table by flash id
    found = false;
    i = 0;
    while(flash_info_tb[i].flash_id) {
        if (flash_info_tb[i].flash_id == id) {
            // found;
            found = true;
            break;
        }
        i++;
    }
    if (!found) {
        // other flash type detection - those not compatible with intel
        // such Spansion flash - TBD
        diag_printf("flash type not supported 0x%08x\n", id);
        return;
    }

#ifdef FLASH_DEBUG
    flash_info_dump(&flash_info_tb[i]);
#endif

    // prepare current flash_dev
    flash_dev.info.flash_id = id;
    flash_dev.info.super_block_loc = flash_info_tb[i].super_block_loc;

    iros_flash_super_block_t * super_block;
    super_block = (iros_flash_super_block_t *)flash_dev.info.super_block_loc;

    if ((super_block->head_sig == IROS_FLASH_SUPER_BLOCK_HEAD_SIG) &&
        (super_block->tail_sig == IROS_FLASH_SUPER_BLOCK_TAIL_SIG)) {
        // use the user defined super block
        flash_dev.info.super_block = *super_block;
    } else {
        // use the default super block
        flash_dev.info.super_block = flash_info_tb[i].super_block;
    }

    if (flash_dev.info.flash_id != flash_dev.info.super_block.flash_id) {
        diag_printf("real flash_id 0x%08x work flash_id 0x%08x\n",
                    flash_dev.info.flash_id,
                    flash_dev.info.super_block.flash_id);
        flash_dev.info.flash_id = flash_dev.info.super_block.flash_id;
    }


#ifdef FLASH_DEBUG
    dump_flash_dev(&flash_dev);
#endif

    // prepare blob_info_tab
    // the first blob partition is blob0; the second is blob1
    int blob_idx = 0;
    for (i = 0 ; i < IROS_FLASH_PARTITION_TAB_MAX; i++) {
        if (flash_dev.info.super_block.part_tab[i].part_id &&
            (flash_dev.info.super_block.part_tab[i].part_type == IROS_FLASH_PARTITION_TYPE_BLOB_ECOS)) {
            blob_part_tab[blob_idx] = i;
            blob_info_tab[blob_idx] = (blob_info_desc_t *)flash_dev.info.super_block.part_tab[i].part_loc;
            blob_idx++;
            if (blob_idx == 2) {
                break;
            }
        }
    }
    if (blob_idx != 2) {
        diag_printf("partition table config err\n");
        return;
    }

#ifdef FLASH_DEBUG
    diag_printf("blob 0 at 0x%08x\n",(unsigned int) blob_info_tab[0]);
    diag_printf("blob 1 at 0x%08x\n",(unsigned int) blob_info_tab[1]);
#endif

    // prepare current_blob_desc
    memcpy((char *)&current_blob_desc, (char *)CURRENT_BLOB_DESC_BASEADDR, sizeof(current_blob_desc_t));

#ifdef FLASH_DEBUG
    diag_printf("cur blob desc: %x %x %x %x\n", current_blob_desc.signature,
                                     current_blob_desc.update_timeout,
                                     current_blob_desc.blob_idx,
                                     current_blob_desc.num_retries);
#endif


    //initialize partiton status
    memset(&partition_status, 0, sizeof(iros_flash_partition_status_t));

    // set super block partition index
    for (i = 0 ; i < IROS_FLASH_PARTITION_TAB_MAX; i++) {
        if (flash_dev.info.super_block.part_tab[i].part_id &&
            (flash_dev.info.super_block.part_tab[i].part_type == IROS_FLASH_PARTITION_TYPE_SUPER_BLOCK)) {
            partition_status.super_block_part_index = i;
                break;
        }
    }

#ifdef FLASH_DEBUG
    partition_status_dump(&partition_status);
#endif

    IROS_DPRINTF("flash_writer_setup done\n");

    return;
}

int flash_write_check(char * faddr, char *p, int len, int check, int no_erase);

int flash_write_no_erase(char * faddr, char *p, int len) {
    return flash_write_check(faddr, p, len, 1, 1);
}

int flash_write_core_force(char * faddr, char *p, int len) 
{
    cs_uint32 part_index = flash_part_id_to_part_index(IROS_FLASH_PARTITION_COREDUMP_ID);
    if(flash_dev.info.super_block.part_tab[part_index].part_loc==faddr)
        oper_flash_force_erase_block((void *)faddr);
   return oper_flash_force_write((void *)faddr, (void *)p, len); 
       
}

int flash_write(char * faddr, char *p, int len) {
    return flash_write_check(faddr, p, len, 1, 0);
}

int flash_write_nocheck(char * faddr, char *p, int len) {
    return flash_write_check(faddr, p, len, 0, 0);
}

// when connecting to uart xyzmodem, you should not print any debugging lines inside this fun
int flash_write_check(char * faddr, char *p, int len, int check, int no_erase) {
#if 0
        int return_len;
#endif

#ifdef FLASH_DEBUG
        diag_printf("flash_write start ... to %p len 0x%08x %d\n", faddr, len, no_erase);
#endif

        if (partition_status.work_part_id == 0) {
            diag_printf("flash_write: partition is not specified\n");
            return 0;
        }

        char *new_next_erase_loc = 0;
        char *old_part_loc_erase = 0;

        iros_flash_partition_t *part = 0;
        cs_uint32 block_size = flash_dev.info.super_block.block_size;

        if (partition_status.work_part_id != IROS_FLASH_PARTITION_ID_ANY) {
            cs_uint32 index;
            index = partition_status.work_part_index;
            part = &flash_dev.info.super_block.part_tab[index];
            if ( (partition_status.part_status[index] != IROS_FLASH_PARTITION_WRITE) ||
                !((faddr >= part->part_loc) &&
                  ((faddr + len) <= (part->part_loc + part->part_size)))) {
                diag_printf("flash_write not ready or out of partition id %d range 0x%08x len 0x%08x\n",
                            partition_status.work_part_id, (unsigned int)faddr, len);
                return 0;
            }

            old_part_loc_erase = partition_status.part_loc_erase[index];
            if (old_part_loc_erase < (faddr + len)) {

                // round to next block
                new_next_erase_loc = (char *) ((((unsigned int)faddr + len) + block_size - 1) & (~(block_size - 1)));
                partition_status.part_loc_erase[index] = new_next_erase_loc;
            }

        } else {
            // IROS_FLASH_PARTITION_ID_ANY
            if ((partition_status.part_status_any != IROS_FLASH_PARTITION_WRITE) ||
                !((faddr >= (char *)FLASH_BASE) &&
                  ((faddr + len) <= (flash_dev.info.super_block.flash_size + (char *)FLASH_BASE)))) {
                diag_printf("flash_write not ready or out of range 0x%08x len 0x%08x\n",
                            (unsigned int) faddr, len);
                return 0;
            }
            old_part_loc_erase = partition_status.part_loc_erase_any;
            if (old_part_loc_erase < (faddr + len)) {
                // round to next block
                new_next_erase_loc = (char *)((((unsigned int)faddr + len) + block_size - 1) & (~(block_size - 1)));
                partition_status.part_loc_erase_any = new_next_erase_loc;
            }

        }
        // erase blocks as needed
        if (new_next_erase_loc) {
#ifdef FLASH_DEBUG
            diag_printf("erase blocks from %p to %p\n", old_part_loc_erase, new_next_erase_loc);
#endif
            char *p = old_part_loc_erase;
            for (p = old_part_loc_erase; p < new_next_erase_loc; p += block_size) {
                if (no_erase != 0) {
                    oper_flash_unlock_block((void *)p);
                } else {
                    oper_flash_erase_block((void *)p);
                    oper_flash_unlock_block((void *)p);
                }
            }
        }

        // len+1 make sure the odd number of bytes will be written.
        // odd number len must be the last flash_write to write a file
#if 0
        cyg_uint32 len_half_words = ((len+1) >> 1);
        int i;
#endif
        /* write to flash in bulk */
        oper_flash_bulk_write((void *)faddr, (void *)p, len);
#if 0
        return_len = (len_half_words << 1);
        for (i = 0; i < len_half_words; i++) {
            if (flash_write_half_word_without_erase( (cyg_uint32) faddr + (i<<1), ((unsigned short int *)p)[i]) != 0) {
                return_len = (i << 1);
                break;
            }
        }
#endif

        return len;
}


// ONU app update

#if defined(CYG_HAL_ARM_IMST_ONU)
#if !defined(IROSBOOT)
bool is_in_update = false;

// return 0 if error
// return 1 if ok
int app_update_from_sc_imp(char * buffer, int len, int timeout /*seconds*/) {
        IROS_DPRINTF("app_update_from_sc_imp 0x%08x 0x%x 0x%x\n", buffer, len, timeout);
        return 1;
}


bool is_file_transfer_in_progress = false;

extern unsigned char updateInProgress;
extern unsigned int active_update_port;
extern unsigned int active_mgmt_port;
// return 0 if error
// return 1 if ok
// this is for test only
int app_update_from_sc(int timeout) {
        IROS_DPRINTF("app_update_from_sc\n");
        int result;
        if (updateInProgress) {
                // reboot
                diag_printf("duplicated update ignored\n");
                result = 0;
        } else {
                extern int app_update_init(int timeout);
                extern unsigned char updateInProgress;
                updateInProgress = 1;
                result = app_update_init(timeout);
                /*is_file_transfer_in_progress = true;*/
                active_update_port = active_mgmt_port;
        }
        return result;
}


bool app_update_in_progress(void) {
    int cur_blob_idx = current_blob_desc.blob_idx;
    if (blob_info_tab[cur_blob_idx]->is_not_confirmed) {
        return true;
    }
    return false;
}


// return 0 if error
// return 1 if ok
int app_update_init(int timeout) {
        IROS_DPRINTF("app_update_init\n");

#if 0
        extern int prepare_mif_upgrade (char module,bool need_confirm_from_server);
        if (prepare_mif_upgrade (1, true) != 0)
            return 0;
#endif
        current_blob_desc.update_timeout = timeout;
        extern int app_update_file_transfer_init();
        app_update_file_transfer_init();
        return 1;
}




bool is_ethload(void) {
    int image_download_direction;
    HAL_READ_UINT32(IMST_IMAGE_DOWNLOAD, image_download_direction);
    image_download_direction &= IMAGE_DOWNLOAD_BITS;
    if (image_download_direction == IMAGE_FROM_ETH) {
        return true;
    }
    return false;

}

// return 0 if error
// return 1 if ok
int app_update_continue_after_reboot(void) {
        blob_info_desc_t *blob_info;
        //unsigned int short is_not_executed;

        IROS_DPRINTF("app_update_commit\n");



        // if it is in upgrade stage, turn off is_not_executed flag
        int cur_blob_idx = current_blob_desc.blob_idx;
        blob_info = blob_info_tab[cur_blob_idx];
        if (blob_info->is_not_confirmed) {
                // we are in update stage
                is_in_update = true;
        }

        return 1;
}


// return 0 if error
// return 1 if ok
int app_update_commit(void) {
        diag_printf("app_update_commit\n");


        blob_info_desc_t *blob_info;
        unsigned int short is_not_confirmed;
     unsigned int short  is_not_executed;

        IROS_DPRINTF("app_update_commit\n");

        is_in_update = false; // turn off update timeout timer
        current_blob_desc.update_timeout = 0;

        // if it is in upgrade stage, turn off is_not_confirmed flag
        int cur_blob_idx = current_blob_desc.blob_idx;
        blob_info = blob_info_tab[cur_blob_idx];

        diag_printf("blob_info 0x%08x\n",(unsigned int) blob_info);
        if (blob_info_tab[cur_blob_idx]->is_not_confirmed != 0) {
                // turn off the flag
            //diag_printf("is not confirmed\n");

                is_not_confirmed = 0;

                int part_index = blob_part_tab[cur_blob_idx];
                //int max_blob_size = flash_dev.info.super_block.part_tab[part_index].part_size;

                if (flash_part_write_init(part_index, (char*)blob_info)) { /* prepare to write flash without erase */
                    flash_write_no_erase((char *)&blob_info->is_not_confirmed, (char *)&is_not_confirmed, sizeof(unsigned int short));

                    //diag_printf("idx %x is_not_confirmed %x timeout 0x%x\n", cur_blob_idx, is_not_confirmed, current_blob_desc.update_timeout);
                    diag_printf("cur blob committed\n");


                    if (blob_info->is_not_executed) {
                        // turn off the flag
                        is_not_executed = 0;
                        flash_write_no_erase((char *)&blob_info->is_not_executed, (char *)&is_not_executed, sizeof(unsigned int short));
                        diag_printf("cur blob executed\n");
                    }

                    flash_part_write_done(part_index);

                    return 1;
                } else {
                    diag_printf("cur blob commit failed\n");
                    is_in_update = false;
                    return 0;
                }

        }
        return 0;
}

#endif
#endif

int  save_userdata_to_flash (char * src, unsigned int flash_offset,unsigned int data_lenth)
{
     char *FlashStartAddr;
     void *temp_ptr=0;
     int   UserdataMaxLen=0;

     cs_uint32 part_index = flash_part_id_to_part_index(IROS_FLASH_PARTITION_DATA_ID);
     if (part_index == IROS_FLASH_PARTITION_INDEX_ANY) {
         diag_printf("\nFlash user data partition not found\n");
         return 1;
     }
     else {
         FlashStartAddr = flash_dev.info.super_block.part_tab[part_index].part_loc;
         UserdataMaxLen = flash_dev.info.super_block.part_tab[part_index].part_size;
    }
    if( (flash_offset+data_lenth) > USER_DATA_USED_LEN ){
        diag_printf("data overflow;max size is %d\n",UserdataMaxLen);
         return 1;
    }

    temp_ptr=iros_malloc(IROS_MID_SYS, USER_DATA_USED_LEN );
     if(!temp_ptr){
        diag_printf("save_userdate_to_flash malloc failed\n");
         return 1;
     }

     memcpy(temp_ptr,FlashStartAddr,USER_DATA_USED_LEN);
     memcpy((temp_ptr+flash_offset), src, data_lenth);

     if (flash_part_write_init(part_index, FlashStartAddr)) { /* prepare to write flash */

         flash_write(FlashStartAddr, temp_ptr, USER_DATA_USED_LEN);
         flash_part_write_done(part_index);
     }
     iros_free(temp_ptr);
     return 0;

}

int  get_userdata_from_flash (char * dest, unsigned int flash_offset,unsigned int data_lenth)
{
     char *FlashStartAddr;
     int   UserdataMaxLen=0;
     cs_uint32 part_index = flash_part_id_to_part_index(IROS_FLASH_PARTITION_DATA_ID);
     if (part_index == IROS_FLASH_PARTITION_INDEX_ANY) {
       diag_printf("\nFlash user data partition not found\n");
       return 1;
     }
     else {
       FlashStartAddr = flash_dev.info.super_block.part_tab[part_index].part_loc;
       UserdataMaxLen = flash_dev.info.super_block.part_tab[part_index].part_size;
     }
     if( (flash_offset+data_lenth) > USER_DATA_USED_LEN ){
      diag_printf("data overflow;USER_DATA_USED_LEN 0x%x\n",USER_DATA_USED_LEN);
       return 1;
     }

     memcpy((char *)dest, (void *)(FlashStartAddr+flash_offset),data_lenth);
     return 0;

}

#if 1
#define USER_DATA_LEN	0x4000
typedef unsigned int uint32;
typedef struct
{
	uint32 head_len;
	uint32 tlv_area_len;
}tlv_area_head_t;

//需调用的外部接口
extern int serial_port_tlv_infor_get(int *len, char **data, int *state_free);
extern int serial_port_tlv_infor_handle(int len, char *data, int opcode);
extern int serial_port_running_config_show(void);

extern int broadcast_storm_threshold_tlv_infor_get(int *len, char **value, int *free_need);
extern int broadcast_storm_threshold_tlv_infor_handle(int len, char *data, int opcode);
extern int broadcast_storm_threshold_running_config_show(void);

extern int igmp_mode_tlv_infor_get(int *len, char **value, int *free_need);
extern int igmp_mode_tlv_infor_handle(int len, char *data, int opcode);
extern int igmp_mode_running_config_show(void);

#if (GE_RATE_LIMIT == MODULE_YES)
extern int uni_egress_rate_tlv_infor_get(int *len, char **value, int *free_need);
extern int uni_egress_rate_tlv_infor_handle(int len, char *data, int opcode);
extern int uni_egress_rate_running_config_show(void);

extern int uni_ingress_rate_tlv_infor_handle(int len, char *data, int opcode);
extern int uni_ingress_rate_tlv_infor_get(int *len, char **value, int *free_need);
extern int uni_ingress_rate_running_config_show(void);
#endif

#if (PORT_ISOLATE_MODE_SAVE == MODULE_YES)
extern int port_isolate_mode_tlv_infor_get(int *len, char **value, int *free_need);
extern int port_isolate_mode_tlv_infor_handle(int len, char *data, int opcode);
extern int port_isolate_mode_running_config_show(void);
#endif

#if (QINQ_SUPPORT == MODULE_YES)
extern int vlan_qinq_table_tlv_infor_get(int *len, char **value, int *free_need);
extern int vlan_qinq_table_tlv_infor_handle(int len, char *data, int opcode);
extern int vlan_qinq_running_config_show(void);
#endif



//向外提供的接口
extern int save_user_tlv_data_to_flash(void);
extern int get_user_tlv_data_from_flash(int opcode);
extern int start_up_config_erase(void);
extern int running_config_show(void);
extern int start_up_config_syn_to_running_config(void);
extern int start_up_show(void);

//内部接口


//change name
//NULL panduan

#define TLV_BUF_LEN		432
extern int fdb_static_list_tlv_info_get(int *len, char **value, int *free_need);
extern int save_user_tlv_data_to_flash(void)
{
	#if 1
	int ret = 0;
	const uint32 tlv_area_volume = USER_DATA_LEN;
	const uint32 user_data_tlv_offset = TLV_OFFSET;
	tlv_area_head_t tlv_area_head;
	uint32 tlv_area_saved_len = 0;
	char *buf = NULL;
	uint32 buf_len = 0;
	uint32 buf_valid_len = 0;
	uint32 type = 0;
	uint32 len = 0;
	char *value = NULL;
	int free_need = 0;
	char *p = NULL;
	uint32 single_tlv_len = 0;
	
	//buf_len = USER_DATA_LEN;
	buf_len = TLV_BUF_LEN;
	buf_valid_len = 0;
	buf = (char *)iros_malloc(IROS_MID_SYS, buf_len);
	memset(buf, 0, buf_len);

	tlv_area_head.head_len = sizeof(tlv_area_head);
	for(type=TYPE_START;type<TYPE_NUM;type++)		//遍历每个tlv
	{
		switch(type)
		{
		#ifdef HAVE_TERMINAL_SERVER
			case SERIAL_PORT:
				serial_port_tlv_infor_get(&len, &value, &free_need);
				break;
		#endif
			case BROADCAST_STORM_THRESHOLD:
				broadcast_storm_threshold_tlv_infor_get(&len, &value, &free_need);
				break;
			case IGMP_MODE:
				igmp_mode_tlv_infor_get(&len, &value, &free_need);
				break;
		#if (GE_RATE_LIMIT == MODULE_YES)
			case UNI_EGRESS:
				uni_egress_rate_tlv_infor_get(&len, &value, &free_need);
				break;
			case UNI_INGRESS:
				uni_ingress_rate_tlv_infor_get(&len, &value, &free_need);
				break;
		#endif
		#if (PORT_ISOLATE_MODE_SAVE == MODULE_YES)
			case PORT_ISOLATE_MODE:
				port_isolate_mode_tlv_infor_get(&len, &value, &free_need);
				break;
		#endif
		#if (QINQ_SUPPORT == MODULE_YES)
			case VLAN_QINQ:
				vlan_qinq_table_tlv_infor_get(&len, &value, &free_need);
				break;
		#endif
		#if (RPU_MODULE_POE == MODULE_YES)
			case POE_MODULE:
				gwd_onu_tlv_infor_get(&len, &value, &free_need);
				break;
		#endif
			case STATIC_MAC:
				fdb_static_list_tlv_info_get(&len,&value,&free_need);
				break;
			default:
				len = 0;
				value = NULL;
				free_need =0;
				break;
		}
		#if 0
		cs_printf("type :0x%x, len :0x%x\n", type, len);
		#endif
		if(0 != len)
		{
			//do nothing
		}
		else
		{
			continue;
		}
		single_tlv_len = sizeof(type) + sizeof(len) + len;
		#if 0
		cs_printf("buf_len :%d\n", buf_len);
		cs_printf("single_tlv_len :%d\n", single_tlv_len);
		#endif
		if(single_tlv_len > buf_len)
		{
			cs_printf("error: in %s, line :%d, single_tlv_len > buf_len\n", __func__, __LINE__);
		}
		if(tlv_area_volume >= tlv_area_head.head_len + tlv_area_saved_len + single_tlv_len)
		{
			//do nothing
		}
		else
		{
			//tishi
			cs_printf("type :%d and the rest type is not saved to flash, tlv area is full!");
			break;
		}
		
		if(buf_len - buf_valid_len >= single_tlv_len)
		{
			//do nothing
		}
		else
		{
			ret = save_userdata_to_flash(buf, user_data_tlv_offset + tlv_area_head.head_len + tlv_area_saved_len, buf_valid_len);	//保存tlv 数据区 
			if(0 == ret)
			{
				//do nothing
			}
			else
			{
				iros_free(buf);
				buf = NULL;
				iros_free(value);
				value = NULL;
				goto end;
			}
			tlv_area_saved_len = tlv_area_saved_len + buf_valid_len;
			memset(buf, 0, buf_len);
			buf_valid_len = 0;
		}
		
		if(buf_len - buf_valid_len >= single_tlv_len)
		{
			#if 0
			cs_printf("buf_valid_len :%d\n", buf_valid_len);
			#endif
			p = buf + buf_valid_len;
			memcpy(p, (char *)&type, sizeof(type));
			p = p + sizeof(type);
			memcpy(p, (char *)&len, sizeof(len));
			p = p + sizeof(len);
			memcpy(p, (char *)value, len);
			p = p + len;
			buf_valid_len = buf_valid_len + single_tlv_len;
			
			if(free_need != 0)
			{
				iros_free(value);
				value = NULL;
			}
			else
			{
				//do nothing
			}
		}
		else
		{
			if(free_need != 0)
			{
				iros_free(value);
				value = NULL;
			}
			else
			{
				//do nothing
			}
			//tishi
			break;
		}
		
		
	}

	ret = save_userdata_to_flash(buf, user_data_tlv_offset + tlv_area_head.head_len + tlv_area_saved_len, buf_valid_len);		//保存tlv 数据区
	if(0 == ret)
	{
		//do nothing
	}
	else
	{
		iros_free(buf);
		buf = NULL;
		goto end;
	}
	tlv_area_saved_len = tlv_area_saved_len + buf_valid_len;
	memset(buf, 0, buf_len);
	buf_valid_len = 0;

	tlv_area_head.tlv_area_len = tlv_area_saved_len;
	memcpy(buf, &tlv_area_head, sizeof(tlv_area_head_t));
	buf_valid_len = sizeof(tlv_area_head_t);
	ret = save_userdata_to_flash(buf, user_data_tlv_offset, buf_valid_len);	//保存tlv 区 头	
	if(0 == ret)
	{
		//do nothing
	}
	else
	{
		iros_free(buf);
		buf = NULL;
		goto end;
	}
	iros_free(buf);
	buf = NULL;
	
end:	
	return ret;
	#endif
}

extern int fdb_static_list_tlv_info_handle(int length,char *value,int opcode);
extern int get_user_tlv_data_from_flash(int opcode)
{
	#if 0
	cs_printf("in %s\n", __func__);
	#endif
	int ret = 0;
	const uint32 user_data_tlv_offset = TLV_OFFSET;
	tlv_area_head_t tlv_area_head;
	uint32 tlv_area_len = 0;
	char *buf = NULL;
	uint32 buf_len = 0;
	uint32 buf_valid_len = 0;
	uint32 buf_parsed_len = 0;
	uint32 type = 0;
	uint32 len = 0;
	char *value = NULL;
	uint32 tlv_parsed_len = 0;
	uint32 single_tlv_len = 0;
	tlv_parsed_len = 0;
	
	ret = get_userdata_from_flash((char *)&tlv_area_head, user_data_tlv_offset, sizeof(tlv_area_head_t));		//获得tlv 头
	if(0 == ret)
	{
		//do nothing
	}
	else
	{
		goto end;
	}
	#if 0
	cs_printf("tlv_area_head.tlv_area_len :0x%x\n", tlv_area_head.tlv_area_len);
	#endif
	tlv_area_len = tlv_area_head.tlv_area_len;
	buf_len = TLV_BUF_LEN;
	if(0 == buf_len)
	{
		goto end;
	}
	else
	{
		
	}
	buf = (char *)iros_malloc(IROS_MID_SYS,buf_len);
	if(NULL != buf)
	{
		//do nothing
	}
	else
	{
		cs_printf("in %s, malloc error!\n", __func__);
		goto end;
	}
	buf_parsed_len = 0;
	memset(buf, 0, buf_len);
	if(buf_len <= tlv_area_len)
	{
		buf_valid_len = buf_len;
	}
	else
	{
		buf_valid_len = tlv_area_len;
	}
	ret = get_userdata_from_flash(buf, user_data_tlv_offset + tlv_area_head.head_len + tlv_parsed_len, buf_valid_len);		//获得tlv 数据区
	if(0 == ret)
	{
		//do nothing
	}
	else
	{
		iros_free(buf);
		goto end;
	}
	#if 0
	cs_printf("tlv_area_len :0x%x\n", tlv_area_len);
	cs_printf("buf_valid_len :0x%x\n", buf_valid_len);
	cs_printf("tlv_area_head.head_len :0x%x\n", tlv_area_head.head_len);
	cs_printf("tlv_parsed_len :0x%x\n", tlv_parsed_len);
	#endif
	char *p = NULL;
	while(tlv_parsed_len < tlv_area_len)
	{
		p = buf + buf_parsed_len;
		if(buf_valid_len - buf_parsed_len >= sizeof(type) + sizeof(len))
		{
			memcpy((char *)&type, p, sizeof(type));
			p = p + sizeof(type);
			memcpy((char *)&len, p, sizeof(len));
			p = p + sizeof(len);
			#if 0
			cs_printf("type :0x%x\n", type);
			cs_printf("len :0x%x\n", len);
			#endif
			if(buf_valid_len - buf_parsed_len >= sizeof(type) + sizeof(len) + len)
			{
				value = p;
				switch(type)
				{
				#ifdef HAVE_TERMINAL_SERVER
					case SERIAL_PORT:
						serial_port_tlv_infor_handle(len, value, opcode);
						break;
				#endif
					case BROADCAST_STORM_THRESHOLD:
						broadcast_storm_threshold_tlv_infor_handle(len, value, opcode);
						break;
					case IGMP_MODE:
						igmp_mode_tlv_infor_handle(len, value, opcode);
						break;
				#if (GE_RATE_LIMIT == MODULE_YES)
					case UNI_EGRESS:
						uni_egress_rate_tlv_infor_handle(len, value, opcode);
						break;
					case UNI_INGRESS:
						uni_ingress_rate_tlv_infor_handle(len, value, opcode);
						break;
				#endif
				#if (PORT_ISOLATE_MODE_SAVE == MODULE_YES)
					case PORT_ISOLATE_MODE:
						port_isolate_mode_tlv_infor_handle(len, value, opcode);
						break;
				#endif
				#if (QINQ_SUPPORT == MODULE_YES)
					case VLAN_QINQ:
						vlan_qinq_table_tlv_infor_handle(len, value, opcode);
						break;
				#endif
				#if (RPU_MODULE_POE == MODULE_YES)
					case POE_MODULE:
						gwd_onu_tlv_infor_handle(len, value, opcode);
						break;
				#endif
					case STATIC_MAC:
						fdb_static_list_tlv_info_handle(len,value,opcode);
						break;
					default:
						break;
				}
				p = p + len;
				single_tlv_len = sizeof(type) + sizeof(len) + len;
				buf_parsed_len = buf_parsed_len + single_tlv_len;
				tlv_parsed_len = tlv_parsed_len + single_tlv_len;
			}
			else
			{
				if(tlv_area_len - tlv_parsed_len > sizeof(type) + sizeof(len) + len)
				{
					if(buf_len >= sizeof(type) + sizeof(len) + len)
					{
						buf_parsed_len = 0;
						memset(buf, 0, buf_len);
						ret = get_userdata_from_flash(buf, user_data_tlv_offset + tlv_area_head.head_len + tlv_parsed_len, buf_len);		//获得tlv 数据区
						if(0 == ret)
						{
							//do nothing
						}
						else
						{
							iros_free(buf);
							goto end;
						}
					}
					else
					{
						break;
					}
					
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			buf_parsed_len = 0;
			memset(buf, 0, buf_len);
			ret = get_userdata_from_flash(buf, user_data_tlv_offset + tlv_area_head.head_len + tlv_parsed_len, buf_len);		//获得tlv 数据区
			if(0 == ret)
			{
				//do nothing
			}
			else
			{
				iros_free(buf);
				goto end;
			}
		
		}
	}
	
end:
	return ret;
}


extern int start_up_config_erase(void)
{
	#if 1
	int ret = 0;
	const uint32 user_data_tlv_offset = TLV_OFFSET;
	tlv_area_head_t tlv_area_head;
	tlv_area_head.head_len = sizeof(tlv_area_head_t);
	tlv_area_head.tlv_area_len = 0;

	ret = save_userdata_to_flash((char *)&tlv_area_head, user_data_tlv_offset, sizeof(tlv_area_head));	//保存tlv 区 头
	return ret;
	#endif

}


extern int serial_port_module_default_config_recover();
extern int broadcast_storm_module_default_config_recover();

#if 0
extern int default_config_recover(void)
{
	#if 0
	cs_printf("in %s\n", __func__);
	#endif
	int ret = 0;
	int type = 0;
	for(type=TYPE_START;type<TYPE_NUM;type++)		//遍历每个tlv
	{
		switch(type)
		{
			case SERIAL_PORT:
				serial_port_module_default_config_recover();
				break;
			case BROADCAST_STORM_THRESHOLD:
				broadcast_storm_module_default_config_recover();
				break;
			default:
				break;
		}
	}
	return ret;
}
#endif

extern int start_up_config_syn_to_running_config(void)
{
	int ret = 0;

	get_user_tlv_data_from_flash(DATA_RECOVER);
	cs_printf("recover start-up config...\n");

	return ret;
}

extern int start_up_show(void)
{
	int ret = 0;

	ret = get_user_tlv_data_from_flash(DATA_SHOW);

	return ret;
}

extern int running_config_show(void)
{
	int type = 0;
	for(type=0;type<TYPE_NUM;type++)
	{
		switch(type)
		{
		#ifdef HAVE_TERMINAL_SERVER
			case SERIAL_PORT:
				serial_port_running_config_show();
				break;
		#endif
			case BROADCAST_STORM_THRESHOLD:
				broadcast_storm_threshold_running_config_show();
				break;
			case IGMP_MODE:
				igmp_mode_running_config_show();
				break;
		#if (GE_RATE_LIMIT == MODULE_YES)
			case UNI_EGRESS:
				uni_egress_rate_running_config_show();
				break;
			case UNI_INGRESS:
				uni_ingress_rate_running_config_show();
				break;
		#endif
		#if (PORT_ISOLATE_MODE_SAVE == MODULE_YES)
			case PORT_ISOLATE_MODE:
				port_isolate_mode_running_config_show();
				break;
		#endif
		#if (QINQ_SUPPORT == MODULE_YES)
			case VLAN_QINQ:
				vlan_qinq_running_config_show();
				break;
		#endif
			default:
				break;
		}
	}
	return 0;
}



#endif

int iros_flash_write(cs_int8* flash_addr, cs_int8* buf, cs_int32 len)
{
    cs_int32 real_len;
    real_len = flash_write(flash_addr, buf, len);
    if(real_len != len)
        return CS_ERROR;

    return CS_OK;
}

void iros_flash_read(cs_int8* flash_addr, cs_int8* buf, cs_int32 len)
{
    memcpy(buf, flash_addr, len);
    return;
}


/* Read data from offset of userdata partition */
int user_data_config_Read(unsigned int offset, unsigned char *buffer, unsigned int size)
{	
    unsigned char *pConfigAddr = NULL;	
    unsigned int part_index;

    part_index = flash_part_id_to_part_index(IROS_FLASH_PARTITION_DATA_ID);
    if (part_index == IROS_FLASH_PARTITION_INDEX_ANY) {
        
        return 1;
    }
    else {
        pConfigAddr = flash_dev.info.super_block.part_tab[part_index].part_loc + USER_DATA_USED_LEN;
    }

    memcpy(buffer, (pConfigAddr+offset), size);
  
    return 0;
}

/*	write buffer data to userdata partition start address */
int user_data_config_Write( unsigned char *buffer, unsigned int size)
{
    char 	*pWrcfgAddr = NULL;
    int iSize = (int)size;
    char *flash_data = NULL;

    unsigned int part_index;

    part_index = flash_part_id_to_part_index(IROS_FLASH_PARTITION_DATA_ID);
    if (part_index == IROS_FLASH_PARTITION_INDEX_ANY) {
       
        return 1;
    }
    else {
        pWrcfgAddr = flash_dev.info.super_block.part_tab[part_index].part_loc;
    }

    flash_data = iros_malloc(IROS_MID_MALLOC, USER_DATA_USED_LEN + size);
    if (flash_part_write_init(part_index, pWrcfgAddr)) { /* prepare to write flash */
        memcpy(flash_data, pWrcfgAddr, USER_DATA_USED_LEN);
        memcpy(flash_data + USER_DATA_USED_LEN, buffer, size);
        
        flash_write(pWrcfgAddr, flash_data, iSize + USER_DATA_USED_LEN);
        flash_part_write_done(part_index);
    }
    iros_free(flash_data);

    return 0;
}


