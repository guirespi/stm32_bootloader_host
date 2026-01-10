/*
 * ezm_console_arch_siwg917x.c
 *
 *  Created on: Nov 4, 2025
 *      Author: guirespi
 */
#include <stdbool.h>
#include <string.h>

#include "sl_si91x_usart.h"

#include "ezm_log_gen.h"
DEFINE_TAG();

#include "ezm_api_delay.h"
#include "ezm_console_arch_common.h"
#include "ezm_console_arch_internal.h"

#define CONSOLE_ARCH_CHECK_READY() if(console_handler == NULL) return EZM_CONSOLE_ARCH_E_READY;
#define CONSOLE_ARCH_GET_HANDLE ((sl_usart_handle_t)*console_handler)

// From SL
static sl_usart_handle_t *console_handler = NULL;

void __attribute__((weak)) ezm_console_arch_isr_handler(uint32_t event, uint32_t arg) {
	(void)arg;
	print_serial_log_verbose("Console event arrived %lu\n", event);
}

void usart_callback_event(uint32_t event) {
	print_serial_log_verbose("Usart event arrived %lu\n", event);
	ezm_console_event_t event_type = EZM_CONSOLE_EV_UNKNOWN;
	switch (event) 
	{
		case SL_USART_EVENT_SEND_COMPLETE: 
		{
			event_type = EZM_CONSOLE_EV_SEND_COMPLETE;
			break;
		}
		case SL_USART_EVENT_RECEIVE_COMPLETE: 
		{
			event_type = EZM_CONSOLE_EV_RECEIVE_COMPLETE;
			break;	
		}
		default : {
			break;
		}
	}
	ezm_console_arch_isr_handler((uint32_t)event_type, 0);
}

int ezm_console_arch_init(void * console_hdle)
{
	sl_status_t status;
	sl_si91x_usart_control_config_t usart_config;
	
    usart_config.baudrate      = EZM_CONSOLE_BAUDRATE;
    usart_config.mode          = SL_USART_MODE_ASYNCHRONOUS;
    usart_config.parity        = SL_USART_NO_PARITY;
    usart_config.stopbits      = SL_USART_STOP_BITS_1;
    usart_config.hwflowcontrol = SL_USART_FLOW_CONTROL_NONE;
    usart_config.databits      = SL_USART_DATA_BITS_8;
    usart_config.misc_control  = SL_USART_MISC_CONTROL_NONE;
    usart_config.usart_module  = USART_0;
    usart_config.config_enable = ENABLE;
    usart_config.synch_mode    = DISABLE;

	sl_usart_handle_t * usart_handle = (sl_usart_handle_t *)console_hdle;

    status = sl_si91x_usart_init(USART_0, usart_handle);
    if (status != SL_STATUS_OK) {
      print_serial_log_verbose("sl_si91x_usart_initialize: Error Code : 0x%X \n", status);
      return status;
    }
    print_serial_log_verbose("USART initialization is successful \n");
    status = sl_si91x_usart_set_configuration(*usart_handle, &usart_config);
    if (status != SL_STATUS_OK) {
      print_serial_log_verbose("sl_si91x_usart_set_configuration: Error Code : 0x%X \n", status);
      return status;
    }
    print_serial_log_verbose("USART configuration is successful \n");
    status = sl_si91x_usart_multiple_instance_register_event_callback(USART_0, usart_callback_event);
    if (status != SL_STATUS_OK) {
      print_serial_log_verbose("sl_si91x_usart_register_event_callback: Error Code : 0x%X \n", status);
      return status;
    }
    print_serial_log_verbose("USART user event callback registered successfully \n");
    sl_si91x_usart_get_configurations(USART_0, &usart_config);
    print_serial_log_verbose("Console baudrate = %ld \n", usart_config.baudrate);

	if(status == 0)
	{
		console_handler = usart_handle;
	}

	return status == SL_STATUS_OK ? EZM_CONSOLE_ARCH_OK : EZM_CONSOLE_ARCH_E_IO;
}

int ezm_console_arch_send_async(uint8_t * data, uint16_t data_size)
{
	CONSOLE_ARCH_CHECK_READY()
	sl_status_t rt = 0;
	rt = sl_si91x_usart_send_data(CONSOLE_ARCH_GET_HANDLE, data, data_size);
	if(rt != SL_STATUS_OK)
	{
		print_serial_log_verbose("Console USART send error 0x%X\n", rt);
		return EZM_CONSOLE_ARCH_E_IO;
	}
	return EZM_CONSOLE_ARCH_OK;
}

int ezm_console_arch_receive_async(uint8_t * data, uint16_t data_size) 
{
	CONSOLE_ARCH_CHECK_READY()
	sl_status_t rt = sl_si91x_usart_async_receive_data(CONSOLE_ARCH_GET_HANDLE, (void *)(data), data_size);
	if(rt != SL_STATUS_OK)
	{
		print_serial_log_verbose("Console USART receive error 0x%X\n", rt);
		return EZM_CONSOLE_ARCH_E_IO;
	}
	return EZM_CONSOLE_ARCH_OK;
}

int ezm_console_arch_abort_receive(void)
{
	CONSOLE_ARCH_CHECK_READY()
	sl_si91x_usart_control_config_t usart_control = {0};
	usart_control.misc_control = SL_USART_ABORT_RECEIVE;
	int status = sl_si91x_usart_set_configuration(CONSOLE_ARCH_GET_HANDLE, &usart_control);
	if (status != SL_STATUS_OK) {
		print_serial_log_verbose("sl_si91x_usart_set_configuration: Error Code : 0x%X \n", status);
		return EZM_CONSOLE_ARCH_E_IO;
	}
	return EZM_CONSOLE_ARCH_OK;
}
