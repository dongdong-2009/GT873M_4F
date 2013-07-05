#ifndef _OOBSC_HELLO_H_
#define _OOBSC_HELLO_H_

typedef struct
{
    cs_uint16   valid;
    cs_mac_t    dev_mac;
    cs_uint16   llid;
    cs_uint8    hello_state;
    cs_uint8    reg_state;
    cs_uint16   hello_expire;
    cs_uint16   hello_interval;
    cs_uint32   pkt_seq;
    cs_uint32   version;
    cs_uint32   device;
    cs_uint32   dev_type;
    void        *l2ftp;
} cs_dev_t;
#define CS_MAX_DEV_NUM   512

typedef struct
{
    cs_uint32 hello_cnt;
    cs_int32 hello_interval;
    cs_int32 hello_expire;
    cs_dev_t dev[CS_MAX_DEV_NUM];
}cs_hello_info_t;

extern void oobsc_hello_tmfunc();
extern void oobsc_hello_proc(cs_int8 *frame, cs_int32 len);
extern cs_dev_t *oobsc_get_dev_by_macaddr(cs_mac_t *pMac);
extern cs_status oobsc_get_devid_by_macaddr(cs_mac_t *pMac, cs_dev_id_t *id);
extern cs_dev_t *oobsc_get_dev_by_index(cs_int32 index);
extern cs_dev_t *oobsc_get_free_dev();
extern cs_int32 hello_is_active(cs_mac_t *mac);
#endif
