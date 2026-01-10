/*
 * API_log.h
 *
 *  Created on: Aug 5, 2024
 *      Author: guirespi
 */

#ifndef API_API_LOG_INC_API_LOG_H_
#define API_API_LOG_INC_API_LOG_H_

#include <stdint.h>

#ifndef EZM_LOG_LEVEL_CONFIG
#define EZM_LOG_LEVEL_CONFIG EZM_LOG_VERBOSE
#endif

typedef enum
{
	EZM_LOG_NONE = 0,   /* No log showed */
	EZM_LOG_ERROR,      /* Error logs */
	EZM_LOG_WARN,       /* Warn logs */
	EZM_LOG_INFO,       /* Info logs */
	EZM_LOG_DEBUG,      /* Debug logs */
	EZM_LOG_VERBOSE,    /* Debug logs */
}ezm_log_level_t;

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
#define LOG_COLOR_D
#define LOG_COLOR_V       LOG_COLOR(LOG_COLOR_CYAN)

/* Format used by Espressif's logs */
#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%lu) %s:" format LOG_RESET_COLOR "\r\n"

/**
 * @brief Function used to transmit logs.
 *
 * @param data Data to transmit (string)
 * @param data_len Data length to transmit.
 */
typedef void (*ezm_log_transmit_f)(uint8_t * data, uint16_t data_len);

/**
 * @brief Set function which will be used to transmit logs.
 *
 * @param transmit_function Transmit function.
 */
void ezm_log_set_transmit_function(ezm_log_transmit_f transmit_function);
/**
 * @brief Log of hexdump.
 *
 * @param tag Tag
 * @param buffer Buffer to hexdump.
 * @param buff_len Length of buffer.
 * @param log_level Log level.
 */
void ezm_log_buffer_hexdump(const char *tag, const void *buffer, uint16_t buff_len, ezm_log_level_t log_level);
/**
 * @brief Write log.
 *
 * @param log_level Log level
 * @param tag Tag
 * @param format String
 */
void ezm_log_write(ezm_log_level_t log_level, const char * tag, const char * format, ...);
/**
 * @brief Get log timestamp.
 *
 * @return Timestamp.
 */
uint32_t ezm_log_timestamp(void);

/* Macro used to write logs with the Espressif's format.*/
#define EZM_LOG_LEVEL(level, tag, format, ...) do {                                                                                               \
        if ( level > EZM_LOG_LEVEL_CONFIG )    {break;}                                                                                                \
        else if ( level == EZM_LOG_ERROR )     { ezm_log_write(EZM_LOG_ERROR, tag, LOG_FORMAT(E, format), ezm_log_timestamp(), tag, ##__VA_ARGS__); 	} \
        else if ( level == EZM_LOG_WARN )      { ezm_log_write(EZM_LOG_WARN,  tag, LOG_FORMAT(W, format), ezm_log_timestamp(), tag, ##__VA_ARGS__); 	} \
        else if ( level == EZM_LOG_DEBUG )     { ezm_log_write(EZM_LOG_DEBUG, tag, LOG_FORMAT(D, format), ezm_log_timestamp(), tag, ##__VA_ARGS__); 	} \
        else if ( level == EZM_LOG_VERBOSE )   { ezm_log_write(EZM_LOG_VERBOSE, tag, LOG_FORMAT(V, format), ezm_log_timestamp(), tag, ##__VA_ARGS__);} \
        else                              { ezm_log_write(EZM_LOG_INFO,  tag, LOG_FORMAT(I, format), ezm_log_timestamp(), tag, ##__VA_ARGS__);	} \
    } while(0)


#define EZM_LOG_HEXDUMP( tag, buffer, buff_len, level ) \
    do { \
            ezm_log_buffer_hexdump( tag, buffer, buff_len, level); \
    } while(0)

#endif /* API_API_LOG_INC_API_LOG_H_ */
