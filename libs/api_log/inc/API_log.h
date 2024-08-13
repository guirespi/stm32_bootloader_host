/*
 * API_log.h
 *
 *  Created on: Aug 5, 2024
 *      Author: guirespi
 */

#ifndef API_API_LOG_INC_API_LOG_H_
#define API_API_LOG_INC_API_LOG_H_

#include <stdint.h>

typedef enum
{
	LOG_NONE = 0,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
}log_level_t;

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)

/* Format used by Espressif's logs */
#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%u) %s: " format LOG_RESET_COLOR "\r\n"

typedef void (*log_transmit_f)(uint8_t * data, uint16_t data_len);

void log_set_transmit_function(log_transmit_f transmit_function);

void log_buffer_hexdump(const char *tag, const void *buffer, uint16_t buff_len, log_level_t log_level);

void log_write(log_level_t log_level, const char * tag, const char * format, ...);

uint32_t log_timestamp(void);

/* Macro used to write logs with the Espressif's format.*/
#define LOG_LEVEL(level, tag, format, ...) do {                        \
        if (level== LOG_ERROR )         { log_write(LOG_ERROR,      	tag, LOG_FORMAT(E, format), log_timestamp(), tag, ##__VA_ARGS__); 	} \
        else if (level==LOG_WARN )      { log_write(LOG_WARN,       	tag, LOG_FORMAT(W, format), log_timestamp(), tag, ##__VA_ARGS__); 	} \
        else                            { log_write(LOG_INFO,       	tag, LOG_FORMAT(I, format), log_timestamp(), tag, ##__VA_ARGS__);	} \
    } while(0)


#define LOG_HEXDUMP( tag, buffer, buff_len, level ) \
    do { \
            log_buffer_hexdump( tag, buffer, buff_len, level); \
    } while(0)

#endif /* API_API_LOG_INC_API_LOG_H_ */
