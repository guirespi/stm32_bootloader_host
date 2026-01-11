#include <stdbool.h>
#include <string.h>

#include <ezm_pp_drv.h>
#include <ezm_pp_uart_drv.h>

#include <ezm_api_console.h>
#include <ezm_crc_utils.h>
#include <ezm_ut_utils.h>
#include <ezm_net_utils.h>

#include <ezm_log_gen.h>
DEFINE_TAG();


#define EZM_PP_UART_DRV_MAX_PKT_SIZE                                           \
  (EZM_PP_MAX_MESSAGE_SIZE - EZM_PP_UART_DRV_CRC16_SIZE)

#define EZM_PP_UART_DRV_MIN_PKT_SIZE                                           \
  (sizeof(ezm_uart_hdr_t) + EZM_PP_UART_DRV_CRC8_SIZE)

#define EZM_PP_UART_DRV_MAX_BUFFER_SIZE                                         \
  (EZM_PP_MAX_MESSAGE_SIZE + EZM_PP_UART_DRV_CRC16_SIZE)

STATIC_GLOBAL uint8_t uart_drv_buffer[EZM_PP_UART_DRV_MAX_BUFFER_SIZE];
STATIC_GLOBAL uint16_t uart_drv_buffer_len = 0;

bool ezm_pp_uart_validate_frame(uint8_t *frame, uint32_t frame_len) {
  bool rt = false;
  if (frame == NULL || frame_len < EZM_PP_UART_DRV_MIN_PKT_SIZE) {
    return rt;
  }

  ezm_pp_uart_pkt_t *uart_pkt = (ezm_pp_uart_pkt_t *)frame;

  if (uart_pkt->tl_hdr.magic_byte != EZM_PP_UART_DRV_MAGIC_BYTE) {
    return rt;
  }

  if (uart_pkt->tl_hdr.pkt_len != frame_len) {
    return rt;
  }

  // Validate PEC
 if (uart_pkt->tl_hdr.pec_type == EZM_PP_UART_DRV_SEL_CRC) {
    uint16_t calculated_crc =
        ezm_crc16_ccitt((uint8_t *)frame, frame_len);
    if (0 != calculated_crc) {
      return rt;
    }
  } else {
    return rt; // Unknown PEC type
  }

  rt = true;
  return rt;
}

uint32_t ezm_pp_uart_drv_fill_header(void *buffer, uint32_t pp_pkt_len) {
  uint32_t total_len = pp_pkt_len + sizeof(ezm_uart_hdr_t);
  ezm_pp_uart_pkt_t *uart_pkt = (ezm_pp_uart_pkt_t *)buffer;
  uart_pkt->tl_hdr.magic_byte = EZM_PP_UART_DRV_MAGIC_BYTE;
  uart_pkt->tl_hdr.hdr_version = EZM_PP_UART_DRV_HDR_VERSION;
  uart_pkt->tl_hdr.pkt_len = total_len;

  uart_pkt->tl_hdr.pec_type = EZM_PP_UART_DRV_PEC_TYPE_CITT;

  return total_len;
}

uint32_t ezm_pp_uart_drv_read(void *buffer, uint32_t buffer_len) {
  if (buffer == NULL || buffer_len < EZM_PP_UART_DRV_MIN_PKT_SIZE) {
    return 0;
  }

  uint32_t rcvd_len = uart_drv_buffer_len;

  memcpy(buffer, uart_drv_buffer, uart_drv_buffer_len);
  rcvd_len = uart_drv_buffer_len;

  uart_drv_buffer_len = 0;
  memset(uart_drv_buffer, 0, sizeof(uart_drv_buffer));

  return rcvd_len;
}

uint32_t ezm_pp_uart_drv_transmit(void *buffer, uint32_t pp_pkt_len) {
  if (buffer == NULL || pp_pkt_len > EZM_PP_UART_DRV_MAX_PKT_SIZE) {
    return 0;
  }

  uint32_t total_len = pp_pkt_len;
  ezm_pp_uart_pkt_t *uart_pkt = (ezm_pp_uart_pkt_t *)buffer;

  uint8_t *buffer_helper = (uint8_t *)buffer;
  // Calculate and append PEC
  if (uart_pkt->tl_hdr.pec_type == EZM_PP_UART_DRV_SEL_CRC) {
    uart_pkt->tl_hdr.pkt_len += EZM_PP_UART_DRV_SEL_CRC_SIZE;
    total_len += EZM_PP_UART_DRV_SEL_CRC_SIZE;
    uint16_t crc = ezm_htons_u16(ezm_crc16_ccitt((uint8_t *)buffer, pp_pkt_len));
    memcpy((buffer_helper + pp_pkt_len), &crc, sizeof(crc)); 
  }

  print_serial_log_debug("Transmitting %d bytes over UART", total_len);
  if(total_len <64) {
    print_serial_log_buffer(buffer, total_len); 
  }
  ezm_console_send_data(buffer, total_len);

  return total_len;
}

void ezm_pp_uart_drv_listen(void) {
  uint16_t rx_length = 0;
  ezm_console_recv_data(uart_drv_buffer, &rx_length);
  if(rx_length != 0) {
    print_serial_log_debug("Received %d bytes over UART", rx_length);
    print_serial_log_buffer(uart_drv_buffer, rx_length);
    if(ezm_pp_uart_validate_frame(uart_drv_buffer, rx_length)) {
      uart_drv_buffer_len = rx_length - EZM_PP_UART_DRV_SEL_CRC_SIZE;
      ezm_pp_drv_set_msg(EZM_PP_DRV_EV_NEW_MSG, EZM_PP_NO_ACTION, EZM_PP_DRV_SERIAL, 0);
    }
  }
}