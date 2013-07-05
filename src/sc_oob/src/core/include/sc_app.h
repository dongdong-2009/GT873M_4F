#ifndef _OOBSC_APP_H_
#define _OOBSC_APP_H_

#define OOBSC_APP_QUEUE_NAME    "sc_app_queue"
#define OOBSC_APP_QUEUE_DEPTH   128
#define OOBSC_APP_QUEUE_MSGLEN  sizeof(cs_msg_t)
#define OOBSC_APP_QUEUE_MAX_PRI 4

#define OOBSC_APP_THREAD_NAME       "App Thread"
#define OOBSC_APP_THREAD_STACK_SIZE (16*1024)
#define OOBSC_APP_THREAD_PRIORITY   10

extern cs_int32 cs_send_msg_to_app(cs_uint8 msg_type, void * data, cs_uint16 len);
#endif
