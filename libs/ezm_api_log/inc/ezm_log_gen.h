#ifndef _EZM_LOG_GEN_H_
#define _EZM_LOG_GEN_H_
#include <stdlib.h>

#ifndef __FILENAME__
#define __FILENAME__ (__builtin_strrchr("/" __FILE__, '/') + 1)
#endif

#if defined(CONFIG_LOGS_ENABLE)
/**
 * @brief Defines a tag to use for logs
 * within the file. Call this just after including
 * this header
 *
 */
#define DEFINE_TAG() static const char *TAG = __FILENAME__
#include "ezm_api_log.h"
#define print_serial_log(format, ...) \
    EZM_LOG_LEVEL(EZM_LOG_INFO, TAG, "%d " format "\n", __LINE__, ##__VA_ARGS__)
#define print_serial_log_warning(format, ...) \
    EZM_LOG_LEVEL(EZM_LOG_WARN, TAG, "%d " format "\n", __LINE__, ##__VA_ARGS__)
#define print_serial_log_error(format, ...) \
    EZM_LOG_LEVEL(EZM_LOG_ERROR, TAG, "%d " format "\n", __LINE__, ##__VA_ARGS__)
#define print_serial_log_debug(format, ...) \
    EZM_LOG_LEVEL(EZM_LOG_DEBUG, TAG, "%d " format "\n", __LINE__, ##__VA_ARGS__)
#define print_serial_log_verbose(format, ...) \
    EZM_LOG_LEVEL(EZM_LOG_VERBOSE, TAG, "%d " format "\n", __LINE__, ##__VA_ARGS__)
#define print_serial_log_buffer(data, datalen) EZM_LOG_HEXDUMP(TAG, data, datalen, EZM_LOG_DEBUG);
#else
#define DEFINE_TAG()
#define print_serial_log(format, ...)
#define print_serial_log_warning(format, ...)
#define print_serial_log_error(format, ...)
#define print_serial_log_debug(format, ...)
#define print_serial_log_verbose(format, ...)
#define print_serial_log_buffer(data, datalen)
#endif

#endif /* _EZM_LOG_GEN_H_ */