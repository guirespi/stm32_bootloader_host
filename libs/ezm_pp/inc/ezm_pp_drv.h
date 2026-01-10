#ifndef EZM_PP_DRV_H
#define EZM_PP_DRV_H

#include "ezm_pp_pkt.h"
#include <stdint.h>

typedef enum {
  EZM_PP_DRV_EV_NONE = 0, 
  EZM_PP_DRV_EV_NEW_MSG = 1, 
} ezm_pp_drv_ev_t;

typedef enum {
  EZM_PP_DRV_ASYNCRHONOUS_SERIAL = 0x24,
} ezm_pp_drv_phy_medium_id_t;

typedef enum {
  EZM_PP_DRV_SERIAL = 0x05,
  EZM_PP_DRV_MAX,
} ezm_pp_drv_phy_tp_binding_id_t;

typedef struct {
  ezm_pp_uart_pkt_t *uart_pkt;
} ezm_pp_drv_uart_pkt_t;

typedef union {
  void *pkt;
  ezm_pp_drv_uart_pkt_t uart_pkt;
} ezm_pp_drv_pkt_u;

typedef struct {
  uint32_t drv_pkt_len;
  ezm_pp_drv_phy_tp_binding_id_t drv_id;
  ezm_pp_drv_pkt_u drv_pkt;
} ezm_pp_drv_pkt_t;

typedef struct {
  ezm_pp_drv_ev_t ev;
  ezm_pp_action_t action;
  ezm_pp_drv_phy_tp_binding_id_t drv_id;
  uint32_t arg;
} ezm_pp_drv_msg_t;

int ezm_pp_drv_init(void);
uint32_t ezm_pp_drv_get_pkt(ezm_pp_drv_pkt_t *drv_pkt);
uint32_t ezm_pp_drv_send_pkt(const ezm_pp_drv_pkt_t *drv_pkt);
void ezm_pp_drv_set_msg(ezm_pp_drv_ev_t ev, ezm_pp_action_t action, ezm_pp_drv_phy_tp_binding_id_t drv_id, uint32_t arg);
ezm_pp_drv_msg_t ezm_pp_drv_get_msg(void);
void ezm_pp_drv_listen(void);

#endif // EZM_PP_DRV_H