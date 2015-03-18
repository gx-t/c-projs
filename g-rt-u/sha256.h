#ifndef __SHA_256_H__
#define __SHA_256_H__


struct SHA256
{
  int (*FileSha256)(unsigned char* digest, const char* file, int (*callback)(void*, long long), void* ctx);
};

extern struct SHA256 g_sha256;

#endif //__SHA_256_H__

