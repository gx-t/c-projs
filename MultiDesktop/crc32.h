#ifndef __CRC32_H__
#define __CRC32_H__

int crc32_init(void);
void crc32_cleanup(void);
unsigned long crc32(unsigned long crc, unsigned char const *p, size_t len);



#endif //__CRC32_H__