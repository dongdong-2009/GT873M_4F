#ifndef _SC_EVENT_H_
#define _SC_EVENT_H_

typedef struct
{
    cs_node node;
    cs_int32 evt_type;
    void (*handler)(void *data, cs_int32 len);
}sc_event_cb_t;

extern void sc_event_init();
extern void sc_event_reg
    (
    cs_int32 event_type,
    void (*handler)(void *data, cs_int32 len)
    );
extern void sc_event_proc(cs_dev_id_t index,cs_uint8 *frame, cs_int32 len);
cs_int32 sc_event_ack(cs_dev_id_t index, cs_uint8 *frame, cs_int32 len);

#endif

