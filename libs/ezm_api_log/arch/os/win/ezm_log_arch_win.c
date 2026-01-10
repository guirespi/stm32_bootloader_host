#include <stdint.h>
#include <windows.h>
#include <ezm_log_arch_common.h>

uint32_t ezm_log_arch_common_timestamp(void)
{
    return (uint32_t)GetTickCount();
}

