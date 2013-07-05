#ifndef _OOBSC_MSG_H_
#define _OOBSC_MSG_H_
typedef struct
{
    cs_uint8  msg_type;
    cs_uint8  reserved;
    cs_uint16 msg_len;
    void      *data;
}cs_msg_t;

typedef enum
{
    MSG_TYPE_INVALID         = 0,
    MSG_TYPE_PKT             = 1,
    MSG_TYPE_HELLO_TIMEOUT   = 2,
    MSG_TYPE_L2FTP           = 3,
    MSG_TYPE_MAX     
}MSG_TYPE_ENUM;

typedef enum
{
    MSG_PRI_0 = 0,
    MSG_PRI_1 = 1,
    MSG_PRI_2 = 2,
    MSG_PRI_3 = 3
}MSG_PRI_ENUM;

#endif

