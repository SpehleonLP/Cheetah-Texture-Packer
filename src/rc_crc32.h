#ifndef RC_CRC32_H
#define RC_CRC32_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

uint32_t * rc_crc32_table();
uint32_t rc_crc32(uint32_t crc, const uint8_t *buf, uint64_t len);

#ifdef __cplusplus
}
#endif

#endif // RC_CRC32_H
