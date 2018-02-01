#include <windows.h>//for memcpy
#include "sha256.h"

void sha256_starts( sha256_context *ctx )
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;
	
	ctx->state[0] = 0x6A09E667;
	ctx->state[1] = 0xBB67AE85;
	ctx->state[2] = 0x3C6EF372;
	ctx->state[3] = 0xA54FF53A;
	ctx->state[4] = 0x510E527F;
	ctx->state[5] = 0x9B05688C;
	ctx->state[6] = 0x1F83D9AB;
	ctx->state[7] = 0x5BE0CD19;
}

#define	Ch(x, y, z)	((z) ^ ((x) & ((y) ^ (z))))
#define	Maj(x, y, z)	(((x) & (y)) ^ ((z) & ((x) ^ (y))))
#define	Rot32(x, s)	(((x) >> s) | ((x) << (32 - s)))
#define	SIGMA0(x)	(Rot32(x, 2) ^ Rot32(x, 13) ^ Rot32(x, 22))
#define	SIGMA1(x)	(Rot32(x, 6) ^ Rot32(x, 11) ^ Rot32(x, 25))
#define	sigma0(x)	(Rot32(x, 7) ^ Rot32(x, 18) ^ ((x) >> 3))
#define	sigma1(x)	(Rot32(x, 17) ^ Rot32(x, 19) ^ ((x) >> 10))

static void sha256_process( sha256_context *ctx, uint8 data[64] )
{
	static const uint32 SHA256_K[64] =
	{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
     
	uint32 a, b, c, d, e, f, g, h, t, T1, T2, W[64], *H = ctx->state;
  uint8* cp = data;
    
  for (t = 0; t < 16; t++, cp += 4)
  	W[t] = (cp[0] << 24) | (cp[1] << 16) | (cp[2] << 8) | cp[3];
    
  for (t = 16; t < 64; t++)
  	W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];

	a = H[0]; b = H[1]; c = H[2]; d = H[3];
	e = H[4]; f = H[5]; g = H[6]; h = H[7];
		
	for (t = 0; t < 64; t++)
	{
		T1 = h + SIGMA1(e) + Ch(e, f, g) + SHA256_K[t] + W[t];
		T2 = SIGMA0(a) + Maj(a, b, c);
		h = g; g = f; f = e; e = d + T1;
		d = c; c = b; b = a; a = T1 + T2;
	}
	H[0] += a; H[1] += b; H[2] += c; H[3] += d;
	H[4] += e; H[5] += f; H[6] += g; H[7] += h;
}

void sha256_update( sha256_context *ctx, uint8 *input, uint32 length )
{
	uint32 left, fill;

	if( ! length ) return;

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += length;
	ctx->total[0] &= 0xFFFFFFFF;

	if( ctx->total[0] < length )
		ctx->total[1]++;

	if( left && length >= fill )
	{
		memcpy( (void *) (ctx->buffer + left), (void *) input, fill );
		sha256_process( ctx, ctx->buffer );
		length -= fill;
		input  += fill;
		left = 0;
	}

	while( length >= 64 )
	{
		sha256_process( ctx, input );
		length -= 64;
		input  += 64;
	}

	if( length )
		memcpy( (void *) (ctx->buffer + left), (void *) input, length );
}

static uint8 sha256_padding[64] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void sha256_finish( sha256_context *ctx, uint8 digest[32] )
{
	uint32 last, padn, high, low, t, *p;
	uint8 msglen[8], *cp;

	high = ( ctx->total[0] >> 29 ) | ( ctx->total[1] <<  3 );
	low  = ( ctx->total[0] <<  3 );

	msglen[0] = (uint8)(high >> 24);
	msglen[1] = (uint8)(high >> 16);
	msglen[2] = (uint8)(high >> 8);
	msglen[3] = (uint8)high;
	msglen[4] = (uint8)(low >> 24);
	msglen[5] = (uint8)(low >> 16);
	msglen[6] = (uint8)(low >> 8);
	msglen[7] = (uint8)low;

	last = ctx->total[0] & 0x3F;
	padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

	sha256_update( ctx, sha256_padding, padn );
	sha256_update( ctx, msglen, 8 );

	cp = digest;
	p = ctx->state;
	for(t = 0; t < 8; t ++, p ++, cp += 4)
	{
		cp[0] = (uint8)(*p >> 24);
		cp[1] = (uint8)(*p >> 16);
		cp[2] = (uint8)(*p >> 8);
		cp[3] = (uint8)*p;
	}
}
