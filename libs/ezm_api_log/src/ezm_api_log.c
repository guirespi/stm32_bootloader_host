/*
 * API_log.c
 *
 *  Created on: Aug 5, 2024
 *      Author: guirespi
 */
#include "ezm_api_log.h"
#include "ezm_log_arch_common.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static ezm_log_transmit_f log_transmit = NULL;

/**
 * @brief Write log through log_transmit function.
 *
 * @param format String.
 * @param list Variable list with arguments.
 */
static void log_write_s(const char *format, va_list list);

static void log_write_s(const char *format, va_list list) {
  if (log_transmit != NULL) {
    char buffer[256] = {0};
    uint32_t size = vsnprintf(NULL, 0, format, list);
    size++; // For null termination
    vsnprintf(buffer, size, format, list);
  
    (*log_transmit)((uint8_t *)buffer, (uint16_t)strlen(buffer));
  } else {
    vprintf(format, list);
  }
}

void ezm_log_set_transmit_function(ezm_log_transmit_f transmit_function) {
  log_transmit = transmit_function;
}

/* Bytes per line for hex-dump buffer. This is a copied algorithm */
#define BYTES_PER_LINE 16

void ezm_log_buffer_hexdump(const char *tag, const void *buffer,
                            uint16_t buff_len, ezm_log_level_t log_level) {
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
    for (int i = 0; i < BYTES_PER_LINE; i++) {
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
    for (int i = 0; i < bytes_cur_line; i++) {
      if (isprint((int)ptr_line[i])) {
        ptr_hd += sprintf(ptr_hd, "%c", ptr_line[i]);
      } else {
        ptr_hd += sprintf(ptr_hd, ".");
      }
    }
    ptr_hd += sprintf(ptr_hd, "|");

    EZM_LOG_LEVEL(log_level, tag, "%s", hd_buffer);
    buffer += bytes_cur_line;
    buff_len -= bytes_cur_line;
  } while (buff_len);
}

void ezm_log_write(ezm_log_level_t log_level, const char *tag, const char *format,
                   ...) {
  (void)log_level;
  (void)tag;
  va_list list;
  va_start(list, format);
  log_write_s(format, list);
  va_end(list);
}

uint32_t ezm_log_timestamp(void) { return ezm_log_arch_common_timestamp(); }