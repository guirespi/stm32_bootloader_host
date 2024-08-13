/*
 * API_log.c
 *
 *  Created on: Aug 5, 2024
 *      Author: guirespi
 */
#define __GLIBC_USE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "API_log.h"
#include "log_arch_common.h"

static log_transmit_f log_transmit = NULL;

static void log_write_s(const char * format, va_list list)
{
	char * buffer = NULL;
	vasprintf(&buffer, format, list);

	if(log_transmit != NULL)
		(*log_transmit)((uint8_t *)buffer, (uint16_t)strlen(buffer));

	if(buffer != NULL)
		free(buffer);
}

void log_set_transmit_function(log_transmit_f transmit_function)
{
	log_transmit = transmit_function;
}

/* Bytes per line for hex-dump buffer */
#define BYTES_PER_LINE 16

void log_buffer_hexdump(const char *tag, const void *buffer, uint16_t buff_len, log_level_t log_level)
{
    if (buff_len == 0) {
        return;
    }
    const uint8_t *ptr_line;
    char hd_buffer[10 + 3 + BYTES_PER_LINE * 3 + 3 + BYTES_PER_LINE + 1 + 1];
    char *ptr_hd;
    int bytes_cur_line;

    do {
        if (buff_len > BYTES_PER_LINE) {
            bytes_cur_line = BYTES_PER_LINE;
        } else {
            bytes_cur_line = buff_len;
        }

        ptr_line = buffer;

        ptr_hd = hd_buffer;

        ptr_hd += sprintf(ptr_hd, "%p ", buffer);
        for (int i = 0; i < BYTES_PER_LINE; i ++) {
            if ((i & 7) == 0) {
                ptr_hd += sprintf(ptr_hd, " ");
            }
            if (i < bytes_cur_line) {
                ptr_hd += sprintf(ptr_hd, " %02x", ptr_line[i]);
            } else {
                ptr_hd += sprintf(ptr_hd, "   ");
            }
        }
        ptr_hd += sprintf(ptr_hd, "  |");
        for (int i = 0; i < bytes_cur_line; i ++) {
            if (isprint((int)ptr_line[i])) {
                ptr_hd += sprintf(ptr_hd, "%c", ptr_line[i]);
            } else {
                ptr_hd += sprintf(ptr_hd, ".");
            }
        }
        ptr_hd += sprintf(ptr_hd, "|");

        LOG_LEVEL(log_level, tag, "%s", hd_buffer);
        buffer += bytes_cur_line;
        buff_len -= bytes_cur_line;
    } while (buff_len);
}

void log_write(log_level_t log_level, const char * tag, const char * format, ...)
{
    va_list list;
    va_start(list, format);
    log_write_s(format, list);
    va_end(list);
}

uint32_t log_timestamp(void)
{
	return log_arch_common_timestamp();
}

