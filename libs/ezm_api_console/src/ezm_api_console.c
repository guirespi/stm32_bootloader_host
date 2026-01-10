/*
 * API_console.c
 *
 *  Created on: Aug 4, 2024
 *      Author: guirespi
 */
#include <string.h>
#include "ezm_console_arch_common.h"

#ifndef CONFIG_HOST_BUILD
#include "ezm_api_delay.h"
#endif

#include "ezm_api_console.h"
#include "ezm_console_arch_internal.h"

#include "ezm_ut_utils.h"

#define EZM_CONSOLE_CHECK_READY() if(console_state == EZM_CONSOLE_STATE_DISABLE) return EZM_CONSOLE_ARCH_E_READY;

#ifndef CONFIG_HOST_BUILD
STATIC_GLOBAL ezm_delay_t console_listen_tmo;
#endif
STATIC_GLOBAL volatile ezm_console_buffer_t console_buffer;
STATIC_GLOBAL ezm_console_state_t console_state;

/**
 * @brief Resets the console buffer to its initial state
 *
 * @param[in,out] console_buffer Pointer to the console buffer structure to reset
 */
static inline void ezm_console_reset_buffer(volatile ezm_console_buffer_t * console_buffer);
/**
 * @brief Advances the buffer index to the next position
 *
 * @param[in,out] console_buffer Pointer to the console buffer structure to advance
 */
static inline void ezm_console_advance_buffer_index(volatile ezm_console_buffer_t * console_buffer);
/**
 * @brief Retrieves data from the console buffer
 *
 * @param[in] console_buffer Pointer to the console buffer structure to read from
 * @param[out] buffer Pointer to destination buffer where data will be copied
 * @param[out] data_size Pointer to variable that will store the amount of data copied
 */
static inline void ezm_console_get_buffer_data(volatile ezm_console_buffer_t * console_buffer, uint8_t * buffer, uint16_t * data_size);
/**
 * @brief Gets a pointer to the current position in the console buffer
 *
 * @param[in] console_buffer Pointer to the console buffer structure
 * @return Pointer to the current position in the buffer
 */
static inline uint8_t * ezm_console_get_current_buffer_pointer(volatile ezm_console_buffer_t * console_buffer);

static inline uint8_t * ezm_console_get_current_buffer_pointer(volatile ezm_console_buffer_t * console_buffer)
{
	return (void *)(console_buffer->data + console_buffer->index);
}

static inline void ezm_console_reset_buffer(volatile ezm_console_buffer_t * console_buffer)
{
	console_buffer->index = 0;
	memset((void *)console_buffer->data, 0, EZM_CONSOLE_DATA_MAX_SIZE);
}

static inline void ezm_console_get_buffer_data(volatile ezm_console_buffer_t * console_buffer, uint8_t * buffer, uint16_t * data_size)
{
	*data_size = console_buffer->index;
	memcpy(buffer, (void *)console_buffer->data, *data_size);
	console_buffer->index = 0;
}

static inline void ezm_console_advance_buffer_index(volatile ezm_console_buffer_t * console_buffer)
{
	console_buffer->index++;
}

void ezm_console_arch_isr_handler(uint32_t event, uint32_t arg)
{
#ifndef CONFIG_HOST_BUILD
	(void)arg;
	switch (event) 
	{
		case EZM_CONSOLE_EV_SEND_COMPLETE:
			break;
		case EZM_CONSOLE_EV_RECEIVE_COMPLETE:
			ezm_console_advance_buffer_index(&console_buffer);
			if(ezm_delay_read(&console_listen_tmo))
			{
				console_state = EZM_CONSOLE_STATE_RECV_COMPLETE;
			}
			else
			{
				int rt = ezm_console_arch_receive_async((void *)ezm_console_get_current_buffer_pointer(&console_buffer), EZM_CONSOLE_LISTEN_SIZE);
				if(rt != EZM_CONSOLE_ARCH_OK)
				{
					console_state = EZM_CONSOLE_STATE_ERROR;
				}
				else
				{
					ezm_delay_init(&console_listen_tmo, EZM_CONSOLE_LISTEN_TIMEOUT_MS);
				}
			}
			break;
		default :
			break;
	}
#endif
}

int ezm_console_init(ezm_console_hdle console_hdle)
{
	int rt = ezm_console_arch_init((void *)console_hdle);
	if(rt == EZM_CONSOLE_ARCH_OK) {
		console_state = EZM_CONSOLE_STATE_INIT;
	}
	return rt;
}

int ezm_console_send_data(uint8_t * data, uint16_t data_size)
{
	return ezm_console_arch_send_async(data, data_size);
}

int ezm_console_recv_data(uint8_t * buffer, uint16_t * recv_length)
{
#ifndef CONFIG_HOST_BUILD
	if(buffer == NULL || recv_length == NULL)
	{
		return EZM_CONSOLE_ARCH_E_IO;
	}
	
	int rt = EZM_CONSOLE_ARCH_OK;
	switch(console_state)
	{
		case EZM_CONSOLE_STATE_DISABLE: 
		case EZM_CONSOLE_STATE_ERROR:
		{
			rt = EZM_CONSOLE_ARCH_E_IO;
			break;
		}
		case EZM_CONSOLE_STATE_LISTEN:
		{
			rt = EZM_CONSOLE_ARCH_E_BUSY;
			if(ezm_delay_read(&console_listen_tmo))
			{
				rt = ezm_console_arch_abort_receive();
				if(rt == EZM_CONSOLE_ARCH_OK) {
					ezm_console_get_buffer_data(&console_buffer, buffer, recv_length);
					console_state = EZM_CONSOLE_STATE_READY;
				}
			}
			break;
		}
		case EZM_CONSOLE_STATE_RECV_COMPLETE:
		{
			rt = EZM_CONSOLE_ARCH_OK;
			ezm_console_get_buffer_data(&console_buffer, buffer, recv_length);
			console_state = EZM_CONSOLE_STATE_READY;
			break;
		}
		default:
		{
			ezm_console_reset_buffer(&console_buffer);
			rt = ezm_console_arch_receive_async((void *)ezm_console_get_current_buffer_pointer(&console_buffer), EZM_CONSOLE_LISTEN_SIZE);
			if(rt == 0)
			{
				console_state = EZM_CONSOLE_STATE_LISTEN;
				ezm_delay_init(&console_listen_tmo, EZM_CONSOLE_LISTEN_TIMEOUT_MS);
			} else {
				rt = EZM_CONSOLE_ARCH_E_IO;
			}
			break;
		}
	}
	return rt;
#else
	*recv_length = ezm_console_arch_receive_async(buffer, EZM_CONSOLE_LISTEN_SIZE);
	if(*recv_length > 0) {
		return EZM_CONSOLE_ARCH_OK;
	} else {
		return EZM_CONSOLE_ARCH_E_IO;
	}
#endif
}
