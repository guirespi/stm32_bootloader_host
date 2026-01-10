#include "ezm_pp_obj.h"
#include "ezm_ut_utils.h"

ezm_pp_obj_t ezm_pp_host_obj = {
    .phy = EZM_PP_DRV_SERIAL,
    .role = EZM_PP_OBJ_ROLE_HOST,
};

ezm_pp_obj_t ezm_pp_ep_obj = {
    .phy = EZM_PP_DRV_SERIAL,
    .role = EZM_PP_OBJ_ROLE_ENDPOINT,
};

ezm_pp_obj_t * ezm_pp_obj_get_serial_receiver(ezm_pp_uart_pkt_t * uart_pkt){
    ezm_pp_pkt_t * pp_pkt = &uart_pkt->pp_pkt;
    if(pp_pkt->tp_hdr.dst_id == EZM_PP_OBJ_ROLE_ENDPOINT){
        return (ezm_pp_obj_t *)&ezm_pp_ep_obj;
    }
    if(pp_pkt->tp_hdr.dst_id == EZM_PP_OBJ_ROLE_HOST){
        return (ezm_pp_obj_t *)&ezm_pp_host_obj;
    }
    return NULL;
}

ezm_pp_obj_t * ezm_pp_obj_get_serial_sender(ezm_pp_uart_pkt_t * uart_pkt){
    ezm_pp_pkt_t * pp_pkt = &uart_pkt->pp_pkt;
    if(pp_pkt->tp_hdr.src_id == EZM_PP_OBJ_ROLE_ENDPOINT){
    return (ezm_pp_obj_t *)&ezm_pp_ep_obj;
    }
    if(pp_pkt->tp_hdr.src_id == EZM_PP_OBJ_ROLE_HOST){
        return (ezm_pp_obj_t *)&ezm_pp_host_obj;
    }
    return NULL;
}