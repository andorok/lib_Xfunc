// crc32.cpp


#include <stdio.h>

static unsigned int crc32_table[256];

int gen_crc32_table()
{
    unsigned int i, j;
    unsigned int c;

    for (i = 0; i < 256; i++)
    {
        for (c = i << 24, j = 8; j > 0; --j)
	        c = c & 0x80000000 ? (c << 1) ^ 0x04C11DB7 : (c << 1);
        crc32_table[i] = c;
    }

    return 0;
}

unsigned int get_crc32(unsigned char* buf, int len, unsigned int init)
{
    unsigned int crc = init;
    while (len--)
    {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
        buf++;
    }
    return crc;
}
