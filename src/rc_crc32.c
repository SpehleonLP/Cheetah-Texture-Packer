#include "rc_crc32.h"

uint32_t * rc_crc32_table()
{
	static uint32_t table[256];
    static int have_table = 0;
	uint32_t rem;

    /* This check is not thread safe; there is no mutex. */
    if(have_table == 0)
    {
        /* Calculate CRC table. */
        for(int i = 0; i < 256; i++)
        {
            rem = i;  /* remainder from polynomial division */
            for(int j = 0; j < 8; j++)
            {
                if(rem & 1)
                {
                    rem >>= 1;
                    rem ^= 0xedb88320;
                }
                else
                {
                    rem >>= 1;
                }
            }
            table[i] = rem;
        }
        have_table = 1;
    }

	return table;
}


uint32_t rc_crc32(uint32_t crc, const uint8_t *buf, uint64_t len)
{
	uint32_t * table = rc_crc32_table();
    uint32_t octet;
    const uint8_t *p, *q;

    crc = ~crc;
    q = buf + len;
    for(p = buf; p < q; p++)
    {
        octet = *p;  /* Cast to unsigned octet. */
        crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}
