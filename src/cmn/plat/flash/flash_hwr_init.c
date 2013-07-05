#include "iros_config.h"
#include "cs_types.h"
#include "flash_dev_driver.h"
#include "iros_flash_oper.h"

//#define FLASH_DEBUG

/* Find the info for JFFS2 partition */
extern flash_dev_t flash_dev;
extern FLASH_FAMILY_TYPE_T flash_family_type;
extern void oper_flash_read_id(void*);
char* jffs2_flash_base_address;
cyg_uint32 jffs2_flash_offset;
cyg_uint32 jffs2_flash_length;

void get_jffs2_partition_info (void) {
    cyg_uint16 i;

    for (i = 0; i < IROS_FLASH_PARTITION_TAB_MAX; i++) {
        if (flash_dev.info.super_block.part_tab[i].part_id &&
            (flash_dev.info.super_block.part_tab[i].part_type == IROS_FLASH_PARTITION_TYPE_JFFS2)) {
            jffs2_flash_base_address = flash_dev.info.super_block.part_tab[i].part_loc;
            jffs2_flash_length = flash_dev.info.super_block.part_tab[i].part_size;
            jffs2_flash_offset = 0x0;
            break;
        }
    }

    return ;
}



/* This gets filled up in init */
const flash_dev_info_t* flash_dev_info;
static const flash_dev_info_t supported_devices[] = {
    #include "flash_parts.inl"
};
#define NUM_DEVICES (sizeof(supported_devices)/sizeof(flash_dev_info_t))


//----------------------------------------------------------------------------
// Initialize driver details
int
flash_hwr_init(void)
{
    int i;
    flash_data_t id[2];
    
#ifdef HAVE_FLASH_FS
    flash_dev_query(id);
#else
    oper_flash_read_id((void *)&id);
#endif

    // Look through table for device data
    flash_dev_info = supported_devices;
    for (i = 0; i < NUM_DEVICES; i++) {
        if (flash_dev_info->device_id == id[1])
            break;
        flash_dev_info++;
    }

    // Did we find the device? If not, return error.
    if (NUM_DEVICES == i)
        return FLASH_ERR_DRV_WRONG_PART;

#ifdef HAVE_FLASH_FS
    // Hard wired for now
    flash_info.block_size = flash_dev_info->block_size;
    flash_info.blocks = (jffs2_flash_length / flash_dev_info->block_size) * CYGNUM_FLASH_SERIES;
    flash_info.start = (void *)jffs2_flash_base_address;
    flash_info.end = (void *)(jffs2_flash_base_address + (jffs2_flash_length * CYGNUM_FLASH_SERIES));

#ifdef FLASH_DEBUG
    diag_printf("FS id         : 0x%x    0x%x\n",id[1],flash_dev_info->device_id);
    diag_printf("FS block_size : 0x%x\n",flash_info.block_size);
    diag_printf("FS num_blocks : 0x%x\n",flash_info.blocks);
    diag_printf("FS block_start: 0x%x\n",(unsigned int)flash_info.start);
    diag_printf("FS block_end  : 0x%x\n",(unsigned int)flash_info.end);
#endif

#endif

    return FLASH_ERR_OK;
}

//----------------------------------------------------------------------------
// Map a hardware status to a package error
int
flash_hwr_map_error(int e)
{
    return e;
}


//----------------------------------------------------------------------------
// See if a range of FLASH addresses overlaps currently running code
bool
flash_code_overlaps(void *start, void *end)
{
    extern unsigned char _stext[], _etext[];

    return ((((unsigned long)&_stext >= (unsigned long)start) &&
             ((unsigned long)&_stext < (unsigned long)end)) ||
            (((unsigned long)&_etext >= (unsigned long)start) &&
             ((unsigned long)&_etext < (unsigned long)end)));
}

