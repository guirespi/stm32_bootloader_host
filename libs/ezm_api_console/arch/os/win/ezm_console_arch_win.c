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

static HANDLE hSerial;

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
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutConstant = 200;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 200;
    timeouts.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        print_serial_log_error("Error: SetCommTimeouts failed");
        return EZM_CONSOLE_ARCH_E_IO;
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
    } else {
        print_serial_log_verbose("%d bytes written", (int)bytesWritten);
        return EZM_CONSOLE_ARCH_OK;
    }
}

int ezm_console_arch_receive_async(uint8_t * data, uint16_t data_size) 
{
    DWORD bytesRead = 0;

    BOOL success = ReadFile(
        hSerial,
        data,
        data_size, // Leave space for null terminator
        &bytesRead,
        NULL // Must be NULL for non-overlapped I/O
    );

    if (!success) {
        print_serial_log_verbose("Error: ReadFile failed");
    } else {
        print_serial_log_verbose("%d bytes read", (int)bytesRead);
    }
    return bytesRead;
}

int ezm_console_arch_abort_receive(void)
{
    return 0;
}
