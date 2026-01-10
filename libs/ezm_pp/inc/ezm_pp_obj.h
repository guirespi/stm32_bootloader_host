#ifndef EZM_PP_OBJ_H
#define EZM_PP_OBJ_H

#include "ezm_pp_drv.h"

typedef enum __attribute__((packed)) {
  EZM_PP_OBJ_ROLE_HOST = 0,
  EZM_PP_OBJ_ROLE_ENDPOINT = 1,
} ezm_pp_obj_role_t;

typedef struct {
  ezm_pp_drv_phy_tp_binding_id_t phy;
  ezm_pp_obj_role_t role;
} ezm_pp_obj_t;

extern ezm_pp_obj_t ezm_pp_host_obj;
extern ezm_pp_obj_t ezm_pp_ep_obj;

ezm_pp_obj_t * ezm_pp_obj_get_serial_receiver(ezm_pp_uart_pkt_t * uart_pkt);
ezm_pp_obj_t * ezm_pp_obj_get_serial_sender(ezm_pp_uart_pkt_t * uart_pkt);

#endif /* EZM_PP_OBJ_H */