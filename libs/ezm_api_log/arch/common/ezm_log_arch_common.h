/*
 * log_arch_common.h
 *
 *  Created on: Aug 5, 2024
 *      Author: guirespi
 */

#ifndef EZM_LOG_ARCH_COMMON_H_
#define EZM_LOG_ARCH_COMMON_H_

#include <stdint.h>

/**
 * @brief Get arch specific timestamp.
 *
 * @return Arch specific timestamp
 */
uint32_t ezm_log_arch_common_timestamp(void);

#endif /*  EZM_LOG_ARCH_COMMON_H_ */
