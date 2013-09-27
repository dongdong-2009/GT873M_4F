#include "cgi_cmn.h" 
#include "cgi_upgrade.h"
#include "sys_cfg.h"
#include "cgi_system.h"
#include "img_upgrade.h"
#include "mif.h"

cgi_upload_cb_t g_upload_cb;
mif_upgrade_state_t mif_state;

static cs_int8 get_image[] = "Upgrade firmware successfully.";
static cs_uint8 reset_str[]=
        "Upgrade firmware successfully.ONU will be reset in 3 seconds...";
static cs_uint8 g_verify_img_flag = 0;
static cs_boolean g_web_upgrade = FALSE;
extern void iros_system_reset(RESET_TYPE_E reset_type);
#ifdef HAVE_FLASH_FS
extern int fs_mount(void);
extern int fs_umount(void);
#endif

#if 1
static char upload_html[] =
    "<HTML><HEAD></HEAD>"

    "<FORM action=/cgi-bin/get_wlb_image.cgi method=post \
    encType=multipart/form-data >\
    <BODY>Upgrade app firmware(.wlb format): <INPUT type=file name=userfile><INPUT type=submit value=Upload></BODY></FORM>"

    "<FORM action=/get_jffs2_image.cgi method=post \
    encType=multipart/form-data >\
    <BODY>Upgrade JFFS2 image(.wfs format): <INPUT type=file name=userfile><INPUT type=submit value=Upload></BODY></FORM>"
    "</HTML>"

    ;

CYG_HTTPD_IRES_TABLE_ENTRY(cyg_ires_entry_upload, "/upload.html", upload_html, sizeof(upload_html));
#endif

int oper_flash_erase_block(void* block);

cs_boolean cgi_get_web_upgrade_status()
{
    return g_web_upgrade;
}

cs_uint8 cgi_get_img_verify_flag()
{
    return g_verify_img_flag;
}

void cgi_build_upload_response(
        cgi_handler_param_t * p,
        cs_uint8 type, 
        cgi_upload_result_e result)
{
    if(type == CGI_UPLOAD_JFFS2_TYPE)
    {
        if(result == CGI_UPLOAD_SUCCESS)
        {
            cgi_send_int(p, CGI_OK);
            cgi_send_str(p, get_image, strlen(get_image));
        }
        else
        {
            cgi_send_int(p, CGI_ERROR_PARAM);
        }
    }
    else if(type == CGI_UPLOAD_WLB_TYPE)
    {
        
        if(result == CGI_UPLOAD_SUCCESS)
        {
            cgi_send_str(p, reset_str, strlen(reset_str));
            /*delay 3 seconds*/
            cs_thread_delay(3000);
            cgi_sys_reset(p);
        }
        else
        {
            cgi_send_int(p, CGI_ERROR_PARAM);
        }
    }
    else
    {
        
        if(result == CGI_UPLOAD_SUCCESS)
        {
            if(mif_state.tlv_update_successed & 0x1 ||
                mif_state.tlv_update_successed & 0x4 ||
                mif_state.tlv_update_successed & 0x200)
            {
                cgi_send_str(p, reset_str, strlen(reset_str));
                /*delay 3 seconds*/
                cs_thread_delay(3000);
                cgi_sys_reset(p);
            }
            else
            {
                cgi_send_str(p, get_image, strlen(get_image));
            }
        }
        else
        {
            cgi_send_int(p, CGI_ERROR_PARAM);
        }
    }
}

cgi_status cgi_write_upload_data_end(cs_uint8 type, cs_boolean succ)
{
    cs_uint8 tmp[4];
    if(type == CGI_UPLOAD_JFFS2_TYPE)
    {
        flash_part_write_done(g_upload_cb.part_index);
        if(succ)
        {
            fs_mount();
            g_verify_img_flag = 1;
        }
        return CGI_OK;
    }
    else if(type == CGI_UPLOAD_WLB_TYPE)
    {
        img_upgrade_download_stop(0);
        if(succ)
        {
            g_verify_img_flag = 2;
        }
        return CGI_OK;
    }
    else
    {
        if(0 != mif_upgrade(IROS_MID_WEB, tmp, 0, &mif_state, TRUE))
        {
            return CGI_INNER_ERROR;
        }
        else 
        {
            if(succ)
            {
                if(mif_state.tlv_update_successed & 0x1 ||
                    mif_state.tlv_update_successed & 0x4 ||
                    mif_state.tlv_update_successed & 0x200)
                {
                    CGI_DEBUG("need reset\n");
                    g_verify_img_flag = 2;
                }
                else
                {
                    g_verify_img_flag = 1;
                }
            }
            return CGI_OK;
        }
    }
}

cgi_status cgi_write_upload_data(cs_uint8 *data, cs_uint32 len, cs_uint8 type)
{
    if(type == CGI_UPLOAD_JFFS2_TYPE)
    {
        cs_uint8 *dest = NULL;
        dest = flash_dev.info.super_block.part_tab[g_upload_cb.part_index].part_loc +
                  g_upload_cb.curr_len;
        flash_write(dest, data, len);
        g_upload_cb.curr_len += len;

        return CGI_OK;
    }
    else if(type == CGI_UPLOAD_WLB_TYPE)
    {
        img_upgrade_write_buf(data, len);
        return CGI_OK;
    }
    else
    {
        if(0 != mif_upgrade(IROS_MID_WEB, data, len, &mif_state, FALSE))
        {
            mif_upgrade(IROS_MID_WEB, data, 0, &mif_state, TRUE);
            return CGI_INNER_ERROR;
        }
        else
        {
            return CGI_OK;
        }
    }
}

void cgi_img_upgrade_download_callback(img_upgrade_download_result_e result)
{
    if(result == IMG_UPGRADE_DOWNLOAD_OK)
    {
        img_upgrade_confirm_policy_set(1);
    }
}
cgi_status cgi_write_upload_data_prepare(cs_uint8 type, cs_uint32 img_len)
{
    memset(&g_upload_cb, 0, sizeof(g_upload_cb));
    g_upload_cb.img_len = img_len;
    
    if(type == CGI_UPLOAD_JFFS2_TYPE)
    {
        cyg_uint32 part_index = flash_part_id_to_part_index(IROS_FLASH_PARTITION_JFFS2_ID);
        cs_uint8 *flash_des;
        cs_uint8 loop = 0;
        cs_uint32 part_size = 0;
        
        if(IROS_FLASH_PARTITION_INDEX_ANY == part_index)
        {
            return CGI_SDL_FAILED;
        }

        fs_umount();
            
        g_upload_cb.part_index = part_index;
        flash_des = flash_dev.info.super_block.part_tab[part_index].part_loc;
        
        /*eraise this part first*/
        part_size = flash_dev.info.super_block.part_tab[part_index].part_size;
        while(loop*flash_dev.info.super_block.block_size < part_size)
        {
            oper_flash_erase_block(flash_des+(loop*flash_dev.info.super_block.block_size));
            cs_thread_delay(150);
            loop++;
        }
        
        if(flash_part_write_init(part_index, flash_des))
        {
            return CGI_OK;
        }
        else
        {
            return CGI_SDL_FAILED;
        }
        
    }
    else if(type == CGI_UPLOAD_WLB_TYPE)
    {
        if(CS_E_OK != img_upgrade_download_start(CGI_HTTP_BUF_SIZE, 
                              cgi_img_upgrade_download_callback, IMG_UPGRADE_WRITE_PART))
        {
            return CGI_SDL_FAILED;
        }
        else
        {
            img_upgrade_set_mif_support(FALSE);
            return CGI_OK;
        }
    }
    else
    {
        /*for mif*/
        if(0 != prepare_mif_upgrade(IROS_MID_WEB, FALSE))
        {
            return CGI_SDL_FAILED;
        }
        else
        {
            return CGI_OK;
        }
    }
    
}

cs_uint32 cgi_get_image_len(cs_uint8 *pData, cs_uint8 type, cs_uint32 *parse_len)
{
    cs_uint32 img_len = 0;
    cs_uint32 out_len = 0;
    
    img_len = (*pData <<24)   | (*(pData+1) <<16)  | (*(pData+2) <<8) |  *(pData+3);
    out_len = 4;

#ifdef HAVE_HW_OAM
    if(type == CGI_UPLOAD_MIF_TYPE)
    {
        epon_l2ftp_multi_image_header_t *header = (epon_l2ftp_multi_image_header_t*)pData;
        img_len = ntohl(header->length) + sizeof(epon_l2ftp_multi_image_header_t);
        out_len = 0;
    }
#endif

    *parse_len = out_len;
    return img_len;
}

cgi_status cgi_check_image_magic(cs_uint8 *pMagic, cs_uint8 type, cs_uint32 *len)
{
    cs_uint8 *tmp = pMagic;

    if(type == CGI_UPLOAD_JFFS2_TYPE)
    {
        if( (*tmp++!= '.') || (*tmp++!= 'w') ||(*tmp++!= 'f') || (*tmp++!= 's'))
        {
            return CGI_ERROR_PARAM;
        }
        *len = 4;
    }
    else
    {
        #ifdef HAVE_ZTE_OAM
        if(strcmp(ONU_IMG_SIGNATURE_STR2, tmp))
        {
            return CGI_ERROR_PARAM;
        }
        *len = strlen(ONU_IMG_SIGNATURE_STR2);
        #elif HAVE_HW_OAM
        if(type == CGI_UPLOAD_MIF_TYPE)
        {
            epon_l2ftp_multi_image_header_t * header = (epon_l2ftp_multi_image_header_t*)pMagic;
            if(ntohl(header->mif_magic) != EPON_L2FTP_MULTI_IMAGE_MAGIC)
            {
                return CGI_ERROR_PARAM;
            }
            else
            {
                *len = 0;
            }
        }
        else
        {
            if( (*tmp++!= '.') || (*tmp++!= 'w') ||(*tmp++!= 'l') || (*tmp++!= 'b'))
            {
                return CGI_ERROR_PARAM;
            }
            *len = 4;
        }
        #else
        if(type == CGI_UPLOAD_WLB_TYPE){
            if( (*tmp++!= '.') || (*tmp++!= 'w') ||(*tmp++!= 'l') || (*tmp++!= 'b'))
            {
                return CGI_ERROR_PARAM;
            }
            *len = 4;
        }
        #endif
    }

    return CGI_OK;
}

cgi_status cgi_upload_image_process(cgi_handler_param_t * p, cs_uint8 type)
{
    cs_uint8 *data_buf = NULL;
    cs_int32 len = 0;
    cs_uint8 *tmp = NULL;
    cs_uint32 header_len = 0;
    cs_uint32 left_len = 0, read_len = 0, magic_len = 0;
    cgi_status status = CGI_OK;
    cs_uint32 rec_blocks = 0;
    cs_uint32 parse_len = 0;

    g_verify_img_flag = 0;
    g_web_upgrade = TRUE;

    if(img_upgrade_in_progress())
    {
        CGI_LOG("Already in upgrading....\n");
        status = CGI_INNER_ERROR;
        goto PROCESS_ERROR;
    }

    data_buf=iros_malloc(IROS_MID_UPGRADE, CGI_HTTP_BUF_SIZE);
    if(NULL == data_buf )
    {
        CGI_LOG("\nmalloc image buffer error\n");
        status = CGI_INNER_ERROR;
        goto PROCESS_ERROR;
    }

    len = cgi_get_current_data(data_buf, CGI_HTTP_BUF_SIZE, p);
    tmp = strstr(data_buf, "application/octet-stream\r\n\r\n");
    if(tmp == NULL)
    {
        /*read data from socket*/
        len = cgi_read_socket_data(data_buf, CGI_HTTP_BUF_SIZE, p);
        tmp = strstr(data_buf, "application/octet-stream\r\n\r\n");
        if(tmp == NULL)
        {
            CGI_LOG("can not get the right header\n");
            status = CGI_INNER_ERROR;
            goto PROCESS_ERROR;
        }
    }
    header_len = (cs_uint32)(tmp - data_buf) + 28;
    tmp += 28;
  /* CGI_LOG("check magic\n");*/
    /*check the magic number*/
    status = cgi_check_image_magic(tmp, type, &magic_len);
    if(CGI_OK != status)
    {
        CGI_LOG("magic is not right %d\n", status);
        goto PROCESS_ERROR;
    }    

    header_len += magic_len;
    tmp += magic_len;

    /*get img len*/
    left_len = cgi_get_image_len(tmp, type, &parse_len);
    /*CGI_LOG("image len is %x\n", left_len);*/
    header_len += parse_len;
    tmp += parse_len;
    len -= header_len;

    /*prepare*/
    status = cgi_write_upload_data_prepare(type, left_len);
    if(CGI_OK != status)
    {
        CGI_LOG("upload data prepare fail %d\n", status);
        goto PROCESS_ERROR;
    }
    
    /*write current data*/
    status  = cgi_write_upload_data(tmp, len, type);
    if(CGI_OK != status)
    {
        CGI_LOG("write data fail %d\n", status);
        goto FLASH_WRITE_FAILURE;
    }
    cs_printf(".");
    rec_blocks++;
    left_len -= len;

    while(left_len > 0)
    {
        if(left_len > CGI_HTTP_BUF_SIZE)
        {
            read_len = CGI_HTTP_BUF_SIZE;
        }
        else
        {
            read_len = left_len;
        }
         
        len = cgi_read_socket_data(data_buf, read_len, p);
        if(len <= 0)
        {
            CGI_LOG("read socket error!\n");
            status = CGI_INNER_ERROR;
            
            goto FLASH_WRITE_FAILURE;
        }
        status = cgi_write_upload_data(data_buf, len, type);
        if(CGI_OK != status)
        {
            CGI_LOG("write data fail %d in loop\n", status);
            goto FLASH_WRITE_FAILURE;
        }
        left_len -= len;
        cs_printf(".");
        rec_blocks++;
        if(rec_blocks%32 == 0)
        {
            cs_printf("\n");
        }
    }

    /*upload finish*/
    status = cgi_write_upload_data_end(type, TRUE);
    if(CGI_OK != status)
    {
        CGI_LOG("write end fail %d\n", status);
        goto PROCESS_ERROR;
    }

    iros_free(data_buf);
    g_web_upgrade = FALSE;
    return CGI_OK;
    
FLASH_WRITE_FAILURE:    
    cgi_write_upload_data_end(type, FALSE);
    
PROCESS_ERROR:
    if(NULL != data_buf)
    {
        iros_free(data_buf);
        data_buf = NULL;
    }
    cgi_clean_socket_data(p);
    /*cgi_build_upload_response(p, type, CGI_UPLOAD_FAIL);*/
    g_web_upgrade = FALSE;
    return status;
    
}

cgi_status cgi_get_wlb_image(cgi_handler_param_t * p)
{
    CGI_DEBUG("cgi_get_wlb_image enter \n");
    if(CGI_OK == cgi_upload_image_process(p, CGI_UPLOAD_WLB_TYPE))
    {
        CGI_LOG("wlb image ext enter ok, please reset onu now!!!\n");
        cgi_send_int(p, CGI_OK);
        /*delay 3 seconds*/
        cs_thread_delay(3000);
        cgi_sys_reset(p);
        return CGI_OK;
    }
    else
    {
        cgi_send_int(p, CGI_INNER_ERROR);
        return CGI_INNER_ERROR;
    }
	iros_system_reset(1);
}

cgi_status cgi_get_mif_image(cgi_handler_param_t * p)
{
    CGI_DEBUG("cgi_get_mif_image enter \n");
    if(CGI_OK == cgi_upload_image_process(p, CGI_UPLOAD_MIF_TYPE))
    {
        cgi_send_int(p, CGI_OK);
        if(g_verify_img_flag == 2)
        {
            /*delay 3 seconds*/
            cs_thread_delay(3000);
            cgi_sys_reset(p);
        }
        return CGI_OK;
    }
    else
    {
        cgi_send_int(p, CGI_INNER_ERROR);
        return CGI_INNER_ERROR;
    }
}

#ifdef HAVE_FLASH_FS
cgi_status cgi_get_jffs2_image(cgi_handler_param_t * p)
{
    CGI_DEBUG("cgi_get_jffs2_image enter \n");
    
    if(CGI_OK == cgi_upload_image_process(p, CGI_UPLOAD_JFFS2_TYPE))
    {
        cgi_send_int(p, CGI_OK);
        return CGI_OK;
    }
    else
    {
        cgi_send_int(p, CGI_INNER_ERROR);
        return CGI_INNER_ERROR;
    }
}
#endif



