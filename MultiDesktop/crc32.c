#include <windows.h>
#include "crc32.h"

#define CRCPOLY_BE 0x04c11db7
#define CRC_BE_BITS 8

static unsigned long *crc32_table;

unsigned long crc32(unsigned long crc, unsigned char const *p, size_t len)
{
	while (len--)
		crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *p++];
	
	return crc;
}

int crc32_init(void)
{
	unsigned i, j;
	unsigned long crc = 0x80000000;

	crc32_table = HeapAlloc(GetProcessHeap(), 0, (1 << CRC_BE_BITS) * sizeof(unsigned long));
	if(!crc32_table)
		return -1;

	crc32_table[0] = 0;

	for (i = 1; i < 1 << CRC_BE_BITS; i <<= 1) {
		crc = (crc << 1) ^ ((crc & 0x80000000) ? CRCPOLY_BE : 0);
		for (j = 0; j < i; j++)
			crc32_table[i + j] = crc ^ crc32_table[j];
	}
	return 0;
}

void crc32_cleanup(void)
{
	if (crc32_table)
		HeapFree(GetProcessHeap(), 0, crc32_table);
	crc32_table = NULL;
}
