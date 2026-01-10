#include "ezm_log_arch_common.h"

#include "si91x_device.h"
#include "clock_update.h"
#include "core_cm4.h"
#include "cmsis_os2.h"

uint32_t ezm_log_arch_common_timestamp(void)
{
	return osKernelGetTickCount();
}

