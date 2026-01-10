#ifndef EZM_PP_PKT_PROCESSOR_H
#define EZM_PP_PKT_PROCESSOR_H

#include "ezm_pp_drv.h"

void ezm_pp_pkt_processor_get_pkt(const ezm_pp_drv_pkt_t *drv_pkt,
                                  ezm_pp_obj_t ** sender, ezm_pp_obj_t ** receiver, 
                                  uint32_t tlp_length, ezm_pp_pkt_t **pp_pkt,
                                  uint32_t *pp_pkt_length);

ezm_pp_action_t ezm_pp_pkt_processor_message(ezm_pp_obj_t *sender,
                                             ezm_pp_obj_t *receiver,
                                             uint32_t *pkt_length,
                                             ezm_pp_pkt_t *rx_pkt,
                                             ezm_pp_pkt_t *tx_pkt);

#endif // EZM_PP_PKT_PROCESSOR_H