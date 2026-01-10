#include "ezm_pp_drv.h"
#include "ezm_pp_uart_drv.h"

static ezm_pp_drv_msg_t ezm_pp_drv_msg;

static uint32_t ezm_pp_drv_rx_uart(void *buffer);
static uint32_t ezm_pp_drv_tx_uart(const ezm_pp_drv_pkt_t *drv_pkt);

typedef uint32_t (*ezm_pp_drv_rx_f)(void *buffer);
typedef uint32_t (*ezm_pp_drv_tx_f)(const ezm_pp_drv_pkt_t *drv_pkt);

const ezm_pp_drv_rx_f tp_binding_rx[] = {[EZM_PP_DRV_SERIAL] =
                                             ezm_pp_drv_rx_uart};

const ezm_pp_drv_tx_f tp_binding_tx[] = {[EZM_PP_DRV_SERIAL] =
                                             ezm_pp_drv_tx_uart};

static uint32_t ezm_pp_drv_rx_uart(void *buffer) {
  return ezm_pp_uart_drv_read(buffer, EZM_PP_MAX_MESSAGE_SIZE);
}

static uint32_t ezm_pp_drv_tx_uart(const ezm_pp_drv_pkt_t *drv_pkt) {
  return ezm_pp_uart_drv_transmit(drv_pkt->drv_pkt.pkt, drv_pkt->drv_pkt_len);
}

int ezm_pp_drv_init(void) {
  // Initialize drivers here.
  return 0; // Success
}

uint32_t ezm_pp_drv_get_pkt(ezm_pp_drv_pkt_t *drv_pkt) {
  if (drv_pkt == NULL || drv_pkt->drv_pkt.pkt == NULL) {
    return 0;
  }

  drv_pkt->drv_pkt_len = 0;

  if(drv_pkt->drv_id > EZM_PP_DRV_MAX ||
     tp_binding_rx[drv_pkt->drv_id] == NULL) {
    return 0;
  }

  uint32_t rt = tp_binding_rx[drv_pkt->drv_id](drv_pkt->drv_pkt.pkt);

  return rt;
}

uint32_t ezm_pp_drv_send_pkt(const ezm_pp_drv_pkt_t *drv_pkt) {
  if (drv_pkt == NULL) {
    return 0;
  }

  return tp_binding_tx[drv_pkt->drv_id](drv_pkt);
}

void ezm_pp_drv_set_msg(ezm_pp_drv_ev_t ev, ezm_pp_action_t action, ezm_pp_drv_phy_tp_binding_id_t drv_id, uint32_t arg) {
  ezm_pp_drv_msg.ev = ev;
  ezm_pp_drv_msg.drv_id = drv_id;
  ezm_pp_drv_msg.action = action;
  ezm_pp_drv_msg.arg = arg;
}

ezm_pp_drv_msg_t ezm_pp_drv_get_msg(void) {
  ezm_pp_drv_msg_t msg = ezm_pp_drv_msg;
  ezm_pp_drv_msg.ev = EZM_PP_DRV_EV_NONE;
  ezm_pp_drv_msg.drv_id = 0;
  ezm_pp_drv_msg.action = EZM_PP_NO_ACTION;
  ezm_pp_drv_msg.arg = 0;
  return msg;
}

void ezm_pp_drv_listen(void) {
  ezm_pp_uart_drv_listen();
}