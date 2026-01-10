/*
 * arch_stm32f4xx.c
 *
 *  Created on: Aug 4, 2024
 *      Author: guirespi
 */
#include <stdbool.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#include "ezm_console_arch_common.h"

#define CONSOLE_ARCH_UART_TRANSMIT_TIMEOUT (100)
#define CONSOLE_ARCH_UART_BYTE_RECV_TIMEOUT_MS (100)
#define CONSOLE_ARCH_UART_LISTEN_BYTE_SIZE (1)
#define CONSOLE_ARCH_CHECK_READY() if(uart_handle == NULL || console_state == EZM_CONSOLE_STATE_DISABLE) return CONSOLE_ARCH_E_READY;

typedef struct
{
	uint32_t 	start_time; /* Start of timeout from HAL_tick*/
	uint32_t 	duration; /* Duration for timeout */
	bool 		running; /* Is timeout running? */
}ezm_console_timeout_t;

static UART_HandleTypeDef * uart_handle = NULL;
static volatile ezm_console_state_t console_state = EZM_CONSOLE_STATE_DISABLE;

static volatile uint8_t console_buffer[CONSOLE_MAX_RECV_SIZE] = {0};
static volatile uint16_t console_buffer_index = 0;
static volatile ezm_console_timeout_t console_timeout = {0};

/**
 * @brief Copy console buffer into user buffer.
 *
 * @param console_buffer Console buffer.
 * @param console_buffer_size Received data in console buffer.
 * @param user_buffer User buffer.
 * @param recv_len Pointer where the value of 'console_buffer_size' will be copy.
 */
static void ezm_console_copy_console_buffer_to_user_buffer(volatile uint8_t * console_buffer, volatile uint16_t console_buffer_size, uint8_t * user_buffer, uint16_t * recv_len);
/**
 * @brief Start a console timeout.
 *
 * @param duration_ms milliseconds timeout.
 */
static void ezm_console_timeout_start(uint32_t duration_ms);
/**
 * @brief Read the console timeout
 *
 * @return true: timeout reached. false: timeout not reached.
 */
static bool ezm_console_timeout_read(void);

static void ezm_console_copy_console_buffer_to_user_buffer(volatile uint8_t * console_buffer, volatile uint16_t console_buffer_size, uint8_t * user_buffer, uint16_t * recv_len)
{
	memcpy(user_buffer, (void *)console_buffer, console_buffer_size);
	*recv_len = console_buffer_size;
}

static void ezm_console_timeout_start(uint32_t duration_ms)
{
	console_timeout.duration = duration_ms;
	console_timeout.start_time = HAL_GetTick();
	console_timeout.running = true;
}

static bool ezm_console_timeout_read(void)
{
	bool rt = false;
	if(console_timeout.running == true)
	{
		rt = ((HAL_GetTick() - console_timeout.start_time) > console_timeout.duration);
		if(rt)
			console_timeout.running = false;
	}
	return rt;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == uart_handle->Instance)
	{
		console_buffer_index++;
		if(ezm_console_timeout_read())
		{
			console_state = CONSOLE_STATE_RECV_COMPLETE;
		}
		else
		{
			 int rt = HAL_UART_Receive_IT(uart_handle, (void *)console_buffer + console_buffer_index, CONSOLE_ARCH_UART_LISTEN_BYTE_SIZE);
			 if(rt != 0)
			 {
				 console_state = CONSOLE_STATE_ERROR;
			 }
			 else
			 {
				ezm_console_timeout_start(CONSOLE_ARCH_UART_BYTE_RECV_TIMEOUT_MS);
			 }
		}
	}
}

int ezm_console_arch_common_comm_channel_init(void * console_handler)
{
	if(uart_handle != NULL) return CONSOLE_ARCH_OK;
	int rt = 0;

	UART_HandleTypeDef * UartHandle = (UART_HandleTypeDef *) console_handler;

	/* Initialize UART hardware */
	/* UART configuration */
	UartHandle->Init.BaudRate = CONSOLE_UART_BAUDRATE;
	UartHandle->Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle->Init.StopBits = UART_STOPBITS_1;
	UartHandle->Init.Parity = UART_PARITY_NONE;
	UartHandle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartHandle->Init.Mode = UART_MODE_TX_RX;
	UartHandle->Init.OverSampling = UART_OVERSAMPLING_16;

	/* Initialize UART */
	rt = HAL_UART_Init(UartHandle);

	if(rt == 0)
	{
		uart_handle = UartHandle;
		console_state = CONSOLE_STATE_INIT;
	}

	return rt;
}

int ezm_console_arch_common_comm_channel_send(uint8_t * data, uint16_t data_size)
{
	CONSOLE_ARCH_CHECK_READY()
	int rt = 0;

	rt = HAL_UART_Transmit(uart_handle, data, data_size, CONSOLE_ARCH_UART_TRANSMIT_TIMEOUT);

	return rt;
}

int ezm_console_arch_common_comm_channel_receive(uint8_t * data, uint16_t * data_size)
{
	CONSOLE_ARCH_CHECK_READY()
	int rt = CONSOLE_ARCH_OK;

	switch(console_state)
	{
		case CONSOLE_STATE_LISTEN:
		{
			rt = CONSOLE_ARCH_E_BUSY;
			if(ezm_console_timeout_read())
			{
				rt = CONSOLE_ARCH_OK;
				HAL_UART_AbortReceive_IT(uart_handle);
				ezm_console_copy_console_buffer_to_user_buffer(console_buffer, console_buffer_index, data, data_size);
				console_state = CONSOLE_STATE_READY;
			}
			break;
		}
		case CONSOLE_STATE_RECV_COMPLETE:
		{
			rt = CONSOLE_ARCH_OK;
			ezm_console_copy_console_buffer_to_user_buffer(console_buffer, console_buffer_index, data, data_size);
			console_state = CONSOLE_STATE_READY;
			break;
		}
		case CONSOLE_STATE_ERROR:
		{
			rt = CONSOLE_ARCH_E_IO;
			break;
		}
		default:
		{
			console_buffer_index = 0;
			memset((void *)console_buffer, 0, CONSOLE_MAX_RECV_SIZE);
			rt = HAL_UART_Receive_IT(uart_handle, (uint8_t *)(console_buffer + console_buffer_index), CONSOLE_ARCH_UART_LISTEN_BYTE_SIZE);
			if(rt == 0)
			{
				console_state = CONSOLE_STATE_LISTEN;
				ezm_console_timeout_start(CONSOLE_ARCH_UART_BYTE_RECV_TIMEOUT_MS);
			}
			break;
		}
	}
	return rt;
}
