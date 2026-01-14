/*
 * ezm_console_arch_siwg917x.c
 *
 *  Created on: Nov 4, 2025
 *      Author: guirespi
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>

#include "ezm_log_gen.h"
DEFINE_TAG();

#include "ezm_console_arch_common.h"
#include "ezm_console_arch_internal.h"

#define CONSOLE_ARCH_BUF_SIZE (1024*5)

static HANDLE listener_thread;
static HANDLE parser_thread;
static HANDLE hSerial;

static volatile uint8_t rx_buf[CONSOLE_ARCH_BUF_SIZE];
static uint8_t output_buf[CONSOLE_ARCH_BUF_SIZE];
static uint16_t output_length;
static uint16_t output_index;
static uint16_t expected_len;
static volatile size_t rx_head = 0;
static size_t rx_tail = 0;
CRITICAL_SECTION rx_lock;

typedef enum {
    WAIT_SYNC,
    WAIT_LEN,
    WAIT_PAYLOAD,
} parser_state_t;

static parser_state_t state = WAIT_SYNC;
static volatile bool pending = false;

void rx_push(uint8_t *data, size_t len)
{
    EnterCriticalSection(&rx_lock);
    for (size_t i = 0; i < len; i++)
    {
        rx_buf[rx_head] = data[i];
        rx_head = (rx_head + 1) % CONSOLE_ARCH_BUF_SIZE;
    }
    LeaveCriticalSection(&rx_lock);
}

static DWORD WINAPI uart_thread(void *arg)
{
    HANDLE hCom = (HANDLE)arg;
    uint8_t temp[256];
    DWORD bytes;

    while (1)
    {
        if (ReadFile(hCom, temp, sizeof(temp), &bytes, NULL))
        {
            if (bytes > 0)
                rx_push(temp, bytes);   // NEVER parse here
        }
    }
}

static DWORD WINAPI parse_rx(void *arg)
{
    while (true)
    {
        if(rx_head != rx_tail) {
            EnterCriticalSection(&rx_lock);
            uint8_t b = rx_buf[rx_tail];
            rx_tail = (rx_tail + 1) % CONSOLE_ARCH_BUF_SIZE;
            LeaveCriticalSection(&rx_lock);
            
            switch (state)
            {
            case WAIT_SYNC:
                if (b == 0x55)
                {
                    output_buf[0] = b;
                    output_index = 1;
                    state = WAIT_LEN;
                }
                break;
    
            case WAIT_LEN:
                output_buf[output_index++] = b;
                if(output_index > 4) {
                    expected_len = *(uint16_t *)&output_buf[2];
                    state = WAIT_PAYLOAD;
                }
                break;
    
            case WAIT_PAYLOAD:
                output_buf[output_index++] = b;
                if (output_index >= (expected_len)) {
                    pending = true;
                    while(pending){
                        Sleep(10);
                    }
                    state = WAIT_SYNC;
                }
                break;
            }
        }
    }
}

void __attribute__((weak)) ezm_console_arch_isr_handler(uint32_t event, uint32_t arg) {
	(void)arg;
	print_serial_log_verbose("Console event arrived %lu\n", event);
}

void usart_callback_event(uint32_t event) {
	ezm_console_arch_isr_handler((uint32_t)event, 0);
}

int ezm_console_arch_init(void * console_hdle)
{
    // LPCSTR portName = "\\\\.\\COM7"; // Replace COM3 with your actual port
    LPCSTR portName = (LPCSTR)console_hdle;
    print_serial_log("Oppening %s", portName);

    hSerial = CreateFile(
        portName,
        GENERIC_READ | GENERIC_WRITE, // Access mode: read and write
        0,                            // Share mode: must be 0 for comm devices
        NULL,                         // Security attributes: default
        OPEN_EXISTING,                // Creation disposition: must be OPEN_EXISTING
        FILE_ATTRIBUTE_NORMAL,        // Flags/attributes: can use FILE_FLAG_OVERLAPPED for async I/O
        NULL                          // Template file: NULL for comm devices
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            // Handle error: serial port not found
            print_serial_log_error("Error: COM port not found");
        } else {
            // Handle other errors (e.g., access denied if already open)
            print_serial_log_error("Error: Could not open COM port, GetLastError() = %d", GetLastError());
        }
        return EZM_CONSOLE_ARCH_E_IO; 
    }

    DCB dcbSerialParams;
    ZeroMemory(&dcbSerialParams, sizeof(dcbSerialParams));
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        print_serial_log_error("Error: GetCommState failed");
        return EZM_CONSOLE_ARCH_E_IO;
    } else {
        dcbSerialParams.fInX  = FALSE;
        dcbSerialParams.fOutX = FALSE; 
        dcbSerialParams.BaudRate = CBR_115200;      // Set baud rate (e.g., 9600)
        dcbSerialParams.ByteSize = 8;             // Data bits
        dcbSerialParams.Parity = NOPARITY;        // Parity
        dcbSerialParams.StopBits = ONESTOPBIT;    // Stop bits
        dcbSerialParams.fBinary = TRUE;           // Enable binary mode

        if (!SetCommState(hSerial, &dcbSerialParams)) {
            print_serial_log_error("Error: SetCommState failed");
            return EZM_CONSOLE_ARCH_E_IO;
        }
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 200;
    timeouts.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        print_serial_log_error("Error: SetCommTimeouts failed");
        return EZM_CONSOLE_ARCH_E_IO;
    }


    if(listener_thread == NULL && parser_thread == NULL) {
        listener_thread = CreateThread(NULL, 0, uart_thread, hSerial, 0, NULL);
        parser_thread = CreateThread(NULL, 0, parse_rx, NULL, 0, NULL);
        InitializeCriticalSection(&rx_lock);
    }
    return EZM_CONSOLE_ARCH_OK;
}

int ezm_console_arch_send_async(uint8_t * data, uint16_t data_size)
{
    DWORD bytesWritten;

    BOOL success = WriteFile(
        hSerial,
        data,
        data_size,
        &bytesWritten,
        NULL // Must be NULL for non-overlapped I/O
    );

    if (!success) {
        print_serial_log_verbose("Error: WriteFile failed");\
        return EZM_CONSOLE_ARCH_E_IO;
    }
    return EZM_CONSOLE_ARCH_OK;
}

int ezm_console_arch_receive_async(uint8_t * data, uint16_t data_size) 
{
    int ret_size = 0; 
    if(pending) {
        memcpy(data, output_buf, output_index);
        ret_size = output_index;
        pending = false;
    }
    return ret_size;
}

int ezm_console_arch_abort_receive(void)
{
    return 0;
}
