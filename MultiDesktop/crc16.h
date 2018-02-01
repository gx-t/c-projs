#ifndef __CRC_16_H__
#define __CRC_16_H__


#define CRC_INIT 0xFFFF
unsigned short crcsum(const unsigned char*, unsigned long, unsigned short crc);

#endif //__CRC16_H__