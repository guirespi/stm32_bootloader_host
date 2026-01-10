/*
 * log_arch_common.c
 *
 *  Created on: Aug 5, 2024
 *      Author: guirespi
 */
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "ezm_log_arch_common.h"

uint32_t ezm_log_arch_common_timestamp(void)
{
	return HAL_GetTick();
}

