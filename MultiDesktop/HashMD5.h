#ifndef __HASH_MD5_H__
#define __HASH_MD5_H__

struct md5_context 
{
  DWORD total[2];
  DWORD state[4];
  BYTE buffer[64];
};
void md5_starts( struct md5_context *ctx );
void md5_update( struct md5_context *ctx, BYTE *input, DWORD length );
void md5_finish( struct md5_context *ctx, DWORD digest[4] );

#endif //__HASH_MD5_H__

