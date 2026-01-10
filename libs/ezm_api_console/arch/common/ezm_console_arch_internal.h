#ifndef EZM_CONSOLE_ARCH_INTERNAL_H_
#define EZM_CONSOLE_ARCH_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Console architecture specific ISR handler.
 * 
 * @details This is a weak function that can be overridden by the user to handle console events.
 *
 * @param event Event. More info in ezm_console_event_t enum.
 * @param arg Argument.
 */
void ezm_console_arch_isr_handler(uint32_t event, uint32_t arg);

#ifdef __cplusplus
}
#endif

#endif /* EZM_CONSOLE_ARCH_INTERNAL_H_ */