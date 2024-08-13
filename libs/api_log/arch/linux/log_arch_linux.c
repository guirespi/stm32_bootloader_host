/*
 * log_arch_common.c
 *
 *  Created on: Aug 5, 2024
 *      Author: guirespi
 */
#include <stdint.h>
#include <time.h>

#include "log_arch_common.h"

uint32_t log_arch_common_timestamp(void)
{
	return (uint32_t)time(NULL);
}

