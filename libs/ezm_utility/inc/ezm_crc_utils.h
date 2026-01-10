#ifndef EZM_CRC_UTILS_H_
#define EZM_CRC_UTILS_H_

#include <stdint.h>

uint16_t ezm_crc16_ccitt(const uint8_t *data, uint32_t length);
uint32_t ezm_crc32_ieee_seed(uint32_t crc, const uint8_t *data, uint32_t length);

#endif // EZM_CRC_UTILS_H_