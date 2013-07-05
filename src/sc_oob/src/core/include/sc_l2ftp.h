/* Copyright: (c) 2005 ImmenStar Inc. All Rights Reserved. */

#ifndef _SC_L2FTP_H_
#define _SC_L2FTP_H_

typedef struct sc_l2ftp_record {
    cs_uint32   timer;
    cs_uint32   total_block;
    cs_uint16   block_next;
    cs_uint8    end_of_image;
    cs_uint8    reserved;
    cs_uint32   offset;
    cs_uint32   retry;
    cs_uint8    *image_ptr;
    cs_uint32   image_len;
    cs_uint32   dev_index;
} sc_l2ftp_record_t;

extern void cs_l2ftp_proc(cs_ether_frame_t *frame);
#endif

