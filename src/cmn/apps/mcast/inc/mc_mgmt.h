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

DEFINITIONS:  "DEVICE" means the Cortina Systems?LynxD SDK product.
"You" or "CUSTOMER" means the entity or individual that uses the SOFTWARE.
"SOFTWARE" means the Cortina Systems?SDK software.

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
#ifndef _MC_CONFIG_H
#define _MC_CONFIG_H

#include "mc_type.h"




extern mc_port_t *mc_port_create();
extern void mc_port_init (cs_port_id_t portid, mc_port_t *port);
extern cs_status mc_port_add(mc_port_t *port, mc_node_t *mc_node);
extern mc_port_t *mc_port_find(mc_node_t *mc_node, cs_port_id_t port_id);
extern cs_boolean mc_is_uplink_port(mc_node_t *mc_node, cs_port_id_t port_id);
extern mc_mode_t mc_node_get_mode(const mc_node_t *mc_node);
extern cs_status mc_node_set_mode(mc_node_t *mc_node,mc_mode_t mode);
extern mc_fwd_engine_t mc_node_get_fwd_engine(const mc_node_t *mc_node);
extern cs_status mc_node_set_fwd_engine(mc_node_t *mc_node, mc_fwd_engine_t engine);
extern mc_vlan_learn_t mc_node_get_vlan_lrn(const mc_node_t *mc_node);
extern cs_status mc_node_set_vlan_lrn(mc_node_t *mc_node, mc_vlan_learn_t lrn_mode);
extern cs_status mc_node_get_up_port(const mc_node_t *mc_node, mc_up_port_cfg_t *up_cfg);
extern cs_status mc_node_set_up_port(mc_node_t *mc_node, const mc_up_port_cfg_t *up_cfg);
extern cs_uint16 mc_node_get_max_grp(const mc_node_t *mc_node);
extern cs_status mc_node_set_max_grp(mc_node_t *mc_node, cs_uint16 max_grp);
extern cs_uint16 mc_node_get_grp_num(const mc_node_t *mc_node);
extern cs_uint32 mc_node_get_gmi(const mc_node_t *mc_node);
extern cs_uint32 mc_node_set_gmi(mc_node_t *mc_node, cs_uint32 gmi);
extern cs_uint32 mc_node_get_robustness(const mc_node_t *mc_node);
extern cs_status mc_node_set_robustness(mc_node_t *mc_node, cs_uint32 robustness);
extern cs_uint32 mc_node_get_query_interval(const mc_node_t *mc_node);
extern cs_status mc_node_set_query_interval(mc_node_t *mc_node, cs_uint32 interval);
extern cs_uint32 mc_node_get_query_rsp_interval(const mc_node_t *mc_node);
extern cs_status mc_node_set_query_rsp_interval(mc_node_t *mc_node, cs_uint32 interval);
extern cs_uint32 mc_node_get_last_member_query(const mc_node_t *mc_node);
extern cs_status mc_node_set_last_member_query(mc_node_t *mc_node, cs_uint32 interval);
extern cs_uint32 mc_node_get_last_member_count(const mc_node_t *mc_node);
extern cs_status mc_node_set_last_member_count(mc_node_t *mc_node, cs_uint32 count);
extern mc_leave_mode_t mc_node_get_leave_mode(mc_node_t *mc_node, cs_port_id_t portid);
extern cs_status mc_node_set_leave_mode(
    mc_node_t *mc_node, 
    cs_port_id_t portid, 
    mc_leave_mode_t mode);
extern cs_uint16 mc_node_get_port_max_grp(mc_node_t *mc_node, cs_port_id_t portid);
extern cs_status mc_node_set_port_max_grp(
                                            mc_node_t *mc_node, 
                                            cs_port_id_t portid, 
                                            cs_uint16 max_grp);
extern cs_uint32 mc_node_get_port_grp_num(mc_node_t *mc_node, cs_port_id_t portid);
extern cs_status mc_node_set_port_ingress_filter(
    mc_node_t *node, 
    mc_port_ingress_filter_routine_t filter);
extern cs_status mc_node_set_port_egress_filter(
    mc_node_t *node, 
    mc_port_egress_filter_routine_t filter);


#endif /* #ifndef _MC_CONFIG_H */

