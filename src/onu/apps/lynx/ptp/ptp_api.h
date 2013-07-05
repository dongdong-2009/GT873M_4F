#ifndef _PTP_API_H
#define _PTP_API_H

#include "ptp_type.h"

void ptpv2_init();
ptp_mode_t onu_ptp_get_mode();
cs_status onu_ptp_set_mode(ptp_mode_t mode);
cs_uint32 ptp_get_rx_time_correction();
void ptp_set_rx_time_correction(cs_uint32 corr);
cs_uint32 ptp_get_tx_time_correction();
void ptp_set_tx_time_correction(cs_uint32 corr);

#endif /* #ifndef _PTP_API_H */

