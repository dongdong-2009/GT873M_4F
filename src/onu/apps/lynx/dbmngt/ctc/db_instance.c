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

DEFINITIONS:  "DEVICE" means the Cortina Systems? Daytona SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems? SDK software.

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
#include <stdio.h>
#include "plat_common.h"
#include "db_api.h"
#include "db_instance.h"
#include "sdl_api.h"
#ifdef HAVE_DB_MANAGMENT

db_data_confg_t g_ctcCfgTable[] = 
{
    /* name                        id                               type                maxlen                          */
    {"loid passwd",            DB_CTC_LOID_PASSWD_ID,            DB_TYPE_BYTES,        sizeof(db_ctc_loid_pwd_t),       0  },
    {"trm svr0 cfg",           DB_CTC_TRM_SVR0_CFG_ID,           DB_TYPE_BYTES,        sizeof(db_ts_cfg_t),       0  },
    {"trm svr1 cfg",           DB_CTC_TRM_SVR1_CFG_ID,           DB_TYPE_BYTES,        sizeof(db_ts_cfg_t),       0  },
    {"trm svr2 cfg",           DB_CTC_TRM_SVR2_CFG_ID,           DB_TYPE_BYTES,        sizeof(db_ts_cfg_t),       0  },
    {"trm svr3 cfg",           DB_CTC_TRM_SVR3_CFG_ID,           DB_TYPE_BYTES,        sizeof(db_ts_cfg_t),       0  },
    {"admin user",             DB_CTC_ADMIN_INFO_ID,             DB_TYPE_BYTES,        sizeof(db_ctc_user_t),           0  },
    {"power user",             DB_CTC_USER_INFO_ID,              DB_TYPE_BYTES,        sizeof(db_ctc_user_t),           0  },
    {"port info",              DB_CTC_PORT_INFO_ID,              DB_TYPE_BYTES,        sizeof(db_ctc_port_t),           0  },   
  
};

cs_status db_ctc_set_default()
{
    cs_status ret = CS_E_OK;
    db_ctc_loid_pwd_t loid;
    db_ctc_user_t admin;
    db_ctc_user_t user; 
    cs_mac_t mac;
    db_ctc_port_t   port_info;
    cs_uint8 max_port;
    cs_uint32 i;


    startup_config_read(
        CFG_ID_SWITCH_PORT_NUM, sizeof(max_port), &max_port);
    max_port = max_port >DB_CTC_MAX_PORT ? DB_CTC_MAX_PORT:max_port;

    startup_config_read(CFG_ID_MAC_ADDRESS,CS_MACADDR_LEN, &mac.addr[0]);
    
    /* Loid and passwd */ 
    memset(&loid,0x00,sizeof(loid));
    ret = db_read(DB_CTC_LOID_PASSWD_ID,(cs_uint8 *) &loid, sizeof(loid));
    if(ret != CS_E_OK){
        memset(&loid,0x00,sizeof(loid));
        sprintf (loid.loid, "%02x%02x%02x%02x%02x%02x",mac.addr[0], 
            mac.addr[1], 
            mac.addr[2], 
            mac.addr[3], 
            mac.addr[4], 
            mac.addr[5]);
        sprintf(loid.passwd,"1111");
        db_write(DB_CTC_LOID_PASSWD_ID,(cs_uint8 *) &loid, sizeof(loid));
    }

    /* init the admin account*/
    memset(&admin,0x00,sizeof(admin));
    strcpy(admin.usr,DB_CTC_DEFAULT_ADMIN_NAME);
    strcpy(admin.pwd,DB_CTC_DEFAULT_ADMIN_PWD);
    db_write(DB_CTC_ADMIN_INFO_ID,(cs_uint8*) &admin,sizeof(admin));

    /* init the user account*/
    memset(&user,0x00,sizeof(user));
    strcpy(user.usr,DB_CTC_DEFAULT_USR_NAME);
    strcpy(user.pwd,DB_CTC_DEFAULT_USR_PWD);
    db_write(DB_CTC_USER_INFO_ID,(cs_uint8*) &user,sizeof(user));


    memset(&port_info,0x00,sizeof(port_info));
    for(i = 0 ; i < max_port; i++){
        port_info.port[i].admin= SDL_PORT_ADMIN_UP;
        port_info.port[i].auto_neg = SDL_PORT_AUTONEG_UP;
        port_info.port[i].work_mode = SDL_PORT_AUTO_10_100_1000;
        port_info.port[i].pause = FALSE; 
    }

    db_write(DB_CTC_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
    return CS_E_OK;
}

void db_init()
{
    db_core_init(DB_DATA_OFFSET,FALSE,FALSE);
    db_table_init(
            g_ctcCfgTable,
            sizeof(g_ctcCfgTable)/sizeof(g_ctcCfgTable[0]),
            DB_CUR_VERSION,
            db_ctc_set_default,
            NULL);

}


void db_restore()
{
    db_ctc_port_t   port_info;
   
    cs_callback_context_t     context;
    cs_uint8 i = 0;
    cs_status ret = CS_E_OK;
    cs_uint8 max_port;

    startup_config_read(
            CFG_ID_SWITCH_PORT_NUM, sizeof(max_port), &max_port);
    max_port = max_port >DB_CTC_MAX_PORT ? DB_CTC_MAX_PORT:max_port;

    memset(&port_info,0x00,sizeof(port_info));
    db_read(DB_CTC_PORT_INFO_ID,(cs_uint8 *) &port_info, sizeof(port_info));
    for(i = 0; i < max_port;i ++){  
        /*set flow contrl*/
        ret = epon_request_onu_port_flow_ctrl_set(context, ONU_DEVICEID_FOR_API,
                ONU_LLIDPORT_FOR_API, i+1, port_info.port[i].pause);
        if(ret != CS_E_OK){
            cs_printf("flow ctrl set fail\n");
        }     
        
        /* set port admin */
        ret = epon_request_onu_port_admin_set(context, 
                0,0, i+1, port_info.port[i].admin);
        if(ret != CS_E_OK){
            cs_printf("set port admin status failed ,ret = %d \n",ret);
        }

        /* set port auto neg */
        if (port_info.port[i].auto_neg == SDL_PORT_AUTONEG_DOWN
                || port_info.port[i].work_mode != SDL_PORT_AUTO_10_100_1000) {
                ret = epon_request_onu_port_status_set(context, 
                        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, i+1, port_info.port[i].work_mode);
        } else {
                ret = epon_request_onu_port_status_set(context, 
                        ONU_DEVICEID_FOR_API, ONU_LLIDPORT_FOR_API, i+1, SDL_PORT_AUTO_10_100_1000);
        }

        if(ret != CS_E_OK){
            cs_printf("set port auto status failed ,ret = %d \n",ret);
        }

    }
}

#endif
