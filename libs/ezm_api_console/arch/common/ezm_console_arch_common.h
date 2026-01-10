/*
 * console_arch_common.h
 *
 *  Created on: Aug 4, 2024
 *      Author: guirespi
 */

#ifndef API_API_CONSOLE_ARCH_COMMON_CONSOLE_ARCH_COMMON_H_
#define API_API_CONSOLE_ARCH_COMMON_CONSOLE_ARCH_COMMON_H_

#include <stdint.h>
#include "ezm_api_console_def.h"

typedef enum
{
	EZM_CONSOLE_STATE_DISABLE		= 0,
	EZM_CONSOLE_STATE_INIT,
	EZM_CONSOLE_STATE_READY,
	EZM_CONSOLE_STATE_LISTEN,
	EZM_CONSOLE_STATE_RECV_COMPLETE,
	EZM_CONSOLE_STATE_ERROR,
} ezm_console_state_t;

typedef enum
{
	EZM_CONSOLE_ARCH_OK = 0, /*< No error */
	EZM_CONSOLE_ARCH_E_READY, /*< Operation is in progress or is already set */
	EZM_CONSOLE_ARCH_E_BUSY	, /*< Ongoing operation */
	EZM_CONSOLE_ARCH_E_IO	, /*< Error related to input/output of arch related functions */
}ezm_console_arch_err_t;

/**
 * @brief Initializes the console architecture-specific components
 * @param console_hdle Handle to console instance
 * @return int Returns 0 on success, otherwise error code on failure
 */
int ezm_console_arch_init(void * console_hdle);
/**
 * @brief Sends data asynchronously through the console
 * @param data Pointer to the data buffer to send
 * @param data_size Size of data to send in bytes
 * @return int Returns 0 on success, otherwise error code on failure
 */
int ezm_console_arch_send_async(uint8_t * data, uint16_t data_size);
/**
 * @brief Receives data asynchronously through the console
 * @param data Pointer to the buffer where received data will be stored
 * @param data_size Maximum size of data to receive in bytes
 * @return int Returns 0 on success, otherwise error code on failure
 */
int ezm_console_arch_receive_async(uint8_t * data, uint16_t data_size);
/**
 * @brief Aborts any ongoing receive operation
 * @return int Returns 0 on success, otherwise error code on failure
 */
int ezm_console_arch_abort_receive(void);

#endif /* API_API_CONSOLE_ARCH_COMMON_CONSOLE_ARCH_COMMON_H_ */
