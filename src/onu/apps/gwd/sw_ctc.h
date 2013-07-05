#ifndef SW_CTC_H
#define SW_CTC_H


#define IS_EPON_ONU_CTC_SW_UNI_PORT_VALID(port_idx) \
       ((((port_idx) > 0)&&((port_idx) <=oam_plat_get_switch_port_num()) )|| ((port_idx) == CTC_PORT_ID_ALL_UNI))

#endif
