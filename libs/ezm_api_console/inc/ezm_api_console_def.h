#ifndef EZM_API_CONSOLE_DEF_H_
#define EZM_API_CONSOLE_DEF_H_

#define EZM_CONSOLE_DATA_MAX_SIZE (5*1024) /*! Maximum data size for console buffer */
#define EZM_CONSOLE_BAUDRATE (115200) /*! Console default baudrate */
#ifndef CONFIG_HOST_BUILD
#define EZM_CONSOLE_LISTEN_SIZE (1) /*! Number of bytes to listen for ISR */
#else
#define EZM_CONSOLE_LISTEN_SIZE (EZM_CONSOLE_DATA_MAX_SIZE) /*! Number of bytes to listen for ISR */
#endif
#define EZM_CONSOLE_LISTEN_TIMEOUT_MS (100) /*! Timeout to consider end of reception in ms for each byte */

typedef struct{
    volatile uint8_t data[EZM_CONSOLE_DATA_MAX_SIZE];
    volatile uint32_t index : 16;
    uint32_t reserved : 16;
} ezm_console_buffer_t;

typedef enum __attribute__((__packed__))
{
	EZM_CONSOLE_EV_SEND_COMPLETE = 0, /*< Async send complete */
	EZM_CONSOLE_EV_RECEIVE_COMPLETE = 1, /*< Async receive complete */
	EZM_CONSOLE_EV_UNKNOWN, /*< Unknown event by driver */
} ezm_console_event_t;

#endif /* EZM_API_CONSOLE_DEF_H_ */
