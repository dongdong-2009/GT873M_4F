#ifndef _OOBSC_ENV_H_
#define _OOBSC_ENV_H_

#define MAX_ONU_IMAGE_SIZE  (1024*1024)

#define MAX_RX_BUFF_SIZE    (1518)
#define SC_MGMT_PORT        "eth0"

typedef struct
{
    cs_uint32 image_size;
    cs_int8 *image_buf;
}cs_image_t;

extern cs_int32 cs_dev_image(cs_int8 *file);
extern void cs_start(cs_int8 *ifName);
extern cs_int32 oobsc_pkt_send(cs_int8 *frame, cs_int32 len);
extern void oobsc_mac_get(cs_mac_t *mac);
extern void cs_msg_recv_handler_reg
    (
    cs_int32 (*handler)(cs_uint8 msg_type, void *data, cs_uint16 len)
    );

extern cs_int8 oobsc_mac[CS_MACADDR_LEN];
#endif
