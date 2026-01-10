#ifndef EZM_PP_UART_DRV_H_
#define EZM_PP_UART_DRV_H_

#include <stdint.h>

#define EZM_PP_UART_DRV_SEL_CRC (EZM_PP_UART_DRV_PEC_TYPE_CITT)
#define EZM_PP_UART_DRV_SEL_CRC_SIZE (EZM_PP_UART_DRV_CRC16_SIZE)
#define EZM_PP_UART_DRV_MAGIC_BYTE (0x55)
#define EZM_PP_UART_DRV_HDR_VERSION (0x00)
#define EZM_PP_UART_DRV_PEC_TYPE_SMBUS (0x00) // CRC-8
#define EZM_PP_UART_DRV_PEC_TYPE_CITT (0x01)  // CRC-16
#define EZM_PP_UART_DRV_CRC8_SIZE (1)
#define EZM_PP_UART_DRV_CRC16_SIZE (2)

#endif // EZM_PP_UART_DRV_H_

uint32_t ezm_pp_uart_drv_fill_header(void *buffer, uint32_t pp_pkt_len);
uint32_t ezm_pp_uart_drv_read(void *buffer, uint32_t buffer_len);
uint32_t ezm_pp_uart_drv_transmit(void *buffer, uint32_t pp_pkt_len);
void ezm_pp_uart_drv_listen(void);