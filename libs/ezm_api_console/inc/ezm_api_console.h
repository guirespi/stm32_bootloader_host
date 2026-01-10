#ifndef EZM_API_CONSOLE_H_
#define EZM_API_CONSOLE_H_

#include <stdint.h>
#include "ezm_api_console_def.h"

typedef void * ezm_console_hdle;

/**
 * @brief Init console.
 *
 * @param comm_channel_hdle Communication channel hanndle. (Expected an UART handle)
 * @return
 * 			- 0 if no error.
 */
int ezm_console_init(ezm_console_hdle comm_channel_hdle);
/**
 * @brief Send data through console.
 *
 * @param data Data
 * @param data_size Data size.
 * @return
 * 			- 0 if no error.
 */
int ezm_console_send_data(uint8_t * data, uint16_t data_size);
/**
 * @brief Receive variable data size through console.
 *
 * @param buffer Buffer.
 * @param recv_length Pointer where received length will be put.
 * @return
 * 			- 0 if received finish.
 */
int ezm_console_recv_data(uint8_t * buffer, uint16_t * recv_length);

#endif /* EZM_API_CONSOLE_H_ */
