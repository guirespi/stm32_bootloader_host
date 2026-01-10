#include <ezm_net_utils.h>

int is_little_endian(void)
{
    const uint16_t test = 0x0001;
    return *((const uint8_t *)&test);
}

uint16_t ezm_htons_u16(uint16_t x)
{
    if (is_little_endian()) {
        return (uint16_t)((x << 8) | (x >> 8));
    }
    return x;
}